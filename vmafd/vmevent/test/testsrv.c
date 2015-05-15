#include "includes.h"

void clientTest();
void dbLocalTest();
void localAdd();

DWORD
EventLogCloneEventContainerFromDbEventEntryArray(
    PEVENTLOG_DB_EVENT_ENTRY    pDbPackageEntryArray,
    DWORD                       dwDbPackageEntryNums,
    PEVENTLOG_CONTAINER         *ppPkgContainer
    );
DWORD
EventLogCloneEventEntryContentsFromDbEventEntry(
    PEVENTLOG_DB_EVENT_ENTRY    pDbPkgEntrySrc,
    PEVENTLOG_ENTRY             pPkgEntryDst
    );

int
main(void)
{
    printf("1. Enumerate\n2. Add Item\n3. Fill 5000 entries\n>");
    char ch = getchar();
    if(ch == '1')
        clientTest();
    else if(ch == '2')
        localAdd();
    else if(ch == '3')
        dbLocalTest();

    return 0;
}

void dbLocalTest()
{
    int i = 0, total = 0;
    RP_STR pszMessage[50];
    RP_STR pszDescription[50];
    DWORD dwError = 0, dwCount = 0;

    dwError = EventLogDbInitialize("/storage/db/vmware-vmafd/vmevent/vmevent.db");
    printf("EventLogDbInitialize = %d\n", dwError);

    HEVENTLOG_DB hDB = NULL;
    dwError = EventLogDbCreateContext(&hDB);
    printf("CreateContext = %d, hDB = %lu\n", dwError, (unsigned long)hDB);

    PEVENTLOG_DB_EVENT_ENTRY pEventArray = NULL;
    dwError = EventLogDbEnumEvents(hDB, 0, 2, &pEventArray, false, &dwCount);
    printf("EnumEvents = %d, pEvents = %lu, dwCount=%d\n",
            dwError, (unsigned long)pEventArray, dwCount);

    dwError = EventLogDbGetTotalEventCount(hDB, &dwCount);
    total = 5000 - (int)dwCount;

    for(i = 0; i < total; ++i)
    {
        RP_PWSTR pMsg, pDesc;
        sprintf(pszMessage, "Message - %d", i);
        sprintf(pszDescription, "Description - %d", i);

        dwError = EventLogAllocateStringWFromA(pszMessage, &pMsg);
        dwError = EventLogAllocateStringWFromA(pszDescription, &pDesc);

        EVENTLOG_DB_EVENT_ENTRY entry;
        entry.pwszEventMessage = pMsg;
        entry.pwszEventDesc = pDesc;
        entry.dwID = i;

        dwError = EventLogDbAddEvent(hDB, &entry);
        if(dwError != 0)
            break;
    }

    dwError = EventLogDbGetTotalEventCount(hDB, &dwCount);
    printf("GetTotalEventCount = %d, count = %d\n", dwError, dwCount);

    EventLogDbReleaseContext(hDB);

    EventLogDbShutdown();
    printf("EventLogDbShutdown = %d\n", dwError);
}

void localAdd()
{
    DWORD dwError = 0;
    char serverName[50];
    printf("Server:");
    scanf("%s", serverName);

    dwError = EventLogAdd(
                serverName,
                VMEVENT_RPC_TCP_END_POINT,
                1,
                1,
                "Message-Test");
    printf("Result = %d\n", dwError);
}

void clientTest()
{
    DWORD dwError = 0, dwHandle = 0;
    char serverName[50];
    printf("Server:");
    scanf("%s", serverName);

    dwError = EventLogInitEnumEventsHandle(serverName,
            VMEVENT_RPC_TCP_END_POINT, &dwHandle);
    printf("EventLogInitEnumEventsHandle = %d, Handle = %d\n", dwError, dwHandle);

    if(dwError == 0)
    {
        EVENTLOG_CONTAINER *pEventContainer = NULL;
        dwError = EventLogEnumEvents(serverName,
                VMEVENT_RPC_TCP_END_POINT,
                dwHandle, 0, 10, &pEventContainer);
        printf("EventLogEnumEvents = %d, pEventContainer = %ld\n",
                dwError, (long)pEventContainer);
        if(dwError == 0 && pEventContainer)
        {
            DWORD dwIndex = 0;
            for(; dwIndex < pEventContainer->dwCount; ++dwIndex)
            {
                char *pszMessage = NULL;
                EventLogAllocateStringAFromW(
                        pEventContainer->pPkgEntries[dwIndex].pszMessage,
                        &pszMessage);
                printf("EventID = %d, Message = %s\n",
                        (int)pEventContainer->pPkgEntries[dwIndex].dwEventId,
                        pszMessage);

                EventLogFreeStringA(pszMessage);
            }
        }
    }
}

