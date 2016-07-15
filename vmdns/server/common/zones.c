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

#define LOCKREAD_ZONE_LIST(bLocked) \
{ \
    VMDNS_LOCKREAD(gpDNSDriverGlobals->pZoneList->pLock); \
    bLocked = TRUE; \
}

#define UNLOCKREAD_ZONE_LIST(bLocked) \
if (bLocked) \
{ \
    VMDNS_UNLOCKREAD(gpDNSDriverGlobals->pZoneList->pLock); \
}

#define LOCKWRITE_ZONE_LIST(bLocked) \
{ \
    VMDNS_LOCKWRITE(gpDNSDriverGlobals->pZoneList->pLock); \
    bLocked = TRUE; \
}

#define UNLOCKWRITE_ZONE_LIST(bLocked) \
if (bLocked) \
{ \
    VMDNS_UNLOCKWRITE(gpDNSDriverGlobals->pZoneList->pLock); \
}

#define LOCKREAD_ZONE(bLocked) \
{ \
    VMDNS_LOCKREAD(pZone->pLock); \
    bLocked = TRUE; \
}

#define UNLOCKREAD_ZONE(bLocked) \
if (bLocked) \
{ \
    VMDNS_UNLOCKREAD(pZone->pLock); \
}

#define LOCKWRITE_ZONE(bLocked) \
{ \
    VMDNS_LOCKWRITE(pZone->pLock); \
    bLocked = TRUE; \
}

#define UNLOCKWRITE_ZONE(bLocked) \
if (bLocked) \
{ \
    VMDNS_UNLOCKWRITE(pZone->pLock); \
}

static
DWORD
VmDnsZoneGetSoa(
    PVMDNS_ZONE     pZone,
    PVMDNS_RECORD*  ppSoa
    );

static
VOID
VmDnsFreeZone(
    PVMDNS_ZONE* ppZone
    );

static
DWORD
VmDnsUpdateExistingZone(
    PVMDNS_ZONE_INFO pZoneInfo,
    PVMDNS_ZONE      pZone,
    BOOL             bDirSync
    );

static
DWORD
VmDnsCopyZoneInfo(
    PVMDNS_ZONE         pZone,
    PVMDNS_ZONE_INFO    pZoneInfo
    );

static
DWORD
VmDnsZoneFindRecord(
    PVMDNS_ZONE         pZone,
    PVMDNS_RECORD       pRecord
    );

static
DWORD
VmDnsZoneFindNameEntry(
    PVMDNS_ZONE         pZone,
    PCSTR               pszName,
    PVMDNS_NAME_ENTRY*  ppNameEntry
    );

static
DWORD
VmDnsZoneRemoveNameEntry(
    PVMDNS_ZONE         pZone,
    PCSTR               pszName,
    PVMDNS_NAME_ENTRY*  ppNameEntry
    );

static
DWORD
VmDnsZoneEnsureNameEntry(
    PVMDNS_ZONE         pZone,
    PSTR                pszName,
    BOOL                bDirSync,
    PVMDNS_NAME_ENTRY*  ppNameEntry
    );

static
DWORD
VmDnsZoneGetRecordShortName(
    DWORD   dwRecordType,
    PCSTR   pszName,
    PCSTR   pszZone,
    PSTR*   ppszShortName
    );

static
DWORD
VmDnsZoneGetQueryRecord(
    PVMDNS_RECORD   pRecord,
    PCSTR           pszZone,
    PVMDNS_RECORD*  ppQueryRecord
    );

static
BOOLEAN
VmDnsZoneValidateZoneInfo(
    PVMDNS_ZONE_INFO pZoneInfo
    );

static
BOOLEAN
VmDnsZoneIsRecordCompatible(
    PCSTR           pszZoneName,
    VMDNS_RR_TYPE   recordType
    );

DWORD
VmdnsZoneBeginUpdate(
    PVMDNS_ZONE_UPDATE_CONTEXT* ppCtx
    )
{
    return ERROR_SUCCESS;
}

DWORD
VmdnsZoneEndUpdate(
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx
    )
{
    return ERROR_SUCCESS;
}

