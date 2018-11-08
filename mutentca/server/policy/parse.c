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
_LwCAPolicyGetSNsFromJson(
    PLWCA_JSON_OBJECT           pJson,
    PLWCA_POLICY_CFG_OBJ_ARRAY  *ppSNs
    );

static
DWORD
_LwCAPolicyGetSANsFromJson(
    PLWCA_JSON_OBJECT           pJson,
    PLWCA_POLICY_CFG_OBJ_ARRAY  *ppSANs
    );

static
DWORD
_LwCAPolicyGetCfgObjectsFromJson(
    PLWCA_JSON_OBJECT           pJsonArray,
    PLWCA_POLICY_CFG_OBJ_ARRAY  *ppObjArray
    );

static
DWORD
_LwCAPolicyVerifyAndGetCfgObjEnums(
    PCSTR                       pcszType,
    PCSTR                       pcszMatch,
    PCSTR                       pcszValue,
    PLWCA_POLICY_CFG_TYPE       pType,
    PLWCA_POLICY_CFG_MATCH      pMatch
    );

static
DWORD
_LwCAPolicyGetTypeEnum(
    PCSTR                       pcszType,
    PLWCA_POLICY_CFG_TYPE       pType
    );

static
DWORD
_LwCAPolicyVerifyValidStringValue(
    PCSTR                       pcszMatch,
    PCSTR                       pcszValue
    );

