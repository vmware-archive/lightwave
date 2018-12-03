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

typedef struct _VMDIR_EXTRACT_EVENT_TEST_CONTEXT
{
    PVMDIR_REPLICATION_UPDATE   pIndividualUpdate;
    PVMDIR_REPLICATION_UPDATE   pCombinedUpdate;
    USN                         usn;
    PVMDIR_REPLICATION_UPDATE   pExpectedCombinedUpdate;
} VMDIR_EXTRACT_EVENT_TEST_CONTEXT, *PVMDIR_EXTRACT_EVENT_TEST_CONTEXT;

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

    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CHANGED,
            "101",
            101,
            101,
            2,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pUpdate);

    VmDirAllocateAttrAndMetaData(
            ATTR_CN,
            "newuser_cn",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pUpdate);

    VmDirAllocateAttrAndMetaData(
            ATTR_SN,
            "newuser_sn",
            101,
            101,
            2,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pUpdate);

    *state = pUpdate;

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
    PVMDIR_REPL_ATTRIBUTE_METADATA     pReplMetaData = NULL;

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pUpdate);
    assert_int_equal(dwError, 0);
    assert_non_null(pUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pUpdate->pEntry);
    assert_int_equal(dwError, 0);
    assert_non_null(pUpdate->pEntry);

    pUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;

    dwError = VmDirLinkedListCreate(&pUpdate->pMetaDataList);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPL_ATTRIBUTE_METADATA), (PVOID*)&pReplMetaData);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateStringA(ATTR_MEMBER, &pReplMetaData->pszAttrType);
    assert_int_equal(dwError, 0);

    dwError = VmDirMetaDataDeserialize(
            "100:1:de62357c-e8b1-4532-9c81-91c4f58c3248:20180726170334.296:100",
            &pReplMetaData->pMetaData);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pUpdate->pMetaDataList, (PVOID)pReplMetaData, NULL);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListCreate(&pUpdate->pValueMetaDataList);
    assert_int_equal(dwError, 0);

    //Add Value MetaData
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

    //Delete value MetaData
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

//PopulateMustAttributes Setup Functions
int
VmDirTestSetupExtractEventPopulateMustAttributes(
    VOID    **state
    )
{
    DWORD                               dwError = 0;
    PVMDIR_REPLICATION_UPDATE           pIndividualUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE           pCombinedUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE           pExpectedCombinedUpdate = NULL;
    PVMDIR_EXTRACT_EVENT_TEST_CONTEXT   pTestContext = NULL;

    dwError = VmDirTestAllocateReplUpdate(&pIndividualUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);

    pIndividualUpdate->syncState = LDAP_SYNC_ADD;

    /*
     * Add a may attr newuser_sn and a must attr newuser_cn to the individual update but not to the combined update.
     * The expected behavior is that the must attr should be copied to the combined update by VmDirExtractEventPopulateMustAttributes
     */
    VmDirAllocateAttrAndMetaData(
            ATTR_SN,
            "newuser_sn",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);

    VmDirAllocateAttrAndMetaData(
            ATTR_CN,
            "newuser_cn",
            101,
            101,
            2,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);

    dwError = VmDirTestAllocateReplUpdate(&pCombinedUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CREATED,
            "100",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);

    pCombinedUpdate->syncState = LDAP_SYNC_ADD;
    pCombinedUpdate->partnerUsn = 100;

    dwError = VmDirTestAllocateReplUpdate(&pExpectedCombinedUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CREATED,
            "100",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pExpectedCombinedUpdate);

    pExpectedCombinedUpdate->syncState = LDAP_SYNC_ADD;
    pExpectedCombinedUpdate->partnerUsn = 100;

    //Verify Attr MetaData is as expected in pIndividualUpdate
    VmDirAllocateAttrAndMetaData(
            ATTR_CN,
            "newuser_cn",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pExpectedCombinedUpdate);

    dwError = VmDirAllocateMemory(sizeof(VMDIR_EXTRACT_EVENT_TEST_CONTEXT), (PVOID*)&pTestContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    pTestContext->pIndividualUpdate = pIndividualUpdate;
    pTestContext->pCombinedUpdate = pCombinedUpdate;
    pTestContext->pExpectedCombinedUpdate = pExpectedCombinedUpdate;

    *state = pTestContext;

cleanup:
    return dwError;

error:
    print_message("%s failed with error %d", __FUNCTION__, dwError);
    goto cleanup;
}

//PopulateOperationAttributes Setup Functions
int
VmDirSetupExtractEventPopulateOperationAttributes(
    VOID **state
    )
{
    DWORD    dwError = 0;
    PVMDIR_REPLICATION_UPDATE    pUpdate = NULL;

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pUpdate);
    assert_int_equal(dwError, 0);
    assert_non_null(pUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pUpdate->pEntry);
    assert_int_equal(dwError, 0);
    assert_non_null(pUpdate->pEntry);

    pUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pUpdate->partnerUsn = 100;
    pUpdate->syncState = LDAP_SYNC_ADD;

    dwError = VmDirLinkedListCreate(&pUpdate->pMetaDataList);
    assert_int_equal(dwError, 0);

    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CHANGED,
            "101",
            101,
            101,
            2,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pUpdate);

    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID,
            "e7f6eae8-9902-4270-91ee-1ab36c898580",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pUpdate);

    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CREATED,
            "100",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pUpdate);

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

