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
_VmAfdJsonResultPasswordCB(
    PVOID pUserData,
    PCSTR pszKey,
    PVM_JSON_RESULT_VALUE pValue
    )
{
    DWORD dwError = 0;
    PSTR *ppszPassword = NULL;
    PSTR pszPassword = NULL;

    if (!pUserData || !pszKey || !pValue)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!VmAfdStringCompareA("password", pszKey, TRUE))
    {
        if (pValue->nType == JSON_RESULT_STRING)
        {
            dwError = VmAfdAllocateStringA(pValue->value.pszValue, &pszPassword);
            BAIL_ON_VMAFD_ERROR(dwError);

            ppszPassword = (PSTR *)pUserData;
            *ppszPassword = pszPassword;
        }
    }

cleanup:
    return dwError;

error:
    VMAFD_SAFE_FREE_MEMORY(pszPassword);
    goto cleanup;
}


static
DWORD
_VmAfdGetHttpResultPassword(
    PCSTR pszResult,
    PSTR *ppszPassword
    )
{
    DWORD dwError = 0;
    PSTR pszPassword = NULL;
    PVM_JSON_RESULT pJsonResult = NULL;
    PVM_JSON_POSITION pPosition = NULL;

    dwError = VmJsonResultInit(&pJsonResult);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmJsonResultLoadString(pszResult, pJsonResult);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmJsonResultGetRootPosition(pJsonResult, &pPosition);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmJsonResultIterateObjectAt(
                  pPosition,
                  &pszPassword,
                 _VmAfdJsonResultPasswordCB);
    BAIL_ON_VMAFD_ERROR(dwError);

    /* pszPassword could be NULL if force is not set or not aged */
    /* caller handles this scenario */
    *ppszPassword = pszPassword;

cleanup:
    VmJsonResultFreeHandle(pJsonResult);
    return dwError;

error:
    VMAFD_SAFE_FREE_MEMORY(pszPassword);
    goto cleanup;
}

DWORD
VmAfdRestPasswordRefresh(
    PCSTR pszServer,
    PCSTR pszDomain,
    PCSTR pszUser,
    PCSTR pszPass,
    BOOLEAN bForce,
    BOOLEAN bInsecure,
    PSTR *ppszNewPass
    )
{
    DWORD dwError = 0;
    PSTR pszToken = NULL;
    PSTR pszNewPass = NULL;
    PSTR pszUrl = NULL;
    PVM_HTTP_CLIENT pHttpClient = NULL;
    PSTR pszParamString = NULL;
    PCSTR pszResult = NULL;
    PWSTR pwszCAPath = NULL;
    PSTR pszCAPath = NULL;

    if (IsNullOrEmptyString(pszServer) ||
        IsNullOrEmptyString(pszDomain) ||
        IsNullOrEmptyString(pszUser) ||
        IsNullOrEmptyString(pszPass) ||
        !ppszNewPass)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (bInsecure)
    {
        pszCAPath = NULL;
    }
    else
    {
        dwError = VmAfSrvGetCAPath(&pwszCAPath);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringAFromW(pwszCAPath, &pszCAPath);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    /* acquire token */
    dwError = VmAfdAcquireOIDCToken(
                  pszServer,
                  pszDomain,
                  pszUser,
                  pszPass,
                  pszCAPath,
                  VMAFD_OIDC_VMDIR_SCOPE,
                  &pszToken);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                  &pszParamString,
                  "?force=%s",
                  bForce?"true":"false");
    BAIL_ON_VMAFD_ERROR(dwError);

    /* make rest url */
    dwError = VmFormatUrl(
                  "https",
                  pszServer,
                  VMDIR_REST_API_HTTPS_PORT,
                  VMDIR_REST_API_BASE"/"VMDIR_REST_API_PASSWORD_REFRESH_CMD,
                  pszParamString,
                  &pszUrl);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmHttpClientInit(&pHttpClient, pszCAPath);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmHttpClientSetToken(pHttpClient,
                                   VMHTTP_TOKEN_TYPE_BEARER,
                                   pszToken);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmHttpClientPerform(pHttpClient, VMHTTP_METHOD_POST, pszUrl);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmHttpClientGetResult(pHttpClient, &pszResult);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = _VmAfdGetHttpResultPassword(pszResult, &pszNewPass);
    BAIL_ON_VMAFD_ERROR(dwError);

    /* pszPassword could be NULL if force is not set or not aged */
    /* caller handles this scenario */
    *ppszNewPass = pszNewPass;

cleanup:
    VmHttpClientFreeHandle(pHttpClient);
    VMAFD_SAFE_FREE_MEMORY(pszCAPath);
    VMAFD_SAFE_FREE_MEMORY(pwszCAPath);
    VMAFD_SAFE_FREE_STRINGA(pszParamString);
    VMAFD_SAFE_FREE_STRINGA(pszUrl);
    VMAFD_SAFE_FREE_STRINGA(pszToken);
    return dwError;

error:
    VMAFD_SAFE_FREE_STRINGA(pszNewPass);
    goto cleanup;
}
