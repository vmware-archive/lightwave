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

#define VMDNS_FREE_RECORD_LIST_ENTRY(pRecordListEntry) \
if (pRecordListEntry) \
{ \
    VmDnsFreeRecordEntry(pRecordListEntry); \
    pRecordListEntry = NULL; \
}

#define GET_RECORD_ENTRY(entry) \
    CONTAINING_RECORD(entry, VMDNS_RECORD_ENTRY, ListEntry)

static
DWORD
VmDnsCreateRecordListEntry(
    PVMDNS_RECORD           pRecord,
    PVMDNS_RECORD_ENTRY*    ppRecordListEntry
);

static
VOID
VmDnsFreeRecordEntry(
    PVMDNS_RECORD_ENTRY  pRecordEntry
    );

DWORD
VmDnsCreateNameEntry(
    PCSTR               pszZone,
    PCSTR               pszName,
    PVMDNS_NAME_ENTRY*  ppNameEntry
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_NAME_ENTRY  pNameEntry = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pszZone, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pszName, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppNameEntry, dwError);

    dwError = VmDnsAllocateMemory(sizeof(VMDNS_NAME_ENTRY), (VOID*)&pNameEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringA(pszZone, &pNameEntry->pszZone);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringA(pszName, &pNameEntry->pszName);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateRWLock(&pNameEntry->pLock);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppNameEntry = pNameEntry;

cleanup:
    return dwError;
error:
    VmDnsDeleteNameEntry(pNameEntry, FALSE);
    goto cleanup;
}

DWORD
VmDnsDeleteNameEntry(
    PVMDNS_NAME_ENTRY   pNameEntry,
    BOOL                bDirSync
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PSINGLE_LIST_ENTRY pEntry = NULL;
    PSINGLE_LIST_ENTRY pEntryTemp = NULL;
    PVMDNS_RECORD_ENTRY pRecordEntry = NULL;
    PVMDNS_RECORD pRecord = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pNameEntry, dwError);

    pEntry = pNameEntry->Records.Next;
    while (pEntry)
    {
        pRecordEntry = GET_RECORD_ENTRY(pEntry);
        pRecord = pRecordEntry->Record;
        if (bDirSync && pRecord)
        {
            dwError = VmDnsStoreDeleteZoneRecord(pNameEntry->pszZone, pRecord);
            BAIL_ON_VMDNS_ERROR(dwError);
        }

        pEntryTemp = pEntry->Next;
        VmDnsFreeRecordEntry(pRecordEntry);
        pEntry = pEntryTemp;
    }

    VMDNS_SAFE_FREE_STRINGA(pNameEntry->pszName);
    VMDNS_SAFE_FREE_STRINGA(pNameEntry->pszZone);
    VmDnsFreeRWLock(pNameEntry->pLock);
    VmDnsFreeMemory(pNameEntry);

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsNameEntryListRecord(
    PVMDNS_NAME_ENTRY       pNameEntry,
    PVMDNS_RECORD_ARRAY*    ppRecords
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_RECORD_ARRAY pRecords = NULL;
    PSINGLE_LIST_ENTRY pEntry = NULL;
    PVMDNS_RECORD_ENTRY pRecordEntry = NULL;
    DWORD dwRecords = 0;
    DWORD idx = 0;

    BAIL_ON_VMDNS_INVALID_POINTER(pNameEntry, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppRecords, dwError);

    pEntry = pNameEntry->Records.Next;
    while (pEntry)
    {
        ++dwRecords;
        pEntry = pEntry->Next;
    }

    dwError = VmDnsAllocateMemory(sizeof(VMDNS_RECORD_ARRAY), (VOID*)&pRecords);
    BAIL_ON_VMDNS_ERROR(dwError);

    if (dwRecords > 0)
    {
        dwError = VmDnsAllocateMemory(sizeof(VMDNS_RECORD)*dwRecords,
                                      (VOID*)&(pRecords->Records));
        BAIL_ON_VMDNS_ERROR(dwError);

        pEntry = pNameEntry->Records.Next;
        while (pEntry)
        {
            pRecordEntry = GET_RECORD_ENTRY(pEntry);
            dwError = VmDnsCopyRecord(pRecordEntry->Record, &pRecords->Records[idx]);
            BAIL_ON_VMDNS_ERROR(dwError);

            pEntry = pEntry->Next;
            ++idx;
        }
    }

    pRecords->dwCount = dwRecords;

    *ppRecords = pRecords;

cleanup:
    return dwError;
error:
    VMDNS_FREE_RECORD_ARRAY(pRecords);
    goto cleanup;
}

