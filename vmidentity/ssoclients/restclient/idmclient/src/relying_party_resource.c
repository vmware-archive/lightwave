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
IdmRelyingPartyRegister(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    const IDM_RELYING_PARTY_DATA* pRelyingParty,
    IDM_RELYING_PARTY_DATA** ppRelyingPartyReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    IDM_RELYING_PARTY_DATA* pRelyingPartyReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || pRelyingParty == NULL
        || ppRelyingPartyReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_URI,
        tenant,
        "relyingparty",
        NULL,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        pRelyingParty,
        (DataObjectToJsonFunc) IdmRelyingPartyDataToJson,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_POST,
        (JsonToDataObjectFunc) IdmJsonToRelyingPartyData,
        (void**) &pRelyingPartyReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppRelyingPartyReturn = pRelyingPartyReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppRelyingPartyReturn, (DataObjectToJsonFunc) IdmRelyingPartyDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmRelyingPartyDataDelete(pRelyingPartyReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmRelyingPartyGetAll(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    IDM_RELYING_PARTY_ARRAY_DATA** ppRelyingPartyArrayReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    IDM_RELYING_PARTY_ARRAY_DATA* pRelyingPartyArrayReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || ppRelyingPartyArrayReturn == NULL
        || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_POST_URI,
        tenant,
        "relyingparty",
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
        (JsonToDataObjectFunc) IdmJsonToRelyingPartyArrayData,
        (void**) &pRelyingPartyArrayReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppRelyingPartyArrayReturn = pRelyingPartyArrayReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonArray(*ppRelyingPartyArrayReturn, (DataObjectToJsonFunc) IdmRelyingPartyArrayDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmRelyingPartyArrayDataDelete(pRelyingPartyArrayReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmRelyingPartyGet(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    IDM_RELYING_PARTY_DATA** ppRelyingPartyReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    IDM_RELYING_PARTY_DATA* pRelyingPartyReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || IS_NULL_OR_EMPTY_STRING(name)
        || ppRelyingPartyReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_POST_URI,
        tenant,
        "relyingparty",
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
        (JsonToDataObjectFunc) IdmJsonToRelyingPartyData,
        (void**) &pRelyingPartyReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppRelyingPartyReturn = (IDM_RELYING_PARTY_DATA*) pRelyingPartyReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppRelyingPartyReturn, (DataObjectToJsonFunc) IdmRelyingPartyDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmRelyingPartyDataDelete(pRelyingPartyReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmRelyingPartyUpdate(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    const IDM_RELYING_PARTY_DATA* pRelyingParty,
    IDM_RELYING_PARTY_DATA** ppRelyingPartyReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    IDM_RELYING_PARTY_DATA* pRelyingPartyReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || IS_NULL_OR_EMPTY_STRING(name)
        || pRelyingParty == NULL || ppRelyingPartyReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_URI,
        tenant,
        "relyingparty",
        name,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        pRelyingParty,
        (DataObjectToJsonFunc) IdmRelyingPartyDataToJson,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_PUT,
        (JsonToDataObjectFunc) IdmJsonToRelyingPartyData,
        (void**) &pRelyingPartyReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppRelyingPartyReturn = pRelyingPartyReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppRelyingPartyReturn, (DataObjectToJsonFunc) IdmRelyingPartyDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmRelyingPartyDataDelete(pRelyingPartyReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmRelyingPartyDelete(
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
        "relyingparty",
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
