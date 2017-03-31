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

SSOERROR
SSOJwkParseFromSet(
    PSSO_JWK* pp,
    PCSTRING pszJwkSet)
{
    SSOERROR e = SSOERROR_NONE;

    PSSO_JWK p = NULL;
    PSSO_JSON pJson = NULL;
    PSSO_JSON pJsonArrayKeys = NULL;
    PSSO_JSON pJsonObjectKey = NULL;
    PSSO_JSON pJsonValueUse = NULL;
    PSSO_JSON pJsonValueKeyType = NULL;
    PSSO_JSON pJsonValueAlgorithm = NULL;
    PSSO_JSON pJsonValueModulus = NULL;
    PSSO_JSON pJsonValueExponent = NULL;
    PSSO_JSON pJsonArrayCertificates = NULL;
    PSSO_JSON pJsonValueCertificate = NULL;
    PSTRING pszStringValue = NULL;
    bool x5cNull = false;

    ASSERT_NOT_NULL(pp);
    ASSERT_NOT_NULL(pszJwkSet);

    e = SSOMemoryAllocate(sizeof(SSO_JWK), (void**) &p);
    BAIL_ON_ERROR(e);

    e = SSOJsonParse(&pJson, pszJwkSet);
    BAIL_ON_ERROR(e);
    e = SSOJsonObjectGet(pJson, "keys", &pJsonArrayKeys);
    BAIL_ON_ERROR(e);
    e = SSOJsonArrayGet(pJsonArrayKeys, 0, &pJsonObjectKey);
    BAIL_ON_ERROR(e);

    e = SSOJsonObjectGet(pJsonObjectKey, "use", &pJsonValueUse);
    BAIL_ON_ERROR(e);
    e = SSOJsonStringValue(pJsonValueUse, &pszStringValue);
    BAIL_ON_ERROR(e);
    if (!SSOStringEqual(pszStringValue, "sig"))
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }
    SSOStringFree(pszStringValue);
    pszStringValue = NULL;

    e = SSOJsonObjectGet(pJsonObjectKey, "kty", &pJsonValueKeyType);
    BAIL_ON_ERROR(e);
    e = SSOJsonStringValue(pJsonValueKeyType, &pszStringValue);
    BAIL_ON_ERROR(e);
    if (!SSOStringEqual(pszStringValue, "RSA"))
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }
    SSOStringFree(pszStringValue);
    pszStringValue = NULL;

    e = SSOJsonObjectGet(pJsonObjectKey, "alg", &pJsonValueAlgorithm);
    BAIL_ON_ERROR(e);
    e = SSOJsonStringValue(pJsonValueAlgorithm, &pszStringValue);
    BAIL_ON_ERROR(e);
    if (!SSOStringEqual(pszStringValue, "RS256"))
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }
    SSOStringFree(pszStringValue);
    pszStringValue = NULL;

    e = SSOJsonObjectGet(pJsonObjectKey, "n", &pJsonValueModulus);
    BAIL_ON_ERROR(e);
    e = SSOJsonStringValue(pJsonValueModulus, &p->pszModulus);
    BAIL_ON_ERROR(e);

    e = SSOJsonObjectGet(pJsonObjectKey, "e", &pJsonValueExponent);
    BAIL_ON_ERROR(e);
    e = SSOJsonStringValue(pJsonValueExponent, &p->pszExponent);
    BAIL_ON_ERROR(e);

    e = SSOJsonObjectGet(pJsonObjectKey, "x5c", &pJsonArrayCertificates);
    BAIL_ON_ERROR(e);
    e = SSOJsonIsNull(pJsonArrayCertificates, &x5cNull);
    BAIL_ON_ERROR(e);
    if (!x5cNull)
    {
        // x5c is optional, it will be present in the jwks endpoint response, but not in HOK tokens
        e = SSOJsonArrayGet(pJsonArrayCertificates, 0, &pJsonValueCertificate);
        BAIL_ON_ERROR(e);
        e = SSOJsonStringValue(pJsonValueCertificate, &p->pszCertificate);
        BAIL_ON_ERROR(e);
    }

    *pp = p;

error:

    if (e != SSOERROR_NONE)
    {
        SSOJwkDelete(p);
    }

    SSOJsonDelete(pJson);
    SSOJsonDelete(pJsonArrayKeys);
    SSOJsonDelete(pJsonObjectKey);
    SSOJsonDelete(pJsonValueUse);
    SSOJsonDelete(pJsonValueKeyType);
    SSOJsonDelete(pJsonValueAlgorithm);
    SSOJsonDelete(pJsonValueModulus);
    SSOJsonDelete(pJsonValueExponent);
    SSOJsonDelete(pJsonArrayCertificates);
    SSOJsonDelete(pJsonValueCertificate);
    SSOStringFree(pszStringValue);

    return e;
}

void
SSOJwkDelete(
    PSSO_JWK p)
{
    if (p != NULL)
    {
        SSOStringFree(p->pszModulus);
        SSOStringFree(p->pszExponent);
        SSOStringFree(p->pszCertificate);
        SSOMemoryFree(p, sizeof(SSO_JWK));
    }
}

PCSTRING
SSOJwkGetCertificate(
    PCSSO_JWK p)
{
    ASSERT_NOT_NULL(p);
    return p->pszCertificate;
}

