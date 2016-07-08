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



#ifndef MDB_H_
#define MDB_H_


// config.c
extern DWORD
MDBInitConfig();

// indexer.c

DWORD
MdbCreateEIDIndex(
    PVDIR_DB_TXN     pTxn,
    ENTRYID          eId,
    VDIR_BERVALUE *  encodedEntry,
    BOOLEAN          bIsNewIndex);

DWORD
MDBDeleteParentIdIndex(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_BERVALUE *     pdn,
    ENTRYID             entryId);

DWORD
MdbUpdateAttrMetaData(
    PVDIR_DB_TXN     pTxn,
    VDIR_ATTRIBUTE * attr,
    ENTRYID          entryId,
    ULONG            ulOPMask);

DWORD
MdbUpdateIndicesForAttr(
    PVDIR_DB_TXN        txn,
    VDIR_BERVALUE *     attrType,
    VDIR_BERVARRAY      attrVals, // Normalized Attribute Values
    unsigned            numVals,
    ENTRYID             entryId,
    ULONG               ulOPMask
    );

DWORD
MDBCreateParentIdIndex(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_BERVALUE *     pdn,
    ENTRYID             entryId);

void
MDBEntryIdToDBT(
    ENTRYID         eId,
    PVDIR_DB_DBT    pDBT);

DWORD
VmDirMDBGetCandidates(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_FILTER*        pFilter,
    ENTRYID             eStartingId
    );

// init.c
DWORD
MDBToBackendError(
    DWORD               dwMDBError,
    DWORD               dwFromMDBError,
    DWORD               dwToBEError,
    PVDIR_BACKEND_CTX   pBECtx,
    PCSTR               pszErrorContext);

//void
//MDBCloseSequences(
//    VOID);

#endif /* MDB_H_ */
