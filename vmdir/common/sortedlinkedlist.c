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
VmDirSortedLinkedListCreate(
    PFN_SORTED_LINKEDLIST_INSERT_COMPARE    pCompareFunc,
    PVDIR_SORTED_LINKED_LIST*               ppSortedLinkedList
    )
{
    DWORD                       dwError = 0;
    PVDIR_SORTED_LINKED_LIST    pSortedLinkedList = NULL;

    if (!pCompareFunc || !ppSortedLinkedList)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(sizeof(VDIR_SORTED_LINKED_LIST), (PVOID*)&pSortedLinkedList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pSortedLinkedList->pList);
    BAIL_ON_VMDIR_ERROR(dwError);

    pSortedLinkedList->pCompareFunc = pCompareFunc;

    *ppSortedLinkedList = pSortedLinkedList;

cleanup:
    return dwError;

error:
    VmDirFreeSortedLinkedList(pSortedLinkedList);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

VOID
VmDirFreeSortedLinkedList(
    PVDIR_SORTED_LINKED_LIST    pSortedList
    )
{
    if (pSortedList)
    {
        VmDirFreeLinkedList(pSortedList->pList);
        VMDIR_SAFE_FREE_MEMORY(pSortedList);
    }
}

DWORD
VmDirSortedLinkedListInsert(
    PVDIR_SORTED_LINKED_LIST    pSortedList,
    PVOID                       pElement
    )
{
    DWORD                     dwError = 0;
    PVDIR_LINKED_LIST         pList = NULL;
    PVDIR_LINKED_LIST_NODE    pCurrNode = NULL;
    PVDIR_LINKED_LIST_NODE    pNewNode = NULL;

    if (!pSortedList || !pElement)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pList = pSortedList->pList;

    if (pList->pHead == NULL || pSortedList->pCompareFunc(pElement, pList->pHead->pElement))
    {
        dwError = VmDirLinkedListInsertHead(pList, pElement, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        pCurrNode = pList->pHead;

        while (pCurrNode->pNext && pSortedList->pCompareFunc(pCurrNode->pNext->pElement, pElement))
        {
            pCurrNode = pCurrNode->pNext;
        }

        if (pCurrNode->pNext == NULL)
        {
            dwError = VmDirLinkedListInsertTail(pList, pElement, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            dwError = VmDirAllocateMemory(sizeof(VDIR_LINKED_LIST_NODE), (PVOID*)&pNewNode);
            BAIL_ON_VMDIR_ERROR(dwError);

            pNewNode->pElement = pElement;

            //Manipulate pNext and pPrev
            pNewNode->pNext = pCurrNode->pNext;
            pNewNode->pPrev = pCurrNode;

            pNewNode->pNext->pPrev = pNewNode;
            pCurrNode->pNext = pNewNode;
        }
    }

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pNewNode);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}
