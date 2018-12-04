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
    PVDIR_WATCH_SESSION*    ppWatchSession,
    PVDIR_EVENT_REPO        pEventRepo
    )
{
    DWORD               dwError = 0;
    PVDIR_WATCH_SESSION pWatchSession = NULL;

    if (!ppWatchSession || !pEventRepo)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(sizeof(VDIR_WATCH_SESSION), (PVOID*)pWatchSession);
    BAIL_ON_VMDIR_ERROR(dwError);

    pWatchSession->pEventRepo = pEventRepo;

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
    PVDIR_WATCH_SESSION         pWatchSession,
    DWORD                       eventCount
    )
{
    DWORD               dwError = 0;
    DWORD               dwListError = 0;
    DWORD               dwSentCount = 0;
    PVDIR_EVENT         pEvent = NULL;
    PVDIR_OPERATION     pOperation = NULL;
    PVDIR_LINKED_LIST   pSendEventList = NULL;

    if (!pWatchSession)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirLinkedListCreate(&pSendEventList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirExternalOperationCreate(NULL, -1, LDAP_REQ_SEARCH, NULL, &pOperation);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (dwSentCount < eventCount)
    {
        dwError = VmDirEventRepoGetNextReadyEvent(
                pWatchSession->pEventRepo, &pWatchSession->pRepoCookie, &pWatchSession->pRepoCookie);
        if (dwError == VMDIR_ERROR_WATCH_ENDOFLIST)
        {
            dwListError = dwError;
            dwError = 0;
            break;
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirMatchEntryWithFilter(
                pOperation, pEvent->pEventData->pCurEntry, pWatchSession->pszFilter);
        if (dwError == VMDIR_LDAP_ERROR_PRE_CONDITION)
        {
            dwError = 0;
            continue;
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLinkedListInsertTail(
                pSendEventList, (PVOID)pWatchSession->pRepoCookie->pEventData, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwSentCount++;
    }

    // TODO: Call send event function api for grpc

    BAIL_ON_VMDIR_ERROR(dwListError);

cleanup:
    VmDirFreeOperation(pOperation);
    VmDirFreeLinkedList(pSendEventList);
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
