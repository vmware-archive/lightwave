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



#ifndef MDB_STORE_INTERFACE_H_
#define MDB_STORE_INTERFACE_H_

// entry.c
DWORD
VmDirMDBSimpleEIdToEntry(
    ENTRYID     eId,
    PVDIR_ENTRY      pEntry);

DWORD
VmDirMDBSimpleDnToEntry(
    PSTR        pszObjectDn,
    PVDIR_ENTRY pEntry
    );

DWORD
VmDirMDBMaxEntryId(
    PVDIR_BACKEND_CTX   pBECtx,
    ENTRYID *           pEId);

DWORD
VmDirMDBGetNextUSN(
    PVDIR_BACKEND_CTX   pBECtx,
    USN *               usn);

DWORD
VmDirMDBAddEntry(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_ENTRY         pEntry);

DWORD
VmDirMDBCheckRefIntegrity(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_ENTRY         pEntry);

DWORD
VmDirMDBDeleteEntry(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_MODIFICATION  pMods,
    PVDIR_ENTRY         pEntry);

DWORD
VmDirMDBDNToEntry(
    PVDIR_BACKEND_CTX           pBECtx,
    PVDIR_SCHEMA_CTX            pSchemaCtx,
    VDIR_BERVALUE*              pDn,
    PVDIR_ENTRY                 pEntry,
    VDIR_BACKEND_ENTRY_LOCKTYPE entryLockType);

DWORD
VmDirMDBDNToEntryId(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_BERVALUE*      pDn,
    ENTRYID*            pEId);

DWORD
VmDirMDBEIdToEntry(
    PVDIR_BACKEND_CTX           pBECtx,
    PVDIR_SCHEMA_CTX            pSchemaCtx,
    ENTRYID                     eId,
    PVDIR_ENTRY                 pEntry,
    VDIR_BACKEND_ENTRY_LOCKTYPE entryLockType);

DWORD
VmDirMDBModifyEntry(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_MODIFICATION*  pMods,
    PVDIR_ENTRY         pEntry);

// indexer.c
DWORD
VmDirMDBCheckIfALeafNode(
    PVDIR_BACKEND_CTX   pBECtx,
    ENTRYID             eId,
    PBOOLEAN            pIsLeafEntry);

DWORD
VmDirMDBGetAttrMetaData(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_ATTRIBUTE *    attr,
    ENTRYID             entryId);

DWORD
VmDirMDBGetAllAttrsMetaData(
    PVDIR_BACKEND_CTX           pBECtx,
    ENTRYID                     entryId,
    PATTRIBUTE_META_DATA_NODE * ppAttrMetaDataNode,
    int *                       pnumAttrMetaData
    );

DWORD
VmDirMDBGetCandidates(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_FILTER*        pFilter);

// txn.c
DWORD
VmDirMDBTxnBegin(
    PVDIR_BACKEND_CTX        pBECtx,
    VDIR_BACKEND_TXN_MODE    txnMode
    );

DWORD
VmDirMDBTxnAbort(
    PVDIR_BACKEND_CTX   pBECtx
    );

DWORD
VmDirMDBTxnCommit(
    PVDIR_BACKEND_CTX   pBECtx
    );

// init.c
PVDIR_BACKEND_INTERFACE
VmDirMDBBEInterface (
    VOID);

DWORD
VmDirMDBBEInit ( // beInit
    VOID);

DWORD
VmDirMDBInitializeDB ( // DBOpen
    VOID);

DWORD
VmDirMDBInitializeIndexDB( // IndexOpen
    VOID);

DWORD
VmDirMDBShutdownDB( // Shutdown
    VOID);

DWORD
VmDirMDBGlobalIndexStructAdd( // MDBIndexAdd
    PVDIR_CFG_ATTR_INDEX_DESC   pIndexDesc);

// attrindexing.c

/*
 * Create new attribute indices for a batch of entries
 */
DWORD
VmDirMDBIndicesCreate(
    PVDIR_CFG_ATTR_INDEX_DESC*  ppIndexDesc,
    DWORD                       dwNumIndices,
    DWORD                       dwStartEntryId,
    DWORD                       dwBatchSize);

DWORD
VmDirSetMdbBackendState(
    DWORD               dwFileTransferState,
    DWORD               *pdwLogNum,
    DWORD               *pdwDbSizeMb,
    DWORD               *pdwDbMapSizeMb,
    char                *pDbPath,
    DWORD               dwDbPathSize);
#endif /* MDB_STORE_INTERFACE_H_ */
