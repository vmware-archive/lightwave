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
PVDIR_LINKED_LIST_NODE
_VmDirMergeSortInternal(
    PVDIR_LINKED_LIST_NODE    pFirstListHead,
    mergeSortCompareFunc      pCompareFunc
    );

static
PVDIR_LINKED_LIST_NODE
_VmDirMergeSortLinkedListMerge(
    PVDIR_LINKED_LIST_NODE    pFirstListHead,
    PVDIR_LINKED_LIST_NODE    pSecondListHead,
    mergeSortCompareFunc      pCompareFunc
    );

static
PVDIR_LINKED_LIST_NODE
_VmDirMergeSortSplit(
    PVDIR_LINKED_LIST_NODE    pFirstListHead
    );


DWORD
VmDirMergeSort(
    PVDIR_LINKED_LIST       pList,
    mergeSortCompareFunc    pCompareFunc
    )
{
    DWORD                     dwError = 0;
    PVDIR_LINKED_LIST_NODE    pTemp = NULL;

    if (!pList || !pCompareFunc)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (VmDirLinkedListGetSize(pList) > 1)
    {
        pList->pHead = _VmDirMergeSortInternal(pList->pHead, pCompareFunc);

        //Update Tail
        pTemp = pList->pHead;
        while(pTemp && pTemp->pNext)
        {
            pTemp = pTemp->pNext;
        }
        pList->pTail = pTemp;
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

static
PVDIR_LINKED_LIST_NODE
_VmDirMergeSortInternal(
    PVDIR_LINKED_LIST_NODE    pFirstListHead,
    mergeSortCompareFunc      pCompareFunc
    )
{
    if (pFirstListHead == NULL || pFirstListHead->pNext == NULL)
    {
        return pFirstListHead;
    }

    PVDIR_LINKED_LIST_NODE pSecondListHead = _VmDirMergeSortSplit(pFirstListHead);

    pFirstListHead = _VmDirMergeSortInternal(pFirstListHead, pCompareFunc);
    pSecondListHead = _VmDirMergeSortInternal(pSecondListHead, pCompareFunc);

    return _VmDirMergeSortLinkedListMerge(pFirstListHead, pSecondListHead, pCompareFunc);
}

static
PVDIR_LINKED_LIST_NODE
_VmDirMergeSortSplit(
    PVDIR_LINKED_LIST_NODE    pFirstListHead
    )
{
    PVDIR_LINKED_LIST_NODE    pFastPtr = NULL;
    PVDIR_LINKED_LIST_NODE    pSlowPtr = NULL;
    PVDIR_LINKED_LIST_NODE    pSecondListHead = NULL;

    pFastPtr = pSlowPtr = pFirstListHead;

    while (pFastPtr->pNext && pFastPtr->pNext->pNext)
    {
        pFastPtr = pFastPtr->pNext->pNext;
        pSlowPtr = pSlowPtr->pNext;
    }

    pSecondListHead = pSlowPtr->pNext;
    pSecondListHead->pPrev = NULL;

    pSlowPtr->pNext = NULL;

    return pSecondListHead;
}

static
PVDIR_LINKED_LIST_NODE
_VmDirMergeSortLinkedListMerge(
    PVDIR_LINKED_LIST_NODE    pFirstListHead,
    PVDIR_LINKED_LIST_NODE    pSecondListHead,
    mergeSortCompareFunc      pCompareFunc
    )
{
    if (pFirstListHead == NULL)
    {
        return pSecondListHead;
    }

    if (pSecondListHead == NULL)
    {
        return pFirstListHead;
    }

    if (pCompareFunc(pFirstListHead->pElement, pSecondListHead->pElement))
    {
        pFirstListHead->pNext = _VmDirMergeSortLinkedListMerge(
                pFirstListHead->pNext, pSecondListHead, pCompareFunc);
        pFirstListHead->pNext->pPrev = pFirstListHead;
        pFirstListHead->pPrev = NULL;
        return pFirstListHead;
    }
    else
    {
        pSecondListHead->pNext = _VmDirMergeSortLinkedListMerge(
                pFirstListHead, pSecondListHead->pNext, pCompareFunc);
        pSecondListHead->pNext->pPrev = pSecondListHead;
        pSecondListHead->pPrev = NULL;
        return pSecondListHead;
    }
}
