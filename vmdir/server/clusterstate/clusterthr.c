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

#include "includes.h"

static PVMDIR_MUTEX gConnStateMutex = NULL;
static PVMDIR_COND gConnLostCond = NULL;

static
DWORD
_VmDirClusterStateThreadFun(
    PVOID pArg
    );

static
DWORD
_VmDirClusterConnThreadFun(
    PVOID pArg
    );

static
VOID
_VmDirSendNodePing(
    VOID
    );

static
VOID
_VmDirSendPingCtl(
    PVMDIR_NODE_STATE pNode
    );

static
DWORD
_VmDirSendStateCtl(
    PVMDIR_NODE_STATE pNode,
    PSTR dn,
    PSTR  *ppszAttrs,
    LDAPControl **srvCtrls
    );

static
DWORD
_VmDirGetLdapResult(
    PVMDIR_NODE_STATE pNode,
    LdapRpcResult *lrr
    );

static
VOID
_VmDirGetPingResult(
    PVMDIR_NODE_STATE pNode,
    PVMDIR_PING_REPLY pPingReply
    );

static
DWORD
_VmDirPingReply(
    unsigned int term,
    PCSTR pLeader,
    unsigned int *currentTerm,
    unsigned long long  *status
    );

static
DWORD
_VmDirVoteReply(
    unsigned int term,
    PCSTR pszFQDN,
    unsigned int *currentTerm,
    unsigned int  *voteGranted
    );

static
VOID
_VmHandlePingReply(
    PVMDIR_PING_REPLY pPingReply
    );

static
VOID
_VmDirSendNodeVote(
    );

static
VOID
_VmDirEvaluateRaftState(
   UINT64 *waitMS
   );

static
VOID
_VmDirGetVoteResult(
    PVMDIR_NODE_STATE pNode,
    PVMDIR_VOTE_REPLY pVoteReply
    );

static
VOID
_VmHandleVoteReply(
    PVMDIR_VOTE_REPLY pVoteReply,
    BOOLEAN *pbDoneVote
    );

static
VOID
_VmDirSendVoteCtl(
    PVMDIR_NODE_STATE pNode,
    int term
    );

static
VOID
_VmDirEvaluateRpcResult(
   UINT64 *waitMS
   );

static
VOID
_VmDirGetRaftParamters(
  VOID
  );

VDIR_RAFT_STATE gRaftState = {0};
VDIR_RAFT_CFG gRaftCfg = {RaftElectionTimeoutDefault, RaftPingIntervalDefault};

