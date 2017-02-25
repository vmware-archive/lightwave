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

/*
 * jwt = b64(json_object).b64(json_object).b64(signature_bytes) where b64 is base64url
 */

static const PCSTRING s_pszJsonHeaderString = "{\"alg\":\"RS256\"}";

static
SSOERROR
SSOJwtSplitIntoParts(
    PCSTRING psz,
    PSTRING* ppsz1,
    PSTRING* ppsz2,
    PSTRING* ppsz3,
    PSTRING* ppszSignedData)
{
    SSOERROR e = SSOERROR_NONE;

    PSTRING psz1 = NULL;
    PSTRING psz2 = NULL;
    PSTRING psz3 = NULL;
    PSTRING pszSignedData = NULL;

    int firstDotIndex = -1;
    int secondDotIndex = -1;
    int i = 0;

    size_t length = SSOStringLength(psz);

    for (i = 0; i < length; i++)
    {
        // psz is UTF-8 but we can do this since it's base64url decoded which means all chars are ASCII
        if ('.' == psz[i])
        {
            if (-1 == firstDotIndex)
            {
                firstDotIndex = i;
            }
            else if (-1 == secondDotIndex)
            {
                secondDotIndex = i;
            }
            else
            {
                e = SSOERROR_INVALID_ARGUMENT;
                BAIL_ON_ERROR(e);
            }
        }
    }

    if (-1 == secondDotIndex ||
        0 == firstDotIndex ||
        firstDotIndex + 1 == secondDotIndex ||
        length - 1 == secondDotIndex)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOStringAllocateSubstring(psz, 0, firstDotIndex - 1, &psz1);
    BAIL_ON_ERROR(e);
    e = SSOStringAllocateSubstring(psz, firstDotIndex + 1, secondDotIndex - 1, &psz2);
    BAIL_ON_ERROR(e);
    e = SSOStringAllocateSubstring(psz, secondDotIndex + 1, length - 1, &psz3);
    BAIL_ON_ERROR(e);
    e = SSOStringAllocateSubstring(psz, 0, secondDotIndex - 1, &pszSignedData);
    BAIL_ON_ERROR(e);

    *ppsz1 = psz1;
    *ppsz2 = psz2;
    *ppsz3 = psz3;
    *ppszSignedData = pszSignedData;

error:

    if (e != SSOERROR_NONE)
    {
        SSOStringFree(psz1);
        SSOStringFree(psz2);
        SSOStringFree(psz3);
        SSOStringFree(pszSignedData);
    }

   return e;
}

SSOERROR
SSOJwtParse(
    PSSO_JWT* pp,
    PCSTRING psz)
{
    SSOERROR e = SSOERROR_NONE;
    PSSO_JWT p = NULL;
    PSTRING psz1 = NULL;
    PSTRING psz2 = NULL;
    PSTRING psz3 = NULL;
    PSTRING pszJsonHeader = NULL;
    PSTRING pszJsonPayload = NULL;
    PSSO_JSON pJsonHeader = NULL;
    PSSO_JSON pJsonValue = NULL;
    PSTRING pszSignatureAlgorithm = NULL;

    ASSERT_NOT_NULL(pp);
    ASSERT_NOT_NULL(psz);

    e = SSOMemoryAllocate(sizeof(SSO_JWT), (void**) &p);
    BAIL_ON_ERROR(e);

    e = SSOJwtSplitIntoParts(psz, &psz1, &psz2, &psz3, &p->pszSignedData);
    BAIL_ON_ERROR(e);

    // part1: json header, check that this is an RS256 signed JWT
    e = SSOBase64UrlDecodeToString(psz1, &pszJsonHeader);
    BAIL_ON_ERROR(e);

    e = SSOJsonParse(&pJsonHeader, pszJsonHeader);
    BAIL_ON_ERROR(e);

    e = SSOJsonObjectGet(pJsonHeader, "alg", &pJsonValue);
    BAIL_ON_ERROR(e);

    e = SSOJsonStringValue(pJsonValue, &pszSignatureAlgorithm);
    BAIL_ON_ERROR(e);

    if (!SSOStringEqual(pszSignatureAlgorithm, "RS256"))
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    // part2: json payload
    e = SSOBase64UrlDecodeToString(psz2, &pszJsonPayload);
    BAIL_ON_ERROR(e);

    e = SSOJsonParse(&p->pJsonPayload, pszJsonPayload);
    BAIL_ON_ERROR(e);

    // part3: signature
    e = SSOBase64UrlDecodeToBytes(psz3, &p->pSignatureBytes, &p->signatureByteCount);
    BAIL_ON_ERROR(e);

    *pp = p;

error:

    SSOStringFree(psz1);
    SSOStringFree(psz2);
    SSOStringFree(psz3);
    SSOStringFree(pszJsonHeader);
    SSOStringFree(pszJsonPayload);
    SSOJsonDelete(pJsonHeader);
    SSOJsonDelete(pJsonValue);
    SSOStringFree(pszSignatureAlgorithm);

    if (e != SSOERROR_NONE)
    {
        SSOJwtDelete(p);
    }

    return e;
}

