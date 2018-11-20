/*
 * Copyright © 2012-2018 VMware, Inc.  All Rights Reserved.
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
 * Filename: raftLdapRpc.c
 */

#include "includes.h"

static
DWORD
_VmDirRequestVoteGetReply(
    UINT32 term,
    char *candidateId,
    unsigned long long lastLogIndex,
    UINT32 lastLogTerm,
    UINT32 *currentTerm,
    UINT32 *voteGranted
    );

static
DWORD
_VmDirAppendEntriesGetReply(
    UINT32 term,
    char *leader,
    unsigned long long preLogIndex,
    UINT32 preLogTerm,
    unsigned long long leaderCommit,
    int entrySize,
    char *entries,
    UINT32 *currentTerm,
    unsigned long long *status
    );

DWORD
VmDirRaftLdapRpcAppendEntries(
    PVMDIR_PEER_PROXY pProxySelf,
    int term,
    PSTR leader,
    unsigned long long preLogIndex,
    int preLogTerm,
    unsigned long long leaderCommit,
    int entrySize,
    PSTR entries,
    int *currentTerm,
    unsigned long long *status)
{
    DWORD dwError = 0;
    VDIR_BERVALUE bvEntiers = {0};
    VDIR_APPEND_ENTRIES_CONTROL_VALUE appendEntriesCtrlValue = {0};
    LDAPControl     ldapCtr = {0};
    LDAPControl*    srvCtrls[2] = {&ldapCtr, NULL};
    PSTR  ppszAttrs[] = {ATTR_RAFT_TERM, ATTR_RAFT_STATE, NULL };
    LDAPMessage* searchRes = NULL;
    LDAPMessage* pEntry = NULL;
    struct berval** ppVals = NULL;
    struct timeval  tv = {0};
    int localTerm = 0;
    int retVal = 0;
    unsigned long long localStatus = 0;
    LDAP* pLd = NULL;

    if (!pProxySelf->pLd || pProxySelf->proxy_state == RPC_DISCONN)
    {
       BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_SERVER_DOWN);
    }

    pLd = pProxySelf->pLd;

    if (entrySize > 1)
    {
        bvEntiers.lberbv.bv_val = entries;
        bvEntiers.lberbv.bv_len = entrySize;
    } else
    {
       bvEntiers.lberbv.bv_val = "p";
       bvEntiers.lberbv.bv_len = 1;
    }

    appendEntriesCtrlValue.term = term;
    appendEntriesCtrlValue.leader = leader;
    appendEntriesCtrlValue.preLogIndex = preLogIndex;
    appendEntriesCtrlValue.preLogTerm = preLogTerm;
    appendEntriesCtrlValue.leaderCommit = leaderCommit;
    appendEntriesCtrlValue.entries = bvEntiers;

    dwError = VmDirCreateAppendEntriesCtrl(&appendEntriesCtrlValue, &ldapCtr);
    BAIL_ON_VMDIR_ERROR(dwError);

    tv.tv_sec = gVmdirGlobals.dwRaftElectionTimeoutMS;

    retVal = ldap_search_ext_s(pLd, RAFT_LDAPRPC_APPEND_ENTRIES_DN, LDAP_SCOPE_BASE,
                                NULL, ppszAttrs, TRUE, srvCtrls, NULL, &tv, 0, &searchRes);
    if (retVal)
    {
       if (retVal == VMDIR_ERROR_UNWILLING_TO_PERFORM)
       {
           BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_UNWILLING_TO_PERFORM);
       }

       pProxySelf->proxy_state = RPC_DISCONN;
       BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_CANNOT_CONNECT_VMDIR);
    }

    pEntry = ldap_first_entry(pLd, searchRes);
    if (pEntry == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NOT_FOUND);
    }

    ppVals = ldap_get_values_len(pLd, pEntry, ATTR_RAFT_TERM);
    if (ldap_count_values_len(ppVals) == 0)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NOT_FOUND);
    }
    localTerm = VmDirStringToIA(ppVals[0]->bv_val);
    VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppVals);

    ppVals = ldap_get_values_len(pLd, pEntry, ATTR_RAFT_STATE);
    if (ldap_count_values_len(ppVals) == 0)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NOT_FOUND);
    }
    localStatus = VmDirStringToLA(ppVals[0]->bv_val, NULL, 10);
    VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppVals);

    *currentTerm = localTerm;
    *status = localStatus;

