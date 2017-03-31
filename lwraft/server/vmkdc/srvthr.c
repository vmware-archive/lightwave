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
 * Module Name: Vmkdc main
 *
 * Filename: srvthr.c
 *
 * Abstract: Routines to handle server threads start/stop
 *
 */
#include "includes.h"

#if 0
VOID
VmKdcSrvThrAdd(
    PVMKDC_THREAD_INFO   pThrInfo
    )
{
    BOOLEAN bInLock = FALSE;

    assert(pThrInfo);

    VMKDC_LOCK_MUTEX(bInLock, gVmkdcGlobals.mutex);

    if (gVmkdcGlobals.pSrvThrInfo)
    {
        pThrInfo->pNext = gVmkdcGlobals.pSrvThrInfo;
    }
    gVmkdcGlobals.pSrvThrInfo = pThrInfo;

    VMKDC_UNLOCK_MUTEX(bInLock, gVmkdcGlobals.mutex);
}

VOID
VmKdcSrvThrInit(
    PVMKDC_THREAD_INFO   pThrInfo,
    PVMKDC_MUTEX        pAltMutex,
    PVMKDC_COND         pAltCond,
    BOOLEAN             bJoinFlag
    )
{
    assert (pThrInfo);

    if (pAltMutex && pAltMutex != pThrInfo->mutex)
    {
        pThrInfo->mutexUsed = pAltMutex;
    }
    else
    {
        VmKdcAllocateMutex(&pThrInfo->mutex);
        pThrInfo->mutexUsed = pThrInfo->mutex;
    }

    if (pAltCond && pAltCond != pThrInfo->condition)
    {
        pThrInfo->conditionUsed = pAltCond;
    }
    else
    {
        VmKdcAllocateCondition(&pThrInfo->condition);
        pThrInfo->conditionUsed = pThrInfo->condition;
    }

    pThrInfo->bJoinThr = bJoinFlag;

    return;
}


VOID
VmKdcSrvThrFree(
    PVMKDC_THREAD_INFO   pThrInfo
    )
{
    assert(pThrInfo);

    if (pThrInfo->conditionUsed && pThrInfo->conditionUsed == pThrInfo->condition)
    {
        VMKDC_SAFE_FREE_CONDITION(pThrInfo->condition);
        pThrInfo->conditionUsed = NULL;
    }

    if (pThrInfo->mutexUsed && pThrInfo->mutexUsed == pThrInfo->mutex)
    {
        VMKDC_SAFE_FREE_MUTEX( pThrInfo->mutex );
        pThrInfo->mutexUsed = NULL;
    }

    VmKdcFreeVmKdcThread( &(pThrInfo->tid) );

    VMKDC_SAFE_FREE_MEMORY(pThrInfo);
}
#endif

VOID
VmKdcSrvThrShutdown(
    PVMKDC_THREAD_INFO   pThrInfo
    )
{
#if 0
    assert(pThrInfo);

    VmKdcSrvThrSignal(pThrInfo);

    if (pThrInfo->bJoinThr)
    {
        VmKdcThreadJoin(&pThrInfo->tid, NULL);
    }

    VmKdcSrvThrFree(pThrInfo);
#endif
}


#if 0
VOID
VmKdcSrvThrSignal(
    PVMKDC_THREAD_INFO   pThrInfo
    )
{
    BOOLEAN bInLock = FALSE;

    assert(pThrInfo);

    if (pThrInfo->mutexUsed && pThrInfo->conditionUsed)
    {
        VMKDC_LOCK_MUTEX(bInLock, pThrInfo->mutexUsed);

        VmKdcConditionSignal(pThrInfo->conditionUsed);

        VMKDC_UNLOCK_MUTEX(bInLock, pThrInfo->mutexUsed);
    }

    return;
}
#endif
