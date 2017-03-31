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
VOID
VmKdcStopSrvThreads(
    VOID);


static
VOID
VmKdcCleanupGlobals(
    PVMKDC_GLOBALS pGlobals);
    

/*
 * Server shutdown
 */
VOID
VmKdcShutdown(
    VOID)
{
    PVMKDC_GLOBALS pGlobals = &gVmkdcGlobals;
#if 0
    VmKdcRpcServerShutdown();
#endif

    // Free gVmkdcGlobals.pSrvThrInfo
    VmKdcStopSrvThreads();

    /* Synchronize with KDC thread it is really gone */
    pthread_mutex_lock(&pGlobals->mutex);
    while (pGlobals->vmkdcdState == VMKDC_STOPPING)
    {
        pthread_cond_wait(&pGlobals->stateCond,
                          &pGlobals->mutex);
    }
    pthread_mutex_unlock(&pGlobals->mutex);

#ifndef _WIN32
    VmKdcSrvCloseSocketAcceptFd();
#endif

    VmKdcTerminateDirectory(&gVmkdcGlobals);

    VmKdcDestroyKrb5(gVmkdcGlobals.pKrb5Ctx);

    VmKdcCleanupGlobals(&gVmkdcGlobals);
}

static
VOID
VmKdcStopSrvThreads(
    VOID)
{
#if 0
    PVMKDC_THREAD_INFO   pThrInfo = NULL;

    pthread_mutex_lock(&gVmkdcGlobals.mutex);

    pThrInfo = gVmkdcGlobals.pSrvThrInfo;

    pthread_mutex_unlock(&gVmkdcGlobals.mutex);

    // do shutdown outside lock as mutex is used for other resources too
    while (pThrInfo)
    {
        PVMKDC_THREAD_INFO pNext = pThrInfo->pNext;

        VmKdcSrvThrShutdown(pThrInfo); // this free pThrInfo
        pThrInfo = pNext;
    }
#endif

    return;
}

static
VOID
VmKdcCleanupGlobals(
    PVMKDC_GLOBALS pGlobals
    )
{
    // Free Server global 'gVmkdcServerGlobals' upon shutdown
    pthread_mutex_destroy(&pGlobals->mutex);
// Adam: TBD VmKdcFreeAbsoluteSecurityDescriptor(&gVmkdcGlobals.gpVmKdcSrvSD);
}
