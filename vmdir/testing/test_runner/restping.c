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
_TestMapPingResult(
    PCSTR   pszResult
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

    if (searchResp.iLdapCode != 0 || searchResp.iResultCount != 1)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
    }
error:
    return dwError;
}

DWORD
VmDirTestRestPing(
    PCSTR pszServer,
    DWORD dwPort,
    PCSTR pszToken,
    PCSTR pszCAPath
    )
{
    DWORD dwError = 0;
    PVM_HTTP_CLIENT pHttpClient = NULL;
    PCSTR   pszResult = NULL;
    PSTR    pszUrl = NULL;

    if (!pszServer || !pszToken)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmHttpClientInit(&pHttpClient, pszCAPath);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmHttpClientSetQueryParam(
                  pHttpClient,
                  VMDIR_STR_DN,
                  PERSISTED_DSE_ROOT_DN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmHttpClientSetQueryParam(
                  pHttpClient,
                  VMDIR_STR_SCOPE,
                  VMDIR_STR_SCOPE_BASE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmFormatUrl(
        "https",
        pszServer,
        dwPort,
        VMDIR_REST_LDAP_BASE,
        NULL,
        &pszUrl);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmHttpClientSetToken(
        pHttpClient,
        VMHTTP_TOKEN_TYPE_BEARER,
        pszToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmHttpClientPerform(pHttpClient, VMHTTP_METHOD_GET, pszUrl);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmHttpClientGetResult(pHttpClient, &pszResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _TestMapPingResult(pszResult);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmHttpClientFreeHandle(pHttpClient);
    VMDIR_SAFE_FREE_MEMORY(pszUrl);
    return dwError;

error:
    goto cleanup;
}

