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

////////////////////////////
// Test Context definitions
////////////////////////////

typedef struct _VMDIR_PARSE_SYNC_DONE_CTL_TEST_CONTEXT
{
    LDAPControl*                      pSearchResCtrl;
    PVMDIR_REPLICATION_UPDATE_LIST    pReplUpdateList;
} VMDIR_PARSE_SYNC_DONE_CTL_TEST_CONTEXT, *PVMDIR_PARSE_SYNC_DONE_CTL_TEST_CONTEXT;

typedef struct _VMDIR_UPDATE_LIST_TEST_CONTEXT
{
    PVMDIR_REPLICATION_UPDATE_LIST    pReplUpdateList;
    PVMDIR_REPLICATION_UPDATE_LIST    pExpectedReplUpdateList;
} VMDIR_UPDATE_LIST_TEST_CONTEXT, *PVMDIR_UPDATE_LIST_TEST_CONTEXT;

int
VmDirTestSetupReplUpdateListParseSyncDoneCtl(
    VOID    **state
    )
{
    DWORD                                      dwError = 0;
    LDAPControl*                               pSearchResCtrl;
    PVMDIR_REPLICATION_UPDATE_LIST             pReplUpdateList = NULL;
    PVMDIR_PARSE_SYNC_DONE_CTL_TEST_CONTEXT    pParseSyncDoneCtlTestContext = NULL;

    dwError = VmDirAllocateMemory(
            sizeof(VMDIR_PARSE_SYNC_DONE_CTL_TEST_CONTEXT),
            (PVOID*)&pParseSyncDoneCtlTestContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(sizeof(LDAPControl), (PVOID*)&pSearchResCtrl);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                &pSearchResCtrl->ldctl_value.bv_val,
                "12345,7ef77c0f-cff1-4239-b293-39a2b302d5bd:12345,8ef77c0f-cff1-4239-b293-39a2b302d5bd:12346,9ef77c0f-cff1-4239-b293-39a2b302d5bd:12347");
    BAIL_ON_VMDIR_ERROR(dwError);

    pSearchResCtrl->ldctl_value.bv_len = VmDirStringLenA(pSearchResCtrl->ldctl_value.bv_val);

    dwError = VmDirReplUpdateListAlloc(&pReplUpdateList);
    BAIL_ON_VMDIR_ERROR(dwError);

    pParseSyncDoneCtlTestContext->pSearchResCtrl = pSearchResCtrl;
    pParseSyncDoneCtlTestContext->pReplUpdateList = pReplUpdateList;

    *state = pParseSyncDoneCtlTestContext;

cleanup:
    return dwError;

error:
    print_message("%s failed with error %d", __FUNCTION__, dwError);
    goto cleanup;
}

VOID
VmDirTestReplUpdateListParseSyncDoneCtl(
    VOID    **state
    )
{
    DWORD                                      dwError = 0;
    PVMDIR_PARSE_SYNC_DONE_CTL_TEST_CONTEXT    pParseSyncDoneCtlTestContext = NULL;

    pParseSyncDoneCtlTestContext = (PVMDIR_PARSE_SYNC_DONE_CTL_TEST_CONTEXT) *state;

    dwError = VmDirReplUpdateListParseSyncDoneCtl(
            pParseSyncDoneCtlTestContext->pReplUpdateList,
            &pParseSyncDoneCtlTestContext->pSearchResCtrl
            );
    assert_int_equal(dwError, 0);

    assert_int_equal(12345, pParseSyncDoneCtlTestContext->pReplUpdateList->newHighWaterMark);
    assert_string_equal(
                "7ef77c0f-cff1-4239-b293-39a2b302d5bd:12345,8ef77c0f-cff1-4239-b293-39a2b302d5bd:12346,9ef77c0f-cff1-4239-b293-39a2b302d5bd:12347",
                pParseSyncDoneCtlTestContext->pReplUpdateList->pNewUtdVector->pszUtdVector
                );
}

int
VmDirTestTeardownReplUpdateListParseSyncDoneCtl(
    VOID    **state
    )
{
    PVMDIR_PARSE_SYNC_DONE_CTL_TEST_CONTEXT    pParseSyncDoneCtlTestContext = NULL;

    pParseSyncDoneCtlTestContext = (PVMDIR_PARSE_SYNC_DONE_CTL_TEST_CONTEXT) *state;

    VmDirFreeReplUpdateList(pParseSyncDoneCtlTestContext->pReplUpdateList);
    VMDIR_SAFE_FREE_MEMORY(pParseSyncDoneCtlTestContext->pSearchResCtrl);

    VMDIR_SAFE_FREE_MEMORY(pParseSyncDoneCtlTestContext);

    return 0;
}

/*
 * TestCase 1:
 *     Input:
 *         - USN: 100 Add Entry
 *         - USN: 101 Modify add attr
 *         - USN: 102 Modify del attr
 *     Expected:
 *         - USN: 100 Add Entry
 *         - USN: 102 Modify del attr (only attrMetaData)
 */