void
SSOJwtDelete(
    PSSO_JWT p)
{
    if (p != NULL)
    {
        SSOJsonDelete(p->pJsonPayload);
        SSOStringFree(p->pszSignedData);
        SSOMemoryFreeArray(p->pSignatureBytes, p->signatureByteCount, sizeof(unsigned char));
        SSOMemoryFree(p, sizeof(SSO_JWT));
    }
}

SSOERROR
SSOJwtGetStringClaim(
    PCSSO_JWT p,
    PCSTRING pszKey,
    PSTRING* ppszValue /* OUT */)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING pszValue = NULL;
    PSSO_JSON pJsonValue = NULL;

    ASSERT_NOT_NULL(p);
    ASSERT_NOT_NULL(pszKey);
    ASSERT_NOT_NULL(ppszValue);

    e = SSOJsonObjectGet(p->pJsonPayload, pszKey, &pJsonValue);
    BAIL_ON_ERROR(e);

    e = SSOJsonStringValue(pJsonValue, &pszValue);
    BAIL_ON_ERROR(e);

    *ppszValue = pszValue;

error:

    SSOJsonDelete(pJsonValue);

    return e;
}

SSOERROR
SSOJwtGetStringArrayClaim(
    PCSSO_JWT p,
    PCSTRING pszKey,
    PSTRING** pppszValue, /* OUT */
    size_t* pArraySize)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING* ppszValue = NULL;
    PSSO_JSON pJsonArray = NULL;
    PSSO_JSON pJsonArrayElement = NULL;
    size_t arraySize = 0;
    size_t i = 0;

    ASSERT_NOT_NULL(p);
    ASSERT_NOT_NULL(pszKey);
    ASSERT_NOT_NULL(pppszValue);
    ASSERT_NOT_NULL(pArraySize);

    e = SSOJsonObjectGet(p->pJsonPayload, pszKey, &pJsonArray);
    BAIL_ON_ERROR(e);

    e = SSOJsonArraySize(pJsonArray, &arraySize);
    BAIL_ON_ERROR(e);

    e = SSOMemoryAllocateArray(arraySize, sizeof(PSTRING), (void**) &ppszValue);
    BAIL_ON_ERROR(e);

    for (i = 0; i < arraySize; i++)
    {
        e = SSOJsonArrayGet(pJsonArray, i, &pJsonArrayElement);
        BAIL_ON_ERROR(e);

        e = SSOJsonStringValue(pJsonArrayElement, ppszValue + i);
        BAIL_ON_ERROR(e);

        SSOJsonDelete(pJsonArrayElement);
        pJsonArrayElement = NULL;
    }

    *pppszValue = ppszValue;
    *pArraySize = arraySize;

error:

    SSOJsonDelete(pJsonArray);
    SSOJsonDelete(pJsonArrayElement);

    if (e != SSOERROR_NONE)
    {
        SSOMemoryFreeArray(ppszValue, arraySize, sizeof(PSTRING));
    }

    return e;
}

SSOERROR
SSOJwtGetLongClaim(
    PCSSO_JWT p,
    PCSTRING pszKey,
    SSO_LONG* pValue)
{
    SSOERROR e = SSOERROR_NONE;
    SSO_LONG value = 0;
    PSSO_JSON pJsonValue = NULL;

    ASSERT_NOT_NULL(p);
    ASSERT_NOT_NULL(pszKey);
    ASSERT_NOT_NULL(pValue);

    e = SSOJsonObjectGet(p->pJsonPayload, pszKey, &pJsonValue);
    BAIL_ON_ERROR(e);

    e = SSOJsonLongValue(pJsonValue, &value);
    BAIL_ON_ERROR(e);

    *pValue = value;

error:

    SSOJsonDelete(pJsonValue);

    return e;
}

SSOERROR
SSOJwtGetJsonClaim(
    PCSSO_JWT p,
    PCSTRING pszKey,
    PSTRING* ppszValue /* OUT */)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING pszValue = NULL;
    PSSO_JSON pJsonValue = NULL;

    ASSERT_NOT_NULL(p);
    ASSERT_NOT_NULL(pszKey);
    ASSERT_NOT_NULL(ppszValue);

    e = SSOJsonObjectGet(p->pJsonPayload, pszKey, &pJsonValue);
    BAIL_ON_ERROR(e);

    e = SSOJsonToString(pJsonValue, &pszValue);
    BAIL_ON_ERROR(e);

    *ppszValue = pszValue;

