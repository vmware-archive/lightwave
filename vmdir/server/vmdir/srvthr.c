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
 * Module Name: Vmdir main
 *
 * Filename: srvthr.c
 *
 * Abstract: Routines to handle server threads start/stop
 *
 */
#include "includes.h"

VOID
VmDirSrvThrAdd(
    PVDIR_THREAD_INFO   pThrInfo
    )
{
    BOOLEAN bInLock = FALSE;

    assert(pThrInfo);

    VMDIR_LOCK_MUTEX(bInLock, gVmdirGlobals.mutex);

    if (gVmdirGlobals.pSrvThrInfo)
    {
        pThrInfo->pNext = gVmdirGlobals.pSrvThrInfo;
    }
    gVmdirGlobals.pSrvThrInfo = pThrInfo;

    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirGlobals.mutex);
}

DWORD
VmDirSrvThrInit(
    PVDIR_THREAD_INFO   *ppThrInfo,
    PVMDIR_MUTEX        pAltMutex, /* OPTIONAL */
    PVMDIR_COND         pAltCond, /* OPTIONAL */
    BOOLEAN             bJoinFlag
    )
{
    DWORD dwError = 0;
    PVDIR_THREAD_INFO pThrInfo = NULL;

    dwError = VmDirAllocateMemory(sizeof(*pThrInfo), (PVOID)&pThrInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pAltMutex == NULL)
    {
        dwError = VmDirAllocateMutex(&pThrInfo->mutex);
        BAIL_ON_VMDIR_ERROR(dwError);
        pThrInfo->mutexUsed = pThrInfo->mutex;
    }
    else
    {
        pThrInfo->mutexUsed = pAltMutex;
    }

    if (pAltCond == NULL)
    {
        dwError = VmDirAllocateCondition(&pThrInfo->condition);
        BAIL_ON_VMDIR_ERROR(dwError);
        pThrInfo->conditionUsed = pThrInfo->condition;
    }
    else
    {
        pThrInfo->conditionUsed = pAltCond;
    }

    pThrInfo->bJoinThr = bJoinFlag;

    *ppThrInfo = pThrInfo;

cleanup:
    return dwError;
error:
    VmDirSrvThrFree(pThrInfo);
    goto cleanup;
}


VOID
VmDirSrvThrFree(
    PVDIR_THREAD_INFO   pThrInfo
    )
{
    if (pThrInfo == NULL)
    {
        return;
    }

    if (pThrInfo->conditionUsed && pThrInfo->conditionUsed == pThrInfo->condition)
    {
        VMDIR_SAFE_FREE_CONDITION(pThrInfo->condition);
        pThrInfo->conditionUsed = NULL;
    }

    if (pThrInfo->mutexUsed && pThrInfo->mutexUsed == pThrInfo->mutex)
    {
        VMDIR_SAFE_FREE_MUTEX( pThrInfo->mutex );
        pThrInfo->mutexUsed = NULL;
    }

    VmDirFreeVmDirThread( &(pThrInfo->tid) );

    VMDIR_SAFE_FREE_MEMORY(pThrInfo);
}

VOID
VmDirSrvThrShutdown(
    PVDIR_THREAD_INFO   pThrInfo
    )
{
    assert(pThrInfo);

    VmDirSrvThrSignal(pThrInfo);

    if (pThrInfo->bJoinThr)
    {
        VmDirThreadJoin(&pThrInfo->tid, NULL);
    }

    VmDirSrvThrFree(pThrInfo);
}


VOID
VmDirSrvThrSignal(
    PVDIR_THREAD_INFO   pThrInfo
    )
{
    BOOLEAN bInLock = FALSE;

    assert(pThrInfo);

    if (pThrInfo->mutexUsed && pThrInfo->conditionUsed)
    {
        VMDIR_LOCK_MUTEX(bInLock, pThrInfo->mutexUsed);

        VmDirConditionSignal(pThrInfo->conditionUsed);

        VMDIR_UNLOCK_MUTEX(bInLock, pThrInfo->mutexUsed);
    }

    return;
}
