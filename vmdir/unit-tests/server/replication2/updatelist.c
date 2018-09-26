#include "includes.h"

static
VOID
_VmDirReplUpdateListExpandVerifyAddEntry(
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate
    );

static
VOID
_VmDirReplUpdateListExpandVerifyAddAttr(
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate
    );

static
VOID
_VmDirReplUpdateListExpandVerifyDeleteAttr(
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate
    );

static
VOID
_VmDirReplUpdateListExpandVerifyMultiValueAttr(
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate
    );

static
VOID
_VmDirReplUpdateListExpandVerifyAddMultiValue(
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate
    );

static
VOID
_VmDirReplUpdateListExpandVerifyDeleteMultiValue(
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate
    );

static
VOID
_VmDirReplUpdateListExpandVerifyAddTombstone(
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate
    );

static
VOID
_VmDirReplUpdateListExpandVerifyOperationAttr(
    USN                          usn,
    UINT64                       version,
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate
    );

//setup and teardown function
int
VmDirSetupReplUpdateListTest(
    VOID    **state
    )
{
    DWORD                           dwError = 0;
    PVMDIR_REPLICATION_UPDATE_LIST  pUpdateList = NULL;

    dwError = VmDirReplUpdateListAlloc(&pUpdateList);
    assert_int_equal(dwError, 0);

    *state = pUpdateList;

    return 0;
}

int
VmDirSetupReplUpdateListExpand_ModifyTest(
    VOID    **state
    )
{
    DWORD                             dwError = 0;
    PVMDIR_REPLICATION_UPDATE         pUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE_LIST    pReplUpdateList = NULL;

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE_LIST), (PVOID*)&pReplUpdateList);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListCreate(&pReplUpdateList->pLinkedList);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pUpdate);
    assert_int_equal(dwError, 0);
    assert_non_null(pUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pUpdate->pEntry);
    assert_int_equal(dwError, 0);
    assert_non_null(pUpdate->pEntry);

    dwError = VmDirAllocateStringA("lw-test-node-1", &pUpdate->pszPartner);
    assert_int_equal(dwError, 0);

    dwError = VmDirStringToBervalContent("cn=group,dc=lw-testdom,dc=com", &pUpdate->pEntry->dn);
    assert_int_equal(dwError, 0);

    pUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;

    pUpdate->syncState = LDAP_SYNC_MODIFY;

    dwError = VmDirLinkedListCreate(&pUpdate->pMetaDataList);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListCreate(&pUpdate->pValueMetaDataList);
    assert_int_equal(dwError, 0);

    //add attribute
    VmDirAllocateAttrAndMetaData(ATTR_CN, "newuser_cn", 101, 101, 1, pUpdate);

    //delete attribute
    VmDirAllocateAttrMetaData(ATTR_DESCRIPTION, 102, 102, 2, pUpdate);

    //multi value attribute
    VmDirAllocateMultiValueAttr(ATTR_MEMBER, 103, 103, pUpdate);

    //add value metadata
    VmDirAllocateAttrValueMetaData("member:104:1:de62357c-e8b1-4532-9c81-91c4f58c3248:de62357c-e8b1-4532-9c81-91c4f58c3248:20180726170334.296:620972:0:50:cn=administrator@test-20002,cn=Users,dc=test-20002", pUpdate);

    //delete value metadata
    VmDirAllocateAttrValueMetaData("member:105:1:de62357c-e8b1-4532-9c81-91c4f58c3248:de62357c-e8b1-4532-9c81-91c4f58c3248:20180726170334.296:620972:1:52:cn=administrator@test-20003,cn=\0Users,dc=\0test-20003", pUpdate);

    //operational attribute
    VmDirAllocateAttrAndMetaData(ATTR_USN_CHANGED, "105", 105, 105, 6, pUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID, "e7f6eae8-9902-4270-91ee-1ab36c898580", 100, 100, 1, pUpdate);

    dwError = VmDirLinkedListInsertHead(pReplUpdateList->pLinkedList, pUpdate, NULL);
    assert_int_equal(dwError, 0);

    *state = pReplUpdateList;

    return 0;
}

int
VmDirSetupReplUpdateListExpand_AddTest(
    VOID    **state
    )
{
    DWORD                             dwError = 0;
    PVMDIR_REPLICATION_UPDATE         pUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE_LIST    pReplUpdateList = NULL;

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE_LIST), (PVOID*)&pReplUpdateList);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListCreate(&pReplUpdateList->pLinkedList);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pUpdate);
    assert_int_equal(dwError, 0);
    assert_non_null(pUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pUpdate->pEntry);
    assert_int_equal(dwError, 0);
    assert_non_null(pUpdate->pEntry);

    dwError = VmDirAllocateStringA("lw-test-node-1", &pUpdate->pszPartner);
    assert_int_equal(dwError, 0);

    dwError = VmDirStringToBervalContent("cn=group,dc=lw-testdom,dc=com", &pUpdate->pEntry->dn);
    assert_int_equal(dwError, 0);

    pUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;

    pUpdate->syncState = LDAP_SYNC_ADD;

    dwError = VmDirLinkedListCreate(&pUpdate->pMetaDataList);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListCreate(&pUpdate->pValueMetaDataList);
    assert_int_equal(dwError, 0);

    //attributes added as part of Add
    VmDirAllocateAttrAndMetaData(ATTR_USN_CREATED, "100", 100, 100, 1, pUpdate);

    //add attribute
    VmDirAllocateAttrAndMetaData(ATTR_CN, "newuser_cn", 101, 101, 1, pUpdate);

    //delete attribute
    VmDirAllocateAttrMetaData(ATTR_DESCRIPTION, 102, 102, 2, pUpdate);

    //multi value attribute
    VmDirAllocateMultiValueAttr(ATTR_MEMBER, 103, 103, pUpdate);

    //add value metadata
    VmDirAllocateAttrValueMetaData("member:104:1:de62357c-e8b1-4532-9c81-91c4f58c3248:de62357c-e8b1-4532-9c81-91c4f58c3248:20180726170334.296:620972:0:50:cn=administrator@test-20002,cn=Users,dc=test-20002", pUpdate);

    //delete value metadata
    VmDirAllocateAttrValueMetaData("member:105:1:de62357c-e8b1-4532-9c81-91c4f58c3248:de62357c-e8b1-4532-9c81-91c4f58c3248:20180726170334.296:620972:1:52:cn=administrator@test-20003,cn=\0Users,dc=\0test-20003", pUpdate);

    //operational attribute
    VmDirAllocateAttrAndMetaData(ATTR_USN_CHANGED, "105", 105, 105, 6, pUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID, "e7f6eae8-9902-4270-91ee-1ab36c898580", 100, 100, 1, pUpdate);

    dwError = VmDirLinkedListInsertHead(pReplUpdateList->pLinkedList, pUpdate, NULL);
    assert_int_equal(dwError, 0);

    *state = pReplUpdateList;

    return 0;
}

