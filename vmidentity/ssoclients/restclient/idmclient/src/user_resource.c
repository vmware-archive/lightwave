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

static PCSTRING const TENANT_URI = "/idm/tenant";
static PCSTRING const TENANT_POST_URI = "/idm/post/tenant";

SSOERROR
IdmExternalUserCreate(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    const IDM_PRINCIPAL_DATA* pPrincipal,
    bool** ppCreated,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    bool* pCreated = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || pPrincipal == NULL
        || ppCreated == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_URI,
        tenant,
        "users",
        NULL,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        pPrincipal,
        (DataObjectToJsonFunc) IdmPrincipalDataToJson,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_POST,
        (JsonToDataObjectFunc) RestJsonToBooleanData,
        (void**) &pCreated,
        &pError);
    BAIL_ON_ERROR(e);

    *ppCreated = pCreated;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppCreated, (DataObjectToJsonFunc) RestBooleanDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        RestBooleanDataDelete(pCreated);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmUserGet(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    IDM_USER_DATA** ppUserReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING upn = NULL;
    PSTRING resourceUri = NULL;
    IDM_USER_DATA* pUserReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || IS_NULL_OR_EMPTY_STRING(name)
        || IS_NULL_OR_EMPTY_STRING(domain) || ppUserReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildUPN(name, domain, &upn);
    BAIL_ON_ERROR(e);

    e = RestBuildResourceUri(
        pClient,
        TENANT_POST_URI,
        tenant,
        "users",
        upn,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        NULL,
        NULL,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_POST,
        (JsonToDataObjectFunc) IdmJsonToUserData,
        (void**) &pUserReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppUserReturn = pUserReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppUserReturn, (DataObjectToJsonFunc) IdmUserDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmUserDataDelete(pUserReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(upn);
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmUserGetGroups(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    bool nested,
    IDM_GROUP_ARRAY_DATA** ppGroupArrayReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING upn = NULL;
    PSTRING resourceUri = NULL;
    IDM_GROUP_ARRAY_DATA* pGroupArrayReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || IS_NULL_OR_EMPTY_STRING(name)
        || IS_NULL_OR_EMPTY_STRING(domain) || ppGroupArrayReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildUPN(name, domain, &upn);
    BAIL_ON_ERROR(e);

    e = RestBuildResourceUri(
        pClient,
        TENANT_POST_URI,
        tenant,
        "users",
        upn,
        "groups",
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestAppendQueryStringOnResourceUri("nested", nested ? "TRUE" : "FALSE", true, resourceUri, &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        NULL,
        NULL,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_POST,
        (JsonToDataObjectFunc) IdmJsonToGroupArrayData,
        (void**) &pGroupArrayReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppGroupArrayReturn = pGroupArrayReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonArray(*ppGroupArrayReturn, (DataObjectToJsonFunc) IdmGroupArrayDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmGroupArrayDataDelete(pGroupArrayReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(upn);
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmExternalUserDelete(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING upn = NULL;
    PSTRING resourceUri = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || IS_NULL_OR_EMPTY_STRING(name)
        || IS_NULL_OR_EMPTY_STRING(domain) || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildUPN(name, domain, &upn);
    BAIL_ON_ERROR(e);

    e = RestBuildResourceUri(
        pClient,
        TENANT_URI,
        tenant,
        "users",
        upn,
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
    SSOStringFree(upn);
    SSOStringFree(resourceUri);

    return e;
}
