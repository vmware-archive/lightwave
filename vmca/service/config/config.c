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
VMCAConfigLoadFile(
    PCSTR                   pcszFilePath,
    PVMCA_JSON_OBJECT       *ppJsonConfig
    )
{
    DWORD                   dwError = 0;
    PVMCA_JSON_OBJECT       pJsonConfig = NULL;

    if (IsNullOrEmptyString(pcszFilePath) || !ppJsonConfig)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAJsonLoadObjectFromFile(pcszFilePath, &pJsonConfig);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppJsonConfig = pJsonConfig;

cleanup:

    return dwError;

error:

    VMCAJsonCleanupObject(pJsonConfig);
    if (ppJsonConfig)
    {
        *ppJsonConfig = NULL;
    }

    goto cleanup;
}

DWORD
VMCAConfigGetComponent(
    PVMCA_JSON_OBJECT       pJson,
    PCSTR                   pcszComponentName,
    PVMCA_JSON_OBJECT       *ppJsonConfig
    )
{
    DWORD                   dwError = 0;
    PVMCA_JSON_OBJECT       pJsonConfig = NULL;

    if (!pJson || IsNullOrEmptyString(pcszComponentName) || !ppJsonConfig)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAJsonGetObjectFromKey(pJson, pcszComponentName, &pJsonConfig);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppJsonConfig = pJsonConfig;

cleanup:

    return dwError;

error:

    if (ppJsonConfig)
    {
        *ppJsonConfig = NULL;
    }

    goto cleanup;
}

DWORD
VMCAConfigGetStringParameter(
    PVMCA_JSON_OBJECT       pJson,
    PCSTR                   pcszParameter,
    PSTR                    *ppszParameterValue
    )
{
    DWORD           dwError = 0;
    PSTR            pszParameterValue = NULL;

    if (!pJson || IsNullOrEmptyString(pcszParameter) || !ppszParameterValue)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAJsonGetStringFromKey(pJson, pcszParameter, &pszParameterValue);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppszParameterValue = pszParameterValue;

cleanup:

    return dwError;

error:

    VMCA_SAFE_FREE_STRINGA(pszParameterValue);
    if (ppszParameterValue)
    {
        *ppszParameterValue = NULL;
    }

    goto cleanup;
}

