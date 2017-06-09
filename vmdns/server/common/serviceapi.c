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


DWORD
VmDnsSrvInitialize(
    BOOL bUseDirectoryStore
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_FORWARDER_CONETXT pForwarderContext = NULL;
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

    if (VmDnsIsUpdatePermitted(pRecord->dwType))
    {
        dwError = VmDnsSrvQueryRecords(
                            pZoneObject,
                            pRecord->pszName,
                            pRecord->dwType,
                            0,
                            &pRecordList
                            );
        BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

        if (pRecordList &&
            VmDnsRecordListGetSize(pRecordList) == 1)
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }

    dwError = VmDnsStoreAddZoneRecord(
                        pszName,
                        pRecord
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCachePurgeRecord(pZoneObject, pRecord->pszName);
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

    if (VmDnsIsUpdatePermitted(pRecord->dwType))
    {
        dwError = VmDnsSrvQueryRecords(
                            pZoneObject,
                            pRecord->pszName,
                            pRecord->dwType,
                            0,
                            &pRecordList
                            );
        BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

        if (pRecordList &&
            VmDnsRecordListGetSize(pRecordList) < 1)
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDNS_ERROR(dwError);
        }
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
VmDnsSrvQueryRecords(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PCSTR               pszName,
    VMDNS_RR_TYPE       dwType,
    DWORD               dwOptions,
    PVMDNS_RECORD_LIST  *ppRecordList
    )
{
    DWORD dwError = 0;
    PVMDNS_RECORD_LIST pRecordList = NULL;
    PSTR pszZone = NULL;
    PSTR szNameFqdn = NULL;
    PCSTR szNameQuery = NULL;

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

    if (!pRecordList)
    {
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
    DWORD dwError = ERROR_SUCCESS;
    PSTR pszAddressRecordName = NULL;
    PSTR pszLdapRecordName = NULL;
    PSTR pszKrbRecordName = NULL;
    PSTR pszLdapDcRecordName = NULL;
    PSTR pszKrbDcRecordName = NULL;
    DWORD idx = 0;

    VMDNS_ZONE_INFO zoneInfo = {
        .pszName                = pInitInfo->pszDomain,
        .pszPrimaryDnsSrvName   = pInitInfo->pszDcSrvName,
        .pszRName               = "",
        .serial                 = 0,
        .refreshInterval        = VMDNS_DEFAULT_REFRESH_INTERVAL,
        .retryInterval          = VMDNS_DEFAULT_RETRY_INTERVAL,
        .expire                 = VMDNS_DEFAULT_EXPIRE,
        .minimum                = VMDNS_DEFAULT_TTL
    };

    VMDNS_RECORD nsRecord = {
        .pszName                = pInitInfo->pszDomain,
        .Data.NS.pNameHost      = pInitInfo->pszDcSrvName,
        .iClass                 = VMDNS_CLASS_IN,
        .dwType                 = VMDNS_RR_TYPE_NS,
        .dwTtl                  = VMDNS_DEFAULT_TTL
    };

    VMDNS_RECORD ldapSrvRecord = {
        .pszName                = VMDNS_LDAP_SRV_NAME,
        .dwType                 = VMDNS_RR_TYPE_SRV,
        .iClass                 = VMDNS_CLASS_IN,
        .dwTtl                  = VMDNS_DEFAULT_TTL,
        .Data.SRV.pNameTarget   = pInitInfo->pszDcSrvName,
        .Data.SRV.wPriority     = 1,
        .Data.SRV.wWeight       = 1,
        .Data.SRV.wPort         = VMDNS_DEFAULT_LDAP_PORT
    };

    VMDNS_RECORD kerberosSrvRecord = {
        .pszName                = VMDNS_KERBEROS_SRV_NAME,
        .dwType                 = VMDNS_RR_TYPE_SRV,
        .iClass                 = VMDNS_CLASS_IN,
        .dwTtl                  = VMDNS_DEFAULT_TTL,
        .Data.SRV.pNameTarget   = pInitInfo->pszDcSrvName,
        .Data.SRV.wPriority     = 1,
        .Data.SRV.wWeight       = 1,
        .Data.SRV.wPort         = VMDNS_DEFAULT_KDC_PORT
    };

    VMDNS_RECORD ldapDcSrvRecord = {
        .pszName                = VMDNS_LDAP_DC_SRV_NAME,
        .dwType                 = VMDNS_RR_TYPE_SRV,
        .iClass                 = VMDNS_CLASS_IN,
        .dwTtl                  = VMDNS_DEFAULT_TTL,
        .Data.SRV.pNameTarget   = pInitInfo->pszDcSrvName,
        .Data.SRV.wPriority     = 1,
        .Data.SRV.wWeight       = 1,
        .Data.SRV.wPort         = VMDNS_DEFAULT_LDAP_PORT
    };

    VMDNS_RECORD kerberosDcSrvRecord = {
        .pszName                = VMDNS_KERBEROS_DC_SRV_NAME,
        .dwType                 = VMDNS_RR_TYPE_SRV,
        .iClass                 = VMDNS_CLASS_IN,
        .dwTtl                  = VMDNS_DEFAULT_TTL,
        .Data.SRV.pNameTarget   = pInitInfo->pszDcSrvName,
        .Data.SRV.wPriority     = 1,
        .Data.SRV.wWeight       = 1,
        .Data.SRV.wPort         = VMDNS_DEFAULT_KDC_PORT
    };

    if (IsNullOrEmptyString(pInitInfo->pszDcSrvName) ||
        IsNullOrEmptyString(pInitInfo->pszDomain))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateStringA(
                        pInitInfo->pszDcSrvName,
                        &pszAddressRecordName
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringPrintfA(
                            &pszLdapRecordName,
                            "%s.%s",
                            VMDNS_LDAP_SRV_NAME,
                            pInitInfo->pszDomain
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    /* LDAP DC SRV record entry */
    dwError = VmDnsAllocateStringPrintfA(
                            &pszLdapDcRecordName,
                            "%s.%s",
                            VMDNS_LDAP_DC_SRV_NAME,
                            pInitInfo->pszDomain
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringPrintfA(
                            &pszKrbRecordName,
                            "%s.%s",
                            VMDNS_KERBEROS_SRV_NAME,
                            pInitInfo->pszDomain
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    /* Kerberos DC SRV record entry */
    dwError = VmDnsAllocateStringPrintfA(
                            &pszKrbDcRecordName,
                            "%s.%s",
                            VMDNS_KERBEROS_DC_SRV_NAME,
                            pInitInfo->pszDomain
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsMakeFQDN(
                    pInitInfo->pszDcSrvName,
                    pInitInfo->pszDomain,
                    &pszAddressRecordName
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsStoreCreateZone(&zoneInfo);
    dwError = (dwError == ERROR_ALREADY_EXISTS) ? ERROR_SUCCESS : dwError;
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsStoreAddZoneRecord(
                        pInitInfo->pszDomain,
                        &nsRecord
                        );
    dwError = (dwError == ERROR_ALREADY_EXISTS) ?  ERROR_SUCCESS : dwError;
    BAIL_ON_VMDNS_ERROR(dwError);

    ldapSrvRecord.pszName = pszLdapRecordName;
    dwError = VmDnsStoreAddZoneRecord(
                        pInitInfo->pszDomain,
                        &ldapSrvRecord
                        );
    dwError = (dwError == ERROR_ALREADY_EXISTS) ? ERROR_SUCCESS : dwError;
    BAIL_ON_VMDNS_ERROR(dwError);

    kerberosSrvRecord.pszName = pszKrbRecordName;
    dwError = VmDnsStoreAddZoneRecord(
                        pInitInfo->pszDomain,
                        &kerberosSrvRecord
                        );
    dwError = (dwError == ERROR_ALREADY_EXISTS) ? ERROR_SUCCESS : dwError;
    BAIL_ON_VMDNS_ERROR(dwError);

    ldapDcSrvRecord.pszName = pszLdapDcRecordName;
    dwError = VmDnsStoreAddZoneRecord(
                        pInitInfo->pszDomain,
                        &ldapDcSrvRecord
                        );
    dwError = (dwError == ERROR_ALREADY_EXISTS) ? ERROR_SUCCESS : dwError;
    BAIL_ON_VMDNS_ERROR(dwError);

    kerberosDcSrvRecord.pszName = pszKrbDcRecordName;
    dwError = VmDnsStoreAddZoneRecord(
                        pInitInfo->pszDomain,
                        &kerberosDcSrvRecord
                        );
    dwError = (dwError == ERROR_ALREADY_EXISTS) ? ERROR_SUCCESS : dwError;
    BAIL_ON_VMDNS_ERROR(dwError);

    if (!VmDnsCheckIfIPV4AddressA(pInitInfo->pszDcSrvName) &&
        !VmDnsCheckIfIPV6AddressA(pInitInfo->pszDcSrvName))
    {
        dwError = VmDnsMakeFQDN(
                    pInitInfo->pszDcSrvName,
                    pInitInfo->pszDomain,
                    &pszAddressRecordName
                    );
        BAIL_ON_VMDNS_ERROR(dwError);

        if (VmDnsIsAuthorityZone(pszAddressRecordName, pInitInfo->pszDomain))
        {
            for (idx = 0; idx < pInitInfo->IpV4Addrs.dwCount; ++idx)
            {
                VMDNS_RECORD ip4AddrRecord =
                {
                    .pszName = pszAddressRecordName,
                    .dwType = VMDNS_RR_TYPE_A,
                    .iClass = VMDNS_CLASS_IN,
                    .dwTtl = VMDNS_DEFAULT_TTL,
                    .Data.A.IpAddress = pInitInfo->IpV4Addrs.Addrs[idx]
                };

                dwError = VmDnsStoreAddZoneRecord(
                                pInitInfo->pszDomain,
                                &ip4AddrRecord
                                );
                dwError = (dwError == ERROR_ALREADY_EXISTS) ? ERROR_SUCCESS : dwError;
                BAIL_ON_VMDNS_ERROR(dwError);
            }

            for (idx = 0; idx < pInitInfo->IpV6Addrs.dwCount; ++idx)
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
                            pInitInfo->IpV6Addrs.Addrs[idx].IP6Byte,
                            sizeof(pInitInfo->IpV6Addrs.Addrs[idx].IP6Byte)
                            );
                BAIL_ON_VMDNS_ERROR(dwError);

                dwError = VmDnsStoreAddZoneRecord(
                                pInitInfo->pszDomain,
                                &ip6AddrRecord
                                );
                dwError = (dwError == ERROR_ALREADY_EXISTS) ? ERROR_SUCCESS : dwError;
                BAIL_ON_VMDNS_ERROR(dwError);
            }
        }
    }
    dwError = VmDnsCacheLoadZoneFromStore(
                            gpSrvContext->pCacheContext,
                            pInitInfo->pszDomain
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VMDNS_SAFE_FREE_STRINGA(pszAddressRecordName);
    VMDNS_SAFE_FREE_STRINGA(pszLdapRecordName);
    VMDNS_SAFE_FREE_STRINGA(pszKrbRecordName);
    VMDNS_SAFE_FREE_STRINGA(pszLdapDcRecordName);
    VMDNS_SAFE_FREE_STRINGA(pszKrbDcRecordName);
    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsSrvCleanupDomain(
    PVMDNS_INIT_INFO            pInitInfo
    )
{
    DWORD dwError = ERROR_SUCCESS;
    VMDNS_RECORD srvRecord = { 0 };
    PVMDNS_RECORD_LIST pNsRecordList = NULL;
    PVMDNS_RECORD_LIST pLdapSrvRecordList = NULL;
    PVMDNS_RECORD_LIST pKerberosSrvRecordList = NULL;
    PVMDNS_RECORD_LIST pIpV4RecordList = NULL;
    PVMDNS_RECORD_LIST pIpV6RecordList = NULL;
    DWORD idx = 0;
    PVMDNS_ZONE_OBJECT pZoneObject = NULL;

    PSTR pszAddressRecordName = NULL;

    if (IsNullOrEmptyString(pInitInfo->pszDcSrvName))
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
                    pInitInfo->pszDomain,
                    &pZoneObject
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringA(
                        pInitInfo->pszDcSrvName,
                        &pszAddressRecordName
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    // Query NS record(s)
    dwError = VmDnsSrvQueryRecords(
                    pZoneObject,
                    pInitInfo->pszDomain,
                    VMDNS_RR_TYPE_NS,
                    0,
                    &pNsRecordList
                    );
    if (dwError)
    {
        VMDNS_LOG_ERROR("%s, failed to get NS records.", __FUNCTION__);
    }

    // Query LDAP TCP SRV record(s)
    dwError = VmDnsSrvQueryRecords(
                    pZoneObject,
                    VMDNS_LDAP_SRV_NAME,
                    VMDNS_RR_TYPE_SRV,
                    0,
                    &pLdapSrvRecordList
                    );
    if (dwError)
    {
        VMDNS_LOG_ERROR("%s, failed to get LDAP SRV records.", __FUNCTION__);
    }

    // Query KDC TCP SRV record(s)
    dwError = VmDnsSrvQueryRecords(
                    pZoneObject,
                    VMDNS_KERBEROS_SRV_NAME,
                    VMDNS_RR_TYPE_SRV,
                    0,
                    &pKerberosSrvRecordList
                    );
    if (dwError)
    {
        VMDNS_LOG_ERROR("%s, failed to get KDC SRV records.", __FUNCTION__);
    }

    // Query A record(s)
    dwError = VmDnsSrvQueryRecords(
                    pZoneObject,
                    pszAddressRecordName,
                    VMDNS_RR_TYPE_A,
                    0,
                    &pIpV4RecordList
                    );
    if (dwError)
    {
        VMDNS_LOG_ERROR("%s, failed to get IPV4 address records.", __FUNCTION__);
    }

    // Query AAAA record(s)
    dwError = VmDnsSrvQueryRecords(
                    pZoneObject,
                    pszAddressRecordName,
                    VMDNS_RR_TYPE_AAAA,
                    0,
                    &pIpV6RecordList
                    );
    if (dwError)
    {
        VMDNS_LOG_ERROR("%s, failed to get IPV6 address records.", __FUNCTION__);
    }

    // Remove NS record
    if (pNsRecordList)
    {
        for (idx = 0; idx < pNsRecordList->dwCurrentSize; ++idx)
        {
            if (!VmDnsStringCompareA(
                        pNsRecordList->ppRecords[idx]->pRecord->Data.NS.pNameHost,
                        pInitInfo->pszDcSrvName,
                        FALSE))
            {
                dwError = VmDnsSrvDeleteRecord(
                                    pZoneObject,
                                    pNsRecordList->ppRecords[idx]->pRecord
                                    );
                VMDNS_LOG_INFO(
                    "Cleanup NS record %s from zone %s, status: %u.",
                    pNsRecordList->ppRecords[idx]->pRecord->pszName,
                    pInitInfo->pszDomain,
                    dwError
                    );
            }
        }
    }

    srvRecord.pszName = VMDNS_LDAP_SRV_NAME;
    srvRecord.dwType = VMDNS_RR_TYPE_SRV;
    srvRecord.iClass = VMDNS_CLASS_IN;
    srvRecord.Data.SRV.pNameTarget = pInitInfo->pszDcSrvName;

    // Remove LDAP TCP SRV record(s)
    if (pLdapSrvRecordList)
    {
        for (idx = 0; idx < pLdapSrvRecordList->dwCurrentSize; ++idx)
        {
            if (VmDnsMatchRecord(
                        &srvRecord,
                        pLdapSrvRecordList->ppRecords[idx]->pRecord))
            {
                dwError = VmDnsSrvDeleteRecord(
                                    pZoneObject,
                                    pLdapSrvRecordList->ppRecords[idx]->pRecord
                                    );
                VMDNS_LOG_INFO(
                        "Cleanup LDAP:SRV record %s from zone %s, status: %u.",
                        pLdapSrvRecordList->ppRecords[idx]->pRecord->pszName,
                        pInitInfo->pszDomain,
                        dwError
                        );
            }
        }
    }

    srvRecord.pszName = VMDNS_KERBEROS_SRV_NAME;

    // Remove KDC TCP SRV record(s)
    if (pKerberosSrvRecordList)
    {
        for (idx = 0; idx < pKerberosSrvRecordList->dwCurrentSize; ++idx)
        {
            if (VmDnsMatchRecord(
                        &srvRecord,
                        pKerberosSrvRecordList->ppRecords[idx]->pRecord))
            {
                dwError = VmDnsSrvDeleteRecord(
                                    pZoneObject,
                                    pKerberosSrvRecordList->ppRecords[idx]->pRecord
                                    );
                VMDNS_LOG_INFO(
                        "Cleanup KDC:SRV record %s from zone %s, status: %u.",
                        pKerberosSrvRecordList->ppRecords[idx]->pRecord->pszName,
                        pInitInfo->pszDomain,
                        dwError
                        );
            }
        }
    }

    // Remove A record(s)
    if (pIpV4RecordList)
    {
        for (idx = 0; idx < pIpV4RecordList->dwCurrentSize; ++idx)
        {
            dwError = VmDnsSrvDeleteRecord(
                            pZoneObject,
                            pIpV4RecordList->ppRecords[idx]->pRecord
                            );
            VMDNS_LOG_INFO(
                    "Cleanup A record %s from zone %s done, status: %u.",
                    pIpV4RecordList->ppRecords[idx]->pRecord->pszName,
                    pInitInfo->pszDomain,
                    dwError
                    );
        }
    }

    // Remove AAAA record(s)
    if (pIpV6RecordList)
    {
        for (idx = 0; idx < pIpV6RecordList->dwCurrentSize; ++idx)
        {
            dwError = VmDnsSrvDeleteRecord(
                        pZoneObject,
                        pIpV6RecordList->ppRecords[idx]->pRecord
                        );
            VMDNS_LOG_INFO(
                    "Cleanup AAAA record %s from zone %s done, status: %u.",
                    pIpV6RecordList->ppRecords[idx]->pRecord->pszName,
                    pInitInfo->pszDomain,
                    dwError
                    );
        }
    }

cleanup:
    VmDnsZoneObjectRelease(pZoneObject);
    VmDnsRecordListRelease(pKerberosSrvRecordList);
    VmDnsRecordListRelease(pLdapSrvRecordList);
    VmDnsRecordListRelease(pNsRecordList);
    VmDnsRecordListRelease(pIpV4RecordList);
    VmDnsRecordListRelease(pIpV6RecordList);
    VMDNS_SAFE_FREE_STRINGA(pszAddressRecordName);
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
