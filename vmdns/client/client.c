
/*
* Copyright (C) 2011 VMware, Inc. All rights reserved.
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
*
* Authors  : Sriram Nambakam (snambakam@vmware.com)
*
*/

#include "includes.h"
#include "vmdns_h.h"

static
VOID
VmDnsFreeForwarders(
    PVMDNS_FORWARDERS       pForwarder
    );

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


// Public client interfaces for RPC calls

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
    handle_t hBinding = NULL;
    PVMDNS_SERVER_CONTEXT pServerContext = NULL;
    BOOLEAN bBindingAuth = FALSE;
    CHAR szRpcPort[] = VMDNS_RPC_TCP_END_POINT;

    dwError = VmDnsAllocateMemory(
                            sizeof(*pServerContext),
                            (PVOID)&pServerContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    if (!IsNullOrEmptyString(pszUserName) &&
        !IsNullOrEmptyString(pszPassword))
    {
        bBindingAuth = TRUE;
    }

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

    *ppServerContext = pServerContext;
    pServerContext = NULL;

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
    PSTR**                  pppszForwarders,
    PDWORD                  pdwCount
    )
{
    DWORD dwError = 0;
    PVMDNS_FORWARDERS pDnsForwarders = NULL;
    PSTR* ppszForwarders = NULL;
    DWORD dwCount = 0;

    dwError = VmDnsValidateContext(pServerContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    BAIL_ON_VMDNS_INVALID_POINTER(pppszForwarders, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pdwCount, dwError);

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

        dwError = VmDnsAllocateMemory(
                                pDnsForwarders->dwCount * sizeof(PSTR*),
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

    *pppszForwarders = ppszForwarders;
    *pdwCount = dwCount;

cleanup:

    if (pDnsForwarders)
    {
        VmDnsFreeForwarders(pDnsForwarders);
    }

    return dwError;

error:

    if (pppszForwarders)
    {
        *pppszForwarders = NULL;
    }

    if (pdwCount)
    {
        *pdwCount = 0;
    }

    if (ppszForwarders)
    {
        VmDnsFreeStringCountedArrayA(ppszForwarders, dwCount);
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
                            ppZoneInfoArray
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
                        &pRecordArray
                        );
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        dwError = VmDnsRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppRecordArray = pRecordArray;

cleanup:
    return dwError;

error:
    VMDNS_FREE_RECORD_ARRAY(pRecordArray);
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
                        &pRecordArray
                        );
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        dwError = VmDnsRpcGetErrorCode(THIS_CATCH);
    }
    DCETHREAD_ENDTRY;
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppRecordArray = pRecordArray;

cleanup:
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
        VmDnsRpcFreeStringA(pZoneInfo->pszName);
        VmDnsRpcFreeStringA(pZoneInfo->pszPrimaryDnsSrvName);
        VmDnsRpcFreeStringA(pZoneInfo->pszRName);
        VmDnsRpcFreeMemory(pZoneInfo);
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
        VmDnsRpcFreeMemory(pZoneInfoArray);
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
        for (; idx < pRecordArray->dwCount; ++idx)
        {
            VmDnsRpcClearRecord(&pRecordArray->Records[idx]);
        }
        VmDnsRpcFreeMemory(pRecordArray);
    }
}

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
            VmDnsRpcFreeString(&pForwarder->ppszName[idx]);
        }
        VmDnsRpcFreeMemory(pForwarder);
    }
}
