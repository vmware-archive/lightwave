/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : database.c
 *
 * Abstract :
 *
 */
#include "includes.h"

DWORD
EventLogDbDatabaseInitialize(
    const char * pszAppDBName,
    sqlite3 ** ppDB
    )
{
    DWORD dwError = 0;

    dwError = EventLogDbCreateAppDatabase(pszAppDBName, ppDB);
    BAIL_ON_EVENTLOG_ERROR(dwError);

#ifndef _WIN32
    if (chown(pszAppDBName, 0, 0) < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_EVENTLOG_ERROR(dwError);
    }

    if (chmod(pszAppDBName, S_IRUSR | S_IWUSR) < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_EVENTLOG_ERROR(dwError);
    }
#endif

error:

    return dwError;
}

DWORD
EventLogDbCreateAppDatabase(
    const char *pszAppDBName,
    sqlite3 ** ppDB
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
    BAIL_ON_EVENTLOG_ERROR(dwError);
 
    dwError = EventLogDbSqliteExecuteTransaction(
                        pDB,
                        "PRAGMA page_size = 4096;"
                        );
    BAIL_ON_EVENTLOG_ERROR(dwError);

    dwError = EventLogDbSqliteExecuteTransaction(
                        pDB,
                        "PRAGMA default_cache_size = 10000;"
                        );
    BAIL_ON_EVENTLOG_ERROR(dwError);

    EventLogDbBeginTransaction(pDB);
    bInTx = TRUE;

    dwError = EventLogDbSqliteExecuteTransaction(
    pDB,
        "CREATE TABLE IF NOT EXISTS EventLogTable ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL,"
        "EventID INTEGER,"
        "EventType INTEGER,"
        "Message VARCHAR(2000) COLLATE NOCASE,"
        "Description VARCHAR(2000) COLLATE NOCASE)"
        );
    BAIL_ON_EVENTLOG_ERROR(dwError);

    EventLogDbCommitTransaction(pDB);

    *ppDB = pDB;

cleanup:

    return dwError;

error:

    if (bInTx)
    {
    EventLogDbRollbackTransaction(pDB);
    bInTx = FALSE;
    }

    if (pDB)
    {
        EventLogDbDatabaseClose(pDB);
    }

    *ppDB = NULL;

    goto cleanup;
}

DWORD
EventLogDbResetAppDatabase(
    VOID
    )
{
    DWORD dwError = 0;
    BOOL bInTx = FALSE;
    PEVENTLOG_DB_CONTEXT pDbContext = NULL;

    dwError = EventLogInternalDbCreateContext(&pDbContext);
    BAIL_ON_EVENTLOG_ERROR(dwError);

    dwError = EventLogDbBeginTransaction(pDbContext->pDb);
    BAIL_ON_EVENTLOG_ERROR(dwError);
    bInTx = TRUE;

    dwError = EventLogDbSqliteExecuteTransaction(
                pDbContext->pDb,
                "DELETE FROM EventLogTable;"
                );
    BAIL_ON_EVENTLOG_ERROR(dwError);

cleanup:

    if (pDbContext)
    {
        EventLogDbReleaseContext(pDbContext);
    }

    return dwError;

error:

    if (pDbContext && bInTx)
    {
        EventLogDbRollbackTransaction(pDbContext->pDb);
    }

    goto cleanup;
}

