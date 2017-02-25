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

#ifndef _COMMON_H_
#define _COMMON_H_

// SSO_HTTP_CLIENT

SSOERROR
SSOHttpClientGlobalInit();

void
SSOHttpClientGlobalCleanup();

SSOERROR
SSOHttpClientNew(
    PSSO_HTTP_CLIENT* pp);

void
SSOHttpClientDelete(
    PSSO_HTTP_CLIENT p);

SSOERROR
SSOHttpClientSendPostForm(
    PCSSO_HTTP_CLIENT p,
    PCSTRING pszUrl,
    PCSTRING* ppszHeaders, /* OPT */
    size_t headerCount,
    PSSO_KEY_VALUE_PAIR* ppPairs,
    size_t numPairs,
    PSTRING* ppszOutResponse, /* OUT */
    long* pOutHttpStatusCode /* OUT */);

SSOERROR
SSOHttpClientSendPostJson(
    PCSSO_HTTP_CLIENT p,
    PCSTRING pszUrl,
    PCSTRING* ppszHeaders, /* OPT */
    size_t headerCount,
    PCSTRING pszJsonBody,
    PSTRING* ppszOutResponse, /* OUT */
    long* pOutHttpStatusCode /* OUT */);

SSOERROR
SSOHttpClientSendPutJson(
    PCSSO_HTTP_CLIENT p,
    PCSTRING pszUrl,
    PCSTRING* ppszHeaders, /* OPT */
    size_t headerCount,
    PCSTRING pszJsonBody,
    PSTRING* ppszOutResponse, /* OUT */
    long* pOutHttpStatusCode /* OUT */);

SSOERROR
SSOHttpClientSendDelete(
    PCSSO_HTTP_CLIENT p,
    PCSTRING pszUrl,
    PCSTRING* ppszHeaders, /* OPT */
    size_t headerCount,
    PCSTRING pszJsonBody,
    PSTRING* ppszOutResponse, /* OUT */
    long* pOutHttpStatusCode /* OUT */);

SSOERROR
SSOHttpClientSendGet(
    PCSSO_HTTP_CLIENT p,
    PCSTRING pszUrl,
    PCSTRING* ppszHeaders, /* OPT */
    size_t headerCount,
    PSTRING* ppszOutResponse, /* OUT */
    long* pOutHttpStatusCode /* OUT */);

// SSO_KEY_VALUE_PAIR

SSOERROR
SSOKeyValuePairNew(
    PSSO_KEY_VALUE_PAIR* pp,
    PCSTRING pszKey,
    PCSTRING pszValue);

void
SSOKeyValuePairDelete(
    PSSO_KEY_VALUE_PAIR p);

PCSTRING
SSOKeyValuePairGetKey(
    PCSSO_KEY_VALUE_PAIR p);

PCSTRING
SSOKeyValuePairGetValue(
    PCSSO_KEY_VALUE_PAIR p);

// memory

SSOERROR
SSOMemoryAllocate(
    size_t size,
    void** pp /* OUT */);

SSOERROR
SSOMemoryAllocateArray(
    size_t count,
    size_t size,
    void** pp /* OUT */);

SSOERROR
SSOMemoryReallocate(
    size_t size,
    void** pp /* IN,OUT */);

void
SSOMemoryCopy(
    void* destination,
    const void* source,
    size_t size);

void
SSOMemoryClear(
    void* p, /* OPT */
    size_t size);

void
SSOMemoryFree(
    void* p, /* OPT */
    size_t size);

void
SSOMemoryFreeArray(
    void* p /* OPT */,
    size_t count,
    size_t size);

void
SSOMemoryFreeArrayOfObjects(
    void** pp /* OPT */,
    size_t count,
    GenericDestructorFunction pDestructor);

// SSO_STRING_BUILDER

SSOERROR
SSOStringBuilderNew(
    PSSO_STRING_BUILDER* pp);

void
SSOStringBuilderDelete(
    PSSO_STRING_BUILDER p);

SSOERROR
SSOStringBuilderAppend(
    PSSO_STRING_BUILDER p,
    PCSTRING pszString);

SSOERROR
SSOStringBuilderGetString(
    PCSSO_STRING_BUILDER p,
    PSTRING* ppsz /* OUT */);

// strings

