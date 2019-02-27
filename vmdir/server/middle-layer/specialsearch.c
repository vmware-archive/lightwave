/*
 * Copyright © 2012-2017 VMware, Inc.  All Rights Reserved.
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

/*
 * Return TRUE if search request require special handling.
 * If TRUE, the request will be served within this function.
 *
 * In case of error, pLdapResult->errCode/pszErrMsg will be set.
 */
BOOLEAN
VmDirHandleSpecialSearch(
    PVDIR_OPERATION    pOp,
    PVDIR_LDAP_RESULT  pLdapResult
    )
{
    DWORD   dwError = 0;
    size_t  i = 0;
    BOOLEAN bHasTxn = FALSE;
    BOOLEAN bRefresh = FALSE;
    PVDIR_ENTRY_ARRAY   pEntryArray = NULL;
    VDIR_SPECIAL_SEARCH_ENTRY_TYPE entryType = REGULAR_SEARCH_ENTRY_TYPE;
    VMDIR_INTEGRITY_CHECK_JOB_STATE integrityCheckStat = INTEGRITY_CHECK_JOB_NONE;
    PVMDIR_DB_INTEGRITY_JOB    pDBIntegrityCheckJob = NULL;

    static PCSTR pszEntryType[] =
    {
            "DSE Root",
            "Schema Entry",
            "Server Status",
            "Replication Status",
            "Schema Repl Status",
            "Integrity Check Status",
            "Cluster State Ping",
            "Cluster Vote",
            "Raft State",
            "DB Cross Check Status",
            "DB Integrity Check Status",
    };

    if ( !pOp || !pLdapResult )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pEntryArray = &pOp->internalSearchEntryArray;

    if (VmDirIsSearchForDseRootEntry( pOp ))
    {
        entryType = SPECIAL_SEARCH_ENTRY_TYPE_DSE_ROOT;
        pEntryArray->iSize = 1;

        dwError = VmDirAllocateMemory(
                sizeof(VDIR_ENTRY), (PVOID*)&pEntryArray->pEntry);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = pOp->pBEIF->pfnBESimpleIdToEntry(
                DSE_ROOT_ENTRY_ID, pEntryArray->pEntry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pLdapResult->pszErrMsg),
                "%s Entry search failed.", pszEntryType[entryType]);

        dwError = VmDirBuildComputedAttribute(pOp, pEntryArray->pEntry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pLdapResult->pszErrMsg),
                "%s Entry send failed.", pszEntryType[entryType]);
    }
    else if (VmDirIsSearchForSchemaEntry( pOp ))
    {
        entryType = SPECIAL_SEARCH_ENTRY_TYPE_SCHEMA_ENTRY;
        pEntryArray->iSize = 1;

        dwError = VmDirSubSchemaSubEntry(&pEntryArray->pEntry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pLdapResult->pszErrMsg),
                "%s Entry search failed.", pszEntryType[entryType]);
    }
    else if (VmDirIsSearchForServerStatus(pOp))
    {
        entryType = SPECIAL_SEARCH_ENTRY_TYPE_SERVER_STATUS;
        pEntryArray->iSize = 1;

        dwError = VmDirServerStatusEntry(&pEntryArray->pEntry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pLdapResult->pszErrMsg),
                "%s Entry search failed.", pszEntryType[entryType]);
    }
    else if (VmDirIsSearchForReplicationStatus(pOp))
    {
        entryType = SPECIAL_SEARCH_ENTRY_TYPE_REPL_STATUS;
        pEntryArray->iSize = 1;

        dwError = VmDirReplicationStatusEntry(&pEntryArray->pEntry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pLdapResult->pszErrMsg),
                "%s Entry search failed.", pszEntryType[entryType]);
    }
    else if (VmDirIsSearchForSchemaReplStatus(pOp, &bRefresh))
    {
        entryType = SPECIAL_SEARCH_ENTRY_TYPE_SCHEMA_REPL_STATUS;

        if (bRefresh)
        {
            dwError = VmDirSchemaReplStatusEntriesRefresh();
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pLdapResult->pszErrMsg),
                    "%s Entry refresh failed.", pszEntryType[entryType]);
        }
        else
        {
            dwError = VmDirSchemaReplStatusEntriesRetrieve(
                    pEntryArray, pOp->request.searchReq.scope);
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pLdapResult->pszErrMsg),
                    "%s Entry search failed.", pszEntryType[entryType]);
        }
    }
    else if (VmDirIsSearchForIntegrityCheckStatus(pOp, &integrityCheckStat))
    {
        BOOLEAN bIsMember = FALSE;

        entryType = SPECIAL_SEARCH_ENTRY_TYPE_INTEGRITY_CHECK_STATUS;

        dwError = VmDirIsBindDnMemberOfSystemDomainAdmins(NULL, &pOp->conn->AccessInfo, &bIsMember);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (!bIsMember)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INSUFFICIENT_ACCESS);
        }

        if (integrityCheckStat == INTEGRITY_CHECK_JOB_START ||
            integrityCheckStat == INTEGRITY_CHECK_JOB_RECHECK)
        {
            dwError = VmDirIntegrityCheckStart(integrityCheckStat, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (integrityCheckStat == INTEGRITY_CHECK_JOB_STOP)
        {
            VmDirIntegrityCheckStop();
        }
        else if (integrityCheckStat == INTEGRITY_CHECK_JOB_SHOW_SUMMARY)
        {
            dwError = VmDirIntegrityCheckShowStatus(&pEntryArray->pEntry);
            BAIL_ON_VMDIR_ERROR(dwError);

            if (pEntryArray->pEntry)
            {
                pEntryArray->iSize = 1;
            }
        }
    }
    else if (VmDirIsSearchForDBIntegrityCheckStatus(pOp, &pDBIntegrityCheckJob))
    {
        BOOLEAN bIsMember = FALSE;

        entryType = SPECIAL_SEARCH_ENTRY_TYPE_DB_INTEGRITY_CHECK_STATUS;

        dwError = VmDirIsBindDnMemberOfSystemDomainAdmins(NULL, &pOp->conn->AccessInfo, &bIsMember);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (!bIsMember)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INSUFFICIENT_ACCESS);
        }

        if (pDBIntegrityCheckJob->state == DB_INTEGRITY_CHECK_JOB_START)
        {
            dwError = VmDirDBIntegrityCheckStart(pDBIntegrityCheckJob);
            BAIL_ON_VMDIR_ERROR(dwError);

            pDBIntegrityCheckJob = NULL; //transfer ownership
        }
        else if (pDBIntegrityCheckJob->state == DB_INTEGRITY_CHECK_JOB_STOP)
        {
            VmDirDBIntegrityCheckStop();
        }
        else if (pDBIntegrityCheckJob->state == DB_INTEGRITY_CHECK_JOB_SHOW_SUMMARY)
        {
            dwError = VmDirDBIntegrityCheckShowStatus(&pEntryArray->pEntry);
            BAIL_ON_VMDIR_ERROR(dwError);

            if (pEntryArray->pEntry)
            {
                pEntryArray->iSize = 1;
            }
        }
        else if (pDBIntegrityCheckJob->state == DB_INTEGRITY_CHECK_JOB_NONE)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_PARAMETER);
        }
    }
    else if (VmDirIsSearchForDBCrossCheckStatus(pOp))
    {
        BOOLEAN bIsMember = FALSE;

        entryType = SPECIAL_SEARCH_ENTRY_TYPE_DB_CROSS_CHECK_STATUS;
        pEntryArray->iSize = 0; // nothing to send back, check vmdir log for result for now.

        // TODO, should change to use gVmdirdSDGlobals.pSDdcAdminGX
        dwError = VmDirIsBindDnMemberOfSystemDomainAdmins(NULL, &pOp->conn->AccessInfo, &bIsMember);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (!bIsMember)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INSUFFICIENT_ACCESS);
        }

        dwError = VmDirInitDBCrossChkThread();
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (VmDirIsSearchForRaftPing(pOp))
    {
        entryType = SPECIAL_SEARCH_ENTRY_TYPE_RAFT_PING;

        VMDIR_LOG_DEBUG(
                LDAP_DEBUG_RPC,
                "RAFT_PING CONTROL %s, %d",
                pOp->raftPingCtrl->value.raftPingCtrlVal.pszFQDN,
                pOp->raftPingCtrl->value.raftPingCtrlVal.term);

        dwError = VmDirPingReplyEntry(&pOp->raftPingCtrl->value.raftPingCtrlVal, &pEntryArray->pEntry);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pEntryArray->pEntry)
        {
            pEntryArray->iSize = 1;
        }
    }
    else if (VmDirIsSearchForRaftVote(pOp))
    {
        entryType = SPECIAL_SEARCH_ENTRY_TYPE_RAFT_VOTE;

        VMDIR_LOG_DEBUG(
                LDAP_DEBUG_RPC,
                "RAFT_VOTE CONTROL %s, %d",
                pOp->raftVoteCtrl->value.raftVoteCtrlVal.pszCandidateId,
                pOp->raftVoteCtrl->value.raftVoteCtrlVal.term);

        dwError = VmDirVoteReplyEntry(&pOp->raftVoteCtrl->value.raftVoteCtrlVal, &pEntryArray->pEntry);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pEntryArray->pEntry)
        {
            pEntryArray->iSize = 1;
        }
    }
    else if (VmDirIsSearchForStatePing(pOp))
    {
        PVDIR_STATE_PING_CONTROL_VALUE pStatePing = &pOp->statePingCtrl->value.statePingCtrlVal;
        PVMDIR_REPLICATION_METRICS pReplMetrics = NULL;
        USN local = 0;

        entryType = SPECIAL_SEARCH_ENTRY_TYPE_STATE_PING;
        pEntryArray->iSize = 0; // don't send anything back

        if (!IsNullOrEmptyString(pStatePing->pszFQDN) &&
            !IsNullOrEmptyString(pStatePing->pszInvocationId) &&
            VmDirReplMetricsCacheFind(pStatePing->pszFQDN, &pReplMetrics) == 0 &&
#ifdef REPLICATION_V2
            VmDirUTDVectorGlobalCacheLookup(pStatePing->pszInvocationId, &local) == 0
#else
            VmDirUTDVectorCacheLookup(pStatePing->pszInvocationId, &local) == 0
#endif
            )
        {
            if (pStatePing->maxOrigUsn >= local)
            {
                VmMetricsHistogramUpdate(pReplMetrics->pUsnBehind, pStatePing->maxOrigUsn - local);
            }
            else
            {
                VMDIR_LOG_WARNING(
                        VMDIR_LOG_MASK_ALL,
                        "received maxorigusn=%" PRId64 " from %s (%s) smaller than utdvector entry=%" PRId64,
                        pStatePing->maxOrigUsn,
                        pStatePing->pszFQDN,
                        pStatePing->pszInvocationId,
                        local);
            }
        }
    }

    if (entryType != REGULAR_SEARCH_ENTRY_TYPE)
    {
        /*
         * Read txn for preventing server crash (PR 1634501)
         */
        dwError = pOp->pBEIF->pfnBETxnBegin(pOp->pBECtx, VDIR_BACKEND_TXN_READ);
        BAIL_ON_VMDIR_ERROR(dwError);

        bHasTxn = TRUE;

        for (i = 0; i < pEntryArray->iSize; i++)
        {
            dwError = VmDirSendSearchEntry(pOp, &pEntryArray->pEntry[i]);
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pLdapResult->pszErrMsg),
                    "%s Entry send failed.", pszEntryType[entryType]);
        }
    }

