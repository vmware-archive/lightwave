/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ?~@~\License?~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ?~@~\AS IS?~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "vmincludes.h"
#include "vmhttpclient.h"

static
DWORD
_VmHttpClientInitTest();

static
DWORD
_VmHttpClientSetParamTest();

static
DWORD
_VmHttpClientChangeValueTest();

static
DWORD
_VmHttpClientUrlEncodeTest();

static
DWORD
_VmHttpClientStatusCodeTest();

static
DWORD
_VmHttpClientHeaderTest();

static
DWORD
_VmHttpClientSetBodyTest();

static
DWORD
_VmHttpClientSetupHOTKTest();

static
DWORD
_VmHttpBase64Test();

DWORD
VmHttpClientTest()
{
    DWORD dwError = 0;

    dwError = _VmHttpClientInitTest();
    dwError += _VmHttpClientSetParamTest();
    dwError += _VmHttpClientChangeValueTest();
    dwError += _VmHttpClientUrlEncodeTest();
    dwError += _VmHttpClientStatusCodeTest();
    dwError += _VmHttpClientHeaderTest();
    dwError += _VmHttpClientSetBodyTest();
    dwError += _VmHttpClientSetupHOTKTest();
    dwError += _VmHttpBase64Test();

    return dwError;
}

static
DWORD
_VmHttpClientInitTest()
{
    DWORD dwError = 0;
    PVM_HTTP_CLIENT pClient = NULL;

    dwError = VmHttpClientInit(&pClient, NULL);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    fprintf(stdout, "PASS: httpclient init test\n");

cleanup:
    VmHttpClientFreeHandle(pClient);
    return dwError;

error:
    fprintf(stderr, "FAIL: [%s,%d], Error: %d",__FILE__, __LINE__, dwError); \
    goto cleanup;
}

