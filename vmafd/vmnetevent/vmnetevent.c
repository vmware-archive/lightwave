/*
 * Copyright © 2012-2017 VMware, Inc.  All Rights Reserved.
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
VOID
VmNetEventFreeEventHandle(
    PVMNETEVENT_HANDLE pEventHandle
    );

static
VOID
VmNetEventReleaseEventHandle(
    PVMNETEVENT_HANDLE pEventHandle
    );

DWORD
VmNetEventRegister(
    VMNET_EVENT_TYPE vmEventType,
    PFN_VMNETEVENT_CALLBACK pfnCallBack,
    PVMNETEVENT_HANDLE* ppEventHandle
    )
{
    DWORD dwError = 0;

    PVMNETEVENT_HANDLE pEventHandle = NULL;

    if (!ppEventHandle)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMNETEVENT_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(
                          sizeof(VMNETEVENT_HANDLE),
                          (PVOID*)&pEventHandle
                          );
    BAIL_ON_VMNETEVENT_ERROR(dwError);


    dwError = VmNetEventOpenConnection(
                          vmEventType,
                          &pEventHandle->fd
                          );
    BAIL_ON_VMNETEVENT_ERROR(dwError);

    dwError = VmNetEventWaitOnEvent(
                          pEventHandle->fd,
                          pfnCallBack,
                          &pEventHandle->eventThread
                          );
    BAIL_ON_VMNETEVENT_ERROR(dwError);

    pEventHandle->nRefCount = 1;
    pEventHandle->peventThread = &pEventHandle->eventThread;

    *ppEventHandle = pEventHandle;

cleanup:

    return dwError;
error:

    if (ppEventHandle)
    {
        *ppEventHandle = NULL;
    }
    if (pEventHandle)
    {
        VmNetEventReleaseEventHandle(pEventHandle);
    }
    goto cleanup;
}

VOID
VmNetEventUnregister(
    PVMNETEVENT_HANDLE pEventHandle
    )
{
    if (pEventHandle)
    {
        VmNetEventReleaseEventHandle(pEventHandle);
    }
}

static
VOID
VmNetEventFreeEventHandle(
    PVMNETEVENT_HANDLE pEventHandle
    )
{
    if (pEventHandle)
    {
        VmNetEventCloseConnection(pEventHandle->fd);
        //Join thread
    }
    VMAFD_SAFE_FREE_MEMORY(pEventHandle);
}

static
VOID
VmNetEventReleaseEventHandle(
    PVMNETEVENT_HANDLE pEventHandle
    )
{
    if (pEventHandle)
    {
        if (InterlockedDecrement(&pEventHandle->nRefCount) == 0)
        {
            VmNetEventFreeEventHandle(pEventHandle);
        }
    }
}

