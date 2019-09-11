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



#include "includes.h"

static
VOID
ComputeRequiredAccess(
    SearchReq*  pSearchReq
    );

static
int
VmDirProcessCandidateList(
    VDIR_OPERATION *    pOperation
    );

static
int
_VmDirInternalSearchDispatch(
    PVDIR_OPERATION     pOperation
    );

static
int
_VmDirProcessNormalSearch(
    PVDIR_OPERATION     pOperation
    );

int
VmDirMLSearch(
    PVDIR_OPERATION pOperation
    )
{
    int     retVal = 0;
    PSTR    pszLocalErrMsg = NULL;

    pOperation->pBEIF = VmDirBackendSelect(pOperation->reqDn.lberbv.bv_val);
    assert(pOperation->pBEIF);

    retVal = VmDirInternalSearch(pOperation);
    BAIL_ON_VMDIR_ERROR(retVal);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);
    return pOperation->ldapResult.errCode;

error:
    VMDIR_SET_LDAP_RESULT_ERROR( &(pOperation->ldapResult), retVal, pszLocalErrMsg);
    goto cleanup;
}

/* InternalSearch: Interface that can be used "internally" by the server code. One of the main differences between
 * this function and MLSearch is that this function does not send back an LDAP result to the client.
 *
 * Return: VmDir level error code.  Also, pOperation->ldapResult content is set.
 */
int
VmDirInternalSearch(
    PVDIR_OPERATION pOperation
    )
{
    int     retVal = LDAP_SUCCESS;
    BOOLEAN bHasTxn = FALSE;
    PSTR    pszLocalErrMsg = NULL;
    PVDIR_LDAP_RESULT   pResult = &(pOperation->ldapResult);
    PVDIR_OPERATION_ML_METRIC  pMLMetrics = NULL;

    assert(pOperation && pOperation->pBEIF);

    pMLMetrics = &pOperation->MLMetrics;
    VMDIR_COLLECT_TIME(pMLMetrics->iMLStartTime);

    // compute required access for this search
    ComputeRequiredAccess(&pOperation->request.searchReq);

    // Normalize (base) DN
    retVal = VmDirNormalizeDN( &(pOperation->reqDn), pOperation->pSchemaCtx );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "DN normalization failed - (%u)(%s)",
                                  retVal, VDIR_SAFE_STRING(VmDirSchemaCtxGetErrorMsg(pOperation->pSchemaCtx)) );

    if (VmDirHandleSpecialSearch( pOperation, pResult )) // TODO, add &pszLocalErrMsg
    {
        retVal = pResult->errCode ? pResult->errCode : pResult->vmdirErrCode;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(retVal, pszLocalErrMsg, "Special search failed - (%u)", retVal);

        goto cleanup;  // done special search
    }

    if (pOperation->dbCopyCtrl) {
        retVal = VmDirExecDbCopyCtrl(pOperation);
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    if (pOperation->syncReqCtrl != NULL) // Replication
    {
        pOperation->opType = VDIR_OPERATION_TYPE_REPL;
    }

    if (pOperation->pReplAgrDisableCtrl)
    {
        retVal = VmDirExecReplAgrEnableDisableCtrl(BERVAL_NORM_VAL(pOperation->reqDn), FALSE);
        BAIL_ON_VMDIR_ERROR(retVal);
    }
    else if (pOperation->pReplAgrEnableCtrl)
    {
        retVal = VmDirExecReplAgrEnableDisableCtrl(BERVAL_NORM_VAL(pOperation->reqDn), TRUE);
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    // If base is not ROOT, read lock the base object (DnToEntryId index entry) to make sure it exists, and it does
    // not get deleted during this search processing.
    if (pOperation->reqDn.lberbv.bv_len != 0)
    {
        VMDIR_COLLECT_TIME(pMLMetrics->iBETxnBeginStartTime);

        retVal = pOperation->pBEIF->pfnBETxnBegin( pOperation->pBECtx, VDIR_BACKEND_TXN_READ );
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "txn begin (%u)(%s)",
                retVal, VDIR_SAFE_STRING(pOperation->pBEErrorMsg));
        bHasTxn = TRUE;

        VMDIR_COLLECT_TIME(pMLMetrics->iBETxnBeginEndTime);

        // Lookup in the DN index.
        retVal = pOperation->pBEIF->pfnBEDNToEntryId(
            pOperation->pBECtx,
            &(pOperation->reqDn),
            &pOperation->request.searchReq.baseEID );
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "DNToEID (%u)(%s)",
                retVal, VDIR_SAFE_STRING(pOperation->pBEErrorMsg));
    }

    // start txn if not has one already.
    if (! pOperation->pBECtx->pBEPrivate)
    {
        retVal = pOperation->pBEIF->pfnBETxnBegin( pOperation->pBECtx, VDIR_BACKEND_TXN_READ );
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "txn begin (%u)(%s)",
                                      retVal, VDIR_SAFE_STRING(pOperation->pBEErrorMsg));
        bHasTxn = TRUE;
    }

    retVal = AppendDNFilter( pOperation );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "Appending DN filter failed.");

    retVal = _VmDirInternalSearchDispatch(pOperation);
    BAIL_ON_VMDIR_ERROR(retVal);

    VMDIR_COLLECT_TIME(pMLMetrics->iBETxnCommitStartTime);

    retVal = pOperation->pBEIF->pfnBETxnCommit( pOperation->pBECtx);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "txn commit (%u)(%s)",
                                  retVal, VDIR_SAFE_STRING(pOperation->pBEErrorMsg));
    bHasTxn = FALSE;

    VMDIR_COLLECT_TIME(pMLMetrics->iBETxnCommitEndTime);

