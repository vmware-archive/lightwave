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

DWORD
VmcaDbCreateContext(
    PVMCA_DB_CONTEXT* ppDbContext
    )
{
    DWORD dwError = 0;
    PVMCA_DB_CONTEXT pDbContext = NULL;
    BOOLEAN bInLock = FALSE;

    VMCA_LOG_(VMCA_LOG_LEVEL_DEBUG,"Creating VMCA database context");
    VMCA_LOCK_MUTEX(bInLock, &gVmcaDbGlobals.mutex);

    if (gVmcaDbGlobals.pDbContextList)
    {
        pDbContext = gVmcaDbGlobals.pDbContextList;
        gVmcaDbGlobals.pDbContextList = gVmcaDbGlobals.pDbContextList->pNext;

        pDbContext->pNext = NULL;

        gVmcaDbGlobals.dwNumCachedContexts--;
    }
    else
    {
        VMCA_LOG_DEBUG("Allocating database context for %s", gVmcaDbGlobals.pszDbPath);
        dwError = VMCAAllocateMemory(sizeof(*pDbContext), (PVOID*)&pDbContext);
        BAIL_ON_VMCA_ERROR(dwError);

        dwError = sqlite3_open(
            gVmcaDbGlobals.pszDbPath,
            &pDbContext->pDb);
        BAIL_ON_VMCA_ERROR(dwError);

        dwError = sqlite3_busy_timeout(
                    pDbContext->pDb,
                    5000);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    *ppDbContext = pDbContext;

cleanup:
    VMCA_UNLOCK_MUTEX(bInLock, &gVmcaDbGlobals.mutex);
    VMCA_LOG_DEBUG("VMCA database context created Error = %d", dwError);
    return dwError;

error:

    *ppDbContext = NULL;

    if (pDbContext)
    {
        VmcaDbFreeContext(pDbContext);
    }

    goto cleanup;
}

VOID
VmcaDbReleaseContext(
    PVMCA_DB_CONTEXT pDbContext
    )
{
    BOOLEAN bInLock = FALSE;
    VMCA_LOG_(VMCA_LOG_LEVEL_DEBUG,"Releasing VMCA database context");

    // Rollback outstanding explicit transaction if exists.
    VmcaDbCtxRollbackTransaction(pDbContext);

    VMCA_LOCK_MUTEX(bInLock, &gVmcaDbGlobals.mutex);
    if (gVmcaDbGlobals.dwNumCachedContexts <
                                gVmcaDbGlobals.dwMaxNumCachedContexts)
    {
        pDbContext->pNext = gVmcaDbGlobals.pDbContextList;
        gVmcaDbGlobals.pDbContextList = pDbContext;

        gVmcaDbGlobals.dwNumCachedContexts++;
    }
    else
    {
        VMCA_LOG_(VMCA_LOG_LEVEL_DEBUG,"Freeing VMCA database context");
        VmcaDbFreeContext(pDbContext);
    }

    VMCA_UNLOCK_MUTEX(bInLock, &gVmcaDbGlobals.mutex);
    VMCA_LOG_(VMCA_LOG_LEVEL_DEBUG,"VMCA database context released");
}

VOID
VmcaDbFreeContext(
    PVMCA_DB_CONTEXT pDbContext
    )
{
    while (pDbContext)
    {
        PVMCA_DB_CONTEXT pContext = pDbContext;

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
        if (pContext->pQueryRevokedCertificates)
        {
            sqlite3_finalize(pContext->pQueryRevokedCertificates);
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
        if (pContext->pVerifyCertQuery)
        {
            sqlite3_finalize(pContext->pVerifyCertQuery);
        }
        if (pContext->pGetCertID)
        {
            sqlite3_finalize(pContext->pGetCertID);
        }
        if (pContext->pRevokeCert)
        {
            sqlite3_finalize(pContext->pRevokeCert);
        }
        if (pContext->pRevokedCertCount)
        {
            sqlite3_finalize(pContext->pRevokedCertCount);
        }
        if (pContext->pGetCrlNumber)
        {
            sqlite3_finalize(pContext->pGetCrlNumber);
        }
        if (pContext->pSetCrlNumber)
        {
            sqlite3_finalize(pContext->pSetCrlNumber);
        }

        if (pContext->pDb)
        {
            VmcaDbDatabaseClose(pContext->pDb);
        }

        VMCAFreeMemory(pContext);
    }
}
