/*
 * Copyright 2012-2016 VMware, Inc.  All Rights Reserved.
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

#include <lw/types.h>
#include <lw/hash.h>
#include <lw/security-api.h>

#include <vmsuperlogging.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_THREAD_PRIORITY_DELTA 10

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
    int                  serverId;
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
    USN                  initialNextUSN; // used for server restore only
    USN                  maxOriginatingUSN;  // Cache value to prevent
                                             // excessive searching
    VDIR_BERVALUE        bvServerObjName;
    DWORD                dwDomainFunctionalLevel;
    // Data that controls the thread that cleans up deleted entries.
    DWORD                dwTombstoneExpirationPeriod;
    DWORD                dwTombstoneThreadFrequency;
} VMDIR_SERVER_GLOBALS, *PVMDIR_SERVER_GLOBALS;

extern VMDIR_SERVER_GLOBALS gVmdirServerGlobals;

typedef struct _VMDIRD_STATE_GLOBALS
{
    PVMDIR_MUTEX  pMutex;
    VDIR_SERVER_STATE vmdirdState;
    VDIR_SERVER_STATE targetState;
} VMDIRD_STATE_GLOBALS;

extern VMDIRD_STATE_GLOBALS gVmdirdStateGlobals;

typedef struct _VMDIR_GLOBALS
{
    // NOTE: order of fields MUST stay in sync with struct initializer...

    // static fields initialized during server startup.
    // their values never change, so no access protection necessary.
    PSTR                            pszBootStrapSchemaFile;
    BOOLEAN                         bPatchSchema;
    PSTR                            pszBDBHome;

    BOOLEAN                         bAllowInsecureAuth;
    BOOLEAN                         bAllowAdminLockout;
    BOOLEAN                         bDisableVECSIntegration;

    PDWORD                          pdwLdapListenPorts;
    DWORD                           dwLdapListenPorts;
    PDWORD                          pdwLdapsListenPorts;
    DWORD                           dwLdapsListenPorts;
    PDWORD                          pdwLdapConnectPorts;
    DWORD                           dwLdapConnectPorts;
    PDWORD                          pdwLdapsConnectPorts;
    DWORD                           dwLdapsConnectPorts;
    PSTR                            pszRestListenPort;

    DWORD                           dwLdapRecvTimeoutSec;
    // following fields are protected by mutex
    PVMDIR_MUTEX                    mutex;
    PVDIR_THREAD_INFO               pSrvThrInfo;
    BOOLEAN                         bReplNow;

#if !defined(_WIN32) || defined(HAVE_DCERPC_WIN32)
    dcethread*                      pRPCServerThread;
#endif

#ifdef _WIN32
    HANDLE                          hStopServiceEvent;
#endif

    BOOLEAN                         bRegisterTcpEndpoint;

    // To synchronize creation and use of replication agreements.
    PVMDIR_MUTEX                    replAgrsMutex;
    PVMDIR_COND                     replAgrsCondition;

    // To synchronize first replication cycle done state and vdcpromo exit criteria.
    PVMDIR_MUTEX                    replCycleDoneMutex;
    PVMDIR_COND                     replCycleDoneCondition;
    DWORD                           dwReplCycleCounter;

    // To synchronize replication reads and writes.
    PVMDIR_RWLOCK                   replRWLock;

    // Upper limit (local USNs only < this number) on updates that can be replicated out "safely".
    USN                             limitLocalUsnToBeSupplied;

    // Operation threads shutdown synchronize counter, synchronize value 0
    PVMDIR_SYNCHRONIZE_COUNTER      pOperationThrSyncCounter;
    // Waiting for all LDAP ports are ready for accepting services
    PVMDIR_SYNCHRONIZE_COUNTER      pPortListenSyncCounter;


    pthread_t                       pIPCServerThread;

    PVM_DIR_CONNECTION              pConnection;
    PVMDIR_MUTEX                    pMutexIPCConnection;

    PVMDIR_MUTEX                    pFlowCtrlMutex;
    DWORD                           dwMaxFlowCtrlThr;
    PVMSUPERLOGGING                 pLogger;
    UINT64                          iServerStartupTime;
    // Limit the index scan to hunt for good filter
    DWORD                           dwMaxIndexScan;
    // # of candidate in search filter - used for search optimization
    DWORD                           dwSmallCandidateSet;
    // Limit index scan for the best effort sizelimit search
    DWORD                           dwMaxSizelimitScan;

    DWORD                           dwLdapSearchTimeoutSec;
    BOOLEAN                         bAllowImportOpAttrs;
    BOOLEAN                         bTrackLastLoginTime;
    BOOLEAN                         bPagedSearchReadAhead;

    // The following three counter is for database copy feature
    // registra key configuration.
    DWORD                           dwCopyDbWritesMin;
    DWORD                           dwCopyDbIntervalInSec;
    DWORD                           dwCopyDbBlockWriteInSec;
    // Collect stats for estimate elapsed time with database copy
    DWORD                           dwLdapWrites;
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

typedef struct _VMDIR_TRACK_LAST_LOGIN_TIME
{
    PVMDIR_MUTEX    pMutex;
    PVMDIR_COND     pCond;
    PVMDIR_TSSTACK  pTSStack;
} VMDIR_TRACK_LAST_LOGIN_TIME, *PVMDIR_TRACK_LAST_LOGIN_TIME;

extern VMDIR_TRACK_LAST_LOGIN_TIME gVmdirTrackLastLoginTime;

typedef struct _VMDIR_INTEGRITY_JOB *PVMDIR_INTEGRITY_JOB;

typedef struct _VMDIR_INTEGRITY_CHECK_GLOBALS
{
    PVMDIR_MUTEX            pMutex;
    PVMDIR_INTEGRITY_JOB    pJob;

} VMDIR_INTEGRITY_CHECK_GLOBALS, *PVMDIR_INTEGRITY_CHECK_GLOBALS;

extern VMDIR_INTEGRITY_CHECK_GLOBALS gVmdirIntegrityCheck;

typedef enum
{
    INTEGRITY_CHECK_JOB_NONE = 0,
    INTEGRITY_CHECK_JOB_START,
    INTEGRITY_CHECK_JOB_STOP,
    INTEGRITY_CHECK_JOB_FINISH,
    INTEGRITY_CHECK_JOB_RECHECK,
    INTEGRITY_CHECK_JOB_INVALID,
    INTEGRITY_CHECK_JOB_SHOW_SUMMARY
} VMDIR_INTEGRITY_CHECK_JOB_STATE, *PVMDIR_INTEGRITY_CHECK_JOB_STATE;

typedef enum
{
    INTEGRITY_CHECK_JOBCXT_NONE = 0,
    INTEGRITY_CHECK_JOBCTX_VALID,
    INTEGRITY_CHECK_JOBCTX_INVALID,
    INTEGRITY_CHECK_JOBCTX_SKIP,
    INTEGRITY_CHECK_JOBCTX_ABORT
} VMDIR_INTEGRITY_CHECK_JOBCTX_STATE, *PVMDIR_INTEGRITY_CHECK_JOBCTX_STATE;

// krb.c
DWORD
VmDirKrbRealmNameNormalize(
    PCSTR       pszName,
    PSTR*       ppszNormalizeName
    );

#ifdef VMDIR_ENABLE_PAC
DWORD
VmDirKrbGetAuthzInfo(
    PCSTR pszUpnName,
    VMDIR_AUTHZ_INFO** ppInfo
    );

VOID
VmDirKrbFreeAuthzInfo(
    VMDIR_AUTHZ_INFO* pInfo
    );
#endif

// utils.c
VDIR_SERVER_STATE
VmDirdState(
    VOID
    );

VOID
VmDirdStateSet(
    VDIR_SERVER_STATE   state
    );

VDIR_SERVER_STATE
VmDirdGetTargetState(
    VOID
    );

VOID
VmDirdSetTargetState(
    VDIR_SERVER_STATE   state
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

DWORD
VmDirReplicationStatusEntry(
    PVDIR_ENTRY*    ppEntry
    );

// srvthr.c
VOID
VmDirSrvThrAdd(
    PVDIR_THREAD_INFO   pThrInfo
    );

DWORD
VmDirSrvThrInit(
    PVDIR_THREAD_INFO   *ppThrInfo,
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

// tracklastlogin.c
VOID
VmDirAddTrackLastLoginItem(
    PCSTR   pszDN
    );

// integritycheck.c
DWORD
VmDirEntrySHA1Digest(
    PVDIR_ENTRY pEntry,
    PSTR        pOutSH1DigestBuf
    );

DWORD
VmDirIntegrityCheckStart(
    VMDIR_INTEGRITY_CHECK_JOB_STATE jobState
    );

VOID
VmDirIntegrityCheckStop(
    VOID
    );

DWORD
VmDirIntegrityCheckShowStatus(
    PVDIR_ENTRY*    ppEntry
    );

#ifdef __cplusplus
}
#endif

#endif /* __VMDIRMAIN_H__ */