DWORD
VmDnsNameEntryCopyAllRecords(
    PVMDNS_NAME_ENTRY   pNameEntry,
    PVMDNS_RECORD       pRecordsBuffer,
    DWORD               dwSize,
    DWORD*              pdwCopied
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PSINGLE_LIST_ENTRY pEntry = NULL;
    PVMDNS_RECORD_ENTRY pRecordEntry = NULL;
    DWORD idx = 0;
    DWORD idx2 = 0;

    BAIL_ON_VMDNS_INVALID_POINTER(pNameEntry, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pRecordsBuffer, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pdwCopied, dwError);

    if (dwSize < VmDnsNameEntryGetRecordCount(pNameEntry))
    {
        dwError = ERROR_INSUFFICIENT_BUFFER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pEntry = pNameEntry->Records.Next;
    while (pEntry)
    {
        pRecordEntry = GET_RECORD_ENTRY(pEntry);
        dwError = VmDnsCopyRecord(pRecordEntry->Record, &pRecordsBuffer[idx]);
        BAIL_ON_VMDNS_ERROR(dwError);

        pEntry = pEntry->Next;
        ++idx;
    }

    *pdwCopied = idx;

cleanup:
    return dwError;
error:
    for (; idx2 <= idx; ++idx2)
    {
        VmDnsClearRecord(&pRecordsBuffer[idx2]);
    }
    goto cleanup;
}


DWORD
VmDnsNameEntryAddRecord(
    PVMDNS_NAME_ENTRY   pNameEntry,
    PVMDNS_RECORD       pRecord,
    BOOL                bDirSync
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_RECORD_ENTRY pRecordListEntry = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pNameEntry, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pRecord, dwError);

    if (bDirSync)
    {
        dwError = VmDnsStoreAddZoneRecord(pNameEntry->pszZone, pRecord);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCreateRecordListEntry(pRecord, &pRecordListEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    PushEntryList(&(pNameEntry->Records), &(pRecordListEntry->ListEntry));

cleanup:
    return dwError;
error:
    VMDNS_FREE_RECORD_LIST_ENTRY(pRecordListEntry);
    VMDNS_LOG_ERROR("%s failed %u.", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDnsNameEntryDeleteRecord(
    PVMDNS_NAME_ENTRY   pNameEntry,
    PVMDNS_RECORD       pRecord,
    BOOL                bDirSync
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PSINGLE_LIST_ENTRY pEntry = NULL;
    PSINGLE_LIST_ENTRY pEntryRemoved = NULL;
    PVMDNS_RECORD_ENTRY pRecordEntry = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pNameEntry, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pRecord, dwError);

    if (bDirSync)
    {
        dwError = VmDnsStoreDeleteZoneRecord(pNameEntry->pszZone, pRecord);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pEntry = &pNameEntry->Records;
    while (pEntry->Next)
    {
        pRecordEntry = GET_RECORD_ENTRY(pEntry->Next);
        if (VmDnsCompareRecord(pRecord, pRecordEntry->Record))
        {
            pEntryRemoved = PopEntryList(pEntry);
            VMDNS_FREE_RECORD_LIST_ENTRY(pRecordEntry);
            break;
        }
        pEntry = pEntry->Next;
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsNameEntryFindRecord(
    PVMDNS_NAME_ENTRY   pNameEntry,
    PVMDNS_RECORD       pRecord
    )
{
    DWORD dwError = ERROR_NOT_FOUND;
    PSINGLE_LIST_ENTRY pEntry = NULL;
    PVMDNS_RECORD_ENTRY pRecordEntry = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pNameEntry, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pRecord, dwError);

    pEntry = pNameEntry->Records.Next;
    while (pEntry)
    {
        pRecordEntry = GET_RECORD_ENTRY(pEntry);
        if (VmDnsCompareRecord(pRecord, pRecordEntry->Record))
        {
            dwError = ERROR_SUCCESS;
            break;
        }
        pEntry = pEntry->Next;
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsNameEntryGetSoaRecord(
    PVMDNS_NAME_ENTRY   pNameEntry,
    PVMDNS_RECORD*      ppRecord
    )
{
    DWORD dwError = ERROR_NOT_FOUND;
    PSINGLE_LIST_ENTRY pEntry = NULL;
    PVMDNS_RECORD_ENTRY pRecordEntry = NULL;
    PVMDNS_RECORD pSoa = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pNameEntry, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppRecord, dwError);

    pEntry = pNameEntry->Records.Next;
    if (pEntry)
    {
        pRecordEntry = GET_RECORD_ENTRY(pEntry);
        if (pRecordEntry->Record->dwType == VMDNS_RR_TYPE_SOA)
        {
            dwError = VmDnsDuplicateRecord(pRecordEntry->Record, &pSoa);
            BAIL_ON_VMDNS_ERROR(dwError);

            *ppRecord = pSoa;
            dwError = ERROR_SUCCESS;
        }
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsNameEntryUpdateSoaRecord(
    PVMDNS_NAME_ENTRY   pNameEntry,
    PVMDNS_ZONE_INFO    pZoneInfo,
    BOOL                bDirSync
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_RECORD pSoa = NULL;
    PVMDNS_RECORD pNewSoa = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pNameEntry, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pZoneInfo, dwError);

    dwError = VmDnsNameEntryGetSoaRecord(pNameEntry, &pSoa);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCreateSoaRecord(pZoneInfo, &pNewSoa);
    BAIL_ON_VMDNS_ERROR(dwError);

    if (!VmDnsCompareRecord(pSoa, pNewSoa))
    {
        dwError = VmDnsNameEntryDeleteRecord(pNameEntry, pSoa, bDirSync);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsNameEntryAddRecord(pNameEntry, pNewSoa, bDirSync);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:
    VMDNS_FREE_RECORD(pSoa);
    VMDNS_FREE_RECORD(pNewSoa);
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsNameEntryGetRecordCount(
    PVMDNS_NAME_ENTRY   pNameEntry
    )
{
    PSINGLE_LIST_ENTRY pEntry = NULL;
    DWORD dwCount = 0;

    pEntry = pNameEntry->Records.Next;
    while (pEntry)
    {
        ++dwCount;
        pEntry = pEntry->Next;
    }

    return dwCount;
}

static
DWORD
VmDnsCreateRecordListEntry(
    PVMDNS_RECORD           pRecord,
    PVMDNS_RECORD_ENTRY*    ppRecordListEntry
)
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_RECORD_ENTRY pRecordEntry = NULL;
    PVMDNS_RECORD           pNewRecord = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pRecord, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppRecordListEntry, dwError);

    dwError = VmDnsAllocateMemory(sizeof(VMDNS_RECORD_ENTRY), (VOID*)&pRecordEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsDuplicateRecord(pRecord, &pNewRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

    pRecordEntry->Record = pNewRecord;

    *ppRecordListEntry = pRecordEntry;

cleanup:
    return dwError;
error:
    VMDNS_SAFE_FREE_MEMORY(pNewRecord);
    VMDNS_SAFE_FREE_MEMORY(pRecordEntry);
    goto cleanup;
}

static
VOID
VmDnsFreeRecordEntry(
    PVMDNS_RECORD_ENTRY   pRecordEntry
    )
{
    if (pRecordEntry)
    {
        VMDNS_FREE_RECORD(pRecordEntry->Record);
        VMDNS_SAFE_FREE_MEMORY(pRecordEntry);
    }
}

