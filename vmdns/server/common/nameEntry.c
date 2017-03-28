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

DWORD
VmDnsNameEntryCreate(
    PCSTR               pszName,
    PVMDNS_NAME_ENTRY   *ppNameEntry
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_NAME_ENTRY  pNameEntry = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pszName, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppNameEntry, dwError);

    dwError = VmDnsAllocateMemory(sizeof(VMDNS_NAME_ENTRY), (VOID*)&pNameEntry);
    BAIL_ON_VMDNS_ERROR(dwError);

    pNameEntry->lRefCount = 1;

    dwError = VmDnsAllocateStringA(pszName, &pNameEntry->pszName);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRecordListCreate(&pNameEntry->pRecords);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppNameEntry = pNameEntry;

cleanup:
    return dwError;
error:
    VmDnsNameEntryRelease(pNameEntry);
    goto cleanup;
}

ULONG
VmDnsNameEntryAddRef(
    PVMDNS_NAME_ENTRY  pNameEntry
    )
{
    return InterlockedIncrement(&pNameEntry->lRefCount);
}

VOID
VmDnsNameEntryRelease(
    PVMDNS_NAME_ENTRY   pNameEntry
    )
{
    if (pNameEntry && InterlockedDecrement(&pNameEntry->lRefCount) == 0)
    {
        VmDnsNameEntryDelete(pNameEntry);
    }
}

VOID
VmDnsNameEntryDelete(
    PVMDNS_NAME_ENTRY   pNameEntry
    )
{
    if (pNameEntry)
    {
        VmDnsRecordListRelease(pNameEntry->pRecords);
        VMDNS_SAFE_FREE_STRINGA(pNameEntry->pszName);
        VmDnsFreeMemory(pNameEntry);
    }
}

DWORD
VmDnsNameEntryReplaceRecords(
    PVMDNS_NAME_ENTRY   pNameEntry,
    PVMDNS_RECORD_LIST  pRecordList
    )
{
    DWORD dwError = 0;

    VmDnsRecordListRelease(pNameEntry->pRecords);
    pNameEntry->pRecords = pRecordList;
    VmDnsRecordListAddRef(pNameEntry->pRecords);

    return dwError;
}

DWORD
VmDnsNameEntryAddRecord(
    PVMDNS_NAME_ENTRY       pNameEntry,
    PVMDNS_RECORD_OBJECT    pRecordObject
    )
{
    DWORD dwError = 0;

    dwError = VmDnsRecordListAdd(pNameEntry->pRecords, pRecordObject);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsNameEntryRemoveRecord(
    PVMDNS_NAME_ENTRY       pNameEntry,
    PVMDNS_RECORD_OBJECT    pRecordObject
    )
{
    DWORD dwError = 0;

    dwError = VmDnsRecordListRemove(pNameEntry->pRecords, pRecordObject);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsNameEntryGetRecords(
    PVMDNS_NAME_ENTRY       pNameEntry,
    VMDNS_RR_TYPE           rrType,
    PVMDNS_RECORD_LIST      *ppRecordList
    )
{
    DWORD dwError = 0, i = 0;
    PVMDNS_RECORD_OBJECT pRecordObj = NULL;
    DWORD dwSize = VmDnsRecordListGetSize(pNameEntry->pRecords);
    PVMDNS_RECORD_LIST pRecordList = NULL;

    dwError = VmDnsRecordListCreate(&pRecordList);
    BAIL_ON_VMDNS_ERROR(dwError);

    for (i = 0; i < dwSize; ++i)
    {
        pRecordObj = VmDnsRecordListGetRecord(pNameEntry->pRecords, i);
        if (pRecordObj->pRecord->dwType == rrType || rrType == VMDNS_RR_QTYPE_ANY)
        {
            dwError = VmDnsRecordListAdd(pRecordList, pRecordObj);
            BAIL_ON_VMDNS_ERROR(dwError);
        }

        VmDnsRecordObjectRelease(pRecordObj);
        pRecordObj = NULL;
    }

    *ppRecordList = pRecordList;

cleanup:
    VmDnsRecordObjectRelease(pRecordObj);
    return dwError;

error:
    VmDnsRecordListRelease(pRecordList);
    goto cleanup;
}