cleanup:
    VmDirDBIntegrityCheckJobFree(pDBIntegrityCheckJob);
    if (bHasTxn)
    {
        pOp->pBEIF->pfnBETxnCommit(pOp->pBECtx);
    }

    return entryType != REGULAR_SEARCH_ENTRY_TYPE;

error:
    VmDirLog( LDAP_DEBUG_ANY, "VmDirHandleSpecialSearch: (%d)(%s)",
            dwError, VDIR_SAFE_STRING(pLdapResult->pszErrMsg) );
    pLdapResult->vmdirErrCode = dwError;

    goto cleanup;
}

/* From RFC 4512 (section 5.1): An LDAP server SHALL provide information about itself and other information that is
 * specific to each server. This is represented as a group of attributes located in the root DSE, which is named with
 * the DN with zero RDNs. These attributes are retrievable, subject to access control and other restrictions, if a
 * client performs a Search operation with an empty baseObject, scope of baseObject, the filter "(objectClass=*),
 * and the attributes field listing the names of the desired attributes. It is noted that root DSE attributes are
 * operational and, like other operational attributes, are not returned in search requests unless requested by name.
 * The root DSE SHALL NOT be included if the client performs a subtree search starting from the root.
*/

BOOLEAN
VmDirIsSearchForDseRootEntry(
    PVDIR_OPERATION pOp
    )
{
    BOOLEAN     bRetVal = FALSE;
    SearchReq * pSearchReq = NULL;

    pSearchReq = &(pOp->request.searchReq);

    if ( pSearchReq->scope == LDAP_SCOPE_BASE && pOp->reqDn.lberbv.bv_len == 0       &&
         pSearchReq->filter->choice == LDAP_FILTER_PRESENT                           &&
         pSearchReq->filter->filtComp.present.lberbv.bv_len == ATTR_OBJECT_CLASS_LEN &&
         pSearchReq->filter->filtComp.present.lberbv.bv_val != NULL                  &&
         VmDirStringNCompareA( ATTR_OBJECT_CLASS, pSearchReq->filter->filtComp.present.lberbv.bv_val, ATTR_OBJECT_CLASS_LEN, FALSE) == 0
       )
    {
        bRetVal = TRUE;
    }

    return bRetVal;
}

