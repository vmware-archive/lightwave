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
    VDIR_FILTER *      f
    );

static
int
ProcessCandidateList(
    VDIR_OPERATION *    pOperation
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

    if (pOperation->conn->bIsAnonymousBind && !VmDirIsSearchForDseRootEntry( pOperation ))
    {
        retVal = LDAP_INSUFFICIENT_ACCESS;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "Not bind/authenticate yet" );
    }

    // AnonymousBind is handled when retrieving search candidate result
    // DSE_ROOT_DN and PERSISTED_DSE_ROOT_DN, SCHEMA_NAMING_CONTEXT_DN
    // SUB_SCHEMA_SUB_ENTRY_DN should allow anonymous bind READ
    retVal = VmDirInternalSearch( pOperation);
    BAIL_ON_VMDIR_ERROR(retVal);

cleanup:

    VmDirSendLdapResult( pOperation );

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
    int        retVal = LDAP_SUCCESS;
    ENTRYID    eId = 0;
    int        deadLockRetries = 0;
    BOOLEAN    bHasTxn = FALSE;
    PSTR       pszLocalErrMsg = NULL;
    PVDIR_LDAP_RESULT   pResult = &(pOperation->ldapResult);

    assert(pOperation && pOperation->pBEIF);

    // Normalize (base) DN
    retVal = VmDirNormalizeDN( &(pOperation->reqDn), pOperation->pSchemaCtx );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "DN normalization failed - (%u)(%s)",
                                  retVal, VDIR_SAFE_STRING(VmDirSchemaCtxGetErrorMsg(pOperation->pSchemaCtx)) );

    if (VmDirHandleSpecialSearch( pOperation, pResult )) // TODO, add &pszLocalErrMsg
    {
        retVal = pResult->errCode;
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

    retVal = BuildCandidateList( pOperation, pOperation->request.searchReq.filter);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "BuildCandidateList failed.");

    if (pOperation->request.searchReq.filter->computeResult == FILTER_RES_TRUE)
    {
        retVal = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg,
                                      "Full scan of Entry DB is required. Refine your search.");
    }

    retVal = ProcessCandidateList( pOperation );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg,
                                   "ProcessCandidateList failed. (%u)(%s)",
                                   retVal,
                                   VDIR_SAFE_STRING( pOperation->ldapResult.pszErrMsg) );

    retVal = pOperation->pBEIF->pfnBETxnCommit( pOperation->pBECtx);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "txn commit (%u)(%s)",
                                  retVal, VDIR_SAFE_STRING(pOperation->pBEErrorMsg));

    bHasTxn = FALSE;

cleanup:

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

    if ( !pszBindDN || !pbIsMemberOf || !pAccessRoleBitmap )
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
    } else if (getAccessInfo == VDIR_ACCESS_DCCLIENT_GROUP_MEMBER_INFO)
    {
        if (*pAccessRoleBitmap & VDIR_ACCESS_DCCLIENT_GROUP_MEMBER_VALID_INFO)
        {
            *pbIsMemberOf = (*pAccessRoleBitmap & VDIR_ACCESS_IS_DCCLIENT_GROUP_MEMBER) != 0;
            goto cleanup;
        }
        pszGroupDN = gVmdirServerGlobals.bvDCClientGroupDN.lberbv_val;
    } else
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pszGroupDN == NULL)
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
    } else
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

/* BuildCandidateList: Process indexed attributes, and build a complete candidates list which is a super set of
 * the result set that we need to send back to the client.
 */

