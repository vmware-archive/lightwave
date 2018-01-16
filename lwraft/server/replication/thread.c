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
 * Module Name: Replication thread
 *
 * Filename: thread.c
 *
 * Abstract:
 *  The directory server replication algorithm is an implementation of the Raft paper
 *  (referred as "the paper" in the source file comments) for replicating LDAP entry changes (Add/Modify/Delete/Moddn):
 *  Diego Ongaro and John Ousterhout, “In Search of an Understandable Consensus Algorithm (Extended Version)”, May 20, 2014
 *  The Raft protocol replaces (or as an alternative to) vmdird multi-master replication algorithm.
 *  See the functional specification for detail: https://wiki.eng.vmware.com/Lightwave/Raft#Project_Milestone
 */

#include "includes.h"

extern DWORD AttrListToEntry(PVDIR_SCHEMA_CTX, PSTR, PSTR*, PVDIR_ENTRY);
extern DWORD VmDirAddModSingleAttributeReplace(PVDIR_OPERATION, PCSTR, PCSTR, PVDIR_BERVALUE);
extern DWORD VmDirCloneStackOperation(PVDIR_OPERATION, PVDIR_OPERATION, VDIR_OPERATION_TYPE, ber_tag_t, PVDIR_SCHEMA_CTX);
extern int VmDirEntryAttrValueNormalize(PVDIR_ENTRY, BOOLEAN);
extern DWORD VmDirSyncCounterReset(PVMDIR_SYNCHRONIZE_COUNTER pSyncCounter, int syncValue);
extern DWORD VmDirConditionBroadcast(PVMDIR_COND pCondition);

static DWORD _VmDirAppendEntriesRpc(PVMDIR_SERVER_CONTEXT *ppServer, PVMDIR_PEER_PROXY pProxySelf, int);
static DWORD _VmDirRequestVoteRpc(PVMDIR_SERVER_CONTEXT *pServer, PVMDIR_PEER_PROXY pProxySelf);
static DWORD _VmDirReplicationThrFun(PVOID);
static DWORD _VmDirRaftVoteSchdThread();
static DWORD _VmDirRaftLogCompactThread();
static DWORD _VmDirRaftLogApplyThread();
static DWORD _VmDirRaftCompactLogs(int);
static DWORD _VmDirStartProxies(VOID);
static DWORD _VmDirRpcConnect(PVMDIR_SERVER_CONTEXT *ppServer, PVMDIR_PEER_PROXY pProxySelf);
static VOID _VmDirApplyLogsUpto(UINT64 indexToApply);
static DWORD _VmDirApplyLog(unsigned long long);
static VOID _VmDirWaitPeerThreadsShutdown();
static DWORD _VmDirDeleteRaftProxy(char *dn_norm);
static VOID _VmDirRemovePeerInLock(PCSTR pHostname);
static DWORD _VmDirGetRaftQuorumOverride(BOOLEAN bForceKeyRead);
static VOID _VmDirEvaluateVoteResult(UINT64 *waitTime);
static VOID _VmDirPersistTerm(int term);

DWORD VmDirRaftGetFollowers(PDEQUE followers);

PVMDIR_MUTEX gRaftStateMutex = NULL;
PVMDIR_MUTEX gRaftRpcReplyMutex = NULL;

PVMDIR_COND gRaftRequestPendingCond = NULL;
PVMDIR_COND gRaftRequestVoteCond = NULL;
PVMDIR_COND gRaftAppendEntryReachConsensusCond = NULL;
PVMDIR_COND gGotVoteResultCond = NULL;
PVMDIR_COND gPeersReadyCond = NULL;
PVMDIR_COND gRaftNewLogCond = NULL;

VDIR_RAFT_STATE gRaftState = {0};
PVDIR_RAFT_LOG gEntries = NULL;
VDIR_RAFT_LOG gLogEntry = {0};

static char gNewPartner[VMDIR_MAX_LDAP_URI_LEN] = {0};
static DWORD gQuorumOverride = 0;

VOID VmDirNewPartner(PCSTR *hostname)
{
    memcpy(gNewPartner, hostname, strlen((char *)hostname));
}

DWORD
InitializeReplicationThread(
    void)
{
    DWORD               dwError = 0;
    PVDIR_THREAD_INFO   pThrInfo = NULL;

    dwError = VmDirAllocateMutex(&gRaftStateMutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&gRaftRpcReplyMutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateCondition(&gRaftRequestPendingCond);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateCondition(&gPeersReadyCond);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateCondition(&gRaftRequestVoteCond);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateCondition(&gRaftAppendEntryReachConsensusCond);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateCondition(&gGotVoteResultCond);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateCondition(&gRaftNewLogCond);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (gVmdirGlobals.dwRaftElectionTimeoutMS < 10 ||
        gVmdirGlobals.dwRaftPingIntervalMS < 20 ||
        gVmdirGlobals.dwRaftElectionTimeoutMS <= (gVmdirGlobals.dwRaftPingIntervalMS << 1))
    {
        dwError = LDAP_OPERATIONS_ERROR;
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
          "InitializeReplicationThread: Raft parameter Error %d: %s must be greater than twice of %s",
          dwError, "RaftElectionTimeoutMS", "RaftPingIntervalMS");
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "InitializeReplicationThread Raft parameters: %s=%d, %s=%d",
      "RaftElectionTimeoutMS", gVmdirGlobals.dwRaftElectionTimeoutMS,
      "RaftPingIntervalMS", gVmdirGlobals.dwRaftPingIntervalMS);

    dwError = VmDirSrvThrInit(
            &pThrInfo,
            gVmdirGlobals.replAgrsMutex,       // alternative mutex
            gVmdirGlobals.replAgrsCondition,   // alternative cond
            TRUE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateThread(
            &pThrInfo->tid,
            pThrInfo->bJoinThr,
            _VmDirReplicationThrFun,
            pThrInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSrvThrAdd(pThrInfo);

cleanup:

    return dwError;

error:

    if (pThrInfo)
    {
        VmDirSrvThrFree(pThrInfo);
    }

    goto cleanup;
}

/*
 * This thread detect ping recieve timeout (not get ping from leader for a peroid of time of time of time),
 * and start a vote when this occur or start an new vote if the request vote got a split vote.
 */
static
DWORD
_VmDirRaftVoteSchdThread()
{
    int dwError = 0;
    int term = 0;

    gRaftState.role = VDIR_RAFT_ROLE_FOLLOWER;
    //Set initial election timeout higher to avoid triggering new election due to slow startup.
    UINT64 waitTime = (gVmdirGlobals.dwRaftElectionTimeoutMS << 1) + 5000;
    BOOLEAN bLock = FALSE;

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirRaftVoteSchdThread: started.");

    while(1)
    {
        UINT64 now = {0};

        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            goto done;
        }

        VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "_VmDirRaftVoteSchdThread wait %llu ms", waitTime);
        VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);

        VmDirConditionTimedWait(gRaftRequestVoteCond, gRaftStateMutex, waitTime);

        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            goto done;
        }

        if (gRaftState.clusterSize < 2)
        {
            VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
            continue;
        }

        if (gRaftState.role == VDIR_RAFT_ROLE_CANDIDATE)
        {
            //Split votes
            goto startVote;
        } else if (gRaftState.role == VDIR_RAFT_ROLE_FOLLOWER)
        {
            UINT64 waitTimeRemain = 0;

            now = VmDirGetTimeInMilliSec();
            waitTimeRemain = now - gRaftState.lastPingRecvTime;
            if (waitTimeRemain >= gVmdirGlobals.dwRaftElectionTimeoutMS)
            {
                //Hasn't recieved ping for an duration of gVmdirGlobals.dwRaftElectionTimeoutMS - switch to candidate
                gRaftState.role = VDIR_RAFT_ROLE_CANDIDATE;
                goto startVote;
            } else
            {
                VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
                continue;
            }
        } else
        {
            //Server is a leader
            waitTime = gVmdirGlobals.dwRaftElectionTimeoutMS;
            VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
            continue;
        }

startVote:
        // Reelection triggered, increment metrics counter
        VmMetricsCounterIncrement(pElectionTriggerCount);

        // Stay in gRaftStateMutex
        do
        {
            dwError = 0;
            if (_VmDirPeersIdleInLock() < (gRaftState.clusterSize/2))
            {
                 //Wait gPeersReadyCond only if not enough peers are Ready
                 dwError = VmDirConditionTimedWait(gPeersReadyCond, gRaftStateMutex, gVmdirGlobals.dwRaftPingIntervalMS);
            }
        } while (dwError == ETIMEDOUT);

        if (dwError)
        {
            //waitTime or other error during wait for gPeersReadyCond.
            VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
            if (dwError != ETIMEDOUT)
            {
                VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirRaftVoteSchdThread: wait gPeersReadyCond error: %d", dwError);
            }
            continue;
        }

        term = ++gRaftState.currentTerm;
        VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

        _VmDirPersistTerm(term);

        VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
        if (gRaftState.currentTerm != term)
        {
            //gRaftState.currentTerm changed during persisting term,
            //  or duing mutex unlock/lock, start over again.
            waitTime = gVmdirGlobals.dwRaftElectionTimeoutMS;
            VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
            continue;
        }

        if (gRaftState.votedForTerm == gRaftState.currentTerm &&
            gRaftState.votedFor.lberbv_len > 0)
        {
            //I have voted for someone else in this term via RequestVoteGetReply,
            gRaftState.role = VDIR_RAFT_ROLE_FOLLOWER;
            gRaftState.lastPingRecvTime = VmDirGetTimeInMilliSec();
            waitTime = gVmdirGlobals.dwRaftElectionTimeoutMS;
            VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
            continue;
        }

        VmDirFreeBervalContent(&gRaftState.votedFor);
        dwError = VmDirAllocateBerValueAVsnprintf(&gRaftState.votedFor, "%s", gRaftState.hostname.lberbv_val);
        gRaftState.votedForTerm = gRaftState.currentTerm;
        gRaftState.voteConsensusCnt = 1; //vote for self
        gRaftState.voteDeniedCnt = 0;
        gRaftState.voteConsensusTerm = gRaftState.currentTerm;
        gRaftState.cmd = ExecReqestVote;

        //Now invoke paralle RPC calls to all (available) peers
        VmDirConditionBroadcast(gRaftRequestPendingCond);

        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirRaftVoteSchdThread: wait vote result; role %d term %d",
                       gRaftState.role, gRaftState.currentTerm);

        //Wait for (majority of) peer threads to complete their RRC calls or timeout.
        VmDirConditionTimedWait(gGotVoteResultCond, gRaftStateMutex, gVmdirGlobals.dwRaftElectionTimeoutMS);
        gRaftState.cmd = ExecNone;
        VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

        //Evaluating vote outcome
        _VmDirEvaluateVoteResult(&waitTime);
    }

done:
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirRaftVoteSchdThread: shutdown completed.");
    return dwError;
}

/* Server implementation of RPC which initiates
 * vote on the follower node
 */
DWORD
VmDirRaftFollowerInitiateVoteSrv(VOID)
{
    DWORD   dwError = 0;
    UINT64  now = 0;
    BOOLEAN bLock = FALSE;

    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);

    // if not a follower ignore the call
    if (gRaftState.role != VDIR_RAFT_ROLE_FOLLOWER)
    {
        VMDIR_LOG_INFO(
                VMDIR_LOG_MASK_ALL, "Ignoring initiate vote as role is not Follower");
        goto cleanup;
    }

    // Set the last ping received to be older than timeout
    now = VmDirGetTimeInMilliSec();
    gRaftState.lastPingRecvTime = now - gVmdirGlobals.dwRaftElectionTimeoutMS;

    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

    // signal the relection thread
    VmDirConditionSignal(gRaftRequestVoteCond);

cleanup:
    // always unlock mutex
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
    return dwError;

}

/* Server implementation of RPC initiated at a leader node to find
 * followers and send initiate vote RPC to them
 */
DWORD
VmDirRaftStartVoteSrv(VOID)
{
    DWORD   dwError = 0;
    char    szHostName[VMDIR_MAX_HOSTNAME_LEN] = {0};
    BOOLEAN bLock = FALSE;
    BOOLEAN bRPCSucceeded = FALSE;
    DEQUE   followers = {0};
    PSTR    pszFollower = NULL;
    PSTR    pszDcAccountPwd = NULL;
    PVMDIR_SERVER_CONTEXT pServer = NULL;

    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);

    // valid only if I am a leader
    if (gRaftState.role != VDIR_RAFT_ROLE_LEADER)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_ROLE);
    }
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

    dwError = VmDirRaftGetFollowers(&followers);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirReadDCAccountPassword(&pszDcAccountPwd);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetHostName(szHostName, sizeof(szHostName)-1);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (!dequeIsEmpty(&followers) && !bRPCSucceeded)
    {
        dequePopLeft(&followers, (PVOID*)&pszFollower);

        dwError = VmDirOpenServerA(pszFollower, gVmdirServerGlobals.dcAccountUPN.lberbv_val,
                NULL, pszDcAccountPwd, 0, NULL, &pServer);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_LOG_INFO(
                VMDIR_LOG_MASK_ALL,
                "Starting forced vote from node %s to node %s",
                szHostName,
                pszFollower);

        // RPC call
        dwError = VmDirRaftFollowerInitiateVote(pServer);
        if (dwError)
        {
            // do not bail keep trying next follower
            VMDIR_LOG_INFO(
                    VMDIR_LOG_MASK_ALL,
                    "Initiate vote failed for %s, trying next follower",
                    pszFollower);
        }
        else
        {
            // succeeded for one break from loop
            bRPCSucceeded = TRUE;
        }

        VMDIR_SAFE_FREE_MEMORY(pszFollower);
        VmDirCloseServer(pServer);
        pServer = NULL;
    }
    // if none of the RPCs succeeded will bail here
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    // always unlock mutex
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
    VMDIR_SAFE_FREE_MEMORY(pszDcAccountPwd);
    dequeFreeStringContents(&followers);
    return dwError;

error:
    if (pServer)
    {
        VmDirCloseServer( pServer);
    }
    VMDIR_SAFE_FREE_MEMORY(pszFollower);
    goto cleanup;
}

/*
 * Logs from lastApplied to highest index may or maynot committed though the highest committed
 * log must have been persisted in this server per the committing and voting algorithm.
 * Those logs should be indirectly committed via a no-op entry as suggested in secion 5.4.2 and 8.
 */
