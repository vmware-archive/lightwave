/*
 * Copyright © 2019 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the �~@~\License�~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an �~@~\AS IS�~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "includes.h"

DWORD
RaftCliStopProcessA(
    DWORD   dwGroupId
    )
{
    DWORD   dwError = 0;

    dwError = VmDirStopPostProcess(dwGroupId);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
RaftCliStartProcessA(
    DWORD   dwGroupId
    )
{
    DWORD   dwError = 0;

    dwError = VmDirStartPostProcess(dwGroupId);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
RaftCliListProcessesA(
    VOID
    )
{
    DWORD   dwError = 0;
    PVMDIR_PROCESS_LIST    pProcessList = NULL;
    DWORD   dwProcessCount = 0;
    DWORD   dwLoopCount = 0;

    dwError = VmDirListPostProcesses(&pProcessList, &dwProcessCount);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwLoopCount = 0; dwLoopCount < dwProcessCount; dwLoopCount++)
    {
        fprintf(stdout, "Group:%d Status:%d\n", pProcessList->dwGroupId, pProcessList->dwState);
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pProcessList);
    return dwError;

error:
    goto cleanup;
}
