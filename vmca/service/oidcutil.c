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
VMCAVerifyOIDCHOTKTokenSignature(
    PSTR                pszSignatureHex,
    PCSTR               pszHOTKPEM,
    VMCA_HTTP_REQ_OBJ*  pVMCARequest
    );

DWORD
VMCAVerifyOIDCBearerToken(
    PVMCA_AUTHORIZATION_PARAM pAuthorization,
    VMCA_HTTP_REQ_OBJ*  pVMCARequest,
    PVMCA_ACCESS_TOKEN* ppAccessToken
    )
{
    DWORD dwError = 0;
    POIDC_ACCESS_TOKEN pOIDCToken = NULL;
    PSTR pszSigningCertificatePEM = NULL;

    if (!pAuthorization || !pVMCARequest || !ppAccessToken)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAGetTenantSigningCert(&pszSigningCertificatePEM);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = OidcAccessTokenBuild(
            &pOIDCToken,
            pAuthorization->pszAuthorizationToken,
            pszSigningCertificatePEM,
            NULL,
            VMCA_DEFAULT_SCOPE_STRING,
            VMCA_DEFAULT_CLOCK_TOLERANCE);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAMakeOIDCAccessToken(pOIDCToken, pAuthorization, ppAccessToken);
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:
    VMCA_SAFE_FREE_MEMORY(pszSigningCertificatePEM);
    return dwError;

error:
    VMCA_LOG_ERROR("%s failed with error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VMCAVerifyOIDCHOTKToken(
    PVMCA_AUTHORIZATION_PARAM pAuthorization,
    VMCA_HTTP_REQ_OBJ*  pVMCARequest,
    PVMCA_ACCESS_TOKEN* ppAccessToken
    )
{
    DWORD dwError = 0;
    POIDC_ACCESS_TOKEN pOIDCToken = NULL;
    PSTR pszSigningCertificatePEM = NULL;
    PSTR pszAuthTokenCp = NULL;
    PSTR pszTokenData = NULL;       // don't free - used for strtok
    PSTR pszSignatureHex = NULL;    // don't free - used for strtok

    if (!pAuthorization || !pVMCARequest || !ppAccessToken)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAGetTenantSigningCert(&pszSigningCertificatePEM);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAAllocateStringA(pAuthorization->pszAuthorizationToken, &pszAuthTokenCp);
    BAIL_ON_VMCA_ERROR(dwError);

    pszTokenData = VMCAStringTokA(pszAuthTokenCp, ":", &pszSignatureHex);

    dwError = OidcAccessTokenBuild(
            &pOIDCToken,
            pszTokenData,
            pszSigningCertificatePEM,
            NULL,
            VMCA_DEFAULT_SCOPE_STRING,
            VMCA_DEFAULT_CLOCK_TOLERANCE);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAVerifyOIDCHOTKTokenSignature(
            pszSignatureHex,
            OidcAccessTokenGetHolderOfKeyPEM(pOIDCToken),
            pVMCARequest);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAMakeOIDCAccessToken(pOIDCToken, pAuthorization, ppAccessToken);
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:
    VMCA_SAFE_FREE_MEMORY(pszSigningCertificatePEM);
    VMCA_SAFE_FREE_MEMORY(pszAuthTokenCp);
    return dwError;

error:
    VMCA_LOG_ERROR("%s failed with error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VMCAVerifyOIDCHOTKTokenSignature(
    PSTR                pszSignatureHex,
    PCSTR               pszHOTKPEM,
    VMCA_HTTP_REQ_OBJ*  pVMCARequest
    )
{
    DWORD   dwError = 0;
    unsigned char*  signature = NULL;
    unsigned char*  sha256Body = NULL;
    size_t          signatureSize = 0;
    size_t          sha256BodySize = 0;
    PSTR    pszSha256BodyHex = NULL;
    PSTR    pszBlob = NULL;
    EVP_PKEY*   pPubKey = NULL;
    BOOLEAN     verified = FALSE;

    // HOK token must provide signature and PEM cert
    if (IsNullOrEmptyString(pszSignatureHex) ||
        IsNullOrEmptyString(pszHOTKPEM))
    {
        VMCA_LOG_ERROR("%s: Missing signature or PEM", __FUNCTION__);
        dwError = VMCA_ERROR_AUTH_BAD_DATA;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    // decode hex-encoded signature to binary
    dwError = VMCAHexStringToBytes(pszSignatureHex, &signature, &signatureSize);
    BAIL_ON_VMCA_ERROR(dwError);

    // convert PEM to public key
    dwError = VMCAConvertPEMToPublicKey(pszHOTKPEM, &pPubKey);
    BAIL_ON_VMCA_ERROR(dwError);

    // http request must have content-type and date headers
    if (IsNullOrEmptyString(pVMCARequest->pszContentType) ||
        IsNullOrEmptyString(pVMCARequest->pszDate))
    {
        VMCA_LOG_ERROR("%s: Missing content-type or date", __FUNCTION__);
        dwError = VMCA_ERROR_AUTH_BAD_DATA;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pVMCARequest->pszPayload))
    {
        // sanitize body
        VMCAStringTrimSpace(pVMCARequest->pszPayload);

        // sha256 digest of body
        dwError = VMCAComputeMessageDigest(
                EVP_sha256(),
                pVMCARequest->pszPayload,
                VMCAStringLenA(pVMCARequest->pszPayload),
                &sha256Body,
                &sha256BodySize);
        BAIL_ON_VMCA_ERROR(dwError);

        // hexadecimal encoding of the digest - use lower case
        dwError = VMCABytesToHexString(sha256Body, sha256BodySize, &pszSha256BodyHex, TRUE);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    // recreate blob to use in signature validation
    dwError = VMCAAllocateStringPrintfA(
            &pszBlob,
            "%s\n%s\n%s\n%s\n%s",
            pVMCARequest->pszMethod,
            VMCA_SAFE_STRING(pszSha256BodyHex),
            pVMCARequest->pszContentType,
            pVMCARequest->pszDate,
            pVMCARequest->pszUri);
    BAIL_ON_VMCA_ERROR(dwError);

    // verify signature
    dwError = VMCAVerifyRSASignature(
            pPubKey,
            EVP_sha256(),
            pszBlob,
            VMCAStringLenA(pszBlob),
            signature,
            signatureSize,
            &verified);
    BAIL_ON_VMCA_ERROR(dwError);

    if (!verified)
    {
        VMCA_LOG_ERROR("%s: Bad signature", __FUNCTION__);
        dwError = VMCA_ERROR_AUTH_BAD_DATA;
        BAIL_ON_VMCA_ERROR(dwError);
    }

cleanup:
    VMCA_SAFE_FREE_MEMORY(signature);
    VMCA_SAFE_FREE_MEMORY(sha256Body);
    VMCA_SAFE_FREE_MEMORY(pszSha256BodyHex);
    VMCA_SAFE_FREE_MEMORY(pszBlob);
    EVP_PKEY_free(pPubKey);
    return dwError;

error:
    VMCA_LOG_ERROR("%s failed with error (%d)", __FUNCTION__, dwError);
    // TODO - log error and let it through for now
    dwError = 0;
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
    DWORD   dwError = 0;
    POIDC_SERVER_METADATA pMetadata = NULL;
    PCSTR   pszServer = "localhost";
    int     nPortNumber = 443;
    PSTR    pszTenant = NULL;
    PSTR    pszSigningCertPEM = NULL;

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
                                pszTenant,
                                NULL /* pszTlsCAPath: NULL means skip TLS validation, pass LIGHTWAVE_TLS_CA_PATH to turn on */);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAAllocateStringA(
            OidcServerMetadataGetSigningCertificatePEM(pMetadata),
            &pszSigningCertPEM);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppszSigningCertPEM = pszSigningCertPEM;

cleanup:
    VMCA_SAFE_FREE_MEMORY(pszTenant);
    OidcServerMetadataDelete(pMetadata);
    return dwError;

error:
    VMCA_LOG_ERROR("%s failed with error (%d)", __FUNCTION__, dwError);
    VMCA_SAFE_FREE_MEMORY(pszSigningCertPEM);
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
    VMCA_LOG_ERROR("%s failed with error (%d)", __FUNCTION__, dwError);
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
    VMCA_LOG_ERROR("%s failed with error (%d)", __FUNCTION__, dwError);
    VMCA_SAFE_FREE_MEMORY(pDomain);
    if (ppDomain)
        *ppDomain = NULL;
    goto cleanup;
}
