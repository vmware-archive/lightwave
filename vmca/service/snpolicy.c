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
VMCAPolicySNOperationParse(
    json_t                          *pJsonRule,
    PDWORD                          pdwOperationsLen,
    PVMCA_SNPOLICY_OPERATION        **pppOperations
    );

static
VOID
VMCAPolicySNOperationFree(
    PVMCA_SNPOLICY_OPERATION        pOperation
    );

static
VOID
VMCAPolicySNOperationArrayFree(
    PVMCA_SNPOLICY_OPERATION        *ppOperations,
    DWORD                           dwArrayLen
    );


DWORD
VMCAPolicySNLoad(
    json_t                          *pJsonRules,
    PVMCA_POLICY                    pPolicy
    )
{
    DWORD                           dwError = 0;
    json_t                          *pJsonRulesMatch = NULL;
    json_t                          *pJsonRulesValidate = NULL;
    VMCA_POLICY_RULES               rules = { 0 };

    if (!pJsonRules || !pPolicy)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (!(pJsonRulesMatch = json_object_get(pJsonRules, "match")))
    {
        VMCA_LOG_ERROR(
            "[%s,%d] Failed to get SN policy match rule from config (%s)",
            __FUNCTION__,
            __LINE__,
            VMCA_POLICY_FILE_PATH);
        dwError = VMCA_JSON_PARSE_ERROR;
        BAIL_ON_JSON_PARSE_ERROR(dwError);
    }

    if (!(pJsonRulesValidate  = json_object_get(pJsonRules, "validate")))
    {
        VMCA_LOG_ERROR(
            "[%s,%d] SN policy must have either validate rule or action rule in config (%s)",
            __FUNCTION__,
            __LINE__,
            VMCA_POLICY_FILE_PATH);
        dwError = VMCA_JSON_PARSE_ERROR;
        BAIL_ON_JSON_PARSE_ERROR(dwError);
    }

    dwError = VMCAPolicySNOperationParse(
                            pJsonRulesMatch,
                            &rules.SN.dwMatchLen,
                            &rules.SN.ppMatch);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAPolicySNOperationParse(
                            pJsonRulesValidate,
                            &rules.SN.dwValidateLen,
                            &rules.SN.ppValidate);
    BAIL_ON_VMCA_ERROR(dwError);

    rules.SN.bEnabled = TRUE;

    pPolicy->Rules = rules;


cleanup:

    return dwError;

error:

    VMCAPolicySNFree(&rules);
    pPolicy->Rules.SN.bEnabled = FALSE;
    if (pPolicy->Rules.SN.ppMatch)
    {
        VMCAPolicySNOperationArrayFree(pPolicy->Rules.SN.ppMatch, pPolicy->Rules.SN.dwMatchLen);
        pPolicy->Rules.SN.ppMatch = NULL;
        pPolicy->Rules.SN.dwMatchLen = 0;
    }
    if (pPolicy->Rules.SN.ppValidate)
    {
        VMCAPolicySNOperationArrayFree(pPolicy->Rules.SN.ppValidate, pPolicy->Rules.SN.dwValidateLen);
        pPolicy->Rules.SN.ppValidate = NULL;
        pPolicy->Rules.SN.dwValidateLen = 0;
    }

    goto cleanup;
}

VOID
VMCAPolicySNFree(
    PVMCA_POLICY_RULES      pPolicyRules
    )
{
    if (pPolicyRules)
    {
        pPolicyRules->SN.bEnabled = FALSE;
        if (pPolicyRules->SN.ppMatch)
        {
            VMCAPolicySNOperationArrayFree(pPolicyRules->SN.ppMatch, pPolicyRules->SN.dwMatchLen);
            pPolicyRules->SN.ppMatch = NULL;
            pPolicyRules->SN.dwMatchLen = 0;
        }
        if (pPolicyRules->SN.ppValidate)
        {
            VMCAPolicySNOperationArrayFree(pPolicyRules->SN.ppValidate, pPolicyRules->SN.dwValidateLen);
            pPolicyRules->SN.ppValidate = NULL;
            pPolicyRules->SN.dwValidateLen = 0;
        }

        pPolicyRules = NULL;
    }
}


