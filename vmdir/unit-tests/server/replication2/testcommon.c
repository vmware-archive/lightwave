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

PVMDIR_REPL_ATTRIBUTE_METADATA
VmDirFindAttrMetaData(
    PVDIR_LINKED_LIST    pList,
    PSTR                 pszAttrType
    )
{
    DWORD                             dwError = 0;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pMetaData = NULL;
    PVDIR_LINKED_LIST_NODE            pNode = NULL;

    dwError = VmDirLinkedListGetHead(pList, &pNode);
    assert_int_equal(dwError, 0);

    while (pNode)
    {
        pMetaData = pNode->pElement;

        if (VmDirStringCompareA(pMetaData->pszAttrType, pszAttrType, FALSE) == 0)
        {
            break;
        }

        pNode = pNode->pNext;
        pMetaData = NULL;
    }

    return pMetaData;
}

VOID
VmDirAllocateAttrMetaData(
    PSTR                         pszAttrType,
    USN                          localUsn,
    USN                          origUsn,
    UINT64                       version,
    PVMDIR_REPLICATION_UPDATE    pUpdate
    )
{
    DWORD                             dwError = 0;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pReplMetaData = NULL;

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

    dwError = VmDirLinkedListInsertHead(pUpdate->pMetaDataList, (PVOID)pReplMetaData, NULL);
    assert_int_equal(dwError, 0);
}

VOID
VmDirAllocateAttrAndMetaData(
    PSTR                         pszAttrType,
    PSTR                         pszValue,
    USN                          localUsn,
    USN                          origUsn,
    UINT64                       version,
    PVMDIR_REPLICATION_UPDATE    pUpdate
    )
{
    DWORD              dwError = 0;
    PVDIR_ATTRIBUTE    pAttr = NULL;

    //Populate Attr MetaData
    VmDirAllocateAttrMetaData(pszAttrType, localUsn, origUsn, version, pUpdate);

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

    //Insert pAttr to pEntry
    pAttr->next = pUpdate->pEntry->attrs;
    pUpdate->pEntry->attrs = pAttr;
}

VOID
VmDirAllocateMultiValueAttr(
    PSTR                         pszAttrType,
    USN                          localUSN,
    USN                          origUSN,
    PVMDIR_REPLICATION_UPDATE    pUpdate
    )
{
    DWORD              dwError = 0;
    PVDIR_ATTRIBUTE    pAttr = NULL;

    dwError = VmDirAllocateMemory(sizeof(VDIR_ATTRIBUTE), (PVOID*)&pAttr);
    assert_int_equal(dwError, 0);

    dwError = VmDirStringToBervalContent(pszAttrType, &pAttr->type);
    assert_int_equal(dwError, 0);

    VmDirAllocateAttrMetaData(pszAttrType, localUSN, origUSN, 1, pUpdate);

    dwError = VmDirAllocateMemory(sizeof(VDIR_BERVALUE) * (4), (PVOID*)&pAttr->vals);
    assert_int_equal(dwError, 0);

    dwError = VmDirStringToBervalContent(
            "cn=administrator@test-20002,cn=Users,dc=test-20002", &pAttr->vals[0]);
    assert_int_equal(dwError, 0);

    dwError = VmDirStringToBervalContent(
            "cn=administrator@test-20004,cn=Users,dc=test-20004", &pAttr->vals[1]);
    assert_int_equal(dwError, 0);

    dwError = VmDirStringToBervalContent(
            "cn=administrator@test-20005,cn=Users,dc=test-20005", &pAttr->vals[2]);
    assert_int_equal(dwError, 0);

    pAttr->numVals = 3;

    //Insert pAttr to pEntry
    pAttr->next = pUpdate->pEntry->attrs;
    pUpdate->pEntry->attrs = pAttr;
}

VOID
VmDirAllocateAttrValueMetaData(
    PSTR                         pszValueMetaData,
    PVMDIR_REPLICATION_UPDATE    pUpdate
    )
{
    DWORD                              dwError = 0;
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pValueMetaData = NULL;

    dwError = VmDirValueMetaDataDeserialize(pszValueMetaData, &pValueMetaData);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListInsertHead(pUpdate->pValueMetaDataList, (PVOID)pValueMetaData, NULL);
    assert_int_equal(dwError, 0);
}
