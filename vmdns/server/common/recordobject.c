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
VmDnsRecordObjectCreate(
    PVMDNS_RECORD           pRecord,
    PVMDNS_RECORD_OBJECT    *ppRecordObj
    )
{
    DWORD dwError = 0;
    PVMDNS_RECORD_OBJECT pRecordObj = NULL;

    if (!ppRecordObj)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(
                    sizeof(VMDNS_RECORD_OBJECT),
                    (PVOID*)&pRecordObj);
    BAIL_ON_VMDNS_ERROR(dwError);

    pRecordObj->lRefCount = 1;
    pRecordObj->pRecord = pRecord;

    *ppRecordObj = pRecordObj;

cleanup:
    return dwError;

error:

    if (ppRecordObj)
    {
        *ppRecordObj = NULL;
    }

    VmDnsRecordObjectRelease(pRecordObj);
    goto cleanup;
}

DWORD
VmDnsRecordObjectAddRef(
    PVMDNS_RECORD_OBJECT    pRecordObj
    )
{
    DWORD dwError = 0;
    if (pRecordObj)
    {
        dwError = InterlockedIncrement(&pRecordObj->lRefCount);
    }
    return dwError;
}

VOID
VmDnsRecordObjectRelease(
    PVMDNS_RECORD_OBJECT    pRecordObj
    )
{
    if (pRecordObj)
    {
        if (0 == InterlockedDecrement(&pRecordObj->lRefCount))
        {
            VMDNS_FREE_RECORD(pRecordObj->pRecord);
            VMDNS_SAFE_FREE_MEMORY(pRecordObj);
        }
    }
}
