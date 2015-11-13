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
 * Filename: interface.h
 *
 * Abstract:
 *
 * dns main module api
 *
 */

#ifndef __VMDNSMAIN_H__
#define __VMDNSMAIN_H__

#ifndef _WIN32
#include <lwrpcrt/lwrpcrt.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    VMDNSD_STARTUP = 0,
    VMDNSD_RUNNING,
    VMDNS_SHUTDOWN

} VMDNS_SERVER_STATE;


typedef struct _VMDNS_GLOBALS
{
    // NOTE: order of fields MUST stay in sync with struct initializer...
    // static fields initialized during server startup.
    // their values never change, so no access protection necessary.
    PSTR                            pszLogFile;
    DWORD                           dwMaximumOldFiles;
    DWORD                           dwMaxLogSizeBytes;
    int                             iListenPort;
    int                             iSocketFd;

    // following fields are protected by mutex
    PVMDNS_MUTEX                    pMutex;
    VMDNS_SERVER_STATE              vmdnsdState;
    dcethread*                      pRPCServerThread;
    BOOLEAN                         bRegisterTcpEndpoint;
    BOOLEAN                         bEnableDNSProtocol;

} VMDNS_GLOBALS, *PVMDNS_GLOBALS;

extern VMDNS_GLOBALS gVmdnsGlobals;

// utils.c
VMDNS_SERVER_STATE
VmDnsdState(
    VOID
    );

// shutdown.c
VOID
VmDnsShutdown(
    VOID
    );

#ifdef __cplusplus
}
#endif

#endif /* __VMDNSMAIN_H__ */
