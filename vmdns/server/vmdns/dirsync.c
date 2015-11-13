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

static VMDNS_DIR_SYNC_CONTEXT gDirSyncContext = { 0 };

static
DWORD
VmDnsDirCleanupSyncThread(
    );

static
DWORD
VmDnsDirInitSyncThread(
    );

static
DWORD
VmDnsDirSyncThread(
    PVOID   pArgs
    );

DWORD
VmDnsStartDirectorySync(
    )
{
    DWORD dwError = 0;

    dwError  = VmDnsCreateInitZoneContainer();
    if (dwError)
    {
        // vmdir might not be running yet. This is fine, we'll retry when we
        // add a zone.
        VmDnsLog(VMDNS_LOG_LEVEL_WARNING, "Failed to create zone container %u.", dwError);
    }

    dwError = VmDnsDirInitSyncThread();
    BAIL_ON_VMDNS_ERROR(dwError);

    if (!gDirSyncContext.pSyncThread)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    gDirSyncContext.bShutdown = FALSE;

    dwError = VmDnsCreateThread(
                    gDirSyncContext.pSyncThread,
                    FALSE,
                    VmDnsDirSyncThread,
                    &gDirSyncContext);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsStopDirectorySync(
    )
{
    DWORD dwError = 0;

    if (!gDirSyncContext.pRefreshEvent ||
        !gDirSyncContext.pSyncThread)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    gDirSyncContext.bShutdown = TRUE;
    dwError = VmDnsConditionSignal(gDirSyncContext.pRefreshEvent);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsThreadJoin(gDirSyncContext.pSyncThread, &dwError);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirCleanupSyncThread();
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsDirSyncZones(
    )
{
    DWORD dwError = 0;
    PVMDNS_DIR_CONTEXT pDirContext = NULL;
    PVMDNS_DIR_DNS_INFO pDirZoneInfo = NULL;
    DWORD dwCount = 0;
    PSTR* ppszForwarders = NULL;

    dwError = VmDnsDirConnect("localhost", &pDirContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirGetLotusZoneInfo(pDirContext, &pDirZoneInfo);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirUpdateCachedZoneInfo(pDirZoneInfo);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDirGetForwarders(
                        pDirContext,
                        &ppszForwarders,
                        &dwCount);
    if (dwError == ERROR_NO_DATA)
    {
        dwError = 0;
    }
    else
    {
        BAIL_ON_VMDNS_ERROR(dwError);
        dwError = VmDnsSetForwarders(
                            VmDnsGetForwarderContext(),
                            dwCount,
                            ppszForwarders);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:

    if (pDirZoneInfo)
    {
        VmDnsCleanupZoneInfo(pDirZoneInfo);
    }

    VmDnsFreeStringCountedArrayA(ppszForwarders, dwCount);

    VmDnsDirClose(pDirContext);

    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsDirSyncThread(
    PVOID                   pArgs
    )
{
    DWORD dwError = 0;
    PVMDNS_DIR_SYNC_CONTEXT pDirSyncContext = (PVMDNS_DIR_SYNC_CONTEXT)pArgs;

    pDirSyncContext->bRunning = TRUE;

    while (!pDirSyncContext->bShutdown)
    {
        dwError = VmDnsDirSyncZones();
        if (dwError)
        {
            VMDNS_LOG_ERROR("dirsync failed with %u.", dwError);
        }
        else
        {
            VmDnsConditionalSetState(VMDNS_READY, VMDNS_INITIALIZED);
        }

        if (!pDirSyncContext->bShutdown)
        {
            dwError = VmDnsConditionTimedWait(
                                    gDirSyncContext.pRefreshEvent,
                                    gDirSyncContext.pThreadLock,
                                    5 * 1000
                                    );
            if (dwError != ETIMEDOUT &&
                dwError != WSAETIMEDOUT &&
                dwError != ERROR_SUCCESS)
            {
                BAIL_ON_VMDNS_ERROR(dwError);
            }
        }
    }

cleanup:

    gDirSyncContext.bRunning = FALSE;

    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsDirInitSyncThread(
    )
{
    DWORD dwError = 0;
    PVMDNS_THREAD pSyncThread = NULL;
    PVMDNS_COND pRefreshEvent = NULL;
    PVMDNS_MUTEX pThreadMutex = NULL;

    dwError = VmDnsAllocateCondition(&pRefreshEvent);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateMutex(&pThreadMutex);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateMemory(sizeof(*pSyncThread), (PVOID *)&pSyncThread);
    BAIL_ON_VMDNS_ERROR(dwError);

    gDirSyncContext.pRefreshEvent = pRefreshEvent;
    gDirSyncContext.pThreadLock = pThreadMutex;
    gDirSyncContext.pSyncThread = pSyncThread;

    pSyncThread = NULL;
    pThreadMutex = NULL;
    pSyncThread = NULL;

cleanup:

    return dwError;

error:

    if (pThreadMutex)
    {
        VmDnsFreeMutex(pThreadMutex);
    }

    if (pRefreshEvent)
    {
        VmDnsFreeCondition(pRefreshEvent);
    }

    if (pSyncThread)
    {
        VmDnsFreeMemory(pSyncThread);
    }

    goto cleanup;
}

DWORD
VmDnsDirCleanupSyncThread(
    )
{
    DWORD dwError = 0;

    if (gDirSyncContext.bRunning)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    VmDnsFreeThread(gDirSyncContext.pSyncThread);
    gDirSyncContext.pThreadLock = NULL;

    VmDnsFreeCondition(gDirSyncContext.pRefreshEvent);
    gDirSyncContext.pRefreshEvent = NULL;

    VmDnsFreeMutex(gDirSyncContext.pThreadLock);
    gDirSyncContext.pThreadLock = NULL;

cleanup:

    return dwError;

error:

    goto cleanup;
}