static
DWORD
_VmHttpClientSetParamTest()
{
    DWORD dwError = 0;
    PVM_HTTP_CLIENT pClient = NULL;
    PCSTR ppszKeys[] = {"key1", "key2", "key3"};
    PCSTR ppszValues[] = {"value1", "value2", "value3"};

    const char *QUERY_STRING = "key1=value1&key2=value2&key3=value3";
    const int QUERY_STRING_LENGTH = strlen(QUERY_STRING);

    int i = 0;
    int nLength = 0;
    PSTR pszQuery = NULL;

    dwError = VmHttpClientInit(&pClient, NULL);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    for (i = 0; i < sizeof(ppszKeys) / sizeof(ppszKeys[0]); ++i)
    {
        dwError = VmHttpClientSetQueryParam(pClient, ppszKeys[i], ppszValues[i]);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = VmHttpClientGetQueryStringLength(pClient, &nLength);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    if (nLength != QUERY_STRING_LENGTH)
    {
        fprintf(stderr, "FAIL: Expected %d. Got %d\n", QUERY_STRING_LENGTH, nLength);
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = VmHttpClientGetQueryString(pClient, &pszQuery);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    if (VmStringCompareA(QUERY_STRING, pszQuery, TRUE))
    {
        fprintf(stderr, "FAIL: Expected %s. Got %s\n", QUERY_STRING, pszQuery);
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    fprintf(stdout, "PASS: httpclient set param test\n");

cleanup:
    VM_COMMON_SAFE_FREE_MEMORY(pszQuery);
    VmHttpClientFreeHandle(pClient);
    return dwError;

error:
    fprintf(stderr, "FAIL: [%s,%d], Error: %d",__FILE__, __LINE__, dwError); \
    goto cleanup;
}

static
DWORD
_VmHttpClientChangeValueTest()
{
    DWORD dwError = 0;
    PVM_HTTP_CLIENT pClient = NULL;
    PCSTR ppszKeys[] = {"key1", "key2", "key3"};
    PCSTR ppszValues[] = {"value1", "value2", "value3"};
    PCSTR pszKey = "key2";
    PCSTR pszNewValue = "value22";

    const char *QUERY_STRING = "key1=value1&key2=value22&key3=value3";

    int i = 0;
    PSTR pszQuery = NULL;

    dwError = VmHttpClientInit(&pClient, NULL);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    for (i = 0; i < sizeof(ppszKeys) / sizeof(ppszKeys[0]); ++i)
    {
        dwError = VmHttpClientSetQueryParam(pClient, ppszKeys[i], ppszValues[i]);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = VmHttpClientSetQueryParam(pClient, pszKey, pszNewValue);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmHttpClientGetQueryString(pClient, &pszQuery);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    if (VmStringCompareA(QUERY_STRING, pszQuery, TRUE))
    {
        fprintf(stderr, "FAIL: Expected %s. Got %s\n", QUERY_STRING, pszQuery);
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    fprintf(stdout, "PASS: httpclient overwrite value test\n");

cleanup:
    VM_COMMON_SAFE_FREE_MEMORY(pszQuery);
    VmHttpClientFreeHandle(pClient);
    return dwError;

error:
    fprintf(stderr, "FAIL: [%s,%d], Error: %d",__FILE__, __LINE__, dwError); \
    goto cleanup;
}

static
DWORD
_VmHttpClientUrlEncodeTest()
{
    DWORD           dwError = 0;
    PVM_HTTP_CLIENT pClient = NULL;
    PCSTR           pcszDecoded = "(&(cn=testId)(objectClass=vmwCertificationAuthority))";
    PCSTR           pcszExpectedEncoded = "%28%26%28cn%3DtestId%29%28objectClass%3DvmwCertificationAuthority%29%29";
    PSTR            pszActualEncoded = NULL;

    dwError = VmHttpClientInit(&pClient, NULL);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmHttpUrlEncodeString(pClient, pcszDecoded, &pszActualEncoded);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    if (VmStringCompareA(pszActualEncoded, pcszExpectedEncoded, TRUE))
    {
        fprintf(stderr, "FAIL: Encode URL Expected '%s'. Got '%s'\n",
            pcszExpectedEncoded, pszActualEncoded);
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    fprintf(stdout, "PASS: httpclient encode URL test\n");

cleanup:
    VM_COMMON_SAFE_FREE_STRINGA(pszActualEncoded);
    VmHttpClientFreeHandle(pClient);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmHttpClientStatusCodeTest()
{
    DWORD dwError = 0;
    PVM_HTTP_CLIENT pClient = NULL;
    long statusCode = 200;
    long expectedStatusCode = 200;
    long actualStatusCode = 0;

    dwError = VmHttpClientInit(&pClient, NULL);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    pClient->nStatus = statusCode;

    dwError = VmHttpClientGetStatusCode(pClient, &actualStatusCode);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    if (actualStatusCode != expectedStatusCode)
    {
        fprintf(stderr, "FAIL: Status Code - Expected: %ld, Actual: %ld\n",
            expectedStatusCode, actualStatusCode);
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    fprintf(stdout, "PASS: httpclient status code test\n");

cleanup:
    VmHttpClientFreeHandle(pClient);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmHttpClientHeaderTest()
{
    DWORD           dwError = 0;
    PVM_HTTP_CLIENT pClient = NULL;
    PCSTR           pcszKey = "Content-Type";
    PCSTR           pcszValue = "application/json";
    PCSTR           pcszExpectedHeader = "Content-Type: application/json";
    PCSTR           pcszActualHeader = NULL;

    dwError = VmHttpClientInit(&pClient, NULL);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmHttpClientSetHeader(pClient, pcszKey, pcszValue);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    pcszActualHeader = pClient->pHeaders[0].data;
    if (VmStringCompareA(pcszActualHeader, pcszExpectedHeader, TRUE))
    {
        fprintf(stderr, "FAIL: Set header test: Expected %s, Actual %s\n",
            pcszExpectedHeader, pcszActualHeader);
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    fprintf(stdout, "PASS: httpclient set header test\n");

cleanup:
    VmHttpClientFreeHandle(pClient);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmHttpClientSetBodyTest()
{
    DWORD               dwError = 0;
    PVM_HTTP_CLIENT     pClient = NULL;
    PCSTR               pcszBody = "some random body";

    dwError = VmHttpClientInit(&pClient, NULL);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmHttpClientSetBody(pClient, pcszBody);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    if (VmStringCompareA(pcszBody, pClient->pszBody, TRUE))
    {
        fprintf(stderr, "FAIL: Set body test: Expected: '%s', Actual: '%s'\n",
            pcszBody, pClient->pszBody);
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    fprintf(stdout, "PASS: httpclient set body test\n");

cleanup:
    VmHttpClientFreeHandle(pClient);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmHttpClientSetupHOTKTest()
{
    DWORD               dwError = 0;
    PVM_HTTP_CLIENT     pClient = NULL;
    PCSTR               pcszAccessToken = "access-token";
    PCSTR               pcszSignature = "signature";
    PCSTR               pcszReqTime = "Thu, 11 Oct 2018 17:42:39 GMT";
    PCSTR               pcszExpectedAuthz = "Authorization: hotk-pk access-token:signature";
    PCSTR               pcszExpectedDate = "Date: Thu, 11 Oct 2018 17:42:39 GMT";
    PCSTR               pcszExpectedContentType = "Content-Type: application/json";
    PCSTR               pcszReqBody = NULL;
    struct curl_slist   *pHeader = NULL;
    BOOLEAN             found = FALSE;
    PSTR                pszHeaderData = NULL;

    dwError = VmHttpClientInit(&pClient, NULL);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmHttpClientSetupHOTK(pClient,
                                    pcszAccessToken,
                                    pcszSignature,
                                    pcszReqTime,
                                    pcszReqBody
                                    );
    BAIL_ON_VM_COMMON_ERROR(dwError);

    pHeader = pClient->pHeaders;

    if (!pHeader)
    {
        fprintf(stderr, "FAIL: HOTK Setup Test: Unable to find any headers\n");
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    while (pHeader)
    {
        pszHeaderData = pHeader->data;
        found = !VmStringCompareA(pszHeaderData, pcszExpectedAuthz, TRUE) ||
                !VmStringCompareA(pszHeaderData, pcszExpectedDate, TRUE) ||
                !VmStringCompareA(pszHeaderData, pcszExpectedContentType, TRUE);
        if (!found)
        {
            fprintf(stderr, "FAIL: HOTK Setup Test: Header Data %s didn't "
                            "match any expected\n", pszHeaderData);
            dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
            BAIL_ON_VM_COMMON_ERROR(dwError);
        }

        pHeader = pHeader->next;
    }

    fprintf(stdout, "PASS: httpclient Setup HOTK Test\n");

cleanup:
    VmHttpClientFreeHandle(pClient);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmHttpBase64Test()
{
    DWORD       dwError = 0;
    DWORD       dwLen = 0;
    PCSTR       pcszExpectedDecodedString = "Hello World";
    PCSTR       pcszExpectedEncodedString = "SGVsbG8gV29ybGQ=";
    PSTR        pszActualEncodedString = NULL;
    PSTR        pszActualDecodedString = NULL;

    dwError = VmEncodeToBase64((PBYTE)pcszExpectedDecodedString,
                               VmStringLenA(pcszExpectedDecodedString),
                               (PBYTE *)&pszActualEncodedString,
                               &dwLen
                               );
    BAIL_ON_VM_COMMON_ERROR(dwError);
    if (VmStringCompareA(pcszExpectedEncodedString, pszActualEncodedString, TRUE))
    {
        fprintf(stderr, "FAIL: Base64 Encode Test: Expected '%s' Found '%s'\n",
                pcszExpectedEncodedString, pszActualEncodedString);
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = VmDecodeToBase64((PBYTE)pszActualEncodedString,
                               dwLen,
                               (PBYTE *)&pszActualDecodedString,
                               &dwLen
                               );
    BAIL_ON_VM_COMMON_ERROR(dwError);
    if (VmStringCompareA(pcszExpectedDecodedString, pszActualDecodedString, TRUE))
    {
        fprintf(stderr, "FAIL: Base64 Decode Test: Expected '%s' Found '%s'\n",
            pcszExpectedDecodedString, pszActualDecodedString);
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    fprintf(stdout, "PASS: Base64 Encode-Decode Test\n");

cleanup:
    VM_COMMON_SAFE_FREE_STRINGA(pszActualDecodedString);
    VM_COMMON_SAFE_FREE_STRINGA(pszActualEncodedString);

    return dwError;
error:
    goto cleanup;
}
