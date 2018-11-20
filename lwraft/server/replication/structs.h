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
 * Module Name: Directory replication
 *
 * Filename: structs.h
 *
 * Abstract:
 *
 *
 * Private Structures
 *
 */

typedef struct _VMDIR_REPLICATION_PAGE_ENTRY
{
    LDAPMessage *entry;
    int entryState;
    PSTR pszDn;
    DWORD dwDnLength;
    int errVal;
} VMDIR_REPLICATION_PAGE_ENTRY, *PVMDIR_REPLICATION_PAGE_ENTRY;

typedef struct _VMDIR_REPLICATION_PAGE
{
    PSTR pszFilter;
    LDAPControl syncReqCtrl;
    LDAPMessage *searchRes;
    LDAPControl **searchResCtrls;
    VMDIR_REPLICATION_PAGE_ENTRY *pEntries;
    int iEntriesRequested;
    int iEntriesReceived;
    USN lastSupplierUsnProcessed;
    int iEntriesProcessed;
    int iEntriesOutOfSequence;
} VMDIR_REPLICATION_PAGE, *PVMDIR_REPLICATION_PAGE;

typedef struct _VMDIR_REPLICATION_PASSWORD
{
    PCSTR pszPassword;
    time_t *pPasswordFailTime;
} VMDIR_REPLICATION_PASSWORD, *PVMDIR_REPLICATION_PASSWORD;

typedef struct _VMDIR_REPLICATION_CREDENTIALS
{
    PSTR pszUPN;
    PSTR pszDN;
    PSTR pszPassword;
    PSTR pszOldPassword;
    BOOLEAN bChanged;
} VMDIR_REPLICATION_CREDENTIALS, *PVMDIR_REPLICATION_CREDENTIALS;

typedef struct _VMDIR_REPLICATON_CONNECTION
{
    LDAP *pLd;
    PSTR pszConnectionDescription;
} VMDIR_REPLICATION_CONNECTION, *PVMDIR_REPLICATION_CONNECTION;

typedef struct _VMDIR_REPLICATION_CONTEXT
{
    PVDIR_SCHEMA_CTX pSchemaCtx;
    BOOLEAN bFirstReplicationCycle;
    time_t stLastTimeTriedToFillHoleInDirectory;
    PSTR pszKrb5ErrorMsg;
} VMDIR_REPLICATION_CONTEXT, *PVMDIR_REPLICATION_CONTEXT;

#define RAFT_NOOP 0
#define RAFT_MULTILOG 1

typedef enum _VDIR_RAFT_EXEC_CMD
{
    ExecNone = 0, //no outstanding command
    ExecReqestVote, // There is a RequestVote outstanding
    ExecAppendEntries, //There is a AppendEntries outstanding
    ExecPing //ExecAppendEntries with empty log entry
} VDIR_RAFT_EXEC_CMD;

typedef enum _VDIR_RAFT_PROXY_STATE
{
    RPC_IDLE = 0, //The peer thread is idle and ready to handle new request
    RPC_BUSY,     //The peer thread is in process of serving a RPC request.
    RPC_DISCONN,  //The peer thread is disconnected from the remote server or DCERPC service.
    PENDING_ADD   //The peer thread is created, but waiting for a successsful DCE RPC call to the remote server
} VDIR_RAFT_PROXY_STATE;

typedef struct  _REQUEST_VOTE_ARGS
{
    /* [in] */ UINT32 term;
    /* [in] */ char *candidateId;
    /* [in] */ unsigned long long lastLogIndex;
    /* [in] */ UINT32 lastLogTerm;
    /* [out] */ UINT32 currentTerm;
    /*
     * voteGranted:
     * 0 - granted,
     * 2 - not granted due to peer highest logIndex > mine,
     * 1 - not granted due to split vote or other reasons
     */
    /* [out] */ UINT32 voteGranted;
} REQUEST_VOTE_ARGS;

