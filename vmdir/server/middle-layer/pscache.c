/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
 * Module Name: Directory middle layer
 *
 * Filename: pscache.c
 *
 * Abstract:
 *
 * Paged search cache
 *
 */

#include "includes.h"

static
DWORD
_VmDirPagedSearchWorkerThread(
    PVOID pArg
    );

static
VOID
VmDirPagedSearchCacheRecordFree(
    PVDIR_PAGED_SEARCH_RECORD pSearchRecord
    );

static
LW_PCVOID
PagedSearchRecordGetKey(
    PLW_HASHTABLE_NODE      pNode,
    PVOID                   pUnused
    )
{
    PVDIR_PAGED_SEARCH_RECORD pSearchRecord = NULL;

    pSearchRecord = LW_STRUCT_FROM_FIELD(pNode, VDIR_PAGED_SEARCH_RECORD, Node);

    return pSearchRecord->pszGuid;
}

VOID
VmDirPagedSearchCacheFree(
    VOID
    )
{
    PLW_HASHTABLE_NODE pNode = NULL;
    LW_HASHTABLE_ITER iter = LW_HASHTABLE_ITER_INIT;
    PVDIR_PAGED_SEARCH_RECORD pSearchRecord = NULL;

    if (gPagedSearchCache.pHashTbl != NULL)
    {
        while ((pNode = LwRtlHashTableIterate(gPagedSearchCache.pHashTbl, &iter)))
        {
            pSearchRecord = LW_STRUCT_FROM_FIELD(pNode, VDIR_PAGED_SEARCH_RECORD, Node);
            LwRtlHashTableRemove(gPagedSearchCache.pHashTbl, pNode);
            VmDirPagedSearchCacheRecordFree(pSearchRecord);
        }

        LwRtlFreeHashTable(&gPagedSearchCache.pHashTbl);
    }

    VMDIR_SAFE_FREE_MUTEX(gPagedSearchCache.mutex);
}

