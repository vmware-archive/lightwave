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
 * Module Name: Directory main
 *
 * Filename: interface.h
 *
 * Abstract:
 *
 * Directory main module api
 *
 */

#ifndef __VMDIRMAIN_H__
#define __VMDIRMAIN_H__

#ifndef _WIN32
#include <lwrpcrt/lwrpcrt.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Plugin logic has four hook points per LDAP operation -
 *
 * 0. PreModApplyModifyOp - provide a hook point to manipulate PModification
 *    before applying it to entry.
 * per operation specific preparation (.e.g. normalize DN, schema check,..etc)
 * 1. PreOp  - before making backend interface call for this operation
 * be->ldap_op
 * 2. PostOp - after making backend interface call for this operation but before commit/abort
 * be->commit/abort
 * 3. PostCommitOp - after commit/abort backed changes for this operation
 */

// plugin function prototype
typedef DWORD (*VDIR_OP_PLUGIN_FUNCTION)(
                PVDIR_OPERATION     pOperation,      // current operation
                PVDIR_ENTRY         pEntry,          // entry been manipulated
                DWORD          dwResult         // latest operation return value
                );

typedef struct _VDIR_OP_PLUGIN_INFO *PVDIR_OP_PLUGIN_INFO;

typedef struct _VMDIR_OP_PLUGIN_GLOBALS
{
    // NOTE: order of fields MUST stay in sync with struct initializer...
    PVDIR_OP_PLUGIN_INFO     pPreAddPluginInfo;
    PVDIR_OP_PLUGIN_INFO     pPostAddCommitPluginInfo;
    PVDIR_OP_PLUGIN_INFO     pPreModApplyModifyPluginInfo;
    PVDIR_OP_PLUGIN_INFO     pPreModifyPluginInfo;
    PVDIR_OP_PLUGIN_INFO     pPostModifyCommitPluginInfo;
    PVDIR_OP_PLUGIN_INFO     pPreModApplyDeletePluginInfo;
    PVDIR_OP_PLUGIN_INFO     pPostDeleteCommitPluginInfo;

} VMDIR_OP_PLUGIN_GLOBALS, *PVMDIR_OP_PLUGIN_GLOBALS;

extern VMDIR_OP_PLUGIN_GLOBALS  gVmdirPluginGlobals;

typedef struct _VMDIR_SERVER_GLOBALS
{
    // NOTE: order of fields MUST stay in sync with struct initializer...

    VDIR_BERVALUE        invocationId;
    VDIR_BERVALUE        bvDefaultAdminDN;
    unsigned char        serverId;
    VDIR_BERVALUE        systemDomainDN;
    VDIR_BERVALUE        delObjsContainerDN;
    VDIR_BERVALUE        bvDCGroupDN;
    VDIR_BERVALUE        bvDCClientGroupDN;
    VDIR_BERVALUE        bvServicesRootDN;
    VDIR_BERVALUE        serverObjDN;
    VDIR_BERVALUE        dcAccountDN;   // Domain controller account DN
    VDIR_BERVALUE        dcAccountUPN;  // Domain controller account UPN
    int                  replInterval;
    int                  replPageSize;
    VDIR_BERVALUE        utdVector; // In string format, it is stored as: <serverId1>:<origUsn1>;<serverId2>:<origUsn2>;...
    PSTR                 pszSiteName;
    BOOLEAN              isIPV4AddressPresent;
    BOOLEAN              isIPV6AddressPresent;
} VMDIR_SERVER_GLOBALS, *PVMDIR_SERVER_GLOBALS;

extern VMDIR_SERVER_GLOBALS gVmdirServerGlobals;

typedef enum _VMDIR_RUNMODE
{
    VMDIR_RUNMODE_NORMAL,
    VMDIR_RUNMODE_STANDALONE,
    VMDIR_RUNMODE_RESTORE
} VMDIR_RUNMODE;

typedef struct _VMDIR_RUNMODE_GLOBALS
{
    PVMDIR_MUTEX  pMutex;
    VMDIR_RUNMODE mode;
} VMDIR_RUNMODE_GLOBALS;

extern VMDIR_RUNMODE_GLOBALS gVmdirRunmodeGlobals;