int
VmDirTestSetupUpdateListExpand_AddWithDelAttr(
    VOID    **state
    )
{
    DWORD                              dwError = 0;
    PVMDIR_REPLICATION_UPDATE          pIndividualUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE          pCombinedUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE_LIST     pReplUpdateList = NULL;
    PVMDIR_REPLICATION_UPDATE_LIST     pExpectedReplUpdateList = NULL;
    PVMDIR_UPDATE_LIST_TEST_CONTEXT    pUpdateListTestContext = NULL;

    //Input
    dwError = VmDirReplUpdateListAlloc(&pReplUpdateList);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pReplUpdateList);

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pCombinedUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pCombinedUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pCombinedUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pCombinedUpdate->pEntry);

    pCombinedUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pCombinedUpdate->syncState = LDAP_SYNC_ADD;

    dwError = VmDirLinkedListCreate(&pCombinedUpdate->pMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pCombinedUpdate->pValueMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateAttrAndMetaData(
            ATTR_CN,
            "newuser_cn",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CREATED,
            "100",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID,
            "f7f6eae8-9902-4270-91ee-1ab36c898580",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CHANGED,
            "102",
            102,
            100,
            2,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);

    VmDirAllocateAttrMetaData(
            ATTR_SN,
            102,
            102,
            2,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);

    dwError = VmDirLinkedListInsertHead(pReplUpdateList->pLinkedList, pCombinedUpdate, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    //Expected (USN: 100 - Add)
    dwError = VmDirReplUpdateListAlloc(&pExpectedReplUpdateList);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pExpectedReplUpdateList);

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pIndividualUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pIndividualUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate->pEntry);

    pIndividualUpdate->partnerUsn = 100;
    pIndividualUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pIndividualUpdate->syncState = LDAP_SYNC_ADD;

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pValueMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateAttrAndMetaData(
            ATTR_CN,
            "newuser_cn",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID,
            "f7f6eae8-9902-4270-91ee-1ab36c898580",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CREATED,
            "100",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CHANGED,
            "100",
            100,
            100,
            2,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);

    dwError = VmDirLinkedListInsertHead(
            pExpectedReplUpdateList->pLinkedList, pIndividualUpdate, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);
    pIndividualUpdate = NULL;

    //Expected (USN: 102 - Modify)
    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pIndividualUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pIndividualUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate->pEntry);

    pIndividualUpdate->partnerUsn = 102;
    pIndividualUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pIndividualUpdate->syncState = LDAP_SYNC_MODIFY;

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pValueMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateAttrMetaData(
            ATTR_SN,
            102,
            102,
            2,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID,
            "f7f6eae8-9902-4270-91ee-1ab36c898580",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CHANGED,
            "102",
            102,
            100,
            2,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);

    dwError = VmDirLinkedListInsertTail(
            pExpectedReplUpdateList->pLinkedList, pIndividualUpdate, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);
    pIndividualUpdate = NULL;

    //UpateListTestContext
    dwError = VmDirAllocateMemory(
            sizeof(PVMDIR_UPDATE_LIST_TEST_CONTEXT), (PVOID*)&pUpdateListTestContext);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pUpdateListTestContext);

    pUpdateListTestContext->pReplUpdateList = pReplUpdateList;
    pUpdateListTestContext->pExpectedReplUpdateList = pExpectedReplUpdateList;

    *state = pUpdateListTestContext;

cleanup:
    return dwError;

error:
    print_message("%s failed with error %d", __FUNCTION__, dwError);
    goto cleanup;
}

/*
 * TestCase 2:
 *     Input:
 *         - USN: 100 Add Entry
 *         - USN: 101 Modify add multi value attr (values A, B, C)
 *         - USN: 102 Modify add value (A)
 *         - USN: 103 Modify del value (D)
 *     Expected:
 *         - USN: 100 Add Entry
 *         - USN: 101 Modify add multi value attr (values: B, C, D)
 *         - USN: 102 Modify add value (A)
 *         - USN: 103 Modify del value (D)
 */
