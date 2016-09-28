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
VmcaDbQueryAllCertificates(
    PVMCA_DB_CONTEXT            pDbContext,
    PVMCA_DB_CERTIFICATE_ENTRY* ppCertEntryArray,
    PDWORD                      pdwCount
    );

DWORD
VmcaDbQueryCertificatesPaged(
    PVMCA_DB_CONTEXT         pDbContext,
    DWORD                    dwStartIndex,
    DWORD                    dwNumPackages,
    VMCA_DB_CERTIFICATE_STATUS       dwStatus,
    PVMCA_DB_CERTIFICATE_ENTRY*  ppPackageEntryArray,
    PDWORD                   pdwCount
    );

/* database.c */

DWORD
VmcaDbDatabaseInitialize(
    const char * pszDBName
    );

DWORD
VmcaDbCreateAppDatabase(
    const char *pszDBName
    );

DWORD
VmcaDbResetAppDatabase(
    VOID
    );

DWORD
VmcaDbSqliteExecuteTransaction(
    sqlite3 * pDB,
    RP_PSTR sqlStr
    );

DWORD
VmcaDbPrepareSql(
    sqlite3*       pDB,
    sqlite3_stmt** hs,
    char *         SqlStr
    );

DWORD
VmcaDbStepSql(
    sqlite3_stmt *hs
    );

DWORD
VmcaDbBeginTransaction(
    sqlite3 *pDb
    );

DWORD
VmcaDbCommitTransaction(
    sqlite3 *pDB
    );

DWORD
VmcaDbRollbackTransaction(
    sqlite3 *pDB
    );

VOID
VmcaDbDatabaseClose(
    sqlite3* pDB
    );

DWORD
VmcaDbFillValues(
    sqlite3_stmt*   pSqlStatement,
    PVMCA_DB_COLUMN pColumnArray,
    DWORD           dwNumColumns
    );

/* libmain.c */

#ifdef __cplusplus
}
#endif

#endif // _PROTOTYPES_H_

