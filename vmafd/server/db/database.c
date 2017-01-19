/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : database.c
 *
 * Abstract :
 *
 */
#include "includes.h"

#ifdef _WIN32
#pragma warning(disable : 4996 4995)
#endif

static
VOID
VmAfdSqliteLogCallback(
        PVOID *pArg,
        DWORD  dwError,
        PCSTR  pszMessage
        );

static
DWORD
VecsDbLocalAuthenticationTablesCreate (
        sqlite3 *pDb
        );

static
DWORD
CdcCreateAppDatabase(
    const char* pszDBName
    );

DWORD
VecsDbDatabaseInitialize(
    const char * pszAppDBName
    )
{
    DWORD dwError = 0;

    dwError = VecsDbCreateAppDatabase(pszAppDBName);
    BAIL_ON_VECS_ERROR(dwError);

    sqlite3_config(SQLITE_CONFIG_LOG, VmAfdSqliteLogCallback, NULL);

#ifndef _WIN32
    // NT has no correspoding ownership or mod change ?
    if (chown(pszAppDBName, 0, 0) < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_VECS_ERROR(dwError);
    }

    if (chmod(pszAppDBName, S_IRUSR | S_IWUSR) < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_VECS_ERROR(dwError);
    }
#endif
error:

    return dwError;
}

