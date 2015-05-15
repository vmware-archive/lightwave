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
 * Filename: Event.c
 *
 * Abstract:
 *
 * Thinapp EventLogsitory Database
 *    BOOLEAN bHasIconBlob = FALSE;
 * Event persistence
 *
 */

#include "includes.h"

DWORD
EventLogDbQueryAllEvents(
    PEVENTLOG_DB_CONTEXT         pDbContext,
    PEVENTLOG_DB_EVENT_ENTRY*  ppEventEntryArray,
    BOOL                     bGetApps,
    PDWORD                   pdwCount
    );


DWORD
EventLogDbQueryEventsPaged(
    PEVENTLOG_DB_CONTEXT         pDbContext,
    DWORD                    dwStartIndex,
    DWORD                    dwNumEvents,
    PEVENTLOG_DB_EVENT_ENTRY*  ppEventEntryArray,
    BOOL                     bGetApps,
    PDWORD                   pdwCount
    );

DWORD
EventLogDbAddEvent(
    HEVENTLOG_DB                hDatabase,
    PEVENTLOG_DB_EVENT_ENTRY  pEventEntry
    )
{
    PEVENTLOG_DB_CONTEXT pDbContext = NULL;
    DWORD dwError = 0;
    BOOLEAN bInTx = FALSE;

    sqlite3_stmt* pDbQuery = NULL;
    int index = 1;

    pDbContext = (PEVENTLOG_DB_CONTEXT)hDatabase;
    assert(pDbContext);
    assert(pEventEntry);   

    if (!pDbContext || !pEventEntry)
        return ERROR_INVALID_PARAMETER;

    if (!pDbContext->pAddEventQuery)
    {
        CHAR szQuery[] =
                "INSERT INTO EventLogTable "
                "(Message, "
                " Description ) "
                "VALUES(?1, ?2 );";  

        dwError = sqlite3_prepare_v2(    
                        pDbContext->pDb,
                        szQuery,
                        -1,
                        &pDbContext->pAddEventQuery,
                        NULL);
        BAIL_ON_EVENTLOG_ERROR(dwError);
    }

    pDbQuery = pDbContext->pAddEventQuery;

    if (!pDbContext->bInTx)
    {
		dwError = EventLogDbBeginTransaction(pDbContext->pDb);
		BAIL_ON_EVENTLOG_ERROR(dwError);   

		bInTx = TRUE;
    }

    dwError = sqlite3_bind_text16(
                            pDbQuery,
                            index++,
                            pEventEntry->pwszEventMessage,
                            -1,
                            SQLITE_TRANSIENT);
    BAIL_ON_EVENTLOG_ERROR(dwError);

    dwError = sqlite3_bind_text16(
                            pDbQuery,
                            index++,
                            pEventEntry->pwszEventDesc,
                            -1,
                            SQLITE_TRANSIENT);
    BAIL_ON_EVENTLOG_ERROR(dwError);

    dwError = EventLogDbStepSql(pDbQuery);
    BAIL_ON_EVENTLOG_ERROR(dwError);

    if (!pDbContext->bInTx && bInTx) 
    {
		dwError = EventLogDbCommitTransaction(pDbContext->pDb);
		BAIL_ON_EVENTLOG_ERROR(dwError);
    }

cleanup:

    if (pDbQuery)
    {
        sqlite3_reset(pDbQuery);
    }

    return dwError;

error:

    if (!pDbContext->bInTx && bInTx)
    {
        EventLogDbRollbackTransaction(pDbContext->pDb);
    }
    goto cleanup;
}