typedef struct _APPEND_ENTRIES_ARGS
{
    /* [in] */ UINT32 term;
    /* [in] */ char *leader;
    /* [in] */ unsigned long long preLogIndex;
    /* [in] */ UINT32 preLogTerm;
    /* [in] */ unsigned long long leaderCommit;
    /* [in] */ int  entriesSize;
    /* [in] */ char *entries;
    /* [out] */ UINT32 currentTerm;
    /* [out] */ unsigned long long status;
} APPEND_ENTRIES_ARGS;

typedef struct _VMDIR_PEER_PROXY
{
    VMDIR_THREAD tid;
    BOOLEAN isDeleted;
    VDIR_RAFT_PROXY_STATE proxy_state;
    char raftPeerHostname[VMDIR_MAX_LDAP_URI_LEN];
    BOOLEAN bLogReplicated;
    UINT64 matchIndex;
    LDAP* pLd;
    struct _VMDIR_PEER_PROXY *pNext;
} VMDIR_PEER_PROXY, *PVMDIR_PEER_PROXY;

/*
 * current log db. always present.
*/
#define LOG_DB_CURRENT  0

/*
 * previous log db. used on two non-overlapping usage scenarios
 *
 * 1. when migrating from older systems, log and state are combined
 *    and previous db will point to the older main db.
 *
 * 2. when current log db reaches a log count threshold,
      current db is set as previous and a new log db is set
      as current.
*/
#define LOG_DB_PREVIOUS 1

/* total number of log dbs supported */
#define LOG_DB_COUNT    2

typedef struct _VDIR_RAFT_LOG_DB_STATE
{
    UINT64 startLogIndex; /* start log index in this db */
    UINT64 endLogIndex; /* end log index in this db */
    PSTR pszLogDN; /* dn of the log db */
} VDIR_RAFT_LOG_DB_STATE, *PVDIR_RAFT_LOG_DB_STATE;

typedef struct _VDIR_RAFT_STAT
{
    VDIR_RAFT_ROLE role; //This server's current Raft role
    VDIR_RAFT_EXEC_CMD cmd; //Ongoing RPC command
    VDIR_BERVALUE hostname; //This server's hostname
    int clusterSize; //number of servers in raft clust, updated initially and when adding/removing nodes
    UINT32 voteConsensusCnt; //number of positive ballots collected from peers, plus 1 for self.
    UINT32 voteDeniedCnt; //number of negative ballots collected from peers
    UINT32 voteConsensusTerm; //term associated with above voteConsensusCnt
    BOOLEAN initialized;//Whether the stat is loaded from the persistent store after server start.
    UINT64 lastPingRecvTime; //time stamp of the last ping or appendEntries received from leader
    UINT64 commitIndex; //the highest log entry index known to be commited
    UINT32 commitIndexTerm; //the term associated with commitIndex
    UINT64 lastLogIndex; //the log entry index with the highest index.
    UINT64 firstLogIndex; //the first log entry index
    unsigned long long indexToApply; //log index that the log application thread can apply.
    int opCounts; //number of logs created, as a Raft leader, since last logs compaction.
    UINT32 lastLogTerm;  //the term associated with lastLogIndex
    VDIR_BERVALUE leader; //leader's hostname for referal
    BOOLEAN disallowUpdates; //disallow external LDAP add/modify/delete; momently for newly elected leader
    PVMDIR_PEER_PROXY proxies;   //A list of elements, each for a peer proxy
    /* is there a previous log? */
    BOOLEAN bHasPrevLog;
    /* ctx of log entry so that PrepareCommit can commit it (applicable when using log db)*/
    VDIR_BACKEND_CTX beCtx;
    //Below are persistent variables, i.e. must be writen to entry cn=persiststate,cn=raftcontext once changed
    UINT64 lastApplied; //Index of highest log entry that have been applied to directory entry.
    UINT32 currentTerm;
    UINT32 votedForTerm;
    VDIR_BERVALUE votedFor;
} VDIR_RAFT_STATE, *PVDIR_RAFT_STATE;

typedef struct _VDIR_RAFT_COMMIT_CTX
{
    unsigned long long logIndex;
    int logTerm;
    int logRequestCode;
} VDIR_RAFT_COMMIT_CTX, *PVDIR_RAFT_COMMIT_CTX;