int VmDirTestTeardownExtractEventMustAttr(
    VOID    **state
    )
{
    PVMDIR_REPLICATION_UPDATE           pIndividualUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE           pCombinedUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE           pExpectedCombinedUpdate = NULL;
    PVMDIR_EXTRACT_EVENT_TEST_CONTEXT   pTestContext = NULL;

    pTestContext = (PVMDIR_EXTRACT_EVENT_TEST_CONTEXT) *state;
    pIndividualUpdate = pTestContext->pIndividualUpdate;
    pCombinedUpdate = pTestContext->pCombinedUpdate;
    pExpectedCombinedUpdate = pTestContext->pExpectedCombinedUpdate;

    VmDirFreeReplUpdate(pIndividualUpdate);
    VmDirFreeReplUpdate(pCombinedUpdate);
    VmDirFreeReplUpdate(pExpectedCombinedUpdate);

    VMDIR_SAFE_FREE_MEMORY(pTestContext);

    return 0;
}

/*
 * AttributeChanges unit test functions
 * Input:
 *     pCombinedUpdate:
 *         USN: 101    ATTR: USN_CHANGED
 *         USN: 100    ATTR: CN
 *         USN: 101    ATTR: SN
 * Expected outcome:
 *     pIndividualUpdate:
 *         USN: 101    ATTR: SN
 *     pCombinedUpdate:
 *         USN: 101    ATTR: USN_CHANGED
 *         USN: 100    ATTR: CN
 */
