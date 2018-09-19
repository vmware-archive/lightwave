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



/*
 * Module Name: Replication update
 *
 * Filename: update.c
 *
 * Abstract:
 *
 */

#include "includes.h"

static
VOID
_VmDirReplicationUpdateTombStoneEntrySyncState(
    PVMDIR_REPLICATION_UPDATE   pUpdate
    );

static
int
_VmDirMergeSortCompareINT64Descending(
    const PVOID    pInt1,
    const PVOID    pInt2
    );

int
VmDirReplUpdateCreate(
    LDAP*                           pLd,
    LDAPMessage*                    pEntry,
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr,
    PVMDIR_REPLICATION_UPDATE*      ppCombinedUpdate
    )
{
    int                          retVal = LDAP_SUCCESS;
    PVMDIR_REPLICATION_UPDATE    pReplUpdate = NULL;
    LDAPControl**                ppCtrls = NULL;

    retVal = VmDirAllocateMemory(sizeof(*pReplUpdate), (PVOID*) &pReplUpdate);
    if (retVal)
    {
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
    }

    retVal = ldap_get_entry_controls(pLd, pEntry, &ppCtrls);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    pReplUpdate->syncState = -1;
    pReplUpdate->partnerUsn = 0;

    retVal = ParseAndFreeSyncStateControl(&ppCtrls, &pReplUpdate->syncState, &pReplUpdate->partnerUsn);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    retVal = VmDirAllocateStringA(pReplAgr->dcConn.pszHostname, &pReplUpdate->pszPartner);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    retVal = VmDirParseBerToEntry(pEntry->lm_ber, &pReplUpdate->pEntry, NULL, NULL);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    retVal = VmDirReplGetAttrMetaDataList(pReplUpdate->pEntry, &pReplUpdate->pMetaDataList);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    retVal = VmDirReplGetAttrValueMetaDataList(pReplUpdate->pEntry, &pReplUpdate->pValueMetaDataList);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    *ppCombinedUpdate = pReplUpdate;

cleanup:
    if (ppCtrls)
    {
        ldap_controls_free(ppCtrls);
    }
    return retVal;

ldaperror:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s: error = (%d)",
            __FUNCTION__,
            retVal);

    VmDirFreeReplUpdate(pReplUpdate);
    goto cleanup;
}

VOID
VmDirFreeReplUpdate(
    PVMDIR_REPLICATION_UPDATE   pUpdate
    )
{
    if (pUpdate)
    {
        VMDIR_SAFE_FREE_MEMORY(pUpdate->pszPartner);
        VmDirFreeEntry(pUpdate->pEntry);
        VmDirFreeReplMetaDataList(pUpdate->pMetaDataList);
        VmDirFreeValueMetaDataList(pUpdate->pValueMetaDataList);
    }

    VMDIR_SAFE_FREE_MEMORY(pUpdate);
}

VOID
VmDirReplUpdateApply(
    PVMDIR_REPLICATION_UPDATE   pReplUpdate
    )
{
    int errVal = 0;
    int entryState = 0;

    if (!pReplUpdate)
    {
        errVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_SIMPLE_LDAP_ERROR(errVal);
    }

    _VmDirReplicationUpdateTombStoneEntrySyncState(pReplUpdate);

    entryState = pReplUpdate->syncState;

    if (entryState == LDAP_SYNC_ADD)
    {
        errVal = ReplAddEntry(pReplUpdate);
    }
    else if (entryState == LDAP_SYNC_MODIFY)
    {
        errVal = ReplModifyEntry(pReplUpdate);
    }
    else if (entryState == LDAP_SYNC_DELETE)
    {
        errVal = ReplDeleteEntry(pReplUpdate);
    }
    else
    {
        errVal = LDAP_OPERATIONS_ERROR;
    }

    BAIL_ON_SIMPLE_LDAP_ERROR(errVal);

    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL,
                    "%s: sync_state = (%d) error = (%d)",
                    __FUNCTION__,
                    pReplUpdate->syncState,
                    errVal);

cleanup:
    return;

ldaperror:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "%s: sync_state = (%d) error = (%d)",
                    __FUNCTION__,
                    pReplUpdate->syncState,
                    errVal);

    goto cleanup;
}