DWORD
VmDirInitClusterStateThread(
    VOID
    )
{
    DWORD dwError = 0;
    PVDIR_THREAD_INFO pThrInfo = NULL;
    PVDIR_THREAD_INFO pConnThrInfo = NULL;
    PSTR pszLocalErrorMsg = NULL;

    dwError = VmDirAllocateMutex(&gConnStateMutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateCondition(&gConnLostCond);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvThrInit(&pThrInfo, NULL, NULL, TRUE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateThread(
                &pThrInfo->tid,
                pThrInfo->bJoinThr,
                _VmDirClusterStateThreadFun,
                pThrInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSrvThrAdd(pThrInfo);

    dwError = VmDirSrvThrInit(&pConnThrInfo, NULL, NULL, TRUE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateThread(
                &pConnThrInfo->tid,
                pConnThrInfo->bJoinThr,
                _VmDirClusterConnThreadFun,
                pConnThrInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSrvThrAdd(pConnThrInfo);

    _VmDirGetRaftParamters();

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "ClusterState thread started");

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error %d", __FUNCTION__, dwError);

    VmDirSrvThrFree(pThrInfo);

    goto cleanup;
}

static
DWORD
_VmDirClusterStateThreadFun(
    PVOID pArg
    )
{
    DWORD dwError = 0;
    BOOLEAN bInLock = FALSE;
    BOOLEAN bClusterStateLock = FALSE;
    PVDIR_THREAD_INFO pThreadInfo = (PVDIR_THREAD_INFO)pArg;
    UINT64 uWaitTime = gRaftCfg.dwRaftPingIntervalMS;

    gRaftState.role = VDIR_RAFT_ROLE_FOLLOWER;
    gRaftState.cmd = ExecNone;

    // TODO we should adjust thread priority to high

    while (VmDirdState() != VMDIRD_STATE_SHUTDOWN)
    {
        BOOLEAN bDelayVote = FALSE;
        if (VmDirdState() != VMDIRD_STATE_NORMAL ||
            gVmdirServerGlobals.serverObjDN.bvnorm_len == 0 ||
            gVmdirServerGlobals.bvServerObjName.lberbv_val == NULL)
        {
            //Wait server to initialize global variables.
            VmDirSleep(1000);
            continue;
        }

        if (gpClusterState->bReload)
        {   // cluster member change event occurred, reload cache
            VmDirClusterLoadCache();  // ignore error

            VMDIR_RWLOCK_READLOCK(bClusterStateLock, gpClusterState->pRWLock, 0);
            VmDirLoadRaftState();
            VMDIR_RWLOCK_UNLOCK(bClusterStateLock, gpClusterState->pRWLock);
        }

        if (gpClusterState->pSiteListSelf == NULL)
        {
           //Wait for loading ClusterCache
           VmDirSleep(1000);
           continue;
        }

        VMDIR_RWLOCK_READLOCK(bClusterStateLock, gpClusterState->pRWLock, 0);
        if (!gRaftState.initialized)
        {
            VmDirLoadRaftState();
        }
        VMDIR_RWLOCK_UNLOCK(bClusterStateLock, gpClusterState->pRWLock);

        if (uWaitTime > 0)
        {
            VMDIR_LOCK_MUTEX(bInLock, pThreadInfo->mutexUsed);
            VmDirConditionTimedWait(
                    pThreadInfo->conditionUsed,
                    pThreadInfo->mutexUsed,
                    uWaitTime);
            VMDIR_UNLOCK_MUTEX(bInLock, pThreadInfo->mutexUsed);
        }

        _VmDirEvaluateRaftState(&uWaitTime);
        if (gRaftState.cmd == ExecNone)
        {
            assert(uWaitTime > 0);
            continue;
        }

        VMDIR_LOG_DEBUG(LDAP_DEBUG_RPC, "%s: sending RPC cmd %d", __func__, gRaftState.cmd);
        if (gRaftState.cmd == ExecPing)
        {
            _VmDirSendNodePing();  // regular heart beat and state ping
        } else if (gRaftState.cmd == ExecReqestVote)
        {
            _VmDirSendNodeVote(&bDelayVote);
            if (bDelayVote)
            {
                //if bDelayVote is TRUE, then there is not enough connected peers to obain Raft consensus
                //   Pause to avoid wasting term numbers;
                VmDirSleep(RaftDelayVoteMS);
                continue;
            }
        }

        _VmDirEvaluateRpcResult(&uWaitTime);
    }

    bInLock = FALSE;
    VMDIR_LOCK_MUTEX(bInLock, gConnStateMutex);
    VmDirConditionSignal(gConnLostCond);
    VMDIR_UNLOCK_MUTEX(bInLock, gConnStateMutex);

    return dwError;
}

//A thread trying to make connections to peers
static
DWORD
_VmDirClusterConnThreadFun(
    PVOID pArg
    )
{
    DWORD dwError = 0;
    BOOLEAN bInLock = FALSE;

    while (VmDirdState() != VMDIRD_STATE_SHUTDOWN)
    {
        UINT64 waitTime = 3000;

        if (!gRaftState.initialized)
        {
            VmDirSleep(1000);
            continue;
        }

        VMDIR_LOCK_MUTEX(bInLock, gConnStateMutex);
        (VOID)VmDirConditionTimedWait(gConnLostCond, gConnStateMutex, waitTime);
        VMDIR_UNLOCK_MUTEX(bInLock, gConnStateMutex);

        VMDIR_RWLOCK_READLOCK(bInLock, gpClusterState->pRWLock, 0);
        for (int i=0; i < gpClusterState->pSiteListSelf->dwArySize; i++)
        {
            if (VmDirdState() == VMDIRD_STATE_SHUTDOWN ||
                gpClusterState->pSiteListSelf->ppNodeStateAry[i]==NULL)
            {
                break;
            }

            if(!gpClusterState->pSiteListSelf->ppNodeStateAry[i]->bIsActive ||
                gpClusterState->pSiteListSelf->ppNodeStateAry[i]->bIsSelf)
            {
                continue;
            }

            PVMDIR_LDP_CONN pLdpConn = &gpClusterState->pSiteListSelf->ppNodeStateAry[i]->nodeLDP;
            if (pLdpConn->connState != DC_CONNECTION_STATE_CONNECTING)
            {
                //Only _VmDirClusterStateThreadFun can set connState to CONNECTING
                continue;
            }

            dwError = VmDirClusterNodeLDPConn(gpClusterState->pSiteListSelf->ppNodeStateAry[i], TRUE);
            if (dwError == 0 && pLdpConn->dwLdapError == 0)
            {
                pLdpConn->connState = DC_CONNECTION_STATE_CONNECTED;
                VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: connection to %s established",
                               __func__, gpClusterState->pSiteListSelf->ppNodeStateAry[i]->pszFQDN);
            }
        }
        VMDIR_RWLOCK_UNLOCK(bInLock, gpClusterState->pRWLock);
    }

    return dwError;
}

static
VOID
_VmDirSendNodePing(
    VOID
    )
{
    BOOLEAN bInLock = FALSE;
    VMDIR_PING_REPLY pingReply = {0};
    int i = 0;

    // get the new UTDVector
    // parse it to per node
    // loop thr LDP to send update

    VMDIR_RWLOCK_READLOCK(bInLock, gpClusterState->pRWLock, 0);
    {
        if (!gpClusterState->pSiteListSelf)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: SiteListSelf is not set, error %d", __func__, VMDIR_ERROR_INVALID_STATE);
            goto cleanup;
        }

        for (i=0; i < gpClusterState->pSiteListSelf->dwArySize; i++)
        {
            if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
            {
                goto cleanup;
            }

            if (gpClusterState->pSiteListSelf->ppNodeStateAry[i]==NULL)
            {
              break;
            }

            if (!gpClusterState->pSiteListSelf->ppNodeStateAry[i]->bIsActive ||
                 gpClusterState->pSiteListSelf->ppNodeStateAry[i]->bIsSelf)
            {
                continue;
            }

            gpClusterState->pSiteListSelf->ppNodeStateAry[i]->bCtlSent = FALSE;
            _VmDirSendPingCtl(gpClusterState->pSiteListSelf->ppNodeStateAry[i]);
        }
    }

    {
        for (i=0; i < gpClusterState->pSiteListSelf->dwArySize; i++)
        {
            if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
            {
                goto cleanup;
            }
            if (gpClusterState->pSiteListSelf->ppNodeStateAry[i]==NULL)
            {
              break;
            }

            if (gpClusterState->pSiteListSelf->ppNodeStateAry[i]->bIsSelf ||
                !gpClusterState->pSiteListSelf->ppNodeStateAry[i]->bIsActive ||
                !gpClusterState->pSiteListSelf->ppNodeStateAry[i]->bCtlSent)
            {
                //Don't try to get search result the request wasn't sent out.
                continue;
            }
            _VmDirGetPingResult(gpClusterState->pSiteListSelf->ppNodeStateAry[i], &pingReply);
            _VmHandlePingReply(&pingReply);
        }
    }
    VMDIR_RWLOCK_UNLOCK(bInLock, gpClusterState->pRWLock);

cleanup:
    VmDirFreeBervalContent(&pingReply.fromFqdn);
    VMDIR_RWLOCK_UNLOCK(bInLock, gpClusterState->pRWLock);
    return;
}

static
VOID
_VmDirSendPingCtl(
    PVMDIR_NODE_STATE pNode
    )
{
    DWORD dwError = 0;
    PSTR  ppszAttrs[] = {ATTR_RAFT_CURRENT_TERM, ATTR_RAFT_STATUS, ATTR_HIGHEST_COMMITTED_USN, NULL };
    LDAPControl     ldapCtr = {0};
    LDAPControl*    srvCtrls[2] = {&ldapCtr, NULL};

    dwError = VmDirCreateRaftPingCtrlContent(
                gpClusterState->pNodeSelf->pszFQDN,     //candiateId
                gRaftState.currentTerm,
                &ldapCtr);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirSendStateCtl(pNode, LDAPRPC_PING_DN, ppszAttrs, srvCtrls);
    BAIL_ON_VMDIR_ERROR(dwError);

    pNode->bCtlSent = TRUE;

cleanup:
    VmDirFreeCtrlContent(&ldapCtr);
    return;

error:
    goto cleanup;
}

static
VOID
_VmDirGetPingResult(
    PVMDIR_NODE_STATE pNode,
    PVMDIR_PING_REPLY pPingReply
    )
{
    DWORD dwError = 0;

    VDIR_BERVALUE attrRatfStatusValue = {0};
    VDIR_BERVALUE attrCurrentTermValue ={0};
    VDIR_BERVALUE attrHighestCommitUsnValue = {0};
    LdapRpcResult lrr[] =  {{ATTR_RAFT_STATUS, &attrRatfStatusValue},
                            {ATTR_RAFT_CURRENT_TERM, &attrCurrentTermValue},
                            {ATTR_HIGHEST_COMMITTED_USN, &attrHighestCommitUsnValue},
                            {0}};

   dwError = VmDirAllocateBerValueAVsnprintf(&pPingReply->fromFqdn, "%s", pNode->pszFQDN);
   BAIL_ON_VMDIR_ERROR(dwError);

   dwError = pPingReply->dwError = _VmDirGetLdapResult(pNode, lrr);
   BAIL_ON_VMDIR_ERROR(dwError);

   dwError = VmDirStringToUINT64(attrRatfStatusValue.lberbv_val, NULL, &pPingReply->raftStatus);
   BAIL_ON_VMDIR_ERROR(dwError);

   dwError = VmDirStringToUINT32(attrCurrentTermValue.lberbv_val, NULL, &pPingReply->currentTerm);
   BAIL_ON_VMDIR_ERROR(dwError);

   dwError = VmDirStringToINT64(attrHighestCommitUsnValue.lberbv_val, NULL, &pPingReply->localUSN);
   BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
   VmDirFreeBervalContent(&attrRatfStatusValue);
   VmDirFreeBervalContent(&attrCurrentTermValue);
   VmDirFreeBervalContent(&attrHighestCommitUsnValue);
   return;

error:
   VMDIR_LOG_ERROR(LDAP_DEBUG_RPC, "%s for node %s failed, error %d",
                   __FUNCTION__, pNode->pszFQDN,  dwError);
   goto cleanup;
}

static
DWORD
_VmDirGetLdapResult(PVMDIR_NODE_STATE pNode, LdapRpcResult *lrr)
{
  // loops through results from search
   int msgtype  = LDAP_RES_SEARCH_ENTRY;
   DWORD dwError = 0;
   int ldapError = 0;
   int msgcount = 0;
   LDAPMessage *res = NULL;
   PSTR dn = NULL;
   struct timeval timeout = {1, 0};
   PVMDIR_LDP_CONN pLdpConn = &pNode->nodeLDP;
   BerElement *ber = NULL;
   BerValue **vals = NULL;
   int pos = 0;
   char *attribute = NULL;
   int i = 0;

   if (lrr == NULL || pNode == NULL || pNode->bIsSelf ||
       pNode->bIsActive==FALSE || pLdpConn == NULL)
   {
       BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
   }

   if (pLdpConn->msgid < 0)
   {
       BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
   }

   if (pLdpConn->pLd == NULL ||
       pLdpConn->connState != DC_CONNECTION_STATE_CONNECTED)
   {
       BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_SERVER_DOWN);
   }

   for(msgcount = 0; msgtype != LDAP_RES_SEARCH_RESULT; msgcount++)
   {
      if (res)
      {
         ldap_memfree(res);
         res = NULL;
      }

      timeout.tv_sec = RaftGetResultTimeoutSec;
      timeout.tv_usec = 0;
      ldapError = ldap_result(pLdpConn->pLd, pLdpConn->msgid, 0, &timeout, &res);
      switch(ldapError)
      {
         case -1:
            ldap_get_option(pLdpConn->pLd, LDAP_OPT_RESULT_CODE, &ldapError);
            VMDIR_LOG_DEBUG(LDAP_DEBUG_RPC, "%s: ldap_result got error: %s",
              __func__, ldap_err2string(ldapError));
            dwError = VmDirMapLdapError(ldapError);
            BAIL_ON_VMDIR_ERROR(dwError);
         case 0:
            VMDIR_LOG_DEBUG(LDAP_DEBUG_RPC, "%s: ldap_result: timeout expired", __func__);
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NETWORK_TIMEOUT);
         default:
            break;
      };

      msgtype = ldap_msgtype(res);
      if (msgtype != LDAP_RES_SEARCH_ENTRY)
      {
         continue;
      }

      dn = ldap_get_dn(pLdpConn->pLd, res);
      ldap_memfree(dn);

      // loops through attributes and values
      attribute = ldap_first_attribute(pLdpConn->pLd, res, &ber);
      while((attribute))
      {
         vals = ldap_get_values_len(pLdpConn->pLd, res, attribute);
         for(pos = 0; pos < ldap_count_values_len(vals); pos++)
         {
            for(i=0; lrr[i].attrName; i++)
            {
                if (VmDirStringCompareA(attribute, lrr[i].attrName, FALSE)==0)
                {
                   VMDIR_LOG_DEBUG(LDAP_DEBUG_RPC, "%s: attr %s = %s", __func__, attribute, vals[pos]->bv_val);
                   dwError = VmDirAllocateBerValueAVsnprintf(lrr[i].attrValue, "%s", vals[pos]->bv_val);
                   BAIL_ON_VMDIR_ERROR(dwError);
                   break;
                }
            }
         }
         ber_bvecfree( vals );
         VMDIR_SAFE_FREE_MEMORY(attribute);
         attribute = ldap_next_attribute(pLdpConn->pLd, res, ber);
      }
      if (ber)
      {
          ber_free(ber, 1);
          ber = NULL;
      }
   }

   // Parses search result
   ldap_parse_result(pLdpConn->pLd, res, &ldapError, NULL, NULL, NULL, NULL, 0);
   VMDIR_LOG_DEBUG(LDAP_DEBUG_RPC, "%s: msgid %d, ldapError %i, entries: %d",
                  __func__, pLdpConn->msgid, ldapError, msgcount-1);
   dwError = VmDirMapLdapError(ldapError);
   BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (res)
    {
       ldap_memfree(res);
    }
    if (ber)
    {
        ber_free(ber, 1);
    }
    VMDIR_SAFE_FREE_MEMORY(attribute);
    return dwError;

error:
    if (dwError == VMDIR_ERROR_UNAVAILABLE ||
        dwError == VMDIR_ERROR_NETWORK_TIMEOUT ||
        dwError == VMDIR_ERROR_SERVER_DOWN)
    {
        BOOLEAN bInLock = FALSE;

        VDIR_SAFE_UNBIND_EXT_S(pLdpConn->pLd);

        VMDIR_LOCK_MUTEX(bInLock, gConnStateMutex);
        pLdpConn->connState = DC_CONNECTION_STATE_CONNECTING;
        pLdpConn->dwLdapError = dwError;
        VmDirConditionSignal(gConnLostCond);
        VMDIR_UNLOCK_MUTEX(bInLock, gConnStateMutex);
    }

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: from %s error %d ",
       __func__, (pNode && pNode->pszFQDN)?pNode->pszFQDN:"unknown", dwError);
    goto cleanup;
}

