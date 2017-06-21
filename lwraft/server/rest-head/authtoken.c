/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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
VmDirRESTAuthTokenInit(
    PVDIR_REST_AUTH_TOKEN*  ppAuthToken
    )
{
    DWORD   dwError = 0;
    PVDIR_REST_AUTH_TOKEN   pAuthToken = NULL;

    if (!ppAuthToken)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_REST_AUTH_TOKEN), (PVOID*)&pAuthToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppAuthToken = pAuthToken;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    VmDirFreeRESTAuthToken(pAuthToken);
    goto cleanup;
}

DWORD
VmDirRESTAuthTokenParse(
    PVDIR_REST_AUTH_TOKEN   pAuthToken,
    PCSTR                   pszAuthData
    )
{
    DWORD   dwError = 0;
    DWORD   dwAFDError = 0;
    DWORD   dwOIDCError = 0;
    PSTR    pszAuthDataCp = NULL;
    PSTR    pszTokenType = NULL;
    PSTR    pszAccessToken = NULL;
    PSTR    pszDCName = NULL;
    PSTR    pszDomainName = NULL;
    POIDC_SERVER_METADATA   pOidcMetadata = NULL;
    POIDC_ACCESS_TOKEN      pOidcAccessToken = NULL;

    if (!pAuthToken || IsNullOrEmptyString(pszAuthData))
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringA(pszAuthData, &pszAuthDataCp);
    BAIL_ON_VMDIR_ERROR(dwError);

    pszTokenType = VmDirStringTokA(pszAuthDataCp, " ", &pszAccessToken);
    if (IsNullOrEmptyString(pszTokenType) ||
        IsNullOrEmptyString(pszAccessToken))
    {
        dwError = VMDIR_ERROR_AUTH_BAD_DATA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (VmDirStringCompareA(pszTokenType, "Bearer", FALSE) == 0)
    {
        pAuthToken->tokenType = VDIR_REST_AUTH_TOKEN_BEARER;
    }
    else if (VmDirStringCompareA(pszTokenType, "hotk-pk", FALSE) == 0)
    {
        pAuthToken->tokenType = VDIR_REST_AUTH_TOKEN_HOTK;
    }
    else
    {
        dwError = VMDIR_ERROR_AUTH_METHOD_NOT_SUPPORTED;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwAFDError = gpVdirVmAfdAPI->pfnGetDCName(NULL, &pszDCName);
    dwError = dwAFDError ? VMDIR_ERROR_AFD_UNAVAILABLE : 0;
    BAIL_ON_VMDIR_ERROR(dwError);

    dwAFDError = gpVdirVmAfdAPI->pfnGetDomainName(NULL, &pszDomainName);
    dwError = dwAFDError ? VMDIR_ERROR_AFD_UNAVAILABLE : 0;
    BAIL_ON_VMDIR_ERROR(dwError);

    dwOIDCError = OidcServerMetadataAcquire(
            &pOidcMetadata,
            pszDCName,
            VMDIR_REST_OIDC_PORT,
            pszDomainName,
            NULL /* pszTlsCAPath: NULL means skip TLS validation, pass LIGHTWAVE_TLS_CA_PATH to turn on */);
    dwError = dwOIDCError ? VMDIR_ERROR_OIDC_UNAVAILABLE : 0;
    BAIL_ON_VMDIR_ERROR(dwError);

    dwOIDCError = OidcAccessTokenBuild(
            &pOidcAccessToken,
            pszAccessToken,
            OidcServerMetadataGetSigningCertificatePEM(pOidcMetadata),
            NULL,
            VMDIR_REST_DEFAULT_SCOPE,
            VMDIR_REST_DEFAULT_CLOCK_TOLERANCE);
    dwError = dwOIDCError ? VMDIR_ERROR_OIDC_UNAVAILABLE : 0;
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
            OidcAccessTokenGetSubject(pOidcAccessToken),
            &pAuthToken->pszBindUPN);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszAuthDataCp);
    VMDIR_SAFE_FREE_MEMORY(pszDCName);
    VMDIR_SAFE_FREE_MEMORY(pszDomainName);
    OidcServerMetadataDelete(pOidcMetadata);
    OidcAccessTokenDelete(pOidcAccessToken);
    return dwError;

error:
    // don't log error if VMDIR_ERROR_AUTH_METHOD_NOT_SUPPORTED,
    // because it will try other available auth methods
    if (dwError != VMDIR_ERROR_AUTH_METHOD_NOT_SUPPORTED)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "%s failed, error (%d) AFD error (%d) OIDC error (%d)",
                __FUNCTION__,
                dwError,
                dwAFDError,
                dwOIDCError);
    }
    goto cleanup;
}

VOID
VmDirFreeRESTAuthToken(
    PVDIR_REST_AUTH_TOKEN   pAuthToken
    )
{
    if (pAuthToken)
    {
        VMDIR_SAFE_FREE_MEMORY(pAuthToken->pszBindUPN);
        VMDIR_SAFE_FREE_MEMORY(pAuthToken);
    }
}
