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
 * Filename: prototypes.h
 *
 * Abstract:
 *
 * Thinapp Repository Database
 *
 * Function prototypes
 *
 */

#ifndef _PROTOTYPES_H_
#define _PROTOTYPES_H_

#ifdef __cplusplus
extern "C" {
#endif


/* database.c */

DWORD
EventLogDbDatabaseInitialize(
    const char * pszLeaseDBName,
    sqlite3 ** ppDB
    );

DWORD
EventLogDbCreateAppDatabase(
    const char *pszLeaseDBName,
    sqlite3 ** ppDB
    );

DWORD
EventLogDbResetAppDatabase(
    VOID
    );

DWORD
EventLogDbSqliteExecuteTransaction(
    sqlite3 * pDB,
    PSTR sqlStr
    );

DWORD
EventLogDbPrepareSql(
    sqlite3 * pDB,
    sqlite3_stmt **hs,
    char *SqlStr
    );

DWORD
EventLogDbStepSql(
    sqlite3_stmt *hs
    );

DWORD
EventLogDbBeginTransaction(
    sqlite3 *pDb
    );

DWORD
EventLogDbCommitTransaction(
    sqlite3 *pDB
    );

DWORD
EventLogDbRollbackTransaction(
    sqlite3 *pDB
    );

DWORD
EventLogServerDbAddLease(
    sqlite3 *pDB,
    PSTR pszUserName,
    PSTR pszAppName
    );

VOID
EventLogDbDatabaseClose(
    sqlite3* pDB
    );

DWORD
EventLogDbFillValues(
    sqlite3_stmt*       pSqlStatement,
    PEVENTLOG_DB_COLUMN pColumnArray,
    DWORD               dwNumColumns
    );

/* application.c */


#ifdef __cplusplus
}
#endif

#endif // _PROTOTYPES_H_

