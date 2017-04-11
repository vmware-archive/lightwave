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


/*
* Module Name:  zonelist.c
*
* Abstract: Zone object and operations
*
*/

#include "includes.h"

static
DWORD
VmDnsZoneCopySoaInfo(
    PVMDNS_ZONE_OBJECT      pZoneObject,
    PVMDNS_RECORD_OBJECT    pRecordObject,
    PVMDNS_ZONE_INFO        pZoneInfo
    );

static
DWORD
VmDnsZoneCreateSoaRecord(
    PVMDNS_ZONE_INFO        pZoneInfo,
    PVMDNS_RECORD_OBJECT    *ppSoaObject
    );

static
DWORD
VmDnsZoneGetSoaRecord(
    PVMDNS_ZONE_OBJECT      pZoneObject,
    PVMDNS_RECORD_OBJECT    *ppSoaObject
    );

static
DWORD
VmDnsZoneFindNameEntry(
    PVMDNS_ZONE_OBJECT      pZoneObject,
    PCSTR                   pszName,
    PVMDNS_NAME_ENTRY       *ppNameEntry
    );

static
DWORD
VmDnsZoneAddNameEntry(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PVMDNS_NAME_ENTRY   pNameEntry
    );

DWORD
VmDnsZoneCreate(
    PVMDNS_ZONE_INFO    pZoneInfo,
    PVMDNS_ZONE_OBJECT  *ppZoneObject
    )
{
    DWORD dwError = 0;
    PVMDNS_ZONE_OBJECT pZoneObject = NULL;
    PVMDNS_RECORD_OBJECT pSoaObject = NULL;
    PVMDNS_RECORD_LIST pRecordList = NULL;

    dwError = VmDnsZoneCreateSoaRecord(pZoneInfo, &pSoaObject);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRecordListCreate(&pRecordList);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRecordListAdd(pRecordList, pSoaObject);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneCreateFromRecordList(
                        pZoneInfo->pszName,
                        pRecordList,
                        &pZoneObject);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppZoneObject = pZoneObject;

cleanup:
    VmDnsRecordObjectRelease(pSoaObject);
    return dwError;

error:
    VmDnsZoneObjectRelease(pZoneObject);
    goto cleanup;
}

