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
LwCAConfigLoadFile(
    PCSTR                   pcszFilePath,
    PLWCA_JSON_OBJECT       *ppJsonConfig
    )
{
    DWORD                   dwError = 0;
    PLWCA_JSON_OBJECT       pJsonConfig = NULL;

    if (IsNullOrEmptyString(pcszFilePath) || !ppJsonConfig)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonLoadObjectFromFile(pcszFilePath, &pJsonConfig);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppJsonConfig = pJsonConfig;

cleanup:

    return dwError;

error:

    LwCAJsonCleanupObject(pJsonConfig);
    if (ppJsonConfig)
    {
        *ppJsonConfig = NULL;
    }

    goto cleanup;
}

DWORD
LwCAConfigGetComponent(
    PLWCA_JSON_OBJECT       pJson,
    PCSTR                   pcszComponentName,
    PLWCA_JSON_OBJECT       *ppJsonConfig
    )
{
    DWORD                   dwError = 0;
    PLWCA_JSON_OBJECT       pJsonConfig = NULL;

    if (!pJson || IsNullOrEmptyString(pcszComponentName) || !ppJsonConfig)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonGetObjectFromKey(pJson, pcszComponentName, &pJsonConfig);
    BAIL_ON_LWCA_ERROR(dwError);

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
LwCAConfigGetStringParameter(
    PLWCA_JSON_OBJECT       pJson,
    PCSTR                   pcszParameter,
    PSTR                    *ppszParameterValue
    )
{
    DWORD           dwError = 0;
    PSTR            pszParameterValue = NULL;

    if (!pJson || IsNullOrEmptyString(pcszParameter) || !ppszParameterValue)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAJsonGetStringFromKey(pJson, pcszParameter, &pszParameterValue);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppszParameterValue = pszParameterValue;

cleanup:

    return dwError;

error:

    LWCA_SAFE_FREE_STRINGA(pszParameterValue);
    if (ppszParameterValue)
    {
        *ppszParameterValue = NULL;
    }

    goto cleanup;
}

