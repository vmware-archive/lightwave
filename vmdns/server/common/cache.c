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
* Module Name:  forward.c
*
* Abstract: VMware Domain Name Service.
*
* DNS forwarding routines
*/

#include "includes.h"

DWORD
VmDnsCoreInit(
    BOOL bUseDirectoryStore
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_ZONE_LIST pZoneList = NULL;
    PVMDNS_FORWARDER_CONETXT pForwarderContext = NULL;

    if (gpDNSDriverGlobals->pZoneList != NULL ||
        gpDNSDriverGlobals->pForwarderContext != NULL)
    {
        dwError = ERROR_ALREADY_INITIALIZED;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsZoneListInit(&pZoneList);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsForwarderInit(&pForwarderContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    gpDNSDriverGlobals->pZoneList = pZoneList;
    gpDNSDriverGlobals->pForwarderContext = pForwarderContext;
    gpDNSDriverGlobals->bUseDirectoryStore = bUseDirectoryStore;
    VmDnsSetState(VMDNS_UNINITIALIZED);

cleanup:
    return dwError;

error:

    if (pZoneList)
    {
        VmDnsZoneListCleanup(pZoneList);
    }

    if (pForwarderContext)
    {
        VmDnsForwarderCleanup(pForwarderContext);
    }

    VmDnsCoreCleanup();

    goto cleanup;
}

VOID
VmDnsCoreCleanup()
{
    VmDnsSetState(VMDNS_UNINITIALIZED);

    if (gpDNSDriverGlobals->pZoneList)
    {
        VmDnsZoneListCleanup(gpDNSDriverGlobals->pZoneList);
        gpDNSDriverGlobals->pZoneList = NULL;
    }

    if (gpDNSDriverGlobals->pForwarderContext)
    {
        VmDnsForwarderCleanup(gpDNSDriverGlobals->pForwarderContext);
        gpDNSDriverGlobals->pForwarderContext = NULL;
    }
}

VMDNS_STATE VmDnsGetState()
{
    return gpDNSDriverGlobals->state;
}

VMDNS_STATE VmDnsSetState(
    VMDNS_STATE newState
    )
{
    VMDNS_STATE oldState = InterlockedExchange(
                                (LONG*)&gpDNSDriverGlobals->state,
                                newState);
    if (oldState != newState)
    {
        VMDNS_LOG_INFO("State changed from %u to %u", oldState, newState);
    }
    return oldState;
}

VMDNS_STATE VmDnsConditionalSetState(
    VMDNS_STATE newState,
    VMDNS_STATE oldState
    )
{
    VMDNS_STATE initState = InterlockedCompareExchange(
                (LONG*)&gpDNSDriverGlobals->state,
                newState,
                oldState);
    if (initState != newState)
    {
        if (initState != oldState)
        {
            VMDNS_LOG_DEBUG(
                "%s State not changed. Init state %u different from "
                "conditional initial state %u",
                __FUNCTION__,
                initState,
                oldState);
        }
        else
        {
            VMDNS_LOG_INFO("State changed from %u to %u", oldState, newState);
        }
    }

    return initState;
}

UINT32
VmDnsInitialize(
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx,
    PVMDNS_INIT_INFO            pInitInfo
    )
{
    DWORD dwError = ERROR_SUCCESS;
    VMDNS_ZONE_INFO zoneInfo = { 0 };
    VMDNS_RECORD nsRecord = { 0 };
    VMDNS_RECORD ldapSrvRecord = { 0 };
    VMDNS_RECORD kerberosSrvRecord = { 0 };
    VMDNS_RECORD ip4AddrRecord = { 0 };
    VMDNS_RECORD ip6AddrRecord = { 0 };
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

    // Add NS record
    nsRecord.pszName = pInitInfo->pszDomain;
    nsRecord.Data.NS.pNameHost = pInitInfo->pszDcSrvName;
    nsRecord.iClass = VMDNS_CLASS_IN;
    nsRecord.dwType = VMDNS_RR_TYPE_NS;
    nsRecord.dwTtl = VMDNS_DEFAULT_TTL;

    dwError = VmDnsZoneAddRecord(pCtx,
        pInitInfo->pszDomain,
        &nsRecord,
        TRUE);
    if (dwError == ERROR_ALREADY_EXISTS)
    {
        dwError = ERROR_SUCCESS;
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

    dwError = VmDnsZoneAddRecord(
                    pCtx,
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

    dwError = VmDnsZoneAddRecord(
                    pCtx,
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

        dwError = VmDnsZoneAddRecord(
                        pCtx,
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

        dwError = VmDnsZoneAddRecord(
                        pCtx,
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

UINT32
VmDnsUninitialize(
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx,
    PVMDNS_INIT_INFO            pInitInfo
    )
{
    DWORD dwError = ERROR_SUCCESS;
    VMDNS_RECORD srvRecord = { 0 };
    PVMDNS_RECORD_ARRAY pNsRecordArray = NULL;
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

    // Query NS record(s)
    dwError = VmDnsZoneQuery(
                        pInitInfo->pszDomain,
                        pInitInfo->pszDomain,
                        VMDNS_RR_TYPE_NS,
                        &pNsRecordArray);
    if (dwError)
    {
        VMDNS_LOG_ERROR("%s, failed to get NS records.", __FUNCTION__);
    }

    // Query LDAP TCP SRV record(s)
    dwError = VmDnsZoneQuery(
                        pInitInfo->pszDomain,
                        VMDNS_LDAP_SRV_NAME,
                        VMDNS_RR_TYPE_SRV,
                        &pLdapSrvRecordArray);
    if (dwError)
    {
        VMDNS_LOG_ERROR("%s, failed to get LDAP SRV records.", __FUNCTION__);
    }

    // Query KDC TCP SRV record(s)
    dwError = VmDnsZoneQuery(
                        pInitInfo->pszDomain,
                        VMDNS_KERBEROS_SRV_NAME,
                        VMDNS_RR_TYPE_SRV,
                        &pKerberosSrvRecordArray);
    if (dwError)
    {
        VMDNS_LOG_ERROR("%s, failed to get KDC SRV records.", __FUNCTION__);
    }

    // Query A record(s)
    dwError = VmDnsZoneQuery(
                        pInitInfo->pszDomain,
                        pszAddressRecordName,
                        VMDNS_RR_TYPE_A,
                        &pIpV4RecordArray);
    if (dwError)
    {
        VMDNS_LOG_ERROR("%s, failed to get IPV4 address records.", __FUNCTION__);
    }

    // Query AAAA record(s)
    dwError = VmDnsZoneQuery(
                        pInitInfo->pszDomain,
                        pszAddressRecordName,
                        VMDNS_RR_TYPE_AAAA,
                        &pIpV6RecordArray);
    if (dwError)
    {
        VMDNS_LOG_ERROR("%s, failed to get IPV6 address records.", __FUNCTION__);
    }

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

    // Remove NS record
    if (pNsRecordArray)
    {
        for (idx = 0; idx < pNsRecordArray->dwCount; ++idx)
        {
            if (!VmDnsStringCompareA(
                            pNsRecordArray->Records[idx].Data.NS.pNameHost,
                            pInitInfo->pszDcSrvName,
                            FALSE))
            {
                dwError = VmDnsZoneDeleteRecord(pCtx,
                                    pInitInfo->pszDomain,
                                    &pNsRecordArray->Records[idx],
                                    TRUE);
                VMDNS_LOG_INFO(
                        "Cleanup NS record %s from zone %s, status: %u.",
                        pNsRecordArray->Records[idx].pszName,
                        pInitInfo->pszDomain,
                        dwError);
            }
        }
    }

    srvRecord.pszName = VMDNS_LDAP_SRV_NAME;
    srvRecord.dwType = VMDNS_RR_TYPE_SRV;
    srvRecord.iClass = VMDNS_CLASS_IN;
    srvRecord.Data.SRV.pNameTarget = pInitInfo->pszDcSrvName;

    // Remove LDAP TCP SRV record(s)
    if (pLdapSrvRecordArray)
    {
        for (idx = 0; idx < pLdapSrvRecordArray->dwCount; ++idx)
        {
            if (VmDnsMatchRecord(
                        &srvRecord,
                        &pLdapSrvRecordArray->Records[idx]))
            {
                dwError = VmDnsZoneDeleteRecord(pCtx,
                                    pInitInfo->pszDomain,
                                    &pLdapSrvRecordArray->Records[idx],
                                    TRUE);
                VMDNS_LOG_INFO(
                        "Cleanup LDAP:SRV record %s from zone %s, status: %u.",
                        pLdapSrvRecordArray->Records[idx].pszName,
                        pInitInfo->pszDomain,
                        dwError);
            }
        }
    }

    srvRecord.pszName = VMDNS_KERBEROS_SRV_NAME;

    // Remove KDC TCP SRV record(s)
    if (pKerberosSrvRecordArray)
    {
        for (idx = 0; idx < pKerberosSrvRecordArray->dwCount; ++idx)
        {
            if (VmDnsMatchRecord(
                        &srvRecord,
                        &pKerberosSrvRecordArray->Records[idx]))
            {
                dwError = VmDnsZoneDeleteRecord(pCtx,
                                        pInitInfo->pszDomain,
                                        &pKerberosSrvRecordArray->Records[idx],
                                        TRUE);
                VMDNS_LOG_INFO(
                        "Cleanup KDC:SRV record %s from zone %s, status: %u.",
                        pKerberosSrvRecordArray->Records[idx].pszName,
                        pInitInfo->pszDomain,
                        dwError);
            }
        }
    }

    // Remove A record(s)
    if (pIpV4RecordArray)
    {
        for (idx = 0; idx < pIpV4RecordArray->dwCount; ++idx)
        {
            dwError = VmDnsZoneDeleteRecord(pCtx,
                                pInitInfo->pszDomain,
                                &pIpV4RecordArray->Records[idx],
                                TRUE);
            VMDNS_LOG_INFO(
                   "Cleanup A record %s from zone %s done, status: %u.",
                    pIpV4RecordArray->Records[idx].pszName,
                    pInitInfo->pszDomain,
                    dwError);
        }
    }

    // Remove AAAA record(s)
    if (pIpV6RecordArray)
    {
        for (idx = 0; idx < pIpV6RecordArray->dwCount; ++idx)
        {
            dwError = VmDnsZoneDeleteRecord(pCtx,
                                pInitInfo->pszDomain,
                                &pIpV6RecordArray->Records[idx],
                                TRUE);
            VMDNS_LOG_INFO(
                   "Cleanup AAAA record %s from zone %s done, status: %u.",
                    pIpV6RecordArray->Records[idx].pszName,
                    pInitInfo->pszDomain,
                    dwError);
        }
    }

    VmDnsSetState(VMDNS_UNINITIALIZED);

cleanup:
    VMDNS_FREE_RECORD_ARRAY(pKerberosSrvRecordArray);
    VMDNS_FREE_RECORD_ARRAY(pLdapSrvRecordArray);
    VMDNS_FREE_RECORD_ARRAY(pNsRecordArray);
    VMDNS_FREE_RECORD_ARRAY(pIpV4RecordArray);
    VMDNS_FREE_RECORD_ARRAY(pIpV6RecordArray);
    VMDNS_SAFE_FREE_STRINGA(pszAddressRecordName);
    return dwError;
error:
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);
    goto cleanup;
}
