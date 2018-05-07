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

static
DWORD
_VmDirBackupDBRInternal(
    PVMDIR_SERVER_CONTEXT   hServer,
    PCSTR                   pszBackupPath
    );

/*
 * API to backup database at the server side
 */
DWORD
VmDirBackupDB(
    PVMDIR_SERVER_CONTEXT hServer,
    PCSTR       pszBackupPath
    )
{
    DWORD   dwError = 0;

    if (!hServer || !hServer->hBinding ||
        IsNullOrEmptyString(pszBackupPath))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = _VmDirBackupDBRInternal(hServer, pszBackupPath);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "%s failed. Error[%d]\n", __FUNCTION__, dwError);
    goto cleanup;
}

/*
 * RPC call to server to backup database
 */
static
DWORD
_VmDirBackupDBRInternal(
    PVMDIR_SERVER_CONTEXT   hServer,
    PCSTR                   pszBackupPath
    )
{
    DWORD   dwError = 0;
    PWSTR   pwszBackupPath = NULL;

    dwError = VmDirAllocateStringWFromA(pszBackupPath, &pwszBackupPath);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_RPC_TRY
    {
        dwError = RpcVmDirBackupDB(
            hServer->hBinding,
            pwszBackupPath);
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pwszBackupPath);
    return dwError;

error:
    goto cleanup;
}
