#include "includes.h"

static
DWORD
InitializeDatabase(
    VOID
    );

/*
static
DWORD
InitializeEventLog(
		VOID
		);
*/

DWORD
EventLogInitialize(
    )
{
    DWORD dwError = 0;

    dwError = InitializeDatabase();
    BAIL_ON_VMEVENT_ERROR(dwError);

#ifndef _WIN32
    // Construct SD for the EventLog-Service resources (to be access controlled)
    dwError = ConstructSDForEventLogServ(
			&gEventLogServerGlobals.gSecurityDescriptor
			);
    BAIL_ON_VMEVENT_ERROR(dwError);
#endif
//    dwError = EventLogRPCInit();
//    BAIL_ON_VMEVENT_ERROR(dwError);

error:

    return dwError;
}


static
DWORD
InitializeDatabase(
    VOID
    )
{
    DWORD dwError = 0;
#ifndef _WIN32
    PSTR pszDefaultEventLogDbPath = "/storage/db/vmware-vmafd/vmevent/vmevent.db";
#else
    PSTR pszDefaultEventLogDbPath = "C:\\ProgramData\\VMware\\cis\\data\\vmafdd\\vmevent\\vmevent.db";
#endif

    dwError = EventLogDbInitialize(pszDefaultEventLogDbPath);
    BAIL_ON_VMEVENT_ERROR(dwError);

error:

    return dwError;
}


VOID
EventLogShutdown(
    VOID
    )
{
    EventLogRPCShutdown();

    EventLogServiceShutdown();

    EventLogDbShutdown();

#ifndef _WIN32
    if (gEventLogServerGlobals.gSecurityDescriptor)
    {
        PSID pOwnerSID = NULL;
        BOOLEAN bDefaulted = FALSE;
        BOOLEAN bDaclPresent = FALSE;
        PACL pDacl = NULL;

        if (STATUS_SUCCESS == RtlGetOwnerSecurityDescriptor(
                                    gEventLogServerGlobals.gSecurityDescriptor,
                                    &pOwnerSID,
                                    &bDefaulted))
        {
            RTL_FREE(&pOwnerSID);
        }

        if (STATUS_SUCCESS == RtlGetDaclSecurityDescriptor(
                                    gEventLogServerGlobals.gSecurityDescriptor,
                                    &bDaclPresent,
                                    &pDacl,
                                    &bDefaulted) && bDaclPresent && pDacl)
        {
            EventLogFreeMemory(pDacl);
        }

        EventLogFreeMemory(gEventLogServerGlobals.gSecurityDescriptor);
    }
#endif
}

