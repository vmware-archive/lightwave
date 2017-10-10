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
 * Module Name: Kdc main
 *
 * Filename: interface.h
 *
 * Abstract:
 *
 * Kdc main module api
 *
 */

#ifndef __VMKDCMAIN_H__
#define __VMKDCMAIN_H__

#ifndef _WIN32
#include <lwrpcrt/lwrpcrt.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    VMKDCD_STARTUP = 0,
    VMKDCD_RUNNING,
    VMKDC_STOPPING,
    VMKDC_SHUTDOWN,
} VMKDC_SERVER_STATE;

#if 1
// Incomplete type to compile at this level
struct _VMKDC_KRB5_CONTEXT;
struct _VMKDC_MIT_KEYTAB_FILE;
struct _VMKDC_DIRECTORY;
struct _VMKDC_CRYPTO;
struct _VMKDC_KRB5_CONTEXT;
struct _VMKDC_KEY;
#endif

typedef struct _VMKDC_GLOBALS
{
    // NOTE: order of fields MUST stay in sync with struct initializer...

    // static fields initialized during server startup.
    // their values never change, so no access protection necessary.
    PSTR                            pszDefaultRealm;
    int                             iListenPort;
    int                             iClockSkew;
    int                             iMaxLife;
    int                             iMaxRenewableLife;
    INT64                           iAcceptSock;
    INT64                           iAcceptSockUdp;
    INT64                           iAcceptSock6;
    INT64                           iAcceptSock6Udp;
    int                             addrLen;
    int                             addrLen6;

    // following fields are protected by mutex
    pthread_t                       thread;
    pthread_mutex_t                 mutex;
    pthread_cond_t                  cond;
    pthread_attr_t                  attrDetach;
    
    pthread_cond_t                  stateCond;
    VMKDC_SERVER_STATE              vmkdcdState;

    DWORD                           workerThreadMax;
    DWORD                           workerThreadCount;

    dcethread*                      pRPCServerThread;

    // Enable DCE/RPC TCP endpoint when true
    BOOLEAN                         bRegisterTcpEndpoint;

    struct _VMKDC_KRB5_CONTEXT      *pKrb5Ctx;
    struct _VMKDC_KEY               *masterKey;
    struct _VMKDC_DIRECTORY         *pDirectory;
    struct _VMKDC_KRB5_CONTEXT      *pKrb5MasterKey;
} VMKDC_GLOBALS, *PVMKDC_GLOBALS;

extern VMKDC_GLOBALS gVmkdcGlobals;

typedef struct _VMKDC_REQUEST
{
    int requestSocket;
    BOOLEAN bRequestIsUdp;
    UCHAR *requestBuf;
    DWORD requestAllocLen;
    DWORD requestBufLen;
    PVOID pvClientAddr;
    DWORD dwClientAddrLen;
    struct _VMKDC_KEY *masterKey;
} VMKDC_REQUEST, *PVMKDC_REQUEST;

typedef struct _VMKDC_CONTEXT
{
    int tag; // Maybe needed
    PVMKDC_GLOBALS pGlobals;
    PVMKDC_REQUEST pRequest;
} VMKDC_CONTEXT, *PVMKDC_CONTEXT;

//vmkdc/main.c
DWORD
VmKdcServiceStartup(
    VOID
    );

VOID
VmKdcServiceShutdown(
    VOID
    );

// utils.c
VMKDC_SERVER_STATE
VmKdcdState(
    VOID
    );

#if 0
// srvthr.c
VOID
VmKdcSrvThrAdd(
    PVMKDC_THREAD_INFO   pThrInfo
    );

VOID
VmKdcSrvThrInit(
    PVMKDC_THREAD_INFO   pThrInfo,
    pthread_mutex_t      pAltMutex,
    pthread_cond_t       pAltCond,
    BOOLEAN             bJoinFlag
    );

VOID
VmKdcSrvThrFree(
    PVMKDC_THREAD_INFO   pThrInfo
    );

VOID
VmKdcSrvThrShutdown(
    PVMKDC_THREAD_INFO   pThrInfo
    );

VOID
VmKdcSrvThrSignal(
    PVMKDC_THREAD_INFO   pThrInfo
    );
#endif

void
VmKdcSrvSetSocketAcceptFd(
    int fd
    );

int
VmKdcSrvGetSocketAcceptFd(
    VOID
    );

// shutdown.c
VOID
VmKdcShutdown(
    VOID
    );

#ifdef __cplusplus
}
#endif

#endif /* __VMKDCMAIN_H__ */