int
VmDirSetupReplUpdateListExpand_DeleteTest(
    VOID    **state
    )
{
    DWORD                             dwError = 0;
    PVMDIR_REPLICATION_UPDATE         pUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE_LIST    pReplUpdateList = NULL;

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE_LIST), (PVOID*)&pReplUpdateList);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListCreate(&pReplUpdateList->pLinkedList);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pUpdate);
    assert_int_equal(dwError, 0);
    assert_non_null(pUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pUpdate->pEntry);
    assert_int_equal(dwError, 0);
    assert_non_null(pUpdate->pEntry);

    dwError = VmDirAllocateStringA("lw-test-node-1", &pUpdate->pszPartner);
    assert_int_equal(dwError, 0);

    dwError = VmDirStringToBervalContent("cn=group,dc=lw-testdom,dc=com", &pUpdate->pEntry->dn);
    assert_int_equal(dwError, 0);

    pUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;

    pUpdate->syncState = LDAP_SYNC_DELETE;

    dwError = VmDirLinkedListCreate(&pUpdate->pMetaDataList);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListCreate(&pUpdate->pValueMetaDataList);
    assert_int_equal(dwError, 0);

    //delete attribute
    VmDirAllocateAttrMetaData(ATTR_DESCRIPTION, 102, 102, 2, pUpdate);

    //Indicates entry is deleted
    VmDirAllocateAttrAndMetaData(ATTR_IS_DELETED, "true", 103, 103, 1, pUpdate);

    //operational attribute
    VmDirAllocateAttrAndMetaData(ATTR_USN_CHANGED, "105", 103, 103, 3, pUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID, "e7f6eae8-9902-4270-91ee-1ab36c898580", 100, 100, 1, pUpdate);

    dwError = VmDirLinkedListInsertHead(pReplUpdateList->pLinkedList, pUpdate, NULL);
    assert_int_equal(dwError, 0);

    *state = pReplUpdateList;

    return 0;
}

int
VmDirSetupReplUpdateListExpand_AddTombstoneTest(
    VOID    **state
    )
{
    DWORD                             dwError = 0;
    PVMDIR_REPLICATION_UPDATE         pUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE_LIST    pReplUpdateList = NULL;

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE_LIST), (PVOID*)&pReplUpdateList);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListCreate(&pReplUpdateList->pLinkedList);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pUpdate);
    assert_int_equal(dwError, 0);
    assert_non_null(pUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pUpdate->pEntry);
    assert_int_equal(dwError, 0);
    assert_non_null(pUpdate->pEntry);

    dwError = VmDirAllocateStringA("lw-test-node-1", &pUpdate->pszPartner);
    assert_int_equal(dwError, 0);

    dwError = VmDirStringToBervalContent("cn=group,dc=lw-testdom,dc=com", &pUpdate->pEntry->dn);
    assert_int_equal(dwError, 0);

    pUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;

    pUpdate->syncState = LDAP_SYNC_ADD;

    dwError = VmDirLinkedListCreate(&pUpdate->pMetaDataList);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListCreate(&pUpdate->pValueMetaDataList);
    assert_int_equal(dwError, 0);

    //attributes added as part of Add
    VmDirAllocateAttrAndMetaData(ATTR_USN_CREATED, "100", 100, 100, 1, pUpdate);

    //delete attribute
    VmDirAllocateAttrMetaData(ATTR_DESCRIPTION, 102, 102, 2, pUpdate);

    VmDirAllocateAttrAndMetaData(
            ATTR_LAST_KNOWN_DN,
            "cn=group,dc=lw-testdom,dc=com",
            103,
            103,
            1,
            pUpdate);

    //Indicates entry is deleted
    VmDirAllocateAttrAndMetaData(ATTR_IS_DELETED, "true", 103, 103, 1, pUpdate);

    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_CLASS, OC_DELETED_OBJECT, 103, 103, 2, pUpdate);

    //operational attribute
    VmDirAllocateAttrAndMetaData(ATTR_USN_CHANGED, "103", 103, 103, 3, pUpdate);
    VmDirAllocateAttrAndMetaData(
            ATTR_OBJECT_GUID, "e7f6eae8-9902-4270-91ee-1ab36c898580", 100, 100, 1, pUpdate);

    dwError = VmDirLinkedListInsertHead(pReplUpdateList->pLinkedList, pUpdate, NULL);
    assert_int_equal(dwError, 0);

    *state = pReplUpdateList;

    return 0;
}

int
VmDirTeardownReplUpdateListTest(
    VOID    **state
    )
{
    VmDirFreeReplUpdateList((PVMDIR_REPLICATION_UPDATE_LIST)*state);

    return 0;
}

VOID
VmDirReplUpdateListParseSyncDoneCtl_ValidInput(
    VOID    **state
    )
{
    DWORD                           dwError = 0;
    PVMDIR_REPLICATION_UPDATE_LIST  pReplUpdate = NULL;
    LDAPControl*                    pSearchResCtrl = NULL;

    pReplUpdate = *state;
    dwError = VmDirAllocateMemory(sizeof(LDAPControl), (PVOID*)&pSearchResCtrl);
    assert_int_equal(dwError, 0);

    dwError = VmDirAllocateStringPrintf(
                &pSearchResCtrl->ldctl_value.bv_val,
                "12345,7ef77c0f-cff1-4239-b293-39a2b302d5bd:12345,8ef77c0f-cff1-4239-b293-39a2b302d5bd:12346,9ef77c0f-cff1-4239-b293-39a2b302d5bd:12347"
                );
    assert_int_equal(dwError, 0);

    pSearchResCtrl->ldctl_value.bv_len = VmDirStringLenA(pSearchResCtrl->ldctl_value.bv_val);

    dwError = VmDirReplUpdateListParseSyncDoneCtl(pReplUpdate, &pSearchResCtrl);
    assert_int_equal(dwError, 0);

    assert_int_equal(12345, pReplUpdate->newHighWaterMark);
    assert_string_equal(
                "7ef77c0f-cff1-4239-b293-39a2b302d5bd:12345,8ef77c0f-cff1-4239-b293-39a2b302d5bd:12346,9ef77c0f-cff1-4239-b293-39a2b302d5bd:12347",
                pReplUpdate->pNewUtdVector->pszUtdVector
                );
}

