#include "includes.h"

EVENTLOG_SERVER_GLOBALS gEventLogServerGlobals =
{
    VMEVENT_SF_INIT(.gSecurityDescriptor, NULL),
    VMEVENT_SF_INIT(.pRPCServerThread, NULL)
};

HANDLE           gpEventLog;