DWORD
EventLogDbGetTotalEventCount(
    HEVENTLOG_DB     hDatabase,
    PDWORD           pdwNumEvents
    )
{
    PEVENTLOG_DB_CONTEXT pDbContext = NULL;
    DWORD dwError = 0;
    DWORD dwNumRows = 0;

    pDbContext = (PEVENTLOG_DB_CONTEXT)hDatabase;
    assert(pDbContext);

    if (!pDbContext->pTotalEventCountQuery)
    {
        CHAR szQuery[] = "SELECT COUNT(*) FROM EventLogTable;";

        dwError = sqlite3_prepare_v2(
                        pDbContext->pDb,
                        szQuery,
                        -1,
                        &pDbContext->pTotalEventCountQuery,
                        NULL);
        BAIL_ON_EVENTLOG_ERROR(dwError);
    }

    dwError = EventLogDbStepSql(pDbContext->pTotalEventCountQuery);
    if (dwError == SQLITE_ROW)
    {
        assert(sqlite3_column_count(pDbContext->pTotalEventCountQuery) == 1);

        dwNumRows = sqlite3_column_int(pDbContext->pTotalEventCountQuery, 0);

        dwError = ERROR_SUCCESS;
    }
    BAIL_ON_EVENTLOG_ERROR(dwError);

    *pdwNumEvents = dwNumRows;

cleanup:

    if (pDbContext->pTotalEventCountQuery)
    {
        sqlite3_reset(pDbContext->pTotalEventCountQuery);
    }

    return dwError;

error:

    *pdwNumEvents = 0;

    goto cleanup;
}    BOOLEAN bHasIconBlob = FALSE;

DWORD
EventLogDbEnumEvents(
    HEVENTLOG_DB             hDatabase,
    DWORD                    dwStartIndex,
    DWORD                    dwNumEvents,
    PEVENTLOG_DB_EVENT_ENTRY*  ppEventEntryArray,
    BOOL                     bGetApps,
    PDWORD                   pdwCount
    )
{
    PEVENTLOG_DB_CONTEXT pDbContext = NULL;
    DWORD dwError = 0;
    PEVENTLOG_DB_EVENT_ENTRY pEventEntryArray = NULL;
    DWORD dwCount = 0;

    pDbContext = (PEVENTLOG_DB_CONTEXT)hDatabase;
    assert(pDbContext);

    if (dwNumEvents > 0)
    {
        dwError = EventLogDbQueryEventsPaged(
                        pDbContext,
                        dwStartIndex,
                        dwNumEvents,
                        &pEventEntryArray,
			bGetApps,
                        &dwCount);
    }
    else
    {
        dwError = EventLogDbQueryAllEvents(
                        pDbContext,
                        &pEventEntryArray,
			bGetApps,
                        &dwCount);
    }
    BAIL_ON_EVENTLOG_ERROR(dwError);

    *ppEventEntryArray = pEventEntryArray;
    *pdwCount = dwCount;

cleanup:

    return dwError;

error:

    *ppEventEntryArray = NULL;
    *pdwCount = 0;

    if (pEventEntryArray)
    {
        EventLogDbFreeEventEntryArray(pEventEntryArray, dwCount);
    }

    goto cleanup;
}