/*
 * ReplUpdateListExpand unit test functions
 * Input:
 *     pReplUpdateList->pList (SyncState: Add)
 *         - USN: 100 ATTR: USN_CREATED OBJECT_GUID
 *         - USN: 101 ATTR: CN (Attr Add)
 *         - USN: 102 ATTR: DESCRPTION (Attr Delete)
 *         - USN: 103 ATTR: MEMBER OPERAITON: ADD (Multi value attribute)
 *         - USN: 104 OPERATION: Add new value
 *         - USN: 105 OPERATION: Delete value ATTR: USN_CHANGED
 * Expected outcome:
 *     pReplUpdateList->pList
 *         - USN: 100 ATTR: USN_CREATED, CN (must attrs), OBJECT_GUID, USN_CHANGED (SyncState: Add)
 *         - USN: 101 ATTR: CN ATTR: USN_CHANGED, OBJECT_GUID (SyncState: Modify)
 *         - USN: 102 ATTR: DESCRPTION (only metadata) ATTR: USN_CHANGED,
 *                          OBJECT_GUID (SyncState: Modify)
 *         - USN: 103 ATTR: MEMBER OPERAITON: ADD (Multi value) ATTR: USN_CHANGED, OBJECT_GUID
 *                    (SyncState: Modify)
 *           Values:
 *               - cn=administrator@test-20003,cn=\0Users,dc=test-\020003
 *               - cn=administrator@test-20004,cn=Users,dc=test-20004
 *               - cn=administrator@test-20005,cn=Users,dc=test-20005
 *         - USN: 104 OPERATION: Add new value ATTR: USN_CHANGED, OBJECT_GUID
 *                    (SyncState: Modify)
 *           Values:
 *               - cn=administrator@test-20002,cn=Users,dc=test-20002
 *         - USN: 105 OPERATION: Delete value ATTR: USN_CHANGED, OBJECT_GUID
 *                    (SyncState: Modify)
 *           Values:
 *               - cn=administrator@test-20003,cn=\0Users,dc=test-\020003
 */
VOID
VmDirReplUpdateListExpand_AddTest(
    VOID    **state
    )
{
    DWORD                             dwError = 0;
    PVDIR_LINKED_LIST_NODE            pNode = NULL;
    PVMDIR_REPLICATION_UPDATE         pIndividualUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE_LIST    pReplUpdateList = NULL;

    pReplUpdateList = *state;

    //Contents of pReplUpdateList will be freed in the teardown function
    dwError = VmDirReplUpdateListExpand(pReplUpdateList);
    assert_int_equal(dwError, 0);

    /*
     * pReplUpdateList->pLinkedList should contains changes in ascending order
     * Verify USN: 100 pIndividualUpdate - Add Entry
     */
    dwError = VmDirLinkedListGetHead(pReplUpdateList->pLinkedList, &pNode);
    assert_int_equal(dwError, 0);

    pIndividualUpdate = pNode->pElement;
    pNode = pNode->pNext;

    _VmDirReplUpdateListExpandVerifyAddEntry(pIndividualUpdate);
    _VmDirReplUpdateListExpandVerifyOperationAttr(100, 6, pIndividualUpdate);

    //Verify USN: 101 pIndividualUpdate - Add Attribute
    pIndividualUpdate = pNode->pElement;
    pNode = pNode->pNext;

    _VmDirReplUpdateListExpandVerifyAddAttr(pIndividualUpdate);
    _VmDirReplUpdateListExpandVerifyOperationAttr(101, 6, pIndividualUpdate);

    //Verify USN: 102 pIndividualUpdate - Delete Attribute
    pIndividualUpdate = pNode->pElement;
    pNode = pNode->pNext;

    _VmDirReplUpdateListExpandVerifyDeleteAttr(pIndividualUpdate);
    _VmDirReplUpdateListExpandVerifyOperationAttr(102, 6, pIndividualUpdate);

    //Verify USN: 103 pIndividualUpdate - Add MultiValue Attribute
    pIndividualUpdate = pNode->pElement;
    pNode = pNode->pNext;

    _VmDirReplUpdateListExpandVerifyMultiValueAttr(pIndividualUpdate);
    _VmDirReplUpdateListExpandVerifyOperationAttr(103, 6, pIndividualUpdate);

    //Verify USN: 104 pIndividualUpdate - Add MultiValue
    pIndividualUpdate = pNode->pElement;
    pNode = pNode->pNext;

    _VmDirReplUpdateListExpandVerifyAddMultiValue(pIndividualUpdate);
    _VmDirReplUpdateListExpandVerifyOperationAttr(104, 6, pIndividualUpdate);

    //Verify USN: 105 pIndividualUpdate - Delete MultiValue
    pIndividualUpdate = pNode->pElement;
    pNode = pNode->pNext;

    _VmDirReplUpdateListExpandVerifyDeleteMultiValue(pIndividualUpdate);
    _VmDirReplUpdateListExpandVerifyOperationAttr(105, 6, pIndividualUpdate);

    assert_null(pNode);
}

/*
 * ReplUpdateListExpand unit test functions
 * Input:
 *     pReplUpdateList->pList (SyncState: Modify)
 *         - USN: 101 ATTR: CN (Attr Add)
 *         - USN: 102 ATTR: DESCRPTION (Attr Delete)
 *         - USN: 103 ATTR: MEMBER OPERAITON: ADD (Multi value attribute)
 *         - USN: 104 OPERATION: Add new value
 *         - USN: 105 OPERATION: Delete value, USN_CHANGED
 *           (OBJECT_GUID for all changes)
 * Expected outcome:
 *     pReplUpdateList->pList
 *         - USN: 101 ATTR: CN ATTR: USN_CHANGED, OBJECT_GUID (SyncState: Modify)
 *         - USN: 102 ATTR: DESCRPTION (only metadata) ATTR: USN_CHANGED,
 *                          OBJECT_GUID (SyncState: Modify)
 *         - USN: 103 ATTR: MEMBER OPERAITON: ADD (Multi value) ATTR: USN_CHANGED, OBJECT_GUID
 *                    (SyncState: Modify)
 *           Values:
 *               - cn=administrator@test-20003,cn=\0Users,dc=test-\020003
 *               - cn=administrator@test-20004,cn=Users,dc=test-20004
 *               - cn=administrator@test-20005,cn=Users,dc=test-20005
 *         - USN: 104 OPERATION: Add new value ATTR: USN_CHANGED, OBJECT_GUID
 *                    (SyncState: Modify)
 *           Values:
 *               - cn=administrator@test-20002,cn=Users,dc=test-20002
 *         - USN: 105 OPERATION: Delete value ATTR: USN_CHANGED, OBJECT_GUID
 *                    (SyncState: Modify)
 *           Values:
 *               - cn=administrator@test-20003,cn=\0Users,dc=test-\020003
 */
