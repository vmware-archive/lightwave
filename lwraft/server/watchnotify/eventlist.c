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
VmDirEventListInit(
    PVDIR_EVENT_LIST*   ppEventList
    )
{
    DWORD               dwError = 0;
    PVDIR_EVENT_LIST    pEventList = NULL;

    if (!ppEventList)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(sizeof(VDIR_EVENT_LIST), (PVOID *)&pEventList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&pEventList->pMutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppEventList = pEventList;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    VmDirEventListFree(pEventList);
    goto cleanup;
}

DWORD
VmDirEventListGetNext(
    PVDIR_EVENT_LIST    pEventList,
    PVDIR_EVENT_NODE    pCurEvent,
    PVDIR_EVENT_NODE*   ppNextEvent
    )
{
    DWORD               dwError = 0;
    PVDIR_EVENT_NODE    pNextEvent = NULL;

    if (!pEventList || !ppNextEvent)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (pCurEvent == pEventList->pTail || !pEventList->pTail)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_WATCH_ENDOFLIST);
    }

    if (pCurEvent)
    {
        pNextEvent = pCurEvent->pNext;
        if (!pNextEvent->pEvent->bIsEventReady)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_WATCH_ENDOFLIST);
        }
        else
        {
            dwError = VmDirEventNodeRelease(pCurEvent);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else
    {
        pNextEvent = pEventList->pTail;
    }

    dwError = VmDirEventNodeAcquire(pNextEvent);
    BAIL_ON_VMDIR_ERROR(dwError);
    *ppNextEvent = pNextEvent;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirEventListAddNode(
    PVDIR_EVENT_LIST    pEventList,
    PVDIR_EVENT_NODE    pEventNode,
    BOOL                bInLock
    )
{
    DWORD               dwError = 0;

    if (!pEventList || !pEventNode)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pEventNode->pEventList = pEventList;

    VMDIR_LOCK_MUTEX(bInLock, pEventList->pMutex);

    if (pEventList->pTail)
    {
        pEventList->pTail->pNext = pEventNode;
        pEventList->pTail = pEventNode;
    }
    else
    {
        pEventList->pHead = pEventNode;
        pEventList->pTail = pEventNode;
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, pEventList->pMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirEventListFreeHead(
    PVDIR_EVENT_LIST    pEventList,
    BOOL                bInLock
    )
{
    DWORD               dwError = 0;
    PVDIR_EVENT_NODE    pTemp = NULL;

    if (!pEventList)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_LOCK_MUTEX(bInLock, pEventList->pMutex);

    if (pEventList->pHead)
    {
        pTemp = pEventList->pHead;
        pEventList->pHead = pTemp->pNext;
        VmDirEventNodeFree(pTemp);
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, pEventList->pMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

VOID
VmDirEventListFree(
    PVDIR_EVENT_LIST    pEventList
    )
{
    PVDIR_EVENT_NODE    pTemp = NULL;

    if (pEventList)
    {
        while (pEventList->pHead)
        {
            pTemp = pEventList->pHead->pNext;
            VmDirEventNodeFree(pEventList->pHead);
            pEventList->pHead = pTemp;
        }
        VmDirFreeMutex(pEventList->pMutex);
        VMDIR_SAFE_FREE_MEMORY(pEventList);
    }
}
