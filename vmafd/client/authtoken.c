/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the �~@~\License�~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an �~@~\AS IS�~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "includes.h"

static
DWORD
_VmAfdOidcClientBuild(
    PCSTR           pcszServer,
    PCSTR           pcszDomain,
    PCSTR           pcszLocalCertsPath,
    POIDC_CLIENT*   ppClient
    )
{
    DWORD           dwError         = 0;
    DWORD           dwSSOError      = 0;
    PSTR            pszServer       = NULL;
    PSTR            pszDomain       = NULL;
    POIDC_CLIENT    pClient         = NULL;

    if (!ppClient)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pcszServer) || !IsNullOrEmptyString(pcszDomain))
    {
        if (!IsNullOrEmptyString(pcszServer) && !IsNullOrEmptyString(pcszDomain))
        {
            dwError = VmAfdAllocateStringA(pcszServer, &pszServer);
            BAIL_ON_VMAFD_ERROR(dwError);

            dwError = VmAfdAllocateStringA(pcszDomain, &pszDomain);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        else
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }
    else
    {
        dwError = VmAfdGetDCNameA(
                    VMAFD_DEFAULT_SERVER,
                    &pszServer
                    );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdGetDomainNameA(
                    VMAFD_DEFAULT_SERVER,
                    &pszDomain
                    );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwSSOError = OidcClientBuild(
                &pClient,
                pszServer,
                VMAFD_OIDC_PORT,
                pszDomain,
                NULL,
                pcszLocalCertsPath
                );
    if (dwSSOError)
    {
        dwError = VmAfdOidcToVmafdError(dwSSOError);
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppClient = pClient;

cleanup:
    VMAFD_SAFE_FREE_STRINGA(pszServer);
    VMAFD_SAFE_FREE_STRINGA(pszDomain);

    return dwError;

error:
    if (pClient)
    {
        VmAfdOidcClientDelete(pClient);
    }

    goto cleanup;
}

DWORD
VmAfdOidcClientBuild(
    PCSTR           pcszServer,     /* IN OPTIONAL (Required if Domain is specified) */
    PCSTR           pcszDomain,     /* IN OPTIONAL (Required if Server is specified)*/
    POIDC_CLIENT*   ppClient        /* OUT */
    )
{
    return _VmAfdOidcClientBuild(
               pcszServer,
               pcszDomain,
               LIGHTWAVE_TLS_CA_PATH,
               ppClient);
}

DWORD
VmAfdOidcClientAcquireToken(
    POIDC_CLIENT    pClient,                /* IN */
    PCSTR           pcszDomain,             /* IN OPTIONAL (Required if username/password is given) */
    PCSTR           pcszUsername,           /* IN OPTIONAL (Required if domain/password is given) */
    PCSTR           pcszPassword,           /* IN OPTIONAL (Required if username/domain is given) */
    PCSTR           pcszScope,              /* IN */
    PSTR*           ppszAccessToken         /* OUT */
    )
{
    DWORD                           dwError                 = 0;
    DWORD                           dwSSOError              = 0;
    PSTR                            pszUsername             = NULL;
    PSTR                            pszPassword             = NULL;
    PSTR                            pszDomain               = NULL;
    PSTR                            pszUpn                  = NULL;
    PSTR                            pszAccessToken          = NULL;
    POIDC_TOKEN_SUCCESS_RESPONSE    pTokenSuccessResponse   = NULL;
    POIDC_ERROR_RESPONSE            pTokenErrorResponse     = NULL;

    if (!ppszAccessToken || !pClient || IsNullOrEmptyString(pcszScope)) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pcszUsername) || !IsNullOrEmptyString(pcszPassword) || !IsNullOrEmptyString(pcszDomain))
    {
        if (!IsNullOrEmptyString(pcszUsername) && !IsNullOrEmptyString(pcszPassword) && !IsNullOrEmptyString(pcszDomain))
        {
            dwError = VmAfdAllocateStringA(pcszDomain, &pszDomain);
            BAIL_ON_VMAFD_ERROR(dwError);

            dwError = VmAfdAllocateStringA(pcszUsername, &pszUsername);
            BAIL_ON_VMAFD_ERROR(dwError);

            dwError = VmAfdAllocateStringA(pcszPassword, &pszPassword);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        else
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }
    else
    {
        dwError = VmAfdGetDomainNameA(
                    VMAFD_DEFAULT_SERVER,
                    &pszDomain
                    );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdGetMachineAccountInfoA(
                    VMAFD_DEFAULT_SERVER,
                    &pszUsername,
                    &pszPassword
                    );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringAVsnprintf(
                &pszUpn,
                "%s@%s",
                pszUsername,
                pszDomain
                );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwSSOError = OidcClientAcquireTokensByPassword(
                pClient,
                pszUpn,
                pszPassword,
                pcszScope,
                &pTokenSuccessResponse,
                &pTokenErrorResponse
                );
    if (dwSSOError)
    {
        dwError = VmAfdOidcToVmafdError(dwSSOError);
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pTokenErrorResponse)
    {
        dwError = VMAFD_OIDC_ERROR_RESPONSE;
        VmAfdLog(VMAFD_DEBUG_ERROR,
                "%s: Failed to acquire OIDC token. OIDC Error: %s, ErrorDescription: %s",
                __FUNCTION__,
                OidcErrorResponseGetError(pTokenErrorResponse),
                OidcErrorResponseGetErrorDescription(pTokenErrorResponse));
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!pTokenSuccessResponse)
    {
        dwError = VMAFD_OIDC_EMPTY_RESPONSE;
        VmAfdLog(VMAFD_DEBUG_ERROR,
                "%s: Failed to acquire OIDC token. Empty Response from server.",
                __FUNCTION__);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringA(
                OidcTokenSuccessResponseGetAccessToken(pTokenSuccessResponse),
                &pszAccessToken
                );
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszAccessToken = pszAccessToken;

cleanup:
    VMAFD_SAFE_FREE_STRINGA(pszUpn);
    VMAFD_SAFE_FREE_STRINGA(pszUsername);
    VMAFD_SECURE_FREE_STRINGA(pszPassword);
    VMAFD_SAFE_FREE_STRINGA(pszDomain);

    if (pTokenSuccessResponse)
    {
        OidcTokenSuccessResponseDelete(pTokenSuccessResponse);
    }
    if (pTokenErrorResponse)
    {
        OidcErrorResponseDelete(pTokenErrorResponse);
    }

    return dwError;

error:
    VMAFD_SAFE_FREE_STRINGA(pszAccessToken);

    goto cleanup;
}

VOID
VmAfdOidcClientDelete(
    POIDC_CLIENT pClient
    )
{
    if (pClient)
    {
        OidcClientDelete(pClient);
    }
}

DWORD
VmAfdAcquireTokenForVmDirREST(
    PCSTR pszServer,
    PCSTR pszDomain,
    PCSTR pszUser,
    PCSTR pszPass,
    PCSTR pszLocalCertsPath,
    PSTR *ppszToken
    )
{
    DWORD dwError = 0;
    PSTR pszToken = NULL;
    POIDC_CLIENT pClient = NULL;

    if (IsNullOrEmptyString(pszServer) ||
        IsNullOrEmptyString(pszDomain) ||
        IsNullOrEmptyString(pszUser) ||
        IsNullOrEmptyString(pszPass) ||
        !ppszToken)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = _VmAfdOidcClientBuild(
                  pszServer,
                  pszDomain,
                  pszLocalCertsPath,
                  &pClient);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdOidcClientAcquireToken(
                  pClient,
                  pszDomain,
                  pszUser,
                  pszPass,
                  "openid rs_vmdir",
                  &pszToken);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszToken = pszToken;

cleanup:
    VmAfdOidcClientDelete(pClient);
    return dwError;

error:
    VMAFD_SAFE_FREE_STRINGA(pszToken);
    goto cleanup;
}