VOID
VmDirReplUpdateListExpand_ModifyTest(
    VOID    **state
    )
{
    DWORD                             dwError = 0;
    PVDIR_LINKED_LIST_NODE            pNode = NULL;
    PVMDIR_REPLICATION_UPDATE         pIndividualUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE_LIST    pReplUpdateList = NULL;

    pReplUpdateList = *state;

    //Contents of pReplUpdateList will be freed in the teardown function
    dwError = VmDirReplUpdateListExpand(pReplUpdateList);
    assert_int_equal(dwError, 0);

    /*
     * pReplUpdateList->pLinkedList should contains changes in ascending order
     * Verify USN: 101 pIndividualUpdate - Add Attribute
     */
    dwError = VmDirLinkedListGetHead(pReplUpdateList->pLinkedList, &pNode);
    assert_int_equal(dwError, 0);

    pIndividualUpdate = pNode->pElement;
    pNode = pNode->pNext;

    _VmDirReplUpdateListExpandVerifyAddAttr(pIndividualUpdate);
    _VmDirReplUpdateListExpandVerifyOperationAttr(101, 6, pIndividualUpdate);

    //Verify USN: 102 pIndividualUpdate - Delete Attribute
    pIndividualUpdate = pNode->pElement;
    pNode = pNode->pNext;

    _VmDirReplUpdateListExpandVerifyDeleteAttr(pIndividualUpdate);
    _VmDirReplUpdateListExpandVerifyOperationAttr(102, 6, pIndividualUpdate);

    //Verify USN: 103 pIndividualUpdate - Add MultiValue Attribute
    pIndividualUpdate = pNode->pElement;
    pNode = pNode->pNext;

    _VmDirReplUpdateListExpandVerifyMultiValueAttr(pIndividualUpdate);
    _VmDirReplUpdateListExpandVerifyOperationAttr(103, 6, pIndividualUpdate);

    //Verify USN: 104 pIndividualUpdate - Add MultiValue
    pIndividualUpdate = pNode->pElement;
    pNode = pNode->pNext;

    _VmDirReplUpdateListExpandVerifyAddMultiValue(pIndividualUpdate);
    _VmDirReplUpdateListExpandVerifyOperationAttr(104, 6, pIndividualUpdate);

    //Verify USN: 105 pIndividualUpdate - Delete MultiValue
    pIndividualUpdate = pNode->pElement;
    pNode = pNode->pNext;

    _VmDirReplUpdateListExpandVerifyDeleteMultiValue(pIndividualUpdate);
    _VmDirReplUpdateListExpandVerifyOperationAttr(105, 6, pIndividualUpdate);

    assert_null(pNode);
}

/*
 * ReplUpdateListExpand unit test functions
 * Input:
 *     pReplUpdateList->pList (SyncState: Delete)
 *         - USN: 102 ATTR: DESCRPTION (Attr Delete)
 *         - USN: 103 (Delete Entry)
 *           USN: 103 ATTR: IS_DELETED, USN_CHANGED
 *           (OBJECT_GUID with all changes)
 * Expected outcome:
 *     pReplUpdateList->pList
 *         - USN: 102 ATTR: DESCRPTION ATTR: USN_CHANGED, OBJECT_GUID (SyncState: Modify)
 *         - USN: 103 ATTR: USN_CHANGED, OBJECT_GUID, IS_DELETED (SyncState: Delete)
 */
VOID
VmDirReplUpdateListExpand_DeleteTest(
    VOID    **state
    )
{
    DWORD                             dwError = 0;
    PVDIR_ATTRIBUTE                   pAttr = NULL;
    PVDIR_LINKED_LIST_NODE            pNode = NULL;
    PVMDIR_REPLICATION_UPDATE         pIndividualUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE_LIST    pReplUpdateList = NULL;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pReplMetaData = NULL;

    pReplUpdateList = *state;

    //Contents of pReplUpdateList will be freed in the teardown function
    dwError = VmDirReplUpdateListExpand(pReplUpdateList);
    assert_int_equal(dwError, 0);

    /*
     * pReplUpdateList->pLinkedList should contains changes in ascending order
     * Verify USN: 102 pIndividualUpdate - delete attribute
     */
    dwError = VmDirLinkedListGetHead(pReplUpdateList->pLinkedList, &pNode);
    assert_int_equal(dwError, 0);

    pIndividualUpdate = pNode->pElement;
    pNode = pNode->pNext;

    _VmDirReplUpdateListExpandVerifyDeleteAttr(pIndividualUpdate);
    _VmDirReplUpdateListExpandVerifyOperationAttr(102, 3, pIndividualUpdate);

    //Verify USN: 105 pIndividualUpdate - entry delete
    pIndividualUpdate = pNode->pElement;
    pNode = pNode->pNext;

    assert_int_equal(pIndividualUpdate->syncState, LDAP_SYNC_DELETE);
    assert_int_equal(pIndividualUpdate->partnerUsn, 103);
    assert_string_equal(pIndividualUpdate->pszPartner, "lw-test-node-1");
    assert_string_equal(pIndividualUpdate->pEntry->dn.lberbv_val, "cn=group,dc=lw-testdom,dc=com");

    pReplMetaData = VmDirFindAttrMetaData(pIndividualUpdate->pMetaDataList, ATTR_IS_DELETED);
    assert_non_null(pReplMetaData);

    assert_string_equal(pReplMetaData->pszAttrType, ATTR_IS_DELETED);
    assert_int_equal(pReplMetaData->pMetaData->localUsn, 103);
    assert_int_equal(pReplMetaData->pMetaData->version, 1);
    assert_string_equal(
            pReplMetaData->pMetaData->pszOrigInvoId, "7ef77c0f-cff1-4239-b293-39a2b302d5bd");
    assert_string_equal(pReplMetaData->pMetaData->pszOrigTime, "20180702222545.584");
    assert_int_equal(pReplMetaData->pMetaData->origUsn, 103);

    pAttr = VmDirFindAttrByName(pIndividualUpdate->pEntry, ATTR_IS_DELETED);
    assert_non_null(pAttr);
    assert_string_equal(pAttr->type.lberbv_val, ATTR_IS_DELETED);
    assert_string_equal(pAttr->vals[0].lberbv_val, "true");

    //Verify Operation attributes
    _VmDirReplUpdateListExpandVerifyOperationAttr(103, 3, pIndividualUpdate);
}