static
int
BuildCandidateList(
    PVDIR_OPERATION    pOperation,
    VDIR_FILTER *      f
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
            BOOLEAN         bDoneANDCandidateBuild = FALSE;
            int             iMaxIndexScan = gVmdirGlobals.dwMaxIndexScan;  // should be always > 0

            VMDIR_LOG_VERBOSE( LDAP_DEBUG_FILTER, "Filter choice: AND" );

            // Look for the "special" filters that ONLY need to be processed. Currently, special filters are:
            //      - equality match filter on a unique attribute (Priority 1)
            //      - filter on usnChanged attribute (Priority 2)
            //          replication issue query -b "" -s sub "usnChanged>=xxx"

            for (nextFilter = f->filtComp.complex; nextFilter != NULL; nextFilter = nextFilter->next)
            {
                if (nextFilter->choice == LDAP_FILTER_GE && VmDirStringCompareA( nextFilter->filtComp.ava.type.lberbv.bv_val,
                                                                        ATTR_USN_CHANGED, FALSE ) == 0)
                {
                    specialFilter = nextFilter;
                    continue; // keep looking for equality match filter on an indexed attribute;
                }
                else // look for equality match filter on an indexed attribute
                {
                    if (nextFilter->choice == LDAP_FILTER_EQUALITY)
                    {
                        PVDIR_CFG_ATTR_INDEX_DESC   pIdxDesc = NULL;
                        USHORT                      usVersion = 0;

                        pIdxDesc = VmDirAttrNameToReadIndexDesc( nextFilter->filtComp.ava.type.lberbv.bv_val, usVersion,
                                                                 &usVersion);

                        if (pIdxDesc != NULL)
                        {   // if we have "INDEXED_ATTRIBUTE=DATA" filter, make it special
                            specialFilter = nextFilter;
                            break;
                        }
                    }
                }
            }
            // We found a special filter. Just build the candidate list for this filter, and we are done with building
            // the candidates list for this AND filter
            if (specialFilter != NULL)
            {
                specialFilter->iMaxIndexScan = iMaxIndexScan;
                retVal = pOperation->pBEIF->pfnBEGetCandidates( pOperation->pBECtx, specialFilter);
                if (retVal == VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND)
                {
                    retVal = LDAP_SUCCESS;

                }
                else if (retVal == 0 && specialFilter->candidates)
                {
                    PSTR pszCandidates = NULL;
                    if (pOperation->pszFilters)
                    {
                        retVal = VmDirAllocateStringPrintf(&pszCandidates, "%s, %s:%d", pOperation->pszFilters, specialFilter->filtComp.ava.type.lberbv.bv_val, specialFilter->candidates->size);
                    }
                    else
                    {
                        retVal = VmDirAllocateStringPrintf(&pszCandidates, "%s:%d", specialFilter->filtComp.ava.type.lberbv.bv_val, specialFilter->candidates->size);
                    }
                    VMDIR_SAFE_FREE_STRINGA(pOperation->pszFilters);
                    pOperation->pszFilters = pszCandidates;
                }
                BAIL_ON_VMDIR_ERROR( retVal );

                if ( specialFilter->candidates != NULL )
                {
                    if ( specialFilter->candidates->size <= specialFilter->iMaxIndexScan )
                    {
                        bDoneANDCandidateBuild = TRUE;
                    }
                    else
                    {   // special filter attempt failed, clean up candidates.
                        DeleteCandidates( &(specialFilter->candidates) );
                    }
                }
            }

            if ( !bDoneANDCandidateBuild )
            {
                BOOLEAN     bStopFilter = FALSE;

                //////////////////////////////////////////////////////////////////////
                // First pass.
                //////////////////////////////////////////////////////////////////////
                // For each AND filters, we limit the number of index scan allowed.
                // If the filter exceeds this limit, we abandon it and move on to next one.
                // The loop stop once we have a good positive filter/candidate.
                //////////////////////////////////////////////////////////////////////
                for ( nextFilter = f->filtComp.complex;
                      nextFilter != NULL;
                      nextFilter = nextFilter->next
                    )
                {
                    if ( nextFilter == specialFilter )
                    {   // try special filter with limit scan already, skip it.
                        continue;
                    }

                    nextFilter->iMaxIndexScan = iMaxIndexScan;
                    retVal = BuildCandidateList( pOperation, nextFilter);
                    BAIL_ON_VMDIR_ERROR( retVal );

                    if ( nextFilter->iMaxIndexScan > 0              &&
                         nextFilter->candidates != NULL             &&
                         nextFilter->candidates->size > nextFilter->iMaxIndexScan
                       )
                    {
                        // index scan exceeds limit, clean up candidates.
                        DeleteCandidates( &(nextFilter->candidates) );
                        continue;
                    }

                    if ( nextFilter->candidates != NULL              &&
                         nextFilter->candidates->size > 0            &&
                         nextFilter->candidates->positive == TRUE
                       )
                    {
                        // we have a filter with limited number of positive candidates:-)
                        bStopFilter = TRUE;
                        break;
                    }
                }

                if ( bStopFilter == FALSE )
                {
                    //////////////////////////////////////////////////////////////////////
                    // Second pass.
                    //////////////////////////////////////////////////////////////////////
                    // Run through all filters w/o index scan limit.
                    //////////////////////////////////////////////////////////////////////
                    for ( nextFilter = f->filtComp.complex;
                          nextFilter != NULL;
                          nextFilter = nextFilter->next
                        )
                    {
                        nextFilter->iMaxIndexScan = 0;
                        retVal = BuildCandidateList( pOperation, nextFilter);
                        BAIL_ON_VMDIR_ERROR( retVal );
                    }
                }
            }

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
              retVal = BuildCandidateList( pOperation, nextFilter);
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

            nextFilter = f->filtComp.complex;
            retVal = BuildCandidateList( pOperation, nextFilter);
            BAIL_ON_VMDIR_ERROR( retVal );

            NotFilterResults( nextFilter, f );
            break;

        case LDAP_FILTER_EQUALITY:
        case LDAP_FILTER_SUBSTRINGS:
        case FILTER_ONE_LEVEL_SEARCH:
        case LDAP_FILTER_GE:
          retVal = pOperation->pBEIF->pfnBEGetCandidates( pOperation->pBECtx, f);
          if (retVal != 0)
          {
              if ( retVal == VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND ) // SJ-TBD: What about DB_DEADLOCK error ??
              {
                  retVal = LDAP_SUCCESS;
              }
          }
          else if (f->candidates)
          {
              PSTR pszCandidates = NULL;
              if (pOperation->pszFilters)
              {
                  retVal = VmDirAllocateStringPrintf(&pszCandidates, "%s, %s:%d", pOperation->pszFilters, f->filtComp.ava.type.lberbv.bv_val, f->candidates->size);
              }
              else
              {
                  retVal = VmDirAllocateStringPrintf(&pszCandidates, "%s:%d", f->filtComp.ava.type.lberbv.bv_val, f->candidates->size);
              }
              VMDIR_SAFE_FREE_STRINGA(pOperation->pszFilters);
              pOperation->pszFilters = pszCandidates;
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
        if (pOperation->opType == VDIR_OPERATION_TYPE_INTERNAL)
        {   //TODO, we should have a hard limit on the cl->size we handle
            bInternalSearch = TRUE;
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
            //skip entries we sent before
            if (bPageResultsCtrl && lastEID > 0)
            {
                if (cl->eIds[i] == lastEID)
                {
                    lastEID = 0;
                }
                continue;
            }

            VMDIR_LOG_DEBUG( LDAP_DEBUG_FILTER, "ProcessCandidateList EID(%u)", cl->eIds[i]);

            pSrEntry = bInternalSearch ?
                        (pOperation->internalSearchEntryArray.pEntry + pOperation->internalSearchEntryArray.iSize) : &srEntry;

            retVal = pOperation->pBEIF->pfnBEIdToEntry(
                        pOperation->pBECtx,
                        pOperation->pSchemaCtx,
                        cl->eIds[i],
                        pSrEntry,
                        VDIR_BACKEND_ENTRY_LOCK_READ);

            if (retVal == 0)
            {
                if (CheckIfEntryPassesFilter( pOperation, pSrEntry, pOperation->request.searchReq.filter) == FILTER_RES_TRUE)
                {
                    retVal = VmDirBuildComputedAttribute( pOperation, pSrEntry );
                    BAIL_ON_VMDIR_ERROR( retVal );

                    if (bInternalSearch)
                    {
                        pOperation->internalSearchEntryArray.iSize++;
                        pSrEntry = NULL;    // EntryArray takes over *pSrEntry content
                    }
                    else
                    {
                        retVal = VmDirSendSearchEntry( pOperation, pSrEntry );
                        if (retVal == VMDIR_ERROR_INSUFFICIENT_ACCESS)
                        {
                            VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL,
                                               "Access deny on search entry result [%s,%d] (bindedDN-%s) (targetDn-%s)\n",
                                               __FILE__, __LINE__, pOperation->conn->AccessInfo.pszBindedDn, pSrEntry->dn.lberbv.bv_val);
                            // make sure search continues
                            retVal = 0;
                        }
                        BAIL_ON_VMDIR_ERROR( retVal );

                        if (pSrEntry->bSearchEntrySent)
                        {
                            numSentEntries++;
                        }
                    }
                }

                //We have sent one page size of entries, so we can break here
                if (bPageResultsCtrl && numSentEntries == dwPageSize){
                    retVal = VmDirStringPrintFA(
                            pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.cookie,
                            VMDIR_MAX_I64_ASCII_STR_LEN,
                            "%u",
                            pSrEntry->eId);
                    BAIL_ON_VMDIR_ERROR( retVal );
                    break;
                }

                VmDirFreeEntryContent( pSrEntry );
                pSrEntry = NULL; // Reset to NULL so that DeleteEntry is no-op.
            }
            else
            {
                // Ignore BdbEIdToEntry errors.
                VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL, "ProcessCandiateList BEIdToEntry EID(%u), error (%u)",
                                                       cl->eIds[i], retVal);
                retVal = 0;
            }
        }

        VMDIR_LOG_VERBOSE( LDAP_DEBUG_FILTER, "(%d) candiates processed and (%d) entries sent", cl->size, numSentEntries);
    }

cleanup:

    pOperation->dwSentEntries = numSentEntries;
    VmDirFreeEntryContent( pSrEntry );

    return retVal;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "ProcessCandiateList failed. (%u)", retVal);
    goto cleanup;
}
