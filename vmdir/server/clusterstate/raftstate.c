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

BOOLEAN
VmDirSchemaSupportSingleMaster()
{
    DWORD dwError = 0;
    PVDIR_ATTRIBUTE pAttr = NULL;
    VDIR_ENTRY_ARRAY entryArray = {0};
    PVDIR_SCHEMA_CTX pSchemaCtx = NULL;
    PVDIR_SCHEMA_AT_DESC pATDesc = NULL;
    BOOLEAN bSchemaOK = FALSE;

    dwError = VmDirSchemaCtxAcquire( &pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSimpleEqualFilterInternalSearch(PERSISTED_DSE_ROOT_DN, LDAP_SCOPE_BASE,
                    ATTR_OBJECT_CLASS, OC_DSE_ROOT, &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (entryArray.iSize == 0)
    {
        dwError = LDAP_NO_SUCH_OBJECT;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pAttr =  VmDirEntryFindAttribute(ATTR_RAFT_TERM, entryArray.pEntry);
    if (pAttr == NULL)
    {
        dwError = VmDirSchemaAttrNameToDescriptor(pSchemaCtx, ATTR_RAFT_TERM, &pATDesc);
        if (dwError)
        {
           VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
             "%s: attribute %s not found - schema doens't support the single master feature",
             __func__, ATTR_RAFT_TERM);
           goto cleanup;
        }
     }
     bSchemaOK = TRUE;

cleanup:
     VmDirFreeEntryArrayContent(&entryArray);
     if (pSchemaCtx)
     {
         VmDirSchemaCtxRelease(pSchemaCtx);
     }
     pSchemaCtx = NULL;
     return bSchemaOK;

error:
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

    dwError = VmDirSimpleEqualFilterInternalSearch(PERSISTED_DSE_ROOT_DN, LDAP_SCOPE_BASE,
                    ATTR_OBJECT_CLASS, OC_DSE_ROOT, &entryArray);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "internalSearch; dn %s", PERSISTED_DSE_ROOT_DN);

    if (entryArray.iSize == 0)
    {
        dwError = LDAP_NO_SUCH_OBJECT;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "No entry found; dn %s", PERSISTED_DSE_ROOT_DN);
    }

    pAttr =  VmDirEntryFindAttribute(ATTR_RAFT_TERM, entryArray.pEntry);
    if (pAttr == NULL)
    {
        //First time run after the server is upgraded.
        // Note that the schema must have upgraded to support this feature,
        // i.e., calling VmDirSchemaSupportSingleMaster has returned TRUE before invokeing this function.
        VmDirPersistTerm(1);
        gRaftState.currentTerm = 1;
    } else
    {
        gRaftState.currentTerm = VmDirStringToIA((PCSTR)pAttr->vals[0].lberbv.bv_val);
    }

    gRaftState.cmd = ExecNone;
    gRaftState.role = VDIR_RAFT_ROLE_FOLLOWER;
    gRaftState.lastPingRecvTime = VmDirGetTimeInMilliSec(); //Set for request vote timeout.
    gRaftState.lastPingSendTime = 0;
    gRaftState.votedForTerm = 0;

    dwError = VmDirSetRaftClusterSize(FALSE);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "VmDirSetRaftClusterSize");

    gRaftState.initialized = 1;
    gpClusterState->bEnabled = TRUE;

