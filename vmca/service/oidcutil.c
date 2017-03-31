/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ~@~\License~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ~@~\AS IS~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "includes.h"

DWORD
VMCAMakeOIDCAccessToken(
    POIDC_ACCESS_TOKEN pOIDCToken,
    PVMCA_AUTHORIZATION_PARAM pAuthorization,
    PVMCA_ACCESS_TOKEN* ppAccessToken
    );

DWORD
VMCAGetTenantSigningCert(
    PSTR* ppszSigningCertPEM
    );

DWORD
VMCAGetDomainName(
    PSTR* ppDomain
    );

DWORD
VMCAVerifyOIDC(
    PVMCA_AUTHORIZATION_PARAM pAuthorization,
    PVMCA_ACCESS_TOKEN* ppAccessToken
    )
{
    DWORD dwError = 0;
    POIDC_ACCESS_TOKEN pOIDCToken = NULL;
    PSTR pszSigningCertificatePEM = NULL;

    if (!pAuthorization)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = OidcClientGlobalInit();
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAGetTenantSigningCert(&pszSigningCertificatePEM);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = OidcAccessTokenBuild(
                            &pOIDCToken,
                            pAuthorization->pszAuthorizationToken,
                            pszSigningCertificatePEM,
                            NULL,
                            VMCA_DEFAULT_SCOPE_STRING,
                            VMCA_DEFAULT_CLOCK_TOLERANCE
                            );
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAMakeOIDCAccessToken(pOIDCToken, pAuthorization, ppAccessToken);
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

VOID
VMCAFreeOIDC(
    PVMCA_ACCESS_TOKEN pAccessToken
    )
{
    if (pAccessToken)
    {
        if (pAccessToken->pOidcToken)
        {
            OidcAccessTokenDelete(pAccessToken->pOidcToken);
            pAccessToken->pOidcToken = NULL;
        }
        VMCA_SAFE_FREE_MEMORY(pAccessToken);
    }
}

DWORD
VMCAGetTenantSigningCert(
    PSTR* ppszSigningCertPEM
    )
{
    DWORD dwError = 0;
    POIDC_SERVER_METADATA pMetadata = NULL;
    PCSTR pszServer = "localhost";
    int nPortNumber = 443;
    PSTR pszTenant = NULL;

    if (!ppszSigningCertPEM)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAGetDomainName(&pszTenant);

    dwError = OidcServerMetadataAcquire(
                                &pMetadata,
                                pszServer,
                                nPortNumber,
                                pszTenant);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppszSigningCertPEM = pMetadata->pszSigningCertificatePEM;

cleanup:
    return dwError;

error:
    if (ppszSigningCertPEM)
        *ppszSigningCertPEM = NULL;
    goto cleanup;
}

DWORD
VMCAMakeOIDCAccessToken(
    POIDC_ACCESS_TOKEN pOIDCToken,
    PVMCA_AUTHORIZATION_PARAM pAuthorization,
    PVMCA_ACCESS_TOKEN* ppAccessToken
    )
{
    DWORD dwError = 0;
    PVMCA_ACCESS_TOKEN pAccessToken = NULL;

    if (!pOIDCToken || !ppAccessToken)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocateMemory(
                            sizeof(VMCA_ACCESS_TOKEN),
                            (PVOID*)&pAccessToken
                            );
    BAIL_ON_VMCA_ERROR(dwError);

    pAccessToken->tokenType = pAuthorization->tokenType;

    pAccessToken->pszSubjectName = OidcAccessTokenGetSubject(pOIDCToken);

    if (pAccessToken->pszSubjectName == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    OidcAccessTokenGetGroups(
                      pOIDCToken,
                      &pAccessToken->pszGroups,
                      (size_t *) &pAccessToken->dwGroupSize
                      );

    if (pAccessToken->pszGroups == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    pAccessToken->pOidcToken = pOIDCToken;
    pOIDCToken = NULL;

    *ppAccessToken = pAccessToken;

cleanup:

    return dwError;
error:
    if (ppAccessToken)
        *ppAccessToken = NULL;
    if (pAccessToken)
        VMCAFreeAccessToken(pAccessToken);

    goto cleanup;
}

DWORD
VMCAGetDomainName(
    PSTR* ppDomain
    )
{
    DWORD dwError = 0;
    PSTR pDomain = NULL;
    PVMW_CFG_CONNECTION pConnection = NULL;
    PVMW_CFG_KEY pRootKey = NULL;
    PVMW_CFG_KEY pParamsKey = NULL;

    if (!ppDomain)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VmwConfigOpenConnection(&pConnection);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmwConfigOpenRootKey(
                            pConnection,
                            "HKEY_LOCAL_MACHINE",
                            0,
                            KEY_READ,
                            &pRootKey);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmwConfigOpenKey(
                            pConnection,
                            pRootKey,
                            VMAFD_CONFIG_PARAMETER_KEY_PATH,
                            0,
                            KEY_READ,
                            &pParamsKey);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmwConfigReadStringValue(
                            pParamsKey,
                            NULL,
                            VMAFD_REG_KEY_DOMAIN_NAME,
                            &pDomain);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppDomain = pDomain;
cleanup:
    if (pParamsKey)
        VmwConfigCloseKey(pParamsKey);
    if (pRootKey)
        VmwConfigCloseKey(pRootKey);
    if (pConnection)
        VmwConfigCloseConnection(pConnection);
    return dwError;
error:
    VMCA_SAFE_FREE_MEMORY(pDomain);
    if (ppDomain)
        *ppDomain = NULL;
    goto cleanup;
}