static
SSOERROR
SSOJwkBase64UrlToBigNum(
    PCSTRING pszBase64Url,
    BIGNUM** ppBigNum /* OUT */)
{
    SSOERROR e = SSOERROR_NONE;

    BIGNUM* pBigNum = NULL;
    unsigned char* pBytes = NULL;
    size_t byteCount = 0;

    e = SSOBase64UrlDecodeToBytes(
        pszBase64Url,
        &pBytes,
        &byteCount);
    BAIL_ON_ERROR(e);

    pBigNum = BN_bin2bn(pBytes, byteCount, NULL);
    if (NULL == pBigNum)
    {
        e = SSOERROR_OPENSSL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    *ppBigNum = pBigNum;

error:

    SSOMemoryFreeArray(pBytes, byteCount, 1);

    return e;
}

SSOERROR
SSOJwkToPublicKeyPEM(
    PCSSO_JWK p,
    PSTRING* ppsz /* OUT */)
{
    SSOERROR e = SSOERROR_NONE;

    PSTRING psz = NULL;
    EVP_PKEY* pKey = NULL;
    RSA* pRsa = NULL;
    BIGNUM* pModulusBigNum = NULL;
    BIGNUM* pExponentBigNum = NULL;
    BIO* pBio = NULL;
    BUF_MEM *pBuffMem = NULL;

    ASSERT_NOT_NULL(p);
    ASSERT_NOT_NULL(ppsz);

    e = SSOJwkBase64UrlToBigNum(p->pszModulus, &pModulusBigNum);
    BAIL_ON_ERROR(e);
    e = SSOJwkBase64UrlToBigNum(p->pszExponent, &pExponentBigNum);
    BAIL_ON_ERROR(e);

    pRsa = RSA_new();
    if (NULL == pRsa)
    {
        e = SSOERROR_OPENSSL_FAILURE;
        BAIL_ON_ERROR(e);
    }
    pRsa->n = pModulusBigNum;
    pRsa->e = pExponentBigNum;

    pKey = EVP_PKEY_new();
    if (NULL == pKey)
    {
        e = SSOERROR_OPENSSL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    if (0 == EVP_PKEY_assign_RSA(pKey, pRsa))
    {
        e = SSOERROR_OPENSSL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    pBio = BIO_new(BIO_s_mem());
    if (NULL == pBio)
    {
        e = SSOERROR_OPENSSL_FAILURE;
        BAIL_ON_ERROR(e);
    }
    if (0 == PEM_write_bio_PUBKEY(pBio, pKey))
    {
        e = SSOERROR_OPENSSL_FAILURE;
        BAIL_ON_ERROR(e);
    }

    BIO_get_mem_ptr(pBio, &pBuffMem);
    e = SSOMemoryAllocate(pBuffMem->length + 1, (void**) &psz); // +1 for string terminator
    BAIL_ON_ERROR(e);
    SSOMemoryCopy(psz, pBuffMem->data, pBuffMem->length);

    *ppsz = psz;

error:

    if (e != SSOERROR_NONE)
    {
        SSOStringFree(psz);
    }

    EVP_PKEY_free(pKey); // this will free pRsa, which in turn will free pModulusBigNum & pExponentBigNum
    BIO_free(pBio);

    return e;
}

SSOERROR
SSOJwkToCertificatePEM(
    PCSSO_JWK p,
    PSTRING* ppsz /* OUT */)
{
    SSOERROR e = SSOERROR_NONE;

    PSTRING psz = NULL;
    PSTRING pszSubstring = NULL;
    PSSO_STRING_BUILDER pSB = NULL;
    const int chunkWidth = 64;

    int start = 0;
    int length = 0;

    ASSERT_NOT_NULL(p);
    ASSERT_NOT_NULL(ppsz);
    ASSERT_NOT_NULL(p->pszCertificate); // x5c is optional

    length = SSOStringLength(p->pszCertificate);

    e = SSOStringBuilderNew(&pSB);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(pSB, "-----BEGIN CERTIFICATE-----\n");
    BAIL_ON_ERROR(e);

    while (start + chunkWidth <= length)
    {
        e = SSOStringAllocateSubstring(p->pszCertificate, start, start + chunkWidth - 1, &pszSubstring);
        BAIL_ON_ERROR(e);
        e = SSOStringBuilderAppend(pSB, pszSubstring);
        BAIL_ON_ERROR(e);
        e = SSOStringBuilderAppend(pSB, "\n");
        BAIL_ON_ERROR(e);
        SSOStringFree(pszSubstring);
        pszSubstring = NULL;

        start += chunkWidth;
    }

    if (start < length)
    {
        e = SSOStringAllocateSubstring(p->pszCertificate, start, length - 1, &pszSubstring);
        BAIL_ON_ERROR(e);
        e = SSOStringBuilderAppend(pSB, pszSubstring);
        BAIL_ON_ERROR(e);
        e = SSOStringBuilderAppend(pSB, "\n");
        BAIL_ON_ERROR(e);
        SSOStringFree(pszSubstring);
        pszSubstring = NULL;
    }

    e = SSOStringBuilderAppend(pSB, "-----END CERTIFICATE-----");
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderGetString(pSB, &psz);
    BAIL_ON_ERROR(e);

    *ppsz = psz;

error:

    if (e != SSOERROR_NONE)
    {
        SSOStringFree(psz);
    }

    SSOStringFree(pszSubstring);
    SSOStringBuilderDelete(pSB);

    return e;
}