/*
 * ReplUpdateListExpand unit test functions
 * Input:
 *     pReplUpdateList->pList (SyncState: Add)
 *         - USN: 100 ATTR: USN_CREATED, OBJECT_GUID
 *         - USN: 102 ATTR: DESCRPTION (Attr Delete)
 *         - USN: 103 (Delete Entry)
 *           USN: 103 ATTR: LAST_KNOWN_DN, IS_DELETED, OBJECT_CLASS, USN_CHANGED
 * Expected outcome:
 *     pReplUpdateList->pList
 *         - USN: 100 ATTR: USN_CREATED, OBJECT_CLASS,
 *                          ENTRY_DN, USN_CHANGED, OBJECT_GUID (SyncState: Add)
 *         - USN: 102 ATTR: DESCRPTION ATTR: USN_CHANGED, OBJECT_GUID (SyncState: Modify)
 *         - USN: 103 ATTR: USN_CHANGED, OBJECT_GUID, IS_DELETED,
 *                          LAST_KNOWN_DN, OBJECT_CLASS (SyncState: Delete)
 */
VOID
VmDirReplUpdateListExpand_AddTombstoneTest(
    VOID    **state
    )
{
    DWORD                             dwError = 0;
    PVDIR_ATTRIBUTE                   pAttr = NULL;
    PVDIR_LINKED_LIST_NODE            pNode = NULL;
    PVMDIR_REPLICATION_UPDATE         pIndividualUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE_LIST    pReplUpdateList = NULL;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pReplMetaData = NULL;

    pReplUpdateList = *state;

    //Contents of pReplUpdateList will be freed in the teardown function
    dwError = VmDirReplUpdateListExpand(pReplUpdateList);
    assert_int_equal(dwError, 0);

    /*
     * pReplUpdateList->pLinkedList should contains changes in ascending order
     * Verify USN: 100 pIndividualUpdate - add entry
     */
    dwError = VmDirLinkedListGetHead(pReplUpdateList->pLinkedList, &pNode);
    assert_int_equal(dwError, 0);

    pIndividualUpdate = pNode->pElement;
    pNode = pNode->pNext;

    _VmDirReplUpdateListExpandVerifyAddTombstone(pIndividualUpdate);
    _VmDirReplUpdateListExpandVerifyOperationAttr(100, 3, pIndividualUpdate);

    pIndividualUpdate = pNode->pElement;
    pNode = pNode->pNext;

    _VmDirReplUpdateListExpandVerifyDeleteAttr(pIndividualUpdate);
    _VmDirReplUpdateListExpandVerifyOperationAttr(102, 3, pIndividualUpdate);

    //Verify USN: 103 pIndividualUpdate - entry delete
    pIndividualUpdate = pNode->pElement;
    pNode = pNode->pNext;

    assert_int_equal(pIndividualUpdate->syncState, LDAP_SYNC_DELETE);
    assert_int_equal(pIndividualUpdate->partnerUsn, 103);
    assert_string_equal(pIndividualUpdate->pszPartner, "lw-test-node-1");
    assert_string_equal(pIndividualUpdate->pEntry->dn.lberbv_val, "cn=group,dc=lw-testdom,dc=com");

    pReplMetaData = VmDirFindAttrMetaData(pIndividualUpdate->pMetaDataList, ATTR_OBJECT_CLASS);
    assert_non_null(pReplMetaData);

    assert_string_equal(pReplMetaData->pszAttrType, ATTR_OBJECT_CLASS);
    assert_int_equal(pReplMetaData->pMetaData->localUsn, 103);
    assert_int_equal(pReplMetaData->pMetaData->version, 2);
    assert_string_equal(
            pReplMetaData->pMetaData->pszOrigInvoId, "7ef77c0f-cff1-4239-b293-39a2b302d5bd");
    assert_string_equal(pReplMetaData->pMetaData->pszOrigTime, "20180702222545.584");
    assert_int_equal(pReplMetaData->pMetaData->origUsn, 103);

    pReplMetaData = VmDirFindAttrMetaData(pIndividualUpdate->pMetaDataList, ATTR_LAST_KNOWN_DN);
    assert_non_null(pReplMetaData);

    assert_string_equal(pReplMetaData->pszAttrType, ATTR_LAST_KNOWN_DN);
    assert_int_equal(pReplMetaData->pMetaData->localUsn, 103);
    assert_int_equal(pReplMetaData->pMetaData->version, 1);
    assert_string_equal(
            pReplMetaData->pMetaData->pszOrigInvoId, "7ef77c0f-cff1-4239-b293-39a2b302d5bd");
    assert_string_equal(pReplMetaData->pMetaData->pszOrigTime, "20180702222545.584");
    assert_int_equal(pReplMetaData->pMetaData->origUsn, 103);

    pReplMetaData = VmDirFindAttrMetaData(pIndividualUpdate->pMetaDataList, ATTR_IS_DELETED);
    assert_non_null(pReplMetaData);

    assert_string_equal(pReplMetaData->pszAttrType, ATTR_IS_DELETED);
    assert_int_equal(pReplMetaData->pMetaData->localUsn, 103);
    assert_int_equal(pReplMetaData->pMetaData->version, 1);
    assert_string_equal(
            pReplMetaData->pMetaData->pszOrigInvoId, "7ef77c0f-cff1-4239-b293-39a2b302d5bd");
    assert_string_equal(pReplMetaData->pMetaData->pszOrigTime, "20180702222545.584");
    assert_int_equal(pReplMetaData->pMetaData->origUsn, 103);

    pAttr = VmDirFindAttrByName(pIndividualUpdate->pEntry, ATTR_OBJECT_CLASS);
    assert_non_null(pAttr);
    assert_string_equal(pAttr->type.lberbv_val, ATTR_OBJECT_CLASS);
    assert_string_equal(pAttr->vals[0].lberbv_val, OC_DELETED_OBJECT);

    pAttr = VmDirFindAttrByName(pIndividualUpdate->pEntry, ATTR_LAST_KNOWN_DN);
    assert_non_null(pAttr);
    assert_string_equal(pAttr->type.lberbv_val, ATTR_LAST_KNOWN_DN);
    assert_string_equal(pAttr->vals[0].lberbv_val, "cn=group,dc=lw-testdom,dc=com");

    pAttr = VmDirFindAttrByName(pIndividualUpdate->pEntry, ATTR_IS_DELETED);
    assert_non_null(pAttr);
    assert_string_equal(pAttr->type.lberbv_val, ATTR_IS_DELETED);
    assert_string_equal(pAttr->vals[0].lberbv_val, "true");

    //Verify Operation attributes
    _VmDirReplUpdateListExpandVerifyOperationAttr(103, 3, pIndividualUpdate);
}

