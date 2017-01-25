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

static PCSTRING const TENANT_URI = "/vmdir/tenant";
static PCSTRING const TENANT_POST_URI = "/vmdir/post/tenant";

SSOERROR
VmdirGroupCreate(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    const VMDIR_GROUP_DATA* pGroup,
    VMDIR_GROUP_DATA** ppGroupReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    VMDIR_GROUP_DATA* pGroupReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || pGroup == NULL
        || ppGroupReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        TENANT_URI,
        tenant,
        "groups",
        NULL,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        pGroup,
        (DataObjectToJsonFunc) VmdirGroupDataToJson,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_POST,
        (JsonToDataObjectFunc) VmdirJsonToGroupData,
        (void**) &pGroupReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppGroupReturn = pGroupReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppGroupReturn, (DataObjectToJsonFunc) VmdirGroupDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        VmdirGroupDataDelete(pGroupReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
VmdirGroupGet(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    VMDIR_GROUP_DATA** ppGroupReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING upn = NULL;
    PSTRING resourceUri = NULL;
    VMDIR_GROUP_DATA* pGroupReturn = NULL;
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
        (JsonToDataObjectFunc) VmdirJsonToGroupData,
        (void**) &pGroupReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppGroupReturn = pGroupReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppGroupReturn, (DataObjectToJsonFunc) VmdirGroupDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        VmdirGroupDataDelete(pGroupReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(upn);
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
VmdirGroupUpdate(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    const VMDIR_GROUP_DATA* pGroup,
    VMDIR_GROUP_DATA** ppGroupReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING upn = NULL;
    PSTRING resourceUri = NULL;
    VMDIR_GROUP_DATA* pGroupReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || IS_NULL_OR_EMPTY_STRING(name)
        || IS_NULL_OR_EMPTY_STRING(domain) || pGroup == NULL || ppGroupReturn == NULL
        || ppError == NULL)
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
        "groups",
        upn,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        pGroup,
        (DataObjectToJsonFunc) VmdirGroupDataToJson,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_PUT,
        (JsonToDataObjectFunc) VmdirJsonToGroupData,
        (void**) &pGroupReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppGroupReturn = pGroupReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppGroupReturn, (DataObjectToJsonFunc) VmdirGroupDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        VmdirGroupDataDelete(pGroupReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(upn);
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
VmdirGroupDelete(
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

SSOERROR
VmdirGroupAddMembers(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    VMDIR_STRING_ARRAY_DATA* pMembers,
    VMDIR_MEMBER_TYPE memberType,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING upn = NULL;
    PSTRING resourceUri = NULL;
    REST_SERVER_ERROR* pError = NULL;
    size_t i = 0;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || IS_NULL_OR_EMPTY_STRING(name)
        || IS_NULL_OR_EMPTY_STRING(domain) || pMembers == NULL || ppError == NULL)
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
        "groups",
        upn,
        "members",
        &resourceUri);
    BAIL_ON_ERROR(e);

    for (i = 0; i < pMembers->length; i++)
    {
        e = RestAppendQueryStringOnResourceUri("members", pMembers->ppEntry[i], i == 0, resourceUri, &resourceUri);
        BAIL_ON_ERROR(e);
    }

    e = RestAppendQueryStringOnResourceUri("type", IDM_MEMBER_TYPE_ENUMS[memberType], false, resourceUri, &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        NULL,
        NULL,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_PUT,
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

SSOERROR
VmdirGroupGetMembers(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    VMDIR_MEMBER_TYPE memberType,
    size_t limit,
    VMDIR_SEARCH_RESULT_DATA** ppSearchResultReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING upn = NULL;
    PSTRING s = NULL;
    PSTRING resourceUri = NULL;
    VMDIR_SEARCH_RESULT_DATA* pSearchResultReturn = NULL;
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
        (JsonToDataObjectFunc) VmdirJsonToSearchResultData,
        (void**) &pSearchResultReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppSearchResultReturn = pSearchResultReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppSearchResultReturn, (DataObjectToJsonFunc) VmdirSearchResultDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        VmdirSearchResultDataDelete(pSearchResultReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(upn);
    SSOStringFree(s);
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
VmdirGroupRemoveMembers(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    VMDIR_STRING_ARRAY_DATA* pMembers,
    VMDIR_MEMBER_TYPE memberType,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING upn = NULL;
    PSTRING resourceUri = NULL;
    REST_SERVER_ERROR* pError = NULL;
    size_t i = 0;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || IS_NULL_OR_EMPTY_STRING(name)
        || IS_NULL_OR_EMPTY_STRING(domain) || pMembers == NULL || ppError == NULL)
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
        "groups",
        upn,
        "members",
        &resourceUri);
    BAIL_ON_ERROR(e);

    for (i = 0; i < pMembers->length; i++)
    {
        e = RestAppendQueryStringOnResourceUri("members", pMembers->ppEntry[i], i == 0, resourceUri, &resourceUri);
        BAIL_ON_ERROR(e);
    }

    e = RestAppendQueryStringOnResourceUri("type", IDM_MEMBER_TYPE_ENUMS[memberType], false, resourceUri, &resourceUri);
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

SSOERROR
VmdirGroupGetParents(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    VMDIR_GROUP_ARRAY_DATA** ppGroupArrayReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING upn = NULL;
    PSTRING resourceUri = NULL;
    VMDIR_GROUP_ARRAY_DATA* pGroupArrayReturn = NULL;
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
        (JsonToDataObjectFunc) VmdirJsonToGroupArrayData,
        (void**) &pGroupArrayReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppGroupArrayReturn = pGroupArrayReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonArray(*ppGroupArrayReturn, (DataObjectToJsonFunc) VmdirGroupArrayDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        VmdirGroupArrayDataDelete(pGroupArrayReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(upn);
    SSOStringFree(resourceUri);

    return e;
}
