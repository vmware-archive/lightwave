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
VmDirWatchSessionManagerInit(
    PVDIR_WATCH_SESSION_MANAGER*    ppWatchSessionManager,
    PVDIR_EVENT_REPO                pEventRepo
    )
{
    DWORD                       dwError = 0;
    PVDIR_WATCH_SESSION_MANAGER    pWatchSessionManager = NULL;

    if (!ppWatchSessionManager || !pEventRepo)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(sizeof(VDIR_WATCH_SESSION_MANAGER), (PVOID*)&pWatchSessionManager);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirQueueInit(&pWatchSessionManager->pActiveQueue);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirQueueInit(&pWatchSessionManager->pInactiveQueue);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(
            &pWatchSessionManager->pDeletedMap,
            LwRtlHashDigestPointer,
            LwRtlHashEqualPointer,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&pWatchSessionManager->pMutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    pWatchSessionManager->pEventRepo = pEventRepo;

    *ppWatchSessionManager = pWatchSessionManager;

cleanup:
    return dwError;

error:
    VmDirWatchSessionManagerFree(pWatchSessionManager);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirWatchSessionManagerAddNewSession(
    PVDIR_WATCH_SESSION_MANAGER pWatchSessionManager,
    BOOL                        bPrevVersion,
    DWORD                       startRevision,
    PSTR                        pszFilter,
    PVOID                       pConnectionHndl,
    VDIR_BERVALUE               subTreeDn,
    DWORD*                      pWatchId
    )
{
    DWORD               dwError = 0;
    BOOL                bInLock = FALSE;
    PVDIR_WATCH_SESSION pWatchSession = NULL;

    if (!pszFilter || !pConnectionHndl || !pWatchSessionManager || !pWatchId)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirWatchSessionInit(&pWatchSession, pWatchSessionManager->pEventRepo);
    BAIL_ON_VMDIR_ERROR(dwError);

    pWatchSession->bPrevVersion = bPrevVersion;
    pWatchSession->startRevision = startRevision;
    pWatchSession->pszFilter = pszFilter;
    pWatchSession->pConnectionHndl = pConnectionHndl;
    pWatchSession->subTreeDn = subTreeDn;

    dwError = VmDirEventRepoGetNextReadyEvent(
            pWatchSessionManager->pEventRepo,
            &pWatchSession->pRepoCookie,
            &pWatchSession->pRepoCookie
            );
    if (dwError == VMDIR_ERROR_WATCH_ENDOFLIST)
    {
        dwError = 0;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOCK_MUTEX(bInLock, pWatchSessionManager->pMutex);

    pWatchSession->watchSessionId = pWatchSessionManager->watchSessionCounter++;

    VMDIR_UNLOCK_MUTEX(bInLock, pWatchSessionManager->pMutex);

    dwError = VmDirWatchSessionManagerAddActiveSession(pWatchSessionManager, pWatchSession);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pWatchId = pWatchSession->watchSessionId;

cleanup:
    return dwError;

error:
    VmDirWatchSessionFree(pWatchSession);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirWatchSessionManagerGetNextSession(
    PVDIR_WATCH_SESSION_MANAGER pWatchSessionManager,
    PVDIR_WATCH_SESSION*        ppWatchSession
    )
{
    DWORD               dwError = 0;
    BOOL                bInLock = FALSE;
    PVDIR_WATCH_SESSION pWatchSession = NULL;

    if (!pWatchSessionManager || !ppWatchSession)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    while (!pWatchSession)
    {
        dwError = VmDirQueueDequeue(
                bInLock, pWatchSessionManager->pActiveQueue, 500, (PVOID*)&pWatchSession);
        if (dwError == VMDIR_ERROR_QUEUE_EMPTY)
        {
            *ppWatchSession = NULL;
            dwError = 0;
            goto cleanup;
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        if (!LwRtlHashMapFindKey(
                    pWatchSessionManager->pDeletedMap,
                    NULL,
                    (PVOID)(intptr_t)pWatchSession->watchSessionId))
        {
            // watch session is deleted get next
            dwError = LwRtlHashMapRemove(
                    pWatchSessionManager->pDeletedMap,
                    (PVOID)(intptr_t)pWatchSession->watchSessionId,
                    NULL);
            BAIL_ON_VMDIR_ERROR(dwError);

            VmDirWatchSessionFree(pWatchSession);
            pWatchSession = NULL;
        }
    }

    *ppWatchSession = pWatchSession;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirWatchSessionManagerAddActiveSession(
    PVDIR_WATCH_SESSION_MANAGER pWatchSessionManager,
    PVDIR_WATCH_SESSION         pWatchSession
    )
{
    DWORD   dwError = 0;
    BOOL    bInLock = FALSE;

    if (!pWatchSessionManager || !pWatchSession)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirQueueEnqueue(bInLock, pWatchSessionManager->pActiveQueue, (PVOID)pWatchSession);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirWatchSessionManagerAddInactiveSession(
    PVDIR_WATCH_SESSION_MANAGER pWatchSessionManager,
    PVDIR_WATCH_SESSION         pWatchSession
    )
{
    DWORD   dwError = 0;
    BOOL    bInLock = FALSE;

    if (!pWatchSessionManager || !pWatchSession)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirQueueEnqueue(bInLock, pWatchSessionManager->pInactiveQueue, (PVOID)pWatchSession);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirWatchSessionManagerActivateSessions(
    PVDIR_WATCH_SESSION_MANAGER pWatchSessionManager
    )
{
    BOOL                bInLock = FALSE;
    DWORD               dwError = 0;
    PVDIR_WATCH_SESSION pWatchSession = NULL;

    if (!pWatchSessionManager)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    while (TRUE)
    {
        dwError = VmDirQueueDequeue(
                bInLock, pWatchSessionManager->pInactiveQueue, 0, (PVOID*)pWatchSession);
        if (dwError == VMDIR_ERROR_QUEUE_EMPTY)
        {
            dwError = 0;
            break;
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirQueueEnqueue(
                bInLock, pWatchSessionManager->pActiveQueue, (PVOID)pWatchSession);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirWatchSessionManagerDeleteSession(
    PVDIR_WATCH_SESSION_MANAGER pWatchSessionManager,
    DWORD                       dwWatchSessionId
    )
{
    DWORD   dwError = 0;

    if (!pWatchSessionManager)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = LwRtlHashMapInsert(
            pWatchSessionManager->pDeletedMap,
            (PVOID)(uintptr_t)dwWatchSessionId,
            (PVOID)(uintptr_t)dwWatchSessionId,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

VOID
VmDirWatchSessionManagerFree(
    PVDIR_WATCH_SESSION_MANAGER pWatchSessionManager
    )
{
    DWORD               dwError = 0;
    BOOL                bInLock = FALSE;
    PVDIR_WATCH_SESSION pWatchSession = NULL;

    if (pWatchSessionManager)
    {
        while (!dwError)
        {
            dwError = VmDirQueueDequeue(
                    bInLock, pWatchSessionManager->pActiveQueue, 0, (PVOID*)&pWatchSession);
            VmDirWatchSessionFree(pWatchSession);
        }
        dwError = 0;
        VmDirQueueFree(pWatchSessionManager->pActiveQueue);

        while (!dwError)
        {
            dwError = VmDirQueueDequeue(
                    bInLock, pWatchSessionManager->pInactiveQueue, 0, (PVOID*)&pWatchSession);
            VmDirWatchSessionFree(pWatchSession);
        }
        VmDirQueueFree(pWatchSessionManager->pInactiveQueue);

        LwRtlHashMapClear(pWatchSessionManager->pDeletedMap, VmDirSimpleHashMapPairFree, NULL);
        LwRtlFreeHashMap(&pWatchSessionManager->pDeletedMap);

        VmDirFreeMutex(pWatchSessionManager->pMutex);
        VMDIR_SAFE_FREE_MEMORY(pWatchSessionManager);
    }
}
