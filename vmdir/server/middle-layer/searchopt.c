/*
 * Copyright © 2019 VMware, Inc.  All Rights Reserved.
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
char
_VmDirIsReverseSearch(
    PVDIR_FILTER pFilter
    )
{
    BOOLEAN bReverseSearch = FALSE;

    if (pFilter &&
        pFilter->choice == LDAP_FILTER_SUBSTRINGS &&
        VmDirStringCompareA(pFilter->filtComp.subStrings.type.lberbv.bv_val, ATTR_DN, FALSE) == 0 &&
        pFilter->filtComp.subStrings.final.lberbv.bv_len != 0)
    {
        bReverseSearch = TRUE;
    }

    return bReverseSearch;
}

// Evaluate the top level AND filters and find out the best candidate for iterator based search.
//  The criteria are:
// 1. filter type.
// 2. search scope
// 3. Whether it is a paged based search or with a sizelimit
// 4. attribute value (which may either statically configured or based on search history)
//    this capability is not implemented yet.
void
VmDirEvaluateIteratorCandidate(
    PVDIR_OPERATION     pOp,
    PVDIR_FILTER        pFilter
    )
{
    DWORD   dwError = 0;
    int     iNewPri = 0;
    int     iSearchType = 0;
    PVDIR_INDEX_CFG pIndexCfg = NULL;
    PSTR    pszAttrType = NULL;
    PSTR    pszAttrNormVal = NULL;
    char    eIdBytes[sizeof(ENTRYID)+1] = {0};

    SearchReq*  pSr = &pOp->request.searchReq;
    BOOLEAN     bPagedSearch = pOp->showPagedResultsCtrl!=NULL;

    if (!pFilter || !pOp)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (!gVmdirServerGlobals.searchOptMap.bMapLoaded ||
        // disable search optimization for non-paged scenario
        (!VMDIR_IS_OP_CTRL_PAGE_SEARCH(pOp) && !(gVmdirGlobals.dwEnableSearchOptimization & 1)) ||
        // disable search optimization for paged scenario
        (VMDIR_IS_OP_CTRL_PAGE_SEARCH(pOp)  && !(gVmdirGlobals.dwEnableSearchOptimization & 2)))
    {
        goto cleanup;
    }

    iSearchType = pFilter->choice;

    if (iSearchType == LDAP_FILTER_PRESENT)
    {
        pszAttrType = pFilter->filtComp.present.lberbv.bv_val;
        pszAttrNormVal = NULL;
    }
    else if (iSearchType == LDAP_FILTER_SUBSTRINGS)
    {
        pszAttrType = pFilter->filtComp.ava.type.lberbv.bv_val;
        if (pFilter->filtComp.ava.value.lberbv.bv_val)
        {
            pszAttrNormVal = BERVAL_NORM_VAL(pFilter->filtComp.ava.value);
        }
        else
        {
            pszAttrNormVal = BERVAL_NORM_VAL(pFilter->filtComp.subStrings.final);
        }
    }
    else if (// iSearchType == LDAP_FILTER_GE ||    not yet supported
             // iSearchType == LDAP_FILTER_LE ||    not yet supported
             iSearchType == LDAP_FILTER_EQUALITY)
    {
        pszAttrType = pFilter->filtComp.ava.type.lberbv.bv_val;
        pszAttrNormVal = BERVAL_NORM_VAL(pFilter->filtComp.ava.value);
    }
    else if (iSearchType == FILTER_ONE_LEVEL_SEARCH)
    {
        pszAttrType = ATTR_PARENT_ID;

        assert(pOp->request.searchReq.baseEID);
        dwError = VmDirStringNPrintFA(
                eIdBytes,
                sizeof(ENTRYID)+1,
                sizeof(ENTRYID),
                "%"PRId64"", pOp->request.searchReq.baseEID);
        BAIL_ON_VMDIR_ERROR(dwError);

        pszAttrNormVal = &eIdBytes[0];
    }
    else
    {
        VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL,
                "%s: unhandled search optimization, searchType 0x%02x",
                __func__, iSearchType);
        goto cleanup;
    }

    dwError = VmDirIndexCfgAcquire(pszAttrType, VDIR_INDEX_READ, &pIndexCfg);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!pIndexCfg)
    {
        //Not indexed attribute, cannot iterator on it.
        goto cleanup;
    }

    {
        VDIR_SEARCHOPT_PARAM    searchOptParam =
            {iSearchType, bPagedSearch, pSr->sizeLimit, pSr->timeLimit, pszAttrType, pszAttrNormVal};

        iNewPri = VmDirGetSearchPri(&searchOptParam);
    }

    if (iNewPri > pSr->iteratorSearchPlan.pri)
    {
        pSr->iteratorSearchPlan.pri = iNewPri;

        dwError = VmDirStringToBervalContent(pszAttrType, &pSr->iteratorSearchPlan.attr);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pszAttrNormVal)
        {
            dwError = VmDirStringToBervalContent(pszAttrNormVal, &pSr->iteratorSearchPlan.attrNormVal);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        pSr->iteratorSearchPlan.iterSearchType = iSearchType;
        pSr->iteratorSearchPlan.bReverseSearch = _VmDirIsReverseSearch(pFilter);

        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL,
            "%s: searchType 0x%02x paged %d sizeLimit %d attr:val %s:%s newPri %d",
            __func__, iSearchType, bPagedSearch, pSr->sizeLimit, pszAttrType, pszAttrNormVal?pszAttrNormVal:"none", iNewPri);
    }
    else
    {
        VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL,
            "%s: searchType 0x%02x paged %d sizeLimit %d attr %s new Pri %d -- new Pri <= old Pri, keep old attr",
            __func__, iSearchType, bPagedSearch, pSr->sizeLimit, pszAttrType, iNewPri);
    }

cleanup:
    VmDirIndexCfgRelease(pIndexCfg);
    return;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
            "%s: error: level 1 AND filter components: 0x%02x paged %d sizeLimit %d %s = %s newPri %d",
            __func__, iSearchType, bPagedSearch, pSr->sizeLimit, pszAttrType, pszAttrNormVal, pSr->iteratorSearchPlan.pri);
    goto cleanup;
}

/*
 * Run the whole filter, if passed:
 *   send entry over the wire if pEntry is NULL
 *   OR
 *   cache entry into pEntry for in-memory use case (REST,INTERNAL_OP)
 *
 */
