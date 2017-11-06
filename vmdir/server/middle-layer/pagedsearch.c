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
VOID
_VmDirPagedSearchContextRecordFree(
    PVDIR_PAGED_SEARCH_RECORD  pPagedSearchContext
    );

static
VOID
_VmDirPagedSearchContextRecordFreeInLock(
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
BOOLEAN
_VmDirPagedSearchUpdateInProgressFlag(
    PVDIR_PAGED_SEARCH_RECORD  pPagedSearchContext,
    BOOLEAN                    bUpdateValue
    );

static
DWORD
_VmDirProcessNextPage(
    PVDIR_OPERATION            pOperation,
    PVDIR_PAGED_SEARCH_RECORD  pPagedSearchContext
    );

static
DWORD
_VmDirPagedSearchGetEntryID(
    PVDIR_PAGED_SEARCH_RECORD pPagedSearchContext,
    ENTRYID*                  pEId
    );

static
DWORD
_VmDirPagedSearchPerformActionOnFilteredCandidate(
    PVDIR_OPERATION pOperation,
    PVDIR_ENTRY     pEntry
    );

static
DWORD
_VmDirPagedSearchUpdateCookie(
    PVDIR_OPERATION            pOperation,
    PVDIR_PAGED_SEARCH_RECORD  pPagedSearchContext,
    PBOOLEAN                   pbSearchComplete
    );

static
DWORD
_VmDirPagedSearchValidateRequest(
    PVDIR_OPERATION            pOperation,
    PVDIR_PAGED_SEARCH_RECORD  pPagedSearchContext
    );

/*
static
VOID
_VmDirPagedSearchFreeStaleContextFromCache(
    VOID
    );
*/
DWORD
VmDirProcessPagedSearch(
    PVDIR_OPERATION  pOperation
    )
{
    DWORD dwError = 0;
    BOOLEAN bSearchComplete = FALSE;
    PVDIR_PAGED_SEARCH_RECORD pPagedSearchContext = NULL;

    if (pOperation == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_UNAVAILABLE);
    }

    dwError = _VmDirPagedSearchGetContext(pOperation, &pPagedSearchContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirPagedSearchValidateRequest(pOperation, pPagedSearchContext);
    BAIL_ON_VMDIR_ERROR(dwError);

   /*
    * TODO - Add logic to free stale page context entries (abandoned by client)
    * in the next page search task implementation
    *
    * _VmDirPagedSearchFreeStaleContextFromCache();
    */

    dwError = _VmDirProcessNextPage(pOperation, pPagedSearchContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirPagedSearchUpdateCookie(pOperation, pPagedSearchContext, &bSearchComplete);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (bSearchComplete == TRUE)
    {
        _VmDirPagedSearchContextRecordFree(pPagedSearchContext);
    }
    else
    {
        _VmDirPagedSearchUpdateInProgressFlag(pPagedSearchContext, FALSE);
    }

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

/*static
VOID
_VmDirPagedSearchFreeStaleContextFromCache(
    VOID
    )
{
    BOOLEAN         bInLock = FALSE;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};

    VMDIR_LOCK_MUTEX(bInLock, gPagedSearchCtxCache.mutex);
    while (LwRtlHashMapIterate(gPagedSearchCtxCache.pHashMap, &iter, &pair))
    {
        PVDIR_PAGED_SEARCH_RECORD pPagedSearchContext = (PVDIR_PAGED_SEARCH_RECORD) pair.pValue;

        if (pPagedSearchContext != NULL)
        {
            if (time(NULL) - pPagedSearchContext->tLastClientRead >= SECONDS_IN_HOUR &&
                pPagedSearchContext->bSearchInProgress == FALSE)
            {
                _VmDirPagedSearchContextRecordFreeInLock(pPagedSearchContext);
            }
        }
    }
    VMDIR_UNLOCK_MUTEX(bInLock, gPagedSearchCtxCache.mutex);
}*/

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
        _VmDirPagedSearchContextRecordFreeInLock((PVDIR_PAGED_SEARCH_RECORD) pPair->pValue);
    }
}

static
VOID
_VmDirPagedSearchContextRecordFree(
    PVDIR_PAGED_SEARCH_RECORD pPagedSearchContext
    )
{
    BOOLEAN bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gPagedSearchCtxCache.mutex);
    _VmDirPagedSearchContextRecordFreeInLock(pPagedSearchContext);
    VMDIR_UNLOCK_MUTEX(bInLock, gPagedSearchCtxCache.mutex);

}

