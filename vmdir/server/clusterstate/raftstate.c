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



/*
 * Module Name: raftstate
 *
 * Filename: raftstate.c
 *
 *
 */

#include "includes.h"

//Create the initial persistent state
DWORD
VmDirInitRaftPsState(
    VOID
    )
{
    DWORD dwError = 0;
    PVDIR_SCHEMA_CTX pSchemaCtx = NULL;
    PSTR pszLogEntryDn = NULL;

    PSTR ppContex[] = { ATTR_OBJECT_CLASS,  "vmwDirCfg",
                        ATTR_CN, RAFT_CONTEXT_CONTAINER_NAME,
                        NULL };

    dwError = VmDirSchemaCtxAcquire( &pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSimpleEntryCreate(pSchemaCtx, ppContex, RAFT_CONTEXT_DN, RAFT_CONTEXT_ENTRY_ID);
    BAIL_ON_VMDIR_ERROR(dwError);

    PSTR ppPersisteState[] = { ATTR_OBJECT_CLASS, OC_CLASS_RAFT_PERSIST_STATE,
                               ATTR_CN, RAFT_PERSIST_STATE_NAME,
                               ATTR_RAFT_TERM, "1",
                               NULL };
    dwError = VmDirSimpleEntryCreate(pSchemaCtx, ppPersisteState, RAFT_PERSIST_STATE_DN, RAFT_PERSIST_STATE_ENTRY_ID);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }
    VMDIR_SAFE_FREE_MEMORY(pszLogEntryDn);
    if (dwError==0)
    {
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: succeeded; currentTerm %d cluster size %d",
                   __func__, gRaftState.currentTerm, gRaftState.clusterSize);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: error %d", __func__, dwError);
    goto cleanup;
}

VOID
VmDirLoadRaftState(
    VOID
    )
{
    DWORD dwError = 0;
    PSTR  pszLocalErrorMsg = NULL;
    PVDIR_ATTRIBUTE pAttr = NULL;
    VDIR_ENTRY_ARRAY entryArray = {0};
    PVDIR_SCHEMA_CTX pSchemaCtx = NULL;

    dwError = VmDirSchemaCtxAcquire( &pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSimpleEqualFilterInternalSearch(RAFT_PERSIST_STATE_DN, LDAP_SCOPE_BASE,
                    ATTR_OBJECT_CLASS, OC_CLASS_RAFT_PERSIST_STATE, &entryArray);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "internalSearch; dn %s", RAFT_PERSIST_STATE_DN);

    if (entryArray.iSize == 0)
    {
        dwError = LDAP_NO_SUCH_OBJECT;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "No entry found; dn %s", RAFT_PERSIST_STATE_DN);
    }

    pAttr =  VmDirEntryFindAttribute(ATTR_RAFT_TERM, entryArray.pEntry);
    if (pAttr == NULL)
    {
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "Cannot find attr %s", ATTR_RAFT_TERM);
    }
    gRaftState.currentTerm = VmDirStringToIA((PCSTR)pAttr->vals[0].lberbv.bv_val);
    gRaftState.cmd = ExecNone;
    gRaftState.role = VDIR_RAFT_ROLE_FOLLOWER;
    gRaftState.lastPingRecvTime = VmDirGetTimeInMilliSec(); //Set for request vote timeout.
    gRaftState.lastPingSendTime = 0;
    gRaftState.votedForTerm = 0;

    dwError = VmDirSetRaftClusterSize(FALSE);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "VmDirSetRaftClusterSize");

    gRaftState.initialized = 1;

cleanup:
    VmDirFreeEntryArrayContent(&entryArray);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }
    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: term %d", __func__, gRaftState.currentTerm);
    return;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: error %d; %s", __func__, dwError, VDIR_SAFE_STRING(pszLocalErrorMsg) );
    assert(0);
    goto cleanup;
}

