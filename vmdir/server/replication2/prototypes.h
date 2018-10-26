/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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
 * Module Name: Directory indexer
 *
 * Filename: prototypes.h
 *
 * Abstract:
 *
 * Function prototypes
 *
 */

#ifndef _CFG_PROTOTYPES_H_
#define _CFG_PROTOTYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

// thread.c

DWORD
InitializeReplicationThread(
    void);

int
VmDirFirstReplicationCycle(
    PCSTR                           pszHostname,
    VMDIR_REPLICATION_AGREEMENT*    pReplAgr
    );

DWORD
VmDirCacheKrb5Creds(
    PCSTR   pszUPN,
    PCSTR   pszPwd,
    PSTR*   ppszErrorMsg
    );

// replentry.c
int
ReplAddEntry(
    PVMDIR_REPLICATION_UPDATE   pUpdate
    );

int
ReplDeleteEntry(
    PVMDIR_REPLICATION_UPDATE   pUpdate
    );

int
ReplModifyEntry(
    PVMDIR_REPLICATION_UPDATE   pUpdate
    );

// metrics.c
DWORD
VmDirReplMetricsInit(
    PSTR                        pszHostname,
    PSTR                        pszSite,
    PVMDIR_REPLICATION_METRICS* ppReplMetrics
    );

VOID
VmDirFreeReplMetrics(
    PVMDIR_REPLICATION_METRICS  pReplMetrics
    );

// firstreplcycle.c
VOID
VmDirShutdownDB(
    VOID
    );

int
VmDirPatchDSERoot(
    PVDIR_SCHEMA_CTX    pSchemaCtx
    );

// replcookies.c
int
VmDirReplCookieUpdate(
    PVDIR_SCHEMA_CTX                pSchemaCtx,
    USN                             lastUsnProcessed,
    PVMDIR_UTDVECTOR_CACHE          pUtdVector,
    VMDIR_REPLICATION_AGREEMENT *   replAgr
    );

// dbswap.c
DWORD
VmDirSwapDB(
    PCSTR   dbHomeDir
    );

DWORD
VmDirPrepareSwapDBInfo(
    PCSTR                   pszHostName,    // partner server object cn
    PVMDIR_SWAP_DB_INFO*    ppSwapDBInfo
    );

VOID
VmDirFreeSwapDBInfo(
    PVMDIR_SWAP_DB_INFO pSwapDBInfo
    );

//consume.c
VOID
VmDirConsumePartner(
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr
    );

// dbcopy.c
int
VmDirCopyRemoteDB(
    PCSTR   pszHostname,
    PCSTR   dbHomeDir
    );

// metadata.c
DWORD
VmDirReplGetAttrMetaDataList(
    PVDIR_ENTRY           pEntry,
    PVDIR_LINKED_LIST*    ppMetaDataList
    );

DWORD
VmDirReplSetAttrNewMetaData(
    PVDIR_OPERATION     pOperation,
    PVDIR_ENTRY         pSupplierEntry,
    PLW_HASHMAP         pMetaDataMap
    );

// conflictresolution.c
int
VmDirReplResolveConflicts(
    PVDIR_OPERATION     pOperation,
    PVDIR_ENTRY         pSupplierEntry,
    PLW_HASHMAP         pMetaDataMap
    );

//updatelist.c
DWORD
VmDirReplUpdateListAlloc(
    PVMDIR_REPLICATION_UPDATE_LIST* ppReplUpdateList
    );

int
VmDirReplUpdateListFetch(
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr,
    PVMDIR_REPLICATION_UPDATE_LIST* ppReplUpdateList
    );

VOID
VmDirReplUpdateListApply(
    PVMDIR_REPLICATION_UPDATE_LIST  pReplUpdateList
    );

DWORD
VmDirReplUpdateListExpand(
    PVMDIR_REPLICATION_UPDATE_LIST    pReplUpdateList
    );

VOID
VmDirFreeReplUpdateList(
    PVMDIR_REPLICATION_UPDATE_LIST  pUpdateList
    );

int
VmDirReplUpdateListParseSyncDoneCtl(
    PVMDIR_REPLICATION_UPDATE_LIST  pReplUpdateList,
    LDAPControl**                   ppSearchResCtrls
    );

VOID
VmDirFreeReplUpdateListLinkedList(
    PVDIR_LINKED_LIST    pUpdateLinkedList
    );

//update.c
int
VmDirReplUpdateCreate(
    LDAP*                           pLd,
    LDAPMessage*                    pEntry,
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr,
    PVMDIR_REPLICATION_UPDATE*      ppUpdate
    );