DWORD
VmDirPingReplyEntry(
    PVDIR_RAFT_PING_CONTROL_VALUE pCscv,
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

    if (gpClusterState == NULL || gpClusterState->bEnabled == FALSE)
    {
        //This server hasn't enabled the signle master feature.
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_UNWILLING_TO_PERFORM);
    }

    dwError = _VmDirPingReply(pCscv->term, pCscv->pszFQDN, &currentTerm, &status);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA(currentTermStr, sizeof(currentTermStr), "%u", currentTerm);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA(statusStr, sizeof(statusStr), "%llu", status);
    BAIL_ON_VMDIR_ERROR(dwError);

    {
        PSTR ppAttrs[] = {ATTR_DN, LDAPRPC_PING_DN,
                          ATTR_CN, "ping",
                          ATTR_OBJECT_CLASS, OC_CLUSTER_STATE,
                          ATTR_RAFT_CURRENT_TERM, currentTermStr,
                          ATTR_RAFT_STATUS, statusStr,
                          NULL };
        dwError = VmDirAttrListToNewEntry(pSchemaCtx, LDAPRPC_PING_DN, ppAttrs, FALSE, &pEntry);
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
VmDirVoteReplyEntry(
    PVDIR_RAFT_VOTE_CONTROL_VALUE pCvcv,
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

    if (gpClusterState == NULL || gpClusterState->bEnabled == FALSE)
    {
        //This server hasn't enabled the signle master feature.
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_UNWILLING_TO_PERFORM);
    }

    dwError = _VmDirVoteReply(pCvcv->term, pCvcv->pszCandidateId, &currentTerm, &voteGranted);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA(currentTermStr, sizeof(currentTermStr), "%u", currentTerm);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA(voteGrantedStr, sizeof(voteGrantedStr), "%llu", voteGranted);
    BAIL_ON_VMDIR_ERROR(dwError);

    {
        PSTR ppAttrs[] = {ATTR_DN, LDAPRPC_VOTE_DN,
                          ATTR_CN, "Vote",
                          ATTR_OBJECT_CLASS, OC_CLUSTER_STATE,
                          ATTR_RAFT_CURRENT_TERM, currentTermStr,
                          ATTR_RAFT_VOTE_GRANTED, voteGrantedStr,
                          NULL };
        dwError = VmDirAttrListToNewEntry(pSchemaCtx, LDAPRPC_VOTE_DN, ppAttrs, FALSE, &pEntry);
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

/*
 * Follower side LDAP based RPC handler
 * This function processes request sent by the leader
 */
static
DWORD
_VmDirPingReply(
    unsigned int term,
    PCSTR pLeader,
    unsigned int *currentTerm,
    unsigned long long  *status
    )
{
    DWORD dwError = 0;
    int oldTerm = 0;
    int newTerm = 0;
    BOOLEAN bInLock = FALSE;
    static unsigned int got_ping_cnt = 0;

    VMDIR_RWLOCK_WRITELOCK(bInLock, gpClusterState->pRWLock, 0);
    if (!gRaftState.initialized)
    {
        dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    oldTerm = gRaftState.currentTerm;
    if (term >= gRaftState.currentTerm)
    {
        gRaftState.role = VDIR_RAFT_ROLE_FOLLOWER;
        newTerm = gRaftState.currentTerm = term;
        gRaftState.lastPingRecvTime = VmDirGetTimeInMilliSec();
        if (gRaftState.leader.lberbv.bv_len == 0 ||
            VmDirStringCompareA(gRaftState.leader.lberbv.bv_val, pLeader, FALSE) !=0)
        {
            if (gRaftState.leader.lberbv.bv_len > 0)
            {
                VmDirFreeBervalContent(&gRaftState.leader);
            }
            dwError = VmDirAllocateBerValueAVsnprintf(&gRaftState.leader, "%s", pLeader);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    } else
    {
       //The ping is not sent from legitimate leader, tell the peer to become follower
       newTerm = gRaftState.currentTerm;
    }

    *status = 0;
    *currentTerm = newTerm;

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, gpClusterState->pRWLock);
    if (!dwError && newTerm > oldTerm)
    {
        VmDirPersistTerm(term);
    }

    if (!dwError && (got_ping_cnt++ % 20 == 0))
    {
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: got Ping from %s, term %d newTerm %d",
          __func__, pLeader, term, newTerm);
    }
    return dwError;

error:
    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: got Ping from %s term %d error: %d",
       __func__, pLeader, gRaftState.currentTerm, dwError);
    goto cleanup;
}

/*
 * Leader side LDAP based RPC handler
 * This function processes reply sent by follwer
 */
static
VOID
_VmHandlePingReply(PVMDIR_PING_REPLY pPingReply)
{
    DWORD dwError = 0;
    int oldTerm = 0;
    int newTerm = 0;

    if (pPingReply == NULL || pPingReply->fromFqdn.lberbv_val == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (pPingReply->dwError)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, pPingReply->dwError);
    }

    oldTerm = gRaftState.currentTerm;
    if (pPingReply->currentTerm > gRaftState.currentTerm)
    {
        gRaftState.role = VDIR_RAFT_ROLE_FOLLOWER;
        newTerm = gRaftState.currentTerm = pPingReply->currentTerm;
        gRaftState.lastPingRecvTime = VmDirGetTimeInMilliSec();
        VmDirPersistTerm(newTerm);
        goto cleanup;
    }

    //_VmDirEvaluateRaftState will evaluae server' state.

cleanup:
    if (!dwError)
    {
      VMDIR_LOG_DEBUG(LDAP_DEBUG_RPC, "%s: got reply from %s, currentTerm %d raftStatus %llu localUsn %llu",
        __func__, pPingReply->fromFqdn.lberbv_val, pPingReply->currentTerm, pPingReply->raftStatus, pPingReply->localUSN);
    }
    return;

error:
    VMDIR_LOG_ERROR(LDAP_DEBUG_RPC, "%s: reply from %s error %d", __func__,
      (pPingReply && pPingReply->fromFqdn.lberbv_val)?pPingReply->fromFqdn.lberbv_val:"unknown", dwError);
    goto cleanup;
}

