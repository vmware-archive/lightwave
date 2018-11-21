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

DWORD
VmDirWatchSessionInit(
    PVDIR_WATCH_SESSION*    ppWatchSession
    )
{
    DWORD               dwError = 0;
    PVDIR_WATCH_SESSION pWatchSession = NULL;

    if (!ppWatchSession)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(sizeof(VDIR_WATCH_SESSION), (PVOID*)pWatchSession);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppWatchSession = pWatchSession;

cleanup:
    return dwError;

error:
    VmDirWatchSessionFree(pWatchSession);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirWatchSessionSendEvents(
    PVDIR_WATCH_SESSION pWatchSession,
    DWORD               eventCount
    )
{
    DWORD dwError = 0;

    if (!pWatchSession)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    // TODO: Call send event function api for grpc
cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

VOID
VmDirWatchSessionFree(
    PVDIR_WATCH_SESSION pWatchSession
    )
{
    if (pWatchSession)
    {
        if (pWatchSession->pRepoCookie)
        {
            VmDirEventRelease(pWatchSession->pRepoCookie);
        }
        VmDirFreeBervalContent(&pWatchSession->subTreeDn);
        VMDIR_SAFE_FREE_MEMORY(pWatchSession->pszFilter);
        VMDIR_SAFE_FREE_MEMORY(pWatchSession);
    }
}
