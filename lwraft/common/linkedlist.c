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

#include "includes.h"

DWORD
VmDirLinkedListCreate(
    PVDIR_LINKED_LIST*  ppLinkedList
    )
{
    DWORD   dwError = 0;
    PVDIR_LINKED_LIST   pLinkedList = NULL;

    if (!ppLinkedList)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_LINKED_LIST),
            (PVOID*)&pLinkedList);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppLinkedList = pLinkedList;

cleanup:
    return dwError;

error:
    VmDirFreeLinkedList(pLinkedList);
    goto cleanup;
}

DWORD
VmDirLinkedListGetHead(
    PVDIR_LINKED_LIST       pLinkedList,
    PVDIR_LINKED_LIST_NODE* ppHead
    )
{
    DWORD   dwError = 0;

    if (!pLinkedList || !ppHead)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppHead = pLinkedList->pHead;

error:
    return dwError;
}

DWORD
VmDirLinkedListGetTail(
    PVDIR_LINKED_LIST       pLinkedList,
    PVDIR_LINKED_LIST_NODE* ppTail
    )
{
    DWORD   dwError = 0;

    if (!pLinkedList || !ppTail)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppTail = pLinkedList->pTail;

error:
    return dwError;
}

DWORD
VmDirLinkedListInsertHead(
    PVDIR_LINKED_LIST       pLinkedList,
    PVOID                   pElement,
    PVDIR_LINKED_LIST_NODE* ppHead
    )
{
    DWORD   dwError = 0;
    PVDIR_LINKED_LIST_NODE  pHead   = NULL;

    if (!pLinkedList || !pElement)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_LINKED_LIST_NODE),
            (PVOID*)&pHead);
    BAIL_ON_VMDIR_ERROR(dwError);

    pHead->pElement = pElement;
    pHead->pList = pLinkedList;

    if (pLinkedList->pHead)
    {
        pHead->pPrev = pLinkedList->pHead;
        pLinkedList->pHead->pNext = pHead;
    }
    else
    {
        pLinkedList->pTail = pHead;
    }

    pLinkedList->pHead = pHead;
    if (ppHead)
    {
        *ppHead = pHead;
    }

    pLinkedList->iSize++;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pHead);
    goto cleanup;
}

DWORD
VmDirLinkedListInsertTail(
    PVDIR_LINKED_LIST       pLinkedList,
    PVOID                   pElement,
    PVDIR_LINKED_LIST_NODE* ppTail
    )
{
    DWORD   dwError = 0;
    PVDIR_LINKED_LIST_NODE  pTail   = NULL;

    if (!pLinkedList || !pElement)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_LINKED_LIST_NODE),
            (PVOID*)&pTail);
    BAIL_ON_VMDIR_ERROR(dwError);

    pTail->pElement = pElement;
    pTail->pList = pLinkedList;

    if (pLinkedList->pTail)
    {
        pTail->pNext = pLinkedList->pTail;
        pLinkedList->pTail->pPrev = pTail;
    }
    else
    {
        pLinkedList->pHead = pTail;
    }

    pLinkedList->pTail = pTail;
    if (ppTail)
    {
        *ppTail = pTail;
    }

    pLinkedList->iSize++;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pTail);
    goto cleanup;
}

DWORD
VmDirLinkedListRemove(
    PVDIR_LINKED_LIST       pLinkedList,
    PVDIR_LINKED_LIST_NODE  pNode
    )
{
    DWORD   dwError = 0;

    if (!pLinkedList || !pNode)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pNode->pNext)
    {
        pNode->pNext->pPrev = pNode->pPrev;
    }
    if (pNode->pPrev)
    {
        pNode->pPrev->pNext = pNode->pNext;
    }
    if (pNode == pLinkedList->pHead)
    {
        pLinkedList->pHead = pNode->pPrev;
    }
    if (pNode == pLinkedList->pTail)
    {
        pLinkedList->pTail = pNode->pNext;
    }
    VMDIR_SAFE_FREE_MEMORY(pNode);

    pLinkedList->iSize--;

error:
    return dwError;
}

size_t
VmDirLinkedListGetSize(
    PVDIR_LINKED_LIST   pLinkedList
    )
{
    size_t  iRtn = 0;

    if (pLinkedList)
    {
        iRtn = pLinkedList->iSize;
    }
    return iRtn;
}

BOOLEAN
VmDirLinkedListIsEmpty(
    PVDIR_LINKED_LIST   pLinkedList
    )
{
    return pLinkedList == NULL || pLinkedList->pHead == NULL;
}

VOID
VmDirFreeLinkedList(
    PVDIR_LINKED_LIST   pLinkedList
    )
{
    if (pLinkedList)
    {
        while (!VmDirLinkedListIsEmpty(pLinkedList))
        {
            VmDirLinkedListRemove(pLinkedList, pLinkedList->pHead);
        }
        VMDIR_SAFE_FREE_MEMORY(pLinkedList);
    }
}