DWORD
VmDirPagedSearchCacheInit(
    VOID
    )
{
    DWORD dwError = 0;

    dwError = VmDirAllocateMutex(&gPagedSearchCache.mutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashTable(
                    &gPagedSearchCache.pHashTbl,
                    PagedSearchRecordGetKey,
                    LwRtlHashDigestPstr,
                    LwRtlHashEqualPstr,
                    NULL,
                    VMDIR_PAGED_SEARCH_CACHE_HASH_TABLE_SIZE);
    dwError = LwNtStatusToWin32Error(dwError);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;
error:
    VmDirPagedSearchCacheFree();
    goto cleanup;
}

static
VOID
_VmDirPagedSearchEntryListFree(
    PVDIR_PAGED_SEARCH_ENTRY_LIST pEntryList
    )
{
    if (pEntryList)
    {
        VMDIR_SAFE_FREE_MEMORY(pEntryList->pEntryIds);
        VmDirFreeMemory(pEntryList);
    }
}

static
DWORD
VmDirPagedSearchCreateThread(
    PVDIR_PAGED_SEARCH_RECORD pSearchRecord
    )
{
    DWORD dwError = 0;
    PVDIR_THREAD_INFO pThrInfo = NULL;

    dwError = VmDirSrvThrInit(&pThrInfo, NULL, NULL, FALSE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateThread(
                &pThrInfo->tid,
                pThrInfo->bJoinThr,
                _VmDirPagedSearchWorkerThread,
                pSearchRecord);
    BAIL_ON_VMDIR_ERROR(dwError);

    pSearchRecord->pThreadInfo = pThrInfo;

cleanup:
    return dwError;

error:
    VmDirSrvThrFree(pThrInfo);
    goto cleanup;
}

static
VOID
VmDirPagedSearchCacheRecordFree(
    PVDIR_PAGED_SEARCH_RECORD pSearchRecord
    )
{
    PVOID pvData = NULL;
    BOOLEAN bInLock = FALSE;

    if (pSearchRecord == NULL)
    {
        return;
    }

    VMDIR_LOCK_MUTEX(bInLock, gPagedSearchCache.mutex);
    (VOID)LwRtlHashTableRemove(gPagedSearchCache.pHashTbl, &pSearchRecord->Node);
    VMDIR_UNLOCK_MUTEX(bInLock, gPagedSearchCache.mutex);

    VmDirFreeStringA(pSearchRecord->pszGuid);

    DeleteFilter(pSearchRecord->pFilter);

    while (dequePop(pSearchRecord->pQueue, (PVOID*)&pvData) == 0)
    {
        VmDirFreeMemory(pvData);
    }
    dequeFree(pSearchRecord->pQueue);

    VmDirFreeMutex(pSearchRecord->mutex);
    VmDirFreeCondition(pSearchRecord->pDataAvailable);

    VmDirSrvThrFree(pSearchRecord->pThreadInfo);

    VmDirFreeMemory(pSearchRecord);
}

static
VOID
_RefPagedSearchRecord(
    PVDIR_PAGED_SEARCH_RECORD pSearchRecord
    )
{
    InterlockedIncrement(&pSearchRecord->dwRefCount);
}

static
VOID
_DerefPagedSearchRecord(
    PVDIR_PAGED_SEARCH_RECORD pSearchRecord
    )
{
    if (InterlockedDecrement(&pSearchRecord->dwRefCount) == 0)
    {
        VmDirPagedSearchCacheRecordFree(pSearchRecord);
    }
}

static
DWORD
VmDirPagedSearchCacheNodeAllocate(
    PVDIR_PAGED_SEARCH_RECORD *ppSearchRec,
    PVDIR_OPERATION pOperation,
    DWORD dwCandidatesProcessed
    )
{
    DWORD dwError = 0;
    PVDIR_PAGED_SEARCH_RECORD pSearchRecord = NULL;
    char szGuidStr[VMDIR_GUID_STR_LEN] = {0};
    uuid_t guid = {0};
    VDIR_BERVALUE strFilter = VDIR_BERVALUE_INIT;

    dwError = VmDirAllocateMemory(sizeof(*pSearchRecord), (PVOID)&pSearchRecord);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirUuidGenerate(&guid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirUuidToStringLower(&guid, szGuidStr, sizeof(szGuidStr));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(szGuidStr, &pSearchRecord->pszGuid);
    BAIL_ON_VMDIR_ERROR(dwError);

    //
    // Capture the original search filter and mark it as undeletable. Because
    // the ava strings aren't allocated separately but are part of the request
    // we have to specifically clone them.
    //
    dwError = FilterToStrFilter(pOperation->request.searchReq.filter, &strFilter);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = StrFilterToFilter(strFilter.lberbv.bv_val, &pSearchRecord->pFilter);
    BAIL_ON_VMDIR_ERROR(dwError);

    pSearchRecord->pTotalCandidates = pOperation->request.searchReq.filter->candidates;
    pOperation->request.searchReq.filter->candidates = NULL;
    pSearchRecord->dwPageSize = pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.pageSize;
    //
    // dwCandidatesProcessed is the count of entries of pFilter->candidates
    // that we went through building up the first page of results. We add
    // one because we want to start our next search past the point of where
    // we've already been.
    //
    pSearchRecord->dwCandidatesProcessed = dwCandidatesProcessed + 1;

    dwError = dequeCreate(&pSearchRecord->pQueue);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&pSearchRecord->mutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateCondition(&pSearchRecord->pDataAvailable);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirPagedSearchCreateThread(pSearchRecord);
    BAIL_ON_VMDIR_ERROR(dwError);

    LwRtlHashTableResizeAndInsert(
                gPagedSearchCache.pHashTbl,
                &pSearchRecord->Node,
                NULL);

    pSearchRecord->dwRefCount = 1;

    pSearchRecord->tLastClientRead = time(NULL);

    *ppSearchRec = pSearchRecord;

cleanup:
    VmDirFreeBervalContent(&strFilter);
    return dwError;
error:
    VmDirPagedSearchCacheRecordFree(pSearchRecord);
    goto cleanup;
}

DWORD
VmDirPagedSearchCacheInsert(
    PVDIR_OPERATION pOperation,
    DWORD dwCandidatesProcessed
    )
{
    DWORD dwError = 0;
    PVDIR_PAGED_SEARCH_RECORD pSearchRecord = NULL;

    if (*pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.cookie == '\0')
    {
        dwError = VmDirPagedSearchCacheNodeAllocate(
                    &pSearchRecord,
                    pOperation,
                    dwCandidatesProcessed);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirStringCpyA(
                    pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.cookie,
                    VMDIR_ARRAY_SIZE(pOperation->showPagedResultsCtrl->value.pagedResultCtrlVal.cookie),
                    pSearchRecord->pszGuid
                    );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;
error:
    VmDirPagedSearchCacheRecordFree(pSearchRecord);
    goto cleanup;
}

static
PVDIR_PAGED_SEARCH_RECORD
VmDirPagedSearchCacheFind(
    PCSTR pszCookie
    )
{
    DWORD dwError = 0;
    PLW_HASHTABLE_NODE pNode = NULL;
    PVDIR_PAGED_SEARCH_RECORD pSearchRecord = NULL;
    BOOLEAN bInLock = FALSE;

    if (IsNullOrEmptyString(pszCookie))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_LOCK_MUTEX(bInLock, gPagedSearchCache.mutex);
    dwError = LwRtlHashTableFindKey(
                    gPagedSearchCache.pHashTbl,
                    &pNode,
                    (PVOID)pszCookie);
    dwError = LwNtStatusToWin32Error(dwError);
    BAIL_ON_VMDIR_ERROR(dwError);

    pSearchRecord = LW_STRUCT_FROM_FIELD(pNode, VDIR_PAGED_SEARCH_RECORD, Node);
    _RefPagedSearchRecord(pSearchRecord);

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gPagedSearchCache.mutex);
    return pSearchRecord;
error:
    goto cleanup;
}

static
VOID
VmDirPagedSearchCacheCullWorkerThread(
    PVDIR_PAGED_SEARCH_RECORD pSearchRecord
    )
{
    VmDirSrvThrSignal(pSearchRecord->pThreadInfo);
}

static
VOID
VmDirPagedSearchCacheReaderSignal_inlock(
    PVDIR_PAGED_SEARCH_RECORD pSearchRecord
    )
{
    VmDirConditionSignal(pSearchRecord->pDataAvailable);
}

static
DWORD
VmDirPagedSearchCacheAddData(
    PVDIR_PAGED_SEARCH_RECORD pSearchRecord,
    PVDIR_PAGED_SEARCH_ENTRY_LIST pEntryList
    )
{
    DWORD dwError = 0;
    BOOLEAN bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, pSearchRecord->mutex);
    dwError = dequePush(pSearchRecord->pQueue, pEntryList);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirPagedSearchCacheReaderSignal_inlock(pSearchRecord);
cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, pSearchRecord->mutex);
    return dwError;
error:
    goto cleanup;
}

/*
 * Waits until there's data from the worker thread to read (or until the
 * server's shutdown).
 */
static
DWORD
_VmDirPagedSearchCacheWaitAndRead_inlock(
    PVDIR_PAGED_SEARCH_RECORD pSearchRecord,
    PVDIR_PAGED_SEARCH_ENTRY_LIST *ppEntryList
    )
{
    DWORD dwError = 0;
    PVDIR_PAGED_SEARCH_ENTRY_LIST pEntryList = NULL;

    while (TRUE)
    {
        dwError = dequePopLeft(pSearchRecord->pQueue, (PVOID*)&pEntryList);
        if (dwError == 0)
        {
            break;
        }
        else if (pSearchRecord->bProcessingCompleted)
        {
            //
            // Nothing in the queue and processing's complete so we must have
            // read all the data.
            //
            pSearchRecord->bSearchCompleted = TRUE;
            dwError = 0;
            break;
        }
        else
        {
            (VOID)VmDirConditionTimedWait(
                    pSearchRecord->pDataAvailable,
                    pSearchRecord->mutex,
                    VMDIR_PSCACHE_READ_TIMEOUT);
        }

        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_UNAVAILABLE);
        }
    }

    *ppEntryList = pEntryList;

    pSearchRecord->tLastClientRead = time(NULL);

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDirPagedSearchCacheRead(
    PCSTR pszCookie,
    ENTRYID **ppValidatedEntries,
    DWORD *pdwEntryCount
    )
{
    DWORD dwError = 0;
    PVDIR_PAGED_SEARCH_RECORD pSearchRecord = NULL;
    BOOLEAN bInLock = FALSE;
    PVDIR_PAGED_SEARCH_ENTRY_LIST pEntryList = NULL;

    pSearchRecord = VmDirPagedSearchCacheFind(pszCookie);
    if (pSearchRecord == NULL)
    {
        //
        // Barring the client sending us an invalid cookie, failure here
        // means that the worker thread timed out and freed the cache.
        //
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NOT_FOUND);
    }

    VMDIR_LOCK_MUTEX(bInLock, pSearchRecord->mutex);

    dwError = _VmDirPagedSearchCacheWaitAndRead_inlock(
                pSearchRecord,
                &pEntryList);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pSearchRecord->bSearchCompleted)
    {
        VmDirPagedSearchCacheCullWorkerThread(pSearchRecord);
    }
    else
    {
        *ppValidatedEntries = pEntryList->pEntryIds;
        *pdwEntryCount = pEntryList->dwCount;

        //
        // We transferred ownership of pEntryList->pEntryIds above but we still
        // need to delete the rest of the structure.
        //
        pEntryList->pEntryIds = NULL;
        _VmDirPagedSearchEntryListFree(pEntryList);
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, pSearchRecord->mutex);
    if (pSearchRecord != NULL)
    {
        _DerefPagedSearchRecord(pSearchRecord);
    }
    return dwError;