error:

    SSOJsonDelete(pJsonValue);

    return e;
}

SSOERROR
SSOJwtHasClaim(
    PCSSO_JWT p,
    PCSTRING pszKey,
    bool* pHasClaim)
{
    SSOERROR e = SSOERROR_NONE;
    bool isNull = false;
    PSSO_JSON pJsonValue = NULL;

    ASSERT_NOT_NULL(p);
    ASSERT_NOT_NULL(pszKey);
    ASSERT_NOT_NULL(pHasClaim);

    e = SSOJsonObjectGet(p->pJsonPayload, pszKey, &pJsonValue);
    BAIL_ON_ERROR(e);

    e = SSOJsonIsNull(pJsonValue, &isNull);
    BAIL_ON_ERROR(e);

    *pHasClaim = !isNull;

error:

    SSOJsonDelete(pJsonValue);

    return e;
}

SSOERROR
SSOJwtVerifySignature(
    PCSSO_JWT p,
    PCSTRING pszCertificatePEM,
    bool* pValid)
{
    SSOERROR e = SSOERROR_NONE;
    bool valid = false;

    ASSERT_NOT_NULL(p);
    ASSERT_NOT_NULL(pszCertificatePEM);
    ASSERT_NOT_NULL(pValid);

    e = SSOVerifyRSASignature(
        SSO_DIGEST_METHOD_SHA256,
        (unsigned char*) p->pszSignedData,  // const unsigned char* pData,
        SSOStringLength(p->pszSignedData),  // size_t dataSize,
        p->pSignatureBytes,                 // const unsigned char* pRSASignature,
        p->signatureByteCount,              // size_t rsaSignatureSize,
        pszCertificatePEM,
        &valid);
    BAIL_ON_ERROR(e);

    *pValid = valid;

error:

    return e;
}

SSOERROR
SSOJwtCreateSignedJwtString(
    PCSTRING pszJsonPayloadString,
    PCSTRING pszPrivateKeyPEM,
    PSTRING* ppsz /* OUT */)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING psz = NULL;
    PSTRING pszJsonHeaderStringB64 = NULL;
    PSTRING pszJsonPayloadStringB64 = NULL;
    PSTRING pszToSign = NULL;
    PSTRING pszSignatureB64 = NULL;
    PSSO_STRING_BUILDER pSB = NULL;
    unsigned char* pSignatureBytes = NULL;
    size_t signatureSize = 0;

    ASSERT_NOT_NULL(pszJsonPayloadString);
    ASSERT_NOT_NULL(pszPrivateKeyPEM);
    ASSERT_NOT_NULL(ppsz);

    e = SSOBase64UrlEncodeToString(
        (unsigned char*) s_pszJsonHeaderString,
        SSOStringLength(s_pszJsonHeaderString),
        &pszJsonHeaderStringB64);
    BAIL_ON_ERROR(e);

    e = SSOBase64UrlEncodeToString(
        (unsigned char*) pszJsonPayloadString,
        SSOStringLength(pszJsonPayloadString),
        &pszJsonPayloadStringB64);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderNew(&pSB);
    BAIL_ON_ERROR(e);
    e = SSOStringBuilderAppend(pSB, pszJsonHeaderStringB64);
    BAIL_ON_ERROR(e);
    e = SSOStringBuilderAppend(pSB, ".");
    BAIL_ON_ERROR(e);
    e = SSOStringBuilderAppend(pSB, pszJsonPayloadStringB64);
    BAIL_ON_ERROR(e);
    e = SSOStringBuilderGetString(pSB, &pszToSign);
    BAIL_ON_ERROR(e);

    e = SSOComputeRSASignature(
        SSO_DIGEST_METHOD_SHA256,
        (const unsigned char*) pszToSign,
        SSOStringLength(pszToSign),
        pszPrivateKeyPEM,
        &pSignatureBytes,
        &signatureSize);
    BAIL_ON_ERROR(e);

    e = SSOBase64UrlEncodeToString(
        pSignatureBytes,
        signatureSize,
        &pszSignatureB64);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(pSB, ".");
    BAIL_ON_ERROR(e);
    e = SSOStringBuilderAppend(pSB, pszSignatureB64);
    BAIL_ON_ERROR(e);
    e = SSOStringBuilderGetString(pSB, &psz);
    BAIL_ON_ERROR(e);

    *ppsz = psz;

error:

    if (e != SSOERROR_NONE)
    {
        SSOStringFree(psz);
    }

    SSOStringFree(pszJsonHeaderStringB64);
    SSOStringFree(pszJsonPayloadStringB64);
    SSOStringFree(pszToSign);
    SSOStringFree(pszSignatureB64);
    SSOStringBuilderDelete(pSB);
    SSOMemoryFreeArray(pSignatureBytes, signatureSize, sizeof(unsigned char));

    return e;
}
