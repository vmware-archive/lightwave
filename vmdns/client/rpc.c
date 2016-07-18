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
 * Module   : binding.c
 *
 * Abstract :
 *
 *            VMware dns Service
 *
 *            system RPC functions
 *
 */

#include "includes.h"

DWORD
VmDnsRpcStringBindingCompose(
    PCSTR pszProtSeq,
    PCSTR pszNetworkAddr,
    PCSTR pszEndPoint,
    PSTR* ppszStringBinding
)
{
    DWORD dwError = ERROR_SUCCESS;

    rpc_string_binding_compose(
        NULL,
        (PBYTE)pszProtSeq,
        (PBYTE)pszNetworkAddr,
        (PBYTE)pszEndPoint,
        NULL,
        (PBYTE*)ppszStringBinding,
        &dwError);

    return dwError;
}

DWORD
VmDnsRpcBindingFromStringBinding(
  PCSTR pszStringBinding,
  handle_t* pBinding
)
{
    DWORD dwError = ERROR_SUCCESS;

    rpc_binding_from_string_binding(
        (PBYTE)pszStringBinding,
        pBinding,
        &dwError);

    return dwError;
}

DWORD
VmDnsRpcBindingSetAuthInfo(
  handle_t pBinding,
  unsigned long ulAuthnLevel,
  unsigned long ulAuthnSvc,
  unsigned long ulAuthzService
)
{
    DWORD dwError = ERROR_SUCCESS;

    rpc_binding_set_auth_info(
        pBinding,
        NULL,         /* Server Principal Name */
        ulAuthnLevel,
        ulAuthnSvc,
        NULL,         /* Auth Identity */
        ulAuthzService,
        &dwError);

    return dwError;
}

DWORD
VmDnsRpcFreeString(
    PSTR* ppszString
)
{
    DWORD dwError = ERROR_SUCCESS;

    rpc_string_free((PBYTE*)ppszString, &dwError);

    return dwError;
}

DWORD
VmDnsRpcFreeBinding(
    handle_t* pBinding
)
{
    DWORD dwError = ERROR_SUCCESS;

    rpc_binding_free(pBinding, &dwError);

    return dwError;
}

VOID
VmDnsRpcClientFreeMemory(
PVOID pMemory
)
{
    if (pMemory)
    {
        DWORD rpcStatus = ERROR_SUCCESS;
        rpc_sm_client_free(pMemory, &rpcStatus);
    }
}

VOID
VmDnsRpcClientFreeStringArrayA(
PSTR*  ppszStrArray,
DWORD  dwCount
)
{
    DWORD iStr = 0;

    for (; iStr < dwCount; iStr++)
    {
        VmDnsRpcClientFreeStringA(ppszStrArray[iStr]);
    }
    VmDnsRpcClientFreeMemory(ppszStrArray);
}

VOID
VmDnsRpcClientFreeStringA(
PSTR pszStr
)
{
    if (pszStr)
    {
        VmDnsRpcClientFreeMemory(pszStr);
    }
}

DWORD
VmDnsAllocateFromRpcRecordArray(
    PVMDNS_RECORD_ARRAY pRecordArray,
    PVMDNS_RECORD_ARRAY *ppRecordArray
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_RECORD_ARRAY pRecordArrayTemp = NULL;

    dwError = VmDnsAllocateMemory(sizeof(VMDNS_RECORD_ARRAY),
                                     (PVOID*)&pRecordArrayTemp);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateMemory(sizeof(VMDNS_RECORD)*pRecordArray->dwCount,
                                     (PVOID*)&pRecordArrayTemp->Records);
    BAIL_ON_VMDNS_ERROR(dwError);

    for (; idx < pRecordArray->dwCount; ++idx)
    {
        dwError = VmDnsCopyRecord(&pRecordArray->Records[idx],
                                    &pRecordArrayTemp->Records[idx]);
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    pRecordArrayTemp->dwCount = pRecordArray->dwCount;

    *ppRecordArray = pRecordArrayTemp;

cleanup:
    return dwError;
error:

    VMDNS_FREE_RECORD_ARRAY(pRecordArrayTemp);
    if (ppRecordArray)
    {
        *ppRecordArray = NULL;
    }
    goto cleanup;
}

VOID
VmDnsRpcClientFreeRpcRecordArray(
    PVMDNS_RECORD_ARRAY pRecordArray
    )
{
    DWORD idx = 0;
    if (pRecordArray)
    {

        if (pRecordArray->Records)
        {
            for (; idx < pRecordArray->dwCount; ++idx)
            {
                VmDnsRpcClearRecord(&pRecordArray->Records[idx]);
            }
            VmDnsRpcClientFreeMemory(pRecordArray->Records);
        }
        VmDnsRpcClientFreeMemory(pRecordArray);
    }
}

DWORD
VmDnsAllocateFromRpcZoneInfoArray(
    PVMDNS_ZONE_INFO_ARRAY pZoneInfoArray,
    PVMDNS_ZONE_INFO_ARRAY *ppZoneInfoArray
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;

    PVMDNS_ZONE_INFO_ARRAY pZoneInfoArrayTemp = NULL;
    dwError = VmDnsAllocateMemory(sizeof(VMDNS_ZONE_INFO_ARRAY),
                                     (PVOID*)&pZoneInfoArrayTemp);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateMemory(sizeof(VMDNS_ZONE_INFO)*pZoneInfoArray->dwCount,
                                     (PVOID*)&pZoneInfoArrayTemp->ZoneInfos);
    BAIL_ON_VMDNS_ERROR(dwError);

    for (; idx < pZoneInfoArray->dwCount; ++idx)
    {
        dwError = VmDnsCopyFromZoneInfo(&pZoneInfoArray->ZoneInfos[idx],
                                        &pZoneInfoArrayTemp->ZoneInfos[idx]);
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    pZoneInfoArrayTemp->dwCount = pZoneInfoArray->dwCount;

    *ppZoneInfoArray = pZoneInfoArrayTemp;

cleanup:
    return dwError;
error:

    if (pZoneInfoArrayTemp)
    {
        VmDnsFreeZoneInfoArray(pZoneInfoArrayTemp);
    }
    if (*ppZoneInfoArray)
    {
        *ppZoneInfoArray = NULL;
    }
    goto cleanup;
}

VOID
VmDnsRpcClientFreeZoneInfo(
    PVMDNS_ZONE_INFO pZoneInfo
    )
{
    if (pZoneInfo)
    {
        VmDnsRpcClientFreeMemory(pZoneInfo->pszName);
        VmDnsRpcClientFreeMemory(pZoneInfo->pszPrimaryDnsSrvName);
        VmDnsRpcClientFreeMemory(pZoneInfo->pszRName);
    }
}

VOID
VmDnsRpcClientFreeZoneInfoArray(
    PVMDNS_ZONE_INFO_ARRAY pZoneInfoArray
    )
{
    DWORD idx = 0;
    if (pZoneInfoArray)
    {
        for (; idx < pZoneInfoArray->dwCount; ++idx)
        {
            VmDnsRpcClientFreeZoneInfo(&pZoneInfoArray->ZoneInfos[idx]);
        }
        VmDnsRpcClientFreeMemory(pZoneInfoArray);
    }
}