DWORD
VmDirSetRaftClusterSize(
    BOOLEAN bNeedClusterStateLock
    )
{
    int i = 0;
    DWORD dwError = 0;
    BOOLEAN bInLock = FALSE;
    int newClusterSize = 0;

    if (bNeedClusterStateLock)
    {
        VMDIR_RWLOCK_WRITELOCK(bInLock, gpClusterState->pRWLock, 0);
    }

    if(!gpClusterState->pSiteListSelf)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    for (i=0; i < gpClusterState->pSiteListSelf->dwArySize; i++)
    {
        if (gpClusterState->pSiteListSelf->ppNodeStateAry[i]==NULL)
              break;
        if (gpClusterState->pSiteListSelf->ppNodeStateAry[i]->bIsActive)
        {
            newClusterSize++;
        }
    }

    if (newClusterSize != gRaftState.clusterSize)
    {
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: clusterSize changed from %d to %d",
                       __func__, gRaftState.clusterSize, newClusterSize);
        gRaftState.clusterSize = newClusterSize;
    }

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, gpClusterState->pRWLock);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: error %d", __func__, dwError);
    goto cleanup;
}

/*
 * Set ppszLeader to raft leader's server name if it exists
 */
DWORD
VmDirRaftGetLeaderString(
    PSTR *ppszLeader
    )
{
    BOOLEAN bLock = FALSE;
    PSTR pszLeader = NULL;
    DWORD dwError = 0;

    VMDIR_RWLOCK_READLOCK(bLock, gpClusterState->pRWLock, 0);
    if (gpClusterState == NULL || gpClusterState->pNodeSelf == NULL ||
        gpClusterState->pNodeSelf->pszFQDN == NULL)
    {
       BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
    }

    if (gRaftState.clusterSize < 2)
    {
        //Standalone server, show self as the leader.
        dwError = VmDirAllocateStringPrintf(&pszLeader, "%s", gpClusterState->pNodeSelf->pszFQDN);
    } else if (gRaftState.role == VDIR_RAFT_ROLE_FOLLOWER && gRaftState.leader.lberbv_len > 0 )
    {
        dwError = VmDirAllocateStringPrintf(&pszLeader, "%s", gRaftState.leader.lberbv_val);
    } else if (gRaftState.role == VDIR_RAFT_ROLE_LEADER && gpClusterState->pNodeSelf->pszFQDN)
    {
        dwError = VmDirAllocateStringPrintf(&pszLeader, "%s", gpClusterState->pNodeSelf->pszFQDN);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszLeader = pszLeader;

cleanup:
    VMDIR_RWLOCK_UNLOCK(bLock, gpClusterState->pRWLock);
    return dwError;

error:
    goto cleanup;
}

/*
 * Get raft active followers
 */
DWORD
VmDirRaftGetFollowers(
    PDEQUE pFollowers
    )
{
    BOOLEAN bLock = FALSE;
    DWORD dwError = 0;
    PSTR pFollower = NULL;

    VMDIR_RWLOCK_READLOCK(bLock, gpClusterState->pRWLock, 0);
    if (gRaftState.clusterSize < 2)
    {
        //Standalong server, don't show self as a follower.
        goto cleanup;
    }

    if (gpClusterState == NULL || gpClusterState->pNodeSelf == NULL ||
        gpClusterState->pNodeSelf->pszFQDN == NULL)
    {
       BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
    }

    if (gRaftState.role == VDIR_RAFT_ROLE_FOLLOWER)
    {
        dwError = VmDirAllocateStringPrintf(&pFollower, "%s", gpClusterState->pNodeSelf->pszFQDN);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = dequePush(pFollowers, pFollower);
        BAIL_ON_VMDIR_ERROR(dwError);
        pFollower = NULL;
    } else if (gRaftState.role == VDIR_RAFT_ROLE_LEADER)
    {
        int i = 0;
        for (i=0; i < gpClusterState->pSiteListSelf->dwArySize; i++)
        {
            PVMDIR_NODE_STATE pNode = gpClusterState->pSiteListSelf->ppNodeStateAry[i];
            if (pNode==NULL)
            {
                break;
            }
            if (pNode->bIsSelf || pNode->bIsActive == FALSE)
            {
                continue;
            }
            // list active followers only
            dwError = VmDirAllocateStringPrintf(&pFollower, "%s", pNode->pszFQDN);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = dequePush(pFollowers, pFollower);
            BAIL_ON_VMDIR_ERROR(dwError);
            pFollower = NULL;
        }
    }

cleanup:
    VMDIR_RWLOCK_UNLOCK(bLock, gpClusterState->pRWLock);
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
VmDirRaftGetState(
    PDEQUE pStateQueue
    )
{
    BOOLEAN bLock = FALSE;
    DWORD dwError = 0;
    PSTR pNode = NULL;

    VMDIR_RWLOCK_READLOCK(bLock, gpClusterState->pRWLock, 0);

    if (gpClusterState == NULL || gpClusterState->pNodeSelf == NULL ||
        gpClusterState->pNodeSelf->pszFQDN == NULL)
    {
       BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
    }

    dwError = VmDirAllocateStringPrintf(&pNode, "node: %s", gpClusterState->pNodeSelf->pszFQDN);
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

    dwError = VmDirAllocateStringPrintf(&pNode, "term: %u", gRaftState.currentTerm);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = dequePush(pStateQueue, pNode);
    BAIL_ON_VMDIR_ERROR(dwError);
    pNode = NULL;

    if (gRaftState.role == VDIR_RAFT_ROLE_FOLLOWER &&
        gRaftState.clusterSize > 1 &&
        gRaftState.leader.lberbv_len > 0)
    {
       dwError = VmDirAllocateStringPrintf(&pNode, "leader: %s", gRaftState.leader.lberbv_val);
       BAIL_ON_VMDIR_ERROR(dwError);

       dwError = dequePush(pStateQueue, pNode);
       BAIL_ON_VMDIR_ERROR(dwError);
       pNode = NULL;
    } else if (gRaftState.role == VDIR_RAFT_ROLE_LEADER)
    {
        int i = 0;
        for (i=0; i < gpClusterState->pSiteListSelf->dwArySize; i++){
            PVMDIR_NODE_STATE pStateNode = gpClusterState->pSiteListSelf->ppNodeStateAry[i];
            if (pStateNode==NULL)
            {
                break;
            }
            if (pStateNode->bIsSelf||pStateNode->bIsActive == FALSE)
            {
                continue;
            }

            PVMDIR_LDP_CONN pLdpConn = &pStateNode->nodeLDP;
            dwError = VmDirAllocateStringPrintf(&pNode, "follower: %s %s", pStateNode->pszFQDN,
                        (pLdpConn && pLdpConn->connState==DC_CONNECTION_STATE_CONNECTED)? "active":"disconnected");
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = dequePush(pStateQueue, pNode);
            BAIL_ON_VMDIR_ERROR(dwError);
            pNode = NULL;
        }
    }

cleanup:
    VMDIR_RWLOCK_UNLOCK(bLock, gpClusterState->pRWLock);
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
VmDirRaftGetMembers(
    PDEQUE pMembers
    )
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

VOID
VmDirClusterDeleteNode(
    PVDIR_ENTRY pEntry
    )
{
    DWORD dwError = 0;
    BOOLEAN bInLock = FALSE;
    PVMDIR_NODE_STATE pNode = NULL;
    PSTR pzaName = NULL;
    VDIR_BERVALUE serverDnRdn = VDIR_BERVALUE_INIT;
    PSTR pszHost = NULL;

    dwError = VmDirGetRdn(&pEntry->dn, &serverDnRdn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRdnToNameValue(&serverDnRdn, &pzaName, &pszHost);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_RWLOCK_WRITELOCK(bInLock, gpClusterState->pRWLock, 0);
    if (gpClusterState->pNodeSelf==NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
    }

    if (VmDirStringCompareA(pszHost, gpClusterState->pNodeSelf->pszFQDN, FALSE)==0)
    {
        goto cleanup;
    }

    if (LwRtlHashMapFindKey(gpClusterState->phmNodes, (PVOID*)&pNode, pszHost) == 0)
    {
        pNode->bIsActive = FALSE;
        gpClusterState->bReload = TRUE;
        gRaftState.initialized = FALSE;
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: set %s to InActive", __func__, pszHost);
    }
    VMDIR_RWLOCK_UNLOCK(bInLock, gpClusterState->pRWLock);
cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszHost);
    VMDIR_SAFE_FREE_MEMORY(pzaName);
    VmDirFreeBervalContent(&serverDnRdn);
    return;

error:
    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: dn: %s error %d ", __func__, pEntry->dn.bvnorm_val, dwError);
    goto cleanup;
}
