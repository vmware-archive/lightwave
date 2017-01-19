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
 * Filename: dbcontext.c
 *
 * Abstract:
 *
 * VMware Certificate Server Database
 *
 * Database Context Management
 *
 */

#include "includes.h"

static
DWORD
VmAfdDbGetSqliteOpenMode(
      VMAFD_DB_MODE vmafdOpenMode,
      PDWORD        pdwOpenMode
      );

static
VOID
VecsDbFreeStmtArray (
      PVECS_DB_STMT_ARRAY pStmtArray
      );

DWORD
VecsDbCreateContext(
    PVECS_DB_CONTEXT* ppDbContext,
    VMAFD_DB_MODE     vmafdOpenMode
    )
{
    DWORD dwError = 0;
    PVECS_DB_CONTEXT pDbContext = NULL;
    DWORD dwSqlite3OpenMode = 0;

    VECS_DB_LOCK_MUTEX(&gVecsDbGlobals.mutex);
    if (gVecsDbGlobals.pDbContextList)
    {
        PVECS_DB_CONTEXT pDbContextCursor = gVecsDbGlobals.pDbContextList;
        PVECS_DB_CONTEXT pDbContextPrev = NULL;

        while (pDbContextCursor)
        {
            if (pDbContextCursor->dbOpenMode == vmafdOpenMode)
            {
                pDbContext = pDbContextCursor;
                if (pDbContextPrev)
                {
                    pDbContextPrev->pNext = pDbContextCursor->pNext;
                }
                else
                {
                    gVecsDbGlobals.pDbContextList = pDbContext->pNext;
                }
                pDbContext->pNext = NULL;
                gVecsDbGlobals.dwNumCachedContexts--;
                break;
            }

            pDbContextPrev = pDbContextCursor;
            pDbContextCursor = pDbContextCursor->pNext;
        }
    }
    if (!pDbContext)
    {
        dwError = VmAfdAllocateMemory(sizeof(*pDbContext), (PVOID*)&pDbContext);
        BAIL_ON_VECS_ERROR(dwError);

        dwError = VmAfdDbGetSqliteOpenMode(vmafdOpenMode, &dwSqlite3OpenMode);
        BAIL_ON_VECS_ERROR(dwError);

        dwError = sqlite3_open_v2(
                    gVecsDbGlobals.pszDbPath,
                    &pDbContext->pDb,
                    dwSqlite3OpenMode,
                    NULL
                    );
        BAIL_ON_VECS_ERROR(dwError);

        pDbContext->dbOpenMode = vmafdOpenMode;

        dwError = sqlite3_busy_timeout(
                    pDbContext->pDb,
                    60000);
        BAIL_ON_VECS_ERROR(dwError);
    }

    *ppDbContext = pDbContext;

cleanup:

    VECS_DB_UNLOCK_MUTEX(&gVecsDbGlobals.mutex);

    return dwError;

error:

    *ppDbContext = NULL;

    if (pDbContext)
    {
        VecsDbFreeContext(pDbContext);
    }

    goto cleanup;
}

VOID
VecsDbReleaseContext(
    PVECS_DB_CONTEXT pDbContext
    )
{
    // Rollback outstanding explicit transaction if exists.
    VecsDbCtxRollbackTransaction(pDbContext);

    VECS_DB_LOCK_MUTEX(&gVecsDbGlobals.mutex);

    if (gVecsDbGlobals.dwNumCachedContexts <
                                gVecsDbGlobals.dwMaxNumCachedContexts)
    {
        pDbContext->pNext = gVecsDbGlobals.pDbContextList;
        gVecsDbGlobals.pDbContextList = pDbContext;

        gVecsDbGlobals.dwNumCachedContexts++;
    }
    else
    {
        VecsDbFreeContext(pDbContext);
    }

    VECS_DB_UNLOCK_MUTEX(&gVecsDbGlobals.mutex);
}

