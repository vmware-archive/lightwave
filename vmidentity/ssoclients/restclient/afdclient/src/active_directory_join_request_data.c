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

SSOERROR
AfdActiveDirectoryJoinRequestDataNew(
    AFD_ACTIVE_DIRECTORY_JOIN_REQUEST_DATA** ppActiveDirectoryJoinRequest,
    PCSTRING username,
    PCSTRING password,
    PCSTRING domain,
    PCSTRING ou)
{
    SSOERROR e = SSOERROR_NONE;
    AFD_ACTIVE_DIRECTORY_JOIN_REQUEST_DATA* pActiveDirectoryJoinRequest = NULL;

    if (ppActiveDirectoryJoinRequest == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(AFD_ACTIVE_DIRECTORY_JOIN_REQUEST_DATA), (void**) &pActiveDirectoryJoinRequest);
    BAIL_ON_ERROR(e);

    if (username != NULL)
    {
        e = RestStringDataNew(&(pActiveDirectoryJoinRequest->username), username);
        BAIL_ON_ERROR(e);
    }

    if (password != NULL)
    {
        e = RestStringDataNew(&(pActiveDirectoryJoinRequest->password), password);
        BAIL_ON_ERROR(e);
    }

    if (domain != NULL)
    {
        e = RestStringDataNew(&(pActiveDirectoryJoinRequest->domain), domain);
        BAIL_ON_ERROR(e);
    }

    if (ou != NULL)
    {
        e = RestStringDataNew(&(pActiveDirectoryJoinRequest->ou), ou);
        BAIL_ON_ERROR(e);
    }

    *ppActiveDirectoryJoinRequest = pActiveDirectoryJoinRequest;

    error:

    if (e != SSOERROR_NONE)
    {
        AfdActiveDirectoryJoinRequestDataDelete(pActiveDirectoryJoinRequest);
    }

    return e;
}

void
AfdActiveDirectoryJoinRequestDataDelete(
    AFD_ACTIVE_DIRECTORY_JOIN_REQUEST_DATA* pActiveDirectoryJoinRequest)
{
    if (pActiveDirectoryJoinRequest != NULL)
    {
        RestStringDataDelete(pActiveDirectoryJoinRequest->username);
        RestStringDataDelete(pActiveDirectoryJoinRequest->password);
        RestStringDataDelete(pActiveDirectoryJoinRequest->domain);
        RestStringDataDelete(pActiveDirectoryJoinRequest->ou);
        SSOMemoryFree(pActiveDirectoryJoinRequest, sizeof(AFD_ACTIVE_DIRECTORY_JOIN_REQUEST_DATA));
    }
}

SSOERROR
AfdActiveDirectoryJoinRequestDataToJson(
    const AFD_ACTIVE_DIRECTORY_JOIN_REQUEST_DATA* pActiveDirectoryJoinRequest,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pActiveDirectoryJoinRequest != NULL)
    {
        e = RestDataToJson(
            pActiveDirectoryJoinRequest->username,
            REST_JSON_OBJECT_TYPE_STRING,
            NULL,
            "username",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pActiveDirectoryJoinRequest->password,
            REST_JSON_OBJECT_TYPE_STRING,
            NULL,
            "password",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pActiveDirectoryJoinRequest->domain,
            REST_JSON_OBJECT_TYPE_STRING,
            NULL,
            "domain",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pActiveDirectoryJoinRequest->ou, REST_JSON_OBJECT_TYPE_STRING, NULL, "ou", pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
AfdJsonToActiveDirectoryJoinRequestData(
    PCSSO_JSON pJson,
    AFD_ACTIVE_DIRECTORY_JOIN_REQUEST_DATA** ppActiveDirectoryJoinRequest)
{
    SSOERROR e = SSOERROR_NONE;
    AFD_ACTIVE_DIRECTORY_JOIN_REQUEST_DATA* pActiveDirectoryJoinRequest = NULL;

    if (pJson == NULL || ppActiveDirectoryJoinRequest == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(AFD_ACTIVE_DIRECTORY_JOIN_REQUEST_DATA), (void**) &pActiveDirectoryJoinRequest);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "username",
        (void**) &(pActiveDirectoryJoinRequest->username));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "password",
        (void**) &(pActiveDirectoryJoinRequest->password));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "domain",
        (void**) &(pActiveDirectoryJoinRequest->domain));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "ou",
        (void**) &(pActiveDirectoryJoinRequest->ou));
    BAIL_ON_ERROR(e);

    *ppActiveDirectoryJoinRequest = pActiveDirectoryJoinRequest;

    error:

    if (e != SSOERROR_NONE)
    {
        AfdActiveDirectoryJoinRequestDataDelete(pActiveDirectoryJoinRequest);
    }

    return e;
}
