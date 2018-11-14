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
_LwCAPolicyValidateInternal(
    PLWCA_POLICIES              pPoliciesAllowed,
    PLWCA_REQ_CONTEXT           pReqContext,
    X509_REQ                    *pRequest,
    PLWCA_CERT_VALIDITY         pValidity,
    LWCA_POLICY_CHECKS          policyChecks,
    BOOLEAN                     *pbIsValid
    );

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
    X509_REQ                    *pRequest,
    PLWCA_CERT_VALIDITY         pValidity,
    LWCA_POLICY_TYPE            policyType,
    LWCA_POLICY_CHECKS          policyChecks,
    BOOLEAN                     *pbIsValid
    )
{
    DWORD dwError = 0;
    BOOLEAN bIsValid = FALSE;

    if (!pPolicyCtx || !pReqContext || !pRequest || !pbIsValid)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    switch (policyType)
    {
    case LWCA_POLICY_TYPE_CA:
        if (pPolicyCtx->pCAPoliciesAllowed)
        {
            dwError = _LwCAPolicyValidateInternal(
                        pPolicyCtx->pCAPoliciesAllowed,
                        pReqContext,
                        pRequest,
                        pValidity,
                        policyChecks,
                        &bIsValid);
            BAIL_ON_LWCA_ERROR(dwError);
            BAIL_ON_LWCA_POLICY_VIOLATION(bIsValid);
        }
        else
        {
            LWCA_LOG_INFO("Policy Engine: No CA Policies defined in policy config. Passing.");
            bIsValid = TRUE;
        }

        break;

    case LWCA_POLICY_TYPE_CERTIFICATE:
        if (pPolicyCtx->pCertPoliciesAllowed)
        {
            dwError = _LwCAPolicyValidateInternal(
                        pPolicyCtx->pCertPoliciesAllowed,
                        pReqContext,
                        pRequest,
                        pValidity,
                        policyChecks,
                        &bIsValid);
            BAIL_ON_LWCA_ERROR(dwError);
            BAIL_ON_LWCA_POLICY_VIOLATION(bIsValid);
        }
        else
        {
            LWCA_LOG_INFO("Policy Engine: No Certificate Policies defined in policy config. Passing.");
            bIsValid = TRUE;
        }

        break;

    default:
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

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

DWORD
LwCAPolicyGetCertDuration(
    PLWCA_POLICY_CONTEXT                pPolicyCtx,
    LWCA_POLICY_TYPE                    policyType,
    DWORD                               *pdwDuration
    )
{
    DWORD dwError = 0;
    DWORD dwDuration = 0;

    if (!pPolicyCtx || !pdwDuration)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    switch (policyType)
    {
    case LWCA_POLICY_TYPE_CA:
        if (pPolicyCtx->pCAPoliciesAllowed)
        {
            dwDuration = pPolicyCtx->pCAPoliciesAllowed->dwCertDuration;
        }
        break;

    case LWCA_POLICY_TYPE_CERTIFICATE:
        if (pPolicyCtx->pCertPoliciesAllowed)
        {
            dwDuration = pPolicyCtx->pCertPoliciesAllowed->dwCertDuration;
        }
        break;

    default:
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *pdwDuration = dwDuration;

cleanup:
    return dwError;

error:
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

static
DWORD
_LwCAPolicyValidateInternal(
    PLWCA_POLICIES              pPoliciesAllowed,
    PLWCA_REQ_CONTEXT           pReqContext,
    X509_REQ                    *pRequest,
    PLWCA_CERT_VALIDITY         pValidity,
    LWCA_POLICY_CHECKS          policyChecks,
    BOOLEAN                     *pbIsValid
    )
{
    DWORD dwError = 0;
    BOOLEAN bIsValid = FALSE;

    // TODO: Check expiration date

    if ( policyChecks == LWCA_POLICY_CHECK_NONE )
    {
        bIsValid = TRUE;
    }

    if ( (policyChecks & LWCA_POLICY_CHECK_SN) )
    {
        if (pPoliciesAllowed->pSNs)
        {
            dwError = LwCAPolicyValidateSNPolicy(pPoliciesAllowed->pSNs, pReqContext, pRequest, &bIsValid);
            BAIL_ON_LWCA_ERROR(dwError);
            BAIL_ON_LWCA_POLICY_VIOLATION(bIsValid);
        }
        else
        {
            LWCA_LOG_INFO("Policy Engine: No SN policies defined in policy config. Passing");
            bIsValid = TRUE;
        }
    }

    if ( (policyChecks & LWCA_POLICY_CHECK_SAN) )
    {
        if (pPoliciesAllowed->pSANs)
        {
            dwError = LwCAPolicyValidateSANPolicy(
                        pPoliciesAllowed->pSANs,
                        pPoliciesAllowed->bMultiSANEnabled,
                        pReqContext,
                        pRequest,
                        &bIsValid);
            BAIL_ON_LWCA_ERROR(dwError);
            BAIL_ON_LWCA_POLICY_VIOLATION(bIsValid);
        }
        else
        {
            LWCA_LOG_INFO("Policy Engine: No SAN policies defined in policy config. Passing");
            bIsValid = TRUE;
        }
    }

    if ( (policyChecks & LWCA_POLICY_CHECK_KEY_USAGE) )
    {
        dwError = LwCAPolicyValidateKeyUsagePolicy(pPoliciesAllowed->dwKeyUsage, pRequest, &bIsValid);
        BAIL_ON_LWCA_ERROR(dwError);
        BAIL_ON_LWCA_POLICY_VIOLATION(bIsValid);
    }

    if ( (policyChecks & LWCA_POLICY_CHECK_DURATION) )
    {
        if (pValidity)
        {
            dwError = LwCAPolicyValidateCertDurationPolicy(pPoliciesAllowed->dwCertDuration, pValidity, &bIsValid);
            BAIL_ON_LWCA_ERROR(dwError);
            BAIL_ON_LWCA_POLICY_VIOLATION(bIsValid);
        }
        else
        {
            LWCA_LOG_INFO("Validity not provided. Duration will be taken from the policy config.");
            bIsValid = TRUE;
        }
    }

    *pbIsValid = bIsValid;

cleanup:
    return dwError;

error:
    *pbIsValid = FALSE;
    goto cleanup;
}
