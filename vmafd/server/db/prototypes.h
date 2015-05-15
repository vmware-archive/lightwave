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
 * Module Name: VMware Certificate Server
 *
 * Filename: prototypes.h
 *
 * Abstract:
 *
 * VMware Certificate Server Database
 *
 * Function prototypes
 *
 */

#ifndef _PROTOTYPES_H_
#define _PROTOTYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

/* certificates.c */

DWORD
VecsDbQueryAllCertificates(
    PVECS_DB_CONTEXT            pDbContext,
    PVECS_DB_CERTIFICATE_ENTRY* ppCertEntryArray,
    PDWORD                      pdwCount
    );


/* database.c */

DWORD
VecsDbDatabaseInitialize(
    const char * pszDBName
    );

DWORD
VecsDbCreateAppDatabase(
    const char *pszDBName
    );

DWORD
VecsDbResetAppDatabase(
    VOID
    );

DWORD
VecsDbSqliteExecuteTransaction(
    sqlite3 * pDB,
    PSTR sqlStr
    );

DWORD
VecsDbPrepareSql(
    sqlite3*       pDB,
    sqlite3_stmt** hs,
    char *         SqlStr
    );

DWORD
VecsDbStepSql(
    sqlite3_stmt *hs
    );

DWORD
VecsDbBeginTransaction(
    sqlite3 *pDb
    );

DWORD
VecsDbCommitTransaction(
    sqlite3 *pDB
    );

DWORD
VecsDbRollbackTransaction(
    sqlite3 *pDB
    );

VOID
VecsDbDatabaseClose(
    sqlite3* pDB
    );

DWORD
VecsDbFillValues(
    sqlite3_stmt*   pSqlStatement,
    PVECS_DB_COLUMN pColumnArray,
    DWORD           dwNumColumns
    );

DWORD
VecsBindtext(
    sqlite3_stmt* pSqlStatement,
    PSTR pszParamName,
    PSTR pszValue
    );

DWORD
VecsBindWideString(
    sqlite3_stmt* pSqlStatement,
    PSTR pszParamName,
    PWSTR pszValue
    );


DWORD
VecsBindDword(
    sqlite3_stmt* pSqlStatement,
    PSTR pszParamName,
    DWORD dwValue
    );


DWORD
VecsBindBlob(
    sqlite3_stmt* pSqlStatement,
    PSTR pszParamName,
    PBYTE pszValue,
    DWORD dwSize
    );

DWORD
VecsGetColunmIndexFromName(
    sqlite3_stmt* pSqlStatement,
    PSTR pszCloumnName
    );

DWORD
VecsDBGetColumnString(
    sqlite3_stmt* pSqlStatement,
    PSTR pszColumnName,
    PWSTR* pszValue
    );

DWORD
VecsDBGetColumnBlob(
    sqlite3_stmt* pSqlStatement,
    PSTR pszColumnName,
    PBYTE* pszValue,
    PDWORD pdwLen);

DWORD
VecsDBGetColumnInt(
    sqlite3_stmt* pSqlStatement,
    PSTR pszColumnName,
    PDWORD pdwValue);
DWORD
VecsBindString(
    sqlite3_stmt* pSqlStatement,
    PSTR pszParamName,
    PSTR pszValue
    );

DWORD
VecsCheckifTableExists(
    sqlite3 * pDB ,
    const char *psztableName,
    PDWORD bExist);

DWORD
VecsDbQueryCertificateByAlias(
    PVECS_DB_CONTEXT            pDbContext,
    PVECS_DB_CERTIFICATE_ENTRY* ppCertEntryArray,
    PDWORD                      pdwCount,
    PCWSTR                      pwszAlias
    );

DWORD
VecsDbGetCertificateCount(  PVECS_DB_CONTEXT pDbContext,
                            PCWSTR pwszAlias,
                            DWORD *pdwCount );

/* libmain.c */

#ifdef __cplusplus
}
#endif

#endif // _PROTOTYPES_H_

