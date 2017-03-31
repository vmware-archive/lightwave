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

static PCSTRING const AD_PROVIDER_URI = "/afd/provider/ad";
static PCSTRING const AD_PROVIDER_POST_URI = "/afd/post/provider/ad";

SSOERROR
AfdAdProviderJoin(
    PCREST_CLIENT pClient,
    const AFD_ACTIVE_DIRECTORY_JOIN_REQUEST_DATA* pActiveDirectoryJoinRequest,
    AFD_ACTIVE_DIRECTORY_JOIN_INFO_DATA** ppActiveDirectoryJoinInfoReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    AFD_ACTIVE_DIRECTORY_JOIN_INFO_DATA* pActiveDirectoryJoinInfoReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || pActiveDirectoryJoinRequest == NULL
        || ppActiveDirectoryJoinInfoReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        AD_PROVIDER_URI,
        NULL,
        NULL,
        NULL,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        pActiveDirectoryJoinRequest,
        (DataObjectToJsonFunc) AfdActiveDirectoryJoinRequestDataToJson,
        pClient->pAccessToken,
        resourceUri,
        REST_HTTP_METHOD_TYPE_POST,
        (JsonToDataObjectFunc) AfdJsonToActiveDirectoryJoinInfoData,
        (void**) &pActiveDirectoryJoinInfoReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppActiveDirectoryJoinInfoReturn = pActiveDirectoryJoinInfoReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(
            *ppActiveDirectoryJoinInfoReturn,
            (DataObjectToJsonFunc) AfdActiveDirectoryJoinInfoDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        AfdActiveDirectoryJoinInfoDataDelete(pActiveDirectoryJoinInfoReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}

SSOERROR
AfdAdProviderLeave(
    PCREST_CLIENT pClient,
    const REST_CREDENTIALS_DATA* pCredentials,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || pCredentials == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        AD_PROVIDER_URI,
        NULL,
        NULL,
        NULL,
        NULL,
        &resourceUri);
    BAIL_ON_ERROR(e);

    e = RestBuildAndExecuteHttp(
        pCredentials,
        (DataObjectToJsonFunc) RestCredentialsDataToJson,
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
AfdAdProviderGetStatus(
    PCREST_CLIENT pClient,
    AFD_ACTIVE_DIRECTORY_JOIN_INFO_DATA** ppActiveDirectoryJoinInfoReturn,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING resourceUri = NULL;
    AFD_ACTIVE_DIRECTORY_JOIN_INFO_DATA* pActiveDirectoryJoinInfoReturn = NULL;
    REST_SERVER_ERROR* pError = NULL;

    if (pClient == NULL || ppActiveDirectoryJoinInfoReturn == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestBuildResourceUri(
        pClient,
        AD_PROVIDER_POST_URI,
        NULL,
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
        (JsonToDataObjectFunc) AfdJsonToActiveDirectoryJoinInfoData,
        (void**) &pActiveDirectoryJoinInfoReturn,
        &pError);
    BAIL_ON_ERROR(e);

    *ppActiveDirectoryJoinInfoReturn = pActiveDirectoryJoinInfoReturn;

    // debug
    if (DEBUG)
    {
        RestDebugJsonObject(
            *ppActiveDirectoryJoinInfoReturn,
            (DataObjectToJsonFunc) AfdActiveDirectoryJoinInfoDataToJson);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        AfdActiveDirectoryJoinInfoDataDelete(pActiveDirectoryJoinInfoReturn);
        *ppError = pError;
    }

    // cleanup
    SSOStringFree(resourceUri);

    return e;
}