DWORD
EventLogDbGetEvent(
    PEVENTLOG_DB_CONTEXT        pDbContext,
    PCWSTR                  pwszEventMessage,
    PEVENTLOG_DB_EVENT_ENTRY* ppEventEntry
    )
{
    DWORD dwError = 0;
    PEVENTLOG_DB_EVENT_ENTRY pEventEntry = NULL;

    EVENTLOG_DB_COLUMN_VALUE id =
    {
        VMEVENT_SF_INIT(.data.ppdwValue, NULL),
        VMEVENT_SF_INIT(.dataLength, 0)
    };
    EVENTLOG_DB_COLUMN_VALUE msg =
    {
        VMEVENT_SF_INIT(.data.ppwszValue, NULL),
        VMEVENT_SF_INIT(.dataLength, 0)
    };
    EVENTLOG_DB_COLUMN_VALUE description =
    {
        VMEVENT_SF_INIT(.data.ppwszValue, NULL),
        VMEVENT_SF_INIT(.dataLength, 0)
    };
    EVENTLOG_DB_COLUMN columns[] =
    {
        {
            VMEVENT_SF_INIT(.pszName, "ID"),
            VMEVENT_SF_INIT(.columnType, EVENTLOG_DB_COLUMN_TYPE_INT32),
            VMEVENT_SF_INIT(.pValue, &id)
        },
        {
            VMEVENT_SF_INIT(.pszName, "Message"),
            VMEVENT_SF_INIT(.columnType, EVENTLOG_DB_COLUMN_TYPE_STRING),
            VMEVENT_SF_INIT(.pValue, &msg)
        },
        {
            VMEVENT_SF_INIT(.pszName, "Description"),
            VMEVENT_SF_INIT(.columnType, EVENTLOG_DB_COLUMN_TYPE_STRING),
            VMEVENT_SF_INIT(.pValue, &description)
        }
    };

    assert(ppEventEntry);
    if (ppEventEntry == NULL)
        return ERROR_INVALID_PARAMETER;
    *ppEventEntry = NULL;

    if (!pDbContext->pQuerySingleEvent)
    {
        CHAR szQuery[] = "SELECT "
                                 "ID,"
                                 "Message,"
                                 "Description "
                         "FROM EventLogTable "
                         "WHERE Message = ?1;";

        dwError = sqlite3_prepare_v2(
                        pDbContext->pDb,
                        szQuery,
                        -1,
                        &pDbContext->pQuerySingleEvent,
                        NULL);
        BAIL_ON_EVENTLOG_ERROR(dwError);
    }

    dwError = sqlite3_bind_text16(
                    pDbContext->pQuerySingleEvent,
                    1,
                    pwszEventMessage,
                    -1,
                    SQLITE_TRANSIENT);
    BAIL_ON_EVENTLOG_ERROR(dwError);

    dwError = EventLogDbStepSql(pDbContext->pQuerySingleEvent);

    if (dwError == SQLITE_ROW)
    {
        DWORD dwID = 0;
        PDWORD pdwID = &dwID;


        dwError = EventLogAllocateMemory(sizeof(EVENTLOG_DB_EVENT_ENTRY),
                                     (PVOID*)&pEventEntry);
        BAIL_ON_EVENTLOG_ERROR(dwError);

        id.data.ppdwValue = &pdwID;
        id.dataLength     = sizeof(dwID);

        msg.data.ppwszValue             = &pEventEntry->pwszEventMessage;
        msg.dataLength              = 0;

        description.data.ppwszValue      = &pEventEntry->pwszEventDesc;
        description.dataLength       = 0;


        dwError = EventLogDbFillValues(
                                   pDbContext->pQuerySingleEvent,
                                   &columns[0],
                                   sizeof(columns)/sizeof(columns[0]));
        BAIL_ON_EVENTLOG_ERROR(dwError);


        *ppEventEntry = pEventEntry;
    }
    else if (dwError == SQLITE_DONE)
    {
        dwError = 0;
    }
    BAIL_ON_EVENTLOG_ERROR(dwError);

cleanup:

    if (pDbContext->pQuerySingleEvent)
    {
        sqlite3_reset(pDbContext->pQuerySingleEvent);
    }

    return dwError;

error:

    if (pEventEntry)
    {
        EventLogDbFreeEventEntry(pEventEntry);
    }

    goto cleanup;
}


