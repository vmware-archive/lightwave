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

typedef struct _VMDIR_UTDVECTOR_CACHE
{
    PSTR            pszUtdVector;   // In string
    PLW_HASHMAP     pUtdVectorMap;  // In hash map
    PVMDIR_RWLOCK   pUtdVectorLock;

} VMDIR_UTDVECTOR_CACHE, *PVMDIR_UTDVECTOR_CACHE;

//TODO_REMOVE_REPLV2
typedef struct _VMDIR_REPL_DEADLOCKDETECTION_VECTOR
{
    PSTR          pszInvocationId;
    PLW_HASHMAP   pEmptyPageSentMap;
    PVMDIR_MUTEX  pMutex;
} VMDIR_REPL_DEADLOCKDETECTION_VECTOR, *PVMDIR_REPL_DEADLOCKDETECTION_VECTOR;

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
    VDIR_BERVALUE        bvSchemaManagersGroupDN;
    VDIR_BERVALUE        bvServicesRootDN;
    VDIR_BERVALUE        serverObjDN;
    VDIR_BERVALUE        dcAccountDN;   // Domain controller account DN
    VDIR_BERVALUE        dcAccountUPN;  // Domain controller account UPN
    int                  replInterval;
    int                  replPageSize;

    PVMDIR_UTDVECTOR_CACHE  pUtdVectorCache;

    PSTR                 pszSiteName;
    BOOLEAN              isIPV4AddressPresent;
    BOOLEAN              isIPV6AddressPresent;
    USN                  initialNextUSN; // used for server restore only
    VDIR_BERVALUE        bvServerObjName;
    DWORD                dwDomainFunctionalLevel;
    // Data that controls the thread that cleans up deleted entries.
    DWORD                dwTombstoneExpirationPeriod;
    DWORD                dwTombstoneThreadFrequency;
    DWORD                dwMaxInternalSearchLimit;
    DWORD                dwEfficientReadOpTimeMS;

    // Flag that indicates whether this instance is promoted
    // It is set at two places:
    // 1) At the end of VmDirSrvSetupHostInstance for the 1st node
    // 2) At the end of LoadServerGlobals for other nodes
    BOOLEAN              bPromoted;

    //TODO_REMOVE_REPLV2
    PVMDIR_REPL_DEADLOCKDETECTION_VECTOR  pReplDeadlockDetectionVector;

} VMDIR_SERVER_GLOBALS, *PVMDIR_SERVER_GLOBALS;

extern VMDIR_SERVER_GLOBALS gVmdirServerGlobals;

typedef struct _VMDIRD_SD_GLOBALS
{
    PSECURITY_DESCRIPTOR_RELATIVE pSDdcAdminGX;
} VMDIRD_SD_GLOBALS;

extern VMDIRD_SD_GLOBALS gVmdirdSDGlobals;

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
    BOOLEAN                         bEnableRegionalMaster;

    PDWORD                          pdwLdapListenPorts;
    DWORD                           dwLdapListenPorts;
    PDWORD                          pdwLdapsListenPorts;
    DWORD                           dwLdapsListenPorts;
    PDWORD                          pdwLdapConnectPorts;
    DWORD                           dwLdapConnectPorts;
    PDWORD                          pdwLdapsConnectPorts;
    DWORD                           dwLdapsConnectPorts;
    DWORD                           dwHTTPListenPort;
    DWORD                           dwHTTPSListenPort;
    DWORD                           dwHTTPSApiListenPort;

    DWORD                           dwLdapRecvTimeoutSec;
    DWORD                           dwLdapConnectTimeoutSec;
    DWORD                           dwOperationsThreadTimeoutInMilliSec;
    DWORD                           dwReplConsumerThreadTimeoutInMilliSec;
    DWORD                           dwEmptyPageCnt; //TODO_REMOVE_REPLV2
    DWORD                           dwSupplierThrTimeoutInMilliSec;
    DWORD                           dwWriteTimeoutInMilliSec;

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

    // Upper limit (local USNs only < this number) on updates that can be replicated out "safely".
    USN                             limitLocalUsnToBeSupplied;

    // Operation threads shutdown synchronize counter, synchronize value 0
    PVMDIR_SYNCHRONIZE_COUNTER      pOperationThrSyncCounter;
    // Waiting for all LDAP ports are ready for accepting services
    PVMDIR_SYNCHRONIZE_COUNTER      pPortListenSyncCounter;

    // IPC server variables
    PVDIR_THREAD_INFO               pIPCSrvThrInfo;
    PVM_DIR_CONNECTION              pIPCConn;
    // IPC needs to keep running in special cases, it needs its own shutdown flag
    BOOLEAN                         bIPCShutdown;

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

    // Collect stats for estimate elapsed time with database copy
    DWORD                           dwLdapWrites;

    SSL_CTX*                        gpVdirSslCtx;
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