DWORD
LwCAPolicyParseCfgPolicies(
    PLWCA_JSON_OBJECT           pJson,
    PLWCA_POLICIES              *ppPolicies
    )
{
    DWORD dwError = 0;
    PLWCA_JSON_OBJECT pSNPolicyJson = NULL;
    PLWCA_JSON_OBJECT pSANPolicyJson = NULL;
    PLWCA_JSON_OBJECT pKeyUsagePolicyJson = NULL;
    PLWCA_JSON_OBJECT pCertDurationPolicyJson = NULL;
    PLWCA_POLICIES pPolicies = NULL;

    if (!pJson || !ppPolicies)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAPoliciesInit(FALSE, NULL, NULL, 0, 0, &pPolicies);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetObjectFromKey(pJson, TRUE, LWCA_SN_POLICY_KEY, &pSNPolicyJson);
    BAIL_ON_LWCA_POLICY_CFG_ERROR_WITH_MSG(dwError, "Failed to get SNPolicy from policy config");

    if (pSNPolicyJson)
    {
        dwError = _LwCAPolicyGetSNsFromJson(pSNPolicyJson, &pPolicies->pSNs);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonGetObjectFromKey(pJson, TRUE, LWCA_SAN_POLICY_KEY, &pSANPolicyJson);
    BAIL_ON_LWCA_POLICY_CFG_ERROR_WITH_MSG(dwError, "Failed to get SANPolicy from policy config");

    if (pSANPolicyJson)
    {
        dwError = LwCAJsonGetBooleanFromKey(pSANPolicyJson, TRUE, LWCA_MULTI_SAN_KEY, &pPolicies->bMultiSANEnabled);
        BAIL_ON_LWCA_POLICY_CFG_ERROR_WITH_MSG(dwError, "Failed to get multiSAN boolean value from policy config");

        dwError = _LwCAPolicyGetSANsFromJson(pSANPolicyJson, &pPolicies->pSANs);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonGetObjectFromKey(pJson, TRUE, LWCA_KEY_USAGE_POLICY_KEY, &pKeyUsagePolicyJson);
    BAIL_ON_LWCA_POLICY_CFG_ERROR_WITH_MSG(dwError, "Failed to get KeyUsagePolicy from policy config");

    if (pKeyUsagePolicyJson)
    {
        dwError = LwCAJsonGetUnsignedIntegerFromKey(pKeyUsagePolicyJson, TRUE, LWCA_ALLOWED_KEY_USAGES_KEY, &pPolicies->dwKeyUsage);
        // keyUsage can have max 9 bits set. So, the max value with 1st 9 bits set is 511
        if (dwError || pPolicies->dwKeyUsage > 511)
        {
            dwError = LWCA_POLICY_CONFIG_PARSE_ERROR;
            BAIL_ON_LWCA_ERROR_WITH_MSG(dwError, "Invalid KeyUsagePolicy value in policy config. Expected 0-511 (9 bits)");
        }
    }

    dwError = LwCAJsonGetObjectFromKey(pJson, TRUE, LWCA_CERT_DURATION_POLICY_KEY, &pCertDurationPolicyJson);
    BAIL_ON_LWCA_POLICY_CFG_ERROR_WITH_MSG(dwError, "Failed to get CertDurationPolicy value from policy config");

    if (pCertDurationPolicyJson)
    {
        dwError = LwCAJsonGetUnsignedIntegerFromKey(pCertDurationPolicyJson, TRUE, LWCA_ALLOWED_DAYS_KEY, &pPolicies->dwCertDuration);
        BAIL_ON_LWCA_POLICY_CFG_ERROR_WITH_MSG(dwError, "Failed to get MaxAllowedDays value from policy config");
    }

    *ppPolicies = pPolicies;

cleanup:
    return dwError;

error:
    LwCAPoliciesFree(pPolicies);
    if (*ppPolicies)
    {
        *ppPolicies = NULL;
    }

    goto cleanup;
}

static
DWORD
_LwCAPolicyGetSNsFromJson(
    PLWCA_JSON_OBJECT           pJson,
    PLWCA_POLICY_CFG_OBJ_ARRAY  *ppSNs
    )
{
    DWORD dwError = 0;
    PLWCA_JSON_OBJECT pCNPolicyJsonArray = NULL;
    PLWCA_POLICY_CFG_OBJ_ARRAY pObjArray = NULL;

    if (!pJson || !ppSNs)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonGetObjectFromKey(pJson, TRUE, LWCA_ALLOWED_CN_KEY, &pCNPolicyJsonArray);
    BAIL_ON_LWCA_POLICY_CFG_ERROR_WITH_MSG(dwError, "Failed to get AllowedCNs value from policy config");

    if (!LwCAJsonIsArray(pCNPolicyJsonArray))
    {
        dwError = LWCA_POLICY_CONFIG_PARSE_ERROR;
        BAIL_ON_LWCA_ERROR_WITH_MSG(dwError, "Failed to read AllowedCNs value from policy config. Expected json array");
    }

    dwError = _LwCAPolicyGetCfgObjectsFromJson(pCNPolicyJsonArray, &pObjArray);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppSNs = pObjArray;

cleanup:
    return dwError;

error:
    LwCAPolicyCfgObjArrayFree(pObjArray);
    if (ppSNs)
    {
        *ppSNs = NULL;
    }

    goto cleanup;
}

static
DWORD
_LwCAPolicyGetSANsFromJson(
    PLWCA_JSON_OBJECT           pJson,
    PLWCA_POLICY_CFG_OBJ_ARRAY  *ppSANs
    )
{
    DWORD dwError = 0;
    PLWCA_JSON_OBJECT pSANPolicyJsonArray = NULL;
    PLWCA_POLICY_CFG_OBJ_ARRAY pObjArray = NULL;

    if (!pJson || !ppSANs)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonGetObjectFromKey(pJson, TRUE, LWCA_ALLOWED_SAN_KEY, &pSANPolicyJsonArray);
    BAIL_ON_LWCA_POLICY_CFG_ERROR_WITH_MSG(dwError, "Failed to get AllowedSANs value from policy config");

    if (!LwCAJsonIsArray(pSANPolicyJsonArray))
    {
        dwError = LWCA_POLICY_CONFIG_PARSE_ERROR;
        BAIL_ON_LWCA_ERROR_WITH_MSG(dwError, "Failed to read AllowedSANs value from policy config. Expected json array");
    }

    dwError = _LwCAPolicyGetCfgObjectsFromJson(pSANPolicyJsonArray, &pObjArray);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppSANs = pObjArray;

cleanup:
    return dwError;

error:
    LwCAPolicyCfgObjArrayFree(pObjArray);
    if (ppSANs)
    {
        *ppSANs = NULL;
    }

    goto cleanup;
}

static
DWORD
_LwCAPolicyGetCfgObjectsFromJson(
    PLWCA_JSON_OBJECT           pJsonArray,
    PLWCA_POLICY_CFG_OBJ_ARRAY  *ppObjArray
    )
{
    DWORD dwError = 0;
    DWORD dwIdx = 0;
    DWORD dwArraySize = 0;
    PSTR pszType = NULL;
    PSTR pszMatch = NULL;
    PSTR pszValue = NULL;
    PSTR pszPrefix = NULL;
    PSTR pszSuffix = NULL;
    PLWCA_JSON_OBJECT pJsonValue = NULL;
    LWCA_POLICY_CFG_TYPE type = 0;
    LWCA_POLICY_CFG_MATCH match = 0;
    PLWCA_POLICY_CFG_OBJ_ARRAY pObjArray = NULL;

    dwArraySize = LwCAJsonArraySize(pJsonArray);
    if (dwArraySize <= 0)
    {
        goto ret;
    }

    dwError = LwCAPolicyCfgObjArrayAllocate(dwArraySize, &pObjArray);
    BAIL_ON_LWCA_ERROR(dwError);

    for ( ; dwIdx < dwArraySize ; ++dwIdx )
    {
        dwError = LwCAJsonArrayGetBorrowedRef(pJsonArray, dwIdx, &pJsonValue);
        BAIL_ON_LWCA_POLICY_CFG_ERROR_WITH_MSG(dwError, "Failed to get json array object from policy config");

        dwError = LwCAJsonGetStringFromKey(pJsonValue, FALSE, LWCA_CFG_OBJ_TYPE_KEY, &pszType);
        BAIL_ON_LWCA_POLICY_CFG_ERROR_WITH_MSG(dwError, "Failed to find required key 'type' in policy config object");

        dwError = LwCAJsonGetStringFromKey(pJsonValue, FALSE, LWCA_CFG_OBJ_MATCH_KEY, &pszMatch);
        BAIL_ON_LWCA_POLICY_CFG_ERROR_WITH_MSG(dwError, "Failed to find required key 'match' in policy config object");

        dwError = LwCAJsonGetStringFromKey(pJsonValue, TRUE, LWCA_CFG_OBJ_VALUE_KEY, &pszValue);
        BAIL_ON_LWCA_POLICY_CFG_ERROR_WITH_MSG(dwError, "Failed to get value of key='value' from policy config object");

        dwError = LwCAJsonGetStringFromKey(pJsonValue, TRUE, LWCA_CFG_OBJ_PREFIX_KEY, &pszPrefix);
        BAIL_ON_LWCA_POLICY_CFG_ERROR_WITH_MSG(dwError, "Failed to get value of key='prefix' from policy config object");

        dwError = LwCAJsonGetStringFromKey(pJsonValue, TRUE, LWCA_CFG_OBJ_SUFFIX_KEY, &pszSuffix);
        BAIL_ON_LWCA_POLICY_CFG_ERROR_WITH_MSG(dwError, "Failed to get value of key='suffix' from policy config object");

        dwError = _LwCAPolicyVerifyAndGetCfgObjEnums(pszType, pszMatch, pszValue, &type, &match);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = LwCAPolicyCfgObjInit(type, match, pszValue, pszPrefix, pszSuffix, &pObjArray->ppObj[dwIdx]);
        BAIL_ON_LWCA_ERROR(dwError);

        pObjArray->dwCount = dwIdx + 1;

        LWCA_SAFE_FREE_STRINGA(pszType);
        LWCA_SAFE_FREE_STRINGA(pszMatch);
        LWCA_SAFE_FREE_STRINGA(pszValue);
        LWCA_SAFE_FREE_STRINGA(pszPrefix);
        LWCA_SAFE_FREE_STRINGA(pszSuffix);
        pJsonValue = NULL;
    }

ret:
    *ppObjArray = pObjArray;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszType);
    LWCA_SAFE_FREE_STRINGA(pszMatch);
    LWCA_SAFE_FREE_STRINGA(pszValue);
    LWCA_SAFE_FREE_STRINGA(pszPrefix);
    LWCA_SAFE_FREE_STRINGA(pszSuffix);

    return dwError;

error:
    LwCAPolicyCfgObjArrayFree(pObjArray);
    if (ppObjArray)
    {
        *ppObjArray = NULL;
    }

    goto cleanup;
}

static
DWORD
_LwCAPolicyVerifyAndGetCfgObjEnums(
    PCSTR                       pcszType,
    PCSTR                       pcszMatch,
    PCSTR                       pcszValue,
    PLWCA_POLICY_CFG_TYPE       pType,
    PLWCA_POLICY_CFG_MATCH      pMatch
    )
{
    DWORD dwError = 0;
    LWCA_POLICY_CFG_TYPE type = 0;
    LWCA_POLICY_CFG_MATCH match = 0;

    if (LwCAStringCompareA(pcszMatch, LWCA_MATCH_VALUE_CONSTANT, TRUE) == 0)
    {
        match = LWCA_POLICY_CFG_MATCH_CONSTANT;

        dwError = _LwCAPolicyGetTypeEnum(pcszType, &type);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = _LwCAPolicyVerifyValidStringValue(pcszMatch, pcszValue);
        BAIL_ON_LWCA_ERROR(dwError);
    }
    else if (LwCAStringCompareA(pcszMatch, LWCA_MATCH_VALUE_ANY, TRUE) == 0)
    {
        match = LWCA_POLICY_CFG_MATCH_ANY;

        dwError = _LwCAPolicyGetTypeEnum(pcszType, &type);
        BAIL_ON_LWCA_ERROR(dwError);
    }
    else if (LwCAStringCompareA(pcszMatch, LWCA_MATCH_VALUE_REGEX, TRUE) == 0)
    {
        match = LWCA_POLICY_CFG_MATCH_REGEX;

        dwError = _LwCAPolicyGetTypeEnum(pcszType, &type);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = _LwCAPolicyVerifyValidStringValue(pcszMatch, pcszValue);
        BAIL_ON_LWCA_ERROR(dwError);
    }
    else if (LwCAStringCompareA(pcszMatch, LWCA_MATCH_VALUE_PRIVATE, TRUE) == 0)
    {
        match = LWCA_POLICY_CFG_MATCH_PRIVATE;

        dwError = _LwCAPolicyGetTypeEnum(pcszType, &type);
        BAIL_ON_LWCA_ERROR(dwError);

        if (type != LWCA_POLICY_CFG_TYPE_IP)
        {
            dwError = LWCA_POLICY_CONFIG_PARSE_ERROR;
            BAIL_ON_LWCA_ERROR_WITH_MSG(dwError, "Invalid type-match combo in policy config. match=private only valid for type=ip");
        }
    }
    else if (LwCAStringCompareA(pcszMatch, LWCA_MATCH_VALUE_PUBLIC, TRUE) == 0)
    {
        match = LWCA_POLICY_CFG_MATCH_PUBLIC;

        dwError = _LwCAPolicyGetTypeEnum(pcszType, &type);
        BAIL_ON_LWCA_ERROR(dwError);

        if (type != LWCA_POLICY_CFG_TYPE_IP)
        {
            dwError = LWCA_POLICY_CONFIG_PARSE_ERROR;
            BAIL_ON_LWCA_ERROR_WITH_MSG(dwError, "Invalid type-match combo in policy config. match=public only valid for type=ip");
        }
  }
    else if (LwCAStringCompareA(pcszMatch, LWCA_MATCH_VALUE_INZONE, TRUE) == 0)
    {
        match = LWCA_POLICY_CFG_MATCH_INZONE;

        dwError = _LwCAPolicyGetTypeEnum(pcszType, &type);
        BAIL_ON_LWCA_ERROR(dwError);

        if (type == LWCA_POLICY_CFG_TYPE_NAME)
        {
            dwError = LWCA_POLICY_CONFIG_PARSE_ERROR;
            BAIL_ON_LWCA_ERROR_WITH_MSG(dwError, "Invalid type-match combo in policy config. match=inzone, type=name not allowed");
        }
    }
    else if (LwCAStringCompareA(pcszMatch, LWCA_MATCH_VALUE_REQ_HOSTNAME, TRUE) == 0)
    {
        match = LWCA_POLICY_CFG_MATCH_REQ_HOSTNAME;

        dwError = _LwCAPolicyGetTypeEnum(pcszType, &type);
        BAIL_ON_LWCA_ERROR(dwError);

        if (type != LWCA_POLICY_CFG_TYPE_NAME)
        {
            dwError = LWCA_POLICY_CONFIG_PARSE_ERROR;
            BAIL_ON_LWCA_ERROR_WITH_MSG(dwError, "Invalid type-match combo in policy config. match=req.hostname only valid for type=name");
        }
    }
    else if (LwCAStringCompareA(pcszMatch, LWCA_MATCH_VALUE_REQ_FQDN, TRUE) == 0)
    {
        match = LWCA_POLICY_CFG_MATCH_REQ_FQDN;

        dwError = _LwCAPolicyGetTypeEnum(pcszType, &type);
        BAIL_ON_LWCA_ERROR(dwError);

        if (type != LWCA_POLICY_CFG_TYPE_FQDN)
        {
            dwError = LWCA_POLICY_CONFIG_PARSE_ERROR;
            BAIL_ON_LWCA_ERROR_WITH_MSG(dwError, "Invalid type-match combo in policy config. match=req.fqdn only valid for type=fqdn");
        }
    }
    else
    {
        dwError = LWCA_POLICY_CONFIG_PARSE_ERROR;
        BAIL_ON_LWCA_ERROR_WITH_MSG(dwError, "Invalid value for key 'match' in policy config");
    }

    *pType = type;
    *pMatch = match;

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_LwCAPolicyGetTypeEnum(
    PCSTR                       pcszType,
    PLWCA_POLICY_CFG_TYPE       pType
    )
{
    DWORD dwError = 0;
    LWCA_POLICY_CFG_TYPE type = 0;

    if (LwCAStringCompareA(pcszType, LWCA_TYPE_VALUE_IP, TRUE) == 0)
    {
        type = LWCA_POLICY_CFG_TYPE_IP;
    }
    else if (LwCAStringCompareA(pcszType, LWCA_TYPE_VALUE_NAME, TRUE) == 0)
    {
        type = LWCA_POLICY_CFG_TYPE_NAME;
    }
    else if (LwCAStringCompareA(pcszType, LWCA_TYPE_VALUE_FQDN, TRUE) == 0)
    {
        type = LWCA_POLICY_CFG_TYPE_FQDN;
    }
    else
    {
        dwError = LWCA_POLICY_CONFIG_PARSE_ERROR;
        BAIL_ON_LWCA_ERROR_WITH_MSG(dwError, "Invalid value for key 'type' in policy config");
    }

    *pType = type;

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_LwCAPolicyVerifyValidStringValue(
    PCSTR                       pcszMatch,
    PCSTR                       pcszValue
    )
{
    DWORD dwError = 0;
    PSTR pszErrMsg = NULL;

    dwError = LwCAAllocateStringPrintfA(
                &pszErrMsg,
                "Invalid object in policy config, 'value' field expected for match=%s",
                pcszMatch);
    BAIL_ON_LWCA_ERROR(dwError);

    if (IsNullOrEmptyString(pcszValue))
    {
        dwError = LWCA_POLICY_CONFIG_PARSE_ERROR;
        BAIL_ON_LWCA_ERROR_WITH_MSG(dwError, pszErrMsg);
    }

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszErrMsg);
    return dwError;

error:
    goto cleanup;
}