static
VOID
_VmDirEvaluateVoteResult(UINT64 *waitTime)
{
    BOOLEAN bLock = FALSE;
    BOOLEAN bLockRpcReply = FALSE;
    DWORD dwError = 0;
    UINT64 uWaitTime = 0;
    unsigned long long logIdx = 0;
    PVDIR_SCHEMA_CTX pSchemaCtx = NULL;
    VDIR_OPERATION ldapOp = {0};
    char logEntryDn[RAFT_CONTEXT_DN_MAX_LEN] = {0};
    VDIR_ENTRY raftEntry = {0};
    char termStr[VMDIR_MAX_I64_ASCII_STR_LEN] = {0};
    char logIndexStr[VMDIR_MAX_I64_ASCII_STR_LEN] = {0};
    char objectGuidStr[VMDIR_GUID_STR_LEN];
    uuid_t guid = {0};
    BOOLEAN bHasTxn = FALSE;
    unsigned long long logIdxToCommit = 0;
    unsigned long long applyIdxStart = 0;
    unsigned int logTermToCommit = 0;

    //This mutex serializes with other Raft RPC handlers
    VMDIR_LOCK_MUTEX(bLockRpcReply, gRaftRpcReplyMutex);

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInitStackOperation(&ldapOp, VDIR_OPERATION_TYPE_REPL, LDAP_REQ_MODIFY, pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

    ldapOp.pBEIF = VmDirBackendSelect(NULL);
    assert(ldapOp.pBEIF);

    //Once returned successfully, it would block any other write transactions
    dwError = ldapOp.pBEIF->pfnBETxnBegin(ldapOp.pBECtx, VDIR_BACKEND_TXN_WRITE);
    BAIL_ON_VMDIR_ERROR(dwError);

    bHasTxn = TRUE;

    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);

    //block external updates until leader transition complete or end of this function.
    gRaftState.disallowUpdates = TRUE;

    //Now evalute vote outcome
    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
      "_VmDirEvaluateVoteResult: evaluting vote outcome: term %d role %d consensusTerm %d consensusCnt %d",
      gRaftState.currentTerm, gRaftState.role, gRaftState.voteConsensusTerm, gRaftState.voteConsensusCnt);

    uWaitTime = gVmdirGlobals.dwRaftElectionTimeoutMS;

    if (gRaftState.role != VDIR_RAFT_ROLE_CANDIDATE)
    {
        //Become follower via other means
        goto cleanup;
    }

    if (gRaftState.currentTerm != gRaftState.voteConsensusTerm ||
        gRaftState.voteConsensusCnt < (gRaftState.clusterSize/2 + 1))
    {
        //Split vote; wait randomly with a mean value dwRaftPingIntervalMS
        uWaitTime = (UINT64)(rand()%(gVmdirGlobals.dwRaftPingIntervalMS>>1));
        goto cleanup;
    }

    //Got majority of votes.
    gRaftState.role = VDIR_RAFT_ROLE_LEADER;
    applyIdxStart = gRaftState.lastApplied + 1;
    logIdxToCommit = gRaftState.lastLogIndex + 1;
    logTermToCommit = gRaftState.currentTerm;
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

    //Create a no-op Log, gLogEntry can be set or cleared only
    //  when a thread holds MDB write transaction mutex.
    gLogEntry.index = logIdxToCommit;
    gLogEntry.term = logTermToCommit;
    gLogEntry.entryId = 0;
    gLogEntry.requestCode = 0;
    gLogEntry.chglog.lberbv_len = 0;
    gLogEntry.chglog.lberbv_val = NULL;

    dwError = _VmDirPackLogEntry(&gLogEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA(logEntryDn, sizeof(logEntryDn), "%s=%llu,%s",
                ATTR_CN, gLogEntry.index, RAFT_LOGS_CONTAINER_DN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA(termStr, sizeof(termStr), "%d", gLogEntry.term);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA(logIndexStr, sizeof(logIndexStr), "%llu", gLogEntry.index);
    BAIL_ON_VMDIR_ERROR(dwError);

    {
        PSTR ppLogEntry[] = {ATTR_DN, logEntryDn,
                             ATTR_CN, logIndexStr,
                             ATTR_OBJECT_CLASS,  OC_CLASS_RAFT_LOG_ENTRY,
                             ATTR_RAFT_LOGINDEX, logIndexStr,
                             ATTR_RAFT_TERM, termStr,
                             NULL };
        dwError = AttrListToEntry(pSchemaCtx, logEntryDn, ppLogEntry, &raftEntry);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetParentDN(&raftEntry.dn, &raftEntry.pdn );
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirUuidGenerate (&guid);
    VmDirUuidToStringLower(&guid, objectGuidStr, sizeof(objectGuidStr));

    dwError = VmDirAllocateStringA(objectGuidStr, &raftEntry.pszGuid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirEntryAddSingleValueStrAttribute(&raftEntry, ATTR_OBJECT_GUID, raftEntry.pszGuid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirEntryAddBervArrayAttribute(&raftEntry, ATTR_RAFT_LOG_ENTRIES, &gLogEntry.packRaftLog, 1);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirComputeObjectSecurityDescriptor(NULL, &raftEntry, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    raftEntry.eId = VmDirRaftLogEntryId(gLogEntry.index);
    dwError = ldapOp.pBEIF->pfnBEEntryAdd(ldapOp.pBECtx, &raftEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldapOp.pBEIF->pfnBETxnCommit(ldapOp.pBECtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    //VmDirRaftPrepareCommit now owns gLogEntry
    bHasTxn = FALSE;

    for (logIdx = applyIdxStart; logIdx < logIdxToCommit; logIdx++)
    {
        _VmDirApplyLog(logIdx);
    }

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
      "_VmDirEvaluateVoteResult: completed log (%llu %u)", logIdxToCommit, logTermToCommit);

cleanup:
    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
    gRaftState.disallowUpdates = FALSE;
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

    if (bHasTxn)
    {
        _VmDirChgLogFree(&gLogEntry);
        ldapOp.pBEIF->pfnBETxnAbort(ldapOp.pBECtx);
    }
    VMDIR_UNLOCK_MUTEX(bLockRpcReply, gRaftRpcReplyMutex);
    VmDirFreeOperationContent(&ldapOp);
    VmDirFreeEntryContent(&raftEntry);
    *waitTime = uWaitTime;
    return;

error:
    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
    gRaftState.role = VDIR_RAFT_ROLE_FOLLOWER;
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
      "_VmDirEvaluateVoteResult: log(%llu, %d) error %d", logIdxToCommit, logTermToCommit, dwError);
    goto cleanup;
}

static
DWORD
VmDirRaftPeerThread(void *ctx)
{
    BOOLEAN bLock = FALSE;
    UINT64 pingTimeout = gVmdirGlobals.dwRaftPingIntervalMS;
    UINT64 prevPingTime = {0};
    UINT64 now = {0};
    int cmd = ExecNone;
    PVMDIR_SERVER_CONTEXT pServer = NULL;
    PVMDIR_PEER_PROXY pProxySelf = (PVMDIR_PEER_PROXY)ctx;
    PSTR pPeerHostName = pProxySelf->raftPeerHostname;

    do
    {
       if(_VmDirRpcConnect(&pServer, pProxySelf)==0)
       {
           break;
       }
       VmDirSleep(3000);
    } while (VmDirdState() != VMDIRD_STATE_SHUTDOWN);

    while(1)
    {
        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN || pProxySelf->isDeleted)
        {
            goto done;
        }

        VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "VmDirRaftPeerThread: wait RequestPendingCond (peer %s); role %d term %d",
                        pPeerHostName, gRaftState.role, gRaftState.currentTerm);

        VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
        pProxySelf->proxy_state = RPC_IDLE;
        if (_VmDirPeersIdleInLock() >= (gRaftState.clusterSize/2))
        {
            VmDirConditionSignal(gPeersReadyCond);
        }
        VmDirConditionTimedWait(gRaftRequestPendingCond, gRaftStateMutex, pingTimeout);
        pProxySelf->proxy_state = RPC_BUSY;
        VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

appendEntriesRepeat:
        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN || pProxySelf->isDeleted)
        {
            goto done;
        }

        VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "VmDirRaftPeerThread: exit RequestPendingCond (peer %s); role %d term %d",
                        pPeerHostName, gRaftState.role, gRaftState.currentTerm);
        VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
        cmd = gRaftState.cmd;
        if (gRaftState.role == VDIR_RAFT_ROLE_LEADER)
        {
            now = VmDirGetTimeInMilliSec();
            if (cmd == ExecNone)
            {
                VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "VmDirRaftPeerThread: current gRaftState.cmd %d (peer %s); role %d term %d",
                                gRaftState.cmd, pPeerHostName, gRaftState.role, gRaftState.currentTerm);
                if ((now - prevPingTime) >= gVmdirGlobals.dwRaftPingIntervalMS)
                {
                    prevPingTime = now;
                    pingTimeout = gVmdirGlobals.dwRaftPingIntervalMS;
                    cmd = ExecPing;
                    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "VmDirRaftPeerThread: will ExecPing to peer %s; role %d term %d",
                                    pPeerHostName, gRaftState.role, gRaftState.currentTerm);
                } else
                {
                    pingTimeout = now - prevPingTime;
                    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
                    continue;
                }
            } else
            {
                // AppendEntriesRpc - reset ping time timeout
                pingTimeout = gVmdirGlobals.dwRaftPingIntervalMS;
                prevPingTime = now;
            }
        } else
        {
            //Reset prevPingTime so that ping will be sent immediately switching to leader.
            prevPingTime = 0;
        }
        VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

        VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "VmDirRaftPeerThread: to exe gRaftState.cmd %d (peer %s); role %d term %d",
                        gRaftState.cmd, pPeerHostName, gRaftState.role, gRaftState.currentTerm);
        switch(cmd)
        {
            case ExecReqestVote:
                 _VmDirRequestVoteRpc(&pServer, pProxySelf);
                 break;
            case ExecAppendEntries:
                 _VmDirAppendEntriesRpc(&pServer, pProxySelf, ExecAppendEntries);
                 break;
            case ExecPing:
                 _VmDirAppendEntriesRpc(&pServer, pProxySelf, ExecPing);
                 break;
            case ExecNone:
                 continue;
            default:
                 VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "VmDirRaftPeerThread request type unknown %d", gRaftState.cmd);
                 continue;
        }

        VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
        if (gRaftState.role == VDIR_RAFT_ROLE_LEADER &&
            gRaftState.cmd == ExecAppendEntries &&
            gEntries && gEntries->index > pProxySelf->matchIndex)
        {
            //During AppendEntriesRpc, new AppendEntries is initiated,
            //  (e.g. other peers proceed faster than this peer), and the completed RPC
            //  needs to replicate the new gEntries without waiting.
            VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
            goto appendEntriesRepeat;
        }

        if (gRaftState.cmd == ExecReqestVote)
        {
            //Don't send more than one votes to the same peer in this round (term) of votes.
            gRaftState.cmd = ExecNone;
        }

        VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
    }

done:
    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "VmDirRaftPeerThread: thread for peer %s exits", pPeerHostName);
    if (pServer)
    {
        VmDirCloseServer( pServer);
    }
    pProxySelf->isDeleted = TRUE;
    pProxySelf->tid = 0;
    return 0;
}

static
VOID
_VmDirRemovePeerInLock(PCSTR pHostname)
{
    PVMDIR_PEER_PROXY curProxy = NULL;

    for(curProxy=gRaftState.proxies; curProxy; curProxy=curProxy->pNext)
    {
        if (VmDirStringCompareA(curProxy->raftPeerHostname, pHostname, FALSE)==0)
        {
            break;
        }
    }
    if (curProxy)
    {
        if (curProxy->isDeleted == FALSE && curProxy->proxy_state != PENDING_ADD)
        {
            curProxy->isDeleted = TRUE;
            gRaftState.clusterSize--;
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirRemovePeerInLock: raft cluster size changed to %d", gRaftState.clusterSize);
        }
    }
}

static
DWORD
_VmDirNewPeerProxyInLock(PCSTR pHostname, VDIR_RAFT_PROXY_STATE state)
{
    DWORD dwError = 0;
    PVMDIR_PEER_PROXY pPp = NULL;
    PVMDIR_PEER_PROXY curProxy = NULL;

    for(curProxy=gRaftState.proxies; curProxy; curProxy=curProxy->pNext)
    {
        if (VmDirStringCompareA(curProxy->raftPeerHostname, pHostname, FALSE)==0)
        {
            break;
        }
    }

    if (curProxy && !curProxy->isDeleted)
    {
       //Peer exists already.
       goto cleanup;
    }

    if (curProxy && curProxy->isDeleted)
    {
        //Add the previously deleted back.
        pPp = curProxy;
    } else
    {
        dwError = VmDirAllocateMemory( sizeof(VMDIR_PEER_PROXY), (PVOID*)&pPp);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirStringCpyA(pPp->raftPeerHostname, sizeof(pPp->raftPeerHostname), pHostname);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (gRaftState.proxies == NULL)
    {
        gRaftState.proxies = pPp;
    } else if (pPp->isDeleted)
    {
       pPp->isDeleted = FALSE;
    } else
    {
        pPp->pNext = gRaftState.proxies;
        gRaftState.proxies = pPp;
    }

    //If the proxy is in promo process, then it Will be set to RpcIdle when receving a vote request from the peer.
    pPp->proxy_state = state;

    dwError = VmDirCreateThread(&pPp->tid, TRUE, VmDirRaftPeerThread, (PVOID)pPp);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "VmDirNewPeerProxy: added new peer proxy for host %s", pHostname);
cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirNewPeerProxy: added new peer proxy for host %s error %d", pHostname, dwError);
    goto cleanup;
}

static
DWORD
_VmDirReplicationThrFun(
    PVOID   pArg
    )
{
    int dwError = 0;
    BOOLEAN bInReplAgrsLock = FALSE;
    BOOLEAN bInReplCycleDoneLock = FALSE;
    PVDIR_THREAD_INFO pRaftVoteSchdThreadInfo = NULL;
    PVDIR_THREAD_INFO pRaftLogApplyThreadInfo = NULL;
    PVDIR_THREAD_INFO pRaftLogCompactThreadInfo = NULL;
    BOOLEAN bGlobalsLoaded = FALSE;
    PSTR pszLocalErrorMsg = NULL;
    BOOLEAN bSignaledConnThreads = FALSE;

    dwError = _VmDirRaftLoadGlobals(&pszLocalErrorMsg);
    if (dwError == 0)
    {
        bGlobalsLoaded = TRUE;
    }

    if (!bGlobalsLoaded)
    {
        BOOLEAN firstServer = FALSE;

        VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
        dwError = 0;

        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "_VmDirReplicationThrFun: waiting for promoting ...");
        //server has not complete vdcpromo, wait signal triggered by vdcpromo

        //In case lwraftd restart while waiting for promo, this call will advance raft logs.
        _VmDirLoadRaftState();

        VMDIR_LOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);
        dwError = VmDirConditionWait( gVmdirGlobals.replAgrsCondition, gVmdirGlobals.replAgrsMutex );
        BAIL_ON_VMDIR_ERROR( dwError);
        VMDIR_UNLOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);

        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            dwError = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR( dwError);
        }

        if (gNewPartner[0] != '\0')
        {
            dwError=VmDirFirstReplicationCycle(gNewPartner);
            BAIL_ON_VMDIR_ERROR( dwError );

            VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "_VmDirReplicationThrFun: complete vdcpromo from partner %s.", gNewPartner);
        } else
        {
            firstServer = TRUE;
            VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "_VmDirReplicationThrFun: complete vdcpromo as first server.");
        }

        if (firstServer)
        {
            //Wake up LDAP connection threads so that account can be provisioned via LDAP
            VMDIR_LOCK_MUTEX(bInReplCycleDoneLock, gVmdirGlobals.replCycleDoneMutex);
            VmDirConditionSignal(gVmdirGlobals.replCycleDoneCondition);
            VMDIR_UNLOCK_MUTEX(bInReplCycleDoneLock, gVmdirGlobals.replCycleDoneMutex);
        }

        //Wait until vdcpromo has completed adding the DC to cluster.
        int retryCnt = 0;
        while(TRUE)
        {
            dwError = _VmDirRaftLoadGlobals(&pszLocalErrorMsg);
            if (dwError == 0 || retryCnt++ > 10)
            {
                break;
            }
            VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
            VmDirSleep(3000);
        }
    }

    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "_VmDirReplicationThrFun: _VmDirRaftLoadGlobals");

    dwError = _VmDirLoadRaftState();
    BAIL_ON_VMDIR_ERROR(dwError);

    _VmDirGetRaftQuorumOverride(TRUE);

    dwError = _VmDirStartProxies();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvThrInit(
            &pRaftVoteSchdThreadInfo,
            gVmdirGlobals.replAgrsMutex,
            gVmdirGlobals.replAgrsCondition,
            TRUE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateThread(
            &pRaftVoteSchdThreadInfo->tid,
            pRaftVoteSchdThreadInfo->bJoinThr,
            _VmDirRaftVoteSchdThread,
            pRaftVoteSchdThreadInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSrvThrAdd(pRaftVoteSchdThreadInfo);

    dwError = VmDirSrvThrInit(
            &pRaftLogApplyThreadInfo,
            gVmdirGlobals.replAgrsMutex,
            gVmdirGlobals.replAgrsCondition,
            TRUE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateThread(
            &pRaftLogApplyThreadInfo->tid,
            pRaftLogApplyThreadInfo->bJoinThr,
            _VmDirRaftLogApplyThread,
            pRaftLogApplyThreadInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSrvThrAdd(pRaftLogApplyThreadInfo);

    dwError = VmDirSrvThrInit(
            &pRaftLogCompactThreadInfo,
            gVmdirGlobals.replAgrsMutex,
            gVmdirGlobals.replAgrsCondition,
            TRUE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateThread(
            &pRaftLogCompactThreadInfo->tid,
            pRaftLogCompactThreadInfo->bJoinThr,
            _VmDirRaftLogCompactThread,
            pRaftLogCompactThreadInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSrvThrAdd(pRaftLogCompactThreadInfo);

    VMDIR_LOCK_MUTEX(bInReplCycleDoneLock, gVmdirGlobals.replCycleDoneMutex);
    VmDirConditionSignal(gVmdirGlobals.replCycleDoneCondition);
    VMDIR_UNLOCK_MUTEX(bInReplCycleDoneLock, gVmdirGlobals.replCycleDoneMutex);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirReplicationThrFun: started.");

    while (1)
    {
        if (!bSignaledConnThreads && VmDirdState() == VMDIRD_STATE_NORMAL)
        {
            //Wake up LDAP connection threads - signal again, connection threads may
            // have missed the signal.
            VMDIR_LOCK_MUTEX(bInReplCycleDoneLock, gVmdirGlobals.replCycleDoneMutex);
            VmDirConditionSignal(gVmdirGlobals.replCycleDoneCondition);
            VMDIR_UNLOCK_MUTEX(bInReplCycleDoneLock, gVmdirGlobals.replCycleDoneMutex);
            bSignaledConnThreads = TRUE;
        }
        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            BOOLEAN bLock = FALSE;
            VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
            VmDirConditionBroadcast(gRaftRequestPendingCond);
            VmDirConditionBroadcast(gPeersReadyCond);
            VmDirConditionSignal(gRaftAppendEntryReachConsensusCond);
            VmDirConditionSignal(gRaftRequestVoteCond);
            VmDirConditionSignal(gGotVoteResultCond);
            VmDirConditionSignal(gRaftNewLogCond);
            VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
            _VmDirWaitPeerThreadsShutdown();
            goto cleanup;
        }
        VmDirSleep( 3000 );
    }
    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirReplicationThrFun: shutdown completed.");

cleanup:
    VMDIR_UNLOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);
    VMDIR_UNLOCK_MUTEX(bInReplCycleDoneLock, gVmdirGlobals.replCycleDoneMutex);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);

    return 0;

error:
    if (VmDirdState() != VMDIRD_STATE_SHUTDOWN)
    {
        VmDirdStateSet( VMDIRD_STATE_FAILURE );
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "_VmDirReplicationThrFun: Replication has failed with unrecoverable error %d", dwError);
    }

    if (pRaftVoteSchdThreadInfo)
    {
         VmDirSrvThrFree(pRaftVoteSchdThreadInfo);
    }
    goto cleanup;
}

DWORD
VmDirGetReplCycleCounter(
    VOID
    )
{
    BOOLEAN bInLock = FALSE;
    DWORD   dwCount = 0;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirGlobals.replCycleDoneMutex);
    dwCount = gVmdirGlobals.dwReplCycleCounter;
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirGlobals.replCycleDoneMutex);

    return dwCount;
}