cleanup:
    VmDirFreeCtrlContent(&ldapCtr);
    VDIR_SAFE_LDAP_MSGFREE(searchRes);
    VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppVals);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: term %d preLogIndex %llu preLogTerm %d error %d",
                    __func__,  term, preLogIndex, preLogTerm, dwError);
    goto cleanup;
}

DWORD
VmDirAppendEntriesReplyEntry(
    PVDIR_APPEND_ENTRIES_CONTROL_VALUE pv,
    PVDIR_ENTRY*    ppEntry
    )
{
    DWORD dwError = 0;
    PVDIR_ENTRY pEntry = NULL;
    PVDIR_SCHEMA_CTX pSchemaCtx = NULL;
    unsigned int currentTerm = 0;
    unsigned long long status = {0};
    char currentTermStr[VMDIR_MAX_I64_ASCII_STR_LEN] = {0};
    char statusStr[VMDIR_MAX_I64_ASCII_STR_LEN] = {0};

    dwError = _VmDirAppendEntriesGetReply(pv->term, pv->leader, pv->preLogIndex, pv->preLogTerm,
                pv->leaderCommit, pv->entries.lberbv.bv_len, pv->entries.lberbv.bv_val,
                &currentTerm, &status);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA(currentTermStr, sizeof(currentTermStr), "%u", currentTerm);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA(statusStr, sizeof(statusStr), "%llu", status);
    BAIL_ON_VMDIR_ERROR(dwError);

    {
        PSTR ppAttrs[] = {ATTR_DN, RAFT_LDAPRPC_APPEND_ENTRIES_DN,
                          ATTR_CN, "appendentries",
                          ATTR_RAFT_TERM, currentTermStr,
                          ATTR_RAFT_STATE, statusStr,
                          NULL };
        dwError = VmDirAttrListToNewEntry(pSchemaCtx, RAFT_LDAPRPC_APPEND_ENTRIES_DN, ppAttrs, FALSE, &pEntry);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetParentDN(&pEntry->dn, &pEntry->pdn);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppEntry = pEntry;

cleanup:
    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }
    return dwError;

error:
    VmDirFreeEntryContent(pEntry);
    goto cleanup;
}

DWORD
VmDirRequestVoteReplyEntry(
    PVDIR_REQUEST_VOTE_CONTROL_VALUE pv,
    PVDIR_ENTRY*    ppEntry
    )
{
    DWORD dwError = 0;
    PVDIR_ENTRY pEntry = NULL;
    PVDIR_SCHEMA_CTX pSchemaCtx = NULL;
    unsigned int currentTerm = 0;
    unsigned int voteGranted = {0};
    char currentTermStr[VMDIR_MAX_I64_ASCII_STR_LEN] = {0};
    char voteGrantedStr[VMDIR_MAX_I64_ASCII_STR_LEN] = {0};

    dwError = _VmDirRequestVoteGetReply(pv->term, pv->candidateId, pv->lastLogIndex,
                 pv->lastLogTerm, &currentTerm, &voteGranted);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA(currentTermStr, sizeof(currentTermStr), "%u", currentTerm);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA(voteGrantedStr, sizeof(voteGrantedStr), "%llu", voteGranted);
    BAIL_ON_VMDIR_ERROR(dwError);

    {
        PSTR ppAttrs[] = {ATTR_DN, RAFT_LDAPRPC_REQUEST_VOTE_DN,
                          ATTR_CN, "requestvote",
                          ATTR_RAFT_TERM, currentTermStr,
                          ATTR_RAFT_STATE, voteGrantedStr,
                          NULL };
        dwError = VmDirAttrListToNewEntry(pSchemaCtx, RAFT_LDAPRPC_REQUEST_VOTE_DN, ppAttrs, FALSE, &pEntry);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetParentDN(&pEntry->dn, &pEntry->pdn);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppEntry = pEntry;

cleanup:
    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }
    return dwError;

error:
    VmDirFreeEntryContent(pEntry);
    goto cleanup;
}

