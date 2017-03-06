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
VmAfdCleanupGlobals(
    VOID
    );

static
VOID
VmAfdVmDirClientShutdown(
    VOID
    );

/*
 * Server shutdown
 */
VOID
VmAfdServerShutdown(
    VOID)
{
    if (gVmafdGlobals.bEnableRPC)
    {
      VmAfdRpcServerShutdown();
    }

    VmAfdIpcServerShutDown();
    VmAfdTearDownStoreHashMap();

    VmAfdVmDirClientShutdown();

    if (gVmafdGlobals.pCertUpdateThr)
    {
        VmAfdShutdownCertificateThread(
            gVmafdGlobals.pCertUpdateThr);
        gVmafdGlobals.pCertUpdateThr = NULL;
    }

    if (gVmafdGlobals.pPassRefreshThr)
    {
        VmAfdShutdownPassRefreshThread(
            gVmafdGlobals.pPassRefreshThr);
        gVmafdGlobals.pPassRefreshThr = NULL;
    }

    if (gVmafdGlobals.pCdcContext)
    {
        CdcShutdownCdcService(
            gVmafdGlobals.pCdcContext);
        gVmafdGlobals.pCdcContext= NULL;
    }

#if 0
//TODO: Comment out DDNS client code for now
    if (gVmafdGlobals.pDdnsContext)
    {
        VmDdnsShutdown(
            gVmafdGlobals.pDdnsContext);
        gVmafdGlobals.pDdnsContext = NULL;
    }

    if (gVmafdGlobals.pSourceIpContext)
    {
        VmAfdShutdownSrcIpThread(
            gVmafdGlobals.pSourceIpContext);
        gVmafdGlobals.pSourceIpContext = NULL;
    }
#endif

    VmAfCfgShutdown();

    VmAfdLogTerminate();

    VmAfdCleanupGlobals();

    VmAfdOpenSSLShutdown();
}

static
VOID
VmAfdVmDirClientShutdown(
    VOID)
{
    VmDirLogTerminate();
}

static
VOID
VmAfdCleanupGlobals(
    VOID
    )
{
    // Free Server global 'gVmafdServerGlobals' upon shutdown
    VMAFD_SAFE_FREE_STRINGA(gVmafdGlobals.pszLogFile);
    VMAFD_SAFE_FREE_STRINGA(gVmafdGlobals.pszKrb5Config);
    VMAFD_SAFE_FREE_STRINGA(gVmafdGlobals.pszKrb5Keytab);
    VMAFD_SAFE_FREE_MEMORY(gVmafdGlobals.pwszMachineId);
    pthread_mutex_destroy(&gVmafdGlobals.mutex);
    pthread_rwlock_destroy(&gVmafdGlobals.rwlockStoreMap);
    pthread_mutex_destroy(&gVmafdGlobals.mutexCreateStore);
    pthread_mutex_destroy(&gVmafdGlobals.mutexStoreState);
}
