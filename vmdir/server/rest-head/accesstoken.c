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
VmDirRESTAccessTokenInit(
    PVDIR_REST_ACCESS_TOKEN*    ppAccessToken
    )
{
    DWORD   dwError = 0;
    PVDIR_REST_ACCESS_TOKEN pAccessToken = NULL;

    if (!ppAccessToken)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_REST_ACCESS_TOKEN), (PVOID*)&pAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppAccessToken = pAccessToken;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirFreeRESTAccessToken(pAccessToken);
    goto cleanup;
}

DWORD
VmDirRESTAccessTokenParse(
    PVDIR_REST_ACCESS_TOKEN pAccessToken,
    PSTR                    pszAuthData
    )
{
    DWORD   dwError = 0;
    PSTR    pszTokenType = NULL;
    PSTR    pszAccessToken = NULL;
    PSTR    pszDomainName = NULL;
    POIDC_SERVER_METADATA   pOidcMetadata = NULL;
    POIDC_ACCESS_TOKEN      pOidcAccessToken = NULL;

    if (!pAccessToken || IsNullOrEmptyString(pszAuthData))
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pszTokenType = VmDirStringTokA(pszAuthData, " ", &pszAccessToken);
    if (IsNullOrEmptyString(pszTokenType) ||
        IsNullOrEmptyString(pszAccessToken))
    {
        dwError = VMDIR_ERROR_AUTH_BAD_DATA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (VmDirStringCompareA(pszTokenType, "Bearer", FALSE) == 0)
    {
        pAccessToken->tokenType = VDIR_REST_ACCESS_TOKEN_BEARER;
    }
    else
    {
        dwError = VMDIR_ERROR_AUTH_METHOD_NOT_SUPPORTED;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirDomainDNToName(
            BERVAL_NORM_VAL(gVmdirServerGlobals.systemDomainDN),
            &pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = OidcServerMetadataAcquire(
            &pOidcMetadata,
            VMDIR_REST_OIDC_SERVER,
            VMDIR_REST_OIDC_PORT,
            pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = OidcAccessTokenBuild(
            &pOidcAccessToken,
            pszAccessToken,
            OidcServerMetadataGetSigningCertificatePEM(pOidcMetadata),
            NULL,
            VMDIR_REST_DEFAULT_SCOPE,
            VMDIR_REST_DEFAULT_CLOCK_TOLERANCE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
            OidcAccessTokenGetSubject(pOidcAccessToken),
            &pAccessToken->pszBindUPN);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDomainName);
    OidcServerMetadataDelete(pOidcMetadata);
    OidcAccessTokenDelete(pOidcAccessToken);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );
    goto cleanup;
}

VOID
VmDirFreeRESTAccessToken(
    PVDIR_REST_ACCESS_TOKEN pAccessToken
    )
{
    if (pAccessToken)
    {
        VMDIR_SAFE_FREE_MEMORY(pAccessToken->pszBindUPN);
        VMDIR_SAFE_FREE_MEMORY(pAccessToken);
    }
}