//Send Requests to peers, and return rigthaway when enought grants (or denies) have received.
static
VOID
_VmDirSendNodeVote(
    BOOLEAN *pbDelayVote)
{
    DWORD dwError = 0;
    BOOLEAN bInLock = FALSE;
    VMDIR_VOTE_REPLY voteReply = {0};
    int term = 0;
    BOOLEAN bDoneVote = FALSE;
    BOOLEAN bDelayVote = FALSE;
    int total_votes_sent = 0;
    int i = 0;

    VMDIR_RWLOCK_WRITELOCK(bInLock, gpClusterState->pRWLock, 0);
    term = ++gRaftState.currentTerm;
    gRaftState.voteConsensusTerm = term;
    gRaftState.voteConsensusCnt = 1; //vote for self
    gRaftState.voteDeniedCnt = 0;
    if (gRaftState.votedFor.lberbv_len > 0)
    {
        VmDirFreeBervalContent(&gRaftState.votedFor);
    }
    dwError = VmDirAllocateBerValueAVsnprintf(&gRaftState.votedFor, "%s",
                gpClusterState->pNodeSelf->pszFQDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_RWLOCK_UNLOCK(bInLock, gpClusterState->pRWLock);

    VmDirPersistTerm(term);

    VMDIR_RWLOCK_READLOCK(bInLock, gpClusterState->pRWLock, 0);
    if (gRaftState.role != VDIR_RAFT_ROLE_CANDIDATE || gRaftState.currentTerm > term)
    {

        //Server's role or term changed during persist the new term
        // Will evaluate the server's state
        goto cleanup;
    }

    if (!gpClusterState->pSiteListSelf)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: SiteListSelf is not set, error %d", __func__, VMDIR_ERROR_INVALID_STATE);
        goto cleanup;
    }

    for (i=0; i < gpClusterState->pSiteListSelf->dwArySize; i++)
    {
        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            goto cleanup;
        }

        if (gpClusterState->pSiteListSelf->ppNodeStateAry[i]==NULL)
        {
            break;
        }

        if (!gpClusterState->pSiteListSelf->ppNodeStateAry[i]->bIsActive ||
             gpClusterState->pSiteListSelf->ppNodeStateAry[i]->bIsSelf)
        {
            continue;
        }

        gpClusterState->pSiteListSelf->ppNodeStateAry[i]->bCtlSent = FALSE;
        _VmDirSendVoteCtl(gpClusterState->pSiteListSelf->ppNodeStateAry[i], term);
        if (gpClusterState->pSiteListSelf->ppNodeStateAry[i]->bCtlSent)
        {
            total_votes_sent++;
        }
    }

    if (total_votes_sent < (gRaftState.clusterSize/2))
    {
        //Not enough peers to get consensus, need to initiate vote at later time.
        bDelayVote = TRUE;
        goto cleanup;
    }

    for (i=0; i < gpClusterState->pSiteListSelf->dwArySize; i++)
    {
        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            goto cleanup;
        }

        if (gpClusterState->pSiteListSelf->ppNodeStateAry[i] == NULL)
        {
            break;
        }

        if (gpClusterState->pSiteListSelf->ppNodeStateAry[i]->bIsSelf ||
            !gpClusterState->pSiteListSelf->ppNodeStateAry[i]->bIsActive ||
            !gpClusterState->pSiteListSelf->ppNodeStateAry[i]->bCtlSent)
        {
            //Don't try to get search result if the request wasn't send out.
            continue;
        }

        _VmDirGetVoteResult(gpClusterState->pSiteListSelf->ppNodeStateAry[i], &voteReply);
        if (voteReply.dwError == VMDIR_ERROR_UNWILLING_TO_PERFORM)
        {
            //The peer has not enabled this feature, or in progress doing upgrade.
            // Delay vote, and avoid wasting term numbers
            VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
              "%s: peer %s appears supporting RegionalMaster feature by bindary, but may have not enabled it",
              __func__, voteReply.fromFqdn.lberbv_val);
            bDelayVote = TRUE;
            goto cleanup;
        }

        _VmHandleVoteReply(&voteReply, &bDoneVote);
        if (bDoneVote)
        {
            break;
        }
    }

    VMDIR_RWLOCK_UNLOCK(bInLock, gpClusterState->pRWLock);

