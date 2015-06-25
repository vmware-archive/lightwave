/*
 * Copyright (c) VMware Inc.  All rights Reserved.
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
