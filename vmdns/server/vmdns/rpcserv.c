/*
 * Copyright (C) 2011 VMware, Inc. All rights reserved.
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
 *
 * Authors  : Krishna Ganugapati (krishnag@vmware.com)
 *            Sriram Nambakam (snambakam@vmware.com)
 *
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

    dwError = VmDnsCheckAccess(hBinding, FALSE);
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

    dwError = VmDnsCheckAccess(hBinding, FALSE);
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

    dwError = VmDnsCheckAccess(hBinding, FALSE);
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

    dwError = VmDnsCheckAccess(hBinding, FALSE);
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

    dwError = VmDnsCheckAccess(hBinding, FALSE);
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

    dwError = VmDnsCheckAccess(hBinding, FALSE);
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

    dwError = VmDnsCheckAccess(hBinding, FALSE);
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

    BAIL_ON_VMDNS_INVALID_POINTER(pszZone, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppRecordArray, dwError);
    *ppRecordArray = NULL;

    dwError = VmDnsCheckAccess(hBinding, FALSE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneListRecord(pszZone, &pRecordArrayTemp);
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

    BAIL_ON_VMDNS_INVALID_POINTER(pszForwarder, dwError);

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

    if (ppDnsForwarders)
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

    dwError = VmDnsCheckAccess(hBinding, TRUE);
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

    BAIL_ON_VMDNS_INVALID_POINTER(pszForwarder, dwError);

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
    VMDNS_RECORD srvRecord = {0};
    VMDNS_RECORD addrRecord = {0};
    DWORD idx = 0;
    VMDNS_STATE oldState;

    oldState = VmDnsConditionalSetState(VMDNS_INITIALIZING, VMDNS_UNINITIALIZED);
    if (oldState != VMDNS_UNINITIALIZED)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

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
        dwError = 0;
    }
    BAIL_ON_VMDNS_ERROR(dwError);

    // Add SRV record
    srvRecord.pszName = VMDNS_LDAP_SRV_NAME;
    srvRecord.dwType = VMDNS_RR_TYPE_SRV;
    srvRecord.iClass = VMDNS_CLASS_IN;
    srvRecord.dwTtl = VMDNS_DEFAULT_TTL;
    srvRecord.Data.SRV.pNameTarget = pInitInfo->pszDcSrvName;
    srvRecord.Data.SRV.wPriority = 1;
    srvRecord.Data.SRV.wWeight = 1;
    srvRecord.Data.SRV.wPort = VMDNS_DEFAULT_LDAP_PORT;

    dwError = VmDnsZoneAddRecord(pCtx,
                    pInitInfo->pszDomain,
                    &srvRecord,
                    TRUE); // LDAP should be up by now
    BAIL_ON_VMDNS_ERROR(dwError);

    // Add A record(s)
    addrRecord.pszName = pInitInfo->pszDcSrvName;
    addrRecord.dwType = VMDNS_RR_TYPE_A;
    addrRecord.iClass = VMDNS_CLASS_IN;
    addrRecord.dwTtl = VMDNS_DEFAULT_TTL;
    for (idx = 0; idx < pInitInfo->IpV4Addrs.dwCount; ++idx)
    {
        addrRecord.Data.A.IpAddress = pInitInfo->IpV4Addrs.Addrs[idx];

        dwError = VmDnsZoneAddRecord(pCtx,
                        pInitInfo->pszDomain,
                        &addrRecord,
                        TRUE);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    VmDnsSetState(VMDNS_INITIALIZED);

cleanup:
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
    PVMDNS_RECORD_ARRAY pSrvRecordArray = NULL;
    PVMDNS_RECORD_ARRAY pAddrRecordArray = NULL;
    DWORD idx = 0;
    VMDNS_STATE oldState;

    dwError = VmDnsZoneQuery(
                pInitInfo->pszDomain,
                VMDNS_LDAP_SRV_NAME,
                VMDNS_RR_TYPE_SRV,
                &pSrvRecordArray);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneQuery(
                pInitInfo->pszDomain,
                pInitInfo->pszDcSrvName,
                VMDNS_RR_TYPE_A,
                &pAddrRecordArray);
    BAIL_ON_VMDNS_ERROR(dwError);

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

    // Remove SRV record(s)
    for (idx = 0; idx < pSrvRecordArray->dwCount; ++idx)
    {
        if (VmDnsMatchRecord(&srvRecord, &pSrvRecordArray->Records[idx]))
        {
            dwError = VmDnsZoneDeleteRecord(pCtx,
                            pInitInfo->pszDomain,
                            &pSrvRecordArray->Records[idx],
                            TRUE);
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }

    // Remove A record(s)
    for (idx = 0; idx < pAddrRecordArray->dwCount; ++idx)
    {
        dwError = VmDnsZoneDeleteRecord(pCtx,
                        pInitInfo->pszDomain,
                        &pAddrRecordArray->Records[idx],
                        TRUE);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    VmDnsSetState(VMDNS_UNINITIALIZED);

cleanup:
    VMDNS_FREE_RECORD_ARRAY(pSrvRecordArray);
    VMDNS_FREE_RECORD_ARRAY(pAddrRecordArray);
    return dwError;
error:
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);
    goto cleanup;
}