DWORD
VmDnsZoneCreate(
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx,
    PVMDNS_ZONE_INFO            pZoneInfo,
    BOOL                        bDirSync
    )
{
    DWORD dwError = 0;
    PVMDNS_ZONE           pZone = NULL;
    PVMDNS_ZONE           pExistingZone = NULL;
    PVMDNS_HASH_TABLE     pNameEntries = NULL;
    PVMDNS_ZONE_ENTRY     pZoneEntry = NULL;
    PVMDNS_RECORD         pSoaRecord = NULL;
    PVMDNS_NAME_ENTRY     pSoaNameEntry = NULL;
    BOOL                  bZoneListLocked = FALSE;
    PVMDNS_RWLOCK         pLock = NULL;
    PSTR                  pszZoneName = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pZoneInfo, dwError);
    BAIL_ON_VMDNS_EMPTY_STRING(pZoneInfo->pszName, dwError);

    if (!VmDnsZoneValidateZoneInfo(pZoneInfo))
    {
        dwError = ERROR_INVALID_DATA;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    LOCKWRITE_ZONE_LIST(bZoneListLocked);

    dwError = VmDnsZoneFindByName(pZoneInfo->pszName, &pExistingZone);
    pExistingZone = NULL;
    if (dwError != ERROR_NOT_FOUND)
    {
        VmDnsLog(VMDNS_LOG_LEVEL_INFO,
            "%s zone already exists: %s",
            __FUNCTION__,
            pZoneInfo->pszName);

        dwError = ERROR_ALREADY_EXISTS;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    // Construct the SOA record
    dwError = VmDnsCreateSoaRecord(pZoneInfo, &pSoaRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

    // Create a new name entry hashtable
    dwError = VmDnsHashTableAllocate(
            &pNameEntries,
            DEFAULT_NAME_HASHTABLE_SIZE
            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateRWLock(&pLock);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringA(pZoneInfo->pszName, &pszZoneName);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateMemory(sizeof(VMDNS_ZONE), (void**)&pZone);
    BAIL_ON_VMDNS_ERROR(dwError);

    pZone->pNameEntries = pNameEntries;
    pNameEntries = NULL;

    pZone->pLock = pLock;
    pLock = NULL;

    pZone->pszName = pszZoneName;
    pszZoneName = NULL;

    // Create a name entry for SOA
    dwError = VmDnsZoneEnsureNameEntry( pZone,
                                        VMDNS_SOA_RECORD_NAME,
                                        bDirSync,
                                        &pSoaNameEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsNameEntryAddRecord(pSoaNameEntry, pSoaRecord, FALSE);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateMemory(sizeof(VMDNS_ZONE_ENTRY), (void**)&pZoneEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    pZoneEntry->pZone = pZone;
    pZone = NULL;

    if (bDirSync)
    {
        dwError = VmDnsStoreCreateZone(pZoneInfo);
        if (dwError == ERROR_ALREADY_EXISTS)
        {
            dwError = ERROR_SUCCESS;
        }
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    // Add the new zone to the zone list
    PushEntryList(&gpDNSDriverGlobals->pZoneList->Zones, &pZoneEntry->ListEntry);
    pZoneEntry = NULL;

    VMDNS_LOG_DEBUG("Created zone: %s", pZoneInfo->pszName);
cleanup:
    UNLOCKWRITE_ZONE_LIST(bZoneListLocked);
    VMDNS_FREE_RECORD(pSoaRecord);
    return dwError;

error:
    if (pZoneEntry)
    {
        VmDnsFreeZone((PVMDNS_ZONE*)&pZoneEntry->pZone);
        VMDNS_SAFE_FREE_MEMORY(pZoneEntry);
    }

    if (pZone)
    {
        VmDnsFreeZone(&pZone);
    }

    if (pNameEntries)
    {
        VmDnsHashTableFree(pNameEntries);
        pZone = NULL;
    }

    if (pLock)
    {
        VmDnsFreeRWLock(pLock);
    }

    if (pszZoneName)
    {
        VMDNS_SAFE_FREE_STRINGA(pszZoneName);
    }

    if (dwError != ERROR_ALREADY_EXISTS)
    {
        VMDNS_LOG_ERROR(
            "%s failed to create zone. Error %u",
            __FUNCTION__,
            dwError);
    }

    goto cleanup;
}

DWORD
VmDnsZoneList(
    PVMDNS_ZONE_INFO_ARRAY *ppZoneArray
    )
{
    DWORD                   dwError = 0;
    PSINGLE_LIST_ENTRY      pEntry = NULL;
    PVMDNS_ZONE_ENTRY       pZoneEntry = NULL;
    PVMDNS_ZONE_INFO_ARRAY  pZoneArray = NULL;
    ULONG                   count = 0;
    DWORD                   idx = 0;
    BOOL                    bZoneListLocked = FALSE;

    BAIL_ON_VMDNS_INVALID_POINTER(ppZoneArray, dwError);

    if (VmDnsGetState() != VMDNS_READY)
    {
        dwError = ERROR_NOT_READY;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    LOCKREAD_ZONE_LIST(bZoneListLocked);

    pEntry = gpDNSDriverGlobals->pZoneList->Zones.Next;
    while (pEntry)
    {
        ++count;
        pEntry = pEntry->Next;
    }

    if (count < 1)
    {
        dwError = ERROR_NOT_FOUND;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(sizeof(VMDNS_ZONE_INFO_ARRAY),
                                 (void**)&pZoneArray);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateMemory(sizeof(VMDNS_ZONE_INFO)*count,
                                 (void**)&pZoneArray->ZoneInfos);
    BAIL_ON_VMDNS_ERROR(dwError);

    pEntry = gpDNSDriverGlobals->pZoneList->Zones.Next;
    while (pEntry)
    {
        pZoneEntry = CONTAINING_RECORD(pEntry, VMDNS_ZONE_ENTRY, ListEntry);

        dwError = VmDnsCopyZoneInfo(pZoneEntry->pZone, &pZoneArray->ZoneInfos[idx]);
        BAIL_ON_VMDNS_ERROR(dwError);

        pEntry = pEntry->Next;
        idx++;
    }

    pZoneArray->dwCount = count;

    *ppZoneArray = pZoneArray;

    VMDNS_LOG_DEBUG("Listing zone returned %u zones.", idx);

cleanup:
    UNLOCKREAD_ZONE_LIST(bZoneListLocked);
    return dwError;

error:
    VMDNS_FREE_ZONE_INFO_ARRAY(pZoneArray);
    goto cleanup;
}

DWORD
VmDnsZoneUpdate(
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx,
    PVMDNS_ZONE_INFO            pZoneInfo,
    BOOL                        bDirSync
    )
{
    DWORD       dwError = 0;
    PVMDNS_ZONE pZone = NULL;
    BOOL        bZoneLocked = FALSE;
    BOOL        bZoneListLocked = FALSE;

    BAIL_ON_VMDNS_INVALID_POINTER(pZoneInfo, dwError);

    LOCKREAD_ZONE_LIST(bZoneListLocked);

    dwError = VmDnsZoneFindByName(pZoneInfo->pszName, &pZone);
    if (dwError == ERROR_NOT_FOUND)
    {
        dwError = VmDnsZoneCreate(pCtx, pZoneInfo, bDirSync);
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        LOCKWRITE_ZONE(bZoneLocked);

        dwError = VmDnsUpdateExistingZone(pZoneInfo, pZone, bDirSync);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:
    UNLOCKWRITE_ZONE(bZoneLocked);
    UNLOCKREAD_ZONE_LIST(bZoneListLocked);
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDnsZoneDelete(
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx,
    PCSTR                       pszZone
    )
{
    DWORD dwError = ERROR_NOT_FOUND;
    PVMDNS_ZONE_ENTRY pZoneEntry = NULL;
    PSINGLE_LIST_ENTRY pEntry = NULL;
    PSINGLE_LIST_ENTRY pEntryTemp = NULL;
    BOOL                bZoneListLocked = FALSE;

    BAIL_ON_VMDNS_EMPTY_STRING(pszZone, dwError);

    LOCKWRITE_ZONE_LIST(bZoneListLocked);

    pEntry = &gpDNSDriverGlobals->pZoneList->Zones;
    while (pEntry->Next)
    {
        pZoneEntry = CONTAINING_RECORD(pEntry->Next, VMDNS_ZONE_ENTRY, ListEntry);
        if (VmDnsStringCompareA(pZoneEntry->pZone->pszName, pszZone, FALSE) == 0)
        {
            pEntryTemp = PopEntryList(pEntry);
            VmDnsFreeZone(&pZoneEntry->pZone);
            VmDnsFreeMemory(pZoneEntry);
            dwError = ERROR_SUCCESS;

            break;
        }
        pEntry = pEntry->Next;
    }

    dwError = VmDnsStoreDeleteZone(pszZone);
    BAIL_ON_VMDNS_ERROR(dwError);

    VMDNS_LOG_DEBUG("Successfully deleted zone %s.", pszZone);

cleanup:
    UNLOCKWRITE_ZONE_LIST(bZoneListLocked);
    return dwError;

error:
    VMDNS_LOG_ERROR("Deleting zone %s failed %u.", pszZone, dwError);
    goto cleanup;
}

/*
 * Add a record to a zone.
 */
DWORD
VmDnsZoneAddRecord(
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx,
    PCSTR                       pszZone,
    PVMDNS_RECORD               pRecord,
    BOOL                        bDirSync
    )
{
    DWORD                   dwError = 0;
    PVMDNS_ZONE             pZone = NULL;
    PVMDNS_NAME_ENTRY       pNameEntry = NULL;
    BOOL                    bZoneLocked = FALSE;
    BOOL                    bZoneListLocked = FALSE;
    PVMDNS_RECORD           pQueryRecord = NULL;

    BAIL_ON_VMDNS_EMPTY_STRING(pszZone, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pRecord, dwError);

    if (!VmDnsValidateRecord(pRecord))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_AND_LOG_ON_VMDNS_ERROR(dwError, VMDNS_LOG_LEVEL_ERROR);
    }

    dwError = VmDnsZoneGetQueryRecord(pRecord, pszZone, &pQueryRecord);
    BAIL_AND_LOG_ON_VMDNS_ERROR(dwError, VMDNS_LOG_LEVEL_ERROR);

    LOCKREAD_ZONE_LIST(bZoneListLocked);

    dwError = VmDnsZoneFindByName(pszZone, &pZone);
    BAIL_AND_LOG_MESSAGE_ON_VMDNS_ERROR(
        dwError,
        VMDNS_LOG_LEVEL_ERROR,
        "Failed to find the required zone to add a record to.");

    if (!VmDnsZoneIsRecordCompatible(
            pZone->pszName,
            pRecord->dwType))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    LOCKWRITE_ZONE(bZoneLocked);

    dwError = VmDnsZoneFindRecord(pZone, pQueryRecord);
    if (dwError != ERROR_NOT_FOUND)
    {
        BAIL_AND_LOG_MESSAGE_ON_VMDNS_ERROR(
            dwError,
            VMDNS_LOG_LEVEL_ERROR,
            "Failed to check existing record while adding a record.");
    }

    if (dwError == ERROR_NOT_FOUND)
    {
        dwError = VmDnsZoneEnsureNameEntry( pZone,
                                            pQueryRecord->pszName,
                                            bDirSync,
                                            &pNameEntry);
        BAIL_AND_LOG_MESSAGE_ON_VMDNS_ERROR(
            dwError,
            VMDNS_LOG_LEVEL_ERROR,
            "Failed to ensure zone name entry while adding a record.");

        dwError = VmDnsNameEntryAddRecord(pNameEntry, pQueryRecord, bDirSync);
        BAIL_AND_LOG_MESSAGE_ON_VMDNS_ERROR(
            dwError,
            VMDNS_LOG_LEVEL_ERROR,
            "Failed to add a record.");
    }
    else
    {
        dwError = ERROR_ALREADY_EXISTS;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    VMDNS_LOG_INFO("Added record %s %u to zone %s",
                    pRecord->pszName,
                    pRecord->dwType,
                    pszZone);
cleanup:
    UNLOCKWRITE_ZONE(bZoneLocked);
    UNLOCKREAD_ZONE_LIST(bZoneListLocked);
    VmDnsClearRecord(pQueryRecord);
    VMDNS_SAFE_FREE_MEMORY(pQueryRecord);

    return dwError;

error:
    if (dwError != ERROR_ALREADY_EXISTS)
    {
        VMDNS_LOG_ERROR("Adding record %s %u to zone %s failed %u.",
                        pRecord->pszName,
                        pRecord->dwType,
                        pszZone,
                        dwError);
    }

    goto cleanup;
}

DWORD
VmDnsZoneDeleteRecord(
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx,
    PCSTR                       pszZone,
    PVMDNS_RECORD               pRecord,
    BOOL                        bDirSync
    )
{
    DWORD                   dwError = 0;
    PVMDNS_ZONE             pZone = NULL;
    PVMDNS_NAME_ENTRY       pNameEntry = NULL;
    PVMDNS_NAME_ENTRY       pNameEntryRemoved = NULL;
    BOOL                    bZoneLocked = FALSE;
    BOOL                    bZoneListLocked = FALSE;
    PVMDNS_RECORD           pQueryRecord = NULL;

    BAIL_ON_VMDNS_EMPTY_STRING(pszZone, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pRecord, dwError);

    dwError = VmDnsZoneGetQueryRecord(pRecord, pszZone, &pQueryRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

    LOCKREAD_ZONE_LIST(bZoneListLocked);

    dwError = VmDnsZoneFindByName(pszZone, &pZone);
    BAIL_ON_VMDNS_ERROR(dwError);

    LOCKWRITE_ZONE(bZoneLocked);

    dwError = VmDnsZoneFindRecord(pZone, pQueryRecord);
    if (dwError != ERROR_NOT_FOUND)
    {
        BAIL_ON_VMDNS_ERROR(dwError);
        dwError = VmDnsZoneFindNameEntry(pZone,
                        pQueryRecord->pszName,
                        &pNameEntry);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsNameEntryDeleteRecord(pNameEntry, pQueryRecord, bDirSync);
        BAIL_ON_VMDNS_ERROR(dwError);

        VMDNS_LOG_INFO("Deleted record %s %u from zone %s",
                        pRecord->pszName,
                        pRecord->dwType,
                        pszZone);

        if (!pNameEntry->Records.Next)
        {
            VMDNS_LOG_INFO("Removing the empty name entry %s from zone %s",
                            pNameEntry->pszName,
                            pszZone);

            dwError = VmDnsZoneRemoveNameEntry(
                            pZone,
                            pNameEntry->pszName,
                            &pNameEntryRemoved);
            BAIL_ON_VMDNS_ERROR(dwError);

            dwError = VmDnsDeleteNameEntry(
                            pNameEntryRemoved,
                            FALSE);
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }

cleanup:
    UNLOCKWRITE_ZONE(bZoneLocked);
    UNLOCKREAD_ZONE_LIST(bZoneListLocked);
    VmDnsClearRecord(pQueryRecord);
    VMDNS_SAFE_FREE_MEMORY(pQueryRecord);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsZoneFindAndDeleteRecords(
    PVMDNS_ZONE_UPDATE_CONTEXT  pCtx,
    PCSTR                       pszZone,
    PCSTR                       pszName,
    BOOL                        bDirSync
    )
{
    DWORD                   dwError = 0;
    PVMDNS_ZONE             pZone = NULL;
    PVMDNS_NAME_ENTRY       pNameEntry = NULL;
    BOOL                    bZoneLocked = FALSE;
    BOOL                bZoneListLocked = FALSE;

    if (VmDnsGetState() != VMDNS_READY)
    {
        dwError = ERROR_NOT_READY;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    BAIL_ON_VMDNS_EMPTY_STRING(pszZone, dwError);
    BAIL_ON_VMDNS_EMPTY_STRING(pszName, dwError);

    LOCKREAD_ZONE_LIST(bZoneListLocked);

    dwError = VmDnsZoneFindByName(pszZone, &pZone);
    BAIL_ON_VMDNS_ERROR(dwError);

    LOCKWRITE_ZONE(bZoneLocked);

    dwError = VmDnsZoneFindNameEntry(pZone, pszName, &pNameEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneRemoveNameEntry(pZone, pszName, &pNameEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    VMDNS_LOG_DEBUG("Deleted record(s) %s from zone %s", pszName, pszZone);

cleanup:
    UNLOCKWRITE_ZONE(bZoneLocked);
    UNLOCKREAD_ZONE_LIST(bZoneListLocked);
    return dwError;

error:
    VMDNS_LOG_ERROR("Failed deleting records named %s from zone %s, error %u.",
                    pszName, pszZone, dwError);
    goto cleanup;
}

DWORD
VmDnsZoneQuery(
    PCSTR                   pszZone,
    PCSTR                   pszName,
    VMDNS_RR_TYPE           type,
    PVMDNS_RECORD_ARRAY*    ppRecords
    )
{
    DWORD                   dwError = 0;
    PVMDNS_ZONE             pZone = NULL;
    PVMDNS_RECORD_ARRAY     pRecords = NULL;
    PVMDNS_NAME_ENTRY       pNameEntry = NULL;
    BOOL                    bZoneLocked = FALSE;
    BOOL                    bZoneListLocked = FALSE;
    PSTR                    pszShortName = NULL;

    if (VmDnsGetState() != VMDNS_READY)
    {
        dwError = ERROR_NOT_READY;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    BAIL_ON_VMDNS_EMPTY_STRING(pszZone, dwError);
    BAIL_ON_VMDNS_EMPTY_STRING(pszName, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppRecords, dwError);

    if (!VmDnsIsSupportedRecordType(type))
    {
        dwError = ERROR_NOT_SUPPORTED;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsZoneGetRecordShortName(
                    type,
                    pszName,
                    pszZone,
                    &pszShortName);
    BAIL_ON_VMDNS_ERROR(dwError);

    LOCKREAD_ZONE_LIST(bZoneListLocked);

    dwError = VmDnsZoneFindByName(pszZone, &pZone);
    BAIL_ON_VMDNS_ERROR(dwError);

    LOCKREAD_ZONE(bZoneLocked);

    dwError = VmDnsZoneFindNameEntry(
                    pZone,
                    pszShortName ? pszShortName : pszName,
                    &pNameEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsNameEntryListRecord(pNameEntry, &pRecords, type);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppRecords = pRecords;

    VMDNS_LOG_DEBUG("Queried %u records from zone %s",
        (*ppRecords)->dwCount, pszZone);

cleanup:
    UNLOCKREAD_ZONE(bZoneLocked);
    UNLOCKREAD_ZONE_LIST(bZoneListLocked);
    VMDNS_SAFE_FREE_STRINGA(pszShortName);
    return dwError;

error:
    VMDNS_FREE_RECORD_ARRAY(pRecords);
    VMDNS_LOG_ERROR("Failed querying records from zone %s, error %u",
                    pszZone, dwError);
    goto cleanup;
}

DWORD
VmDnsZoneListRecord(
    PCSTR                   pszZone,
    PVMDNS_RECORD_ARRAY*    ppRecords
    )
{
    DWORD                   dwError = 0;
    DWORD                   dwRecordCount = 0;
    DWORD                   idx = 0;
    DWORD                   dwCopiedCount = 0;
    PVMDNS_ZONE             pZone = NULL;
    PVMDNS_HASH_TABLE       pNameEntries = NULL;
    PVMDNS_HASH_TABLE_NODE  pNode = NULL;
    PVMDNS_NAME_ENTRY       pNameEntry = NULL;
    PVMDNS_RECORD_ARRAY     pAllRecords = NULL;
    VMDNS_HASH_TABLE_ITER   iter = VMDNS_HASH_TABLE_ITER_INIT;
    BOOL                    bZoneLocked = FALSE;
    BOOL                    bZoneListLocked = FALSE;

    if (VmDnsGetState() != VMDNS_READY)
    {
        dwError = ERROR_NOT_READY;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    BAIL_ON_VMDNS_EMPTY_STRING(pszZone, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppRecords, dwError);

    LOCKREAD_ZONE_LIST(bZoneListLocked);

    dwError = VmDnsZoneFindByName(pszZone, &pZone);
    BAIL_ON_VMDNS_ERROR(dwError);

    LOCKREAD_ZONE(bZoneLocked);

    pNameEntries = pZone->pNameEntries;

    while ((pNode = VmDnsHashTableIterate(pNameEntries, &iter)))
    {
        dwRecordCount += VmDnsNameEntryGetRecordCount(pNode->pData);
    }

    dwError = VmDnsAllocateMemory(sizeof(VMDNS_RECORD_ARRAY),
                                    (void**)&pAllRecords);
    BAIL_ON_VMDNS_ERROR(dwError);

    if (dwRecordCount > 0)
    {
        dwError = VmDnsAllocateMemory(sizeof(VMDNS_RECORD)*dwRecordCount,
                                        (void**)&pAllRecords->Records);
        BAIL_ON_VMDNS_ERROR(dwError);

        VmDnsHashTableResetIter(&iter);

        while ((pNode = VmDnsHashTableIterate(pNameEntries, &iter)))
        {
            pNameEntry = (PVMDNS_NAME_ENTRY)pNode->pData;
            dwError = VmDnsNameEntryCopyAllRecords( pNameEntry,
                                                    &pAllRecords->Records[idx],
                                                    dwRecordCount - idx,
                                                    &dwCopiedCount);
            idx += dwCopiedCount;
        }
    }

    pAllRecords->dwCount = dwRecordCount;
    *ppRecords = pAllRecords;
    VMDNS_LOG_DEBUG("Listed %u records from zone %s",
                    (*ppRecords)->dwCount, pszZone);

cleanup:
    UNLOCKREAD_ZONE(bZoneLocked);
    UNLOCKREAD_ZONE_LIST(bZoneListLocked);
    return dwError;

error:
    VMDNS_LOG_ERROR("Failed listing records from zone %s, error %u",
                    pszZone, dwError);
    VMDNS_FREE_RECORD_ARRAY(pAllRecords);
    goto cleanup;
}

DWORD
VmDnsZoneFindByName(
    PCSTR           pszZone,
    PVMDNS_ZONE*    ppZone
    )
{
    DWORD               dwError = ERROR_NOT_FOUND;
    PSINGLE_LIST_ENTRY  pEntry = NULL;
    PVMDNS_ZONE_ENTRY   pZoneEntry = NULL;
    BOOL                bZoneListLocked = FALSE;


    BAIL_ON_VMDNS_EMPTY_STRING(pszZone, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppZone, dwError);

    LOCKREAD_ZONE_LIST(bZoneListLocked);

    pEntry = gpDNSDriverGlobals->pZoneList->Zones.Next;
    while (pEntry)
    {
        pZoneEntry = CONTAINING_RECORD(pEntry, VMDNS_ZONE_ENTRY, ListEntry);
        if (VmDnsStringCompareA(pszZone, pZoneEntry->pZone->pszName, FALSE) == 0)
        {
            *ppZone = pZoneEntry->pZone;
            dwError = ERROR_SUCCESS;
            break;
        }

        pEntry = pEntry->Next;
    }

cleanup:
    UNLOCKREAD_ZONE_LIST(bZoneListLocked);
    return dwError;

error:
    VMDNS_LOG_ERROR("%s failed with error %u.", __FUNCTION__, dwError);

    goto cleanup;
}

static
DWORD
VmDnsZoneGetSoa(
    PVMDNS_ZONE     pZone,
    PVMDNS_RECORD*  ppSoa
    )
{
    DWORD                   dwError = 0;
    PVMDNS_NAME_ENTRY       pNameEntry = NULL;
    BOOL                bZoneLocked = FALSE;

    BAIL_ON_VMDNS_INVALID_POINTER(pZone, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppSoa, dwError);

    LOCKREAD_ZONE(bZoneLocked);

    dwError = VmDnsZoneFindNameEntry(pZone, VMDNS_SOA_RECORD_NAME, &pNameEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsNameEntryGetSoaRecord(pNameEntry, ppSoa);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    UNLOCKREAD_ZONE(bZoneLocked);
    return dwError;

error:
    goto cleanup;
}


static
VOID
VmDnsFreeZone(
    PVMDNS_ZONE* ppZone
    )
{
    if (ppZone && *ppZone)
    {
        VMDNS_LOCKWRITE((*ppZone)->pLock);

        VmDnsHashTableFree((*ppZone)->pNameEntries);
        VMDNS_SAFE_FREE_MEMORY((*ppZone)->pszName);

        VMDNS_UNLOCKWRITE((*ppZone)->pLock);

        VmDnsFreeRWLock((*ppZone)->pLock);
        VMDNS_SAFE_FREE_MEMORY(*ppZone);

        *ppZone = NULL;
    }
}

static
DWORD
VmDnsCopyZoneInfo(
    PVMDNS_ZONE         pZone,
    PVMDNS_ZONE_INFO    pZoneInfo)
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_RECORD pSoa = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pZone, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pZoneInfo, dwError);

    dwError = VmDnsAllocateStringA(pZone->pszName, &pZoneInfo->pszName);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneGetSoa(pZone, &pSoa);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringA(pSoa->Data.SOA.pNamePrimaryServer,
                                    &pZoneInfo->pszPrimaryDnsSrvName);
    BAIL_ON_VMDNS_ERROR(dwError);
    dwError = VmDnsAllocateStringA(pSoa->Data.SOA.pNameAdministrator,
                                    &pZoneInfo->pszRName);
    BAIL_ON_VMDNS_ERROR(dwError);

    pZoneInfo->expire = pSoa->Data.SOA.dwExpire;
    pZoneInfo->minimum = pSoa->Data.SOA.dwDefaultTtl;
    pZoneInfo->refreshInterval = pSoa->Data.SOA.dwRefresh;
    pZoneInfo->retryInterval = pSoa->Data.SOA.dwRetry;
    pZoneInfo->serial = pSoa->Data.SOA.dwSerialNo;
    pZoneInfo->dwFlags = pZone->dwFlags;
    pZoneInfo->dwZoneType = VmDnsIsReverseZoneName(pZone->pszName) ?
                        VMDNS_ZONE_TYPE_REVERSE : VMDNS_ZONE_TYPE_FORWARD;

cleanup:
    VMDNS_FREE_RECORD(pSoa);
    return dwError;

error:
    VmDnsClearZoneInfo(pZoneInfo);
    goto cleanup;
}

static
DWORD
VmDnsUpdateExistingZone(
    PVMDNS_ZONE_INFO pZoneInfo,
    PVMDNS_ZONE      pZone,
    BOOL             bDirSync
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_NAME_ENTRY pNameEntry = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pZoneInfo, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pZone, dwError);

    dwError = VmDnsZoneFindNameEntry(pZone, VMDNS_SOA_RECORD_NAME, &pNameEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    if (bDirSync)
    {
        dwError = VmDnsStoreUpdateZone(pZoneInfo);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsNameEntryUpdateSoaRecord(pNameEntry, pZoneInfo, FALSE);
    BAIL_ON_VMDNS_ERROR(dwError);

    pZone->dwFlags = pZoneInfo->dwFlags;

    VMDNS_LOG_DEBUG("Successfully updated zone %s.", pZoneInfo->pszName);

cleanup:
    return dwError;

error:
    VMDNS_LOG_ERROR("Failed updating zone %s, error %u",
                    pZoneInfo->pszName, dwError);

    goto cleanup;
}

static
DWORD
VmDnsZoneFindRecord(
    PVMDNS_ZONE         pZone,
    PVMDNS_RECORD       pRecord
    )
{
    DWORD dwError = ERROR_NOT_FOUND;
    PVMDNS_NAME_ENTRY pNameEntry = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pZone, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pRecord, dwError);

    dwError = VmDnsZoneFindNameEntry(
                pZone,
                pRecord->pszName,
                &pNameEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsNameEntryFindRecord(pNameEntry, pRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;
error:
    VMDNS_LOG_DEBUG("%s failed with error %u.", __FUNCTION__, dwError);
    goto cleanup;
}

static
DWORD
VmDnsZoneFindNameEntry(
    PVMDNS_ZONE         pZone,
    PCSTR               pszName,
    PVMDNS_NAME_ENTRY*  ppNameEntry
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_NAME_ENTRY pNameEntry = NULL;

    dwError = VmDnsHashTableGet(
                    pZone->pNameEntries,
                    pszName,
                    (PVOID *)&pNameEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppNameEntry = pNameEntry;

cleanup:
    return dwError;
error:
    goto cleanup;
}

static
DWORD
VmDnsZoneRemoveNameEntry(
    PVMDNS_ZONE         pZone,
    PCSTR               pszName,
    PVMDNS_NAME_ENTRY*  ppNameEntry
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_NAME_ENTRY pNameEntry = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pZone, dwError);
    BAIL_ON_VMDNS_EMPTY_STRING(pszName, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppNameEntry, dwError);

    dwError = VmDnsHashTableGet(
                pZone->pNameEntries,
                pszName,
                (PVOID *)&pNameEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsHashTableRemove(
                pZone->pNameEntries,
                pszName);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppNameEntry = pNameEntry;

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
VmDnsZoneEnsureNameEntry(
    PVMDNS_ZONE         pZone,
    PSTR                pszName,
    BOOL                bDirSync,
    PVMDNS_NAME_ENTRY*  ppNameEntry
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_NAME_ENTRY pNameEntry = NULL;

    dwError = VmDnsHashTableGet(
                        pZone->pNameEntries,
                        pszName,
                        (PVOID *)&pNameEntry);

    if (dwError == ERROR_NOT_FOUND)
    {
        dwError = VmDnsCreateNameEntry(pZone->pszName, pszName, &pNameEntry);
        BAIL_ON_VMDNS_ERROR(dwError);

        VmDnsHashTableInsert(pZone->pNameEntries, pNameEntry->pszName, pNameEntry);
    }

    BAIL_ON_VMDNS_ERROR(dwError);

    *ppNameEntry = pNameEntry;

cleanup:
    return dwError;
error:
    VmDnsDeleteNameEntry(pNameEntry, bDirSync);
    VMDNS_LOG_ERROR("%s failed with error %u.", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDnsZoneListInit(
    PVMDNS_ZONE_LIST* ppDnsList
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_ZONE_LIST pZoneList = NULL;

    if (!ppDnsList)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(
                        sizeof(VMDNS_ZONE_LIST),
                        (PVOID*)&pZoneList);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateRWLock(&pZoneList->pLock);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppDnsList = pZoneList;
    pZoneList = NULL;

cleanup:
    return dwError;

error:

    if (pZoneList)
    {
        VmDnsZoneListCleanup(pZoneList);
    }

    goto cleanup;
}

VOID
VmDnsZoneListCleanup(
    PVMDNS_ZONE_LIST pZoneList
    )
{
    PSINGLE_LIST_ENTRY pEntry = NULL;
    PVMDNS_ZONE_ENTRY  pZoneEntry = NULL;
    BOOL               bZoneListLocked = FALSE;

    if (pZoneList)
    {
        LOCKWRITE_ZONE_LIST(bZoneListLocked);
        pEntry = pZoneList->Zones.Next;
        while (pEntry)
        {
            pZoneEntry = CONTAINING_RECORD(pEntry, VMDNS_ZONE_ENTRY, ListEntry);
            pEntry = pEntry->Next;
            VmDnsFreeZone(&pZoneEntry->pZone);
            VMDNS_SAFE_FREE_MEMORY(pZoneEntry);
        }
        pZoneList->Zones.Next = NULL;

        UNLOCKWRITE_ZONE_LIST(bZoneListLocked);
        VMDNS_FREE_RWLOCK(pZoneList->pLock);
        VMDNS_SAFE_FREE_MEMORY(pZoneList);
    }
}

DWORD
VmDnsZoneRestoreRecordFQDN(
    PCSTR               pszDomainName,
    PVMDNS_RECORD_ARRAY pRecords
    )
{
    DWORD dwError = 0;
    DWORD idx = 0;
    PVMDNS_RECORD pRecord = NULL;
    PSTR  pszTemp = NULL;
    BAIL_ON_VMDNS_EMPTY_STRING(pszDomainName, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pRecords, dwError);

    for (; idx < pRecords->dwCount; ++idx)
    {
        pRecord = &pRecords->Records[idx];
        if (VmDnsIsShortNameRecordType(pRecord->dwType))
        {
            dwError = VmDnsMakeFQDN(pRecord->pszName, pszDomainName, &pszTemp);
            BAIL_ON_VMDNS_ERROR(dwError);

            if (pszTemp)
            {
                VMDNS_SAFE_FREE_STRINGA(pRecord->pszName);
                pRecord->pszName = pszTemp;
                pszTemp = NULL;
            }
        }

        if (pRecord->dwType == VMDNS_RR_TYPE_SRV)
        {
            dwError = VmDnsMakeFQDN(
                        pRecord->Data.SRV.pNameTarget,
                        pszDomainName,
                        &pszTemp);
            BAIL_ON_VMDNS_ERROR(dwError);

            if (pszTemp)
            {
                VMDNS_SAFE_FREE_STRINGA(pRecord->Data.SRV.pNameTarget);
                pRecord->Data.SRV.pNameTarget = pszTemp;
                pszTemp = NULL;
            }
        }

        if (pRecord->dwType == VMDNS_RR_TYPE_NS)
        {
            dwError = VmDnsMakeFQDN(
                        pRecord->Data.NS.pNameHost,
                        pszDomainName,
                        &pszTemp);
            BAIL_ON_VMDNS_ERROR(dwError);

            if (pszTemp)
            {
                VMDNS_SAFE_FREE_STRINGA(pRecord->Data.NS.pNameHost);
                pRecord->Data.NS.pNameHost = pszTemp;
                pszTemp = NULL;
            }
        }
    }

cleanup:
    return dwError;

error:

    VMDNS_SAFE_FREE_STRINGA(pszTemp);
    goto cleanup;
}

static
DWORD
VmDnsZoneGetRecordShortName(
    DWORD   dwRecordType,
    PCSTR   pszName,
    PCSTR   pszZone,
    PSTR*   ppszShortName
    )
{
    DWORD dwError = 0;
    PSTR  pszShortName = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(ppszShortName, dwError);
    BAIL_ON_VMDNS_EMPTY_STRING(pszName, dwError);
    BAIL_ON_VMDNS_EMPTY_STRING(pszZone, dwError);

    if (VmDnsIsShortNameRecordType(dwRecordType))
    {
        dwError = VmDnsAllocateStringA(pszName, &pszShortName);
        BAIL_ON_VMDNS_ERROR(dwError);

        VmDnsTrimDomainNameSuffix(pszShortName, pszZone);
        *ppszShortName = pszShortName;
    }
    else
    {
        *ppszShortName = NULL;
    }

cleanup:
    return dwError;

error:
    VMDNS_SAFE_FREE_STRINGA(pszShortName);
    goto cleanup;
}

static
DWORD
VmDnsZoneGetQueryRecord(
    PVMDNS_RECORD   pRecord,
    PCSTR           pszZone,
    PVMDNS_RECORD*  ppQueryRecord
    )
{
    DWORD dwError = 0;
    PSTR  pszShortName = NULL;
    PVMDNS_RECORD pQueryRecord = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pRecord, dwError);
    BAIL_ON_VMDNS_EMPTY_STRING(pszZone, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppQueryRecord, dwError);

    dwError = VmDnsDuplicateRecord(pRecord, &pQueryRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneGetRecordShortName(
                pRecord->dwType,
                pRecord->pszName,
                pszZone,
                &pszShortName);
    BAIL_ON_VMDNS_ERROR(dwError);

    if (pszShortName)
    {
        VMDNS_SAFE_FREE_STRINGA(pQueryRecord->pszName);
        pQueryRecord->pszName = pszShortName;
        pszShortName = NULL;
    }

    if (pQueryRecord->dwType == VMDNS_RR_TYPE_SRV)
    {
        VmDnsTrimDomainNameSuffix(pQueryRecord->Data.SRV.pNameTarget, pszZone);
    }

    *ppQueryRecord = pQueryRecord;

cleanup:
    VMDNS_SAFE_FREE_STRINGA(pszShortName);
    return dwError;

error:
    VmDnsClearRecord(pQueryRecord);
    VMDNS_SAFE_FREE_MEMORY(pQueryRecord);
    goto cleanup;
}

static
BOOLEAN
VmDnsZoneValidateZoneInfo(
    PVMDNS_ZONE_INFO pZoneInfo
    )
{
    if (pZoneInfo &&
        (VmDnsStringLenA(pZoneInfo->pszName) <= VMDNS_NAME_LENGTH_MAX) &&
        (VmDnsStringLenA(pZoneInfo->pszPrimaryDnsSrvName) <= VMDNS_NAME_LENGTH_MAX) &&
        (VmDnsStringLenA(pZoneInfo->pszRName) <= VMDNS_NAME_LENGTH_MAX))
    {
        return TRUE;
    }

    VMDNS_LOG_DEBUG("%s validation of zone info failed.", __FUNCTION__);

    return FALSE;
}

static
BOOLEAN
VmDnsZoneIsRecordCompatible(
    PCSTR           pszZoneName,
    VMDNS_RR_TYPE   recordType
    )
{
    VMDNS_ZONE_TYPE zoneType = VMDNS_ZONE_TYPE_FORWARD;
    if (VmDnsIsReverseZoneName(pszZoneName))
    {
        zoneType = VMDNS_ZONE_TYPE_REVERSE;
    }

    if ((zoneType == VMDNS_ZONE_TYPE_FORWARD &&
        recordType == VMDNS_RR_TYPE_PTR) ||
        (zoneType == VMDNS_ZONE_TYPE_REVERSE &&
            (recordType != VMDNS_RR_TYPE_PTR &&
            recordType != VMDNS_RR_TYPE_SOA &&
            recordType != VMDNS_RR_TYPE_NS)))
    {
        return FALSE;
    }

    return TRUE;
}