DWORD
EventLogDbQueryAllEvents(
    PEVENTLOG_DB_CONTEXT         pDbContext,
    PEVENTLOG_DB_EVENT_ENTRY*  ppEventEntryArray,
    BOOL                     bGetApps,
    PDWORD                   pdwCount
    )
{
    DWORD dwError = 0;
    PEVENTLOG_DB_EVENT_ENTRY pEventEntryArray = NULL;
    DWORD iEntry = 0;
    DWORD dwTotalEntries = 0;
    DWORD dwEntriesAvailable = 0;

    EVENTLOG_DB_COLUMN_VALUE id =
    {
        VMEVENT_SF_INIT(.data.ppdwValue, NULL),
        VMEVENT_SF_INIT(.dataLength, 0)
    };
    EVENTLOG_DB_COLUMN_VALUE msg =
    {
        VMEVENT_SF_INIT(.data.ppwszValue, NULL),
        VMEVENT_SF_INIT(.dataLength, 0)
    };
    EVENTLOG_DB_COLUMN_VALUE description =
    {
        VMEVENT_SF_INIT(.data.ppwszValue, NULL),
        VMEVENT_SF_INIT(.dataLength, 0)
    };

    EVENTLOG_DB_COLUMN columns[] =
    {
        {
            VMEVENT_SF_INIT(.pszName, "ID"),
            VMEVENT_SF_INIT(.columnType, EVENTLOG_DB_COLUMN_TYPE_INT32),
            VMEVENT_SF_INIT(.pValue, &id)
        },
        {
            VMEVENT_SF_INIT(.pszName, "Message"),
            VMEVENT_SF_INIT(.columnType, EVENTLOG_DB_COLUMN_TYPE_STRING),
            VMEVENT_SF_INIT(.pValue, &msg)
        },
        {
            VMEVENT_SF_INIT(.pszName, "Description"),
            VMEVENT_SF_INIT(.columnType, EVENTLOG_DB_COLUMN_TYPE_STRING),
            VMEVENT_SF_INIT(.pValue, &description)
        }

    };

    if (!pDbContext->pQueryAllEvents)
    {
        CHAR szQuery[] = "SELECT "
                                 "ID,"
                                 "Message,"
                                 "Description "

                         "FROM   EventLogTable;";

        dwError = sqlite3_prepare_v2(
                        pDbContext->pDb,
                        szQuery,
                        -1,
                        &pDbContext->pQueryAllEvents,
                        NULL);
        BAIL_ON_EVENTLOG_ERROR(dwError);
    }

    do
    {
        dwError = EventLogDbStepSql(pDbContext->pQueryAllEvents);

        if (dwError == SQLITE_ROW)
        {
            PEVENTLOG_DB_EVENT_ENTRY pEventEntry = NULL;
            DWORD dwID = 0;
            PDWORD pdwID = &dwID;

            if (!dwEntriesAvailable)
            {
                PEVENTLOG_DB_EVENT_ENTRY pNewArray = NULL;
                DWORD dwNumIncr    = 10;
                DWORD dwNumEntries = dwTotalEntries + dwNumIncr;

                dwError = EventLogAllocateMemory(
                                sizeof(*pNewArray) * dwNumEntries,
                                (PVOID*)&pNewArray);
                BAIL_ON_EVENTLOG_ERROR(dwError);

                if (pEventEntryArray)
                {
                    memcpy( (PBYTE)pNewArray,
                            (PBYTE)pEventEntryArray,
                            sizeof(*pEventEntry) * dwTotalEntries);

                    EventLogFreeMemory(pEventEntryArray);
                }

                pEventEntryArray      = pNewArray;
                dwTotalEntries     += dwNumIncr;
                dwEntriesAvailable += dwNumIncr;
            }

            pEventEntry = &pEventEntryArray[iEntry++];
            dwEntriesAvailable--;

            id.data.ppdwValue = &pdwID;
            id.dataLength     = sizeof(dwID);

            msg.data.ppwszValue             = &pEventEntry->pwszEventMessage;
            msg.dataLength              = 0;

            description.data.ppwszValue      = &pEventEntry->pwszEventDesc;
            description.dataLength       = 0;

            dwError = EventLogDbFillValues(
                            pDbContext->pQueryAllEvents,
                            &columns[0],
                            sizeof(columns)/sizeof(columns[0]));
            BAIL_ON_EVENTLOG_ERROR(dwError);

        }
        else if (dwError == ERROR_SUCCESS)
        {
            break;
        }
        BAIL_ON_EVENTLOG_ERROR(dwError);

    } while (dwError != SQLITE_DONE);

    *ppEventEntryArray = pEventEntryArray;
    *pdwCount = iEntry;

cleanup:

    if (pDbContext->pQueryAllEvents)
    {
        sqlite3_reset(pDbContext->pQueryAllEvents);
    }

    return dwError;

error:

    *ppEventEntryArray = NULL;
    *pdwCount = 0;

    if (pEventEntryArray)
    {
        EventLogDbFreeEventEntryArray(pEventEntryArray, dwTotalEntries);
    }

    goto cleanup;
}

