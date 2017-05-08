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

#include <lw/types.h>
#include <lw/hash.h>
#include <lw/security-api.h>

#include <vmsuperlogging.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REPL_THREAD_SCHED_PRIORITY 10

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
    VDIR_BERVALUE        systemDomainDN;
    VDIR_BERVALUE        delObjsContainerDN;    //TODO, delete this
    VDIR_BERVALUE        bvDCGroupDN;
    VDIR_BERVALUE        bvDCClientGroupDN;     //TODO, delete this
    VDIR_BERVALUE        bvServicesRootDN;
    VDIR_BERVALUE        serverObjDN;
    VDIR_BERVALUE        dcAccountDN;   // Domain controller account DN
    VDIR_BERVALUE        dcAccountUPN;  // Domain controller account UPN
    int                  replInterval;
    int                  replPageSize;
    VDIR_BERVALUE        utdVector; // In string format, it is stored as: <serverId1>:<origUsn1>;<serverId2>:<origUsn2>;...
    PSTR                 pszSiteName;           //TODO, delete this
    BOOLEAN              isIPV4AddressPresent;
    BOOLEAN              isIPV6AddressPresent;
    USN                  initialNextUSN; // used for server restore only
    USN                  maxOriginatingUSN;  // Cache value to prevent
                                             // excessive searching
    VDIR_BERVALUE        bvServerObjName;
    DWORD                dwDomainFunctionalLevel;
} VMDIR_SERVER_GLOBALS, *PVMDIR_SERVER_GLOBALS;

extern VMDIR_SERVER_GLOBALS gVmdirServerGlobals;

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
    BOOLEAN                         bIsLDAPPortOpen;
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
    DWORD                           dwReplCycleCounter;

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
    DWORD                           dwEnableRaftReferral;
    DWORD                           dwRaftElectionTimeoutMS;
    DWORD                           dwRaftPingIntervalMS;
    //Raft logs to keep in 1000
    DWORD                           dwRaftKeeplogs;
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

typedef struct _VMDIR_URGENT_REPL
{
    // To Synchronize Urgent Replication Request
    PVMDIR_MUTEX                         pUrgentReplMutex;
    BOOLEAN                              bUrgentReplicationPending;
    DWORD                                dwUrgentReplResponseCount;
    DWORD                                dwUrgentReplTimeout;
    USN                                  consensusUSN;
    PSTR                                 pUTDVector;
    /*
     * Used by RPC thread to notify urgentReplicationCoordinator thread
     * if rpc response was received.
     */
    PVMDIR_MUTEX                         pUrgentReplResponseRecvMutex;
    PVMDIR_COND                          pUrgentReplResponseRecvCondition;
    BOOLEAN                              bUrgentReplResponseRecv;
    /*
     * Used by writer thread to notify urgentReplicationCoordinator thread
     * to start urgent replication cycle immediately.
     */
    PVMDIR_MUTEX                         pUrgentReplThreadMutex;
    PVMDIR_COND                          pUrgentReplThreadCondition;
    BOOLEAN                              bUrgentReplThreadPredicate;
    /*
     * Used by urgentReplicationCoordinator thread to notify writer threads
     * that urgent replication cycle is completed. It is a broadcast.
     */
    PVMDIR_MUTEX                         pUrgentReplDoneMutex;
    PVMDIR_COND                          pUrgentReplDoneCondition;
    BOOLEAN                              bUrgentReplDone;
    /*
     * Used by RPC thread to notify Replicaton thread to start urgent repl
     * uses bReplNow predicate for proper synchronization.
     */
    PVMDIR_MUTEX                         pUrgentReplStartMutex;
    PVMDIR_COND                          pUrgentReplStartCondition;

    PVMDIR_STRONG_WRITE_PARTNER_CONTENT  pUrgentReplPartnerTable;
    PVMDIR_URGENT_REPL_SERVER_LIST       pUrgentReplServerList;
} VMDIR_URGENT_REPL, *PVMDIR_URGENT_REPL;

