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
_VmDirReplUpdateInsertToMetaDataList(
    PVDIR_LINKED_LIST    pMetaDataList,
    USN                  usn
    );

static
VOID
_VmDirReplUpdateInsertToValueMetaDataList(
    PVDIR_LINKED_LIST    pValueMetaDataList,
    USN                  usn
    );

//Case1: Only MetaData present and No Duplicate USN
VOID
VmDirReplUpdateToUSNList_NoDupMetaDataOnly(
    VOID**    state
    )
{
    DWORD                             dwError = 0;
    PVDIR_LINKED_LIST                 pMetaDataList = NULL;
    PVDIR_LINKED_LIST                 pUSNList = NULL;
    PVDIR_LINKED_LIST_NODE            pNode = NULL;
    PVMDIR_REPLICATION_UPDATE         pReplUpdate = NULL;

    // Create and populate MetaDataList
    dwError = VmDirLinkedListCreate(&pMetaDataList);
    assert_int_equal(dwError, 0);

    _VmDirReplUpdateInsertToMetaDataList(pMetaDataList, 405);
    _VmDirReplUpdateInsertToMetaDataList(pMetaDataList, 415);
    _VmDirReplUpdateInsertToMetaDataList(pMetaDataList, 425);
    _VmDirReplUpdateInsertToMetaDataList(pMetaDataList, 435);

    // Populate pReplUpdate with required fields
    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pReplUpdate);
    assert_int_equal(dwError, 0);

    pReplUpdate->pMetaDataList = pMetaDataList;

    // Actual Functionality
    dwError = VmDirReplUpdateToUSNList(pReplUpdate, &pUSNList);
    assert_int_equal(dwError, 0);

    // Validate USNList, pNext pointer
    VmDirLinkedListGetHead(pUSNList, &pNode);

    assert_int_equal((USN)pNode->pElement, 435);
    assert_int_equal((USN)pNode->pNext->pElement, 425);
    assert_int_equal((USN)pNode->pNext->pNext->pElement, 415);
    assert_int_equal((USN)pNode->pNext->pNext->pNext->pElement, 405);
    assert_null(pNode->pNext->pNext->pNext->pNext);

    // Validate USNList, pPrev pointer
    VmDirLinkedListGetTail(pUSNList, &pNode);

    assert_int_equal((USN)pNode->pElement, 405);
    assert_int_equal((USN)pNode->pPrev->pElement, 415);
    assert_int_equal((USN)pNode->pPrev->pPrev->pElement, 425);
    assert_int_equal((USN)pNode->pPrev->pPrev->pPrev->pElement, 435);
    assert_null(pNode->pPrev->pPrev->pPrev->pPrev);

    // Free pMetaDataList and its contents
    VmDirLinkedListGetHead(pMetaDataList, &pNode);

    while (pNode)
    {
        VmDirFreeReplMetaData((PVMDIR_REPL_ATTRIBUTE_METADATA)pNode->pElement);
        pNode->pElement = NULL;

        pNode = pNode->pNext;
    }

    VmDirFreeLinkedList(pMetaDataList);
    pReplUpdate->pMetaDataList = NULL;

    VmDirFreeLinkedList(pUSNList);
    VMDIR_SAFE_FREE_MEMORY(pReplUpdate);
}

//Case2: Only MetaData present and has only Duplicate USN
VOID
VmDirReplUpdateToUSNList_DupMetaDataOnly(
    VOID**    state
    )
{
    DWORD                             dwError = 0;
    PVDIR_LINKED_LIST                 pMetaDataList = NULL;
    PVDIR_LINKED_LIST                 pUSNList = NULL;
    PVMDIR_REPLICATION_UPDATE         pReplUpdate = NULL;
    PVDIR_LINKED_LIST_NODE            pNode = NULL;

    dwError = VmDirLinkedListCreate(&pMetaDataList);
    assert_int_equal(dwError, 0);

    _VmDirReplUpdateInsertToMetaDataList(pMetaDataList, 405);
    _VmDirReplUpdateInsertToMetaDataList(pMetaDataList, 405);
    _VmDirReplUpdateInsertToMetaDataList(pMetaDataList, 405);
    _VmDirReplUpdateInsertToMetaDataList(pMetaDataList, 405);

    // Populate pReplUpdate with required fields
    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pReplUpdate);
    assert_int_equal(dwError, 0);

    pReplUpdate->pMetaDataList = pMetaDataList;

    // Actual Functionality
    dwError = VmDirReplUpdateToUSNList(pReplUpdate, &pUSNList);
    assert_int_equal(dwError, 0);

    // Validate USNList, pNext pointer
    VmDirLinkedListGetHead(pUSNList, &pNode);

    assert_int_equal((USN)pNode->pElement, 405);
    assert_null(pNode->pNext);

    // Validate USNList, pPrev pointer
    VmDirLinkedListGetTail(pUSNList, &pNode);

    assert_int_equal((USN)pNode->pElement, 405);
    assert_null(pNode->pPrev);

    // Free pMetaDataList and its contents
    VmDirLinkedListGetHead(pMetaDataList, &pNode);

    while (pNode)
    {
        VmDirFreeReplMetaData((PVMDIR_REPL_ATTRIBUTE_METADATA)pNode->pElement);
        pNode->pElement = NULL;

        pNode = pNode->pNext;
    }

    VmDirFreeLinkedList(pMetaDataList);
    pReplUpdate->pMetaDataList = NULL;

    VmDirFreeLinkedList(pUSNList);
    VMDIR_SAFE_FREE_MEMORY(pReplUpdate);
}