error:
    goto cleanup;
}

static
BOOLEAN
_VmDirPagedSearchProcessEntry(
    PVDIR_OPERATION pOperation,
    ENTRYID eId
    )
{
    DWORD dwError = 0;
    VDIR_ENTRY srEntry = {0};
    BOOLEAN bInclude = FALSE;

    dwError = pOperation->pBEIF->pfnBESimpleIdToEntry(eId, &srEntry);
    if (dwError == 0)
    {
        if (CheckIfEntryPassesFilter(pOperation, &srEntry, pOperation->request.searchReq.filter) == FILTER_RES_TRUE)
        {
            bInclude = TRUE;
        }

        VmDirFreeEntryContent(&srEntry);
    }

    return bInclude;
}

static
VOID
_VmDirPagedSearchProcessEntries(
    PVDIR_OPERATION pOperation,
    PVDIR_PAGED_SEARCH_RECORD pSearchRecord,
    PVDIR_PAGED_SEARCH_ENTRY_LIST pEntryList
    )
{
    DWORD dwCount = 0;

    for (; pSearchRecord->dwCandidatesProcessed < (DWORD)pSearchRecord->pTotalCandidates->size;)
    {
        if (_VmDirPagedSearchProcessEntry(pOperation, pSearchRecord->pTotalCandidates->eIds[pSearchRecord->dwCandidatesProcessed]))
        {
            pEntryList->pEntryIds[dwCount++] = pSearchRecord->pTotalCandidates->eIds[pSearchRecord->dwCandidatesProcessed];
        }

        pSearchRecord->dwCandidatesProcessed += 1;

        if (dwCount == pSearchRecord->dwPageSize)
        {
            break;
        }
    }

    pEntryList->dwCount = dwCount;
}

