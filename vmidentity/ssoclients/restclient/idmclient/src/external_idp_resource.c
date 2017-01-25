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
IdmExternalIdpRegister(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    const IDM_EXTERNAL_IDP_DATA* pExternalIdp,
    IDM_EXTERNAL_IDP_DATA** ppExternalIdpReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    IDM_EXTERNAL_IDP_DATA* pExternalIdpReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || pExternalIdp == NULL
        || ppExternalIdpReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_URI,
        tenant,
        "externalidp",
        NULL,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        pExternalIdp,
        (DataObjectToJsonFunc) IdmExternalIdpDataToJson,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_POST,
        (JsonToDataObjectFunc) IdmJsonToExternalIdpData,
        (void**) &pExternalIdpReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppExternalIdpReturn = pExternalIdpReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppExternalIdpReturn, (DataObjectToJsonFunc) IdmExternalIdpDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmExternalIdpDataDelete(pExternalIdpReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmExternalIdpRegisterByMetadata(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING metadata,
    IDM_EXTERNAL_IDP_DATA** ppExternalIdpReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    IDM_EXTERNAL_IDP_DATA* pExternalIdpReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || IS_NULL_OR_EMPTY_STRING(metadata)
        || ppExternalIdpReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_URI,
        tenant,
        "externalidp",
        NULL,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        metadata,
        (DataObjectToJsonFunc) RestStringDataToJson,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_POST,
        (JsonToDataObjectFunc) IdmJsonToExternalIdpData,
        (void**) &pExternalIdpReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppExternalIdpReturn = pExternalIdpReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppExternalIdpReturn, (DataObjectToJsonFunc) IdmExternalIdpDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmExternalIdpDataDelete(pExternalIdpReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmExternalIdpGetAll(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    IDM_EXTERNAL_IDP_ARRAY_DATA** ppExternalIdpArrayReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    IDM_EXTERNAL_IDP_ARRAY_DATA* pExternalIdpArrayReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || ppExternalIdpArrayReturn == NULL
        || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_POST_URI,
        tenant,
        "externalidp",
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
        (JsonToDataObjectFunc) IdmJsonToExternalIdpArrayData,
        (void**) &pExternalIdpArrayReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppExternalIdpArrayReturn = pExternalIdpArrayReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonArray(*ppExternalIdpArrayReturn, (DataObjectToJsonFunc) IdmExternalIdpArrayDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmExternalIdpArrayDataDelete(pExternalIdpArrayReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmExternalIdpGet(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING entityId,
    IDM_EXTERNAL_IDP_DATA** ppExternalIdpReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    IDM_EXTERNAL_IDP_DATA* pExternalIdpReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || IS_NULL_OR_EMPTY_STRING(entityId)
        || ppExternalIdpReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_POST_URI,
        tenant,
        "externalidp",
        entityId,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        NULL,
        NULL,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_POST,
        (JsonToDataObjectFunc) IdmJsonToExternalIdpData,
        (void**) &pExternalIdpReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppExternalIdpReturn = pExternalIdpReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppExternalIdpReturn, (DataObjectToJsonFunc) IdmExternalIdpDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmExternalIdpDataDelete(pExternalIdpReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmExternalIdpDelete(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING entityId,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || IS_NULL_OR_EMPTY_STRING(entityId)
        || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_URI,
        tenant,
        "externalidp",
        entityId,
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
