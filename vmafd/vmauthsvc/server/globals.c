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
 * Module Name: Authsvc main
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
    CHAR gLogFileName[] = "C:\\tmp\\vmauthsvclog.txt";
#else
#define gLogFileName  		NULL
#endif

VMAUTHSVC_GLOBALS gVmauthsvcGlobals =
    {
        // NOTE: order of fields MUST stay in sync with struct definition...
        VMAUTHSVC_SF_INIT(.pszLogFile, gLogFileName),
        VMAUTHSVC_SF_INIT(.iListenPort, VMAUTHSVC_PORT),
        VMAUTHSVC_SF_INIT(.iSocketFd, -1),

        // following fields are protected by mutex
        VMAUTHSVC_SF_INIT(.mutex, PTHREAD_MUTEX_INITIALIZER),
        VMAUTHSVC_SF_INIT(.vmauthsvcdState, VMAUTHSVCD_STARTUP),
        VMAUTHSVC_SF_INIT(.pRPCServerThread, NULL),
        VMAUTHSVC_SF_INIT(.bRegisterTcpEndpoint, FALSE),
    };

int  vmauthsvc_syslog = 0;
int  vmauthsvc_debug = 0;