DWORD
EventLogDbSqliteExecuteTransaction(
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
    BAIL_ON_EVENTLOG_ERROR(dwError);

    dwError = sqlite3_step(stmt);
    if (dwError == SQLITE_DONE)
    {
        dwError = 0;
    }
    BAIL_ON_EVENTLOG_ERROR(dwError);

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
EventLogDbPrepareSql(
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
EventLogDbStepSql(
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
EventLogDbBeginTransaction(
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
EventLogDbCtxBeginTransaction(
	HEVENTLOG_DB hDatabase
    )
{
    PEVENTLOG_DB_CONTEXT pDbContext = NULL;
	DWORD dwError = 0;

	pDbContext = (PEVENTLOG_DB_CONTEXT)hDatabase;
	assert(pDbContext);

	// Sqllite does not support nested tx, this will fail if we already in tx.
	dwError = EventLogDbBeginTransaction(pDbContext->pDb);
	if (dwError == SQLITE_OK)
	{
		pDbContext->bInTx = TRUE;
	}

	return dwError;
}

DWORD
EventLogDbCommitTransaction(
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
EventLogDbCtxCommitTransaction(
	HEVENTLOG_DB hDatabase
    )
{
    PEVENTLOG_DB_CONTEXT pDbContext = NULL;
	DWORD dwError = 0;

	pDbContext = (PEVENTLOG_DB_CONTEXT)hDatabase;
	assert(pDbContext);

	if (pDbContext->bInTx)
	{
		dwError = EventLogDbCommitTransaction(pDbContext->pDb);
		if (dwError == SQLITE_OK)
		{
			pDbContext->bInTx = FALSE;
		}
	}

	return dwError;
}

DWORD
EventLogDbRollbackTransaction(
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
EventLogDbCtxRollbackTransaction(
	HEVENTLOG_DB hDatabase
    )
{
    PEVENTLOG_DB_CONTEXT pDbContext = NULL;
	DWORD dwError = 0;

    pDbContext = (PEVENTLOG_DB_CONTEXT)hDatabase;
	assert(pDbContext);

	if (pDbContext->bInTx)
	{
		dwError = EventLogDbRollbackTransaction(pDbContext->pDb);
		pDbContext->bInTx = FALSE;
	}

	return dwError;
}

VOID
EventLogDbDatabaseClose(
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
EventLogDbFillValues(
    sqlite3_stmt*   pSqlStatement,
    PEVENTLOG_DB_COLUMN pColumnArray,
    DWORD           dwNumColumns
    )
{
    DWORD dwError = 0;
    DWORD iColumn = 0;

    for (; iColumn < dwNumColumns; iColumn++)
    {
        DWORD dwLen = 0;
        PEVENTLOG_DB_COLUMN pColumn = &pColumnArray[iColumn];

        switch (pColumn->columnType)
        {
            case EVENTLOG_DB_COLUMN_TYPE_INT32:

                if (!pColumn->pValue->data.ppdwValue ||
                    !*pColumn->pValue->data.ppdwValue)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_EVENTLOG_ERROR(dwError);
                }

                **pColumn->pValue->data.ppdwValue =
                        sqlite3_column_int(pSqlStatement, iColumn);
                pColumn->pValue->dataLength = sizeof(DWORD);

                break;

            case EVENTLOG_DB_COLUMN_TYPE_STRING:

                dwLen = sqlite3_column_bytes(pSqlStatement, iColumn);

                if (dwLen > 0)
                {
                    unsigned const char* pszValue =
                            sqlite3_column_text(pSqlStatement, iColumn);

                    if (!pColumn->pValue->data.ppwszValue)
                    {
                        dwError = ERROR_INVALID_PARAMETER;
                        BAIL_ON_EVENTLOG_ERROR(dwError);
                    }

                    dwError = EventLogAllocateStringWFromA(
                                    pszValue,
                                    pColumn->pValue->data.ppwszValue);
                    BAIL_ON_EVENTLOG_ERROR(dwError);

#ifndef _WIN32
                    dwError = LwWc16sLen(
                                    *pColumn->pValue->data.ppwszValue,
                                    &pColumn->pValue->dataLength);
#else
                    pColumn->pValue->dataLength = wcslen(
                                    *pColumn->pValue->data.ppwszValue);
#endif
                    BAIL_ON_EVENTLOG_ERROR(dwError);
                }

                break;

            case EVENTLOG_DB_COLUMN_TYPE_BLOB:

                dwLen = sqlite3_column_bytes(pSqlStatement, iColumn);
                if (dwLen > 0)
                {
                    unsigned const char* pszValue =
                            sqlite3_column_blob(pSqlStatement, iColumn);

                    if (!pColumn->pValue->data.ppValue)
                    {
                        dwError = ERROR_INVALID_PARAMETER;
                        BAIL_ON_EVENTLOG_ERROR(dwError);
                    }

                    dwError = EventLogAllocateMemory(
                                    dwLen,
                                    (PVOID*)pColumn->pValue->data.ppValue);
                    BAIL_ON_EVENTLOG_ERROR(dwError);

                    pColumn->pValue->dataLength = dwLen;

                    memcpy(*pColumn->pValue->data.ppValue, pszValue, dwLen);
                }

                break;

            case EVENTLOG_DB_COLUMN_TYPE_UNKNOWN:
            default:

                dwError = ERROR_NOT_SUPPORTED;
                BAIL_ON_EVENTLOG_ERROR(dwError);
        }
    }

error:

    return dwError;
}

