/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

static PCSTRING REST_SCHEME_TYPE_ENUMS[] =
{
    "http",
    "https"
};

static PCSTRING REST_ACCESS_TOKEN_TYPE_ENUMS[] =
{
    "Bearer",
    "HOK",
    "SAML",
    "SAML"
};

SSOERROR
RestBuildResourceUri(
    PCREST_CLIENT pClient,
    PCSTRING tenantPath,
    PCSTRING tenant,
    PCSTRING resourcePath,
    PCSTRING resource,
    PCSTRING subResourcePath,
    PSTRING* ppResourceUri)
{
    SSOERROR e = SSOERROR_NONE;

    PCSTRING colonDoubleSlash = "://";
    PCSTRING colon = ":";
    PCSTRING slash = "/";

    PSSO_STRING_BUILDER sb = NULL;
    PSTRING serverHost = NULL;
    PSTRING serverPort = NULL;
    PSTRING pResourceUri = NULL;

    PCSTRING domainName = "";
    int cdcFlags = 0;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenantPath) || ppResourceUri == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOStringBuilderNew(&sb);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, REST_SCHEME_TYPE_ENUMS[pClient->schemeType]);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, colonDoubleSlash);
    BAIL_ON_ERROR(e);

    if (pClient->highAvailabilityEnabled)
    {
        e = SSOCdcGetAffinitizedHost(
            pClient->pCdc,
            domainName,
            cdcFlags,
            &serverHost);
        BAIL_ON_ERROR(e);
    }
    else
    {
        e = SSOStringAllocate(pClient->serverHost, &serverHost);
        BAIL_ON_ERROR(e);
    }

    e = SSOStringBuilderAppend(sb, serverHost);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, colon);
    BAIL_ON_ERROR(e);

    e = SSOStringAllocateFromInt((int) pClient->serverPort, &serverPort);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, serverPort);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, tenantPath);
    BAIL_ON_ERROR(e);

    if (tenant != NULL)
    {
        e = SSOStringBuilderAppend(sb, slash);
        BAIL_ON_ERROR(e);

        e = SSOStringBuilderAppend(sb, tenant);
        BAIL_ON_ERROR(e);
    }

    if (resourcePath != NULL)
    {
        e = SSOStringBuilderAppend(sb, slash);
        BAIL_ON_ERROR(e);

        e = SSOStringBuilderAppend(sb, resourcePath);
        BAIL_ON_ERROR(e);
    }

    if (resource != NULL)
    {
        e = SSOStringBuilderAppend(sb, slash);
        BAIL_ON_ERROR(e);

        e = SSOStringBuilderAppend(sb, resource);
        BAIL_ON_ERROR(e);
    }

    if (subResourcePath != NULL)
    {
        e = SSOStringBuilderAppend(sb, slash);
        BAIL_ON_ERROR(e);

        e = SSOStringBuilderAppend(sb, subResourcePath);
        BAIL_ON_ERROR(e);
    }

    e = SSOStringBuilderGetString(sb, &pResourceUri);
    BAIL_ON_ERROR(e);

    *ppResourceUri = pResourceUri;

    // debug
    if (DEBUG)
    {
        fprintf(stdout, "%s\n", "resource uri:");
        fprintf(stdout, "%s\n\n", *ppResourceUri);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        SSOStringFree(pResourceUri);
    }

    // cleanup
    SSOStringFree(serverHost);
    SSOStringFree(serverPort);
    SSOStringBuilderDelete(sb);

    return e;
}

