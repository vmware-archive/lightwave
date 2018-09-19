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
_VmDirAllocateAttrAndMetaData(
    PSTR                         pszAttrType,
    PSTR                         pszValue,
    USN                          localUsn,
    USN                          origUsn,
    UINT64                       version,
    PVMDIR_REPLICATION_UPDATE    pUpdate
    );

//AttributeChanges Setup Functions
int
VmDirSetupExtractEventAttributeChanges(
    VOID    **state
    )
{
    DWORD                        dwError = 0;
    PVMDIR_REPLICATION_UPDATE    pUpdate = NULL;

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pUpdate);
    assert_int_equal(dwError, 0);
    assert_non_null(pUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pUpdate->pEntry);
    assert_int_equal(dwError, 0);
    assert_non_null(pUpdate->pEntry);

    pUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;

    dwError = VmDirLinkedListCreate(&pUpdate->pMetaDataList);
    assert_int_equal(dwError, 0);

    _VmDirAllocateAttrAndMetaData(ATTR_USN_CHANGED, "101", 101, 101, 2, pUpdate);

    _VmDirAllocateAttrAndMetaData(ATTR_CN, "newuser_cn", 100, 100, 1, pUpdate);

    _VmDirAllocateAttrAndMetaData(ATTR_SN, "newuser_sn", 101, 100, 2, pUpdate);

    *state = pUpdate;

    return 0;
}

//AttributeChanges TearDown functions
int
VmDirTeardownExtractEvent(
    VOID    **state
    )
{
    PVMDIR_REPLICATION_UPDATE    pUpdate = NULL;

    pUpdate = (PVMDIR_REPLICATION_UPDATE)*state;

    VmDirFreeReplUpdate(pUpdate);

    return 0;
}

//AttributeValueChanges Setup functions
int
VmDirSetupExtractEventAttributeValueChanges(
    VOID    **state
    )
{
    DWORD                              dwError = 0;
    PSTR                               pszValueMetaData = NULL;
    PVDIR_ATTRIBUTE                    pAttr = NULL;
    PVMDIR_REPLICATION_UPDATE          pUpdate = NULL;
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pValueMetaData = NULL;

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pUpdate);
    assert_int_equal(dwError, 0);
    assert_non_null(pUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pUpdate->pEntry);
    assert_int_equal(dwError, 0);
    assert_non_null(pUpdate->pEntry);

    pUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;

    dwError = VmDirLinkedListCreate(&pUpdate->pValueMetaDataList);
    assert_int_equal(dwError, 0);

    //Create Value MetaData
    dwError = VmDirAllocateStringA(
            "member:101:1:de62357c-e8b1-4532-9c81-91c4f58c3248:de62357c-e8b1-4532-9c81-91c4f58c3248:20180726170334.296:101:0:1:B",
            &pszValueMetaData);
    assert_int_equal(dwError, 0);

    dwError = VmDirValueMetaDataDeserialize(pszValueMetaData, &pValueMetaData);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pUpdate->pValueMetaDataList, (PVOID)pValueMetaData, NULL);
    assert_int_equal(dwError, 0);
    pValueMetaData = NULL;

    VMDIR_SAFE_FREE_MEMORY(pszValueMetaData);

    dwError = VmDirAllocateStringA(
            "member:102:2:de62357c-e8b1-4532-9c81-91c4f58c3248:de62357c-e8b1-4532-9c81-91c4f58c3248:20180726170334.296:102:1:1:C",
            &pszValueMetaData);
    assert_int_equal(dwError, 0);

    dwError = VmDirValueMetaDataDeserialize(pszValueMetaData, &pValueMetaData);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pUpdate->pValueMetaDataList, (PVOID)pValueMetaData, NULL);
    assert_int_equal(dwError, 0);
    pValueMetaData = NULL;

    VMDIR_SAFE_FREE_MEMORY(pszValueMetaData);

    // Add Corresponding Attribute
    dwError = VmDirAllocateMemory(sizeof(VDIR_ATTRIBUTE), (PVOID*)&pAttr);
    assert_int_equal(dwError, 0);

    dwError = VmDirStringToBervalContent(ATTR_MEMBER, &pAttr->type);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateMemory(sizeof(VDIR_BERVALUE) * (3), (PVOID*)&pAttr->vals);
    assert_int_equal(dwError, 0);

    dwError = VmDirStringToBervalContent("A", &pAttr->vals[0]);
    assert_int_equal(dwError, 0);

    dwError = VmDirStringToBervalContent("B", &pAttr->vals[1]);
    assert_int_equal(dwError, 0);

    pAttr->numVals = 2;

    pAttr->next = pUpdate->pEntry->attrs;
    pUpdate->pEntry->attrs = pAttr;

    *state = pUpdate;

    return 0;
}

