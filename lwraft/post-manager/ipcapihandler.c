/*
 * Copyright © 2019 VMware, Inc.  All Rights Reserved.
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
VmDirLocalAPIHandler(
    PVM_DIR_SECURITY_CONTEXT    pSecurityContext,
    PBYTE                       pRequest,
    DWORD                       dwRequestSize,
    PBYTE                       *ppResponse,
    DWORD                       *pdwResponseSize
    )
{
    DWORD   dwError = 0;
    UINT32  uApiType = 0;
    PBYTE   pResponse = NULL;
    DWORD   dwResponseSize = 0;

    if (dwRequestSize < sizeof(UINT32))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!pSecurityContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!VmDirIsRootSecurityContext(pSecurityContext))
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                         "%s: Access Denied",
                         __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    uApiType = *((PUINT32)pRequest);

    switch(uApiType)
    {
      case VMDIR_IPC_PROCESS_START:

        dwError = VmDirIpcServerStart(
                        pRequest,
                        dwRequestSize,
                        &pResponse,
                        &dwResponseSize
                        );
        break;

      case VMDIR_IPC_PROCESS_STOP:

        dwError = VmDirIpcServerStop(
                        pRequest,
                        dwRequestSize,
                        &pResponse,
                        &dwResponseSize
                        );
        break;

      case VMDIR_IPC_PROCESS_LIST:

        dwError = VmDirIpcGetProcessList(
                        pRequest,
                        dwRequestSize,
                        &pResponse,
                        &dwResponseSize
                        );
        break;

      default:
        dwError = ERROR_INVALID_PARAMETER;
        break;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

cleanup:
    return dwError;

error:
    if (ppResponse)
    {
        *ppResponse = NULL;
    }
    if (pdwResponseSize)
    {
        *pdwResponseSize = 0;
    }
    if (pResponse)
    {
        VMDIR_SAFE_FREE_MEMORY(pResponse);
    }
    goto cleanup;
}