static
VOID
_VmDirWaitPeerThreadsShutdown()
{
    PVMDIR_PEER_PROXY pPeerProxy = NULL;
    VMDIR_THREAD tid = 0;
    BOOLEAN bLock = FALSE;

    pPeerProxy = gRaftState.proxies;
    while (pPeerProxy)
    {
        PVMDIR_PEER_PROXY pNext = pPeerProxy->pNext;
        VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
        if (pPeerProxy->tid == 0)
        {
            pPeerProxy = pNext;
            VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
            continue;
        }
        tid = pPeerProxy->tid;
        VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
        VmDirThreadJoin(&tid, NULL);
        pPeerProxy = pNext;
    }
}

static
DWORD
_VmDirRequestVoteRpc(PVMDIR_SERVER_CONTEXT *ppServer, PVMDIR_PEER_PROXY pProxySelf)
{
    DWORD dwError = 0;
    REQUEST_VOTE_ARGS reqVoteArgs = {0};
    PVMDIR_SERVER_CONTEXT pServer = *ppServer;
    PCSTR pPeerHostName = pProxySelf->raftPeerHostname;
    BOOLEAN bLock = FALSE;
    BOOLEAN waitSignaled = FALSE;

    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
    reqVoteArgs.candidateId = gRaftState.hostname.lberbv_val;
    reqVoteArgs.term = gRaftState.currentTerm;
    reqVoteArgs.lastLogIndex = gRaftState.lastLogIndex;
    reqVoteArgs.lastLogTerm = gRaftState.lastLogTerm;

    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

    dwError = VmDirRaftRequestVote(pServer, reqVoteArgs.term, reqVoteArgs.candidateId,
                                  (UINT32)reqVoteArgs.lastLogIndex, reqVoteArgs.lastLogTerm,
                                  &reqVoteArgs.currentTerm, &reqVoteArgs.voteGranted);
    if (dwError)
    {
        if (dwError == rpc_s_connect_rejected || dwError == rpc_s_connect_timed_out ||
            dwError == rpc_s_cannot_connect || dwError == rpc_s_connection_closed ||
            dwError == rpc_s_host_unreachable || pServer == NULL)
        {
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
              "_VmDirRequestVoteRpc: not connected or disconnected peer %s dcerpc error %d", pPeerHostName, dwError);
            pProxySelf->proxy_state = RPC_DISCONN;
            _VmDirRpcConnect(ppServer, pProxySelf);
            goto done;
        } else if (dwError == rpc_s_auth_method)
        {
             VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
               "_VmDirRequestVoteRpc: error rpc_s_auth_method, peer %s", pPeerHostName);
             pProxySelf->proxy_state = RPC_DISCONN;
             _VmDirRpcConnect(ppServer, pProxySelf);
             goto done;
        }

        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
          "_VmDirRequestVoteRpc: RPC call VmDirRaftRequestVote failed to peer %s error %d role %d term %d",
          pPeerHostName, dwError, gRaftState.role, gRaftState.currentTerm);
        goto done;
    }

    bLock = FALSE;
    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
    if (reqVoteArgs.currentTerm > gRaftState.currentTerm)
    {
        int oldTerm = 0;
        /* The vote must have been denied in this case.
         * The peer may also has its log index larger than mine in which case this server should
         * start a new vote the sooner the better.  However, we can't tell it is the case,
         * thus simply treat it as a split vote. We may improve the vote efficiency if we have an
         * additional a OUT parameter to tell this condition.
         */
        gRaftState.role = VDIR_RAFT_ROLE_FOLLOWER;
        gRaftState.lastPingRecvTime = VmDirGetTimeInMilliSec();
        oldTerm = gRaftState.currentTerm;
        gRaftState.currentTerm = reqVoteArgs.currentTerm;
        gRaftState.voteDeniedCnt++;
        //Wakeup _VmDirRaftVoteSchdThread
        waitSignaled = TRUE;
        VmDirConditionSignal(gGotVoteResultCond);
        //Avoid deadlock on MDB write mutex during persisting term.
        VmDirConditionSignal(gRaftAppendEntryReachConsensusCond);
        VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

        _VmDirPersistTerm(reqVoteArgs.currentTerm);

        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirRequestVoteRpc: peer (%s) term %d > current term %d, change role to follower",
                       pPeerHostName, reqVoteArgs.currentTerm, oldTerm);
        goto done;
    }

    if (gRaftState.role != VDIR_RAFT_ROLE_CANDIDATE)
    {
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
          "_VmDirRequestVoteRpc: server role was changed to %d - forfeit this RPC result; term %d; peer: (%s)(term %d)",
          gRaftState.role, gRaftState.currentTerm, pPeerHostName, reqVoteArgs.currentTerm);
        goto done;
    }

    if (reqVoteArgs.voteGranted != 0)
    {
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
           "_VmDirRequestVoteRpc: vote denied from peer %s role %d term %d granted-code %d",
          pPeerHostName, gRaftState.role, gRaftState.currentTerm, reqVoteArgs.voteGranted);
        gRaftState.voteDeniedCnt++;
        if (reqVoteArgs.voteGranted == 2)
        {
            //Peer has a larger highest logIndex, switch to follower,
            // so don't send request vote anymore for this term.
            gRaftState.role = VDIR_RAFT_ROLE_FOLLOWER;
            gRaftState.lastPingRecvTime = VmDirGetTimeInMilliSec();
        }
        //Denied due to split vote or any other reasons - stay as a candidate.
        goto done;
    }

    //Now vote is granted by the peer
    if (gRaftState.currentTerm == gRaftState.voteConsensusTerm)
    {
        gRaftState.voteConsensusCnt++;
        if (gRaftState.voteConsensusCnt >= (gRaftState.clusterSize/2 + 1))
        {
            //Got majority of votes, wake up _VmDirRaftVoteSchdThread which will change role to leader.
            waitSignaled = TRUE;
            VmDirConditionSignal(gGotVoteResultCond);

            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
              "_VmDirRequestVoteRpc: vote granted from %s and reached majority consensusCount %d for term %d; become leader in term %d",
              pPeerHostName, gRaftState.voteConsensusCnt, gRaftState.voteConsensusTerm, gRaftState.currentTerm);
            goto done;
        }
    }

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirRequestVoteRpc: vote granted from peer %s voteConsensusCnt %d (term %d)",
                   pPeerHostName, gRaftState.voteConsensusCnt, gRaftState.voteConsensusTerm);

done:
    if (!waitSignaled && (gRaftState.voteConsensusCnt - 1 + gRaftState.voteDeniedCnt) >= _VmDirPeersConnectedInLock())
    {
        //Also wakeup vote _VmDirRaftVoteSchdThread if all available peers received votes.
        VmDirConditionSignal(gGotVoteResultCond);
    }
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
    return 0;
}

static
DWORD
_VmDirAppendEntriesRpc(PVMDIR_SERVER_CONTEXT *ppServer, PVMDIR_PEER_PROXY pProxySelf, int cmd)
{
    DWORD dwError = 0;
    APPEND_ENTRIES_ARGS args = {0};
    PCSTR pPeerHostName = pProxySelf->raftPeerHostname;
    PVMDIR_SERVER_CONTEXT pServer = *ppServer;
    BOOLEAN bLock = FALSE;
    unsigned long long startLogIndex = 0;
    VDIR_RAFT_LOG curChgLog = {0};
    VDIR_RAFT_LOG preChgLog = {0};

    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
    if (gRaftState.role != VDIR_RAFT_ROLE_LEADER)
    {
        goto cleanup;
    }

    args.leader = gRaftState.hostname.lberbv_val;
    if (cmd == ExecAppendEntries)
    {
        if (gEntries == NULL || gEntries->packRaftLog.lberbv_len == 0)
        {
            //caller has given up and removed gEntries.
            VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL,
              "_VmDirAppendEntriesRpc: ExecAppendEntries gEntry==%s, peer %s, term %d, lastLogIndex %llu",
              gEntries?"notNull":"null", pPeerHostName, gRaftState.currentTerm, gRaftState.lastLogIndex);
            goto cleanup;
        }
        args.entriesSize = gEntries->packRaftLog.lberbv_len;
        dwError = VmDirAllocateAndCopyMemory(gEntries->packRaftLog.lberbv_val, args.entriesSize, (PVOID*)&args.entries);
        BAIL_ON_VMDIR_ERROR(dwError);

        startLogIndex = gEntries->index;
        VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
        dwError = _VmDirGetPrevLogArgs(&args.preLogIndex, &args.preLogTerm, gEntries->index - 1, __LINE__);
        BAIL_ON_VMDIR_ERROR(dwError);
    } else if (cmd == ExecPing)
    {
        //This is a Ping
        args.entriesSize = 0;
        args.entries = NULL;
        startLogIndex = gRaftState.lastLogIndex;
        VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
        dwError = _VmDirGetPrevLogArgs(&args.preLogIndex, &args.preLogTerm, startLogIndex+1, __LINE__);
        BAIL_ON_VMDIR_ERROR(dwError);
    } else
    {
         VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
           "_VmDirAppendEntriesRpc invalid cmd %d peer (%s), preLogIndex %llu, firstLogIndex %llu",
           cmd, pPeerHostName, args.preLogIndex, gRaftState.firstLogIndex);
         assert(0);
    }

