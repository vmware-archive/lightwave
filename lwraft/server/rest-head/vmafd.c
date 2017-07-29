/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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
VmDirRESTLoadVmAfdAPI(
    PVDIR_VMAFD_API*    ppVmAfdApi
    )
{
    DWORD   dwError = 0;
    PVDIR_VMAFD_API pVmAfdApi = NULL;

    if (!ppVmAfdApi)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(sizeof(VDIR_VMAFD_API), (PVOID*)&pVmAfdApi);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirOpenVmAfdClientLib(&pVmAfdApi->pVmAfdLib);
    BAIL_ON_VMDIR_ERROR(dwError);

    pVmAfdApi->pfnGetDCName =
            (PFN_VMAFD_GET_DC_NAME)VmDirGetLibSym(
                    pVmAfdApi->pVmAfdLib, "VmAfdGetDCNameA");
    dwError = pVmAfdApi->pfnGetDCName ? 0 : VMDIR_ERROR_NOT_FOUND;
    BAIL_ON_VMDIR_ERROR(dwError);

    pVmAfdApi->pfnGetDomainName =
            (PFN_VMAFD_GET_DOMAIN_NAME)VmDirGetLibSym(
                    pVmAfdApi->pVmAfdLib, "VmAfdGetDomainNameA");
    dwError = pVmAfdApi->pfnGetDomainName ? 0 : VMDIR_ERROR_NOT_FOUND;
    BAIL_ON_VMDIR_ERROR(dwError);

    pVmAfdApi->pfnGetMachineAccountInfo =
            (PFN_VMAFD_GET_MACHINE_ACCOUNT_INFO)VmDirGetLibSym(
                    pVmAfdApi->pVmAfdLib, "VmAfdGetMachineAccountInfoA");
    dwError = pVmAfdApi->pfnGetMachineAccountInfo ? 0 : VMDIR_ERROR_NOT_FOUND;
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppVmAfdApi = pVmAfdApi;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    VmDirRESTUnloadVmAfdAPI(pVmAfdApi);
    goto cleanup;
}

VOID
VmDirRESTUnloadVmAfdAPI(
    PVDIR_VMAFD_API pVmAfdApi
    )
{
    if (pVmAfdApi)
    {
        VmDirCloseLibrary(pVmAfdApi->pVmAfdLib);
        VMDIR_SAFE_FREE_MEMORY(pVmAfdApi);
    }
}