int
VmDirTestSetupUpdateListExpand_AddWithMultiValueAttr(
    VOID    **state
    )
{
    DWORD                              dwError = 0;
    VDIR_BERVARRAY                     pbvVals = NULL;
    PVMDIR_REPLICATION_UPDATE          pIndividualUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE          pCombinedUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE_LIST     pReplUpdateList = NULL;
    PVMDIR_REPLICATION_UPDATE_LIST     pExpectedReplUpdateList = NULL;
    PVMDIR_UPDATE_LIST_TEST_CONTEXT    pUpdateListTestContext = NULL;

    //Input
    dwError = VmDirReplUpdateListAlloc(&pReplUpdateList);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pReplUpdateList);

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pCombinedUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pCombinedUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pCombinedUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pCombinedUpdate->pEntry);

    pCombinedUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pCombinedUpdate->syncState = LDAP_SYNC_ADD;

    dwError = VmDirLinkedListCreate(&pCombinedUpdate->pMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pCombinedUpdate->pValueMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateAttrAndMetaData(
            ATTR_CN,
            "newuser_cn",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CREATED,
            "100",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID,
            "f7f6eae8-9902-4270-91ee-1ab36c898580",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_BERVALUE) * (4), (PVOID*)&pbvVals);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToBervalContent(
            "cn=administrator@test-20002,cn=Users,dc=test-20002", &pbvVals[0]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToBervalContent(
            "cn=administrator@test-20004,cn=Users,dc=test-20004", &pbvVals[1]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToBervalContent(
            "cn=administrator@test-20005,cn=Users,dc=test-20005", &pbvVals[2]);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateMultiValueAttr(
            ATTR_MEMBER,
            101,
            101,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pbvVals,
            3,
            pCombinedUpdate);

    //add value metadata
    VmDirAllocateAttrValueMetaData("member:102:1:de62357c-e8b1-4532-9c81-91c4f58c3248:de62357c-e8b1-4532-9c81-91c4f58c3248:20180726170334.296:620972:0:50:cn=administrator@test-20002,cn=Users,dc=test-20002", pCombinedUpdate);

    //delete value metadata
    VmDirAllocateAttrValueMetaData("member:103:1:de62357c-e8b1-4532-9c81-91c4f58c3248:de62357c-e8b1-4532-9c81-91c4f58c3248:20180726170334.296:620972:1:50:cn=administrator@test-20003,cn=Users,dc=test-20003", pCombinedUpdate);

    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CHANGED,
            "103",
            103,
            100,
            4,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);

    dwError = VmDirLinkedListInsertHead(pReplUpdateList->pLinkedList, pCombinedUpdate, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    //Expected (USN: 100 - Add)
    dwError = VmDirReplUpdateListAlloc(&pExpectedReplUpdateList);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pExpectedReplUpdateList);

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pIndividualUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pIndividualUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate->pEntry);

    pIndividualUpdate->partnerUsn = 100;
    pIndividualUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pIndividualUpdate->syncState = LDAP_SYNC_ADD;

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pValueMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateAttrAndMetaData(
            ATTR_CN,
            "newuser_cn",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID,
            "f7f6eae8-9902-4270-91ee-1ab36c898580",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CREATED,
            "100",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CHANGED,
            "100",
            100,
            100,
            4,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);

    dwError = VmDirLinkedListInsertHead(
            pExpectedReplUpdateList->pLinkedList, pIndividualUpdate, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);
    pIndividualUpdate = NULL;

    //Expected (USN: 101 - Modify add multi value attr)
    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pIndividualUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pIndividualUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate->pEntry);

    pIndividualUpdate->partnerUsn = 101;
    pIndividualUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pIndividualUpdate->syncState = LDAP_SYNC_MODIFY;

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pValueMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID,
            "f7f6eae8-9902-4270-91ee-1ab36c898580",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CHANGED,
            "101",
            101,
            100,
            4,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_BERVALUE) * (4), (PVOID*)&pbvVals);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToBervalContent(
            "cn=administrator@test-20004,cn=Users,dc=test-20004", &pbvVals[0]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToBervalContent(
            "cn=administrator@test-20005,cn=Users,dc=test-20005", &pbvVals[1]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToBervalContent(
            "cn=administrator@test-20003,cn=Users,dc=test-20003", &pbvVals[2]);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateMultiValueAttr(
            ATTR_MEMBER,
            101,
            101,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pbvVals,
            3,
            pIndividualUpdate);

    dwError = VmDirLinkedListInsertTail(
            pExpectedReplUpdateList->pLinkedList, pIndividualUpdate, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);
    pIndividualUpdate = NULL;

    //Expected (USN: 102 Modify add value)
    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pIndividualUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pIndividualUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate->pEntry);

    pIndividualUpdate->partnerUsn = 102;
    pIndividualUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pIndividualUpdate->syncState = LDAP_SYNC_MODIFY;

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pValueMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID,
            "f7f6eae8-9902-4270-91ee-1ab36c898580",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CHANGED,
            "102",
            102,
            100,
            4,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);

    //add value metadata
    VmDirAllocateAttrValueMetaData("member:102:1:de62357c-e8b1-4532-9c81-91c4f58c3248:de62357c-e8b1-4532-9c81-91c4f58c3248:20180726170334.296:620972:0:50:cn=administrator@test-20002,cn=Users,dc=test-20002", pIndividualUpdate);

    dwError = VmDirLinkedListInsertTail(
            pExpectedReplUpdateList->pLinkedList, pIndividualUpdate, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);
    pIndividualUpdate = NULL;

    //Expected (USN: 103 Modify del value)
    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pIndividualUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pIndividualUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate->pEntry);

    pIndividualUpdate->partnerUsn = 103;
    pIndividualUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pIndividualUpdate->syncState = LDAP_SYNC_MODIFY;

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pValueMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID,
            "f7f6eae8-9902-4270-91ee-1ab36c898580",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CHANGED,
            "103",
            103,
            100,
            4,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);

    //del value metadata
    VmDirAllocateAttrValueMetaData("member:103:1:de62357c-e8b1-4532-9c81-91c4f58c3248:de62357c-e8b1-4532-9c81-91c4f58c3248:20180726170334.296:620972:1:50:cn=administrator@test-20003,cn=Users,dc=test-20003", pIndividualUpdate);

    dwError = VmDirLinkedListInsertTail(
            pExpectedReplUpdateList->pLinkedList, pIndividualUpdate, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);
    pIndividualUpdate = NULL;

    //UpateListTestContext
    dwError = VmDirAllocateMemory(
            sizeof(PVMDIR_UPDATE_LIST_TEST_CONTEXT), (PVOID*)&pUpdateListTestContext);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pUpdateListTestContext);

    pUpdateListTestContext->pReplUpdateList = pReplUpdateList;
    pUpdateListTestContext->pExpectedReplUpdateList = pExpectedReplUpdateList;

    *state = pUpdateListTestContext;

cleanup:
    return dwError;

error:
    print_message("%s failed with error %d", __FUNCTION__, dwError);
    goto cleanup;
}

/*
 * TestCase 3:
 *     Input:
 *         - USN: 100 Add Entry
 *         - USN: 101 Add one value to multi value attr
 *         - USN: 102 Del value from multi value attr
 *         (Special case where we have only attr metadata and value metadata for delete)
 *     Expected:
 *         - USN: 100 Add Entry
 *         - USN: 101 Add one value to multi value attr
 *         - USN: 102 Del value from multi value attr
 */