/* From RFC 4512 (section 4.4): To read schema attributes from the subschema (sub)entry, clients MUST issue a Search
 * operation where baseObject is the DN of the subschema (sub)entry, scope is baseObject, filter is
 * "(objectClass=subschema)", and the attributes field lists the names of the desired schema attributes (as they are
 * operational). Note: the "(objectClass=subschema)" filter allows LDAP servers that gateway to X.500 to detect that
 * subentry information is being requested.
*/

BOOLEAN
VmDirIsSearchForSchemaEntry(
    PVDIR_OPERATION  pOp
    )
{
    BOOLEAN     bRetVal = FALSE;
    SearchReq * pSearchReq = NULL;

    pSearchReq = &(pOp->request.searchReq);

    // scope must be base
    if (pSearchReq->scope == LDAP_SCOPE_BASE &&
        VmDirStringCompareA( pOp->reqDn.lberbv.bv_val, SUB_SCHEMA_SUB_ENTRY_DN, FALSE) == 0)
    {
        // filter can be (objectClass=subschema)
        if (pSearchReq->filter->choice == LDAP_FILTER_EQUALITY                              &&
            pSearchReq->filter->filtComp.ava.type.lberbv.bv_len == ATTR_OBJECT_CLASS_LEN    &&
            pSearchReq->filter->filtComp.ava.type.lberbv.bv_val != NULL                     &&
            VmDirStringNCompareA( ATTR_OBJECT_CLASS, pSearchReq->filter->filtComp.ava.type.lberbv.bv_val, ATTR_OBJECT_CLASS_LEN, FALSE) == 0 &&
            pSearchReq->filter->filtComp.ava.value.lberbv.bv_len == OC_SUB_SCHEMA_LEN       &&
            pSearchReq->filter->filtComp.ava.value.lberbv.bv_val != NULL                    &&
            VmDirStringNCompareA( OC_SUB_SCHEMA, pSearchReq->filter->filtComp.ava.value.lberbv.bv_val, OC_SUB_SCHEMA_LEN, FALSE) == 0
            )
        {
            bRetVal = TRUE;
        }
        // filter can be (objectClass=*)
        else if (pSearchReq->filter->choice == LDAP_FILTER_PRESENT                              &&
                 pSearchReq->filter->filtComp.present.lberbv.bv_len == ATTR_OBJECT_CLASS_LEN    &&
                 pSearchReq->filter->filtComp.present.lberbv.bv_val != NULL                     &&
                 VmDirStringNCompareA( ATTR_OBJECT_CLASS, pSearchReq->filter->filtComp.present.lberbv.bv_val, ATTR_OBJECT_CLASS_LEN, FALSE) == 0
                 )
        {
            bRetVal = TRUE;
        }
    }

    return bRetVal;
}