static
VOID
_VmDirReplUpdateListExpandVerifyAddEntry(
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate
    )
{
    USN                               localUsn = 0;
    DWORD                             dwError = 0;
    PVDIR_ATTRIBUTE                   pAttr = NULL;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pReplMetaData = NULL;

    assert_int_equal(pIndividualUpdate->syncState, LDAP_SYNC_ADD);
    assert_int_equal(pIndividualUpdate->partnerUsn, 100);
    assert_string_equal(pIndividualUpdate->pszPartner, "lw-test-node-1");
    assert_string_equal(pIndividualUpdate->pEntry->dn.lberbv_val, "cn=group,dc=lw-testdom,dc=com");

    pReplMetaData = VmDirFindAttrMetaData(pIndividualUpdate->pMetaDataList, ATTR_USN_CREATED);
    assert_non_null(pReplMetaData);

    assert_string_equal(pReplMetaData->pszAttrType, ATTR_USN_CREATED);
    assert_int_equal(pReplMetaData->pMetaData->localUsn, 100);
    assert_int_equal(pReplMetaData->pMetaData->version, 1);
    assert_string_equal(
            pReplMetaData->pMetaData->pszOrigInvoId, "7ef77c0f-cff1-4239-b293-39a2b302d5bd");
    assert_string_equal(pReplMetaData->pMetaData->pszOrigTime, "20180702222545.584");
    assert_int_equal(pReplMetaData->pMetaData->origUsn, 100);

    pReplMetaData = VmDirFindAttrMetaData(pIndividualUpdate->pMetaDataList, ATTR_CN);
    assert_non_null(pReplMetaData);

    assert_string_equal(pReplMetaData->pszAttrType, ATTR_CN);
    assert_int_equal(pReplMetaData->pMetaData->localUsn, 100);
    assert_int_equal(pReplMetaData->pMetaData->version, 1);
    assert_string_equal(
            pReplMetaData->pMetaData->pszOrigInvoId, "7ef77c0f-cff1-4239-b293-39a2b302d5bd");
    assert_string_equal(pReplMetaData->pMetaData->pszOrigTime, "20180702222545.584");
    assert_int_equal(pReplMetaData->pMetaData->origUsn, 100);

    pAttr = VmDirFindAttrByName(pIndividualUpdate->pEntry, ATTR_USN_CREATED);
    assert_non_null(pAttr);
    assert_string_equal(pAttr->type.lberbv_val, ATTR_USN_CREATED);
    dwError = VmDirStringToUINT64(pAttr->vals[0].lberbv_val, NULL, &localUsn);
    assert_int_equal(dwError, 0);
    assert_int_equal(localUsn, 100);

    pAttr = VmDirFindAttrByName(pIndividualUpdate->pEntry, ATTR_CN);
    assert_non_null(pAttr);
    assert_string_equal(pAttr->type.lberbv_val, ATTR_CN);
    assert_string_equal(pAttr->vals[0].lberbv_val, "newuser_cn");
}

static
VOID
_VmDirReplUpdateListExpandVerifyAddAttr(
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate
    )
{
    PVDIR_ATTRIBUTE                   pAttr = NULL;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pReplMetaData = NULL;

    assert_int_equal(pIndividualUpdate->syncState, LDAP_SYNC_MODIFY);
    assert_int_equal(pIndividualUpdate->partnerUsn, 101);
    assert_string_equal(pIndividualUpdate->pszPartner, "lw-test-node-1");
    assert_string_equal(pIndividualUpdate->pEntry->dn.lberbv_val, "cn=group,dc=lw-testdom,dc=com");

    //Verify Attr MetaData is as expected
    pReplMetaData = VmDirFindAttrMetaData(pIndividualUpdate->pMetaDataList, ATTR_CN);
    assert_non_null(pReplMetaData);

    assert_string_equal(pReplMetaData->pszAttrType, ATTR_CN);
    assert_int_equal(pReplMetaData->pMetaData->localUsn, 101);
    assert_int_equal(pReplMetaData->pMetaData->version, 1);
    assert_string_equal(
            pReplMetaData->pMetaData->pszOrigInvoId, "7ef77c0f-cff1-4239-b293-39a2b302d5bd");
    assert_string_equal(pReplMetaData->pMetaData->pszOrigTime, "20180702222545.584");
    assert_int_equal(pReplMetaData->pMetaData->origUsn, 101);

    //Verify Attr is as expected
    pAttr = VmDirFindAttrByName(pIndividualUpdate->pEntry, ATTR_CN);
    assert_non_null(pAttr);
    assert_string_equal(pAttr->type.lberbv_val, ATTR_CN);
    assert_string_equal(pAttr->vals[0].lberbv_val, "newuser_cn");

}

static
VOID
_VmDirReplUpdateListExpandVerifyDeleteAttr(
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate
    )
{
    PVDIR_ATTRIBUTE                   pAttr = NULL;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pReplMetaData = NULL;

    assert_int_equal(pIndividualUpdate->syncState, LDAP_SYNC_MODIFY);
    assert_int_equal(pIndividualUpdate->partnerUsn, 102);
    assert_string_equal(pIndividualUpdate->pszPartner, "lw-test-node-1");
    assert_string_equal(pIndividualUpdate->pEntry->dn.lberbv_val, "cn=group,dc=lw-testdom,dc=com");

    //Verify Attr MetaData is as expected
    pReplMetaData = VmDirFindAttrMetaData(pIndividualUpdate->pMetaDataList, ATTR_DESCRIPTION);
    assert_non_null(pReplMetaData);

    assert_string_equal(pReplMetaData->pszAttrType, ATTR_DESCRIPTION);
    assert_int_equal(pReplMetaData->pMetaData->localUsn, 102);
    assert_int_equal(pReplMetaData->pMetaData->version, 2);
    assert_string_equal(
            pReplMetaData->pMetaData->pszOrigInvoId, "7ef77c0f-cff1-4239-b293-39a2b302d5bd");
    assert_string_equal(pReplMetaData->pMetaData->pszOrigTime, "20180702222545.584");
    assert_int_equal(pReplMetaData->pMetaData->origUsn, 102);

    //Verify Attr is as expected
    pAttr = VmDirFindAttrByName(pIndividualUpdate->pEntry, ATTR_DESCRIPTION);
    assert_null(pAttr);
}

