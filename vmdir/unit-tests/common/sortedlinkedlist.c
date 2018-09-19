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
BOOLEAN
_VmDirSortedLinkedListInsertCompare_Ascending(
    PVOID    pElement,
    PVOID    pNewElement
    );

static
BOOLEAN
_VmDirSortedLinkedListInsertCompare_Descending(
    PVOID    pElement,
    PVOID    pNewElement
    );

static
BOOLEAN
_VmDirLinkedListIsSorted(
    PVDIR_LINKED_LIST    pList,
    BOOLEAN              bDescending
    );

//Testing Ascending
VOID
VmDirSortedLinkedListInsert_AscendingOneElement(
    VOID    **state
    )
{
    DWORD                       dwError = 0;
    PVDIR_SORTED_LINKED_LIST    pSortedLinkedList = NULL;

    dwError = VmDirSortedLinkedListCreate(
            _VmDirSortedLinkedListInsertCompare_Ascending,
            &pSortedLinkedList);
    assert_int_equal(dwError, 0); 

    dwError = VmDirSortedLinkedListInsert(pSortedLinkedList, (PVOID)3);
    assert_int_equal(dwError, 0);

    assert_true(_VmDirLinkedListIsSorted(pSortedLinkedList->pList, FALSE));

    VmDirFreeSortedLinkedList(pSortedLinkedList);
}

VOID
VmDirSortedLinkedListInsert_AscendingTwoElements(
    VOID    **state
    )
{
    DWORD                       dwError = 0;
    PVDIR_SORTED_LINKED_LIST    pSortedLinkedList = NULL;

    //Two Element List - Insert to Head
    dwError = VmDirSortedLinkedListCreate(
            _VmDirSortedLinkedListInsertCompare_Ascending,
            &pSortedLinkedList);
    assert_int_equal(dwError, 0);

    dwError = VmDirSortedLinkedListInsert(pSortedLinkedList, (PVOID)3);
    assert_int_equal(dwError, 0);

    dwError = VmDirSortedLinkedListInsert(pSortedLinkedList, (PVOID)2);
    assert_int_equal(dwError, 0);

    assert_true(_VmDirLinkedListIsSorted(pSortedLinkedList->pList, FALSE));

    VmDirFreeSortedLinkedList(pSortedLinkedList);

    //Two Element List - Insert to Tail
    dwError = VmDirSortedLinkedListCreate(
            _VmDirSortedLinkedListInsertCompare_Ascending,
            &pSortedLinkedList);
    assert_int_equal(dwError, 0);

    dwError = VmDirSortedLinkedListInsert(pSortedLinkedList, (PVOID)3);
    assert_int_equal(dwError, 0);

    dwError = VmDirSortedLinkedListInsert(pSortedLinkedList, (PVOID)4);
    assert_int_equal(dwError, 0);

    assert_true(_VmDirLinkedListIsSorted(pSortedLinkedList->pList, FALSE));

    VmDirFreeSortedLinkedList(pSortedLinkedList);
}