/*
 * For server runtime status
 * The search pattern is :
 * BASE:    cn=serverstatus
 * SCOPE:   BASE
 * FILTER:  (objectclass=*)
 */
BOOLEAN
VmDirIsSearchForServerStatus(
    PVDIR_OPERATION     pOp
    )
{
    BOOLEAN         bRetVal = FALSE;
    SearchReq*      pSearchReq = NULL;
    PVDIR_FILTER    pFilter = NULL;

    pSearchReq = &(pOp->request.searchReq);
    pFilter = pSearchReq->filter;

    assert( pFilter != NULL );

    if (pSearchReq->scope == LDAP_SCOPE_BASE
        &&
        (
         pOp->reqDn.lberbv.bv_val != NULL                                               &&
         VmDirStringCompareA(pOp->reqDn.lberbv.bv_val, SERVER_STATUS_DN, FALSE) == 0
        )
        &&
        (
         pFilter->choice == LDAP_FILTER_PRESENT                                         &&
         pFilter->filtComp.present.lberbv.bv_len == ATTR_OBJECT_CLASS_LEN               &&
         pFilter->filtComp.present.lberbv.bv_val != NULL                                &&
         VmDirStringNCompareA( ATTR_OBJECT_CLASS, pFilter->filtComp.present.lberbv.bv_val, ATTR_OBJECT_CLASS_LEN, FALSE) == 0
        )
        )
    {
        bRetVal = TRUE;
    }

    return bRetVal;

}

