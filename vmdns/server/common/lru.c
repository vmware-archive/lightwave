/*
* Copyright � 2012-2015 VMware, Inc.  All Rights Reserved.
*
* Licensed under the Apache License, Version 2.0 (the �License�); you may not
* use this file except in compliance with the License.  You may obtain a copy
* of the License at http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an �AS IS� BASIS, without
* warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
* License for the specific language governing permissions and limitations
* under the License.
*/

#include "includes.h"


static
DWORD
VmDnsLruClearList(PVMDNS_LRU_LIST pLruList);

DWORD
VmDnsLruInitialize(
    PVMDNS_ZONE_OBJECT pZoneObject,
    LPVMDNS_PURGE_ENTRY_PROC pPurgeEntryProc,
    PVMDNS_LRU_LIST* ppLruList
    )
{
    DWORD dwError = 0;
    PVMDNS_LRU_LIST pLruListTemp = NULL;

    assert(pZoneObject);
    assert(pPurgeEntryProc);

    if (!ppLruList)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(
                        sizeof(VMDNS_LRU_LIST),
                        (PVOID*)&pLruListTemp);
    BAIL_ON_VMDNS_ERROR(dwError);

    pLruListTemp->dwCurrentCount = 0;
    pLruListTemp->dwMaxCount = VMDNS_LRU_SIZE;
    pLruListTemp->dwUpperThreshold = (pLruListTemp->dwMaxCount *
                                        VMDNS_LRU_UPPERTHRES) / 100;

    pLruListTemp->dwLowerThreshold = (pLruListTemp->dwMaxCount *
                                        VMDNS_LRU_LOWERTHRES) / 100;

    pLruListTemp->pZoneObject = pZoneObject;
    pLruListTemp->pPurgeEntryProc = pPurgeEntryProc;

    dwError = VmDnsAllocateMutex(&pLruListTemp->pLock);
    BAIL_ON_VMDNS_ERROR(dwError);

    InitializeListHead(&pLruListTemp->LruListHead);

    *ppLruList = pLruListTemp;

cleanup:

    return dwError;

error:

    if (ppLruList)
    {
        *ppLruList = NULL;
    }

    VmDnsLruFree(pLruListTemp);
    goto cleanup;
}

VOID
VmDnsLruFree(
    PVMDNS_LRU_LIST pLruList
    )
{
    if (pLruList)
    {
        if (!IsListEmpty(&pLruList->LruListHead))
        {
            assert(0 == VmDnsLruClearList(pLruList));
        }

        VmDnsFreeMutex(pLruList->pLock);
        VmDnsFreeMemory(pLruList);
    }
}


