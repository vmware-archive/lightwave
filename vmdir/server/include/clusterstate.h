/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

#ifndef __VMDIR_CLUSTER_STATE_INTERFACE_H__
#define __VMDIR_CLUSTER_STATE_INTERFACE_H__

// raftstate.c
DWORD
VmDirRaftGetLeaderString(
    PSTR *ppszLeader
    );


DWORD
VmDirRaftGetFollowers(
    PDEQUE pFollowers
    );

DWORD
VmDirRaftGetMembers(
    PDEQUE pMembers
    );

DWORD
VmDirRaftGetState(
    PDEQUE pStateQueue
    );

VOID
VmDirClusterDeleteNode(
    PVDIR_ENTRY pEntry
    );

// statecache.c
VOID
VmDirClusterSetCacheReload(
    VOID
    );

// clusterthr.c
DWORD
VmDirPingReplyEntry(
    PVDIR_RAFT_PING_CONTROL_VALUE pCscv,
    PVDIR_ENTRY *ppEntry
    );

DWORD
VmDirVoteReplyEntry(
    PVDIR_RAFT_VOTE_CONTROL_VALUE pCvcv,
    PVDIR_ENTRY *ppEntry
    );

// libmain.c
DWORD
VmDirClusterLibInit(
    VOID
    );

#endif