VOID
VmDirExtractEventAttributeChanges_ValidInput(
    VOID    **state
    )
{
    DWORD                             dwError = 0;
    PVMDIR_REPLICATION_UPDATE         pCombinedUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE         pIndividualUpdate = NULL;
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
    pReplMetaData = VmDirFindAttrMetaData(pIndividualUpdate->pMetaDataList, ATTR_SN);
    assert_non_null(pReplMetaData);

    assert_string_equal(pReplMetaData->pszAttrType, ATTR_SN);
    assert_int_equal(pReplMetaData->pMetaData->localUsn, 101);
    assert_int_equal(pReplMetaData->pMetaData->version, 2);
    assert_string_equal(
            pReplMetaData->pMetaData->pszOrigInvoId, "7ef77c0f-cff1-4239-b293-39a2b302d5bd");
    assert_string_equal(pReplMetaData->pMetaData->pszOrigTime, "20180702222545.584");
    assert_int_equal(pReplMetaData->pMetaData->origUsn, 101);

    //Verify Attr is as expected
    pAttr = VmDirFindAttrByName(pIndividualUpdate->pEntry, ATTR_SN);
    assert_non_null(pAttr);
    assert_string_equal(pAttr->type.lberbv_val, ATTR_SN);
    assert_string_equal(pAttr->vals[0].lberbv_val, "newuser_sn");

    //Verify pUpdate contents USN: 100 (Attr: CN and UsnChanged)
    //Verify Attr MetaData is as expected
    pReplMetaData = VmDirFindAttrMetaData(pCombinedUpdate->pMetaDataList, ATTR_CN);
    assert_non_null(pReplMetaData);

    assert_string_equal(pReplMetaData->pszAttrType, ATTR_CN);
    assert_int_equal(pReplMetaData->pMetaData->localUsn, 100);
    assert_int_equal(pReplMetaData->pMetaData->version, 1);
    assert_string_equal(
            pReplMetaData->pMetaData->pszOrigInvoId, "7ef77c0f-cff1-4239-b293-39a2b302d5bd");
    assert_string_equal(pReplMetaData->pMetaData->pszOrigTime, "20180702222545.584");
    assert_int_equal(pReplMetaData->pMetaData->origUsn, 100);

    pReplMetaData = VmDirFindAttrMetaData(pCombinedUpdate->pMetaDataList, ATTR_USN_CHANGED);
    assert_non_null(pReplMetaData);

    assert_string_equal(pReplMetaData->pszAttrType, ATTR_USN_CHANGED);
    assert_int_equal(pReplMetaData->pMetaData->localUsn, 101);
    assert_int_equal(pReplMetaData->pMetaData->version, 2);
    assert_string_equal(
            pReplMetaData->pMetaData->pszOrigInvoId, "7ef77c0f-cff1-4239-b293-39a2b302d5bd");
    assert_string_equal(pReplMetaData->pMetaData->pszOrigTime, "20180702222545.584");
    assert_int_equal(pReplMetaData->pMetaData->origUsn, 101);

    //Verify Attr is as expected
    pAttr = VmDirFindAttrByName(pCombinedUpdate->pEntry, ATTR_USN_CHANGED);
    assert_non_null(pAttr);
    assert_string_equal(pAttr->type.lberbv_val, ATTR_USN_CHANGED);
    assert_string_equal(pAttr->vals[0].lberbv_val, "101");

    pAttr = VmDirFindAttrByName(pCombinedUpdate->pEntry, ATTR_CN);
    assert_non_null(pAttr);
    assert_string_equal(pAttr->type.lberbv_val, ATTR_CN);
    assert_string_equal(pAttr->vals[0].lberbv_val, "newuser_cn");

    VmDirFreeReplUpdate(pIndividualUpdate);
}

/*
 * AttributeValue Changes unit test functions
 * Input:
 *     pCombinedUpdate:
 *         USN: 101 ATTR: MEMBER Value: B OP: ADD
 *         USN: 102 ATTR: MEMBER Value: C OP: DEL
 *         ATTR: MEMBER Value: A, B
 * Expected outcome:
 *     pIndividualUpdate:
 *         USN: 102 ATTR: MEMBER Value: C OP: DEL
 *         ATTR: MEMBER Value: A, B, C
 *     pIndividualUpdate:
 *         USN: 101 ATTR: MEMBER Value: B OP: ADD
 *         ATTR: MEMBER Value: A, C
 */
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
    pAttr = VmDirFindAttrByName(pCombinedUpdate->pEntry, ATTR_MEMBER);
    assert_non_null(pAttr);

    assert_string_equal(pAttr->type.lberbv_val, ATTR_MEMBER);
    assert_string_equal(pAttr->vals[0].lberbv_val, "A");
    assert_string_equal(pAttr->vals[1].lberbv_val, "B");
    assert_string_equal(pAttr->vals[2].lberbv_val, "C");
    assert_int_equal(pAttr->numVals, 3);

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

    pAttr = VmDirFindAttrByName(pCombinedUpdate->pEntry, ATTR_MEMBER);
    assert_non_null(pAttr);

    assert_string_equal(pAttr->type.lberbv_val, ATTR_MEMBER);
    assert_string_equal(pAttr->vals[0].lberbv_val, "A");
    assert_string_equal(pAttr->vals[1].lberbv_val, "C");
    assert_int_equal(pAttr->numVals, 2);

    assert_int_equal(VmDirLinkedListGetSize(pIndividualUpdate->pValueMetaDataList), 1);
    assert_int_equal(VmDirLinkedListGetSize(pCombinedUpdate->pValueMetaDataList), 0);

    VmDirFreeReplUpdate(pIndividualUpdate);
}

/*
 * PopulateMustAttributes unit test functions
 * Input:
 *     pIndividualUpdate:
 *         USN: 101    ATTR: CN
 *         USN: 100    ATTR: SN
 * Expected outcome:
 *     pCombinedUpdate:
 *         USN: 100    ATTR: CN
 */
