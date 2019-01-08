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
VmDirEventRepoInit(
    PVDIR_EVENT_REPO*   ppEventRepo
    )
{
    DWORD               dwError = 0;
    PVDIR_EVENT         pEvent = NULL;
    PVDIR_EVENT_REPO    pEventRepo = NULL;

    if (!ppEventRepo)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(sizeof(VDIR_EVENT_REPO), (PVOID *)&pEventRepo);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirQueueInit(&pEventRepo->pPendingQueue);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pEventRepo->pReadyEventList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(sizeof(VDIR_EVENT), (PVOID *)&pEvent);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListInsertTail(pEventRepo->pReadyEventList, pEvent, &pEvent->pListNode);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppEventRepo = pEventRepo;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    VmDirEventRepoFree(pEventRepo);
    goto cleanup;
}

DWORD
VmDirEventRepoGetTail(
     PVDIR_EVENT_REPO   pEventRepo,
     PVDIR_EVENT*       ppTail
     )
{
    DWORD                   dwError = 0;
    PVDIR_LINKED_LIST_NODE  pListTail = NULL;

    if (!pEventRepo || !ppTail)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirLinkedListGetTail(pEventRepo->pReadyEventList, &pListTail);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirEventAcquire(pListTail->pElement);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppTail = (PVDIR_EVENT)pListTail->pElement;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirEventRepoGetNextReadyEvent(
    PVDIR_EVENT_REPO            pEventRepo,
    PVDIR_EVENT_REPO_COOKIE*    ppEventRepoCookie,
    PVDIR_EVENT*                ppNextEvent
    )
{
    DWORD                   dwError = 0;
    PVDIR_EVENT             pNextEvent = NULL;
    PVDIR_EVENT             pCurEvent = NULL;
    PVDIR_LINKED_LIST_NODE  pTail = NULL;

    if (!pEventRepo || !ppNextEvent || !ppEventRepoCookie || !*ppEventRepoCookie)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirLinkedListGetTail(pEventRepo->pReadyEventList, &pTail);
    BAIL_ON_VMDIR_ERROR(dwError);

    pCurEvent = *ppEventRepoCookie;

    if (pCurEvent->pListNode != pTail)
    {
        pNextEvent = (PVDIR_EVENT)pCurEvent->pListNode->pNext->pElement;
        dwError = VmDirEventRelease(pCurEvent);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_WATCH_ENDOFLIST);
    }

    dwError = VmDirEventAcquire(pNextEvent);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppNextEvent = pNextEvent;
    *ppEventRepoCookie = pNextEvent;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirEventRepoAddPendingEvent(
    PVDIR_EVENT_REPO    pEventRepo,
    PVDIR_EVENT         pEvent,
    BOOLEAN             bInLock
    )
{
    DWORD   dwError = 0;

    if (!pEventRepo || !pEvent)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirQueueEnqueue(bInLock, pEventRepo->pPendingQueue, pEvent);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirEventRepoAddPendingEventExternal(
    PVDIR_EVENT pEvent,
    BOOL        bInLock
    )
{
    DWORD   dwError = 0;

    if (!pEvent)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirEventRepoAddPendingEvent(gpEventRepo, pEvent, bInLock);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirEventRepoSync(
    PVDIR_EVENT_REPO    pEventRepo,
    int64_t             iTimeoutMs
    )
{
    DWORD                   dwError = 0;
    PVDIR_EVENT             pEvent = NULL;
    PVDIR_EVENT             pTail = NULL;
    BOOLEAN                 bInLock = FALSE;
    PVMDIR_MUTEX            pMutex = NULL;
    PVDIR_LINKED_LIST_NODE  pListTail = NULL;

    if (!pEventRepo)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirQueueDequeue(bInLock, pEventRepo->pPendingQueue, iTimeoutMs, (PVOID*)&pEvent);
    if (dwError == VMDIR_ERROR_QUEUE_EMPTY)
    {
        dwError = 0; // nothing to sync return success
        goto cleanup;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    pMutex = pEvent->pMutex;

    VMDIR_LOCK_MUTEX(bInLock, pMutex);

    while (!pEvent->bIsEventReady)
    {
        dwError = VmDirConditionWait(pEvent->pCond, pEvent->pMutex);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_UNLOCK_MUTEX(bInLock, pMutex);

    if (pEvent->bIsSuccessful)
    {
        dwError = VmDirLinkedListGetTail(pEventRepo->pReadyEventList, &pListTail);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pListTail)
        {
            pTail = (PVDIR_EVENT)pListTail->pElement;
            pMutex = pTail->pMutex;

            VMDIR_LOCK_MUTEX(bInLock, pMutex);
        }

        dwError = VmDirLinkedListInsertTail(
                pEventRepo->pReadyEventList, (PVOID)pEvent, &pEvent->pListNode);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        VmDirEventFree(pEvent);
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, pMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

BOOL
VmDirEventRepoIsAtHead(
    PVDIR_EVENT_REPO    pEventRepo,
    PVDIR_EVENT         pCurEvent
    )
{
    BOOL                    bIsHead = FALSE;
    PVDIR_LINKED_LIST_NODE  pHead = NULL;

    if (pEventRepo && pCurEvent)
    {
        VmDirLinkedListGetHead(pEventRepo->pReadyEventList, &pHead);

        if (pHead && pCurEvent->pListNode == pHead->pNext)
        {
            bIsHead = TRUE;
        }
    }

    return bIsHead;
}

VOID
VmDirEventRepoFree(
    PVDIR_EVENT_REPO    pEventRepo
    )
{
    DWORD                   dwError = 0;
    BOOLEAN                 bInLock = FALSE;
    PVDIR_EVENT             pEvent = NULL;
    PVDIR_LINKED_LIST_NODE  pListHead = NULL;

    if (pEventRepo)
    {
        while (!dwError)
        {
            dwError = VmDirQueueDequeue(bInLock, pEventRepo->pPendingQueue, 0, (PVOID*)&pEvent);
            VmDirEventFree(pEvent);
        }
        VmDirQueueFree(pEventRepo->pPendingQueue);

        VmDirLinkedListGetHead(pEventRepo->pReadyEventList, &pListHead);
        while(pListHead)
        {
            VmDirEventFree((PVDIR_EVENT)pListHead->pElement);
            VmDirLinkedListRemove(pEventRepo->pReadyEventList, pListHead);
            VmDirLinkedListGetHead(pEventRepo->pReadyEventList, &pListHead);
        }
        VmDirFreeLinkedList(pEventRepo->pReadyEventList);

        VMDIR_SAFE_FREE_MEMORY(pEventRepo);
    }
}
