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
int
BuildCandidateList(
    PVDIR_OPERATION    pOperation,
    VDIR_FILTER *      f,
    ENTRYID            eStartingId
    );

static
int
ProcessCandidateList(
    VDIR_OPERATION *    pOperation
    );

static
DWORD
SetPagedSearchCookie(
    PVDIR_OPERATION pOp,
    ENTRYID eId,
    DWORD dwCandidatesProcessed
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

    retVal = VmDirInternalSearch( pOperation);
    BAIL_ON_VMDIR_ERROR(retVal);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);
    return pOperation->ldapResult.errCode;

error:
    VMDIR_SET_LDAP_RESULT_ERROR( &(pOperation->ldapResult), retVal, pszLocalErrMsg);
    goto cleanup;
}

static
DWORD
ProcessPreValidatedEntries(
    PVDIR_OPERATION pOperation,
    DWORD dwEntryCount,
    ENTRYID *pValidatedEntries
    )
{
    DWORD   i = 0;
    DWORD   dwError = 0;
    DWORD   dwSentEntries = 0;
    BOOLEAN bInternalSearch = FALSE;
    BOOLEAN bStoreRsltInMem = FALSE;
    VDIR_ENTRY  srEntry = {0};
    PVDIR_ENTRY pSrEntry = NULL;

    if (dwEntryCount == 0)
    {
        goto cleanup;
    }

    bInternalSearch = pOperation->opType == VDIR_OPERATION_TYPE_INTERNAL;
    bStoreRsltInMem = pOperation->request.searchReq.bStoreRsltInMem;

    if (bInternalSearch || bStoreRsltInMem)
    {
        VmDirFreeEntryArrayContent(&pOperation->internalSearchEntryArray);
        dwError = VmDirAllocateMemory(
                sizeof(VDIR_ENTRY) * (dwEntryCount + 1),
                (PVOID*)&pOperation->internalSearchEntryArray.pEntry);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (; i < dwEntryCount; ++i)
    {
        pSrEntry = bInternalSearch || bStoreRsltInMem ?
                (pOperation->internalSearchEntryArray.pEntry +
                 pOperation->internalSearchEntryArray.iSize) : &srEntry;

        dwError = pOperation->pBEIF->pfnBESimpleIdToEntry(
                pValidatedEntries[i], pSrEntry);
        if (dwError != 0)
        {
            // Ignore errors resolving ENTRYIDs.
            VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL,
                    "%s pfnBESimpleIdToEntry EID(%u), error (%u)",
                    __FUNCTION__, pValidatedEntries[i], dwError);
            continue;
        }

        dwError = VmDirBuildComputedAttribute(pOperation, pSrEntry);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirSendSearchEntry(pOperation, pSrEntry);
        if (dwError == VMDIR_ERROR_INSUFFICIENT_ACCESS)
        {
            VMDIR_LOG_WARNING(
                    VMDIR_LOG_MASK_ALL,
                    "Access deny on search entry result [%s,%d] (bindedDN-%s) (targetDn-%s)",
                    __FILE__,
                    __LINE__,
                    pOperation->conn->AccessInfo.pszBindedDn,
                    pSrEntry->dn.lberbv.bv_val);

            // make sure search continues
            dwError = 0;
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pSrEntry->bSearchEntrySent)
        {
            dwSentEntries++;
            if (bInternalSearch || bStoreRsltInMem)
            {
                pOperation->internalSearchEntryArray.iSize++;
                pSrEntry = NULL;    // EntryArray takes over *pSrEntry content
            }
        }

        VmDirFreeEntryContent(pSrEntry);
        pSrEntry = NULL;
    }

    dwError = SetPagedSearchCookie(
                pOperation,
                pValidatedEntries[dwEntryCount - 1],
                0);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    pOperation->dwSentEntries = dwSentEntries;
    VmDirFreeEntryContent(pSrEntry);
    return dwError;

error:
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
    int        retVal = LDAP_SUCCESS;
    ENTRYID    eId = 0;
    int        deadLockRetries = 0;
    BOOLEAN    bHasTxn = FALSE;
    PSTR       pszLocalErrMsg = NULL;
    PVDIR_LDAP_RESULT  pResult = &(pOperation->ldapResult);
    ENTRYID eStartingId = 0;
    ENTRYID *pValidatedEntries = NULL;
    DWORD dwEntryCount = 0;
    BOOLEAN bUseOldSearch = TRUE;

    assert(pOperation && pOperation->pBEIF);

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

    if (pOperation->syncReqCtrl != NULL) // Replication
    {
        pOperation->lowestPendingUncommittedUsn =
                    pOperation->pBEIF->pfnBEGetLeastOutstandingUSN( pOperation->pBECtx, FALSE );

        VmDirLog( LDAP_DEBUG_REPL, "Replication request USN (%u)",  pOperation->lowestPendingUncommittedUsn );
    }

    // If base is not ROOT, read lock the base object (DnToEntryId index entry) to make sure it exists, and it does
    // not get deleted during this search processing.
    if (pOperation->reqDn.lberbv.bv_len != 0)
    {
        // ************************************************************************************
        // transaction retry loop begin.  make sure all function within are retry agnostic.
        // ************************************************************************************
txnretry:
        if (bHasTxn)
        {
            pOperation->pBEIF->pfnBETxnAbort( pOperation->pBECtx );
            bHasTxn = FALSE;
        }

        deadLockRetries++;
        if (deadLockRetries > MAX_DEADLOCK_RETRIES)
        {
            retVal = VMDIR_ERROR_LOCK_DEADLOCK;
            BAIL_ON_VMDIR_ERROR( retVal );
        }
        else
        {
            retVal = pOperation->pBEIF->pfnBETxnBegin( pOperation->pBECtx, VDIR_BACKEND_TXN_READ );
            BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "txn begin (%u)(%s)",
                                          retVal, VDIR_SAFE_STRING(pOperation->pBEErrorMsg));
            bHasTxn = TRUE;

            // Lookup in the DN index.
            retVal = pOperation->pBEIF->pfnBEDNToEntryId( pOperation->pBECtx, &(pOperation->reqDn), &eId );
            if (retVal != 0)
            {
                switch (retVal)
                {
                    case VMDIR_ERROR_BACKEND_DEADLOCK:
                        goto txnretry; // Possible retry.

                    default:
                        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "DNToEID (%u)(%s)",
                                                      retVal, VDIR_SAFE_STRING(pOperation->pBEErrorMsg));
                }
            }
        }
        // ************************************************************************************
        // transaction retry loop end.
        // ************************************************************************************
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

    if (gVmdirGlobals.bPagedSearchReadAhead)
    {
        if (pOperation->showPagedResultsCtrl != NULL &&
            !IsNullOrEmptyString(pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.cookie))
        {
            retVal = VmDirPagedSearchCacheRead(
                        pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.cookie,
                        &pValidatedEntries,
                        &dwEntryCount);
            BAIL_ON_VMDIR_ERROR(retVal);

            bUseOldSearch = FALSE;
        }
    }

    if (bUseOldSearch)
    {
        retVal = BuildCandidateList(pOperation, pOperation->request.searchReq.filter, eStartingId);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(retVal, pszLocalErrMsg, "BuildCandidateList failed.");

        if (pOperation->request.searchReq.filter->computeResult == FILTER_RES_TRUE)
        {
            retVal = VMDIR_ERROR_UNWILLING_TO_PERFORM;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(
                retVal,
                pszLocalErrMsg,
                "Full scan of Entry DB is required. Refine your search.");
        }

        retVal = ProcessCandidateList(pOperation);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(
            retVal,
            pszLocalErrMsg,
            "ProcessCandidateList failed. (%u)(%s)",
            retVal,
            VDIR_SAFE_STRING(pOperation->ldapResult.pszErrMsg));
    }
    else if (pValidatedEntries != NULL)
    {
        retVal = ProcessPreValidatedEntries(
                    pOperation,
                    dwEntryCount,
                    pValidatedEntries);
        BAIL_ON_VMDIR_ERROR(retVal);
    }
    else if (pOperation->showPagedResultsCtrl != NULL)
    {
        pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.cookie[0] = '\0';
    }

    retVal = pOperation->pBEIF->pfnBETxnCommit( pOperation->pBECtx);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "txn commit (%u)(%s)",
                                  retVal, VDIR_SAFE_STRING(pOperation->pBEErrorMsg));

    bHasTxn = FALSE;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pValidatedEntries);
    VMDIR_SAFE_FREE_MEMORY( pszLocalErrMsg );

    return retVal;

