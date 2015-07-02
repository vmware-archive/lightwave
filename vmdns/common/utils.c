/*
 * Copyright (C) 2011 VMware, Inc. All rights reserved.
 *
 * Module   : utils.c
 *
 * Abstract :
 *
 *            VMware dns Service
 *
 */

#include "includes.h"

VOID
VmDnsClearZoneInfo(
    PVMDNS_ZONE_INFO    pZoneInfo
    )
{
    if (pZoneInfo)
    {
        VMDNS_SAFE_FREE_STRINGA(pZoneInfo->pszName);
        VMDNS_SAFE_FREE_STRINGA(pZoneInfo->pszPrimaryDnsSrvName);
        VMDNS_SAFE_FREE_STRINGA(pZoneInfo->pszRName);
    }
}

VOID
VmDnsClearZoneInfoArray(
    PVMDNS_ZONE_INFO_ARRAY  pZoneInfoArray
    )
{
    DWORD idx = 0;
    if (pZoneInfoArray)
    {
        for (;idx < pZoneInfoArray->dwCount; ++idx)
        {
            VmDnsClearZoneInfo(&pZoneInfoArray->ZoneInfos[idx]);
        }
    }
}

DWORD
VmDnsRpcCopyRecordArray(
    PVMDNS_RECORD_ARRAY pRecordArray,
    PVMDNS_RECORD_ARRAY *ppRecordArray
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_RECORD_ARRAY pRecordArrayTemp = NULL;

    dwError = VmDnsRpcAllocateMemory(sizeof(VMDNS_RECORD_ARRAY),
                                     (PVOID*)&pRecordArrayTemp);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRpcAllocateMemory(sizeof(VMDNS_RECORD)*pRecordArray->dwCount,
                                     (PVOID*)&pRecordArrayTemp->Records);
    BAIL_ON_VMDNS_ERROR(dwError);

    for (; idx < pRecordArray->dwCount; ++idx)
    {
        dwError = VmDnsRpcCopyRecord(&pRecordArray->Records[idx],
                                    &pRecordArrayTemp->Records[idx]);
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    pRecordArrayTemp->dwCount = pRecordArray->dwCount;

    *ppRecordArray = pRecordArrayTemp;

cleanup:
    return dwError;
error:
    VmDnsRpcFreeRecordArray(pRecordArrayTemp);
    if (ppRecordArray)
    {
        *ppRecordArray = NULL;
    }
    goto cleanup;
}

VOID
VmDnsRpcFreeRecordArray(
    PVMDNS_RECORD_ARRAY pRecordArray
    )
{
    DWORD idx = 0;
    if (pRecordArray)
    {
        for (; idx < pRecordArray->dwCount; ++idx)
        {
            VmDnsRpcClearRecord(&pRecordArray->Records[idx]);
        }
        VmDnsRpcFreeMemory(pRecordArray);
    }
}

DWORD
VmDnsRpcCopyZoneInfo(
    PVMDNS_ZONE_INFO pZoneInfoSrc,
    PVMDNS_ZONE_INFO pZoneInfoDest
    )
{
    DWORD dwError = ERROR_SUCCESS;

    BAIL_ON_VMDNS_INVALID_POINTER(pZoneInfoSrc, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pZoneInfoDest, dwError);

    dwError = VmDnsRpcAllocateStringA(pZoneInfoSrc->pszName,
                                    &pZoneInfoDest->pszName);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRpcAllocateStringA(pZoneInfoSrc->pszPrimaryDnsSrvName,
                                    &pZoneInfoDest->pszPrimaryDnsSrvName);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRpcAllocateStringA(pZoneInfoSrc->pszRName,
                                    &pZoneInfoDest->pszRName);
    BAIL_ON_VMDNS_ERROR(dwError);

    pZoneInfoDest->dwFlags = pZoneInfoSrc->dwFlags;
    pZoneInfoDest->dwZoneType = pZoneInfoSrc->dwZoneType;
    pZoneInfoDest->expire = pZoneInfoSrc->expire;
    pZoneInfoDest->minimum = pZoneInfoSrc->minimum;
    pZoneInfoDest->refreshInterval = pZoneInfoSrc->refreshInterval;
    pZoneInfoDest->retryInterval = pZoneInfoSrc->retryInterval;
    pZoneInfoDest->serial = pZoneInfoSrc->serial;

cleanup:
    return dwError;
error:
    VmDnsRpcFreeMemory(pZoneInfoDest->pszName);
    VmDnsRpcFreeMemory(pZoneInfoDest->pszPrimaryDnsSrvName);
    VmDnsRpcFreeMemory(pZoneInfoDest->pszRName);
    VmDnsRpcFreeMemory(pZoneInfoDest);
    goto cleanup;
}

DWORD
VmDnsRpcCopyZoneInfoArray(
    PVMDNS_ZONE_INFO_ARRAY pZoneInfoArray,
    PVMDNS_ZONE_INFO_ARRAY *ppZoneInfoArray
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;

    PVMDNS_ZONE_INFO_ARRAY pZoneInfoArrayTemp = NULL;
    dwError = VmDnsRpcAllocateMemory(sizeof(VMDNS_ZONE_INFO_ARRAY),
                                     (PVOID*)&pZoneInfoArrayTemp);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRpcAllocateMemory(sizeof(VMDNS_ZONE_INFO)*pZoneInfoArray->dwCount,
                                     (PVOID*)&pZoneInfoArrayTemp->ZoneInfos);
    BAIL_ON_VMDNS_ERROR(dwError);

    for (; idx < pZoneInfoArray->dwCount; ++idx)
    {
        dwError = VmDnsRpcCopyZoneInfo(&pZoneInfoArray->ZoneInfos[idx],
                                        &pZoneInfoArrayTemp->ZoneInfos[idx]);
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    pZoneInfoArrayTemp->dwCount = pZoneInfoArray->dwCount;

    *ppZoneInfoArray = pZoneInfoArrayTemp;

cleanup:
    return dwError;
error:
    VmDnsRpcFreeZoneInfoArray(pZoneInfoArrayTemp);
    if (*ppZoneInfoArray)
    {
        *ppZoneInfoArray = NULL;
    }
    goto cleanup;
}

VOID
VmDnsRpcFreeZoneInfo(
    PVMDNS_ZONE_INFO pZoneInfo
    )
{
    if (pZoneInfo)
    {
        VmDnsRpcFreeMemory(pZoneInfo->pszName);
        VmDnsRpcFreeMemory(pZoneInfo->pszPrimaryDnsSrvName);
        VmDnsRpcFreeMemory(pZoneInfo->pszRName);
        VmDnsRpcFreeMemory(pZoneInfo);
    }
}

VOID
VmDnsRpcFreeZoneInfoArray(
    PVMDNS_ZONE_INFO_ARRAY pZoneInfoArray
    )
{
    DWORD idx = 0;
    if (pZoneInfoArray)
    {
        for (; idx < pZoneInfoArray->dwCount; ++idx)
        {
            VmDnsRpcFreeZoneInfo(&pZoneInfoArray->ZoneInfos[idx]);
        }
        VmDnsFreeMemory(pZoneInfoArray);
    }
}
