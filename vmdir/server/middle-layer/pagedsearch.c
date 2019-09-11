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
DWORD
_VmDirPagedSearchContextNodeAllocate(
    PVDIR_OPERATION            pOperation,
    PVDIR_PAGED_SEARCH_RECORD  *ppSearchRecord
    );

static
VOID
_VmDirPagedSearchFreeMapPair(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    );

static
DWORD
_VmDirPagedSearchContextRecordAdd(
    PVDIR_PAGED_SEARCH_RECORD   pPagedSearchContext
    );

static
VOID
_VmDirPagedSearchContextRecordRemove(
    PVDIR_PAGED_SEARCH_RECORD   pPagedSearchContext
    );

static
VOID
_VmDirPagedSearchContextRecordRemoveInLock(
    PVDIR_PAGED_SEARCH_RECORD   pPagedSearchContext
    );

static
VOID
_VmDirPagedSearchContextRecordFree(
    PVDIR_PAGED_SEARCH_RECORD  pPagedSearchContext
    );

static
DWORD
_VmDirPagedSearchGetContext(
    PVDIR_OPERATION            pOperation,
    PVDIR_PAGED_SEARCH_RECORD *ppPagedSearchContext
    );

static
DWORD
_VmDirPagedSearchGetContextUsingCookie(
    PCSTR                      pszCookie,
    PVDIR_PAGED_SEARCH_RECORD* ppPagedSearchContext
    );

static
DWORD
_VmDirNextPageViaCandidateList(
    PVDIR_OPERATION            pOperation,
    PVDIR_PAGED_SEARCH_RECORD  pPagedSearchContext
    );

static
DWORD
_VmDirNextPageViaIterator(
    PVDIR_OPERATION            pOperation,
    PVDIR_PAGED_SEARCH_RECORD  pPagedSearchContext
    );

static
DWORD
_VmDirPagedSearchUpdateCookie(
    PVDIR_OPERATION            pOperation,
    PVDIR_PAGED_SEARCH_RECORD  pPagedSearchContext
    );

static
DWORD
_VmDirPagedSearchValidateRequest(
    PVDIR_OPERATION            pOperation,
    PVDIR_PAGED_SEARCH_RECORD  pPagedSearchContext
    );

static
VOID
_VmDirUpdatePagedSearchPlan(
    PVDIR_OPERATION             pOperation,
    PVDIR_PAGED_SEARCH_RECORD   pPagedSearchContext
    );