typedef struct _VMDIR_WRITE_QUEUE
{
    PVDIR_LINKED_LIST  pList;
} VMDIR_WRITE_QUEUE, *PVMDIR_WRITE_QUEUE;

typedef struct _VMDIR_SERVER_OPERATIONS_GLOBALS
{
    PVMDIR_MUTEX         pMutex;
    USN                  maxCommittedUSN;
    PVMDIR_WRITE_QUEUE   pWriteQueue;
} VMDIR_SERVER_OPERATIONS_GLOBALS, *PVMDIR_SERVER_OPERATIONS_GLOBALS;

extern VMDIR_SERVER_OPERATIONS_GLOBALS gVmDirServerOpsGlobals;

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

typedef struct _VMDIR_INTEGRITY_REPORT
{
    PSTR        pszPartner;
    PLW_HASHMAP pMismatchMap;
    PLW_HASHMAP pMissingMap;
    DWORD       dwMismatchCnt;
    DWORD       dwMissingCnt;

} VMDIR_INTEGRITY_REPORT, *PVMDIR_INTEGRITY_REPORT;

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

DWORD
VmDirCheckPortAvailability(
    DWORD   dwPort
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

VOID
VmDirWaitForLDAPOpThr(
    PBOOLEAN pbStopped
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
    VMDIR_INTEGRITY_CHECK_JOB_STATE jobState,
    PVMDIR_BKGD_TASK_CTX            pBkgdTaskCtx    // if triggered by background thread
    );

VOID
VmDirIntegrityCheckStop(
    VOID
    );

DWORD
VmDirIntegrityCheckShowStatus(
    PVDIR_ENTRY*    ppEntry
    );

// integrityrpt.c
DWORD
VmDirIntegrityReportCreate(
    PVMDIR_INTEGRITY_REPORT*    ppReport
    );

DWORD
VmDirIntegrityReportLoadFile(
    PVMDIR_INTEGRITY_REPORT pReport,
    PCSTR                   pszFilePath
    );

DWORD
VmDirIntegrityReportWriteToFile(
    PVMDIR_INTEGRITY_REPORT pReport,
    PCSTR                   pszFilePath
    );

DWORD
VmDirIntegrityReportRemoveNonOverlaps(
    PVMDIR_INTEGRITY_REPORT pReport,    // this gets modified
    PVMDIR_INTEGRITY_REPORT pUpdate
    );

VOID
VmDirFreeIntegrityReport(
    PVMDIR_INTEGRITY_REPORT pReport
    );

VOID
VmDirFreeIntegrityReportList(
    PVDIR_LINKED_LIST   pReports
    );

// metrics.c
DWORD
VmDirRpcMetricsInit(
    VOID
    );

VOID
VmDirRpcMetricsUpdate(
    METRICS_RPC_OPS operation,
    uint64_t        iStartTime,
    uint64_t        iEndTime
    );

VOID
VmDirRpcMetricsShutdown(
    VOID
    );

DWORD
VmDirSrvStatMetricsInit(
    VOID
    );

VOID
VmDirSrvStatMetricsUpdate(
    METRICS_SRV_STAT srvStat,
    uint64_t        iValue
    );

VOID
VmDirSrvStatMetricsShutdown(
    VOID
    );

// promote/dcactmgmt.c
DWORD
VmDirCreateDomainController(
    PCSTR   pszDomain,
    PCSTR   pszDCName
    );

size_t
VmDirWriteQueueSize(
    PVMDIR_WRITE_QUEUE          pWriteQueue
    );

#ifdef __cplusplus
}
#endif

#endif /* __VMDIRMAIN_H__ */