static
DWORD
_VmDirPagedSearchEntryListAlloc(
    PVDIR_PAGED_SEARCH_RECORD pSearchRecord,
    PVDIR_PAGED_SEARCH_ENTRY_LIST *ppEntryList
    )
{
    DWORD dwError = 0;
    PVDIR_PAGED_SEARCH_ENTRY_LIST pEntryList = NULL;

    dwError = VmDirAllocateMemory(sizeof(*pEntryList), (PVOID*)&pEntryList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
                sizeof(ENTRYID) * pSearchRecord->dwPageSize,
                (PVOID*)&pEntryList->pEntryIds);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppEntryList = pEntryList;

cleanup:
    return dwError;
error:
    _VmDirPagedSearchEntryListFree(pEntryList);
    goto cleanup;
}

static
VOID
_VmDirPagedSearchCacheWaitForClientCompletion(
    PVDIR_PAGED_SEARCH_RECORD pSearchRecord
    )
{
    BOOLEAN bInLock = FALSE;
    DWORD dwError = 0;

    //
    // Sleep for our timeout period or until the client has read all the data
    // (at which point bSearchCompleted will be set and the condition will be
    // signalled). If we timeout we're going to clear the cache forcibly, which
    // will cause the client to fallback to the old search semantics. Note
    // that the timeout is reset everytime the client issues a read request.
    //
    VMDIR_LOCK_MUTEX(bInLock, pSearchRecord->pThreadInfo->mutexUsed);

    pSearchRecord->bProcessingCompleted = TRUE;

    while (TRUE)
    {
        if (pSearchRecord->bSearchCompleted)
        {
            break;
        }
        dwError = VmDirConditionTimedWait(
                    pSearchRecord->pThreadInfo->conditionUsed,
                    pSearchRecord->pThreadInfo->mutexUsed,
                    gVmdirGlobals.dwLdapRecvTimeoutSec);
        if (dwError == ETIMEDOUT)
        {
            if ((time(NULL) - pSearchRecord->tLastClientRead) > gVmdirGlobals.dwLdapRecvTimeoutSec)
            {
                break;
            }
        }
    }

    VMDIR_UNLOCK_MUTEX(bInLock, pSearchRecord->pThreadInfo->mutexUsed);
}