DWORD
CdcDbInitialize(
    PCSTR pszDbPath
    )
{
    DWORD dwError = 0;

    if (IsNullOrEmptyString(pszDbPath))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = CdcCreateAppDatabase(pszDbPath);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VecsDbCreateAppDatabase(
    const char *pszAppDBName
    )
{
    DWORD dwError = 0;
    sqlite3 * pDB = NULL;
    BOOL bInTx = FALSE;
    DWORD bTableExists = 0;

    dwError = sqlite3_open_v2(
                        pszAppDBName,
                        &pDB,
                        SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE,
                        NULL
                        );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsCheckifTableExists(pDB, "CertTable", &bTableExists);
    BAIL_ON_VECS_ERROR(dwError);

    if (bTableExists != 1) {

        dwError = VecsDbSqliteExecuteTransaction(
                            pDB,
                            "PRAGMA page_size = 2048;"
                            );
        BAIL_ON_VECS_ERROR(dwError);

        dwError = VecsDbSqliteExecuteTransaction(
                            pDB,
                            "PRAGMA default_cache_size = 10000;"
                            );
        BAIL_ON_VECS_ERROR(dwError);


        VecsDbBeginTransaction(pDB);
        bInTx = TRUE;

        dwError = VecsDbSqliteExecuteTransaction(
                        pDB,
                        "CREATE TABLE IF NOT EXISTS StoreTable ("
                        "StoreID INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL,"
                        "StoreName  VARCHAR(128)  COLLATE NOCASE NOT NULL,"
                        "Password VARCHAR(128),"
                        "Salt VARCHAR(128),"
                        "UNIQUE(StoreName));"
                        );
        BAIL_ON_VECS_ERROR(dwError);

        dwError = VecsDbSqliteExecuteTransaction(
                                pDB,
                                "CREATE TABLE IF NOT EXISTS CertTable ("
                                "ID INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL,"
                                "StoreID INTEGER,"
                                "Alias  VARCHAR(128)  COLLATE NOCASE NOT NULL,"
                                "Serial      VARCHAR(256)  COLLATE NOCASE,"
                                "Date        DATE DEFAULT (STRFTIME('%s','now')),"
                                "CertSize    INTEGER,"
                                "CertBlob    BLOB,"
                                "Password    VARCHAR(256),"
                                "EntryType   INTEGER  DEFAULT 0,"
                                "KeySize      INTEGER,"
                                "PrivateKey   BLOB,"
                                "AutoRefresh  INTEGER, "
                                "UNIQUE(StoreID, Alias),"
                                "CHECK( EntryType == 1"
                                " OR EntryType == 2"
                                " OR EntryType == 3"
                                " OR EntryType == 4"
                                " OR EntryType == 5),"
                                "FOREIGN KEY(StoreID) REFERENCES StoreTable(StoreID));"
                                );
        BAIL_ON_VECS_ERROR(dwError);

        dwError = VecsDbSqliteExecuteTransaction(
                                pDB,
                                "CREATE TRIGGER delete_store "
                                "AFTER DELETE on StoreTable "
                                "BEGIN "
                                "DELETE FROM CertTable "
                                "WHERE StoreID = old.StoreID;"
                                "END;");
        BAIL_ON_VECS_ERROR(dwError);

        dwError = VecsDbSqliteExecuteTransaction(
                        pDB,
                        "CREATE TABLE IF NOT EXISTS PassTable ("
                        "ID INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL,"
                        "AccountName  VARCHAR(128)  COLLATE NOCASE NOT NULL,"
                        "Password    VARCHAR(256),"
                        "UNIQUE(AccountName) ON CONFLICT REPLACE);"
                        );
        BAIL_ON_VECS_ERROR(dwError);

        dwError = VecsDbLocalAuthenticationTablesCreate(pDB);
        BAIL_ON_VECS_ERROR (dwError);

        dwError = VecsDbSqliteExecuteTransaction(
        pDB,
            "CREATE INDEX IF NOT EXISTS idxVecsAlias ON CertTable (Alias);"
            );

        BAIL_ON_VECS_ERROR(dwError);

        dwError = VecsDbSqliteExecuteTransaction(
        pDB,
        "CREATE INDEX IF NOT EXISTS idxVecsSerial ON CertTable (Serial);"
        );

        BAIL_ON_VECS_ERROR(dwError);

        dwError = VecsDbSqliteExecuteTransaction(
        pDB,
        "CREATE INDEX IF NOT EXISTS idxAccount ON PassTable (AccountName);"
        );

        BAIL_ON_VECS_ERROR(dwError);

        VecsDbCommitTransaction(pDB);
    }

cleanup:

    if (pDB)
    {
        VecsDbDatabaseClose(pDB);
    }

    return dwError;

error:

    if (bInTx)
    {
        VecsDbRollbackTransaction(pDB);
    }

    goto cleanup;
}

DWORD
VecsDbResetAppDatabase(
    VOID
    )
{
    DWORD dwError = 0;
    BOOL bInTx = FALSE;
    PVECS_DB_CONTEXT pDbContext = NULL;

    dwError = VecsDbCreateContext(&pDbContext, VMAFD_DB_MODE_WRITE);
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsDbBeginTransaction(pDbContext->pDb);
    BAIL_ON_VECS_ERROR(dwError);
    bInTx = TRUE;

    dwError = VecsDbSqliteExecuteTransaction(
                pDbContext->pDb,
                "DELETE FROM CERTTABLE;"
                );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsDbCommitTransaction(pDbContext->pDb);
    BAIL_ON_VECS_ERROR(dwError);

cleanup:

    if (pDbContext)
    {
        VecsDbReleaseContext(pDbContext);
    }

    return dwError;

error:


    if (pDbContext && bInTx)
    {
        VecsDbRollbackTransaction(pDbContext->pDb);
    }

    goto cleanup;
}

DWORD
VecsDbSqliteExecuteTransaction(
    sqlite3 * pDB,
    PSTR sqlStr
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
    BAIL_ON_VECS_ERROR(dwError);

    dwError = sqlite3_step(stmt);
    if (dwError == SQLITE_DONE)
    {
        dwError = 0;
    }
    BAIL_ON_VECS_ERROR(dwError);

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
VecsDbPrepareSql(
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
VecsDbStepSql(
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
VecsDbBeginTransaction(
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
VecsDbCtxBeginTransaction(
    PVECS_DB_CONTEXT    pDbContext
    )
{
    DWORD dwError = 0;

    assert(pDbContext);

    // Sqllite does not support nested tx, this will fail if we already in tx.
    dwError = VecsDbBeginTransaction(pDbContext->pDb);
    if (dwError == SQLITE_OK)
    {
        pDbContext->bInTx = TRUE;
    }

    return dwError;
}

DWORD
VecsDbCommitTransaction(
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
VecsDbCtxCommitTransaction(
    PVECS_DB_CONTEXT    pDbContext
    )
{
    DWORD dwError = 0;

    assert(pDbContext);

    if (pDbContext->bInTx)
    {
        dwError = VecsDbCommitTransaction(pDbContext->pDb);
        if (dwError == SQLITE_OK)
        {
            pDbContext->bInTx = FALSE;
        }
    }

    return dwError;
}

DWORD
VecsDbRollbackTransaction(
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
VecsDbCtxRollbackTransaction(
    PVECS_DB_CONTEXT    pDbContext
    )
{
    DWORD dwError = 0;

    assert(pDbContext);

    if (pDbContext->bInTx)
    {
        dwError = VecsDbRollbackTransaction(pDbContext->pDb);
        pDbContext->bInTx = FALSE;
    }

    return dwError;
}

VOID
VecsDbDatabaseClose(
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
VecsDbFillValues(
    sqlite3_stmt*   pSqlStatement,
    PVECS_DB_COLUMN pColumnArray,
    DWORD           dwNumColumns
    )
{
    DWORD dwError = 0;
    DWORD iColumn = 0;

    for (; iColumn < dwNumColumns; iColumn++)
    {
        DWORD dwLen = 0;
        PVECS_DB_COLUMN pColumn = &pColumnArray[iColumn];

        switch (pColumn->columnType)
        {
            case VECS_DB_COLUMN_TYPE_INT32:

                if (!pColumn->pValue->data.ppdwValue ||
                    !*pColumn->pValue->data.ppdwValue)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VECS_ERROR(dwError);
                }

                **pColumn->pValue->data.ppdwValue =
                        sqlite3_column_int(pSqlStatement, iColumn);
                pColumn->pValue->dataLength = sizeof(DWORD);

                break;

            case VECS_DB_COLUMN_TYPE_INT64:

                if (!pColumn->pValue->data.ppullValue ||
                    !*pColumn->pValue->data.ppullValue)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VECS_ERROR(dwError);
                }

                **pColumn->pValue->data.ppullValue =
                        sqlite3_column_int64(pSqlStatement, iColumn);
                pColumn->pValue->dataLength = sizeof(ULONG64);

                break;

            case VECS_DB_COLUMN_TYPE_STRING:

                dwLen = sqlite3_column_bytes(pSqlStatement, iColumn);

                if (dwLen > 0)
                {
                    unsigned const char* pszValue =
                            sqlite3_column_text(pSqlStatement, iColumn);

                    if (!pColumn->pValue->data.ppwszValue)
                    {
                        dwError = ERROR_INVALID_PARAMETER;
                        BAIL_ON_VECS_ERROR(dwError);
                    }

#ifdef _NoCode
                    dwError = VmAfdAllocateStringWFromA(
                                    pszValue,
                                    pColumn->pValue->data.ppwszValue);
                    BAIL_ON_VECS_ERROR(dwError);

                    dwError = VmAfdGetStringLengthW(
                                    *pColumn->pValue->data.ppwszValue,
                                    &pColumn->pValue->dataLength);
                    BAIL_ON_VECS_ERROR(dwError);
#else
                    dwError = VmAfdAllocateStringWFromA(
                                    (PCSTR)pszValue,
                                    pColumn->pValue->data.ppwszValue);
                    BAIL_ON_VECS_ERROR(dwError);

                    dwError = VmAfdGetStringLengthW(
                                    *pColumn->pValue->data.ppwszValue,
                                    (PSIZE_T)&pColumn->pValue->dataLength);
                    BAIL_ON_VECS_ERROR(dwError);

#endif
                }

                break;

            case VECS_DB_COLUMN_TYPE_BLOB:

                dwLen = sqlite3_column_bytes(pSqlStatement, iColumn);
                if (dwLen > 0)
                {
                    unsigned const char* pszValue =
                            (unsigned const char*)sqlite3_column_blob(pSqlStatement, iColumn);

                    if (!pColumn->pValue->data.ppValue)
                    {
                        dwError = ERROR_INVALID_PARAMETER;
                        BAIL_ON_VECS_ERROR(dwError);
                    }
                    // Allocate more space so that we can still do
                    // Strlen on this buffer if needed.
                    dwError = VmAfdAllocateMemory(
                                    dwLen + sizeof(wchar16_t),
                                    (PVOID*)pColumn->pValue->data.ppValue);
                    BAIL_ON_VECS_ERROR(dwError);

                    pColumn->pValue->dataLength = dwLen;

                    memcpy(*pColumn->pValue->data.ppValue, pszValue, dwLen);
                }

                break;

            case VECS_DB_COLUMN_TYPE_UNKNOWN:
            default:

                dwError = ERROR_NOT_SUPPORTED;
                BAIL_ON_VECS_ERROR(dwError);
        }
    }

error:

    return dwError;
}



DWORD
VecsBindWideString(
    sqlite3_stmt* pSqlStatement,
    PSTR pszParamName,
    PCWSTR pwszValue
    )
{
    DWORD dwError = 0;
    int indx = -1;
    indx = sqlite3_bind_parameter_index( pSqlStatement, pszParamName);
    // This returns zero when pszParam Name is not found, which is
    // little strange since it means SQLITE_OK :(
    if ( indx == 0) {
        dwError = SQLITE_NOTFOUND;
    }
    BAIL_ON_VECS_ERROR(dwError);


    // if the data string does not live thru the SQL Operation,
    // SQLITE_STATIC need to be converted to SQLITE_TRANSIENT
    // if it is TRANSIENT, SQLite will make a copy of the data
    // and keep it around for the sql_step function.
    //
    // However in the case of VECS the copy is not needed and we are
    // *not* making a copy of the data.
    // if the data is destroyed before the actual SQL operation happens
    // it can lead to failures.

    if (pwszValue)
    {
        dwError = sqlite3_bind_text16(
                                    pSqlStatement,
                                    indx,
                                    pwszValue,
                                    -1,
                                    SQLITE_STATIC );
    }
    else
    {
        dwError = sqlite3_bind_null(pSqlStatement, indx);
    }
    BAIL_ON_VECS_ERROR(dwError);
error :
    return dwError;
}



DWORD
VecsBindString(
    sqlite3_stmt* pSqlStatement,
    PSTR pszParamName,
    PSTR pszValue
    )
{
    DWORD dwError = 0;
    int indx = -1;
    indx = sqlite3_bind_parameter_index( pSqlStatement, pszParamName);
    // This returns zero when pszParam Name is not found, which is
    // little strange since it means SQLITE_OK :(
    if ( indx == 0) {
        dwError = SQLITE_NOTFOUND;
    }
    BAIL_ON_VECS_ERROR(dwError);


    // if the data string does not live thru the SQL Operation,
    // SQLITE_STATIC need to be converted to SQLITE_TRANSIENT
    // if it is TRANSIENT, SQLite will make a copy of the data
    // and keep it around for the sql_step function.
    //
    // However in the case of VECS the copy is not needed and we are
    // *not* making a copy of the data.
    // if the data is destroyed before the actual SQL operation happens
    // it can lead to failures.

    if (pszValue)
    {
        dwError = sqlite3_bind_text(
                                        pSqlStatement,
                                        indx,
                                        pszValue,
                                        -1,
                                        SQLITE_STATIC );
    }
    else
    {
        dwError = sqlite3_bind_null(pSqlStatement, indx);
    }
    BAIL_ON_VECS_ERROR(dwError);
error :
    return dwError;
}



DWORD
VecsBindDword(
    sqlite3_stmt* pSqlStatement,
    PSTR pszParamName,
    DWORD dwValue
    )
{
    DWORD dwError = 0;
    int indx = -1;
    indx = sqlite3_bind_parameter_index( pSqlStatement, pszParamName);
    // This returns zero when pszParam Name is not found, which is
    // little strange since it means SQLITE_OK :(
    if ( indx == 0) {
        dwError = SQLITE_NOTFOUND;
    }
    BAIL_ON_VECS_ERROR(dwError);

    dwError = sqlite3_bind_int(
                            pSqlStatement,
                            indx,
                            dwValue);
    BAIL_ON_VECS_ERROR(dwError);
error :
    return dwError;
}


DWORD
VecsBindBlob(
    sqlite3_stmt* pSqlStatement,
    PSTR pszParamName,
    PBYTE pszValue,
    DWORD dwSize
    )
{
    DWORD dwError = 0;
    int indx = -1;
    indx = sqlite3_bind_parameter_index( pSqlStatement, pszParamName);
    // This returns zero when pszParam Name is not found, which is
    // little strange since it means SQLITE_OK :(
    if ( indx == 0) {
        dwError = SQLITE_NOTFOUND;
    }
    BAIL_ON_VECS_ERROR(dwError);

    if (pszValue)
    {
        dwError = sqlite3_bind_blob(
                        pSqlStatement,
                        indx,
                        pszValue,
                        dwSize,
                        SQLITE_STATIC);
    }
    else
    {
        dwError = sqlite3_bind_null(pSqlStatement, indx);
    }
    BAIL_ON_VECS_ERROR(dwError);

error:
    return dwError;
}

DWORD
VecsGetColunmIndexFromName(
    sqlite3_stmt* pSqlStatement,
    PSTR pszCloumnName
    )
{
    int iCurrColumn = 0;
    int iMaxColumn = 0;
    int indx = -1;
    iMaxColumn = sqlite3_column_count(pSqlStatement);

    while ( iCurrColumn < iMaxColumn) {
        if(strcmp(pszCloumnName,
            sqlite3_column_name(pSqlStatement, iCurrColumn)) == 0) {
            indx = iCurrColumn;
            break;
        }
        iCurrColumn++;
    }
    return indx;
}



DWORD
VecsDBGetColumnString(
    sqlite3_stmt* pSqlStatement,
    PSTR pszColumnName,
    PWSTR* pszValue)
{
    DWORD dwError = 0;
    int indx = -1;
    PWSTR psztmpValue = NULL;
    int nType = 0;

    indx = VecsGetColunmIndexFromName(pSqlStatement, pszColumnName);
    if ( indx == - 1)
    {
        dwError = SQLITE_NOTFOUND;
    }
    BAIL_ON_VECS_ERROR(dwError);

    nType = sqlite3_column_type(pSqlStatement, indx);
    if ( ( nType != SQLITE_TEXT) && (nType != SQLITE_NULL)){
        dwError = SQLITE_MISMATCH;
    }
    BAIL_ON_VECS_ERROR(dwError);

    psztmpValue = (PWSTR)sqlite3_column_text16(pSqlStatement, indx);
    if ( psztmpValue != NULL) {
    dwError = VmAfdAllocateStringW(
                psztmpValue,
                pszValue);
    BAIL_ON_VECS_ERROR(dwError);
    } else {
        *pszValue = NULL;
    }
error :
    return dwError;

}

DWORD
VecsDBGetColumnBlob(
    sqlite3_stmt* pSqlStatement,
    PSTR pszColumnName,
    PBYTE* pszValue,
    PDWORD pdwLen)
{
    DWORD dwError = 0;
    DWORD dwLen = 0;
    int indx = -1;
    PSTR psztmpValue = NULL;
    int nType = 0;
    indx = VecsGetColunmIndexFromName(pSqlStatement, pszColumnName);
    if ( indx == - 1)
    {
        dwError = SQLITE_NOTFOUND;
    }
    BAIL_ON_VECS_ERROR(dwError);

    nType = sqlite3_column_type(pSqlStatement, indx);
    if ( ( nType != SQLITE_BLOB) && (nType != SQLITE_NULL)){
        dwError = SQLITE_MISMATCH;
    }

    BAIL_ON_VECS_ERROR(dwError);

    dwLen = sqlite3_column_bytes(pSqlStatement, indx);
    psztmpValue = (PSTR) sqlite3_column_blob(pSqlStatement, indx);
    if ( psztmpValue != NULL) {

    // Allocate more to make it Zero Padded Memory Location
    dwError = VmAfdAllocateMemory(
                                dwLen + sizeof(wchar16_t),
                                (PVOID*)pszValue);
    BAIL_ON_VECS_ERROR(dwError);

    memcpy(*pszValue, psztmpValue, dwLen);
    *pdwLen = dwLen;
    } else {
        *pszValue = psztmpValue;
    }
error :
    return dwError;

}


DWORD
VecsDBGetColumnInt(
    sqlite3_stmt* pSqlStatement,
    PSTR pszColumnName,
    PDWORD pdwValue)
{
    DWORD dwError = 0;
    int indx = -1;
    int nType = 0;
    indx = VecsGetColunmIndexFromName(pSqlStatement, pszColumnName);
    if ( indx == - 1)
    {
        dwError = SQLITE_NOTFOUND;
    }
    BAIL_ON_VECS_ERROR(dwError);

    nType = sqlite3_column_type(pSqlStatement, indx);
    if ( ( nType != SQLITE_INTEGER) && (nType != SQLITE_NULL)){
        dwError = SQLITE_MISMATCH;
    }

    BAIL_ON_VECS_ERROR(dwError);
    *pdwValue = (DWORD) sqlite3_column_int(pSqlStatement, indx);

error :
    return dwError;
}

DWORD
VecsCheckifTableExists(
    sqlite3 * pDB ,
    const char *psztableName,
    PDWORD bExist)
{
    DWORD dwError = 0;
    DWORD dwEntriesAvailable = 0;
    char szQuery[] = "SELECT 1 as count FROM sqlite_master"
                     " WHERE type='table'"
                     " AND name=:table_name;";
    sqlite3_stmt * pSqlStatement = NULL;
    dwError = sqlite3_prepare_v2(
                pDB,
                szQuery,
                -1,
                &pSqlStatement,
                NULL);
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsBindString(pSqlStatement ,
                            ":table_name",
                            (PSTR)psztableName);
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsDbStepSql(pSqlStatement);
    if (( dwError == SQLITE_ROW ) || (dwError = SQLITE_DONE)) {
        dwError = 0;
    }
    BAIL_ON_VECS_ERROR(dwError);

    dwError  = VecsDBGetColumnInt(
                        pSqlStatement,
                        "count",
                        &dwEntriesAvailable);
    BAIL_ON_VECS_ERROR(dwError);
    if ( dwEntriesAvailable == 1) {
        *bExist = 1;
    }

error :
    if ( pSqlStatement != NULL) {
        sqlite3_finalize(pSqlStatement);
        pSqlStatement = NULL;
    }
    return dwError;
}


static
VOID
VmAfdSqliteLogCallback(
        PVOID *pArg,
        DWORD  dwError,
        PCSTR  pszMessage
        )
{
    VmAfdLog(VMAFD_DEBUG_ANY,
             "[SQLITE_ERROR:%d] - %s",
             dwError,
             IsNullOrEmptyString(pszMessage)?"":pszMessage
             );
}


static
DWORD
VecsDbLocalAuthenticationTablesCreate (
        sqlite3 *pDB
        )
{
    DWORD dwError = 0;

    if (!pDB)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VECS_ERROR (dwError);
    }

    dwError = VecsDbSqliteExecuteTransaction (
                                pDB,
                                "CREATE TABLE IF NOT EXISTS SDTable ("
                                "ID INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL,"
                                "StoreID INTEGER,"
                                "Revision INTEGER,"
                                "ContextSize INTEGER,"
                                "ContextBlob BLOB,"
                                "IsAclPresent INTEGER,"
                                "UNIQUE(StoreID),"
                                "CHECK( IsAclPresent == 0"
                                " OR IsAclPresent == 1),"
                                "FOREIGN KEY(StoreID) REFERENCES StoreTable(StoreID));"
                                );

    BAIL_ON_VECS_ERROR (dwError);

    dwError = VecsDbSqliteExecuteTransaction (
                                pDB,
                                "CREATE TABLE IF NOT EXISTS AceTable ("
                                "ID INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL,"
                                "StoreID INTEGER,"
                                "ContextSize INTEGER,"
                                "ContextBlob BLOB,"
                                "AccessMask INTEGER,"
                                "AccessType INTEGER,"
                                "UNIQUE(StoreID, ContextBlob),"
                                "FOREIGN KEY(StoreID) REFERENCES StoreTable(StoreID));"
                                );

    BAIL_ON_VECS_ERROR (dwError);

    dwError = VecsDbSqliteExecuteTransaction(
                                pDB,
                                "CREATE TRIGGER delete_store_SD "
                                "AFTER DELETE on StoreTable "
                                "BEGIN "
                                "DELETE FROM SDTable "
                                "WHERE StoreID = old.StoreID;"
                                "END;");
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsDbSqliteExecuteTransaction(
                                pDB,
                                "CREATE TRIGGER delete_store_Ace "
                                "AFTER DELETE on StoreTable "
                                "BEGIN "
                                "DELETE FROM AceTable "
                                "WHERE StoreID = old.StoreID;"
                                "END;");
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsDbSqliteExecuteTransaction (
                                 pDB,
                                 "CREATE TRIGGER delete_ace_on_insert "
                                 "AFTER INSERT on AceTable "
                                 "FOR EACH ROW "
                                 "BEGIN "
                                 "DELETE FROM AceTable "
                                 "WHERE AccessMask = 0;"
                                 "END;"
                                  );
    BAIL_ON_VECS_ERROR (dwError);

    dwError = VecsDbSqliteExecuteTransaction (
                                  pDB,
                                  "CREATE TRIGGER delete_ace_on_update "
                                  "AFTER UPDATE on AceTable "
                                  "FOR EACH ROW "
                                  "BEGIN "
                                  "DELETE FROM AceTable "
                                  "WHERE AccessMask = 0;"
                                  "END;"
                                  );
    BAIL_ON_VECS_ERROR (dwError);



cleanup:
    return dwError;

error:
    goto cleanup;
}


static
DWORD
CdcCreateAppDatabase(
    const char* pszDBName
    )
{
    DWORD dwError = 0;
    sqlite3 * pDB = NULL;
    BOOL bInTx = FALSE;
    DWORD bTableExists = 0;

    dwError = sqlite3_open_v2(
                        pszDBName,
                        &pDB,
                        SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE,
                        NULL
                        );
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VecsCheckifTableExists(pDB, "DCTable", &bTableExists);
    BAIL_ON_VECS_ERROR(dwError);

    if (bTableExists != 1)
    {

        VecsDbBeginTransaction(pDB);
        bInTx = TRUE;

        dwError = VecsDbSqliteExecuteTransaction(
                        pDB,
                        "CREATE TABLE IF NOT EXISTS DCTable ("
                        "DCID INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL,"
                        "DCName  VARCHAR(256)  COLLATE NOCASE NOT NULL,"
                        "Site VARCHAR(128),"
                        "Domain VARCHAR(256),"
                        "LastPing DATE DEFAULT (STRFTIME('%s','now')),"
                        "PingResponse INTEGER,"
                        "PingError    INTEGER,"
                        "IsAlive INTEGER,"
                        "UNIQUE(DCName,Domain));"
                        );
        BAIL_ON_VECS_ERROR(dwError);

        dwError = VecsDbSqliteExecuteTransaction(
                        pDB,
                        "CREATE TABLE IF NOT EXISTS AffinitizedDC ("
                        "DCID INTEGER,"
                        "Domain VARCHAR(256),"
                        "AffinitizedSince DATE DEFAULT (STRFTIME('%s','now')),"
                        "UNIQUE(Domain),"
                        "FOREIGN KEY(DCID) REFERENCES DCTable(DCID));"
                        );
        BAIL_ON_VECS_ERROR(dwError);

        dwError = VecsDbSqliteExecuteTransaction(
                        pDB,
                        "CREATE TABLE IF NOT EXISTS AfdProperties ("
                        "Property VARCHAR(128) COLLATE NOCASE NOT NULL,"
                        "Value INTEGER,"
                        " UNIQUE(Property)"
                        " ON CONFLICT REPLACE);"
                        );
        BAIL_ON_VECS_ERROR(dwError);

        dwError = VecsDbSqliteExecuteTransaction(
                       pDB,
                       "CREATE TABLE IF NOT EXISTS DCServiceStatus ("
                       "DCID INTEGER,"
                       "ServiceName VARCHAR(128) COLLATE NOCASE NOT NULL,"
                       "Port INTEGER,"
                       "IsAlive INTEGER,"
                       "LastHeartbeat INTEGER,"
                       "FOREIGN KEY(DCID) REFERENCES DCTable(DCID),"
                       "UNIQUE(ServiceName,Port,DCID));"
                       );
        BAIL_ON_VECS_ERROR(dwError);

        dwError = VecsDbSqliteExecuteTransaction(
                                pDB,
                                "CREATE TRIGGER delete_dc "
                                "AFTER DELETE on DCTable "
                                "BEGIN "
                                "DELETE FROM AffinitizedDC "
                                "WHERE DCID = old.DCID;"
                                "END;");
        BAIL_ON_VECS_ERROR(dwError);

        dwError = VecsDbSqliteExecuteTransaction(
                                pDB,
                                "CREATE TRIGGER delete_dc_status "
                                "AFTER DELETE on DCTable "
                                "BEGIN "
                                "DELETE FROM DCServiceStatus "
                                "WHERE DCID = old.DCID;"
                                "END;");
        BAIL_ON_VECS_ERROR(dwError);


        VecsDbCommitTransaction(pDB);
    }

cleanup:

    if (pDB)
    {
        VecsDbDatabaseClose(pDB);
    }

    return dwError;

error:

    if (bInTx)
    {
        VecsDbRollbackTransaction(pDB);
    }

    goto cleanup;
}