extern VMDIR_URGENT_REPL gVmdirUrgentRepl;

typedef struct _VMDIR_TRACK_LAST_LOGIN_TIME
{
    PVMDIR_MUTEX    pMutex;
    PVMDIR_COND     pCond;
    PVMDIR_TSSTACK  pTSStack;
} VMDIR_TRACK_LAST_LOGIN_TIME, *PVMDIR_TRACK_LAST_LOGIN_TIME;

extern VMDIR_TRACK_LAST_LOGIN_TIME gVmdirTrackLastLoginTime;

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

VOID
VmDirdSetLimitLocalUsnToBeSupplied(
    USN usn
    );

USN
VmDirdGetLimitLocalUsnToBeSupplied(
    VOID
    );

DWORD
VmDirServerStatusEntry(
    PVDIR_ENTRY*    ppEntry
    );

DWORD
VmDirRaftStateEntry(
    PVDIR_ENTRY*    ppEntry
    );

DWORD
VmDirReplicationStatusEntry(
    PVDIR_ENTRY*    ppEntry
    );

//urgentreplthread.c
VOID
VmDirUrgentReplSignalUrgentReplCoordinatorThreadResponseRecv(
    VOID
    );

VOID
VmDirUrgentReplSignalUrgentReplCoordinatorThreadStart(
    VOID
    );

DWORD
VmDirTimedWaitForUrgentReplDone(
    UINT64 timeout,
    UINT64 startTime
    );

BOOLEAN
VmDirUrgentReplCondTimedWait(
    VOID
    );

VOID
VmDirUrgentReplSignal(
    VOID
    );

//urgentrepl.c
BOOLEAN
VmDirdGetUrgentReplicationRequest(
    VOID
    );

BOOLEAN
VmDirdGetUrgentReplicationRequest_InLock(
    VOID
    );

VOID
VmDirdSetUrgentReplicationRequest(
    BOOLEAN bUrgentRepl
    );

VOID
VmDirdSetUrgentReplicationRequest_InLock(
    BOOLEAN bUrgentRepl
    );

PVMDIR_URGENT_REPL_SERVER_LIST
VmDirdGetUrgentReplicationServerList(
    VOID
    );

PVMDIR_URGENT_REPL_SERVER_LIST
VmDirdGetUrgentReplicationServerList_InLock(
    VOID
    );

DWORD
VmDirdAddToUrgentReplicationServerList(
    PSTR    pszUrgentReplicationServer
    );

DWORD
VmDirdAddToUrgentReplicationServerList_InLock(
    PSTR    pszUrgentReplicationServer
    );

VOID
VmDirdFreeUrgentReplicationServerList(
    VOID
    );

VOID
VmDirdFreeUrgentReplicationServerList_InLock(
    VOID
    );

DWORD
VmDirdInitiateUrgentRepl(
    PSTR   pszServerName
    );

VOID
VmDirSendAllUrgentReplicationResponse(
    VOID
    );

DWORD
VmDirdUrgentReplSetUtdVector(
    PCSTR pUTDVector
    );

PCSTR
VmDirdUrgentReplGetUtdVector(
    VOID
    );

PCSTR
VmDirdUrgentReplGetUtdVector_InLock(
    VOID
    );

VOID
VmDirReplUpdateUrgentReplCoordinatorTableForRequest(
    VOID
    );

VOID
VmDirReplUpdateUrgentReplCoordinatorTableForResponse(
    PVMDIR_REPL_UTDVECTOR pUtdVector,
    PCSTR pszInvocationId,
    PSTR pszHostName
    );

VOID
VmDirReplUpdateUrgentReplCoordinatorTableForResponse_InLock(
    PSTR  pInvocationId,
    USN   confirmedUSN,
    PVMDIR_STRONG_WRITE_PARTNER_CONTENT pReplicationPartnerEntry
    );

