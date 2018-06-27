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
VmDirExternalEntryAttributeReplace(
    PVDIR_CONNECTION    pConn,
    PCSTR               pszEntryDn,
    PCSTR               pszAttrName,
    PVDIR_BERVALUE      pBervAttrValue
    )
{
    DWORD dwError = 0;
    PVDIR_OPERATION pModifyOp = NULL;
    PVDIR_MODIFICATION pMod = NULL;

    if (!pConn ||
        IsNullOrEmptyString(pszEntryDn) ||
        IsNullOrEmptyString(pszAttrName) ||
        !pBervAttrValue)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirExternalOperationCreate(
            NULL, -1, LDAP_REQ_MODIFY, pConn, &pModifyOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    pModifyOp->protocol = VDIR_OPERATION_PROTOCOL_REST;

    pModifyOp->reqDn.lberbv.bv_val = (PSTR)pszEntryDn;
    pModifyOp->reqDn.lberbv.bv_len = VmDirStringLenA(pszEntryDn);

    dwError = VmDirAllocateMemory(sizeof(*pMod), (PVOID)&pMod);
    BAIL_ON_VMDIR_ERROR(dwError);

    pMod->operation = MOD_OP_REPLACE;
    pMod->next = NULL;

    dwError = VmDirModAddSingleValueAttribute(
        pMod,
        pModifyOp->pSchemaCtx,
        pszAttrName,
        pBervAttrValue->lberbv.bv_val,
        pBervAttrValue->lberbv.bv_len);
    BAIL_ON_VMDIR_ERROR(dwError);

    pModifyOp->request.modifyReq.dn.lberbv.bv_val = (PSTR)pszEntryDn;
    pModifyOp->request.modifyReq.dn.lberbv.bv_len = VmDirStringLenA(pszEntryDn);
    pModifyOp->request.modifyReq.mods = pMod;
    pMod = NULL;
    pModifyOp->request.modifyReq.numMods = 1;

    dwError = VmDirMLModify(pModifyOp);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_VDIR_MODIFICTION(pMod);
    VmDirFreeOperation(pModifyOp);
    return dwError;

error:
    goto cleanup;
}