static
DWORD
VMCAPolicySNOperationParse(
    json_t                          *pJsonRule,
    PDWORD                          pdwOperationsLen,
    PVMCA_SNPOLICY_OPERATION        **pppOperations
    )
{
    DWORD                           dwError = 0;
    DWORD                           dwIdx = 0;
    PCSTR                           pcszDataTemp = NULL;
    PCSTR                           pcszWithTemp = NULL;
    size_t                          szArrayLen = 0;
    json_t                          *pJsonTemp = NULL;
    json_t                          *pJsonDataTemp = NULL;
    json_t                          *pJsonWithTemp = NULL;
    PVMCA_SNPOLICY_OPERATION        *ppOperations = NULL;

    if (!pJsonRule || !pppOperations)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (!json_is_array(pJsonRule))
    {
        VMCA_LOG_ERROR(
            "[%s,%d] SN policy rule in config (%s) must be an array",
            __FUNCTION__,
            __LINE__,
            VMCA_POLICY_FILE_PATH);
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    szArrayLen = json_array_size(pJsonRule);
    if (szArrayLen < 1)
    {
        VMCA_LOG_ERROR(
            "[%s,%d] SN policy rule in config (%s) must be an array with at least 1 object",
            __FUNCTION__,
            __LINE__,
            VMCA_POLICY_FILE_PATH);
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocateMemory(
                        sizeof(PVMCA_SNPOLICY_OPERATION) * (DWORD)szArrayLen,
                        (PVOID *)&ppOperations);
    BAIL_ON_VMCA_ERROR(dwError);

    json_array_foreach(pJsonRule, dwIdx, pJsonTemp)
    {
        if (!(pJsonDataTemp = json_object_get(pJsonTemp, "data")))
        {
            VMCA_LOG_ERROR(
                "[%s,%d] SN policy rule in config (%s) must have \"data\" clause",
                __FUNCTION__,
                __LINE__,
                VMCA_POLICY_FILE_PATH);
            dwError = VMCA_JSON_PARSE_ERROR;
            BAIL_ON_JSON_PARSE_ERROR(dwError);
        }

        if (!(pJsonWithTemp = json_object_get(pJsonTemp, "with")))
        {
            VMCA_LOG_ERROR(
                "[%s,%d] SN policy rule in config (%s) must have \"with\" clause",
                __FUNCTION__,
                __LINE__,
                VMCA_POLICY_FILE_PATH);
            dwError = VMCA_JSON_PARSE_ERROR;
            BAIL_ON_JSON_PARSE_ERROR(dwError);
        }

        pcszDataTemp = json_string_value(pJsonDataTemp);
        if (IsNullOrEmptyString(pcszDataTemp))
        {
            VMCA_LOG_ERROR(
                "[%s,%d] SN policy rule in config (%s) cannot have empty \"data\" clause",
                __FUNCTION__,
                __LINE__,
                VMCA_POLICY_FILE_PATH);
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMCA_ERROR(dwError);
        }

        pcszWithTemp = json_string_value(pJsonWithTemp);
        if (IsNullOrEmptyString(pcszWithTemp))
        {
            VMCA_LOG_ERROR(
                "[%s,%d] SN policy rule in config (%s) cannot have empty \"with\" clause",
                __FUNCTION__,
                __LINE__,
                VMCA_POLICY_FILE_PATH);
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMCA_ERROR(dwError);
        }

        dwError = VMCAAllocateMemory(
                            sizeof(VMCA_SNPOLICY_OPERATION),
                            (PVOID *)&ppOperations[dwIdx]);
        BAIL_ON_VMCA_ERROR(dwError);

        dwError = VMCAAllocateStringA(
                            pcszWithTemp,
                            &ppOperations[dwIdx]->pszWith);
        BAIL_ON_VMCA_ERROR(dwError);

        dwError = VMCAAllocateStringA(
                            pcszDataTemp,
                            &ppOperations[dwIdx]->pszData);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    *pdwOperationsLen   = (DWORD)szArrayLen;
    *pppOperations      = ppOperations;

cleanup:

    return dwError;

error:

    VMCAPolicySNOperationArrayFree(ppOperations, (DWORD)szArrayLen);
    if (pppOperations)
    {
        *pppOperations = NULL;
    }

    goto cleanup;
}

static
VOID
VMCAPolicySNOperationFree(
    PVMCA_SNPOLICY_OPERATION        pOperation
    )
{
    if (pOperation)
    {
        if (pOperation->pszData)
        {
            VMCA_SAFE_FREE_STRINGA(pOperation->pszData);
            pOperation->pszData = NULL;
        }
        if (pOperation->pszWith)
        {
            VMCA_SAFE_FREE_STRINGA(pOperation->pszWith);
            pOperation->pszWith = NULL;
        }

        pOperation = NULL;
    }
}

static
VOID
VMCAPolicySNOperationArrayFree(
    PVMCA_SNPOLICY_OPERATION        *ppOperations,
    DWORD                           dwArrayLen
    )
{
    DWORD       dwIdx = 0;

    if (ppOperations)
    {
        if (dwArrayLen > 0)
        {
            for (; dwIdx < dwArrayLen; ++dwIdx)
            {
                if (ppOperations[dwIdx])
                {
                    VMCAPolicySNOperationFree(ppOperations[dwIdx]);
                    ppOperations[dwIdx] = NULL;
                }
            }
        }

        VMCA_SAFE_FREE_MEMORY(ppOperations);
        ppOperations = NULL;
    }
}