DWORD
VmDirRaftLdapRpcRequestVote(
    PVMDIR_PEER_PROXY pProxySelf,
    int term,
    PSTR candidateId,
    unsigned long long lastLogIndex,
    int lastLogTerm,
    int * currentTerm,
    int * voteGranted)
{
    DWORD dwError = 0;
    VDIR_REQUEST_VOTE_CONTROL_VALUE requestVoteCtrlValue = {term, candidateId, lastLogIndex, lastLogTerm};
    PSTR  ppszAttrs[] = {ATTR_RAFT_TERM, ATTR_RAFT_STATE, NULL };
    LDAPControl ldapCtr = {0};
    LDAPControl* srvCtrls[2] = {&ldapCtr, NULL};
    LDAPMessage* searchRes = NULL;
    LDAPMessage* pEntry = NULL;
    struct berval** ppVals = NULL;
    struct timeval  tv = {0};
    int localTerm = 0;
    int localVoteGranted = 0;
    int retVal = 0;
    LDAP* pLd = NULL;

    if (!pProxySelf->pLd || pProxySelf->proxy_state == RPC_DISCONN)
    {
       BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_SERVER_DOWN);
    }

    pLd = pProxySelf->pLd;

    dwError = VmDirCreateRequestVoteCtrl( &requestVoteCtrlValue, &ldapCtr);
    BAIL_ON_VMDIR_ERROR(dwError);

    tv.tv_sec = gVmdirGlobals.dwRaftElectionTimeoutMS;
    retVal = ldap_search_ext_s(pProxySelf->pLd, RAFT_LDAPRPC_REQUEST_VOTE_DN, LDAP_SCOPE_BASE,
                                NULL, ppszAttrs, TRUE, srvCtrls, NULL, &tv, 0, &searchRes);
    if (retVal)
    {
       if (retVal == VMDIR_ERROR_UNWILLING_TO_PERFORM)
       {
           BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_UNWILLING_TO_PERFORM);
       }

       pProxySelf->proxy_state = RPC_DISCONN;
       BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_CANNOT_CONNECT_VMDIR);
    }

    pEntry = ldap_first_entry(pLd, searchRes);
    if (pEntry == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NOT_FOUND);
    }

    ppVals = ldap_get_values_len(pLd, pEntry, ATTR_RAFT_TERM);
    if (ldap_count_values_len(ppVals) == 0)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NOT_FOUND);
    }
    localTerm = VmDirStringToIA(ppVals[0]->bv_val);
    VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppVals);

    ppVals = ldap_get_values_len(pLd, pEntry, ATTR_RAFT_STATE);
    if (ldap_count_values_len(ppVals) == 0)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NOT_FOUND);
    }
    localVoteGranted = VmDirStringToIA(ppVals[0]->bv_val);
    VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppVals);

    *currentTerm = localTerm;
    *voteGranted = localVoteGranted;

cleanup:
    VmDirFreeCtrlContent(&ldapCtr);
    VDIR_SAFE_LDAP_MSGFREE(searchRes);
    VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppVals);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: term %d lastLogIndex %llu lastLogTerm %d error %d",
                    __func__,  term, lastLogIndex, lastLogTerm, dwError);
    goto cleanup;
}

