/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

/*
*
* Module   : client.c
*
* Abstract :
*
*            VMware dns Service
*
*            Client API
*
*            Core API
*/

#include "includes.h"
#include "vmdns_h.h"

static
DWORD
VmDnsRpcGetErrorCode(
dcethread_exc* pDceException
)
{
    DWORD dwError = 0;

    dwError = dcethread_exc_getstatus(pDceException);

#ifndef _WIN32
    dwError = LwNtStatusToWin32Error(LwRpcStatusToNtStatus(dwError));
#endif

    return dwError;
}

static
DWORD
VmDnsValidateContext(PVMDNS_SERVER_CONTEXT pServerContext)
{
    DWORD dwError = 0;
    if (!pServerContext || !pServerContext->hBinding)
    {
        dwError = ERROR_INVALID_PARAMETER;
    }

    return dwError;
}

static
VOID
VmDnsRpcFreeForwarders(
    PVMDNS_FORWARDERS       pForwarder
    );

static
DWORD
VmDnsCheckRecord(
    PVMDNS_RECORD pRecord
    );

// Public client interfaces for RPC calls

VMDNS_API
DWORD
VmDnsOpenServerWithTimeOutA(
    PCSTR pszNetworkAddress,
    PCSTR pszUserName,
    PCSTR pszDomain,
    PCSTR pszPassword,
    DWORD dwFlags,
    PVOID pReserved,
    DWORD dwTimeOut,
    PVMDNS_SERVER_CONTEXT *ppServerContext
    )
{
    DWORD dwError = 0;
    handle_t hBinding = NULL;
    PVMDNS_SERVER_CONTEXT pServerContext = NULL;
    CHAR szRpcPort[] = VMDNS_RPC_TCP_END_POINT;

    dwError = VmDnsAllocateMemory(
                            sizeof(*pServerContext),
                            (PVOID)&pServerContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCreateBindingHandleA(
                            pszNetworkAddress,
                            szRpcPort,
                            pszUserName,
                            pszDomain,
                            pszPassword,
                            &hBinding);
    BAIL_ON_VMDNS_ERROR(dwError);

    pServerContext->hBinding = hBinding;
    hBinding = NULL;

    rpc_mgmt_set_com_timeout(pServerContext->hBinding, dwTimeOut, &dwError);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppServerContext = pServerContext;
    pServerContext = NULL;

cleanup:

    return dwError;

error:

    VmDnsCloseServer(pServerContext);
    goto cleanup;
}

VMDNS_API
DWORD
VmDnsOpenServerA(
    PCSTR pszNetworkAddress,
    PCSTR pszUserName,
    PCSTR pszDomain,
    PCSTR pszPassword,
    DWORD dwFlags,
    PVOID pReserved,
    PVMDNS_SERVER_CONTEXT *ppServerContext
    )
{
    DWORD dwError = 0;
    PVMDNS_SERVER_CONTEXT pServerContext = NULL;

    dwError = VmDnsOpenServerWithTimeOutA(
                                  pszNetworkAddress,
                                  pszUserName,
                                  pszDomain,
                                  pszPassword,
                                  dwFlags,
                                  pReserved,
                                  0,
                                  &pServerContext
                                  );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppServerContext = pServerContext;

cleanup:

    return dwError;

error:

    VmDnsCloseServer(pServerContext);
    goto cleanup;
}


VMDNS_API
VOID
VmDnsCloseServer(PVMDNS_SERVER_CONTEXT pServerContext)
{
    if (pServerContext->hBinding)
    {
        DWORD dwError = 0;
        rpc_binding_free(&pServerContext->hBinding, &dwError);
        pServerContext->hBinding = NULL;
    }

    VMDNS_SAFE_FREE_MEMORY(pServerContext);
}

VMDNS_API
DWORD
VmDnsGetForwardersA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PVMDNS_FORWARDERS*      ppForwarders
    )
{
    DWORD dwError = 0;
    PVMDNS_FORWARDERS pDnsForwarders = NULL;
    PVMDNS_FORWARDERS pDnsForwardersOutput = NULL;
    PSTR* ppszForwarders = NULL;
    DWORD dwCount = 0;

    dwError = VmDnsValidateContext(pServerContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    BAIL_ON_VMDNS_INVALID_POINTER(ppForwarders, dwError);

    DCETHREAD_TRY
    {
        dwError = VmDnsRpcGetForwarders(
                            pServerContext->hBinding,
                            &pDnsForwarders);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        dwError = VmDnsRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMDNS_ERROR(dwError);

    if (pDnsForwarders)
    {
        DWORD i = 0;
        PSTR szTemp = NULL;

        if (pDnsForwarders->dwCount > 0)
        {
            dwError = VmDnsAllocateMemory(
                                    pDnsForwarders->dwCount * sizeof(PSTR),
                                    (PVOID*)&ppszForwarders);
            BAIL_ON_VMDNS_ERROR(dwError);

            for (i = 0; i < pDnsForwarders->dwCount; ++i)
            {
                dwError = VmDnsAllocateStringA(
                                    pDnsForwarders->ppszName[i],
                                    &szTemp
                                    );
                BAIL_ON_VMDNS_ERROR(dwError);

                ppszForwarders[i] = szTemp;
                ++dwCount;
            }
        }

        dwError = VmDnsAllocateMemory(
                                sizeof(VMDNS_FORWARDERS),
                                (PVOID*)&pDnsForwardersOutput);
        BAIL_ON_VMDNS_ERROR(dwError);

        pDnsForwardersOutput->dwCount = pDnsForwarders->dwCount;
        pDnsForwardersOutput->ppszName = ppszForwarders;
    }

    *ppForwarders = pDnsForwardersOutput;

cleanup:

    if (pDnsForwarders)
    {
        VmDnsRpcFreeForwarders(pDnsForwarders);
    }

    return dwError;

error:

    if (ppszForwarders)
    {
        VmDnsFreeStringCountedArrayA(ppszForwarders, dwCount);
    }

    if (pDnsForwardersOutput)
    {
        VMDNS_SAFE_FREE_MEMORY(pDnsForwardersOutput);
    }

    goto cleanup;

}

VMDNS_API
DWORD
VmDnsAddForwarderA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PSTR                    pszForwarders
    )
{
    DWORD dwError = 0;
    dwError = VmDnsValidateContext(pServerContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    BAIL_ON_VMDNS_INVALID_POINTER(pszForwarders, dwError);
    DCETHREAD_TRY
    {
        dwError = VmDnsRpcAddForwarder(
                            pServerContext->hBinding,
                            pszForwarders);
    }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        dwError = VmDnsRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;

}

VMDNS_API
DWORD
VmDnsDeleteForwarderA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PSTR                    pszForwarder
    )
{
    DWORD dwError = 0;

    dwError = VmDnsValidateContext(pServerContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    BAIL_ON_VMDNS_INVALID_POINTER(pszForwarder, dwError);

    DCETHREAD_TRY
    {
        dwError = VmDnsRpcDeleteForwarder(
                        pServerContext->hBinding,
                        pszForwarder);
    }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        dwError = VmDnsRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;

}

VMDNS_API
DWORD
VmDnsInitializeA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PVMDNS_INIT_INFO        pInitInfo
    )
{
    DWORD dwError = 0;

    if (!pServerContext ||
        !pServerContext->hBinding ||
        !pInitInfo)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    DCETHREAD_TRY
    {
        dwError = VmDnsRpcInitialize(
                            pServerContext->hBinding,
                            pInitInfo
                            );
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        dwError = VmDnsRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

VMDNS_API
DWORD
VmDnsUninitializeA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PVMDNS_INIT_INFO        pInitInfo
    )
{
    DWORD dwError = 0;

    if (!pServerContext ||
        !pServerContext->hBinding ||
        !pInitInfo)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    DCETHREAD_TRY
    {
        dwError = VmDnsRpcUninitialize(
                            pServerContext->hBinding,
                            pInitInfo
                            );
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        dwError = VmDnsRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

VMDNS_API
DWORD
VmDnsCreateZoneA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PVMDNS_ZONE_INFO        pZoneInfo
    )
{
    DWORD dwError = 0;

    if (!pServerContext ||
        !pServerContext->hBinding ||
        !pZoneInfo)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    DCETHREAD_TRY
    {
        dwError = VmDnsRpcCreateZone(
                            pServerContext->hBinding,
                            pZoneInfo
                            );
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        dwError = VmDnsRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;

}

VMDNS_API
DWORD
VmDnsUpdateZoneA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PVMDNS_ZONE_INFO        pZoneInfo
    )
{
    DWORD dwError = 0;

    if (!pServerContext ||
        !pServerContext->hBinding ||
        !pZoneInfo)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    DCETHREAD_TRY
    {
        dwError = VmDnsRpcUpdateZone(
                            pServerContext->hBinding,
                            pZoneInfo
                            );
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        dwError = VmDnsRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;

}

VMDNS_API
DWORD
VmDnsListZoneA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PVMDNS_ZONE_INFO_ARRAY *ppZoneInfoArray
    )
{
    DWORD dwError = 0;
    PVMDNS_ZONE_INFO_ARRAY pZoneInfoArray = NULL;
    PVMDNS_ZONE_INFO_ARRAY pRpcZoneInfoArray = NULL;

    if (!pServerContext ||
        !pServerContext->hBinding ||
        !ppZoneInfoArray)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    DCETHREAD_TRY
    {
        dwError = VmDnsRpcListZones(
                            pServerContext->hBinding,
                            &pRpcZoneInfoArray
                            );
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        dwError = VmDnsRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateFromRpcZoneInfoArray(
                                    pRpcZoneInfoArray,
                                    &pZoneInfoArray
                                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppZoneInfoArray = pZoneInfoArray;

cleanup:

    if (pRpcZoneInfoArray)
    {
        VmDnsRpcClientFreeZoneInfoArray(pRpcZoneInfoArray);
    }
    return dwError;

error:

    if (pZoneInfoArray)
    {
        VmDnsFreeZoneInfoArray(pZoneInfoArray);
    }
    if (ppZoneInfoArray)
    {
        *ppZoneInfoArray = NULL;
    }
    goto cleanup;

}

VMDNS_API
DWORD
VmDnsDeleteZoneA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PSTR                    pszZone
    )
{
    DWORD dwError = 0;

    if (!pServerContext ||
        !pServerContext->hBinding ||
        !pszZone)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    DCETHREAD_TRY
    {
        dwError = VmDnsRpcDeleteZone(
                            pServerContext->hBinding,
                            pszZone
                            );
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        dwError = VmDnsRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;

}

VMDNS_API
DWORD
VmDnsAddRecordA(
    PVMDNS_SERVER_CONTEXT pServerContext,
    PSTR pszZone,
    PVMDNS_RECORD pRecord
    )
{
    DWORD dwError = 0;

    if (!pServerContext ||
        !pServerContext->hBinding ||
        !pRecord)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCheckRecord(pRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

    DCETHREAD_TRY
    {
        dwError = VmDnsRpcAddRecord(
                                pServerContext->hBinding,
                                pszZone,
                                pRecord
                                );
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        dwError = VmDnsRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

VMDNS_API
DWORD
VmDnsDeleteRecordA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PSTR                    pszZone,
    PVMDNS_RECORD           pRecord
    )
{
    DWORD dwError = 0;

    if (!pServerContext ||
        !pServerContext->hBinding ||
        !pszZone ||
        !pRecord)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    DCETHREAD_TRY
    {
        dwError = VmDnsRpcDeleteRecord(
                            pServerContext->hBinding,
                            pszZone,
                            pRecord
                            );
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        dwError = VmDnsRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

VMDNS_API
DWORD
VmDnsQueryRecordsA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PSTR                    pszZone,
    PSTR                    pszName,
    VMDNS_RR_TYPE           dwType,
    DWORD                   dwOptions,
    PVMDNS_RECORD_ARRAY *   ppRecordArray
    )
{
    DWORD dwError = 0;
    PVMDNS_RECORD_ARRAY pRecordArray = NULL;
    PVMDNS_RECORD_ARRAY pRpcRecordArray = NULL;

    if (!pServerContext ||
        !pServerContext->hBinding ||
        !pszZone ||
        !pszName ||
        !ppRecordArray)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    DCETHREAD_TRY
    {
        dwError = VmDnsRpcQueryRecords(
                        pServerContext->hBinding,
                        pszZone,
                        pszName,
                        dwType,
                        dwOptions,
                        &pRpcRecordArray
                        );
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        dwError = VmDnsRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateFromRpcRecordArray(
                                pRpcRecordArray,
                                &pRecordArray
                                );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppRecordArray = pRecordArray;

cleanup:

    if (pRpcRecordArray)
    {
        VmDnsRpcClientFreeRpcRecordArray(pRpcRecordArray);
    }
    return dwError;

error:
    VMDNS_FREE_RECORD_ARRAY(pRecordArray);

    if (ppRecordArray)
    {
        *ppRecordArray = NULL;
    }
    goto cleanup;
}

VMDNS_API
DWORD
VmDnsListRecordsA(
    PVMDNS_SERVER_CONTEXT   pServerContext,
    PSTR                    pszZone,
    PVMDNS_RECORD_ARRAY*    ppRecordArray
    )
{
    DWORD dwError = 0;
    PVMDNS_RECORD_ARRAY pRecordArray = NULL;
    PVMDNS_RECORD_ARRAY pRpcRecordArray = NULL;

    if (!pServerContext ||
        !pServerContext->hBinding ||
        !ppRecordArray)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    DCETHREAD_TRY
    {
        dwError = VmDnsRpcListRecords(
                        pServerContext->hBinding,
                        pszZone,
                        &pRpcRecordArray
                        );
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        dwError = VmDnsRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateFromRpcRecordArray(
                                pRpcRecordArray,
                                &pRecordArray
                                );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppRecordArray = pRecordArray;

cleanup:

    if (pRpcRecordArray)
    {
        VmDnsRpcClientFreeRpcRecordArray(pRpcRecordArray);
    }
    return dwError;

error:
    VMDNS_FREE_RECORD_ARRAY(pRecordArray);
    goto cleanup;
}

VMDNS_API
VOID
VmDnsFreeZoneInfo(
    PVMDNS_ZONE_INFO        pZoneInfo
    )
{
    if (pZoneInfo)
    {
        VmDnsFreeStringA(pZoneInfo->pszName);
        VmDnsFreeStringA(pZoneInfo->pszPrimaryDnsSrvName);
        VmDnsFreeStringA(pZoneInfo->pszRName);
    }
}

VMDNS_API
VOID
VmDnsFreeZoneInfoArray(
    PVMDNS_ZONE_INFO_ARRAY  pZoneInfoArray
    )
{
    DWORD idx = 0;
    if (pZoneInfoArray)
    {
        for (; idx < pZoneInfoArray->dwCount; ++idx)
        {
            VmDnsFreeZoneInfo(&pZoneInfoArray->ZoneInfos[idx]);
        }
        VmDnsFreeMemory(pZoneInfoArray);
    }
}

VMDNS_API
VOID
VmDnsFreeRecord(
    PVMDNS_RECORD           pRecord
    )
{
    if (pRecord)
    {
        VmDnsRpcClearRecord(pRecord);
        VmDnsRpcFreeMemory(pRecord);
    }
}

VMDNS_API
VOID
VmDnsFreeRecordArray(
    PVMDNS_RECORD_ARRAY     pRecordArray
    )
{
    if (pRecordArray)
    {
        DWORD idx = 0;
        if (pRecordArray->Records)
        {
            for (; idx < pRecordArray->dwCount; ++idx)
            {
                VmDnsClearRecord(&pRecordArray->Records[idx]);
            }

            VmDnsFreeMemory(pRecordArray->Records);
        }
        VmDnsFreeMemory(pRecordArray);
    }
}

VMDNS_API
VOID
VmDnsFreeForwarders(
    PVMDNS_FORWARDERS       pForwarder
    )
{
    DWORD idx = 0;
    if (pForwarder)
    {
        for (; idx < pForwarder->dwCount; ++idx)
        {
            VmDnsFreeStringA(pForwarder->ppszName[idx]);
        }
        VmDnsFreeMemory(pForwarder);
    }
}

static
VOID
VmDnsRpcFreeForwarders(
    PVMDNS_FORWARDERS       pForwarder
    )
{
    DWORD idx = 0;
    if (pForwarder)
    {
        for (; idx < pForwarder->dwCount; ++idx)
        {
            VmDnsRpcFreeString(&pForwarder->ppszName[idx]);
        }
        VmDnsRpcFreeMemory(pForwarder);
    }
}

static
DWORD
VmDnsCheckRecord(
    PVMDNS_RECORD pRecord
    )
{
    DWORD dwError = 0;

    if (!pRecord)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    if (pRecord->dwType == VMDNS_RR_TYPE_A)
    {
        if (VmDnsCheckIfIPV4AddressA(pRecord->pszName))
        {
            dwError = ERROR_INVALID_PARAMETER;
        }
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    if (pRecord->dwType == VMDNS_RR_TYPE_AAAA)
    {
        if (VmDnsCheckIfIPV6AddressA(pRecord->pszName))
        {
            dwError = ERROR_INVALID_PARAMETER;
        }
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    if (pRecord->dwType == VMDNS_RR_TYPE_SRV)
    {
        if (VmDnsCheckIfIPV4AddressA(pRecord->pszName) ||
            VmDnsCheckIfIPV6AddressA(pRecord->pszName))
        {
            dwError = ERROR_INVALID_PARAMETER;
        }
        BAIL_ON_VMDNS_ERROR(dwError);
    }
cleanup:

    return dwError;

error:

    goto cleanup;
}