//AttributeChanges unit test functions
VOID
VmDirExtractEventAttributeChanges_ValidInput(
    VOID    **state
    )
{
    DWORD                             dwError = 0;
    PVMDIR_REPLICATION_UPDATE         pCombinedUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE         pIndividualUpdate = NULL;
    PVDIR_LINKED_LIST_NODE            pNode = NULL;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pReplMetaData = NULL;
    PVDIR_ATTRIBUTE                   pAttr = NULL;

    pCombinedUpdate = (PVMDIR_REPLICATION_UPDATE) *state;

    //Extract Event for USN: 101 (Attr: SN)
    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pIndividualUpdate);
    assert_int_equal(dwError, 0);
    assert_non_null(pIndividualUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pIndividualUpdate->pEntry);
    assert_int_equal(dwError, 0);
    assert_non_null(pIndividualUpdate->pEntry);

    pIndividualUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;

    dwError = VmDirExtractEventAttributeChanges(pCombinedUpdate, 101, pIndividualUpdate);
    assert_int_equal(dwError, 0);

    //Verify Attr MetaData is as expected
    pNode = pIndividualUpdate->pMetaDataList->pHead;
    pReplMetaData = (PVMDIR_REPL_ATTRIBUTE_METADATA)pNode->pElement;

    assert_string_equal(pReplMetaData->pszAttrType, ATTR_SN);
    assert_int_equal(pReplMetaData->pMetaData->localUsn, 101);
    assert_int_equal(pReplMetaData->pMetaData->version, 2);
    assert_string_equal(
            pReplMetaData->pMetaData->pszOrigInvoId, "7ef77c0f-cff1-4239-b293-39a2b302d5bd");
    assert_string_equal(pReplMetaData->pMetaData->pszOrigTime, "20180702222545.584");
    assert_int_equal(pReplMetaData->pMetaData->origUsn, 100);

    assert_null(pNode->pNext);

    //Verify Attr is as expected
    pAttr = pIndividualUpdate->pEntry->attrs;
    assert_string_equal(pAttr->type.lberbv_val, ATTR_SN);
    assert_string_equal(pAttr->vals[0].lberbv_val, "newuser_sn");
    assert_null(pAttr->next);

    //Verify pUpdate contents USN: 100 (Attr: CN and UsnChanged)
    //Verify Attr MetaData is as expected
    pNode = pCombinedUpdate->pMetaDataList->pHead;
    pReplMetaData = (PVMDIR_REPL_ATTRIBUTE_METADATA)pNode->pElement;

    assert_string_equal(pReplMetaData->pszAttrType, ATTR_CN);
    assert_int_equal(pReplMetaData->pMetaData->localUsn, 100);
    assert_int_equal(pReplMetaData->pMetaData->version, 1);
    assert_string_equal(
            pReplMetaData->pMetaData->pszOrigInvoId, "7ef77c0f-cff1-4239-b293-39a2b302d5bd");
    assert_string_equal(pReplMetaData->pMetaData->pszOrigTime, "20180702222545.584");
    assert_int_equal(pReplMetaData->pMetaData->origUsn, 100);

    pNode = pNode->pNext;
    pReplMetaData = (PVMDIR_REPL_ATTRIBUTE_METADATA)pNode->pElement;

    assert_string_equal(pReplMetaData->pszAttrType, ATTR_USN_CHANGED);
    assert_int_equal(pReplMetaData->pMetaData->localUsn, 101);
    assert_int_equal(pReplMetaData->pMetaData->version, 2);
    assert_string_equal(
            pReplMetaData->pMetaData->pszOrigInvoId, "7ef77c0f-cff1-4239-b293-39a2b302d5bd");
    assert_string_equal(pReplMetaData->pMetaData->pszOrigTime, "20180702222545.584");
    assert_int_equal(pReplMetaData->pMetaData->origUsn, 101);

    assert_null(pNode->pNext);

    //Verify Attr is as expected
    pAttr = pCombinedUpdate->pEntry->attrs;
    assert_string_equal(pAttr->type.lberbv_val, ATTR_USN_CHANGED);
    assert_string_equal(pAttr->vals[0].lberbv_val, "101");
    assert_non_null(pAttr->next);

    pAttr = pCombinedUpdate->pEntry->attrs->next;
    assert_string_equal(pAttr->type.lberbv_val, ATTR_CN);
    assert_string_equal(pAttr->vals[0].lberbv_val, "newuser_cn");
    assert_null(pAttr->next);

    VmDirFreeReplUpdate(pIndividualUpdate);
}