DWORD
EventLogDbQueryEventsPaged(
    PEVENTLOG_DB_CONTEXT         pDbContext,
    DWORD                    dwStartIndex,
    DWORD                    dwNumEvents,
    PEVENTLOG_DB_EVENT_ENTRY*  ppEventEntryArray,
    BOOL                     bGetApps,
    PDWORD                   pdwCount
    )
{
    DWORD dwError = 0;
    PEVENTLOG_DB_EVENT_ENTRY pEventEntryArray = NULL;
    DWORD iEntry = 0;
    DWORD dwTotalEntries = 0;
    DWORD dwEntriesAvailable = 0;
    EVENTLOG_DB_COLUMN_VALUE msg =
    {
        VMEVENT_SF_INIT(.data.ppwszValue, NULL),
        VMEVENT_SF_INIT(.dataLength, 0)
    };
    EVENTLOG_DB_COLUMN_VALUE description =
    {
        VMEVENT_SF_INIT(.data.ppwszValue, NULL),
        VMEVENT_SF_INIT(.dataLength, 0)
    };


    EVENTLOG_DB_COLUMN columns[] =
    {
        {
            VMEVENT_SF_INIT(.pszName, "Message"),
            VMEVENT_SF_INIT(.columnType, EVENTLOG_DB_COLUMN_TYPE_STRING),
            VMEVENT_SF_INIT(.pValue, &msg)
        },
        {
            VMEVENT_SF_INIT(.pszName, "Description"),
            VMEVENT_SF_INIT(.columnType, EVENTLOG_DB_COLUMN_TYPE_STRING),
            VMEVENT_SF_INIT(.pValue, &description)
        },

    };

    assert(dwNumEvents > 0);

    if (!pDbContext->pQueryEventsPaged)
    {
        CHAR szQuery[] = "SELECT "
                                 "Message,"
                                 "Description "

                         "FROM   EventLogTable "
                         "ORDER BY ID "
                         "LIMIT ?1 OFFSET ?2;";

        dwError = sqlite3_prepare_v2(
                        pDbContext->pDb,
                        szQuery,
                        -1,
                        &pDbContext->pQueryEventsPaged,
                        NULL);
        BAIL_ON_EVENTLOG_ERROR(dwError);
    }

    dwError = sqlite3_bind_int(
                    pDbContext->pQueryEventsPaged,
                    1,
                    dwNumEvents);
    BAIL_ON_EVENTLOG_ERROR(dwError);

    dwError = sqlite3_bind_int(
                    pDbContext->pQueryEventsPaged,
                    2,
                    dwStartIndex);
    BAIL_ON_EVENTLOG_ERROR(dwError);

    do
    {
        dwError = EventLogDbStepSql(pDbContext->pQueryEventsPaged);

        if (dwError == SQLITE_ROW)
        {
            PEVENTLOG_DB_EVENT_ENTRY pEventEntry = NULL;


            if (!dwEntriesAvailable)
            {
                PEVENTLOG_DB_EVENT_ENTRY pNewArray = NULL;
                DWORD dwNumIncr    = 10;
                DWORD dwNumEntries = dwTotalEntries + dwNumIncr;

                dwError = EventLogAllocateMemory(
                                sizeof(*pNewArray) * dwNumEntries,
                                (PVOID*)&pNewArray);
                BAIL_ON_EVENTLOG_ERROR(dwError);

                if (pEventEntryArray)
                {
                    memcpy( (PBYTE)pNewArray,
                            (PBYTE)pEventEntryArray,
                            sizeof(*pEventEntry) * dwTotalEntries);

                    EventLogFreeMemory(pEventEntryArray);
                }

                pEventEntryArray      = pNewArray;
                dwTotalEntries     += dwNumIncr;
                dwEntriesAvailable += dwNumIncr;
            }

            pEventEntry = &pEventEntryArray[iEntry++];
            dwEntriesAvailable--;

            msg.data.ppwszValue             = &pEventEntry->pwszEventMessage;
            msg.dataLength              = 0;

            description.data.ppwszValue      = &pEventEntry->pwszEventDesc;
            description.dataLength       = 0;


            dwError = EventLogDbFillValues(
                            pDbContext->pQueryEventsPaged,
                            &columns[0],
                            sizeof(columns)/sizeof(columns[0]));
            BAIL_ON_EVENTLOG_ERROR(dwError);

        }
        else if (dwError == ERROR_SUCCESS)
        {
            break;
        }
        BAIL_ON_EVENTLOG_ERROR(dwError);

    } while (dwError != SQLITE_DONE);

    *ppEventEntryArray = pEventEntryArray;
    *pdwCount = iEntry;

cleanup:

    if (pDbContext->pQueryEventsPaged)
    {
        sqlite3_reset(pDbContext->pQueryEventsPaged);
    }

    return dwError;

error:

    *ppEventEntryArray = NULL;
    *pdwCount = 0;

    if (pEventEntryArray)
    {
        EventLogDbFreeEventEntryArray(pEventEntryArray, dwTotalEntries);
    }

    goto cleanup;
}


