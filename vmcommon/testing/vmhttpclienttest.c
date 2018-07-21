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

#include "includes.h"
#include "vmhttpclient.h"

static
DWORD
_VmHttpClientInitTest()
{
    DWORD dwError = 0;
    PVM_HTTP_CLIENT pClient = NULL;

    dwError = VmHttpClientInit(&pClient);
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

    dwError = VmHttpClientInit(&pClient);
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

    dwError = VmHttpClientInit(&pClient);
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

DWORD
VmHttpClientTest()
{
    DWORD dwError = 0;

    dwError = _VmHttpClientInitTest();
    dwError += _VmHttpClientSetParamTest();
    dwError += _VmHttpClientChangeValueTest();

    return dwError;
}