DWORD
VmDirReplGetUrgentReplCoordinatorTableEntry_InLock(
    PCSTR pszRemoteServerInvocationId,
    PSTR  pszRemoteServerName,
    PVMDIR_STRONG_WRITE_PARTNER_CONTENT *ppReplicationPartnerEntry
    );

VOID
VmDirReplUpdateUrgentReplCoordinatorTableForDelete(
    PVMDIR_REPLICATION_AGREEMENT  pReplAgr
    );

VOID
VmDirReplFreeUrgentReplCoordinatorTable(
    VOID
    );

VOID
VmDirReplFreeUrgentReplCoordinatorTable_InLock(
    VOID
    );

DWORD
VmDirReplGetUrgentReplResponseCount(
    VOID
    );

DWORD
VmDirReplGetUrgentReplResponseCount_InLock(
    VOID
    );

VOID
VmDirReplUpdateUrgentReplResponseCount(
    VOID
    );

VOID
VmDirReplResetUrgentReplResponseCount(
    VOID
    );

VOID
VmDirReplResetUrgentReplResponseCount_InLock(
    VOID
    );

VOID
VmDirReplSetUrgentReplResponseRecvCondition(
    BOOLEAN bUrgentReplResponseRecv
    );

VOID
VmDirReplSetUrgentReplResponseRecvCondition_InLock(
    BOOLEAN bUrgentReplResponseRecv
    );

BOOLEAN
VmDirReplGetUrgentReplResponseRecvCondition(
    VOID
    );

BOOLEAN
VmDirReplGetUrgentReplResponseRecvCondition_InLock(
    VOID
    );

VOID
VmDirReplSetUrgentReplThreadCondition(
    BOOLEAN bUrgentReplThreadPredicate
    );

BOOLEAN
VmDirReplGetUrgentReplThreadCondition(
    VOID
    );

PVMDIR_STRONG_WRITE_PARTNER_CONTENT
VmDirReplGetUrgentReplCoordinatorTable(
    VOID
    );

PVMDIR_STRONG_WRITE_PARTNER_CONTENT
VmDirReplGetUrgentReplCoordinatorTable_InLock(
    VOID
    );

VOID
VmDirReplSetUrgentReplDoneCondition(
    BOOLEAN bUrgentReplDone
    );

VOID
VmDirReplSetUrgentReplDoneCondition_InLock(
    BOOLEAN bUrgentReplDone
    );

BOOLEAN
VmDirReplGetUrgentReplDoneCondition(
    VOID
    );

BOOLEAN
VmDirReplGetUrgentReplDoneCondition_InLock(
    VOID
    );

USN
VmDirGetUrgentReplConsensus(
    VOID
    );

USN
VmDirGetUrgentReplConsensus_InLock(
    VOID
    );

VOID
VmDirSetUrgentReplConsensus_InLock(
    USN
    );

BOOLEAN
VmDirUrgentReplUpdateConsensus(
    VOID
    );

DWORD
VmDirGetUrgentReplTimeout(
    VOID
    );

DWORD
VmDirGetUrgentReplTimeout_InLock(
    VOID
    );

VOID
VmDirSetUrgentReplTimeout(
    DWORD dwTimeout
    );

VOID
VmDirSetUrgentReplTimeout_InLock(
    DWORD dwTimeout
    );

BOOLEAN
VmDirGetUrgentReplicationPending(
    VOID
    );

BOOLEAN
VmDirGetUrgentReplicationPending_InLock(
    VOID
    );

VOID
VmDirSetUrgentReplicationPending(
    BOOLEAN bUrgentReplicationPending
    );

VOID
VmDirSetUrgentReplicationPending_InLock(
    BOOLEAN bUrgentReplicationPending
    );

VOID
VmDirReplFreeUrgentReplPartnerEntry_InLock(
    PVMDIR_STRONG_WRITE_PARTNER_CONTENT pUrgentReplPartnerTable
    );

DWORD
VmDirGetReplicationPartnerCount(
    VOID
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

#ifdef __cplusplus
}
#endif

#endif /* __VMDIRMAIN_H__ */