DWORD
VmDirLdapRpcConnect(
    PVMDIR_PEER_PROXY pProxySelf
    )
{
    DWORD dwError = 0;
    PSTR pszDomainName = NULL;
    BOOLEAN bLock = FALSE;
    int logCnt = 0;
    LDAP* pLd = NULL;

    VDIR_SAFE_UNBIND_EXT_S(pProxySelf->pLd);

    while(VmDirdState() != VMDIRD_STATE_SHUTDOWN && pProxySelf->isDeleted == FALSE)
    {
       VMDIR_SAFE_FREE_STRINGA(pszDomainName);

       dwError = VmDirDomainDNToName(gVmdirServerGlobals.systemDomainDN.bvnorm_val, &pszDomainName);
       BAIL_ON_VMDIR_ERROR(dwError);

       VDIR_SAFE_UNBIND_EXT_S(pLd);

       dwError = VmDirConnectLDAPServerWithMachineAccount(pProxySelf->raftPeerHostname, pszDomainName, &pLd);
       if (dwError)
       {
           if (logCnt++ % 10 == 0)
          {
              VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
                "%s: failed to connect to host %s with error %d, will retry.",
                __func__, pProxySelf->raftPeerHostname, dwError);
          }
          VmDirSleep(gVmdirGlobals.dwRaftElectionTimeoutMS);
          continue;
       }

       VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
       if (pProxySelf->proxy_state == PENDING_ADD)
       {
           gRaftState.clusterSize++;
       }
       pProxySelf->proxy_state = RPC_BUSY;
       pProxySelf->pLd = pLd;
       VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);

       VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: conection to %s established with current raft cluster size %d",
                      __func__, pProxySelf->raftPeerHostname, gRaftState.clusterSize);
       break;
    }

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDomainName);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmDirRequestVoteGetReply(
    UINT32 term,
    char *candidateId,
    unsigned long long lastLogIndex,
    UINT32 lastLogTerm,
    UINT32 *currentTerm,
    UINT32 *voteGranted
    )
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
        VmDirPersistTerm(newTerm);
    }

    if (!dwError)
    {
      VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
        "%s: granted=%d candidateId %s term %d lastLogTerm %d; server term %d (old term %d) role %d votedForTerm %d votedFor %s",
        __func__, iVoteGranted, candidateId, term, lastLogTerm, gRaftState.currentTerm, oldTerm, gRaftState.role,
        gRaftState.votedForTerm, VDIR_SAFE_STRING(bvVotedFor.lberbv_val));
    }
    VmDirFreeBervalContent(&bvVotedFor);
    return dwError;

error:
    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
      "%s: candidateId %s term %d lastLogTerm %d; server term %d (old term %d) role %d error=%d",
      __func__, candidateId, term, lastLogTerm, gRaftState.currentTerm, oldTerm, gRaftState.role, dwError);
    goto cleanup;
}

