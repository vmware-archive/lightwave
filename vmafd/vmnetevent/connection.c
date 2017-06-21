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

DWORD
VmNetEventOpenConnection(
    VMNET_EVENT_TYPE eventType,
    PVMNETEVENT_FD pEventFD
    )
{
    DWORD dwError = 0;

    if (!pEventFD)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMNETEVENT_ERROR(dwError);
    }

    dwError = gVmNetEventPackage.pfnOpenConnection(eventType, pEventFD);
    BAIL_ON_VMNETEVENT_ERROR(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmNetEventWaitOnEvent(
    VMNETEVENT_FD EventFd,
    PFN_VMNETEVENT_CALLBACK pfnCallBack,
    pthread_t* pEventThread
    )
{
  DWORD dwError = 0;

  if (!pEventThread)
  {
      dwError = ERROR_INVALID_PARAMETER;
      BAIL_ON_VMNETEVENT_ERROR(dwError);
  }

  dwError = gVmNetEventPackage.pfnWaitEvent(EventFd, pfnCallBack, pEventThread);
  BAIL_ON_VMNETEVENT_ERROR(dwError);

cleanup:

  return dwError;
error:

  goto cleanup;
}

VOID
VmNetEventCloseConnection(
    VMNETEVENT_FD EventFd
    )
{
        gVmNetEventPackage.pfnCloseConnection(EventFd);
}
