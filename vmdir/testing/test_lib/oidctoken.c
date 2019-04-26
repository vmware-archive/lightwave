/*
 * Copyright © 2098 VMware, Inc.  All Rights Reserved.
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

/*
 * Acquire oidc token
 */
DWORD
VmDirTestOidcTokenAcquire(
    PCSTR       pszSSOServer,
    DWORD       dwSSOPort,
    PVMDIR_OIDC_ACQUIRE_TOKEN_INFO pTokenInfo,
    PSTR*       ppszToken
    )
{
    DWORD dwError = 0;
    DWORD dwSSOError = 0;
    POIDC_CLIENT    pClient = NULL;
    POIDC_TOKEN_SUCCESS_RESPONSE    pSuccessResponse = NULL;
    POIDC_ERROR_RESPONSE           pFailResponse = NULL;
    PSTR  pszToken = NULL;

    if (IsNullOrEmptyString(pszSSOServer) ||
        !ppszToken ||
        !pTokenInfo ||
        !pTokenInfo->pszDomain ||
        !pTokenInfo->pszScope)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_PARAMETER);
    }

    dwSSOError = OidcClientBuild(
        &pClient,
        pszSSOServer,
        dwSSOPort,
        pTokenInfo->pszDomain,
        NULL,
        pTokenInfo->pszLocalTLSCertPath);
    BAIL_ON_SSO_ERROR(dwSSOError);

    switch (pTokenInfo->method)
    {
        case METHOD_PASSWORD:
            dwSSOError = OidcClientAcquireTokensByPassword(
                pClient,
                pTokenInfo->pszUPN,
                pTokenInfo->pszPassword,
                pTokenInfo->pszScope,
                &pSuccessResponse,
                &pFailResponse);
            BAIL_ON_SSO_ERROR(dwSSOError);

            break;
        default:
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_AUTH_METHOD_NOT_SUPPORTED);
    }

    if (pFailResponse)
    {
        goto ssoerror;
    }

    if (!pSuccessResponse)
    {
        printf("%s no odic success response\n", __FUNCTION__);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
    }

    dwSSOError = VmDirAllocateStringA(
        OidcTokenSuccessResponseGetAccessToken(pSuccessResponse),
        &pszToken);
    BAIL_ON_SSO_ERROR(dwSSOError);

    *ppszToken = pszToken;

cleanup:
    if (pSuccessResponse)
    {
        OidcTokenSuccessResponseDelete(pSuccessResponse);
    }
    if (pFailResponse)
    {
        OidcErrorResponseDelete(pFailResponse);
    }
    if (pClient)
    {
        OidcClientDelete(pClient);
    }
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszToken);
    goto cleanup;

ssoerror:
    if (pFailResponse)
    {
        printf("%s OIDC Error (%s/%s)\n", __FUNCTION__,
            OidcErrorResponseGetError(pFailResponse),
            OidcErrorResponseGetErrorDescription(pFailResponse));
    }
    printf("%s OIDC Error (%d)\n", __FUNCTION__, dwSSOError);

    // TODO map error
    dwError = dwSSOError;
    goto error;
}
