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
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
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
    PSTR    pszAuthDataCp = NULL;
    PSTR    pszTokenType = NULL;
    PSTR    pszTokenData = NULL;
    PSTR    pszSignature = NULL;

    if (!pAuthToken || IsNullOrEmptyString(pszAuthData))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateStringA(pszAuthData, &pszAuthDataCp);
    BAIL_ON_VMDIR_ERROR(dwError);

    pszTokenType = VmDirStringTokA(pszAuthDataCp, " ", &pszTokenData);
    if (IsNullOrEmptyString(pszTokenType) ||
        IsNullOrEmptyString(pszTokenData))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_AUTH_BAD_DATA);
    }

    if (VmDirStringCompareA(pszTokenType, "Bearer", FALSE) == 0)
    {
        pAuthToken->tokenType = VDIR_REST_AUTH_TOKEN_BEARER;

        dwError = VmDirAllocateStringA(pszTokenData, &pAuthToken->pszAccessToken);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (VmDirStringCompareA(pszTokenType, "hotk-pk", FALSE) == 0)
    {
        pAuthToken->tokenType = VDIR_REST_AUTH_TOKEN_HOTK;

        pszTokenData = VmDirStringTokA(pszTokenData, ":", &pszSignature);

        dwError = VmDirAllocateStringA(pszTokenData, &pAuthToken->pszAccessToken);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_AUTH_METHOD_NOT_SUPPORTED);
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszAuthDataCp);
    return dwError;

error:
    // don't log error if VMDIR_ERROR_AUTH_METHOD_NOT_SUPPORTED,
    // because it will try other available auth methods
    if (dwError != VMDIR_ERROR_AUTH_METHOD_NOT_SUPPORTED)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "%s failed, error (%d)",
                __FUNCTION__,
                dwError);
    }
    goto cleanup;
}

DWORD
VmDirRESTAuthTokenValidate(
    PVDIR_REST_AUTH_TOKEN   pAuthToken
    )
{
    DWORD   i = 0;
    DWORD   dwError = 0;
    DWORD   dwOIDCError = 0;
    size_t  numOfGroups = 0;
    BOOLEAN bCacheRefreshed = FALSE;
    PSTR    pszTemp = NULL;
    PSTR    pszOIDCSigningCertPEM = NULL;
    const PSTRING* ppGroupsArray = NULL;
    POIDC_ACCESS_TOKEN  pOidcAccessToken = NULL;

    if (!pAuthToken)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

retry:
    VMDIR_SAFE_FREE_MEMORY(pszOIDCSigningCertPEM);
    dwError = VmDirRESTCacheGetOIDCSigningCertPEM(
            gpVdirRestCache, &pszOIDCSigningCertPEM);

    // cache isn't setup - cannot support token auth
    dwError = dwError ? VMDIR_ERROR_AUTH_METHOD_NOT_SUPPORTED : 0;
    BAIL_ON_VMDIR_ERROR(dwError);

    OidcAccessTokenDelete(pOidcAccessToken);
    dwOIDCError = OidcAccessTokenBuild(
            &pOidcAccessToken,
            pAuthToken->pszAccessToken,
            pszOIDCSigningCertPEM,
            NULL,
            VMDIR_REST_DEFAULT_SCOPE,
            VMDIR_REST_DEFAULT_CLOCK_TOLERANCE);
    dwError = VmDirOidcToVmdirError(dwOIDCError);

    // no need to refresh cache if user provided a bad token
    if (dwError && dwError != VMDIR_ERROR_AUTH_BAD_DATA && !bCacheRefreshed)
    {
        dwError = VmDirRESTCacheRefresh(gpVdirRestCache);
        BAIL_ON_VMDIR_ERROR(dwError);
        bCacheRefreshed = TRUE;

        goto retry;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
            OidcAccessTokenGetSubject(pOidcAccessToken),
            &pAuthToken->pszBindUPN);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Log if the user belongs to LW admin group

    OidcAccessTokenGetGroups(
            pOidcAccessToken, &ppGroupsArray, &numOfGroups);

    if (ppGroupsArray)
    {
        for (i = 0; i < numOfGroups; i++)
        {
            // Get string after domain name
            pszTemp = VmDirStringChrA(ppGroupsArray[i], '\\');

            if (pszTemp && ++pszTemp)
            {
                if (!VmDirStringCompareA(pszTemp, "Administrators",FALSE))
                {
                    VMDIR_LOG_VERBOSE(
                            VMDIR_LOG_MASK_ALL,
                            "OIDC token for user: %s has lightwave admin group membership",
                            pAuthToken->pszBindUPN);
                }
            }
        }
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszOIDCSigningCertPEM);
    OidcAccessTokenDelete(pOidcAccessToken);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d) OIDC error (%d)",
            __FUNCTION__,
            dwError,
            dwOIDCError);
    goto cleanup;
}

VOID
VmDirFreeRESTAuthToken(
    PVDIR_REST_AUTH_TOKEN   pAuthToken
    )
{
    if (pAuthToken)
    {
        VMDIR_SAFE_FREE_MEMORY(pAuthToken->pszAccessToken);
        VMDIR_SAFE_FREE_MEMORY(pAuthToken->pszBindUPN);
        VMDIR_SAFE_FREE_MEMORY(pAuthToken);
    }
}
