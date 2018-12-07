/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 */

#include "includes.h"

static
DWORD
_VmDirNotifyThread(
    void *pContext
    );

DWORD
VmDirNotifyThreadInit(
    PVDIR_WATCH_SESSION_MANAGER pWatchSessionManager
    )
{
    DWORD               dwError = 0;
    PVDIR_THREAD_INFO   pThreadInfo = NULL;

    if (!pWatchSessionManager)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirSrvThrInit(
            &pThreadInfo, NULL, NULL, TRUE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateThread(
            &pThreadInfo->tid,
            pThreadInfo->bJoinThr,
            _VmDirNotifyThread,
            (PVOID)pWatchSessionManager);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSrvThrAdd(pThreadInfo);

cleanup:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmDirNotifyThread(
    void *pContext
    )
{
    DWORD                       dwError = 0;
    PVDIR_WATCH_SESSION         pWatchSession = NULL;
    PVDIR_WATCH_SESSION_MANAGER pWatchSessionManager = NULL;

    pWatchSessionManager = (PVDIR_WATCH_SESSION_MANAGER)pContext;

    if (!pWatchSessionManager)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    while (VmDirdState() != VMDIRD_STATE_SHUTDOWN)
    {
        dwError = VmDirWatchSessionManagerGetNextSession(pWatchSessionManager, &pWatchSession);
        BAIL_ON_VMDIR_ERROR(dwError);
        if (!pWatchSession)
        {
            continue;
        }

        dwError = VmDirWatchSessionSendEvents(pWatchSession, 1);
        if (dwError == VMDIR_ERROR_WATCH_ENDOFLIST)
        {
            dwError = VmDirWatchSessionManagerAddInactiveSession(
                    pWatchSessionManager, pWatchSession);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (dwError)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "Failed to send events to session with ID (%d), error (%d)",
                    pWatchSession->watchSessionId, dwError);

            dwError = VmDirWatchSessionManagerAddInactiveSession(
                    pWatchSessionManager, pWatchSession);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirWatchSessionManagerDeleteSession(
                    pWatchSessionManager, pWatchSession->watchSessionId);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            dwError = VmDirWatchSessionManagerAddActiveSession(
                    pWatchSessionManager, pWatchSession);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}
