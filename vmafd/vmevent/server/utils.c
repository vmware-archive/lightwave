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
 * Filename: utils.c
 *
 * Abstract:
 *
 * Utility functions
 *
 */

#include "includes.h"




VOID
EventLogRpcFreeEventContainer(
    PEVENTLOG_CONTAINER pContainer
    )
{
    if (pContainer->pPkgEntries)
    {
        EventLogRpcFreeEventEntryArray(pContainer->pPkgEntries, pContainer->dwCount);
    }

    EVENTLOG_RPC_SAFE_FREE_MEMORY(pContainer);
}


VOID
EventLogRpcFreeEventEntryArray(
   PEVENTLOG_ENTRY pPkgEntryArray,
   DWORD               dwCount
   )
{
    DWORD iEntry = 0;

    for (; iEntry < dwCount; iEntry++)
    {
        EventLogRpcFreeEventEntryContents(&pPkgEntryArray[iEntry]);
    }

    EVENTLOG_RPC_SAFE_FREE_MEMORY(pPkgEntryArray);
}


VOID
EventLogRpcFreeEventEntryContents(
    PEVENTLOG_ENTRY pPkgEntry
    )
{
    //EVENTLOG_RPC_SAFE_FREE_MEMORY(pPkgEntry->pszMessage);

}

VOID
EventLogFreeEventContainer(
    PEVENTLOG_CONTAINER pContainer
    )
{
    if (pContainer->pPkgEntries)
    {
        EventLogFreeEventEntryArray(pContainer->pPkgEntries, pContainer->dwCount);
    }

    EventLogFreeMemory(pContainer);
}

VOID
EventLogFreeEventEntryArray(
    PEVENTLOG_ENTRY pPkgEntryArray,
    DWORD               dwCount
    )
{
    DWORD iEntry = 0;

    for (; iEntry < dwCount; iEntry++ )
    {
        EventLogFreeEventEntryContents(&pPkgEntryArray[iEntry]);
    }

    EventLogFreeMemory(pPkgEntryArray);
}

VOID
EventLogFreeEventEntryContents(
    PEVENTLOG_ENTRY pPkgEntry
    )
{
    //VMEVENT_SAFE_FREE_MEMORY(pPkgEntry->pszMessage);
}

DWORD
EventLogGetApplicationVersion(
    VOID
    )
{
    DWORD dwVersion = 0;

    dwVersion = 123;

    return dwVersion;
}

