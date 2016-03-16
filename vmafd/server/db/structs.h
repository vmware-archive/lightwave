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
 * Module Name: VMware Certificate Server Database
 *
 * Filename: defines.h
 *
 * Abstract:
 *
 * VMware Certificate Service Database
 *
 * Private Structures
 *
 */


#ifndef _VECS_DB_STRUCTS_H__
#define _VECS_DB_STRUCTS_H__

typedef enum
{
    VMAFD_DB_MODE_UNKNOWN = 0,
    VMAFD_DB_MODE_READ,
    VMAFD_DB_MODE_WRITE
}VMAFD_DB_MODE;

typedef enum
{
    VECS_DB_COLUMN_TYPE_UNKNOWN = 0,
    VECS_DB_COLUMN_TYPE_INT32,
    VECS_DB_COLUMN_TYPE_INT64,
    VECS_DB_COLUMN_TYPE_STRING,
    VECS_DB_COLUMN_TYPE_BLOB
} VECS_DB_COLUMN_TYPE;

typedef struct __VECS_DB_COLUMN_VALUE
{
    union
    {
        PBYTE*    ppValue;
        PDWORD*   ppdwValue;
        PULONG64* ppullValue;
        PWSTR*    ppwszValue;
    } data;
    size_t dataLength;
} VECS_DB_COLUMN_VALUE, *PVECS_DB_COLUMN_VALUE;

typedef struct __VECS_DB_COLUMN
{
    PSTR                  pszName;
    VECS_DB_COLUMN_TYPE   columnType;
    PVECS_DB_COLUMN_VALUE pValue;

} VMCA_DB_COLUMN, *PVECS_DB_COLUMN;

typedef struct _VECS_DB_STMT_ARRAY
{
    sqlite3_stmt** pStmtToExecute;
    DWORD dwCount;
} VECS_DB_STMT_ARRAY, *PVECS_DB_STMT_ARRAY;

typedef struct __VECS_DB_CONTEXT
{
    sqlite3 * pDb;
    VMAFD_DB_MODE dbOpenMode;
    BOOLEAN bInTx; // Explicit transaction

    sqlite3_stmt* pAddCertQuery;
    sqlite3_stmt* pQueryAllCertificates;
    sqlite3_stmt* pQueryCertificatesPaged;
    sqlite3_stmt* pTotalCertCountQuery;
    sqlite3_stmt* pDelCertQuery;
    sqlite3_stmt* pUpdateCertQuery;
    sqlite3_stmt* pUpdateCertAttrQuery;
    sqlite3_stmt* pVerifyCertQuery;
    sqlite3_stmt* pGetCertFromAliasQuery;
    sqlite3_stmt* pGetCountQuery;
    sqlite3_stmt* pCreateStoreTable;
    sqlite3_stmt* pDeleteStore;
    sqlite3_stmt* pQueryStoreTable;
    sqlite3_stmt* pGetPKeyFromAlias;
    sqlite3_stmt* pQueryInfoLevel1;
    sqlite3_stmt* pQueryInfoLevel2;
    sqlite3_stmt* pQueryTypeByAlias;
    sqlite3_stmt* pQueryDateByAlias;
    sqlite3_stmt* pQuerySetSecurityDescriptor;
    sqlite3_stmt* pQueryGetSecurityDescriptor;
    sqlite3_stmt* pQueryAddAces;
    sqlite3_stmt* pQueryGetAces;
    PVECS_DB_STMT_ARRAY pQueryAddAcesArray;
    struct __VECS_DB_CONTEXT * pNext;

} VECS_DB_CONTEXT;

typedef struct __VECS_DB_GLOBALS
{
    pthread_mutex_t     mutex;

    PSTR            pszDbPath;

    DWORD           dwMaxNumCachedContexts;
    DWORD           dwNumCachedContexts;

    PVECS_DB_CONTEXT pDbContextList;

} VECS_DB_GLOBALS, *PVECS_DB_GLOBALS;

typedef struct __VECS_DB_ERROR_CODE_NAME_DESC
{
    int         code;
    const char* name;
    const char* desc;

} VECS_DB_ERROR_CODE_NAME_DESC, *PVECS_DB_ERROR_CODE_NAME_DESC;

#endif // _VECS_DB_STRUCTS_H__