VOID
VmDirTestExtractEventPopulateMustAttributes_ValidInput(
    VOID    **state
    )
{
    DWORD                               dwError = 0;
    PVMDIR_REPLICATION_UPDATE           pCombinedUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE           pIndividualUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE           pExpectedCombinedUpdate = NULL;
    PVMDIR_EXTRACT_EVENT_TEST_CONTEXT   pTestContext = NULL;

    pTestContext = (PVMDIR_EXTRACT_EVENT_TEST_CONTEXT) *state;
    pIndividualUpdate = pTestContext->pIndividualUpdate;
    pCombinedUpdate = pTestContext->pCombinedUpdate;
    pExpectedCombinedUpdate = pTestContext->pExpectedCombinedUpdate;

    dwError = VmDirExtractEventPopulateMustAttributes(pIndividualUpdate, pCombinedUpdate);
    assert_int_equal(dwError, 0);
    assert_true(VmDirTestCompareReplUpdate(pExpectedCombinedUpdate, pCombinedUpdate));
}

/*
 * PopulateOperationAttributes unit test functions
 * Input:
 *     pCombinedUpdate:
 *         USN: 101    ATTR: USN_CHANGED
 *         USN: 100    ATTR: OBJECT_GUID
 *     pIndividualUpdate:
 *         USN: 101    ATTR: LAST_KNOWN_DN
 *         USN: 101    ATTR: IS_DELETED
 *         USN: 101    ATTR: OBJECT_CLASS
 * Expected outcome:
 *     pIndividualUpdate->syncState = LDAP_SYNC_DELETE
 *     pIndividualUpdate:
 *         USN: 101    ATTR: USN_CHANGED
 *         USN: 100    ATTR: OBJECT_GUID
 *         USN: 101    ATTR: LAST_KNOWN_DN
 *         USN: 101    ATTR: IS_DELETED
 *     pCombinedUpdate:
 *         USN: 101    ATTR: USN_CHANGED
 *         USN: 100    ATTR: OBJECT_GUID
 *         USN: 100    ATTR: ENTRY_DN
 *         USN: 100    ATTR: OBJECT_CLASS
 */
