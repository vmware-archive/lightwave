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

static PCSTRING IDM_MEMBER_TYPE_ENUMS[] =
{
    "USER",
    "GROUP",
    "SOLUTIONUSER",
    "ALL"
};

static PCSTRING const TENANT_POST_URI = "/idm/post/tenant";

SSOERROR
IdmGroupGet(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    IDM_GROUP_DATA** ppGroupReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING upn = NULL;
    PSTRING resourceUri = NULL;
    IDM_GROUP_DATA* pGroupReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || IS_NULL_OR_EMPTY_STRING(name)
        || IS_NULL_OR_EMPTY_STRING(domain) || ppGroupReturn == NULL || ppError == NULL)
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
        "groups",
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
        (JsonToDataObjectFunc) IdmJsonToGroupData,
        (void**) &pGroupReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppGroupReturn = pGroupReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppGroupReturn, (DataObjectToJsonFunc) IdmGroupDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        IdmGroupDataDelete(pGroupReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(upn);
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmGroupGetMembers(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    IDM_MEMBER_TYPE memberType,
    size_t limit,
    IDM_SEARCH_RESULT_DATA** ppSearchResultReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING upn = NULL;
    PSTRING s = NULL;
    PSTRING resourceUri = NULL;
    IDM_SEARCH_RESULT_DATA* pSearchResultReturn = NULL;
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
        TENANT_POST_URI,
        tenant,
        "groups",
        upn,
        "members",
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestAppendQueryStringOnResourceUri("type", IDM_MEMBER_TYPE_ENUMS[memberType], true, resourceUri, &resourceUri);
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
    SSOStringFree(upn);
    SSOStringFree(s);
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
IdmGroupGetParents(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
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
        "groups",
        upn,
        "parents",
        &resourceUri);
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
