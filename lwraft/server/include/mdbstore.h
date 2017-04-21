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
    PVDIR_ENTRY pEntry
    );

DWORD
VmDirMDBSimpleDnToEntry(
    PSTR        pszObjectDn,
    PVDIR_ENTRY pEntry
    );

DWORD
VmDirMDBMaxEntryId(
    ENTRYID *   pEId
    );

DWORD
VmDirMDBGetNextUSN(
    PVDIR_BACKEND_CTX   pBECtx,
    USN *               usn
    );

DWORD
VmDirMDBAddEntry(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_ENTRY         pEntry
    );

DWORD
VmDirMDBCheckRefIntegrity(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_ENTRY         pEntry
    );

DWORD
VmDirMDBDeleteEntry(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_MODIFICATION  pMods,
    PVDIR_ENTRY         pEntry
    );

DWORD
VmDirMDBDNToEntry(
    PVDIR_BACKEND_CTX           pBECtx,
    PVDIR_SCHEMA_CTX            pSchemaCtx,
    VDIR_BERVALUE*              pDn,
    PVDIR_ENTRY                 pEntry,
    VDIR_BACKEND_ENTRY_LOCKTYPE entryLockType
    );

DWORD
VmDirMDBDNToEntryId(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_BERVALUE*      pDn,
    ENTRYID*            pEId
    );

DWORD
VmDirMDBObjectGUIDToEntryId(
    PVDIR_BACKEND_CTX   pBECtx,
    PCSTR               pszObjectGUID,
    ENTRYID*            pEId
    );

DWORD
VmDirMDBEIdToEntry(
    PVDIR_BACKEND_CTX           pBECtx,
    PVDIR_SCHEMA_CTX            pSchemaCtx,
    ENTRYID                     eId,
    PVDIR_ENTRY                 pEntry,
    VDIR_BACKEND_ENTRY_LOCKTYPE entryLockType
    );

DWORD
VmDirMDBModifyEntry(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_MODIFICATION*  pMods,
    PVDIR_ENTRY         pEntry
    );

// readutil.c
DWORD
VmDirMDBCheckIfALeafNode(
    PVDIR_BACKEND_CTX   pBECtx,
    ENTRYID             eId,
    PBOOLEAN            pIsLeafEntry
    );

DWORD
VmDirMDBGetAttrMetaData(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_ATTRIBUTE *    attr,
    ENTRYID             entryId
    );

DWORD
VmDirMDBGetAllAttrsMetaData(
    PVDIR_BACKEND_CTX           pBECtx,
    ENTRYID                     entryId,
    PATTRIBUTE_META_DATA_NODE * ppAttrMetaDataNode,
    int *                       pnumAttrMetaData
    );

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
    VOID
    );

DWORD
VmDirMDBInitializeDB (
    VOID
    );

DWORD
VmDirMDBShutdownDB(
    VOID
    );

DWORD
VmDirSetMdbBackendState(
    MDB_state_op        op,
    DWORD               *pdwLogNum,
    DWORD               *pdwDbSizeMb,
    DWORD               *pdwDbMapSizeMb,
    PSTR                pszDbPath,
    DWORD               dwDbPathSize
    );

int
VmDirRaftPrepareCommit(
    unsigned long long *pLogIndex,
    unsigned int *pLogTerm
    );

VOID
VmDirRaftPostCommit(
    unsigned long long logIndex,
    unsigned int logTerm
    );

VOID
VmDirRaftCommitFail(
    VOID
    );

// generic.c
DWORD
VmDirMDBDupKeyGetValues(
    PVDIR_BACKEND_CTX   pBECtx,
    PCSTR               pszKey,
    PVMDIR_STRING_LIST* ppStrList
    );

DWORD
VmDirMDBDupKeySetValues(
    PVDIR_BACKEND_CTX   pBECtx,
    PCSTR               pszKey,
    PVMDIR_STRING_LIST  pStrList
    );

DWORD
VmDirMDBUniqKeyGetValue(
    PVDIR_BACKEND_CTX   pBECtx,
    PCSTR               pszKey,
    PSTR*               ppszValue
    );

DWORD
VmDirMDBUniqKeySetValue(
    PVDIR_BACKEND_CTX   pBECtx,
    PCSTR               pszKey,
    PCSTR               pszValue
    );

// index.c
DWORD
VmDirMDBInitializeIndexDB(
    VOID
    );

VOID
VmDirMDBShutdownIndexDB(
    VOID
    );

DWORD
VmDirMDBIndexOpen(
    PVDIR_INDEX_CFG     pIndexCfg
    );

BOOLEAN
VmDirMDBIndexExist(
    PVDIR_INDEX_CFG     pIndexCfg
    );

DWORD
VmDirMDBIndexDelete(
    PVDIR_INDEX_CFG     pIndexCfg
    );

DWORD
VmDirMDBIndicesPopulate(
    PLW_HASHMAP pIndexCfgs,
    ENTRYID     startEntryId,
    DWORD       dwBatchSize
    );

// iterate.c
DWORD
VmDirMDBIndexIteratorInit(
    PVDIR_INDEX_CFG                 pIndexCfg,
    PSTR                            pszInitVal, // optional
    PVDIR_BACKEND_INDEX_ITERATOR*   ppIterator
    );

DWORD
VmDirMDBIndexIterate(
    PVDIR_BACKEND_INDEX_ITERATOR    pIterator,
    PSTR*                           ppszVal,
    ENTRYID*                        pEId
    );

VOID
VmDirMDBIndexIteratorFree(
    PVDIR_BACKEND_INDEX_ITERATOR    pIterator
    );

// config.c
DWORD
VmDirMDBConfigureFsync(
    BOOLEAN bFsyncOn
    );

#endif /* MDB_STORE_INTERFACE_H_ */