DWORD
VmDirCheckAndSendEntry(
    ENTRYID             eId,
    PVDIR_OPERATION     pOp,
    PVDIR_ENTRY         pEntry,
    PBOOLEAN            pbEntrySent
    )
{
    DWORD       dwError = 0;
    VDIR_ENTRY  entry = {0};
    PVDIR_ENTRY pLocalEntry = pEntry ? pEntry : &entry;

    *pbEntrySent = FALSE;

    if (VMDIR_IS_OP_CTRL_DIGEST(pOp) && pEntry)
    {
        // digestCtrl used for external Ldap search only.
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_OPERATION_NOT_PERMITTED);
    }

    dwError = pOp->pBEIF->pfnBEIdToEntry(
            pOp->pBECtx,
            pOp->pSchemaCtx,
            eId,
            pLocalEntry,
            VDIR_BACKEND_ENTRY_LOCK_READ);
    if (dwError == VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND)
    {
        VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL, "%s EidToEntry lookup failed EID (%llu), error (%d)",
                __FUNCTION__, eId, dwError);
        dwError = 0;
        goto cleanup;
    }

    BAIL_ON_VMDIR_ERROR(dwError);

    if (CheckIfEntryPassesFilter(pOp, pLocalEntry, pOp->request.searchReq.filter) == FILTER_RES_TRUE)
    {
        if (pEntry  &&
            pOp->internalSearchEntryArray.iSize >= gVmdirServerGlobals.dwMaxInternalSearchLimit)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INTERNAL_SEARCH_LIMIT);
        }

        dwError = VmDirBuildComputedAttribute(pOp, pLocalEntry);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (VMDIR_IS_OP_CTRL_DIGEST(pOp))
        {
            CHAR sha1Digest[SHA_DIGEST_LENGTH] = {0};

            dwError = VmDirEntrySHA1Digest(pLocalEntry, sha1Digest);
            BAIL_ON_VMDIR_ERROR(dwError);

            if (memcmp(sha1Digest, pOp->digestCtrl->value.digestCtrlVal.sha1Digest, SHA_DIGEST_LENGTH) == 0)
            {
                VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL,
                    "%s digest match %s", __FUNCTION__, pLocalEntry->dn.lberbv.bv_val);
                goto cleanup;
            }
            else
            {
                VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL,
                    "%s digest mismatch %s", __FUNCTION__, pLocalEntry->dn.lberbv.bv_val);
            }
        }

        dwError = VmDirSendSearchEntry(pOp, pLocalEntry);
        if (dwError == VMDIR_ERROR_INSUFFICIENT_ACCESS)
        {
            VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL,
                "Access deny on search entry result [%s,%d] (bindedDN-%s) (targetDn-%s)",
                __FILE__, __LINE__, pOp->conn->AccessInfo.pszBindedDn,
                pLocalEntry->dn.lberbv.bv_val);

            // make sure search continues
            dwError = 0;
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pLocalEntry->bSearchEntrySent == TRUE)
        {
            *pbEntrySent = TRUE;
            if (pLocalEntry == pEntry)
            {
                pOp->internalSearchEntryArray.iSize++;
                pLocalEntry = NULL;
            }
        }
    }

cleanup:
    if (pLocalEntry)
    {
        VmDirFreeEntryContent(pLocalEntry);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s failed on eId %llu, error(%d)", __FUNCTION__, eId, dwError);
    goto cleanup;
}