//Case3: Only MetaData present and has Duplicate and unique USN.
VOID
VmDirReplUpdateToUSNList_MetaDataOnly(
    VOID**    state
    )
{
    DWORD                             dwError = 0;
    PVDIR_LINKED_LIST                 pMetaDataList = NULL;
    PVDIR_LINKED_LIST                 pUSNList = NULL;
    PVMDIR_REPLICATION_UPDATE         pReplUpdate = NULL;
    PVDIR_LINKED_LIST_NODE            pNode = NULL;

    dwError = VmDirLinkedListCreate(&pMetaDataList);
    assert_int_equal(dwError, 0);

    _VmDirReplUpdateInsertToMetaDataList(pMetaDataList, 405);
    _VmDirReplUpdateInsertToMetaDataList(pMetaDataList, 405);
    _VmDirReplUpdateInsertToMetaDataList(pMetaDataList, 425);
    _VmDirReplUpdateInsertToMetaDataList(pMetaDataList, 435);

    // Populate pReplUpdate with required fields
    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pReplUpdate);
    assert_int_equal(dwError, 0);

    pReplUpdate->pMetaDataList = pMetaDataList;

    // Actual Functionality
    dwError = VmDirReplUpdateToUSNList(pReplUpdate, &pUSNList);
    assert_int_equal(dwError, 0);

    // Validate USNList, pNext pointer
    VmDirLinkedListGetHead(pUSNList, &pNode);

    assert_int_equal((USN)pNode->pElement, 435);
    assert_int_equal((USN)pNode->pNext->pElement, 425);
    assert_int_equal((USN)pNode->pNext->pNext->pElement, 405);
    assert_null(pNode->pNext->pNext->pNext);

    // Validate USNList, pPrev pointer
    VmDirLinkedListGetTail(pUSNList, &pNode);

    assert_int_equal((USN)pNode->pElement, 405);
    assert_int_equal((USN)pNode->pPrev->pElement, 425);
    assert_int_equal((USN)pNode->pPrev->pPrev->pElement, 435);
    assert_null(pNode->pPrev->pPrev->pPrev);

    // Free pMetaDataList and its contents
    VmDirLinkedListGetHead(pMetaDataList, &pNode);

    while (pNode)
    {
        VmDirFreeReplMetaData((PVMDIR_REPL_ATTRIBUTE_METADATA)pNode->pElement);
        pNode->pElement = NULL;

        pNode = pNode->pNext;
    }

    VmDirFreeLinkedList(pMetaDataList);
    pReplUpdate->pMetaDataList = NULL;

    VmDirFreeLinkedList(pUSNList);
    VMDIR_SAFE_FREE_MEMORY(pReplUpdate);
}