int
VmDirTestSetupUpdateListExpand_AddWithDelValue(
    VOID    **state
    )
{
    DWORD                              dwError = 0;
    VDIR_BERVARRAY                     pbvVals = NULL;
    PVMDIR_REPLICATION_UPDATE          pIndividualUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE          pCombinedUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE_LIST     pReplUpdateList = NULL;
    PVMDIR_REPLICATION_UPDATE_LIST     pExpectedReplUpdateList = NULL;
    PVMDIR_UPDATE_LIST_TEST_CONTEXT    pUpdateListTestContext = NULL;

    //Input
    dwError = VmDirReplUpdateListAlloc(&pReplUpdateList);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pReplUpdateList);

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pCombinedUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pCombinedUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pCombinedUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pCombinedUpdate->pEntry);

    pCombinedUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pCombinedUpdate->syncState = LDAP_SYNC_ADD;

    dwError = VmDirLinkedListCreate(&pCombinedUpdate->pMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pCombinedUpdate->pValueMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateAttrAndMetaData(
            ATTR_CN,
            "newuser_cn",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CREATED,
            "100",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID,
            "f7f6eae8-9902-4270-91ee-1ab36c898580",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_BERVALUE) * (1), (PVOID*)&pbvVals);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateMultiValueAttr(
            ATTR_MEMBER,
            101,
            101,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pbvVals,
            0,
            pCombinedUpdate);

    //delete value metadata
    VmDirAllocateAttrValueMetaData("member:102:1:de62357c-e8b1-4532-9c81-91c4f58c3248:de62357c-e8b1-4532-9c81-91c4f58c3248:20180726170334.296:620972:1:50:cn=administrator@test-20003,cn=Users,dc=test-20003", pCombinedUpdate);

    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CHANGED,
            "102",
            102,
            100,
            3,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);

    dwError = VmDirLinkedListInsertHead(pReplUpdateList->pLinkedList, pCombinedUpdate, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    //Expected (USN: 100 - Add)
    dwError = VmDirReplUpdateListAlloc(&pExpectedReplUpdateList);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pExpectedReplUpdateList);

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pIndividualUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pIndividualUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate->pEntry);

    pIndividualUpdate->partnerUsn = 100;
    pIndividualUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pIndividualUpdate->syncState = LDAP_SYNC_ADD;

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pValueMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateAttrAndMetaData(
            ATTR_CN,
            "newuser_cn",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID,
            "f7f6eae8-9902-4270-91ee-1ab36c898580",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CREATED,
            "100",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CHANGED,
            "100",
            100,
            100,
            3,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);

    dwError = VmDirLinkedListInsertHead(
            pExpectedReplUpdateList->pLinkedList, pIndividualUpdate, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);
    pIndividualUpdate = NULL;

    //Expected (USN: 101 - Modify add multi value attr)
    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pIndividualUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pIndividualUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate->pEntry);

    pIndividualUpdate->partnerUsn = 101;
    pIndividualUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pIndividualUpdate->syncState = LDAP_SYNC_MODIFY;

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pValueMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID,
            "f7f6eae8-9902-4270-91ee-1ab36c898580",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CHANGED,
            "101",
            101,
            100,
            3,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_BERVALUE) * (2), (PVOID*)&pbvVals);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToBervalContent(
            "cn=administrator@test-20003,cn=Users,dc=test-20003", &pbvVals[0]);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateMultiValueAttr(
            ATTR_MEMBER,
            101,
            101,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pbvVals,
            1,
            pIndividualUpdate);

    dwError = VmDirLinkedListInsertTail(
            pExpectedReplUpdateList->pLinkedList, pIndividualUpdate, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);
    pIndividualUpdate = NULL;

    //Expected (USN: 102 Modify del value)
    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pIndividualUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pIndividualUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate->pEntry);

    pIndividualUpdate->partnerUsn = 102;
    pIndividualUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pIndividualUpdate->syncState = LDAP_SYNC_MODIFY;

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pValueMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID,
            "f7f6eae8-9902-4270-91ee-1ab36c898580",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CHANGED,
            "102",
            102,
            100,
            3,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);

    //del value metadata
    VmDirAllocateAttrValueMetaData("member:102:1:de62357c-e8b1-4532-9c81-91c4f58c3248:de62357c-e8b1-4532-9c81-91c4f58c3248:20180726170334.296:620972:1:50:cn=administrator@test-20003,cn=Users,dc=test-20003", pIndividualUpdate);

    dwError = VmDirLinkedListInsertTail(
            pExpectedReplUpdateList->pLinkedList, pIndividualUpdate, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);
    pIndividualUpdate = NULL;

    //UpateListTestContext
    dwError = VmDirAllocateMemory(
            sizeof(PVMDIR_UPDATE_LIST_TEST_CONTEXT), (PVOID*)&pUpdateListTestContext);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pUpdateListTestContext);

    pUpdateListTestContext->pReplUpdateList = pReplUpdateList;
    pUpdateListTestContext->pExpectedReplUpdateList = pExpectedReplUpdateList;

    *state = pUpdateListTestContext;

cleanup:
    return dwError;

error:
    print_message("%s failed with error %d", __FUNCTION__, dwError);
    goto cleanup;
}

/*
 * TestCase 4:
 *     Input:
 *         - USN: 100 Add Entry
 *         - USN: 101 Modify add must attr 'CN'
 *     Expected:
 *         - USN: 100 Add Entry with must attr 'CN'
 *         - USN: 101 Modify add must attr 'CN'
 */
