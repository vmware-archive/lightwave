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
 * Module   : rpcserv.c
 *
 * Abstract :
 *
 *            VMware dns Service
 *
 *            Server API
 *
 *            RPC Entry Points
 */

#include "includes.h"

UINT32
VmDnsRpcInitialize(
    handle_t            hBinding,
    PVMDNS_INIT_INFO    pInitInfo
    )
{
    DWORD dwError = ERROR_SUCCESS;

    dwError = VmDnsCheckAccess(hBinding, TRUE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSrvInitDomain(pInitInfo);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;
error:
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);
    goto cleanup;
}

UINT32
VmDnsRpcUninitialize(
    handle_t            hBinding,
    PVMDNS_INIT_INFO    pInitInfo
    )
{
    DWORD dwError = ERROR_SUCCESS;

    dwError = VmDnsCheckAccess(hBinding, TRUE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSrvCleanupDomain(pInitInfo);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;
error:
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);
    goto cleanup;
}

UINT32
VmDnsRpcCreateZone(
    handle_t            hBinding,
    PVMDNS_ZONE_INFO    pZoneInfo
    )
{
    DWORD dwError = ERROR_SUCCESS;

    dwError = VmDnsCheckAccess(hBinding, TRUE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSrvZoneCreate(pZoneInfo);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;
error:
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);
    goto cleanup;
}

