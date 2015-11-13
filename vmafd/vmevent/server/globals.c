/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : globals.c
 *
 * Abstract :
 *
 */
#include "includes.h"

EVENTLOG_SERVER_GLOBALS gEventLogServerGlobals =
{
    VMEVENT_SF_INIT(.gSecurityDescriptor, NULL),
    VMEVENT_SF_INIT(.pRPCServerThread, NULL)
};

HANDLE           gpEventLog;

