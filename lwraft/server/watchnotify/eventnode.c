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
VmDirEventNodeInit(
    PVDIR_EVENT         pEvent,
    PVDIR_EVENT_NODE*   ppEventNode
    )
{
    DWORD               dwError = 0;
    PVDIR_EVENT_NODE    pEventNode = NULL;

    if (!ppEventNode || !pEvent)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(sizeof(VDIR_EVENT_NODE), (PVOID *)&pEventNode);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&pEventNode->pMutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    pEventNode->pEvent = pEvent;

    *ppEventNode = pEventNode;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    VmDirEventNodeFree(pEventNode);
    goto cleanup;
}

DWORD
VmDirEventNodeRelease(
    PVDIR_EVENT_NODE   pEventNode
    )
{
    DWORD   dwError = 0;
    BOOL    bInLock = FALSE;

    if (!pEventNode)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_LOCK_MUTEX(bInLock, pEventNode->pMutex);

    if (pEventNode->refCount == 0)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_WATCH_INVALID_REFCOUNT);
    }
    pEventNode->refCount--;

    if (pEventNode->refCount == 0)
    {
        dwError = VmDirEventListFreeHead(pEventNode->pEventList, TRUE);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, pEventNode->pMutex);
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirEventNodeAcquire(
    PVDIR_EVENT_NODE   pEventNode
    )
{
    BOOL    bInLock = FALSE;
    DWORD   dwError = 0;

    if (!pEventNode)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_LOCK_MUTEX(bInLock, pEventNode->pMutex);

    pEventNode->refCount++;

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, pEventNode->pMutex);
    return dwError;

error:
    goto cleanup;
}

VOID
VmDirEventNodeFree(
    PVDIR_EVENT_NODE    pEventNode
    )
{
    if (pEventNode)
    {
        VmDirEventFree(pEventNode->pEvent);
        VmDirFreeMutex(pEventNode->pMutex);
        VMDIR_SAFE_FREE_MEMORY(pEventNode);
    }
}
