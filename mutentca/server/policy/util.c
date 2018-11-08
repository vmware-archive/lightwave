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
LwCAPolicyCfgObjInit(
    LWCA_POLICY_CFG_TYPE            type,
    LWCA_POLICY_CFG_MATCH           match,
    PCSTR                           pcszValue,
    PCSTR                           pcszPrefix,
    PCSTR                           pcszSuffix,
    PLWCA_POLICY_CFG_OBJ            *ppObj
    )
{
    DWORD dwError = 0;
    PLWCA_POLICY_CFG_OBJ pObj = NULL;

    if (!ppObj)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_POLICY_CFG_OBJ), (PVOID*)&pObj);
    BAIL_ON_LWCA_ERROR(dwError);

    pObj->type = type;
    pObj->match = match;

    if (!IsNullOrEmptyString(pcszValue))
    {
        dwError = LwCAAllocateStringA(pcszValue, &pObj->pszValue);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pcszPrefix))
    {
        dwError = LwCAAllocateStringA(pcszPrefix, &pObj->pszPrefix);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pcszSuffix))
    {
        dwError = LwCAAllocateStringA(pcszSuffix, &pObj->pszSuffix);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (match == LWCA_POLICY_CFG_MATCH_REGEX)
    {
        dwError = LwCARegexInit(pcszValue, &pObj->pRegex);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppObj = pObj;

cleanup:
    return dwError;

error:
    LwCAPolicyCfgObjFree(pObj);
    if (ppObj)
    {
        *ppObj = NULL;
    }

    goto cleanup;
}

DWORD
LwCAPolicyCfgObjArrayAllocate(
    DWORD                           dwCount,
    PLWCA_POLICY_CFG_OBJ_ARRAY      *ppObjArray
    )
{
    DWORD dwError = 0;
    PLWCA_POLICY_CFG_OBJ_ARRAY pObjArray = NULL;

    if (!ppObjArray || dwCount == 0)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(
                sizeof(LWCA_POLICY_CFG_OBJ_ARRAY),
                (PVOID*)&pObjArray);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateMemory(
                sizeof(LWCA_POLICY_CFG_OBJ) * dwCount,
                (PVOID*)&pObjArray->ppObj);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppObjArray = pObjArray;

cleanup:
    return dwError;

error:
    LwCAPolicyCfgObjArrayFree(pObjArray);
    goto cleanup;
}

DWORD
LwCAPolicyCfgObjArrayInit(
    PLWCA_POLICY_CFG_OBJ            *ppObj,
    DWORD                           dwCount,
    PLWCA_POLICY_CFG_OBJ_ARRAY      *ppObjArray
    )
{
    DWORD dwError = 0;
    DWORD dwIdx = 0;
    PLWCA_POLICY_CFG_OBJ_ARRAY pObjArray = NULL;

    if (!ppObj || !ppObjArray || dwCount == 0)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAPolicyCfgObjArrayAllocate(dwCount, &pObjArray);
    BAIL_ON_LWCA_ERROR(dwError);

    for ( ; dwIdx < dwCount ; ++dwIdx )
    {
        dwError = LwCAPolicyCfgObjCopy(ppObj[dwIdx], &pObjArray->ppObj[dwIdx]);
        BAIL_ON_LWCA_ERROR(dwError);

        pObjArray->dwCount++;
    }

    *ppObjArray = pObjArray;

cleanup:
    return dwError;

error:
    LwCAPolicyCfgObjArrayFree(pObjArray);
    if (ppObjArray)
    {
        *ppObjArray = NULL;
    }

    goto cleanup;
}

DWORD
LwCAPoliciesInit(
    BOOLEAN                         bMultiSANEnabled,
    PLWCA_POLICY_CFG_OBJ_ARRAY      pSNs,
    PLWCA_POLICY_CFG_OBJ_ARRAY      pSANs,
    DWORD                           dwKeyUsage,
    DWORD                           dwCertDuration,
    PLWCA_POLICIES                  *ppPolicies
    )
{
    DWORD dwError = 0;
    PLWCA_POLICIES pPolicies = NULL;

    if (!ppPolicies)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_POLICIES), (PVOID*)&pPolicies);
    BAIL_ON_LWCA_ERROR(dwError);

    if (pSNs)
    {
        dwError = LwCAPolicyCfgObjArrayCopy(pSNs, &pPolicies->pSNs);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (pSANs)
    {
        dwError = LwCAPolicyCfgObjArrayCopy(pSANs, &pPolicies->pSANs);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pPolicies->bMultiSANEnabled = bMultiSANEnabled;
    pPolicies->dwKeyUsage = dwKeyUsage;
    pPolicies->dwCertDuration = dwCertDuration;

    *ppPolicies = pPolicies;

cleanup:
    return dwError;

error:
    LwCAPoliciesFree(pPolicies);
    if (ppPolicies)
    {
        *ppPolicies = NULL;
    }

    goto cleanup;
}

DWORD
LwCAPolicyCfgObjCopy(
    PLWCA_POLICY_CFG_OBJ            pObjIn,
    PLWCA_POLICY_CFG_OBJ            *ppObjOut
    )
{
    DWORD dwError = 0;
    PLWCA_POLICY_CFG_OBJ pObj = NULL;

    if (!pObjIn || !ppObjOut)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAPolicyCfgObjInit(
                pObjIn->type,
                pObjIn->match,
                pObjIn->pszValue,
                pObjIn->pszPrefix,
                pObjIn->pszSuffix,
                &pObj);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppObjOut = pObj;

cleanup:
    return dwError;

error:
    LwCAPolicyCfgObjFree(pObj);
    if (ppObjOut)
    {
        *ppObjOut = NULL;
    }

    goto cleanup;
}

DWORD
LwCAPolicyCfgObjArrayCopy(
    PLWCA_POLICY_CFG_OBJ_ARRAY      pObjArrayIn,
    PLWCA_POLICY_CFG_OBJ_ARRAY      *ppObjArrayOut
    )
{
    DWORD dwError = 0;
    PLWCA_POLICY_CFG_OBJ_ARRAY pObjArray = NULL;

    if (!pObjArrayIn || !ppObjArrayOut)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAPolicyCfgObjArrayInit(pObjArrayIn->ppObj, pObjArrayIn->dwCount, &pObjArray);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppObjArrayOut = pObjArray;

cleanup:
    return dwError;

error:
    LwCAPolicyCfgObjArrayFree(pObjArray);
    if (ppObjArrayOut)
    {
        *ppObjArrayOut = NULL;
    }

    goto cleanup;
}

VOID
LwCAPolicyCfgObjFree(
    PLWCA_POLICY_CFG_OBJ            pObj
    )
{
    if (pObj)
    {
        LWCA_SAFE_FREE_STRINGA(pObj->pszValue);
        LWCA_SAFE_FREE_STRINGA(pObj->pszPrefix);
        LWCA_SAFE_FREE_STRINGA(pObj->pszSuffix);
        LwCARegexFree(pObj->pRegex);
        LWCA_SAFE_FREE_MEMORY(pObj);
    }
}

VOID
LwCAPolicyCfgObjArrayFree(
    PLWCA_POLICY_CFG_OBJ_ARRAY      pObjArray
    )
{
    DWORD dwIdx = 0;

    if (pObjArray)
    {
        for ( ; dwIdx < pObjArray->dwCount ; ++dwIdx )
        {
            LwCAPolicyCfgObjFree(pObjArray->ppObj[dwIdx]);
        }
        LWCA_SAFE_FREE_MEMORY(pObjArray->ppObj);
        LWCA_SAFE_FREE_MEMORY(pObjArray);
    }
}

VOID
LwCAPoliciesFree(
    PLWCA_POLICIES                  pPolicies
    )
{
    if (pPolicies)
    {
        LwCAPolicyCfgObjArrayFree(pPolicies->pSNs);
        LwCAPolicyCfgObjArrayFree(pPolicies->pSANs);
        LWCA_SAFE_FREE_MEMORY(pPolicies);
    }
}
