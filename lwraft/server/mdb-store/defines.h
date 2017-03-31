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
 * Module Name: Directory mdb-store
 *
 * Filename: defines.h
 *
 * Abstract:
 *
 * Directory mdb-store module
 *
 * Definitions
 *
 */

// key for ENTRY and USN sequence number
#define BE_MDB_SEQ_DB_NAME              "MDB_SEQUENCE_DB"
#define BE_MDB_GENERIC_DUPKEY_DB_NAME   "MDB_GNERIC_DUPKEY_DB"
#define BE_MDB_GENERIC_UNIQKEY_DB_NAME  "MDB_GNERIC_UNIQKEY_DB"
#define BE_MDB_ENTRYID_SEQ_KEY          1
#define BE_MDB_USN_SEQ_KEY              2

// MDB configure parameters
#define BE_MDB_ENV_MAX_READERS        1000
#define BE_MDB_ENV_MAX_DBS            100

#ifdef _WIN32
#define BE_MDB_ENV_MAX_MEM_MAPSIZE    1073741824   // 1 GB for now, as MDB does preallocate on Windows
#else
#define BE_MDB_ENV_MAX_MEM_MAPSIZE    21474836480  // 20 GB - 20*1024*1024*1024
#endif

// MDB specific type mapping
typedef MDB_dbi             VDIR_DB,     *PVDIR_DB;
typedef MDB_env             VDIR_DB_ENV, *PVDIR_DB_ENV;
typedef MDB_val             VDIR_DB_DBT, *PVDIR_DB_DBT;
typedef MDB_cursor          VDIR_DB_DBC, *PVDIR_DB_DBC;
typedef MDB_txn             VDIR_DB_TXN, *PVDIR_DB_TXN;

// NOTE: mdb_hash_t is defined in mdb.c and not exposed in mdb.h
typedef unsigned long long  mdb_hash_t;
typedef mdb_hash_t          VDIR_DB_SEQ_T, *PVDIR_DB_SEQ_T;