//AttributeValue Changes unit test functions
VOID
VmDirExtractEventAttributeValueChanges_ValidInput(
    VOID    **state
    )
{
    DWORD                              dwError = 0;
    VDIR_BERVALUE                      valueMetaData = VDIR_BERVALUE_INIT;
    PVMDIR_REPLICATION_UPDATE          pCombinedUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE          pIndividualUpdate = NULL;
    PVDIR_LINKED_LIST_NODE             pNode = NULL;
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pValueMetaData = NULL;
    PVDIR_ATTRIBUTE                    pAttr = NULL;

    pCombinedUpdate = (PVMDIR_REPLICATION_UPDATE) *state;
    assert_non_null(pCombinedUpdate);

    //Extract Event for USN: 102 (Value: C)
    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pIndividualUpdate);
    assert_int_equal(dwError, 0);
    assert_non_null(pIndividualUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pIndividualUpdate->pEntry);
    assert_int_equal(dwError, 0);
    assert_non_null(pIndividualUpdate->pEntry);

    pIndividualUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;

    dwError = VmDirExtractEventAttributeValueChanges(pCombinedUpdate, 102, pIndividualUpdate);
    assert_int_equal(dwError, 0);

    //validate extracted value metadata
    pNode = pIndividualUpdate->pValueMetaDataList->pHead;
    pValueMetaData = (PVMDIR_VALUE_ATTRIBUTE_METADATA) pNode->pElement;

    dwError = VmDirValueMetaDataSerialize(pValueMetaData, &valueMetaData);
    assert_int_equal(dwError, 0);

    assert_string_equal(valueMetaData.lberbv_val, "member:102:2:de62357c-e8b1-4532-9c81-91c4f58c3248:de62357c-e8b1-4532-9c81-91c4f58c3248:20180726170334.296:102:1:1:C");
    VmDirFreeBervalContent(&valueMetaData);

    assert_int_equal(VmDirLinkedListGetSize(pIndividualUpdate->pValueMetaDataList), 1);
    assert_int_equal(VmDirLinkedListGetSize(pCombinedUpdate->pValueMetaDataList), 1);

    assert_null(pNode->pNext);
    assert_null(pIndividualUpdate->pEntry->attrs);
    VmDirFreeValueMetaDataList(pIndividualUpdate->pValueMetaDataList);

    //validate pUpdate entry attrs
    pAttr = pCombinedUpdate->pEntry->attrs;
    assert_string_equal(pAttr->type.lberbv_val, ATTR_MEMBER);
    assert_string_equal(pAttr->vals[0].lberbv_val, "A");
    assert_string_equal(pAttr->vals[1].lberbv_val, "B");
    assert_string_equal(pAttr->vals[2].lberbv_val, "C");
    assert_int_equal(pAttr->numVals, 3);
    assert_null(pAttr->next);

    //Extract Event for USN: 101 (Value: B)
    dwError = VmDirExtractEventAttributeValueChanges(pCombinedUpdate, 101, pIndividualUpdate);
    assert_int_equal(dwError, 0);

    pNode = pIndividualUpdate->pValueMetaDataList->pHead;
    pValueMetaData = (PVMDIR_VALUE_ATTRIBUTE_METADATA) pNode->pElement;

    dwError = VmDirValueMetaDataSerialize(pValueMetaData, &valueMetaData);
    assert_int_equal(dwError, 0);

    assert_string_equal(valueMetaData.lberbv_val, "member:101:1:de62357c-e8b1-4532-9c81-91c4f58c3248:de62357c-e8b1-4532-9c81-91c4f58c3248:20180726170334.296:101:0:1:B");
    VmDirFreeBervalContent(&valueMetaData);

    assert_null(pNode->pNext);
    assert_null(pIndividualUpdate->pEntry->attrs);

    pAttr = pCombinedUpdate->pEntry->attrs;
    assert_string_equal(pAttr->type.lberbv_val, ATTR_MEMBER);
    assert_string_equal(pAttr->vals[0].lberbv_val, "A");
    assert_string_equal(pAttr->vals[1].lberbv_val, "C");
    assert_int_equal(pAttr->numVals, 2);
    assert_null(pAttr->next);

    assert_int_equal(VmDirLinkedListGetSize(pIndividualUpdate->pValueMetaDataList), 1);
    assert_int_equal(VmDirLinkedListGetSize(pCombinedUpdate->pValueMetaDataList), 0);

    VmDirFreeReplUpdate(pIndividualUpdate);
}

