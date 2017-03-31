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

static PCSTRING IDM_TENANT_CONFIG_TYPE_ENUMS[] =
{
    "ALL",
    "LOCKOUT",
    "PASSWORD",
    "PROVIDER",
    "TOKEN",
    "BRAND",
    "AUTHENTICATION"
};

static PCSTRING IDM_MEMBER_TYPE_ENUMS[] =
{
    "USER",
    "GROUP",
    "SOLUTIONUSER",
    "ALL"
};

static PCSTRING IDM_SEARCH_TYPE_ENUMS[] =
{
    "CERT_SUBJECTDN",
    "NAME"
};

static PCSTRING const TENANT_URI = "/idm/tenant";
static PCSTRING const TENANT_POST_URI = "/idm/post/tenant";

SSOERROR
IdmTenantCreate(
    PCREST_CLIENT pClient,
    const IDM_TENANT_DATA* pTenant,
    IDM_TENANT_DATA** ppTenantReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    IDM_TENANT_DATA* pTenantReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || pTenant == NULL || ppTenantReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_URI,
        NULL,
        NULL,
        NULL,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        pTenant,
        (DataObjectToJsonFunc) IdmTenantDataToJson,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_POST,
        (JsonToDataObjectFunc) IdmJsonToTenantData,
        (void**) &pTenantReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppTenantReturn = pTenantReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppTenantReturn, (DataObjectToJsonFunc) IdmTenantDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmTenantDataDelete(pTenantReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmTenantGet(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    IDM_TENANT_DATA** ppTenantReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    IDM_TENANT_DATA* pTenantReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || ppTenantReturn == NULL
        || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_POST_URI,
        tenant,
        NULL,
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
        (JsonToDataObjectFunc) IdmJsonToTenantData,
        (void**) &pTenantReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppTenantReturn = pTenantReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppTenantReturn, (DataObjectToJsonFunc) IdmTenantDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmTenantDataDelete(pTenantReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmTenantDelete(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_URI,
        tenant,
        NULL,
        NULL,
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

SSOERROR
IdmTenantGetConfig(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    IDM_TENANT_CONFIG_TYPE_ARRAY* pTenantConfigTypes,
    IDM_TENANT_CONFIGURATION_DATA** ppTenantConfigurationReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    IDM_TENANT_CONFIGURATION_DATA* pTenantConfigurationReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;
    size_t i = 0;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || ppTenantConfigurationReturn == NULL
        || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_POST_URI,
        tenant,
        "config",
        NULL,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    for (i = 0; i < pTenantConfigTypes->length; i++)
    {
        e = RestAppendQueryStringOnResourceUri(
            "type",
            IDM_TENANT_CONFIG_TYPE_ENUMS[*(pTenantConfigTypes->ppEntry[i])],
            i == 0,
            resourceUri,
            &resourceUri);
        BAIL_ON_ERROR(e);
    }

    e = RestBuildAndExecuteHttp(
        NULL,
        NULL,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_POST,
        (JsonToDataObjectFunc) IdmJsonToTenantConfigurationData,
        (void**) &pTenantConfigurationReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppTenantConfigurationReturn = pTenantConfigurationReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppTenantConfigurationReturn, (DataObjectToJsonFunc) IdmTenantConfigurationDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmTenantConfigurationDataDelete(pTenantConfigurationReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmTenantUpdateConfig(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    const IDM_TENANT_CONFIGURATION_DATA* pTenantConfiguration,
    IDM_TENANT_CONFIGURATION_DATA** ppTenantConfigurationReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    IDM_TENANT_CONFIGURATION_DATA* pTenantConfigurationReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || pTenantConfiguration == NULL
        || ppTenantConfigurationReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_URI,
        tenant,
        "config",
        NULL,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        pTenantConfiguration,
        (DataObjectToJsonFunc) IdmTenantConfigurationDataToJson,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_PUT,
        (JsonToDataObjectFunc) IdmJsonToTenantConfigurationData,
        (void**) &pTenantConfigurationReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppTenantConfigurationReturn = pTenantConfigurationReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppTenantConfigurationReturn, (DataObjectToJsonFunc) IdmTenantConfigurationDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmTenantConfigurationDataDelete(pTenantConfigurationReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmTenantSearch(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING domain,
    PCSTRING query,
    IDM_MEMBER_TYPE memberType,
    IDM_SEARCH_TYPE searchBy,
    size_t limit,
    IDM_SEARCH_RESULT_DATA** ppSearchResultReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING s = NULL;
    PSTRING resourceUri = NULL;
    IDM_SEARCH_RESULT_DATA* pSearchResultReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || ppSearchResultReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_POST_URI,
        tenant,
        "search",
        NULL,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestAppendQueryStringOnResourceUri("domain", domain, true, resourceUri, &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestAppendQueryStringOnResourceUri("query", query, false, resourceUri, &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestAppendQueryStringOnResourceUri("type", IDM_MEMBER_TYPE_ENUMS[memberType], false, resourceUri, &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestAppendQueryStringOnResourceUri("searchBy", IDM_SEARCH_TYPE_ENUMS[searchBy], false, resourceUri, &resourceUri);
    BAIL_ON_ERROR(e);

    e = SSOStringAllocateFromInt((int) limit, &s);
    BAIL_ON_ERROR(e);

    e = RestAppendQueryStringOnResourceUri("limit", s, false, resourceUri, &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        NULL,
        NULL,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_POST,
        (JsonToDataObjectFunc) IdmJsonToSearchResultData,
        (void**) &pSearchResultReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppSearchResultReturn = pSearchResultReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppSearchResultReturn, (DataObjectToJsonFunc) IdmSearchResultDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmSearchResultDataDelete(pSearchResultReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(s);
    SSOStringFree(resourceUri);

    return e;
}