//Case4: Only ValueMetaData present and has Duplicate and unique USN.
VOID
VmDirReplUpdateToUSNList_ValueMetaDataOnly(
    VOID**    state
    )
{
    DWORD                             dwError = 0;
    PVDIR_LINKED_LIST                 pValueMetaDataList = NULL;
    PVDIR_LINKED_LIST                 pUSNList = NULL;
    PVMDIR_REPLICATION_UPDATE         pReplUpdate = NULL;
    PVDIR_LINKED_LIST_NODE            pNode = NULL;

    dwError = VmDirLinkedListCreate(&pValueMetaDataList);
    assert_int_equal(dwError, 0);

    _VmDirReplUpdateInsertToValueMetaDataList(pValueMetaDataList, 405);
    _VmDirReplUpdateInsertToValueMetaDataList(pValueMetaDataList, 415);
    _VmDirReplUpdateInsertToValueMetaDataList(pValueMetaDataList, 425);
    _VmDirReplUpdateInsertToValueMetaDataList(pValueMetaDataList, 405);

    // Populate pReplUpdate with required fields
    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pReplUpdate);
    assert_int_equal(dwError, 0);

    pReplUpdate->pValueMetaDataList = pValueMetaDataList;

    // Actual Functionality
    dwError = VmDirReplUpdateToUSNList(pReplUpdate, &pUSNList);
    assert_int_equal(dwError, 87);

    assert_null(pUSNList);

    // Free pValueMetaDataList and its contents
    VmDirLinkedListGetHead(pValueMetaDataList, &pNode);

    while (pNode)
    {
        VmDirFreeValueMetaData((PVMDIR_VALUE_ATTRIBUTE_METADATA)pNode->pElement);
        pNode->pElement = NULL;

        pNode = pNode->pNext;
    }

    VmDirFreeLinkedList(pValueMetaDataList);
    pReplUpdate->pValueMetaDataList = NULL;

    VmDirFreeLinkedList(pUSNList);
    VMDIR_SAFE_FREE_MEMORY(pReplUpdate);
}

//case5: Both ValueMetaData and MetaData with duplicate values
VOID
VmDirReplUpdateToUSNList_MetaDataAndValueMetaData(
    VOID**    state
    )
{
    DWORD                             dwError = 0;
    PVDIR_LINKED_LIST                 pValueMetaDataList = NULL;
    PVDIR_LINKED_LIST                 pMetaDataList = NULL;
    PVDIR_LINKED_LIST                 pUSNList = NULL;
    PVMDIR_REPLICATION_UPDATE         pReplUpdate = NULL;
    PVDIR_LINKED_LIST_NODE            pNode = NULL;

    dwError = VmDirLinkedListCreate(&pMetaDataList);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListCreate(&pValueMetaDataList);
    assert_int_equal(dwError, 0);

    _VmDirReplUpdateInsertToMetaDataList(pMetaDataList, 405);
    _VmDirReplUpdateInsertToMetaDataList(pMetaDataList, 417);
    _VmDirReplUpdateInsertToMetaDataList(pMetaDataList, 415);

    _VmDirReplUpdateInsertToValueMetaDataList(pValueMetaDataList, 405);
    _VmDirReplUpdateInsertToValueMetaDataList(pValueMetaDataList, 425);
    _VmDirReplUpdateInsertToValueMetaDataList(pValueMetaDataList, 405);

    // Populate pReplUpdate with required fields
    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pReplUpdate);
    assert_int_equal(dwError, 0);

    pReplUpdate->pMetaDataList = pMetaDataList;
    pReplUpdate->pValueMetaDataList = pValueMetaDataList;

    // Actual Functionality
    dwError = VmDirReplUpdateToUSNList(pReplUpdate, &pUSNList);
    assert_int_equal(dwError, 0);

    // Validate USNList, pNext pointer
    VmDirLinkedListGetHead(pUSNList, &pNode);

    assert_int_equal((USN)pNode->pElement, 425);
    assert_int_equal((USN)pNode->pNext->pElement, 417);
    assert_int_equal((USN)pNode->pNext->pNext->pElement, 415);
    assert_int_equal((USN)pNode->pNext->pNext->pNext->pElement, 405);
    assert_null(pNode->pNext->pNext->pNext->pNext);

    // Validate USNList, pPrev pointer
    VmDirLinkedListGetTail(pUSNList, &pNode);

    assert_int_equal((USN)pNode->pElement, 405);
    assert_int_equal((USN)pNode->pPrev->pElement, 415);
    assert_int_equal((USN)pNode->pPrev->pPrev->pElement, 417);
    assert_int_equal((USN)pNode->pPrev->pPrev->pPrev->pElement, 425);
    assert_null(pNode->pPrev->pPrev->pPrev->pPrev);

    // Free pValueMetaDataList and its contents
    VmDirLinkedListGetHead(pValueMetaDataList, &pNode);

    while (pNode)
    {
        VmDirFreeValueMetaData((PVMDIR_VALUE_ATTRIBUTE_METADATA)pNode->pElement);
        pNode->pElement = NULL;

        pNode = pNode->pNext;
    }

    VmDirFreeLinkedList(pValueMetaDataList);
    pReplUpdate->pValueMetaDataList = NULL;

    // Free pMetaDataList and its contents
    VmDirLinkedListGetHead(pMetaDataList, &pNode);

    while (pNode)
    {
        VmDirFreeReplMetaData((PVMDIR_REPL_ATTRIBUTE_METADATA)pNode->pElement);
        pNode->pElement = NULL;

        pNode = pNode->pNext;
    }

    VmDirFreeLinkedList(pMetaDataList);
    pReplUpdate->pMetaDataList = NULL;

    VmDirFreeLinkedList(pUSNList);
    VMDIR_SAFE_FREE_MEMORY(pReplUpdate);
}

