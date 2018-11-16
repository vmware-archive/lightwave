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
 * under the License.
 */

#include "includes.h"

DWORD
VmDirQueueInit(
    PVDIR_QUEUE*    ppQueue
    )
{
    DWORD       dwError = 0;
    PVDIR_QUEUE pQueue = NULL;

    if (!ppQueue)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(sizeof(VDIR_QUEUE), (PVOID*)&pQueue);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&pQueue->pMutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateCondition(&pQueue->pCond);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppQueue = pQueue;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    VmDirQueueFree(pQueue);
    goto cleanup;
}

DWORD
VmDirQueueEnqueue(
    BOOL                bInLock,
    PVDIR_QUEUE         pQueue,
    PVOID               pElement
    )
{
    DWORD dwError = 0;
    PVDIR_QUEUE_NODE pQueueNode = NULL;

    if (!pQueue || !pElement)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(sizeof(VDIR_QUEUE_NODE), (PVOID*)&pQueueNode);
    BAIL_ON_VMDIR_ERROR(dwError);

    pQueueNode->pElement = pElement;

    VMDIR_LOCK_MUTEX(bInLock, pQueue->pMutex);

    if (pQueue->pHead)
    {
        pQueue->pTail->pNext = pQueueNode;
        pQueue->pTail = pQueueNode;
    }
    else
    {
        pQueue->pHead = pQueueNode;
        pQueue->pTail = pQueueNode;
        VmDirConditionSignal(pQueue->pCond);
    }
    pQueue->iSize++;

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, pQueue->pMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    VMDIR_SAFE_FREE_MEMORY(pQueueNode);
    goto cleanup;
}

DWORD
VmDirQueueDequeue(
    BOOL        bInLock,
    PVDIR_QUEUE pQueue,
    int64_t     iTimeoutMs,
    PVOID*      ppElement
    )
{
    DWORD dwError = 0;
    PVDIR_QUEUE_NODE pTemp= NULL;

    if (!pQueue || !ppElement)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_PARAMETER);
    }

    VMDIR_LOCK_MUTEX(bInLock, pQueue->pMutex);

    if (!pQueue->pHead)
    {
        if (iTimeoutMs < 0) // Blocking
        {
            while (!pQueue->pHead)
            {
                dwError = VmDirConditionWait(pQueue->pCond, pQueue->pMutex);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
        else if (iTimeoutMs > 0) // Waiting
        {
            VmDirConditionTimedWait(pQueue->pCond, pQueue->pMutex, iTimeoutMs);
            if (!pQueue->pHead)
            {
                dwError = VMDIR_ERROR_QUEUE_EMPTY;
            }
        }
        else // Non Blocking
        {
            dwError = VMDIR_ERROR_QUEUE_EMPTY;
        }
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pQueue->iSize--;
    pTemp = pQueue->pHead;
    pQueue->pHead = pQueue->pHead->pNext;
    *ppElement = pTemp->pElement;
    VMDIR_SAFE_FREE_MEMORY(pTemp);

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, pQueue->pMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

BOOL
VmDirQueueCompare(
    PVDIR_QUEUE pLeftQueue,
    PVDIR_QUEUE pRightQueue
    )
{
    BOOL returnValue = FALSE;
    PVDIR_QUEUE_NODE pHeadLeft = NULL;
    PVDIR_QUEUE_NODE pHeadRight = NULL;

    if (!pLeftQueue || !pRightQueue)
    {
        return FALSE;
    }

    pHeadLeft = pLeftQueue->pHead;
    pHeadRight = pRightQueue->pHead;

    if (pLeftQueue->iSize != pRightQueue->iSize)
    {
        returnValue = FALSE;
    }
    else if(!pHeadLeft && !pHeadRight)
    {
        returnValue = TRUE;
    }
    else
    {
        returnValue = TRUE;
        while (pHeadLeft && pHeadRight)
        {
            if (pHeadLeft->pElement != pHeadRight->pElement)
            {
                returnValue = FALSE;
                break;
            }
            pHeadLeft = pHeadLeft->pNext;
            pHeadRight = pHeadRight->pNext;
        }

        if (returnValue && (pHeadLeft || pHeadRight))
        {
            returnValue = FALSE;
        }
    }

    return returnValue;
}

VOID
VmDirQueueFree(
    PVDIR_QUEUE pQueue
    )
{
    PVDIR_QUEUE_NODE pTemp = NULL;
    BOOLEAN bInLock = FALSE;

    if (pQueue)
    {
        VMDIR_LOCK_MUTEX(bInLock, pQueue->pMutex);
        while(pQueue->pHead)
        {
            pTemp = pQueue->pHead->pNext;
            VMDIR_SAFE_FREE_MEMORY(pQueue->pHead);
            pQueue->pHead = pTemp;
        }
        VMDIR_UNLOCK_MUTEX(bInLock, pQueue->pMutex);

        VmDirFreeMutex(pQueue->pMutex);
        VmDirFreeCondition(pQueue->pCond);
        VMDIR_SAFE_FREE_MEMORY(pQueue);
    }
}
