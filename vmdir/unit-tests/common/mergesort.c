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
int
_VmDirMergeSortCompareIntDescending(
    const PVOID    pInt1,
    const PVOID    pInt2
    );

static
int
_VmDirMergeSortCompareIntAscending(
    const PVOID    pInt1,
    const PVOID    pInt2
    );

static
BOOLEAN
_VmDirMergeSortLinkedListIsSorted(
    PVDIR_LINKED_LIST    pList,
    BOOLEAN              bDescending
    );

VOID
VmDirMergeSort_OneNodeInput(
    VOID    **state
    )
{
    PVDIR_LINKED_LIST    pList = NULL;
    DWORD                dwError = 0;

    // One Node
    dwError = VmDirLinkedListCreate(&pList);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pList, (PVOID)10, NULL);
    assert_int_equal(dwError, 0);

    dwError = VmDirMergeSort(pList, _VmDirMergeSortCompareIntDescending);
    assert_int_equal(dwError, 0);
    assert_non_null(pList);
    assert_true(_VmDirMergeSortLinkedListIsSorted(pList, TRUE));

    VmDirFreeLinkedList(pList);
    pList = NULL;
}

VOID
VmDirMergeSort_TwoNodesInput(
    VOID    **state
    )
{
    PVDIR_LINKED_LIST    pList = NULL;
    DWORD                dwError = 0;

    //Two Node
    dwError = VmDirLinkedListCreate(&pList);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pList, (PVOID)10, NULL);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pList, (PVOID)12, NULL);
    assert_int_equal(dwError, 0);

    dwError = VmDirMergeSort(pList, _VmDirMergeSortCompareIntDescending);
    assert_int_equal(dwError, 0);
    assert_non_null(pList);
    assert_true(_VmDirMergeSortLinkedListIsSorted(pList, TRUE));

    VmDirFreeLinkedList(pList);
    pList = NULL;

    //Two Node
    dwError = VmDirLinkedListCreate(&pList);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pList, (PVOID)10, NULL);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pList, (PVOID)10, NULL);
    assert_int_equal(dwError, 0);

    dwError = VmDirMergeSort(pList, _VmDirMergeSortCompareIntDescending);
    assert_int_equal(dwError, 0);
    assert_non_null(pList);
    assert_true(_VmDirMergeSortLinkedListIsSorted(pList, TRUE));

    VmDirFreeLinkedList(pList);
    pList = NULL;

    //Two Node
    dwError = VmDirLinkedListCreate(&pList);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pList, (PVOID)10, NULL);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pList, (PVOID)8, NULL);
    assert_int_equal(dwError, 0);

    dwError = VmDirMergeSort(pList, _VmDirMergeSortCompareIntDescending);
    assert_int_equal(dwError, 0);
    assert_non_null(pList);
    assert_true(_VmDirMergeSortLinkedListIsSorted(pList, TRUE));

    VmDirFreeLinkedList(pList);
    pList = NULL;

}

VOID
VmDirMergeSort_ThreeNodesInput(
    VOID    **state
    )
{
    PVDIR_LINKED_LIST    pList = NULL;
    DWORD                dwError = 0;

    //Three Node
    dwError = VmDirLinkedListCreate(&pList);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pList, (PVOID)10, NULL);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pList, (PVOID)12, NULL);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pList, (PVOID)14, NULL);
    assert_int_equal(dwError, 0);

    dwError = VmDirMergeSort(pList, _VmDirMergeSortCompareIntDescending);
    assert_int_equal(dwError, 0);
    assert_non_null(pList);
    assert_true(_VmDirMergeSortLinkedListIsSorted(pList, TRUE));

    VmDirFreeLinkedList(pList);
    pList = NULL;

    //Three Node
    dwError = VmDirLinkedListCreate(&pList);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pList, (PVOID)10, NULL);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pList, (PVOID)10, NULL);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pList, (PVOID)10, NULL);
    assert_int_equal(dwError, 0);

    dwError = VmDirMergeSort(pList, _VmDirMergeSortCompareIntDescending);
    assert_int_equal(dwError, 0);
    assert_non_null(pList);
    assert_true(_VmDirMergeSortLinkedListIsSorted(pList, TRUE));

    VmDirFreeLinkedList(pList);
    pList = NULL;

    //Three Node
    dwError = VmDirLinkedListCreate(&pList);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pList, (PVOID)10, NULL);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pList, (PVOID)8, NULL);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pList, (PVOID)6, NULL);
    assert_int_equal(dwError, 0);

    dwError = VmDirMergeSort(pList, _VmDirMergeSortCompareIntDescending);
    assert_int_equal(dwError, 0);
    assert_non_null(pList);
    assert_true(_VmDirMergeSortLinkedListIsSorted(pList, TRUE));

    VmDirFreeLinkedList(pList);
    pList = NULL;

    //Three Node
    dwError = VmDirLinkedListCreate(&pList);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pList, (PVOID)12, NULL);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pList, (PVOID)14, NULL);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pList, (PVOID)16, NULL);
    assert_int_equal(dwError, 0);

    dwError = VmDirMergeSort(pList, _VmDirMergeSortCompareIntDescending);
    assert_int_equal(dwError, 0);
    assert_non_null(pList);
    assert_true(_VmDirMergeSortLinkedListIsSorted(pList, TRUE));

    VmDirFreeLinkedList(pList);
    pList = NULL;
}