cleanup:
    *pbDelayVote = bDelayVote;
    VmDirFreeBervalContent(&voteReply.fromFqdn);
    VMDIR_RWLOCK_UNLOCK(bInLock, gpClusterState->pRWLock);
    return;

error:
    goto cleanup;
}

//Evalute the node state when the timer expired, and
// determine what action (send Ping, RequestVote or None) to take.
// If action is None, then determine the next waitMS for the timer.
static
VOID
_VmDirEvaluateRaftState(
   UINT64 *waitMS
   )
{
    UINT64 now = {0};

    gRaftState.cmd = ExecNone; //No command to execute by default.

    if (gRaftState.clusterSize <= 1)
    {
        //Not promoted or Raft state have not been initialized.
        *waitMS = gRaftCfg.dwRaftElectionTimeoutMS;
        goto done;
    }

    now = VmDirGetTimeInMilliSec();
    if (gRaftState.role == VDIR_RAFT_ROLE_FOLLOWER)
    {
        UINT64 timeSinceLastPingRecv = now - gRaftState.lastPingRecvTime;
        if (timeSinceLastPingRecv >= gRaftCfg.dwRaftElectionTimeoutMS)
        {
           gRaftState.cmd = ExecReqestVote;
           gRaftState.role = VDIR_RAFT_ROLE_CANDIDATE;
           *waitMS = 0;
        } else
        {
            gRaftState.cmd = ExecNone;
            *waitMS = gRaftCfg.dwRaftElectionTimeoutMS - timeSinceLastPingRecv;
        }
    } else if (gRaftState.role == VDIR_RAFT_ROLE_LEADER)
    {
        UINT64 timeSinceLastPingSent = now - gRaftState.lastPingSendTime;
        if (timeSinceLastPingSent >= gRaftCfg.dwRaftPingIntervalMS)
        {
           gRaftState.cmd = ExecPing;
           gRaftState.lastPingSendTime = now;
           *waitMS = 0;
        } else
        {
           gRaftState.cmd = ExecNone;
           *waitMS = gRaftCfg.dwRaftPingIntervalMS - timeSinceLastPingSent;
        }
    } else if(gRaftState.role == VDIR_RAFT_ROLE_CANDIDATE)
    {
        //Previous had a split vote and has waited for a random time
        gRaftState.cmd = ExecReqestVote;
        *waitMS = 0;
    } else
    {
        *waitMS = (UINT64)100;
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: invalid RAFT role %d", __func__, gRaftState.role);
    }

done:
    return;
}