UINT32
VmDnsRpcUpdateZone(
    handle_t            hBinding,
    PVMDNS_ZONE_INFO    pZoneInfo
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_ZONE_OBJECT pZoneObject = NULL;

    if (!pZoneInfo || IsNullOrEmptyString(pZoneInfo->pszName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCheckAccess(hBinding, TRUE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSrvFindZone(pZoneInfo->pszName, &pZoneObject);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSrvZoneUpdate(pZoneObject, pZoneInfo);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VmDnsZoneObjectRelease(pZoneObject);
    return dwError;
error:
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);
    goto cleanup;
}

UINT32
VmDnsRpcDeleteZone(
    handle_t            hBinding,
    PDNS_STRING         pszZone
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_ZONE_OBJECT pZoneObject = NULL;

    if (IsNullOrEmptyString(pszZone))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCheckAccess(hBinding, TRUE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSrvFindZone(pszZone, &pZoneObject);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSrvZoneDelete(pZoneObject);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VmDnsZoneObjectRelease(pZoneObject);
    return dwError;
error:
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);
    goto cleanup;
}

UINT32
VmDnsRpcAddRecord(
    handle_t            hBinding,
    PDNS_STRING         pszZone,
    PVMDNS_RECORD       pRecord
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_ZONE_OBJECT pZoneObject = NULL;

    if (IsNullOrEmptyString(pszZone) || !pRecord)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCheckAccess(hBinding, FALSE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSrvFindZone(pszZone, &pZoneObject);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSrvAddRecord(pZoneObject, pRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VmDnsZoneObjectRelease(pZoneObject);
    return dwError;
error:
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);
    goto cleanup;
}

UINT32
VmDnsRpcDeleteRecord(
    handle_t            hBinding,
    PDNS_STRING         pszZone,
    PVMDNS_RECORD       pRecord
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_ZONE_OBJECT pZoneObject = NULL;

    if (IsNullOrEmptyString(pszZone) || !pRecord)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCheckAccess(hBinding, FALSE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSrvFindZone(pszZone, &pZoneObject);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSrvDeleteRecord(pZoneObject, pRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VmDnsZoneObjectRelease(pZoneObject);
    return dwError;
error:
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);
    goto cleanup;
}

UINT32
VmDnsRpcQueryRecords(
    handle_t            hBinding,
    PDNS_STRING         pszZone,
    PDNS_STRING         pszName,
    VMDNS_RR_TYPE       dwType,
    DWORD               dwOptions,
    PVMDNS_RECORD_ARRAY *ppRecordArray
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_RECORD_LIST pRecordList = NULL;
    PVMDNS_RECORD_ARRAY pRecordArray = NULL;
    PVMDNS_ZONE_OBJECT pZoneObject = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(ppRecordArray, dwError);
    *ppRecordArray = NULL;

    if (IsNullOrEmptyString(pszZone) || IsNullOrEmptyString(pszName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCheckAccess(hBinding, FALSE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSrvFindZone(pszZone, &pZoneObject);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSrvQueryRecords(pZoneObject, pszName, dwType, 0, &pRecordList);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRpcCopyRecordArray(pRecordList, &pRecordArray);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppRecordArray = pRecordArray;
    pRecordArray = NULL;

cleanup:
    VmDnsZoneObjectRelease(pZoneObject);
    VmDnsRecordListRelease(pRecordList);
    return dwError;
error:
    if (pRecordArray)
    {
        VmDnsRpcFreeRecordArray(pRecordArray);
    }
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);
    goto cleanup;
}

UINT32
VmDnsRpcListZones(
    handle_t                hBinding,
    PVMDNS_ZONE_INFO_ARRAY *ppZoneArray
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_ZONE_INFO_ARRAY pZoneArrayTemp = NULL;
    PVMDNS_ZONE_INFO_ARRAY pZoneArray = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(ppZoneArray, dwError);
    *ppZoneArray = NULL;

    dwError = VmDnsCheckAccess(hBinding, FALSE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSrvListZones(&pZoneArrayTemp);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRpcCopyZoneInfoArray(pZoneArrayTemp, &pZoneArray);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppZoneArray = pZoneArray;
    pZoneArray = NULL;

cleanup:
    VMDNS_FREE_ZONE_INFO_ARRAY(pZoneArrayTemp);
    return dwError;
error:
    if (pZoneArray)
    {
        VmDnsRpcFreeZoneInfoArray(pZoneArray);
    }
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);
    goto cleanup;
}

UINT32
VmDnsRpcListRecords(
    handle_t            hBinding,
    PDNS_STRING         pszZone,
    PVMDNS_RECORD_ARRAY *ppRecordArray
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_RECORD_LIST pRecordList = NULL;
    PVMDNS_RECORD_ARRAY pRecordArray = NULL;
    PVMDNS_ZONE_OBJECT pZoneObject = NULL;

    BAIL_ON_VMDNS_EMPTY_STRING(pszZone, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppRecordArray, dwError);

    *ppRecordArray = NULL;

    dwError = VmDnsCheckAccess(hBinding, FALSE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSrvFindZone(pszZone, &pZoneObject);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSrvListRecords(pZoneObject, &pRecordList);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRpcCopyRecordArray(pRecordList, &pRecordArray);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppRecordArray = pRecordArray;
    pRecordArray = NULL;

cleanup:
    VmDnsZoneObjectRelease(pZoneObject);
    VmDnsRecordListRelease(pRecordList);
    return dwError;
error:
    if (pRecordArray)
    {
        VmDnsRpcFreeRecordArray(pRecordArray);
    }
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);
    goto cleanup;
}

UINT32
VmDnsRpcAddForwarder(
    handle_t            hBinding,
    PDNS_STRING         pszForwarder
    )
{
    DWORD dwError = ERROR_SUCCESS;

    BAIL_ON_VMDNS_EMPTY_STRING(pszForwarder, dwError);

    dwError = VmDnsCheckAccess(hBinding, TRUE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSrvAddForwarder(pszForwarder);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

VOID
VmDnsRpcFreeForwarders(
    PVMDNS_FORWARDERS   pDnsForwarders
    )
{

    if (pDnsForwarders)
    {
        if (pDnsForwarders->ppszName)
        {
            VmDnsRpcServerFreeStringArrayA(
                        pDnsForwarders->ppszName,
                        pDnsForwarders->dwCount);
        }

        VmDnsRpcFreeMemory(pDnsForwarders);
    }
}

DWORD
VmDnsRpcAllocateForwarders(
    PDNS_STRING* ppszForwarders,
    DWORD dwCount,
    PVMDNS_FORWARDERS* ppDnsForwarders
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_FORWARDERS pDnsForwarders = NULL;
    DWORD iForwarder = 0;

    if (!ppDnsForwarders)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsRpcAllocateMemory(
                        sizeof(VMDNS_FORWARDERS),
                        (PVOID*)&pDnsForwarders);
    BAIL_ON_VMDNS_ERROR(dwError);

    if (dwCount > 0)
    {
        dwError = VmDnsRpcAllocateMemory(
                            sizeof(PDNS_STRING) * dwCount,
                            (PVOID*)&pDnsForwarders->ppszName);
        BAIL_ON_VMDNS_ERROR(dwError);

        for (; iForwarder < dwCount; iForwarder++)
        {
            dwError = VmDnsRpcAllocateStringA(
                                ppszForwarders[iForwarder],
                                &pDnsForwarders->ppszName[iForwarder]);
            BAIL_ON_VMDNS_ERROR(dwError);

            ++pDnsForwarders->dwCount;
        }
    }

    pDnsForwarders->dwCount = dwCount;

    *ppDnsForwarders = pDnsForwarders;

cleanup:

    return dwError;

error:

    if (ppDnsForwarders)
    {
        *ppDnsForwarders = NULL;
    }

    if (pDnsForwarders)
    {
        VmDnsRpcFreeForwarders(pDnsForwarders);
    }

    goto cleanup;
}

UINT32
VmDnsRpcGetForwarders(
    handle_t hBinding,
    PVMDNS_FORWARDERS* ppDnsForwarders
    )
{
    DWORD dwError = 0;
    PDNS_STRING* ppszForwarders = NULL;
    DWORD dwCount = 0;
    PVMDNS_FORWARDERS pDnsForwarders = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(ppDnsForwarders, dwError);

    dwError = VmDnsCheckAccess(hBinding, FALSE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSrvGetForwarders(
                        &ppszForwarders,
                        &dwCount);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRpcAllocateForwarders(
                    ppszForwarders,
                    dwCount,
                    &pDnsForwarders);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppDnsForwarders = pDnsForwarders;

cleanup:

    if (ppszForwarders)
    {
        VmDnsFreeStringCountedArrayA(ppszForwarders, dwCount);
    }

    return dwError;

error:

    if (ppDnsForwarders)
    {
        *ppDnsForwarders = NULL;
    }

    if (pDnsForwarders)
    {
        VmDnsRpcFreeForwarders(pDnsForwarders);
    }

    goto cleanup;
}

UINT32
VmDnsRpcDeleteForwarder(
    handle_t hBinding,
    PDNS_STRING     pszForwarder
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMDNS_EMPTY_STRING(pszForwarder, dwError);

    dwError = VmDnsCheckAccess(hBinding, TRUE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSrvDeleteForwarder(
                        pszForwarder);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

