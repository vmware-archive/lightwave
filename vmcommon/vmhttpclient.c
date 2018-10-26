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


static
DWORD
_BuildStringToSign(
    PCSTR    pcszRequestMethod,
    PCSTR    pcszRequestBody,
    PCSTR    pcszContentType,
    PCSTR    pcszRequestTime,
    PCSTR    pcszRequestURI,
    PSTR     *ppszHashedRawStr,
    size_t   *pHashRawLen
    );

static
DWORD
_VmHttpClientSetCurlOptInt(
    PVM_HTTP_CLIENT pHttpClient,
    CURLoption curlOption,
    int intValue)
{
    DWORD dwError = 0;
    CURLcode curlCode = CURLE_OK;

    curlCode = curl_easy_setopt(pHttpClient->pCurl, curlOption, intValue);
    BAIL_AND_LOG_ON_VM_COMMON_CURL_ERROR(dwError, curlCode);

error:
    return dwError;
}

static
DWORD
_VmHttpClientSetCurlOptPtr(
    PVM_HTTP_CLIENT pHttpClient,
    CURLoption curlOption,
    const void* pValue)
{
    DWORD dwError = 0;
    CURLcode curlCode = CURLE_OK;

    curlCode = curl_easy_setopt(pHttpClient->pCurl, curlOption, pValue);
    BAIL_AND_LOG_ON_VM_COMMON_CURL_ERROR(dwError, curlCode);

error:
    return dwError;
}

static
size_t
_VmWriteMemoryCallback(
    char *pMem,
    size_t nSize,
    size_t nMemSize,
    PVOID  pUserData
    )
{
    DWORD dwError = 0;
    size_t nNewLen = nSize * nMemSize;
    PVM_HTTP_CLIENT pClient = NULL;

    if (!pMem|| !pUserData)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    pClient = (PVM_HTTP_CLIENT)pUserData;

    dwError = VmReallocateMemoryWithInit(
            (PVOID)pClient->pszResult,
            pClient->nResultLen,
            (PVOID *)&pClient->pszResult,
            pClient->nResultLen + nNewLen + 1);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmCopyMemory(
                  (PVOID)&pClient->pszResult[pClient->nResultLen],
                  nNewLen,
                  pMem,
                  nNewLen);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    pClient->nResultLen += nNewLen;

cleanup:
    return nNewLen;

error:
    nNewLen = 0;
    goto cleanup;
}

DWORD
VmHttpClientInit(
    PVM_HTTP_CLIENT *ppClient,
    PCSTR pszCAPath
    )
{
    DWORD dwError = 0;
    PVM_HTTP_CLIENT pClient = NULL;

    if (!ppClient)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = VmAllocateMemory(sizeof(VM_HTTP_CLIENT), (PVOID *)&pClient);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    pClient->pCurl = curl_easy_init();
    if (!pClient->pCurl)
    {
        dwError = VM_COMMON_ERROR_CURL_INIT_FAILURE;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pszCAPath))
    {
        dwError = VmAllocateStringA(pszCAPath, &pClient->pszCAPath);
        BAIL_ON_VM_COMMON_ERROR(dwError);

        /* set ca path and cert verify options*/
        dwError = _VmHttpClientSetCurlOptInt(pClient, CURLOPT_SSL_VERIFYPEER, 1);
        BAIL_ON_VM_COMMON_ERROR(dwError);

        dwError = _VmHttpClientSetCurlOptInt(pClient, CURLOPT_SSL_VERIFYHOST, 2);
        BAIL_ON_VM_COMMON_ERROR(dwError);

        dwError = _VmHttpClientSetCurlOptPtr(
                      pClient,
                      CURLOPT_CAPATH,
                      pClient->pszCAPath);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }
    else /* skip cert verify */
    {
        dwError = _VmHttpClientSetCurlOptInt(pClient, CURLOPT_SSL_VERIFYPEER, 0);
        BAIL_ON_VM_COMMON_ERROR(dwError);

        dwError = _VmHttpClientSetCurlOptInt(pClient, CURLOPT_SSL_VERIFYHOST, 0);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    *ppClient = pClient;

cleanup:
    return dwError;

error:
    VmHttpClientFreeHandle(pClient);
    goto cleanup;
}