ReplicateLog:
    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
    args.term = gRaftState.currentTerm;
    args.leaderCommit = gRaftState.commitIndex;

    if (gRaftState.role != VDIR_RAFT_ROLE_LEADER)
    {
        //Check the role again to present sending outdated RPC calls.
       VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
         "_VmDirAppendEntriesRpc no longer a leader (no RPC sent), peer %s preLog (%llu %d) role %d",
         pPeerHostName, args.preLogIndex,args.term, gRaftState.role);
       goto cleanup;
    }

    if (args.preLogIndex < gRaftState.firstLogIndex)
    {
       dwError = LDAP_OPERATIONS_ERROR;
       VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                       "_VmDirAppendEntriesRpc error: no log exists for follower (%s), preLogIndex %llu, firstLogIndex %llu",
                       pPeerHostName, args.preLogIndex, gRaftState.firstLogIndex);
       BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL,
      "_VmDirAppendEntriesRpc: startLogIdx %llu term %u preLogIdx %llu preLogTerm %u entSize %u peer %s",
      startLogIndex, args.term, args.preLogIndex, args.preLogTerm, args.entriesSize, pPeerHostName);

    dwError = VmDirRaftAppendEntries(pServer, args.term, args.leader,
                                  (UINT32)args.preLogIndex, args.preLogTerm,
                                   args.leaderCommit,
                                   args.entriesSize, args.entries,
                                   &args.currentTerm, &args.status);
    if (dwError)
    {
        if (dwError == rpc_s_connect_rejected || dwError == rpc_s_connect_timed_out ||
            dwError == rpc_s_cannot_connect || dwError == rpc_s_connection_closed ||
            dwError == rpc_s_host_unreachable || pServer == NULL)
        {
            pProxySelf->proxy_state = RPC_DISCONN;
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
              "_VmDirAppendEntriesRpc: not connected or disconnected peer %s dcerpc error %d", pPeerHostName, dwError);
            _VmDirRpcConnect(ppServer, pProxySelf);
        } else if (dwError == rpc_s_auth_method)
        {
            pProxySelf->proxy_state = RPC_DISCONN;
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirAppendEntriesRpc: rpc_s_auth_method peer %s", pPeerHostName);
            _VmDirRpcConnect(ppServer, pProxySelf);
        } else if (dwError == VMDIR_ERROR_UNWILLING_TO_PERFORM)
        {
            //Peer may be in process of starting up
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
              "_VmDirAppendEntriesRpc: peer %s rejected, peer term %d term %d priTerm %d error %d",
              pPeerHostName, args.currentTerm, gRaftState.currentTerm, args.term, dwError);
            VmDirSleep(1000);
        } else
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
              "_VmDirAppendEntriesRpc: cmd %d peer %s commitIdx %llu leaderComit %llu startLogIdx %llu preLogIdx %llu term %d error %d",
              cmd, pPeerHostName, gRaftState.commitIndex, args.leaderCommit, startLogIndex, args.preLogIndex, gRaftState.currentTerm, dwError);
        }
        goto cleanup;
    }

    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);

    if (args.currentTerm > gRaftState.currentTerm)
    {
        //Remote has higher term, swtich to follower.
        int oldTerm = 0;
        gRaftState.role = VDIR_RAFT_ROLE_FOLLOWER;
        gRaftState.lastPingRecvTime = VmDirGetTimeInMilliSec(); //Set for request vote timeout.
        oldTerm = gRaftState.currentTerm;
        gRaftState.currentTerm = args.currentTerm;

        //wakeup the waiting thread who is holding MDB write lock, so that persisting term can go through.
        //The waken up thread will evaluate the server's role and abort that transaction.
        VmDirConditionSignal(gRaftAppendEntryReachConsensusCond);
        VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

        _VmDirPersistTerm(args.currentTerm);

        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
          "_VmDirAppendEntriesRpc: peer (%s) term %d > current term %d, change role to follower",
          pPeerHostName, args.currentTerm, oldTerm);

        goto cleanup;
    }

    if (gRaftState.role != VDIR_RAFT_ROLE_LEADER)
    {
        //Other RPC calls or events changed the server's role, forfeit the current RPC call result.
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
          "_VmDirAppendEntriesRpc: server role changed after RPC call; role %d term %d cmd %d; peer: (%s) term %d",
          gRaftState.role, gRaftState.currentTerm, gRaftState.cmd, pPeerHostName, args.currentTerm);
        goto cleanup;
    }

    if (args.status != 0)
    {
        unsigned long long peerLogIndexToFetch = 0;
        //Remote doesn't contain log with preLogIndex, try a lower preLogIndex
        VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

        //Fetch the prev log downward
        VMDIR_SAFE_FREE_MEMORY(args.entries);
        args.entries = NULL;
        args.entriesSize = 0;

        _VmDirChgLogFree(&preChgLog);

        //When status is not 0, it passes the peer's last log index, we can fetch logs
        //  backward from there (plus a margin) to save time for a much lagged follower.
        if (args.preLogIndex > (args.status + RAFT_PREVLOG_FETCH_MARGIN))
        {
            peerLogIndexToFetch = args.status + RAFT_PREVLOG_FETCH_MARGIN;
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
              "_VmDirAppendEntriesRpc big logindex gap at follwer %s: (%llu %llu), fetch backward from %llu",
              pPeerHostName, args.preLogIndex, args.status, peerLogIndexToFetch);
        }
        else
        {
           peerLogIndexToFetch = args.preLogIndex;
        }

        dwError = _VmDirFetchLogEntry(peerLogIndexToFetch, &preChgLog, __LINE__);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (preChgLog.index == 0)
        {
            dwError = LDAP_OPERATIONS_ERROR;
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirAppendEntriesRpc invalid local server missing prev logIndex %llu error %d",
                            args.preLogIndex, dwError);
        }

        args.entriesSize = preChgLog.packRaftLog.lberbv_len;
        dwError = VmDirAllocateAndCopyMemory(preChgLog.packRaftLog.lberbv_val, args.entriesSize, (PVOID*)&args.entries);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _VmDirGetPrevLogArgs(&args.preLogIndex, &args.preLogTerm, preChgLog.index - 1, __LINE__);
        BAIL_ON_VMDIR_ERROR(dwError);

        goto ReplicateLog;
    }

    //The peer has confirmed the matching preLogIndex.
    if (args.entries)
    {
        _VmDirChgLogFree(&curChgLog);
        curChgLog.packRaftLog.lberbv_len = args.entriesSize;
        dwError = VmDirAllocateAndCopyMemory(args.entries, args.entriesSize, (PVOID*)&curChgLog.packRaftLog.lberbv_val);
        BAIL_ON_VMDIR_ERROR(dwError);
        curChgLog.packRaftLog.bOwnBvVal = TRUE;

        dwError = _VmDirUnpackLogEntry(&curChgLog);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if(curChgLog.index > pProxySelf->matchIndex)
    {
        pProxySelf->matchIndex = curChgLog.index;
    }

    if (args.preLogIndex > pProxySelf->matchIndex)
    {
        pProxySelf->matchIndex = args.preLogIndex;
    }

    if (cmd == ExecAppendEntries)
    {
        if (curChgLog.index > 0 && gEntries && curChgLog.index == gEntries->index)
        {
            //This is AppendEntries with uncommitted log in args.entries. Now the gap has closed or no gap.
            int consensusCnt = 0;

            pProxySelf->bLogReplicated = TRUE;

            consensusCnt = _VmDirGetAppendEntriesConsensusCountInLock();
            if (consensusCnt >= (gRaftState.clusterSize/2 + 1))
            {
                 VmDirConditionSignal(gRaftAppendEntryReachConsensusCond);
            }
            VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "_VmDirAppendEntriesRpc: got consent from %s current logIdx %llu term %d",
                            pPeerHostName, gEntries->index, gRaftState.currentTerm);
            goto cleanup;
        }
    } else
    {
        //This is a ping
        if (args.preLogIndex == startLogIndex)
        {
            /* Now the peer is now in sycn. */
            VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL,
                       "_VmDirAppendEntriesRpc: peer %s in sync or closed gap with starting logIndex %llu term %d",
                       pPeerHostName, startLogIndex, gRaftState.currentTerm);
            goto cleanup;
        }
    }

    if (args.preLogIndex >= startLogIndex)
    {
        goto cleanup;
    }

    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL,
      "_VmDirAppendEntriesRpc: need to catchup logs for peer %s StartLogIndex %llu preLogIndex %llu entries %s",
      pPeerHostName, startLogIndex, args.preLogIndex, args.entries?"not null":"null");

    /*
     * For Ping or AppendEntries with gap exists.
     * Will fetch and replicate logs upward until gap closed.
     */

    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

    _VmDirChgLogFree(&preChgLog);

    dwError = _VmDirGetNextLog(args.preLogIndex + 1, startLogIndex, &preChgLog,  __LINE__);
    if (preChgLog.index == 0)
    {
       if (cmd == ExecAppendEntries)
       {
           //Log with startLogIndex is uncommitted
           goto cleanup;
       }

       dwError =  LDAP_OPERATIONS_ERROR;
       VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
         "_VmDirAppendEntriesRpc: fail to close gap for peer %s StartLogIdx %llu preLogIdx %llu cmd %d",
         pPeerHostName, startLogIndex, args.preLogIndex, cmd);
       goto error;
    }

    args.preLogIndex = preChgLog.index;
    args.preLogTerm = preChgLog.term;

    VMDIR_SAFE_FREE_MEMORY(args.entries);
    args.entries = NULL;
    args.entriesSize = 0;

    _VmDirChgLogFree(&curChgLog);
    dwError = _VmDirGetNextLog(args.preLogIndex+1, startLogIndex, &curChgLog,  __LINE__);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (curChgLog.index == 0)
    {
        //Reached startLogIndex which has an un-committed log or ping with gap closed.
        goto ReplicateLog;
    }

    //Gap has yet to close.
    args.entriesSize = curChgLog.packRaftLog.lberbv_len;
    dwError = VmDirAllocateAndCopyMemory(curChgLog.packRaftLog.lberbv_val, args.entriesSize, (PVOID*)&args.entries);
    BAIL_ON_VMDIR_ERROR(dwError);

    goto ReplicateLog;

cleanup:
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
    VMDIR_SAFE_FREE_MEMORY(args.entries);
    args.entries = NULL;
    args.entriesSize = 0;
    _VmDirChgLogFree(&preChgLog);
    _VmDirChgLogFree(&curChgLog);

    if (dwError == 0)
    {
        VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL,
          "_VmDirAppendEntriesRpc: rpc call (cmd %d) completed for peer %s; startLogIdx %llu term %d",
          cmd, pPeerHostName, startLogIndex, gRaftState.currentTerm);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
      "_VmDirAppendEntriesRpc: error rpc call (cmd %d) peer %s, startLogIdx %llu term %d error %d",
      cmd, pPeerHostName, startLogIndex, gRaftState.currentTerm, dwError);
    goto cleanup;
}

