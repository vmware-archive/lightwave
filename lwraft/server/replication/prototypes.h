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
 * Module Name: Directory indexer
 *
 * Filename: prototypes.h
 *
 * Abstract:
 *
 * Function prototypes
 *
 */

#ifndef _CFG_PROTOTYPES_H_
#define _CFG_PROTOTYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

// thread.c

extern VDIR_RAFT_STATE gRaftState;

extern DWORD
VmDirReplSchemaEntryPreAdd(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry);

extern DWORD
VmDirReplSchemaEntryPostAdd(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry);

extern DWORD
VmDirReplSchemaEntryPreMoidify(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry);

extern DWORD
VmDirReplSchemaEntryPostMoidify(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry);

DWORD
InitializeReplicationThread(
    void);

int
VmDirFirstReplicationCycle(
    PSTR pPeerHostURI
    );

BOOLEAN
_VmDirRaftPeerIsReady(
    PCSTR pPeerHostName
    );

DWORD
VmDirInitRaftPsState(
    VOID
    );

DWORD
_VmDirLoadRaftState(
    VOID
    );

DWORD
_VmDirUpdateRaftPsState(
    int term,
    BOOLEAN updateVotedForTerm,
    UINT32 votedForTerm,
    PVDIR_BERVALUE pVotedFor,
    UINT64 lastApplied,
    UINT64 firstLog
    );

DWORD
VmDirAddRaftEntry(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    PVDIR_RAFT_LOG pLogEntry,
    PVDIR_OPERATION pOp
    );

DWORD
_VmDirLogLookup(
    unsigned long long logIndex,
    UINT32 logTerm,
    BOOLEAN *pbLogFound,
    BOOLEAN *pbTermMatch
    );

DWORD
_VmDirDeleteAllLogs(
    unsigned long long startLogIndex
    );

DWORD
_VmDirPersistLog(
    PVDIR_RAFT_LOG pLogEntry
    );

DWORD
_VmDirFetchLogEntry(
    unsigned long long logIndex,
    PVDIR_RAFT_LOG pLogEntry,
    int
    );

DWORD
_VmdirDeleteLog(
    PSTR pDn
    );

DWORD
_VmDirGetPrevLogArgs(
    unsigned long long*,
    UINT32*,
    UINT64,
    int
    );

DWORD
_VmDirGetNextLog(
    UINT64,
    UINT64,
    PVDIR_RAFT_LOG,
    int
    );

VOID
_VmDirClearProxyLogReplicatedInLock(
   VOID
   );

DWORD
_VmDirGetAppendEntriesConsensusCountInLock(
    VOID
    );

DWORD
_VmDirPeersConnectedInLock(
    VOID
    );

DWORD
_VmDirPeersIdleInLock(
    VOID
    );

DWORD
_VmDirPackLogEntry(
    PVDIR_RAFT_LOG pLogEntry
    );

DWORD
_VmDirUnpackLogEntry(
    PVDIR_RAFT_LOG pLogEntry
    );

VOID
_VmDirEncodeUINT32(
    unsigned char ** ppbuf,
    UINT32 value
    );

UINT32
_VmDirDecodeUINT32(
    unsigned char ** ppbuf
    );

VOID
_VmDirEncodeUINT64(
    unsigned char ** ppbuf,
    UINT64 value
    );

UINT64
_VmDirDecodeUINT64(
    unsigned char ** ppbuf
    );

VOID
_VmDirChgLogFree(
    PVDIR_RAFT_LOG chgLog
    );

DWORD
_VmDirGetLogTerm(
    UINT64 index,
    UINT32 *term
    );

DWORD
_VmDirRaftLoadGlobals(
    PSTR *
    );

#ifdef __cplusplus
}
#endif

#endif // _CFG_PROTOTYPES_H_