int
VmDirTestSetupUpdateListExpand_AddWithMustAttr(
    VOID    **state
    )
{
    DWORD                              dwError = 0;
    PVMDIR_REPLICATION_UPDATE          pIndividualUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE          pCombinedUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE_LIST     pReplUpdateList = NULL;
    PVMDIR_REPLICATION_UPDATE_LIST     pExpectedReplUpdateList = NULL;
    PVMDIR_UPDATE_LIST_TEST_CONTEXT    pUpdateListTestContext = NULL;

    //Input
    dwError = VmDirReplUpdateListAlloc(&pReplUpdateList);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pReplUpdateList);

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pCombinedUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pCombinedUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pCombinedUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pCombinedUpdate->pEntry);

    pCombinedUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pCombinedUpdate->syncState = LDAP_SYNC_ADD;

    dwError = VmDirLinkedListCreate(&pCombinedUpdate->pMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pCombinedUpdate->pValueMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateAttrAndMetaData(
            ATTR_CN,
            "newuser_cn",
            101,
            101,
            2,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CREATED,
            "100",
            100,
            100,
            1,
            "e7f6eae8-9902-4270-91ee-1ab36c898580",
            pCombinedUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID,
            "f7f6eae8-9902-4270-91ee-1ab36c898580",
            100,
            100,
            1,
            "e7f6eae8-9902-4270-91ee-1ab36c898580",
            pCombinedUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CHANGED,
            "101",
            101,
            100,
            2,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);

    dwError = VmDirLinkedListInsertHead(pReplUpdateList->pLinkedList, pCombinedUpdate, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    //Expected (USN: 100 - Add)
    dwError = VmDirReplUpdateListAlloc(&pExpectedReplUpdateList);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pExpectedReplUpdateList);

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pIndividualUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pIndividualUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate->pEntry);

    pIndividualUpdate->partnerUsn = 100;
    pIndividualUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pIndividualUpdate->syncState = LDAP_SYNC_ADD;

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pValueMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateAttrAndMetaData(
            ATTR_CN,
            "newuser_cn",
            100,
            100,
            1,
            "e7f6eae8-9902-4270-91ee-1ab36c898580",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID,
            "f7f6eae8-9902-4270-91ee-1ab36c898580",
            100,
            100,
            1,
            "e7f6eae8-9902-4270-91ee-1ab36c898580",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CREATED,
            "100",
            100,
            100,
            1,
            "e7f6eae8-9902-4270-91ee-1ab36c898580",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CHANGED,
            "100",
            100,
            100,
            2,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);

    dwError = VmDirLinkedListInsertHead(
            pExpectedReplUpdateList->pLinkedList, pIndividualUpdate, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);
    pIndividualUpdate = NULL;

    //Expected (USN: 102 - Modify)
    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pIndividualUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pIndividualUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate->pEntry);

    pIndividualUpdate->partnerUsn = 101;
    pIndividualUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pIndividualUpdate->syncState = LDAP_SYNC_MODIFY;

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pValueMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateAttrAndMetaData(
            ATTR_CN,
            "newuser_cn",
            101,
            101,
            2,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID,
            "f7f6eae8-9902-4270-91ee-1ab36c898580",
            100,
            100,
            1,
            "e7f6eae8-9902-4270-91ee-1ab36c898580",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CHANGED,
            "101",
            101,
            100,
            2,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);

    dwError = VmDirLinkedListInsertTail(
            pExpectedReplUpdateList->pLinkedList, pIndividualUpdate, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);
    pIndividualUpdate = NULL;

    //UpateListTestContext
    dwError = VmDirAllocateMemory(
            sizeof(PVMDIR_UPDATE_LIST_TEST_CONTEXT), (PVOID*)&pUpdateListTestContext);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pUpdateListTestContext);

    pUpdateListTestContext->pReplUpdateList = pReplUpdateList;
    pUpdateListTestContext->pExpectedReplUpdateList = pExpectedReplUpdateList;

    *state = pUpdateListTestContext;

cleanup:
    return dwError;

error:
    print_message("%s failed with error %d", __FUNCTION__, dwError);
    goto cleanup;
}

/*
 * TestCase 5:
 *     Input:
 *         - USN: 100 Add Entry
 *         - USN: 101 Modify add must attr 'CN'
 *         - USN: 102 Modify del must attr 'CN'
 *     Output:
 *         - USN: 100 Add Entry
 *         - USN: 102 add attr metadata of must attr 'CN'
 */
