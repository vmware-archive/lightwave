/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 */

#include "includes.h"

DWORD
VmDirEventInit(
    PVDIR_EVENT*    ppEvent
    )
{
    DWORD       dwError = 0;
    PVDIR_EVENT pEvent = NULL;

    if (!ppEvent)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(sizeof(VDIR_EVENT), (PVOID *)&pEvent);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(sizeof(VDIR_EVENT_DATA), (PVOID *)&pEvent->pEventData);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&pEvent->pMutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateCondition(&pEvent->pCond);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppEvent = pEvent;

cleanup:
    return dwError;

error:
    VmDirEventFree(pEvent);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirEventRelease(
    PVDIR_EVENT   pEvent
    )
{
    DWORD           dwError = 0;
    BOOL            bInLock = FALSE;
    PVMDIR_MUTEX    pMutex = NULL;

    if (!pEvent)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pMutex = pEvent->pMutex;

    VMDIR_LOCK_MUTEX(bInLock, pMutex);

    if (pEvent->refCount == 0)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_WATCH_INVALID_REFCOUNT);
    }
    pEvent->refCount--;

    if (pEvent->refCount == 0)
    {
        dwError = VmDirLinkedListRemove(pEvent->pListNode->pList, pEvent->pListNode);
        BAIL_ON_VMDIR_ERROR(dwError);
        VMDIR_UNLOCK_MUTEX(bInLock, pMutex);
        VmDirEventFree(pEvent);
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, pMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirEventAcquire(
    PVDIR_EVENT   pEvent
    )
{
    BOOL    bInLock = FALSE;
    DWORD   dwError = 0;

    if (!pEvent)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_LOCK_MUTEX(bInLock, pEvent->pMutex);

    pEvent->refCount++;

    VMDIR_UNLOCK_MUTEX(bInLock, pEvent->pMutex);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

VOID
VmDirEventFree(
    PVDIR_EVENT pEvent
    )
{
    if (pEvent)
    {
        VmDirFreeMutex(pEvent->pMutex);
        VmDirFreeCondition(pEvent->pCond);
        if (pEvent->pEventData)
        {
            VmDirFreeEntry(pEvent->pEventData->pCurEntry);
            VmDirFreeEntry(pEvent->pEventData->pPrevEntry);
        }
        VMDIR_SAFE_FREE_MEMORY(pEvent->pEventData);
        VMDIR_SAFE_FREE_MEMORY(pEvent);
    }
}