//Helper Functions
static
VOID
_VmDirAllocateAttrAndMetaData(
    PSTR                         pszAttrType,
    PSTR                         pszValue,
    USN                          localUsn,
    USN                          origUsn,
    UINT64                       version,
    PVMDIR_REPLICATION_UPDATE    pUpdate
    )
{
    DWORD                             dwError = 0;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pReplMetaData = NULL;
    PVDIR_ATTRIBUTE                   pAttr = NULL;

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPL_ATTRIBUTE_METADATA), (PVOID*)&pReplMetaData);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateStringA(pszAttrType, &pReplMetaData->pszAttrType);
    assert_int_equal(dwError, 0);

    dwError = VmDirMetaDataCreate(
            localUsn,
            version,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            "20180702222545.584",
            origUsn,
            &pReplMetaData->pMetaData);
    assert_int_equal(dwError, 0);

    //Populate pAttr
    dwError = VmDirAllocateMemory(sizeof(VDIR_ATTRIBUTE), (PVOID*)&pAttr);
    assert_int_equal(dwError, 0);

    dwError = VmDirStringToBervalContent(pszAttrType, &pAttr->type);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateMemory(sizeof(VDIR_BERVALUE) * (2), (PVOID*)&pAttr->vals);
    assert_int_equal(dwError, 0);

    dwError = VmDirStringToBervalContent(pszValue, &pAttr->vals[0]);
    assert_int_equal(dwError, 0);

    pAttr->numVals = 1;

    //Insert MetaData to pUpdate
    dwError = VmDirLinkedListInsertHead(pUpdate->pMetaDataList, (PVOID)pReplMetaData, NULL);
    assert_int_equal(dwError, 0);

    //Insert pAttr to pEntry
    pAttr->next = pUpdate->pEntry->attrs;
    pUpdate->pEntry->attrs = pAttr;
}