//Search internally to get all peer computer hosts and start the peer threads.
static
DWORD
_VmDirStartProxies(
    VOID
)
{
    DWORD dwError = ERROR_SUCCESS;
    VDIR_BERVALUE dcContainerDN = VDIR_BERVALUE_INIT;
    PSTR pHostname = NULL;
    PSTR pszName = NULL;
    VDIR_BERVALUE dcContainerDNrdn = VDIR_BERVALUE_INIT;
    VDIR_ENTRY_ARRAY entryArray = {0};
    int i = 0;
    BOOLEAN bLock = FALSE;

    VmDirGetParentDN(&(gVmdirServerGlobals.dcAccountDN), &dcContainerDN);
    if (dcContainerDN.lberbv.bv_len == 0)
    {
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    dwError = VmDirSimpleEqualFilterInternalSearch(dcContainerDN.lberbv.bv_val,
                    LDAP_SCOPE_ONE, ATTR_OBJECT_CLASS, OC_COMPUTER, &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
    gRaftState.clusterSize = 1;
    for (i = 0; i < entryArray.iSize; i++)
    {
        dwError = VmDirNormalizeDNWrapper( &(entryArray.pEntry[i].dn));
        BAIL_ON_VMDIR_ERROR(dwError);
        if (VmDirStringCompareA(entryArray.pEntry[i].dn.bvnorm_val,
                                gVmdirServerGlobals.dcAccountDN.bvnorm_val, FALSE) == 0)
        {
            //The server is self.
            continue;
        }
        VmDirFreeBervalContent(&dcContainerDNrdn);
        dwError = VmDirGetRdn(&entryArray.pEntry[i].dn, &dcContainerDNrdn);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_SAFE_FREE_STRINGA(pHostname);
        VMDIR_SAFE_FREE_STRINGA(pszName);

        dwError = VmDirRdnToNameValue(&dcContainerDNrdn, &pszName, &pHostname);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _VmDirNewPeerProxyInLock(pHostname, RPC_DISCONN);
        BAIL_ON_VMDIR_ERROR(dwError);

        gRaftState.clusterSize++;
    }

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirStartProxies: raft cluster size %d", gRaftState.clusterSize);

cleanup:
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
    VmDirFreeEntryArrayContent(&entryArray);
    VmDirFreeBervalContent(&dcContainerDN);
    VmDirFreeBervalContent(&dcContainerDNrdn);
    VMDIR_SAFE_FREE_STRINGA(pHostname);
    VMDIR_SAFE_FREE_STRINGA(pszName);
    return dwError;

error:
    goto cleanup;
}

DWORD
_VmDirRequestVoteGetReply(UINT32 term, char *candidateId, unsigned long long lastLogIndex,
    UINT32 lastLogTerm, UINT32 *currentTerm, UINT32 *voteGranted)
{
    DWORD dwError = 0;
    UINT32 iVoteGranted = 0;
    BOOLEAN bLock = FALSE;
    BOOLEAN bLockRpcReply = FALSE;
    int oldTerm = 0;
    int newTerm = 0;
    VDIR_BERVALUE bvVotedFor = VDIR_BERVALUE_INIT;

    *voteGranted = iVoteGranted = 1; //Default to denied with reason split vote or other than larger highest logIndex of mine.

    if (!gRaftState.initialized)
    {
        //Don't participate in vote if this server has not been initialized.
        VmDirSleep(gVmdirGlobals.dwRaftPingIntervalMS); //Pause a little so that peers won't waste term sequence numbers.
        dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_LOCK_MUTEX(bLockRpcReply, gRaftRpcReplyMutex);
    //Serialize appendEntriesRpc and requestVoteRpc handlers

    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
    oldTerm = gRaftState.currentTerm;

    if (term < gRaftState.currentTerm || gRaftState.lastLogTerm > lastLogTerm ||
        (gRaftState.lastLogTerm == lastLogTerm && gRaftState.lastLogIndex > lastLogIndex) ||
        (gRaftState.role == VDIR_RAFT_ROLE_LEADER && term == gRaftState.currentTerm))
    {
	//My term is larger than the requester, or my highest log/term are larger, then deny the vote.
        if (gRaftState.lastLogTerm > lastLogTerm ||
            (gRaftState.lastLogTerm == lastLogTerm && gRaftState.lastLogIndex > lastLogIndex))
        {
            *voteGranted = iVoteGranted = 2;
        }
        goto cleanup;
    }

    if (gRaftState.votedFor.lberbv_len > 0 && gRaftState.votedForTerm == term &&
        VmDirStringCompareA(gRaftState.votedFor.lberbv_val, candidateId, FALSE) != 0)
    {
        //I have voted for (granted to) a different requester in the same term, deny the vote request.
        goto cleanup;
    }

    //Grant the vote.
    iVoteGranted = 0;

    //Remeber (persist) the candidate I have voted for and the associated term.
    if (gRaftState.votedFor.lberbv_len > 0)
    {
        VmDirFreeBervalContent(&gRaftState.votedFor);
    }

    dwError = VmDirAllocateBerValueAVsnprintf(&gRaftState.votedFor, "%s", candidateId);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirBervalContentDup(&gRaftState.votedFor, &bvVotedFor);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (term > gRaftState.currentTerm)
    {
        //Peer has higher term, switch to follower if not yet.
        gRaftState.currentTerm = term;
        gRaftState.role = VDIR_RAFT_ROLE_FOLLOWER;
        if (iVoteGranted == 2)
        {
            //Force an election timeout.
            gRaftState.lastPingRecvTime = 0;
        } else
        {
            gRaftState.lastPingRecvTime = VmDirGetTimeInMilliSec(); //Set for request vote timeout.
        }
    }
    if (iVoteGranted == 0)
    {
        gRaftState.votedForTerm = gRaftState.currentTerm;
        *voteGranted = 0;
    }
    *currentTerm = newTerm = gRaftState.currentTerm;
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
    VMDIR_UNLOCK_MUTEX(bLockRpcReply, gRaftRpcReplyMutex);

    if (!dwError && newTerm > oldTerm)
    {
        _VmDirPersistTerm(newTerm);
    }

    if (!dwError)
    {
      VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
        "_VmDirRequestVoteGetReply: granted=%d candidateId %s term %d lastLogTerm %d; server term %d (old term %d) role %d votedForTerm %d votedFor %s",
        iVoteGranted, candidateId, term, lastLogTerm, gRaftState.currentTerm, oldTerm, gRaftState.role,
        gRaftState.votedForTerm, VDIR_SAFE_STRING(bvVotedFor.lberbv_val));
    }
    VmDirFreeBervalContent(&bvVotedFor);
    return dwError;

error:
    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
      "_VmDirRequestVoteGetReply: candidateId %s term %d lastLogTerm %d; server term %d (old term %d) role %d error=%d",
      candidateId, term, lastLogTerm, gRaftState.currentTerm, oldTerm, gRaftState.role, dwError);
    goto cleanup;
}

DWORD
VmDirAppendEntriesGetReply(
    UINT32 term,
    char *leader,
    unsigned long long preLogIndex,
    UINT32 preLogTerm,
    unsigned long long leaderCommit,
    int entrySize,
    char *entries,
    UINT32 *currentTerm,
    unsigned long long *status
    )
{
    DWORD dwError = 0;
    BOOLEAN bLock = FALSE;
    BOOLEAN bLockRpcReply = FALSE;
    int oldTerm = 0;
    int newTerm = 0;
    BOOLEAN bLogFound = FALSE;
    BOOLEAN bTermMatch = FALSE;
    VDIR_RAFT_LOG chgLog = {0};
    static int logCnt = 0;
    static time_t prevLogTime = {0};
    time_t now = {0};
    BOOLEAN bLeaderChanged = FALSE;
    BOOLEAN bFatalError = FALSE;
    unsigned long long priCommitIndex = 0;
    unsigned long long lastLogIndex = 0;

    *status = 0;
    *currentTerm = 0;

    if (!gRaftState.initialized || !_VmDirRaftPeerIsReady(leader))
    {
        //Don't try to replicate if this server is not initialized or the peer thread is not ready.
        dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_LOCK_MUTEX(bLockRpcReply, gRaftRpcReplyMutex);
    //Serialize appendEntriesRpc, requestVoteRpc handlers and _VmDirEvaluateVoteResult

    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);

    if (gRaftState.leader.lberbv.bv_len == 0 ||
        VmDirStringCompareA(gRaftState.leader.lberbv.bv_val, leader, FALSE) !=0 )
    {
        bLeaderChanged = TRUE;
    }

    priCommitIndex = gRaftState.commitIndex;

    if (bLeaderChanged)
    {
        VmDirFreeBervalContent(&gRaftState.leader);
        dwError = VmDirAllocateBerValueAVsnprintf(&gRaftState.leader, "%s", leader);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    oldTerm = gRaftState.currentTerm;

    if (gRaftState.currentTerm > term)
    {
        //Tell remote to switch to follower
        *currentTerm = gRaftState.currentTerm;
        *status = lastLogIndex;
        goto cleanup;
    }

    if (gRaftState.role != VDIR_RAFT_ROLE_FOLLOWER &&
        gRaftState.currentTerm < term)
    {
        //I am not a follower yet and my term is smaller than peer's term,
        //switch to follower, and let peer send a fresh appendEntries.
        *currentTerm = newTerm = gRaftState.currentTerm = term;
        gRaftState.role = VDIR_RAFT_ROLE_FOLLOWER;
        gRaftState.lastPingRecvTime = VmDirGetTimeInMilliSec();
        dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    //peer's term == my term, keep as a follower or switch to follower if not
    gRaftState.role = VDIR_RAFT_ROLE_FOLLOWER;
    gRaftState.lastPingRecvTime = VmDirGetTimeInMilliSec();
    *currentTerm = newTerm = gRaftState.currentTerm = term;
    lastLogIndex = gRaftState.lastLogIndex;
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

    if (preLogIndex == 0)
    {
        //No any log yet, considered a match
        bLogFound = bTermMatch = TRUE;
    } else
    {
        dwError = _VmDirLogLookup(preLogIndex, preLogTerm, &bLogFound, &bTermMatch);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!bLogFound || !bTermMatch)
    {
        //Tell remote to decrement preLogIndexand for an older logEntry, starting from lastLogIndex.
        *status = lastLogIndex;
        goto cleanup;
    }

    //Now preLogIndex found locally and has a macthing term - delete all logs > preLogIndex,
    //  those logs are uncommitted, and were replicated from old leaders.
    dwError = _VmDirDeleteAllLogs(preLogIndex+1, &bFatalError);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (entrySize > 0)
    {
        chgLog.packRaftLog.lberbv_len = entrySize;
        dwError = VmDirAllocateAndCopyMemory(entries, entrySize, (PVOID*)&chgLog.packRaftLog.lberbv_val);
        BAIL_ON_VMDIR_ERROR(dwError);
        chgLog.packRaftLog.bOwnBvVal = TRUE;

        dwError = _VmDirUnpackLogEntry(&chgLog);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _VmDirPersistLog(&chgLog);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
        gRaftState.lastLogIndex = chgLog.index;
        gRaftState.lastLogTerm = chgLog.term;
        gRaftState.indexToApply = gRaftState.commitIndex = VMDIR_MIN(leaderCommit, preLogIndex);
        gRaftState.opCounts++;
        VmDirConditionSignal(gRaftNewLogCond);
        VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
    } else
    {
        VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
        gRaftState.indexToApply = gRaftState.commitIndex = VMDIR_MIN(leaderCommit, preLogIndex);
        VmDirConditionSignal(gRaftNewLogCond);
        VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
    }

    *status = 0;

cleanup:
    if (dwError == 0 && logCnt++ % 10 == 0)
    {
        now = time(&now);
        if ((now - prevLogTime) > 30)
        {
            prevLogTime = now;
            //Log once every 30 seconds, over 10 calls
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
              "VmDirAppendEntriesGetReply: entSize %d leader %s term %d leaderCommit %llu preLogIdx %llu preLogTerm %d commitIdx %llu priCommitIdx %llu currentTerm %d oldTerm %d role %d status %llu",
              entrySize, leader, term, leaderCommit, preLogIndex, preLogTerm,
              gRaftState.commitIndex, priCommitIndex, *currentTerm, oldTerm,
              gRaftState.role, *status);
        }
    }
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
    VMDIR_UNLOCK_MUTEX(bLockRpcReply, gRaftRpcReplyMutex);

    if (newTerm > oldTerm)
    {
        _VmDirPersistTerm(newTerm);
    }

    _VmDirChgLogFree(&chgLog);

    //Raft inconsistency may occur if fatal error detected.
    assert(!bFatalError);

    return dwError;

error:
     VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
              "VmDirAppendEntriesGetReply: entSize %d leader %s term %d leaderCommit %llu preLogIdx %llu preLogTerm %d commitIdx %llu priCommitIdx %llu currentTerm %d oldTerm %d role %d error %d",
              entrySize, leader, term, leaderCommit, preLogIndex, preLogTerm,
              gRaftState.commitIndex, priCommitIndex, *currentTerm, oldTerm,
              gRaftState.role, dwError);
    goto cleanup;
}

/*  The following three callbacks are (conditionally, sequentially) invoked by MDB txn_commit to
 *  ensure protocol safety due to a modification from the original Raft algorithm, i.e. it tries
 *  to commit LDAP operation, log entry and the persistent state changes in the same MDB transaction at
 *  Raft leader instead of persisting the Raft log at the leader first before obtaining consensus from peers.
 *
 *  This callback is invoted first by MDB txn_commit when the LDAP transaction is ready to commit after
 *  the log entry is created. When this function returns 0 (after consensus has reached or a standalone server),
 *  txn_commit will proceed with its commit process. Or this function may return non-zero to explicitly abort
 *  the local transaction which is only allowed when the server is no longer a Raft leader.
 *
 *  MDB callback VmDirRaftPostCommit would be invoked for the same transaction when this function return 0, and
 *  after the transaction has successfully committed (peristed) locally, which would update commitIndex/lastApplied.
 *
 *  In a rare case (when disk is full), the current transaction is implicitly aborted within txn_commit, and
 *  MDB callback VmDirRaftCommitFail will be invoked (though this function returned 0) which would put
 *  the server to Follower role to avoid the same log index/term being reused.
 *
 *  See the functional spec section 3.3.5.2 for detail.
 */
int VmDirRaftPrepareCommit(void **ppCtx)
{
    int dwError = 0;
    BOOLEAN bLock = FALSE;
    unsigned int currentTerm = 0;
    int getConsensusRetry = 0;
    int changeToFollower = 0;
    DWORD waitTimeout = 0;
    PVDIR_RAFT_COMMIT_CTX pCtx = NULL;

    *ppCtx = NULL;
    if (gLogEntry.index == 0)
    {
       /*
        * This transaciton has no log entry to replicate:
        * 1. If the server is a leader, then it may need to update Raft PS state.
        *    in such case, just commit it locally.
        * 2. If the server is a candidate or follower, always allow it to commit transaction
        *    for Raft state and local log entry.
        */
        goto raft_commit_done;
    }

    /* Use very large timeout value for new leader replicating no-op log entry so that a far
     * behind peer can catch up with the leader if that peer is needed for reaching consensus.
     */
    waitTimeout = gLogEntry.requestCode == 0?LARGE_TIMEOUT_VALUE_MS:gVmdirGlobals.dwRaftElectionTimeoutMS;

    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
    if (_VmDirGetRaftQuorumOverride(FALSE) || gRaftState.clusterSize < 2)
    {
        //This is a standalone server or QuorumOverride is set
        goto raft_commit_done;
    }

get_consensus_begin:
    do
    {
        dwError = 0;
        if (gRaftState.role != VDIR_RAFT_ROLE_LEADER || VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            /* VmDirRaftPrepareCommit can abort a user transaciton only when it is no longer a leader
             * or the server is to be shutdown to ensure that the longIndex used by the aborted transaction
             * can never be reused.
             */
            dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (_VmDirPeersIdleInLock() < (gRaftState.clusterSize/2))
        {
            //Fewer than half of peer threads are idle
            if (getConsensusRetry >= 2)
            {
                /* Cannot get half of peer threads to service in twice of election timeout.
                 * Switch to follower to prevent reusing raft the same logindex/term
                 */
                gRaftState.role = VDIR_RAFT_ROLE_FOLLOWER;
                dwError = LDAP_OPERATIONS_ERROR;
                changeToFollower = 1;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            getConsensusRetry++;

            //Wait if not enough peer threads are Ready to accept RPC request.
            dwError = VmDirConditionTimedWait(gPeersReadyCond, gRaftStateMutex, waitTimeout);
        }
    } while (dwError == ETIMEDOUT);

    if (dwError)
    {
        gRaftState.role = VDIR_RAFT_ROLE_FOLLOWER;
        dwError = LDAP_OPERATIONS_ERROR;
        changeToFollower = 2;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (gRaftState.role != VDIR_RAFT_ROLE_LEADER || VmDirdState() == VMDIRD_STATE_SHUTDOWN)
    {
        //Check again since waiting gPeersReadyCond may take time.
        dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    //Now invoke paralle RPC calls to all (available) peers
    VmDirConditionBroadcast(gRaftRequestPendingCond);

    gRaftState.cmd = ExecAppendEntries;
    //gEntries is accessed by proxy threads.
    gEntries = &gLogEntry;
    currentTerm = gRaftState.currentTerm;
    _VmDirClearProxyLogReplicatedInLock();

    //Wait for majority peers to replicate the log.
    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL,
      "VmDirRaftPrepareCommit: wait gRaftAppendEntryReachConsensusCond; role %d term %d",
      gRaftState.role, gRaftState.currentTerm);

    VmDirConditionTimedWait(gRaftAppendEntryReachConsensusCond, gRaftStateMutex, waitTimeout);

    if (gRaftState.role != VDIR_RAFT_ROLE_LEADER ||
        gRaftState.currentTerm != gLogEntry.term ||
        VmDirdState() == VMDIRD_STATE_SHUTDOWN)
    {
        //Check again since the AppendEntryRpc may change role.
        //The leader only tries to count consensus on log with the current term
        dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (_VmDirGetAppendEntriesConsensusCountInLock() < (gRaftState.clusterSize/2 + 1))
    {
        //Check ConsensusCount again
        VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL,
          "VmDirRaftPrepareCommit: no consensus reached on log entry: logIndex %lu role %d term %d, will retry",
          gLogEntry.index, gRaftState.role, gRaftState.currentTerm);

        if (getConsensusRetry >= 2)
        {
            /* Cannot get consensus in twice of election timeout.
             * Switch to follower to prevent reusing raft the same logindex/term
             */
            gRaftState.role = VDIR_RAFT_ROLE_FOLLOWER;
            dwError = LDAP_OPERATIONS_ERROR;
            changeToFollower = 3;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        getConsensusRetry++;
        goto get_consensus_begin;
    }

raft_commit_done:
    //The log entry can be committed locally,
    gRaftState.opCounts++;
    if (gLogEntry.index > 0)
    {
        dwError = VmDirAllocateMemory(sizeof(VDIR_RAFT_COMMIT_CTX), (PVOID*)&pCtx);
        BAIL_ON_VMDIR_ERROR(dwError);

        pCtx->logIndex = gLogEntry.index;
        pCtx->logTerm = gLogEntry.term;
        pCtx->logRequestCode = gLogEntry.requestCode;
        *ppCtx = (void *)pCtx;

        //either VmDirRaftPostCommit or VmDirRaftCommitFail (but not both)
        //  owns pCtx who will free the memory and unlock gRaftStateMutex
        VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL,
          "VmDirRaftPrepareCommit: succeeded; server role %d term %d lastApplied %llu",
          gRaftState.role, gRaftState.currentTerm, gRaftState.lastApplied);
    } else
    {
        VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
    }

cleanup:
    _VmDirChgLogFree(&gLogEntry);
    gEntries = NULL;
    if (dwError==0)
    {
      VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL,
        "VmDirRaftPrepareCommit: succeeded; server role %d term %d lastApplied %llu",
        gRaftState.role, gRaftState.currentTerm, gRaftState.lastApplied);
    }
    return dwError;

error:
    gRaftState.cmd = ExecNone;
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
      "VmDirRaftPrepareCommit: logEntry index %llu role %d term %d(%d) lastLogIndex %llu changeToFollower %d error %d",
      gLogEntry.index, gRaftState.role, gRaftState.currentTerm, currentTerm,
      gRaftState.lastLogIndex, changeToFollower, dwError);
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
    goto cleanup;
}

/* This is the callback invoted by MDB txn_commit when it has successfully
 * committed (persisted) the transaction, and the callback can safely set
 * its volatile state variablies
 */
VOID
VmDirRaftPostCommit(void *ctx)
{
    PVDIR_RAFT_COMMIT_CTX pCtx = NULL;

    if (ctx)
    {
        BOOLEAN bLock = TRUE;

        pCtx = (PVDIR_RAFT_COMMIT_CTX)ctx;

        gRaftState.commitIndex = gRaftState.lastLogIndex = pCtx->logIndex;
        gRaftState.commitIndexTerm = gRaftState.lastLogTerm = pCtx->logTerm;
        if (pCtx->logRequestCode != 0)
        {
            //not non-op
            gRaftState.lastApplied = pCtx->logIndex;
        }
        gRaftState.cmd = ExecNone;
        VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "VmDirRaftPostCommit: log (%llu %d) lastApplied %llu logOp %d",
                        pCtx->logIndex, pCtx->logTerm, gRaftState.lastApplied, pCtx->logRequestCode);
        VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
        VMDIR_SAFE_FREE_MEMORY(ctx);
    }
}

/* This is the callback invoted by MDB txn_commit when it has failed to
 * persist the transaction when trying to write WAL or MDB meta page
 * (due to disk full/failure). It prevents the server from resuing logIndex/logTerm
 * for new client request.
 */
VOID
VmDirRaftCommitFail(void *ctx)
{
    if (ctx)
    {
        BOOLEAN bLock = TRUE;

        gRaftState.role = VDIR_RAFT_ROLE_FOLLOWER;
        gRaftState.lastPingRecvTime = VmDirGetTimeInMilliSec();
        gRaftState.cmd = ExecNone;
        VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
        VMDIR_SAFE_FREE_MEMORY(ctx);
    }
}

//This create chglog for the LDAP Add right becore calling pfnBETxnCommit, At this poiont,
//  all validations of the LDAP add have completed, and the all changes associated with
//  the LDAP Add (including indices creation) have been applied to the MDB backends
DWORD VmDirAddRaftPreCommit(PVDIR_ENTRY pEntry, PVDIR_OPERATION pAddOp)
{
    DWORD dwError = 0;
    char *p = NULL;
    BOOLEAN bLock = FALSE;
    VDIR_BERVALUE encodedEntry = VDIR_BERVALUE_INIT;

    if (pEntry->eId < NEW_ENTRY_EID_PREFIX)
    {
       //Don't create log for entry with system assigned eid.
       goto cleanup;
    }

    if ((p=VmDirStringCaseStrA(pEntry->dn.bvnorm_val, RAFT_CONTEXT_DN)) &&
         VmDirStringCompareA(p, RAFT_CONTEXT_DN, FALSE)==0)
    {
        //Don't create log entry for raft context entry.
        if (pEntry->eId >= NEW_ENTRY_EID_PREFIX)
        {
            //don't use raft assigned eid;
            pEntry->eId = 0;
        }
        goto cleanup;
    }

    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
    if (_VmDirGetRaftQuorumOverride(FALSE))
    {
        ;
    }
    else if (gRaftState.clusterSize >= 2 && gRaftState.role != VDIR_RAFT_ROLE_LEADER)
    {
        dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    gLogEntry.index = gRaftState.commitIndex + 1;
    gLogEntry.term = gRaftState.currentTerm;
    gLogEntry.entryId = pEntry->eId;
    gLogEntry.requestCode = LDAP_REQ_ADD;

    dwError = VmDirEncodeEntry( pEntry, &encodedEntry );
    BAIL_ON_VMDIR_ERROR(dwError);

    gLogEntry.chglog.lberbv_len = encodedEntry.lberbv.bv_len;
    gLogEntry.chglog.lberbv_val = encodedEntry.lberbv.bv_val;
    gLogEntry.chglog.bOwnBvVal = TRUE;

    dwError = _VmDirPackLogEntry(&gLogEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

    dwError = VmDirAddRaftEntry(pEntry->pSchemaCtx, &gLogEntry, pAddOp);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
    return dwError;

error:
    _VmDirChgLogFree(&gLogEntry);
    goto cleanup;
}

DWORD VmDirModifyRaftPreCommit(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    ENTRYID entryId,
    char *dn,
    PVDIR_MODIFICATION pmods,
    PVDIR_OPERATION pModifyOp)
{
    DWORD dwError = 0;
    char *p = NULL;
    BOOLEAN bLock = FALSE;
    VDIR_BERVALUE encodedMods = VDIR_BERVALUE_INIT;

    if ((p=VmDirStringCaseStrA(dn, RAFT_CONTEXT_DN)) &&
        VmDirStringCompareA(p, RAFT_CONTEXT_DN, FALSE)==0)
    {
        goto cleanup;
    }

    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
    if (_VmDirGetRaftQuorumOverride(FALSE))
    {
        ;
    }
    else if (gRaftState.clusterSize >= 2 && gRaftState.role != VDIR_RAFT_ROLE_LEADER)
    {
        dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirEncodeMods(pmods, &encodedMods);
    BAIL_ON_VMDIR_ERROR(dwError);

    gLogEntry.index = gRaftState.commitIndex + 1;
    gLogEntry.term = gRaftState.currentTerm;
    gLogEntry.entryId = entryId;
    gLogEntry.requestCode = LDAP_REQ_MODIFY;
    gLogEntry.chglog.lberbv_len = encodedMods.lberbv.bv_len;
    gLogEntry.chglog.lberbv_val = encodedMods.lberbv.bv_val;
    gLogEntry.chglog.bOwnBvVal = TRUE;
    encodedMods.lberbv.bv_val = NULL;
    encodedMods.lberbv.bv_len = 0;
    dwError = _VmDirPackLogEntry(&gLogEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

    dwError = VmDirAddRaftEntry(pSchemaCtx, &gLogEntry, pModifyOp);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
    return dwError;

error:
    _VmDirChgLogFree(&gLogEntry);
    VmDirFreeBervalContent(&encodedMods);
    goto cleanup;
}

DWORD VmDirDeleteRaftPreCommit(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    EntryId eid,
    char *dn,
    PVDIR_OPERATION pDeleteOp)
{
    DWORD dwError = 0;
    char *p = NULL;
    BOOLEAN bLock = FALSE;

    if ((p=VmDirStringCaseStrA(dn, RAFT_CONTEXT_DN)) &&
        VmDirStringCompareA(p, RAFT_CONTEXT_DN, FALSE)==0)
    {
        goto cleanup;
    }

    dwError = _VmDirDeleteRaftProxy(dn);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
    if (_VmDirGetRaftQuorumOverride(FALSE))
    {
        ;
    }
    else if (gRaftState.clusterSize >= 2 && gRaftState.role != VDIR_RAFT_ROLE_LEADER)
    {
        dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    gLogEntry.index = gRaftState.commitIndex + 1;
    gLogEntry.term = gRaftState.currentTerm;
    gLogEntry.entryId = eid;
    gLogEntry.requestCode = LDAP_REQ_DELETE;
    gLogEntry.chglog.lberbv_len = 0;
    gLogEntry.chglog.lberbv_val = NULL;
    dwError = _VmDirPackLogEntry(&gLogEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

    dwError = VmDirAddRaftEntry(pSchemaCtx, &gLogEntry, pDeleteOp);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
    return dwError;

error:
    _VmDirChgLogFree(&gLogEntry);
    goto cleanup;
}

DWORD
VmDirAddRaftProxy(PVDIR_ENTRY pEntry)
{
    DWORD dwError = 0;
    VDIR_BERVALUE peerRdn = VDIR_BERVALUE_INIT;
    VDIR_BERVALUE dcContainerDN = VDIR_BERVALUE_INIT;
    PSTR pHostname = NULL;
    PSTR  pszName = NULL;
    BOOLEAN bLock = FALSE;

    if (pEntry->dn.bvnorm_val == NULL ||
        gVmdirServerGlobals.dcAccountDN.bvnorm_val == NULL ||
        pEntry->pdn.bvnorm_len == 0 ||
        VmDirStringCompareA(pEntry->dn.bvnorm_val, gVmdirServerGlobals.dcAccountDN.bvnorm_val, FALSE) == 0)
    {
        //Don't add server as peer if this server has not been configured yet or the server is self
        goto cleanup;
    }

    VmDirGetParentDN(&(gVmdirServerGlobals.dcAccountDN), &dcContainerDN);
    if (dcContainerDN.lberbv.bv_len == 0)
    {
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    if (VmDirStringCompareA(pEntry->pdn.bvnorm_val, dcContainerDN.bvnorm_val, FALSE) != 0)
    {
       goto cleanup;
    }

    dwError = VmDirGetRdn(&pEntry->dn, &peerRdn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRdnToNameValue(&peerRdn, &pszName, &pHostname);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (VmDirStringCompareA(gRaftState.hostname.lberbv.bv_val, pHostname, FALSE)==0)
    {
        goto cleanup;
    }

    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
    dwError = _VmDirNewPeerProxyInLock(pHostname, PENDING_ADD);
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirFreeBervalContent(&dcContainerDN);
    VmDirFreeBervalContent(&peerRdn);
    VMDIR_SAFE_FREE_MEMORY(pHostname);
    VMDIR_SAFE_FREE_MEMORY(pszName);
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDirUpdateRaftLogChangedAttr(
    PVDIR_OPERATION pOperation,
    PVDIR_ENTRY     pEntry
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    UINT64  iLogIdxChgd = 0;
    PSTR    pszLogIdxChgd = NULL;
    VDIR_BERVALUE   bvLogIdxChgd = {0};

    if (!pOperation || !pEntry)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pOperation->opType == VDIR_OPERATION_TYPE_REPL ||
        (pEntry->eId &&
         (pEntry->eId < NEW_ENTRY_EID_PREFIX ||
          pEntry->eId >= LOG_ENTRY_EID_PREFIX)))
    {
        goto cleanup;
    }

    VMDIR_LOCK_MUTEX(bInLock, gRaftStateMutex);
    iLogIdxChgd = gRaftState.commitIndex + 1;
    VMDIR_UNLOCK_MUTEX(bInLock, gRaftStateMutex);

    dwError = VmDirAllocateStringPrintf(&pszLogIdxChgd, "%"PRIu64, iLogIdxChgd);
    BAIL_ON_VMDIR_ERROR(dwError);

    switch (pOperation->reqCode)
    {
    case LDAP_REQ_ADD:

        dwError = VmDirEntryAddSingleValueStrAttribute(
                pEntry, ATTR_RAFT_LOG_CHANGED, pszLogIdxChgd);
        BAIL_ON_VMDIR_ERROR(dwError);
        break;

    case LDAP_REQ_MODIFY:

        bvLogIdxChgd.lberbv.bv_val = pszLogIdxChgd;
        bvLogIdxChgd.lberbv.bv_len = VmDirStringLenA(pszLogIdxChgd);

        dwError = VmDirOperationAddModReq(
                pOperation,
                LDAP_MOD_REPLACE,
                ATTR_RAFT_LOG_CHANGED,
                &bvLogIdxChgd,
                1);
        BAIL_ON_VMDIR_ERROR(dwError);
        break;

    default:

        break;
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLogIdxChgd);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

static
DWORD
_VmDirDeleteRaftProxy(char *dn_norm)
{
    DWORD dwError = 0;
    VDIR_BERVALUE peerRdn = VDIR_BERVALUE_INIT;
    VDIR_BERVALUE dcContainerDN = VDIR_BERVALUE_INIT;
    VDIR_BERVALUE parentDN = VDIR_BERVALUE_INIT;
    VDIR_BERVALUE bvDN = VDIR_BERVALUE_INIT;
    PSTR pHostname = NULL;
    PSTR  pszName = NULL;
    BOOLEAN bLock = FALSE;

    if (dn_norm == NULL)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    bvDN.lberbv.bv_val = dn_norm;
    bvDN.lberbv.bv_len = VmDirStringLenA(dn_norm);

    dwError =  VmDirGetParentDN(&bvDN, &parentDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (VmDirStringCompareA(dn_norm, gVmdirServerGlobals.dcAccountDN.bvnorm_val, FALSE) == 0)
    {
        /*
         * Deleting a Raft leader should follow the procedure:
         *  1. Shutdown the server which is the raft leader
         *  2. Send deleting account LDAP operation for this account toward the new Raft leader (or to be standalone server).
         *  3. Shutdown the server with the account deleted above (better to rename the data.mdb once it is shutdown).
         */
        dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "VmDirDeleteRaftProxy: deleting account with raft leader is not allowed; error %d", dwError);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetParentDN(&(gVmdirServerGlobals.dcAccountDN), &dcContainerDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (dcContainerDN.lberbv.bv_len == 0)
    {
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    if (VmDirStringCompareA(parentDN.lberbv.bv_val, dcContainerDN.bvnorm_val, FALSE) != 0)
    {
       //Not a machine account
       goto cleanup;
    }

    dwError = VmDirGetRdn(&bvDN, &peerRdn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRdnToNameValue(&peerRdn, &pszName, &pHostname);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
    _VmDirRemovePeerInLock(pHostname);
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

cleanup:
    VmDirFreeBervalContent(&dcContainerDN);
    VmDirFreeBervalContent(&peerRdn);
    VmDirFreeBervalContent(&parentDN);
    VMDIR_SAFE_FREE_MEMORY(pHostname);
    VMDIR_SAFE_FREE_MEMORY(pszName);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmDirRpcConnect(PVMDIR_SERVER_CONTEXT *ppServer, PVMDIR_PEER_PROXY pProxySelf)
{
    DWORD dwError = 0;
    PVMDIR_SERVER_CONTEXT pServer = NULL;
    PSTR pszDcAccountPwd = NULL;
    PCSTR pPeerHostName = pProxySelf->raftPeerHostname;
    UINT32 remoteServerState = 0;
    BOOLEAN bLock = FALSE;
    static int logCnt = 0;

    assert(ppServer);

    if (*ppServer)
    {
        VmDirCloseServer(*ppServer);
        *ppServer = NULL;
    }

    while(VmDirdState() != VMDIRD_STATE_SHUTDOWN && pProxySelf->isDeleted == FALSE)
    {
       VMDIR_SAFE_FREE_MEMORY(pszDcAccountPwd);
       dwError = VmDirReadDCAccountPassword(&pszDcAccountPwd);
       BAIL_ON_VMDIR_ERROR( dwError );

       if (pServer)
       {
           VmDirCloseServer( pServer);
           pServer = NULL;
       }
       dwError = VmDirOpenServerA(pPeerHostName, gVmdirServerGlobals.dcAccountUPN.lberbv_val,
                              NULL, pszDcAccountPwd, 0, NULL, &pServer);
       if (dwError == rpc_s_connect_rejected || dwError == rpc_s_connect_timed_out ||
           dwError == rpc_s_cannot_connect || dwError == rpc_s_connection_closed ||
           dwError == rpc_s_auth_method || dwError == rpc_s_host_unreachable)
       {
          if (logCnt++ % 10 == 0)
          {
              VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
                                "_VmDirRpcConnect: not connected or authorization error %d host %s, will retry to connect.",
                                dwError, pPeerHostName);
          }
          VmDirSleep(gVmdirGlobals.dwRaftPingIntervalMS>>1);
          continue;
       } else if (dwError != 0)
       {
           if (logCnt++ % 5 == 0)
           {
               VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL, "_VmDirRpcConnect: VmDirOpenServerA error %d host %s",
                          dwError, pPeerHostName);
           }
           goto error;
       }

       //Try to make an rpc call.
       dwError = VmDirGetState(pServer, &remoteServerState);
       if (dwError != 0 || remoteServerState != VMDIRD_STATE_NORMAL)
       {
           VmDirCloseServer( pServer);
           pServer = NULL;
           if (logCnt++ % 5 == 0)
           {
               VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL, "_VmDirRpcConnect: cannot get server state for host %s, will retry.",
                                 pPeerHostName);
           }
           VmDirSleep(3000);
           continue;
       }

       VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
       if (pProxySelf->proxy_state == PENDING_ADD)
       {
           pProxySelf->proxy_state = RPC_BUSY;
           gRaftState.clusterSize++;
       }
       VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

       VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirRpcConnect: RPC to %s established with current raft cluster size %d",
                      pPeerHostName, gRaftState.clusterSize);
       *ppServer = pServer;
       pServer = NULL;
       break;
    }

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDcAccountPwd);
    return dwError;

error:
    if (pServer)
    {
        VmDirCloseServer( pServer);
    }
    goto cleanup;
}

static
VOID
_VmDirApplyLogsUpto(UINT64 indexToApply)
{
     UINT64 logIdx = 0;
     UINT64 logIdxStart = gRaftState.lastApplied + 1;

     VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "_VmDirApplyLogsUpto: logIdxStart %llu upto logIdx %llu", logIdxStart, indexToApply);
     for (logIdx = logIdxStart; logIdx <= indexToApply; logIdx++)
     {
         _VmDirApplyLog(logIdx);
     }
}

static
DWORD
_VmDirApplyLog(unsigned long long indexToApply)
{
    DWORD dwError = 0;
    VDIR_RAFT_LOG logEntry = {0};
    VDIR_ENTRY entry = {0};
    PVDIR_SCHEMA_CTX pSchemaCtx = NULL;
    VDIR_OPERATION ldapOp = {0};
    VDIR_OPERATION modOp = {0};
    char logEntryDn[RAFT_CONTEXT_DN_MAX_LEN] = {0};
    VDIR_BERVALUE bvIndexApplied = VDIR_BERVALUE_INIT;
    PSTR pszLocalErrorMsg = NULL;
    char logIndexStr[VMDIR_MAX_I64_ASCII_STR_LEN] = {0};
    char opStr[RAFT_CONTEXT_DN_MAX_LEN] = {0};
    BOOLEAN bLock = FALSE;
    BOOLEAN bHasTxn = FALSE;
    unsigned long long priCommitIndex = 0;

    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
    if (indexToApply <= gRaftState.lastApplied)
    {
        //already applied.
        VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirApplyLog: ignore allready applied LogIndex %llu lastApplied %llu",
                     indexToApply, gRaftState.lastApplied);
        goto cleanup;
    }
    priCommitIndex = gRaftState.commitIndex;
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

    dwError = _VmDirFetchLogEntry(indexToApply, &logEntry, __LINE__);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (logEntry.requestCode == 0)
    {
        //no-op
        VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
        gRaftState.lastApplied = indexToApply;
        if ( gRaftState.commitIndex < logEntry.index)
        {
             gRaftState.commitIndex = logEntry.index;
             gRaftState.commitIndexTerm = logEntry.term;
        }
        VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
          "_VmDirApplyLog: log-no-op (%llu %u) indexToApply: %llu priCommitIdx %llu commitIndex %llu term %d",
          logEntry.index, logEntry.term, indexToApply, priCommitIndex,
          gRaftState.commitIndex, gRaftState.currentTerm);
        goto cleanup;
    }

    dwError = VmDirSchemaCtxAcquire( &pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (logEntry.requestCode == LDAP_REQ_ADD)
    {
        dwError = VmDirStringCpyA(opStr, sizeof(opStr), "Add");
        BAIL_ON_VMDIR_ERROR(dwError);

        entry.encodedEntrySize = logEntry.chglog.lberbv_len;
        dwError = VmDirAllocateAndCopyMemory(logEntry.chglog.lberbv_val, entry.encodedEntrySize,
                                             (PVOID *)&entry.encodedEntry);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirDecodeEntry(pSchemaCtx, &entry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg), "VmDirDecodeEntry with logIndx %llu", indexToApply);

        entry.eId = logEntry.entryId;

        dwError = VmDirGetParentDN(&entry.dn, &entry.pdn );
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg), "VmDirGetParentDN with logIndx %llu", indexToApply);

        dwError = VmDirInitStackOperation( &ldapOp, VDIR_OPERATION_TYPE_REPL, LDAP_REQ_ADD, pSchemaCtx );
        BAIL_ON_VMDIR_ERROR(dwError);

        VmDirFreeEntry(ldapOp.request.addReq.pEntry);
        ldapOp.request.addReq.pEntry = &entry;

        ldapOp.pBEIF = VmDirBackendSelect(NULL);
        assert(ldapOp.pBEIF);

        dwError = VmDirEntryAttrValueNormalize(&entry, FALSE /*all attributes*/);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirSchemaModMutexAcquire(&ldapOp);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrorMsg, "SchemaModMutexAcquire - PreAdd");

        dwError = VmDirReplSchemaEntryPreAdd(&ldapOp, &entry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrorMsg, "VmDirReplSchemaEntryPreAdd");

        dwError = ldapOp.pBEIF->pfnBETxnBegin(ldapOp.pBECtx, VDIR_BACKEND_TXN_WRITE);
        BAIL_ON_VMDIR_ERROR(dwError);
        bHasTxn = TRUE;

        dwError = ldapOp.pBEIF->pfnBEEntryAdd(ldapOp.pBECtx, &entry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
              "pfnBEEntryAdd  %s from logIndx %llu", entry.dn.lberbv_val, indexToApply);
    } else if (logEntry.requestCode == LDAP_REQ_MODIFY)
    {
        dwError = VmDirStringCpyA(opStr, sizeof(opStr), "Modify");
        BAIL_ON_VMDIR_ERROR(dwError);

        ModifyReq*   modReq = NULL;
        BOOLEAN bDnModified = FALSE;

        dwError = VmDirInitStackOperation( &ldapOp, VDIR_OPERATION_TYPE_REPL, LDAP_REQ_MODIFY, pSchemaCtx );
        BAIL_ON_VMDIR_ERROR(dwError);

        ldapOp.pBEIF = VmDirBackendSelect(NULL);
        assert(ldapOp.pBEIF);

        modReq = &(ldapOp.request.modifyReq);

        dwError = VmDirDecodeMods(pSchemaCtx, logEntry.chglog.lberbv_val, &modReq->mods);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg), "VmDirDecodeMods from logIndx %llu", indexToApply);

        dwError = ldapOp.pBEIF->pfnBETxnBegin(ldapOp.pBECtx, VDIR_BACKEND_TXN_WRITE);
        BAIL_ON_VMDIR_ERROR(dwError);
        bHasTxn = TRUE;

        dwError = ldapOp.pBEIF->pfnBEIdToEntry(ldapOp.pBECtx, pSchemaCtx,
                                               logEntry.entryId, &entry, VDIR_BACKEND_ENTRY_LOCK_WRITE);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg), "pfnBEIdToEntry from logIndx %llu", indexToApply);

        dwError = VmDirBervalContentDup(&entry.dn, &modReq->dn);
        BAIL_ON_VMDIR_ERROR(dwError);

        // Apply modify operations to the current entry (in pack format)
        dwError = VmDirApplyModsToEntryStruct(pSchemaCtx, modReq, &entry, &bDnModified, &pszLocalErrorMsg );
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrorMsg, "ApplyModsToEntryStruct (%s)", pszLocalErrorMsg);

        dwError = VmDirSchemaModMutexAcquire(&ldapOp);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg), "VmDirSchemaModMutexAcquire for Modify");

        dwError = VmDirReplSchemaEntryPreMoidify(&ldapOp, &entry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrorMsg, "VmDirReplSchemaEntryPreMoidify");

        dwError = ldapOp.pBEIF->pfnBEEntryModify(ldapOp.pBECtx, modReq->mods, &entry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrorMsg, "BEEntryModify, (%s)",
                                      VDIR_SAFE_STRING(ldapOp.pBEErrorMsg));
    } else if (logEntry.requestCode == LDAP_REQ_DELETE)
    {
        dwError = VmDirStringCpyA(opStr, sizeof(opStr),  "Delete");
        BAIL_ON_VMDIR_ERROR(dwError);

        DeleteReq *delReq = NULL;
        ModifyReq *modReq = NULL;

        dwError = VmDirInitStackOperation( &ldapOp, VDIR_OPERATION_TYPE_REPL, LDAP_REQ_DELETE, pSchemaCtx );
        BAIL_ON_VMDIR_ERROR(dwError);

        ldapOp.pBEIF = VmDirBackendSelect(NULL);
        assert(ldapOp.pBEIF);

        dwError = ldapOp.pBEIF->pfnBETxnBegin(ldapOp.pBECtx, VDIR_BACKEND_TXN_WRITE);
        BAIL_ON_VMDIR_ERROR(dwError);
        bHasTxn = TRUE;

        dwError = ldapOp.pBEIF->pfnBEIdToEntry(ldapOp.pBECtx, pSchemaCtx,
                                               logEntry.entryId, &entry, VDIR_BACKEND_ENTRY_LOCK_WRITE);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg),
                                     "Delete entry pfnBEIdToEntry eId %llu", logEntry.entryId);

        dwError = VmDirEntryUnpack(&entry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg), "Delete entry VmDirEntryUnpack");

        delReq = &(ldapOp.request.deleteReq);
        modReq = &(ldapOp.request.modifyReq);

        dwError = VmDirBervalContentDup(&(entry.dn), &(delReq->dn));
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirNormalizeDN( &(delReq->dn), pSchemaCtx );
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrorMsg, "Delete entry, normalizationDN");

        dwError = VmDirNormalizeMods(pSchemaCtx, modReq->mods, &pszLocalErrorMsg );
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirGetParentDN(&entry.dn, &entry.pdn );
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrorMsg, "Delete entry; get ParentDn");

        dwError = GenerateDeleteAttrsMods( &ldapOp, &entry );
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrorMsg, "GenerateDeleteAttrsMods");

        dwError = VmDirNormalizeMods(pSchemaCtx, modReq->mods, &pszLocalErrorMsg );
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirApplyModsToEntryStruct(pSchemaCtx, modReq, &entry, NULL, &pszLocalErrorMsg );
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrorMsg, "Delete entry VmDirApplyModsToEntryStruct");

        dwError = ldapOp.pBEIF->pfnBEEntryDelete(ldapOp.pBECtx, modReq->mods, &entry );
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg), "pfnBEEntryDelete from logIndx %llu", indexToApply);

        dwError = DeleteRefAttributesValue(&ldapOp, &(entry.dn));
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg),
                                     "DeleteRefAttributesValue from logIndx %llu", indexToApply);
    } else
    {
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrorMsg, "Invalid log format");
    }

    dwError = VmDirCloneStackOperation(&ldapOp, &modOp, VDIR_OPERATION_TYPE_INTERNAL, LDAP_REQ_MODIFY, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA(logEntryDn, sizeof(logEntryDn),  "%s=%llu,%s",
                    ATTR_CN, logEntry.index, RAFT_LOGS_CONTAINER_DN);
    BAIL_ON_VMDIR_ERROR(dwError);

    modOp.reqDn.lberbv.bv_val = logEntryDn;
    modOp.reqDn.lberbv.bv_len = VmDirStringLenA(logEntryDn);

    dwError = VmDirStringPrintFA(logIndexStr, sizeof(logIndexStr), "%llu", indexToApply);
    BAIL_ON_VMDIR_ERROR(dwError);

    bvIndexApplied.lberbv.bv_val  = logIndexStr;
    bvIndexApplied.lberbv.bv_len = VmDirStringLenA(logIndexStr);

    dwError = VmDirAddModSingleAttributeReplace(&modOp, RAFT_PERSIST_STATE_DN,
                                                ATTR_RAFT_LAST_APPLIED, &bvIndexApplied);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
          "VmDirAddModSingleAttributeReplace mode on %s = %llu ", ATTR_RAFT_LAST_APPLIED, logIndexStr);

    modOp.bSuppressLogInfo = TRUE;
    dwError = VmDirInternalModifyEntry(&modOp);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
          "VmDirInternalModifyEntry mod on %s = %llu ", ATTR_RAFT_LAST_APPLIED, logIndexStr);

    dwError = ldapOp.pBEIF->pfnBETxnCommit(ldapOp.pBECtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (logEntry.requestCode == LDAP_REQ_ADD)
    {
        dwError = VmDirEntryUnpack(&entry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrorMsg, "VmDirEntryUnpack)");

        dwError = VmDirReplSchemaEntryPostAdd(&ldapOp, &entry);
        if (dwError)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirApplyLog: VmDirReplSchemaEntryPostAdd %s - error(%d)",
                    entry.dn.lberbv_val, dwError);
            dwError = 0; //don't hold off applying the next raft log since the base transaction has committed.
        }
    } else if (logEntry.requestCode == LDAP_REQ_MODIFY)
    {
        dwError = VmDirReplSchemaEntryPostMoidify(&ldapOp, &entry);
        if (dwError)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirApplyLog: VmDirReplSchemaEntryPostMoidify %s - error(%d)",
                      entry.dn.lberbv_val, dwError);
            dwError = 0; //don't hold off applying the next raft log since the base transaction has committed.
        }
    }

    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
    gRaftState.lastApplied = indexToApply;
    if ( gRaftState.commitIndex < logEntry.index)
    {
         gRaftState.commitIndex = logEntry.index;
         gRaftState.commitIndexTerm = logEntry.term;
    }
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

    if (logEntry.requestCode == LDAP_REQ_ADD)
    {
        dwError = VmDirAddRaftProxy(&entry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg), "VmDirAddRaftProxy %s",
                                 VDIR_SAFE_STRING(entry.dn.lberbv.bv_val));
    } else if (logEntry.requestCode == LDAP_REQ_DELETE)
    {
        dwError = _VmDirDeleteRaftProxy(BERVAL_NORM_VAL(entry.dn));
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg), "VmDirDeleteRaftProxy %s",
                                 VDIR_SAFE_STRING(entry.dn.lberbv.bv_val));
    }

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
      "_VmDirApplyLog: succeeded %llu %s %s priCommitIdx %llu commitIndex %llu term %d",
      indexToApply, opStr, VDIR_SAFE_STRING(entry.dn.lberbv.bv_val), priCommitIndex,
      gRaftState.commitIndex, gRaftState.currentTerm);

