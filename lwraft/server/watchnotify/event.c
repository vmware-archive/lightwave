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
VmDirEventAddEventData(
    PVDIR_EVENT         pEvent,
    PVDIR_EVENT_DATA    pEventData
    )
{
    BOOL                    bInLock = FALSE;
    DWORD                   dwError = 0;
    PVMDIR_MUTEX            pMutex = NULL;
    PVDIR_EVENT_DATA_NODE   pNewNode = NULL;

    if (!pEvent || !pEventData)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pMutex = pEvent->pMutex;

    dwError = VmDirAllocateMemory(sizeof(VDIR_EVENT_DATA_NODE), (PVOID*)&pNewNode);
    BAIL_ON_VMDIR_ERROR(dwError);

    pNewNode->pEventData = pEventData;

    VMDIR_LOCK_MUTEX(bInLock, pMutex);

    if (!pEvent->pEventDataHead)
    {
        pEvent->pEventDataHead = pNewNode;
        pEvent->pEventDataTail = pNewNode;
    }
    else
    {
        pEvent->pEventDataTail->pNext = pNewNode;
        pEvent->pEventDataTail = pNewNode;
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, pMutex);
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pNewNode);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirEventDecodeEventData(
    PVDIR_EVENT pEvent
    )
{
    DWORD                   dwError = 0;
    PVDIR_ENTRY             pPrevEntry = NULL;
    PVDIR_EVENT_DATA_NODE   pTemp = NULL;
    PVDIR_SCHEMA_CTX        pSchemaCtx = NULL;

    if (!pEvent)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    pTemp = pEvent->pEventDataHead;

    while (pTemp)
    {
        pPrevEntry = pTemp->pEventData->pPrevEntry;
        if (pPrevEntry)
        {
            dwError = VmDirDecodeEntry(pSchemaCtx, pPrevEntry);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        pTemp = pTemp->pNext;
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirEventRelease(
    PVDIR_EVENT pEvent
    )
{
    DWORD                   dwError = 0;
    BOOL                    bInLock = FALSE;
    PVMDIR_MUTEX            pMutex = NULL;
    PVDIR_LINKED_LIST_NODE  pHead = NULL;

    if (!pEvent)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirLinkedListGetHead(pEvent->pListNode->pList, &pHead);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pEvent->pListNode == pHead) // at fake head do nothing
    {
        goto cleanup;
    }

    pMutex = pEvent->pMutex;

    VMDIR_LOCK_MUTEX(bInLock, pMutex);

    if (pEvent->refCount == 0)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_WATCH_INVALID_REFCOUNT);
    }
    pEvent->refCount--;

    if (pEvent->refCount == 0 && pEvent->pListNode == pHead->pNext)
    {
        dwError = VmDirLinkedListRemove(pEvent->pListNode->pList, pHead->pNext);
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
    PVDIR_EVENT_DATA_NODE   pTemp = NULL;

    if (pEvent)
    {
        VmDirFreeMutex(pEvent->pMutex);
        VmDirFreeCondition(pEvent->pCond);

        pTemp = pEvent->pEventDataHead;
        while (pTemp)
        {
            pEvent->pEventDataHead = pTemp->pNext;

            VmDirFreeEntry(pTemp->pEventData->pCurEntry);
            VmDirFreeEntry(pTemp->pEventData->pPrevEntry);
            VMDIR_SAFE_FREE_MEMORY(pTemp->pEventData);

            VMDIR_SAFE_FREE_MEMORY(pTemp);
            pTemp = pEvent->pEventDataHead;
        }

        VMDIR_SAFE_FREE_MEMORY(pEvent);
    }
}