cleanup:
    VmDirFreeEntryArrayContent(&entryArray);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }
    if (gpClusterState->bEnabled)
    {
      VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: term %d", __func__, gRaftState.currentTerm);
    }
    else
    {
      VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: the Single Master feature is disabled",
                      __func__, gRaftState.currentTerm);
    }
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
        gpClusterState->pNodeSelf->srvObj.pszFQDN == NULL)
    {
       BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
    }

    if (gRaftState.clusterSize < 2)
    {
        //Standalone server, show self as the leader.
        dwError = VmDirAllocateStringPrintf(&pszLeader, "%s", gpClusterState->pNodeSelf->srvObj.pszFQDN);
    } else if (gRaftState.role == VDIR_RAFT_ROLE_FOLLOWER && gRaftState.leader.lberbv_len > 0 )
    {
        dwError = VmDirAllocateStringPrintf(&pszLeader, "%s", gRaftState.leader.lberbv_val);
    } else if (gRaftState.role == VDIR_RAFT_ROLE_LEADER && gpClusterState->pNodeSelf->srvObj.pszFQDN)
    {
        dwError = VmDirAllocateStringPrintf(&pszLeader, "%s", gpClusterState->pNodeSelf->srvObj.pszFQDN);
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
        gpClusterState->pNodeSelf->srvObj.pszFQDN == NULL)
    {
       BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
    }

    if (gRaftState.role == VDIR_RAFT_ROLE_FOLLOWER)
    {
        dwError = VmDirAllocateStringPrintf(&pFollower, "%s", gpClusterState->pNodeSelf->srvObj.pszFQDN);
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
            dwError = VmDirAllocateStringPrintf(&pFollower, "%s", pNode->srvObj.pszFQDN);
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
        gpClusterState->pNodeSelf->srvObj.pszFQDN == NULL)
    {
       BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
    }

    dwError = VmDirAllocateStringPrintf(&pNode, "node: %s", gpClusterState->pNodeSelf->srvObj.pszFQDN);
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
            dwError = VmDirAllocateStringPrintf(&pNode, "follower: %s %s", pStateNode->srvObj.pszFQDN,
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
    VDIR_BERVALUE serverContainerDN = VDIR_BERVALUE_INIT;
    PSTR pHostname = NULL;
    VDIR_ENTRY_ARRAY entryArray = {0};
    int i = 0;

    VmDirGetParentDN(&(gVmdirServerGlobals.serverObjDN), &serverContainerDN);
    if (serverContainerDN.lberbv.bv_len == 0)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, LDAP_OPERATIONS_ERROR);
    }

    dwError = VmDirSimpleEqualFilterInternalSearch(serverContainerDN.lberbv.bv_val,
                    LDAP_SCOPE_ONE, ATTR_OBJECT_CLASS, OC_DIR_SERVER, &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < entryArray.iSize; i++)
    {
        dwError = VmDirDnLastRDNToCn(entryArray.pEntry[i].dn.lberbv_val, &pHostname);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = dequePush(pMembers, pHostname);
        BAIL_ON_VMDIR_ERROR(dwError);
        pHostname = NULL;
    }

cleanup:
    VmDirFreeBervalContent(&serverContainerDN);
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

    if (VmDirStringCompareA(pszHost, gpClusterState->pNodeSelf->srvObj.pszFQDN, FALSE)==0)
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

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, gpClusterState->pRWLock);

    VMDIR_SAFE_FREE_MEMORY(pszHost);
    VMDIR_SAFE_FREE_MEMORY(pzaName);
    VmDirFreeBervalContent(&serverDnRdn);
    return;

error:
    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: dn: %s error %d ", __func__, pEntry->dn.bvnorm_val, dwError);
    goto cleanup;
}

VOID
VmDirPersistTerm(
    int term
    )
{
    DWORD dwError = 0;
    PVDIR_SCHEMA_CTX pSchemaCtx = NULL;
    CHAR pszTerm[VMDIR_MAX_I64_ASCII_STR_LEN] = {0};
    VDIR_BERVALUE berTerm = VDIR_BERVALUE_INIT;

    dwError = VmDirSchemaCtxAcquire( &pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

    assert(term > 0);
    dwError = VmDirStringPrintFA(pszTerm , sizeof(pszTerm), "%d", term );
    BAIL_ON_VMDIR_ERROR(dwError);

    berTerm.lberbv.bv_val = pszTerm;
    berTerm.lberbv.bv_len = VmDirStringLenA(pszTerm);

    dwError = VmDirInternalEntryAttributeReplace(pSchemaCtx, PERSISTED_DSE_ROOT_DN, ATTR_RAFT_TERM, &berTerm);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }
    return;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: error %d term %d currentTerm %d; server role %d", __func__,
                    dwError, term, gRaftState.currentTerm, gRaftState.role);
    if (VmDirdState() != VMDIRD_STATE_SHUTDOWN)
    {
        assert(0); //Raft cannot garantee protocol safety if new term cannot be persisted.
    }
    goto cleanup;
}