/*
 * For replication runtime status
 * The search pattern is :
 * BASE:    cn=replicationstatus
 * SCOPE:   BASE
 * FILTER:  (objectclass=*)
 */
BOOLEAN
VmDirIsSearchForReplicationStatus(
    PVDIR_OPERATION     pOp
    )
{
    BOOLEAN         bRetVal = FALSE;
    SearchReq*      pSearchReq = &(pOp->request.searchReq);
    PVDIR_FILTER    pFilter = pSearchReq ? pSearchReq->filter: NULL;

    if (pSearchReq != NULL
        &&
        pFilter != NULL
        &&
        pSearchReq->scope == LDAP_SCOPE_BASE
        &&
        (
         pOp->reqDn.lberbv.bv_val != NULL                                               &&
         VmDirStringCompareA(pOp->reqDn.lberbv.bv_val, REPLICATION_STATUS_DN, FALSE) == 0
        )
        &&
        (
         pFilter->choice == LDAP_FILTER_PRESENT                                         &&
         pFilter->filtComp.present.lberbv.bv_len == ATTR_OBJECT_CLASS_LEN               &&
         pFilter->filtComp.present.lberbv.bv_val != NULL                                &&
         VmDirStringNCompareA( ATTR_OBJECT_CLASS, pFilter->filtComp.present.lberbv.bv_val, ATTR_OBJECT_CLASS_LEN, FALSE) == 0
        )
        )
    {
        bRetVal = TRUE;
    }

    return bRetVal;

}

/*
 * For schema replication status
 * The search pattern is:
 * BASE:    cn=schemareplstatus
 * FILTER:  (objectclass=*)
 * SCOPE:   BASE - returns the pseudo entry that tells if refresh is in progress
 *          ONELEVEL - returns the status entries
 *          SUBTREE - returns the pseudo entry + status entries
 *
 * Optional:
 * ATTRS:   refresh - trigger server to refresh schema replication status
 */
BOOLEAN
VmDirIsSearchForSchemaReplStatus(
    PVDIR_OPERATION     pOp,
    PBOOLEAN            pbRefresh
    )
{
    BOOLEAN         bRetVal = FALSE;
    SearchReq*      pSearchReq = &(pOp->request.searchReq);
    PSTR            pszDN = pOp->reqDn.lberbv.bv_val;
    PVDIR_FILTER    pFilter = pSearchReq ? pSearchReq->filter : NULL;

    if (pSearchReq != NULL                                                  &&
        pszDN != NULL                                                       &&
        VmDirStringCompareA(pszDN, SCHEMA_REPL_STATUS_DN, FALSE) == 0       &&
        pFilter != NULL                                                     &&
        pFilter->choice == LDAP_FILTER_PRESENT                              &&
        pFilter->filtComp.present.lberbv.bv_len == ATTR_OBJECT_CLASS_LEN    &&
        pFilter->filtComp.present.lberbv.bv_val != NULL                     &&
        VmDirStringNCompareA(ATTR_OBJECT_CLASS, pFilter->filtComp.present.lberbv.bv_val, ATTR_OBJECT_CLASS_LEN, FALSE) == 0)
    {
        bRetVal = TRUE;
    }

    if (pbRefresh && pSearchReq->attrs)
    {
        PSTR pszAttr = pSearchReq->attrs[0].lberbv.bv_val;
        if (VmDirStringCompareA(pszAttr, "refresh", FALSE) == 0)
        {
            *pbRefresh = TRUE;
        }
    }

    return bRetVal;
}