int
VmDirTestSetupUpdateListExpand_AddWithDelMustAttr(
    VOID    **state
    )
{
    DWORD                              dwError = 0;
    PVMDIR_REPLICATION_UPDATE          pIndividualUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE          pCombinedUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE_LIST     pReplUpdateList = NULL;
    PVMDIR_REPLICATION_UPDATE_LIST     pExpectedReplUpdateList = NULL;
    PVMDIR_UPDATE_LIST_TEST_CONTEXT    pUpdateListTestContext = NULL;

    //Input
    dwError = VmDirReplUpdateListAlloc(&pReplUpdateList);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pReplUpdateList);

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pCombinedUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pCombinedUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pCombinedUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pCombinedUpdate->pEntry);

    pCombinedUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pCombinedUpdate->syncState = LDAP_SYNC_ADD;

    dwError = VmDirLinkedListCreate(&pCombinedUpdate->pMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pCombinedUpdate->pValueMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateAttrAndMetaData(
            ATTR_SN,
            "newuser_sn",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CREATED,
            "100",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID,
            "f7f6eae8-9902-4270-91ee-1ab36c898580",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CHANGED,
            "102",
            102,
            100,
            2,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);

    VmDirAllocateAttrMetaData(
            ATTR_CN, 102, 102, 2, "7ef77c0f-cff1-4239-b293-39a2b302d5bd", pCombinedUpdate);

    dwError = VmDirLinkedListInsertHead(pReplUpdateList->pLinkedList, pCombinedUpdate, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    //Expected (USN: 100 - Add)
    dwError = VmDirReplUpdateListAlloc(&pExpectedReplUpdateList);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pExpectedReplUpdateList);

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pIndividualUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pIndividualUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate->pEntry);

    pIndividualUpdate->partnerUsn = 100;
    pIndividualUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pIndividualUpdate->syncState = LDAP_SYNC_ADD;

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pValueMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateAttrAndMetaData(
            ATTR_SN,
            "newuser_sn",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID,
            "f7f6eae8-9902-4270-91ee-1ab36c898580",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CREATED,
            "100",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CHANGED,
            "100",
            100,
            100,
            2,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);

    dwError = VmDirLinkedListInsertHead(
            pExpectedReplUpdateList->pLinkedList, pIndividualUpdate, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);
    pIndividualUpdate = NULL;

    //Expected (USN: 102 - Modify)
    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pIndividualUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pIndividualUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate->pEntry);

    pIndividualUpdate->partnerUsn = 102;
    pIndividualUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pIndividualUpdate->syncState = LDAP_SYNC_MODIFY;

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pValueMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateAttrMetaData(
            ATTR_CN, 102, 102, 2, "7ef77c0f-cff1-4239-b293-39a2b302d5bd", pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID,
            "f7f6eae8-9902-4270-91ee-1ab36c898580",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CHANGED,
            "102",
            102,
            100,
            2,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);

    dwError = VmDirLinkedListInsertTail(
            pExpectedReplUpdateList->pLinkedList, pIndividualUpdate, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);
    pIndividualUpdate = NULL;

    //UpateListTestContext
    dwError = VmDirAllocateMemory(
            sizeof(PVMDIR_UPDATE_LIST_TEST_CONTEXT), (PVOID*)&pUpdateListTestContext);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pUpdateListTestContext);

    pUpdateListTestContext->pReplUpdateList = pReplUpdateList;
    pUpdateListTestContext->pExpectedReplUpdateList = pExpectedReplUpdateList;

    *state = pUpdateListTestContext;

cleanup:
    return dwError;

error:
    print_message("%s failed with error %d", __FUNCTION__, dwError);
    goto cleanup;
}

/*
 * TestCase 6:
 *     Input:
 *         - USN: 100 Add Tombstone Entry
 *     Expected:
 *         - USN: 100 Add Entry
 *         - USN: 101 Delete Entry
 */
int
VmDirTestSetupUpdateListExpand_AddTombstone(
    VOID    **state
    )
{
    DWORD                              dwError = 0;
    PVMDIR_REPLICATION_UPDATE          pIndividualUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE          pCombinedUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE_LIST     pReplUpdateList = NULL;
    PVMDIR_REPLICATION_UPDATE_LIST     pExpectedReplUpdateList = NULL;
    PVMDIR_UPDATE_LIST_TEST_CONTEXT    pUpdateListTestContext = NULL;

    //Input
    dwError = VmDirReplUpdateListAlloc(&pReplUpdateList);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pReplUpdateList);

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pCombinedUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pCombinedUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pCombinedUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pCombinedUpdate->pEntry);

    dwError = VmDirStringToBervalContent(
            "cn=newuser#objectGUID:f7f6eae8-9902-4270-91ee-1ab36c898580,cn=deleted objects,dc=lw,dc=local", &pCombinedUpdate->pEntry->dn);
    BAIL_ON_VMDIR_ERROR(dwError);

    pCombinedUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pCombinedUpdate->syncState = LDAP_SYNC_ADD;

    dwError = VmDirLinkedListCreate(&pCombinedUpdate->pMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pCombinedUpdate->pValueMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateAttrAndMetaData(
            ATTR_CN,
            "newuser_cn",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CREATED,
            "100",
            100,
            100,
            1,
            "e7f6eae8-9902-4270-91ee-1ab36c898580",
            pCombinedUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID,
            "f7f6eae8-9902-4270-91ee-1ab36c898580",
            100,
            100,
            1,
            "e7f6eae8-9902-4270-91ee-1ab36c898580",
            pCombinedUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CHANGED,
            "101",
            101,
            100,
            2,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_LAST_KNOWN_DN,
            "cn=newuser,cn=users,dc=lw,dc=local",
            101,
            101,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_CLASS,
            "deletedObject",
            101,
            101,
            2,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_IS_DELETED,
            "true",
            101,
            101,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);

    dwError = VmDirLinkedListInsertHead(pReplUpdateList->pLinkedList, pCombinedUpdate, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    //Expected (USN: 100 - Add)
    dwError = VmDirReplUpdateListAlloc(&pExpectedReplUpdateList);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pExpectedReplUpdateList);

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pIndividualUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pIndividualUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate->pEntry);

    pIndividualUpdate->partnerUsn = 100;
    pIndividualUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pIndividualUpdate->syncState = LDAP_SYNC_ADD;

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pValueMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToBervalContent(
            "cn=newuser,cn=users,dc=lw,dc=local", &pIndividualUpdate->pEntry->dn);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateAttrAndMetaData(
            ATTR_CN,
            "newuser_cn",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID,
            "f7f6eae8-9902-4270-91ee-1ab36c898580",
            100,
            100,
            1,
            "e7f6eae8-9902-4270-91ee-1ab36c898580",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CREATED,
            "100",
            100,
            100,
            1,
            "e7f6eae8-9902-4270-91ee-1ab36c898580",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CHANGED,
            "100",
            100,
            100,
            2,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_DN,
            "cn=newuser,cn=users,dc=lw,dc=local",
            100,
            100,
            1,
            "e7f6eae8-9902-4270-91ee-1ab36c898580",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_CLASS,
            "deletedObject",
            100,
            100,
            1,
            "e7f6eae8-9902-4270-91ee-1ab36c898580",
            pIndividualUpdate);

    dwError = VmDirLinkedListInsertHead(
            pExpectedReplUpdateList->pLinkedList, pIndividualUpdate, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);
    pIndividualUpdate = NULL;

    //Expected (USN: 101 - Delete)
    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pIndividualUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pIndividualUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate->pEntry);

    pIndividualUpdate->partnerUsn = 101;
    pIndividualUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pIndividualUpdate->syncState = LDAP_SYNC_DELETE;

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pValueMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToBervalContent(
            "cn=newuser#objectGUID:f7f6eae8-9902-4270-91ee-1ab36c898580,cn=deleted objects,dc=lw,dc=local", &pIndividualUpdate->pEntry->dn);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID,
            "f7f6eae8-9902-4270-91ee-1ab36c898580",
            100,
            100,
            1,
            "e7f6eae8-9902-4270-91ee-1ab36c898580",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CHANGED,
            "101",
            101,
            100,
            2,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_LAST_KNOWN_DN,
            "cn=newuser,cn=users,dc=lw,dc=local",
            101,
            101,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_CLASS,
            "deletedObject",
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

    dwError = VmDirLinkedListInsertTail(
            pExpectedReplUpdateList->pLinkedList, pIndividualUpdate, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);
    pIndividualUpdate = NULL;

    //UpateListTestContext
    dwError = VmDirAllocateMemory(
            sizeof(PVMDIR_UPDATE_LIST_TEST_CONTEXT), (PVOID*)&pUpdateListTestContext);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pUpdateListTestContext);

    pUpdateListTestContext->pReplUpdateList = pReplUpdateList;
    pUpdateListTestContext->pExpectedReplUpdateList = pExpectedReplUpdateList;

    *state = pUpdateListTestContext;

