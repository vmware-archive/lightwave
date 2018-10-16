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
VmDirEventInit(
    PVDIR_EVENT*    ppEvent
    )
{
    DWORD       dwError = 0;
    PVDIR_EVENT pEvent = NULL;

    if (!ppEvent)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(sizeof(VDIR_EVENT), (PVOID *)&pEvent);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppEvent = pEvent;

cleanup:
    return dwError;

error:
    VmDirEventFree(pEvent);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

VOID
VmDirEventFree(
    PVDIR_EVENT pEvent
    )
{
    if (pEvent)
    {
        VmDirFreeEntry(pEvent->pCurEntry);
        VmDirFreeEntry(pEvent->pPrevEntry);
        VMDIR_SAFE_FREE_MEMORY(pEvent);
    }
}
