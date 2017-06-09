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


/*
*
* Module Name:  cache.c
*
* Abstract: VMware Domain Name Service.
*
* DNS Cache Main Interface
*/

#include "includes.h"

static
DWORD
VmDnsCacheRefreshThread(
    PVOID   pArgs
    );

static
DWORD
VmDnsCacheInitRefreshThread(
    PVMDNS_CACHE_CONTEXT    pContext
    );

static
DWORD
VmDnsCacheStopRefreshThread(
    PVMDNS_CACHE_CONTEXT    pContext
    );

static
DWORD
VmDnsCacheCleanupRefreshThread(
    PVMDNS_CACHE_CONTEXT    pContext
    );

static
DWORD
VmDnsCacheLoadInitialData(
    PVMDNS_CACHE_CONTEXT    pContext
    );

static
DWORD
VmDnsCachePurgeLRU(
    PVMDNS_CACHE_CONTEXT    pContext
    );

DWORD
VmDnsCacheInitialize(
    PVMDNS_CACHE_CONTEXT    *ppContext
    )
{
    DWORD dwError = 0;
    PVMDNS_CACHE_CONTEXT pContext = NULL;

    dwError = VmDnsAllocateMemory(
                    sizeof(VMDNS_CACHE_CONTEXT),
                    (PVOID*)&pContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneListInit(&pContext->pZoneList);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCacheInitRefreshThread(pContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateRWLock(&pContext->pLock);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCreateThread(
                    pContext->pRefreshThread,
                    FALSE,
                    VmDnsCacheRefreshThread,
                    pContext);
     BAIL_ON_VMDNS_ERROR(dwError);

     *ppContext = pContext;

cleanup:
    return dwError;

error:
    VmDnsCacheCleanup(pContext);
    goto cleanup;
}

VOID
VmDnsCacheCleanup(
    PVMDNS_CACHE_CONTEXT    pContext
    )
{
    if (pContext)
    {
        if (pContext->pRefreshThread)
        {
            VmDnsCacheStopRefreshThread(pContext);
        }

        VmDnsCacheCleanupRefreshThread(pContext);
        VmDnsZoneListCleanup(pContext->pZoneList);
    }
}

DWORD
VmDnsCacheAddZone(
    PVMDNS_CACHE_CONTEXT    pContext,
    PVMDNS_ZONE_INFO        pZoneInfo
    )
{
    DWORD dwError = 0;
    PVMDNS_ZONE_OBJECT pZoneObject = NULL;
    BOOL bLocked = FALSE;

    if (!pZoneInfo || !pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    VmDnsLockWrite(pContext->pLock);
    bLocked = TRUE;

    dwError = VmDnsZoneListFindZone(
                    pContext->pZoneList,
                    pZoneInfo->pszName,
                    &pZoneObject);
    if (!dwError)
    {
        dwError = ERROR_ALREADY_EXISTS;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsZoneCreate(pZoneInfo, &pZoneObject);
    BAIL_ON_VMDNS_ERROR(dwError);

    VmDnsZoneListAddZone(pContext->pZoneList, pZoneObject);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    if (bLocked)
    {
        VmDnsUnlockWrite(pContext->pLock);
    }

    VmDnsZoneObjectRelease(pZoneObject);
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsCacheUpdateZone(
     PVMDNS_CACHE_CONTEXT   pContext,
     PVMDNS_ZONE_INFO       pZoneInfo
    )
{
    DWORD dwError = 0;
    PVMDNS_ZONE_OBJECT pZoneObject = NULL;
    BOOL bLocked = FALSE;

    if (!pZoneInfo)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    VmDnsLockWrite(pContext->pLock);
    bLocked = TRUE;

    dwError = VmDnsZoneListFindZone(
                    pContext->pZoneList,
                    pZoneInfo->pszName,
                    &pZoneObject);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneUpdate(pZoneObject, pZoneInfo);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    if (bLocked)
    {
        VmDnsUnlockWrite(pContext->pLock);
    }

    VmDnsZoneObjectRelease(pZoneObject);

    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsCacheLoadZoneFromStore(
    PVMDNS_CACHE_CONTEXT    pContext,
    PCSTR                   pZoneName
    )
{
    DWORD dwError = 0;
    PVMDNS_RECORD_LIST pList = NULL;
    PVMDNS_ZONE_OBJECT pZoneObject = NULL;
    BOOL bLocked = FALSE;

    if (IsNullOrEmptyString(pZoneName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    VmDnsLockWrite(pContext->pLock);
    bLocked = TRUE;

    dwError = VmDnsStoreGetRecords(pZoneName, pZoneName, &pList);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneCreateFromRecordList(pZoneName, pList, &pZoneObject);
    BAIL_ON_VMDNS_ERROR(dwError);

    VmDnsZoneListAddZone(pContext->pZoneList, pZoneObject);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    if (bLocked)
    {
        VmDnsUnlockWrite(pContext->pLock);
    }

    VmDnsRecordListRelease(pList);
    VmDnsZoneObjectRelease(pZoneObject);

    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsCacheRemoveZone(
    PVMDNS_CACHE_CONTEXT    pContext,
    PVMDNS_ZONE_OBJECT      pZoneObject
    )
{
    DWORD dwError = 0;
    BOOL bLocked = FALSE;

    if (!pZoneObject)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    VmDnsLockWrite(pContext->pLock);
    bLocked = TRUE;

    dwError = VmDnsZoneListRemoveZone(pContext->pZoneList, pZoneObject);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    if (bLocked)
    {
        VmDnsUnlockWrite(pContext->pLock);
    }

    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsCacheGetZoneName(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PSTR                *pszZoneName
    )
{
    DWORD dwError = 0;

    if (!pZoneObject)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsZoneGetName(pZoneObject, pszZoneName);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsCacheListZones(
    PVMDNS_CACHE_CONTEXT    pContext,
    PVMDNS_ZONE_INFO_ARRAY  *ppZoneArray
    )
{
    DWORD dwError = 0;
    BOOL bLocked = FALSE;

    VmDnsLockRead(pContext->pLock);
    bLocked = TRUE;

    dwError = VmDnsZoneListGetZones(pContext->pZoneList, ppZoneArray);
    BAIL_ON_VMDNS_ERROR(dwError);
cleanup:

    if (bLocked)
    {
        VmDnsUnlockRead(pContext->pLock);
    }

    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsCacheFindZone(
    PVMDNS_CACHE_CONTEXT    pContext,
    PCSTR                   szZoneName,
    PVMDNS_ZONE_OBJECT      *ppZoneObject
    )
{
    DWORD dwError = 0;
    PVMDNS_ZONE_OBJECT pZoneObject = NULL;
    BOOL bLocked = FALSE;

    if (!pContext || IsNullOrEmptyString(szZoneName) || !ppZoneObject)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    VmDnsLockRead(pContext->pLock);
    bLocked = TRUE;

    dwError = VmDnsZoneListFindZone(
                    pContext->pZoneList,
                    szZoneName,
                    &pZoneObject);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppZoneObject = pZoneObject;

cleanup:

    if (bLocked)
    {
        VmDnsUnlockRead(pContext->pLock);
    }

    return dwError;

error:
    VmDnsZoneObjectRelease(pZoneObject);
    goto cleanup;
}

DWORD
VmDnsCacheFindZoneByQName(
    PVMDNS_CACHE_CONTEXT    pContext,
    PCSTR                   szQName,
    PVMDNS_ZONE_OBJECT      *ppZoneObject
    )
{
    DWORD dwError = 0;
    PVMDNS_ZONE_OBJECT pZoneObject = NULL;
    BOOL bLocked = FALSE;

    if (!pContext || IsNullOrEmptyString(szQName) || !ppZoneObject)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    VmDnsLockRead(pContext->pLock);
    bLocked = TRUE;

    dwError = VmDnsZoneListFindZoneByQName(
                    pContext->pZoneList,
                    szQName,
                    &pZoneObject);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppZoneObject = pZoneObject;

cleanup:
    if (bLocked)
    {
        VmDnsUnlockRead(pContext->pLock);
    }

    return dwError;

error:
    VmDnsZoneObjectRelease(pZoneObject);
    goto cleanup;
}

DWORD
VmDnsCachePurgeRecord(
    PVMDNS_ZONE_OBJECT pZoneObject,
    PCSTR              pszRecord
    )
{
    PVMDNS_RECORD_LIST pList = NULL;
    DWORD dwError = 0;

    if (VmDnsStringCompareA(pZoneObject->pszName, pszRecord, FALSE) == 0)
    {
        //update SOA record
        dwError = VmDnsStoreGetRecords(
                        pZoneObject->pszName,
                        pZoneObject->pszName,
                        &pList
                        );
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsZoneUpdateRecords(
                            pZoneObject,
                            pZoneObject->pszName,
                            pList
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        VmDnsLog(
            VMDNS_LOG_LEVEL_DEBUG,
            "Refreshed (%s) in Cache",
            pZoneObject->pszName
            );
    }
    else
    {
        dwError = VmDnsZonePurgeRecords(pZoneObject, pszRecord);
        BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND)

        if (dwError == ERROR_NOT_FOUND)
        {
            VmDnsLog(
                VMDNS_LOG_LEVEL_DEBUG,
                "Record (%s) not found in Cache",
                pszRecord
                );

            dwError = 0;
        }
        else
        {
            VmDnsLog(
                VMDNS_LOG_LEVEL_DEBUG,
                "Succesfully Purged (%s) from Cache",
                pszRecord
                );
        }
    }

cleanup:
    if (pList)
    {
        VmDnsRecordListRelease(pList);
    }

    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsCachePurgeRecordProc(
    PVOID pData,
    PCSTR pszZone,
    PCSTR pszNode
    )
{
    PVMDNS_CACHE_CONTEXT pCacheContext = (PVMDNS_CACHE_CONTEXT) pData;
    PVMDNS_ZONE_OBJECT pZoneObject = NULL;
    DWORD dwError = 0;

    dwError = VmDnsCacheFindZone(
                    pCacheContext,
                    pszZone,
                    &pZoneObject
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCachePurgeRecord(pZoneObject, pszNode);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VmDnsZoneObjectRelease(pZoneObject);

    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsCacheSyncZoneProc(
    PVOID pData,
    PCSTR pszZone
    )
{
    PVMDNS_CACHE_CONTEXT pCacheContext = (PVMDNS_CACHE_CONTEXT) pData;
    PVMDNS_ZONE_OBJECT pZoneObject = NULL;
    DWORD dwError = 0;

    dwError = VmDnsCacheFindZone(
                        pCacheContext,
                        pszZone,
                        &pZoneObject
                        );
    BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

    if (dwError == ERROR_NOT_FOUND)
    {
        dwError = VmDnsCacheLoadZoneFromStore(pCacheContext, pszZone);
        BAIL_ON_VMDNS_ERROR(dwError);

        VmDnsLog(
            VMDNS_LOG_LEVEL_DEBUG,
            "Added Zone %s to Cache",
            pszZone
            );
    }

cleanup:
    VmDnsZoneObjectRelease(pZoneObject);

    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsCacheRemoveZoneProc(
    PVOID pData,
    PCSTR pszZone
    )
{
    PVMDNS_CACHE_CONTEXT pCacheContext = (PVMDNS_CACHE_CONTEXT) pData;
    PVMDNS_ZONE_OBJECT pZoneObject = NULL;
    DWORD dwError = 0;

    dwError = VmDnsCacheFindZone(pCacheContext, pszZone, &pZoneObject);
    BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

    if (dwError == ERROR_NOT_FOUND)
    {
        //already deleted (probably same node)
        VmDnsLog(
            VMDNS_LOG_LEVEL_DEBUG,
            "Zone (%s) already deleted from cache",
            pszZone
            );

        dwError = 0;
    }
    else
    {
        VmDnsLog(
            VMDNS_LOG_LEVEL_DEBUG,
            "Removing Zone (%s) From Cache",
            pszZone
            );

        dwError = VmDnsCacheRemoveZone(pCacheContext, pZoneObject);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:
    VmDnsZoneObjectRelease(pZoneObject);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
VmDnsCacheRefreshThread(
    PVOID   pArgs
    )
{
    DWORD dwError = 0;
    DWORD newUSN = 0;
    PVMDNS_CACHE_CONTEXT pCacheContext = (PVMDNS_CACHE_CONTEXT)pArgs;
    pCacheContext->bRunning = TRUE;
    pCacheContext->dwLastUSN = 0;
    dwError = VmDnsStoreGetReplicationStatus(&(pCacheContext->dwLastUSN));

    while (!pCacheContext->bShutdown)
    {
        if (VMDNS_READY != VmDnsSrvGetState())
        {
            dwError = VmDnsCacheLoadInitialData(pCacheContext);
            if (dwError)
            {
                VMDNS_LOG_ERROR("Loading intial data failed with %u.", dwError);
            }
            else
            {
                VmDnsSrvSetState(VMDNS_READY);
            }
        }
        newUSN = 0;
        VmDnsStoreGetReplicationStatus(&newUSN);
        if (pCacheContext->dwLastUSN != 0)
        {
            // Refresh LRU, Cache etc.
            dwError = VmDnsCacheSyncZones(
                            pCacheContext->dwLastUSN,
                            newUSN,
                            pCacheContext
                            );
            BAIL_ON_VMDNS_ERROR(dwError)
        }
        else
        {
            VMDNS_LOG_ERROR("Failed to get replication status %u.", dwError);
        }
        if (newUSN != 0)
        {
            pCacheContext->dwLastUSN = newUSN;

            dwError = VmDnsCachePurgeLRU(pCacheContext);
            BAIL_ON_VMDNS_ERROR(dwError);
        }
        if (!pCacheContext->bShutdown)
        {
            dwError = VmDnsConditionTimedWait(
                                pCacheContext->pRefreshEvent,
                                pCacheContext->pThreadLock,
                                5 * 1000
                                );
            if (dwError != ETIMEDOUT &&
                dwError != WSAETIMEDOUT &&
                dwError != ERROR_SUCCESS)
            {
                BAIL_ON_VMDNS_ERROR(dwError);
            }
        }
    }

cleanup:
    pCacheContext->bRunning = FALSE;

    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsCacheSyncZones(
    DWORD                dwLastChangedUSN,
    DWORD                dwNewUSN,
    PVMDNS_CACHE_CONTEXT pCacheContext
    )
{
    DWORD dwError = 0;

    dwError = VmDnsStoreSyncDeleted(
                        dwLastChangedUSN,
                        dwNewUSN,
                        VmDnsCacheRemoveZoneProc,
                        (PVOID) pCacheContext
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsStoreSyncNewObjects(
                        dwLastChangedUSN,
                        dwNewUSN,
                        VmDnsCacheSyncZoneProc,
                        VmDnsCachePurgeRecordProc,
                        (PVOID) pCacheContext
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
VmDnsCachePurgeLRU(
    PVMDNS_CACHE_CONTEXT    pContext
    )
{
    DWORD dwError = 0;
    DWORD dwPurge = 0;
    BOOL bCacheLocked = FALSE;
    BOOL bZoneLocked = FALSE;
    PVMDNS_ZONE_OBJECT pZoneObject = NULL;
    PVMDNS_LRU_LIST pLruList = NULL;
    DWORD i;

    if (!pContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    VmDnsLockRead(pContext->pLock);
    bCacheLocked = TRUE;

    for (i = 0; i < VMDNS_MAX_ZONES; i++)
    {
        pZoneObject = pContext->pZoneList->Zones[i];

        if (!pZoneObject)
        {
            continue;
        }

        dwError = VmDnsZoneIsPurgingNeeded(pZoneObject, &dwPurge);
        BAIL_ON_VMDNS_ERROR(dwError)

        if (dwPurge)
        {
            VmDnsLockWrite(pZoneObject->pLock);
            bZoneLocked = TRUE;
            pLruList = pZoneObject->pLruList;

            DWORD dwCount = (pLruList->dwMaxCount *
                            VmDnsLruGetPurgeRate(pLruList)) / 100;

            dwError = VmDnsLruTrimEntries(pLruList, dwCount);
            BAIL_ON_VMDNS_ERROR(dwError);

            VmDnsUnlockWrite(pZoneObject->pLock);
            bZoneLocked = FALSE;
        }
    }

cleanup:
    if (bZoneLocked)
    {
        VmDnsUnlockWrite(pZoneObject->pLock);
    }

    if (bCacheLocked)
    {
        VmDnsUnlockRead(pContext->pLock);
    }

    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsCacheEvictEntryProc(
    PVMDNS_NAME_ENTRY  pNameEntry,
    PVMDNS_ZONE_OBJECT pZoneObject
    )
{
    //assumes locks for cache and hashmap are already held
    DWORD dwError = 0;

    //Remove if not SOA
    if (VmDnsStringCompareA(
                pZoneObject->pszName,
                pNameEntry->pszName,
                FALSE
                ) != 0)
    {
        dwError = VmDnsZoneRemoveNameEntry(
                            pZoneObject,
                            pNameEntry
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        VmDnsLog(
            VMDNS_LOG_LEVEL_DEBUG,
            "Purged (%s) from Zone (%s) Cache",
            pNameEntry->pszName,
            pZoneObject->pszName
            );
    }
    else
    {
        dwError = ERROR_INVALID_PARAMETER;
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

static
DWORD
VmDnsCacheInitRefreshThread(
    PVMDNS_CACHE_CONTEXT    pContext
    )
{
    DWORD dwError = 0;
    PVMDNS_THREAD pRefreshThread = NULL;
    PVMDNS_COND pRefreshEvent = NULL;
    PVMDNS_MUTEX pThreadMutex = NULL;

    dwError = VmDnsAllocateCondition(&pRefreshEvent);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateMutex(&pThreadMutex);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateMemory(sizeof(*pRefreshThread), (PVOID *)&pRefreshThread);
    BAIL_ON_VMDNS_ERROR(dwError);

    pContext->pRefreshEvent = pRefreshEvent;
    pContext->pThreadLock = pThreadMutex;
    pContext->pRefreshThread = pRefreshThread;

cleanup:

    return dwError;

error:

    VmDnsFreeMutex(pThreadMutex);
    VmDnsFreeCondition(pRefreshEvent);
    VmDnsFreeMemory(pRefreshThread);

    goto cleanup;
}

static
DWORD
VmDnsCacheStopRefreshThread(
    PVMDNS_CACHE_CONTEXT    pContext
    )
{
    DWORD dwError = 0;

    if (!pContext->pRefreshEvent ||
        !pContext->pRefreshThread)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pContext->bShutdown = TRUE;
    dwError = VmDnsConditionSignal(pContext->pRefreshEvent);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsThreadJoin(pContext->pRefreshThread, &dwError);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCacheCleanupRefreshThread(pContext);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDnsCacheCleanupRefreshThread(
    PVMDNS_CACHE_CONTEXT    pContext
    )
{
    DWORD dwError = 0;

    if (pContext->bRunning)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    VmDnsFreeThread(pContext->pRefreshThread);
    pContext->pThreadLock = NULL;

    VmDnsFreeCondition(pContext->pRefreshEvent);
    pContext->pRefreshEvent = NULL;

    VmDnsFreeMutex(pContext->pThreadLock);
    pContext->pThreadLock = NULL;

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDnsCacheLoadInitialData(
    PVMDNS_CACHE_CONTEXT    pContext
    )
{
    DWORD dwError = 0;
    PSTR *ppszZones = NULL;
    DWORD dwCount = 0, i = 0;
    PVMDNS_ZONE_OBJECT pZoneObject = NULL;
    PVMDNS_RECORD_LIST pList = NULL;
    PSTR *ppszForwarders = NULL;

    VmDnsLockWrite(pContext->pLock);

    dwError = VmDnsStoreInitialize();
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsStoreListZones(&ppszZones, &dwCount);
    for (i = 0; i < dwCount; ++i)
    {
        VmDnsRecordListRelease(pList);
        pList = NULL;

        dwError = VmDnsStoreGetRecords(ppszZones[i], ppszZones[i], &pList);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsZoneCreateFromRecordList(ppszZones[i], pList, &pZoneObject);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsZoneListAddZone(pContext->pZoneList, pZoneObject);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsStoreGetForwarders(&dwCount, &ppszForwarders);
    BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NO_DATA);

    dwError = 0;

    if (dwCount > 0 && *ppszForwarders)
    {
        dwError = VmDnsSetForwarders(gpSrvContext->pForwarderContext, dwCount, ppszForwarders);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:

    VmDnsFreeStringArrayA(ppszZones);
    VmDnsUnlockWrite(pContext->pLock);
    VmDnsRecordListRelease(pList);
    VmDnsZoneObjectRelease(pZoneObject);
    if (ppszForwarders)
    {
        VmDnsFreeStringCountedArrayA(ppszForwarders, dwCount);
    }
    return dwError;

error:

    goto cleanup;
}