cleanup:
    return dwError;

error:
    print_message("%s failed with error %d", __FUNCTION__, dwError);
    goto cleanup;
}

/*
 * TestCase 7:
 *     Input:
 *         - USN: 100 Add Tombstone Entry
 *     Expected:
 *         - USN: 100 Add Entry
 *         - USN: 102 Modify add attr 'SN' MetaData
 *         - USN: 103 Delete Entry
 */
int
VmDirTestSetupUpdateListExpand_AddWithModifyAndTombstone(
    VOID    **state
    )
{
    DWORD                              dwError = 0;
    PVMDIR_REPLICATION_UPDATE          pIndividualUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE          pCombinedUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE_LIST     pReplUpdateList = NULL;
    PVMDIR_REPLICATION_UPDATE_LIST     pExpectedReplUpdateList = NULL;
    PVMDIR_UPDATE_LIST_TEST_CONTEXT    pUpdateListTestContext = NULL;

    //Input
    dwError = VmDirReplUpdateListAlloc(&pReplUpdateList);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pReplUpdateList);

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pCombinedUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pCombinedUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pCombinedUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pCombinedUpdate->pEntry);

    dwError = VmDirStringToBervalContent(
            "cn=newuser#objectGUID:f7f6eae8-9902-4270-91ee-1ab36c898580,cn=deleted objects,dc=lw,dc=local", &pCombinedUpdate->pEntry->dn);
    BAIL_ON_VMDIR_ERROR(dwError);

    pCombinedUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pCombinedUpdate->syncState = LDAP_SYNC_ADD;

    dwError = VmDirLinkedListCreate(&pCombinedUpdate->pMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pCombinedUpdate->pValueMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateAttrAndMetaData(
            ATTR_CN,
            "newuser_cn",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CREATED,
            "100",
            100,
            100,
            1,
            "e7f6eae8-9902-4270-91ee-1ab36c898580",
            pCombinedUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID,
            "f7f6eae8-9902-4270-91ee-1ab36c898580",
            100,
            100,
            1,
            "e7f6eae8-9902-4270-91ee-1ab36c898580",
            pCombinedUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CHANGED,
            "103",
            103,
            100,
            4,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_LAST_KNOWN_DN,
            "cn=newuser,cn=users,dc=lw,dc=local",
            103,
            103,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_CLASS,
            "deletedObject",
            103,
            103,
            2,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_IS_DELETED,
            "true",
            103,
            103,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pCombinedUpdate);
    VmDirAllocateAttrMetaData(
            ATTR_SN, 102, 102, 2, "7ef77c0f-cff1-4239-b293-39a2b302d5bd", pCombinedUpdate);

    dwError = VmDirLinkedListInsertHead(pReplUpdateList->pLinkedList, pCombinedUpdate, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    //Expected (USN: 100 - Add)
    dwError = VmDirReplUpdateListAlloc(&pExpectedReplUpdateList);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pExpectedReplUpdateList);

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pIndividualUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pIndividualUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate->pEntry);

    pIndividualUpdate->partnerUsn = 100;
    pIndividualUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pIndividualUpdate->syncState = LDAP_SYNC_ADD;

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pValueMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToBervalContent(
            "cn=newuser,cn=users,dc=lw,dc=local", &pIndividualUpdate->pEntry->dn);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateAttrAndMetaData(
            ATTR_CN,
            "newuser_cn",
            100,
            100,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID,
            "f7f6eae8-9902-4270-91ee-1ab36c898580",
            100,
            100,
            1,
            "e7f6eae8-9902-4270-91ee-1ab36c898580",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CREATED,
            "100",
            100,
            100,
            1,
            "e7f6eae8-9902-4270-91ee-1ab36c898580",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CHANGED,
            "100",
            100,
            100,
            4,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_DN,
            "cn=newuser,cn=users,dc=lw,dc=local",
            100,
            100,
            1,
            "e7f6eae8-9902-4270-91ee-1ab36c898580",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_CLASS,
            "deletedObject",
            100,
            100,
            1,
            "e7f6eae8-9902-4270-91ee-1ab36c898580",
            pIndividualUpdate);

    dwError = VmDirLinkedListInsertHead(
            pExpectedReplUpdateList->pLinkedList, pIndividualUpdate, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);
    pIndividualUpdate = NULL;

    //Expected (USN: 102 - Modify del attr)
    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pIndividualUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pIndividualUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate->pEntry);

    dwError = VmDirStringToBervalContent(
            "cn=newuser,cn=users,dc=lw,dc=local", &pIndividualUpdate->pEntry->dn);
    BAIL_ON_VMDIR_ERROR(dwError);

    pIndividualUpdate->partnerUsn = 102;
    pIndividualUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pIndividualUpdate->syncState = LDAP_SYNC_MODIFY;

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pValueMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID,
            "f7f6eae8-9902-4270-91ee-1ab36c898580",
            100,
            100,
            1,
            "e7f6eae8-9902-4270-91ee-1ab36c898580",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CHANGED,
            "102",
            102,
            100,
            4,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrMetaData(
            ATTR_SN,
            102,
            102,
            2,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);

    dwError = VmDirLinkedListInsertTail(
            pExpectedReplUpdateList->pLinkedList, pIndividualUpdate, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);
    pIndividualUpdate = NULL;

    //Expected (USN: 103 - Delete Entry)
    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pIndividualUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pIndividualUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pIndividualUpdate->pEntry);

    dwError = VmDirStringToBervalContent(
            "cn=newuser#objectGUID:f7f6eae8-9902-4270-91ee-1ab36c898580,cn=deleted objects,dc=lw,dc=local", &pIndividualUpdate->pEntry->dn);
    BAIL_ON_VMDIR_ERROR(dwError);

    pIndividualUpdate->partnerUsn = 103;
    pIndividualUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pIndividualUpdate->syncState = LDAP_SYNC_DELETE;

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pIndividualUpdate->pValueMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID,
            "f7f6eae8-9902-4270-91ee-1ab36c898580",
            100,
            100,
            1,
            "e7f6eae8-9902-4270-91ee-1ab36c898580",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_USN_CHANGED,
            "103",
            103,
            100,
            4,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_LAST_KNOWN_DN,
            "cn=newuser,cn=users,dc=lw,dc=local",
            103,
            103,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_CLASS,
            "deletedObject",
            103,
            103,
            2,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_IS_DELETED,
            "true",
            103,
            103,
            1,
            "7ef77c0f-cff1-4239-b293-39a2b302d5bd",
            pIndividualUpdate);

    dwError = VmDirLinkedListInsertTail(
            pExpectedReplUpdateList->pLinkedList, pIndividualUpdate, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);
    pIndividualUpdate = NULL;

    //UpateListTestContext
    dwError = VmDirAllocateMemory(
            sizeof(PVMDIR_UPDATE_LIST_TEST_CONTEXT), (PVOID*)&pUpdateListTestContext);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert_non_null(pUpdateListTestContext);

    pUpdateListTestContext->pReplUpdateList = pReplUpdateList;
    pUpdateListTestContext->pExpectedReplUpdateList = pExpectedReplUpdateList;

    *state = pUpdateListTestContext;

