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
    PSTR    pszSignatureHex = NULL;

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

        pszTokenData = VmDirStringTokA(pszTokenData, ":", &pszSignatureHex);
        if (IsNullOrEmptyString(pszTokenData))
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_AUTH_BAD_DATA);
        }

        dwError = VmDirAllocateStringA(pszTokenData, &pAuthToken->pszAccessToken);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (!IsNullOrEmptyString(pszSignatureHex))
        {
            dwError = VmDirAllocateStringA(pszSignatureHex, &pAuthToken->pszSignatureHex);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
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

    dwOIDCError = OidcAccessTokenParse(
            &pOidcAccessToken,
            pAuthToken->pszAccessToken);
    dwError = VmDirOidcToVmdirError(dwOIDCError);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
            OidcAccessTokenGetSubject(pOidcAccessToken),
            &pAuthToken->pszBindUPN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
            OidcAccessTokenGetHolderOfKeyPEM(pOidcAccessToken),
            &pAuthToken->pszHOTKPEM);
    BAIL_ON_VMDIR_ERROR(dwError);

retry:
    VMDIR_SAFE_FREE_MEMORY(pszOIDCSigningCertPEM);
    dwError = VmDirRESTCacheGetOIDCSigningCertPEM(gpVdirRestCache, &pszOIDCSigningCertPEM);
    if (dwError == VMDIR_ERROR_NOT_FOUND && !bCacheRefreshed)
    {
        dwError = VmDirRESTCacheRefresh(gpVdirRestCache);
        BAIL_ON_VMDIR_ERROR(dwError);
        bCacheRefreshed = TRUE;
        goto retry;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwOIDCError = OidcAccessTokenValidate(
            pOidcAccessToken,
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

    // log if the user belongs to LW admin group
    OidcAccessTokenGetGroups(pOidcAccessToken, &ppGroupsArray, &numOfGroups);
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

DWORD
VmDirRESTAuthTokenValidatePOP(
    PVDIR_REST_AUTH_TOKEN   pAuthToken,
    PVDIR_REST_OPERATION    pRestOp
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

    if (!pAuthToken || !pRestOp)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    // we don't need POP validation unless it's a HOK token
    if (pAuthToken->tokenType != VDIR_REST_AUTH_TOKEN_HOTK)
    {
        goto cleanup;
    }

    // HOK token must provide signature and PEM cert
    if (IsNullOrEmptyString(pAuthToken->pszSignatureHex) ||
        IsNullOrEmptyString(pAuthToken->pszHOTKPEM))
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: Missing signature or PEM", __FUNCTION__);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_AUTH_BAD_DATA);
    }

    // decode hex-encoded signature to binary
    dwError = VmDirHexStringToBytes(pAuthToken->pszSignatureHex, &signature, &signatureSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    // convert PEM to public key
    dwError = VmDirConvertPEMToPublicKey(pAuthToken->pszHOTKPEM, &pPubKey);
    BAIL_ON_VMDIR_ERROR(dwError);

    // http request must have content-type and date headers
    if (IsNullOrEmptyString(pRestOp->pszContentType) ||
        IsNullOrEmptyString(pRestOp->pszDate))
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: Missing content-type or date", __FUNCTION__);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_AUTH_BAD_DATA);
    }

    if (!IsNullOrEmptyString(pRestOp->pszBody))
    {
        // sanitize body
        VmDirStringTrimSpace(pRestOp->pszBody);

        // sha256 digest of body
        dwError = VmDirComputeMessageDigest(
                EVP_sha256(),
                pRestOp->pszBody,
                VmDirStringLenA(pRestOp->pszBody),
                &sha256Body,
                &sha256BodySize);
        BAIL_ON_VMDIR_ERROR(dwError);

        // hexadecimal encoding of the digest - use lower case
        dwError = VmDirBytesToHexString(sha256Body, sha256BodySize, &pszSha256BodyHex, TRUE);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // recreate blob to use in signature validation
    dwError = VmDirAllocateStringPrintf(
            &pszBlob,
            "%s\n%s\n%s\n%s\n%s",
            pRestOp->pszMethod,
            VDIR_SAFE_STRING(pszSha256BodyHex),
            pRestOp->pszContentType,
            pRestOp->pszDate,
            pRestOp->pszURI);
    BAIL_ON_VMDIR_ERROR(dwError);

    // verify signature
    dwError = VmDirVerifyRSASignature(
            pPubKey,
            EVP_sha256(),
            pszBlob,
            VmDirStringLenA(pszBlob),
            signature,
            signatureSize,
            &verified);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!verified)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: Bad signature", __FUNCTION__);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_AUTH_BAD_DATA);
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(signature);
    VMDIR_SAFE_FREE_MEMORY(sha256Body);
    VMDIR_SAFE_FREE_MEMORY(pszSha256BodyHex);
    VMDIR_SAFE_FREE_MEMORY(pszBlob);
    EVP_PKEY_free(pPubKey);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    // TODO - log error and let it through for now
    dwError = 0;
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
        VMDIR_SAFE_FREE_MEMORY(pAuthToken->pszSignatureHex);
        VMDIR_SAFE_FREE_MEMORY(pAuthToken->pszBindUPN);
        VMDIR_SAFE_FREE_MEMORY(pAuthToken->pszHOTKPEM);
        VMDIR_SAFE_FREE_MEMORY(pAuthToken);
    }
}
