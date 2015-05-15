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
 * Module Name: ThinAppRepoService
 *
 * Filename: repodb.h
 *
 * Abstract:
 *
 * Thinapp Repository Database
 *
 */

#ifndef __EVENTLOG_DB_H__
#define __EVENTLOG_DB_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef void *  HEVENTLOG_DB;

typedef HEVENTLOG_DB * PHEVENTLOG_DB;


typedef struct _EVENTLOG_DB_EVENT_ENTRY
{
    PWSTR pwszEventMessage;
    PWSTR pwszEventDesc;
    DWORD dwID;

} EVENTLOG_DB_EVENT_ENTRY, *PEVENTLOG_DB_EVENT_ENTRY;

typedef struct _EVENTLOG_DB_EVENT_CONTAINER
{
    DWORD dwCount;
    DWORD dwTotalCount;
    PEVENTLOG_DB_EVENT_ENTRY pEventEntries;
} EVENTLOG_DB_EVENT_CONTAINER, *PEVENTLOG_DB_EVENT_CONTAINER;


DWORD
EventLogDbInitialize(
    PCSTR pszDbPath
    );

VOID
EventLogDbShutdown(
    VOID
    );

DWORD
EventLogDbReset(
    VOID
    );

DWORD
EventLogDbCreateContext(
   HEVENTLOG_DB * phDatabase
    );

DWORD
EventLogDbCtxBeginTransaction(
    HEVENTLOG_DB hDatabase
    );

DWORD
EventLogDbCtxCommitTransaction(
    HEVENTLOG_DB hDatabase
    );

DWORD
EventLogDbCtxRollbackTransaction(
    HEVENTLOG_DB hDatabase
    );

/*
** Note: The caller is responsible for the contents of the
** EVENTLOG_DB_EVENT_ENTRY struct.  Its contents will not be modified or
** freed inside this function call.
*/
DWORD
EventLogDbAddEvent(
    HEVENTLOG_DB hDatabase,
    PEVENTLOG_DB_EVENT_ENTRY pEventEntry
    );

DWORD
EventLogDbGetTotalEventCount(
    HEVENTLOG_DB hDatabase,
    PDWORD       pdwNumEvents
    );

DWORD
EventLogDbEnumEvents(
    HEVENTLOG_DB hDatabase,
    DWORD                       dwStartIndex,
    DWORD                       dwNumEvents,
    PEVENTLOG_DB_EVENT_ENTRY*   ppEventEntryArray,
    BOOL                        bGetApps,
    PDWORD                      pdwCount
    );

DWORD
EventLogDbDeleteEvent(
    HEVENTLOG_DB hDatabase,
    PCWSTR           pwszEventGUID
    );


VOID
EventLogDbReleaseContext(
    HEVENTLOG_DB hDatabase
    );

VOID
EventLogDbFreeContext(
    HEVENTLOG_DB hDatabase
    );

const char*
EventLogDbErrorCodeToName(
    int code
    );

VOID
EventLogDbFreeEventEntryArray(
    PEVENTLOG_DB_EVENT_ENTRY pEventEntryArray,
    DWORD                   dwCount
    );

VOID
EventLogDbFreeEventEntry(
    PEVENTLOG_DB_EVENT_ENTRY pEventEntry
    );

VOID
EventLogDbFreeEventEntryArray(
    PEVENTLOG_DB_EVENT_ENTRY pEventEntryArray,
    DWORD                   dwCount
    );

VOID
EventLogDbFreeEventEntryContents(
    PEVENTLOG_DB_EVENT_ENTRY pEventEntry
    );


#ifdef __cplusplus
}
#endif

#endif /* __EVENTLOG_DB_H__ */


