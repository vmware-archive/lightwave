/*
 * Copyright © 2019 VMware, Inc.  All Rights Reserved.
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
_TestMapSearchResult(
    PCSTR                       pszResult,
    PVMDIR_SEARCH_OP_CONTEXT    pOpCtx
    );

static
DWORD
_TestSendRestSearch(
    PVMDIR_SEARCH_TEST_CONTEXT pContext,
    PVMDIR_SEARCH_TEST_CASE    pCase,
    PVMDIR_SEARCH_OP_CONTEXT   pOpCtx
    );

static
DWORD
_TestValidateRestSearchResult(
    PVMDIR_SEARCH_TEST_CASE     pCase,
    PVMDIR_SEARCH_OP_CONTEXT    pOpCtx
    );

static
DWORD
_TestExecuteRestSearchCase(
    PVMDIR_SEARCH_TEST_CONTEXT  pContext,
    PVMDIR_SEARCH_TEST_CASE     pCase
    );

static
DWORD
_TestExecuteRestPagedSearch(
    PVMDIR_SEARCH_TEST_CONTEXT  pContext,
    PVMDIR_SEARCH_TEST_CASE     pCase
    );

static
DWORD
_TestExecuteRestNormalSearch(
    PVMDIR_SEARCH_TEST_CONTEXT  pContext,
    PVMDIR_SEARCH_TEST_CASE     pCase
    );

static
DWORD
_TestExecuteRestSearchCase(
    PVMDIR_SEARCH_TEST_CONTEXT  pContext,
    PVMDIR_SEARCH_TEST_CASE     pCase
    );

DWORD
TestExecuteRestSearch(
    PVMDIR_SEARCH_TEST_CONTEXT  pContext
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;

    {
        VMDIR_SEARCH_TEST_CASE testTbl[] = TEST_SEARCH_CASE_1;

        for (dwCnt=0; dwCnt < sizeof(testTbl)/sizeof(testTbl[0]); dwCnt++)
        {
            dwError = _TestExecuteRestSearchCase(pContext, &testTbl[dwCnt]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        printf("TEST_REST_SEARCH_CASE_1 succeeded\n");
        fflush(stdout);
    }

error:
    return dwError;
}

static
DWORD
_TestExecuteRestSearchCase(
    PVMDIR_SEARCH_TEST_CONTEXT  pContext,
    PVMDIR_SEARCH_TEST_CASE     pCase
    )
{
    DWORD   dwError = 0;

    if (pCase->definition.dwPageSize > 0)
    {
        dwError = _TestExecuteRestPagedSearch(pContext, pCase);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = _TestExecuteRestNormalSearch(pContext, pCase);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:
    return dwError;
}

static
DWORD
_TestExecuteRestNormalSearch(
    PVMDIR_SEARCH_TEST_CONTEXT  pContext,
    PVMDIR_SEARCH_TEST_CASE     pCase
    )
{
    DWORD   dwError = 0;
    VMDIR_SEARCH_OP_CONTEXT    opCtx = {0};

    dwError = _TestSendRestSearch(pContext, pCase, &opCtx);
    BAIL_ON_VMDIR_ERROR(dwError);


    dwError = _TestValidateRestSearchResult(pCase, &opCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    TestFreeSearchOpCtxContent(&opCtx);

    return dwError;

error:
    goto cleanup;
}

static
DWORD
_TestExecuteRestPagedSearch(
    PVMDIR_SEARCH_TEST_CONTEXT  pContext,
    PVMDIR_SEARCH_TEST_CASE     pCase
    )
{
    DWORD   dwError = 0;
    VMDIR_SEARCH_OP_CONTEXT    opCtx = {0};

    opCtx.bPaged = TRUE;
    opCtx.dwPageSize = pCase->definition.dwPageSize;

    while (!opCtx.bDone)
    {
        dwError = _TestSendRestSearch(pContext, pCase, &opCtx);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _TestValidateRestSearchResult(pCase, &opCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    TestFreeSearchOpCtxContent(&opCtx);

    return dwError;

error:
    goto cleanup;
}

static
DWORD
_TestMapSearchResult(
    PCSTR                       pszResult,
    PVMDIR_SEARCH_OP_CONTEXT    pOpCtx
    )
{
    DWORD   dwError = 0;
    VMDIR_JSON_RESULT_LDAP_SEARCH_RESPONSE  searchResp={0};

    VM_JSON_OBJECT_MAP searchRespMap[] =
    {
        {"error_code",          JSON_RESULT_INTEGER,    {(VOID *)&searchResp.iLdapCode}},
        {"error_message",       JSON_RESULT_STRING,     {(VOID *)&searchResp.pszLdapMsg}},
        {"paged_results_cookie",JSON_RESULT_STRING,     {(VOID *)&searchResp.pszPagedCookies}},
        {"result_count",        JSON_RESULT_INTEGER,    {(VOID *)&searchResp.iResultCount}},
        {NULL, JSON_RESULT_INVALID, {NULL}}
    };

    dwError = VmJsonResultMapObject(
        pszResult,
        searchRespMap);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pOpCtx->bPaged)
    {
        VMDIR_SAFE_FREE_MEMORY(pOpCtx->pszRestPagedCookie);

        if (searchResp.pszPagedCookies && VmDirStringLenA(searchResp.pszPagedCookies) > 0)
        {
            dwError = VmDirAllocateStringA(searchResp.pszPagedCookies, &pOpCtx->pszRestPagedCookie);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            pOpCtx->bDone = TRUE;
        }
    }

    pOpCtx->dwResultCode = searchResp.iLdapCode;
    pOpCtx->dwResultCount = searchResp.iResultCount;
    pOpCtx->dwTotalResultCount += pOpCtx->dwResultCount;

cleanup:
    return dwError;

error:
    goto cleanup;;
}

static
DWORD
_TestSendRestSearch(
    PVMDIR_SEARCH_TEST_CONTEXT pContext,
    PVMDIR_SEARCH_TEST_CASE    pCase,
    PVMDIR_SEARCH_OP_CONTEXT   pOpCtx
    )
{
    DWORD dwError = 0;
    PVM_HTTP_CLIENT pHttpClient = NULL;
    PCSTR   pszResult = NULL;
    PSTR    pszUrl = NULL;
    PSTR    pszDN = NULL;
    CHAR    pszPageSize[VMDIR_SIZE_16] = {0};

    if (!pContext || !pCase)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmHttpClientInit(&pHttpClient, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pszDN, "%s,%s", pCase->definition.pszBaseDN, pContext->pTestState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmHttpClientSetQueryParam(
                  pHttpClient,
                  VMDIR_STR_DN,
                  pszDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmHttpClientSetQueryParam(
                  pHttpClient,
                  VMDIR_STR_SCOPE,
                  SCOPE_NUM_TO_STR(pCase->definition.iScope));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmHttpClientSetQueryParam(
                  pHttpClient,
                  VMDIR_STR_FILTER,
                  pCase->definition.pszFilter);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pCase->definition.dwPageSize > 0)
    {
        dwError = VmDirStringPrintFA(
            pszPageSize,
            sizeof(pszPageSize)-1,
            "%d",
            pCase->definition.dwPageSize);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmHttpClientSetQueryParam(
                      pHttpClient,
                      VMDIR_STR_PAGE_SIZE,
                      pszPageSize);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pOpCtx->pszRestPagedCookie)
    {
        dwError = VmHttpClientSetQueryParam(
                      pHttpClient,
                      VMDiR_STR_PAGE_COOKIE,
                      pOpCtx->pszRestPagedCookie);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmFormatUrl(
        "https",
        pContext->pTestState->pszServerName,
        DEFAULT_HTTPS_PORT_NUM,
        VMDIR_REST_LDAP_BASE,
        NULL,
        &pszUrl);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmHttpClientSetToken(
        pHttpClient,
        VMHTTP_TOKEN_TYPE_BEARER,
        pContext->pTestState->pszAdminAccessToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmHttpClientPerform(pHttpClient, VMHTTP_METHOD_GET, pszUrl);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmHttpClientGetResult(pHttpClient, &pszResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _TestMapSearchResult(pszResult, pOpCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmHttpClientFreeHandle(pHttpClient);
    VMDIR_SAFE_FREE_MEMORY(pszUrl);
    VMDIR_SAFE_FREE_MEMORY(pszDN);

    return dwError;

error:
    goto cleanup;
}

static
DWORD
_TestValidateRestSearchResult(
    PVMDIR_SEARCH_TEST_CASE     pCase,
    PVMDIR_SEARCH_OP_CONTEXT    pOpCtx
    )
{
    DWORD   dwError = 0;


    if (pOpCtx->dwResultCode)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_TEST_SEARCH_ERROR_RESULT_CODE);
    }

    if (pCase->result.iNumEntryReceived != pOpCtx->dwTotalResultCount)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_TEST_SEARCH_ERROR_TOTAL_ENTRY);
    }

cleanup:
    return dwError;

error:
    printf("failed test desc: %s ...\n", pCase->definition.pszDesc);
    printf("failed rest test result: result %d received %d,\n",
        pOpCtx->dwResultCode,
        pOpCtx->dwTotalResultCount);

    goto cleanup;
}