/*
 * For integrity check status
 * The search pattern is:
 * BASE:    cn=integritycheckstatus
 * FILTER:  (objectclass=*)
 * SCOPE:   BASE - with optional attribute operation=start|stop|recheck
 *          ONELEVEL - returns job summary with optional attribute detail
 *
 */
BOOLEAN
VmDirIsSearchForIntegrityCheckStatus(
    PVDIR_OPERATION                     pOp,
    PVMDIR_INTEGRITY_CHECK_JOB_STATE    pState
    )
{
    BOOLEAN         bRetVal = FALSE;
    SearchReq*      pSearchReq = &(pOp->request.searchReq);
    PSTR            pszDN = pOp->reqDn.lberbv.bv_val;
    PVDIR_FILTER    pFilter = pSearchReq ? pSearchReq->filter : NULL;

    if (pSearchReq != NULL                                                  &&
        pszDN != NULL                                                       &&
        VmDirStringCompareA(pszDN, INTEGRITY_CHECK_STATUS_DN, FALSE) == 0   &&
        pFilter != NULL                                                     &&
        pFilter->choice == LDAP_FILTER_PRESENT                              &&
        pFilter->filtComp.present.lberbv.bv_len == ATTR_OBJECT_CLASS_LEN    &&
        pFilter->filtComp.present.lberbv.bv_val != NULL                     &&
        VmDirStringNCompareA(ATTR_OBJECT_CLASS, pFilter->filtComp.present.lberbv.bv_val, ATTR_OBJECT_CLASS_LEN, FALSE) == 0)
    {
        bRetVal = TRUE;
    }

    if (pSearchReq->scope == LDAP_SCOPE_BASE && pState && pSearchReq->attrs)
    {
        PSTR pszAttr = pSearchReq->attrs[0].lberbv.bv_val;
        if (VmDirStringCompareA(pszAttr, "start", FALSE) == 0)
        {
            *pState = INTEGRITY_CHECK_JOB_START;
        }
        else if (VmDirStringCompareA(pszAttr, "stop", FALSE) == 0)
        {
            *pState = INTEGRITY_CHECK_JOB_STOP;
        }
        else if (VmDirStringCompareA(pszAttr, "recheck", FALSE) == 0)
        {
            *pState = INTEGRITY_CHECK_JOB_RECHECK;
        }
        else
        {
            *pState = INTEGRITY_CHECK_JOB_NONE;
        }
    }
    else if (pSearchReq->scope == LDAP_SCOPE_ONELEVEL && pState )
    {
        *pState = INTEGRITY_CHECK_JOB_SHOW_SUMMARY;
    }

    return bRetVal;
}