//case6: Both ValueMetaData and MetaData with same value
VOID
VmDirReplUpdateToUSNList_MetaDataAndValueMetaDataSame(
    VOID**    state
    )
{
    DWORD                             dwError = 0;
    PVDIR_LINKED_LIST                 pValueMetaDataList = NULL;
    PVDIR_LINKED_LIST                 pMetaDataList = NULL;
    PVDIR_LINKED_LIST                 pUSNList = NULL;
    PVMDIR_REPLICATION_UPDATE         pReplUpdate = NULL;
    PVDIR_LINKED_LIST_NODE            pNode = NULL;

    dwError = VmDirLinkedListCreate(&pMetaDataList);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListCreate(&pValueMetaDataList);
    assert_int_equal(dwError, 0);

    _VmDirReplUpdateInsertToMetaDataList(pMetaDataList, 405);
    _VmDirReplUpdateInsertToMetaDataList(pMetaDataList, 405);
    _VmDirReplUpdateInsertToMetaDataList(pMetaDataList, 405);

    _VmDirReplUpdateInsertToValueMetaDataList(pValueMetaDataList, 405);
    _VmDirReplUpdateInsertToValueMetaDataList(pValueMetaDataList, 405);
    _VmDirReplUpdateInsertToValueMetaDataList(pValueMetaDataList, 405);

    // Populate pReplUpdate with required fields
    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pReplUpdate);
    assert_int_equal(dwError, 0);

    pReplUpdate->pMetaDataList = pMetaDataList;
    pReplUpdate->pValueMetaDataList = pValueMetaDataList;

    // Actual Functionality
    dwError = VmDirReplUpdateToUSNList(pReplUpdate, &pUSNList);
    assert_int_equal(dwError, 0);

    // Validate USNList, pNext pointer
    VmDirLinkedListGetHead(pUSNList, &pNode);

    assert_int_equal((USN)pNode->pElement, 405);
    assert_null(pNode->pNext);

    // Validate USNList, pPrev pointer
    VmDirLinkedListGetTail(pUSNList, &pNode);

    assert_int_equal((USN)pNode->pElement, 405);
    assert_null(pNode->pPrev);

    // Free pValueMetaDataList and its contents
    VmDirLinkedListGetHead(pValueMetaDataList, &pNode);

    while (pNode)
    {
        VmDirFreeValueMetaData((PVMDIR_VALUE_ATTRIBUTE_METADATA)pNode->pElement);
        pNode->pElement = NULL;

        pNode = pNode->pNext;
    }

    VmDirFreeLinkedList(pValueMetaDataList);
    pReplUpdate->pValueMetaDataList = NULL;

    // Free pMetaDataList and its contents
    VmDirLinkedListGetHead(pMetaDataList, &pNode);

    while (pNode)
    {
        VmDirFreeReplMetaData((PVMDIR_REPL_ATTRIBUTE_METADATA)pNode->pElement);
        pNode->pElement = NULL;

        pNode = pNode->pNext;
    }

    VmDirFreeLinkedList(pMetaDataList);
    pReplUpdate->pMetaDataList = NULL;

    VmDirFreeLinkedList(pUSNList);
    VMDIR_SAFE_FREE_MEMORY(pReplUpdate);
}

