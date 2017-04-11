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
IdmOidcClientRegister(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    const IDM_OIDC_CLIENT_METADATA_DATA* pOidcClientMetadata,
    IDM_OIDC_CLIENT_DATA** ppOidcClientReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    IDM_OIDC_CLIENT_DATA* pOidcClientReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || pOidcClientMetadata == NULL
        || ppOidcClientReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_URI,
        tenant,
        "oidcclient",
        NULL,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        pOidcClientMetadata,
        (DataObjectToJsonFunc) IdmOidcClientMetadataDataToJson,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_POST,
        (JsonToDataObjectFunc) IdmJsonToOidcClientData,
        (void**) &pOidcClientReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppOidcClientReturn = pOidcClientReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppOidcClientReturn, (DataObjectToJsonFunc) IdmOidcClientDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmOidcClientDataDelete(pOidcClientReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmOidcClientGetAll(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    IDM_OIDC_CLIENT_ARRAY_DATA** ppOidcClientArrayReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    IDM_OIDC_CLIENT_ARRAY_DATA* pOidcClientArrayReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || ppOidcClientArrayReturn == NULL
        || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_POST_URI,
        tenant,
        "oidcclient",
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
        (JsonToDataObjectFunc) IdmJsonToOidcClientArrayData,
        (void**) &pOidcClientArrayReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppOidcClientArrayReturn = pOidcClientArrayReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonArray(*ppOidcClientArrayReturn, (DataObjectToJsonFunc) IdmOidcClientArrayDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmOidcClientArrayDataDelete(pOidcClientArrayReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmOidcClientGet(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING clientId,
    IDM_OIDC_CLIENT_DATA** ppOidcClientReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    IDM_OIDC_CLIENT_DATA* pOidcClientReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || IS_NULL_OR_EMPTY_STRING(clientId)
        || ppOidcClientReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_POST_URI,
        tenant,
        "oidcclient",
        clientId,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        NULL,
        NULL,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_POST,
        (JsonToDataObjectFunc) IdmJsonToOidcClientData,
        (void**) &pOidcClientReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppOidcClientReturn = pOidcClientReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppOidcClientReturn, (DataObjectToJsonFunc) IdmOidcClientDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmOidcClientDataDelete(pOidcClientReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmOidcClientUpdate(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING clientId,
    const IDM_OIDC_CLIENT_METADATA_DATA* pOidcClientMetadata,
    IDM_OIDC_CLIENT_DATA** ppOidcClientReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    IDM_OIDC_CLIENT_DATA* pOidcClientReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || IS_NULL_OR_EMPTY_STRING(clientId)
        || pOidcClientMetadata == NULL || ppOidcClientReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_URI,
        tenant,
        "oidcclient",
        clientId,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        pOidcClientMetadata,
        (DataObjectToJsonFunc) IdmOidcClientMetadataDataToJson,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_PUT,
        (JsonToDataObjectFunc) IdmJsonToOidcClientData,
        (void**) &pOidcClientReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppOidcClientReturn = pOidcClientReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppOidcClientReturn, (DataObjectToJsonFunc) IdmOidcClientDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmOidcClientDataDelete(pOidcClientReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmOidcClientDelete(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING clientId,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || IS_NULL_OR_EMPTY_STRING(clientId)
        || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_URI,
        tenant,
        "oidcclient",
        clientId,
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