SSOERROR
SSOStringAllocate(
    PCSTRING psz,
    PSTRING* ppsz /* OUT */);

SSOERROR
SSOStringAllocateFromInt(
    int i,
    PSTRING* ppsz /* OUT */);

SSOERROR
SSOStringAllocateSubstring(
    PCSTRING psz,
    size_t startIndex,
    size_t endIndex,
    PSTRING* ppsz /* OUT */);

SSOERROR
SSOStringConcatenate(
    PCSTRING psz1,
    PCSTRING psz2,
    PSTRING* ppsz /* OUT */);

SSOERROR
SSOStringReplace(
    PCSTRING pszInput,
    PCSTRING pszFind,
    PCSTRING pszReplace,
    PSTRING* ppsz /* OUT */);

size_t
SSOStringLength(
    PCSTRING psz);

bool
SSOStringEqual(
    PCSTRING psz1, /* OPT */
    PCSTRING psz2  /* OPT */);

void
SSOStringFree(
    PSTRING psz /* OPT */);

void
SSOStringFreeAndClear(
    PSTRING psz /* OPT */);

// Json wrapper calls

void
SSOJsonDelete(
    PSSO_JSON pJson);

SSOERROR
SSOJsonObjectNew(
    PSSO_JSON* ppJson);

SSOERROR
SSOJsonArrayNew(
    PSSO_JSON* ppJson);

SSOERROR
SSOJsonStringNew(
    PSSO_JSON* ppJson,
    PCSTRING value);

SSOERROR
SSOJsonIntegerNew(
    PSSO_JSON* ppJson,
    INTEGER value);

SSOERROR
SSOJsonLongNew(
    PSSO_JSON* ppJson,
    SSO_LONG value);

SSOERROR
SSOJsonBooleanNew(
    PSSO_JSON* ppJson,
    bool value);

SSOERROR
SSOJsonObjectSet(
    PSSO_JSON pJson,
    PCSTRING key,
    PCSSO_JSON pJsonValue);

SSOERROR
SSOJsonIsObject(
    PCSSO_JSON pJson,
    bool* pBool);

SSOERROR
SSOJsonObjectSize(
    PCSSO_JSON pJson,
    size_t* pSize);

SSOERROR
SSOJsonObjectGet(
    PCSSO_JSON pJson,
    PCSTRING key,
    PSSO_JSON* ppJsonValue);

SSOERROR
SSOJsonStringValue(
    PCSSO_JSON pJson,
    PSTRING* pValue);

SSOERROR
SSOJsonIntegerValue(
    PCSSO_JSON pJson,
    INTEGER* pValue);

SSOERROR
SSOJsonLongValue(
    PCSSO_JSON pJson,
    SSO_LONG* pValue);

SSOERROR
SSOJsonBooleanValue(
    PCSSO_JSON pJson,
    bool* pValue);

SSOERROR
SSOJsonArrayAppend(
    PSSO_JSON pJson,
    PCSSO_JSON pJsonValue);

SSOERROR
SSOJsonArraySize(
    PCSSO_JSON pJson,
    size_t* pSize);

SSOERROR
SSOJsonArrayGet(
    PCSSO_JSON pJson,
    size_t index,
    PSSO_JSON* ppJsonValue);

SSOERROR
SSOJsonToString(
    PCSSO_JSON pJson,
    PSTRING* ppString);

void
SSOJsonIteratorDelete(
    PSSO_JSON_ITERATOR pJsonIter);

SSOERROR
SSOJsonObjectIterator(
    PCSSO_JSON pJson,
    PSSO_JSON_ITERATOR* ppJsonIter);

SSOERROR
SSOJsonObjectIteratorHasNext(
    PCSSO_JSON_ITERATOR pJsonIter,
    bool* pBool);

SSOERROR
SSOJsonObjectIteratorNext(
    PCSSO_JSON pJson,
    PCSSO_JSON_ITERATOR pJsonIter,
    PSSO_JSON_ITERATOR* ppJsonIterNext);

SSOERROR
SSOJsonObjectIteratorKey(
    PCSSO_JSON_ITERATOR pJsonIter,
    PCSTRING* ppKey);

SSOERROR
SSOJsonObjectIteratorValue(
    PCSSO_JSON_ITERATOR pJsonIter,
    PSSO_JSON* ppJsonValue);

