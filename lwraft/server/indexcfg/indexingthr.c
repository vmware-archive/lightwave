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

DWORD
InitializeIndexingThread(
    VOID
    )
{
    DWORD   dwError = 0;

    dwError = VmDirSrvThrInit(
                &gVdirIndexGlobals.pThrInfo,
                gVdirIndexGlobals.mutex,
                gVdirIndexGlobals.cond,
                TRUE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateThread(
            &gVdirIndexGlobals.pThrInfo->tid,
            gVdirIndexGlobals.pThrInfo->bJoinThr,
            VmDirIndexingThreadFun,
            gVdirIndexGlobals.pThrInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirIndexingThreadFun(
    PVOID   pArg
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    BOOLEAN bResume = FALSE;
    VDIR_SERVER_STATE   vmdirState = VMDIRD_STATE_UNDEFINED;
    PVDIR_INDEXING_TASK pTask = NULL;

resume:
    while (1)
    {
        vmdirState = VmDirdState();
        if (vmdirState == VMDIRD_STATE_SHUTDOWN)
        {
            break;
        }
        else if (vmdirState != VMDIRD_STATE_NORMAL)
        {
            VmDirSleep(1000);
            continue;
        }

        VMDIR_LOCK_MUTEX(bInLock, gVdirIndexGlobals.mutex);

        if (!bResume)
        {
            PVDIR_INDEX_UPD pIndexUpd = gVdirIndexGlobals.pIndexUpd;

            // record current progress
            dwError = VmDirIndexingTaskRecordProgress(pTask, pIndexUpd);
            BAIL_ON_VMDIR_ERROR(dwError);

            // apply index updates
            dwError = VmDirIndexUpdApply(pIndexUpd);
            BAIL_ON_VMDIR_ERROR(dwError);

            VmDirIndexUpdFree(pIndexUpd);
            gVdirIndexGlobals.pIndexUpd = NULL;

            // compute new task
            VmDirFreeIndexingTask(pTask);
            dwError = VmDirIndexingTaskCompute(&pTask);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (VmDirIndexingTaskIsNoop(pTask))
        {
            dwError = VmDirConditionWait(
                    gVdirIndexGlobals.cond,
                    gVdirIndexGlobals.mutex);
            BAIL_ON_VMDIR_ERROR(dwError);

            continue;
        }

        VMDIR_UNLOCK_MUTEX(bInLock, gVdirIndexGlobals.mutex);

        dwError = VmDirIndexingTaskPopulateIndices(pTask);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirIndexingTaskValidateScopes(pTask);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirIndexingTaskDeleteIndices(pTask);
        BAIL_ON_VMDIR_ERROR(dwError);
        bResume = FALSE;
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVdirIndexGlobals.mutex);
    VmDirFreeIndexingTask(pTask);
    return dwError;

error:
    if (dwError == ERROR_INVALID_STATE)
    {
        bResume = TRUE;
        goto resume;
    }
    else
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                "%s failed, error (%d)", __FUNCTION__, dwError );
    }
    goto cleanup;
}