//Evalue RPC (Ping or RequestVote) result
// once enough the RPC calls have been responded,
// or without enough calls being responded within the timeout peroid.
static
VOID
_VmDirEvaluateRpcResult(
   UINT64 *waitMS
   )
{
    UINT64 now = {0};
    UINT64 uWaitTime = 0;

    now = VmDirGetTimeInMilliSec();

    if (gRaftState.cmd == ExecReqestVote)
    {
       if (gRaftState.role != VDIR_RAFT_ROLE_CANDIDATE)
       {
           //Become follower via other means
           goto done;
       }
       if(gRaftState.currentTerm != gRaftState.voteConsensusTerm ||
         gRaftState.voteConsensusCnt < (gRaftState.clusterSize/2 + 1))
       {
            //Split vote; wait randomly with a mean value dwRaftPingIntervalMS
            uWaitTime = (UINT64)(rand()%(gRaftCfg.dwRaftPingIntervalMS>>1));
            goto done;
       }

       //Got majority of votes.
       gRaftState.role = VDIR_RAFT_ROLE_LEADER;
       gRaftState.lastPingSendTime = 0; //Send ping right way.
       uWaitTime = 0;
    } else if (gRaftState.cmd == ExecPing)
    {
       if (gRaftState.role == VDIR_RAFT_ROLE_FOLLOWER)
       {
           //Become follower via other means.
           uWaitTime = now - gRaftState.lastPingRecvTime;
           if(uWaitTime < gRaftCfg.dwRaftElectionTimeoutMS)
           {
               uWaitTime = gRaftCfg.dwRaftElectionTimeoutMS - uWaitTime;
           }
       } else if (gRaftState.role == VDIR_RAFT_ROLE_LEADER)
       {
           //Stay as a leader
           uWaitTime = now - gRaftState.lastPingSendTime;
           if (uWaitTime < gRaftCfg.dwRaftPingIntervalMS)
           {
              uWaitTime = gRaftCfg.dwRaftPingIntervalMS - uWaitTime;
           }
       }
       //Become candidate via other means, uWaitTime is default to 0
    } else if (gRaftState.cmd == ExecNone)
    {
       VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: invalid RAFT state cmd %d role %d",
         __func__, gRaftState.cmd, gRaftState.role);
       assert(0);
    }

done:
    return;
}

//Retrieve RequestVote result from LDAP client session.
static
VOID
_VmDirGetVoteResult(
    PVMDIR_NODE_STATE pNode,
    PVMDIR_VOTE_REPLY pVoteReply)
{
    DWORD dwError = 0;

    VDIR_BERVALUE attrCurrentTermValue = {0};
    VDIR_BERVALUE attrVoteGranted = {0};

    LdapRpcResult lrr[] =  {{ATTR_RAFT_CURRENT_TERM, &attrCurrentTermValue},
                            {ATTR_RAFT_VOTE_GRANTED, &attrVoteGranted},
                            {0}};

   dwError = VmDirAllocateBerValueAVsnprintf(&pVoteReply->fromFqdn, "%s", pNode->pszFQDN);
   BAIL_ON_VMDIR_ERROR(dwError);

   dwError = pVoteReply->dwError = _VmDirGetLdapResult(pNode, lrr);
   BAIL_ON_VMDIR_ERROR(dwError);

   pVoteReply->voteGranted=VmDirStringToIA(attrVoteGranted.lberbv_val);
   pVoteReply->currentTerm=VmDirStringToIA(attrCurrentTermValue.lberbv_val);

cleanup:
   VmDirFreeBervalContent(&attrVoteGranted);
   VmDirFreeBervalContent(&attrCurrentTermValue);
   return;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error %d", __FUNCTION__, dwError);
   goto cleanup;
}

