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

static
UINT32
VmDnsInitialize(
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx,
    PVMDNS_INIT_INFO            pInitInfo
);

static
UINT32
VmDnsUninitialize(
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx,
    PVMDNS_INIT_INFO            pInitInfo
);

UINT32
VmDnsRpcInitialize(
    handle_t            hBinding,
    PVMDNS_INIT_INFO    pInitInfo
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx = NULL;

    dwError = VmDnsCheckAccess(hBinding, TRUE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmdnsZoneBeginUpdate(&pCtx);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsInitialize(pCtx, pInitInfo);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmdnsZoneEndUpdate(pCtx);
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
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx = NULL;

    dwError = VmDnsCheckAccess(hBinding, TRUE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmdnsZoneBeginUpdate(&pCtx);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsUninitialize(pCtx, pInitInfo);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmdnsZoneEndUpdate(pCtx);
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
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx = NULL;

    dwError = VmDnsCheckAccess(hBinding, TRUE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmdnsZoneBeginUpdate(&pCtx);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneCreate(pCtx, pZoneInfo, TRUE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmdnsZoneEndUpdate(pCtx);
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
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx = NULL;

    dwError = VmDnsCheckAccess(hBinding, TRUE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmdnsZoneBeginUpdate(&pCtx);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneUpdate(pCtx, pZoneInfo, TRUE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmdnsZoneEndUpdate(pCtx);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
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
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx = NULL;

    dwError = VmDnsCheckAccess(hBinding, TRUE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmdnsZoneBeginUpdate(&pCtx);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneDelete(pCtx, pszZone);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmdnsZoneEndUpdate(pCtx);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
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
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx = NULL;

    dwError = VmDnsCheckAccess(hBinding, TRUE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmdnsZoneBeginUpdate(&pCtx);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneAddRecord(pCtx, pszZone, pRecord, TRUE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmdnsZoneEndUpdate(pCtx);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
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
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx = NULL;

    dwError = VmDnsCheckAccess(hBinding, TRUE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmdnsZoneBeginUpdate(&pCtx);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneDeleteRecord(pCtx, pszZone, pRecord, TRUE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmdnsZoneEndUpdate(pCtx);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
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
    PVMDNS_RECORD_ARRAY pRecordArrayTemp = NULL;
    PVMDNS_RECORD_ARRAY pRecordArray = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(ppRecordArray, dwError);
    *ppRecordArray = NULL;

    dwError = VmDnsCheckAccess(hBinding, FALSE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneQuery(pszZone, pszName, dwType, &pRecordArrayTemp);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneRestoreRecordFQDN(pszZone, pRecordArrayTemp);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRpcCopyRecordArray(pRecordArrayTemp, &pRecordArray);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppRecordArray = pRecordArray;
    pRecordArray = NULL;

cleanup:
    VMDNS_FREE_RECORD_ARRAY(pRecordArrayTemp);
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

    dwError = VmDnsZoneList(&pZoneArrayTemp);
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
    PVMDNS_RECORD_ARRAY pRecordArrayTemp = NULL;
    PVMDNS_RECORD_ARRAY pRecordArray = NULL;

    BAIL_ON_VMDNS_EMPTY_STRING(pszZone, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppRecordArray, dwError);
    *ppRecordArray = NULL;

    dwError = VmDnsCheckAccess(hBinding, FALSE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneListRecord(pszZone, &pRecordArrayTemp);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneRestoreRecordFQDN(pszZone, pRecordArrayTemp);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRpcCopyRecordArray(pRecordArrayTemp, &pRecordArray);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppRecordArray = pRecordArray;
    pRecordArray = NULL;

cleanup:
    VMDNS_FREE_RECORD_ARRAY(pRecordArrayTemp);
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

    dwError = VmDnsAddForwarder(
                        VmDnsGetForwarderContext(),
                        pszForwarder);
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

    pDnsForwarders->dwCount = 0;

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

    dwError = VmDnsGetForwarders(
                        VmDnsGetForwarderContext(),
                        &ppszForwarders,
                        &dwCount);
    BAIL_ON_VMDNS_ERROR(dwError);

    if (dwCount > 0)
    {
        dwError = VmDnsRpcAllocateForwarders(
                        ppszForwarders,
                        dwCount,
                        &pDnsForwarders);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

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

    dwError = VmDnsDeleteForwarder(
                        VmDnsGetForwarderContext(),
                        pszForwarder);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

static
UINT32
VmDnsInitialize(
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx,
    PVMDNS_INIT_INFO            pInitInfo
)
{
    DWORD dwError = ERROR_SUCCESS;
    VMDNS_ZONE_INFO zoneInfo = {0};
    VMDNS_RECORD ldapSrvRecord = {0};
    VMDNS_RECORD kerberosSrvRecord = {0};
    VMDNS_RECORD ip4AddrRecord = {0};
    VMDNS_RECORD ip6AddrRecord = {0};
    DWORD idx = 0;
    VMDNS_STATE oldState;
    PSTR pszAddressRecordName = NULL;

    oldState = VmDnsConditionalSetState(VMDNS_INITIALIZING, VMDNS_UNINITIALIZED);
    if (oldState != VMDNS_UNINITIALIZED)
    {
        VmDnsLog(VMDNS_LOG_LEVEL_ERROR,
            "%s failed. Invalid current state %u, expecting %u.",
            __FUNCTION__,
            oldState,
            VMDNS_UNINITIALIZED);
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (IsNullOrEmptyString(pInitInfo->pszDcSrvName))
    {
        VmDnsLog(VMDNS_LOG_LEVEL_ERROR,
            "%s failed. Null or empty DC server name.",
            __FUNCTION__);
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateStringA(pInitInfo->pszDcSrvName, &pszAddressRecordName);
    BAIL_ON_VMDNS_ERROR(dwError);

    VmDnsTrimDomainNameSuffix(pszAddressRecordName, pInitInfo->pszDomain);

    zoneInfo.pszName = pInitInfo->pszDomain;
    zoneInfo.pszPrimaryDnsSrvName = pInitInfo->pszDcSrvName;
    zoneInfo.pszRName = "";
    zoneInfo.serial = 0;
    zoneInfo.refreshInterval = VMDNS_DEFAULT_REFRESH_INTERVAL;
    zoneInfo.retryInterval = VMDNS_DEFAULT_RETRY_INTERVAL;
    zoneInfo.expire = VMDNS_DEFAULT_EXPIRE;
    zoneInfo.minimum = VMDNS_DEFAULT_TTL;

    dwError = VmDnsZoneCreate(pCtx, &zoneInfo, TRUE);
    if (dwError == ERROR_ALREADY_EXISTS)
    {
        VMDNS_LOG_INFO("%s zone %s already exists.", __FUNCTION__, zoneInfo.pszName);
        dwError = 0;
    }
    BAIL_ON_VMDNS_ERROR(dwError);

    // Add LDAP SRV record
    ldapSrvRecord.pszName = VMDNS_LDAP_SRV_NAME;
    ldapSrvRecord.dwType = VMDNS_RR_TYPE_SRV;
    ldapSrvRecord.iClass = VMDNS_CLASS_IN;
    ldapSrvRecord.dwTtl = VMDNS_DEFAULT_TTL;
    ldapSrvRecord.Data.SRV.pNameTarget = pInitInfo->pszDcSrvName;
    ldapSrvRecord.Data.SRV.wPriority = 1;
    ldapSrvRecord.Data.SRV.wWeight = 1;
    ldapSrvRecord.Data.SRV.wPort = VMDNS_DEFAULT_LDAP_PORT;

    dwError = VmDnsZoneAddRecord(pCtx,
                    pInitInfo->pszDomain,
                    &ldapSrvRecord,
                    TRUE); // LDAP should be up by now
    if (dwError == ERROR_ALREADY_EXISTS)
    {
        VMDNS_LOG_INFO(
            "%s: LDAP TCP SRV record already exists: %s",
            __FUNCTION__,
            ldapSrvRecord.Data.SRV.pNameTarget);
        dwError = ERROR_SUCCESS;
    }
    BAIL_AND_LOG_MESSAGE_ON_VMDNS_ERROR(
        dwError,
        VMDNS_LOG_LEVEL_ERROR,
        "Adding LDAP SRV record failed.");
    VMDNS_LOG_INFO(
        "%s: Added LDAP TCP SRV record for %s",
        __FUNCTION__,
        ldapSrvRecord.Data.SRV.pNameTarget);

    // Add Kerberos SRV record
    kerberosSrvRecord.pszName = VMDNS_KERBEROS_SRV_NAME;
    kerberosSrvRecord.dwType = VMDNS_RR_TYPE_SRV;
    kerberosSrvRecord.iClass = VMDNS_CLASS_IN;
    kerberosSrvRecord.dwTtl = VMDNS_DEFAULT_TTL;
    kerberosSrvRecord.Data.SRV.pNameTarget = pInitInfo->pszDcSrvName;
    kerberosSrvRecord.Data.SRV.wPriority = 1;
    kerberosSrvRecord.Data.SRV.wWeight = 1;
    kerberosSrvRecord.Data.SRV.wPort = VMDNS_DEFAULT_KDC_PORT;

    dwError = VmDnsZoneAddRecord(pCtx,
                    pInitInfo->pszDomain,
                    &kerberosSrvRecord,
                    TRUE);
    if (dwError == ERROR_ALREADY_EXISTS)
    {
        VMDNS_LOG_INFO(
            "%s: KDC TCP SRV record already exists: %s",
            __FUNCTION__,
            kerberosSrvRecord.Data.SRV.pNameTarget);
        dwError = ERROR_SUCCESS;
    }
    BAIL_ON_VMDNS_ERROR(dwError);
    VMDNS_LOG_INFO(
        "%s: Added KDC TCP SRV record for %s",
        __FUNCTION__,
        kerberosSrvRecord.Data.SRV.pNameTarget);

    // Add A record(s)
    ip4AddrRecord.pszName = pszAddressRecordName;
    ip4AddrRecord.dwType = VMDNS_RR_TYPE_A;
    ip4AddrRecord.iClass = VMDNS_CLASS_IN;
    ip4AddrRecord.dwTtl = VMDNS_DEFAULT_TTL;
    for (idx = 0; idx < pInitInfo->IpV4Addrs.dwCount; ++idx)
    {
        ip4AddrRecord.Data.A.IpAddress = pInitInfo->IpV4Addrs.Addrs[idx];

        dwError = VmDnsZoneAddRecord(pCtx,
                        pInitInfo->pszDomain,
                        &ip4AddrRecord,
                        TRUE);
        if (dwError == ERROR_ALREADY_EXISTS)
        {
            dwError = ERROR_SUCCESS;
            continue;
        }
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    // Add AAAA record(s)
    ip6AddrRecord.pszName = pszAddressRecordName;
    ip6AddrRecord.dwType = VMDNS_RR_TYPE_AAAA;
    ip6AddrRecord.iClass = VMDNS_CLASS_IN;
    ip6AddrRecord.dwTtl = VMDNS_DEFAULT_TTL;
    for (idx = 0; idx < pInitInfo->IpV6Addrs.dwCount; ++idx)
    {
        dwError = VmDnsCopyMemory(
                          ip6AddrRecord.Data.AAAA.Ip6Address.IP6Byte,
                          sizeof(ip6AddrRecord.Data.AAAA.Ip6Address.IP6Byte),
                          pInitInfo->IpV6Addrs.Addrs[idx].IP6Byte,
                          sizeof(pInitInfo->IpV6Addrs.Addrs[idx].IP6Byte));
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsZoneAddRecord(pCtx,
                        pInitInfo->pszDomain,
                        &ip6AddrRecord,
                        TRUE);
        if (dwError == ERROR_ALREADY_EXISTS)
        {
            dwError = ERROR_SUCCESS;
            continue;
        }
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    VmDnsSetState(VMDNS_INITIALIZED);

cleanup:
    VMDNS_SAFE_FREE_STRINGA(pszAddressRecordName);
    return dwError;
error:
    VmDnsSetState(VMDNS_UNINITIALIZED);
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);
    goto cleanup;
}

static
UINT32
VmDnsUninitialize(
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx,
    PVMDNS_INIT_INFO            pInitInfo
)
{
    DWORD dwError = ERROR_SUCCESS;
    VMDNS_RECORD srvRecord = {0};
    PVMDNS_RECORD_ARRAY pLdapSrvRecordArray = NULL;
    PVMDNS_RECORD_ARRAY pKerberosSrvRecordArray = NULL;
    PVMDNS_RECORD_ARRAY pIpV4RecordArray = NULL;
    PVMDNS_RECORD_ARRAY pIpV6RecordArray = NULL;
    DWORD idx = 0;
    VMDNS_STATE oldState;
    PSTR pszAddressRecordName = NULL;

    if (IsNullOrEmptyString(pInitInfo->pszDcSrvName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateStringA(pInitInfo->pszDcSrvName, &pszAddressRecordName);
    BAIL_ON_VMDNS_ERROR(dwError);

    VmDnsTrimDomainNameSuffix(pszAddressRecordName, pInitInfo->pszDomain);

    // Try initialized state first, then ready state, because state never goes
    // back to initialized from ready, we won't have a race condition.
    oldState = VmDnsConditionalSetState(VMDNS_UNINITIALIZING, VMDNS_INITIALIZED);
    if (oldState != VMDNS_INITIALIZED)
    {
        oldState = VmDnsConditionalSetState(VMDNS_UNINITIALIZING, VMDNS_READY);
        if (oldState != VMDNS_READY)
        {
            dwError = ERROR_INVALID_STATE;
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }

    srvRecord.pszName = VMDNS_LDAP_SRV_NAME;
    srvRecord.dwType = VMDNS_RR_TYPE_SRV;
    srvRecord.iClass = VMDNS_CLASS_IN;
    srvRecord.Data.SRV.pNameTarget = pInitInfo->pszDcSrvName;

    // Remove LDAP TCP SRV record(s)
    dwError = VmDnsZoneQuery(
                pInitInfo->pszDomain,
                VMDNS_LDAP_SRV_NAME,
                VMDNS_RR_TYPE_SRV,
                &pLdapSrvRecordArray);
    if (!dwError)
    {
        for (idx = 0; idx < pLdapSrvRecordArray->dwCount; ++idx)
        {
            if (VmDnsMatchRecord(&srvRecord, &pLdapSrvRecordArray->Records[idx]))
            {
                dwError = VmDnsZoneDeleteRecord(pCtx,
                                pInitInfo->pszDomain,
                                &pLdapSrvRecordArray->Records[idx],
                                TRUE);
                VMDNS_LOG_ERROR(
                    "%s failed to delete LDAP SRV record. Error %u.",
                    __FUNCTION__,
                    dwError);
            }
        }
    }

    srvRecord.pszName = VMDNS_KERBEROS_SRV_NAME;

    // Remove KDC TCP SRV record(s)
    dwError = VmDnsZoneQuery(
                pInitInfo->pszDomain,
                VMDNS_KERBEROS_SRV_NAME,
                VMDNS_RR_TYPE_SRV,
                &pKerberosSrvRecordArray);
    if (!dwError)
    {
        for (idx = 0; idx < pKerberosSrvRecordArray->dwCount; ++idx)
        {
            if (VmDnsMatchRecord(&srvRecord, &pKerberosSrvRecordArray->Records[idx]))
            {
                dwError = VmDnsZoneDeleteRecord(pCtx,
                                pInitInfo->pszDomain,
                                &pKerberosSrvRecordArray->Records[idx],
                                TRUE);
                VMDNS_LOG_ERROR(
                    "%s failed to delete KERBEROS SRV record. Error %u.",
                    __FUNCTION__,
                    dwError);
            }
        }
    }

    // Remove A record(s)
    dwError = VmDnsZoneQuery(
                pInitInfo->pszDomain,
                pszAddressRecordName,
                VMDNS_RR_TYPE_A,
                &pIpV4RecordArray);
    if (!dwError)
    {
        for (idx = 0; idx < pIpV4RecordArray->dwCount; ++idx)
        {
            dwError = VmDnsZoneDeleteRecord(pCtx,
                            pInitInfo->pszDomain,
                            &pIpV4RecordArray->Records[idx],
                            TRUE);
                VMDNS_LOG_ERROR(
                    "%s failed to delete address record. Error %u.",
                    __FUNCTION__,
                    dwError);
        }
    }

    // Remove AAAA record(s)
    dwError = VmDnsZoneQuery(
                pInitInfo->pszDomain,
                pszAddressRecordName,
                VMDNS_RR_TYPE_AAAA,
                &pIpV6RecordArray);
    if (!dwError)
    {
        for (idx = 0; idx < pIpV6RecordArray->dwCount; ++idx)
        {
            dwError = VmDnsZoneDeleteRecord(pCtx,
                            pInitInfo->pszDomain,
                            &pIpV6RecordArray->Records[idx],
                            TRUE);
                VMDNS_LOG_ERROR(
                    "%s failed to delete ipv6 address record. Error %u.",
                    __FUNCTION__,
                    dwError);
        }
    }

    VmDnsSetState(VMDNS_UNINITIALIZED);

cleanup:
    VMDNS_FREE_RECORD_ARRAY(pKerberosSrvRecordArray);
    VMDNS_FREE_RECORD_ARRAY(pLdapSrvRecordArray);
    VMDNS_FREE_RECORD_ARRAY(pIpV4RecordArray);
    VMDNS_FREE_RECORD_ARRAY(pIpV6RecordArray);
    VMDNS_SAFE_FREE_STRINGA(pszAddressRecordName);
    return dwError;
error:
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);
    goto cleanup;
}
