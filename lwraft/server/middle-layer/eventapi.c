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
 * under the License.
 */

#include "includes.h"

DWORD
VmDirMLGetCurrentEvent(
    PVDIR_EVENT*    ppEvent
    )
{
    DWORD   dwError = 0;
    PVDIR_EVENT pEvent = NULL;

    if (!ppEvent)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirEventInit(&pEvent);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirEventRepoAddPendingEventExternal(pEvent, FALSE);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppEvent = pEvent;

cleanup:
    return dwError;

error:
    VmDirEventFree(pEvent);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirMLAddEventData(
    PVDIR_EVENT     pEvent,
    BOOLEAN         bHasTxn,
    PVDIR_ENTRY     pCurEntry,
    VDIR_EVENT_TYPE eventType
    )
{
    DWORD               dwError = 0;
    PVDIR_ENTRY         pPrevEntry = NULL;
    PVDIR_EVENT_DATA    pEventData = NULL;

    if (!pEvent || !pCurEntry)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(sizeof(VDIR_EVENT), (PVOID *)&pEventData);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (eventType != VDIR_EVENT_ADD)
    {
        dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID *)&pPrevEntry);
        BAIL_ON_VMDIR_ERROR(dwError);

        pPrevEntry->encodedEntry = pCurEntry->encodedEntry;
        pPrevEntry->encodedEntrySize = pCurEntry->encodedEntrySize;
    }

    pEventData->bIsGroupUpdate = bHasTxn;
    pEventData->pPrevEntry = pPrevEntry;
    pEventData->pCurEntry = pCurEntry;
    pEventData->eventType = eventType;

    dwError = VmDirEventAddEventData(pEvent, pEventData);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pEventData);
    VmDirFreeEntry(pCurEntry);
    VmDirFreeEntry(pPrevEntry);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirMLMarkEventReady(
    PVDIR_EVENT     pEvent,
    BOOLEAN         bHasTxn,
    BOOLEAN         bSuccessful
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;

    if (!pEvent)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirEventDecodeEventData(pEvent);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOCK_MUTEX(bInLock, pEvent->pMutex);

    pEvent->bIsSuccessful = bSuccessful;
    pEvent->bIsEventReady = TRUE;

    VMDIR_UNLOCK_MUTEX(bInLock, pEvent->pMutex);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}