cleanup:
    if (modOp.pBECtx)
    {
        modOp.pBECtx->pBEPrivate = NULL; //Make sure that calls commit/abort only once.
    }
    VmDirFreeOperationContent(&modOp);
    (VOID)VmDirSchemaModMutexRelease(&ldapOp);
    ldapOp.request.addReq.pEntry = NULL;
    VmDirFreeOperationContent(&ldapOp);
    VmDirFreeEntryContent(&entry);
    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    _VmDirChgLogFree(&logEntry);
    return dwError;

error:
    if (bHasTxn)
    {
        ldapOp.pBEIF->pfnBETxnAbort(ldapOp.pBECtx);
    }
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirApplyLog: dn %s error: %s errcode: %d", logEntryDn, VDIR_SAFE_STRING(pszLocalErrorMsg), dwError);
    goto cleanup;
}

/* Currently assume each MDB transaction have only one LDAP Add
 * To support more than one LDAP Add, a (static) major id might bitOR into pEntryId
 *  - like VmDirRaftLogIndexToCommit
 * The major id would be reset for each MDB transaction.
 * To support this, raftLog needs to pack multiple LDAP Add(s) into one log.
 */
VOID
VmDirRaftNextNewEntryId(ENTRYID *pEntryId)
{
    *pEntryId = NEW_ENTRY_EID_PREFIX | (gRaftState.commitIndex + 1);
}

