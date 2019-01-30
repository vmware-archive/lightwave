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

static
DWORD
_VmDirMonitorProcess(
    pid_t pid,
    DWORD dwGroupId
    );

static
DWORD
_VmDirExecProgram(
    DWORD dwGroupId
    );

static
DWORD
_VmDirProcessThread(
    PVOID  pArg
    );

DWORD
VmDirStartProcess(
    DWORD   dwGroupId
    )
{
    DWORD           dwError = 0;
    VMDIR_THREAD    tid = -1;
    DWORD           *pdwGroupId = NULL;

    dwError = VmDirAllocateAndCopyMemory(&dwGroupId, sizeof(DWORD), (PVOID*) &pdwGroupId);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateThread(
                &tid,
                TRUE,
                _VmDirProcessThread,
                (PVOID) pdwGroupId);
    BAIL_ON_VMDIR_ERROR(dwError);

    /*TODO: Wait for pProcess->dwStatus to change to RUNNING*/

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: Error: %d", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirStopProcess(
    DWORD   dwGroupId
    )
{
    DWORD           dwError = 0;
    PVMDIR_PROCESS  pProcess = NULL;
    PVMDIR_PROCESS  pProcessRead = NULL;

    dwError = VmDirProcessTableRead(dwGroupId, &pProcess);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pProcess->dwState == VMDIR_STATE_STOPPED ||
        pProcess->dwState == VMDIR_STATE_DEAD)
    {
        goto cleanup;
    }

    dwError = kill(pProcess->pid, SIGTERM);
    if (dwError != 0)
    {
        VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL, "Failed to send signal to process &d errno %d", pProcess->pid, errno);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_GENERIC);
    }

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Sent SIGTERM to process %d", pProcess->pid);

    pProcess->dwState = VMDIR_STATE_STOPPING;

    dwError = VmDirProcessTableUpdate(dwGroupId, pProcess);
    BAIL_ON_VMDIR_ERROR(dwError);

    /*Wait for monitoring thread to set state to VMDIR_STATE_STOPPED*/
    while(TRUE)
    {
        dwError = VmDirProcessTableRead(dwGroupId, &pProcessRead);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pProcessRead->dwState == VMDIR_STATE_STOPPED)
        {
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Post process %d has stopped", pProcessRead->pid);
            /*TODO: Cleanup entry from process table if the process is demoted*/
            break;
        }

        VMDIR_SAFE_FREE_MEMORY(pProcessRead);
        sleep(1);
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pProcess);
    VMDIR_SAFE_FREE_MEMORY(pProcessRead);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: Error: %d", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirStopAllProcesses(
    VOID
    )
{
    DWORD  dwError = 0;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};
    DWORD           dwGroupId = 0;

    while (gPostMgrGlobals.pProcessTable->pProcessMap &&
           LwRtlHashMapIterate(gPostMgrGlobals.pProcessTable->pProcessMap, &iter, &pair))
    {
        dwGroupId = atoi(pair.pKey);

        dwError = VmDirStopProcess(dwGroupId);
        if (dwError != 0)
        {
            VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL, "Failed to stop process for group %d error %d", dwGroupId, dwError);
        }
    }

    return dwError;
}

static
DWORD
_VmDirProcessThread(
    PVOID  pArg
    )
{
    DWORD           dwError = 0;
    pid_t           pid = -1;
    DWORD           dwGroupId = *(DWORD*)pArg;
    VMDIR_PROCESS   process = {0};
    PVMDIR_PROCESS  pProcessRead = NULL;
    DWORD           dwRetryCount = 0;

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Starting process for group %d", dwGroupId);

    for (dwRetryCount = 0; dwRetryCount < 3; dwRetryCount++)
    {
        switch (pid = fork())
        {
        case -1:
            VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL, "Failed to fork errno %d", errno);
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_GENERIC);
            break;

        case 0:
            /* In Postd process*/
            dwError = _VmDirExecProgram(dwGroupId);
            BAIL_ON_VMDIR_ERROR(dwError);
            break;

        default:
            /*In post-manager*/
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Started process %d", pid);

            /*Insert entry into process table*/
            process.pid = pid;
            process.dwState = VMDIR_STATE_STARTING;

            dwError = VmDirProcessTableUpdate(dwGroupId, &process);
            BAIL_ON_VMDIR_ERROR(dwError);

            /*Wait until process fails/stops*/
            dwError = _VmDirMonitorProcess(pid, dwGroupId);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirProcessTableRead(dwGroupId, &pProcessRead);
            BAIL_ON_VMDIR_ERROR(dwError);

            if (pProcessRead->dwState == VMDIR_STATE_DEAD)
            {
                VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Retry: %d. Restarting postd process", dwRetryCount);
                VMDIR_SAFE_FREE_MEMORY(pProcessRead);
                continue;
            }
            else
            {
                goto cleanup;
            }
        }
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pProcessRead);
    VMDIR_SAFE_FREE_MEMORY(pArg);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: Error: %d", __FUNCTION__, dwError);
    goto cleanup;
}

static
DWORD
_VmDirExecProgram(
    DWORD   dwGroupId
    )
{
    DWORD       dwError = 0;
    sigset_t    set;

    sigemptyset(&set);

    /* Put ourselves in our own process group to dissociate from
       the parent's controlling terminal, if any */
    dwError = setpgid(getpid(), getpid());
    BAIL_ON_VMDIR_ERROR(dwError);

    /* Reset the signal mask */
    dwError = pthread_sigmask(SIG_SETMASK, &set, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (execve(gPostMgrGlobals.pszPostdPath, gPostMgrGlobals.ppszPostdArgs, NULL) < 0)
    {
        VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL, "errno=%d, path=%s", errno, gPostMgrGlobals.pszPostdPath);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_GENERIC);
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: Error: %d", __FUNCTION__, dwError);
    goto cleanup;
}

static
DWORD
_VmDirMonitorProcess(
    pid_t   pid,
    DWORD   dwGroupId
    )
{
    DWORD           dwError = 0;
    PVMDIR_PROCESS  pProcess = NULL;
    pid_t           pid2 = -1;

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Monitoring process %d", pid);

    /*Wait for process to fail or stop*/
    while (pid2 != pid)
    {
        pid2 = waitpid(pid, NULL, 0);
        if (pid2 < 0)
        {
            VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL, "waitpid returned errno=%d", errno);
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_GENERIC);
        }
    }

    /*Process has stopped running. Check if it was stopped by us or if it crashed*/
    dwError = VmDirProcessTableRead(dwGroupId, &pProcess);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pProcess->dwState == VMDIR_STATE_STOPPING)
    {
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Group %d process stopped", dwGroupId);
        pProcess->dwState = VMDIR_STATE_STOPPED;
    }
    else
    {
        VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL, "Group %d process failed", dwGroupId);
        pProcess->dwState = VMDIR_STATE_DEAD;
    }

    /*Update the state. The calling function will decide what to do based on the state.*/
    dwError = VmDirProcessTableUpdate(dwGroupId, pProcess);
    BAIL_ON_VMDIR_ERROR(dwError);


cleanup:
    VMDIR_SAFE_FREE_MEMORY(pProcess);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: Error: %d", __FUNCTION__, dwError);
    goto cleanup;
}