VOID
VecsDbFreeContext(
    PVECS_DB_CONTEXT pDbContext
    )
{
    while (pDbContext)
    {
        PVECS_DB_CONTEXT pContext = pDbContext;

        pDbContext = pDbContext->pNext;

        if (pContext->pAddCertQuery)
        {
            sqlite3_finalize(pContext->pAddCertQuery);
        }
        if (pContext->pQueryAllCertificates)
        {
            sqlite3_finalize(pContext->pQueryAllCertificates);
        }
        if (pContext->pQueryCertificatesPaged)
        {
            sqlite3_finalize(pContext->pQueryCertificatesPaged);
        }
        if (pContext->pTotalCertCountQuery)
        {
            sqlite3_finalize(pContext->pTotalCertCountQuery);
        }
        if (pContext->pDelCertQuery)
        {
            sqlite3_finalize(pContext->pDelCertQuery);
        }
        if (pContext->pUpdateCertQuery)
        {
            sqlite3_finalize(pContext->pUpdateCertQuery);
        }
        if (pContext->pGetCertFromAliasQuery)
        {
            sqlite3_finalize(pContext->pGetCertFromAliasQuery);
        }
        if (pContext->pVerifyCertQuery)
        {
            sqlite3_finalize(pContext->pVerifyCertQuery);
        }
        if (pContext->pGetCountQuery)
        {
            sqlite3_finalize(pContext->pGetCountQuery);
        }
        if (pContext->pCreateStoreTable)
        {
            sqlite3_finalize(pContext->pCreateStoreTable);
        }
        if (pContext->pDeleteStore)
        {
            sqlite3_finalize(pContext->pDeleteStore);
        }
        if (pContext->pQueryStoreTable)
        {
            sqlite3_finalize(pContext->pQueryStoreTable);
        }
        if (pContext->pGetPKeyFromAlias)
        {
            sqlite3_finalize(pContext->pGetPKeyFromAlias);
        }
        if (pContext->pQueryInfoLevel1)
        {
            sqlite3_finalize(pContext->pQueryInfoLevel1);
        }
        if (pContext->pQueryInfoLevel2)
        {
            sqlite3_finalize(pContext->pQueryInfoLevel2);
        }
        if (pContext->pQueryTypeByAlias)
        {
            sqlite3_finalize(pContext->pQueryTypeByAlias);
        }
        if (pContext->pQueryDateByAlias)
        {
            sqlite3_finalize(pContext->pQueryDateByAlias);
        }
        if (pContext->pQuerySetSecurityDescriptor)
        {
            sqlite3_finalize(pContext->pQuerySetSecurityDescriptor);
        }
        if (pContext->pQueryGetSecurityDescriptor)
        {
            sqlite3_finalize(pContext->pQueryGetSecurityDescriptor);
        }
        if (pContext->pQueryAddAces)
        {
            sqlite3_finalize (pContext->pQueryAddAces);
        }
        if (pContext->pQueryGetAces)
        {
            sqlite3_finalize (pContext->pQueryGetAces);
        }
        if (pContext->pQueryAddAcesArray)
        {
            VecsDbFreeStmtArray(pContext->pQueryAddAcesArray);
        }


        if (pContext->pDb)
        {
            VecsDbDatabaseClose(pContext->pDb);
        }

        VmAfdFreeMemory(pContext);
    }
}

static
DWORD
VmAfdDbGetSqliteOpenMode(
      VMAFD_DB_MODE vmafdOpenMode,
      PDWORD        pdwOpenMode
      )
{
    DWORD dwError = 0;
    DWORD dwOpenMode = 0;

    if (!pdwOpenMode)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VECS_ERROR(dwError);
    }

    switch (vmafdOpenMode)
    {
        case VMAFD_DB_MODE_READ:
            dwOpenMode = SQLITE_OPEN_READONLY;
            break;

        case VMAFD_DB_MODE_WRITE:
            dwOpenMode = SQLITE_OPEN_READWRITE;
            break;

        default:
            dwError = ERROR_INVALID_PARAMETER;
            break;
    }
    BAIL_ON_VECS_ERROR(dwError);

    *pdwOpenMode = dwOpenMode;

cleanup:

    return dwError;
error:

    goto cleanup;
}

static
VOID
VecsDbFreeStmtArray(
      PVECS_DB_STMT_ARRAY pStmtArray
      )
{
    DWORD dwIndx = 0;

    if (pStmtArray)
    {
        for (; dwIndx < pStmtArray->dwCount; dwIndx++)
        {
            sqlite3_stmt* pStmtCursor = pStmtArray->pStmtToExecute[dwIndx];
            sqlite3_finalize(pStmtCursor);
        }
        VMAFD_SAFE_FREE_MEMORY (pStmtArray->pStmtToExecute);
    }
    VMAFD_SAFE_FREE_MEMORY (pStmtArray);
}
