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

static
VOID
_VmDirPrintWriteQueueContentsInLock(
    PVMDIR_WRITE_QUEUE    pWriteQueue
    );

DWORD
VmDirWriteQueuePush(
    PVDIR_BACKEND_CTX           pBECtx,
    PVMDIR_WRITE_QUEUE          pWriteQueue,
    PVMDIR_WRITE_QUEUE_ELEMENT  pWriteQueueEle
    )
{
    int       dbRetVal = 0;
    USN       localUsn = 0;
    DWORD     dwError = 0;
    BOOLEAN   bInLock = FALSE;

    if (!pBECtx || !pWriteQueue || !pWriteQueueEle)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (pBECtx->wTxnUSN != 0)
    {
        VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s: acquiring multiple usn in same operation context, USN: %" PRId64,
            __FUNCTION__,
            pBECtx->wTxnUSN);
        BAIL_WITH_VMDIR_ERROR(dwError, LDAP_OPERATIONS_ERROR);
    }

    VMDIR_LOCK_MUTEX(bInLock, gVmDirServerOpsGlobals.pMutex);

    if ((dbRetVal = pBECtx->pBE->pfnBEGetNextUSN(pBECtx, &localUsn)) != 0)
    {
        VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s: pfnBEGetNextUSN failed with error code: %d",
            __FUNCTION__,
            dbRetVal);
        BAIL_WITH_VMDIR_ERROR(dwError, LDAP_OPERATIONS_ERROR);
    }

    pWriteQueueEle->usn = localUsn;

    dwError = VmDirLinkedListInsertTail(
            pWriteQueue->pList,
            (PVOID) pWriteQueueEle,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO(LDAP_DEBUG_WRITE_QUEUE, "%s: usn: %"PRId64, __FUNCTION__, localUsn);

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmDirServerOpsGlobals.pMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d) localUsn %"PRId64, dwError, localUsn);
    goto cleanup;
}

DWORD
VmDirWriteQueueWait(
    PVMDIR_WRITE_QUEUE          pWriteQueue,
    PVMDIR_WRITE_QUEUE_ELEMENT  pWriteQueueEle
    )
{
    DWORD      dwError = 0;
    uint64_t   iStartTime = 0;
    BOOLEAN    bInLock = FALSE;
    PVDIR_LINKED_LIST_NODE      pNode = NULL;
    PVMDIR_WRITE_QUEUE_ELEMENT  pWriteQueueHead = NULL;

    if (!pWriteQueue || !pWriteQueueEle)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_LOCK_MUTEX(bInLock, gVmDirServerOpsGlobals.pMutex);

    iStartTime = VmDirGetTimeInMilliSec();

    //default - timeout 60 seconds
    while (VmDirGetTimeInMilliSec() - iStartTime <= gVmdirGlobals.dwWriteTimeoutInMilliSec)
    {
        VmDirLinkedListGetHead(pWriteQueue->pList, &pNode);

        pWriteQueueHead = ((PVMDIR_WRITE_QUEUE_ELEMENT)pNode->pElement);

        if (pWriteQueueHead->usn == pWriteQueueEle->usn)
        {
            VMDIR_LOG_INFO(LDAP_DEBUG_WRITE_QUEUE, "%s: usn: %"PRId64, __FUNCTION__, pWriteQueueEle->usn);
            break;
        }

        dwError = VmDirConditionTimedWait(
                pWriteQueueEle->pCond,
                gVmDirServerOpsGlobals.pMutex,
                gVmdirGlobals.dwWriteTimeoutInMilliSec);
    }

    if (dwError != 0) //ETIMEDOUT
    {
        // After timeout queueHead could have changed
        VmDirLinkedListGetHead(pWriteQueue->pList, &pNode);

        if (pNode)
        {
            pWriteQueueHead = ((PVMDIR_WRITE_QUEUE_ELEMENT)pNode->pElement);

            VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "%s: error: %d QueueHeadUSN: %"PRId64 "MyUSN: %"PRId64,
                __FUNCTION__,
                dwError,
                pWriteQueueHead->usn,
                pWriteQueueEle->usn);
        }

        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_ML_WRITE_TIMEOUT);
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmDirServerOpsGlobals.pMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