cleanup:

    // collect metrics
    VMDIR_COLLECT_TIME(pMLMetrics->iMLEndTime);
    VmDirInternalMetricsUpdate(pOperation);
    VmDirInternalMetricsLogInefficientOp(pOperation);

    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);
    return retVal;

error:

    if (bHasTxn)
    {
        pOperation->pBEIF->pfnBETxnAbort(pOperation->pBECtx);
    }

    VMDIR_SET_LDAP_RESULT_ERROR(&pOperation->ldapResult, retVal, pszLocalErrMsg);
    goto cleanup;
}

/*
 * TODO, to generalize, we should create a strToFilter(pszFilter, &pOutFilter);
 */
DWORD
VmDirSimpleEqualFilterInternalSearch(
        PCSTR               pszBaseDN,
        int                 searchScope,
        PCSTR               pszAttrName,
        PCSTR               pszAttrValue,
        PVDIR_ENTRY_ARRAY   pEntryArray
    )
{
    DWORD           dwError = 0;
    VDIR_OPERATION  searchOP = {0};
    VDIR_BERVALUE   bervDN = VDIR_BERVALUE_INIT;
    PVDIR_FILTER    pFilter = NULL;

    if ( !pszBaseDN || !pszAttrName || !pszAttrValue || !pEntryArray )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirInitStackOperation( &searchOP,
                                       VDIR_OPERATION_TYPE_INTERNAL,
                                       LDAP_REQ_SEARCH,
                                       NULL );
    BAIL_ON_VMDIR_ERROR(dwError);

    bervDN.lberbv.bv_val = (PSTR)pszBaseDN;
    bervDN.lberbv.bv_len = VmDirStringLenA(pszBaseDN);

    searchOP.pBEIF = VmDirBackendSelect( pszBaseDN );
    assert(searchOP.pBEIF);

    dwError = VmDirBervalContentDup( &bervDN, &searchOP.reqDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    searchOP.request.searchReq.scope = searchScope;

    {
    dwError = VmDirAllocateMemory( sizeof( VDIR_FILTER ), (PVOID*)&pFilter );
    BAIL_ON_VMDIR_ERROR(dwError);

    pFilter->choice = LDAP_FILTER_EQUALITY;
    pFilter->filtComp.ava.type.lberbv.bv_val = (PSTR)pszAttrName;
    pFilter->filtComp.ava.type.lberbv.bv_len = VmDirStringLenA(pszAttrName);
    pFilter->filtComp.ava.pATDesc = VmDirSchemaAttrNameToDesc(
                                                    searchOP.pSchemaCtx,
                                                    pszAttrName);
    if (pFilter->filtComp.ava.pATDesc == NULL)
    {
        dwError = VMDIR_ERROR_NO_SUCH_ATTRIBUTE;
        BAIL_ON_VMDIR_ERROR( dwError );
    }
    pFilter->filtComp.ava.value.lberbv.bv_val = (PSTR)pszAttrValue;
    pFilter->filtComp.ava.value.lberbv.bv_len = VmDirStringLenA(pszAttrValue);
    dwError = VmDirSchemaBervalNormalize(               // TODO, may want to have filter code to do this?
                    searchOP.pSchemaCtx,                // so caller does not have to handle this.
                    pFilter->filtComp.ava.pATDesc,
                    &(pFilter->filtComp.ava.value) );
    BAIL_ON_VMDIR_ERROR(dwError);

    pFilter->next = NULL;
    }

    //TODO, ideally, we should take pszFilter and dwError = VmDirStrToFilter(pszFilter, &pFilter);
    searchOP.request.searchReq.filter = pFilter;
    pFilter  = NULL; // search request takes over pFilter


    dwError = VmDirInternalSearch( &searchOP );
    BAIL_ON_VMDIR_ERROR(dwError);

    // caller takes over searchOP.internalSearchEntryArray contents
    pEntryArray->iSize = searchOP.internalSearchEntryArray.iSize;
    pEntryArray->pEntry = searchOP.internalSearchEntryArray.pEntry;
    searchOP.internalSearchEntryArray.iSize = 0;
    searchOP.internalSearchEntryArray.pEntry = NULL;

cleanup:

    VmDirFreeOperationContent(&searchOP);

    if (pFilter)
    {
        DeleteFilter(pFilter);
    }

    return dwError;

error:
    goto cleanup;
}

/*
 * This generic search with pagination is new and isn't mature. Please be
 * careful with the * scope, base, and use an indexed filter.
 * Note that ulPageSize == 0 will ignore paging.
 */
DWORD
VmDirFilterInternalSearch(
        PCSTR               pszBaseDN,
        int                 searchScope,
        PCSTR               pszFilter,
        unsigned long       ulPageSize,
        PSTR                *ppszPageCookie,
        PVDIR_ENTRY_ARRAY   pEntryArray
    )
{
    DWORD           dwError = 0;
    VDIR_OPERATION  searchOP = {0};
    VDIR_BERVALUE   bervDN = VDIR_BERVALUE_INIT;
    PVDIR_FILTER    pFilter = NULL;
    PVDIR_LDAP_CONTROL showPagedResultsCtrl = NULL;
    PSTR pszPageCookie = NULL;

    if ( !pszBaseDN || !pszFilter || !pEntryArray ||
        (ulPageSize != 0 && ppszPageCookie == NULL))
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (ulPageSize != 0)
    {
        dwError = VmDirAllocateMemory( sizeof(VDIR_LDAP_CONTROL), (PVOID *)&showPagedResultsCtrl );
        BAIL_ON_VMDIR_ERROR(dwError);

        showPagedResultsCtrl->value.pagedResultCtrlVal.pageSize = ulPageSize;
        if (ppszPageCookie && *ppszPageCookie)
        {
            VmDirStringNCpyA(showPagedResultsCtrl->value.pagedResultCtrlVal.cookie,
                             VMDIR_ARRAY_SIZE(showPagedResultsCtrl->value.pagedResultCtrlVal.cookie),
                             *ppszPageCookie,
                             VMDIR_ARRAY_SIZE(showPagedResultsCtrl->value.pagedResultCtrlVal.cookie) - 1);
        }
        else
        {
            showPagedResultsCtrl->value.pagedResultCtrlVal.cookie[0] = '\0';
        }
    }

    dwError = VmDirInitStackOperation( &searchOP,
                                       VDIR_OPERATION_TYPE_INTERNAL,
                                       LDAP_REQ_SEARCH,
                                       NULL );
    BAIL_ON_VMDIR_ERROR(dwError);

    bervDN.lberbv.bv_val = (PSTR)pszBaseDN;
    bervDN.lberbv.bv_len = VmDirStringLenA(pszBaseDN);

    searchOP.pBEIF = VmDirBackendSelect( pszBaseDN );
    assert(searchOP.pBEIF);

    dwError = VmDirBervalContentDup( &bervDN, &searchOP.reqDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    searchOP.request.searchReq.scope = searchScope;

    dwError = StrFilterToFilter(pszFilter, &pFilter);
    BAIL_ON_VMDIR_ERROR(dwError);

    searchOP.request.searchReq.filter = pFilter;
    pFilter  = NULL; // search request takes over pFilter

    searchOP.showPagedResultsCtrl = showPagedResultsCtrl;

    dwError = VmDirInternalSearch( &searchOP );
    BAIL_ON_VMDIR_ERROR(dwError);

    // caller takes over searchOP.internalSearchEntryArray contents
    pEntryArray->iSize = searchOP.internalSearchEntryArray.iSize;
    pEntryArray->pEntry = searchOP.internalSearchEntryArray.pEntry;
    searchOP.internalSearchEntryArray.iSize = 0;
    searchOP.internalSearchEntryArray.pEntry = NULL;

    if (showPagedResultsCtrl)
    {
        dwError = VmDirAllocateStringA(showPagedResultsCtrl->value.pagedResultCtrlVal.cookie, &pszPageCookie);
        BAIL_ON_VMDIR_ERROR(dwError);

        *ppszPageCookie = pszPageCookie;
        pszPageCookie = NULL;
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(showPagedResultsCtrl);

    VmDirFreeOperationContent(&searchOP);

    if (pFilter)
    {
        DeleteFilter(pFilter);
    }

    return dwError;

error:
    goto cleanup;
}

/*
 *      Searches the direct group entry to confirm membership
 *      Set pAccessRoleBitmap based on getAccessInfo to avoid redundent internal search.
 */
DWORD
VmDirIsDirectMemberOf(
    PSTR                pszBindDN,
    UINT32              getAccessInfo,
    UINT32              *pAccessRoleBitmap,
    PBOOLEAN            pbIsMemberOf
    )
{
    DWORD               dwError = 0;
    BOOLEAN             bIsMemberOf = FALSE;
    VDIR_ENTRY_ARRAY    entryResultArray = {0};
    PSTR                pszGroupDN = NULL;

    if (!pbIsMemberOf || !pAccessRoleBitmap)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (getAccessInfo == VDIR_ACCESS_DCGROUP_MEMBER_INFO)
    {
         if (*pAccessRoleBitmap & VDIR_ACCESS_DCGROUP_MEMBER_VALID_INFO)
         {
             *pbIsMemberOf = (*pAccessRoleBitmap & VDIR_ACCESS_IS_DCGROUP_MEMBER) != 0;
             goto cleanup;
         }
         pszGroupDN = gVmdirServerGlobals.bvDCGroupDN.lberbv_val;
    }
    else if (getAccessInfo == VDIR_ACCESS_DCCLIENT_GROUP_MEMBER_INFO)
    {
        if (*pAccessRoleBitmap & VDIR_ACCESS_DCCLIENT_GROUP_MEMBER_VALID_INFO)
        {
            *pbIsMemberOf = (*pAccessRoleBitmap & VDIR_ACCESS_IS_DCCLIENT_GROUP_MEMBER) != 0;
            goto cleanup;
        }
        pszGroupDN = gVmdirServerGlobals.bvDCClientGroupDN.lberbv_val;
    }
    else
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pszGroupDN == NULL || pszBindDN == NULL)
    {
        *pbIsMemberOf = FALSE;
        goto cleanup;
    }

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmDirIsDirectMemberOf: internal search for dn %s op %d",
          pszBindDN, getAccessInfo);

    dwError = VmDirSimpleEqualFilterInternalSearch( pszGroupDN,
                                                    LDAP_SCOPE_BASE,
                                                    ATTR_MEMBER,
                                                    pszBindDN,
                                                    &entryResultArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    if ( entryResultArray.iSize > 0)
    {
        bIsMemberOf = TRUE;
    }

    if (getAccessInfo == VDIR_ACCESS_DCGROUP_MEMBER_INFO)
    {
        *pAccessRoleBitmap |= VDIR_ACCESS_DCGROUP_MEMBER_VALID_INFO;
        if (bIsMemberOf)
        {
             *pAccessRoleBitmap |= VDIR_ACCESS_IS_DCGROUP_MEMBER;
        }
    }
    else
    {
        *pAccessRoleBitmap |= VDIR_ACCESS_DCCLIENT_GROUP_MEMBER_VALID_INFO;
        if (bIsMemberOf)
        {
            *pAccessRoleBitmap |= VDIR_ACCESS_IS_DCCLIENT_GROUP_MEMBER;
        }
    }

    *pbIsMemberOf = bIsMemberOf;

cleanup:

    VmDirFreeEntryArrayContent(&entryResultArray);

    return dwError;

error:

    *pbIsMemberOf = FALSE;

    goto cleanup;
}

static
VOID
ComputeRequiredAccess(
    SearchReq*  pSearchReq
    )
{
    DWORD   i = 0, j = 0;
    BOOLEAN bSDAttr = FALSE;
    PSTR    pszAttr = NULL;

    PCSTR   pszSDAttrs[] = { ATTR_OBJECT_SECURITY_DESCRIPTOR, ATTR_ACL_STRING };

    pSearchReq->accessRequired = VMDIR_RIGHT_DS_READ_PROP;

    for (i = 0; pSearchReq->attrs && pSearchReq->attrs[i].lberbv.bv_val; i++)
    {
        bSDAttr = FALSE;
        pszAttr = pSearchReq->attrs[i].lberbv.bv_val;

        for (j = 0; j < VMDIR_ARRAY_SIZE(pszSDAttrs); j++)
        {
            if (VmDirStringCompareA(pszAttr, pszSDAttrs[j], FALSE) == 0)
            {
                bSDAttr = TRUE;
                break;
            }
        }

        if (bSDAttr)
        {
            // SD attributes require only READ_CONTROL
            pSearchReq->accessRequired = VMDIR_ENTRY_READ_ACL;
        }
        else
        {
            // any other attributes require READ_PROP
            pSearchReq->accessRequired = VMDIR_RIGHT_DS_READ_PROP;
            break;  // no need to continue
        }
    }
}

static
DWORD
_GetFilterCandidateLabel(
    VDIR_FILTER *   f,
    PSTR *          ppszLabel
    )
{
    DWORD dwError = 0;

    assert(f && ppszLabel);

    switch (f->choice)
    {
        case LDAP_FILTER_EQUALITY:
        case LDAP_FILTER_GE:
        case LDAP_FILTER_LE:
            *ppszLabel = f->filtComp.ava.type.lberbv.bv_val;
            break;
        case LDAP_FILTER_SUBSTRINGS:
            *ppszLabel = f->filtComp.subStrings.type.lberbv.bv_val;
            break;
        case FILTER_ONE_LEVEL_SEARCH:
            *ppszLabel = f->filtComp.parentDn.lberbv.bv_val;
            break;
        default:
            dwError = ERROR_INVALID_DATA; // It must been one of above
            BAIL_ON_VMDIR_ERROR(dwError);
            break;
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s,%d failed, error(%d)", __FUNCTION__, __LINE__, dwError );
    goto cleanup;
}

static
VOID
_VmDirUpdateNormalSearchPlan(
    PVDIR_OPERATION     pOperation
    )
{
    DWORD   dwError = 0;
    SearchReq *             pSReq = &pOperation->request.searchReq;
    PVDIR_SEARCH_EXEC_PATH  pExecPath = &pOperation->request.searchReq.srvExecPath;

    if (VMDIR_IS_OP_CTRL_SEARCH_PLAN(pOperation))
    {
        pExecPath->iEntrySent = pSReq->iNumEntrySent;
        pExecPath->bPagedSearch = VMDIR_IS_OP_CTRL_PAGE_SEARCH(pOperation);
        // pExecPath->bPagedSearchDone set in paged search code
        // pExecPath->bExceedMaxIteration set in search code

        if (pExecPath->searchAlgo == SEARCH_ALGO_CANDIDATE_LIST)
        {
            pExecPath->candiatePlan.iCandateSize = pSReq->filter->candidates->size;
        }
        else if (pExecPath->searchAlgo == SEARCH_ALGO_ITERATOR)
        {
            // pExecPath->IteratePlan.iNumIteration set in search code

            dwError = VmDirAllocateStringA(pSReq->iteratorSearchPlan.attr.lberbv_val, &pExecPath->pszIndex);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL,
            "%s Algo %d table %s paged %d done %d, sent %d iter %d, cl size %d",
            __FUNCTION__,
            pExecPath->searchAlgo,
            VDIR_SAFE_STRING(pExecPath->pszIndex),
            pExecPath->bPagedSearch,
            pExecPath->bPagedSearchDone,
            pExecPath->iEntrySent,
            pExecPath->IteratePlan.iNumIteration,
            pExecPath->candiatePlan.iCandateSize);
    }

error:
    return;
}

static
int
_VmDirInternalSearchDispatch(
    PVDIR_OPERATION     pOperation
    )
{
    int         retVal = LDAP_SUCCESS;

    if (VMDIR_IS_OP_CTRL_PAGE_SEARCH(pOperation))
    {
        retVal = VmDirProcessPagedSearch(pOperation);
        BAIL_ON_VMDIR_ERROR(retVal);
    }
    else
    {
        retVal = _VmDirProcessNormalSearch(pOperation);
        BAIL_ON_VMDIR_ERROR(retVal);
    }

cleanup:
    return retVal;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error(%d)",
            __FUNCTION__,
            retVal);
    goto cleanup;
}

static
int
_VmDirProcessNormalSearch(
    PVDIR_OPERATION     pOperation
    )
{
    int         retVal = LDAP_SUCCESS;
    PSTR        pszLocalErrMsg = NULL;
    SearchReq   *pSReq = &pOperation->request.searchReq;

    retVal = BuildCandidateList(pOperation, pOperation->request.searchReq.filter, 0);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(retVal, pszLocalErrMsg, "BuildCandidateList failed.");

    if (pOperation->request.searchReq.filter->computeResult == FILTER_RES_TRUE)
    {
        // BuildCandidateList tried to build a full result set but gave up eventually
        // TODO, not sure if this still valid with iterator based search support.
        retVal = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg,
            "Full scan of Entry DB is required. Refine your search.");
    }
    else if (pSReq->srvExecPath.searchAlgo == SEARCH_ALGO_ITERATOR)
    {
        retVal = VmDirSearchViaIterator(pOperation);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "Error on VmDirSearchViaIterator");
    }
    else
    {
        // Candidate set has been built which is a super set of the final result.
        retVal = VmDirProcessCandidateList(pOperation);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(
            retVal, pszLocalErrMsg, "VmDirProcessCandidateList failed. (%u)(%s)",
            retVal, VDIR_SAFE_STRING(pOperation->ldapResult.pszErrMsg));
    }

cleanup:
    _VmDirUpdateNormalSearchPlan(pOperation);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);

    return retVal;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error(%d)",
            __FUNCTION__,
            retVal);
    goto cleanup;
}

