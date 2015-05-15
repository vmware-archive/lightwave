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



#ifndef BDB_H_
#define BDB_H_

// TODO, could make this configurable
#define BDB_MAX_INDEX_ATTRIBUTE     256

#define BDB_FLAGS_ZERO            0
#define BDB_FILE_MODE_ZERO        0
#define BDB_TXN_NULL              NULL
#define BDB_PARENT_TXN_NULL       NULL

#define INDEX_KEY_TYPE_FWD     '0'
#define INDEX_KEY_TYPE_REV     '1'

#define MAX_DB_NAME_LEN         100

// candidates.c
void
AddToCandidates(
    VDIR_CANDIDATES * cans,
    db_seq_t     eId);

// config.c
extern DWORD
InitBdbConfig();

// indexer.c

int
CreateEntryIdIndex(
    DB_TXN *    txn,
    db_seq_t    eId,
    VDIR_BERVALUE *  encodedEntry,
    BOOLEAN     new);

int
CreateId2ChildrenIndex(
    DB_TXN *    txn,
    VDIR_BERVALUE *  pdn,
    db_seq_t    entryId);

int
DeleteEntryIdIndex(
    DB_TXN *    txn,
    db_seq_t    entryId);

int
DeleteParentIdIndex(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_BERVALUE *          pdn,
    db_seq_t            entryId);

int
UpdateAttributeMetaData(
    DB_TXN *    txn,
    VDIR_ATTRIBUTE * attr,
    db_seq_t    entryId,
    ULONG       ulOPMask);

extern int
UpdateIndicesForAttribute(
    DB_TXN *    txn,
    VDIR_BERVALUE *  attrType,
    VDIR_BERVARRAY   attrVals, // Normalized Attribute Values
    unsigned    numVals,
    db_seq_t    entryId,
    ULONG       ulOPMask
    );

extern int
CreateParentIdIndex(
    PVDIR_BACKEND_CTX   pBECtx,
    VDIR_BERVALUE *          pdn,
    db_seq_t            entryId);

extern void
EntryIdToDBT(
    db_seq_t eId,
    DBT *    dbt);

// logmgmt.c
DWORD
InitializeDbChkpointThread(
    VOID
    );

// init.c
DWORD
BdbToBackendError(
    DWORD   dwBdbError,
    DWORD   dwFromBdbError,
    DWORD   dwToBEError,
    PVDIR_BACKEND_CTX   pBECtx);

#endif /* BDB_H_ */
