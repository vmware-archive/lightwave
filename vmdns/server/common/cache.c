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
