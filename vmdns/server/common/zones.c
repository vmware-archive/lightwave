/*
 * Copyright (c) VMware Inc.  All rights Reserved.
 */

#include "includes.h"

#define LOCKREAD_ZONE_LIST() VMDNS_LOCKREAD(gpDNSDriverGlobals->pZoneList->pLock)
#define UNLOCKREAD_ZONE_LIST() VMDNS_UNLOCKREAD(gpDNSDriverGlobals->pZoneList->pLock)
#define LOCKWRITE_ZONE_LIST() VMDNS_LOCKWRITE(gpDNSDriverGlobals->pZoneList->pLock)
#define UNLOCKWRITE_ZONE_LIST() VMDNS_UNLOCKWRITE(gpDNSDriverGlobals->pZoneList->pLock)

#define LOCKREAD_ZONE() VMDNS_LOCKREAD(pZone->pLock)
#define UNLOCKREAD_ZONE() VMDNS_UNLOCKREAD(pZone->pLock)
#define LOCKWRITE_ZONE() VMDNS_LOCKWRITE(pZone->pLock)
#define UNLOCKWRITE_ZONE() VMDNS_UNLOCKWRITE(pZone->pLock)

static
DWORD
VmDnsZoneGetSoa(
    PVMDNS_ZONE     pZone,
    PVMDNS_RECORD*  ppSoa
    );

static
VOID
VmDnsFreeNameEntryNode(
    PVMDNS_HASHTABLE_NODE pNode,
    PVOID pUserData
    );

static
VOID
VmDnsFreeZone(
    PVMDNS_ZONE* ppZone
    );

