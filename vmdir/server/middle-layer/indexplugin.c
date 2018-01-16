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

/*
 * Before new entry is persisted, make sure the entry has searchFlags
 * attribute if it has uniquenessScope attribute.
 */
DWORD
VmDirPluginIndexEntryPreAdd(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult
    )
{
    DWORD   dwError= 0;
    PVDIR_ATTRIBUTE pUSAttr = NULL;
    PVDIR_ATTRIBUTE pSFAttr = NULL;
    PSTR    pszSearchFlags = NULL;
    int     iSearchFlags = 0;

    if (!pOperation->dwSchemaWriteOp)
    {
        goto cleanup;
    }

    // if uniquenessScope exists, make sure that searchFlags & 1 is true
    pUSAttr = VmDirFindAttrByName(pEntry, ATTR_UNIQUENESS_SCOPE);
    if (pUSAttr)
    {
        pSFAttr = VmDirFindAttrByName(pEntry, ATTR_SEARCH_FLAGS);
        if (pSFAttr)
        {
            pszSearchFlags = BERVAL_NORM_VAL(pSFAttr->vals[0]);
            iSearchFlags = VmDirStringToIA(pszSearchFlags);

            if (!(1 & iSearchFlags))
            {
                iSearchFlags |= 1;

                VmDirFreeBervalContent(&pSFAttr->vals[0]);

                dwError = VmDirAllocateStringPrintf(
                        &pSFAttr->vals[0].lberbv.bv_val, "%d", iSearchFlags);
                BAIL_ON_VMDIR_ERROR(dwError);

                pSFAttr->vals[0].lberbv.bv_len =
                        VmDirStringLenA(pSFAttr->vals[0].lberbv.bv_val);
                pSFAttr->vals[0].bOwnBvVal = TRUE;
            }
        }
        else
        {
            iSearchFlags = 1;

            dwError = VmDirEntryAddSingleValueStrAttribute(
                    pEntry, ATTR_SEARCH_FLAGS, "1");
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

/*
 * After new entry is persisted, schedule a new index to be built by
 * background indexing thread.
 */
DWORD
VmDirPluginIndexEntryPostAdd(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult
    )
{
    DWORD   dwError= 0;
    DWORD   i = 0;
    PVDIR_ATTRIBUTE pSFAttr = NULL;
    PVDIR_ATTRIBUTE pCNAttr = NULL;
    PVDIR_ATTRIBUTE pSXAttr = NULL;
    PVDIR_ATTRIBUTE pUSAttr = NULL;
    PSTR    pszSearchFlags = NULL;
    int     iSearchFlags = 0;
    PVMDIR_STRING_LIST  pScopes = NULL;
    PVDIR_INDEX_UPD     pIndexUpd = NULL;

    if (!pOperation->dwSchemaWriteOp)
    {
        goto cleanup;
    }

    pSFAttr = VmDirFindAttrByName(pEntry, ATTR_SEARCH_FLAGS);
    if (pSFAttr)
    {
        pszSearchFlags = BERVAL_NORM_VAL(pSFAttr->vals[0]);
        iSearchFlags = VmDirStringToIA(pszSearchFlags);
        pCNAttr = VmDirFindAttrByName(pEntry, ATTR_CN);

        dwError = VmDirIndexUpdateBegin(NULL, &pIndexUpd);
        BAIL_ON_VMDIR_ERROR(dwError);

        // if searchFlags & 1, schedule index
        if (1 & iSearchFlags)
        {
            pSXAttr = VmDirFindAttrByName(pEntry, ATTR_ATTRIBUTE_SYNTAX);
            dwError = VmDirIndexSchedule(
                    pIndexUpd,
                    pCNAttr->vals[0].lberbv.bv_val,
                    pSXAttr->vals[0].lberbv.bv_val);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        // add uniqueness scopes
        pUSAttr = VmDirFindAttrByName(pEntry, ATTR_UNIQUENESS_SCOPE);
        if (pUSAttr && pUSAttr->numVals > 0)
        {
            dwError = VmDirStringListInitialize(
                    &pScopes, pUSAttr->numVals + 1);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (i = 0; i < pUSAttr->numVals; i++)
            {
                dwError = VmDirStringListAddStrClone(
                        pUSAttr->vals[i].lberbv.bv_val, pScopes);
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            dwError = VmDirIndexAddUniquenessScope(
                    pIndexUpd,
                    pCNAttr->vals[0].lberbv.bv_val,
                    pScopes->pStringList);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        dwError = VmDirIndexUpdateCommit(pIndexUpd);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VmDirStringListFree(pScopes);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    (VOID)VmDirIndexUpdateAbort(pIndexUpd);
    goto cleanup;
}

/*
 * Before mods are applied to entry, check and append any missing mod.
 *
 * If mod->type = uniquenessScope and mod->op = add, append a newmod
 * newmod->type = searchFlags
 * newmod->val = 1
 * newmod->op = add
 *
 * If mod->type = searchFlags and mod->op = del, append a newmod
 * newmod->type = uniquenessScope
 * newmod->op = del
 */
DWORD
VmDirPluginIndexEntryPreModApplyModify(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult
    )
{
    DWORD   dwError= 0;
    BOOLEAN bExist = FALSE;
    BOOLEAN bNeed = FALSE;
    BOOLEAN bDelIdx = FALSE;
    BOOLEAN bNewIdx = FALSE;
    BOOLEAN bNewScope = FALSE;
    PSTR    pszDN = NULL;
    PSTR    pszCN = NULL;
    ModifyReq*  pModReq = &pOperation->request.modifyReq;
    PVDIR_MODIFICATION  pMod = NULL;
    PSTR    pszSearchFlags = NULL;
    int     iSearchFlags = 0;

    if (!pOperation->dwSchemaWriteOp)
    {
        goto cleanup;
    }

    // is it already indexed?
    pszDN = BERVAL_NORM_VAL(pOperation->request.modifyReq.dn);

    dwError = VmDirDnLastRDNToCn(pszDN, &pszCN);
    BAIL_ON_VMDIR_ERROR(dwError);

    bExist = VmDirIndexExist(pszCN);

    // analyze mods what they are intended to do
    for (pMod = pModReq->mods; pMod; pMod = pMod->next)
    {
        PSTR pszAttr = BERVAL_NORM_VAL(pMod->attr.type);
        if (VmDirStringCompareA(pszAttr, ATTR_UNIQUENESS_SCOPE, FALSE) == 0)
        {
            if (pMod->operation == MOD_OP_REPLACE && pMod->attr.numVals > 0)
            {
                bNewScope = TRUE;
            }
            else if (pMod->operation == MOD_OP_ADD)
            {
                bNewScope = TRUE;
            }
        }
        else if (VmDirStringCompareA(pszAttr, ATTR_SEARCH_FLAGS, FALSE) == 0)
        {
            if (pMod->operation == MOD_OP_DELETE && bExist)
            {
                bNewIdx = FALSE;
                bDelIdx = TRUE;
            }
            else if (pMod->operation == MOD_OP_ADD && !bExist)
            {
                pszSearchFlags = BERVAL_NORM_VAL(pMod->attr.vals[0]);
                iSearchFlags = VmDirStringToIA(pszSearchFlags);
                bNeed = 1 & iSearchFlags;

                if (bNeed)
                {
                    bDelIdx = FALSE;
                    bNewIdx = TRUE;
                }
            }
            else // MOD_OP_REPLACE
            {
                if (pMod->attr.numVals == 0 && bExist)
                {
                    bNewIdx = FALSE;
                    bDelIdx = TRUE;
                }
                else if (pMod->attr.numVals > 0)
                {
                    pszSearchFlags = BERVAL_NORM_VAL(pMod->attr.vals[0]);
                    iSearchFlags = VmDirStringToIA(pszSearchFlags);
                    bNeed = 1 & iSearchFlags;

                    if (!bExist && bNeed)
                    {
                        bDelIdx = FALSE;
                        bNewIdx = TRUE;
                    }
                    else if (bExist && !bNeed)
                    {
                        bNewIdx = FALSE;
                        bDelIdx = TRUE;
                    }
                }
            }
        }
    }

    // append additional mods based on analysis
    if (bDelIdx)
    {
        dwError = VmDirAppendAMod(
                pOperation,
                MOD_OP_DELETE,
                ATTR_UNIQUENESS_SCOPE,
                ATTR_UNIQUENESS_SCOPE_LEN,
                NULL,
                0);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (!bExist && !bNewIdx && bNewScope)
    {
        dwError = VmDirAppendAMod(
                pOperation,
                MOD_OP_ADD,
                ATTR_SEARCH_FLAGS,
                ATTR_SEARCH_FLAGS_LEN,
                "1",
                1);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszCN);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

/*
 * Before modified entry is persisted, schedule/delete index and
 * add/delete uniqueness scopes. So modify will be rejected before
 * persisted if any constraint is violated.
 */
DWORD
VmDirPluginIndexEntryPreModify(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    DWORD            dwPriorResult
    )
{
    DWORD   dwError= 0;
    DWORD   i = 0;
    BOOLEAN bExist = FALSE;
    BOOLEAN bNeed = FALSE;
    PVDIR_ATTRIBUTE pCNAttr = NULL;
    PVDIR_ATTRIBUTE pSFAttr = NULL;
    PVDIR_ATTRIBUTE pSXAttr = NULL;
    ModifyReq*  pModReq = &pOperation->request.modifyReq;
    PVDIR_MODIFICATION  pMod = NULL;
    PSTR    pszSearchFlags = NULL;
    int     iSearchFlags = 0;
    PVMDIR_STRING_LIST  pScopes = NULL;
    PVDIR_INDEX_UPD     pIndexUpd = NULL;

    if (!pOperation->dwSchemaWriteOp)
    {
        goto cleanup;
    }

    // is it already indexed?
    pCNAttr = VmDirFindAttrByName(pEntry, ATTR_CN);
    bExist = VmDirIndexExist(pCNAttr->vals[0].lberbv.bv_val);

    // do we need an index?
    pSFAttr = VmDirFindAttrByName(pEntry, ATTR_SEARCH_FLAGS);
    if (pSFAttr)
    {
        pszSearchFlags = BERVAL_NORM_VAL(pSFAttr->vals[0]);
        iSearchFlags = VmDirStringToIA(pszSearchFlags);
        bNeed = 1 & iSearchFlags;
    }

    dwError = VmDirIndexUpdateBegin(pOperation->pBECtx, &pIndexUpd);
    BAIL_ON_VMDIR_ERROR(dwError);

    // schedule index if needed
    if (!bExist && bNeed)
    {
        pSXAttr = VmDirFindAttrByName(pEntry, ATTR_ATTRIBUTE_SYNTAX);
        dwError = VmDirIndexSchedule(
                pIndexUpd,
                pCNAttr->vals[0].lberbv.bv_val,
                pSXAttr->vals[0].lberbv.bv_val);
        BAIL_ON_VMDIR_ERROR(dwError);
        bExist = TRUE;
    }

    // delete index if no longer needed
    else if (bExist && !bNeed)
    {
        dwError = VmDirIndexDelete(pIndexUpd, pCNAttr->vals[0].lberbv.bv_val);
        BAIL_ON_VMDIR_ERROR(dwError);
        bExist = FALSE;
    }

    // update uniqueness scopes
    if (bExist)
    {
        for (pMod = pModReq->mods; pMod; pMod = pMod->next)
        {
            PSTR pszAttr = BERVAL_NORM_VAL(pMod->attr.type);
            if (VmDirStringCompareA(pszAttr, ATTR_UNIQUENESS_SCOPE, FALSE) == 0)
            {
                if (pMod->attr.numVals > 0)
                {
                    dwError = VmDirStringListInitialize(
                            &pScopes, pMod->attr.numVals + 1);
                    BAIL_ON_VMDIR_ERROR(dwError);

                    for (i = 0; i < pMod->attr.numVals; i++)
                    {
                        dwError = VmDirStringListAddStrClone(
                                pMod->attr.vals[i].lberbv.bv_val, pScopes);
                        BAIL_ON_VMDIR_ERROR(dwError);
                    }

                    if (pMod->operation == MOD_OP_DELETE)
                    {
                        dwError = VmDirIndexDeleteUniquenessScope(
                                pIndexUpd,
                                pCNAttr->vals[0].lberbv.bv_val,
                                pScopes->pStringList);
                        BAIL_ON_VMDIR_ERROR(dwError);
                    }
                    else if (pMod->operation == MOD_OP_ADD)
                    {
                        dwError = VmDirIndexAddUniquenessScope(
                                pIndexUpd,
                                pCNAttr->vals[0].lberbv.bv_val,
                                pScopes->pStringList);
                        BAIL_ON_VMDIR_ERROR(dwError);
                    }

                    VmDirStringListFree(pScopes);
                    pScopes = NULL;
                }
            }
        }
    }

    dwError = VmDirIndexUpdateCommit(pIndexUpd);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirStringListFree(pScopes);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    (VOID)VmDirIndexUpdateAbort(pIndexUpd);
    goto cleanup;
}
