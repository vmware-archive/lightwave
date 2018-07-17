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
VmDirInitMaxCommittedUSN(
    PVDIR_BACKEND_INTERFACE   pBE
    )
{
    USN       tmpUSN = 0;
    DWORD     dwError = 0;
    BOOLEAN   bInLock = FALSE;
    VDIR_BACKEND_CTX    beCtx = {0};

    if (!pBE)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    beCtx.pBE = pBE;

    VMDIR_LOCK_MUTEX(bInLock, gVmDirServerOpsGlobals.pMutex);

    dwError = pBE->pfnBEGetNextUSN(&beCtx, &tmpUSN);
    BAIL_ON_VMDIR_ERROR(dwError);

    gVmDirServerOpsGlobals.maxCommittedUSN = tmpUSN;

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmDirServerOpsGlobals.pMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: failed (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

VOID
VmDirUpdateMaxCommittedUSNInLock(
    USN   committedUSN
    )
{
    if (committedUSN > gVmDirServerOpsGlobals.maxCommittedUSN)
    {
        gVmDirServerOpsGlobals.maxCommittedUSN = committedUSN;
    }
}

USN
VmDirGetMaxCommittedUSN(
    VOID
    )
{
    USN   maxCommittedUSN = 0;
    BOOLEAN   bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmDirServerOpsGlobals.pMutex);

    maxCommittedUSN = gVmDirServerOpsGlobals.maxCommittedUSN;

    VMDIR_UNLOCK_MUTEX(bInLock, gVmDirServerOpsGlobals.pMutex);

    return maxCommittedUSN;
}