static
PVOID
VmDnsZoneGetKeyForNameEntryNode(
    PVMDNS_HASHTABLE_NODE pNode,
    PVOID pUserData
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
    PVMDNS_HASHTABLE      pNameEntries = NULL;
    PVMDNS_ZONE_ENTRY     pZoneEntry = NULL;
    PVMDNS_HASHTABLE_NODE pRecordNode = NULL;
    PVMDNS_RECORD         pSoaRecord = NULL;
    PVMDNS_RECORD_ARRAY   pRecordArray = NULL;
    PVMDNS_NAME_ENTRY     pSoaNameEntry = NULL;

    LOCKWRITE_ZONE_LIST();

    BAIL_ON_VMDNS_INVALID_POINTER(pZoneInfo, dwError);

    dwError = VmDnsZoneFindByName(pZoneInfo->pszName, &pExistingZone);
    pExistingZone = NULL;
    if (dwError != ERROR_NOT_FOUND)
    {
        dwError = ERROR_ALREADY_EXISTS;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    // Construct the SOA record
    dwError = VmDnsCreateSoaRecord(pZoneInfo, &pSoaRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

    // Create a new name entry hashtable
    dwError = VmDnsCreateHashTable(
            &pNameEntries,
            VmDnsZoneGetKeyForNameEntryNode,
            VmDnsFreeMemory,
            VmDnsHashDigestPstr,
            VmDnsHashEqualPstr,
            NULL,
            DEFAULT_NAME_HASHTABLE_SIZE
            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateMemory(sizeof(VMDNS_ZONE), (void**)&pZone);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateRWLock(&pZone->pLock);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringA(pZoneInfo->pszName, &pZone->pszName);
    BAIL_ON_VMDNS_ERROR(dwError);

    pZone->pNameEntries = pNameEntries;

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
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    // Add the new zone to the zone list
    PushEntryList(&gpDNSDriverGlobals->pZoneList->Zones, &pZoneEntry->ListEntry);
    pZoneEntry = NULL;

    VMDNS_LOG_DEBUG("Created zone: %s", pZoneInfo->pszName);
cleanup:
    UNLOCKWRITE_ZONE_LIST();
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
        VmDnsHashTableClear(pNameEntries, VmDnsFreeNameEntryNode, NULL);
        VmDnsFreeHashTable(&pNameEntries);
        pZone = NULL;
    }

    if (pRecordNode)
    {
        VmDnsFreeMemory(pRecordNode->pData);
        VmDnsFreeMemory(pRecordNode);
    }

    VMDNS_FREE_RECORD_ARRAY(pRecordArray);

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

    if (VmDnsGetState() != VMDNS_READY)
    {
        dwError = ERROR_NOT_READY;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    LOCKREAD_ZONE_LIST();

    BAIL_ON_VMDNS_INVALID_POINTER(ppZoneArray, dwError);

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
    UNLOCKREAD_ZONE_LIST();
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

    BAIL_ON_VMDNS_INVALID_POINTER(pZoneInfo, dwError);

    LOCKREAD_ZONE_LIST();

    dwError = VmDnsZoneFindByName(pZoneInfo->pszName, &pZone);
    if (dwError == ERROR_NOT_FOUND)
    {
        dwError = VmDnsZoneCreate(pCtx, pZoneInfo, bDirSync);
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        LOCKWRITE_ZONE();
        bZoneLocked = TRUE;

        dwError = VmDnsUpdateExistingZone(pZoneInfo, pZone, bDirSync);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:
    if (bZoneLocked)
    {
        UNLOCKWRITE_ZONE();
    }
    UNLOCKREAD_ZONE_LIST();
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

    LOCKWRITE_ZONE_LIST();

    BAIL_ON_VMDNS_INVALID_POINTER(pszZone, dwError);

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
    UNLOCKWRITE_ZONE_LIST();
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

    LOCKREAD_ZONE_LIST();

    BAIL_ON_VMDNS_INVALID_POINTER(pszZone, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pRecord, dwError);

    dwError = VmDnsZoneFindByName(pszZone, &pZone);
    BAIL_ON_VMDNS_ERROR(dwError);

    LOCKWRITE_ZONE();
    bZoneLocked = TRUE;

    dwError = VmDnsZoneFindRecord(pZone, pRecord);
    if (dwError != ERROR_NOT_FOUND)
    {
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (dwError == ERROR_NOT_FOUND)
    {
        dwError = VmDnsZoneEnsureNameEntry( pZone,
                                            pRecord->pszName,
                                            bDirSync,
                                            &pNameEntry);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsNameEntryAddRecord(pNameEntry, pRecord, bDirSync);
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        dwError = ERROR_ALREADY_EXISTS;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    VMDNS_LOG_DEBUG("Added record %s %u to zone %s",
                    pRecord->pszName,
                    pRecord->dwType,
                    pszZone);
cleanup:
    if (bZoneLocked)
    {
        UNLOCKWRITE_ZONE();
    }
    UNLOCKREAD_ZONE_LIST();
    return dwError;

error:
    VMDNS_LOG_ERROR("Adding record %s %u to zone %s failed %u.",
                    pRecord->pszName,
                    pRecord->dwType,
                    pszZone,
                    dwError);

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
    BOOL                    bZoneLocked = FALSE;

    LOCKREAD_ZONE_LIST();

    BAIL_ON_VMDNS_INVALID_POINTER(pszZone, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pRecord, dwError);

    dwError = VmDnsZoneFindByName(pszZone, &pZone);
    BAIL_ON_VMDNS_ERROR(dwError);

    LOCKWRITE_ZONE();
    bZoneLocked = TRUE;

    dwError = VmDnsZoneFindRecord(pZone, pRecord);
    if (dwError != ERROR_NOT_FOUND)
    {
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (dwError != ERROR_NOT_FOUND)
    {
        dwError = VmDnsZoneFindNameEntry(pZone,
                                        pRecord->pszName,
                                        &pNameEntry);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsNameEntryDeleteRecord(pNameEntry, pRecord, bDirSync);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:
    if (bZoneLocked)
    {
        UNLOCKWRITE_ZONE();
    }
    UNLOCKREAD_ZONE_LIST();
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

    if (VmDnsGetState() != VMDNS_READY)
    {
        dwError = ERROR_NOT_READY;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    LOCKREAD_ZONE_LIST();

    BAIL_ON_VMDNS_INVALID_POINTER(pszZone, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pszName, dwError);

    dwError = VmDnsZoneFindByName(pszZone, &pZone);
    BAIL_ON_VMDNS_ERROR(dwError);

    LOCKWRITE_ZONE();
    bZoneLocked = TRUE;

    dwError = VmDnsZoneFindNameEntry(pZone, pszName, &pNameEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsZoneRemoveNameEntry(pZone, pszName, &pNameEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    VMDNS_LOG_DEBUG("Deleted record(s) %s from zone %s", pszName, pszZone);

cleanup:
    if (bZoneLocked)
    {
        UNLOCKWRITE_ZONE();
    }
    UNLOCKREAD_ZONE_LIST();
    VmDnsDeleteNameEntry(pNameEntry, bDirSync);
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

    if (VmDnsGetState() != VMDNS_READY)
    {
        dwError = ERROR_NOT_READY;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    LOCKREAD_ZONE_LIST();

    BAIL_ON_VMDNS_INVALID_POINTER(pszZone, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pszName, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppRecords, dwError);

    dwError = VmDnsZoneFindByName(pszZone, &pZone);
    BAIL_ON_VMDNS_ERROR(dwError);

    LOCKREAD_ZONE();
    bZoneLocked = TRUE;

    dwError = VmDnsZoneFindNameEntry(pZone, pszName, &pNameEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsNameEntryListRecord(pNameEntry, &pRecords);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppRecords = pRecords;

    VMDNS_LOG_DEBUG("Queried %u records from zone %s",
                    (*ppRecords)->dwCount, pszZone);

cleanup:
    if (bZoneLocked)
    {
        UNLOCKREAD_ZONE();
    }
    UNLOCKREAD_ZONE_LIST();
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
    PVMDNS_HASHTABLE        pNameEntries = NULL;
    PVMDNS_HASHTABLE_NODE   pNode = NULL;
    PVMDNS_NAME_ENTRY       pNameEntry = NULL;
    PVMDNS_RECORD_ARRAY     pAllRecords = NULL;
    VMDNS_HASHTABLE_ITER    iter = VMDNS_HASHTABLE_ITER_INIT;
    BOOL                    bZoneLocked = FALSE;

    if (VmDnsGetState() != VMDNS_READY)
    {
        dwError = ERROR_NOT_READY;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    LOCKREAD_ZONE_LIST();

    BAIL_ON_VMDNS_INVALID_POINTER(pszZone, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppRecords, dwError);

    dwError = VmDnsZoneFindByName(pszZone, &pZone);
    BAIL_ON_VMDNS_ERROR(dwError);

    LOCKREAD_ZONE();
    bZoneLocked = TRUE;

    pNameEntries = pZone->pNameEntries;

    while ((pNode = VmDnsHashTableIterate(pNameEntries, &iter)))
    {
        dwRecordCount += VmDnsNameEntryGetRecordCount(pNode->pData);
    }

    dwError = VmDnsAllocateMemory(sizeof(VMDNS_RECORD_ARRAY),
                                    (void**)&pAllRecords);
    BAIL_ON_VMDNS_ERROR(dwError);

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

    pAllRecords->dwCount = dwRecordCount;

    *ppRecords = pAllRecords;
    VMDNS_LOG_DEBUG("Listed %u records from zone %s",
                    (*ppRecords)->dwCount, pszZone);

cleanup:
    if (bZoneLocked)
    {
        UNLOCKREAD_ZONE();
    }
    UNLOCKREAD_ZONE_LIST();
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

    LOCKREAD_ZONE_LIST();

    BAIL_ON_VMDNS_INVALID_POINTER(pszZone, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppZone, dwError);

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
    UNLOCKREAD_ZONE_LIST();
    return dwError;

error:

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

    LOCKREAD_ZONE();

    BAIL_ON_VMDNS_INVALID_POINTER(pZone, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppSoa, dwError);

    dwError = VmDnsZoneFindNameEntry(pZone, VMDNS_SOA_RECORD_NAME, &pNameEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsNameEntryGetSoaRecord(pNameEntry, ppSoa);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    UNLOCKREAD_ZONE();
    return dwError;

error:
    goto cleanup;
}

static
PVOID
VmDnsZoneGetKeyForNameEntryNode(
    PVMDNS_HASHTABLE_NODE pNode,
    PVOID pUserData
    )
{
	PVMDNS_NAME_ENTRY pNameEntry = NULL;
    PSTR pszKeyString = NULL;

    if (pNode && pNode->pData)
    {
        pNameEntry = (PVMDNS_NAME_ENTRY)pNode->pData;
        if (VmDnsAllocateStringA(pNameEntry->pszName, (VOID*)&pszKeyString) == 0)
        {
            return pszKeyString;
        }
    }

    return "";
}

static
VOID
VmDnsFreeZone(
    PVMDNS_ZONE* ppZone
    )
{
    if (ppZone)
    {
        VMDNS_LOCKWRITE((*ppZone)->pLock);

        VmDnsHashTableClear((*ppZone)->pNameEntries, VmDnsFreeNameEntryNode, NULL);
        VmDnsFreeHashTable(&(*ppZone)->pNameEntries);
        VMDNS_SAFE_FREE_MEMORY((*ppZone)->pszName);

        VMDNS_UNLOCKWRITE((*ppZone)->pLock);

        VmDnsFreeRWLock((*ppZone)->pLock);
        VMDNS_SAFE_FREE_MEMORY(*ppZone);
    }
}

static
VOID
VmDnsFreeNameEntryNode(
    PVMDNS_HASHTABLE_NODE pNode,
    PVOID pUserData
    )
{
    PVMDNS_NAME_ENTRY pNameEntry = NULL;

    if (pNode)
    {
        pNameEntry = (PVMDNS_NAME_ENTRY)pNode->pData;
        VmDnsDeleteNameEntry(pNameEntry, !!pUserData);
        VmDnsFreeMemory(pNode);
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
    pZoneInfo->dwZoneType = pZone->dwZoneType;

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

    dwError = VmDnsNameEntryUpdateSoaRecord(pNameEntry, pZoneInfo, bDirSync);
    BAIL_ON_VMDNS_ERROR(dwError);

    pZone->dwFlags = pZoneInfo->dwFlags;
    pZone->dwZoneType = pZoneInfo->dwZoneType;

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

    dwError = VmDnsZoneFindNameEntry(pZone, pRecord->pszName, &pNameEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsNameEntryFindRecord(pNameEntry, pRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;
error:
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
    PVMDNS_HASHTABLE_NODE pNameEntryNode = NULL;

    dwError = VmDnsHashTableFindKey(
                        pZone->pNameEntries,
                        &pNameEntryNode,
                        pszName);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppNameEntry = (PVMDNS_NAME_ENTRY)pNameEntryNode->pData;

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
    PVMDNS_HASHTABLE_NODE pNameEntryNode = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pZone, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pszName, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppNameEntry, dwError);

    dwError = VmDnsHashTableFindKey(
                        pZone->pNameEntries,
                        &pNameEntryNode,
                        pszName);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsHashTableRemove(pZone->pNameEntries, pNameEntryNode);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppNameEntry = (PVMDNS_NAME_ENTRY)pNameEntryNode->pData;
    pNameEntryNode->pData = NULL;
    VMDNS_SAFE_FREE_MEMORY(pNameEntryNode);

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
    PVMDNS_HASHTABLE_NODE pNameEntryNode = NULL;
    PVMDNS_NAME_ENTRY pNameEntry = NULL;

    dwError = VmDnsHashTableFindKey(
                        pZone->pNameEntries,
                        &pNameEntryNode,
                        pszName);
    if (dwError == ERROR_NOT_FOUND)
    {
        dwError = VmDnsCreateNameEntry(pZone->pszName, pszName, &pNameEntry);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsAllocateMemory(sizeof(VMDNS_HASHTABLE_NODE), (VOID*)&pNameEntryNode);
        BAIL_ON_VMDNS_ERROR(dwError);

        pNameEntryNode->pData = pNameEntry;

        VmDnsHashTableInsert(pZone->pNameEntries, pNameEntryNode, NULL);
    }
    else
    {
        pNameEntry = pNameEntryNode->pData;
    }

    BAIL_ON_VMDNS_ERROR(dwError);

    *ppNameEntry = pNameEntry;

cleanup:
    return dwError;
error:
    VmDnsDeleteNameEntry(pNameEntry, bDirSync);
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
        BAIL_ON_VMDNS_ERROR(ERROR_INVALID_PARAMETER);
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
    if (pZoneList)
    {
        LOCKWRITE_ZONE_LIST();
        pEntry = pZoneList->Zones.Next;
        while (pEntry)
        {
            pZoneEntry = CONTAINING_RECORD(pEntry, VMDNS_ZONE_ENTRY, ListEntry);
            pEntry = pEntry->Next;
            VmDnsFreeZone(&pZoneEntry->pZone);
            VMDNS_SAFE_FREE_MEMORY(pZoneEntry);
        }
        pZoneList->Zones.Next = NULL;

        UNLOCKWRITE_ZONE_LIST();
        VMDNS_FREE_RWLOCK(pZoneList->pLock);
        VMDNS_SAFE_FREE_MEMORY(pZoneList);
    }
}

