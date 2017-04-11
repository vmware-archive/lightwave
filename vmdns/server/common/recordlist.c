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
VmDnsRecordListCreate(
    PVMDNS_RECORD_LIST          *ppList
    )
{
    DWORD dwError = 0;
    PVMDNS_RECORD_LIST pList = NULL;

    if (!ppList)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(sizeof(VMDNS_RECORD_LIST), (PVOID*)&pList);
    BAIL_ON_VMDNS_ERROR(dwError);

    pList->dwCurrentSize = 0;
    pList->dwMaxSize = 10;

    dwError = VmDnsAllocateMemory(
                        sizeof(PVMDNS_RECORD_OBJECT)* pList->dwMaxSize,
                        (PVOID*)&pList->ppRecords);
    BAIL_ON_VMDNS_ERROR(dwError);

    pList->lRefCount = 1;
    *ppList = pList;

cleanup:
    return dwError;

error:

    if (ppList)
    {
        *ppList = NULL;
    }

    VmDnsRecordListRelease(pList);
    goto cleanup;
}

DWORD
VmDnsRecordListAdd(
    PVMDNS_RECORD_LIST          pList,
    PVMDNS_RECORD_OBJECT        pRecord
    )
{
    PVMDNS_RECORD_OBJECT *pNewList = NULL;
    DWORD dwError = 0;

    assert(pList);

    if (pList->dwCurrentSize >= pList->dwMaxSize)
    {
        pList->dwMaxSize = 2 * pList->dwMaxSize;

        dwError = VmDnsAllocateMemory(
                        sizeof(PVMDNS_RECORD_OBJECT) * pList->dwMaxSize,
                        (PVOID*)&pNewList);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsCopyMemory(
                        pNewList,
                        sizeof(PVMDNS_RECORD_OBJECT) * pList->dwMaxSize,
                        pList->ppRecords,
                        sizeof(PVMDNS_RECORD_OBJECT) * pList->dwCurrentSize);
        BAIL_ON_VMDNS_ERROR(dwError);


        VMDNS_SAFE_FREE_MEMORY(pList->ppRecords);
        pList->ppRecords = pNewList;
        pNewList = NULL;
    }

    VmDnsRecordObjectAddRef(pRecord);
    pList->ppRecords[pList->dwCurrentSize] = pRecord;
    ++pList->dwCurrentSize;

cleanup:
    return dwError;

error:

    VMDNS_SAFE_FREE_MEMORY(pNewList);
    goto cleanup;
}


DWORD
VmDnsRecordListRemove(
    PVMDNS_RECORD_LIST          pList,
    PVMDNS_RECORD_OBJECT        pRecord
    )
{
    DWORD dwError = 0, i = 0, j = 0;

    for (i = 0; i < pList->dwCurrentSize; ++i)
    {
        if (pRecord == pList->ppRecords[i] ||
            VmDnsCompareRecord(
                    pList->ppRecords[i]->pRecord,
                    pRecord->pRecord))
        {
            VmDnsRecordObjectRelease(pList->ppRecords[i]);

            for (j = i; i < pList->dwCurrentSize - 1; ++j)
            {
                pList->ppRecords[j] = pList->ppRecords[j + 1];
            }

            --pList->dwCurrentSize;
            break;
        }
    }

    if (i == pList->dwCurrentSize)
    {
        dwError = ERROR_NOT_FOUND;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}


PVMDNS_RECORD_OBJECT
VmDnsRecordListGetRecord(
    PVMDNS_RECORD_LIST          pList,
    UINT                        nIndex
    )
{
    PVMDNS_RECORD_OBJECT        pRecordObject = NULL;

    assert(pList);
    assert(pList->dwCurrentSize > nIndex);

    pRecordObject = pList->ppRecords[nIndex];
    VmDnsRecordObjectAddRef(pRecordObject);

    return pRecordObject;
}

UINT
VmDnsRecordListGetSize(
    PVMDNS_RECORD_LIST          pList
    )
{
    assert(pList);
    return pList->dwCurrentSize;
}

VOID
VmDnsRecordListFree(
    PVMDNS_RECORD_LIST          pList
    )
{
    DWORD i = 0;
    if (pList)
    {
        for (i = 0; i < pList->dwCurrentSize; ++i)
        {
            VmDnsRecordObjectRelease(pList->ppRecords[i]);
        }
        VMDNS_SAFE_FREE_MEMORY(pList->ppRecords);
        VMDNS_SAFE_FREE_MEMORY(pList);
    }
}

DWORD
VmDnsRecordListAddRef(
    PVMDNS_RECORD_LIST          pList
    )
{
    DWORD dwError = 0;
    if (pList)
    {
        dwError = InterlockedIncrement(&pList->lRefCount);
    }
    return dwError;
}

VOID
VmDnsRecordListRelease(
    PVMDNS_RECORD_LIST          pList
    )
{
    if (pList)
    {
        if (0 == InterlockedDecrement(&pList->lRefCount))
        {
            VmDnsRecordListFree(pList);
        }
    }
}

DWORD
VmDnsCopyRecordArray(
    PVMDNS_RECORD_LIST  pRecordList,
    PVMDNS_RECORD       **pppRecordArray
    )
{
    DWORD idx = 0;
    DWORD dwCount = 0;
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_RECORD *ppRecordArrayTemp = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pRecordList, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pppRecordArray, dwError);

    dwCount = pRecordList->dwCurrentSize;

    dwError = VmDnsAllocateMemory(
                        sizeof(VMDNS_RECORD) * dwCount,
                        (PVOID*)&ppRecordArrayTemp
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    for (; idx < dwCount; ++idx)
    {
        dwError = VmDnsAllocateMemory(
                            sizeof(VMDNS_RECORD),
                            (PVOID)&ppRecordArrayTemp[idx]
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsCopyRecord(
                        pRecordList->ppRecords[idx]->pRecord,
                        ppRecordArrayTemp[idx]
                        );
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    *pppRecordArray = ppRecordArrayTemp;

cleanup:
    return dwError;
error:
    VmDnsFreeRecordsArray(ppRecordArrayTemp, dwCount);
    if (pppRecordArray)
    {
        *pppRecordArray = NULL;
    }
    goto cleanup;
}

DWORD
VmDnsRpcCopyRecordArray(
    PVMDNS_RECORD_LIST pRecordList,
    PVMDNS_RECORD_ARRAY *ppRecordArray
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_RECORD_ARRAY pRecordArrayTemp = NULL;

    dwError = VmDnsRpcAllocateMemory(
                        sizeof(VMDNS_RECORD_ARRAY),
                        (PVOID*)&pRecordArrayTemp);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRpcAllocateMemory(
                        sizeof(VMDNS_RECORD)*pRecordList->dwCurrentSize,
                        (PVOID*)&pRecordArrayTemp->Records);
    BAIL_ON_VMDNS_ERROR(dwError);

    for (; idx < pRecordList->dwCurrentSize; ++idx)
    {
        dwError = VmDnsRpcCopyRecord(
                        pRecordList->ppRecords[idx]->pRecord,
                        &pRecordArrayTemp->Records[idx]);
        BAIL_ON_VMDNS_ERROR(dwError);
        ++pRecordArrayTemp->dwCount;
    }

    *ppRecordArray = pRecordArrayTemp;

cleanup:
    return dwError;
error:
    VmDnsRpcFreeRecordArray(pRecordArrayTemp);
    if (ppRecordArray)
    {
        *ppRecordArray = NULL;
    }
    goto cleanup;
}
