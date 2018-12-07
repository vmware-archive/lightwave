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
VmDirWatchSessionSendOneEvent(
    PVDIR_WATCH_SESSION pWatchSession,
    BOOL*               pbEventSent
    )
{
    DWORD                   dwError = 0;
    PVDIR_EVENT             pEvent = NULL;
    PVDIR_OPERATION         pOperation = NULL;
    PVDIR_EVENT_DATA_NODE   pTemp = NULL;
    PVDIR_EVENT_DATA_NODE   pPrev = NULL;
    PVDIR_EVENT_DATA_NODE   pHead = NULL;
    PVDIR_EVENT_DATA_NODE   pIter = NULL;

    if (!pWatchSession || !pbEventSent)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    *pbEventSent = FALSE;

    dwError = VmDirEventRepoGetNextReadyEvent(
            pWatchSession->pEventRepo,
            &pWatchSession->pRepoCookie,
            &pWatchSession->pRepoCookie);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirExternalOperationCreate(NULL, -1, LDAP_REQ_SEARCH, NULL, &pOperation);
    BAIL_ON_VMDIR_ERROR(dwError);

    pEvent = pWatchSession->pRepoCookie;

    if (pEvent->revision < pWatchSession->startRevision)
    {
        goto cleanup;
    }

    pIter = pEvent->pEventDataHead;

    while (pIter)
    {
        dwError = VmDirMatchEntryWithFilter(
                pOperation, pIter->pEventData->pCurEntry, pWatchSession->pszFilter);
        if (dwError == VMDIR_LDAP_ERROR_PRE_CONDITION)
        {
            dwError = 0;
            pIter = pIter->pNext;
            continue;
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateMemory(sizeof(VDIR_EVENT_DATA_NODE), (PVOID *)&pTemp);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pPrev)
        {
            pPrev->pNext = pTemp;
        }

        if (!pHead)
        {
            pHead = pTemp;
        }

        pPrev = pTemp;
        pTemp = NULL;
        pIter = pIter->pNext;
    }

    if (pHead) // List not empty
    {
        // TODO: Call the send API for grpc send pEvent->pEventDataList
        *pbEventSent = TRUE;
    }

cleanup:
    pTemp = pHead;
    while(pTemp)
    {
        VMDIR_SAFE_FREE_MEMORY(pTemp);
        pTemp = pHead->pNext;
        pHead = pTemp;
    }
    VmDirFreeOperation(pOperation);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirWatchSessionSendEvents(
    PVDIR_WATCH_SESSION pWatchSession,
    DWORD               responseCount
    )
{
    DWORD   dwError = 0;
    DWORD   dwSentCount = 0;
    BOOL    bEventSent = FALSE;

    if (!pWatchSession)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    while (dwSentCount < responseCount)
    {
        dwError = VmDirWatchSessionSendOneEvent(pWatchSession, &bEventSent);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (bEventSent)
        {
            dwSentCount++;
        }
    }

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
    DWORD   dwError = 0;

    if (pWatchSession)
    {
        while (VmDirEventRepoIsAtHead(pWatchSession->pEventRepo, pWatchSession->pRepoCookie))
        {
            dwError = VmDirEventRepoGetNextReadyEvent(
                    pWatchSession->pEventRepo,
                    &pWatchSession->pRepoCookie,
                    &pWatchSession->pRepoCookie);
            if (dwError)
            {
                break;
            }
        }
        VmDirEventRelease(pWatchSession->pRepoCookie);

        VmDirFreeBervalContent(&pWatchSession->subTreeDn);
        VMDIR_SAFE_FREE_MEMORY(pWatchSession->pszFilter);
        VMDIR_SAFE_FREE_MEMORY(pWatchSession);
    }
}
