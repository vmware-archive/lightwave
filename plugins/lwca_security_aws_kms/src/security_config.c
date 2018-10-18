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
_LwCASecurityConfigKMSObjectCB(
    PVOID pUserData,
    PCSTR pszKey,
    PVM_JSON_RESULT_VALUE pValue
    );

DWORD
LwCASecurityReadConfigFile(
    PCSTR pszConfigFile,
    PLWCA_SECURITY_CONFIG *ppConfig
    )
{
    DWORD dwError = 0;
    PVM_JSON_RESULT pJsonResult = NULL;
    PVM_JSON_POSITION pPosition = NULL;
    PLWCA_SECURITY_CONFIG pConfig = NULL;

    if (IsNullOrEmptyString(pszConfigFile) || !ppConfig)
    {
        dwError = LWCA_SECURITY_AWS_KMS_INVALID_PARAM;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    dwError = VmJsonResultInit(&pJsonResult);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    dwError = VmJsonResultLoadFromFile(pszConfigFile, pJsonResult);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    dwError = VmJsonResultGetRootPosition(pJsonResult, &pPosition);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    dwError = VmAllocateMemory(
                  sizeof(LWCA_SECURITY_CONFIG),
                  (PVOID *)&pConfig);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    /* {"kms":{"cmk_id":"arn","key_spec":"keyspec"}} */
    dwError = VmJsonResultIterateObjectAt(
                  pPosition,
                  pConfig,
                  _LwCASecurityConfigKMSObjectCB);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

    if (IsNullOrEmptyString(pConfig->pszCMKId) ||
        pConfig->keySpec != DATAKEYSPEC_AES_256)
    {
        dwError = LWCA_SECURITY_AWS_KMS_INVALID_CONFIG;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    *ppConfig = pConfig;

cleanup:
    VmJsonResultFreeHandle(pJsonResult);
    return dwError;

error:
    LwAwsKmsFreeConfig(pConfig);
    goto cleanup;
}

static
DWORD
_TranslateKeySpec(
    PCSTR pszKeySpec,
    DATAKEYSPEC *pKeySpec
    )
{
    DWORD dwError = 0;

    if (VmStringCompareA(pszKeySpec, AES_256_STR, TRUE) != 0)
    {
        dwError = LWCA_SECURITY_AWS_KMS_KEY_SPEC_NOT_SUPPORTED;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    *pKeySpec = DATAKEYSPEC_AES_256;

error:
    return dwError;
}

/*
 * Handles a callback for elements in "kms" object
 * we are interested in {"cmk_id":"arn","keyspec":"keyspec"}
*/
static
DWORD
_LwCASecurityConfigKeysCB(
    PVOID pUserData,
    PCSTR pszKey,
    PVM_JSON_RESULT_VALUE pValue
    )
{
    DWORD dwError = 0;
    PLWCA_SECURITY_CONFIG pConfig = pUserData;

    if (!VmStringCompareA("cmk_id", pszKey, TRUE))
    {
        if (IsNullOrEmptyString(pValue->value.pszValue))
        {
            dwError = LWCA_SECURITY_AWS_KMS_CONFIG_CMK_ID_NOT_FOUND;
            BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
        }
        dwError = VmAllocateStringA(
                      pValue->value.pszValue,
                      &pConfig->pszCMKId);
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }
    else if (!VmStringCompareA("keyspec", pszKey, TRUE))
    {
        if (IsNullOrEmptyString(pValue->value.pszValue))
        {
            dwError = LWCA_SECURITY_AWS_KMS_CONFIG_KEYSPEC_NOT_FOUND;
            BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
        }
        dwError = _TranslateKeySpec(pValue->value.pszValue, &pConfig->keySpec);
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }
    /* dont care about keys we dont know about */

cleanup:
    return dwError;

error:
    goto cleanup;
}

/*
 * Handles a callback at the top level object
 * {"kms":{}}
*/
static
DWORD
_LwCASecurityConfigKMSObjectCB(
    PVOID pUserData,
    PCSTR pszKey,
    PVM_JSON_RESULT_VALUE pValue
    )
{
    DWORD dwError = 0;

    if (VmStringCompareA("kms", pszKey, TRUE) != 0)
    {
        dwError = LWCA_SECURITY_AWS_KMS_CONFIG_ROOT_NOT_FOUND;
        BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);
    }

    dwError = VmJsonResultIterateObjectAt(
                  pValue->value.pObject,
                  pUserData,
                  _LwCASecurityConfigKeysCB);
    BAIL_ON_SECURITY_AWS_KMS_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

VOID
LwAwsKmsFreeConfig(
    PLWCA_SECURITY_CONFIG pConfig
    )
{
    if (pConfig)
    {
        VmFreeMemory(pConfig->pszCMKId);
        VmFreeMemory(pConfig);
    }
}