BOOLEAN
VmDirIsSearchForDBIntegrityCheckStatus(
    PVDIR_OPERATION            pOperation,
    PVMDIR_DB_INTEGRITY_JOB*   ppDBIntegrityCheckJob
    )
{
    DWORD                      dwError = 0;
    PSTR                       pszDN = NULL;
    BOOLEAN                    bRetVal = FALSE;
    SearchReq*                 pSearchReq = NULL;
    PVDIR_FILTER               pFilter = NULL;
    PVMDIR_DB_INTEGRITY_JOB    pDBIntegrityCheckJob = NULL;

    if (!pOperation || !ppDBIntegrityCheckJob)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pszDN = pOperation->reqDn.lberbv.bv_val;
    pSearchReq = &(pOperation->request.searchReq);
    pFilter = pSearchReq ? pSearchReq->filter : NULL;

    if (pszDN && VmDirStringCompareA(pszDN, DB_INTEGRITY_CHECK_STATUS_DN, FALSE) == 0 &&
        pSearchReq && pFilter && pFilter->choice == LDAP_FILTER_PRESENT               &&
        pFilter->filtComp.present.lberbv.bv_val                                       &&
        VmDirStringNCompareA(
            ATTR_OBJECT_CLASS,
            pFilter->filtComp.present.lberbv.bv_val,
            ATTR_OBJECT_CLASS_LEN,
            FALSE) == 0)
    {
        bRetVal = TRUE;

        dwError = VmDirAllocateMemory(sizeof(VMDIR_DB_INTEGRITY_JOB), (PVOID*)&pDBIntegrityCheckJob);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pSearchReq->scope == LDAP_SCOPE_BASE && pSearchReq->attrs)
        {
            PSTR    pszState = pSearchReq->attrs[0].lberbv.bv_val;
            PSTR    pszCmd = pSearchReq->attrs[1].lberbv.bv_val;

            if (VmDirStringCompareA(pszState, "start", FALSE) == 0)
            {
                pDBIntegrityCheckJob->state = DB_INTEGRITY_CHECK_JOB_START;

                if (pszCmd == NULL)
                {
                    pDBIntegrityCheckJob->command = DB_INTEGRITY_CHECK_LIST;
                }
                else if (VmDirStringCompareA(pszCmd, "all", FALSE) == 0)
                {
                    pDBIntegrityCheckJob->command = DB_INTEGRITY_CHECK_ALL;
                }
                else
                {
                    pDBIntegrityCheckJob->command = DB_INTEGRITY_CHECK_SUBDB;

                    dwError = VmDirAllocateStringA(pszCmd, &pDBIntegrityCheckJob->pszDBName);
                    BAIL_ON_VMDIR_ERROR(dwError);
                }
            }
            else if (VmDirStringCompareA(pszState, "stop", FALSE) == 0)
            {
                pDBIntegrityCheckJob->state = DB_INTEGRITY_CHECK_JOB_STOP;
            }
            else
            {
                pDBIntegrityCheckJob->state = DB_INTEGRITY_CHECK_JOB_NONE;
            }
        }
        else if (pSearchReq->scope == LDAP_SCOPE_ONELEVEL)
        {
            pDBIntegrityCheckJob->state = DB_INTEGRITY_CHECK_JOB_SHOW_SUMMARY;
        }

        *ppDBIntegrityCheckJob = pDBIntegrityCheckJob;
        pDBIntegrityCheckJob = NULL;
    }

cleanup:
    VmDirDBIntegrityCheckJobFree(pDBIntegrityCheckJob);
    return bRetVal;

error:
    bRetVal = FALSE;
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

/*
 * For integrity check status
 * The search pattern is:
 * BASE:    cn=dbcrosscheckstatus
 * FILTER:  (objectclass=*)
 * SCOPE:   BASE
 *
 */
BOOLEAN
VmDirIsSearchForDBCrossCheckStatus(
    PVDIR_OPERATION                     pOp
    )
{
    BOOLEAN         bRetVal = FALSE;
    SearchReq*      pSearchReq = &(pOp->request.searchReq);
    PSTR            pszDN = pOp->reqDn.lberbv.bv_val;
    PVDIR_FILTER    pFilter = pSearchReq ? pSearchReq->filter : NULL;

    if (pSearchReq != NULL                                                  &&
        pszDN != NULL                                                       &&
        VmDirStringCompareA(pszDN, DB_CROSS_CHECK_STATUS_DN, FALSE) == 0    &&
        pFilter != NULL                                                     &&
        pFilter->choice == LDAP_FILTER_PRESENT                              &&
        pFilter->filtComp.present.lberbv.bv_len == ATTR_OBJECT_CLASS_LEN    &&
        pFilter->filtComp.present.lberbv.bv_val != NULL                     &&
        VmDirStringNCompareA(ATTR_OBJECT_CLASS, pFilter->filtComp.present.lberbv.bv_val, ATTR_OBJECT_CLASS_LEN, FALSE) == 0)
    {
        bRetVal = TRUE;
    }

    return bRetVal;
}

/*
 * For cluster state
 * The search pattern is:
 * BASE:    cn=clusterstate
 * FILTER:  (objectclass=*)
 * SCOPE:   BASE
 * Control: PVDIR_RAFT_PING_CONTROL_VALUE
 *
 */