static
DWORD
_VmHttpUrlEncodeString(
    PVM_HTTP_CLIENT pClient,
    PCSTR pszString,
    PSTR *ppszEncodedString
    )
{
    DWORD dwError = 0;
    PSTR pszEncodedString = NULL;

    pszEncodedString = curl_easy_escape(
                           pClient->pCurl,
                           pszString,
                           VmStringLenA(pszString));
    if (!pszEncodedString)
    {
        dwError = VM_COMMON_ERROR_NO_MEMORY;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    *ppszEncodedString = pszEncodedString;

cleanup:
    return dwError;

error:
    VM_COMMON_SAFE_FREE_CURL_MEMORY(pszEncodedString);
    goto cleanup;
}

DWORD
VmHttpClientSetQueryParam(
    PVM_HTTP_CLIENT pClient,
    PCSTR pszKey,
    PCSTR pszValue
    )
{
    DWORD dwError = 0;
    PSTR pszEncodedKey = NULL;
    PSTR pszEncodedValue = NULL;

    if (!pClient ||
        !pClient->pCurl ||
        IsNullOrEmptyString(pszKey) ||
        IsNullOrEmptyString(pszValue))
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    if (!pClient->pQueryParamsMap)
    {
        dwError = LwRtlCreateHashMap(
                &pClient->pQueryParamsMap,
                LwRtlHashDigestPstrCaseless,
                LwRtlHashEqualPstrCaseless,
                NULL);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = _VmHttpUrlEncodeString(pClient, pszKey, &pszEncodedKey);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = _VmHttpUrlEncodeString(pClient, pszValue, &pszEncodedValue);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = LwRtlHashMapInsert(
                  pClient->pQueryParamsMap,
                  (PVOID)pszEncodedKey,
                  (PVOID)pszEncodedValue,
                  NULL);
    BAIL_ON_VM_COMMON_ERROR(dwError);

cleanup:
    return dwError;

error:
    VM_COMMON_SAFE_FREE_CURL_MEMORY(pszEncodedKey);
    VM_COMMON_SAFE_FREE_CURL_MEMORY(pszEncodedValue);
    goto cleanup;
}

DWORD
VmHttpClientGetQueryStringLength(
    PVM_HTTP_CLIENT pClient,
    int *pnLength
    )
{
    DWORD dwError = 0;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};
    int nLength = 0;
    const int AMPERSAND_LENGTH = 1;
    const int EQUALS_LENGTH = 1;

    if (!pClient || !pnLength)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    while (pClient->pQueryParamsMap &&
           LwRtlHashMapIterate(pClient->pQueryParamsMap, &iter, &pair))
    {
        if (nLength > 0)
        {
            nLength += AMPERSAND_LENGTH;
        }
        nLength += VmStringLenA(pair.pKey);
        nLength += EQUALS_LENGTH;
        nLength += VmStringLenA(pair.pValue);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    *pnLength = nLength;

error:
    return dwError;
}

DWORD
VmHttpClientGetQueryString(
    PVM_HTTP_CLIENT pClient,
    PSTR *ppszParams
    )
{
    DWORD dwError = 0;
    PSTR pszParams = NULL;
    PSTR pszParamsTemp = NULL;
    int nLength = 0;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};
    BOOLEAN bNeedSeparator = FALSE;

    if (!pClient->pQueryParamsMap)
    {
        dwError = VM_COMMON_ERROR_NO_DATA;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    /* find length required for param formatting (key1=value1&key2=value2) */
    dwError = VmHttpClientGetQueryStringLength(pClient, &nLength);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmAllocateMemory(nLength + 1, (PVOID *)&pszParams);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    /* make query param string (key1=value1&key2=value2) */
    pszParamsTemp = pszParams;
    while (pClient->pQueryParamsMap &&
           LwRtlHashMapIterate(pClient->pQueryParamsMap, &iter, &pair))
    {
        dwError = VmStringPrintFA(
                      pszParamsTemp,
                      nLength,
                      "%s%s=%s",
                      bNeedSeparator ? "&" : "",
                      pair.pKey, pair.pValue);
        BAIL_ON_VM_COMMON_ERROR(dwError);

        pszParamsTemp += VmStringLenA(pszParamsTemp);

        bNeedSeparator = TRUE;
    }

    *ppszParams = pszParams;

cleanup:
    return dwError;

error:
    VM_COMMON_SAFE_FREE_MEMORY(pszParams);
    goto cleanup;
}

static
DWORD
_VmHttpClientSetCurlMethod(
    PVM_HTTP_CLIENT pClient,
    VM_HTTP_METHOD nMethod
    )
{
    DWORD dwError = 0;

    switch(nMethod)
    {
        case VMHTTP_METHOD_GET:
            dwError = _VmHttpClientSetCurlOptInt(pClient, CURLOPT_HTTPGET, 1);
            BAIL_ON_VM_COMMON_ERROR(dwError);
        break;
        case VMHTTP_METHOD_PUT:
            dwError = _VmHttpClientSetCurlOptPtr(pClient, CURLOPT_CUSTOMREQUEST, "PUT");
            BAIL_ON_VM_COMMON_ERROR(dwError);
        break;
        case VMHTTP_METHOD_POST:
            dwError = _VmHttpClientSetCurlOptInt(pClient, CURLOPT_POST, 1);
            BAIL_ON_VM_COMMON_ERROR(dwError);
        break;
        case VMHTTP_METHOD_DELETE:
            dwError = _VmHttpClientSetCurlOptPtr(pClient, CURLOPT_CUSTOMREQUEST, "DELETE");
            BAIL_ON_VM_COMMON_ERROR(dwError);
        break;
        case VMHTTP_METHOD_PATCH:
            dwError = _VmHttpClientSetCurlOptPtr(pClient, CURLOPT_CUSTOMREQUEST, "PATCH");
            BAIL_ON_VM_COMMON_ERROR(dwError);
        break;
        default:
            dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
            BAIL_ON_VM_COMMON_ERROR(dwError);
    }

error:
    return dwError;
}

DWORD
VmHttpClientPerform(
    PVM_HTTP_CLIENT pClient,
    VM_HTTP_METHOD nMethod,
    PCSTR pszUrl
    )
{
    DWORD dwError = 0;
    PSTR pszNewUrl = NULL;
    PSTR pszParams = NULL;
    CURLcode curlCode = CURLE_OK;
    long nStatus = 0;

    if (!pClient || IsNullOrEmptyString(pszUrl))
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = VmHttpClientGetQueryString(pClient, &pszParams);
    if (dwError == VM_COMMON_ERROR_NO_DATA)
    {
        dwError = 0; /* Null/Empty pszParams handled below */
    }
    BAIL_ON_VM_COMMON_ERROR(dwError);

    if (!IsNullOrEmptyString(pszParams))
    {
        dwError = VmAllocateStringPrintf(
                      &pszNewUrl,
                      "%s?%s",
                      pszUrl, pszParams);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = _VmHttpClientSetCurlOptPtr(
                  pClient,
                  CURLOPT_URL,
                  pszNewUrl ? pszNewUrl : pszUrl);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = _VmHttpClientSetCurlOptPtr(
                  pClient,
                  CURLOPT_POSTFIELDS,
                  pClient->pszBody? pClient->pszBody : "");
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = _VmHttpClientSetCurlMethod(pClient, nMethod);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    if (pClient->pHeaders)
    {
        dwError = _VmHttpClientSetCurlOptPtr(
                      pClient,
                      CURLOPT_HTTPHEADER,
                      pClient->pHeaders);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = _VmHttpClientSetCurlOptPtr(
                  pClient,
                  CURLOPT_WRITEFUNCTION,
                  _VmWriteMemoryCallback);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = _VmHttpClientSetCurlOptPtr(
                  pClient,
                  CURLOPT_WRITEDATA,
                  (PVOID)pClient);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    curlCode = curl_easy_perform(pClient->pCurl);
    BAIL_AND_LOG_ON_VM_COMMON_CURL_ERROR(dwError, curlCode);

    curlCode = curl_easy_getinfo(
                   pClient->pCurl,
                   CURLINFO_RESPONSE_CODE,
                   &nStatus);
    BAIL_AND_LOG_ON_VM_COMMON_CURL_ERROR(dwError, curlCode);

    pClient->nStatus = nStatus;

cleanup:
    VM_COMMON_SAFE_FREE_MEMORY(pszParams);
    return dwError;

error:
    goto cleanup;
}

DWORD
VmHttpClientSetToken(
    PVM_HTTP_CLIENT pClient,
    VM_HTTP_TOKEN_TYPE tokenType,
    PCSTR pszToken
    )
{
    DWORD dwError = 0;
    PSTR pszHeaderAuth = NULL;
    PSTR pszHeaderAuthTokenType = NULL;

    if (!pClient || IsNullOrEmptyString(pszToken))
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    switch (tokenType)
    {
        case VMHTTP_TOKEN_TYPE_BEARER:
            pszHeaderAuthTokenType = HEADER_BEARER_AUTH;
            break;
        case VMHTTP_TOKEN_TYPE_HOTK_PK:
            pszHeaderAuthTokenType = HEADER_HOTK_PK_AUTH;
            break;
        default:
            dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
            BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = VmAllocateStringPrintf(
                        &pszHeaderAuth,
                        pszHeaderAuthTokenType,
                        pszToken);
    pClient->pHeaders = curl_slist_append(pClient->pHeaders, pszHeaderAuth);

    if(!pClient->pHeaders)
    {
        dwError = VM_COMMON_ERROR_CURL_FAILURE;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    VM_COMMON_SAFE_FREE_MEMORY(pszHeaderAuth);
    goto cleanup;
}

DWORD
VmHttpClientSkipCertValidation(
    PVM_HTTP_CLIENT pClient
    )
{
    DWORD dwError = 0;
    if (!pClient)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    curl_easy_setopt(pClient->pCurl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(pClient->pCurl, CURLOPT_SSL_VERIFYPEER, 0L);

error:
    return dwError;
}

DWORD
VmHttpClientGetResult(
    PVM_HTTP_CLIENT pClient,
    PCSTR *ppszResult
    )
{
    DWORD dwError = 0;
    if (!pClient || !ppszResult)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    *ppszResult = pClient->pszResult;
error:
    return dwError;
}

DWORD
VmHttpClientRequestPOPSignature(
    VM_HTTP_METHOD  httpMethod,
    PCSTR           pcszRequestURI,
    PCSTR           pcszRequestBody,
    PCSTR           pcszPEM,
    PCSTR           pcszRequestTime,
    PSTR            *ppszSignature
    )
{
    DWORD           dwError = 0;
    size_t          requestBodyLen = 0;
    unsigned char   *pszHashedBody = NULL;
    size_t          hashedBodyLen = 0;
    PSTR            pszHashedBodyHex = NULL;
    PCSTR           pszMethod = NULL;
    PSTR            pszToSign = NULL;
    PSTR            pszTrimBody = NULL;
    size_t          toSignLen = 0;
    unsigned char   *pszRSARaw = NULL;
    size_t          rsaRawLen = 0;
    PSTR            pszSignature = NULL;


    if (IsNullOrEmptyString(pcszRequestURI) ||
        IsNullOrEmptyString(pcszPEM) ||
        IsNullOrEmptyString(pcszRequestTime) ||
        !ppszSignature
        )
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = GetRequestMethodInString(httpMethod, &pszMethod);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmAllocateStringA(VM_COMMON_SAFE_STRING(pcszRequestBody),
                                &pszTrimBody);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    VmStringTrimSpace(pszTrimBody);

    if (!IsNullOrEmptyString(pszTrimBody))
    {
        requestBodyLen = VmStringLenA(pszTrimBody);
        dwError = VmSignatureComputeMessageDigest(VMSIGN_DIGEST_METHOD_SHA256,
                                                  pszTrimBody,
                                                  requestBodyLen,
                                                  &pszHashedBody,
                                                  &hashedBodyLen
                                                  );
        BAIL_ON_VM_COMMON_ERROR(dwError);

        dwError = VmSignatureEncodeHex(pszHashedBody,
                                       hashedBodyLen,
                                       &pszHashedBodyHex
                                       );
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = _BuildStringToSign(pszMethod,
                                 pszHashedBodyHex,
                                 VM_COMMON_HTTP_CONTENT_TYPE_JSON,
                                 pcszRequestTime,
                                 pcszRequestURI,
                                 &pszToSign,
                                 &toSignLen
                                 );
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmSignatureComputeRSASignature(VMSIGN_DIGEST_METHOD_SHA256,
                                             pszToSign,
                                             toSignLen,
                                             pcszPEM,
                                             &pszRSARaw,
                                             &rsaRawLen
                                             );
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmSignatureEncodeHex(pszRSARaw, rsaRawLen, &pszSignature);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    *ppszSignature = pszSignature;

cleanup:
    VM_COMMON_SAFE_FREE_STRINGA(pszHashedBody);
    VM_COMMON_SAFE_FREE_STRINGA(pszTrimBody);
    VM_COMMON_SAFE_FREE_STRINGA(pszHashedBodyHex);
    VM_COMMON_SAFE_FREE_STRINGA(pszToSign);
    VM_COMMON_SAFE_FREE_STRINGA(pszRSARaw);

    return dwError;

error:
    VM_COMMON_SAFE_FREE_STRINGA(pszSignature);
    if (ppszSignature)
    {
        *ppszSignature = NULL;
    }

    goto cleanup;

}

static
VOID
_VmCommonFreeStringMapPair(
    PLW_HASHMAP_PAIR pPair,
    LW_PVOID         pUnused
    )
{
    VM_COMMON_SAFE_FREE_CURL_MEMORY(pPair->pKey);
    VM_COMMON_SAFE_FREE_CURL_MEMORY(pPair->pValue);
}

VOID
VmHttpClientFreeHandle(
    PVM_HTTP_CLIENT pClient
    )
{
    if (pClient)
    {
        VM_COMMON_SAFE_FREE_MEMORY(pClient->pszCAPath);
        VM_COMMON_SAFE_FREE_MEMORY(pClient->pszResult);
        VM_COMMON_SAFE_FREE_MEMORY(pClient->pszBody);
        if (pClient->pQueryParamsMap)
        {
            LwRtlHashMapClear(
                pClient->pQueryParamsMap,
                _VmCommonFreeStringMapPair,
                NULL);
            LwRtlFreeHashMap(&pClient->pQueryParamsMap);
        }
        if (pClient->pCurl)
        {
            curl_easy_cleanup(pClient->pCurl);
        }
        if (pClient->pHeaders)
        {
            curl_slist_free_all(pClient->pHeaders);
        }
        VmFreeMemory(pClient);
    }
}

DWORD
GetRequestMethodInString(
    VM_HTTP_METHOD  httpMethod,
    PCSTR           *ppcszHttpMethod
    )
{
    DWORD   dwError = 0;
    PCSTR    pcszHttpMethod = NULL;

    switch (httpMethod)
    {
        case VMHTTP_METHOD_GET:
            pcszHttpMethod = "GET";
            break;

        case VMHTTP_METHOD_POST:
            pcszHttpMethod = "POST";
            break;

        case VMHTTP_METHOD_PUT:
            pcszHttpMethod = "PUT";
            break;

        case VMHTTP_METHOD_DELETE:
            pcszHttpMethod = "DELETE";
            break;

        case VMHTTP_METHOD_PATCH:
            pcszHttpMethod = "PATCH";
            break;

        default:
            dwError = VM_COMMON_UNSUPPORTED_HTTP_METHOD;
            BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    *ppcszHttpMethod = pcszHttpMethod;

error:
    return dwError;
}

static
DWORD
_BuildStringToSign(
    PCSTR   pcszRequestMethod,
    PCSTR   pcszRequestBody,
    PCSTR   pcszContentType,
    PCSTR   pcszRequestTime,
    PCSTR   pcszRequestURI,
    PSTR    *ppszRaw,
    size_t  *ppRawStrLen
    )
{
    DWORD   dwError = 0;
    PSTR    pszRaw = NULL;

    if (IsNullOrEmptyString(pcszRequestMethod) ||
        IsNullOrEmptyString(pcszContentType) ||
        IsNullOrEmptyString(pcszRequestTime) ||
        IsNullOrEmptyString(pcszRequestURI) ||
        !ppszRaw ||
        !ppRawStrLen
        )
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = VmAllocateStringPrintf(&pszRaw,
                                     "%s\n%s\n%s\n%s\n%s",
                                     pcszRequestMethod,
                                     VM_COMMON_SAFE_STRING(pcszRequestBody),
                                     pcszContentType,
                                     pcszRequestTime,
                                     pcszRequestURI
                                    );
    BAIL_ON_VM_COMMON_ERROR(dwError);

    *ppszRaw = pszRaw;
    *ppRawStrLen = VmStringLenA(pszRaw);

cleanup:
    return dwError;

error:
    VM_COMMON_SAFE_FREE_STRINGA(pszRaw);
    if (ppszRaw)
    {
        *ppszRaw = NULL;
    }
    if (ppRawStrLen)
    {
        *ppRawStrLen = 0;
    }

    goto cleanup;
}
