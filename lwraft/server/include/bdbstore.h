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



#ifndef BDB_STORE_INTERFACE_H_
#define BDB_STORE_INTERFACE_H_

// entry.c
DWORD
BDBSimpleEIdToEntry(
    ENTRYID     eId,
    PVDIR_ENTRY      pEntry);

DWORD
BDBSimpleDnToEntry(
    PSTR        pszObjectDn,
    PVDIR_ENTRY pEntry
    );

DWORD
BdbMaxEntryId(
    PVDIR_BACKEND_CTX   pBECtx,
    ENTRYID *           pEId);

DWORD
BdbGetNextUSN(
    PVDIR_BACKEND_CTX   pBECtx,
    USN *               usn);

DWORD
BdbAddEntry(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_ENTRY         pEntry);

DWORD
BdbCheckRefIntegrity(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_ENTRY         pEntry);

DWORD
BdbDeleteEntry(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_MODIFICATION  pMods,
    PVDIR_ENTRY         pEntry);

DWORD
BdbDNToEntry(
    PVDIR_BACKEND_CTX           pBECtx,
    PVDIR_SCHEMA_CTX            pSchemaCtx,
    VDIR_BERVALUE*              pDn,
    PVDIR_ENTRY                 pEntry,
    VDIR_BACKEND_ENTRY_LOCKTYPE entryLockType);

DWORD
BdbDNToEntryId(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_BERVALUE*      pDn,
    ENTRYID*            pEId);

DWORD
BdbEIdToEntry(
    PVDIR_BACKEND_CTX           pBECtx,
    PVDIR_SCHEMA_CTX            pSchemaCtx,
    ENTRYID                     eId,
    PVDIR_ENTRY                 pEntry,
    VDIR_BACKEND_ENTRY_LOCKTYPE entryLockType);

DWORD
BdbModifyEntry(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_MODIFICATION*  pMods,
    PVDIR_ENTRY         pEntry);

// indexer.c
DWORD
BdbCheckIfALeafNode(
    PVDIR_BACKEND_CTX   pBECtx,
    ENTRYID             eId,
    PBOOLEAN            pIsLeafEntry);

DWORD
BdbGetAttrMetaData(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_ATTRIBUTE *    attr,
    ENTRYID             entryId);

DWORD
BdbGetAllAttrsMetaData(
    PVDIR_BACKEND_CTX           pBECtx,
    ENTRYID                     entryId,
    PATTRIBUTE_META_DATA_NODE * ppAttrMetaDataNode,
    int *                       numAttrMetaData
    );

DWORD
BdbGetCandidates(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_FILTER*        pFilter);

// txn.c
DWORD
BdbTxnBegin(
    PVDIR_BACKEND_CTX        pBECtx,
    VDIR_BACKEND_TXN_MODE    txnMode
    );

DWORD
BdbTxnAbort(
    PVDIR_BACKEND_CTX   pBECtx
    );

DWORD
BdbTxnCommit(
    PVDIR_BACKEND_CTX   pBECtx
    );

// init.c
PVDIR_BACKEND_INTERFACE
BdbBEInterface (
    VOID);

DWORD
BdbBEInit ( // beInit
    VOID);

DWORD
InitializeBDB ( // BdbDBOpen
    VOID);

DWORD
InitializeBDBIndexDB( // BdbIndexOpen
    VOID);

DWORD
ShutdownBDB( // BdbShutdown
    VOID);

DWORD
VmDirBDBGlobalIndexStructAdd( // BdbIndexAdd
    PVDIR_CFG_ATTR_INDEX_DESC   pIndexDesc);

// attrindexing.c

/*
 * Create new attribute indices for a batch of entries
 */
DWORD
VmDirBdbIndicesCreate(
    PVDIR_CFG_ATTR_INDEX_DESC*  ppIndexDesc,
    DWORD   dwNumIndices,
    DWORD   dwStartEntryId,
    DWORD   dwBatchSize);

#endif /* BDB_STORE_INTERFACE_H_ */
