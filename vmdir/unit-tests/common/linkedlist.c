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

VOID
VmDirLinkedListAppendListToTail_ValidInput(
    VOID    **state
    )
{
    DWORD                     dwError = 0;
    PVDIR_LINKED_LIST         pDestList = NULL;
    PVDIR_LINKED_LIST         pSrcList = NULL;
    PVDIR_LINKED_LIST_NODE    pNode = NULL;

    //DestList Empty
    dwError = VmDirLinkedListCreate(&pDestList);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListCreate(&pSrcList);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pSrcList, (PVOID)8, NULL);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pSrcList, (PVOID)6, NULL);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListAppendListToTail(pDestList, pSrcList);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListGetHead(pDestList, &pNode);

    assert_int_equal((int64_t)pNode->pElement, 6);
    assert_int_equal((int64_t)pNode->pNext->pElement, 8);
    assert_null(pNode->pNext->pNext);

    dwError = VmDirLinkedListGetTail(pDestList, &pNode);

    assert_int_equal((int64_t)pNode->pElement, 8);
    assert_int_equal((int64_t)pNode->pPrev->pElement, 6);
    assert_int_equal(pDestList->iSize, 2);
    assert_null(pNode->pPrev->pPrev);

    VmDirFreeLinkedList(pSrcList);
    VmDirFreeLinkedList(pDestList);

    //Merge SrcList with DestList
    dwError = VmDirLinkedListCreate(&pDestList);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pDestList, (PVOID)14, NULL);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pDestList, (PVOID)10, NULL);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListCreate(&pSrcList);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pSrcList, (PVOID)8, NULL);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pSrcList, (PVOID)6, NULL);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListAppendListToTail(pDestList, pSrcList);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListGetHead(pDestList, &pNode);

    assert_int_equal((int64_t)pNode->pElement, 10);
    assert_int_equal((int64_t)pNode->pNext->pElement, 14);
    assert_int_equal((int64_t)pNode->pNext->pNext->pElement, 6);
    assert_int_equal((int64_t)pNode->pNext->pNext->pNext->pElement, 8);
    assert_null(pNode->pNext->pNext->pNext->pNext);

    dwError = VmDirLinkedListGetTail(pDestList, &pNode);

    assert_int_equal((int64_t)pNode->pElement, 8);
    assert_int_equal((int64_t)pNode->pPrev->pElement, 6);
    assert_int_equal((int64_t)pNode->pPrev->pPrev->pElement, 14);
    assert_int_equal((int64_t)pNode->pPrev->pPrev->pPrev->pElement, 10);
    assert_null(pNode->pPrev->pPrev->pPrev->pPrev);

    assert_int_equal(pDestList->iSize, 4);

    VmDirFreeLinkedList(pSrcList);
    VmDirFreeLinkedList(pDestList);
}

VOID
VmDirLinkedListAppendListToTail_InvalidInput(
    VOID    **state
    )
{
    DWORD                     dwError = 0;
    PVDIR_LINKED_LIST         pDestList = NULL;
    PVDIR_LINKED_LIST         pSrcList = NULL;

    dwError = VmDirLinkedListCreate(&pDestList);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListCreate(&pSrcList);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListAppendListToTail(NULL, NULL);
    assert_int_equal(dwError, 9005);

    dwError = VmDirLinkedListAppendListToTail(NULL, pSrcList);
    assert_int_equal(dwError, 9005);

    dwError = VmDirLinkedListAppendListToTail(pDestList, NULL);
    assert_int_equal(dwError, 9005);

    dwError = VmDirLinkedListAppendListToTail(pDestList, pSrcList);
    assert_int_equal(dwError, 0);

    assert_null(pDestList->pHead);
    assert_null(pDestList->pTail);
    assert_int_equal(pDestList->iSize, 0);
    assert_null(pSrcList->pHead);
    assert_null(pSrcList->pTail);
    assert_int_equal(pSrcList->iSize, 0);

    VmDirFreeLinkedList(pSrcList);
    VmDirFreeLinkedList(pDestList);
}