VOID
VmDirReplUpdateApply(
    PVMDIR_REPLICATION_UPDATE   pReplUpdate
    );

VOID
VmDirFreeReplUpdate(
    PVMDIR_REPLICATION_UPDATE   pUpdate
    );

DWORD
VmDirReplUpdateToUSNList(
    PVMDIR_REPLICATION_UPDATE   pUpdate,
    PVDIR_LINKED_LIST*          ppUSNList
    );

DWORD
VmDirReplUpdateSplit(
    PVMDIR_REPLICATION_UPDATE    pUpdate,
    PVDIR_SORTED_LINKED_LIST     pNewReplUpdateList
    );

DWORD
VmDirReplUpdateLocalUsn(
    PVMDIR_REPLICATION_UPDATE    pUpdate,
    PVDIR_LINKED_LIST            pUSNList
    );

DWORD
VmDirReplUpdateExtractEvent(
    PVMDIR_REPLICATION_UPDATE    pCombinedUpdate,
    USN                          usn,
    PVMDIR_REPLICATION_UPDATE*   ppIndividualUpdate
    );

BOOLEAN
VmDirSortedLinkedListInsertCompareReplUpdate(
    PVOID    pElement,
    PVOID    pNewElement
    );

//utdvector.c
DWORD
VmDirUTDVectorCacheInit(
    PVMDIR_UTDVECTOR_CACHE* ppUTDVector
    );

DWORD
VmDirUTDVectorCacheReplace(
    PVMDIR_UTDVECTOR_CACHE  pUTDVector,
    PCSTR                   pszNewUTDVector
    );

DWORD
VmDirUTDVectorCacheToString(
    PVMDIR_UTDVECTOR_CACHE  pUTDVector,
    PSTR*                   ppszUTDVector
    );

DWORD
VmDirUTDVectorCacheLookup(
    PVMDIR_UTDVECTOR_CACHE  pUTDVector,
    PCSTR                   pszInvocationId,
    USN*                    pUsn
    );

DWORD
VmDirUTDVectorCacheAdd(
    PVMDIR_UTDVECTOR_CACHE  pUTDVector,
    PCSTR                   pszInvocationId,
    PCSTR                   pszUsn
    );

VOID
VmDirFreeUTDVectorCache(
    PVMDIR_UTDVECTOR_CACHE  pUTDVector
    );

DWORD
VmDirUTDVectorUpdateNew(
    PVMDIR_UTDVECTOR_CACHE    pNewUTDVector
    );

//valuemetadata.c
DWORD
VmDirReplGetAttrValueMetaDataList(
    PVDIR_ENTRY           pEntry,
    PVDIR_LINKED_LIST*    ppValueMetaDataList
    );

DWORD
VmDirValueMetaDataUpdateLocalUsn(
    PVDIR_ENTRY    pEntry,
    USN            localUsn,
    PDEQUE         pValueMetaDataQueue
    );

DWORD
VmDirValueMetaDataDeleteOldForReplace(
    PVDIR_OPERATION       pModOp,
    PVDIR_MODIFICATION    pMods,
    ENTRYID               entryId
    );

DWORD
VmDirReplSetAttrNewValueMetaData(
    PDEQUE              pValueMetaDataQueue,
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    ENTRYID             entryId,
    USN                 localUsn,
    PVDIR_OPERATION     pModOp
    );

//conflictresolution.c
DWORD
VmDirReplResolveValueMetaDataConflicts(
    PVDIR_OPERATION                    pModOp,
    PVDIR_ATTRIBUTE                    pAttr,
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pSupplierValueMetaData,
    ENTRYID                            entryId,
    PBOOLEAN                           pInScope
    );

//extractevents.c
DWORD
VmDirExtractEventAttributeChanges(
    PVMDIR_REPLICATION_UPDATE    pCombinedUpdate,
    USN                          usn,
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate
    );

DWORD
VmDirExtractEventAttributeValueChanges(
    PVMDIR_REPLICATION_UPDATE    pCombinedUpdate,
    USN                          usn,
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate
    );

DWORD
VmDirExtractEventPopulateMustAttributes(
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate,
    PVMDIR_REPLICATION_UPDATE    pCombinedUpdate
    );

DWORD
VmDirExtractEventPopulateOperationAttributes(
    PVMDIR_REPLICATION_UPDATE    pCombinedUpdate,
    PVMDIR_REPLICATION_UPDATE    pIndividualUpdate
    );

#ifdef __cplusplus
}
#endif

#endif // _CFG_PROTOTYPES_H_

