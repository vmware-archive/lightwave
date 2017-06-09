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

#define LOG_ENTRY_EID_PREFIX 0x4000000000000000
#define NEW_ENTRY_EID_PREFIX 0x2000000000000000
typedef struct _VDIR_RAFT_LOG
{
    UINT64 index;
    UINT32 term;
    UINT64 entryId;           //For entryId to be add/modified/deleted.
    UINT32 requestCode;       //LDAP_REQ_ADD, LDAP_REQ_DELETE or LDAP_REQ_MODIFY
    VDIR_BERVALUE chglog;     //packed entry (LDAP_REQ_ADD), mods(LDAP_REQ_MODIFY) or empty value (LDAP_REQ_DELETE)
#define CHGLOG_ADD_PACKED 'a' //Internally packed for LDAP ADD
#define CHGLOG_MOD_PACKED  'm' //Internally packed LDAP MODIFY
#define CHGLOG_DEL_EID     'd' //entryId is the  entry id to delete.
    VDIR_BERVALUE packRaftLog; //packed for MDB persist or RPC transport in format below
                               //encoded log index(8 bytes), term(4 bytes), entryid(8 bytes),
                               // and requestCode(4 bytes), followed by chglog.
#define RAFT_LOG_HEADER_LEN (sizeof(UINT64) + sizeof(UINT32) + sizeof(UINT64) + sizeof(UINT32))
//Total storage for packRaftLog is RAFT_LOG_HEADER_LEN + chglog data length
} VDIR_RAFT_LOG, *PVDIR_RAFT_LOG;

typedef enum _VDIR_RAFT_ROLE
{
    VDIR_RAFT_ROLE_CANDIDATE = 0,
    VDIR_RAFT_ROLE_FOLLOWER,
    VDIR_RAFT_ROLE_LEADER,
    VDIR_RAFT_ROLE_ALONE //standalone server that is pending cluster initialization
} VDIR_RAFT_ROLE;

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
    /* [out] */ UINT32 status;
} APPEND_ENTRIES_ARGS;

typedef struct _VMDIR_PEER_PROXY
{
    VMDIR_THREAD tid;
    BOOLEAN isDeleted;
    VDIR_RAFT_PROXY_STATE proxy_state;
    char raftPeerHostname[VMDIR_MAX_LDAP_URI_LEN];
    BOOLEAN bLogReplicated;
    UINT64 matchIndex;
    struct _VMDIR_PEER_PROXY *pNext;
} VMDIR_PEER_PROXY, *PVMDIR_PEER_PROXY;

typedef struct _VDIR_RAFT_STAT
{
    VDIR_RAFT_ROLE role; //This server's current Raft role
    VDIR_RAFT_EXEC_CMD cmd; //Ongoing RPC command
    VDIR_BERVALUE hostname; //This server's hostname
    int clusterSize; //number of servers in raft clust, updated initially and when adding/removing nodes
    UINT32 voteConsensusCnt; //number of positive ballots collected from peers, plus 1 for self.
    UINT32 voteDeniedCnt; //number of negative ballots collected from peers
    UINT32 voteConsenusuTerm; //term associated with above voteConsensusCnt
    BOOLEAN rpcSent;    //at least one RPC sent to peers
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
    BOOLEAN disallowUpdates; //disallow external LDAP add/modify/delete; momently for newly elected leader.
    PVMDIR_PEER_PROXY proxies;   //A list of elements, each for a peer proxy
    //Below are persistent variables, i.e. must be writen to entry cn=persiststate,cn=raftcontext once changed
    UINT64 lastApplied; //Index of highest log entry that have been applied to directory entry.
    UINT32 currentTerm;
    UINT32 votedForTerm;
    VDIR_BERVALUE votedFor;
} VDIR_RAFT_STATE, *PVDIR_RAFT_STATE;
