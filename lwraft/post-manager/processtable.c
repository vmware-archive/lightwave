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

    gPostMgrGlobals.pProcessTable = pProcessTable;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed with error (%u)", __FUNCTION__, dwError);
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

    VMDIR_RWLOCK_READLOCK(bInLock, gPostMgrGlobals.pProcessTable->pProcessTableLock, 0);

    dwError = LwRtlHashMapFindKey(gPostMgrGlobals.pProcessTable->pProcessMap, (PVOID*) &pProcess, (PVOID) pszKey);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateAndCopyMemory(pProcess, sizeof(VMDIR_PROCESS), (PVOID*) ppProcess);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, gPostMgrGlobals.pProcessTable->pProcessTableLock);
    VMDIR_SAFE_FREE_STRINGA(pszKey);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed with error (%u)", __FUNCTION__, dwError);
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

    VMDIR_RWLOCK_WRITELOCK(bInLock, gPostMgrGlobals.pProcessTable->pProcessTableLock, 0);

    dwError = LwRtlHashMapFindKey(gPostMgrGlobals.pProcessTable->pProcessMap, (PVOID*) &pExistingProcessEntry, (PVOID) pszKey);
    if (dwError == LW_STATUS_NOT_FOUND)
    {
        dwError = VmDirAllocateAndCopyMemory(pProcess, sizeof(VMDIR_PROCESS), (PVOID*) &pNewProcessEntry);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapInsert(gPostMgrGlobals.pProcessTable->pProcessMap, (PVOID) pszKey, (PVOID) pNewProcessEntry, NULL);
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
    VMDIR_RWLOCK_UNLOCK(bInLock, gPostMgrGlobals.pProcessTable->pProcessTableLock);
    VMDIR_SAFE_FREE_STRINGA(pszKey);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed with error (%u)", __FUNCTION__, dwError);
    VMDIR_SAFE_FREE_MEMORY(pNewProcessEntry);
    goto cleanup;
}

DWORD
VmDirProcessTableGetList(
    PVMDIR_PROCESS_LIST *ppProcessList,
    PDWORD              pdwProcessCount
    )
{
    DWORD               dwError = 0;
    PVMDIR_PROCESS_LIST pProcessList = NULL;
    DWORD               dwCount = 0;
    DWORD               dwLoopCount = 0;
    BOOLEAN             bInLock = FALSE;
    LW_HASHMAP_ITER     iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR     pair = {NULL, NULL};

    if (!ppProcessList || !pdwProcessCount)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_RWLOCK_READLOCK(bInLock, gPostMgrGlobals.pProcessTable->pProcessTableLock, 0);

    dwCount = LwRtlHashMapGetCount(gPostMgrGlobals.pProcessTable->pProcessMap);

    *pdwProcessCount = dwCount;

    dwError = VmDirAllocateMemory(sizeof(VMDIR_PROCESS_LIST) * dwCount, (PVOID*)&pProcessList);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (LwRtlHashMapIterate(gPostMgrGlobals.pProcessTable->pProcessMap, &iter, &pair))
    {
        pProcessList[dwLoopCount].dwGroupId = atoi(pair.pKey);
        pProcessList[dwLoopCount].dwState = ((PVMDIR_PROCESS)pair.pValue)->dwState;

        dwLoopCount++;

        if (dwLoopCount == dwCount)
        {
            break;
        }
    }

    *ppProcessList = pProcessList;

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, gPostMgrGlobals.pProcessTable->pProcessTableLock);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed with error (%u)", __FUNCTION__, dwError);
    VMDIR_SAFE_FREE_MEMORY(pProcessList);
    goto cleanup;
}
