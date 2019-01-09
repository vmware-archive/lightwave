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
DWORD
_VmDirReplUpdateHandleTombStoneAdd(
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate,
    PVMDIR_REPLICATION_UPDATE    pCombinedUpdate
    );
/*
 * Extract all single value attribute metadata and values corresponding to USN
 * Move from pCombinedUpdate and add to pIndividualUpdate
 */
DWORD
VmDirExtractEventAttributeChanges(
    PVMDIR_REPLICATION_UPDATE    pCombinedUpdate,
    USN                          usn,
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate
    )
{
    DWORD                             dwError = 0;
    PLW_HASHMAP                       pAttrMap = NULL;
    PVDIR_ATTRIBUTE                   pCurrAttr = NULL;
    PVDIR_ATTRIBUTE                   pNextAttr = NULL;
    PVDIR_ATTRIBUTE                   pRemainingAttrs = NULL;
    PVDIR_ATTRIBUTE                   pExtractedAttrs = NULL;
    PVDIR_LINKED_LIST                 pNewMetaDataList = NULL;
    PVDIR_LINKED_LIST_NODE            pCurrNode = NULL;
    PVDIR_LINKED_LIST_NODE            pNextNode = NULL;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pReplMetaData = NULL;

    if (!pCombinedUpdate || !pIndividualUpdate)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = LwRtlCreateHashMap(
            &pAttrMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    //Transfer MetaData to pIndividualUpdate
    dwError = VmDirLinkedListCreate(&pNewMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListGetHead(pCombinedUpdate->pMetaDataList, &pCurrNode);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (pCurrNode)
    {
        pReplMetaData = (PVMDIR_REPL_ATTRIBUTE_METADATA) pCurrNode->pElement;

        pNextNode = pCurrNode->pNext;

        //Skip UsnChanged, UsnChanged will be added in VmDirExtractEventPopulateOperationAttributes
        if (pReplMetaData->pMetaData->localUsn == usn &&
            VmDirStringCompareA(pReplMetaData->pszAttrType, ATTR_USN_CHANGED, FALSE) != 0)
        {
            dwError = VmDirLinkedListInsertHead(pNewMetaDataList, pCurrNode->pElement, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirLinkedListRemove(pCombinedUpdate->pMetaDataList, pCurrNode);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = LwRtlHashMapInsert(pAttrMap, pReplMetaData->pszAttrType, NULL, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        pCurrNode = pNextNode;
    }
    pIndividualUpdate->pMetaDataList = pNewMetaDataList;
    pNewMetaDataList = NULL;

    //Transfer attribute to pIndividualUpdate
    pCurrAttr = pCombinedUpdate->pEntry->attrs;
    while (pCurrAttr)
    {
        pNextAttr = pCurrAttr->next;

        if (LwRtlHashMapFindKey(pAttrMap, NULL, pCurrAttr->type.lberbv_val) == 0)
        {
            pCurrAttr->next = pExtractedAttrs;
            pExtractedAttrs = pCurrAttr;
        }
        else
        {
            pCurrAttr->next = pRemainingAttrs;
            pRemainingAttrs = pCurrAttr;
        }

        pCurrAttr = pNextAttr;
    }

    pCombinedUpdate->pEntry->attrs = pRemainingAttrs;
    pIndividualUpdate->pEntry->attrs = pExtractedAttrs;

cleanup:
    if (pAttrMap)
    {
        LwRtlHashMapClear(pAttrMap, VmDirNoopHashMapPairFree, NULL);
    }
    LwRtlFreeHashMap(&pAttrMap);
    return dwError;

error:
    VmDirFreeReplMetaDataList(pNewMetaDataList);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

/*
 * Extract all multi-value attribute metadata and values corresponding to USN
 * Move from pCombinedUpdate and add to pIndividualUpdate
 *
 * Four cases possible here
 *     1) Attribute and values alone no value metadata
 *     2) Attribute and values and value metadata
 *     3) Only value metadata
 *     4) Only value metadata and attribute metadata
 *        (single value present in multivalue attr, delete the value
 *         rather than deleting the attribute)
 *
 * Attribute value needs to be manipulated for the 2nd and 4th case, please look at the below
 * example for better understanding.
 *
 * For example:
 *     - Member Attr Values: A (USN: 100), B
 *     - Value MetaData for B (OP: Add USN:110),  D (OP: Del USN: 130) are present
 * MOD_OP_ADD:
 *     - In order to perform USN: 110, add value 'B'. We need to Remove value 'B' from pAttr
 *       so that no duplicate values exist.
 * MOD_OP_DELETE:
 *     - In order to perform USN: 130, value 'D' has to be present.
 *       We need to add them first, hence add that to pAttr Values.
 *
 * Basically we are trying to mimic the operations in the same order as it happened in
 * the supplier.
 *
 * Final Result will be:
 *     1) pCombinedUpdate (USN: 100) Member Attr Values: A, D
 *     2) pCombinedUpdate (USN: 110) Add value B
 *     3) pCombinedUpdate (USN: 130) Delete value D
 */
DWORD
VmDirExtractEventAttributeValueChanges(
    PVMDIR_REPLICATION_UPDATE    pCombinedUpdate,
    USN                          usn,
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate
    )
{
    DWORD                              dwError = 0;
    PSTR                               pszValue = NULL;
    PVDIR_ATTRIBUTE                    pAttr = NULL;
    PVDIR_LINKED_LIST                  pNewValueMetaDataList = NULL;
    PVDIR_LINKED_LIST_NODE             pCurrNode = NULL;
    PVDIR_LINKED_LIST_NODE             pNextNode = NULL;
    PVDIR_LINKED_LIST_NODE             pTempNode = NULL;
    PVMDIR_REPL_ATTRIBUTE_METADATA     pReplMetaData = NULL;
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pValueMetaData = NULL;

    if (!pCombinedUpdate || !pIndividualUpdate)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirLinkedListCreate(&pNewValueMetaDataList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListGetHead(pCombinedUpdate->pValueMetaDataList, &pCurrNode);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (pCurrNode)
    {
        pValueMetaData = (PVMDIR_VALUE_ATTRIBUTE_METADATA) pCurrNode->pElement;

        pNextNode = pCurrNode->pNext;

        if (pValueMetaData->localUsn == usn)
        {
            dwError = VmDirLinkedListInsertHead(pNewValueMetaDataList, pCurrNode->pElement, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirLinkedListRemove(pCombinedUpdate->pValueMetaDataList, pCurrNode);
            BAIL_ON_VMDIR_ERROR(dwError);

            //Iterate and Get Attr MetaData instead of Attr, to cover deleted attribute scenario
            dwError = VmDirLinkedListGetHead(pCombinedUpdate->pMetaDataList, &pTempNode);
            BAIL_ON_VMDIR_ERROR(dwError);

            while (pTempNode)
            {
                pReplMetaData = (PVMDIR_REPL_ATTRIBUTE_METADATA) pTempNode->pElement;

                if (VmDirStringCompareA(
                            pReplMetaData->pszAttrType, pValueMetaData->pszAttrType, FALSE) == 0)
                {
                    break;
                }

                pReplMetaData = NULL;
                pTempNode = pTempNode->pNext;
            }

            if (pReplMetaData)
            {
                pAttr = VmDirFindAttrByName(pCombinedUpdate->pEntry, pValueMetaData->pszAttrType);
                if (pAttr == NULL)
                {
                    dwError = VmDirAllocateMemory(sizeof(VDIR_ATTRIBUTE), (PVOID*)&pAttr);
                    BAIL_ON_VMDIR_ERROR(dwError);

                    // add one more BerValue as Encode/Decode entry in data store layer needs it.
                    dwError = VmDirAllocateMemory(
                            sizeof(VDIR_BERVALUE) * (pAttr->numVals + 1),
                            (PVOID*)&pAttr->vals);
                    BAIL_ON_VMDIR_ERROR(dwError);

                    dwError = VmDirStringToBervalContent(pValueMetaData->pszAttrType, &pAttr->type);
                    BAIL_ON_VMDIR_ERROR(dwError);

                    pAttr->next = pCombinedUpdate->pEntry->attrs;
                    pCombinedUpdate->pEntry->attrs = pAttr;
                }

                if (pValueMetaData->dwOpCode == MOD_OP_ADD)
                {
                    dwError = VmDirEntryAttributeRemoveValue(pAttr, pValueMetaData->pszValue);
                    BAIL_ON_VMDIR_ERROR(dwError);
                }
                else if (pValueMetaData->dwOpCode == MOD_OP_DELETE)
                {
                    VDIR_BERVALUE    bvValue = VDIR_BERVALUE_INIT;

                    dwError = VmDirAllocateAndCopyMemory(
                            pValueMetaData->pszValue, pValueMetaData->dwValSize, (PVOID*)&pszValue);
                    BAIL_ON_VMDIR_ERROR(dwError);

                    bvValue.lberbv_val = pszValue;
                    bvValue.lberbv_len = pValueMetaData->dwValSize;
                    bvValue.bOwnBvVal = TRUE;
                    pszValue = NULL;

                    //bvValue contents will be copied into pAttr,, should not free bvValue contents
                    dwError = VmDirEntryAttributeAppendBervArray(pAttr, &bvValue, 1);
                    BAIL_ON_VMDIR_ERROR(dwError);
                }
            }
        }

        pCurrNode = pNextNode;
    }

    pIndividualUpdate->pValueMetaDataList = pNewValueMetaDataList;
    pNewValueMetaDataList = NULL;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszValue);
    VmDirFreeValueMetaDataList(pNewValueMetaDataList);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

/*
 * If sync state is Add and any must attrs have been extracted into pIndividualUpdate
 * make a copy of them to pCombinedUpdate.
 */
DWORD
VmDirExtractEventPopulateMustAttributes(
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate,
    PVMDIR_REPLICATION_UPDATE    pCombinedUpdate
    )
{
    DWORD                             dwError = 0;
    PLW_HASHMAP                       pAllMustAttrMap = NULL;
    PVDIR_ATTRIBUTE                   pIndividualUpdateAttr = NULL;
    PVDIR_ATTRIBUTE                   pCombinedUpdateAttr = NULL;
    PVDIR_ATTRIBUTE                   pNewAttr = NULL;
    PVDIR_LINKED_LIST_NODE            pCurrNode = NULL;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pReplMetaData = NULL;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pNewReplMetaData = NULL;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pUsnCreatedMetaData = NULL;

    if (!pCombinedUpdate || !pIndividualUpdate)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (pCombinedUpdate->syncState != LDAP_SYNC_ADD)
    {
        return 0;
    }

    dwError = VmDirLinkedListGetHead(pCombinedUpdate->pMetaDataList, &pCurrNode);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (pCurrNode)
    {
        pReplMetaData = (PVMDIR_REPL_ATTRIBUTE_METADATA) pCurrNode->pElement;

        if (VmDirStringCompareA(pReplMetaData->pszAttrType, ATTR_USN_CREATED, FALSE) == 0)
        {
            pUsnCreatedMetaData = pReplMetaData;
            break;
        }

        pCurrNode = pCurrNode->pNext;
    }

    if (pUsnCreatedMetaData == NULL)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "%s: AttrMetaData not present for: %s",
                __FUNCTION__,
                ATTR_USN_CREATED);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NO_ATTRIBUTE_METADATA);
    }

    //pCombinedUpdate because we have to examine the objectclass
    dwError = VmDirEntryGetAllMustAttrs(pCombinedUpdate->pEntry, &pAllMustAttrMap);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (pIndividualUpdateAttr = pIndividualUpdate->pEntry->attrs;
         pIndividualUpdateAttr;
         pIndividualUpdateAttr = pIndividualUpdateAttr->next)
    {
        if (LwRtlHashMapFindKey(pAllMustAttrMap, NULL, pIndividualUpdateAttr->type.lberbv.bv_val) == 0)
        {
            pCombinedUpdateAttr = VmDirFindAttrByName(
                    pCombinedUpdate->pEntry, pIndividualUpdateAttr->type.lberbv.bv_val);

            if (pCombinedUpdateAttr == NULL)
            {
                //copy Attr MetaData
                dwError = VmDirReplMetaDataCreate(
                        pIndividualUpdateAttr->type.lberbv.bv_val,
                        pCombinedUpdate->partnerUsn,
                        1,
                        pUsnCreatedMetaData->pMetaData->pszOrigInvoId,
                        pUsnCreatedMetaData->pMetaData->pszOrigTime,
                        pUsnCreatedMetaData->pMetaData->origUsn,
                        &pNewReplMetaData);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = VmDirLinkedListInsertHead(
                        pCombinedUpdate->pMetaDataList, (PVOID)pNewReplMetaData, NULL);
                BAIL_ON_VMDIR_ERROR(dwError);
                pNewReplMetaData = NULL;

                //copy Attr
                dwError = VmDirAttributeDup(pIndividualUpdateAttr, &pNewAttr);
                BAIL_ON_VMDIR_ERROR(dwError);

                pNewAttr->next = pCombinedUpdate->pEntry->attrs;
                pCombinedUpdate->pEntry->attrs = pNewAttr;
                pNewAttr = NULL;
            }
        }
    }

cleanup:
    if (pAllMustAttrMap)
    {
        LwRtlHashMapClear(pAllMustAttrMap, VmDirNoopHashMapPairFree, NULL);
    }
    LwRtlFreeHashMap(&pAllMustAttrMap);
    return dwError;

error:
    VmDirFreeAttribute(pNewAttr);
    VmDirFreeReplMetaData(pNewReplMetaData);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

/*
 * Copy operational attributes from pCombinedUpdate to pIndividualUpdate (ObjectGuid and UsnChanged)
 * If Tombstone entry, then ensure entryDN value is copied into pCombinedUpdate from lastknownDN
 */
DWORD
VmDirExtractEventPopulateOperationAttributes(
    PVMDIR_REPLICATION_UPDATE    pCombinedUpdate,
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate
    )
{
    DWORD                             dwError = 0;
    PVDIR_ATTRIBUTE                   pCurrAttr = NULL;
    PVDIR_ATTRIBUTE                   pNewAttr = NULL;
    PVDIR_LINKED_LIST_NODE            pCurrNode = NULL;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pReplMetaData = NULL;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pNewReplMetaData = NULL;

    if (!pCombinedUpdate || !pIndividualUpdate)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pCurrAttr = VmDirFindAttrByName(pIndividualUpdate->pEntry, ATTR_IS_DELETED);
    if (pCurrAttr)
    {
        pIndividualUpdate->syncState = LDAP_SYNC_DELETE;

        if (pCombinedUpdate->syncState == LDAP_SYNC_ADD)
        {
            dwError = _VmDirReplUpdateHandleTombStoneAdd(pIndividualUpdate, pCombinedUpdate);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    //Copy UsnChanged and ObjectGuid to pIndividualUpdate
    for (pCurrAttr = pCombinedUpdate->pEntry->attrs; pCurrAttr; pCurrAttr = pCurrAttr->next)
    {
        if (VmDirStringCompareA(pCurrAttr->type.lberbv_val, ATTR_OBJECT_GUID, FALSE) == 0)
        {
            dwError = VmDirAttributeDup(pCurrAttr, &pNewAttr);
            BAIL_ON_VMDIR_ERROR(dwError);

            pNewAttr->next = pIndividualUpdate->pEntry->attrs;
            pIndividualUpdate->pEntry->attrs = pNewAttr;
            pNewAttr = NULL;
        }

        if (VmDirStringCompareA(pCurrAttr->type.lberbv_val, ATTR_USN_CHANGED, FALSE) == 0)
        {
            dwError = VmDirAttributeDup(pCurrAttr, &pNewAttr);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirAttributeUpdateUsnValue(pNewAttr, pIndividualUpdate->partnerUsn);
            BAIL_ON_VMDIR_ERROR(dwError);

            pNewAttr->next = pIndividualUpdate->pEntry->attrs;
            pIndividualUpdate->pEntry->attrs = pNewAttr;
            pNewAttr = NULL;
        }
    }

    dwError = VmDirLinkedListGetHead(pCombinedUpdate->pMetaDataList, &pCurrNode);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (pCurrNode)
    {
        pReplMetaData = (PVMDIR_REPL_ATTRIBUTE_METADATA) pCurrNode->pElement;

        if (VmDirStringCompareA(pReplMetaData->pszAttrType, ATTR_OBJECT_GUID, FALSE) == 0)
        {
            dwError = VmDirReplMetaDataCreate(
                    pReplMetaData->pszAttrType,
                    pReplMetaData->pMetaData->localUsn,
                    pReplMetaData->pMetaData->version,
                    pReplMetaData->pMetaData->pszOrigInvoId,
                    pReplMetaData->pMetaData->pszOrigTime,
                    pReplMetaData->pMetaData->origUsn,
                    &pNewReplMetaData);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirLinkedListInsertHead(
                    pIndividualUpdate->pMetaDataList, (PVOID)pNewReplMetaData, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
            pNewReplMetaData = NULL;
        }

        if (VmDirStringCompareA(pReplMetaData->pszAttrType, ATTR_USN_CHANGED, FALSE) == 0)
        {
            dwError = VmDirReplMetaDataCreate(
                    pReplMetaData->pszAttrType,
                    pIndividualUpdate->partnerUsn,
                    pReplMetaData->pMetaData->version,
                    pReplMetaData->pMetaData->pszOrigInvoId,
                    pReplMetaData->pMetaData->pszOrigTime,
                    pReplMetaData->pMetaData->origUsn,
                    &pNewReplMetaData);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirLinkedListInsertHead(
                    pIndividualUpdate->pMetaDataList, (PVOID)pNewReplMetaData, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
            pNewReplMetaData = NULL;
        }

        pCurrNode = pCurrNode->pNext;
    }

cleanup:
    return dwError;

error:
    VmDirFreeAttribute(pNewAttr);
    VmDirFreeReplMetaData(pNewReplMetaData);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

static
DWORD
_VmDirReplUpdateHandleTombStoneAdd(
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate,
    PVMDIR_REPLICATION_UPDATE    pCombinedUpdate
    )
{
    DWORD                             dwError = 0;
    PVDIR_ATTRIBUTE                   pCurrAttr = NULL;
    PVDIR_ATTRIBUTE                   pNewAttr = NULL;
    PVDIR_ATTRIBUTE                   pLastKnownAttr = NULL;
    PVDIR_LINKED_LIST_NODE            pCurrNode = NULL;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pReplMetaData = NULL;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pNewReplMetaData = NULL;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pUsnCreatedMetaData = NULL;

    //Change pCombinedUpdate's pEntry->dn from cn=ZZZ#objectguid.. to cn=ZZZ,cn=..
    pLastKnownAttr = VmDirFindAttrByName(pIndividualUpdate->pEntry, ATTR_LAST_KNOWN_DN);
    if (pLastKnownAttr == NULL)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "%s: Attr: %s is not present",
                __FUNCTION__,
                ATTR_LAST_KNOWN_DN);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NO_SUCH_ATTRIBUTE);
    }

    dwError = VmDirLinkedListGetHead(pCombinedUpdate->pMetaDataList, &pCurrNode);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (pCurrNode)
    {
        pReplMetaData = (PVMDIR_REPL_ATTRIBUTE_METADATA) pCurrNode->pElement;

        if (VmDirStringCompareA(pReplMetaData->pszAttrType, ATTR_USN_CREATED, FALSE) == 0)
        {
            pUsnCreatedMetaData = pReplMetaData;
            break;
        }

        pCurrNode = pCurrNode->pNext;
    }

    if (pUsnCreatedMetaData == NULL)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "%s: AttrMetaData not present for: %s",
                __FUNCTION__,
                ATTR_USN_CREATED);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NO_ATTRIBUTE_METADATA);
    }

    dwError = VmDirBervalContentDup(&pLastKnownAttr->vals[0], &pCombinedUpdate->pEntry->dn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListGetHead(pIndividualUpdate->pMetaDataList, &pCurrNode);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (pCurrNode)
    {
        pReplMetaData = (PVMDIR_REPL_ATTRIBUTE_METADATA) pCurrNode->pElement;

        //Copy LastKnownDN attr metadata as EntryDN attr metadata to pCombinedUdpate
        if (VmDirStringCompareA(pReplMetaData->pszAttrType, ATTR_LAST_KNOWN_DN, FALSE) == 0)
        {
            dwError = VmDirReplMetaDataCreate(
                    ATTR_DN,
                    pCombinedUpdate->partnerUsn,
                    1,
                    pUsnCreatedMetaData->pMetaData->pszOrigInvoId,
                    pUsnCreatedMetaData->pMetaData->pszOrigTime,
                    pUsnCreatedMetaData->pMetaData->origUsn,
                    &pNewReplMetaData);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirLinkedListInsertHead(
                    pCombinedUpdate->pMetaDataList, (PVOID)pNewReplMetaData, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
            pNewReplMetaData = NULL;
        }
        else if (VmDirStringCompareA(pReplMetaData->pszAttrType, ATTR_OBJECT_CLASS, FALSE) == 0)
        {
            dwError = VmDirReplMetaDataCreate(
                    ATTR_OBJECT_CLASS,
                    pCombinedUpdate->partnerUsn,
                    1,
                    pUsnCreatedMetaData->pMetaData->pszOrigInvoId,
                    pUsnCreatedMetaData->pMetaData->pszOrigTime,
                    pUsnCreatedMetaData->pMetaData->origUsn,
                    &pNewReplMetaData);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirLinkedListInsertHead(
                    pCombinedUpdate->pMetaDataList, (PVOID)pNewReplMetaData, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
            pNewReplMetaData = NULL;
        }

        pCurrNode = pCurrNode->pNext;
    }

    for (pCurrAttr = pIndividualUpdate->pEntry->attrs; pCurrAttr; pCurrAttr = pCurrAttr->next)
    {
        //Copy LastKnownDN Attr as EntryDN Attr to pCombinedUpdate
        if (VmDirStringCompareA(pCurrAttr->type.lberbv.bv_val, ATTR_LAST_KNOWN_DN, FALSE) == 0)
        {
            dwError = VmDirAttributeDup(pCurrAttr, &pNewAttr);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirStringToBervalContent(ATTR_DN, &pNewAttr->type);
            BAIL_ON_VMDIR_ERROR(dwError);

            pNewAttr->next = pCombinedUpdate->pEntry->attrs;
            pCombinedUpdate->pEntry->attrs = pNewAttr;
            pNewAttr = NULL;
        }
        else if (VmDirStringCompareA(pCurrAttr->type.lberbv.bv_val, ATTR_OBJECT_CLASS, FALSE) == 0)
        {
            dwError = VmDirAttributeDup(pCurrAttr, &pNewAttr);
            BAIL_ON_VMDIR_ERROR(dwError);

            pNewAttr->next = pCombinedUpdate->pEntry->attrs;
            pCombinedUpdate->pEntry->attrs = pNewAttr;
            pNewAttr = NULL;
        }
    }

cleanup:
    return dwError;

error:
    VmDirFreeAttribute(pNewAttr);
    VmDirFreeReplMetaData(pNewReplMetaData);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}