/*
 * This function is used to create ObjectSid for each new entry to add.
 * Since it is called before a MDB txn_begin, it might be called more than once
 * with the same gRaftState.commitIndex (which is unique within a mdb transaction)
 */
UINT64 VmDirRaftLogIndexToCommit()
{
    static UINT64 prevIdx = 0;
    static UINT64 idxMajor = 0;
    UINT64 commitIndex = 0;
    BOOLEAN bLock = FALSE;

    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
    if (gRaftState.commitIndex == prevIdx)
    {
        idxMajor++;
    } else
    {
        prevIdx = gRaftState.commitIndex;
        idxMajor = 0;
    }
    commitIndex = (gRaftState.commitIndex + 1) | (idxMajor << 32);
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

    return commitIndex;
}

BOOLEAN
VmDirRaftDisallowUpdates(PCSTR caller)
{
    BOOLEAN bDisallowUpdates = FALSE;
    BOOLEAN bLock = FALSE;

    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
    if (gRaftState.role == VDIR_RAFT_ROLE_LEADER && gRaftState.disallowUpdates)
    {
        bDisallowUpdates = TRUE;
    }
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

    if (bDisallowUpdates)
    {
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
          "VmDirRaftDisallowUpdates: %s not a leader or during leader transition", caller);
    }
    return bDisallowUpdates;
}

/*
 * Set ppszLeader to raft leader's server name or NULL if the server
 * itself is a leader, the server is a candidate, or the server
 * have not received Ping yet after switching to follower
 */
DWORD
VmDirRaftGetLeader(PSTR *ppszLeader)
{
    BOOLEAN bLock = FALSE;
    PSTR pszLeader = NULL;
    DWORD dwError = 0;

    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
    if (gRaftState.clusterSize >= 2 &&
        gRaftState.role == VDIR_RAFT_ROLE_FOLLOWER &&
        gRaftState.leader.lberbv_len > 0)
    {
       dwError = VmDirAllocateStringPrintf(&pszLeader, "%s", gRaftState.leader.lberbv_val);
       BAIL_ON_VMDIR_ERROR(dwError);
    }
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

    *ppszLeader = pszLeader;

cleanup:
    return dwError;

error:
    goto cleanup;
}

VOID
VmDirRaftGetRole(VDIR_RAFT_ROLE *pRole)
{
    BOOLEAN bLock = FALSE;
    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
    if (gRaftState.clusterSize < 2)
    {
        *pRole = VDIR_RAFT_ROLE_LEADER;
    }
    else
    {
        *pRole = gRaftState.role;
    }
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
}

BOOLEAN
VmDirRaftNeedReferral(PCSTR pszReqDn)
{
    char *p = NULL;
    BOOLEAN bNeedReferral = FALSE;

    if ((p=VmDirStringCaseStrA(pszReqDn, RAFT_CONTEXT_DN)) &&
         VmDirStringCompareA(p, RAFT_CONTEXT_DN, FALSE)==0)
    {
        //Don't offer referral for Raft states or logs.
        goto done;
    }

    if (pszReqDn == NULL || pszReqDn[0] == '\0')
    {
        //Search for DseRoot, don't no need for referral
        goto done;
    }

    bNeedReferral = (gRaftState.role == VDIR_RAFT_ROLE_FOLLOWER);

done:
    return bNeedReferral;
}

//This thread is to apply logs when the instance is a follower,
//     or compact logs when it is idle.
static
DWORD
_VmDirRaftLogApplyThread()
{
    int dwError = 0;

    BOOLEAN bLock = FALSE;
    unsigned long long indexToApply = 0;

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirRaftLogApplyThread: started.");

    while(1)
    {
        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            dwError = 0;
            goto done;
        }

        VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
        dwError = VmDirConditionTimedWait(gRaftNewLogCond, gRaftStateMutex, 10000);
        if(VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
           dwError = 0;
           VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
           goto done;
        }

        if(dwError == ETIMEDOUT || gRaftState.role != VDIR_RAFT_ROLE_FOLLOWER)
        {
            VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
            continue;
        }

        //Received signal gRaftNewLogCond
        indexToApply = gRaftState.indexToApply;
        VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

        _VmDirApplyLogsUpto(indexToApply);
    }

done:
    return dwError;
}

//This thread performs Raft logs compaction
static
DWORD
_VmDirRaftLogCompactThread()
{
    BOOLEAN bLock = FALSE;
    int     iLogsRemain = 0;
    VDIR_RAFT_LOG_TRIM_SCORE   dwScore = RAFT_LOG_TRIM_SCORE_HIGH;

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirRaftLogCompactThread: started.");

    while(1)
    {
        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            goto done;
        }
        VmDirSleep(2000);

continue_compact:
        dwScore = RAFT_LOG_TRIM_SCORE_HIGH;

        VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
        iLogsRemain = (int)(gRaftState.commitIndex - gRaftState.firstLogIndex);

        if (iLogsRemain <= gVmdirGlobals.dwRaftKeeplogs)
        {
            dwScore = RAFT_LOG_TRIM_SCORE_LOW;
        }
        else
        {
            if (gRaftState.opCounts > 0 &&  // has external write since last check
                iLogsRemain < gVmdirGlobals.dwRaftKeeplogs * 2)
            {   // yield to external write
                dwScore = RAFT_LOG_TRIM_SCORE_LOW;
                gRaftState.opCounts = 0;
            }
        }

        VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

        if (dwScore == RAFT_LOG_TRIM_SCORE_LOW)
        {
            continue;
        }

        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            goto done;
        }

        _VmDirRaftCompactLogs(50); //compact at most 50 logs a time
        goto continue_compact;
    }