static
VOID
_VmDirReplUpdateListExpandVerifyMultiValueAttr(
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate
    )
{
    PVDIR_ATTRIBUTE                   pAttr = NULL;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pReplMetaData = NULL;

    assert_int_equal(pIndividualUpdate->syncState, LDAP_SYNC_MODIFY);
    assert_int_equal(pIndividualUpdate->partnerUsn, 103);
    assert_string_equal(pIndividualUpdate->pszPartner, "lw-test-node-1");
    assert_string_equal(pIndividualUpdate->pEntry->dn.lberbv_val, "cn=group,dc=lw-testdom,dc=com");

    //Verify Attr MetaData is as expected
    pReplMetaData = VmDirFindAttrMetaData(pIndividualUpdate->pMetaDataList, ATTR_MEMBER);
    assert_non_null(pReplMetaData);

    assert_string_equal(pReplMetaData->pszAttrType, ATTR_MEMBER);
    assert_int_equal(pReplMetaData->pMetaData->localUsn, 103);
    assert_int_equal(pReplMetaData->pMetaData->version, 1);
    assert_string_equal(
            pReplMetaData->pMetaData->pszOrigInvoId, "7ef77c0f-cff1-4239-b293-39a2b302d5bd");
    assert_string_equal(pReplMetaData->pMetaData->pszOrigTime, "20180702222545.584");
    assert_int_equal(pReplMetaData->pMetaData->origUsn, 103);

    //Verify Attr is as expected
    pAttr = VmDirFindAttrByName(pIndividualUpdate->pEntry, ATTR_MEMBER);
    assert_non_null(pAttr);
    assert_string_equal(pAttr->type.lberbv_val, ATTR_MEMBER);
    assert_int_equal(pAttr->numVals, 3);
    assert_memory_equal(
            pAttr->vals[0].lberbv_val, "cn=administrator@test-20004,cn=Users,dc=test-20004", 50);
    assert_memory_equal(
            pAttr->vals[1].lberbv_val, "cn=administrator@test-20005,cn=Users,dc=test-20005", 50);
    assert_memory_equal(
            pAttr->vals[2].lberbv_val, "cn=administrator@test-20003,cn=\0Users,dc=\0test-20003", 52);
}

static
VOID
_VmDirReplUpdateListExpandVerifyAddMultiValue(
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate
    )
{
    DWORD                              dwError = 0;
    VDIR_BERVALUE                      bvValueMetaData = VDIR_BERVALUE_INIT;
    PVDIR_LINKED_LIST_NODE             pNode = NULL;
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pValueMetaData = NULL;

    assert_int_equal(pIndividualUpdate->syncState, LDAP_SYNC_MODIFY);
    assert_int_equal(pIndividualUpdate->partnerUsn, 104);
    assert_string_equal(pIndividualUpdate->pszPartner, "lw-test-node-1");
    assert_string_equal(pIndividualUpdate->pEntry->dn.lberbv_val, "cn=group,dc=lw-testdom,dc=com");

    dwError = VmDirLinkedListGetHead(pIndividualUpdate->pValueMetaDataList, &pNode);
    assert_int_equal(dwError, 0);

    pValueMetaData = (PVMDIR_VALUE_ATTRIBUTE_METADATA) pNode->pElement;

    dwError = VmDirValueMetaDataSerialize(pValueMetaData, &bvValueMetaData);
    assert_int_equal(dwError, 0);

    //add value metadata
    assert_memory_equal(
           bvValueMetaData.lberbv_val,
           "member:104:1:de62357c-e8b1-4532-9c81-91c4f58c3248:de62357c-e8b1-4532-9c81-91c4f58c3248:20180726170334.296:620972:0:50:cn=administrator@test-20002,cn=Users,dc=test-20002",
           bvValueMetaData.lberbv_len);

    VmDirFreeBervalContent(&bvValueMetaData);
}

static
VOID
_VmDirReplUpdateListExpandVerifyDeleteMultiValue(
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate
    )
{
    DWORD                              dwError = 0;
    VDIR_BERVALUE                      bvValueMetaData = VDIR_BERVALUE_INIT;
    PVDIR_LINKED_LIST_NODE             pNode = NULL;
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pValueMetaData = NULL;

    assert_int_equal(pIndividualUpdate->syncState, LDAP_SYNC_MODIFY);
    assert_int_equal(pIndividualUpdate->partnerUsn, 105);
    assert_string_equal(pIndividualUpdate->pszPartner, "lw-test-node-1");
    assert_string_equal(pIndividualUpdate->pEntry->dn.lberbv_val, "cn=group,dc=lw-testdom,dc=com");

    dwError = VmDirLinkedListGetHead(pIndividualUpdate->pValueMetaDataList, &pNode);
    assert_int_equal(dwError, 0);

    pValueMetaData = (PVMDIR_VALUE_ATTRIBUTE_METADATA) pNode->pElement;

    dwError = VmDirValueMetaDataSerialize(pValueMetaData, &bvValueMetaData);
    assert_int_equal(dwError, 0);

    //add value metadata
    assert_memory_equal(
           bvValueMetaData.lberbv_val,
           "member:105:1:de62357c-e8b1-4532-9c81-91c4f58c3248:de62357c-e8b1-4532-9c81-91c4f58c3248:20180726170334.296:620972:1:52:cn=administrator@test-20003,cn=\0Users,dc=\0test-20003",
           bvValueMetaData.lberbv_len);

    VmDirFreeBervalContent(&bvValueMetaData);
}