DWORD
VmDnsZoneCreateFromRecordList(
    PCSTR szZoneName,
    PVMDNS_RECORD_LIST  pRecordList,
    PVMDNS_ZONE_OBJECT  *ppZoneObject
    )
{
    DWORD dwError = 0;
    PVMDNS_ZONE_OBJECT pZoneObject = NULL;

    if (pRecordList->dwCurrentSize <= 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(
                    sizeof(VMDNS_ZONE_OBJECT),
                    (void**)&pZoneObject);
    BAIL_ON_VMDNS_ERROR(dwError);

    pZoneObject->lRefCount = 1;

    dwError = VmDnsAllocateStringA(szZoneName, &pZoneObject->pszName);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsHashTableAllocate(
                    &pZoneObject->pNameEntries,
                    DEFAULT_NAME_HASHTABLE_SIZE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateRWLock(&pZoneObject->pLock);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsLruInitialize(
                    pZoneObject,
                    VmDnsCacheEvictEntryProc,
                    &pZoneObject->pLruList);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneUpdateRecords(
                    pZoneObject,
                    pZoneObject->pszName,
                    pRecordList);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppZoneObject = pZoneObject;

cleanup:
    return dwError;

error:
    VmDnsZoneObjectRelease(pZoneObject);
    goto cleanup;
}

ULONG
VmDnsZoneObjectAddRef(
    PVMDNS_ZONE_OBJECT  pZoneObject
    )
{
    return InterlockedIncrement(&pZoneObject->lRefCount);
}

VOID
VmDnsZoneObjectRelease(
    PVMDNS_ZONE_OBJECT  pZoneObject
    )
{
    if (pZoneObject && InterlockedDecrement(&pZoneObject->lRefCount) == 0)
    {
        VmDnsLockWrite(pZoneObject->pLock);

        VmDnsLruFree(pZoneObject->pLruList);
        VmDnsHashTableFree(pZoneObject->pNameEntries);
        VMDNS_SAFE_FREE_MEMORY(pZoneObject->pszName);

        VmDnsUnlockWrite(pZoneObject->pLock);

        VmDnsFreeRWLock(pZoneObject->pLock);
        VMDNS_SAFE_FREE_MEMORY(pZoneObject);
    }
}

DWORD
VmDnsZoneUpdate(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PVMDNS_ZONE_INFO    pZoneInfo
    )
{
    DWORD dwError = 0;
    PVMDNS_RECORD_OBJECT pSoaOldObject = NULL;
    PVMDNS_RECORD_OBJECT pSoaNewObject = NULL;

    VmDnsLockWrite(pZoneObject->pLock);

    dwError = VmDnsZoneCreateSoaRecord(pZoneInfo, &pSoaNewObject);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneGetSoaRecord(pZoneObject, &pSoaOldObject);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneRemoveRecord(pZoneObject, pSoaOldObject);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneAddRecord(pZoneObject, pSoaNewObject);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VmDnsUnlockWrite(pZoneObject->pLock);

    VmDnsRecordObjectRelease(pSoaNewObject);
    VmDnsRecordObjectRelease(pSoaOldObject);

    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsZoneGetName(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PSTR                *ppszZoneName
    )
{
    DWORD dwError = 0;
    PSTR pszZoneName = NULL;

    VmDnsLockRead(pZoneObject->pLock);

    dwError = VmDnsAllocateStringA(pZoneObject->pszName, &pszZoneName);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppszZoneName = pszZoneName;

cleanup:

    VmDnsUnlockRead(pZoneObject->pLock);

    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsZoneCopyZoneInfo(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PVMDNS_ZONE_INFO    pZoneInfo
    )
{
    DWORD dwError = 0;
    PVMDNS_RECORD_OBJECT pSoaObject = NULL;

    VmDnsLockRead(pZoneObject->pLock);

    dwError = VmDnsZoneGetSoaRecord(pZoneObject, &pSoaObject);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneCopySoaInfo(pZoneObject, pSoaObject, pZoneInfo);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VmDnsUnlockRead(pZoneObject->pLock);
    VmDnsRecordObjectRelease(pSoaObject);

    return dwError;

error:

    goto cleanup;

}

DWORD
VmDnsZoneUpdateRecords(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PCSTR               pszName,
    PVMDNS_RECORD_LIST  pRecordList
    )
{
    DWORD dwError;
    PVMDNS_NAME_ENTRY pNameEntry = NULL;

    VmDnsLockWrite(pZoneObject->pLock);

    dwError = VmDnsZoneFindNameEntry(pZoneObject, pszName, &pNameEntry);
    BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

    if (!pNameEntry)
    {
        dwError = VmDnsNameEntryCreate(pszName, &pNameEntry);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsZoneAddNameEntry(pZoneObject, pNameEntry);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsLruAddNameEntry(pZoneObject->pLruList, pNameEntry);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsNameEntryReplaceRecords(pNameEntry, pRecordList);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VmDnsNameEntryRelease(pNameEntry);
    VmDnsUnlockWrite(pZoneObject->pLock);
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsZoneAddRecord(
    PVMDNS_ZONE_OBJECT      pZoneObject,
    PVMDNS_RECORD_OBJECT    pRecord
    )
{
    DWORD dwError;
    PVMDNS_NAME_ENTRY pNameEntry = NULL;
    PCSTR pszName = pRecord->pRecord->pszName;

    VmDnsLockWrite(pZoneObject->pLock);

    dwError = VmDnsZoneFindNameEntry(pZoneObject, pszName, &pNameEntry);
    BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

    if (!pNameEntry)
    {
        dwError = VmDnsNameEntryCreate(pszName, &pNameEntry);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsZoneAddNameEntry(pZoneObject, pNameEntry);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsLruAddNameEntry(pZoneObject->pLruList, pNameEntry);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsNameEntryAddRecord(pNameEntry, pRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VmDnsNameEntryRelease(pNameEntry);
    VmDnsUnlockWrite(pZoneObject->pLock);

    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsZoneRemoveRecord(
    PVMDNS_ZONE_OBJECT      pZoneObject,
    PVMDNS_RECORD_OBJECT    pRecord
    )
{
    DWORD dwError;
    PVMDNS_NAME_ENTRY pNameEntry = NULL;
    PCSTR pszName = pRecord->pRecord->pszName;

    VmDnsLockWrite(pZoneObject->pLock);

    dwError = VmDnsZoneFindNameEntry(pZoneObject, pszName, &pNameEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsNameEntryRemoveRecord(pNameEntry, pRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VmDnsNameEntryRelease(pNameEntry);
    VmDnsUnlockWrite(pZoneObject->pLock);

    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsZonePurgeRecords(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PCSTR               pszName
    )
{
    DWORD dwError;
    PVMDNS_NAME_ENTRY pNameEntry = NULL;

    VmDnsLockWrite(pZoneObject->pLock);

    dwError = VmDnsZoneFindNameEntry(pZoneObject, pszName, &pNameEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneRemoveNameEntry(pZoneObject, pNameEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsLruRemoveNameEntry(pZoneObject->pLruList, pNameEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VmDnsUnlockWrite(pZoneObject->pLock);
    VmDnsNameEntryRelease(pNameEntry);

    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsZoneGetRecords(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PCSTR               pszName,
    VMDNS_RR_TYPE       dwType,
    PVMDNS_RECORD_LIST *ppRecordList
    )
{
    DWORD dwError;
    PVMDNS_NAME_ENTRY pNameEntry = NULL;
    PVMDNS_RECORD_LIST pRecordList = NULL;

    VmDnsLockRead(pZoneObject->pLock);

    dwError = VmDnsZoneFindNameEntry(pZoneObject, pszName, &pNameEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsNameEntryGetRecords(pNameEntry, dwType, &pRecordList);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsLruRefreshNameEntry(pZoneObject->pLruList, pNameEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppRecordList = pRecordList;
cleanup:
    VmDnsUnlockRead(pZoneObject->pLock);
    VmDnsNameEntryRelease(pNameEntry);

    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsZoneIsPurgingNeeded(
    PVMDNS_ZONE_OBJECT pZoneObject,
    PDWORD pdwPurge
    )
{
    DWORD dwError = 0;
    BOOL bZoneLocked = FALSE;
    BOOL bLruLocked = FALSE;
    PVMDNS_LRU_LIST pLruList = pZoneObject->pLruList;

    VmDnsLockRead(pZoneObject->pLock);
    bZoneLocked = TRUE;

    dwError = VmDnsLockMutex(pLruList->pLock);
    BAIL_ON_VMDNS_ERROR(dwError);
    bLruLocked = TRUE;

    if (pLruList->dwCurrentCount > 1)
    {
        VmDnsLog(
            VMDNS_LOG_LEVEL_DEBUG,
            "Zone (%s) LRU Cache Size: %d",
            pZoneObject->pszName,
            pLruList->dwCurrentCount
            );
    }

    *pdwPurge = pLruList->dwCurrentCount > pLruList->dwLowerThreshold;

cleanup:
    if (bLruLocked)
    {
        VmDnsUnlockMutex(pLruList->pLock);
    }

    if (bZoneLocked)
    {
        VmDnsUnlockRead(pZoneObject->pLock);
    }

    return dwError;

error:
    goto cleanup;
}

static
DWORD
VmDnsZoneCreateSoaRecord(
    PVMDNS_ZONE_INFO        pZoneInfo,
    PVMDNS_RECORD_OBJECT    *ppSoaObject
    )
{
    DWORD dwError = 0;
    PVMDNS_RECORD pSoaRecord = NULL;
    PVMDNS_RECORD_OBJECT pSoaObject = NULL;

    dwError = VmDnsCreateSoaRecord(pZoneInfo, &pSoaRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRecordObjectCreate(pSoaRecord, &pSoaObject);
    BAIL_ON_VMDNS_ERROR(dwError);

    pSoaRecord = NULL;
    *ppSoaObject = pSoaObject;

cleanup:
    return dwError;

error:

    VMDNS_FREE_RECORD(pSoaRecord);
    goto cleanup;
}

static
DWORD
VmDnsZoneGetSoaRecord(
    PVMDNS_ZONE_OBJECT      pZoneObject,
    PVMDNS_RECORD_OBJECT    *ppSoaObject
    )
{
    DWORD dwError;
    PVMDNS_NAME_ENTRY pNameEntry = NULL;
    PVMDNS_RECORD_LIST pRecordList = NULL;
    PVMDNS_RECORD_OBJECT pSoaObject = NULL;

    dwError = VmDnsZoneFindNameEntry(pZoneObject, pZoneObject->pszName, &pNameEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsNameEntryGetRecords(pNameEntry, VMDNS_RR_TYPE_SOA, &pRecordList);
    BAIL_ON_VMDNS_ERROR(dwError);

    if (pRecordList->dwCurrentSize != 1)
    {
        dwError = ERROR_INTERNAL_ERROR;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pSoaObject = VmDnsRecordListGetRecord(pRecordList, 0);

    *ppSoaObject = pSoaObject;

cleanup:
    VmDnsRecordListRelease(pRecordList);
    VmDnsNameEntryRelease(pNameEntry);

    return dwError;
error:
    goto cleanup;
}

static
DWORD
VmDnsZoneCopySoaInfo(
    PVMDNS_ZONE_OBJECT      pZoneObject,
    PVMDNS_RECORD_OBJECT    pRecordObject,
    PVMDNS_ZONE_INFO        pZoneInfo
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_RECORD pSoa = pRecordObject->pRecord;

    dwError = VmDnsAllocateStringA(pZoneObject->pszName, &pZoneInfo->pszName);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringA(
                    pSoa->Data.SOA.pNamePrimaryServer,
                    &pZoneInfo->pszPrimaryDnsSrvName);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringA(
                    pSoa->Data.SOA.pNameAdministrator,
                    &pZoneInfo->pszRName);
    BAIL_ON_VMDNS_ERROR(dwError);

    pZoneInfo->expire = pSoa->Data.SOA.dwExpire;
    pZoneInfo->minimum = pSoa->Data.SOA.dwDefaultTtl;
    pZoneInfo->refreshInterval = pSoa->Data.SOA.dwRefresh;
    pZoneInfo->retryInterval = pSoa->Data.SOA.dwRetry;
    pZoneInfo->serial = pSoa->Data.SOA.dwSerialNo;
    pZoneInfo->dwFlags = pZoneObject->dwFlags;

    if (VmDnsIsReverseZoneName(pZoneObject->pszName))
    {
        pZoneInfo->dwZoneType = VMDNS_ZONE_TYPE_REVERSE;
    }
    else
    {
        pZoneInfo->dwZoneType = VMDNS_ZONE_TYPE_FORWARD;
    }

cleanup:
    return dwError;

error:
    VmDnsClearZoneInfo(pZoneInfo);
    goto cleanup;
}

static
DWORD
VmDnsZoneFindNameEntry(
    PVMDNS_ZONE_OBJECT      pZoneObject,
    PCSTR                   pszName,
    PVMDNS_NAME_ENTRY       *ppNameEntry
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_NAME_ENTRY pNameEntry = NULL;

    dwError = VmDnsHashTableGet(
                        pZoneObject->pNameEntries,
                        pszName,
                        (PVOID *)&pNameEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    VmDnsNameEntryAddRef(pNameEntry);

    *ppNameEntry = pNameEntry;

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsZoneRemoveNameEntry(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PVMDNS_NAME_ENTRY   pNameEntry
)
{
    DWORD dwError = ERROR_SUCCESS;

    BAIL_ON_VMDNS_INVALID_POINTER(pNameEntry, dwError);

    dwError = VmDnsHashTableRemove(
                    pZoneObject->pNameEntries,
                    pNameEntry->pszName);
    BAIL_ON_VMDNS_ERROR(dwError);

    VmDnsNameEntryRelease(pNameEntry);

 cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
VmDnsZoneAddNameEntry(
    PVMDNS_ZONE_OBJECT  pZoneObject,
    PVMDNS_NAME_ENTRY   pNameEntry
    )
{
    DWORD dwError = ERROR_SUCCESS;

    BAIL_ON_VMDNS_INVALID_POINTER(pNameEntry, dwError);

    dwError = VmDnsHashTableInsert(
                        pZoneObject->pNameEntries,
                        pNameEntry->pszName,
                        pNameEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    VmDnsNameEntryAddRef(pNameEntry);
cleanup:
    return dwError;

error:
    goto cleanup;
}