VOID
VmDirReplUpdateLocalUsn_ValidInput(
    VOID    **state
    )
{
    DWORD                             dwError = 0;
    DWORD                             dwBerSize = 2;
    PVDIR_ENTRY                       pEntry = NULL;
    PVDIR_ATTRIBUTE                   pAttr = NULL;
    PVDIR_LINKED_LIST                 pUSNList = NULL;
    PVDIR_LINKED_LIST_NODE            pNode = NULL;
    PVMDIR_REPLICATION_UPDATE         pReplUpdate = NULL;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pReplMetaData = NULL;

    //construct USNList
    dwError = VmDirLinkedListCreate(&pUSNList);
    assert_int_equal(dwError, 0);

    VmDirLinkedListInsertTail(pUSNList, (PVOID)12, NULL);
    VmDirLinkedListInsertTail(pUSNList, (PVOID)10, NULL);

    //Construct pEntry
    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pEntry);
    assert_int_equal(dwError, 0);

    //Construct USNChanged Attribute
    dwError = VmDirAllocateMemory(sizeof(VDIR_ATTRIBUTE), (PVOID*)&pAttr);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateStringA(ATTR_USN_CHANGED, &pAttr->type.lberbv.bv_val);
    assert_int_equal(dwError, 0);

    pAttr->type.lberbv.bv_len = VmDirStringLenA(pAttr->type.lberbv.bv_val);
    pAttr->type.bOwnBvVal = TRUE;

    dwError = VmDirAllocateMemory(sizeof(VDIR_BERVALUE) * (dwBerSize), (PVOID*)&pAttr->vals);
    assert_int_equal(dwError, 0);

    pAttr->numVals = dwBerSize;

    dwError = VmDirAllocateStringA("164", &pAttr->vals[0].lberbv.bv_val);
    assert_int_equal(dwError, 0);

    pAttr->vals[0].lberbv.bv_len = VmDirStringLenA(pAttr->vals[0].lberbv.bv_val);
    pAttr->vals[0].bOwnBvVal = TRUE;

    pEntry->attrs = pAttr;

    //Construct pReplMetaData
    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPL_ATTRIBUTE_METADATA), (PVOID*)&pReplMetaData);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateMemory(sizeof(VMDIR_ATTRIBUTE_METADATA), (PVOID*)&pReplMetaData->pMetaData);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateStringA(ATTR_USN_CHANGED, &pReplMetaData->pszAttrType);
    assert_int_equal(dwError, 0);

    pReplMetaData->pMetaData->localUsn = 164;

    //Construct pReplUpdate
    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pReplUpdate);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListCreate(&pReplUpdate->pMetaDataList);
    assert_int_equal(dwError, 0);

    VmDirLinkedListInsertHead(pReplUpdate->pMetaDataList, pReplMetaData, NULL);
    pReplMetaData = NULL;

    pReplUpdate->pEntry = pEntry;
    pEntry = NULL;

    //Actual Functionality
    dwError = VmDirReplUpdateLocalUsn(pReplUpdate, pUSNList);
    assert_int_equal(dwError, 0);

    //Test the functionality
    assert_int_equal(pReplUpdate->partnerUsn, 10);
    assert_string_equal(pReplUpdate->pEntry->attrs->vals[0].lberbv_val, "10");

    dwError = VmDirLinkedListGetHead(pReplUpdate->pMetaDataList, &pNode);
    assert_int_equal(dwError, 0);

    pReplMetaData = (PVMDIR_REPL_ATTRIBUTE_METADATA) pNode->pElement;

    assert_int_equal(pReplMetaData->pMetaData->localUsn, 10);

    dwError = VmDirLinkedListGetHead(pUSNList, &pNode);

    assert_int_equal((USN)pNode->pElement, 12);
    assert_null(pNode->pNext);
    assert_null(pNode->pPrev);

    //Free the constructed parameters
    VmDirFreeLinkedList(pUSNList);
    VmDirFreeReplUpdate(pReplUpdate);
}

static
VOID
_VmDirReplUpdateInsertToMetaDataList(
    PVDIR_LINKED_LIST    pMetaDataList,
    USN                  usn
    )
{
    DWORD                             dwError = 0;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pReplMetaData = NULL;

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPL_ATTRIBUTE_METADATA), (PVOID*)&pReplMetaData);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateStringA(ATTR_CN, &pReplMetaData->pszAttrType);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateMemory(
            sizeof(VMDIR_ATTRIBUTE_METADATA), (PVOID*)&pReplMetaData->pMetaData);
    assert_int_equal(dwError, 0);

    pReplMetaData->pMetaData->localUsn = usn;

    VmDirLinkedListInsertTail(pMetaDataList, (PVOID)pReplMetaData, NULL);
}

static
VOID
_VmDirReplUpdateInsertToValueMetaDataList(
    PVDIR_LINKED_LIST    pValueMetaDataList,
    USN                  usn
    )
{
    DWORD                             dwError = 0;
    PVMDIR_VALUE_ATTRIBUTE_METADATA   pValueMetaData = NULL;

    dwError = VmDirAllocateMemory(sizeof(VMDIR_VALUE_ATTRIBUTE_METADATA), (PVOID*)&pValueMetaData);
    assert_int_equal(dwError, 0);

    pValueMetaData->localUsn = usn;

    VmDirLinkedListInsertTail(pValueMetaDataList, pValueMetaData, NULL);
}