//Generate List of USN's present in pCombinedUpdate
DWORD
VmDirReplUpdateToUSNList(
    PVMDIR_REPLICATION_UPDATE   pCombinedUpdate,
    PVDIR_LINKED_LIST*          ppUSNList
    )
{
    USN*                               pLocalUSN = NULL;
    DWORD                              dwError = 0;
    PLW_HASHMAP                        pUSNMap = NULL;
    LW_HASHMAP_ITER                    iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR                    pair = {NULL, NULL};
    LW_HASHMAP_PAIR                    prevPair = {NULL, NULL};
    PVDIR_LINKED_LIST                  pUSNList = NULL;
    PVDIR_LINKED_LIST_NODE             pNode = NULL;
    PVMDIR_REPL_ATTRIBUTE_METADATA     pReplMetaData = NULL;
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pValueMetaData = NULL;

    if (!pCombinedUpdate || !ppUSNList)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = LwRtlCreateHashMap(
            &pUSNMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    //Collect USN from Metadata List
    dwError = VmDirLinkedListGetHead(pCombinedUpdate->pMetaDataList, &pNode);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (pNode)
    {
        pReplMetaData = (PVMDIR_REPL_ATTRIBUTE_METADATA) pNode->pElement;

        dwError = VmDirAllocateMemory(sizeof(USN), (PVOID*)&pLocalUSN);
        BAIL_ON_VMDIR_ERROR(dwError);

        *pLocalUSN = pReplMetaData->pMetaData->localUsn;

        dwError = LwRtlHashMapInsert(pUSNMap, (PVOID)pLocalUSN, NULL, &prevPair);
        BAIL_ON_VMDIR_ERROR(dwError);
        pLocalUSN = NULL;

        VmDirSimpleHashMapPairFreeKeyOnly(&prevPair, NULL);

        pNode = pNode->pNext;
    }

    /*
     * Collect USN from value Metadata List
     * Ignore error, value metadata is not always present
     */
    VmDirLinkedListGetHead(pCombinedUpdate->pValueMetaDataList, &pNode);

    while (pNode)
    {
        pValueMetaData = (PVMDIR_VALUE_ATTRIBUTE_METADATA) pNode->pElement;

        dwError = VmDirAllocateMemory(sizeof(USN), (PVOID*)&pLocalUSN);
        BAIL_ON_VMDIR_ERROR(dwError);

        *pLocalUSN = pValueMetaData->localUsn;

        dwError = LwRtlHashMapInsert(pUSNMap, (PVOID) pLocalUSN, NULL, &prevPair);
        BAIL_ON_VMDIR_ERROR(dwError);
        pLocalUSN = NULL;

        VmDirSimpleHashMapPairFreeKeyOnly(&prevPair, NULL);

        pNode = pNode->pNext;
    }

    //Convert Hashmap to USN list
    dwError = VmDirLinkedListCreate(&pUSNList);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (LwRtlHashMapIterate(pUSNMap, &iter, &pair))
    {
        pLocalUSN = (USN*)pair.pKey;

        dwError = VmDirLinkedListInsertHead(pUSNList, (PVOID)*pLocalUSN, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirMergeSort(pUSNList, _VmDirMergeSortCompareINT64Descending);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppUSNList = pUSNList;

cleanup:
    if (pUSNMap)
    {
        LwRtlHashMapClear(
                pUSNMap,
                VmDirSimpleHashMapPairFreeKeyOnly,
                NULL);
        LwRtlFreeHashMap(&pUSNMap);
    }
    return dwError;

error:
    VmDirFreeLinkedList(pUSNList);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

static
int
_VmDirMergeSortCompareINT64Descending(
    const PVOID    pInt1,
    const PVOID    pInt2
    )
{
    return ((USN)pInt1 > (USN)pInt2);
}
/*
 * Split the pCombinedUpdate into multiple pIndividualUpdate based on the USN's present in
 * pCombinedUpdate. Add the newly generated pIndividualUpdate to pNewReplUpdateList
 *
 * If pCombinedUpdate have changes corresponding to USN: 100, 110, 120
 * At the end pNewReplUpdateList, will have:
 *     - pIndividualUpdate (USN: 110),
 *     - pIndividualUpdate (USN: 120)
 *       (Changed wrt to USN: 100, will be added by the VmDirReplUpdateListExpand)
 */
DWORD
VmDirReplUpdateSplit(
    PVMDIR_REPLICATION_UPDATE    pCombinedUpdate,
    PVDIR_SORTED_LINKED_LIST     pNewReplUpdateList
    )
{
    DWORD                        dwError = 0;
    PVDIR_LINKED_LIST            pUSNList = NULL;
    PVDIR_LINKED_LIST_NODE       pCurrNode = NULL;
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate = NULL;

    if (!pCombinedUpdate || !pNewReplUpdateList)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirReplUpdateToUSNList(pCombinedUpdate, &pUSNList);
    BAIL_ON_VMDIR_ERROR(dwError);

    /*
     * Modify pCombinedUpdate to have the lowest USN because algorithm works by
     * moving changes related to other USN into a new pCombinedUpdate in extractevent
     */
    dwError = VmDirReplUpdateLocalUsn(pCombinedUpdate, pUSNList);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirLinkedListGetHead(pUSNList, &pCurrNode);

    while (pCurrNode)
    {
        dwError = VmDirReplUpdateExtractEvent(
                pCombinedUpdate, (USN) pCurrNode->pElement, &pIndividualUpdate);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirSortedLinkedListInsert(pNewReplUpdateList, (PVOID)pIndividualUpdate);
        BAIL_ON_VMDIR_ERROR(dwError);
        pIndividualUpdate = NULL;

        pCurrNode = pCurrNode->pNext;
    }

cleanup:
    VmDirFreeLinkedList(pUSNList);
    return dwError;

error:
    VmDirFreeReplUpdate(pIndividualUpdate);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

/*
 * Algorithm is written in such a way that, pCombinedUpdate at end will have
 * all the changes corresponding to leastUSN present in pCombinedUpdate.
 * Hence update the UsnChanged attr and metadata with leastUSN present
 * in USNList
 */
DWORD
VmDirReplUpdateLocalUsn(
    PVMDIR_REPLICATION_UPDATE    pCombinedUpdate,
    PVDIR_LINKED_LIST            pUSNList
    )
{
    DWORD                             dwError = 0;
    USN                               localUSN = 0;
    PVDIR_LINKED_LIST_NODE            pCurrNode = NULL;
    PVDIR_LINKED_LIST_NODE            pLeastUSNNode = NULL;
    PVMDIR_REPL_ATTRIBUTE_METADATA    pReplMetaData = NULL;

    if (!pCombinedUpdate || !pUSNList)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VmDirLinkedListGetTail(pUSNList, &pLeastUSNNode);

    //pUSNList should have atleast one USN
    assert(pLeastUSNNode != NULL);

    localUSN = (USN) pLeastUSNNode->pElement;

    pCombinedUpdate->partnerUsn = localUSN;

    // Update USNChanged AttrValue
    dwError = VmDirEntryUpdateUsnChanged(pCombinedUpdate->pEntry, localUSN);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Update USNChanged MetaData
    VmDirLinkedListGetHead(pCombinedUpdate->pMetaDataList, &pCurrNode);

    while (pCurrNode)
    {
        pReplMetaData = (PVMDIR_REPL_ATTRIBUTE_METADATA) pCurrNode->pElement;

        if (VmDirStringCompareA(pReplMetaData->pszAttrType, ATTR_USN_CHANGED, FALSE) == 0)
        {
            pReplMetaData->pMetaData->localUsn = localUSN;
            break;
        }

        pCurrNode = pCurrNode->pNext;
    }

    // MetaDataList should have a attribute USNChanged
    assert(pCurrNode != NULL);

    VmDirLinkedListRemove(pUSNList, pLeastUSNNode);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

/*
 * UpdateExtractEvent is the core of the replication consumer algorithm
 *
 * Funtionality of ExtractEvent is to given a pCombinedUpdate and USN extract all the
 * changes corresponding to the USN into a pIndividualUpdate.
 *
 * Extract Event is done in four steps
 *     - Extract all single value attribute metadata and values corresponding to USN
 *       Move from pCombinedUpdate and add to pIndividualUpdate
 *     - Extract all multi-value attribute metadata and values corresponding to USN
 *       Move from pCombinedUpdate and add to pIndividualUpdate
 *     - If sync state is Add and any must attrs have been extracted into pIndividualUpdate
 *       make a copy of them to pCombinedUpdate.
 *     - Copy operational attributes from pCombinedUpdate to pIndividualUpdate
 *       (ObjectGuid and UsnChanged).
 *       If Tombstone entry, then ensure entryDN value is copied into pCombinedUpdate
 *       from lastknownDN
 */
DWORD
VmDirReplUpdateExtractEvent(
    PVMDIR_REPLICATION_UPDATE    pCombinedUpdate,
    USN                          usn,
    PVMDIR_REPLICATION_UPDATE*   ppIndividualUpdate
    )
{
    DWORD                        dwError = 0;
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate = NULL;

    if (!pCombinedUpdate || !ppIndividualUpdate)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(sizeof(VMDIR_REPLICATION_UPDATE), (PVOID*)&pIndividualUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);

    pIndividualUpdate->syncState =  LDAP_SYNC_MODIFY; //Default value
    pIndividualUpdate->partnerUsn = usn;

    dwError = VmDirAllocateStringA(pCombinedUpdate->pszPartner, &pIndividualUpdate->pszPartner);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pIndividualUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    pIndividualUpdate->pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;

    dwError = VmDirBervalContentDup(&pCombinedUpdate->pEntry->dn, &pIndividualUpdate->pEntry->dn);
    BAIL_ON_VMDIR_ERROR(dwError);

    //Extract Events
    dwError = VmDirExtractEventAttributeChanges(pCombinedUpdate, usn, pIndividualUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirExtractEventAttributeValueChanges(pCombinedUpdate, usn, pIndividualUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirExtractEventPopulateMustAttributes(pIndividualUpdate, pCombinedUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirExtractEventPopulateOperationAttributes(pCombinedUpdate, pIndividualUpdate);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppIndividualUpdate = pIndividualUpdate;

cleanup:
    return dwError;

error:
    VmDirFreeReplUpdate(pIndividualUpdate);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

BOOLEAN
VmDirSortedLinkedListInsertCompareReplUpdate(
    PVOID    pElement,
    PVOID    pNewElement
    )
{
    PVMDIR_REPLICATION_UPDATE    pCombinedUpdate = NULL;
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate = NULL;
    BOOLEAN                      bResult = FALSE;

    pCombinedUpdate = (PVMDIR_REPLICATION_UPDATE) pElement;
    pIndividualUpdate = (PVMDIR_REPLICATION_UPDATE) pNewElement;

    if (pIndividualUpdate->partnerUsn <= pCombinedUpdate->partnerUsn)
    {
        bResult = TRUE;
    }

    return bResult;
}

static
VOID
_VmDirReplicationUpdateTombStoneEntrySyncState(
    PVMDIR_REPLICATION_UPDATE pUpdate
    )
{
    DWORD               dwError = 0;
    PSTR                pszObjectGuid = NULL;
    PSTR                pszTempString = NULL;
    PSTR                pszContext = NULL;
    PSTR                pszDupDn = NULL;
    VDIR_ENTRY_ARRAY    entryArray = {0};
    VDIR_BERVALUE       bvParentDn = VDIR_BERVALUE_INIT;

    dwError = VmDirGetParentDN(&pUpdate->pEntry->dn, &bvParentDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (VmDirIsDeletedContainer(bvParentDn.lberbv_val) &&
        pUpdate->syncState == LDAP_SYNC_ADD)
    {
        dwError = VmDirAllocateStringOfLenA(pUpdate->pEntry->dn.lberbv_val,
                                            pUpdate->pEntry->dn.lberbv_len,
                                            &pszDupDn);
        BAIL_ON_VMDIR_ERROR(dwError);

        // Tombstone DN format: cn=<cn value>#objectGUID:<objectguid value>,<DeletedObjectsContainer>
        pszTempString = VmDirStringStrA(pszDupDn, "#objectGUID:");
        pszObjectGuid = VmDirStringTokA(pszTempString, ",", &pszContext);
        pszObjectGuid = pszObjectGuid + VmDirStringLenA("#objectGUID:");

        dwError = VmDirSimpleEqualFilterInternalSearch("", LDAP_SCOPE_SUBTREE, ATTR_OBJECT_GUID, pszObjectGuid, &entryArray);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (entryArray.iSize == 1)
        {
            pUpdate->syncState = LDAP_SYNC_DELETE;
            VMDIR_LOG_INFO(
                VMDIR_LOG_MASK_ALL,
                "%s: (tombstone handling) change sync state to delete: (%s)",
                __FUNCTION__,
                pUpdate->pEntry->dn.lberbv_val);
        }
    }

cleanup:
    VmDirFreeBervalContent(&bvParentDn);
    VMDIR_SAFE_FREE_MEMORY(pszDupDn);
    VmDirFreeEntryArrayContent(&entryArray);
    return;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s: error = (%d)",
            __FUNCTION__,
            dwError);
    goto cleanup;
}