cleanup:
    return dwError;

error:
    print_message("%s failed with error %d", __FUNCTION__, dwError);
    goto cleanup;
}

VOID
VmDirTestReplUpdateListExpand(
    VOID    **state
    )
{
    DWORD                              dwError = 0;
    PVMDIR_UPDATE_LIST_TEST_CONTEXT    pUpdateListTestContext = NULL;

    pUpdateListTestContext = (PVMDIR_UPDATE_LIST_TEST_CONTEXT) *state;

    dwError = VmDirReplUpdateListExpand(pUpdateListTestContext->pReplUpdateList);
    assert_int_equal(dwError, 0);

    assert_true(
    VmDirTestCompareReplUpdateList(
    pUpdateListTestContext->pReplUpdateList,
    pUpdateListTestContext->pExpectedReplUpdateList));
}

int
VmDirTestTeardownUpdateListExpand(
    VOID    **state
    )
{
    PVMDIR_UPDATE_LIST_TEST_CONTEXT    pUpdateListTestContext = NULL;

    pUpdateListTestContext = (PVMDIR_UPDATE_LIST_TEST_CONTEXT) *state;

    VmDirFreeReplUpdateList(pUpdateListTestContext->pReplUpdateList);
    VmDirFreeReplUpdateList(pUpdateListTestContext->pExpectedReplUpdateList);

    VMDIR_SAFE_FREE_MEMORY(pUpdateListTestContext);

    return 0;
}