DWORD
VmDirProcessPagedSearch(
    PVDIR_OPERATION  pOperation
    )
{
    DWORD dwError = 0;
    PVDIR_PAGED_SEARCH_RECORD pPagedSearchContext = NULL;

    if (pOperation == NULL || !VMDIR_IS_OP_CTRL_PAGE_SEARCH(pOperation))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_UNAVAILABLE);
    }

    if (IsNullOrEmptyString(pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.cookie))
    {
        // Very first paged search call, build candidate list.
        // If too expensive, we will fall back to iterator based algorithm.
        dwError = BuildCandidateList(pOperation, pOperation->request.searchReq.filter, 0);
        BAIL_ON_VMDIR_ERROR(dwError);

        // TODO, not sure if this still valid with iterator based search support.
        if (pOperation->request.searchReq.filter->computeResult == FILTER_RES_TRUE)
        {
            dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    // create or retrieve Paged Search Context
    dwError = _VmDirPagedSearchGetContext(pOperation, &pPagedSearchContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirPagedSearchValidateRequest(pOperation, pPagedSearchContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pPagedSearchContext->searchAlgo == SEARCH_ALGO_ITERATOR)
    {
        dwError = _VmDirNextPageViaIterator(pOperation, pPagedSearchContext);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = _VmDirNextPageViaCandidateList(pOperation, pPagedSearchContext);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmDirPagedSearchUpdateCookie(pOperation, pPagedSearchContext);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pPagedSearchContext)
    {
        _VmDirUpdatePagedSearchPlan(pOperation, pPagedSearchContext);

        if (pPagedSearchContext->bCompete)
        {
            _VmDirPagedSearchContextRecordRemove(pPagedSearchContext);
        }
    }

    return dwError;

error:
    if (pPagedSearchContext)
    {
        pPagedSearchContext->bCompete = TRUE;
    }
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s failed, error(%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirPagedSearchContextInit(
    VOID
    )
{
    DWORD dwError = 0;

    dwError = VmDirAllocateMutex(&gPagedSearchCtxCache.mutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(&gPagedSearchCtxCache.pHashMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;
error:
    VmDirPagedSearchContextFree();
    goto cleanup;
}

VOID
VmDirPagedSearchContextFree(
    VOID
    )
{
    BOOLEAN  bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gPagedSearchCtxCache.mutex);
    if (gPagedSearchCtxCache.pHashMap != NULL)
    {
        LwRtlHashMapClear(gPagedSearchCtxCache.pHashMap, _VmDirPagedSearchFreeMapPair, NULL);
        LwRtlFreeHashMap(&gPagedSearchCtxCache.pHashMap);
    }
    VMDIR_UNLOCK_MUTEX(bInLock, gPagedSearchCtxCache.mutex);

    VMDIR_SAFE_FREE_MUTEX(gPagedSearchCtxCache.mutex);
}

static
VOID
_VmDirUpdatePagedSearchPlan(
    PVDIR_OPERATION             pOperation,
    PVDIR_PAGED_SEARCH_RECORD   pPagedSearchContext
    )
{
    DWORD   dwError = 0;
    SearchReq *             pSReq = &pOperation->request.searchReq;
    PVDIR_SEARCH_EXEC_PATH  pExecPath = &pOperation->request.searchReq.srvExecPath;

    if (VMDIR_IS_OP_CTRL_SEARCH_PLAN(pOperation) && pPagedSearchContext)
    {
        pExecPath->iEntrySent = pSReq->iNumEntrySent;
        pExecPath->bPagedSearch = VMDIR_IS_OP_CTRL_PAGE_SEARCH(pOperation);
        pExecPath->bPagedSearchDone = pPagedSearchContext->bCompete;

        // pExecPath->bExceedMaxIteration set in search code

        if (pPagedSearchContext->searchAlgo == SEARCH_ALGO_CANDIDATE_LIST)
        {
            pExecPath->searchAlgo = SEARCH_ALGO_CANDIDATE_LIST;
            pExecPath->candiatePlan.iCandateSize =
                    pPagedSearchContext->pTotalCandidates ? pPagedSearchContext->pTotalCandidates->size : 0;
        }
        else if (pPagedSearchContext->searchAlgo == SEARCH_ALGO_ITERATOR)
        {
            // pExecPath->IteratePlan.iNumIteration set in search code
            pExecPath->searchAlgo = SEARCH_ALGO_ITERATOR;

            dwError = VmDirAllocateStringA(pPagedSearchContext->iterContext.pszIterTable, &pExecPath->pszIndex);
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
VOID
_VmDirPagedSearchFreeMapPair(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    )
{
    // pKey is pszGuid in the pValue, will be freed as part of pValue
    pPair->pKey = NULL;
    if (pPair->pValue != NULL)
    {
        _VmDirPagedSearchContextRecordFree((PVDIR_PAGED_SEARCH_RECORD) pPair->pValue);
    }
}

static
DWORD
_VmDirPagedSearchContextRecordAdd(
    PVDIR_PAGED_SEARCH_RECORD   pPagedSearchContext
    )
{
    DWORD       dwError = 0;
    DWORD       dwCnt = 0;
    BOOLEAN     bInLock = FALSE;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};
    time_t      tNow = time(NULL);

    VMDIR_LOCK_MUTEX(bInLock, gPagedSearchCtxCache.mutex);

    // age off out dated contexts
    dwCnt = LwRtlHashMapGetCount(gPagedSearchCtxCache.pHashMap);
    if (dwCnt > VMDIR_SIZE_64)
    {
        while (LwRtlHashMapIterate(gPagedSearchCtxCache.pHashMap, &iter, &pair))
        {
            PVDIR_PAGED_SEARCH_RECORD pRec = (PVDIR_PAGED_SEARCH_RECORD)pair.pValue;

            if (tNow - pRec->tLastClientRead > gVmdirGlobals.dwLdapRecvTimeoutSec)
            {   // default 180 (3 minutes) max idle time
                VMDIR_LOG_WARNING(
                    VMDIR_LOG_MASK_ALL,
                    "%s: Paged search record aged off, cookie: %s",
                    __FUNCTION__,
                    pRec->pszGuid);

                _VmDirPagedSearchContextRecordRemoveInLock(pRec);
            }
        }
    }

    if (LwRtlHashMapGetCount(gPagedSearchCtxCache.pHashMap) >= gVmdirGlobals.dwMaxFlowCtrlThr)
    {   // throttle active page search session
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_UNWILLING_TO_PERFORM);
    }

    dwError = LwRtlHashMapInsert(
            gPagedSearchCtxCache.pHashMap,
            pPagedSearchContext->pszGuid,
            pPagedSearchContext,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gPagedSearchCtxCache.mutex);

    return dwError;

error:
    VMDIR_LOG_ERROR(
        VMDIR_LOG_MASK_ALL,
        "%s failed, error(%d)",
        __FUNCTION__,
       dwError);
    goto cleanup;
}

static
VOID
_VmDirPagedSearchContextRecordRemoveInLock(
    PVDIR_PAGED_SEARCH_RECORD   pPagedSearchContext
    )
{
    if (pPagedSearchContext)
    {
        (VOID) LwRtlHashMapRemove(
                   gPagedSearchCtxCache.pHashMap,
                   pPagedSearchContext->pszGuid,
                   NULL);

        _VmDirPagedSearchContextRecordFree(pPagedSearchContext);
    }
}

static
VOID
_VmDirPagedSearchContextRecordRemove(
    PVDIR_PAGED_SEARCH_RECORD   pPagedSearchContext
    )
{
    BOOLEAN bInLock = FALSE;

    if (pPagedSearchContext)
    {
        VMDIR_LOCK_MUTEX(bInLock, gPagedSearchCtxCache.mutex);

        _VmDirPagedSearchContextRecordRemoveInLock(pPagedSearchContext);

        VMDIR_UNLOCK_MUTEX(bInLock, gPagedSearchCtxCache.mutex);
    }
}

static
VOID
_VmDirPagedSearchContextRecordFree(
    PVDIR_PAGED_SEARCH_RECORD pPagedSearchContext
    )
{
    if (pPagedSearchContext)
    {
        VmDirFreeStringA(pPagedSearchContext->pszGuid);
        DeleteCandidates(&pPagedSearchContext->pTotalCandidates);
        VmDirFreeBervalContent(&pPagedSearchContext->bvStrFilter);
        VmDirIterContextFreeContent(&pPagedSearchContext->iterContext);

        VMDIR_SAFE_FREE_MEMORY(pPagedSearchContext);
    }
}

static
DWORD
_VmDirPagedSearchGetContext(
    PVDIR_OPERATION            pOperation,
    PVDIR_PAGED_SEARCH_RECORD *ppPagedSearchContext
    )
{
    DWORD    dwError = 0;
    PSTR     pszCookie = NULL;
    PVDIR_PAGED_SEARCH_RECORD pPagedSearchContext = NULL;

    if (pOperation == NULL ||
        ppPagedSearchContext == NULL ||
        pOperation->showPagedResultsCtrl == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NO_PAGED_SEARCH_CONTEXT);
    }

    pszCookie = pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.cookie;

    if (IsNullOrEmptyString(pszCookie))
    {
        dwError = _VmDirPagedSearchContextNodeAllocate(pOperation, &pPagedSearchContext);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = _VmDirPagedSearchGetContextUsingCookie(pszCookie, &pPagedSearchContext);
        BAIL_ON_VMDIR_ERROR(dwError);

        /*
         * RFC 2696: subsequent page search requests could have different page size, update the page size
         * for every page search.
         */
        pPagedSearchContext->dwPageSize = pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.pageSize;
    }

    VMDIR_LOG_VERBOSE(
        LDAP_DEBUG_TRACE,
        "%s: Paged search record found for cookie: %s",
        __FUNCTION__,
        pszCookie);

    *ppPagedSearchContext = pPagedSearchContext;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
        VMDIR_LOG_MASK_ALL,
        "%s failed, error(%d)",
        __FUNCTION__,
        dwError);
    goto cleanup;
}

static
DWORD
_VmDirPagedSearchGetContextUsingCookie(
    PCSTR                       pszCookie,
    PVDIR_PAGED_SEARCH_RECORD*  ppPagedSearchContext
    )
{
    DWORD   dwError  = 0;
    BOOLEAN bInLock = FALSE;
    time_t  tNow = time(NULL);
    PVDIR_PAGED_SEARCH_RECORD pPagedSearchContext = NULL;

    if (IsNullOrEmptyString(pszCookie) || ppPagedSearchContext == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_LOCK_MUTEX(bInLock, gPagedSearchCtxCache.mutex);

    dwError = LwRtlHashMapFindKey(
                    gPagedSearchCtxCache.pHashMap,
                    (PVOID*)&pPagedSearchContext,
                    pszCookie);
    BAIL_ON_VMDIR_ERROR(dwError);

    // update time stamp
    pPagedSearchContext->tLastClientRead = tNow;

    *ppPagedSearchContext = pPagedSearchContext;

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gPagedSearchCtxCache.mutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(
        VMDIR_LOG_MASK_ALL,
        "%s failed, error(%d)",
        __FUNCTION__,
        dwError);
    //change to appropriate error code for client
    dwError = VMDIR_ERROR_NO_PAGED_SEARCH_CONTEXT;
    goto cleanup;
}

static
DWORD
_VmDirPagedSearchValidateRequest(
    PVDIR_OPERATION            pOperation,
    PVDIR_PAGED_SEARCH_RECORD  pPagedSearchContext
    )
{
    DWORD dwError = 0;
    VDIR_BERVALUE strFilter = VDIR_BERVALUE_INIT;

    if (pOperation == NULL || pPagedSearchContext == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (pPagedSearchContext->dwPageSize == 0)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PAGED_SEARCH_REQUEST);
    }

    strFilter.bOwnBvVal = TRUE;
    // skip validation for the first iteration
    if (pPagedSearchContext->dwCandidatesProcessed != 0)
    {
        dwError = FilterToStrFilter(pOperation->request.searchReq.filter, &strFilter);
        BAIL_ON_VMDIR_ERROR(dwError);

        /*
         * RFC 2696: Subsequent search request should have the same filter
         * if not invalid page search request
         */
        if (VmDirStringCompareA(strFilter.lberbv.bv_val,
                pPagedSearchContext->bvStrFilter.lberbv.bv_val,
                TRUE) != 0)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PAGED_SEARCH_REQUEST);
        }
    }

cleanup:
    VmDirFreeBervalContent(&strFilter);
    return dwError;

error:
    VMDIR_LOG_ERROR(
        VMDIR_LOG_MASK_ALL,
        "%s failed, error(%d)",
        __FUNCTION__,
       dwError);
    goto cleanup;
}

static
DWORD
_VmDirPagedSearchContextNodeAllocate(
    PVDIR_OPERATION            pOperation,
    PVDIR_PAGED_SEARCH_RECORD  *ppPagedSearchContext
    )
{
    DWORD dwError = 0;
    PVDIR_PAGED_SEARCH_RECORD pPagedSearchContext = NULL;
    char szGuidStr[VMDIR_GUID_STR_LEN] = {0};
    uuid_t guid = {0};
    SearchReq *pSReq = &pOperation->request.searchReq;

    if (pOperation == NULL || ppPagedSearchContext == NULL || pOperation->showPagedResultsCtrl == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(sizeof(*pPagedSearchContext), (PVOID)&pPagedSearchContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirUuidGenerate(&guid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirUuidToStringLower(&guid, szGuidStr, sizeof(szGuidStr));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(szGuidStr, &pPagedSearchContext->pszGuid);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Update ownership of ber value
    pPagedSearchContext->bvStrFilter.bOwnBvVal = TRUE;

    /*
     * Capture the original search filter and mark it as undeletable. Because
     * the ava strings aren't allocated separately but are part of the request
     * we have to specifically clone them.
     */
    dwError = FilterToStrFilter(pOperation->request.searchReq.filter, &pPagedSearchContext->bvStrFilter);
    BAIL_ON_VMDIR_ERROR(dwError);

    pPagedSearchContext->dwPageSize = pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.pageSize;

    if (pSReq->srvExecPath.searchAlgo == SEARCH_ALGO_ITERATOR)
    {
        pPagedSearchContext->searchAlgo = SEARCH_ALGO_ITERATOR;
        dwError = VmDirIterContextInitContent(
                &pPagedSearchContext->iterContext,
                &pSReq->iteratorSearchPlan);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (pSReq->srvExecPath.searchAlgo == SEARCH_ALGO_CANDIDATE_LIST)
    {
        pPagedSearchContext->searchAlgo = SEARCH_ALGO_CANDIDATE_LIST;
        pPagedSearchContext->dwCandidatesProcessed = 0;

        pPagedSearchContext->pTotalCandidates = pOperation->request.searchReq.filter->candidates;
        pOperation->request.searchReq.filter->candidates = NULL;
        VmDirSortCandidateList(pPagedSearchContext->pTotalCandidates);
    }

    pPagedSearchContext->tLastClientRead = time(NULL);

    dwError = _VmDirPagedSearchContextRecordAdd(pPagedSearchContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppPagedSearchContext = pPagedSearchContext;

cleanup:
    return dwError;

error:
    _VmDirPagedSearchContextRecordFree(pPagedSearchContext);
    goto cleanup;
}

static
DWORD
_VmDirNextPageViaCandidateList(
    PVDIR_OPERATION            pOperation,
    PVDIR_PAGED_SEARCH_RECORD  pPagedSearchContext
    )
{
    DWORD        dwError = 0;
    DWORD        entryCount = 0;
    ENTRYID      eId = 0;
    PVDIR_ENTRY  pEntry = NULL;
    BOOLEAN      bEntrySent = FALSE;
    BOOLEAN      bStoreRsltInMem = FALSE;

    if (pPagedSearchContext == NULL || pOperation == NULL )
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    bStoreRsltInMem = (pOperation->opType == VDIR_OPERATION_TYPE_INTERNAL) ||
                      pOperation->request.searchReq.bStoreRsltInMem;

    if (!pPagedSearchContext->pTotalCandidates  ||
        pPagedSearchContext->pTotalCandidates->size == 0)
    {
        pPagedSearchContext->bCompete = TRUE;
        goto cleanup;
    }

    while (entryCount < pPagedSearchContext->dwPageSize &&
           pPagedSearchContext->dwCandidatesProcessed < pPagedSearchContext->pTotalCandidates->size)
    {
        bEntrySent = FALSE;

        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_UNAVAILABLE);
        }

        if (bStoreRsltInMem)
        {
            dwError = VmDirAllocOrReallocEntryArray(&pOperation->internalSearchEntryArray);
            BAIL_ON_VMDIR_ERROR(dwError);
            pEntry = (pOperation->internalSearchEntryArray.pEntry + pOperation->internalSearchEntryArray.iSize);
        }

        eId = pPagedSearchContext->pTotalCandidates->eIds[pPagedSearchContext->dwCandidatesProcessed];

        if (eId != pPagedSearchContext->lastEId)
        {
            pPagedSearchContext->lastEId = eId;
            dwError = VmDirCheckAndSendEntry(
                    eId,
                    pOperation,
                    pEntry,
                    &bEntrySent);
            BAIL_ON_VMDIR_ERROR(dwError);

            if (bEntrySent)
            {
               entryCount++;
            }
        }
        pPagedSearchContext->dwCandidatesProcessed++;
     }

     if (pPagedSearchContext->dwCandidatesProcessed >= pPagedSearchContext->pTotalCandidates->size)
     {
         pPagedSearchContext->bCompete = TRUE;
     }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s failed, error(%d)", __FUNCTION__, dwError);
    goto cleanup;
}

BOOLEAN
VmDirCheckEidInMap(
    PLW_HASHMAP     pMap,
    ENTRYID         eId
    )
{
    DWORD   dwError = 0;
    BOOLEAN bIdSent = FALSE;
    CHAR    eidStr[VMDIR_SIZE_64] = {0};

    if (pMap)
    {
        dwError = VmDirStringPrintFA(eidStr, sizeof(eidStr), "%llu", eId);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapFindKey(pMap, NULL, eidStr);
        BAIL_ON_VMDIR_ERROR(dwError);

        bIdSent = TRUE;
    }

cleanup:
    return bIdSent;

error:
    goto cleanup;
}

DWORD
VmDirAddEidToMap(
    PLW_HASHMAP     pMap,
    ENTRYID         eId
    )
{
    DWORD   dwError = 0;
    PSTR    pEidStr = NULL;

    if (pMap)
    {
        dwError = VmDirAllocateStringPrintf(&pEidStr,  "%llu", eId);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapInsert(pMap, pEidStr, (PVOID)pEidStr, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pEidStr);
    goto cleanup;
}

static
DWORD
_VmDirNextPageViaIterator(
    PVDIR_OPERATION            pOperation,
    PVDIR_PAGED_SEARCH_RECORD  pPagedSearchContext
    )
{
    DWORD        dwError = 0;
    DWORD        entryCount = 0;
    PVDIR_ENTRY  pEntry = NULL;
    BOOLEAN bEntrySent = FALSE;
    PVDIR_INDEX_CFG pIndexCfg = NULL;
    PVDIR_BACKEND_INDEX_ITERATOR  pIterator = NULL;
    BOOLEAN     bAdminBind = FALSE;
    BOOLEAN     bStoreRsltInMem = FALSE;
    PVDIR_SEARCH_EXEC_PATH pExecPath = &pOperation->request.searchReq.srvExecPath;

    bStoreRsltInMem = pOperation->opType == VDIR_OPERATION_TYPE_INTERNAL ||
                      pOperation->request.searchReq.bStoreRsltInMem;

    dwError = VmDirIndexCfgAcquire(
            pPagedSearchContext->iterContext.pszIterTable,
            VDIR_INDEX_READ,
            &pIndexCfg);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = pOperation->pBEIF->pfnBEIndexIteratorInit(
            pIndexCfg,
            &pPagedSearchContext->iterContext,
            &pIterator);
    BAIL_ON_VMDIR_ERROR(dwError);

    bAdminBind = pOperation->opType == VDIR_OPERATION_TYPE_INTERNAL ||
                 VMDIR_IS_ADMIN_OR_DC_GROUP_MEMBER(pOperation->conn->AccessInfo.accessRoleBitmap);

    while (pIterator->bHasNext && entryCount < pPagedSearchContext->dwPageSize)
    {
        bEntrySent = FALSE;

        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            goto cleanup;
        }

        if (gVmdirGlobals.dwMaxSearchIteration  &&
            !bAdminBind                         &&
            pPagedSearchContext->iterContext.iIterCount > gVmdirGlobals.dwMaxSearchIteration)
        {
            pExecPath->bExceedMaxIteration = TRUE;

            dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
            VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s reached upper limit (%d) for iterator search (%d)",
                             __FUNCTION__, gVmdirGlobals.dwMaxSearchIteration, dwError);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (!VmDirCheckEidInMap(pPagedSearchContext->iterContext.pSentIDMap,
                pPagedSearchContext->iterContext.eId))
        {
            if (bStoreRsltInMem)
            {
                dwError = VmDirAllocOrReallocEntryArray(&pOperation->internalSearchEntryArray);
                BAIL_ON_VMDIR_ERROR(dwError);
                pEntry = (pOperation->internalSearchEntryArray.pEntry + pOperation->internalSearchEntryArray.iSize);
            }

            dwError = VmDirCheckAndSendEntry(
                    pPagedSearchContext->iterContext.eId,
                    pOperation,
                    pEntry,
                    &bEntrySent);
            BAIL_ON_VMDIR_ERROR(dwError);

            if (bEntrySent)
            {
                dwError = VmDirAddEidToMap(
                        pPagedSearchContext->iterContext.pSentIDMap,
                        pPagedSearchContext->iterContext.eId);
                BAIL_ON_VMDIR_ERROR(dwError);

                entryCount++;
            }
        }
        else
        {
            VMDIR_LOG_VERBOSE(LDAP_DEBUG_FILTER, "%s: eid already sent eId %llu",
                    __func__, pPagedSearchContext->iterContext.eId);
        }

        dwError = pOperation->pBEIF->pfnBEIndexIterate(pIterator, &pPagedSearchContext->iterContext);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pIterator->bHasNext == FALSE)
    {
        pPagedSearchContext->bCompete = TRUE;
    }

    pExecPath->IteratePlan.iNumIteration = pPagedSearchContext->iterContext.iIterCount;

cleanup:
    VmDirIndexCfgRelease(pIndexCfg);
    if (pIterator)
    {
        pOperation->pBEIF->pfnBEIndexIteratorFree(pIterator);
    }

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s failed, error(%d)", __FUNCTION__, dwError);
    goto cleanup;
}

static
DWORD
_VmDirPagedSearchUpdateCookie(
    PVDIR_OPERATION            pOperation,
    PVDIR_PAGED_SEARCH_RECORD  pPagedSearchContext
    )
{
    DWORD dwError = 0;

    if (IsNullOrEmptyString(pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.cookie))
    {
        dwError = VmDirStringCpyA(
                  pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.cookie,
                  VMDIR_ARRAY_SIZE(pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.cookie),
                  pPagedSearchContext->pszGuid);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pPagedSearchContext->bCompete)
    {
        pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.cookie[0] = '\0';
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s failed, error(%d)", __FUNCTION__, dwError);
    goto cleanup;
}
