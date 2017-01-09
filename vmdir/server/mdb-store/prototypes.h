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
DWORD
MDBInitConfig();

// desc.c
DWORD
MdbIndexDescInit(
    PVDIR_INDEX_CFG             pIndexCfg,
    PVDIR_MDB_INDEX_DATABASE*   ppMdbIndexDB
    );

VOID
MdbIndexDescFree(
    PVDIR_MDB_INDEX_DATABASE    pMdbIndexDB
    );

// index.c
DWORD
VmDirMDBIndexGetDBi(
    PVDIR_INDEX_CFG pIndexCfg,
    VDIR_DB*        pDBi
    );

// writeutil.c
DWORD
MdbUpdateKeyValue(
   VDIR_DB             mdbDBi,
   PVDIR_DB_TXN        pTxn,
   PVDIR_DB_DBT        pKey,
   PVDIR_DB_DBT        pValue,
   BOOLEAN             bIsUniqueVal,
   ULONG               ulOPMask);

DWORD
MdbDeleteKeyValue(
    VDIR_DB             mdbDBi,
    PVDIR_DB_TXN        pTxn,
    PVDIR_DB_DBT        pKey,
    PVDIR_DB_DBT        pValue,
    BOOLEAN             bIsUniqueVal);

DWORD
MdbCreateEIDIndex(
    PVDIR_DB_TXN     pTxn,
    ENTRYID          eId,
    VDIR_BERVALUE *  encodedEntry,
    BOOLEAN          bIsNewIndex);

DWORD
MDBDeleteEIdIndex(
    PVDIR_DB_TXN    pTxn,
    ENTRYID         entryId);

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
    PVDIR_DB_TXN        pTxn,
    VDIR_BERVALUE *     entryDN,
    VDIR_BERVALUE *     attrType,
    VDIR_BERVARRAY      attrVals, // Normalized Attribute Values
    unsigned            numVals,
    ENTRYID             entryId,
    ULONG               ulOPMask
    );

DWORD
MdbValidateAttrUniqueness(
    PVDIR_INDEX_CFG     pIndexCfg,
    PSTR                pszAttrVal,
    PSTR                pszEntryDn,
    ULONG               ulOPMask
    );

DWORD
MDBCreateParentIdIndex(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_BERVALUE *     pdn,
    ENTRYID             entryId);

// readutil.c
void
MDBEntryIdToDBT(
    ENTRYID         eId,
    PVDIR_DB_DBT    pDBT);

void
MDBDBTToEntryId(
    PVDIR_DB_DBT    pDBT,
    ENTRYID*        pEID);

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

DWORD
MDBOpenDB(
    PVDIR_DB            pmdbDBi,
    const char *        dbName,
    const char *        fileName,
    PFN_BT_KEY_CMP      btKeyCmpFcn,
    unsigned int        extraFlags);

VOID
MDBCloseDB(
    VDIR_DB    mdbDBi
    );

DWORD
MDBDropDB(
    VDIR_DB    mdbDBi
    );

#endif /* MDB_H_ */
