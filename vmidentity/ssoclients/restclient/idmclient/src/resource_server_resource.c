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
IdmResourceServerRegister(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    const IDM_RESOURCE_SERVER_DATA* pResourceServer,
    IDM_RESOURCE_SERVER_DATA** ppResourceServerReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    IDM_RESOURCE_SERVER_DATA* pResourceServerReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || pResourceServer == NULL
        || ppResourceServerReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_URI,
        tenant,
        "resourceserver",
        NULL,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        pResourceServer,
        (DataObjectToJsonFunc) IdmResourceServerDataToJson,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_POST,
        (JsonToDataObjectFunc) IdmJsonToResourceServerData,
        (void**) &pResourceServerReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppResourceServerReturn = pResourceServerReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppResourceServerReturn, (DataObjectToJsonFunc) IdmResourceServerDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmResourceServerDataDelete(pResourceServerReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmResourceServerGetAll(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    IDM_RESOURCE_SERVER_ARRAY_DATA** ppResourceServerArrayReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    IDM_RESOURCE_SERVER_ARRAY_DATA* pResourceServerArrayReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || ppResourceServerArrayReturn == NULL
        || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_POST_URI,
        tenant,
        "resourceserver",
        NULL,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        NULL,
        NULL,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_POST,
        (JsonToDataObjectFunc) IdmJsonToResourceServerArrayData,
        (void**) &pResourceServerArrayReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppResourceServerArrayReturn = pResourceServerArrayReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonArray(*ppResourceServerArrayReturn, (DataObjectToJsonFunc) IdmResourceServerArrayDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmResourceServerArrayDataDelete(pResourceServerArrayReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmResourceServerGet(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    IDM_RESOURCE_SERVER_DATA** ppResourceServerReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    IDM_RESOURCE_SERVER_DATA* pResourceServerReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || IS_NULL_OR_EMPTY_STRING(name)
        || ppResourceServerReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_POST_URI,
        tenant,
        "resourceserver",
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
        (JsonToDataObjectFunc) IdmJsonToResourceServerData,
        (void**) &pResourceServerReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppResourceServerReturn = pResourceServerReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppResourceServerReturn, (DataObjectToJsonFunc) IdmResourceServerDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmResourceServerDataDelete(pResourceServerReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmResourceServerUpdate(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    const IDM_RESOURCE_SERVER_DATA* pResourceServer,
    IDM_RESOURCE_SERVER_DATA** ppResourceServerReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    IDM_RESOURCE_SERVER_DATA* pResourceServerReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || IS_NULL_OR_EMPTY_STRING(name)
        || pResourceServer == NULL || ppResourceServerReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_URI,
        tenant,
        "resourceserver",
        name,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        pResourceServer,
        (DataObjectToJsonFunc) IdmResourceServerDataToJson,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_PUT,
        (JsonToDataObjectFunc) IdmJsonToResourceServerData,
        (void**) &pResourceServerReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppResourceServerReturn = pResourceServerReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppResourceServerReturn, (DataObjectToJsonFunc) IdmResourceServerDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmResourceServerDataDelete(pResourceServerReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmResourceServerDelete(
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
        "resourceserver",
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
