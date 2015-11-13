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
        VMDNS_SF_INIT(.bEnableDNSProtocol, FALSE)
    };
