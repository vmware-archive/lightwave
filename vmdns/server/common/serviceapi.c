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

#include "includes.h"

static
BOOL
VmDnsIsAuthorityZone(
    PCSTR szFQDN,
    PCSTR szZone
    );

static
BOOLEAN
VmDnsSrvIsRecordCompatible(
    PCSTR           pszZoneName,
    VMDNS_RR_TYPE   recordType
    );

static
DWORD
VmDnsSrvInitRecords(
    PSTR                     pszDomain,
    PSTR                     pszDcSrvName,
    VMDNS_IP4_ADDRESS_ARRAY  IpV4Addrs,
    VMDNS_IP6_ADDRESS_ARRAY  IpV6Addrs,
    PSTR                     pszSiteName /* OPTIONAL */
    );

static
DWORD
VmDnsSrvCleanupDomain(
    PSTR pszDomain,
    PSTR pszDcSrvName,
    PSTR pszSiteName /* OPTIONAL */
    );

static
DWORD
VmDnsSrvCleanupNSRecords(
    PVMDNS_ZONE_OBJECT pZoneObject,
    PSTR pszDomain,
    PSTR pszDcSrvName
    );

static
DWORD
VmDnsSrvCleanupSRVRecords(
    PVMDNS_ZONE_OBJECT pZoneObject,
    PSTR pszDomain,
    PSTR pszDcSrvName,
    PSTR pszSrvName
    );

static
DWORD
VmDnsSrvCleanupAddressRecords(
    PVMDNS_ZONE_OBJECT pZoneObject,
    PSTR pszDomain,
    PSTR pszAddressRecordName,
    VMDNS_RR_TYPE recordType
    );

static
DWORD
VmDnsSrvCleanupPTRRecords(
    PVMDNS_ZONE_OBJECT pZoneObject,
    PSTR pszRecordName
    );

