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



/*
 * Module Name: Bdb data store
 *
 * Filename: logmgmt.c
 *
 * Abstract: handle bdb checkpoint and log aging
 *
 *
 */

#include "includes.h"

static
DWORD
bdbLogfileAging(
    VOID
    );

static
DWORD
VmDirBdbCheckpointThrFun(
    PVOID   pArg
    );

DWORD
InitializeDbChkpointThread(
    VOID)
{
    DWORD               dwError = 0;
    PVDIR_THREAD_INFO   pThrInfo = NULL;

    dwError = VmDirAllocateMemory(
            sizeof(*pThrInfo),
            (PVOID)&pThrInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSrvThrInit(
            pThrInfo,
            NULL,
            NULL,
            TRUE);  // join by main thr

    dwError = VmDirCreateThread(
            &pThrInfo->tid,
            FALSE,
            VmDirBdbCheckpointThrFun,
            pThrInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSrvThrAdd(pThrInfo);

cleanup:

    return (int)dwError;  //TODO, should not cast

error:

    if (pThrInfo)
    {
        VmDirSrvThrFree(pThrInfo);
    }

    goto cleanup;
}

/*
 * Thread to handle db checkpoint and log file aging.
 * 1.  it sleep for x (configurable) minutes.
 * 2.  check how long ago was lost check point
 * 2.1   if longer than y (configurable) minutes then check point
 *
 */
static
DWORD
VmDirBdbCheckpointThrFun(
    PVOID   pArg
    )
{
    DWORD   dwError = 0;
    int     iCFGChkpointIntervalinMIN = 0;
    int     iCFGCheckpointSizeinKB = 0;
    int     iCFGLogAgingIntervalInMIN = 0;

    time_t  tLastLogAge = time(NULL);
    PVDIR_THREAD_INFO   pThrInfo = (PVDIR_THREAD_INFO)pArg;

    //TODO, make this configurable (or even changeable during online)
    iCFGChkpointIntervalinMIN = 3;
    iCFGCheckpointSizeinKB = 100;
    iCFGLogAgingIntervalInMIN = 360;

    while (1)
    {
        struct timespec ts = {0};
        BOOLEAN bInLock = FALSE;

        ts.tv_sec = time(NULL) + (iCFGChkpointIntervalinMIN * 60);
        ts.tv_nsec = 0;

        VMDIR_LOCK_MUTEX(bInLock, pThrInfo->mutexUsed);

        dwError = VmDirConditionTimedWait(
                pThrInfo->conditionUsed,
                pThrInfo->mutexUsed,
                (iCFGChkpointIntervalinMIN * 60)*1000);

        VMDIR_UNLOCK_MUTEX(bInLock, pThrInfo->mutexUsed);

        if (VmDirdState() == VMDIR_SHUTDOWN)
        {
            break;
        }

        // ignore dwError == ETIMEOUT check as rare case and benign checkpoint

        dwError = gVdirBdbGlobals.bdbEnv->txn_checkpoint(
                gVdirBdbGlobals.bdbEnv,
                iCFGCheckpointSizeinKB,
                iCFGChkpointIntervalinMIN ,
                0);
        BAIL_ON_VMDIR_ERROR(dwError);
        VmDirLog( LDAP_DEBUG_TRACE, "Bdb: checkpoint" );

        if (ts.tv_sec - tLastLogAge > (iCFGLogAgingIntervalInMIN * 60))
        {
            dwError = bdbLogfileAging();
            BAIL_ON_VMDIR_ERROR(dwError);

            tLastLogAge = ts.tv_sec;
        }
    }

cleanup:

    // TODO: return dwError ?
    return 0;

error:

    raise(SIGTERM);

    goto cleanup;
}

/*
 * Function to delete unnecessary log files after db checkpoint.
 * It always keep at least X (configurable) log files available.
 */
static
DWORD
bdbLogfileAging(
    VOID
    )
{
    DWORD   dwError = 0;
    int     iCFGMinLogFiles = 0;

    PSTR*   ppszLogFiles = NULL;
    int     iNumLogFiles = 0;
    int     iCnt = 0;

    //TODO, make this configurable
    iCFGMinLogFiles = 40;

    // Get the list of log files
    dwError = gVdirBdbGlobals.bdbEnv->log_archive(
            gVdirBdbGlobals.bdbEnv,
            &ppszLogFiles,
            DB_ARCH_ABS | DB_ARCH_LOG);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (ppszLogFiles != NULL) {

        for (iNumLogFiles = 0; ppszLogFiles[iNumLogFiles]; iNumLogFiles++)
        {
            ;
        }

        if (iNumLogFiles <= iCFGMinLogFiles)
        {
            goto cleanup;
        }

        //TODO, best to sort by lastModified time stamp.
        //bdb log files has format - log.00000000001, log.00000000002 ...
        qsort(ppszLogFiles,
              iNumLogFiles,
              sizeof(PSTR),
              VmDirQsortPPCHARCmp);

        for (iCnt = 0; iCnt < iNumLogFiles - iCFGMinLogFiles; iCnt++)
        {
            dwError = unlink(ppszLogFiles[iCnt]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        VmDirLog( LDAP_DEBUG_ANY,
                "Bdb: Age log file (%s ~ %s)",
                ppszLogFiles[0],
                ppszLogFiles[iCnt-1]);
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(ppszLogFiles);

    return dwError;

error:

    goto cleanup;
}
