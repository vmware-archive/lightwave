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

PSTR dbIntegrityCheckStateToString[] = {
    "Job None",
    "Job Start",
    "Job Inprogress",
    "Job Complete",
    "Job Stopped",
    "Job Failed",
    "Job ShowSummary"
};

PSTR dbIntegrityCheckCommandToString[] = {
    "List DB Names",
    "Check all DBs",
    "Check sub DB"
};

DWORD
VmDirDBIntegrityCheckStart(
    PVMDIR_DB_INTEGRITY_JOB    pDBIntegrityCheckJob
    )
{
    DWORD      dwError = 0;
    BOOLEAN    bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirDBIntegrityCheck.pMutex);

    if (gVmdirDBIntegrityCheck.pJob &&
        gVmdirDBIntegrityCheck.pJob->state == DB_INTEGRITY_CHECK_JOB_INPROGRESS)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_UNWILLING_TO_PERFORM);
    }

    VmDirDBIntegrityCheckJobFree(gVmdirDBIntegrityCheck.pJob);

    pDBIntegrityCheckJob->state = DB_INTEGRITY_CHECK_JOB_INPROGRESS;
    clock_gettime(CLOCK_REALTIME, &pDBIntegrityCheckJob->startTime);

    gVmdirDBIntegrityCheck.pJob = pDBIntegrityCheckJob;

    dwError = VmDirDBIntegrityCheckCreateThread();
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirDBIntegrityCheck.pMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

VOID
VmDirDBIntegrityCheckStop(
    VOID
    )
{
    BOOLEAN    bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirDBIntegrityCheck.pMutex);

    if (gVmdirDBIntegrityCheck.pJob &&
        gVmdirDBIntegrityCheck.pJob->state == DB_INTEGRITY_CHECK_JOB_INPROGRESS)
    {
        gVmdirDBIntegrityCheck.pJob->state = DB_INTEGRITY_CHECK_JOB_STOP;
    }

    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirDBIntegrityCheck.pMutex);
}

