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
VmDirWatchNotifyServerInit(
    VOID
    )
{
    DWORD                       dwError = 0;

    dwError = VmDirEventRepoInit(&gpEventRepo);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirWatchSessionManagerInit(&gpWatchSessionManager, gpEventRepo);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirNotifyThreadInit(gpWatchSessionManager);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSessionManagerThreadInit(gpWatchSessionManager);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VmDirWatchNotifyServerShutdown();
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

VOID
VmDirWatchNotifyServerShutdown(
    VOID
    )
{
    VmDirWatchSessionManagerFree(gpWatchSessionManager);
    VmDirEventRepoFree(gpEventRepo);
}
