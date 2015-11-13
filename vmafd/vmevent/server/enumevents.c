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
 * Filename: enumpkgs.c
 *
 * Abstract:
 *
 * Enum packages
 *
 */

#include "includes.h"

DWORD
EventLogCloneEventContainerFromDbEventEntryArray(
    PEVENTLOG_DB_EVENT_ENTRY   pDbPackageEntryArray,
    DWORD                      dwDbPackageEntryNums,
    PEVENTLOG_CONTAINER        *ppPkgContainer
    );

DWORD
EventLogCloneEventEntryContentsFromDbEventEntry(
    PEVENTLOG_DB_EVENT_ENTRY    pDbPkgEntrySrc,
    PEVENTLOG_ENTRY              pPkgEntryDst
    );

DWORD
EventLogServerInitEnumEventsHandle(
    PDWORD pdwHandle
    )
{
    DWORD dwError = 0;

    *pdwHandle = EventLogGetApplicationVersion();

    return dwError;
}

DWORD
EventLogServerAddEvent(
   DWORD eventID,
   DWORD eventType,
   RP_PWSTR pszMessage
   )
{
   DWORD dwError = 0;

   EVENTLOG_DB_EVENT_ENTRY entry;
   HEVENTLOG_DB hEventLogDb = NULL;

   dwError = EventLogDbCreateContext(&hEventLogDb);
   BAIL_ON_VMEVENT_ERROR(dwError);

   entry.pwszEventMessage = pszMessage;
   entry.pwszEventDesc = NULL;
   entry.dwID = eventID;

   dwError = EventLogDbAddEvent(hEventLogDb, &entry);
   BAIL_ON_VMEVENT_ERROR(dwError);

cleanup:
    if (hEventLogDb)
    {
        EventLogDbReleaseContext(hEventLogDb);
    }
    return dwError;
error:
   goto cleanup;
}

DWORD
EventLogServerEnumEvents(
    DWORD    dwHandle,
    DWORD    dwStartIndex,
    DWORD    dwNumPackages,
    PEVENTLOG_CONTAINER * ppPkgContainer
    )
{
    DWORD dwError = 0;
    PEVENTLOG_CONTAINER pPkgContainer = NULL;
    PEVENTLOG_DB_EVENT_ENTRY pDbPackageEntryArray = NULL;
    DWORD dwDbPackageEntryNums = 0;
    HEVENTLOG_DB hEventLogDb = NULL;

    dwError = EventLogDbCreateContext(&hEventLogDb);
    BAIL_ON_VMEVENT_ERROR(dwError);

    // check the version
    if ((dwNumPackages > 0) &&
        (dwHandle != EventLogGetApplicationVersion()))
    {
        dwError = ERROR_INVALID_HANDLE;
        BAIL_ON_VMEVENT_ERROR(dwError);
    }

    dwError = EventLogDbEnumEvents(
                        hEventLogDb,
                        dwStartIndex,
                        dwNumPackages,
                        &pDbPackageEntryArray,
                        TRUE,
                        &dwDbPackageEntryNums);
    BAIL_ON_VMEVENT_ERROR(dwError);

    // convert db struct into rpc struct ...
    dwError = EventLogCloneEventContainerFromDbEventEntryArray(
                        pDbPackageEntryArray,
                        dwDbPackageEntryNums,
                        &pPkgContainer);
    BAIL_ON_VMEVENT_ERROR(dwError);

    *ppPkgContainer = pPkgContainer;
cleanup:

    if (hEventLogDb)
    {
        EventLogDbReleaseContext(hEventLogDb);
    }

    if (pDbPackageEntryArray)
    {
        EventLogDbFreeEventEntryArray(pDbPackageEntryArray, dwDbPackageEntryNums);
    }

    return dwError;

error:

    *ppPkgContainer = NULL;

    if (pPkgContainer)
    {
        EventLogFreeEventContainer(pPkgContainer);
    }

    goto cleanup;
}

DWORD
EventLogCloneEventContainerFromDbEventEntryArray(
    PEVENTLOG_DB_EVENT_ENTRY    pDbPackageEntryArray,
    DWORD                       dwDbPackageEntryNums,
    PEVENTLOG_CONTAINER         *ppPkgContainer
    )
{
    DWORD dwError = 0;
    PEVENTLOG_CONTAINER pContainer = NULL;

    dwError = EventLogAllocateMemory(sizeof(*pContainer), (PVOID*)&pContainer);
    BAIL_ON_VMEVENT_ERROR(dwError);

    pContainer->dwCount = dwDbPackageEntryNums;

    if (pContainer->dwCount > 0)
    {
        DWORD iEntry = 0;

        dwError = EventLogAllocateMemory(
                    pContainer->dwCount *
                        sizeof(pContainer->pPkgEntries[0]),
                    (PVOID*)&pContainer->pPkgEntries);
        BAIL_ON_VMEVENT_ERROR(dwError);

        for (; iEntry < pContainer->dwCount; iEntry++)
        {
            PEVENTLOG_DB_EVENT_ENTRY pDbPkgEntrySrc =
                     &pDbPackageEntryArray[iEntry];
            PEVENTLOG_ENTRY pPkgEntryDst =
                     &pContainer->pPkgEntries[iEntry];

            dwError = EventLogCloneEventEntryContentsFromDbEventEntry(
                      pDbPkgEntrySrc,
                      pPkgEntryDst
                      );
             BAIL_ON_VMEVENT_ERROR(dwError);
        }
    }

    *ppPkgContainer = pContainer;

cleanup:

    return dwError;

error:

    *ppPkgContainer = NULL;

    if (pContainer)
    {
        EventLogFreeEventContainer(pContainer);
    }

    goto cleanup;
}

DWORD
EventLogCloneEventEntryContentsFromDbEventEntry(
    PEVENTLOG_DB_EVENT_ENTRY    pDbPkgEntrySrc,
    PEVENTLOG_ENTRY             pPkgEntryDst
    )
{
    DWORD dwError =0;

    struct
    {
        PWSTR  pwszSrc;
        PWSTR* ppwszDst;
    }
    values[] =
    {
        {
            VMEVENT_SF_INIT(.pwszSrc, pDbPkgEntrySrc->pwszEventMessage),
            VMEVENT_SF_INIT(.ppwszDst, &pPkgEntryDst->pszMessage)
        }
    };
    DWORD iValue = 0;

    for (; iValue < sizeof(values)/sizeof(values[0]); iValue++)
    {
        if (values[iValue].pwszSrc)
        {
            dwError = EventLogAllocateStringW(
                            values[iValue].pwszSrc,
                            values[iValue].ppwszDst);
            BAIL_ON_VMEVENT_ERROR(dwError);
        }
    }

    pPkgEntryDst->dwEventId = pDbPkgEntrySrc->dwID;
    pPkgEntryDst->dwEventType = 0;

cleanup:

    return dwError;

error:

    goto cleanup;
}
