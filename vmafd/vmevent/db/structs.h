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
 * Module Name: ThinAppRepoService
 *
 * Filename: defines.h
 *
 * Abstract:
 *
 * Thinapp Repository Database
 *
 * Private Structures
 *
 */

typedef enum
{
    EVENTLOG_DB_COLUMN_TYPE_UNKNOWN = 0,
    EVENTLOG_DB_COLUMN_TYPE_INT32,
    EVENTLOG_DB_COLUMN_TYPE_STRING,
    EVENTLOG_DB_COLUMN_TYPE_BLOB
} EVENTLOG_DB_COLUMN_TYPE;

typedef struct _EVENTLOG_DB_COLUMN_VALUE
{
    union
    {
        PBYTE*  ppValue;
        PDWORD* ppdwValue;
        PWSTR*  ppwszValue;
    } data;
    size_t dataLength;
} EVENTLOG_DB_COLUMN_VALUE, *PEVENTLOG_DB_COLUMN_VALUE;

typedef struct _EVENTLOG_DB_COLUMN
{
    PSTR                  pszName;
    EVENTLOG_DB_COLUMN_TYPE   columnType;
    PEVENTLOG_DB_COLUMN_VALUE pValue;

} EVENTLOG_DB_COLUMN, *PEVENTLOG_DB_COLUMN;

typedef struct _EVENTLOG_DB_CONTEXT
{
    sqlite3 * pDb;
    BOOLEAN bInTx; // Explicit transaction

    sqlite3_stmt* pDelEventQuery;
    sqlite3_stmt* pDelEventAppsQuery;
    sqlite3_stmt* pAddEventQuery;
    sqlite3_stmt* pEventIDByNameQuery;
    sqlite3_stmt* pEventVersionIDByNameQuery;
    sqlite3_stmt* pEventCountByNameQuery;
    sqlite3_stmt* pTotalEventCountQuery;
    sqlite3_stmt* pQuerySingleEvent;
    sqlite3_stmt* pQueryAllEvents;
    sqlite3_stmt* pQueryEventsPaged;

    struct _EVENTLOG_DB_CONTEXT * pNext;

} EVENTLOG_DB_CONTEXT, *PEVENTLOG_DB_CONTEXT;

typedef struct _EVENTLOG_DB_GLOBALS
{
    pthread_mutex_t mutex;
    BOOLEAN	        bIsDBOpened;

    PSTR            pszDbPath;

    DWORD           dwMaxNumCachedContexts;
    DWORD           dwNumCachedContexts;

    PEVENTLOG_DB_CONTEXT pDbContextList;

} EVENTLOG_DB_GLOBALS, *PEVENTLOG_DB_GLOBALS;

typedef struct _EVENTLOG_DB_ERROR_CODE_NAME_DESC
{
    int         code;
    const char* name;
    const char* desc;

} EVENTLOG_DB_ERROR_CODE_NAME_DESC, *PEVENTLOG_DB_ERROR_CODE_NAME_DESC;