//Handle RequestVote Reply at RPC sending side.
static
VOID
_VmHandleVoteReply(
    PVMDIR_VOTE_REPLY pVoteReply,
    BOOLEAN *pbDoneVote)
{
    DWORD dwError = 0;
    int oldTerm = 0;
    int newTerm = 0;

    *pbDoneVote = FALSE;
    if (pVoteReply == NULL || pVoteReply->fromFqdn.lberbv_val == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (pVoteReply->dwError)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, pVoteReply->dwError);
    }

    oldTerm = gRaftState.currentTerm;

    if(pVoteReply->currentTerm > gRaftState.currentTerm)
    {
        gRaftState.role = VDIR_RAFT_ROLE_FOLLOWER;
        gRaftState.lastPingRecvTime = VmDirGetTimeInMilliSec();
        newTerm = gRaftState.currentTerm = pVoteReply->currentTerm;
        //The server forfeits the vote.
        goto cleanup;
    }

    //Now we need to check whether the vote is granted.
    if (pVoteReply->voteGranted == 0)
    {
        //Vote granted by peer
        gRaftState.voteConsensusCnt++;
    } else
    {
        //Vote denied by peer
        gRaftState.voteDeniedCnt++;
    }

    if (gRaftState.voteConsensusCnt >= (gRaftState.clusterSize/2 + 1)
        || ((gRaftState.voteConsensusCnt + gRaftState.voteDeniedCnt) >= gRaftState.clusterSize))
    {
        //We can conclude the vote result for this term.
        *pbDoneVote = TRUE;
    }

cleanup:
    if (newTerm > oldTerm)
    {
        VmDirPersistTerm(newTerm);
    }

    if (!dwError)
    {
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: got reply from %s, peer's term %d, server's term %d, granted %d",
          __func__, pVoteReply->fromFqdn.lberbv_val, pVoteReply->currentTerm,
          gRaftState.currentTerm, pVoteReply->voteGranted);
    }
    return;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: reply for %s server term %d  server role %d error %d", __func__,
      (pVoteReply && pVoteReply->fromFqdn.lberbv_val)?pVoteReply->fromFqdn.lberbv_val:"unknown",
      gRaftState.currentTerm, gRaftState.role, dwError);
    goto cleanup;
}

//Send RequestVote control with LDAP search
static
VOID
_VmDirSendVoteCtl(
    PVMDIR_NODE_STATE pNode,
    int term
    )
{
    DWORD dwError = 0;
    PSTR  ppszAttrs[] = {ATTR_RAFT_CURRENT_TERM, ATTR_RAFT_VOTE_GRANTED, NULL };
    LDAPControl     ldapCtr = {0};
    LDAPControl*    srvCtrls[2] = {&ldapCtr, NULL};

    dwError = VmDirCreateRaftVoteCtrlContent(
                gpClusterState->pNodeSelf->pszFQDN,     // CandiateId
                term,
                &ldapCtr);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirSendStateCtl(pNode, LDAPRPC_VOTE_DN, ppszAttrs, srvCtrls);
    BAIL_ON_VMDIR_ERROR(dwError);

    pNode->bCtlSent = TRUE;
cleanup:
    VmDirFreeCtrlContent(&ldapCtr);
    return;

error:
    goto cleanup;

}

//Send control for with Ping or RequestVote
static
DWORD
_VmDirSendStateCtl(
    PVMDIR_NODE_STATE pNode,
    PSTR dn,
    PSTR  *ppszAttrs,
    LDAPControl **srvCtrls
    )
{
    DWORD dwError = 0;
    BOOLEAN bInLock = FALSE;
    LDAPMessage*    pSearchRes = NULL;
    uint64_t        iTimeNow = 0;
    PVMDIR_LDP_CONN pLdpConn = &pNode->nodeLDP;
    int msgid = 0;
    int ret = 0;
    PSTR  pszFilter = "(objectclass=*)";

    if (!pLdpConn)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_PARAMETER);
    }

    if(pLdpConn->pLd == NULL ||
       pLdpConn->connState != DC_CONNECTION_STATE_CONNECTED)
    {
        if (pLdpConn->connState == DC_CONNECTION_STATE_CONNECTING)
        {
            //A newly added node has its state DC_CONNECTION_STATE_CONNECTING
            VMDIR_LOCK_MUTEX(bInLock, gConnStateMutex);
            VmDirConditionSignal(gConnLostCond);
            VMDIR_UNLOCK_MUTEX(bInLock, gConnStateMutex);
        }
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_SERVER_DOWN);
    }

    pLdpConn->msgid = -1;
    ret = ldap_search_ext(
                pLdpConn->pLd,
                dn,
                LDAP_SCOPE_BASE,
                pszFilter,
                &ppszAttrs[0],
                TRUE,
                srvCtrls, // cluster state control
                NULL,
                NULL,
                1,
                &msgid
                );

    if (ret)
    {
        VDIR_SAFE_UNBIND_EXT_S(pLdpConn->pLd);

        VMDIR_LOCK_MUTEX(bInLock, gConnStateMutex);
        pLdpConn->connState = DC_CONNECTION_STATE_CONNECTING;
        pLdpConn->dwLdapError = dwError;
        pLdpConn->iLastErrorTimeStamp = iTimeNow;
        VmDirConditionSignal(gConnLostCond);
        VMDIR_UNLOCK_MUTEX(bInLock, gConnStateMutex);

        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_SERVER_DOWN);
    }

    iTimeNow = time(NULL);
    pLdpConn->iLastSendTimeStamp = iTimeNow;
    pLdpConn->msgid = msgid;

    VMDIR_LOG_DEBUG(LDAP_DEBUG_RPC, "%s: succeeded sending cluste control to %s dn %d",
                   __FUNCTION__, pNode->pszFQDN, dn);