VOID
VmDirWriteQueuePop(
    PVMDIR_WRITE_QUEUE          pWriteQueue,
    PVMDIR_WRITE_QUEUE_ELEMENT  pWriteQueueEle
    )
{
    USN       committedUSN = 0;
    DWORD     dwError = 0;
    BOOLEAN   bInLock = FALSE;
    PVDIR_LINKED_LIST_NODE      pNode = NULL;
    PVMDIR_WRITE_QUEUE_ELEMENT  pWriteQueueHead = NULL;

    if (!pWriteQueue || !pWriteQueueEle)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_LOCK_MUTEX(bInLock, gVmDirServerOpsGlobals.pMutex);

    _VmDirPrintWriteQueueContentsInLock(pWriteQueue);

    VmDirLinkedListGetHead(pWriteQueue->pList, &pNode);

    if (pNode)
    {
        pWriteQueueHead = (PVMDIR_WRITE_QUEUE_ELEMENT) pNode->pElement;

        if (pWriteQueueHead->usn == pWriteQueueEle->usn)
        {
            committedUSN = pWriteQueueEle->usn;
            VmDirUpdateMaxCommittedUSNInLock(committedUSN);
            VMDIR_LOG_INFO(LDAP_DEBUG_WRITE_QUEUE, "%s: usn: %"PRId64, __FUNCTION__, pWriteQueueEle->usn);
        }
        else
        {
            VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "%s: writeQueueHeadUSN %" PRId64 "MyUSN: %" PRId64,
                __FUNCTION__,
                pWriteQueueHead->usn,
                pWriteQueueEle->usn);
        }

        while (pNode &&
              ((PVMDIR_WRITE_QUEUE_ELEMENT)pNode->pElement)->usn != pWriteQueueEle->usn)
        {
            pNode = pNode->pNext;
        }

        VmDirLinkedListRemove(pWriteQueue->pList, pNode);
    }

    if (committedUSN)
    {
        VmDirLinkedListGetHead(pWriteQueue->pList, &pNode);

        if (pNode)
        {
            pWriteQueueHead = (PVMDIR_WRITE_QUEUE_ELEMENT) pNode->pElement;

            if (pWriteQueueHead->usn <= committedUSN)
            {
                VMDIR_LOG_ERROR(
                        VMDIR_LOG_MASK_ALL,
                        "%s: out of order usn in the queue committedUSN:%" PRId64 "nextUSN:%" PRId64,
                        __FUNCTION__,
                        committedUSN,
                        pWriteQueueHead->usn);
            }

            VmDirConditionSignal(pWriteQueueHead->pCond);

            VMDIR_LOG_INFO(
                LDAP_DEBUG_WRITE_QUEUE,
                "%s: next USN Signaled %" PRId64,
                __FUNCTION__,
                pWriteQueueHead->usn);
        }
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmDirServerOpsGlobals.pMutex);
    return;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirWriteQueueElementAllocate(
    PVMDIR_WRITE_QUEUE_ELEMENT*    ppWriteQueueEle
    )
{
    DWORD    dwError = 0;
    PVMDIR_WRITE_QUEUE_ELEMENT    pWriteQueueEleLocal = NULL;

    if (!ppWriteQueueEle)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(sizeof(VMDIR_WRITE_QUEUE_ELEMENT), (PVOID)&pWriteQueueEleLocal);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateCondition(&pWriteQueueEleLocal->pCond);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppWriteQueueEle = pWriteQueueEleLocal;
    pWriteQueueEleLocal = NULL;

cleanup:
    return dwError;

error:
    VmDirWriteQueueElementFree(pWriteQueueEleLocal);
    pWriteQueueEleLocal = NULL;

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

VOID
VmDirWriteQueueElementFree(
    PVMDIR_WRITE_QUEUE_ELEMENT   pWriteQueueEle
    )
{
    if (pWriteQueueEle)
    {
        VMDIR_SAFE_FREE_CONDITION(pWriteQueueEle->pCond);
        VMDIR_SAFE_FREE_MEMORY(pWriteQueueEle);
    }
}

static
VOID
_VmDirPrintWriteQueueContentsInLock(
    PVMDIR_WRITE_QUEUE    pWriteQueue
    )
{
    PVDIR_LINKED_LIST_NODE    pNode = NULL;

    if (VmDirLogGetMask() & LDAP_DEBUG_WRITE_QUEUE)
    {
        VmDirLinkedListGetHead(pWriteQueue->pList, &pNode);
        while (pNode)
        {
            VMDIR_LOG_INFO(
                    LDAP_DEBUG_WRITE_QUEUE,
                    "%s: USNs in the write queue: %"PRId64,
                    __FUNCTION__,
                    ((PVMDIR_WRITE_QUEUE_ELEMENT)pNode->pElement)->usn);
            pNode = pNode->pNext;
        }
    }
}
