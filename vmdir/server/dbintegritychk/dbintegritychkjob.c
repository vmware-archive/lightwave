/*
 * Copyright © 2019 VMware, Inc.  All Rights Reserved.
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
VmDirDBIntegrityCheckGetCommand(
    PVMDIR_DB_INTEGRITY_CHECK_JOB_CMD    pCmd
    )
{
    DWORD      dwError = 0;
    BOOLEAN    bInLock = FALSE;

    if (pCmd == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_PARAMETER);
    }

    VMDIR_LOCK_MUTEX(bInLock, gVmdirDBIntegrityCheck.pMutex);

    *pCmd = gVmdirDBIntegrityCheck.pJob->command;

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirDBIntegrityCheck.pMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirDBIntegrityCheckGetDBName(
    PSTR*    ppszDBName
    )
{
    DWORD      dwError = 0;
    PSTR       pszDBName = NULL;
    BOOLEAN    bInLock = FALSE;

    if (ppszDBName == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_PARAMETER);
    }

    VMDIR_LOCK_MUTEX(bInLock, gVmdirDBIntegrityCheck.pMutex);

    dwError = VmDirAllocateStringA(gVmdirDBIntegrityCheck.pJob->pszDBName, &pszDBName);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszDBName = pszDBName;
    pszDBName = NULL;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDBName);
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirDBIntegrityCheck.pMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

VOID
VmDirDBIntegrityCheckComplete(
    PVMDIR_STRING_LIST    pDBList,
    VMDIR_DB_INTEGRITY_CHECK_JOB_STATE    state
    )
{
    BOOLEAN    bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirDBIntegrityCheck.pMutex);

    clock_gettime(CLOCK_REALTIME, &gVmdirDBIntegrityCheck.pJob->endTime);
    VmDirStringListFree(gVmdirDBIntegrityCheck.pJob->pAllDBNames);
    gVmdirDBIntegrityCheck.pJob->pAllDBNames = pDBList;
    gVmdirDBIntegrityCheck.pJob->state = state;

    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirDBIntegrityCheck.pMutex);
}

DWORD
VmDirDBIntegrityCheckAllocateJobPerDB(
    DWORD    dwSize
    )
{
    DWORD      dwError = 0;
    BOOLEAN    bInLock = FALSE;
    PVMDIR_DB_INTEGRITY_JOB_PER_DB    pJobPerDB = NULL;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirDBIntegrityCheck.pMutex);

    dwError = VmDirAllocateMemory(
            sizeof(VMDIR_DB_INTEGRITY_JOB_PER_DB) * dwSize, (PVOID*) &pJobPerDB);
    BAIL_ON_VMDIR_ERROR(dwError);

    gVmdirDBIntegrityCheck.pJob->pJobPerDB = pJobPerDB;
    gVmdirDBIntegrityCheck.pJob->dwNumJobPerDB = dwSize;

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirDBIntegrityCheck.pMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirDBIntegrityCheckPopulateJobPerDB(
    PSTR    pszDBName
    )
{
    DWORD      dwError = 0;
    DWORD      dwKeysCount = 0;
    BOOLEAN    bInLock = FALSE;
    PVMDIR_DB_INTEGRITY_JOB_PER_DB    pJobPerDB = NULL;
    VDIR_BACKEND_CTX                  beCtx = {0};

    beCtx.pBE = VmDirBackendSelect(NULL);

    dwError = beCtx.pBE->pfnBEBackendGetDBKeysCount(pszDBName, &dwKeysCount);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOCK_MUTEX(bInLock, gVmdirDBIntegrityCheck.pMutex);

    pJobPerDB = gVmdirDBIntegrityCheck.pJob->pJobPerDB +
        gVmdirDBIntegrityCheck.pJob->dwNumValidJobPerDB;

    dwError = VmDirAllocateStringA(pszDBName, &pJobPerDB->pszDBName);
    BAIL_ON_VMDIR_ERROR(dwError);

    clock_gettime(CLOCK_REALTIME, &pJobPerDB->startTime);
    pJobPerDB->state = DB_INTEGRITY_CHECK_JOB_INPROGRESS;
    pJobPerDB->dwTotalKeys = dwKeysCount;

    gVmdirDBIntegrityCheck.pJob->dwNumValidJobPerDB++;

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirDBIntegrityCheck.pMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirDBIntegrityCheckUpdateJobPerIter(
    DWORD    dwJobPerDBIndex,
    DWORD    dwKeysProcessed,
    PVMDIR_DB_INTEGRITY_CHECK_JOB_STATE    pState
    )
{
    DWORD      dwError = 0;
    BOOLEAN    bInLock = FALSE;

    if (pState == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_PARAMETER);
    }

    VMDIR_LOCK_MUTEX(bInLock, gVmdirDBIntegrityCheck.pMutex);

    gVmdirDBIntegrityCheck.pJob->pJobPerDB[dwJobPerDBIndex].dwKeysProcessed = dwKeysProcessed;

    if (gVmdirDBIntegrityCheck.pJob->state == DB_INTEGRITY_CHECK_JOB_STOP)
    {
        gVmdirDBIntegrityCheck.pJob->pJobPerDB[dwJobPerDBIndex].state = DB_INTEGRITY_CHECK_JOB_STOP;
    }

    *pState = gVmdirDBIntegrityCheck.pJob->pJobPerDB[dwJobPerDBIndex].state;

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirDBIntegrityCheck.pMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

VOID
VmDirDBIntegrityCheckUpdateJobComplete(
    DWORD    dwJobPerDBIndex,
    DWORD    dwKeysProcessed,
    VMDIR_DB_INTEGRITY_CHECK_JOB_STATE    state
    )
{
    BOOLEAN    bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirDBIntegrityCheck.pMutex);

    gVmdirDBIntegrityCheck.pJob->pJobPerDB[dwJobPerDBIndex].state = state;
    gVmdirDBIntegrityCheck.pJob->pJobPerDB[dwJobPerDBIndex].dwKeysProcessed = dwKeysProcessed;
    clock_gettime(CLOCK_REALTIME, &gVmdirDBIntegrityCheck.pJob->pJobPerDB[dwJobPerDBIndex].endTime);

    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirDBIntegrityCheck.pMutex);

    return;
}

DWORD
VmDirDBIntegrityCheckJobGetDBName(
    DWORD    dwJobPerDBIndex,
    PSTR*    ppszDBName
    )
{
    DWORD      dwError = 0;
    PSTR       pszDBName = NULL;
    BOOLEAN    bInLock = FALSE;

    if (ppszDBName == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_PARAMETER);
    }

    VMDIR_LOCK_MUTEX(bInLock, gVmdirDBIntegrityCheck.pMutex);

    dwError = VmDirAllocateStringA(
            gVmdirDBIntegrityCheck.pJob->pJobPerDB[dwJobPerDBIndex].pszDBName, &pszDBName);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszDBName = pszDBName;

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirDBIntegrityCheck.pMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

VOID
VmDirDBIntegrityCheckJobFree(
    PVMDIR_DB_INTEGRITY_JOB    pJob
    )
{
    DWORD      dwJobCnt = 0;

    if (pJob)
    {
        VMDIR_SAFE_FREE_MEMORY(pJob->pszDBName);

        VmDirStringListFree(pJob->pAllDBNames);
        pJob->pAllDBNames = NULL;

        for (dwJobCnt = 0; dwJobCnt < pJob->dwNumValidJobPerDB; dwJobCnt++)
        {
            VMDIR_SAFE_FREE_MEMORY(pJob->pJobPerDB[dwJobCnt].pszDBName);
        }

        VMDIR_SAFE_FREE_MEMORY(pJob->pJobPerDB);
        VMDIR_SAFE_FREE_MEMORY(pJob);
    }

    return;
}