cleanup:
    ldap_msgfree(pSearchRes);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s, send %s cluster ctl with dn %s error %d",
            __FUNCTION__, pNode->pszFQDN, dn, dwError);
    goto cleanup;
}

//LDAPRPC handler for RequestVote at reciever side
static
DWORD
_VmDirVoteReply(
    unsigned int term,
    PCSTR candidateId,
    unsigned int *currentTerm,
    unsigned int  *voteGranted
    )
{
    DWORD dwError = 0;
    int oldTerm = 0;
    int newTerm = 0;
    BOOLEAN bInLock = FALSE;

    VMDIR_RWLOCK_WRITELOCK(bInLock, gpClusterState->pRWLock, 0);

    *voteGranted = 1; //default is to deny vote request.
    if (!gRaftState.initialized)
    {
        dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    oldTerm = gRaftState.currentTerm;
    if (term > gRaftState.currentTerm)
    {
        newTerm = gRaftState.currentTerm = term;
        gRaftState.role = VDIR_RAFT_ROLE_FOLLOWER;
        gRaftState.lastPingRecvTime = VmDirGetTimeInMilliSec();
        *voteGranted = 0; //Grant the vote request.

        if (gRaftState.votedFor.lberbv_len > 0)
        {
            VmDirFreeBervalContent(&gRaftState.votedFor);
        }

        dwError = VmDirAllocateBerValueAVsnprintf(&gRaftState.votedFor, "%s", candidateId);
        BAIL_ON_VMDIR_ERROR(dwError);
        goto cleanup;

    } else if (term == gRaftState.currentTerm)
    {
        newTerm = term;
        if(gRaftState.votedFor.lberbv_len > 0 &&
           VmDirStringCompareA(gRaftState.votedFor.lberbv_val, candidateId, FALSE) != 0)
        {
            //I have voted for a different requester in the same term, deny the vote.
            goto cleanup;
        } else
        {
            gRaftState.currentTerm = VDIR_RAFT_ROLE_FOLLOWER;
            gRaftState.lastPingRecvTime = VmDirGetTimeInMilliSec();
            *voteGranted = 0; //Grant the vote request.
            if (gRaftState.votedFor.lberbv_len == 0)
            {
                dwError = VmDirAllocateBerValueAVsnprintf(&gRaftState.votedFor, "%s", candidateId);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
        goto cleanup;

    } else
    {
       newTerm = gRaftState.currentTerm;
        //peer's term < my term
       *voteGranted = 2;
    }

cleanup:
    *currentTerm = newTerm;
    VMDIR_RWLOCK_UNLOCK(bInLock, gpClusterState->pRWLock);

    if (newTerm > oldTerm)
    {
        VmDirPersistTerm(newTerm);
    }
    if (!dwError)
    {
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: reply requestVote for %s, term %d, newterm %d, role %d, granted %d",
           __func__, candidateId, term, newTerm, gRaftState.role, *voteGranted);
    }
    return dwError;

error:
    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
      "%s: reply requestVote for %s term %d newTerm %d (old term %d) role %d error %d",
      __func__, candidateId, term, newTerm, oldTerm, gRaftState.role, dwError);
    goto cleanup;
}

BOOLEAN
VmDirIsRegionalMasterEnabled()
{
    DWORD dwRegionalMaster = 0;

    VmDirGetRegKeyValueDword(
        VMDIR_CONFIG_PARAMETER_V1_KEY_PATH,
        VMDIR_REG_KEY_ENABLE_REGIONAL_MASTER,
        &dwRegionalMaster, 0);

    return dwRegionalMaster;
}

static
VOID
_VmDirGetRaftParamters(VOID)
{
    DWORD dwError = 0;
    VDIR_RAFT_CFG raftCfg = {0, 0};

    dwError = VmDirGetRegKeyValueDword(
        VMDIR_CONFIG_PARAMETER_V1_KEY_PATH,
        VMDIR_REG_KEY_RAFT_ELECTION_TIMEOUT,
        &raftCfg.dwRaftElectionTimeoutMS, RaftElectionTimeoutDefault);
    if (dwError)
    {
        raftCfg.dwRaftElectionTimeoutMS = RaftElectionTimeoutDefault;
    }

    dwError = VmDirGetRegKeyValueDword(
        VMDIR_CONFIG_PARAMETER_V1_KEY_PATH,
        VMDIR_REG_KEY_RAFT_PING_INTERVAL,
        &raftCfg.dwRaftPingIntervalMS, RaftPingIntervalDefault);
    if (dwError)
    {
        raftCfg.dwRaftPingIntervalMS = RaftPingIntervalDefault;
    }

    if (raftCfg.dwRaftPingIntervalMS < RaftPingIntervalMin)
    {
        raftCfg.dwRaftPingIntervalMS = RaftPingIntervalMin;
        VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
          "%s: %s must >= %d ms set it to %d", __func__,
          VMDIR_REG_KEY_RAFT_PING_INTERVAL, RaftPingIntervalMin, raftCfg.dwRaftPingIntervalMS);
    }

    if ((float)raftCfg.dwRaftElectionTimeoutMS/(float)raftCfg.dwRaftPingIntervalMS <= 2.1)
    {
        raftCfg.dwRaftPingIntervalMS = 3*raftCfg.dwRaftPingIntervalMS;
        VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
          "%s: %s must > 2X of %s, set them to {%d, %d}",
          __func__, VMDIR_REG_KEY_RAFT_ELECTION_TIMEOUT, VMDIR_REG_KEY_RAFT_PING_INTERVAL,
          raftCfg.dwRaftElectionTimeoutMS, raftCfg.dwRaftPingIntervalMS);
    }

    gRaftCfg = raftCfg;

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
          "%s: set {%s, %s} to {%d, %d}",
          __func__, VMDIR_REG_KEY_RAFT_PING_INTERVAL, VMDIR_REG_KEY_RAFT_ELECTION_TIMEOUT,
          raftCfg.dwRaftPingIntervalMS, raftCfg.dwRaftElectionTimeoutMS);
}