// should it be special write controL?
BOOLEAN
VmDirIsSearchForRaftPing(
    PVDIR_OPERATION     pOp
    )
{
    BOOLEAN         bRetVal = FALSE;
    SearchReq*      pSearchReq = &(pOp->request.searchReq);
    PSTR            pszDN = pOp->reqDn.lberbv.bv_val;
    PVDIR_FILTER    pFilter = pSearchReq ? pSearchReq->filter : NULL;

    if (pSearchReq != NULL                                                  &&
        pszDN != NULL                                                       &&
        VmDirStringCompareA(pszDN, LDAPRPC_PING_DN, FALSE) == 0             &&
        pFilter != NULL                                                     &&
        pFilter->choice == LDAP_FILTER_PRESENT                              &&
        pFilter->filtComp.present.lberbv.bv_len == ATTR_OBJECT_CLASS_LEN    &&
        pFilter->filtComp.present.lberbv.bv_val != NULL                     &&
        VmDirStringNCompareA(ATTR_OBJECT_CLASS, pFilter->filtComp.present.lberbv.bv_val, ATTR_OBJECT_CLASS_LEN, FALSE) == 0 &&
        pOp->raftPingCtrl)
    {
        bRetVal = TRUE;
    }

    return bRetVal;
}

/*
 * For cluster vote
 * The search pattern is:
 * BASE:    cn=clustevote
 * FILTER:  (objectclass=*)
 * SCOPE:   BASE
 * Control: PVDIR_RAFT_VOTE_CONTROL_VALUE
 *
 */
// should it be special write controL?
BOOLEAN
VmDirIsSearchForRaftVote(
    PVDIR_OPERATION     pOp
    )
{
    BOOLEAN         bRetVal = FALSE;
    SearchReq*      pSearchReq = &(pOp->request.searchReq);
    PSTR            pszDN = pOp->reqDn.lberbv.bv_val;
    PVDIR_FILTER    pFilter = pSearchReq ? pSearchReq->filter : NULL;

    if (pSearchReq != NULL                                                  &&
        pszDN != NULL                                                       &&
        VmDirStringCompareA(pszDN, LDAPRPC_VOTE_DN, FALSE) == 0             &&
        pFilter != NULL                                                     &&
        pFilter->choice == LDAP_FILTER_PRESENT                              &&
        pFilter->filtComp.present.lberbv.bv_len == ATTR_OBJECT_CLASS_LEN    &&
        pFilter->filtComp.present.lberbv.bv_val != NULL                     &&
        VmDirStringNCompareA(ATTR_OBJECT_CLASS, pFilter->filtComp.present.lberbv.bv_val, ATTR_OBJECT_CLASS_LEN, FALSE) == 0 &&
        pOp->raftVoteCtrl)
    {
        bRetVal = TRUE;
    }

    return bRetVal;
}

/*
 * For server state ping
 * The search pattern is:
 * BASE:    cn=ping,cn=serverstate
 * FILTER:  (objectclass=*)
 * SCOPE:   BASE
 * Control: PVDIR_STATE_PING_CONTROL_VALUE
 */
BOOLEAN
VmDirIsSearchForStatePing(
    PVDIR_OPERATION     pOp
    )
{
    BOOLEAN         bRetVal = FALSE;
    PSTR            pszDN = pOp->reqDn.lberbv.bv_val;
    SearchReq*      pSearchReq = &(pOp->request.searchReq);
    PVDIR_FILTER    pFilter = pSearchReq ? pSearchReq->filter : NULL;

    if (pszDN != NULL                                                       &&
        VmDirStringCompareA(pszDN, SERVER_STATE_PING_DN, FALSE) == 0        &&
        pSearchReq != NULL                                                  &&
        pSearchReq->scope == LDAP_SCOPE_BASE                                &&
        pFilter != NULL                                                     &&
        pFilter->choice == LDAP_FILTER_PRESENT                              &&
        pFilter->filtComp.present.lberbv.bv_len == ATTR_OBJECT_CLASS_LEN    &&
        pFilter->filtComp.present.lberbv.bv_val != NULL                     &&
        VmDirStringNCompareA(ATTR_OBJECT_CLASS, pFilter->filtComp.present.lberbv.bv_val, ATTR_OBJECT_CLASS_LEN, FALSE) == 0 &&
        pOp->statePingCtrl)
    {
        bRetVal = TRUE;
    }

    return bRetVal;
}
