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
LwCAPolicyInitCtx(
    PLWCA_JSON_OBJECT           pJson,
    PLWCA_POLICY_CONTEXT        *ppPolicyCtx
    )
{
    DWORD dwError = 0;
    PLWCA_JSON_OBJECT pCAPolicyJson = NULL;
    PLWCA_JSON_OBJECT pCertPolicyJson = NULL;
    PLWCA_POLICY_CONTEXT pPolicyCtx = NULL;

    if (!pJson || !ppPolicyCtx)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_POLICY_CONTEXT), (PVOID*)&pPolicyCtx);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetObjectFromKey(pJson, TRUE, LWCA_CA_POLICY_KEY, &pCAPolicyJson);
    BAIL_ON_LWCA_POLICY_CFG_ERROR_WITH_MSG(dwError, "Failed to get CAPolicies from policy config");

    if (pCAPolicyJson)
    {
        dwError = LwCAPolicyParseCfgPolicies(pCAPolicyJson, &pPolicyCtx->pCAPoliciesAllowed);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonGetObjectFromKey(pJson, TRUE, LWCA_CERT_POLICY_KEY, &pCertPolicyJson);
    BAIL_ON_LWCA_POLICY_CFG_ERROR_WITH_MSG(dwError, "Failed to get CertificatePolicies from policy config");

    if (pCertPolicyJson)
    {
        dwError = LwCAPolicyParseCfgPolicies(pCertPolicyJson, &pPolicyCtx->pCertPoliciesAllowed);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppPolicyCtx = pPolicyCtx;

cleanup:
    return dwError;

error:
    LwCAJsonCleanupObject(pCAPolicyJson);
    LwCAJsonCleanupObject(pCertPolicyJson);
    LWCA_SAFE_FREE_MEMORY(pPolicyCtx);
    if (ppPolicyCtx)
    {
        *ppPolicyCtx = pPolicyCtx;
    }

    goto cleanup;
}

DWORD
LwCAPolicyValidate(
    PLWCA_POLICY_CONTEXT        pPolicyCtx,
    PLWCA_REQ_CONTEXT           pReqContext,
    PSTR                        pszPKCS10Request,
    LWCA_POLICY_TYPE            policyType,
    LWCA_POLICY_CHECKS          policyChecks,
    BOOLEAN                     *pbIsValid
    )
{
    DWORD dwError = 0;
    BOOLEAN bIsValid = FALSE;

    if (!pPolicyCtx || !pReqContext || !pszPKCS10Request || !pbIsValid)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    // TODO: Call individual validate functions (in subsequent commits)

    *pbIsValid = bIsValid;

cleanup:
    return dwError;

error:
    if (pbIsValid)
    {
        *pbIsValid = FALSE;
    }

    goto cleanup;
}

VOID
LwCAPolicyFreeCtx(
    PLWCA_POLICY_CONTEXT        pPolicyCtx
    )
{
    if (pPolicyCtx)
    {
        LwCAPoliciesFree(pPolicyCtx->pCAPoliciesAllowed);
        LwCAPoliciesFree(pPolicyCtx->pCertPoliciesAllowed);
        LWCA_SAFE_FREE_MEMORY(pPolicyCtx);
    }
}
