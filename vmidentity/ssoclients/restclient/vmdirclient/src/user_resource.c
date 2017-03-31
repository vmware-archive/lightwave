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
VmdirUserCreate(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    const VMDIR_USER_DATA* pUser,
    VMDIR_USER_DATA** ppUserReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    VMDIR_USER_DATA* pUserReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || pUser == NULL
        || ppUserReturn == NULL || ppError == NULL)
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
        pUser,
        (DataObjectToJsonFunc) VmdirUserDataToJson,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_POST,
        (JsonToDataObjectFunc) VmdirJsonToUserData,
        (void**) &pUserReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppUserReturn = pUserReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppUserReturn, (DataObjectToJsonFunc) VmdirUserDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        VmdirUserDataDelete(pUserReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
VmdirUserGet(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    VMDIR_USER_DATA** ppUserReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING upn = NULL;
    PSTRING resourceUri = NULL;
    VMDIR_USER_DATA* pUserReturn = NULL;
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
        (JsonToDataObjectFunc) VmdirJsonToUserData,
        (void**) &pUserReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppUserReturn = pUserReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppUserReturn, (DataObjectToJsonFunc) VmdirUserDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        VmdirUserDataDelete(pUserReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(upn);
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
VmdirUserGetGroups(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    bool nested,
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

SSOERROR
VmdirUserUpdate(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    const VMDIR_USER_DATA* pUser,
    VMDIR_USER_DATA** ppUserReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING upn = NULL;
    PSTRING resourceUri = NULL;
    VMDIR_USER_DATA* pUserReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || IS_NULL_OR_EMPTY_STRING(name)
        || IS_NULL_OR_EMPTY_STRING(domain) || pUser == NULL || ppUserReturn == NULL
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
        "users",
        upn,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        pUser,
        (DataObjectToJsonFunc) VmdirUserDataToJson,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_PUT,
        (JsonToDataObjectFunc) VmdirJsonToUserData,
        (void**) &pUserReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppUserReturn = pUserReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppUserReturn, (DataObjectToJsonFunc) VmdirUserDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        VmdirUserDataDelete(pUserReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(upn);
    SSOStringFree(resourceUri);

    return e;
}

static SSOERROR
VmdirUserUpdatePasswordInternal(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    PCSTRING currentPassword,
    PCSTRING newPassword,
    VMDIR_USER_DATA** ppUserReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING upn = NULL;
    PSTRING resourceUri = NULL;
    VMDIR_PASSWORD_RESET_REQUEST_DATA* pPasswordResetRequest = NULL;
    VMDIR_USER_DATA* pUserReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || IS_NULL_OR_EMPTY_STRING(tenant) || IS_NULL_OR_EMPTY_STRING(name)
        || IS_NULL_OR_EMPTY_STRING(domain) || IS_NULL_OR_EMPTY_STRING(newPassword) || ppUserReturn == NULL
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
        "users",
        upn,
        "password",
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = VmdirPasswordResetRequestDataNew(&pPasswordResetRequest, currentPassword, newPassword);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        pPasswordResetRequest,
        (DataObjectToJsonFunc) VmdirPasswordResetRequestDataToJson,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_PUT,
        (JsonToDataObjectFunc) VmdirJsonToUserData,
        (void**) &pUserReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppUserReturn = pUserReturn;

    error:

    if (e != SSOERROR_NONE)
    {
        VmdirUserDataDelete(pUserReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(upn);
    SSOStringFree(resourceUri);
    VmdirPasswordResetRequestDataDelete(pPasswordResetRequest);

    return e;
}

SSOERROR
VmdirUserUpdatePassword(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    PCSTRING currentPassword,
    PCSTRING newPassword,
    VMDIR_USER_DATA** ppUserReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    VMDIR_USER_DATA* pUserReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    e = VmdirUserUpdatePasswordInternal(
        pClient,
        tenant,
        name,
        domain,
        currentPassword,
        newPassword,
        &pUserReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppUserReturn = pUserReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppUserReturn, (DataObjectToJsonFunc) VmdirUserDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        VmdirUserDataDelete(pUserReturn);
        *ppError = pError;
    }

    return e;
}

SSOERROR
VmdirUserResetPassword(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    PCSTRING newPassword,
    VMDIR_USER_DATA** ppUserReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    VMDIR_USER_DATA* pUserReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    e = VmdirUserUpdatePasswordInternal(pClient, tenant, name, domain, NULL, newPassword, &pUserReturn, &pError);
    BAIL_ON_ERROR(e);

    *ppUserReturn = pUserReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(*ppUserReturn, (DataObjectToJsonFunc) VmdirUserDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        VmdirUserDataDelete(pUserReturn);
        *ppError = pError;
    }

    return e;
}

SSOERROR
VmdirUserDelete(
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