DWORD
VmDirAllocOrReallocEntryArray(
    PVDIR_ENTRY_ARRAY   pEntryArray
    )
{
    DWORD dwError = 0;

    assert(pEntryArray);

    if (pEntryArray->iArraySize == 0)
    {
        dwError = VmDirAllocateMemory(
                sizeof(VDIR_ENTRY) * VMDIR_ENTRY_ARRAY_INIT_SIZE,
                (PVOID*)&pEntryArray->pEntry);
        BAIL_ON_VMDIR_ERROR(dwError);

        pEntryArray->iArraySize = VMDIR_ENTRY_ARRAY_INIT_SIZE;
        pEntryArray->iSize = 0;
    }
    else if (pEntryArray->iArraySize == pEntryArray->iSize)
    {
        dwError = VmDirReallocateMemoryWithInit(
                pEntryArray->pEntry,
                (PVOID*)&(pEntryArray->pEntry),
                (pEntryArray->iArraySize * 2) * sizeof(VDIR_ENTRY),
                pEntryArray->iArraySize * sizeof(VDIR_ENTRY));
        BAIL_ON_VMDIR_ERROR(dwError);

        pEntryArray->iArraySize *= 2;
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

int
VmDirSearchViaIterator(
    PVDIR_OPERATION     pOp
    )
{
    DWORD   dwError = 0;
    int     iSentEntryCnt = 0;
    PVDIR_ENTRY     pEntry = NULL;
    PVDIR_INDEX_CFG pIndexCfg = NULL;
    PVDIR_BACKEND_INDEX_ITERATOR    pIterator = NULL;
    VDIR_ITERATOR_CONTEXT           iterContext = {0};
    BOOLEAN     bStoreRsltInMem = FALSE;
    BOOLEAN     bAdminBind = FALSE;
    SearchReq*  pSr = &pOp->request.searchReq;
    PVDIR_SEARCH_EXEC_PATH pExecPath = &pOp->request.searchReq.srvExecPath;

    if (IsNullOrEmptyString(pSr->iteratorSearchPlan.attr.lberbv.bv_val))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    bStoreRsltInMem = pOp->request.searchReq.bStoreRsltInMem ||
                      pOp->opType == VDIR_OPERATION_TYPE_INTERNAL;

    dwError = VmDirIndexCfgAcquire(
            pSr->iteratorSearchPlan.attr.lberbv.bv_val,
            VDIR_INDEX_READ,
            &pIndexCfg);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirIterContextInitContent(
            &iterContext,
            &pSr->iteratorSearchPlan);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = pOp->pBEIF->pfnBEIndexIteratorInit(
            pIndexCfg,
            &iterContext,
            &pIterator);
    BAIL_ON_VMDIR_ERROR(dwError);

    bAdminBind = pOp->opType == VDIR_OPERATION_TYPE_INTERNAL ||
                 VMDIR_IS_ADMIN_OR_DC_GROUP_MEMBER(pOp->conn->AccessInfo.accessRoleBitmap);

    while (pIterator->bHasNext && (pSr->sizeLimit == 0 || iSentEntryCnt < pSr->sizeLimit))
    {
        BOOLEAN bEntrySent = FALSE;

        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            goto cleanup;
        }

        if (gVmdirGlobals.dwMaxSearchIteration  &&
            !bAdminBind                         &&
            iterContext.iIterCount > gVmdirGlobals.dwMaxSearchIteration)
        {
            pExecPath->bExceedMaxIteration = TRUE;

            dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
            VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s reached upper limit (%d) for iterator search (%d)",
                             __FUNCTION__, gVmdirGlobals.dwMaxSearchIteration, dwError);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (!VmDirCheckEidInMap(iterContext.pSentIDMap, iterContext.eId))
        {
            if (bStoreRsltInMem)
            {
                dwError = VmDirAllocOrReallocEntryArray(&pOp->internalSearchEntryArray);
                BAIL_ON_VMDIR_ERROR(dwError);

                pEntry = pOp->internalSearchEntryArray.pEntry + pOp->internalSearchEntryArray.iSize;
            }

            dwError = VmDirCheckAndSendEntry(
                    iterContext.eId,
                    pOp,
                    pEntry,
                    &bEntrySent);
            BAIL_ON_VMDIR_ERROR(dwError);

            if (bEntrySent)
            {
                dwError = VmDirAddEidToMap(
                        iterContext.pSentIDMap,
                        iterContext.eId);
                BAIL_ON_VMDIR_ERROR(dwError);

                iSentEntryCnt++;
            }
        }
        else
        {
            VMDIR_LOG_VERBOSE( LDAP_DEBUG_FILTER, "%s: Already sent eId %llu",
                    __func__, iterContext.eId);
        }

        dwError = pOp->pBEIF->pfnBEIndexIterate(pIterator, &iterContext);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pExecPath->IteratePlan.iNumIteration = iterContext.iIterCount;

cleanup:
    if (pIterator)
    {
        pOp->pBEIF->pfnBEIndexIteratorFree(pIterator);
    }

    VmDirIterContextFreeContent(&iterContext);
    VmDirIndexCfgRelease(pIndexCfg);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s failed, error(%d)", __FUNCTION__, dwError);
    goto cleanup;
}
