/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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
VmDirSchemaModMutexAcquire(
    PVDIR_OPERATION pOperation
    )
{
    DWORD   dwError = 0;
    PSTR    pszDN = NULL;

    if (!pOperation)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pOperation->reqCode == LDAP_REQ_ADD)
    {
        pszDN = BERVAL_NORM_VAL(pOperation->request.addReq.pEntry->dn);
    }
    else if (pOperation->reqCode == LDAP_REQ_MODIFY)
    {
        pszDN = BERVAL_NORM_VAL(pOperation->request.modifyReq.dn);
    }

    if (VmDirStringEndsWith(pszDN, SCHEMA_NAMING_CONTEXT_DN, FALSE) &&
            VmDirStringLenA(pszDN) > (SCHEMA_NAMING_CONTEXT_DN_LEN))
    {
        dwError = VmDirLockMutex(gVdirSchemaGlobals.cacheModMutex);
        BAIL_ON_VMDIR_ERROR(dwError);

        pOperation->bSchemaWriteOp = TRUE;
    }

error:
    return dwError;
}

DWORD
VmDirSchemaModMutexRelease(
    PVDIR_OPERATION pOperation
    )
{
    DWORD   dwError = 0;

    if (!pOperation)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pOperation->bSchemaWriteOp)
    {
        dwError = VmDirUnLockMutex(gVdirSchemaGlobals.cacheModMutex);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:
    return dwError;
}
