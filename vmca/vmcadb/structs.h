/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

typedef enum
{
    VMCA_DB_COLUMN_TYPE_UNKNOWN = 0,
    VMCA_DB_COLUMN_TYPE_INT32,
    VMCA_DB_COLUMN_TYPE_INT64,
    VMCA_DB_COLUMN_TYPE_STRING,
    VMCA_DB_COLUMN_TYPE_BLOB
} VMCA_DB_COLUMN_TYPE;

typedef struct _VMCA_DB_COLUMN_VALUE
{
    union
    {
        PBYTE*    ppValue;
        PDWORD*   ppdwValue;
        PULONG64* ppullValue;
        PWSTR*    ppwszValue;
    } data;
    size_t dataLength;
} VMCA_DB_COLUMN_VALUE, *PVMCA_DB_COLUMN_VALUE;

typedef struct _VMCA_DB_COLUMN
{
    PSTR                  pszName;
    VMCA_DB_COLUMN_TYPE   columnType;
    PVMCA_DB_COLUMN_VALUE pValue;

} VMCA_DB_COLUMN, *PVMCA_DB_COLUMN;

typedef struct _VMCA_DB_CONTEXT
{
    sqlite3 * pDb;
    BOOLEAN bInTx; // Explicit transaction

    sqlite3_stmt* pAddCertQuery;
    sqlite3_stmt* pQueryAllCertificates;
    sqlite3_stmt* pQueryCertificatesPaged;
    sqlite3_stmt* pQueryRevokedCertificates;
    sqlite3_stmt* pTotalCertCountQuery;
    sqlite3_stmt* pDelCertQuery;
    sqlite3_stmt* pUpdateCertQuery;
    sqlite3_stmt* pVerifyCertQuery;
    sqlite3_stmt* pGetCertID;
    sqlite3_stmt* pRevokeCert;
    sqlite3_stmt* pRevokedCertCount;
    sqlite3_stmt* pGetCrlNumber;
    sqlite3_stmt* pSetCrlNumber;

    struct _VMCA_DB_CONTEXT * pNext;

} VMCA_DB_CONTEXT;

typedef struct _VMCA_DB_GLOBALS
{
    pthread_mutex_t mutex;

    PSTR            pszDbPath;

    DWORD           dwMaxNumCachedContexts;
    DWORD           dwNumCachedContexts;

    PVMCA_DB_CONTEXT pDbContextList;

} VMCA_DB_GLOBALS, *PVMCA_DB_GLOBALS;

typedef struct _VMCA_DB_ERROR_CODE_NAME_DESC
{
    int         code;
    const char* name;
    const char* desc;

} VMCA_DB_ERROR_CODE_NAME_DESC, *PVMCA_DB_ERROR_CODE_NAME_DESC;