static
DWORD
_VmDirAppendEntriesGetReply(
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
    int oldRole = VDIR_RAFT_ROLE_FOLLOWER;
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
    static long long latency_max = 0;
    static long long latency_sum = 0;
    static int latency_stat_cnt = 0;
    UINT64 start_t = 0;
    UINT64 end_t = 0;

    *status = 0;
    *currentTerm = 0;

    if (!gRaftState.initialized)
    {
        //Don't try to replicate if this server is not initialized or the peer thread is not ready.
        dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    start_t = VmDirGetTimeInMilliSec();
    // Set lastPingRecvTime to a future time to prevent local transaction write
    //   latency triggering false start voting event. Intentionally setting without
    //   mutex since the mutex waiting might delay this setting.
    gRaftState.lastPingRecvTime = VmDirGetTimeInMilliSec() + 3600000;

    VMDIR_LOCK_MUTEX(bLockRpcReply, gRaftRpcReplyMutex);
    //Serialize appendEntriesRpc, requestVoteRpc handlers and _VmDirEvaluateVoteResult

    VMDIR_LOCK_MUTEX(bLock, gRaftStateMutex);
    oldRole = gRaftState.role;

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
        *status = gRaftState.lastLogIndex;
        goto cleanup;
    }

    if (gRaftState.role != VDIR_RAFT_ROLE_FOLLOWER &&
        gRaftState.currentTerm < term)
    {
        //I am not a follower yet and my term is smaller than peer's term,
        //switch to follower, and let peer send a fresh appendEntries.
        *currentTerm = newTerm = gRaftState.currentTerm = term;
        gRaftState.role = VDIR_RAFT_ROLE_FOLLOWER;
        gRaftState.lastPingRecvTime = VmDirGetTimeInMilliSec(); //Set for request vote timeout
        VmDirConditionSignal(gRaftAppendEntryReachConsensusCond); //wake up thread waiting for commit consensus
        dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    //peer's term == my term, keep as a follower or switch to follower if not
    gRaftState.role = VDIR_RAFT_ROLE_FOLLOWER;
    *currentTerm = newTerm = gRaftState.currentTerm = term;
    lastLogIndex = gRaftState.lastLogIndex;
    if (oldRole != VDIR_RAFT_ROLE_FOLLOWER)
    {
        gRaftState.lastPingRecvTime = VmDirGetTimeInMilliSec(); //Set for election timeout
        VmDirConditionSignal(gRaftAppendEntryReachConsensusCond); //wake up thread waiting for commit consensus
    }

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
    dwError = VmDirDeleteAllLogs(preLogIndex+1, &bFatalError);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (entrySize > 1)
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

    if ((gRaftState.lastApplied + RAFT_APPLIED_GAP_MAX) < gRaftState.lastLogIndex)
    {
        //Applied index is too much behind, let the leader slow down on this peer.
        // allowing the local server to catch up.
        dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR(dwError);
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
              "%s: entSize %d leader %s term %d leaderCommit %llu preLogIdx %llu preLogTerm %d commitIdx %llu priCommitIdx %llu currentTerm %d oldTerm %d role %d status %llu",
              __func__, entrySize, leader, term, leaderCommit, preLogIndex, preLogTerm,
              gRaftState.commitIndex, priCommitIndex, *currentTerm, oldTerm,
              gRaftState.role, *status);
        }
    }
    VMDIR_UNLOCK_MUTEX(bLock, gRaftStateMutex);
    VMDIR_UNLOCK_MUTEX(bLockRpcReply, gRaftRpcReplyMutex);

    if (newTerm > oldTerm)
    {
        VmDirPersistTerm(newTerm);
    }

    VmDirChgLogFree(&chgLog, FALSE);

    //Raft inconsistency may occur if fatal error detected.
    assert(!bFatalError);

    if (dwError == 0)
    {
        end_t = VmDirGetTimeInMilliSec();
        if((end_t - start_t) > latency_max)
        {
            latency_max = end_t - start_t;
        }
        latency_sum += end_t - start_t;
        latency_stat_cnt++;
        if (latency_stat_cnt > RPC_LATENCY_STATS_CNT)
        {
           VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: stats on latency: avg %.2f ms, max: %llu ms with %d operations",
             __func__, (float)latency_sum/(float)RPC_LATENCY_STATS_CNT, latency_max, RPC_LATENCY_STATS_CNT);
           latency_stat_cnt = 0;
           latency_sum = 0;
           latency_max = 0;
        }
    }
    gRaftState.lastPingRecvTime = VmDirGetTimeInMilliSec();

    return dwError;

error:
     VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
              "%s: entSize %d leader %s term %d leaderCommit %llu preLogIdx %llu preLogTerm %d commitIdx %llu priCommitIdx %llu currentTerm %d oldTerm %d lastApplied %llu role %d error %d",
              __func__, entrySize, leader, term, leaderCommit, preLogIndex, preLogTerm,
              gRaftState.commitIndex, priCommitIndex, *currentTerm, oldTerm,
              gRaftState.lastApplied, gRaftState.role, dwError);
    goto cleanup;
}
