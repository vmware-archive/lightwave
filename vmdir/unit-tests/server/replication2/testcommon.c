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
    PCSTR                        pcszOrigInvoId,
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
            pcszOrigInvoId,
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
    PCSTR                        pcszOrigInvoId,
    PVMDIR_REPLICATION_UPDATE    pUpdate
    )
{
    DWORD              dwError = 0;
    PVDIR_ATTRIBUTE    pAttr = NULL;

    //Populate Attr MetaData
    VmDirAllocateAttrMetaData(
            pszAttrType, localUsn, origUsn, version, pcszOrigInvoId, pUpdate);

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
    PCSTR                        pcszOrigInvoId,
    VDIR_BERVARRAY               pbvVals,
    DWORD                        dwVals,
    PVMDIR_REPLICATION_UPDATE    pUpdate
    )
{
    DWORD              dwError = 0;
    PVDIR_ATTRIBUTE    pAttr = NULL;

    dwError = VmDirAllocateMemory(sizeof(VDIR_ATTRIBUTE), (PVOID*)&pAttr);
    assert_int_equal(dwError, 0);

    dwError = VmDirStringToBervalContent(pszAttrType, &pAttr->type);
    assert_int_equal(dwError, 0);

    VmDirAllocateAttrMetaData(
            pszAttrType, localUSN, origUSN, 1, pcszOrigInvoId, pUpdate);

    pAttr->vals = pbvVals;
    pAttr->numVals = dwVals;

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

BOOLEAN
VmDirTestCompareAttr(
    PVDIR_ATTRIBUTE pAttr1,
    PVDIR_ATTRIBUTE pAttr2
    )
{
    DWORD           dwError = 0;
    DWORD           dwValCount = 0;
    PLW_HASHMAP     pAttrValuesMap = NULL;
    LW_HASHMAP_PAIR pair = {NULL, NULL};

    if (!pAttr1 && !pAttr2)
    {
        return TRUE;
    }

    assert_true(pAttr1 && pAttr2);

    assert_int_equal(pAttr1->type.lberbv_len, pAttr2->type.lberbv_len);
    assert_int_equal(VmDirStringNCompareA(pAttr1->type.lberbv_val, pAttr2->type.lberbv_val, pAttr1->type.lberbv_len, FALSE), 0);

    assert_int_equal(pAttr1->numVals, pAttr2->numVals);

    dwError = LwRtlCreateHashMap(
            &pAttrValuesMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    assert_int_equal(dwError, 0);

    for (dwValCount = 0; dwValCount < pAttr1->numVals; dwValCount++)
    {
        dwError = LwRtlHashMapInsert(pAttrValuesMap, pAttr1->vals[dwValCount].lberbv_val, NULL, NULL);
        assert_int_equal(dwError, 0);
    }

    for (dwValCount = 0; dwValCount < pAttr2->numVals; dwValCount++)
    {
        dwError = LwRtlHashMapRemove(pAttrValuesMap, pAttr2->vals[dwValCount].lberbv_val, &pair);
        assert_int_equal(dwError, 0);
    }

    assert_int_equal(LwRtlHashMapGetCount(pAttrValuesMap), 0);

    if (pAttrValuesMap)
    {
        LwRtlFreeHashMap(&pAttrValuesMap);
    }

    return TRUE;
}

BOOLEAN
VmDirTestCompareEntry(
    PVDIR_ENTRY pEntry1,
    PVDIR_ENTRY pEntry2
    )
{
    PVDIR_ATTRIBUTE pAttr1 = NULL;
    PVDIR_ATTRIBUTE pAttr2 = NULL;

    if (!pEntry1 && !pEntry2)
    {
        return TRUE;
    }

    assert_true(pEntry1 && pEntry2);

    assert_int_equal(pEntry1->dn.lberbv_len, pEntry2->dn.lberbv_len);
    assert_int_equal(VmDirStringNCompareA(pEntry1->dn.lberbv_val, pEntry2->dn.lberbv_val, pEntry1->dn.lberbv_len, FALSE), 0);

    pAttr1 = pEntry1->attrs;

    while (pAttr1)
    {
        pAttr2 = VmDirFindAttrByName(pEntry2, pAttr1->type.lberbv_val);
        assert_non_null(pAttr2);

        assert_true(VmDirTestCompareAttr(pAttr1, pAttr2));

        pAttr1 = pAttr1->next;
    }

    return TRUE;
}

BOOLEAN
VmDirTestCompareMetaData(
    PVMDIR_REPL_ATTRIBUTE_METADATA  pReplMetadata1,
    PVMDIR_REPL_ATTRIBUTE_METADATA  pReplMetadata2
    )
{
    if (!pReplMetadata1 && !pReplMetadata2)
    {
        return TRUE;
    }

    assert_true(pReplMetadata1 && pReplMetadata1);

    assert_int_equal(VmDirStringCompareA(pReplMetadata1->pszAttrType, pReplMetadata2->pszAttrType, FALSE), 0);

    assert_false(!pReplMetadata1->pMetaData || !pReplMetadata2->pMetaData);

    assert_int_equal(pReplMetadata1->pMetaData->localUsn, pReplMetadata2->pMetaData->localUsn);
    assert_int_equal(pReplMetadata1->pMetaData->version, pReplMetadata2->pMetaData->version);
    assert_int_equal(pReplMetadata1->pMetaData->origUsn, pReplMetadata2->pMetaData->origUsn);
    assert_int_equal(VmDirStringCompareA(pReplMetadata1->pMetaData->pszOrigInvoId, pReplMetadata2->pMetaData->pszOrigInvoId, FALSE), 0);
    assert_int_equal(VmDirStringCompareA(pReplMetadata1->pMetaData->pszOrigTime, pReplMetadata2->pMetaData->pszOrigTime, FALSE), 0);

    return TRUE;
}

BOOLEAN
VmDirTestCompareMetaDataList(
    PVDIR_LINKED_LIST pList1,
    PVDIR_LINKED_LIST pList2
    )
{
    PVDIR_LINKED_LIST_NODE          pNode = NULL;
    PVMDIR_REPL_ATTRIBUTE_METADATA  pReplMetaData1 = NULL;
    PVMDIR_REPL_ATTRIBUTE_METADATA  pReplMetaData2 = NULL;

    pNode = pList1->pHead;

    while (pNode)
    {
        pReplMetaData1 = (PVMDIR_REPL_ATTRIBUTE_METADATA)pNode->pElement;

        pReplMetaData2 = VmDirFindAttrMetaData(pList2, pReplMetaData1->pszAttrType);
        assert_non_null(pReplMetaData2);

        assert_true(VmDirTestCompareMetaData(pReplMetaData1, pReplMetaData2));

        pNode = pNode->pNext;
    }

    return TRUE;
}

BOOLEAN
VmDirTestCompareReplUpdate(
    PVMDIR_REPLICATION_UPDATE    pUpdate1,
    PVMDIR_REPLICATION_UPDATE    pUpdate2
    )
{
    assert_int_equal(pUpdate1->syncState, pUpdate2->syncState);
    assert_int_equal(pUpdate1->partnerUsn, pUpdate2->partnerUsn);

    assert_true(VmDirTestCompareEntry(pUpdate1->pEntry, pUpdate2->pEntry));

    assert_true(VmDirTestCompareMetaDataList(pUpdate1->pMetaDataList, pUpdate2->pMetaDataList));

    return TRUE;
}

DWORD
VmDirTestAllocateReplUpdate(
    PVMDIR_REPLICATION_UPDATE   *ppReplUpdate
    )
{
    DWORD                       dwError = 0;
    PVMDIR_REPLICATION_UPDATE   pReplUpdate = NULL;

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pReplUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pReplUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pReplUpdate->pMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    pReplUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;

    *ppReplUpdate = pReplUpdate;

cleanup:
    return dwError;

error:
    print_message("%s failed with error %d", __FUNCTION__, dwError);

    if (pReplUpdate)
    {
        VmDirFreeReplUpdate(pReplUpdate);
    }
    goto cleanup;
}

BOOLEAN
VmDirTestCompareReplUpdateList(
    PVMDIR_REPLICATION_UPDATE_LIST    pReplUpdateList1,
    PVMDIR_REPLICATION_UPDATE_LIST    pReplUpdateList2
    )
{
    DWORD                     dwError = 0;
    PVDIR_LINKED_LIST_NODE    pNode1 = NULL;
    PVDIR_LINKED_LIST_NODE    pNode2 = NULL;

    if (!pReplUpdateList1 && !pReplUpdateList2)
    {
        return TRUE;
    }

    if (!pReplUpdateList1 || !pReplUpdateList2)
    {
        return FALSE;
    }

    if (VmDirLinkedListGetSize(pReplUpdateList1->pLinkedList) !=
        VmDirLinkedListGetSize(pReplUpdateList2->pLinkedList))
    {
        return FALSE;
    }

    dwError = VmDirLinkedListGetHead(pReplUpdateList1->pLinkedList, &pNode1);
    assert_int_equal(dwError, 0);

    dwError = VmDirLinkedListGetHead(pReplUpdateList2->pLinkedList, &pNode2);
    assert_int_equal(dwError, 0);

    while (pNode1 && pNode2)
    {
        if (!VmDirTestCompareReplUpdate(pNode1->pElement, pNode2->pElement))
        {
            return FALSE;
        }

        pNode1 = pNode1->pNext;
        pNode2 = pNode2->pNext;
    }

    return TRUE;
}
