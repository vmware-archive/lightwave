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
 * Filename: interface.h
 *
 * Abstract:
 *
 * Authsvc main module api
 *
 */

#ifndef __VMAUTHSVCMAIN_H__
#define __VMAUTHSVCMAIN_H__

#ifndef _WIN32
#include <lwrpcrt/lwrpcrt.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    VMAUTHSVCD_STARTUP = 0,
    VMAUTHSVCD_RUNNING,
    VMAUTHSVC_SHUTDOWN

} VMAUTHSVC_SERVER_STATE;


typedef struct _VMAUTHSVC_GLOBALS
{
    // NOTE: order of fields MUST stay in sync with struct initializer...

    // static fields initialized during server startup.
    // their values never change, so no access protection necessary.
    PSTR                            pszLogFile;
    int                             iListenPort;
    int                             iSocketFd;

    // following fields are protected by mutex
    pthread_mutex_t                 mutex;
    VMAUTHSVC_SERVER_STATE               vmauthsvcdState;
    dcethread*                      pRPCServerThread;
    BOOLEAN                         bRegisterTcpEndpoint;
} VMAUTHSVC_GLOBALS, *PVMAUTHSVC_GLOBALS;

extern VMAUTHSVC_GLOBALS gVmauthsvcGlobals;

// utils.c
VMAUTHSVC_SERVER_STATE
VmAuthsvcdState(
    VOID
    );

// shutdown.c
VOID
VmAuthsvcShutdown(
    VOID
    );

#ifdef __cplusplus
}
#endif

#endif /* __VMAUTHSVCMAIN_H__ */