SSOERROR
SSOJsonParse(
    PSSO_JSON* ppJson,
    PCSTRING pData);


SSOERROR
SSOJsonReset(
    PSSO_JSON pJson,
    PCSSO_JSON pIn);

SSOERROR
SSOJsonEquals(
    PCSSO_JSON pJson1,
    PCSSO_JSON pJson2,
    bool* pBool);

SSOERROR
SSOJsonIsNull(
    PCSSO_JSON pJson,
    bool* pBool);

// JWT

SSOERROR
SSOJwtParse(
    PSSO_JWT* pp,
    PCSTRING psz);

void
SSOJwtDelete(
    PSSO_JWT p);

SSOERROR
SSOJwtGetStringClaim(
    PCSSO_JWT p,
    PCSTRING pszKey,
    PSTRING* ppszValue /* OUT */);

SSOERROR
SSOJwtGetStringArrayClaim(
    PCSSO_JWT p,
    PCSTRING pszKey,
    PSTRING** pppszValue, /* OUT */
    size_t* pCount);

SSOERROR
SSOJwtGetLongClaim(
    PCSSO_JWT p,
    PCSTRING pszKey,
    SSO_LONG* pValue);

SSOERROR
SSOJwtGetJsonClaim(
    PCSSO_JWT p,
    PCSTRING pszKey,
    PSTRING* ppszValue /* OUT */);

SSOERROR
SSOJwtHasClaim(
    PCSSO_JWT p,
    PCSTRING pszKey,
    bool* pHasClaim);

SSOERROR
SSOJwtVerifySignature(
    PCSSO_JWT p,
    PCSTRING pszCertificatePEM,
    bool* pValid);

SSOERROR
SSOJwtCreateSignedJwtString(
    PCSTRING pszJsonPayloadString,
    PCSTRING pszPrivateKeyPEM,
    PSTRING* ppsz /* OUT */);

// JWK

SSOERROR
SSOJwkParseFromSet(
    PSSO_JWK* pp,
    PCSTRING pszJwkSet);

void
SSOJwkDelete(
    PSSO_JWK p);

PCSTRING
SSOJwkGetCertificate(
    PCSSO_JWK p);

SSOERROR
SSOJwkToPublicKeyPEM(
    PCSSO_JWK p,
    PSTRING* ppsz /* OUT */);

SSOERROR
SSOJwkToCertificatePEM(
    PCSSO_JWK p,
    PSTRING* ppsz /* OUT */);

// base64

SSOERROR
SSOBase64UrlEncodeToString(
    const unsigned char* pInput,
    size_t inputLength,
    char** ppszOutput /* OUT */);

SSOERROR
SSOBase64UrlDecodeToString(
    const char* pszInput,
    char** ppszOutput /* OUT */);

SSOERROR
SSOBase64UrlDecodeToBytes(
    const char* pszInput,
    unsigned char** ppOutput, /* OUT */
    size_t* pOutputLength);

// Openssl calls

SSOERROR
SSOComputeMessageDigest(
    SSO_DIGEST_METHOD digestMethod,
    const unsigned char* pData,
    size_t dataSize,
    unsigned char** ppMD,
    size_t* pMDSize);

SSOERROR
SSOComputeRSASignature(
    SSO_DIGEST_METHOD digestMethod,
    const unsigned char* pData,
    size_t dataSize,
    PCSTRING pszRSAPrivateKeyPEM,
    unsigned char** ppRSASignature,
    size_t* pRSASignatureSize);

SSOERROR
SSOVerifyRSASignature(
    SSO_DIGEST_METHOD digestMethod,
    const unsigned char* pData,
    size_t dataSize,
    const unsigned char* pRSASignature,
    size_t rsaSignatureSize,
    PCSTRING pszRSACertificatePEM,
    bool* verifySuccess);

// HA APIs

SSOERROR
SSOCdcGetAffinitizedHost(
    PCSSO_CDC pCdc,
    PCSTRING domainName,
    int cdcFlags,
    PSTRING* ppszAffinitizedHost);

SSOERROR
SSOCdcNew(
    PSSO_CDC* ppCdc);

void
SSOCdcDelete(
    PSSO_CDC pCdc);

#endif