VOID
VmDirMergeSort_DescendingOrder(
    VOID    **state
    )
{
    PVDIR_LINKED_LIST    pList = NULL;
    DWORD                dwError = 0;
    DWORD                dwCount = 0;

    dwError = VmDirLinkedListCreate(&pList);
    assert_int_equal(dwError, 0);

    while (dwCount < 2000)
    {
        dwError = VmDirLinkedListInsertHead(pList, (PVOID)((uint64_t)rand()%20000), NULL);
        assert_int_equal(dwError, 0);
        dwCount++;
    }

    dwError = VmDirMergeSort(pList, _VmDirMergeSortCompareIntDescending);
    assert_int_equal(dwError, 0);
    assert_non_null(pList);
    assert_true(_VmDirMergeSortLinkedListIsSorted(pList, TRUE));

    VmDirFreeLinkedList(pList);
    pList = NULL;
}

VOID
VmDirMergeSort_AscendingOrder(
    VOID    **state
    )
{
    PVDIR_LINKED_LIST    pList = NULL;
    DWORD                dwError = 0;
    DWORD                dwCount = 0;

    dwError = VmDirLinkedListCreate(&pList);
    assert_int_equal(dwError, 0);

    while (dwCount < 2000)
    {
        dwError = VmDirLinkedListInsertHead(pList, (PVOID)((uint64_t)rand()%20000), NULL);
        assert_int_equal(dwError, 0);
        dwCount++;
    }

    dwError = VmDirMergeSort(pList, _VmDirMergeSortCompareIntAscending);
    assert_int_equal(dwError, 0);
    assert_non_null(pList);
    assert_true(_VmDirMergeSortLinkedListIsSorted(pList, FALSE));

    VmDirFreeLinkedList(pList);
    pList = NULL;
}

static
int
_VmDirMergeSortCompareIntDescending(
    const PVOID    pInt1,
    const PVOID    pInt2
    )
{
    return ((uint64_t)pInt1 > (uint64_t)pInt2);
}

static
int
_VmDirMergeSortCompareIntAscending(
    const PVOID    pInt1,
    const PVOID    pInt2
    )
{
    return ((uint64_t)pInt1 < (uint64_t)pInt2);
}

static
BOOLEAN
_VmDirMergeSortLinkedListIsSorted(
    PVDIR_LINKED_LIST    pList,
    BOOLEAN              bDescending
    )
{
    PVDIR_LINKED_LIST_NODE    pTemp = NULL;
    BOOLEAN                   bSorted = TRUE;

    pTemp = pList->pHead;
    while (pTemp)
    {
        if (bDescending)
        {
            if (pTemp->pNext && (uint64_t)pTemp->pElement < (uint64_t)pTemp->pNext->pElement)
            {
                bSorted = FALSE;
                break;
            }
        }
        else
        {
            if (pTemp->pNext && (uint64_t)pTemp->pElement > (uint64_t)pTemp->pNext->pElement)
            {
                bSorted = FALSE;
                break;
            }
        }
        pTemp = pTemp->pNext;
    }

    if (bSorted)
    {
        pTemp = pList->pTail;
        while (pTemp)
        {
            if (bDescending)
            {
                if (pTemp->pPrev && (uint64_t)pTemp->pElement > (uint64_t)pTemp->pPrev->pElement)
                {
                    bSorted = FALSE;
                    break;
                }
            }
            else
            {
                if (pTemp->pPrev && (uint64_t)pTemp->pElement < (uint64_t)pTemp->pPrev->pElement)
                {
                    bSorted = FALSE;
                    break;
                }
            }
            pTemp = pTemp->pPrev;
        }
    }

    return bSorted;
}