done:
    return 0;
}

//Compact up to compactLogsUpto logs
static
DWORD
_VmDirRaftCompactLogs(int compactLogsUpto)
{
    static int logsCompacted = 0;
    static int logsCompactedRound = 0;
    static int failDeleteCnt = 0;
    unsigned long long logIdxToCompact = 0;
    static time_t prevLogTime = {0};
    time_t now = {0};
    int i = 0;
    VDIR_BERVALUE berFirstLog = VDIR_BERVALUE_INIT;
    DWORD dwError = 0;

    if (VmDirdState() != VMDIRD_STATE_NORMAL)
    {
        goto cleanup;
    }

    for (logIdxToCompact=gRaftState.firstLogIndex, i=0;
         i<compactLogsUpto; i++,logIdxToCompact++)
    {
        dwError = _VmdirDeleteLog(logIdxToCompact, TRUE);
        if (dwError == 0)
        {
            gRaftState.firstLogIndex = logIdxToCompact+1;
            logsCompacted++;
        } else if (dwError == VMDIR_ERROR_UNWILLING_TO_PERFORM)
        {
            //server is in readonly mode ?
            goto cleanup;
        } else
        {
            failDeleteCnt++;
        }
    }

    if (logsCompacted > 200)
    {
        //Update firstLogIndex attribute in Raft state for every 200 logs compacted
        dwError = VmDirAllocateBerValueAVsnprintf(&berFirstLog, "%llu", gRaftState.firstLogIndex);
        BAIL_ON_VMDIR_ERROR(dwError);

        (VOID)VmDirInternalEntryAttributeReplace(NULL, RAFT_PERSIST_STATE_DN, ATTR_RAFT_FIRST_LOGINDEX, &berFirstLog);
        logsCompactedRound += logsCompacted;
        logsCompacted = 0;

        now = time(&now);
        if ((now - prevLogTime) > 30)
        {
            prevLogTime = now;
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirRaftCompactLogs %d logs processed, firstLog %llu commitIndex %llu failed deletion %d",
                       logsCompactedRound, gRaftState.firstLogIndex, gRaftState.commitIndex, failDeleteCnt);
            logsCompactedRound = 0;
            failDeleteCnt = 0;
        }
    }

cleanup:
    VmDirFreeBervalContent(&berFirstLog);
    return dwError;

error:
    goto cleanup;
}

/*
 * Set ppszLeader to raft leader's server name if it exists
 */
DWORD
VmDirRaftGetLeaderString(PSTR *ppszLeader)
{
    BOOLEAN bLock = FALSE;
    PSTR pszLeader = NULL;
    DWORD dwError = 0;

    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
    if (gRaftState.clusterSize < 2 && gRaftState.hostname.lberbv_len > 0)
    {
        //Standalone server, show self as the leader.
        dwError = VmDirAllocateStringPrintf(&pszLeader, "%s", gRaftState.hostname.lberbv_val);
    } else if (gRaftState.role == VDIR_RAFT_ROLE_FOLLOWER && gRaftState.leader.lberbv_len > 0 )
    {
        dwError = VmDirAllocateStringPrintf(&pszLeader, "%s", gRaftState.leader.lberbv_val);
    } else if (gRaftState.role == VDIR_RAFT_ROLE_LEADER && gRaftState.hostname.lberbv_len > 0)
    {
        dwError = VmDirAllocateStringPrintf(&pszLeader, "%s", gRaftState.hostname.lberbv_val);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszLeader = pszLeader;

cleanup:
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
    return dwError;

error:
    goto cleanup;
}

/*
 * Get raft active followers
 */
DWORD
VmDirRaftGetFollowers(PDEQUE pFollowers)
{
    BOOLEAN bLock = FALSE;
    DWORD dwError = 0;
    PSTR pFollower = NULL;
    PVMDIR_PEER_PROXY pPeerProxy = NULL;

    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
    if (gRaftState.clusterSize < 2)
    {
        //Standalong server, don't show self as a follower.
        goto cleanup;
    }

    if (gRaftState.role == VDIR_RAFT_ROLE_FOLLOWER && gRaftState.hostname.lberbv_len > 0)
    {
        dwError = VmDirAllocateStringPrintf(&pFollower, "%s", gRaftState.hostname.lberbv_val);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = dequePush(pFollowers, pFollower);
        BAIL_ON_VMDIR_ERROR(dwError);
        pFollower = NULL;
    } else if (gRaftState.role == VDIR_RAFT_ROLE_LEADER)
    {
        for (pPeerProxy=gRaftState.proxies; pPeerProxy != NULL; pPeerProxy = pPeerProxy->pNext)
        {
            if (pPeerProxy->isDeleted || pPeerProxy->proxy_state==RPC_DISCONN)
            {
                continue;
            }
            // list active followers only
            dwError = VmDirAllocateStringPrintf(&pFollower, "%s", pPeerProxy->raftPeerHostname);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = dequePush(pFollowers, pFollower);
            BAIL_ON_VMDIR_ERROR(dwError);
            pFollower = NULL;
        }
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pFollower);
    dequeFreeStringContents(pFollowers);
    goto cleanup;
}

/*
 * Get raft volatile state on at this server
 */
DWORD
VmDirRaftGetState(PDEQUE pStateQueue)
{
    BOOLEAN bLock = FALSE;
    DWORD dwError = 0;
    PSTR pNode = NULL;
    PVMDIR_PEER_PROXY pPeerProxy = NULL;

    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
    if (gRaftState.hostname.lberbv_len == 0)
    {
        //Not set yet during server start or promo
        goto cleanup;
    }

    dwError = VmDirAllocateStringPrintf(&pNode, "node: %s", gRaftState.hostname.lberbv_val);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = dequePush(pStateQueue, pNode);
    BAIL_ON_VMDIR_ERROR(dwError);
    pNode = NULL;

    dwError = VmDirAllocateStringPrintf(&pNode, "role: %s",
                (gRaftState.clusterSize < 2 || gRaftState.role==VDIR_RAFT_ROLE_LEADER)?"leader":
                (gRaftState.role==VDIR_RAFT_ROLE_FOLLOWER?"follower":"candidate"));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = dequePush(pStateQueue, pNode);
    BAIL_ON_VMDIR_ERROR(dwError);
    pNode = NULL;

    dwError = VmDirAllocateStringPrintf(&pNode, "lastIndex: %llu", gRaftState.lastLogIndex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = dequePush(pStateQueue, pNode);
    BAIL_ON_VMDIR_ERROR(dwError);
    pNode = NULL;

    dwError = VmDirAllocateStringPrintf(&pNode, "lastAppliedIndex: %llu", gRaftState.lastApplied);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = dequePush(pStateQueue, pNode);
    BAIL_ON_VMDIR_ERROR(dwError);
    pNode = NULL;

    dwError = VmDirAllocateStringPrintf(&pNode, "term: %u", gRaftState.currentTerm);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = dequePush(pStateQueue, pNode);
    BAIL_ON_VMDIR_ERROR(dwError);
    pNode = NULL;

    if (gRaftState.role == VDIR_RAFT_ROLE_FOLLOWER && gRaftState.leader.lberbv_len > 0)
    {
       dwError = VmDirAllocateStringPrintf(&pNode, "leader: %s", gRaftState.leader.lberbv_val);
       BAIL_ON_VMDIR_ERROR(dwError);

       dwError = dequePush(pStateQueue, pNode);
       BAIL_ON_VMDIR_ERROR(dwError);
       pNode = NULL;
    } else if (gRaftState.role == VDIR_RAFT_ROLE_LEADER)
    {
        for (pPeerProxy=gRaftState.proxies; pPeerProxy != NULL; pPeerProxy = pPeerProxy->pNext)
        {
            if (pPeerProxy->isDeleted)
            {
                continue;
            }
            dwError = VmDirAllocateStringPrintf(&pNode, "follower: %s %s", pPeerProxy->raftPeerHostname,
                        pPeerProxy->proxy_state==RPC_DISCONN?"disconnected":"active");
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = dequePush(pStateQueue, pNode);
            BAIL_ON_VMDIR_ERROR(dwError);
            pNode = NULL;
        }
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pNode);
    dequeFreeStringContents(pStateQueue);
    goto cleanup;
}

/*
 * Get raft cluster members
 */
DWORD
VmDirRaftGetMembers(PDEQUE pMembers)
{
    DWORD dwError = 0;
    VDIR_BERVALUE dcContainerDN = VDIR_BERVALUE_INIT;
    PSTR pHostname = NULL;
    PSTR pszName = NULL;
    VDIR_BERVALUE dcRdn = VDIR_BERVALUE_INIT;
    VDIR_ENTRY_ARRAY entryArray = {0};
    int i = 0;

    VmDirGetParentDN(&(gVmdirServerGlobals.dcAccountDN), &dcContainerDN);
    if (dcContainerDN.lberbv.bv_len == 0)
    {
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    dwError = VmDirSimpleEqualFilterInternalSearch(dcContainerDN.lberbv.bv_val,
                    LDAP_SCOPE_ONE, ATTR_OBJECT_CLASS, OC_COMPUTER, &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < entryArray.iSize; i++)
    {
        dwError = VmDirNormalizeDNWrapper(&(entryArray.pEntry[i].dn));
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirGetRdn(&entryArray.pEntry[i].dn, &dcRdn);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirRdnToNameValue(&dcRdn, &pszName, &pHostname);
        BAIL_ON_VMDIR_ERROR(dwError);

        VmDirFreeBervalContent(&dcRdn);
        VMDIR_SAFE_FREE_STRINGA(pszName);

        dwError = dequePush(pMembers, pHostname);
        BAIL_ON_VMDIR_ERROR(dwError);
        pHostname = NULL;
    }

cleanup:
    VmDirFreeBervalContent(&dcContainerDN);
    VmDirFreeBervalContent(&dcRdn);
    VMDIR_SAFE_FREE_MEMORY(pHostname);
    VmDirFreeEntryArrayContent(&entryArray);
    return dwError;

error:
    dequeFreeStringContents(pMembers);
    goto cleanup;
}

DWORD
_VmdirDeleteLog(unsigned long long logIndex, BOOLEAN bCompactLog)
{
    DWORD dwError = 0;
    PVDIR_SCHEMA_CTX pSchemaCtx = NULL;
    VDIR_OPERATION ldapOp = {0};
    DeleteReq *dr = NULL;
    PSTR pDn = NULL;
    BOOLEAN bLock = FALSE;
    unsigned long long preLogIndex = 0;
    int preLogTerm = 0;

    dwError = VmDirSchemaCtxAcquire( &pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInitStackOperation( &ldapOp, VDIR_OPERATION_TYPE_INTERNAL, LDAP_REQ_DELETE, pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

    ldapOp.pBEIF = VmDirBackendSelect(NULL);
    assert(ldapOp.pBEIF);

    dwError = VmDirAllocateStringPrintf(&pDn, "%s=%llu,%s", ATTR_CN, logIndex, RAFT_LOGS_CONTAINER_DN);
    BAIL_ON_VMDIR_ERROR(dwError);

    ldapOp.reqDn.lberbv.bv_val = pDn;
    ldapOp.reqDn.lberbv.bv_len = VmDirStringLenA(pDn);
    ldapOp.reqDn.bOwnBvVal = TRUE;

    dr = &(ldapOp.request.deleteReq);

    dwError = VmDirAllocateBerValueAVsnprintf(&(dr->dn), "%s", ldapOp.reqDn.lberbv.bv_val);
    BAIL_ON_VMDIR_ERROR(dwError);

    ldapOp.bSuppressLogInfo = TRUE;
    dwError = VmDirInternalDeleteEntry(&ldapOp);
    if (dwError == VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND)
    {
        dwError = 0;
    }
    BAIL_ON_VMDIR_ERROR( dwError );

    if (!bCompactLog)
    {
        //Called from VmDirAppendEntriesGetReply, need to decrement lastLogIndex after the log is deleted
        VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
        if (logIndex <= gRaftState.lastLogIndex)
        {
            dwError = _VmDirGetPrevLogArgs(&preLogIndex, &preLogTerm, logIndex-1, __LINE__);
            BAIL_ON_VMDIR_ERROR(dwError);

            if (preLogIndex==0)
            {
                VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,  "_VmdirDeleteLog: no prev logIndex found for %llu", logIndex-1);
                dwError = LDAP_OPERATIONS_ERROR;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            gRaftState.lastLogIndex = preLogIndex;
            gRaftState.lastLogTerm = preLogTerm;
        }
        VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
          "_VmdirDeleteLog deleted log %llu prevLogIndex %llu lastLogIndex %llu lastLogterm %llu currentTerm %d lastApplied %llu commitLogIndex %llu role %d",
          logIndex, preLogIndex, gRaftState.lastLogIndex, gRaftState.lastLogTerm, gRaftState.currentTerm, gRaftState.lastApplied,
          gRaftState.commitIndex, gRaftState.role);
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
    VmDirFreeOperationContent(&ldapOp);
    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmdirDeleteLog: entry %s error %d", pDn,  dwError);
    goto cleanup;
}

static
VOID
_VmDirPersistTerm(
    int term
    )
{
    DWORD dwError = 0;
    PVDIR_SCHEMA_CTX pSchemaCtx = NULL;
    CHAR pszTerm[VMDIR_MAX_I64_ASCII_STR_LEN] = {0};
    VDIR_BERVALUE berTerm = VDIR_BERVALUE_INIT;
    VDIR_OPERATION ldapOp = {0};
    PSTR pszLocalErrorMsg = NULL;

    dwError = VmDirSchemaCtxAcquire( &pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInitStackOperation( &ldapOp, VDIR_OPERATION_TYPE_INTERNAL, LDAP_REQ_MODIFY, pSchemaCtx );
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "VmDirInitStackOperation");

    ldapOp.pBEIF = VmDirBackendSelect(NULL);
    assert(ldapOp.pBEIF);

    if (term > 0)
    {
        dwError = VmDirStringPrintFA(pszTerm , sizeof(pszTerm), "%d", term );
        BAIL_ON_VMDIR_ERROR(dwError);

        berTerm.lberbv.bv_val = pszTerm;
        berTerm.lberbv.bv_len = VmDirStringLenA(pszTerm);

        dwError = VmDirAddModSingleAttributeReplace(&ldapOp, RAFT_PERSIST_STATE_DN, ATTR_RAFT_TERM, &berTerm);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "VmDirAddModSingleAttributeReplace term");
    }

    ldapOp.bSuppressLogInfo = TRUE;
    dwError = VmDirInternalModifyEntry(&ldapOp);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "VmDirInternalModifyEntry");

cleanup:
    VmDirFreeOperationContent(&ldapOp);

    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);

    if (VmDirdState() != VMDIRD_STATE_SHUTDOWN)
    {
        //Raft cannot garantee protocol safety if new term cannot be persisted.
        assert(dwError==0);
    }

    return;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirPersistTerm: %s error %d term %d currentTerm %d; server role %d",
                    VDIR_SAFE_STRING(pszLocalErrorMsg), dwError, term, gRaftState.currentTerm, gRaftState.role);
    goto cleanup;
}
/*
 * Server restart is needed to enable VMDIR_REG_KEY_RAFT_QUORUM_OVERRIDE.
 * Once the key is set, every transaction commit will read the key until it is reset.
 */
static
DWORD
_VmDirGetRaftQuorumOverride(BOOLEAN bForceKeyRead)
{
    DWORD dwQuorumOverride = 0;

    if (!bForceKeyRead && !gQuorumOverride)
    {
        goto done;
    }

    (VOID)VmDirGetRegKeyValueDword(
        VMDIR_CONFIG_PARAMETER_V1_KEY_PATH,
        VMDIR_REG_KEY_RAFT_QUORUM_OVERRIDE,
        &dwQuorumOverride, 0);

    gQuorumOverride = dwQuorumOverride;

    if (gQuorumOverride)
    {
        VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
          "_VmDirGetRaftQuorumOverride: QuorumOverride is set - no Raft consensus attempted for transaction commit.");
    }

done:
    return gQuorumOverride;
}
