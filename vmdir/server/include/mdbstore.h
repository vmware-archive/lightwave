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

#define MDB_MAIN_DB     "data.mdb"

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

DWORD
VmDirMDBGetAttrValueMetaData(
    PVDIR_BACKEND_CTX   pBECtx,
    ENTRYID             entryId,
    short               attrId,
    PDEQUE              metaValueData
    );

DWORD
VmDirMDBGetAllAttrValueMetaData(
    PVDIR_BACKEND_CTX   pBECtx,
    ENTRYID             entryId,
    PDEQUE              metaValueData
    );

DWORD
VmDirMDBIndexTableReadRecord(
    PVDIR_BACKEND_CTX       pBECtx,
    VDIR_BACKEND_KEY_ORDER  keyOrder,
    PCSTR                   pszIndexName,
    PVDIR_BERVALUE          pBVKey,  // normalize key
    PVDIR_BERVALUE          pBVValue // output copy of value
    );

DWORD
VmDirBVToEntryId(
    PVDIR_BERVALUE  pBV,
    ENTRYID*        pEID
    );

DWORD
VmDirEntryIdToBV(
    ENTRYID         eId,
    PVDIR_BERVALUE  pBV
    );

// writeutil.c
DWORD
VmDirMdbUpdateAttrValueMetaData(
    PVDIR_BACKEND_CTX   pBECtx,
    ENTRYID             entryId,
    short               attrId,
    ULONG               ulOPMask,
    PDEQUE              pValueMetaDataQueue
    );

DWORD
VmDirMdbDeleteAllAttrValueMetaData(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    ENTRYID             entryId
    );

DWORD
VmDirMdbApplyIndicesNewMR(
    VOID
    );

DWORD
VmDirMDBBackendTableWriteRecord(
    PVDIR_BACKEND_CTX       pBECtx,
    VDIR_BACKEND_RECORD_WRITE_TYPE  opType,
    PCSTR                   pszTableName,
    PVDIR_BERVALUE          pBVKey,     // normalize key
    PVDIR_BERVALUE          pBVValue,   // existing value (update/delete)
    PVDIR_BERVALUE          pNewBVValue // new value (create/update)
    );

DWORD
VmDirMDBIndexTableWriteRecord(
    PVDIR_BACKEND_CTX       pBECtx,
    VDIR_BACKEND_RECORD_WRITE_TYPE  opType,
    PVDIR_BERVALUE          pBVDN,
    PCSTR                   pszIndexName,
    PVDIR_BERVALUE          pBVCurrentKey,  // current normalize key
    PVDIR_BERVALUE          pBVNewKey,      // new normalize key
    PVDIR_BERVALUE          pBVEID          // entry id
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
VmDirInitMdbStateGlobals(
    VOID
    );

VOID
VmDirFreeMdbStateGlobals(
    VOID
    );

DWORD
VmDirSetMdbBackendState(
    DWORD               dwFileTransferState,
    DWORD               *pdwLogNum,
    DWORD               *pdwDbSizeMb,
    DWORD               *pdwDbMapSizeMb,
    PSTR                pszDbPath,
    DWORD               dwDbPathSize
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

DWORD
VmDirMDBGetAllDBNames(
    PVMDIR_STRING_LIST*    ppDBList
    );

DWORD
VmDirMDBGetDBKeysCount(
    PSTR     pszDBName,
    PDWORD   pdwKeysCount
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

// this should be move to server/common to avoid layer violation
DWORD
VmDirMDBIndexIteratorInitKey(
    PVDIR_ITERATOR_CONTEXT  pIteratorContext,
    PCSTR                   pszInit
    );

// this should be move to server/common to avoid layer violation
DWORD
VmDirMDBIndexIteratorInitParentIdKey(
    PVDIR_ITERATOR_CONTEXT  pIteratorContext,
    PCSTR                   pszNormValue
    );

DWORD
VmDirMDBIndexIteratorInit(
    PVDIR_INDEX_CFG                  pIndexCfg,
    PVDIR_ITERATOR_CONTEXT           pIterContext,
    PVDIR_BACKEND_INDEX_ITERATOR*    ppIterator
    );

DWORD
VmDirMDBIndexIterate(
    PVDIR_BACKEND_INDEX_ITERATOR    pIterator,
    PVDIR_ITERATOR_CONTEXT          pIterContext
    );

VOID
VmDirMDBIndexIteratorFree(
    PVDIR_BACKEND_INDEX_ITERATOR    pIterator
    );

DWORD
VmDirMDBParentIdIndexIteratorInit(
    ENTRYID                                 parentId,
    ENTRYID                                 childId,
    PVDIR_BACKEND_PARENT_ID_INDEX_ITERATOR* ppIterator
    );

DWORD
VmDirMDBParentIdIndexIterate(
    PVDIR_BACKEND_PARENT_ID_INDEX_ITERATOR  pIterator,
    ENTRYID*                                pEntryId
    );

VOID
VmDirMDBParentIdIndexIteratorFree(
    PVDIR_BACKEND_PARENT_ID_INDEX_ITERATOR  pIterator
    );

DWORD
VmDirMDBEntryBlobIteratorInit(
    ENTRYID                                 EId,
    PVDIR_BACKEND_ENTRYBLOB_ITERATOR*       ppIterator
    );

DWORD
VmDirMDBEntryBlobIterate(
    PVDIR_BACKEND_ENTRYBLOB_ITERATOR    pIterator,
    ENTRYID*                            pEntryId
    );

VOID
VmDirMDBEntryBlobIteratorFree(
    PVDIR_BACKEND_ENTRYBLOB_ITERATOR  pIterator
    );

// config.c
DWORD
VmDirMDBConfigureFsync(
    BOOLEAN bFsyncOn
    );

VOID
VmDirMdbErrorLog(
    int errnum,
    int param,
    const char *funname,
    int lineno
    );

#endif /* MDB_STORE_INTERFACE_H_ */