VOID
VmDirSortedLinkedListInsert_AscendingThreeElements(
    VOID    **state
    )
{
    DWORD                       dwError = 0;
    PVDIR_SORTED_LINKED_LIST    pSortedLinkedList = NULL;

    //Three Element List - Same Element
    dwError = VmDirSortedLinkedListCreate(
            _VmDirSortedLinkedListInsertCompare_Ascending,
            &pSortedLinkedList);
    assert_int_equal(dwError, 0);

    dwError = VmDirSortedLinkedListInsert(pSortedLinkedList, (PVOID)1);
    assert_int_equal(dwError, 0);

    dwError = VmDirSortedLinkedListInsert(pSortedLinkedList, (PVOID)1);
    assert_int_equal(dwError, 0);

    dwError = VmDirSortedLinkedListInsert(pSortedLinkedList, (PVOID)1);
    assert_int_equal(dwError, 0);

    assert_true(_VmDirLinkedListIsSorted(pSortedLinkedList->pList, FALSE));

    VmDirFreeSortedLinkedList(pSortedLinkedList);

    //Three Element List - Insert in descending order
    dwError = VmDirSortedLinkedListCreate(
            _VmDirSortedLinkedListInsertCompare_Ascending,
            &pSortedLinkedList);
    assert_int_equal(dwError, 0);

    dwError = VmDirSortedLinkedListInsert(pSortedLinkedList, (PVOID)3);
    assert_int_equal(dwError, 0);

    dwError = VmDirSortedLinkedListInsert(pSortedLinkedList, (PVOID)2);
    assert_int_equal(dwError, 0);

    dwError = VmDirSortedLinkedListInsert(pSortedLinkedList, (PVOID)1);
    assert_int_equal(dwError, 0);

    assert_true(_VmDirLinkedListIsSorted(pSortedLinkedList->pList, FALSE));
    VmDirFreeSortedLinkedList(pSortedLinkedList);

    //Three Element List - Insert in ascending order
    dwError = VmDirSortedLinkedListCreate(
            _VmDirSortedLinkedListInsertCompare_Ascending,
            &pSortedLinkedList);
    assert_int_equal(dwError, 0);

    dwError = VmDirSortedLinkedListInsert(pSortedLinkedList, (PVOID)1);
    assert_int_equal(dwError, 0);

    dwError = VmDirSortedLinkedListInsert(pSortedLinkedList, (PVOID)2);
    assert_int_equal(dwError, 0);

    dwError = VmDirSortedLinkedListInsert(pSortedLinkedList, (PVOID)3);
    assert_int_equal(dwError, 0);

    assert_true(_VmDirLinkedListIsSorted(pSortedLinkedList->pList, FALSE));

    VmDirFreeSortedLinkedList(pSortedLinkedList);

    //Three Element List - Insert random
    dwError = VmDirSortedLinkedListCreate(
            _VmDirSortedLinkedListInsertCompare_Ascending,
            &pSortedLinkedList);
    assert_int_equal(dwError, 0);

    dwError = VmDirSortedLinkedListInsert(pSortedLinkedList, (PVOID)2);
    assert_int_equal(dwError, 0);

    dwError = VmDirSortedLinkedListInsert(pSortedLinkedList, (PVOID)1);
    assert_int_equal(dwError, 0);

    dwError = VmDirSortedLinkedListInsert(pSortedLinkedList, (PVOID)3);
    assert_int_equal(dwError, 0);

    assert_true(_VmDirLinkedListIsSorted(pSortedLinkedList->pList, FALSE));

    VmDirFreeSortedLinkedList(pSortedLinkedList);
}

VOID
VmDirSortedLinkedListInsert_Ascending(
    VOID    **state
    )
{
    DWORD                       dwError = 0;
    DWORD                       dwCount = 0;
    PVDIR_SORTED_LINKED_LIST    pSortedLinkedList = NULL;

    //2000 elements random
    dwError = VmDirSortedLinkedListCreate(
            _VmDirSortedLinkedListInsertCompare_Ascending,
            &pSortedLinkedList);
    assert_int_equal(dwError, 0);

    while (dwCount < 2000)
    {
        dwError = VmDirSortedLinkedListInsert(pSortedLinkedList, (PVOID)((uint64_t)rand()%20000));
        assert_int_equal(dwError, 0);

        dwCount++;
    }

    assert_true(_VmDirLinkedListIsSorted(pSortedLinkedList->pList, FALSE));

    VmDirFreeSortedLinkedList(pSortedLinkedList);
}

static
BOOLEAN
_VmDirSortedLinkedListInsertCompare_Ascending(
    PVOID    pElement,
    PVOID    pNewElement
    )
{
    BOOLEAN    bResult = FALSE;

    if ((USN)pElement <= (USN)pNewElement)
    {
        bResult = TRUE;
    }

    return bResult;
}

//Testing Descending
VOID
VmDirSortedLinkedListInsert_Descending(
    VOID    **state
    )
{
    DWORD                       dwError = 0;
    DWORD                       dwCount = 0;
    PVDIR_SORTED_LINKED_LIST    pSortedLinkedList = NULL;

    //2000 elements random
    dwError = VmDirSortedLinkedListCreate(
            _VmDirSortedLinkedListInsertCompare_Descending,
            &pSortedLinkedList);
    assert_int_equal(dwError, 0);

    while (dwCount < 2000)
    {
        dwError = VmDirSortedLinkedListInsert(pSortedLinkedList, (PVOID)((uint64_t)rand()%20000));
        assert_int_equal(dwError, 0);

        dwCount++;
    }

    assert_true(_VmDirLinkedListIsSorted(pSortedLinkedList->pList, TRUE));

    VmDirFreeSortedLinkedList(pSortedLinkedList);
}

static
BOOLEAN
_VmDirSortedLinkedListInsertCompare_Descending(
    PVOID    pElement,
    PVOID    pNewElement
    )
{
    BOOLEAN    bResult = FALSE;

    if ((USN)pElement >= (USN)pNewElement)
    {
        bResult = TRUE;
    }

    return bResult;
}

static
BOOLEAN
_VmDirLinkedListIsSorted(
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
