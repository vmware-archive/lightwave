/*
* Copyright (c) VMware Inc. 2015  All rights Reserved.
*
* Module Name:  forward.c
*
* Abstract: VMware Domain Name Service.
*
* DNS forwarding routines
*/
#include "includes.h"

DWORD
VmDnsCoreInit()
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
        gpDNSDriverGlobals->pZoneList = NULL;
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
    return InterlockedExchange(
                (LONG*)&gpDNSDriverGlobals->state,
                newState);
}

VMDNS_STATE VmDnsConditionalSetState(
    VMDNS_STATE newState,
    VMDNS_STATE oldState
    )
{
    return InterlockedCompareExchange(
                (LONG*)&gpDNSDriverGlobals->state,
                newState,
                oldState);
}
