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

static PCSTRING const TENANT_URI = "/vmdir/tenant";
static PCSTRING const TENANT_POST_URI = "/vmdir/post/tenant";

SSOERROR
VmdirSolutionUserCreate(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    const VMDIR_SOLUTION_USER_DATA* pSolutionUser,
    VMDIR_SOLUTION_USER_DATA** ppSolutionUserReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    VMDIR_SOLUTION_USER_DATA* pSolutionUserReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || pSolutionUser == NULL
        || ppSolutionUserReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_URI,
        tenant,
        "solutionusers",
        NULL,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        pSolutionUser,
        (DataObjectToJsonFunc) VmdirSolutionUserDataToJson,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_POST,
        (JsonToDataObjectFunc) VmdirJsonToSolutionUserData,
        (void**) &pSolutionUserReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppSolutionUserReturn = pSolutionUserReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppSolutionUserReturn, (DataObjectToJsonFunc) VmdirSolutionUserDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        VmdirSolutionUserDataDelete(pSolutionUserReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
VmdirSolutionUserGet(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    VMDIR_SOLUTION_USER_DATA** ppSolutionUserReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    VMDIR_SOLUTION_USER_DATA* pSolutionUserReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || IS_NULL_OR_EMPTY_STRING(name)
        || ppSolutionUserReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_POST_URI,
        tenant,
        "solutionusers",
        name,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        NULL,
        NULL,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_POST,
        (JsonToDataObjectFunc) VmdirJsonToSolutionUserData,
        (void**) &pSolutionUserReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppSolutionUserReturn = pSolutionUserReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppSolutionUserReturn, (DataObjectToJsonFunc) VmdirSolutionUserDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        VmdirSolutionUserDataDelete(pSolutionUserReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
VmdirSolutionUserUpdate(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    const VMDIR_SOLUTION_USER_DATA* pSolutionUser,
    VMDIR_SOLUTION_USER_DATA** ppSolutionUserReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    VMDIR_SOLUTION_USER_DATA* pSolutionUserReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || pSolutionUser == NULL
        || ppSolutionUserReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_URI,
        tenant,
        "solutionusers",
        name,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        pSolutionUser,
        (DataObjectToJsonFunc) VmdirSolutionUserDataToJson,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_PUT,
        (JsonToDataObjectFunc) VmdirJsonToSolutionUserData,
        (void**) &pSolutionUserReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppSolutionUserReturn = pSolutionUserReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppSolutionUserReturn, (DataObjectToJsonFunc) VmdirSolutionUserDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        VmdirSolutionUserDataDelete(pSolutionUserReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
VmdirSolutionUserDelete(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || IS_NULL_OR_EMPTY_STRING(name)
        || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_URI,
        tenant,
        "solutionusers",
        name,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        NULL,
        NULL,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_DELETE,
        NULL,
        NULL,
        &pError);
    BAIL_ON_ERROR(e);

    error:

    if (e != SSOERROR_NONE)
    {
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}
