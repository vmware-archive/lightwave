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
 * Module Name: Directory bdb-store
 *
 * Filename: structs.h
 *
 * Abstract:
 *
 * Directory bdb-store module
 *
 * Private Structures
 *
 */

#define BDB_INDEX_OP_TYPE_CREATE    0x0001
#define BDB_INDEX_OP_TYPE_DELETE    0x0002
#define BDB_INDEX_OP_TYPE_UPDATE    0x0004

typedef struct _VDIR_CFG_BDB_DATAFILE_DESC
{
    DB*         pDB;
    PSTR        pszDBName;
    PSTR        pszDBFile;
    BOOLEAN     bIsUnique;

} VDIR_CFG_BDB_DATAFILE_DESC, *PVDIR_CFG_BDB_DATAFILE_DESC;

typedef int (*PFN_BT_KEY_CMP)(DB *db, const DBT *dbt1, const DBT *dbt2);

typedef struct _VDIR_BDB_ENTRY_DATABASE
{
    USHORT                      usNumDataFiles;
    PVDIR_CFG_BDB_DATAFILE_DESC pBdbDataFiles;

    // Btree key comparison function
    PFN_BT_KEY_CMP              btKeyCmpFcn;

} VDIR_BDB_ENTRY_DATABASE, *PVDIR_BDB_ENTRY_DATABASE;

typedef struct _VDIR_BDB_INDEX_DATABASE
{
    PSTR                        pszAttrName;    // Not currently used
    USHORT                      usNumDataFiles;

    // Array of BdbDataFiles indexed by VDIR_CFG_ATTR_INDEX_DESC.iId
    PVDIR_CFG_BDB_DATAFILE_DESC pBdbDataFiles;

    // Btree key comparison function
    PFN_BT_KEY_CMP              btKeyCmpFcn;

} VDIR_BDB_INDEX_DATABASE, *PVDIR_BDB_INDEX_DATABASE;

typedef struct _VDIR_BDB_INDEX_DB_COLLECTION
{
    USHORT                      usNumIndexAttribute;
    USHORT                      usMaxSize;
    PVDIR_BDB_INDEX_DATABASE    pIndexDBs;

} VDIR_BDB_INDEX_DB_COLLECTION, *PVDIR_BDB_INDEX_DB_COLLECTION;

typedef struct _VDIR_BDB_GLOBALS
{
    // NOTE: order of fields MUST stay in sync with struct initializer...
    VDIR_BDB_ENTRY_DATABASE         bdbEntryDB;
    VDIR_BDB_INDEX_DB_COLLECTION    bdbIndexDBs;
    DB_ENV *                        bdbEnv;
    DB_SEQUENCE *                   bdbEidSeq;
    DB_SEQUENCE *                   bdbUsnSeq;

} VDIR_BDB_GLOBALS, *PVDIR_BDB_GLOBALS;

