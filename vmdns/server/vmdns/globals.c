/*
 * Copyright (c) VMware Inc.  All rights Reserved.
 */

/*
 * Module Name: dns main
 *
 * Filename: globals.c
 *
 * Abstract:
 *
 * Globals
 *
 */

#include "includes.h"

// TODO: pgu 
// 1. During installation, write these values to registry
// 2. During service start, read these values from registry
#ifdef _WIN32
    CHAR gLogFileName[] = "C:\\tmp\\vmdnslog.txt";
#else
#define gLogFileName  		NULL
#endif


VMDNS_GLOBALS gVmdnsGlobals =
    {
        // NOTE: order of fields MUST stay in sync with struct definition...
        VMDNS_SF_INIT(.pszLogFile, gLogFileName),
        VMDNS_SF_INIT(.iListenPort, VMDNS_PORT),
        VMDNS_SF_INIT(.iSocketFd, -1),

        // following fields are protected by mutex
        VMDNS_SF_INIT(.pMutex, NULL),
        VMDNS_SF_INIT(.vmdnsdState, VMDNSD_STARTUP),
        VMDNS_SF_INIT(.pRPCServerThread, NULL),
        VMDNS_SF_INIT(.bRegisterTcpEndpoint, FALSE),
    };