DWORD
VmDirDBIntegrityCheckShowStatus(
    PVDIR_ENTRY*    ppEntry
    )
{
    DWORD          dwError = 0;
    BOOLEAN        bInLock = FALSE;
    PVDIR_ENTRY    pEntry = NULL;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirDBIntegrityCheck.pMutex);

    if (gVmdirDBIntegrityCheck.pJob == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_UNWILLING_TO_PERFORM);
    }

    if (gVmdirDBIntegrityCheck.pJob->command == DB_INTEGRITY_CHECK_LIST)
    {
        dwError = VmDirDBIntegrityCheckStatusForListCommand_InLock(
                gVmdirDBIntegrityCheck.pJob, &pEntry);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if(gVmdirDBIntegrityCheck.pJob->command == DB_INTEGRITY_CHECK_ALL ||
            gVmdirDBIntegrityCheck.pJob->command == DB_INTEGRITY_CHECK_SUBDB)
    {
        dwError = VmDirDBIntegrityCheckStatusForDBCommand_InLock(
                gVmdirDBIntegrityCheck.pJob, &pEntry);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppEntry = pEntry;

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirDBIntegrityCheck.pMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirDBIntegrityCheckStatusForListCommand_InLock(
    PVMDIR_DB_INTEGRITY_JOB    pJob,
    PVDIR_ENTRY*               ppEntry
    )
{
    DWORD      dwError = 0;
    DWORD      dwNumAttrs = 0;
    DWORD      dwCnt = 0;
    DWORD      dwIter = 0;
    PSTR*      ppszAttrList = NULL;
    PSTR       pszStartTime = NULL;
    PSTR       pszEndTime = NULL;
    PSTR       pszState = NULL;
    PSTR       pszCommand = NULL;
    PVDIR_ENTRY         pEntry = NULL;
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;
    struct tm           myTM = {0};

    if (pJob == NULL ||
        pJob->state != DB_INTEGRITY_CHECK_JOB_COMPLETE)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_UNWILLING_TO_PERFORM);
    }

    dwNumAttrs = 2 + //cn and oc
                 2 + //start and end time
                 2 + //state and command
                 pJob->pAllDBNames->dwCount; //all DB Names

    gmtime_r(&pJob->startTime.tv_sec, &myTM);

    dwError = VmDirAllocateStringPrintf(
            &pszStartTime,
            "%-20s %4d-%02d-%02d:%02d:%02d:%02d",
            "start time: ",
            myTM.tm_year+1900,
            myTM.tm_mon+1,
            myTM.tm_mday,
            myTM.tm_hour,
            myTM.tm_min,
            myTM.tm_sec);
    BAIL_ON_VMDIR_ERROR(dwError);

    gmtime_r(&pJob->endTime.tv_sec, &myTM);

    dwError = VmDirAllocateStringPrintf(
            &pszEndTime,
            "%-20s %4d-%02d-%02d:%02d:%02d:%02d",
            "end time: ",
            myTM.tm_year+1900,
            myTM.tm_mon+1,
            myTM.tm_mday,
            myTM.tm_hour,
            myTM.tm_min,
            myTM.tm_sec);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pszState,
            "%s%s",
            "state: ",
            dbIntegrityCheckStateToString[pJob->state]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pszCommand,
            "%s%s",
            "command: ",
            dbIntegrityCheckCommandToString[pJob->command]);
    BAIL_ON_VMDIR_ERROR(dwError);

    //populate attributes
    dwError = VmDirAllocateMemory(
            sizeof(PSTR) * ((dwNumAttrs) * 2 + 1), (PVOID)&ppszAttrList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(ATTR_CN, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(DB_INTEGRITY_CHECK_STATUS_CN, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(ATTR_OBJECT_CLASS, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(OC_SERVER_STATUS, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(ATTR_SERVER_RUNTIME_STATUS, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszCommand, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(ATTR_SERVER_RUNTIME_STATUS, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszStartTime, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(ATTR_SERVER_RUNTIME_STATUS, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszEndTime, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(ATTR_SERVER_RUNTIME_STATUS, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszState, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwIter = 0; dwIter < pJob->pAllDBNames->dwCount; dwIter++)
    {
        dwError = VmDirAllocateStringA(ATTR_SERVER_RUNTIME_STATUS, &ppszAttrList[dwCnt++]);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(
                pJob->pAllDBNames->pStringList[dwIter], &ppszAttrList[dwCnt++]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAttrListToNewEntry(
            pSchemaCtx,
            DB_INTEGRITY_CHECK_STATUS_DN,
            ppszAttrList,
            FALSE,
            &pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppEntry = pEntry;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszState);
    VMDIR_SAFE_FREE_MEMORY(pszCommand);
    VMDIR_SAFE_FREE_MEMORY(pszStartTime);
    VMDIR_SAFE_FREE_MEMORY(pszEndTime);
    VmDirFreeStrArray(ppszAttrList);
    VmDirSchemaCtxRelease(pSchemaCtx);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    VmDirFreeEntry(pEntry);
    goto cleanup;
}

DWORD
VmDirDBIntegrityCheckStatusForDBCommand_InLock(
    PVMDIR_DB_INTEGRITY_JOB    pJob,
    PVDIR_ENTRY*               ppEntry
    )
{
    DWORD      dwError = 0;
    DWORD      dwNumAttrs = 0;
    DWORD      dwCnt = 0;
    DWORD      dwIter = 0;
    PSTR*      ppszAttrList = NULL;
    PSTR       pszState = NULL;
    PSTR       pszCommand = NULL;
    PSTR       pszJobPerDBStartTime = NULL;
    PSTR       pszJobPerDBEndTime = NULL;
    PSTR       pszJobPerDBKeysProcessed = NULL;
    PSTR       pszJobPerDBName = NULL;
    PSTR       pszJobPerDBState = NULL;
    PSTR       pszJobPerDBTotalKeys = NULL;
    PVDIR_ENTRY         pEntry = NULL;
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;
    struct tm           myTM = {0};

    if (pJob == NULL ||
        pJob->state == DB_INTEGRITY_CHECK_JOB_START ||
        pJob->state == DB_INTEGRITY_CHECK_JOB_NONE)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_UNWILLING_TO_PERFORM);
    }

    dwNumAttrs = 2 + //cn and oc
                 2 + //state and command
                 pJob->dwNumValidJobPerDB * 6; //dbname, state, keysCnt, totalKeys, start & endtime

    dwError = VmDirAllocateStringPrintf(
            &pszState,
            "%s%s",
            "state: ",
            dbIntegrityCheckStateToString[pJob->state]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pszCommand,
            "%s%s",
            "command: ",
            dbIntegrityCheckCommandToString[pJob->command]);
    BAIL_ON_VMDIR_ERROR(dwError);

    //populate attributes
    dwError = VmDirAllocateMemory(
            sizeof(PSTR) * ((dwNumAttrs) * 2 + 1), (PVOID)&ppszAttrList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(ATTR_CN, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(DB_INTEGRITY_CHECK_STATUS_CN, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(ATTR_OBJECT_CLASS, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(OC_SERVER_STATUS, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(ATTR_SERVER_RUNTIME_STATUS, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszCommand, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(ATTR_SERVER_RUNTIME_STATUS, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszState, &ppszAttrList[dwCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwIter = 0; dwIter < pJob->dwNumValidJobPerDB; dwIter++)
    {
        VMDIR_SAFE_FREE_MEMORY(pszJobPerDBName);
        dwError = VmDirAllocateStringPrintf(
                &pszJobPerDBName,
                "DB Name: %s",
                pJob->pJobPerDB[dwIter].pszDBName);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(ATTR_SERVER_RUNTIME_STATUS, &ppszAttrList[dwCnt++]);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(pszJobPerDBName, &ppszAttrList[dwCnt++]);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_SAFE_FREE_MEMORY(pszJobPerDBState);
        dwError = VmDirAllocateStringPrintf(
                &pszJobPerDBState,
                "state: %s",
                dbIntegrityCheckStateToString[pJob->pJobPerDB[dwIter].state]);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(ATTR_SERVER_RUNTIME_STATUS, &ppszAttrList[dwCnt++]);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(pszJobPerDBState, &ppszAttrList[dwCnt++]);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_SAFE_FREE_MEMORY(pszJobPerDBKeysProcessed);
        dwError = VmDirAllocateStringPrintf(
                &pszJobPerDBKeysProcessed,
                "keys processed: %d",
                pJob->pJobPerDB[dwIter].dwKeysProcessed);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(ATTR_SERVER_RUNTIME_STATUS, &ppszAttrList[dwCnt++]);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(pszJobPerDBKeysProcessed, &ppszAttrList[dwCnt++]);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_SAFE_FREE_MEMORY(pszJobPerDBTotalKeys);
        dwError = VmDirAllocateStringPrintf(
                &pszJobPerDBTotalKeys,
                "total keys: %d",
                pJob->pJobPerDB[dwIter].dwTotalKeys);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(ATTR_SERVER_RUNTIME_STATUS, &ppszAttrList[dwCnt++]);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(pszJobPerDBTotalKeys, &ppszAttrList[dwCnt++]);
        BAIL_ON_VMDIR_ERROR(dwError);

        gmtime_r(&pJob->pJobPerDB[dwIter].startTime.tv_sec, &myTM);

        VMDIR_SAFE_FREE_MEMORY(pszJobPerDBStartTime);
        dwError = VmDirAllocateStringPrintf(
                &pszJobPerDBStartTime,
                "%-20s %4d-%02d-%02d:%02d:%02d:%02d",
                "start time: ",
                myTM.tm_year+1900,
                myTM.tm_mon+1,
                myTM.tm_mday,
                myTM.tm_hour,
                myTM.tm_min,
                myTM.tm_sec);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(ATTR_SERVER_RUNTIME_STATUS, &ppszAttrList[dwCnt++]);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(pszJobPerDBStartTime, &ppszAttrList[dwCnt++]);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pJob->pJobPerDB[dwIter].state == DB_INTEGRITY_CHECK_JOB_COMPLETE)
        {
            gmtime_r(&pJob->pJobPerDB[dwIter].endTime.tv_sec, &myTM);

            VMDIR_SAFE_FREE_MEMORY(pszJobPerDBEndTime);
            dwError = VmDirAllocateStringPrintf(
                    &pszJobPerDBEndTime,
                    "%-20s %4d-%02d-%02d:%02d:%02d:%02d",
                    "end time: ",
                    myTM.tm_year+1900,
                    myTM.tm_mon+1,
                    myTM.tm_mday,
                    myTM.tm_hour,
                    myTM.tm_min,
                    myTM.tm_sec);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirAllocateStringA(ATTR_SERVER_RUNTIME_STATUS, &ppszAttrList[dwCnt++]);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirAllocateStringA(pszJobPerDBEndTime, &ppszAttrList[dwCnt++]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAttrListToNewEntry(
            pSchemaCtx,
            DB_INTEGRITY_CHECK_STATUS_DN,
            ppszAttrList,
            FALSE,
            &pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppEntry = pEntry;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszJobPerDBStartTime);
    VMDIR_SAFE_FREE_MEMORY(pszJobPerDBEndTime);
    VMDIR_SAFE_FREE_MEMORY(pszJobPerDBState);
    VMDIR_SAFE_FREE_MEMORY(pszJobPerDBKeysProcessed);
    VMDIR_SAFE_FREE_MEMORY(pszJobPerDBTotalKeys);
    VMDIR_SAFE_FREE_MEMORY(pszJobPerDBName);
    VMDIR_SAFE_FREE_MEMORY(pszState);
    VMDIR_SAFE_FREE_MEMORY(pszCommand);
    VmDirSchemaCtxRelease(pSchemaCtx);
    VmDirFreeStrArray(ppszAttrList);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    VmDirFreeEntry(pEntry);
    goto cleanup;
}

DWORD
VmDirMDBSubDBIntegrityCheck(
    DWORD    dwJobPerDBIndex
    )
{
    DWORD    dwError = 0;
    DWORD    dwKeysProcessed = 0;
    DWORD    dwTotalKeysProcessed = 0;
    PSTR     pszDBName = NULL;
    VDIR_BACKEND_CTX                beCtx = {0};
    VMDIR_COMPACT_KV_PAIR           data = {0};
    PVDIR_BACKEND_TABLE_ITERATOR    pIterator = NULL;
    VMDIR_DB_INTEGRITY_CHECK_JOB_STATE    state = DB_INTEGRITY_CHECK_JOB_NONE;

    beCtx.pBE = VmDirBackendSelect(NULL);

    dwError = VmDirDBIntegrityCheckJobGetDBName(dwJobPerDBIndex, &pszDBName);
    BAIL_ON_VMDIR_ERROR(dwError);

dbIterator:

    dwError = VmDirDBIntegrityCheckUpdateJobPerIter(
            dwJobPerDBIndex, dwTotalKeysProcessed, &state);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwKeysProcessed = 0;

    if (state == DB_INTEGRITY_CHECK_JOB_STOP || VmDirdState() == VMDIRD_STATE_SHUTDOWN)
    {
        goto cleanup;
    }

    dwError = beCtx.pBE->pfnBEBackendDBIteratorInit(pszDBName, &data, &pIterator);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (pIterator->bHasNext && dwKeysProcessed < VMDIR_SIZE_256)
    {
        dwTotalKeysProcessed++;
        dwKeysProcessed++;

        VmDirFreeMDBIteratorDataContents(&data);

        dwError = beCtx.pBE->pfnBEBackendDBIterator(pIterator, &data);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pIterator->bHasNext)
    {
        dwTotalKeysProcessed--; // same key will be selected again in iterator init
        beCtx.pBE->pfnBEBackendDBIteratorFree(pIterator);
        pIterator = NULL;
    }

    if (!pIterator)
    {
        goto dbIterator;
    }

    VmDirDBIntegrityCheckUpdateJobComplete(
            dwJobPerDBIndex, dwTotalKeysProcessed, DB_INTEGRITY_CHECK_JOB_COMPLETE);

    VMDIR_LOG_INFO(
            VMDIR_LOG_MASK_ALL, "%s: DB: %s keys: %d", __FUNCTION__, pszDBName, dwTotalKeysProcessed);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDBName);
    VmDirFreeMDBIteratorDataContents(&data);
    beCtx.pBE->pfnBEBackendDBIteratorFree(pIterator);
    return dwError;

error:
    VmDirDBIntegrityCheckUpdateJobComplete(
            dwJobPerDBIndex, dwTotalKeysProcessed, DB_INTEGRITY_CHECK_JOB_FAILED);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}
