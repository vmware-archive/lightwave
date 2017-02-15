/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : testsrv.c
 *
 * Abstract :
 *
 */
#include "includes.h"

int __forceCRTManifestCUR = 0;

DWORD clientTest();
DWORD dbLocalTest();
DWORD localAdd();

int
main(void)
{
    DWORD dwError = 0;
    char ch;

    printf("1. Enumerate\n2. Add Item\n3. Fill 5000 entries\n>");
    ch = getchar();
    if (ch == '1')
        dwError = clientTest();
    else if (ch == '2')
        dwError = localAdd();
    else if (ch == '3')
        dwError = dbLocalTest();

    return dwError;
}

DWORD
dbLocalTest()
{
    int i = 0, total = 0;
    RP_STR pszMessage[50];
    RP_STR pszDescription[50];
    RP_PWSTR pMsg = NULL, pDesc = NULL;
    HEVENTLOG_DB hDB = NULL;
    PEVENTLOG_DB_EVENT_ENTRY pEventArray = NULL;
    EVENTLOG_DB_EVENT_ENTRY entry;
    DWORD dwError = 0, dwCount = 0;

#ifndef _WIN32
    PSTR pszDefaultEventLogDbPath = "/storage/db/vmware-vmafd/vmevent/vmevent.db";
#else
    PSTR pszDefaultEventLogDbPath = "C:\\ProgramData\\VMware\\CIS\\data\\vmafdd\\vmevent.db";
#endif

    dwError = EventLogDbInitialize(pszDefaultEventLogDbPath);
    BAIL_ON_VMEVENT_ERROR(dwError);

    dwError = EventLogDbCreateContext(&hDB);
    BAIL_ON_VMEVENT_ERROR(dwError);

    dwError = EventLogDbEnumEvents(hDB, 0, 2, &pEventArray, FALSE, &dwCount);
    BAIL_ON_VMEVENT_ERROR(dwError);

    dwError = EventLogDbGetTotalEventCount(hDB, &dwCount);
    total = 5000 - (int)dwCount;

    for(i = 0; i < total; ++i)
    {
        sprintf(pszMessage, "Message - %d", i);
        sprintf(pszDescription, "Description - %d", i);

        dwError = EventLogAllocateStringWFromA(pszMessage, &pMsg);
        BAIL_ON_VMEVENT_ERROR(dwError);
        dwError = EventLogAllocateStringWFromA(pszDescription, &pDesc);
        BAIL_ON_VMEVENT_ERROR(dwError);

        entry.pwszEventMessage = pMsg;
        entry.pwszEventDesc = pDesc;
        entry.dwID = i;

        dwError = EventLogDbAddEvent(hDB, &entry);
        BAIL_ON_VMEVENT_ERROR(dwError);

        EventLogFreeStringW(pMsg);
        EventLogFreeStringW(pDesc);
    }

    dwError = EventLogDbGetTotalEventCount(hDB, &dwCount);
    BAIL_ON_VMEVENT_ERROR(dwError);
    printf("GetTotalEventCount = %d\n", dwCount);

    EventLogDbReleaseContext(hDB);

    EventLogDbShutdown();

cleanup:
    return dwError;

error:
    VMEVENT_SAFE_FREE_MEMORY(pEventArray);
    EventLogFreeStringW(pMsg);
    EventLogFreeStringW(pDesc);
    printf("Error (%d)\n", dwError);
    goto cleanup;
}

DWORD
localAdd()
{
    DWORD dwError = 0;
    char serverName[50];
    printf("Server:");
    scanf("%s", serverName);

    dwError = EventLogAdd(serverName, 1, 1, "Message-Test");
    BAIL_ON_VMEVENT_ERROR(dwError);

cleanup:
    return dwError;

error:
    printf("Error (%d)\n", dwError);
    goto cleanup;
}

DWORD
clientTest()
{
    DWORD dwError = 0, dwHandle = 0;
    char serverName[50];
    char msg[100];
    DWORD pageSize = 0;
    DWORD offset = 0;
    DWORD index = 0;
    BOOLEAN hasNext = TRUE;
    PSTR pszMessage = NULL;
    PEVENTLOG_CONTAINER pEventContainer = NULL;

    printf("Server:");
    scanf("%s", serverName);

    dwError = EventLogInitEnumEventsHandle(serverName, &dwHandle);
    BAIL_ON_VMEVENT_ERROR(dwError);

    printf("Page size:");
    scanf("%d", &pageSize);

    while (hasNext)
    {
        printf("[press return key]");
        fgets(msg, 100, stdin);

        dwError = EventLogEnumEvents(serverName, dwHandle, offset, pageSize, &pEventContainer);
        BAIL_ON_VMEVENT_ERROR(dwError);

        for (index = 0; index < pEventContainer->dwCount; index++)
        {
            dwError = EventLogAllocateStringAFromW(
                    pEventContainer->pPkgEntries[index].pszMessage,
                    &pszMessage);
            BAIL_ON_VMEVENT_ERROR(dwError);
            printf("EventID = %d, Message = %s\n",
                    (int)pEventContainer->pPkgEntries[index].dwEventId,
                    pszMessage);
            EventLogFreeStringA(pszMessage);
        }
        offset += pageSize;

        if (pageSize > pEventContainer->dwCount)
        {
            hasNext = FALSE;
        }
        VMEVENT_SAFE_FREE_MEMORY(pEventContainer);
    }

cleanup:
    return dwError;

error:
    VMEVENT_SAFE_FREE_MEMORY(pszMessage);
    VMEVENT_SAFE_FREE_MEMORY(pEventContainer);
    printf("Error (%d)\n", dwError);
    goto cleanup;
}

