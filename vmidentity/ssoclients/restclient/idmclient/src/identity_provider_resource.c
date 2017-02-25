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

static SSOERROR
IdmIdentityProviderCreateInternal(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    const IDM_IDENTITY_PROVIDER_DATA* pIdentityProvider,
    bool isProbe,
    IDM_IDENTITY_PROVIDER_DATA** ppIdentityProviderReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    IDM_IDENTITY_PROVIDER_DATA* pIdentityProviderReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || pIdentityProvider == NULL
        || ppIdentityProviderReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_URI,
        tenant,
        "providers",
        NULL,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    if (isProbe == true)
    {
        e = RestAppendQueryStringOnResourceUri("probe", "TRUE", true, resourceUri, &resourceUri);
        BAIL_ON_ERROR(e);
    }
    else
    {
        e = RestAppendQueryStringOnResourceUri("probe", "FALSE", true, resourceUri, &resourceUri);
        BAIL_ON_ERROR(e);
    }

    e = RestBuildAndExecuteHttp(
        pIdentityProvider,
        (DataObjectToJsonFunc) IdmIdentityProviderDataToJson,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_POST,
        (JsonToDataObjectFunc) IdmJsonToIdentityProviderData,
        (void**) &pIdentityProviderReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppIdentityProviderReturn = (IDM_IDENTITY_PROVIDER_DATA*) pIdentityProviderReturn;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmIdentityProviderDataDelete(pIdentityProviderReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmIdentityProviderCreate(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    const IDM_IDENTITY_PROVIDER_DATA* pIdentityProvider,
    IDM_IDENTITY_PROVIDER_DATA** ppIdentityProviderReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_IDENTITY_PROVIDER_DATA* pIdentityProviderReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    e = IdmIdentityProviderCreateInternal(pClient, tenant, pIdentityProvider, false, &pIdentityProviderReturn, &pError);
    BAIL_ON_ERROR(e);

    *ppIdentityProviderReturn = pIdentityProviderReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppIdentityProviderReturn, (DataObjectToJsonFunc) IdmIdentityProviderDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmIdentityProviderDataDelete(pIdentityProviderReturn);
        *ppError = pError;
    }

    return e;
}

SSOERROR
IdmIdentityProviderProbe(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    const IDM_IDENTITY_PROVIDER_DATA* pIdentityProvider,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_IDENTITY_PROVIDER_DATA* pIdentityProviderReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    e = IdmIdentityProviderCreateInternal(pClient, tenant, pIdentityProvider, true, &pIdentityProviderReturn, &pError);
    BAIL_ON_ERROR(e);

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(pIdentityProviderReturn, (DataObjectToJsonFunc) IdmIdentityProviderDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        *ppError = pError;
    }

    IdmIdentityProviderDataDelete(pIdentityProviderReturn);

    return e;
}

SSOERROR
IdmIdentityProviderGetAll(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    IDM_IDENTITY_PROVIDER_ARRAY_DATA** ppIdentityProviderArrayReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    IDM_IDENTITY_PROVIDER_ARRAY_DATA* pIdentityProviderArrayReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || ppIdentityProviderArrayReturn == NULL
        || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_POST_URI,
        tenant,
        "providers",
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
        (JsonToDataObjectFunc) IdmJsonToIdentityProviderArrayData,
        (void**) &pIdentityProviderArrayReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppIdentityProviderArrayReturn = (IDM_IDENTITY_PROVIDER_ARRAY_DATA*) pIdentityProviderArrayReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonArray(*ppIdentityProviderArrayReturn, (DataObjectToJsonFunc) IdmIdentityProviderArrayDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmIdentityProviderArrayDataDelete(pIdentityProviderArrayReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmIdentityProviderGet(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING provider,
    IDM_IDENTITY_PROVIDER_DATA** ppIdentityProviderReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    IDM_IDENTITY_PROVIDER_DATA* pIdentityProviderReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || IS_NULL_OR_EMPTY_STRING(provider)
        || ppIdentityProviderReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_POST_URI,
        tenant,
        "providers",
        provider,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        NULL,
        NULL,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_POST,
        (JsonToDataObjectFunc) IdmJsonToIdentityProviderData,
        (void**) &pIdentityProviderReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppIdentityProviderReturn = pIdentityProviderReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppIdentityProviderReturn, (DataObjectToJsonFunc) IdmIdentityProviderDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmIdentityProviderDataDelete(pIdentityProviderReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmIdentityProviderUpdate(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING provider,
    const IDM_IDENTITY_PROVIDER_DATA* pIdentityProvider,
    IDM_IDENTITY_PROVIDER_DATA** ppIdentityProviderReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    IDM_IDENTITY_PROVIDER_DATA* pIdentityProviderReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || IS_NULL_OR_EMPTY_STRING(provider)
        || pIdentityProvider == NULL || ppIdentityProviderReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_URI,
        tenant,
        "providers",
        provider,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        pIdentityProvider,
        (DataObjectToJsonFunc) IdmIdentityProviderDataToJson,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_PUT,
        (JsonToDataObjectFunc) IdmJsonToIdentityProviderData,
        (void**) &pIdentityProviderReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppIdentityProviderReturn = pIdentityProviderReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppIdentityProviderReturn, (DataObjectToJsonFunc) IdmIdentityProviderDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmIdentityProviderDataDelete(pIdentityProviderReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmIdentityProviderDelete(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING provider,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || IS_NULL_OR_EMPTY_STRING(provider)
        || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_URI,
        tenant,
        "providers",
        provider,
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