DWORD
EventLogDbDeleteEvent(
    HEVENTLOG_DB      hDatabase,
    PCWSTR            pwszEventMessage
    )
{
    PEVENTLOG_DB_CONTEXT pDbContext = NULL;
    DWORD dwError = 0;
    BOOLEAN bInTx = FALSE;

    pDbContext = (PEVENTLOG_DB_CONTEXT)hDatabase;
    assert(pDbContext);

    if (!pDbContext->pDelEventQuery)
    {
        CHAR szQuery[] = "DELETE FROM EventLogTable WHERE Message = ?1;";

        dwError = sqlite3_prepare_v2(
                        pDbContext->pDb,
                        szQuery,
                        -1,
                        &pDbContext->pDelEventQuery,
                        NULL);
        BAIL_ON_EVENTLOG_ERROR(dwError);
    }

    if (!pDbContext->bInTx)
    {
		dwError = EventLogDbBeginTransaction(pDbContext->pDb);
		BAIL_ON_EVENTLOG_ERROR(dwError);

		bInTx = TRUE;
    }

    dwError = sqlite3_bind_text16(
                    pDbContext->pDelEventQuery,
                    1,
                    pwszEventMessage,
                    -1,
                    SQLITE_TRANSIENT);
    BAIL_ON_EVENTLOG_ERROR(dwError);

    dwError = EventLogDbStepSql(pDbContext->pDelEventAppsQuery);
    BAIL_ON_EVENTLOG_ERROR(dwError);

    dwError = EventLogDbStepSql(pDbContext->pDelEventQuery);
    BAIL_ON_EVENTLOG_ERROR(dwError);

    if (!pDbContext->bInTx && bInTx)
    {
	dwError = EventLogDbCommitTransaction(pDbContext->pDb);
	BAIL_ON_EVENTLOG_ERROR(dwError);
    }

cleanup:


    if (pDbContext->pDelEventQuery)
    {
        sqlite3_reset(pDbContext->pDelEventQuery);
    }

    return dwError;

error:

    if (!pDbContext->bInTx && bInTx)
    {
        EventLogDbRollbackTransaction(pDbContext->pDb);
    }

    goto cleanup;
}

