/*
 * Copyright ©2019 VMware, Inc.  All Rights Reserved.
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

DWORD
VmDirProcessTableInit(
    VOID
    )
{
    DWORD                   dwError = 0;
    PVMDIR_PROCESS_TABLE    pProcessTable = NULL;

    dwError = VmDirAllocateMemory(
            sizeof(PVMDIR_PROCESS_TABLE),
            (PVOID*)&pProcessTable);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(
            &pProcessTable->pProcessMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateRWLock(
            &pProcessTable->pProcessTableLock);
    BAIL_ON_VMDIR_ERROR(dwError);

    gpProcessTable = pProcessTable;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%d", dwError);
    VMDIR_SAFE_FREE_MEMORY(pProcessTable);
    LwRtlFreeHashMap(&pProcessTable->pProcessMap);
    VMDIR_SAFE_FREE_RWLOCK(pProcessTable->pProcessTableLock);
    goto cleanup;
}

DWORD
VmDirProcessTableRead(
    DWORD           dwGroupId,
    PVMDIR_PROCESS  *ppProcess
    )
{
    DWORD           dwError = 0;
    BOOLEAN         bInLock = FALSE;
    PVMDIR_PROCESS  pProcess = NULL;
    PSTR            pszKey = NULL;

    if (!ppProcess)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateStringPrintf(&pszKey, "%d", dwGroupId);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_RWLOCK_READLOCK(bInLock, gpProcessTable->pProcessTableLock, 0);

    dwError = LwRtlHashMapFindKey(gpProcessTable->pProcessMap, (PVOID*) &pProcess, (PVOID) pszKey);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateAndCopyMemory(pProcess, sizeof(VMDIR_PROCESS), (PVOID*) ppProcess);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, gpProcessTable->pProcessTableLock);
    VMDIR_SAFE_FREE_STRINGA(pszKey);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%d", dwError);
    VMDIR_SAFE_FREE_MEMORY(*ppProcess);
    goto cleanup;
}

DWORD
VmDirProcessTableUpdate(
    DWORD           dwGroupId,
    PVMDIR_PROCESS  pProcess
    )
{
    DWORD           dwError = 0;
    BOOLEAN         bInLock = FALSE;
    PSTR            pszKey = NULL;
    PVMDIR_PROCESS  pExistingProcessEntry = NULL;
    PVMDIR_PROCESS  pNewProcessEntry = NULL;

    if (!pProcess)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateStringPrintf(&pszKey, "%d", dwGroupId);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_RWLOCK_WRITELOCK(bInLock, gpProcessTable->pProcessTableLock, 0);

    dwError = LwRtlHashMapFindKey(gpProcessTable->pProcessMap, (PVOID*) &pExistingProcessEntry, (PVOID) pszKey);
    if (dwError == LW_STATUS_NOT_FOUND)
    {
        dwError = VmDirAllocateAndCopyMemory(pProcess, sizeof(VMDIR_PROCESS), (PVOID*) &pNewProcessEntry);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapInsert(gpProcessTable->pProcessMap, (PVOID) pszKey, (PVOID) pNewProcessEntry, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        pszKey = NULL;
    }
    else if (dwError == 0)
    {
        dwError = VmDirCopyMemory(pExistingProcessEntry, sizeof(VMDIR_PROCESS), pProcess, sizeof(VMDIR_PROCESS));
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, gpProcessTable->pProcessTableLock);
    VMDIR_SAFE_FREE_STRINGA(pszKey);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%d", dwError);
    VMDIR_SAFE_FREE_MEMORY(pNewProcessEntry);
    goto cleanup;
}
