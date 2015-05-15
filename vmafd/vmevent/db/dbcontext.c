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
 * Module Name: ThinAppEventLogService
 *
 * Filename: dbcontext.c
 *
 * Abstract:
 *
 * Thinapp EventLogsitory Database
 *
 * Database Context Management
 *
 */

#include "includes.h"

DWORD
EventLogInternalDbCreateContext(
    PEVENTLOG_DB_CONTEXT * ppDbContext
    )
{
    DWORD dwError = 0;
    PEVENTLOG_DB_CONTEXT pDbContext = NULL;
    BOOLEAN bInLock = FALSE;

    VMEVENT_LOCK_MUTEX(bInLock, &gEventLogDbGlobals.mutex);

    if (gEventLogDbGlobals.pDbContextList)
    {
        pDbContext = gEventLogDbGlobals.pDbContextList;
        gEventLogDbGlobals.pDbContextList = gEventLogDbGlobals.pDbContextList->pNext;

        pDbContext->pNext = NULL;

        gEventLogDbGlobals.dwNumCachedContexts--;
    }
    else
    {
        dwError = EventLogAllocateMemory(sizeof(*pDbContext), (PVOID*)&pDbContext);
        BAIL_ON_EVENTLOG_ERROR(dwError);

        if (!gEventLogDbGlobals.bIsDBOpened)
        {
			dwError = EventLogDbDatabaseInitialize(
							gEventLogDbGlobals.pszDbPath, // invariant
							&pDbContext->pDb);
			BAIL_ON_EVENTLOG_ERROR(dwError);

			gEventLogDbGlobals.bIsDBOpened = TRUE;
        }
    }

    *ppDbContext = pDbContext;

cleanup:

    VMEVENT_UNLOCK_MUTEX(bInLock, &gEventLogDbGlobals.mutex);

    return dwError;

error:

    *ppDbContext = NULL;

    if (pDbContext)
    {
        EventLogDbFreeContext(pDbContext);
    }

    goto cleanup;
}

VOID
EventLogInternalDbReleaseContext(
    PEVENTLOG_DB_CONTEXT pDbContext
    )
{
    BOOLEAN bInLock = FALSE;

    // Rollback outstanding explicit transaction if exists.
    EventLogDbCtxRollbackTransaction(pDbContext);

    VMEVENT_LOCK_MUTEX(bInLock, &gEventLogDbGlobals.mutex);

    if (gEventLogDbGlobals.dwNumCachedContexts <
                                gEventLogDbGlobals.dwMaxNumCachedContexts)
    {
        pDbContext->pNext = gEventLogDbGlobals.pDbContextList;
        gEventLogDbGlobals.pDbContextList = pDbContext;

        gEventLogDbGlobals.dwNumCachedContexts++;
    }
    else
    {
        EventLogDbFreeContext(pDbContext);
    }

    VMEVENT_UNLOCK_MUTEX(bInLock, &gEventLogDbGlobals.mutex);
}




VOID
EventLogInternalDbFreeContext(
    PEVENTLOG_DB_CONTEXT pDbContext
    )
{

    while (pDbContext)
    {
        PEVENTLOG_DB_CONTEXT pContext = pDbContext;

        pDbContext = pDbContext->pNext;

        if (pContext->pDelEventQuery)
        {
            sqlite3_finalize(pContext->pDelEventQuery);
        }

        if (pContext->pDelEventAppsQuery)
        {
            sqlite3_finalize(pContext->pDelEventAppsQuery);
        }

        if (pContext->pAddEventQuery)
        {
            sqlite3_finalize(pContext->pAddEventQuery);
        }

        if (pContext->pEventIDByNameQuery)
        {
            sqlite3_finalize(pContext->pEventIDByNameQuery);
        }

        if (pContext->pEventVersionIDByNameQuery)
        {
            sqlite3_finalize(pContext->pEventVersionIDByNameQuery);
        }


        if (pContext->pEventCountByNameQuery)
        {
            sqlite3_finalize(pContext->pEventCountByNameQuery);
        }

        if (pContext->pTotalEventCountQuery)
        {
            sqlite3_finalize(pContext->pTotalEventCountQuery);
        }


        if (pContext->pQuerySingleEvent)
        {
            sqlite3_finalize(pContext->pQuerySingleEvent);
        }

        if (pContext->pQueryAllEvents)
        {
            sqlite3_finalize(pContext->pQueryAllEvents);
        }

        if (pContext->pQueryEventsPaged)
        {
            sqlite3_finalize(pContext->pQueryEventsPaged);
        }

        if (pContext->pDb)
        {
            EventLogDbDatabaseClose(pContext->pDb);
        }

        EventLogFreeMemory(pContext);
    }
}

DWORD
EventLogDbCreateContext(
        PHEVENTLOG_DB hDatabase
        )
{
    DWORD dwError = 0;
    PEVENTLOG_DB_CONTEXT pDBContext = NULL;

    dwError = EventLogInternalDbCreateContext(&pDBContext);

    *hDatabase = pDBContext;

    return dwError;
}

VOID
EventLogDbReleaseContext(
        HEVENTLOG_DB hDatabase
        )
{
    PEVENTLOG_DB_CONTEXT pDBContext = NULL;
    pDBContext = (PEVENTLOG_DB_CONTEXT)hDatabase;
    EventLogInternalDbReleaseContext(pDBContext);
}

VOID
EventLogDbFreeContext(
        HEVENTLOG_DB hDatabase
        )
{
    PEVENTLOG_DB_CONTEXT pDBContext = NULL;
    pDBContext = (PEVENTLOG_DB_CONTEXT)hDatabase;
	EventLogInternalDbFreeContext(pDBContext);
}