static
DWORD
_VmDirPagedSearchWorkerThread(
    PVOID pArg
    )
{
    PVDIR_PAGED_SEARCH_RECORD pSearchRecord = (PVDIR_PAGED_SEARCH_RECORD)pArg;
    DWORD dwError = 0;
    PVDIR_PAGED_SEARCH_ENTRY_LIST pEntryIdList = NULL;
    VDIR_OPERATION searchOp = {0};

    VmDirDropThreadPriority(DEFAULT_THREAD_PRIORITY_DELTA);

    dwError = VmDirInitStackOperation(
                &searchOp,
                VDIR_OPERATION_TYPE_INTERNAL,
                LDAP_REQ_SEARCH,
                NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    searchOp.pBEIF = VmDirBackendSelect(NULL);
    searchOp.request.searchReq.filter = pSearchRecord->pFilter;

    while (pSearchRecord->dwCandidatesProcessed < (DWORD)pSearchRecord->pTotalCandidates->size)
    {
        dwError = _VmDirPagedSearchEntryListAlloc(pSearchRecord, &pEntryIdList);
        BAIL_ON_VMDIR_ERROR(dwError);

        _VmDirPagedSearchProcessEntries(&searchOp, pSearchRecord, pEntryIdList);

        if (pEntryIdList->dwCount == 0)
        {
            _VmDirPagedSearchEntryListFree(pEntryIdList);
        }
        else
        {
            dwError = VmDirPagedSearchCacheAddData(pSearchRecord, pEntryIdList);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    _VmDirPagedSearchCacheWaitForClientCompletion(pSearchRecord);

cleanup:
    // This will be freed when the pSearchRecord is released.
    searchOp.request.searchReq.filter = NULL;
    VmDirFreeOperationContent(&searchOp);

    _DerefPagedSearchRecord(pSearchRecord);
    return dwError;
error:
    goto cleanup;
}