static
VOID
_VmDirReplUpdateListExpandVerifyAddTombstone(
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate
    )
{
    PVDIR_ATTRIBUTE                   pAttr = NULL;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pReplMetaData = NULL;

    assert_int_equal(pIndividualUpdate->syncState, LDAP_SYNC_ADD);
    assert_int_equal(pIndividualUpdate->partnerUsn, 100);
    assert_string_equal(pIndividualUpdate->pszPartner, "lw-test-node-1");
    assert_string_equal(pIndividualUpdate->pEntry->dn.lberbv_val, "cn=group,dc=lw-testdom,dc=com");

    pReplMetaData = VmDirFindAttrMetaData(pIndividualUpdate->pMetaDataList, ATTR_USN_CREATED);
    assert_non_null(pReplMetaData);

    assert_string_equal(pReplMetaData->pszAttrType, ATTR_USN_CREATED);
    assert_int_equal(pReplMetaData->pMetaData->localUsn, 100);
    assert_int_equal(pReplMetaData->pMetaData->version, 1);
    assert_string_equal(
            pReplMetaData->pMetaData->pszOrigInvoId, "7ef77c0f-cff1-4239-b293-39a2b302d5bd");
    assert_string_equal(pReplMetaData->pMetaData->pszOrigTime, "20180702222545.584");
    assert_int_equal(pReplMetaData->pMetaData->origUsn, 100);

    pReplMetaData = VmDirFindAttrMetaData(pIndividualUpdate->pMetaDataList, ATTR_DN);
    assert_non_null(pReplMetaData);

    assert_string_equal(pReplMetaData->pszAttrType, ATTR_DN);
    assert_int_equal(pReplMetaData->pMetaData->localUsn, 100);
    assert_int_equal(pReplMetaData->pMetaData->version, 1);
    assert_string_equal(
            pReplMetaData->pMetaData->pszOrigInvoId, "7ef77c0f-cff1-4239-b293-39a2b302d5bd");
    assert_string_equal(pReplMetaData->pMetaData->pszOrigTime, "20180702222545.584");
    assert_int_equal(pReplMetaData->pMetaData->origUsn, 100);

    pReplMetaData = VmDirFindAttrMetaData(pIndividualUpdate->pMetaDataList, ATTR_OBJECT_CLASS);
    assert_non_null(pReplMetaData);

    assert_string_equal(pReplMetaData->pszAttrType, ATTR_OBJECT_CLASS);
    assert_int_equal(pReplMetaData->pMetaData->localUsn, 100);
    assert_int_equal(pReplMetaData->pMetaData->version, 1);
    assert_string_equal(
            pReplMetaData->pMetaData->pszOrigInvoId, "7ef77c0f-cff1-4239-b293-39a2b302d5bd");
    assert_string_equal(pReplMetaData->pMetaData->pszOrigTime, "20180702222545.584");
    assert_int_equal(pReplMetaData->pMetaData->origUsn, 100);

    pAttr = VmDirFindAttrByName(pIndividualUpdate->pEntry, ATTR_USN_CREATED);
    assert_non_null(pAttr);
    assert_string_equal(pAttr->type.lberbv_val, ATTR_USN_CREATED);
    assert_string_equal(pAttr->vals[0].lberbv_val, "100");

    pAttr = VmDirFindAttrByName(pIndividualUpdate->pEntry, ATTR_DN);
    assert_non_null(pAttr);
    assert_string_equal(pAttr->type.lberbv_val, ATTR_DN);
    assert_string_equal(pAttr->vals[0].lberbv_val, "cn=group,dc=lw-testdom,dc=com");

    pAttr = VmDirFindAttrByName(pIndividualUpdate->pEntry, ATTR_OBJECT_CLASS);
    assert_non_null(pAttr);
    assert_string_equal(pAttr->type.lberbv_val, ATTR_OBJECT_CLASS);
    assert_string_equal(pAttr->vals[0].lberbv_val, OC_DELETED_OBJECT);
}

static
VOID
_VmDirReplUpdateListExpandVerifyOperationAttr(
    USN                          usn,
    UINT64                       version,
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate
    )
{
    DWORD                             dwError = 0;
    USN                               localUsn = 0;
    PVDIR_ATTRIBUTE                   pAttr = NULL;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pReplMetaData = NULL;

    pReplMetaData = VmDirFindAttrMetaData(pIndividualUpdate->pMetaDataList, ATTR_USN_CHANGED);
    assert_non_null(pReplMetaData);

    assert_string_equal(pReplMetaData->pszAttrType, ATTR_USN_CHANGED);
    assert_int_equal(pReplMetaData->pMetaData->localUsn, usn);
    assert_int_equal(pReplMetaData->pMetaData->version, version);
    assert_string_equal(
            pReplMetaData->pMetaData->pszOrigInvoId, "7ef77c0f-cff1-4239-b293-39a2b302d5bd");
    assert_string_equal(pReplMetaData->pMetaData->pszOrigTime, "20180702222545.584");
    assert_int_equal(pReplMetaData->pMetaData->origUsn, usn);

    pReplMetaData = VmDirFindAttrMetaData(pIndividualUpdate->pMetaDataList, ATTR_OBJECT_GUID);
    assert_non_null(pReplMetaData);

    assert_string_equal(pReplMetaData->pszAttrType, ATTR_OBJECT_GUID);
    assert_int_equal(pReplMetaData->pMetaData->localUsn, 100);
    assert_int_equal(pReplMetaData->pMetaData->version, 1);
    assert_string_equal(
            pReplMetaData->pMetaData->pszOrigInvoId, "7ef77c0f-cff1-4239-b293-39a2b302d5bd");
    assert_string_equal(pReplMetaData->pMetaData->pszOrigTime, "20180702222545.584");
    assert_int_equal(pReplMetaData->pMetaData->origUsn, 100);

    pAttr = VmDirFindAttrByName(pIndividualUpdate->pEntry, ATTR_USN_CHANGED);
    assert_non_null(pAttr);
    assert_string_equal(pAttr->type.lberbv_val, ATTR_USN_CHANGED);
    dwError = VmDirStringToUINT64(pAttr->vals[0].lberbv_val, NULL, &localUsn);
    assert_int_equal(dwError, 0);
    assert_int_equal(localUsn, usn);

    pAttr = VmDirFindAttrByName(pIndividualUpdate->pEntry, ATTR_OBJECT_GUID);
    assert_non_null(pAttr);
    assert_string_equal(pAttr->type.lberbv_val, ATTR_OBJECT_GUID);
    assert_string_equal(pAttr->vals[0].lberbv_val, "e7f6eae8-9902-4270-91ee-1ab36c898580");
}
