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

typedef struct _VDIR_CFG_MDB_DATAFILE_DESC
{
    MDB_dbi     mdbDBi;
    PSTR        pszDBName;
    PSTR        pszDBFile;
    BOOLEAN     bIsUnique;

} VDIR_CFG_MDB_DATAFILE_DESC, *PVDIR_CFG_MDB_DATAFILE_DESC;

typedef MDB_cmp_func    PFN_BT_KEY_CMP;

typedef struct _VDIR_MDB_ENTRY_DATABASE
{
    USHORT                      usNumDataFiles;
    PVDIR_CFG_MDB_DATAFILE_DESC pMdbDataFiles;

    // Btree key comparison function
    PFN_BT_KEY_CMP*             btKeyCmpFcn;

} VDIR_MDB_ENTRY_DATABASE, *PVDIR_MDB_ENTRY_DATABASE;

typedef struct _VDIR_MDB_INDEX_DATABASE
{
    PSTR                        pszAttrName;    // Used as key in mdbIndexDBs
    USHORT                      usNumDataFiles;

    // Array of MDBDataFiles indexed by VDIR_CFG_ATTR_INDEX_DESC.iId
    PVDIR_CFG_MDB_DATAFILE_DESC pMdbDataFiles;

    // Btree key comparison function
    PFN_BT_KEY_CMP*             btKeyCmpFcn;

} VDIR_MDB_INDEX_DATABASE, *PVDIR_MDB_INDEX_DATABASE;

typedef struct _VDIR_MDB_INDEX_ITERATOR
{
    PVDIR_DB_TXN    pTxn;
    PVDIR_DB_DBC    pCursor;
    PSTR            pszVal;
    ENTRYID         eId;
    BOOLEAN         bAbort;
    BOOLEAN         bHasTxn;
    PVDIR_BACKEND_CTX pBECtx;
} VDIR_MDB_INDEX_ITERATOR, *PVDIR_MDB_INDEX_ITERATOR;

typedef struct _VDIR_MDB_DB
{
    // NOTE: order of fields MUST stay in sync with struct initializer...
    VDIR_MDB_ENTRY_DATABASE         mdbEntryDB;
    PLW_HASHMAP                     mdbIndexDBs;
    MDB_env *                       mdbEnv;
    MDB_dbi                         mdbSeqDBi;
    MDB_dbi                         mdbGenericDupKeyDBi;
    MDB_dbi                         mdbGenericUniqKeyDBi;
    PSTR                            pszDBPath;
    BOOLEAN                         bIsMainDB;
    struct _VDIR_MDB_DB *           pNext;
}VDIR_MDB_DB, *PVDIR_MDB_DB;

typedef struct _VDIR_MDB_GLOBALS
{
    PVDIR_MDB_DB                 pDBs;
} VDIR_MDB_GLOBALS, *PVDIR_MDB_GLOBALS;

typedef struct _VDIR_MDB_STATE_GLOBALS
{
    PVMDIR_MUTEX    pMutex;
    PLW_HASHMAP     pDbPathToStateMap;
} VDIR_MDB_STATE_GLOBALS, *PVDIR_MDB_STATE_GLOBALS;

typedef struct _VDIR_MDB_STATE
{
    int             nDBCopyCount;
    unsigned long   xLogNum;
    unsigned long   dbSizeMb;
    unsigned long   dbMapMb;
    CHAR            bufDBPath[VMDIR_MAX_FILE_NAME_LEN];
    PLW_HASHMAP     pActiveFileHandleMap;
} VDIR_MDB_STATE, *PVDIR_MDB_STATE;
