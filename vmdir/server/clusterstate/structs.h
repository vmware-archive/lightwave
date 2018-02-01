/*
 * Copyright ©2017 VMware, Inc.  All Rights Reserved.
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

typedef struct _VMDIR_NODE_HEARTBEAT
{
    uint64_t    iServiceStartTimestamp;     // service start up timestamp of VMDIR_NODE_STATE.pszFQDN
    uint64_t    iLastHBTimestamp;           // last HB received from VMDIR_NODE_STATE.pszFQDN
} VMDIR_NODE_HEARTBEAT, *PVMDIR_NODE_HEARTBEAT;

typedef struct _VMDIR_NODE_REPL_STATE
{
    DWORD       dwReplStateVersion;         // node supported replstateversion
    USN         seenMyOrgUSN;               // my max org USN seen by VMDIR_NODE_STATE.pszFQDN
    uint64_t    iLastReplCycleTimestamp;    // last done repl cycle timestamp of VMDIR_NODE_STATE.pszFQDN
} VMDIR_NODE_REPL_STATE, *PVMDIR_NODE_REPL_STATE;


typedef struct _VMDIR_LDP_CONN
{
    LDAP*       pLd;
    DWORD       dwLdapError;
    uint64_t    iLastErrorTimeStamp;        // last time connection failed
    DWORD       dwConsecutiveFailedAttempt; // number of consecutive failed attempt so far
    uint64_t    iConnectTimeStamp;          // last successful connection time
    uint64_t    iLastSendTimeStamp;         // last successful payload send time
    int         msgid;                      // the msgid sent by asyn call of ldap_search_ext
    VMDIR_DC_CONNECTION_STATE connState;
} VMDIR_LDP_CONN, *PVMDIR_LDP_CONN;

typedef struct _VMDIR_NODE_STATE
{
    PSTR        pszFQDN;
    PSTR        pszDN;
    PSTR        pszSite;
    PSTR        pszInvocationId;
    DWORD       dwServerId;
    BOOLEAN     bIsSelf;                    // node of this vmdir service
    BOOLEAN     bIsActive;                  // inactive node will be removed during cache reload.

    VMDIR_LDP_CONN          nodeLDP;        // SUNG, TODO use VMDIR_DC_CONNECTION in PR 1994467
    VMDIR_NODE_HEARTBEAT    nodeHB;
    VMDIR_NODE_REPL_STATE   nodeReplState;
    BOOLEAN    bCtlSent;                    //The ping or vote control has sent and pending get result.
} VMDIR_NODE_STATE, *PVMDIR_NODE_STATE;

typedef struct _VMDIR_PING_REPLY
{
    DWORD dwError; //error code if Ping failed, and there is no valid reply data.
    VDIR_BERVALUE fromFqdn; //the peer FQDN sending ping reply
    unsigned int currentTerm;
    uint64_t raftStatus;
    USN localUSN;
} VMDIR_PING_REPLY, *PVMDIR_PING_REPLY;

typedef struct _VMDIR_VOTE_REPLY
{
    DWORD dwError; //error code if request vote failed, and there is no valid reply data.
    VDIR_BERVALUE fromFqdn; //the peer FQDN sending vote reply
    unsigned int currentTerm;
    unsigned int voteGranted;
} VMDIR_VOTE_REPLY, *PVMDIR_VOTE_REPLY;

typedef struct _VMDIR_CLUSTER_SITE_LIST
{
    PSTR                pszSite;
    DWORD               dwArySize;          // size of ppNodeStateAry
    PVMDIR_NODE_STATE*  ppNodeStateAry;     // array of pointers to PVMDIR_NODE_STATE of the same site

    struct _VMDIR_CLUSTER_SITE_LIST* pNextSiteList;
} VMDIR_CLUSTER_SITE_LIST, *PVMDIR_CLUSTER_SITE_LIST;

typedef struct _VMDIR_CLUSTER_STATE
{
    PVMDIR_RWLOCK       pRWLock;
    BOOLEAN             bReload;
    BOOLEAN             bEnabled; //if False, then the Schema doens't support this feature
    PVMDIR_NODE_STATE   pNodeSelf;

    PLW_HASHMAP         phmNodes;           // FQDN to PVMDIR_NODE_STATE

    PVMDIR_CLUSTER_SITE_LIST pSiteList;     // Linked list of sites
    PVMDIR_CLUSTER_SITE_LIST pSiteListSelf; // SiteList self node is in
} VMDIR_CLUSTER_STATE, *PVMDIR_CLUSTER_STATE;

typedef enum _VDIR_RAFT_ROLE
{
    VDIR_RAFT_ROLE_CANDIDATE = 0,
    VDIR_RAFT_ROLE_FOLLOWER,
    VDIR_RAFT_ROLE_LEADER
} VDIR_RAFT_ROLE;

typedef enum _VDIR_RAFT_EXEC_CMD
{
    ExecNone = 0, //no outstanding command
    ExecReqestVote, // There is a RequestVote outstanding
    ExecWaitSendVote, //had a vote flict, wait a radmon time before sending vote again.
    ExecPing // To send Ping
} VDIR_RAFT_EXEC_CMD;

typedef struct _VDIR_RAFT_STAT
{
    VDIR_RAFT_ROLE role; //This server's current Raft role
    VDIR_RAFT_EXEC_CMD cmd; //Ongoing RPC command
    int clusterSize; //number of servers in raft clust, updated initially and when adding/removing nodes
    UINT32 voteConsensusCnt; //number of positive ballots collected from peers, plus 1 for self.
    UINT32 voteDeniedCnt; //number of negative ballots collected from peers
    UINT32 voteConsensusTerm; //term associated with above voteConsensusCnt
    BOOLEAN initialized;//Whether the stat is loaded from the persistent store after server start.
    UINT64 lastPingRecvTime; //time stamp of the last ping or appendEntries received from leader
    UINT64 lastPingSendTime; //time stamp of the last ping sent as a leader
    VDIR_BERVALUE leader; //leader's hostname for referal
    UINT32 currentTerm;
    UINT32 votedForTerm;
    VDIR_BERVALUE votedFor;
} VDIR_RAFT_STATE, *PVDIR_RAFT_STATE;

typedef struct _VDIR_RAFT_CFG
{
    DWORD dwRaftElectionTimeoutMS;
    DWORD dwRaftPingIntervalMS;
} VDIR_RAFT_CFG, *PVDIR_RAFT_CFG;

typedef struct _LdapRpcResult
{
    PSTR attrName;
    VDIR_BERVALUE *attrValue;
} LdapRpcResult, *PLdapRpcResult;

#define RaftPingIntervalMin 200
#define RaftPingIntervalDefault 2000
#define RaftElectionTimeoutDefault 7000
#define RaftGetResultTimeoutSec 1
#define RaftDelayVoteMS 10000

extern VDIR_RAFT_STATE gRaftState;
