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
VmDirSimpleNormDNToEntry(
    PCSTR           pszNormDN,
    PVDIR_ENTRY*    ppEntry
    )
{
    DWORD                   dwError = 0;
    PVDIR_ENTRY             pEntry = NULL;
    PVDIR_BACKEND_INTERFACE pBE = NULL;
    PVDIR_SCHEMA_CTX        pSchemaCtx = NULL;

    if (IsNullOrEmptyString(pszNormDN) || !ppEntry)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pBE = VmDirBackendSelect(NULL);
    assert(pBE);

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert(pSchemaCtx);

    dwError = VmDirAllocateMemory(
            sizeof(*pEntry),
            (PVOID*)&pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = pBE->pfnBESimpleDnToEntry((PSTR)pszNormDN, pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppEntry = pEntry;

cleanup:
    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }

    return dwError;

error:

    if (pEntry)
    {
        VmDirFreeEntry(pEntry);
    }

    goto cleanup;
}

DWORD
VmDirSimpleDNToEntry(
    PCSTR           pszDN,
    PVDIR_ENTRY*    ppEntry
    )
{
    DWORD                   dwError = 0;
    PVDIR_ENTRY             pEntry = NULL;
    PVDIR_BACKEND_INTERFACE pBE = NULL;
    VDIR_BERVALUE           bvDn = VDIR_BERVALUE_INIT;
    PVDIR_SCHEMA_CTX        pSchemaCtx = NULL;

    if (IsNullOrEmptyString(pszDN) || !ppEntry)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pBE = VmDirBackendSelect(NULL);
    assert(pBE);

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert(pSchemaCtx);

    bvDn.lberbv.bv_val = (PSTR)pszDN;
    bvDn.lberbv.bv_len = VmDirStringLenA(bvDn.lberbv.bv_val);

    dwError = VmDirNormalizeDN(&bvDn, pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
            sizeof(*pEntry),
            (PVOID*)&pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = pBE->pfnBESimpleDnToEntry(BERVAL_NORM_VAL(bvDn), pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppEntry = pEntry;

cleanup:
    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }

    VmDirFreeBervalContent(&bvDn);

    return dwError;

error:

    if (pEntry)
    {
        VmDirFreeEntry(pEntry);
    }

    goto cleanup;
}

// Check whether a given group has the specified member as 'pszTargetDN'
// TODO: Wei will remove it if it is obsolete code in the future.
DWORD
VmDirIsGroupHasThisMember(
    PVDIR_OPERATION    pOperation, /* Optional */
    PSTR               pszGroupDN,
    PSTR               pszTargetDN,
    PVDIR_ENTRY        pGroupEntry, /* Optional */
    PBOOLEAN           pbIsGroupMember
    )
{
    DWORD               dwError = ERROR_SUCCESS;
    PVDIR_ENTRY         pGrpEntry = NULL;
    VDIR_BERVALUE       bvGrpDn = VDIR_BERVALUE_INIT;
    VDIR_BERVALUE       bvTarDn = VDIR_BERVALUE_INIT;
    PVDIR_ATTRIBUTE     pAttrMembers = NULL;
    VDIR_BERVALUE       bvMemberDn = VDIR_BERVALUE_INIT;
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;
    BOOLEAN             bIsGroupMember = FALSE;

    if (IsNullOrEmptyString(pszGroupDN) || IsNullOrEmptyString(pszTargetDN))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!pGroupEntry)
    {
        dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
        BAIL_ON_VMDIR_ERROR(dwError);
        assert(pSchemaCtx);

        bvGrpDn.lberbv.bv_val = (PSTR)pszGroupDN;
        bvGrpDn.lberbv.bv_len = VmDirStringLenA(bvGrpDn.lberbv.bv_val);

        dwError = VmDirNormalizeDN(&bvGrpDn, pSchemaCtx);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirSimpleNormDNToEntry(BERVAL_NORM_VAL(bvGrpDn), &pGrpEntry);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        pGrpEntry = pGroupEntry;
    }

    pAttrMembers = VmDirEntryFindAttribute(ATTR_MEMBER, pGrpEntry);
    if (pAttrMembers)
    {
        unsigned int i = 0;
        PSTR pszNormTarDN = NULL;

        bvTarDn.lberbv.bv_val = (PSTR)pszTargetDN;
        bvTarDn.lberbv.bv_len = VmDirStringLenA(bvTarDn.lberbv.bv_val);

        dwError = VmDirNormalizeDN(&bvTarDn, pSchemaCtx);
        BAIL_ON_VMDIR_ERROR(dwError);

        pszNormTarDN = BERVAL_NORM_VAL(bvTarDn);

        for (;i<pAttrMembers->numVals;i++)
        {
            VmDirFreeBervalContent(&bvMemberDn);

            bvMemberDn.lberbv.bv_val = (PSTR)pAttrMembers->vals[i].lberbv.bv_val;
            bvMemberDn.lberbv.bv_len = VmDirStringLenA(bvMemberDn.lberbv.bv_val);

            dwError = VmDirNormalizeDN(&bvMemberDn, pSchemaCtx);
            BAIL_ON_VMDIR_ERROR(dwError);

            if (VmDirStringCompareA(pszNormTarDN,
                                    BERVAL_NORM_VAL(bvMemberDn),
                                    TRUE) == TRUE)
            {
                bIsGroupMember = TRUE;
                break;
            }
        }
    }

    *pbIsGroupMember = bIsGroupMember;

cleanup:
    if (pGrpEntry != pGroupEntry && pGrpEntry)
    {
        VmDirFreeEntry(pGrpEntry);
    }
    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }

    VmDirFreeBervalContent(&bvGrpDn);
    VmDirFreeBervalContent(&bvTarDn);
    VmDirFreeBervalContent(&bvMemberDn);


    return dwError;

error:
    *pbIsGroupMember = FALSE;

    goto cleanup;
}
