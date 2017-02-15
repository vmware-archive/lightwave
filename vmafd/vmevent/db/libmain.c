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
 * Filename: externs.h
 *
 * Abstract:
 *
 * Thinapp EventLogsitory Database
 *
 * Library Entry points
 *
 */

#include "includes.h"

int _forceCRTManifestCUR = 0;

DWORD
EventLogDbInitialize(
    PCSTR pszDbPath
    )
{
    DWORD dwError = 0;

    if (IsNullOrEmptyString(pszDbPath))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_EVENTLOG_ERROR(dwError);
    }

    dwError = EventLogAllocateStringA(pszDbPath, &gEventLogDbGlobals.pszDbPath);
    BAIL_ON_EVENTLOG_ERROR(dwError);

error:

    return dwError;
}

VOID
EventLogDbShutdown(
    VOID
    )
{
    BOOLEAN bInLock = FALSE;

    VMEVENT_LOCK_MUTEX(bInLock, &gEventLogDbGlobals.mutex);

    if (gEventLogDbGlobals.pDbContextList)
    {
        EventLogInternalDbFreeContext(gEventLogDbGlobals.pDbContextList);

        gEventLogDbGlobals.pDbContextList = NULL;
        gEventLogDbGlobals.dwNumCachedContexts = 0;
    }

    VMEVENT_SAFE_FREE_MEMORY(gEventLogDbGlobals.pszDbPath);
    gEventLogDbGlobals.pszDbPath = NULL;

    VMEVENT_UNLOCK_MUTEX(bInLock, &gEventLogDbGlobals.mutex);
}

DWORD
EventLogDbReset(
    VOID
    )
{
    DWORD dwError = 0;

    dwError = EventLogDbResetAppDatabase();

    return dwError;
}

VOID
EventLogDbFreeEventContainer(
    PEVENTLOG_DB_EVENT_CONTAINER pContainer
    )
{
    if (pContainer->pEventEntries)
    {
        EventLogDbFreeEventEntryArray(pContainer->pEventEntries, pContainer->dwCount);
    }

    EventLogFreeMemory(pContainer);
}

VOID
EventLogDbFreeEventEntry(
    PEVENTLOG_DB_EVENT_ENTRY pEventEntry
    )
{
    EventLogDbFreeEventEntryContents(pEventEntry);

    EventLogFreeMemory(pEventEntry);
}

VOID
EventLogDbFreeEventEntryArray(
    PEVENTLOG_DB_EVENT_ENTRY pEventEntryArray,
    DWORD                   dwCount
    )
{
    int iEntry = 0;

    for (; iEntry < dwCount; iEntry++)
    {
        EventLogDbFreeEventEntryContents(&pEventEntryArray[iEntry]);
    }

    EventLogFreeMemory(pEventEntryArray);
}

VOID
EventLogDbFreeEventEntryContents(
    PEVENTLOG_DB_EVENT_ENTRY pEventEntry
    )
{
    VMEVENT_SAFE_FREE_MEMORY(pEventEntry->pwszEventMessage);
}
