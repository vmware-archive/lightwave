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

static const VMCA_POLICY_METHODS VMCAPolicyMethodMap[] =
    {
        {
            VMCA_POLICY_TYPE_SN,        // SNPolicy method map.  SN Policy will verify requestor's
                                        // account lives in hierarchy that can be comprised by
                                        // values in CSR subject field.

                VMCAPolicySNLoad,       // Function to parse and load policy from config.
                VMCAPolicySNFree,       // Function to free SN policy object.
                VMCAPolicySNValidate    // Function to validate request compliant with policy
        }
    };
static const DWORD VMCAPolicyMethodMapSize =
    sizeof(VMCAPolicyMethodMap) / sizeof(VMCA_POLICY_METHODS);

DWORD
VMCAPolicyInit(
    PSTR                pszConfigFilePath,
    PVMCA_POLICY        **pppPolicies
    )
{
    DWORD               dwError = 0;
    DWORD               dwIdx1 = 0;
    DWORD               dwIdx2 = 0;
    PCSTR               pcszKey = NULL;
    json_error_t        jsonError = {0};
    json_t              *pJsonPolicy = NULL;
    json_t              *pJsonPolicyRules = NULL;
    size_t              szJsonFlags = JSON_DECODE_ANY;
    PVMCA_POLICY        *ppPolicies = NULL;

    if (IsNullOrEmptyString(pszConfigFilePath) || !pppPolicies)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (!(pJsonPolicy = json_load_file(pszConfigFilePath, szJsonFlags, &jsonError)))
    {
        VMCA_LOG_ERROR(
                "[%s,%d] Failed to open policy config file (%s). Error: %s:%d",
                __FUNCTION__,
                __LINE__,
                pszConfigFilePath,
                jsonError.text,
                jsonError.line);
        dwError = VMCA_JSON_FILE_LOAD_ERROR;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocateMemory(
                        sizeof(PVMCA_POLICY) * VMCA_POLICY_NUM,
                        (PVOID *)&ppPolicies);
    BAIL_ON_VMCA_ERROR(dwError);

    json_object_foreach(pJsonPolicy, pcszKey, pJsonPolicyRules)
    {
        if (dwIdx1 < VMCA_POLICY_NUM)
        {
            dwError = VMCAAllocateMemory(
                                sizeof(VMCA_POLICY),
                                (PVOID *)&ppPolicies[dwIdx1]);
            BAIL_ON_VMCA_ERROR(dwError);

            if (!VMCAStringCompareA(pcszKey, VMCA_POLICY_SN_NAME, TRUE))
            {
                ppPolicies[dwIdx1]->type = VMCA_POLICY_TYPE_SN;
            }
            /* NOTE: Add any new policy name checks here as an else if case */
            else
            {
                VMCA_LOG_ERROR(
                        "[%s,%d] Unsupported policy (%s) specified in config file (%s)",
                        __FUNCTION__,
                        __LINE__,
                        pcszKey,
                        pszConfigFilePath);
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMCA_ERROR(dwError);
            }

            for (; dwIdx2 < VMCAPolicyMethodMapSize; ++dwIdx2)
            {
                if (ppPolicies[dwIdx1]->type == VMCAPolicyMethodMap[dwIdx2].type)
                {
                    dwError = VMCAPolicyMethodMap[dwIdx2].pfnLoad(
                                                                pJsonPolicyRules,
                                                                ppPolicies[dwIdx1]
                                                                );
                    BAIL_ON_VMCA_ERROR(dwError);
                }
            }

            ++dwIdx1;
        }
        else
        {
            VMCA_LOG_ERROR(
                    "[%s,%d] Too many policies defined in config file (%s). Current limit is %d",
                    __FUNCTION__,
                    __LINE__,
                    pszConfigFilePath,
                    VMCA_POLICY_NUM);
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMCA_ERROR(dwError);
        }
    }

    *pppPolicies = ppPolicies;


cleanup:

    if (pJsonPolicy)
    {
        json_decref(pJsonPolicy);
    }

    return dwError;

error:

    VMCAPolicyArrayFree(ppPolicies);
    if (pppPolicies)
    {
        *pppPolicies = NULL;
    }

    goto cleanup;
}

DWORD
VMCAPolicyValidate(
    PVMCA_POLICY            *ppPolicies,
    PSTR                    pszPKCS10Request,
    PVMCA_REQ_CONTEXT       pReqContext,
    PBOOLEAN                pbIsValid
    )
{
    DWORD                   dwError = 0;
    DWORD                   dwIdx = 0;
    BOOLEAN                 bIsValid = FALSE;

    if (!ppPolicies)
    {
        bIsValid = TRUE;
        goto ret;
    }

    if (IsNullOrEmptyString(pszPKCS10Request) ||
        !pReqContext ||
        !pbIsValid)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    for (; dwIdx < VMCAPolicyMethodMapSize; ++dwIdx)
    {
        dwError = VMCAPolicyMethodMap[dwIdx].pfnValidate(ppPolicies[dwIdx],
                                                         pszPKCS10Request,
                                                         pReqContext,
                                                         &bIsValid);
        BAIL_ON_VMCA_ERROR(dwError);

        if (bIsValid == FALSE)
        {
            break;
        }
    }


ret:

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
VMCAPolicyFree(
    PVMCA_POLICY    pPolicy
    )
{
    DWORD   dwIdx = 0;

    if (pPolicy)
    {
        for (; dwIdx < VMCAPolicyMethodMapSize; ++dwIdx)
        {
            if (pPolicy->type == VMCAPolicyMethodMap[dwIdx].type)
            {
                VMCAPolicyMethodMap[dwIdx].pfnFree(&pPolicy->Rules);
            }
        }

        VMCA_SAFE_FREE_MEMORY(pPolicy);
        pPolicy = NULL;
    }
}

VOID
VMCAPolicyArrayFree(
    PVMCA_POLICY    *ppPolicies
    )
{
    DWORD dwIdx = 0;

    if (ppPolicies)
    {
        for (; dwIdx < VMCA_POLICY_NUM; ++dwIdx)
        {
            if (ppPolicies[dwIdx])
            {
                VMCAPolicyFree(ppPolicies[dwIdx]);
                ppPolicies[dwIdx] = NULL;
            }
        }

        VMCA_SAFE_FREE_MEMORY(ppPolicies);
        ppPolicies = NULL;
    }
}
