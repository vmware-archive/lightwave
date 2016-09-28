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
#include "includes.h"

#ifdef _WIN32
#pragma warning(disable : 4996 4995)
#endif 

DWORD
VmcaDbDatabaseInitialize(
    const char * pszAppDBName
    )
{
    DWORD dwError = 0;

    dwError = VmcaDbCreateAppDatabase(pszAppDBName);
    BAIL_ON_VMCA_ERROR(dwError);

#ifndef _WIN32
    // NT has no correspoding ownership or mod change ? 
    if (chown(pszAppDBName, 0, 0) < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (chmod(pszAppDBName, S_IRUSR | S_IWUSR) < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_VMCA_ERROR(dwError);
    }
#endif
error:

    return dwError;
}

DWORD
VmcaDbCreateAppDatabase(
    const char *pszAppDBName
    )
{
    DWORD dwError = 0;
    sqlite3 * pDB = NULL;
    BOOL bInTx = FALSE;

    dwError = sqlite3_open_v2(
                        pszAppDBName,
                        &pDB,
                        SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, 
                        NULL
                        );
    BAIL_ON_VMCA_ERROR(dwError);
 
    dwError = VmcaDbSqliteExecuteTransaction(
                        pDB,
                        "PRAGMA page_size = 2048;"
                        );
    BAIL_ON_VMCA_ERROR(dwError);

    // dwError = VmcaDbSqliteExecuteTransaction(
    //                     pDB,
    //                     "PRAGMA default_cache_size = 10000;"
    //                     );
    // BAIL_ON_VMCA_ERROR(dwError);

    VmcaDbBeginTransaction(pDB);
    bInTx = TRUE;

// typedef struct _VMCA_DB_CERTIFICATE_ENTRY
// {
//     PWSTR   pwszCommonName;
//     PWSTR   pwszAltNames;
//     PWSTR   pwszOrgName;
//     PWSTR   pwszOrgUnitName;
//     PWSTR   pwszIssuerName;  
//     PWSTR   pwszCountryName;
//     PWSTR   pwszSerial;
//     PWSTR   pwszTimeValidFrom;
//     PWSTR   pwszTimeValidTo;
//     PBYTE   pCertBlob;
//     DWORD   dwCertBlobLength;
//     DWORD   dwRevoked;
// } VMCA_DB_CERTIFICATE_ENTRY, *PVMCA_DB_CERTIFICATE_ENTRY;

    dwError = VmcaDbSqliteExecuteTransaction(
                    pDB,
                    "CREATE TABLE IF NOT EXISTS CertTable ("
                    "ID INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL,"
                    "CommonName  VARCHAR(128)  COLLATE NOCASE NOT NULL,"
                    "AltNames    VARCHAR(256)  COLLATE NOCASE,"
                    "OrgName     VARCHAR(256)  COLLATE NOCASE,"
                    "OrgUnitName VARCHAR(128)  COLLATE NOCASE,"
                    "IssuerName  VARCHAR(256)  COLLATE NOCASE,"
                    "Country     VARCHAR(256)  COLLATE NOCASE,"
                    "Serial      VARCHAR(256)  COLLATE NOCASE,"
                    "ValidFrom   TEXT COLLATE NOCASE,"
                    "ValidUntil  TEXT COLLATE NOCASE,"
                    "CertSize    INTEGER,"
                    "CertBlob    BLOB,"
                    "UNIQUE(Serial) ON CONFLICT REPLACE);"
                    );
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmcaDbSqliteExecuteTransaction(
                    pDB,
                    "CREATE TABLE IF NOT EXISTS RevokedCertsTable ("
                    "ID            INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL,"
                    "CertID        INTEGER,"
                    "RevokedDate   DATE DEFAULT (STRFTIME('%s','now')-600),"
                    "RevokedReason INTEGER,"
                    "Serial        VARCHAR(256)  COLLATE NOCASE,"
                    "UNIQUE(Serial),"
                    "FOREIGN KEY(CertID) REFERENCES CertTable(ID));"
                    );
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmcaDbSqliteExecuteTransaction(
                    pDB,
                    "CREATE TABLE IF NOT EXISTS PropertiesTable ("
                    "PropID            INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL,"
                    "Property          VARCHAR(256) COLLATE NOCASE,"
                    "Version           INTEGER DEFAULT 0,"
                    "Value             BLOB,"
                    "UNIQUE(Property,Version) ON CONFLICT REPLACE);"
                    );
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmcaDbSqliteExecuteTransaction(
                    pDB,
                    "CREATE TRIGGER IF NOT EXISTS delete_revoked_cert "
                    "AFTER DELETE on CertTable "
                    "BEGIN "
                    "DELETE FROM RevokedCertsTable "
                    "WHERE CertID = old.ID;"
                    "END;"
                    );
    BAIL_ON_VMCA_ERROR(dwError);

#if 0
    dwError = VmcaDbSqliteExecuteTransaction(
    pDB,
        "CREATE UNIQUE INDEX IF NOT EXISTS idxCertCommonName ON CertTable (CommonName);"
        );
    BAIL_ON_VMCA_ERROR(dwError);
#endif

    VmcaDbCommitTransaction(pDB);

cleanup:

    if (pDB)
    {
        VmcaDbDatabaseClose(pDB);
    }

    return dwError;

error:

    if (bInTx)
    {
        VmcaDbRollbackTransaction(pDB);
        bInTx = FALSE;
    }

    goto cleanup;
}

DWORD
VmcaDbResetAppDatabase(
    VOID
    )
{
    DWORD dwError = 0;
    BOOL bInTx = FALSE;
    PVMCA_DB_CONTEXT pDbContext = NULL;

    dwError = VmcaDbCreateContext(&pDbContext);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmcaDbBeginTransaction(pDbContext->pDb);
    BAIL_ON_VMCA_ERROR(dwError);
    bInTx = TRUE;

    dwError = VmcaDbSqliteExecuteTransaction(
                pDbContext->pDb,
                "DELETE FROM CERTTABLE;"
                );
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmcaDbCommitTransaction(pDbContext->pDb);
    BAIL_ON_VMCA_ERROR(dwError);

    VMCA_LOG_INFO("Vmca Database reset.");

cleanup:

    if (pDbContext)
    {
        VmcaDbReleaseContext(pDbContext);
    }

    return dwError;

error:

    VMCA_LOG_ERROR("Vmca Database reset failed. Error code:%u (%s)",
        dwError,
        VmcaDbErrorCodeToName(dwError));

    if (pDbContext && bInTx)
    {
        VmcaDbRollbackTransaction(pDbContext->pDb);
    }

    goto cleanup;
}

DWORD
VmcaDbSqliteExecuteTransaction(
    sqlite3 * pDB,
    RP_PSTR sqlStr
    )
{
    sqlite3_stmt *stmt;
    DWORD dwError = 0;

    dwError = sqlite3_prepare_v2(
                        pDB,
                        sqlStr,
                        -1,
                        &stmt,
                        0
                        );
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = sqlite3_step(stmt);
    if (dwError == SQLITE_DONE)
    {
        dwError = 0;
    }
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:
 
    if (stmt)
    {
        sqlite3_finalize(stmt);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmcaDbPrepareSql(
    sqlite3 * pDB,
    sqlite3_stmt **hs,
    char *SqlStr
    )
{
    DWORD dwError = 0;

    dwError = sqlite3_prepare_v2(pDB, SqlStr, -1, hs, 0);
    
    return dwError;
}

DWORD
VmcaDbStepSql(
    sqlite3_stmt *hs
    )
{
    DWORD dwError = 0;

    dwError = sqlite3_step(hs);
    if (dwError == SQLITE_DONE)
    {
        dwError = 0;
    }
    
    return dwError;
}

DWORD
VmcaDbBeginTransaction(
    sqlite3 *pDB
    )
{
    DWORD dwError = 0;

    dwError = sqlite3_exec(
                        pDB,
                        "BEGIN TRANSACTION",
                        NULL,
                        NULL,
                        NULL);

    return dwError;
}
DWORD
VmcaDbCtxBeginTransaction(
    PVMCA_DB_CONTEXT	pDbContext
    )
{
    DWORD dwError = 0;

    assert(pDbContext);

    // Sqllite does not support nested tx, this will fail if we already in tx.
    dwError = VmcaDbBeginTransaction(pDbContext->pDb);
    if (dwError == SQLITE_OK)
    {
        pDbContext->bInTx = TRUE;
    }

    return dwError;
}

DWORD
VmcaDbCommitTransaction(
    sqlite3 *pDB
    )
{
    DWORD dwError = 0;

    dwError = sqlite3_exec(
                        pDB,
                        "COMMIT",
                        NULL,
                        NULL,
                        NULL);

    return dwError;
}


DWORD
VmcaDbCtxCommitTransaction(
    PVMCA_DB_CONTEXT	pDbContext
    )
{
    DWORD dwError = 0;

    assert(pDbContext);

    if (pDbContext->bInTx)
    {
        dwError = VmcaDbCommitTransaction(pDbContext->pDb);
        if (dwError == SQLITE_OK)
        {
            pDbContext->bInTx = FALSE;
        }
    }

    return dwError;
}

DWORD
VmcaDbRollbackTransaction(
    sqlite3 *pDB
    )
{
    DWORD dwError = 0;

    dwError = sqlite3_exec(
                        pDB,
                        "ROLLBACK",
                        NULL,
                        NULL,
                        NULL);

    return dwError;
}


DWORD
VmcaDbCtxRollbackTransaction(
    PVMCA_DB_CONTEXT	pDbContext
    )
{
    DWORD dwError = 0;

    assert(pDbContext);

    if (pDbContext->bInTx)
    {
        dwError = VmcaDbRollbackTransaction(pDbContext->pDb);
        pDbContext->bInTx = FALSE;
    }

    return dwError;
}

VOID
VmcaDbDatabaseClose(
    sqlite3* pDB
    )
{
    sqlite3_close(pDB);
}

/* Note:
 *
 * Integer values are expected to be allocated before being passed in.
 * Strings and Blobs will always be allocated in this routine.
 *
 */
DWORD
VmcaDbFillValues(
    sqlite3_stmt*   pSqlStatement,
    PVMCA_DB_COLUMN pColumnArray,
    DWORD           dwNumColumns
    )
{
    DWORD dwError = 0;
    DWORD iColumn = 0;

    for (; iColumn < dwNumColumns; iColumn++)
    {
        DWORD dwLen = 0;
        PVMCA_DB_COLUMN pColumn = &pColumnArray[iColumn];

        switch (pColumn->columnType)
        {
            case VMCA_DB_COLUMN_TYPE_INT32:

                if (!pColumn->pValue->data.ppdwValue ||
                    !*pColumn->pValue->data.ppdwValue)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMCA_ERROR(dwError);
                }

                **pColumn->pValue->data.ppdwValue =
                        sqlite3_column_int(pSqlStatement, iColumn);
                pColumn->pValue->dataLength = sizeof(DWORD);

                break;

            case VMCA_DB_COLUMN_TYPE_INT64:

                if (!pColumn->pValue->data.ppullValue ||
                    !*pColumn->pValue->data.ppullValue)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMCA_ERROR(dwError);
                }

                **pColumn->pValue->data.ppullValue =
                        sqlite3_column_int64(pSqlStatement, iColumn);
                pColumn->pValue->dataLength = sizeof(ULONG64);

                break;

            case VMCA_DB_COLUMN_TYPE_STRING:

                dwLen = sqlite3_column_bytes(pSqlStatement, iColumn);

                if (dwLen > 0)
                {
                    unsigned const char* pszValue =
                            sqlite3_column_text(pSqlStatement, iColumn);

                    if (!pColumn->pValue->data.ppwszValue)
                    {
                        dwError = ERROR_INVALID_PARAMETER;
                        BAIL_ON_VMCA_ERROR(dwError);
                    }

#ifdef _NoCode
                    dwError = ConvertAnsitoUnicodeString(
                                    pszValue,
                                    pColumn->pValue->data.ppwszValue);
                    BAIL_ON_VMCA_ERROR(dwError);

                    dwError = VMCAGetStringLengthW(
                                    *pColumn->pValue->data.ppwszValue,
                                    &pColumn->pValue->dataLength);
                    BAIL_ON_VMCA_ERROR(dwError);
#else
                    dwError = VMCAAllocateStringWFromA(
                                    (PCSTR)pszValue,
                                    pColumn->pValue->data.ppwszValue);
                    BAIL_ON_VMCA_ERROR(dwError);

                    dwError = VMCAGetStringLengthW(
                                    *pColumn->pValue->data.ppwszValue,
                                    (PSIZE_T)&pColumn->pValue->dataLength);
                    BAIL_ON_VMCA_ERROR(dwError);

#endif
                }

                break;

            case VMCA_DB_COLUMN_TYPE_BLOB:

                dwLen = sqlite3_column_bytes(pSqlStatement, iColumn);
                if (dwLen > 0)
                {
                    unsigned const char* pszValue =
                            (unsigned const char*)sqlite3_column_blob(pSqlStatement, iColumn);
                    
                    if (!pColumn->pValue->data.ppValue)
                    {
                        dwError = ERROR_INVALID_PARAMETER;
                        BAIL_ON_VMCA_ERROR(dwError);
                    }

                    dwError = VMCAAllocateMemory(
                                    dwLen+1,
                                    (PVOID*)pColumn->pValue->data.ppValue);
                    BAIL_ON_VMCA_ERROR(dwError);

                    pColumn->pValue->dataLength = dwLen;

                    memcpy(*pColumn->pValue->data.ppValue, pszValue, dwLen);
                }

                break;

            case VMCA_DB_COLUMN_TYPE_UNKNOWN:
            default:

                dwError = ERROR_NOT_SUPPORTED;
                BAIL_ON_VMCA_ERROR(dwError);
        }
    }

error:

    return dwError;
}

