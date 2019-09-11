/*
 * Copyright ©2019 VMware, Inc.  All Rights Reserved.
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
VmDirIterSearchPlanInitContent(
    int     iSearchType,
    BOOLEAN bReverse,
    PSTR    pszAttrName,
    PSTR    pszAttrVal,
    PVDIR_ITERATOR_SEARCH_PLAN  pIterSearchPlan
    )
{
    DWORD dwError = 0;

    if (!pIterSearchPlan || !pszAttrName)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pIterSearchPlan->iterSearchType = iSearchType;
    pIterSearchPlan->bReverseSearch = bReverse;

    dwError = VmDirAllocateStringA(pszAttrName, &pIterSearchPlan->attr.lberbv_val);
    BAIL_ON_VMDIR_ERROR(dwError);
    pIterSearchPlan->attr.lberbv_len = VmDirStringLenA(pszAttrName);
    pIterSearchPlan->attr.bOwnBvVal = TRUE;

    if (pszAttrVal)
    {
        dwError = VmDirAllocateStringA(pszAttrVal, &pIterSearchPlan->attrNormVal.lberbv_val);
        BAIL_ON_VMDIR_ERROR(dwError);
        pIterSearchPlan->attrNormVal.lberbv_len = VmDirStringLenA(pszAttrVal);
        pIterSearchPlan->attrNormVal.bOwnBvVal = TRUE;
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

VOID
VmDirIterSearchPlanFreeContent(
    PVDIR_ITERATOR_SEARCH_PLAN  pIterSearchPlan
    )
{
    if (pIterSearchPlan)
    {
        VmDirFreeBervalContent(&pIterSearchPlan->attr);
        VmDirFreeBervalContent(&pIterSearchPlan->attrNormVal);
    }

    return;
}

DWORD
VmDirIterContextInitContent(
    PVDIR_ITERATOR_CONTEXT      pIterContext,
    PVDIR_ITERATOR_SEARCH_PLAN  pIterSearchPlan
    )
{
    DWORD dwError = 0;

    if (!pIterContext || !pIterSearchPlan)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pIterContext->iSearchType    = pIterSearchPlan->iterSearchType;
    pIterContext->bReverseSearch = pIterSearchPlan->bReverseSearch;

    if (pIterSearchPlan->attr.lberbv.bv_val)
    {
        dwError = VmDirAllocateStringA(
                pIterSearchPlan->attr.lberbv.bv_val,
                &pIterContext->pszIterTable);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pIterSearchPlan->attrNormVal.lberbv.bv_val)
    {
        if (VmDirStringCompareA(pIterSearchPlan->attr.lberbv_val, ATTR_PARENT_ID, FALSE) == 0)
        {
            dwError = VmDirMDBIndexIteratorInitParentIdKey(
                    pIterContext,
                    pIterSearchPlan->attrNormVal.lberbv_val);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            dwError = VmDirMDBIndexIteratorInitKey(
                    pIterContext,
                    pIterSearchPlan->attrNormVal.lberbv_val);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    dwError = LwRtlCreateHashMap(
            &pIterContext->pSentIDMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    pIterContext->bInit = TRUE;

cleanup:
    return dwError;

error:
    goto cleanup;
}

VOID
VmDirIterContextFreeContent(
    PVDIR_ITERATOR_CONTEXT pIterContext
    )
{
    if (pIterContext)
    {
        VMDIR_SAFE_FREE_MEMORY(pIterContext->pszIterTable);
        VmDirFreeBervalContent(&pIterContext->bvCurrentKey);
        VmDirFreeBervalContent(&pIterContext->bvFilterValue);

        if (pIterContext->pSentIDMap)
        {
            LwRtlHashMapClear(pIterContext->pSentIDMap, VmDirSimpleHashMapPairFreeKeyOnly, NULL);
            LwRtlFreeHashMap(&pIterContext->pSentIDMap);
            pIterContext->pSentIDMap = NULL;
        }
    }
}