DWORD
VmDnsSrvInitialize(
    BOOL bUseDirectoryStore
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_FORWARDER_CONTEXT pForwarderContext = NULL;
    PVMDNS_CACHE_CONTEXT pCacheContext = NULL;
    PVMDNS_SECURITY_CONTEXT pSecurityContext = NULL;

    if (VMDNS_UNINITIALIZED != VmDnsSrvGetState())
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCacheInitialize(&pCacheContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsForwarderInit(&pForwarderContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSecInitialize(&pSecurityContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    gpSrvContext->pCacheContext = pCacheContext;
    gpSrvContext->pForwarderContext = pForwarderContext;
    gpSrvContext->pSecurityContext = pSecurityContext;
    gpSrvContext->bUseDirectoryStore = bUseDirectoryStore;
    VmDnsSrvSetState(VMDNS_INITIALIZED);

cleanup:
    return dwError;

error:

    VmDnsCacheCleanup(pCacheContext);
    VmDnsForwarderCleanup(pForwarderContext);
    VmDnsSrvCleanup();

    goto cleanup;
}

VOID
VmDnsSrvCleanup()
{
    VmDnsCacheCleanup(gpSrvContext->pCacheContext);
    VmDnsForwarderCleanup(gpSrvContext->pForwarderContext);
    VmDnsSrvSetState(VMDNS_UNINITIALIZED);
}

VMDNS_STATE
VmDnsSrvGetState()
{
    return gpSrvContext->state;
}

VMDNS_STATE
VmDnsSrvSetState(
    VMDNS_STATE newState
    )
{
    VMDNS_STATE oldState = InterlockedExchange(
                                    (LONG*)&gpSrvContext->state,
                                    newState
                                    );
    if (oldState != newState)
    {
        VMDNS_LOG_INFO("State changed from %u to %u", oldState, newState);
    }
    return oldState;
}

VMDNS_STATE
VmDnsSrvConditionalSetState(
    VMDNS_STATE newState,
    VMDNS_STATE oldState
    )
{
    VMDNS_STATE initState = InterlockedCompareExchange(
                                    (LONG*)&gpSrvContext->state,
                                    newState,
                                    oldState
                                    );
    if (initState != newState)
    {
        if (initState != oldState)
        {
            VMDNS_LOG_DEBUG(
                "%s State not changed. Init state %u different from "
                "conditional initial state %u",
                __FUNCTION__,
                initState,
                oldState
                );
        }
        else
        {
            VMDNS_LOG_INFO("State changed from %u to %u", oldState, newState);
        }
    }

    return initState;
}

DWORD
VmDnsSrvZoneCreate(
    PVMDNS_ZONE_INFO    pZoneInfo
    )
{
    DWORD dwError = 0;

    if (VMDNS_READY != VmDnsSrvGetState())
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsStoreCreateZone(pZoneInfo);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCacheLoadZoneFromStore(
                        gpSrvContext->pCacheContext,
                        pZoneInfo->pszName
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsSrvZoneUpdate(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PVMDNS_ZONE_INFO    pZoneInfo
    )
{
    DWORD dwError = 0;
    PVMDNS_RECORD_LIST pList = NULL;

    if (!pZoneObject || !pZoneInfo)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (VMDNS_READY != VmDnsSrvGetState())
    {
        dwError = ERROR_NOT_READY;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsStoreUpdateZone(pZoneInfo);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsStoreGetRecords(
                        pZoneInfo->pszName,
                        pZoneInfo->pszName,
                        &pList
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneUpdateRecords(
                pZoneObject,
                pZoneInfo->pszName,
                pList
                );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VmDnsRecordListRelease(pList);
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsSrvZoneDelete(
    PVMDNS_ZONE_OBJECT  pZoneObject
    )
{
    DWORD dwError = 0;
    PSTR pszName = NULL;

    if (!pZoneObject)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (VMDNS_READY != VmDnsSrvGetState())
    {
        dwError = ERROR_NOT_READY;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCacheGetZoneName(
                        pZoneObject,
                        &pszName
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsStoreDeleteZone(pszName);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCacheRemoveZone(
                        gpSrvContext->pCacheContext,
                        pZoneObject
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VMDNS_SAFE_FREE_STRINGA(pszName);

    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsSrvFindZone(
    PCSTR               pszZoneName,
    PVMDNS_ZONE_OBJECT  *ppZoneObject
    )
{
    DWORD dwError = 0;

    if (VMDNS_READY != VmDnsSrvGetState())
    {
        dwError = ERROR_NOT_READY;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError =  VmDnsCacheFindZone(
                        gpSrvContext->pCacheContext,
                        pszZoneName,
                        ppZoneObject
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsSrvAddRecord(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PVMDNS_RECORD       pRecord
    )
{
    DWORD dwError = 0;
    PSTR pszName = NULL;
    PVMDNS_RECORD_LIST pRecordList = NULL;
    VMDNS_RR_TYPE dwType = pRecord->dwType;

    if (VMDNS_READY != VmDnsSrvGetState())
    {
        dwError = ERROR_NOT_READY;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCacheGetZoneName(
                        pZoneObject,
                        &pszName
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (!VmDnsSrvIsRecordCompatible(pszName, pRecord->dwType))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (pRecord->dwType == VMDNS_RR_TYPE_CNAME)
    {
        dwType = VMDNS_RR_QTYPE_ANY;
    }

    dwError = VmDnsSrvGetRecords(pZoneObject, pRecord->pszName, dwType, &pRecordList);

    BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

    dwError = VmDnsIsUpdatePermitted(pRecord->dwType,pRecordList);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsStoreAddZoneRecord(
                        pszName,
                        pRecord
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCachePurgeRecord(pZoneObject, pRecord->pszName, CACHE_PURGE_MODIFICATION);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VmDnsRecordListRelease(pRecordList);
    VMDNS_SAFE_FREE_STRINGA(pszName);

    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsSrvDeleteRecord(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PVMDNS_RECORD   pRecord
    )
{
    DWORD dwError = 0;
    PSTR pszZoneName = NULL;
    PVMDNS_RECORD_LIST pRecordList = NULL;

    if (!pZoneObject)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (VMDNS_READY != VmDnsSrvGetState())
    {
        dwError = ERROR_NOT_READY;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (pRecord->dwType == VMDNS_RR_TYPE_SOA)
    {
        dwError = ERROR_OPERATION_NOT_PERMITTED;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCacheGetZoneName(
                        pZoneObject,
                        &pszZoneName
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsStoreDeleteZoneRecord(
                        pszZoneName,
                        pRecord
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZonePurgeRecords(
                        pZoneObject,
                        pRecord->pszName
                        );
    BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

cleanup:
    VmDnsRecordListRelease(pRecordList);
    VMDNS_SAFE_FREE_STRINGA(pszZoneName);

    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsSrvUpdateRecord(
    PVMDNS_ZONE_OBJECT pZoneObject,
    PVMDNS_RECORD pOldRecord,
    PVMDNS_RECORD pNewRecord
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMDNS_INVALID_POINTER(pZoneObject, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pOldRecord, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pNewRecord, dwError);

    dwError = VmDnsSrvDeleteRecord(
                        pZoneObject,
                        pOldRecord
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSrvAddRecord(
                        pZoneObject,
                        pNewRecord
                        );
    BAIL_ON_VMDNS_ERROR(dwError);


cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsSrvAddRecords(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PVMDNS_RECORD_LIST  pRecords
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    PVMDNS_RECORD_OBJECT pRecordObject = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pZoneObject, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pRecords, dwError);

    for (; dwIndex < pRecords->dwCurrentSize; ++dwIndex)
    {
        pRecordObject = VmDnsRecordListGetRecord(pRecords, dwIndex);

        dwError = VmDnsSrvAddRecord(
                        pZoneObject,
                        pRecordObject->pRecord
                        );
        BAIL_ON_VMDNS_ERROR(dwError);

        VmDnsRecordObjectRelease(pRecordObject);
        pRecordObject = NULL;
    }


cleanup:
    VmDnsRecordObjectRelease(pRecordObject);
    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsSrvDeleteRecords(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PVMDNS_RECORD_LIST pRecords
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;

    BAIL_ON_VMDNS_INVALID_POINTER(pZoneObject, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pRecords, dwError);

    for (; dwIndex < pRecords->dwCurrentSize; ++dwIndex)
    {
        dwError = VmDnsSrvDeleteRecord(
                        pZoneObject,
                        VmDnsRecordListGetRecord(pRecords, dwIndex)->pRecord
                        );
        BAIL_ON_VMDNS_ERROR(dwError);
    }


cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsSrvGetRecords(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PCSTR               pszName,
    VMDNS_RR_TYPE       dwType,
    PVMDNS_RECORD_LIST  *ppRecordList
    )
{
    DWORD dwError = 0;
    PSTR pszZone = NULL;
    PSTR szNameFqdn = NULL;
    PCSTR szNameQuery = NULL;
    PVMDNS_RECORD_LIST pRecordList = NULL;

    dwError = VmDnsCacheGetZoneName(
                pZoneObject,
                &pszZone
                );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsMakeFQDN(
                pszName,
                pszZone,
                &szNameFqdn
                );
    BAIL_ON_VMDNS_ERROR(dwError);

    szNameQuery = szNameFqdn ? szNameFqdn : pszName;

    dwError = VmDnsZoneGetRecords(
                pZoneObject,
                szNameQuery,
                dwType,
                &pRecordList
                );
    BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

    if (!pRecordList || VmDnsRecordListGetSize(pRecordList) == 0)
    {
        VmMetricsCounterIncrement(gVmDnsCounterMetrics[CACHE_CACHE_MISS]);
        dwError = VmDnsStoreGetRecords(
                    pszZone,
                    szNameQuery,
                    &pRecordList
                    );
        BAIL_ON_VMDNS_ERROR(dwError && dwError != ERROR_NOT_FOUND);

        if (pRecordList)
        {
            dwError = VmDnsZoneUpdateRecords(
                        pZoneObject,
                        szNameQuery,
                        pRecordList
                        );
            BAIL_ON_VMDNS_ERROR(dwError);

            VmDnsRecordListRelease(pRecordList);
            pRecordList = NULL;

            dwError = VmDnsZoneGetRecords(
                        pZoneObject,
                        szNameQuery,
                        dwType,
                        &pRecordList
                        );
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }

    *ppRecordList = pRecordList;

cleanup:
    VMDNS_SAFE_FREE_STRINGA(pszZone);
    VMDNS_SAFE_FREE_STRINGA(szNameFqdn);

    return dwError;

error:
    if (ppRecordList)
    {
        *ppRecordList = NULL;
    }

    VmDnsRecordListRelease(pRecordList);

    goto cleanup;
}

DWORD
VmDnsSrvQueryRecords(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PCSTR               pszName,
    VMDNS_RR_TYPE       dwType,
    DWORD               dwOptions,
    PVMDNS_RECORD_LIST  *ppRecordList
    )
{
    DWORD dwError = 0, dwRecordListSize = 0, dwRecursionIndex = 0, i = 0;
    PVMDNS_RECORD_LIST pRecordList = NULL;
    PVMDNS_RECORD_LIST pLinkedRecordList = NULL;
    PVMDNS_RECORD_LIST pTempRecordList = NULL;
    PVMDNS_RECORD_OBJECT pRecordObject = NULL;

    if (!pZoneObject)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (VMDNS_READY != VmDnsSrvGetState())
    {
        dwError = ERROR_NOT_READY;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsRecordListCreate(&pLinkedRecordList);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSrvGetRecords(pZoneObject, pszName, dwType, &pRecordList);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwRecordListSize = VmDnsRecordListGetSize(pRecordList);

    for (; i < dwRecordListSize; i++)
    {
        pRecordObject = VmDnsRecordListGetRecord(pRecordList, i);

        dwError = VmDnsGetLinkedRecords(++dwRecursionIndex,
                                        pZoneObject,
                                        dwType,
                                        pRecordObject->pRecord,
                                        &pTempRecordList
                                        );
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsRecordListAddList(pLinkedRecordList, pTempRecordList);
        BAIL_ON_VMDNS_ERROR(dwError);

        VmDnsRecordListRelease(pTempRecordList);
        pTempRecordList = NULL;
    }

    dwError = VmDnsRecordListAddList(pRecordList, pLinkedRecordList);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppRecordList = pRecordList;

cleanup:
    VmDnsRecordObjectRelease(pRecordObject);
    VmDnsRecordListRelease(pLinkedRecordList);

    return dwError;

error:
    if (ppRecordList)
    {
        *ppRecordList = NULL;
    }

    VmDnsRecordListRelease(pRecordList);

    goto cleanup;
}

DWORD
VmDnsGetLinkedRecords(
    DWORD               dwRecursionIndex,
    PVMDNS_ZONE_OBJECT  pZoneObject,
    VMDNS_RR_TYPE       dwType,
    PVMDNS_RECORD       pRecord,
    PVMDNS_RECORD_LIST  *ppRecordList
    )
{
    DWORD dwError = 0, i = 0, dwRecordListSize = 0;
    PVMDNS_RECORD_LIST pRecordList = NULL;
    PVMDNS_RECORD_LIST pTempRecordList = NULL;
    PVMDNS_RECORD_LIST pLinkedRecordList = NULL;
    PVMDNS_RECORD_OBJECT pRecordObject = NULL;

    if(pRecord->dwType != VMDNS_RR_TYPE_CNAME)
        return 0;

    if(dwRecursionIndex > 4)
    {
        dwError = ERROR_NOT_SUPPORTED;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsRecordListCreate(&pLinkedRecordList);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSrvGetRecords(pZoneObject, pRecord->Data.CNAME.pNameHost, dwType, &pRecordList);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwRecordListSize = VmDnsRecordListGetSize(pRecordList);

    for (; i < dwRecordListSize; i++)
    {
        pRecordObject = VmDnsRecordListGetRecord(pRecordList, i);

        dwError = VmDnsGetLinkedRecords(++dwRecursionIndex,
                                        pZoneObject,
                                        dwType,
                                        pRecordObject->pRecord,
                                        &pTempRecordList
                                        );
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsRecordListAddList(pLinkedRecordList, pTempRecordList);
        BAIL_ON_VMDNS_ERROR(dwError);

        VmDnsRecordListRelease(pTempRecordList);
        pTempRecordList = NULL;
    }

    dwError = VmDnsRecordListAddList(pRecordList, pLinkedRecordList);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppRecordList = pRecordList;

cleanup:
    VmDnsRecordObjectRelease(pRecordObject);
    VmDnsRecordListRelease(pLinkedRecordList);

    return dwError;

error:
    if (ppRecordList)
    {
        *ppRecordList = NULL;
    }

    VmDnsRecordListRelease(pRecordList);

    goto cleanup;
}

DWORD
VmDnsSrvListZones(
    PVMDNS_ZONE_INFO_ARRAY  *ppZoneArray
    )
{
    DWORD dwError = 0;

    if (VMDNS_READY != VmDnsSrvGetState())
    {
        dwError = ERROR_NOT_READY;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCacheListZones(
                        gpSrvContext->pCacheContext,
                        ppZoneArray
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsSrvListRecords(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PVMDNS_RECORD_LIST *ppRecordList
    )
{
    DWORD dwError = 0;
    PVMDNS_RECORD_LIST pRecordList = NULL;
    PSTR pszZone = NULL;

    if (!pZoneObject)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (VMDNS_READY != VmDnsSrvGetState())
    {
        dwError = ERROR_NOT_READY;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCacheGetZoneName(
                        pZoneObject,
                        &pszZone
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsStoreListRecords(
                        pszZone,
                        &pRecordList
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppRecordList = pRecordList;

cleanup:
    VMDNS_SAFE_FREE_STRINGA(pszZone);
    return dwError;

error:
    VmDnsRecordListRelease(pRecordList);
    goto cleanup;
}

DWORD
VmDnsSrvAddForwarder(
    PCSTR     pszForwarder
    )
{
    DWORD dwError = 0;

    if (VMDNS_READY != VmDnsSrvGetState())
    {
        dwError = ERROR_NOT_READY;
        VmDnsLog(VMDNS_LOG_LEVEL_ERROR,"dns server not ready = %d\n", VmDnsSrvGetState());
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAddForwarder(
                    gpSrvContext->pForwarderContext,
                    pszForwarder
                    );
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR,"dns Add Forwarder = %d\n", dwError);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsSrvGetForwarders(
    PSTR**                      pppszForwarders,
    PDWORD                      pdwCount
    )
{
    DWORD dwError = 0;

    if (VMDNS_READY != VmDnsSrvGetState())
    {
        dwError = ERROR_NOT_READY;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsGetForwarders(
                        gpSrvContext->pForwarderContext,
                        pppszForwarders,
                        pdwCount
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsSrvDeleteForwarder(
    PCSTR     pszForwarder
    )
{
    DWORD dwError = 0;

    if (VMDNS_READY != VmDnsSrvGetState())
    {
        dwError = ERROR_NOT_READY;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsDeleteForwarder(
                        gpSrvContext->pForwarderContext,
                        pszForwarder
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsSrvInitDomain(
    PVMDNS_INIT_INFO            pInitInfo
    )
{
    DWORD dwError = 0;

    dwError = VmDnsSrvInitRecords(
                        pInitInfo->pszDomain,
                        pInitInfo->pszDcSrvName,
                        pInitInfo->IpV4Addrs,
                        pInitInfo->IpV6Addrs,
                        NULL);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsSrvInitDomainWithSite(
    PVMDNS_INIT_SITE_INFO       pInitInfo
    )
{
    DWORD dwError = 0;

    dwError = VmDnsSrvInitRecords(
                        pInitInfo->pszDomain,
                        pInitInfo->pszDcSrvName,
                        pInitInfo->IpV4Addrs,
                        pInitInfo->IpV6Addrs,
                        pInitInfo->pszSiteName);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsSrvUninitializeDomain(
    PVMDNS_INIT_INFO            pInitInfo
    )
{
    DWORD dwError = 0;

    dwError = VmDnsSrvCleanupDomain(
                        pInitInfo->pszDomain,
                        pInitInfo->pszDcSrvName,
                        NULL);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsSrvUninitializeDomainWithSite(
    PVMDNS_INIT_SITE_INFO       pInitInfo
    )
{
    DWORD dwError = 0;

    dwError = VmDnsSrvCleanupDomain(
                        pInitInfo->pszDomain,
                        pInitInfo->pszDcSrvName,
                        pInitInfo->pszSiteName);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsSrvInitRecords(
    PSTR                    pszDomain,
    PSTR                    pszDcSrvName,
    VMDNS_IP4_ADDRESS_ARRAY IpV4Addrs,
    VMDNS_IP6_ADDRESS_ARRAY IpV6Addrs,
    PSTR                    pszSiteName /* OPTIONAL */
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PSTR pszAddressRecordName = NULL;
    PSTR pszLdapRecordName = NULL;
    PSTR pszKrbRecordName = NULL;
    PSTR pszLdapDcRecordName = NULL;
    PSTR pszLdapDcSiteRecordName = NULL;
    PSTR pszLdapMSDCSiteRecordName = NULL;
    PSTR pszKrbDcRecordName = NULL;
    DWORD idx = 0;

    VMDNS_ZONE_INFO zoneInfo = {
        .pszName                = pszDomain,
        .pszPrimaryDnsSrvName   = pszDcSrvName,
        .pszRName               = "",
        .serial                 = 0,
        .refreshInterval        = VMDNS_DEFAULT_REFRESH_INTERVAL,
        .retryInterval          = VMDNS_DEFAULT_RETRY_INTERVAL,
        .expire                 = VMDNS_DEFAULT_EXPIRE,
        .minimum                = VMDNS_DEFAULT_TTL
    };

    VMDNS_RECORD nsRecord = {
        .pszName                = pszDomain,
        .Data.NS.pNameHost      = pszDcSrvName,
        .iClass                 = VMDNS_CLASS_IN,
        .dwType                 = VMDNS_RR_TYPE_NS,
        .dwTtl                  = VMDNS_DEFAULT_TTL
    };

    VMDNS_RECORD ldapSrvRecord = {
        .pszName                = VMDNS_LDAP_SRV_NAME,
        .dwType                 = VMDNS_RR_TYPE_SRV,
        .iClass                 = VMDNS_CLASS_IN,
        .dwTtl                  = VMDNS_DEFAULT_TTL,
        .Data.SRV.pNameTarget   = pszDcSrvName,
        .Data.SRV.wPriority     = 1,
        .Data.SRV.wWeight       = 1,
        .Data.SRV.wPort         = VMDNS_DEFAULT_LDAP_PORT
    };

    VMDNS_RECORD kerberosSrvRecord = {
        .pszName                = VMDNS_KERBEROS_SRV_NAME,
        .dwType                 = VMDNS_RR_TYPE_SRV,
        .iClass                 = VMDNS_CLASS_IN,
        .dwTtl                  = VMDNS_DEFAULT_TTL,
        .Data.SRV.pNameTarget   = pszDcSrvName,
        .Data.SRV.wPriority     = 1,
        .Data.SRV.wWeight       = 1,
        .Data.SRV.wPort         = VMDNS_DEFAULT_KDC_PORT
    };

    VMDNS_RECORD ldapDcSrvRecord = {
        .pszName                = VMDNS_LDAP_DC_SRV_NAME,
        .dwType                 = VMDNS_RR_TYPE_SRV,
        .iClass                 = VMDNS_CLASS_IN,
        .dwTtl                  = VMDNS_DEFAULT_TTL,
        .Data.SRV.pNameTarget   = pszDcSrvName,
        .Data.SRV.wPriority     = 1,
        .Data.SRV.wWeight       = 1,
        .Data.SRV.wPort         = VMDNS_DEFAULT_LDAP_PORT
    };

    VMDNS_RECORD ldapDcSiteSrvRecord = {
        .pszName                = VMDNS_LDAP_SRV_NAME,
        .dwType                 = VMDNS_RR_TYPE_SRV,
        .iClass                 = VMDNS_CLASS_IN,
        .dwTtl                  = VMDNS_DEFAULT_TTL,
        .Data.SRV.pNameTarget   = pszDcSrvName,
        .Data.SRV.wPriority     = 1,
        .Data.SRV.wWeight       = 1,
        .Data.SRV.wPort         = VMDNS_DEFAULT_LDAP_PORT
    };

    VMDNS_RECORD ldapMSDCSiteSrvRecord = {
        .pszName                = VMDNS_LDAP_SRV_NAME,
        .dwType                 = VMDNS_RR_TYPE_SRV,
        .iClass                 = VMDNS_CLASS_IN,
        .dwTtl                  = VMDNS_DEFAULT_TTL,
        .Data.SRV.pNameTarget   = pszDcSrvName,
        .Data.SRV.wPriority     = 1,
        .Data.SRV.wWeight       = 1,
        .Data.SRV.wPort         = VMDNS_DEFAULT_LDAP_PORT
    };

    VMDNS_RECORD kerberosDcSrvRecord = {
        .pszName                = VMDNS_KERBEROS_DC_SRV_NAME,
        .dwType                 = VMDNS_RR_TYPE_SRV,
        .iClass                 = VMDNS_CLASS_IN,
        .dwTtl                  = VMDNS_DEFAULT_TTL,
        .Data.SRV.pNameTarget   = pszDcSrvName,
        .Data.SRV.wPriority     = 1,
        .Data.SRV.wWeight       = 1,
        .Data.SRV.wPort         = VMDNS_DEFAULT_KDC_PORT
    };

    if (IsNullOrEmptyString(pszDcSrvName) ||
        IsNullOrEmptyString(pszDomain))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateStringA(
                        pszDcSrvName,
                        &pszAddressRecordName
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringPrintfA(
                            &pszLdapRecordName,
                            "%s.%s",
                            VMDNS_LDAP_SRV_NAME,
                            pszDomain
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    /* LDAP DC SRV record entry */
    dwError = VmDnsAllocateStringPrintfA(
                            &pszLdapDcRecordName,
                            "%s.%s",
                            VMDNS_LDAP_DC_SRV_NAME,
                            pszDomain
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (pszSiteName)
    {
        /* LDAP DC SRV region specific record entry */
        dwError = VmDnsAllocateStringPrintfA(
                                &pszLdapDcSiteRecordName,
                                "%s.%s.%s.%s",
                                VMDNS_LDAP_SRV_NAME,
                                pszSiteName,
                                VMDNS_SRV_SITE_LABEL,
                                pszDomain
                                );
        BAIL_ON_VMDNS_ERROR(dwError);

        /* LDAP MSDCS SRV region specific record entry */
        dwError = VmDnsAllocateStringPrintfA(
                                &pszLdapMSDCSiteRecordName,
                                "%s.%s.%s.%s",
                                VMDNS_LDAP_SRV_NAME,
                                pszSiteName,
                                VMDNS_SRV_MSDCS_SITE_LABEL,
                                pszDomain
                                );
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateStringPrintfA(
                            &pszKrbRecordName,
                            "%s.%s",
                            VMDNS_KERBEROS_SRV_NAME,
                            pszDomain
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    /* Kerberos DC SRV record entry */
    dwError = VmDnsAllocateStringPrintfA(
                            &pszKrbDcRecordName,
                            "%s.%s",
                            VMDNS_KERBEROS_DC_SRV_NAME,
                            pszDomain
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsMakeFQDN(
                    pszDcSrvName,
                    pszDomain,
                    &pszAddressRecordName
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsStoreCreateZone(&zoneInfo);
    dwError = (dwError == ERROR_ALREADY_EXISTS) ? ERROR_SUCCESS : dwError;
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsStoreAddZoneRecord(
                        pszDomain,
                        &nsRecord
                        );
    dwError = (dwError == ERROR_ALREADY_EXISTS) ?  ERROR_SUCCESS : dwError;
    BAIL_ON_VMDNS_ERROR(dwError);

    ldapSrvRecord.pszName = pszLdapRecordName;
    dwError = VmDnsStoreAddZoneRecord(
                        pszDomain,
                        &ldapSrvRecord
                        );
    dwError = (dwError == ERROR_ALREADY_EXISTS) ? ERROR_SUCCESS : dwError;
    BAIL_ON_VMDNS_ERROR(dwError);

    kerberosSrvRecord.pszName = pszKrbRecordName;
    dwError = VmDnsStoreAddZoneRecord(
                        pszDomain,
                        &kerberosSrvRecord
                        );
    dwError = (dwError == ERROR_ALREADY_EXISTS) ? ERROR_SUCCESS : dwError;
    BAIL_ON_VMDNS_ERROR(dwError);

    ldapDcSrvRecord.pszName = pszLdapDcRecordName;
    dwError = VmDnsStoreAddZoneRecord(
                        pszDomain,
                        &ldapDcSrvRecord
                        );
    dwError = (dwError == ERROR_ALREADY_EXISTS) ? ERROR_SUCCESS : dwError;
    BAIL_ON_VMDNS_ERROR(dwError);


    if (pszLdapDcSiteRecordName)
    {
        ldapDcSiteSrvRecord.pszName = pszLdapDcSiteRecordName;
        dwError = VmDnsStoreAddZoneRecord(
                            pszDomain,
                            &ldapDcSiteSrvRecord
                            );
        dwError = (dwError == ERROR_ALREADY_EXISTS) ? ERROR_SUCCESS : dwError;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (pszLdapMSDCSiteRecordName)
    {
        ldapMSDCSiteSrvRecord.pszName = pszLdapMSDCSiteRecordName;
        dwError = VmDnsStoreAddZoneRecord(
                            pszDomain,
                            &ldapMSDCSiteSrvRecord
                            );
        dwError = (dwError == ERROR_ALREADY_EXISTS) ? ERROR_SUCCESS : dwError;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    kerberosDcSrvRecord.pszName = pszKrbDcRecordName;
    dwError = VmDnsStoreAddZoneRecord(
                        pszDomain,
                        &kerberosDcSrvRecord
                        );
    dwError = (dwError == ERROR_ALREADY_EXISTS) ? ERROR_SUCCESS : dwError;
    BAIL_ON_VMDNS_ERROR(dwError);

    if (!VmDnsCheckIfIPV4AddressA(pszDcSrvName) &&
        !VmDnsCheckIfIPV6AddressA(pszDcSrvName))
    {
        dwError = VmDnsMakeFQDN(
                    pszDcSrvName,
                    pszDomain,
                    &pszAddressRecordName
                    );
        BAIL_ON_VMDNS_ERROR(dwError);

        if (VmDnsIsAuthorityZone(pszAddressRecordName, pszDomain))
        {
            for (idx = 0; idx < IpV4Addrs.dwCount; ++idx)
            {
                VMDNS_RECORD ip4AddrRecord =
                {
                    .pszName = pszAddressRecordName,
                    .dwType = VMDNS_RR_TYPE_A,
                    .iClass = VMDNS_CLASS_IN,
                    .dwTtl = VMDNS_DEFAULT_TTL,
                    .Data.A.IpAddress = IpV4Addrs.Addrs[idx]
                };

                dwError = VmDnsStoreAddZoneRecord(
                                pszDomain,
                                &ip4AddrRecord
                                );
                dwError = (dwError == ERROR_ALREADY_EXISTS) ? ERROR_SUCCESS : dwError;
                BAIL_ON_VMDNS_ERROR(dwError);
            }

            for (idx = 0; idx < IpV6Addrs.dwCount; ++idx)
            {
                VMDNS_RECORD ip6AddrRecord =
                {
                    .pszName = pszAddressRecordName,
                    .dwType = VMDNS_RR_TYPE_AAAA,
                    .iClass = VMDNS_CLASS_IN,
                    .dwTtl = VMDNS_DEFAULT_TTL
                };

                dwError = VmDnsCopyMemory(
                            ip6AddrRecord.Data.AAAA.Ip6Address.IP6Byte,
                            sizeof(ip6AddrRecord.Data.AAAA.Ip6Address.IP6Byte),
                            IpV6Addrs.Addrs[idx].IP6Byte,
                            sizeof(IpV6Addrs.Addrs[idx].IP6Byte)
                            );
                BAIL_ON_VMDNS_ERROR(dwError);

                dwError = VmDnsStoreAddZoneRecord(
                                pszDomain,
                                &ip6AddrRecord
                                );
                dwError = (dwError == ERROR_ALREADY_EXISTS) ? ERROR_SUCCESS : dwError;
                BAIL_ON_VMDNS_ERROR(dwError);
            }
        }
    }

    for (idx = 0; idx < 5 || VmDnsSrvGetState() != VMDNS_READY; idx++)
    {
        dwError = VmDnsConditionSignal(gpSrvContext->pCacheContext->pRefreshEvent);
        if (dwError)
        {
            VMDNS_LOG_ERROR("Failed to signal cache refresh thread with %u.", dwError);
        }
        BAIL_ON_VMDNS_ERROR(dwError);
        sleep(5);
    }

    if (VmDnsSrvGetState() != VMDNS_READY)
    {
        dwError = ERROR_INVALID_STATE;
        VMDNS_LOG_ERROR("Failed to populate cache and VMDNS state is not ready: %u.", dwError);
    }
    else
    {
        VMDNS_LOG_INFO("Succesfully populated cache and VMDNS state is ready: %u.", dwError);
    }

    dwError = VmDnsCacheLoadZoneFromStore(
                            gpSrvContext->pCacheContext,
                            pszDomain
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VMDNS_SAFE_FREE_STRINGA(pszAddressRecordName);
    VMDNS_SAFE_FREE_STRINGA(pszLdapRecordName);
    VMDNS_SAFE_FREE_STRINGA(pszKrbRecordName);
    VMDNS_SAFE_FREE_STRINGA(pszLdapDcRecordName);
    VMDNS_SAFE_FREE_STRINGA(pszKrbDcRecordName);
    VMDNS_SAFE_FREE_STRINGA(pszLdapDcSiteRecordName);
    VMDNS_SAFE_FREE_STRINGA(pszLdapMSDCSiteRecordName);
    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsSrvCleanupDomain(
    PSTR pszDomain,
    PSTR pszDcSrvName,
    PSTR pszSiteName /* OPTIONAL */
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_ZONE_OBJECT pZoneObject = NULL;
    PSTR pszAddressRecordName = NULL;
    PSTR pszSRVSitePrefix = NULL;
    PSTR pszSRVSiteMSDCPrefix = NULL;

    if (IsNullOrEmptyString(pszDcSrvName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (VMDNS_READY != VmDnsSrvGetState())
    {
        dwError = ERROR_NOT_READY;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsSrvFindZone(
                    pszDomain,
                    &pZoneObject
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringA(
                    pszDcSrvName,
                    &pszAddressRecordName
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    // Cleanup NS records(s)
    dwError = VmDnsSrvCleanupNSRecords(
                    pZoneObject,
                    pszDomain,
                    pszDcSrvName
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    // Cleanup LDAP TCP SRV record(s)
    dwError = VmDnsSrvCleanupSRVRecords(
                    pZoneObject,
                    pszDomain,
                    pszDcSrvName,
                    VMDNS_LDAP_SRV_NAME
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (!IsNullOrEmptyString(pszSiteName))
    {
        // Cleanup LDAP TCP SRV Site record(s)
        dwError = VmDnsAllocateStringPrintfA(
                        &pszSRVSitePrefix,
                        "%s.%s.%s",
                        VMDNS_LDAP_SRV_NAME,
                        pszSiteName,
                        VMDNS_SRV_SITE_LABEL
                        );
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsSrvCleanupSRVRecords(
                        pZoneObject,
                        pszDomain,
                        pszDcSrvName,
                        pszSRVSitePrefix
                        );
        BAIL_ON_VMDNS_ERROR(dwError);

        // Cleanup LDAP TCP SRV MSDC record(s)
        dwError = VmDnsAllocateStringPrintfA(
                        &pszSRVSiteMSDCPrefix,
                        "%s.%s.%s",
                        VMDNS_LDAP_SRV_NAME,
                        pszSiteName,
                        VMDNS_SRV_MSDCS_SITE_LABEL
                        );
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsSrvCleanupSRVRecords(
                        pZoneObject,
                        pszDomain,
                        pszDcSrvName,
                        pszSRVSiteMSDCPrefix
                        );
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    // Cleanup KDC TCP SRV record(s)
    dwError = VmDnsSrvCleanupSRVRecords(
                    pZoneObject,
                    pszDomain,
                    pszDcSrvName,
                    VMDNS_KERBEROS_SRV_NAME
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    // Cleanup LDAP TCP DC SRV record(s)
    dwError = VmDnsSrvCleanupSRVRecords(
                    pZoneObject,
                    pszDomain,
                    pszDcSrvName,
                    VMDNS_LDAP_DC_SRV_NAME
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    // Cleanup KDC TCP DC SRV record(s)
    dwError = VmDnsSrvCleanupSRVRecords(
                    pZoneObject,
                    pszDomain,
                    pszDcSrvName,
                    VMDNS_KERBEROS_DC_SRV_NAME
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    // Cleanup A record(s)
    dwError = VmDnsSrvCleanupAddressRecords(
                    pZoneObject,
                    pszDomain,
                    pszAddressRecordName,
                    VMDNS_RR_TYPE_A
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    // Cleanup AAAA record(s)
    dwError = VmDnsSrvCleanupAddressRecords(
                    pZoneObject,
                    pszDomain,
                    pszAddressRecordName,
                    VMDNS_RR_TYPE_AAAA
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VmDnsZoneObjectRelease(pZoneObject);
    VMDNS_SAFE_FREE_STRINGA(pszAddressRecordName);
    VMDNS_SAFE_FREE_STRINGA(pszSRVSitePrefix);
    VMDNS_SAFE_FREE_STRINGA(pszSRVSiteMSDCPrefix);

    return dwError;

error:
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmDnsSrvGetInverseRRTypeRecordList(
    PVMDNS_RECORD_LIST pFullRecordList,
    VMDNS_RR_TYPE dwExcludeType,
    PVMDNS_RECORD_LIST *ppParsedRecordList
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    DWORD dwSize = 0;
    PVMDNS_RECORD_OBJECT pRecordObj = NULL;
    PVMDNS_RECORD_LIST pParsedRecordList = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pFullRecordList, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppParsedRecordList, dwError);

    dwError = VmDnsRecordListCreate(&pParsedRecordList);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwSize = VmDnsRecordListGetSize(pFullRecordList);

    for (; dwIndex < dwSize; ++dwIndex)
    {
        VmDnsRecordObjectRelease(pRecordObj);
        pRecordObj = NULL;

        pRecordObj = VmDnsRecordListGetRecord(pFullRecordList, dwIndex);

        if (pRecordObj->pRecord->dwType != dwExcludeType)
        {
            dwError = VmDnsRecordListAdd(
                                pParsedRecordList,
                                pRecordObj
                                );
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }

    *ppParsedRecordList = pParsedRecordList;

cleanup:
    VmDnsRecordObjectRelease(pRecordObj);
    return dwError;

error:
    VmDnsRecordListRelease(pParsedRecordList);
    if (ppParsedRecordList)
    {
        *ppParsedRecordList = NULL;
    }

    goto cleanup;
}


static
BOOL
VmDnsIsAuthorityZone(
    PCSTR szFQDN,
    PCSTR szZone
    )
{
    BOOL bRet = FALSE;
    size_t nFqdnLength = strlen(szFQDN);
    size_t nZoneLength = strlen(szZone);
    if (nFqdnLength >= nZoneLength)
    {
        size_t nStartCursor = nFqdnLength - nZoneLength;

        if (!VmDnsStringNCompareA(
                    &szFQDN[nStartCursor],
                    szZone,
                    nZoneLength,
                    FALSE))
        {
            if (nStartCursor > 0)
            {
                bRet = szFQDN[nStartCursor - 1] == '.';
            }
            else
            {
                bRet = TRUE;
            }

        }
    }
    return bRet;
}

static
BOOLEAN
VmDnsSrvIsRecordCompatible(
    PCSTR           pszZoneName,
    VMDNS_RR_TYPE   recordType
    )
{
    VMDNS_ZONE_TYPE zoneType = VMDNS_ZONE_TYPE_FORWARD;
    if (VmDnsIsReverseZoneName(pszZoneName))
    {
        zoneType = VMDNS_ZONE_TYPE_REVERSE;
    }

    if ((zoneType == VMDNS_ZONE_TYPE_FORWARD &&
        recordType == VMDNS_RR_TYPE_PTR) ||
        (zoneType == VMDNS_ZONE_TYPE_REVERSE &&
        (recordType != VMDNS_RR_TYPE_PTR &&
        recordType != VMDNS_RR_TYPE_SOA &&
        recordType != VMDNS_RR_TYPE_NS)))
    {
        return FALSE;
    }

    return TRUE;
}

static
DWORD
VmDnsSrvCleanupNSRecords(
    PVMDNS_ZONE_OBJECT pZoneObject,
    PSTR pszDomain,
    PSTR pszDcSrvName
)
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_RECORD_LIST pRecordList = NULL;
    DWORD idx = 0;

    if (!pZoneObject ||
        IsNullOrEmptyString(pszDomain) ||
        IsNullOrEmptyString(pszDcSrvName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    // Query NS record(s)
    dwError = VmDnsSrvQueryRecords(
                    pZoneObject,
                    pszDomain,
                    VMDNS_RR_TYPE_NS,
                    0,
                    &pRecordList
                    );
    if (dwError)
    {
        VMDNS_LOG_ERROR("%s, failed to get NS records. Error %d", __FUNCTION__, dwError);
        dwError = ERROR_SUCCESS;
    }

    // Remove NS record(s)
    if (pRecordList)
    {
        for (idx = 0; idx < pRecordList->dwCurrentSize; ++idx)
        {
            PVMDNS_RECORD pRecord = pRecordList->ppRecords[idx]->pRecord;

            if (!VmDnsStringCompareA(
                        pRecord->Data.NS.pNameHost,
                        pszDcSrvName,
                        FALSE))
            {
                dwError = VmDnsSrvDeleteRecord(pZoneObject, pRecord);

                VMDNS_LOG_INFO(
                      "Cleanup NS record %s from zone %s, status: %u.",
                      pRecord->pszName,
                      pszDomain,
                      dwError
                      );

                dwError = ERROR_SUCCESS;
            }
        }
    }

cleanup:
    VmDnsRecordListRelease(pRecordList);
    return dwError;

error:
    VMDNS_LOG_ERROR("%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

static
DWORD
VmDnsSrvCleanupSRVRecords(
    PVMDNS_ZONE_OBJECT pZoneObject,
    PSTR pszDomain,
    PSTR pszDcSrvName,
    PSTR pszSrvName
)
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_RECORD_LIST pRecordList = NULL;
    DWORD idx = 0;
    VMDNS_RECORD srvRecord = { 0 };

    if (!pZoneObject ||
        IsNullOrEmptyString(pszDomain) ||
        IsNullOrEmptyString(pszDcSrvName) ||
        IsNullOrEmptyString(pszSrvName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateStringPrintfA(
                            &srvRecord.pszName,
                            "%s.%s",
                            pszSrvName,
                            pszDomain
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    srvRecord.dwType = VMDNS_RR_TYPE_SRV;
    srvRecord.iClass = VMDNS_CLASS_IN;
    srvRecord.Data.SRV.pNameTarget = pszDcSrvName;

    // Query SRV record(s)
    dwError = VmDnsSrvQueryRecords(
                    pZoneObject,
                    pszSrvName,
                    VMDNS_RR_TYPE_SRV,
                    0,
                    &pRecordList
                    );
    if (dwError)
    {
        VMDNS_LOG_ERROR("%s, failed to get %s SRV records. Error %d",
                        __FUNCTION__,
                        pszSrvName,
                        dwError);
        dwError = ERROR_SUCCESS;
    }

    // Remove SRV record(s)
    if (pRecordList)
    {
        for (idx = 0; idx < pRecordList->dwCurrentSize; ++idx)
        {
            PVMDNS_RECORD pRecord = pRecordList->ppRecords[idx]->pRecord;

            if (VmDnsMatchRecord(&srvRecord, pRecord))
            {
                dwError = VmDnsSrvDeleteRecord(pZoneObject, pRecord);

                VMDNS_LOG_INFO(
                      "Cleanup SRV record %s from zone %s, status: %u.",
                      pRecord->pszName,
                      pszDomain,
                      dwError
                      );

                dwError = ERROR_SUCCESS;
            }
        }
    }

cleanup:
    VmDnsRecordListRelease(pRecordList);
    VMDNS_SAFE_FREE_STRINGA(srvRecord.pszName);
    return dwError;

error:
    VMDNS_LOG_ERROR("%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

static
DWORD
VmDnsSrvCleanupAddressRecords(
    PVMDNS_ZONE_OBJECT pZoneObject,
    PSTR pszDomain,
    PSTR pszAddressRecordName,
    VMDNS_RR_TYPE recordType
)
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_RECORD_LIST pRecordList = NULL;
    PCSTR pszRecordType = NULL;
    PSTR pszIpAddress = NULL;
    PSTR pszReverseZone = NULL;
    PVMDNS_ZONE_OBJECT pReverseZoneObject = NULL;
    PSTR pszRecordName = NULL;
    DWORD idx = 0;

    if (!pZoneObject ||
        IsNullOrEmptyString(pszDomain) ||
        IsNullOrEmptyString(pszAddressRecordName) ||
        (recordType != VMDNS_RR_TYPE_A && recordType != VMDNS_RR_TYPE_AAAA))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsRecordTypeToString(recordType, &pszRecordType);
    BAIL_ON_VMDNS_ERROR(dwError);

    // Query record(s)
    dwError = VmDnsSrvQueryRecords(
                    pZoneObject,
                    pszAddressRecordName,
                    recordType,
                    0,
                    &pRecordList
                    );
    if (dwError)
    {
        VMDNS_LOG_ERROR("%s, failed to get %s address records. Error %d",
                        __FUNCTION__,
                        pszRecordType,
                        dwError);
        dwError = ERROR_SUCCESS;
    }

    // Remove record(s)
    if (pRecordList)
    {
        if (recordType == VMDNS_RR_TYPE_A)
        {
            pszReverseZone = "in-addr.arpa.";
        }
        else
        {
            pszReverseZone = "ip6.arpa.";
        }

        dwError = VmDnsSrvFindZone(
                        pszReverseZone,
                        &pReverseZoneObject
                        );
        BAIL_ON_VMDNS_ERROR(dwError);

        for (idx = 0; idx < pRecordList->dwCurrentSize; ++idx)
        {
            PVMDNS_RECORD pRecord = pRecordList->ppRecords[idx]->pRecord;

            dwError = VmDnsSrvDeleteRecord(pZoneObject, pRecord);

            VMDNS_LOG_INFO(
                  "Cleanup %s record %s from zone %s done, status: %u.",
                  pszRecordType,
                  pRecord->pszName,
                  pszDomain,
                  dwError
                  );

            dwError = ERROR_SUCCESS;

            // Construct the PTR record name from the IP address

            if (recordType == VMDNS_RR_TYPE_A)
            {
                dwError = VmDnsIp4AddressToString(
                                pRecord->Data.A.IpAddress,
                                &pszIpAddress);
                BAIL_ON_VMDNS_ERROR(dwError);
            }
            else
            {
                dwError = VmDnsIp6AddressToString(
                                pRecord->Data.AAAA.Ip6Address,
                                &pszIpAddress);
                BAIL_ON_VMDNS_ERROR(dwError);
            }

            dwError = VmDnsGeneratePtrNameFromIp(
                            pszIpAddress,
                            &pszRecordName);
            BAIL_ON_VMDNS_ERROR(dwError);

            // Cleanup PTR record(s)

            dwError = VmDnsSrvCleanupPTRRecords(
                            pReverseZoneObject,
                            pszRecordName
                            );
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }

cleanup:
    VmDnsRecordListRelease(pRecordList);
    VmDnsZoneObjectRelease(pReverseZoneObject);
    VMDNS_SAFE_FREE_STRINGA(pszIpAddress);
    VMDNS_SAFE_FREE_STRINGA(pszRecordName);
    return dwError;

error:
    VMDNS_LOG_ERROR("%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

static
DWORD
VmDnsSrvCleanupPTRRecords(
    PVMDNS_ZONE_OBJECT pZoneObject,
    PSTR pszRecordName
)
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_RECORD_LIST pRecordList = NULL;
    DWORD idx = 0;

    if (!pZoneObject ||
        IsNullOrEmptyString(pszRecordName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    // Query record(s)
    dwError = VmDnsSrvQueryRecords(
                    pZoneObject,
                    pszRecordName,
                    VMDNS_RR_TYPE_PTR,
                    0,
                    &pRecordList
                    );
    if (dwError)
    {
        VMDNS_LOG_ERROR("%s, failed to get PTR records. Error %d",
                        __FUNCTION__,
                        dwError);
        dwError = ERROR_SUCCESS;
    }

    // Remove record(s)
    if (pRecordList)
    {
        for (idx = 0; idx < pRecordList->dwCurrentSize; ++idx)
        {
            PVMDNS_RECORD pRecord = pRecordList->ppRecords[idx]->pRecord;

            dwError = VmDnsSrvDeleteRecord(pZoneObject, pRecord);

            VMDNS_LOG_INFO(
                  "Cleanup PTR record %s from zone %s done, status: %u.",
                  pRecord->pszName,
                  pZoneObject->pszName,
                  dwError
                  );

            dwError = ERROR_SUCCESS;
        }
    }

cleanup:
    VmDnsRecordListRelease(pRecordList);
    return dwError;

error:
    VMDNS_LOG_ERROR("%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}