error:

    if (bHasTxn)
    {
        pOperation->pBEIF->pfnBETxnAbort( pOperation->pBECtx );
    }

    VMDIR_SET_LDAP_RESULT_ERROR( &(pOperation->ldapResult), retVal, pszLocalErrMsg);

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

/* BuildCandidateList: Process indexed attributes, and build a complete candidates list which is a super set of
 * the result set that we need to send back to the client.
 */

static
int
BuildCandidateList(
    PVDIR_OPERATION    pOperation,
    VDIR_FILTER *      f,
    ENTRYID            eStartingId
    )
{
    int             retVal = LDAP_SUCCESS;
    VDIR_FILTER *   nextFilter = NULL;

    if (f->computeResult != FILTER_RES_NORMAL)
    {
        goto cleanup;
    }

    switch ( f->choice )
    {
        case LDAP_FILTER_AND:
        {
            VDIR_FILTER *   specialFilter = NULL;
            int             iMaxIndexScan = gVmdirGlobals.dwMaxIndexScan;
            int             iSmallCandidateSet = gVmdirGlobals.dwSmallCandidateSet;
            BOOLEAN         bGotPositiveCandidateSet = f->bAncestorGotPositiveCandidateSet;
            SearchReq *     sr = &pOperation->request.searchReq;

            VMDIR_LOG_VERBOSE( LDAP_DEBUG_FILTER, "Filter choice: AND" );

            pOperation->pBECtx->iMaxScanForSizeLimit = 0;
            if (pOperation->showPagedResultsCtrl == 0 && sr->sizeLimit > 0)
            {
                //pfnBEGetCandidates will try to avoid excessive index lookup when there is a sizeLimit hint
                // - only if there is no page control
                pOperation->pBECtx->iMaxScanForSizeLimit = gVmdirGlobals.dwMaxSizelimitScan;
            }

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
            // First pass - evalute non-composite filters in the AND component
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
                    //Continue loop looking for a better candidate set.
                }
            }

            // First pass - evaluate composite filters in the AND component
            // The loop stop once we have a small good positive filter/candidate,
            // otherwise, do not attempt the second pass if got any good positive candidate set.
            // It passes in current bGotPositiveCandidateSet to the composit filters, so that
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

          for (nextFilter = f->filtComp.complex; nextFilter != NULL; nextFilter = nextFilter->next)
          {
              //nextFilter inherents the bAncestorGotPositiveCandidateSet
              nextFilter->bAncestorGotPositiveCandidateSet = f->bAncestorGotPositiveCandidateSet;
              retVal = BuildCandidateList(pOperation, nextFilter, eStartingId);
              BAIL_ON_VMDIR_ERROR( retVal );
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
              if ( retVal == VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND ) // SJ-TBD: What about DB_DEADLOCK error ??
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
    return retVal;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s,%d failed, error(%d)", __FUNCTION__, __LINE__, retVal );
    goto cleanup;
}

static
DWORD
SetPagedSearchCookie(
    PVDIR_OPERATION pOperation,
    ENTRYID eId,
    DWORD dwCandidatesProcessed
    )
{
    DWORD dwError = 0;

    if (gVmdirGlobals.bPagedSearchReadAhead)
    {
        dwError = VmDirPagedSearchCacheInsert(
                    pOperation,
                    dwCandidatesProcessed);
    }

    if (dwError != 0 || !gVmdirGlobals.bPagedSearchReadAhead)
    {
        //
        // We were unable to cache the information necessary. Fallback to the
        // old cookie mechanism.
        //
        dwError = VmDirStringPrintFA(
                    pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.cookie,
                    VMDIR_ARRAY_SIZE(pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.cookie),
                    "%u",
                    eId);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

/* ProcessCandidateList: Goes thru the candidate list, constructed by processing the indexed attributes and logical
 * (AND/OR/NOT) relationship between them, and performs the following logic:
 *     - Read complete entry
 *     - Check if it passes the search filter
 *     - Build memberOf attribute, if needed
 *     - Send the search entry (if it has passed the filter) + computed attributes (e.g. memberOf)
 *
 * Note: This whole processing is not part of an existing/bigger transaction, if any.
 */

static
int
ProcessCandidateList(
    VDIR_OPERATION * pOperation
    )
{
    int               retVal = LDAP_SUCCESS;
    int               i = 0;
    VDIR_CANDIDATES * cl = pOperation->request.searchReq.filter->candidates;
    VDIR_ENTRY        srEntry = {0};
    VDIR_ENTRY *      pSrEntry = NULL;
    int               numSentEntries = 0;
    BOOLEAN           bInternalSearch = FALSE;
    BOOLEAN           bStoreRsltInMem = FALSE;
    BOOLEAN           bPageResultsCtrl = FALSE;
    DWORD             dwPageSize = 0;
    ENTRYID           lastEID = 0;

    /*
     * If the page size is greater than or equal to the sizeLimit value,
     * the server should ignore the control as the request can be satisfied in a single page.
     */
    if (pOperation->showPagedResultsCtrl && (pOperation->request.searchReq.sizeLimit == 0 ||
            pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.pageSize < (DWORD)pOperation->request.searchReq.sizeLimit))
    {
        VmDirLog( LDAP_DEBUG_TRACE, "showPagedResultsCtrl applies to this query." );
        bPageResultsCtrl = TRUE;
        dwPageSize = pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.pageSize;
        lastEID = atoi(pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.cookie);
        pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.cookie[0] = '\0';
    }

    if (cl && cl->size > 0)
    {
        bInternalSearch = pOperation->opType == VDIR_OPERATION_TYPE_INTERNAL;
        bStoreRsltInMem = pOperation->request.searchReq.bStoreRsltInMem;

        if (bInternalSearch || bStoreRsltInMem)
        {   //TODO, we should have a hard limit on the cl->size we handle
            VmDirFreeEntryArrayContent(&pOperation->internalSearchEntryArray);
            retVal = VmDirAllocateMemory(   sizeof(VDIR_ENTRY) * cl->size,
                                            (PVOID*)&pOperation->internalSearchEntryArray.pEntry);
            BAIL_ON_VMDIR_ERROR(retVal);
        }

        for (i = 0, numSentEntries = 0;
             (i < cl->size) && VmDirdState() != VMDIRD_STATE_SHUTDOWN &&
             (pOperation->request.searchReq.sizeLimit == 0 /* unlimited */ ||
              numSentEntries < pOperation->request.searchReq.sizeLimit);
             i++)
        {
            if (!gVmdirGlobals.bPagedSearchReadAhead)
            {
                //skip entries we sent before
                if (bPageResultsCtrl && lastEID > 0)
                {
                    if (cl->eIds[i] == lastEID)
                    {
                        lastEID = 0;
                    }
                    continue;
                }
            }

            pSrEntry = bInternalSearch || bStoreRsltInMem ?
                        (pOperation->internalSearchEntryArray.pEntry + pOperation->internalSearchEntryArray.iSize) : &srEntry;

            retVal = pOperation->pBEIF->pfnBEIdToEntry(
                        pOperation->pBECtx,
                        pOperation->pSchemaCtx,
                        cl->eIds[i],
                        pSrEntry,
                        VDIR_BACKEND_ENTRY_LOCK_READ);

            if (retVal)
            {
                // Ignore BdbEIdToEntry errors.
                VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL,
                        "ProcessCandiateList BEIdToEntry EID(%u), error (%u)",
                        cl->eIds[i], retVal);

                continue;
            }

            if (CheckIfEntryPassesFilter(pOperation, pSrEntry, pOperation->request.searchReq.filter) == FILTER_RES_TRUE)
            {
                BOOLEAN bSendEntry = TRUE;
                CHAR    sha1Digest[SHA_DIGEST_LENGTH] = {0};

                retVal = VmDirBuildComputedAttribute( pOperation, pSrEntry );
                BAIL_ON_VMDIR_ERROR( retVal );

                if (pOperation->digestCtrl)
                {
                    retVal = VmDirEntrySHA1Digest(pSrEntry, sha1Digest);
                    BAIL_ON_VMDIR_ERROR(retVal);

                    if (memcmp(sha1Digest, pOperation->digestCtrl->value.digestCtrlVal.sha1Digest, SHA_DIGEST_LENGTH) == 0)
                    {
                        bSendEntry = FALSE;
                        VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL,"%s digest match %s",
                                           __FUNCTION__, pSrEntry->dn.lberbv.bv_val);
                    }
                    else
                    {
                        VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL,"%s digest mismatch %s",
                                           __FUNCTION__, pSrEntry->dn.lberbv.bv_val);
                    }
                }

                if (bSendEntry)
                {
                    retVal = VmDirSendSearchEntry( pOperation, pSrEntry );
                    if (retVal == VMDIR_ERROR_INSUFFICIENT_ACCESS)
                    {
                        VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL,
                                "Access deny on search entry result [%s,%d] (bindedDN-%s) (targetDn-%s)\n",
                                __FILE__,
                                __LINE__,
                                pOperation->conn->AccessInfo.pszBindedDn,
                                pSrEntry->dn.lberbv.bv_val);
                        // make sure search continues
                        retVal = 0;
                    }
                    BAIL_ON_VMDIR_ERROR( retVal );

                    if (pSrEntry->bSearchEntrySent)
                    {
                        numSentEntries++;
                        if (bInternalSearch || bStoreRsltInMem)
                        {
                            pOperation->internalSearchEntryArray.iSize++;
                            pSrEntry = NULL;    // EntryArray takes over *pSrEntry content
                        }
                    }
                }
            }

            //We have sent one page size of entries, so we can break here
            if (bPageResultsCtrl && numSentEntries == dwPageSize)
            {
                retVal = SetPagedSearchCookie(pOperation, cl->eIds[i], i);
                BAIL_ON_VMDIR_ERROR(retVal);
                break;
            }

            VmDirFreeEntryContent( pSrEntry );
            pSrEntry = NULL; // Reset to NULL so that DeleteEntry is no-op.
        }

        VMDIR_LOG_VERBOSE( LDAP_DEBUG_FILTER, "(%d) candiates processed and (%d) entries sent", cl->size, numSentEntries);
    }

    if ( pOperation->request.searchReq.sizeLimit && numSentEntries < pOperation->request.searchReq.sizeLimit &&
         pOperation->pBECtx->iPartialCandidates)
    {
        retVal = LDAP_UNWILLING_TO_PERFORM;
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "ProcessCandiateList may return none or paritial requested entries with sizelimit %d",
                        pOperation->request.searchReq.sizeLimit);
    }

cleanup:

    pOperation->dwSentEntries = numSentEntries;
    VmDirFreeEntryContent( pSrEntry );

    return retVal;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "ProcessCandiateList failed. (%u)", retVal);
    goto cleanup;
}
