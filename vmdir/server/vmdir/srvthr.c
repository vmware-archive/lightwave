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

VOID
VmDirSrvThrInit(
    PVDIR_THREAD_INFO   pThrInfo,
    PVMDIR_MUTEX        pAltMutex,
    PVMDIR_COND         pAltCond,
    BOOLEAN             bJoinFlag
    )
{
    DWORD dwError = 0;
    assert (pThrInfo);

    if (pAltMutex && pAltMutex != pThrInfo->mutex)
    {
        pThrInfo->mutexUsed = pAltMutex;
    }
    else
    {
        dwError = VmDirAllocateMutex(&pThrInfo->mutex);
        assert(dwError == 0);
        pThrInfo->mutexUsed = pThrInfo->mutex;
    }

    if (pAltCond && pAltCond != pThrInfo->condition)
    {
        pThrInfo->conditionUsed = pAltCond;
    }
    else
    {
        dwError = VmDirAllocateCondition(&pThrInfo->condition);
        assert(dwError == 0);
        pThrInfo->conditionUsed = pThrInfo->condition;
    }

    pThrInfo->bJoinThr = bJoinFlag;

    return;
}


VOID
VmDirSrvThrFree(
    PVDIR_THREAD_INFO   pThrInfo
    )
{
    assert(pThrInfo);

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
