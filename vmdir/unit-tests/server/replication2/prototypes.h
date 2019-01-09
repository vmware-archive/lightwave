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
#ifndef METADATA_H_
#define METADATA_H_

//////////////////
// updatelist.c
//////////////////
int
VmDirTestSetupReplUpdateListParseSyncDoneCtl(
    VOID    **state
    );

VOID
VmDirTestReplUpdateListParseSyncDoneCtl(
    VOID    **state
    );

int
VmDirTestTeardownReplUpdateListParseSyncDoneCtl(
    VOID    **state
    );

int
VmDirTestSetupUpdateListExpand_AddWithDelAttr(
    VOID    **state
    );

int
VmDirTestSetupUpdateListExpand_AddWithMultiValueAttr(
    VOID    **state
    );

int
VmDirTestSetupUpdateListExpand_AddWithDelValue(
    VOID    **state
    );

int
VmDirTestSetupUpdateListExpand_AddWithMustAttr(
    VOID    **state
    );

int
VmDirTestSetupUpdateListExpand_AddWithDelMustAttr(
    VOID    **state
    );

int
VmDirTestSetupUpdateListExpand_AddTombstone(
    VOID    **state
    );

int
VmDirTestSetupUpdateListExpand_AddWithModifyAndTombstone(
    VOID    **state
    );

VOID
VmDirTestReplUpdateListExpand(
    VOID    **state
    );

int
VmDirTestTeardownUpdateListExpand(
    VOID    **state
    );

//////////////////
// update.c
//////////////////
VOID
VmDirReplUpdateToUSNList_NoDupMetaDataOnly(
    VOID**    state
    );
VOID
VmDirReplUpdateToUSNList_DupMetaDataOnly(
    VOID**    state
    );
VOID
VmDirReplUpdateToUSNList_MetaDataOnly(
    VOID**    state
    );
VOID
VmDirReplUpdateToUSNList_ValueMetaDataOnly(
    VOID**    state
    );

VOID
VmDirReplUpdateToUSNList_MetaDataAndValueMetaData(
    VOID**    state
    );

VOID
VmDirReplUpdateToUSNList_MetaDataAndValueMetaDataSame(
    VOID**    state
    );

VOID
VmDirReplUpdateLocalUsn_ValidInput(
    VOID**    state
    );

//////////////////
// extractevents.c
//////////////////
int
VmDirSetupExtractEventAttributeChanges(
    VOID    **state
    );

VOID
VmDirExtractEventAttributeChanges_ValidInput(
    VOID    **state
    );

int
VmDirSetupExtractEventAttributeValueChanges(
    VOID    **state
    );

VOID
VmDirExtractEventAttributeValueChanges_ValidInput(
    VOID    **state
    );

int
VmDirTestSetupExtractEventPopulateMustAttributes(
    VOID    **state
    );

VOID
VmDirTestExtractEventPopulateMustAttributes_ValidInput(
    VOID    **state
    );

int
VmDirSetupExtractEventPopulateOperationAttributes(
    VOID    **state
    );

VOID
VmDirExtractEventPopulateOperationAttributes_ValidInput(
    VOID    **state
    );

int
VmDirTeardownExtractEvent(
    VOID    **state
    );

int VmDirTestTeardownExtractEventMustAttr(
    VOID    **state
    );

/////////////////////
// testcommon.c
////////////////////
PVMDIR_REPL_ATTRIBUTE_METADATA
VmDirFindAttrMetaData(
    PVDIR_LINKED_LIST    pList,
    PSTR                 pszAttrType
    );

VOID
VmDirAllocateAttrMetaData(
    PSTR                         pszAttrType,
    USN                          localUsn,
    USN                          origUsn,
    UINT64                       version,
    PCSTR                        pcszOrigInvoId,
    PVMDIR_REPLICATION_UPDATE    pUpdate
    );

VOID
VmDirAllocateAttrAndMetaData(
    PSTR                         pszAttrType,
    PSTR                         pszValue,
    USN                          localUsn,
    USN                          origUsn,
    UINT64                       version,
    PCSTR                        pcszOrigInvoId,
    PVMDIR_REPLICATION_UPDATE    pUpdate
    );

VOID
VmDirAllocateAttrValueMetaData(
    PSTR                         pszValueMetaData,
    PVMDIR_REPLICATION_UPDATE    pUpdate
    );

VOID
VmDirAllocateMultiValueAttr(
    PSTR                         pszAttrType,
    USN                          localUSN,
    USN                          origUSN,
    PCSTR                        pcszOrigInvoId,
    VDIR_BERVARRAY               pbvVals,
    DWORD                        dwVals,
    PVMDIR_REPLICATION_UPDATE    pUpdate
    );

/*
 * TODO: See if these comparison functions can be moved to the main code instead of testing code,
 * return int instead of bool and not use assert
 * */
BOOLEAN
VmDirTestCompareReplUpdate(
    PVMDIR_REPLICATION_UPDATE    pUpdate1,
    PVMDIR_REPLICATION_UPDATE    pUpdate2
    );

BOOLEAN
VmDirTestCompareMetaDataList(
    PVDIR_LINKED_LIST pList1,
    PVDIR_LINKED_LIST pList2
    );

BOOLEAN
VmDirTestCompareMetaData(
    PVMDIR_REPL_ATTRIBUTE_METADATA  pReplMetadata1,
    PVMDIR_REPL_ATTRIBUTE_METADATA  pReplMetadata2
    );

BOOLEAN
VmDirTestCompareEntry(
    PVDIR_ENTRY pEntry1,
    PVDIR_ENTRY pEntry2
    );

BOOLEAN
VmDirTestCompareAttr(
    PVDIR_ATTRIBUTE pAttr1,
    PVDIR_ATTRIBUTE pAttr2
    );

DWORD
VmDirTestAllocateReplUpdate(
    PVMDIR_REPLICATION_UPDATE   *ppReplUpdate
    );

BOOLEAN
VmDirTestCompareReplUpdateList(
    PVMDIR_REPLICATION_UPDATE_LIST    pReplUpdateList1,
    PVMDIR_REPLICATION_UPDATE_LIST    pReplUpdateList2
    );
#endif