SSOERROR
RestBuildPostEntity(
    const REST_ACCESS_TOKEN* pAccessToken,
    REST_HTTP_METHOD_TYPE httpMethodType,
    PCSSO_JSON pJson,
    PCSTRING httpFormattedDate,
    PCSTRING resourceUri,
    PSTRING* ppPost)
{
    SSOERROR e = SSOERROR_NONE;
    PSSO_STRING_BUILDER sb = NULL;
    PSTRING jsonString = NULL;
    unsigned char* pMd5 = NULL;
    size_t Md5Size = 0;
    PSTRING pMd5Hex = NULL;
    unsigned char* pData = NULL;
    size_t dataSize = 0;
    unsigned char* pSHA256 = NULL;
    size_t SHA256Size = 0;
    unsigned char* pRSASignature = NULL;
    size_t RSASignatureSize = 0;
    PSTRING signedRequest = NULL;

    PCSTRING contentType = "application/json; charset=utf-8";

    PSTRING pPost = NULL;

    if (IS_NULL_OR_EMPTY_STRING(httpFormattedDate) || IS_NULL_OR_EMPTY_STRING(resourceUri) || ppPost == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOStringBuilderNew(&sb);
    BAIL_ON_ERROR(e);

    if (pAccessToken != NULL)
    {
        e = SSOStringBuilderAppend(sb, "access_token=");
        BAIL_ON_ERROR(e);

        e = SSOStringBuilderAppend(sb, pAccessToken->value);
        BAIL_ON_ERROR(e);

        e = SSOStringBuilderAppend(sb, "&");
        BAIL_ON_ERROR(e);

        e = SSOStringBuilderAppend(sb, "token_type=");
        BAIL_ON_ERROR(e);

        e = SSOStringBuilderAppend(sb, REST_ACCESS_TOKEN_TYPE_ENUMS[pAccessToken->type]);
        BAIL_ON_ERROR(e);

        if (pJson != NULL)
        {
            e = SSOJsonToString(pJson, &jsonString);
            BAIL_ON_ERROR(e);
        }
        else
        {
            // data will be a null ended empty string.
            e = SSOMemoryAllocate(1, (void**) &jsonString);
            BAIL_ON_ERROR(e);
        }

        if (pAccessToken->type == REST_ACCESS_TOKEN_TYPE_JWT_HOK || pAccessToken->type == REST_ACCESS_TOKEN_TYPE_SAML_HOK)
        {
            // create MD5 digest.
            e = SSOComputeMessageDigest(SSO_DIGEST_METHOD_MD5, (const unsigned char*) jsonString, SSOStringLength(jsonString), &pMd5, &Md5Size);
            BAIL_ON_ERROR(e);

            // hex encode MD5 digest.
            e = RestEncodeHex(pMd5, Md5Size, &pMd5Hex);
            BAIL_ON_ERROR(e);

            // build signing string.
            e = RestBuildSigningBytes(
                httpMethodType,
                pMd5Hex,
                contentType,
                httpFormattedDate,
                resourceUri,
                &pData,
                &dataSize);
            BAIL_ON_ERROR(e);

            // create SHA256 signature.
            e = SSOComputeRSASignature(
                SSO_DIGEST_METHOD_SHA256,
                pData,
                dataSize,
                pAccessToken->privateKey,
                &pRSASignature,
                &RSASignatureSize);
            BAIL_ON_ERROR(e);

            // hex encoding
            e = RestEncodeHex(pRSASignature, RSASignatureSize, &signedRequest);
            BAIL_ON_ERROR(e);

            e = SSOStringBuilderAppend(sb, "&");
            BAIL_ON_ERROR(e);

            e = SSOStringBuilderAppend(sb, "token_signature=");
            BAIL_ON_ERROR(e);

            e = SSOStringBuilderAppend(sb, signedRequest);
            BAIL_ON_ERROR(e);
        }

        if (pJson != NULL)
        {
            e = SSOStringBuilderAppend(sb, "&");
            BAIL_ON_ERROR(e);

            e = SSOStringBuilderAppend(sb, jsonString);
            BAIL_ON_ERROR(e);
        }
    }

    e = SSOStringBuilderGetString(sb, &pPost);
    BAIL_ON_ERROR(e);

    *ppPost = pPost;

    // debug
    if (DEBUG)
    {
        fprintf(stdout, "%s\n", "post message:");
        fprintf(stdout, "%s\n\n", *ppPost);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        SSOStringFree(pPost);
    }

    // cleanup
    SSOStringBuilderDelete(sb);
    SSOStringFree(jsonString);
    SSOMemoryFree(pMd5, Md5Size);
    SSOStringFree(pMd5Hex);
    SSOMemoryFree(pData, dataSize);
    SSOMemoryFree(pSHA256, SHA256Size);
    SSOMemoryFree(pRSASignature, RSASignatureSize);
    SSOStringFree(signedRequest);

    return e;
}

SSOERROR
RestAppendQueryStringOnResourceUri(
    PCSTRING key,
    PCSTRING value,
    const bool isFirstQuery,
    PSTRING pResourceUriIn,
    PSTRING* ppResourceUriOut)
{
    SSOERROR e = SSOERROR_NONE;
    PSSO_STRING_BUILDER sb = NULL;
    PSTRING pResourceUriOut = NULL;

    if (IS_NULL_OR_EMPTY_STRING(key) || IS_NULL_OR_EMPTY_STRING(value) || pResourceUriIn == NULL || ppResourceUriOut == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOStringBuilderNew(&sb);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, pResourceUriIn);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, isFirstQuery ? "?" : "&");
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, key);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, "=");
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(sb, value);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderGetString(sb, &pResourceUriOut);
    BAIL_ON_ERROR(e);

    *ppResourceUriOut = pResourceUriOut;

    // debug
    if (DEBUG)
    {
        fprintf(stdout, "%s\n", "resource uri with query string:");
        fprintf(stdout, "%s\n\n", *ppResourceUriOut);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        SSOStringFree(pResourceUriOut);
    }

    // cleanup
    SSOStringFree(pResourceUriIn);
    SSOStringBuilderDelete(sb);

    return e;
}
