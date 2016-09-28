/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

VOID
VMCASrvSetState(
    VMCA_SERVER_STATE state
    )
{
    pthread_mutex_lock(&gVMCAServerGlobals.mutex);

    gVMCAServerGlobals.vmcadState = state;

    pthread_mutex_unlock(&gVMCAServerGlobals.mutex);
}

VMCA_SERVER_STATE
VMCASrvGetState(
    VOID
    )
{
    VMCA_SERVER_STATE rtnState;

    pthread_mutex_lock(&gVMCAServerGlobals.mutex);

    rtnState = gVMCAServerGlobals.vmcadState;

    pthread_mutex_unlock(&gVMCAServerGlobals.mutex);

    return rtnState;
}

VOID
VMCASrvSetFuncLevel(
    VMCA_FUNC_LEVEL dwFuncLevel
    )
{
    pthread_mutex_lock(&gVMCAServerGlobals.mutex);

    gVMCAServerGlobals.dwFuncLevel = dwFuncLevel;

    pthread_mutex_unlock(&gVMCAServerGlobals.mutex);
}

VMCA_FUNC_LEVEL
VMCASrvGetFuncLevel(
    VOID
    )
{
    VMCA_FUNC_LEVEL dwFuncLevel;

    pthread_mutex_lock(&gVMCAServerGlobals.mutex);

    dwFuncLevel = gVMCAServerGlobals.dwFuncLevel;

    pthread_mutex_unlock(&gVMCAServerGlobals.mutex);

    return dwFuncLevel;
}

DWORD
VMCASrvSetCA(
    PVMCA_X509_CA pCA
    )
{
    DWORD dwError = 0;
    BOOLEAN bLocked = FALSE;

    VMCA_LOCK_MUTEX(bLocked, &gVMCAServerGlobals.mutex);

    if (gVMCAServerGlobals.pCA)
    {
        VMCAReleaseCA(gVMCAServerGlobals.pCA);
    }

    gVMCAServerGlobals.pCA = VMCAAcquireCA(pCA);

    if (gVMCAServerGlobals.pCA != NULL)
    {
        gVMCAServerGlobals.dwFuncLevel |= VMCA_FUNC_LEVEL_SELF_CA;
    }
    else
    {
        gVMCAServerGlobals.dwFuncLevel &= (~VMCA_FUNC_LEVEL_SELF_CA);
    }

    VMCA_UNLOCK_MUTEX(bLocked, &gVMCAServerGlobals.mutex);

    return dwError;
}

DWORD
VMCASrvValidateCA(
    VOID
    )
{
    DWORD dwError = 0;
    DWORD dwFuncLevel = VMCASrvGetFuncLevel();

    if ((dwFuncLevel & VMCA_FUNC_LEVEL_SELF_CA) != (VMCA_FUNC_LEVEL_SELF_CA))
    {
        dwError = VMCA_ROOT_CA_MISSING;
    }

    return dwError;
}

DWORD
VMCASrvGetCA(
    PVMCA_X509_CA* ppCA
    )
{
    DWORD dwError = 0;
    BOOLEAN bLocked = FALSE;

    VMCA_LOCK_MUTEX(bLocked, &gVMCAServerGlobals.mutex);

    if (!gVMCAServerGlobals.pCA)
    {
        dwError = VMCA_ROOT_CA_MISSING;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    *ppCA = VMCAAcquireCA(gVMCAServerGlobals.pCA);

error:

    VMCA_UNLOCK_MUTEX(bLocked, &gVMCAServerGlobals.mutex);

    return dwError;
}

PVMCA_DIR_SYNC_PARAMS
VMCASrvGetDirSyncParams(
    VOID
    )
{
    PVMCA_DIR_SYNC_PARAMS pDirSyncParams = NULL;
    BOOLEAN bLocked = FALSE;

    VMCA_LOCK_MUTEX(bLocked, &gVMCAServerGlobals.mutex);

    pDirSyncParams = VMCASrvAcquireDirSyncParams(
                            gVMCAServerGlobals.pDirSyncParams);

    VMCA_UNLOCK_MUTEX(bLocked, &gVMCAServerGlobals.mutex);

    return pDirSyncParams;
}

PVMCA_THREAD
VMCASrvGetDirSvcThread(
    VOID
    )
{
    PVMCA_THREAD pDirSvcThr = NULL;
    BOOLEAN bLocked = FALSE;

    VMCA_LOCK_MUTEX(bLocked, &gVMCAServerGlobals.mutex);

    pDirSvcThr = VMCAAcquireThread(gVMCAServerGlobals.pDirSyncThr);

    VMCA_UNLOCK_MUTEX(bLocked, &gVMCAServerGlobals.mutex);

    return pDirSvcThr;
}

VOID
VMCASrvCleanupGlobalState(
    VOID
    )
{
    BOOLEAN bLocked = FALSE;

    VMCA_LOCK_MUTEX(bLocked, &gVMCAServerGlobals.mutex);

    if (gVMCAServerGlobals.pCA)
    {
        VMCAReleaseCA(gVMCAServerGlobals.pCA);
        gVMCAServerGlobals.pCA = NULL;
    }

    VMCA_UNLOCK_MUTEX(bLocked, &gVMCAServerGlobals.mutex);
}



