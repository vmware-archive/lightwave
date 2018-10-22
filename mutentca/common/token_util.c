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
LwCAGetAccessToken(
    PCSTR   pcszServer,
    PCSTR   pcszDomain,
    PCSTR   pcszOidcScope,
    PSTR    *ppszToken
    )
{
    DWORD                           dwError = 0;
    PLWCA_CERTIFICATE               pszCertificate = NULL;
    PSTR                            pszKey = NULL;
    PSTR                            pszToken = NULL;
    PVECS_CERT_ENTRY_A              pEntry = NULL;
    PSTR                            pszCommonName = NULL;
    POIDC_CLIENT                    pOidcClient = NULL;
    POIDC_TOKEN_SUCCESS_RESPONSE    pTokenSuccess = NULL;
    POIDC_ERROR_RESPONSE            pTokenError = NULL;

    if (IsNullOrEmptyString(pcszServer) ||
        IsNullOrEmptyString(pcszDomain) ||
        IsNullOrEmptyString(pcszOidcScope) ||
        !ppszToken)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAGetVecsMutentCACert(&pszCertificate, &pszKey);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAGetCommonNameFromSubject(
        pszCertificate,
        &pszCommonName
        );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = OidcClientBuild(
        &pOidcClient,
        pcszServer,
        LWCA_OIDC_PORT,
        pcszDomain,
        NULL,
        NULL
        );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = OidcClientAcquireTokensBySolutionUserCredentials(
        pOidcClient,
        pszCommonName,
        pszKey,
        pcszOidcScope,
        &pTokenSuccess,
        &pTokenError
        );
    BAIL_ON_LWCA_ERROR(dwError);

    if (pTokenSuccess) {
        dwError = LwCAAllocateStringA(
            OidcTokenSuccessResponseGetAccessToken(pTokenSuccess),
            &pszToken
            );
        BAIL_ON_LWCA_ERROR(dwError);
    } else if (pTokenError) {
        LWCA_LOG_ERROR(
            "Failed to acquire Access Token - Error: (%s). Error Desc: %s",
            OidcErrorResponseGetError(pTokenError),
            OidcErrorResponseGetErrorDescription(pTokenError)
            );
        dwError = LWCA_OIDC_RESPONSE_ERROR;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppszToken = pszToken;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszCertificate);
    LWCA_SAFE_FREE_STRINGA(pszKey);
    LWCA_SAFE_FREE_STRINGA(pszCommonName);
    if (pEntry) {
        VecsFreeCertEntryA(pEntry);
    }
    if (pOidcClient) {
        OidcClientDelete(pOidcClient);
    }
    OidcTokenSuccessResponseDelete(pTokenSuccess);
    OidcErrorResponseDelete(pTokenError);

    return dwError;

error:
    LWCA_SAFE_FREE_STRINGA(pszToken);
    if (ppszToken)
    {
        *ppszToken = NULL;
    }
    goto cleanup;
}