/* BuildCandidateList: Process indexed attributes, and build a complete candidates list which is a super set of
 * the result set that we need to send back to the client.
 */

int
BuildCandidateList(
    PVDIR_OPERATION    pOperation,
    VDIR_FILTER *      f,
    ENTRYID            eStartingId
    )
{
    int             retVal = LDAP_SUCCESS;
    VDIR_FILTER *   nextFilter = NULL;
    SearchReq *     pSearchReq = &pOperation->request.searchReq;
    int             iMaxIndexScan = gVmdirGlobals.dwMaxIndexScan;
    int             iSmallCandidateSet = gVmdirGlobals.dwSmallCandidateSet;

    pOperation->request.searchReq.iBuildCandDepth++;

    if (f->computeResult != FILTER_RES_NORMAL)
    {
        goto cleanup;
    }

    switch ( f->choice )
    {
        case LDAP_FILTER_AND:
        {
            VDIR_FILTER *   specialFilter = NULL;
            BOOLEAN         bGotPositiveCandidateSet = f->bAncestorGotPositiveCandidateSet;

            VMDIR_LOG_VERBOSE( LDAP_DEBUG_FILTER, "Filter choice: AND" );

            // Look for the "special" filters that ONLY need to be processed. Currently, special filters are:
            //      - equality match filter on a unique attribute (Priority 1)
            //      - filter on usnChanged attribute (Priority 2)
            //          replication issue query -b "" -s sub "usnChanged>=xxx"
            for (nextFilter = f->filtComp.complex; nextFilter != NULL; nextFilter = nextFilter->next)
            {
                if (nextFilter->choice == LDAP_FILTER_GE &&
                    VmDirStringCompareA( nextFilter->filtComp.ava.type.lberbv.bv_val, ATTR_USN_CHANGED, FALSE ) == 0)
                {
                    specialFilter = nextFilter;
                    continue; //keep looking for equality match filter on a unique attribute
                }

                if (nextFilter->choice == LDAP_FILTER_EQUALITY)
                {
                    //look for equality match filter on a unique indexed attribute
                    PVDIR_INDEX_CFG pIndexCfg = NULL;
                    BOOLEAN bFoundGlobalUniqIdx = FALSE;

                    retVal = VmDirIndexCfgAcquire(
                            nextFilter->filtComp.ava.type.lberbv.bv_val,
                            VDIR_INDEX_READ,
                            &pIndexCfg);
                    BAIL_ON_VMDIR_ERROR( retVal );

                    bFoundGlobalUniqIdx = pIndexCfg && pIndexCfg->bGlobalUniq;
                    VmDirIndexCfgRelease(pIndexCfg);

                    if (bFoundGlobalUniqIdx)
                    {
                        specialFilter = nextFilter;
                        break;
                    }
                }
            }

            if (specialFilter == NULL)
            {
                goto first_pass;
            }

            // We found a special filter. Just build the candidate list for this filter.
            specialFilter->iMaxIndexScan = 0; //special entry should return all matched entries.
            retVal = BuildCandidateList(pOperation, specialFilter, eStartingId);
            if (retVal == VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND)
            {
                retVal = LDAP_SUCCESS;

            }
            BAIL_ON_VMDIR_ERROR( retVal );

            if ( specialFilter->candidates != NULL )
            {
                if ( specialFilter->candidates->size <= iSmallCandidateSet )
                {
                    goto candidate_build_done;
                }
                bGotPositiveCandidateSet = TRUE;
            }

first_pass:
            // First pass - evaluate NON-COMPOSITE filters in the AND component
            // For each of the filters, we limit the number of index scan allowed.
            // If it exceeds this limit, we abandon it and move on to next one.
            // The loop stop once we have a small good positive filter/candidate.
            // Will not attempt second pass if we got any good positive candidate set.
            //////////////////////////////////////////////////////////////////////
            for ( nextFilter = f->filtComp.complex; nextFilter != NULL; nextFilter = nextFilter->next )
            {
                if ( nextFilter == specialFilter || nextFilter->choice == LDAP_FILTER_AND || nextFilter->choice == LDAP_FILTER_OR )
                {
                    continue;
                }

                nextFilter->iMaxIndexScan = iMaxIndexScan;
                retVal = BuildCandidateList(pOperation, nextFilter, eStartingId);
                BAIL_ON_VMDIR_ERROR( retVal );

                if ( nextFilter->candidates != NULL && nextFilter->candidates->size >= 0 &&
                     nextFilter->candidates->positive == TRUE )
                {
                    if (nextFilter->candidates->size <= iSmallCandidateSet )
                    {
                        // We have a small, positive, candidate set, stop evaluating remaining filters in the AND component
                        goto candidate_build_done;
                    }
                    bGotPositiveCandidateSet = TRUE;
                    // Continue loop looking for a better candidate set.
                }
            }

            // First pass - evaluate COMPOSITE filters in the AND component
            // The loop stop once we have a small good positive filter/candidate,
            // otherwise, do not attempt the second pass if got any good positive candidate set.
            // It passes in current bGotPositiveCandidateSet to the composite filters, so that
            // their evaluations will not go through second pass when bGotPositiveCandidateSet is true.
            for ( nextFilter = f->filtComp.complex; nextFilter != NULL; nextFilter = nextFilter->next )
            {
                if ( nextFilter->choice != LDAP_FILTER_AND && nextFilter->choice != LDAP_FILTER_OR )
                {
                    continue;
                }

                nextFilter->bAncestorGotPositiveCandidateSet = bGotPositiveCandidateSet;
                retVal = BuildCandidateList(pOperation, nextFilter, eStartingId);
                BAIL_ON_VMDIR_ERROR( retVal );

                if ( nextFilter->candidates != NULL && nextFilter->candidates->size >= 0 &&
                     nextFilter->candidates->positive == TRUE)
                {
                    if (nextFilter->candidates->size <= iSmallCandidateSet )
                    {
                        // We have a small, positive, candidate set, stop evaluating remaining filters in the AND component
                        goto candidate_build_done;
                    }
                    bGotPositiveCandidateSet = TRUE;
                    //Continue loop looking for a better candidate set.
                }
            }

            if ( bGotPositiveCandidateSet )
            {
                goto candidate_build_done;
            }

            if (pSearchReq->iOrFilterDepth == 0)
            {
                // To reduce scope, only optimize iteration for external call
                if (pOperation->opType == VDIR_OPERATION_TYPE_EXTERNAL)
                {
                    for ( nextFilter = f->filtComp.complex; nextFilter != NULL; nextFilter = nextFilter->next )
                    {
                       if (// nextFilter->choice == LDAP_FILTER_GE          ||
                           // nextFilter->choice == LDAP_FILTER_LE          ||
                           nextFilter->choice == LDAP_FILTER_EQUALITY       ||
                           nextFilter->choice == LDAP_FILTER_SUBSTRINGS     ||
                           nextFilter->choice == FILTER_ONE_LEVEL_SEARCH    ||
                           nextFilter->choice == LDAP_FILTER_PRESENT)
                       {
                           // Loop through non-composite filters under current AND filter
                           // find the best attribute type for iterator candidate (pri will be set > 0 if found).
                           VmDirEvaluateIteratorCandidate(pOperation, nextFilter);
                       }
                    }

                    if (pSearchReq->iteratorSearchPlan.pri > 0)
                    {
                        // Fall through index iterator because we found a good one.
                        pSearchReq->srvExecPath.searchAlgo = SEARCH_ALGO_ITERATOR;
                        f->computeResult = FILTER_RES_PENDING;

                        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
                            "%s, -- iterate search, page search (%d) table (%s) value (%s)",
                            __FUNCTION__,
                            VMDIR_IS_OP_CTRL_PAGE_SEARCH(pOperation),
                            pSearchReq->iteratorSearchPlan.attr.lberbv.bv_val,
                            pSearchReq->iteratorSearchPlan.attrNormVal.lberbv.bv_val);

                        goto cleanup;
                    }
                }

                //////////////////////////////////////////////////////////////////////
                // Second pass - get here when failing to get any positive candidate set
                // Run through all filters w/o index scan limit.
                //////////////////////////////////////////////////////////////////////
                for ( nextFilter = f->filtComp.complex; nextFilter != NULL; nextFilter = nextFilter->next )
                {
                    nextFilter->iMaxIndexScan = 0;
                    retVal = BuildCandidateList(pOperation, nextFilter, eStartingId);
                    BAIL_ON_VMDIR_ERROR( retVal );
                }
            }

candidate_build_done:
            // First, "AND" no candidates lists or "positive" candidates lists
            for (nextFilter = f->filtComp.complex; nextFilter != NULL; nextFilter = nextFilter->next)
            {
                if (nextFilter->candidates == NULL || nextFilter->candidates->positive == TRUE)
                {
                    AndFilterResults( nextFilter, f);
                }
            }
            // Second, "AND" "negative" candidates lists.
            for (nextFilter = f->filtComp.complex; nextFilter != NULL; nextFilter = nextFilter->next)
            {
                if (nextFilter->candidates != NULL && nextFilter->candidates->positive == FALSE)
                {
                    AndFilterResults( nextFilter, f);
                }
            }

            break;
        }

        case LDAP_FILTER_OR:
          VMDIR_LOG_VERBOSE( LDAP_DEBUG_FILTER, "Filter choice: OR" );

          pOperation->request.searchReq.iOrFilterDepth++;

          for (nextFilter = f->filtComp.complex; nextFilter != NULL; nextFilter = nextFilter->next)
          {
              //nextFilter inherits bAncestorGotPositiveCandidateSet
              nextFilter->bAncestorGotPositiveCandidateSet = f->bAncestorGotPositiveCandidateSet;
              nextFilter->iMaxIndexScan = iMaxIndexScan;

              retVal = BuildCandidateList(pOperation, nextFilter, eStartingId);
              BAIL_ON_VMDIR_ERROR( retVal );

              if (!nextFilter->bLastScanPositive)
              {   // a component failed to produce a complete set, give up on this OR filter
                  pOperation->request.searchReq.iOrFilterDepth--;
                  goto cleanup;
              }
          }
          // First, "OR" no candidates lists or "positive" candidates lists
          for (nextFilter = f->filtComp.complex; nextFilter != NULL; nextFilter = nextFilter->next)
          {
              if (nextFilter->candidates == NULL || nextFilter->candidates->positive == TRUE)
              {
                  OrFilterResults( nextFilter, f);
              }
          }
          /// Second, "OR" "negative" candidates lists.
          for (nextFilter = f->filtComp.complex; nextFilter != NULL; nextFilter = nextFilter->next)
          {
              if (nextFilter->candidates != NULL && nextFilter->candidates->positive == FALSE)
              {
                  OrFilterResults( nextFilter, f);
              }
          }

          pOperation->request.searchReq.iOrFilterDepth--;
          break;

        case LDAP_FILTER_NOT:
            VMDIR_LOG_VERBOSE( LDAP_DEBUG_FILTER, "Filter choice: NOT" );

#if 0
            nextFilter = f->filtComp.complex;
            retVal = BuildCandidateList( pOperation, nextFilter);
            BAIL_ON_VMDIR_ERROR( retVal );

            NotFilterResults( nextFilter, f );
#endif
            //The nextFilter above may contain a super set of valid candidates
            //substracting it from filter f may result in an incorrect result set.
            //For now, simply set the destination filter to FILTER_RES_TRUE.
            // Ignore candidate building on filtComp.complex for NOT filter.
            // We rely on final pass to qualify NOT filter.
            f->computeResult = FILTER_RES_TRUE;
            break;

        case LDAP_FILTER_EQUALITY:
        case LDAP_FILTER_SUBSTRINGS:
        case FILTER_ONE_LEVEL_SEARCH:
        case LDAP_FILTER_GE:
        case LDAP_FILTER_LE:
          retVal = pOperation->pBEIF->pfnBEGetCandidates(pOperation->pBECtx, f, eStartingId);
          if (retVal != 0)
          {
              if ( retVal == VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND )
              {
                  retVal = LDAP_SUCCESS;
              }
          }
          else if (f->candidates)
          {
              PSTR pszCand = NULL;
              PSTR pszCandList = NULL;

              retVal = _GetFilterCandidateLabel(f, &pszCand);
              BAIL_ON_VMDIR_ERROR( retVal );

              retVal = VmDirAllocateStringPrintf(&pszCandList, "%s%s%s:%d",
                      pOperation->pszFilters ? pOperation->pszFilters : "",
                      pOperation->pszFilters ? ", " : "",
                      VDIR_SAFE_STRING(pszCand),
                      f->candidates->size);
              BAIL_ON_VMDIR_ERROR( retVal );

              VMDIR_SAFE_FREE_STRINGA(pOperation->pszFilters);
              pOperation->pszFilters = pszCandList;
          }
          BAIL_ON_VMDIR_ERROR( retVal );
          break;

        case LDAP_FILTER_PRESENT:
            VMDIR_LOG_VERBOSE( LDAP_DEBUG_FILTER, "Filter choice: PRESENT" );

            f->computeResult = FILTER_RES_TRUE; // SJ-TBD: Just for now, not always, needs to be looked into
            f->candidates = NULL;
            break;

        default:
          break;
    }

