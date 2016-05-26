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
VmDirSyncRIDSeqToDB(
    PCSTR   pszDomainDN,
    DWORD   dwRID
    )
{
    DWORD               dwError = 0;
    VDIR_OPERATION      domainOp = {0};
    PVDIR_MODIFICATION  pMod = NULL;

    if (pszDomainDN == NULL)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirInitStackOperation( &domainOp,
                                       VDIR_OPERATION_TYPE_INTERNAL,
                                       LDAP_REQ_MODIFY,
                                       NULL );
    BAIL_ON_VMDIR_ERROR(dwError);

    domainOp.pBEIF = VmDirBackendSelect(domainOp.reqDn.lberbv.bv_val);
    assert(domainOp.pBEIF);

    dwError = VmDirAllocateMemory(sizeof(*pMod), (PVOID*)&domainOp.request.modifyReq.mods);
    BAIL_ON_VMDIR_ERROR(dwError);

    pMod = domainOp.request.modifyReq.mods;

    // Prepare RidSeq attribute
    dwError = VmDirAttributeInitialize(
                VDIR_ATTRIBUTE_SEQUENCE_RID,
                1,
                domainOp.pSchemaCtx,
                &pMod->attr);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                &pMod->attr.vals[0].lberbv.bv_val,
                "%d",
                dwRID);
    BAIL_ON_VMDIR_ERROR(dwError);

    pMod->attr.vals[0].bOwnBvVal = TRUE;
    pMod->attr.vals[0].lberbv.bv_len = VmDirStringLenA(pMod->attr.vals[0].lberbv.bv_val);
    pMod->next = NULL;
    pMod->operation = MOD_OP_REPLACE;

    // Prepare pOrgConfigOp->request.modifyReq
    domainOp.request.modifyReq.dn.lberbv.bv_val = (PSTR)pszDomainDN;
    domainOp.request.modifyReq.dn.lberbv.bv_len = VmDirStringLenA(pszDomainDN);
    domainOp.request.modifyReq.numMods = 1;

    // Prepare Operation for the current domain object
    domainOp.reqDn.lberbv.bv_val = (PSTR)pszDomainDN;
    domainOp.reqDn.lberbv.bv_len = VmDirStringLenA(pszDomainDN);

    dwError = VmDirInternalModifyEntry(&domainOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "DB sync RIDseq (%s)(%d)",
                    VDIR_SAFE_STRING(pszDomainDN),
                    dwRID);

cleanup:

    VmDirFreeOperationContent(&domainOp);

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "DB sync RIDseq failed: (%s)(%d)",
                     VDIR_SAFE_STRING(pszDomainDN),
                     dwRID);

    goto cleanup;
}

