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
VmDirAddCondWriteCtrl(
    PVDIR_OPERATION pOp,
    PCSTR           pszCondWriteFilter
    )
{
    DWORD               dwError = 0;
    PSTR                pszLocalFilter = NULL;
    PVDIR_LDAP_CONTROL  pCondWriteCtrl = NULL;

    if (!pOp || !pszCondWriteFilter)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateStringA(pszCondWriteFilter, &pszLocalFilter);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(sizeof(*pCondWriteCtrl), (PVOID*)&pCondWriteCtrl);
    BAIL_ON_VMDIR_ERROR(dwError);

    pCondWriteCtrl->type = LDAP_CONTROL_CONDITIONAL_WRITE;  // same as ldap control in-place memory
    pCondWriteCtrl->criticality = TRUE;

    pCondWriteCtrl->next = pOp->reqControls;
    pOp->reqControls = pCondWriteCtrl;

    pOp->pCondWriteCtrl = pCondWriteCtrl;
    pOp->pCondWriteCtrl->value.condWriteCtrlVal.pszFilter = pszLocalFilter;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszLocalFilter);
    DeleteControls(&pCondWriteCtrl);

    goto cleanup;
}