VOID
VmDirExtractEventPopulateOperationAttributes_ValidInput(
    VOID    **state
    )
{
    DWORD                             dwError = 0;
    PVDIR_ATTRIBUTE                   pAttr = NULL;
    PVMDIR_REPLICATION_UPDATE         pCombinedUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE         pIndividualUpdate = NULL;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pReplMetaData = NULL;

    pCombinedUpdate = (PVMDIR_REPLICATION_UPDATE) *state;

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pIndividualUpdate);
    assert_int_equal(dwError, 0);
    assert_non_null(pIndividualUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pIndividualUpdate->pEntry);
    assert_int_equal(dwError, 0);
    assert_non_null(pIndividualUpdate->pEntry);

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pMetaDataList);
    assert_int_equal(dwError, 0);

    pIndividualUpdate->partnerUsn = 101;
    pIndividualUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;

    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_CLASS,
            OC_DELETED_OBJECT,
            101,
            101,
            2,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);

    VmDirAllocateAttrAndMetaData(
            ATTR_IS_DELETED,
            "true",
            101,
            101,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);

    VmDirAllocateAttrAndMetaData(
            ATTR_LAST_KNOWN_DN,
            "cn=newuser,cn=users,dc=lw-testdom,dc=com",
            101,
            101,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);

    dwError = VmDirExtractEventPopulateOperationAttributes(pCombinedUpdate, pIndividualUpdate);
    assert_int_equal(dwError, 0);

    assert_int_equal(pIndividualUpdate->syncState, LDAP_SYNC_DELETE);

    //Verify Attr MetaData is as expected in pIndividualUpdate
    pReplMetaData = VmDirFindAttrMetaData(pIndividualUpdate->pMetaDataList, ATTR_USN_CHANGED);
    assert_non_null(pReplMetaData);

    assert_string_equal(pReplMetaData->pszAttrType, ATTR_USN_CHANGED);
    assert_int_equal(pReplMetaData->pMetaData->localUsn, 101);
    assert_int_equal(pReplMetaData->pMetaData->version, 2);
    assert_string_equal(
            pReplMetaData->pMetaData->pszOrigInvoId, "7ef77c0f-cff1-4239-b293-39a2b302d5bd");
    assert_string_equal(pReplMetaData->pMetaData->pszOrigTime, "20180702222545.584");
    assert_int_equal(pReplMetaData->pMetaData->origUsn, 101);

    pReplMetaData = VmDirFindAttrMetaData(pIndividualUpdate->pMetaDataList, ATTR_OBJECT_GUID);
    assert_non_null(pReplMetaData);

    assert_string_equal(pReplMetaData->pszAttrType, ATTR_OBJECT_GUID);
    assert_int_equal(pReplMetaData->pMetaData->localUsn, 100);
    assert_int_equal(pReplMetaData->pMetaData->version, 1);
    assert_string_equal(
            pReplMetaData->pMetaData->pszOrigInvoId, "7ef77c0f-cff1-4239-b293-39a2b302d5bd");
    assert_string_equal(pReplMetaData->pMetaData->pszOrigTime, "20180702222545.584");
    assert_int_equal(pReplMetaData->pMetaData->origUsn, 100);

    //Verify Attr MetaData is as expected in pCombinedUpdate
    pReplMetaData = VmDirFindAttrMetaData(pCombinedUpdate->pMetaDataList, ATTR_DN);
    assert_non_null(pReplMetaData);
    assert_string_equal(pReplMetaData->pszAttrType, ATTR_DN);
    assert_int_equal(pReplMetaData->pMetaData->localUsn, 100);
    assert_int_equal(pReplMetaData->pMetaData->version, 1);
    assert_string_equal(
            pReplMetaData->pMetaData->pszOrigInvoId, "7ef77c0f-cff1-4239-b293-39a2b302d5bd");
    assert_string_equal(pReplMetaData->pMetaData->pszOrigTime, "20180702222545.584");
    assert_int_equal(pReplMetaData->pMetaData->origUsn, 100);

    pReplMetaData = VmDirFindAttrMetaData(pCombinedUpdate->pMetaDataList, ATTR_OBJECT_CLASS);
    assert_non_null(pReplMetaData);

    assert_string_equal(pReplMetaData->pszAttrType, ATTR_OBJECT_CLASS);
    assert_int_equal(pReplMetaData->pMetaData->localUsn, 100);
    assert_int_equal(pReplMetaData->pMetaData->version, 1);
    assert_string_equal(
            pReplMetaData->pMetaData->pszOrigInvoId, "7ef77c0f-cff1-4239-b293-39a2b302d5bd");
    assert_string_equal(pReplMetaData->pMetaData->pszOrigTime, "20180702222545.584");
    assert_int_equal(pReplMetaData->pMetaData->origUsn, 100);

    //Verify Attr value is as expected in pIndividualUpdate
    pAttr = VmDirFindAttrByName(pIndividualUpdate->pEntry, ATTR_USN_CHANGED);
    assert_non_null(pAttr);
    assert_string_equal(pAttr->type.lberbv_val, ATTR_USN_CHANGED);
    assert_string_equal(pAttr->vals[0].lberbv_val, "101");

    pAttr = VmDirFindAttrByName(pIndividualUpdate->pEntry, ATTR_OBJECT_GUID);
    assert_non_null(pAttr);
    assert_string_equal(pAttr->type.lberbv_val, ATTR_OBJECT_GUID);
    assert_string_equal(pAttr->vals[0].lberbv_val, "e7f6eae8-9902-4270-91ee-1ab36c898580");

    //Verify Attr value is as expected in pCombinedUdpate
    pAttr = VmDirFindAttrByName(pCombinedUpdate->pEntry, ATTR_DN);
    assert_non_null(pAttr);
    assert_string_equal(pAttr->type.lberbv_val, ATTR_DN);
    assert_string_equal(pAttr->vals[0].lberbv_val, "cn=newuser,cn=users,dc=lw-testdom,dc=com");
    assert_non_null(pAttr->next);

    pAttr = VmDirFindAttrByName(pCombinedUpdate->pEntry, ATTR_OBJECT_CLASS);
    assert_non_null(pAttr);
    assert_string_equal(pAttr->type.lberbv_val, ATTR_OBJECT_CLASS);
    assert_string_equal(pAttr->vals[0].lberbv_val, OC_DELETED_OBJECT);

    VmDirFreeReplUpdate(pIndividualUpdate);
}