DWORD
VmDnsLruAddNameEntry(
    PVMDNS_LRU_LIST pLruList,
    PVMDNS_NAME_ENTRY pNameEntry
    )
{
    DWORD dwError = 0;
    BOOL bLocked = FALSE;

    assert(pLruList);
    assert(pNameEntry);
    assert(pLruList->pLock);

    dwError = VmDnsLockMutex(pLruList->pLock);
    BAIL_ON_VMDNS_ERROR(dwError);
    bLocked = TRUE;

    if (pLruList->dwCurrentCount >= pLruList->dwMaxCount)
    {
        VmDnsLog(
            VMDNS_LOG_LEVEL_DEBUG,
            "LRU Cache Full, Evicting"
            );

        dwError = VmDnsLruClearEntries(
                            pLruList,
                            10);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    InsertHeadList(&pLruList->LruListHead, &pNameEntry->LruList);
    VmDnsNameEntryAddRef(pNameEntry);

    ++pLruList->dwCurrentCount;

cleanup:

    if (bLocked)
    {
        VmDnsUnlockMutex(pLruList->pLock);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsLruRemoveNameEntry(
    PVMDNS_LRU_LIST pLruList,
    PVMDNS_NAME_ENTRY pNameEntry
    )
{
    DWORD dwError = 0;
    BOOL bLocked = FALSE;

    assert(pLruList);
    assert(pNameEntry);
    assert(pLruList->pLock);

    dwError = VmDnsLockMutex(pLruList->pLock);
    BAIL_ON_VMDNS_ERROR(dwError);
    bLocked = TRUE;

    RemoveEntryList(&pNameEntry->LruList);
    --pLruList->dwCurrentCount;
    VmDnsNameEntryRelease(pNameEntry);

cleanup:

    if (bLocked)
    {
        VmDnsUnlockMutex(pLruList->pLock);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsLruRefreshNameEntry(
    PVMDNS_LRU_LIST pLruList,
    PVMDNS_NAME_ENTRY pNameEntry
    )
{
    DWORD dwError = 0;
    BOOL bLocked = FALSE;

    assert(pLruList);
    assert(pNameEntry);
    assert(pLruList->pLock);

    dwError = VmDnsLockMutex(pLruList->pLock);
    BAIL_ON_VMDNS_ERROR(dwError);
    bLocked = TRUE;

    RemoveEntryList(&pNameEntry->LruList);
    InsertHeadList(&pLruList->LruListHead, &pNameEntry->LruList);

cleanup:

    if (bLocked)
    {
        VmDnsUnlockMutex(pLruList->pLock);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsLruClearEntries(
    PVMDNS_LRU_LIST pLruList,
    DWORD dwCount
    )
{
    DWORD dwError = 0;
    PVMDNS_NAME_ENTRY pNameEntry = NULL;
    PLIST_ENTRY pLink = NULL;

        //start purging from least priority up
    for (pLink = pLruList->LruListHead.Blink;
        (pLink != &pLruList->LruListHead && dwCount > 0);
        pLink = pLink->Blink)
    {
        pNameEntry = CONTAINING_RECORD(pLink, VMDNS_NAME_ENTRY, LruList);

        RemoveEntryList(&pNameEntry->LruList);
        --pLruList->dwCurrentCount;

        dwError = pLruList->pPurgeEntryProc(pNameEntry, pLruList->pZoneObject);
        BAIL_ON_VMDNS_ERROR(dwError && dwError != ERROR_INVALID_PARAMETER);

        VmDnsNameEntryRelease(pNameEntry);

        if (dwError == ERROR_INVALID_PARAMETER)
        {
            dwError = 0; //name entry was SOA record, was not purged
        }
        else
        {
            --dwCount;
        }
    }
cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsLruTrimEntries(
    PVMDNS_LRU_LIST pLruList,
    DWORD dwCount
    )
{
    DWORD dwError = 0;
    BOOL bLocked = FALSE;

    assert(pLruList);
    assert(pLruList->pLock);

    dwError = VmDnsLockMutex(pLruList->pLock);
    BAIL_ON_VMDNS_ERROR(dwError);
    bLocked = TRUE;

    dwError = VmDnsLruClearEntries(pLruList, dwCount);
    BAIL_ON_VMDNS_ERROR(dwError);
cleanup:

    if (bLocked)
    {
        VmDnsUnlockMutex(pLruList->pLock);
    }

    return dwError;

error:

    goto cleanup;

}

DWORD
VmDnsLruGetPurgeInterval(
    PVMDNS_LRU_LIST pLruList
    )
{
    DWORD dwInterval = 120;

    assert(pLruList);

    if (pLruList->dwCurrentCount > pLruList->dwUpperThreshold)
    {
        return 30;
    }
    else if (pLruList->dwCurrentCount > pLruList->dwLowerThreshold)
    {
        return 60;
    }

    return dwInterval;
}

BOOL
VmDnsLruIsPurgingNeeded(
    PVMDNS_LRU_LIST pLruList
    )
{
    return pLruList->dwCurrentCount > pLruList->dwLowerThreshold;
}

DWORD
VmDnsLruGetPurgeRate(
    PVMDNS_LRU_LIST pLruList
    )
{
    return (pLruList->dwCurrentCount > pLruList->dwUpperThreshold) ?
                                VMDNS_LRU_PURGEFAST : VMDNS_LRU_PURGE;
}

static
DWORD
VmDnsLruClearList(PVMDNS_LRU_LIST pLruList)
{
    DWORD dwError = 0;
    BOOL bLocked = FALSE;

    assert(pLruList);
    assert(pLruList->pLock);

    dwError = VmDnsLockMutex(pLruList->pLock);
    BAIL_ON_VMDNS_ERROR(dwError);
    bLocked = TRUE;

    while (!IsListEmpty(&pLruList->LruListHead))
    {
        RemoveEntryList((&pLruList->LruListHead)->Flink)
        pLruList->dwCurrentCount--;
    }

cleanup:

    if (bLocked)
    {
        VmDnsUnlockMutex(pLruList->pLock);
    }

    return dwError;

error:
    goto cleanup;
}

