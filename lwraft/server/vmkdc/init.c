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
DWORD
InitializeResourceLimit(
    VOID
    );

static
VOID
InitializeGlobals(
    PVMKDC_GLOBALS pGlobals)
{
    pGlobals->iAcceptSock = -1;
    pGlobals->iAcceptSockUdp = -1;
    pGlobals->workerThreadMax = 10; // Get from registry configuration
    pGlobals->workerThreadCount = 0; // Total number of running threads
    pthread_mutex_init(&pGlobals->mutex, NULL);
    pthread_cond_init(&pGlobals->cond, NULL);
    pthread_cond_init(&pGlobals->stateCond, NULL);
    pthread_attr_init(&pGlobals->attrDetach);
    pthread_attr_setdetachstate(&pGlobals->attrDetach, TRUE);
}

/*
 * Initialize vmkdcd components
 */
DWORD
VmKdcInit()
{
    DWORD dwError = 0;
    extern VMKDC_GLOBALS gVmkdcGlobals;

    InitializeGlobals(&gVmkdcGlobals);

#ifndef _WIN32
    dwError = InitializeResourceLimit();
    BAIL_ON_VMKDC_ERROR(dwError);
#endif
    dwError = VmKdcInitKrb5(&gVmkdcGlobals.pKrb5Ctx);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcSrvOpenServicePort(&gVmkdcGlobals, VMKDC_SERVICE_PORT_TCP);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcSrvOpenServicePort(&gVmkdcGlobals, VMKDC_SERVICE_PORT_UDP);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcSrvServicePortListen(&gVmkdcGlobals);
    BAIL_ON_VMKDC_ERROR(dwError);

#if 0
    dwError = VmKdcRpcServerInit();
    BAIL_ON_VMKDC_ERROR(dwError);
#endif

    dwError = VmKdcInitConnAcceptThread(&gVmkdcGlobals);
    BAIL_ON_VMKDC_ERROR(dwError);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "VmKdcInit: done!");

error:
    return dwError;
}

/*
 * Main initialization loop.
 */
DWORD
VmKdcInitLoop(
    PVMKDC_GLOBALS pGlobals)
{
    DWORD dwError = 0;
    int sts = 0;
    time_t now = 0;
    struct timespec timeout = {0};

    while (1)
    {
        switch (VmKdcdState())
        {
            case VMKDCD_STARTUP:
                /*
                 * Try to initialize the directory.
                 */
                VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Vmkdc: initializing directory");
                dwError = VmKdcInitializeDirectory(pGlobals);
                if ( VmKdcdState() == VMKDCD_STARTUP    &&
                     dwError == 0
                   )
                {
                    VmKdcdStateSet(VMKDCD_RUNNING);
                    continue;
                }

                /*
                 * Initialization of the directory failed or stopping/shutdown noticed.
                 * Wait for a while before retrying.
                 */
                pthread_mutex_lock(&pGlobals->mutex);
                now = time(NULL);
                timeout.tv_sec = now + VMKDC_DIRECTORY_POLL_SECS;
                timeout.tv_nsec = 0;
                sts = 0;
                while (pGlobals->vmkdcdState == VMKDCD_STARTUP && sts == 0)
                {
                    sts = pthread_cond_timedwait(&pGlobals->stateCond,
                                                 &pGlobals->mutex,
                                                 &timeout);
                }
                pthread_mutex_unlock(&pGlobals->mutex);
                break;

            case VMKDCD_RUNNING:
                /*
                 * Wait until the server state changes.
                 */
                pthread_mutex_lock(&pGlobals->mutex);
                while (pGlobals->vmkdcdState == VMKDCD_RUNNING)
                {
                    pthread_cond_wait(&pGlobals->stateCond,
                                      &pGlobals->mutex);
                }
                pthread_mutex_unlock(&pGlobals->mutex);
                break;

            case VMKDC_STOPPING:
                /*
                 * Notify VmKdcShutdown() STOPPING has been received.
                 * It is now safe to tear down pGlobal mutex/condition
                 * variable resources after this point.
                 */
                VmKdcdStateSet(VMKDC_SHUTDOWN);

                /* don't break here because we can't use the global mutex any more */
                goto cleanup;

            case VMKDC_SHUTDOWN:
                goto cleanup;
        }
    }

cleanup:

    return dwError;
}

/*
 * Set process resource limits
 */
static
DWORD
InitializeResourceLimit(
    VOID
    )
{
    DWORD           dwError = 0;
    BAIL_ON_VMKDC_ERROR(dwError);

#ifndef _WIN32
    struct rlimit   VMLimit = {0};

    // unlimited virtual memory
    VMLimit.rlim_cur = RLIM_INFINITY;
    VMLimit.rlim_max = RLIM_INFINITY;

    dwError = setrlimit(RLIMIT_AS, &VMLimit);
    if (dwError != 0)
    {
        dwError = ERROR_INVALID_CONFIGURATION;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

#endif

error:

    return dwError;
}