cleanup:
    pOperation->request.searchReq.iBuildCandDepth--;
    return retVal;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s,%d failed, error(%d)", __FUNCTION__, __LINE__, retVal );
    goto cleanup;
}

/* VmDirProcessCandidateList: this is invoked only a complete candidate list has been built
 * which is a super set of the result set. The flow will go other functions if build candidate
 * list turned out to use iterator based result set.
 *
 * Note: This whole processing is not part of an existing/bigger transaction, if any.
 */

static
int
VmDirProcessCandidateList(
    VDIR_OPERATION * pOperation
    )
{
    int               retVal = LDAP_SUCCESS;
    int               i = 0;
    VDIR_CANDIDATES * cl = pOperation->request.searchReq.filter->candidates;
    VDIR_ENTRY *      pSrEntry = NULL;
    int               numSentEntries = 0;
    BOOLEAN           bExternalSearch = FALSE;
    BOOLEAN           bInternalSearch = FALSE;
    BOOLEAN           bStoreRsltInMem = FALSE;
    ENTRYID           lastEId = 0;

    bExternalSearch = pOperation->opType == VDIR_OPERATION_TYPE_EXTERNAL;
    bInternalSearch = pOperation->opType == VDIR_OPERATION_TYPE_INTERNAL;
    bStoreRsltInMem = pOperation->request.searchReq.bStoreRsltInMem;

    if (cl == NULL || cl->size == 0)
    {
        goto cleanup;
    }

    // data layer index scan does NOT guarantee uniqueness of CL
    // thus sort CL and skip duplicate in following loop.
    VmDirSortCandidateList(cl);

    if (bInternalSearch || bStoreRsltInMem)
    {   //TODO, we should have a hard limit on the cl->size we handle
        VmDirFreeEntryArrayContent(&pOperation->internalSearchEntryArray);
        retVal = VmDirAllocateMemory(
                sizeof(VDIR_ENTRY) * cl->size,
                (PVOID*)&pOperation->internalSearchEntryArray.pEntry);
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    for (i = 0, numSentEntries = 0;
         (i < cl->size) &&
         (pOperation->request.searchReq.sizeLimit == 0 /* unlimited */ ||
          numSentEntries < pOperation->request.searchReq.sizeLimit);
         i++)
    {
        BOOLEAN bEntrySent = FALSE;

        if (bExternalSearch                         &&
            VmDirdState() == VMDIRD_STATE_SHUTDOWN  &&
            pOperation->syncReqCtrl == NULL)
        {
            retVal = LDAP_UNAVAILABLE; // stop all external search ops, except replication pull
            goto cleanup;
        }

        pSrEntry = bInternalSearch || bStoreRsltInMem ?
                    (pOperation->internalSearchEntryArray.pEntry +
                            pOperation->internalSearchEntryArray.iSize) : NULL;

        if (lastEId == cl->eIds[i])
        {   // skip duplicate EID
            continue;
        }
        lastEId = cl->eIds[i];

        retVal = VmDirCheckAndSendEntry(cl->eIds[i], pOperation, pSrEntry, &bEntrySent);
        BAIL_ON_VMDIR_ERROR(retVal);

        if (bEntrySent)
        {
            numSentEntries++;
        }
    }

    VMDIR_LOG_VERBOSE( LDAP_DEBUG_FILTER, "(%d) candiates processed and (%d) entries sent", cl->size, numSentEntries);

cleanup:
    pOperation->dwSentEntries = numSentEntries;
    return retVal;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "ProcessCandiateList failed. (%u)", retVal);
    goto cleanup;
}