static
VOID
_VmDirPagedSearchContextRecordFreeInLock(
    PVDIR_PAGED_SEARCH_RECORD pPagedSearchContext
    )
{
    if (pPagedSearchContext == NULL)
    {
        return;
    }

    //Remove from hashMap
    (VOID) LwRtlHashMapRemove(gPagedSearchCtxCache.pHashMap,
               pPagedSearchContext->pszGuid,
               NULL);

    //Free Mutex
    VMDIR_SAFE_FREE_MUTEX(pPagedSearchContext->mutex);

    //Free cookie
    VmDirFreeStringA(pPagedSearchContext->pszGuid);
    //Free pFilter and pCandidates
    DeleteFilter(pPagedSearchContext->pFilter);
    //Free BerVal
    VmDirFreeBervalContent(&pPagedSearchContext->bvStrFilter);
    //Free the context structure
    VMDIR_SAFE_FREE_MEMORY(pPagedSearchContext);
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

    //pszCookie cannot be NULL since, it is an char array
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

        if (_VmDirPagedSearchUpdateInProgressFlag(pPagedSearchContext, TRUE) == FALSE)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PAGED_SEARCH_REQUEST);
        }

        /*
         * RFC 2696: subsequent page search requests could have different page size, update the page size
         * for every page search.
         */
        pPagedSearchContext->dwPageSize = pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.pageSize;
        //update time stamp
        pPagedSearchContext->tLastClientRead = time(NULL);
    }

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
    DWORD  dwError  = 0;
    BOOLEAN bInLock = FALSE;
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
BOOLEAN
_VmDirPagedSearchUpdateInProgressFlag(
    PVDIR_PAGED_SEARCH_RECORD  pPagedSearchContext,
    BOOLEAN                    bUpdateValue
    )
{
    DWORD   dwError  = 0;
    BOOLEAN bInLock  = FALSE;
    BOOLEAN bSuccess = FALSE;

    if (pPagedSearchContext == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_LOCK_MUTEX(bInLock, pPagedSearchContext->mutex);
    if ( (bUpdateValue == TRUE && pPagedSearchContext->bSearchInProgress == FALSE) ||
         (bUpdateValue == FALSE && pPagedSearchContext->bSearchInProgress == TRUE) )
    {
        pPagedSearchContext->bSearchInProgress = bUpdateValue;
        bSuccess = TRUE;
    }
    VMDIR_UNLOCK_MUTEX(bInLock, pPagedSearchContext->mutex);

cleanup:
    return bSuccess;

error:
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
    BOOLEAN bInLock = FALSE;
    PVDIR_PAGED_SEARCH_RECORD pPagedSearchContext = NULL;
    char szGuidStr[VMDIR_GUID_STR_LEN] = {0};
    uuid_t guid = {0};

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

    dwError = StrFilterToFilter(pPagedSearchContext->bvStrFilter.lberbv.bv_val, &pPagedSearchContext->pFilter);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&pPagedSearchContext->mutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    pPagedSearchContext->pTotalCandidates = pOperation->request.searchReq.filter->candidates;
    pOperation->request.searchReq.filter->candidates = NULL;
    pPagedSearchContext->dwPageSize = pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.pageSize;

    pPagedSearchContext->dwCandidatesProcessed = 0;

    pPagedSearchContext->tLastClientRead = time(NULL);

    VMDIR_LOCK_MUTEX(bInLock, gPagedSearchCtxCache.mutex);

    dwError = LwRtlHashMapInsert(gPagedSearchCtxCache.pHashMap,
                pPagedSearchContext->pszGuid, pPagedSearchContext, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppPagedSearchContext = pPagedSearchContext;

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gPagedSearchCtxCache.mutex);
    return dwError;
error:
    _VmDirPagedSearchContextRecordFreeInLock(pPagedSearchContext);
    goto cleanup;
}


static
DWORD
_VmDirProcessNextPage(
    PVDIR_OPERATION            pOperation,
    PVDIR_PAGED_SEARCH_RECORD  pPagedSearchContext
    )
{
    DWORD        dwError = 0;
    DWORD        entryCount = 0;
    ENTRYID      eId = 0;
    VDIR_ENTRY   entry = {0};

    if (pPagedSearchContext == NULL || pOperation == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    while (entryCount < pPagedSearchContext->dwPageSize &&
           pPagedSearchContext->dwCandidatesProcessed < pPagedSearchContext->pTotalCandidates->size)
    {
        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_UNAVAILABLE);
        }

        dwError = _VmDirPagedSearchGetEntryID(pPagedSearchContext, &eId);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = pOperation->pBEIF->pfnBESimpleIdToEntry(eId, &entry);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (CheckIfEntryPassesFilter(pOperation, &entry, pOperation->request.searchReq.filter) == FILTER_RES_TRUE)
        {
            dwError = _VmDirPagedSearchPerformActionOnFilteredCandidate(pOperation, &entry);
            BAIL_ON_VMDIR_ERROR(dwError);

            entryCount++;
        }
        pPagedSearchContext->dwCandidatesProcessed++;
    }

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
_VmDirPagedSearchUpdateCookie(
    PVDIR_OPERATION            pOperation,
    PVDIR_PAGED_SEARCH_RECORD  pPagedSearchContext,
    PBOOLEAN                   pbSearchComplete
    )
{
    DWORD dwError = 0;

    if (pPagedSearchContext == NULL ||
        pOperation == NULL ||
        pbSearchComplete == NULL ||
        pOperation->showPagedResultsCtrl == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_LOG_VERBOSE(
        LDAP_DEBUG_FILTER,
        "%s dwCandidatesProcessed:%d totalsize: %d",
        __FUNCTION__,
        pPagedSearchContext->dwCandidatesProcessed,
        pPagedSearchContext->pTotalCandidates->size);

    if (IsNullOrEmptyString(pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.cookie))
    {
        dwError = VmDirStringCpyA(
                  pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.cookie,
                  VMDIR_ARRAY_SIZE(pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.cookie),
                  pPagedSearchContext->pszGuid
                  );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pPagedSearchContext->dwCandidatesProcessed >= pPagedSearchContext->pTotalCandidates->size)
    {
        pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.cookie[0] = '\0';
        *pbSearchComplete = TRUE;
    }

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
_VmDirPagedSearchGetEntryID(
    PVDIR_PAGED_SEARCH_RECORD  pPagedSearchContext,
    ENTRYID*                   pEId
    )
{
    DWORD   dwError = 0;

    if (pEId == NULL ||
        pPagedSearchContext == NULL ||
        pPagedSearchContext->dwCandidatesProcessed >= pPagedSearchContext->pTotalCandidates->size)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    *pEId = pPagedSearchContext->pTotalCandidates->eIds[pPagedSearchContext->dwCandidatesProcessed];

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
_VmDirPagedSearchPerformActionOnFilteredCandidate(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry
    )
{
    DWORD dwError = 0;

    if (pOperation == NULL || pEntry == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirBuildComputedAttribute(pOperation, pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSendSearchEntry(pOperation, pEntry);
    if (dwError == VMDIR_ERROR_INSUFFICIENT_ACCESS)
    {
        VMDIR_LOG_WARNING(
            VMDIR_LOG_MASK_ALL,
            "Access deny on search entry result [%s,%d] (bindedDN-%s) (targetDn-%s)",
            __FILE__,
            __LINE__,
            pOperation->conn->AccessInfo.pszBindedDn,
            pEntry->dn.lberbv.bv_val);

        // make sure search continues
        dwError = 0;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

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