typedef struct _VMDIR_GLOBALS
{
    // NOTE: order of fields MUST stay in sync with struct initializer...

    // static fields initialized during server startup.
    // their values never change, so no access protection necessary.
    PSTR                            pszBootStrapSchemaFile;
    BOOLEAN                         bPatchSchema;
    PSTR                            pszBDBHome;

    int                             iSocketFd;

    BOOLEAN                         bAllowInsecureAuth;
    BOOLEAN                         bAllowAdminLockout;

    PDWORD                          pdwLdapListenPorts;
    DWORD                           dwLdapListenPorts;
    PDWORD                          pdwLdapsListenPorts;
    DWORD                           dwLdapsListenPorts;
    PDWORD                          pdwLdapConnectPorts;
    DWORD                           dwLdapConnectPorts;
    PDWORD                          pdwLdapsConnectPorts;
    DWORD                           dwLdapsConnectPorts;
    DWORD                           dwLdapRecvTimeoutSec;
    // following fields are protected by mutex
    PVMDIR_MUTEX                    mutex;
    VDIR_SERVER_STATE               vmdirdState;
    PVDIR_THREAD_INFO               pSrvThrInfo;
    BOOLEAN                         bReplNow;

#if !defined(_WIN32) || defined(HAVE_DCERPC_WIN32)
    dcethread*                      pRPCServerThread;
#endif

#ifdef _WIN32
    HANDLE                          hStopServiceEvent;
#endif

    BOOLEAN                         bRegisterTcpEndpoint;

    PSECURITY_DESCRIPTOR_ABSOLUTE   gpVmDirSrvSD;

    // To synchronize creation and use of replication agreements.
    PVMDIR_MUTEX                    replAgrsMutex;
    PVMDIR_COND                     replAgrsCondition;

    // To synchronize first replication cycle done state and vdcpromo exit criteria.
    PVMDIR_MUTEX                    replCycleDoneMutex;
    PVMDIR_COND                     replCycleDoneCondition;

    // Upper limit (local USNs only < this number) on updates that can be replicated out "safely".
    USN                             limitLocalUsnToBeReplicated;

    // Operation threads shutdown synchronize counter, synchronize value 0
    PVMDIR_SYNCHRONIZE_COUNTER      pOperationThrSyncCounter;
    // Waiting for all LDAP ports are ready for accepting services
    PVMDIR_SYNCHRONIZE_COUNTER      pPortListenSyncCounter;


    pthread_t                       pIPCServerThread;

    PVM_DIR_CONNECTION              pConnection;
    PVMDIR_MUTEX                    pMutexIPCConnection;

    PVMDIR_MUTEX                    pFlowCtrlMutex;
    DWORD                           dwMaxFlowCtrlThr;
} VMDIR_GLOBALS, *PVMDIR_GLOBALSS;

extern VMDIR_GLOBALS gVmdirGlobals;

typedef struct _VMDIR_KRB_GLOBALS
{
    PVMDIR_MUTEX        pmutex;
    PVMDIR_COND         pcond;
    PCSTR               pszRealm;
    PCSTR               pszDomainDN;
    VDIR_BERVALUE       bervMasterKey;
    BOOLEAN             bTryInit;
} VMDIR_KRB_GLOBALS, *PVMDIR_KRB_GLOBALS;

extern VMDIR_KRB_GLOBALS gVmdirKrbGlobals;

// accountmgmt.c
DWORD
VmDirCreateAccount(
    PCSTR   pszUPNName,
    PCSTR   pszUserName,
    PCSTR   pszPassword,            // optional
    PCSTR   pszEntryDN              // optional
    );

DWORD
VmDirUPNToAccountDN(
    PCSTR       pszUPNName,
    PCSTR       pszAccountRDNAttr,
    PCSTR       pszAccountRDNValue,
    PSTR*       ppszContainerDN
    );

// krb.c
DWORD
VmDirKrbRealmNameNormalize(
    PCSTR       pszName,
    PSTR*       ppszNormalizeName
    );

// utils.c
VDIR_SERVER_STATE
VmDirdState(
    VOID
    );

VOID
VmDirdStateSet(
    VDIR_SERVER_STATE   state
    );

BOOLEAN
VmDirdGetRestoreMode(
    VOID
    );

VMDIR_RUNMODE
VmDirdGetRunMode(
    VOID
    );

VOID
VmDirdSetRunMode(
    VMDIR_RUNMODE mode
    );

USN
VmDirdGetRestoredUSN(
    VOID
    );

VOID
VmDirdSetRestoredUSN(
    USN usn
    );

BOOLEAN
VmDirdGetAllowInsecureAuth(
    VOID
    );

VOID
VmDirGetLdapListenPorts(
    PDWORD* ppdwLdapListenPorts,
    PDWORD  pdwLdapListenPorts
    );

VOID
VmDirGetLdapsListenPorts(
    PDWORD* ppdwLdapsListenPorts,
    PDWORD  pdwLdapsListenPorts
    );

VOID
VmDirGetLdapConnectPorts(
    PDWORD* ppdwLdapConnectPorts,
    PDWORD  pdwLdapConnectPorts
    );

VOID
VmDirGetLdapsConnectPorts(
    PDWORD* ppdwLdapsConnectPorts,
    PDWORD  pdwLdapsConnectPorts
    );

DWORD
VmDirGetAllLdapPortsCount(
    VOID
);

VOID
VmDirdSetReplNow(
    BOOLEAN bReplNow
    );

BOOLEAN
VmDirdGetReplNow(
    VOID
    );

DWORD
VmDirServerStatusEntry(
    PVDIR_ENTRY*    ppEntry
    );

// srvthr.c
VOID
VmDirSrvThrAdd(
    PVDIR_THREAD_INFO   pThrInfo
    );

VOID
VmDirSrvThrInit(
    PVDIR_THREAD_INFO   pThrInfo,
    PVMDIR_MUTEX        pAltMutex,
    PVMDIR_COND         pAltCond,
    BOOLEAN             bJoinFlag
    );

VOID
VmDirSrvThrFree(
    PVDIR_THREAD_INFO   pThrInfo
    );

VOID
VmDirSrvThrShutdown(
    PVDIR_THREAD_INFO   pThrInfo
    );

VOID
VmDirSrvThrSignal(
    PVDIR_THREAD_INFO   pThrInfo
    );

int
VmDirSrvGetLdapListenPort(
    VOID
    );

void
VmDirSrvSetSocketAcceptFd(
    int fd
    );

int
VmDirSrvGetSocketAcceptFd(
    VOID
    );

// shutdown.c
VOID
VmDirShutdown(
    PBOOLEAN pbWaitTimeOut
    );

#ifdef __cplusplus
}
#endif

#endif /* __VMDIRMAIN_H__ */


