/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ?~@~\License?~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ?~@~\AS IS?~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "includes.h"

static VMDIR_BKGD_TASK_CTX tasks[] =
{
        {
                VmDirBkgdSrvStat,
                60 * 10, // every 10 minute
                "bkgdprevtime_update_srvstat",
                {0}
        }
};

DWORD
VmDirBkgdThreadInitialize(
    VOID
    )
{
    DWORD   dwError = 0;
    DWORD   dwNumTasks = 0, i = 0;
    PSTR    pszTmp = NULL;

    dwNumTasks = sizeof(tasks)/sizeof(tasks[0]);

    // load previous timestamps
    for (i = 0; i < dwNumTasks; i++)
    {
        VMDIR_SAFE_FREE_MEMORY(pszTmp);
        (VOID)VmDirBackendUniqKeyGetValue(tasks[i].pszPrevTimeKey, &pszTmp);

        dwError = VmDirStringCpyA(
                tasks[i].pszPrevTime,
                GENERALIZED_TIME_STR_SIZE,
                VDIR_SAFE_STRING(pszTmp));
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // start background thread
    gVmdirBkgdGlobals.bShutdown = FALSE;

    dwError = VmDirSrvThrInit(&gVmdirBkgdGlobals.pThrInfo, NULL, NULL, TRUE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateThread(
            &gVmdirBkgdGlobals.pThrInfo->tid,
            gVmdirBkgdGlobals.pThrInfo->bJoinThr,
            VmDirBkgdThreadFun,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszTmp);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

VOID
VmDirBkgdThreadShutdown(
    VOID
    )
{
    // stop background thread
    gVmdirBkgdGlobals.bShutdown = TRUE;

    if (gVmdirBkgdGlobals.pThrInfo)
    {
        VmDirSrvThrShutdown(gVmdirBkgdGlobals.pThrInfo);
        gVmdirBkgdGlobals.pThrInfo = NULL;
    }
}

DWORD
VmDirBkgdTaskUpdatePrevTime(
    PVMDIR_BKGD_TASK_CTX    pTaskCtx
    )
{
    DWORD   dwError = 0;

    if (!pTaskCtx)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VmDirCurrentGeneralizedTime(
            pTaskCtx->pszPrevTime, sizeof(pTaskCtx->pszPrevTime));

    dwError = VmDirBackendUniqKeySetValue(
            pTaskCtx->pszPrevTimeKey, pTaskCtx->pszPrevTime, TRUE);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirBkgdThreadFun(
    PVOID   pArg
    )
{
    DWORD   dwNumTasks = 0;
    DWORD   i = 0;
    BOOLEAN bInLock = FALSE;
    char    pszExpireTime[GENERALIZED_TIME_STR_SIZE] = {0};
    PVDIR_THREAD_INFO       pThrInfo = NULL;

    pThrInfo = gVmdirBkgdGlobals.pThrInfo;

    // service warm-up time - 1 minute
    VMDIR_LOCK_MUTEX(bInLock, pThrInfo->mutex);
    (VOID)VmDirConditionTimedWait(pThrInfo->condition, pThrInfo->mutex, 1000 * 60);
    VMDIR_UNLOCK_MUTEX(bInLock, pThrInfo->mutex);

    dwNumTasks = sizeof(tasks)/sizeof(tasks[0]);

    while (!gVmdirBkgdGlobals.bShutdown)
    {
        // perform tasks only if promoted
        if (gVmdirServerGlobals.bPromoted)
        {
            for (i = 0; i < dwNumTasks; i++)
            {
                VmDirCurrentGeneralizedTimeWithOffset(
                        pszExpireTime, sizeof(pszExpireTime), tasks[i].dwPeriod);

                if (VmDirStringCompareA(tasks[i].pszPrevTime, pszExpireTime, FALSE) < 0)
                {
                    // don't bail - continue to next task
                    (VOID)tasks[i].pFunc(&tasks[i]);
                }
            }
        }

        // sleep for 10 seconds
        VMDIR_LOCK_MUTEX(bInLock, pThrInfo->mutex);
        (VOID)VmDirConditionTimedWait(pThrInfo->condition, pThrInfo->mutex, 1000 * 10);
        VMDIR_UNLOCK_MUTEX(bInLock, pThrInfo->mutex);
    }

    return 0;
}

DWORD
VmDirBkgdSrvStat(
    PVMDIR_BKGD_TASK_CTX    pTaskCtx
    )
{
    DWORD   dwError = 0;

    if (!pTaskCtx)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

//    dwError = VmDirUpdateSrvStat();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirBkgdTaskUpdatePrevTime(pTaskCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}
