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
AfdActiveDirectoryJoinInfoDataNew(
    AFD_ACTIVE_DIRECTORY_JOIN_INFO_DATA** ppActiveDirectoryJoinInfo,
    PCSTRING alias,
    PCSTRING dn,
    PCSTRING name,
    PCSTRING status)
{
    SSOERROR e = SSOERROR_NONE;
    AFD_ACTIVE_DIRECTORY_JOIN_INFO_DATA* pActiveDirectoryJoinInfo = NULL;

    if (ppActiveDirectoryJoinInfo == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(AFD_ACTIVE_DIRECTORY_JOIN_INFO_DATA), (void**) &pActiveDirectoryJoinInfo);
    BAIL_ON_ERROR(e);

    if (alias != NULL)
    {
        e = RestStringDataNew(&(pActiveDirectoryJoinInfo->alias), alias);
        BAIL_ON_ERROR(e);
    }

    if (dn != NULL)
    {
        e = RestStringDataNew(&(pActiveDirectoryJoinInfo->dn), dn);
        BAIL_ON_ERROR(e);
    }

    if (name != NULL)
    {
        e = RestStringDataNew(&(pActiveDirectoryJoinInfo->name), name);
        BAIL_ON_ERROR(e);
    }

    if (status != NULL)
    {
        e = RestStringDataNew(&(pActiveDirectoryJoinInfo->status), status);
        BAIL_ON_ERROR(e);
    }

    *ppActiveDirectoryJoinInfo = pActiveDirectoryJoinInfo;

    error:

    if (e != SSOERROR_NONE)
    {
        AfdActiveDirectoryJoinInfoDataDelete(pActiveDirectoryJoinInfo);
    }

    return e;
}

void
AfdActiveDirectoryJoinInfoDataDelete(
    AFD_ACTIVE_DIRECTORY_JOIN_INFO_DATA* pActiveDirectoryJoinInfo)
{
    if (pActiveDirectoryJoinInfo != NULL)
    {
        RestStringDataDelete(pActiveDirectoryJoinInfo->alias);
        RestStringDataDelete(pActiveDirectoryJoinInfo->dn);
        RestStringDataDelete(pActiveDirectoryJoinInfo->name);
        RestStringDataDelete(pActiveDirectoryJoinInfo->status);
        SSOMemoryFree(pActiveDirectoryJoinInfo, sizeof(AFD_ACTIVE_DIRECTORY_JOIN_INFO_DATA));
    }
}

SSOERROR
AfdActiveDirectoryJoinInfoDataToJson(
    const AFD_ACTIVE_DIRECTORY_JOIN_INFO_DATA* pActiveDirectoryJoinInfo,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pActiveDirectoryJoinInfo != NULL)
    {
        e = RestDataToJson(pActiveDirectoryJoinInfo->alias, REST_JSON_OBJECT_TYPE_STRING, NULL, "alias", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pActiveDirectoryJoinInfo->dn, REST_JSON_OBJECT_TYPE_STRING, NULL, "dn", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pActiveDirectoryJoinInfo->name, REST_JSON_OBJECT_TYPE_STRING, NULL, "name", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pActiveDirectoryJoinInfo->status, REST_JSON_OBJECT_TYPE_STRING, NULL, "status", pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
AfdJsonToActiveDirectoryJoinInfoData(
    PCSSO_JSON pJson,
    AFD_ACTIVE_DIRECTORY_JOIN_INFO_DATA** ppActiveDirectoryJoinInfo)
{
    SSOERROR e = SSOERROR_NONE;
    AFD_ACTIVE_DIRECTORY_JOIN_INFO_DATA* pActiveDirectoryJoinInfo = NULL;

    if (pJson == NULL || ppActiveDirectoryJoinInfo == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(AFD_ACTIVE_DIRECTORY_JOIN_INFO_DATA), (void**) &pActiveDirectoryJoinInfo);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "alias",
        (void**) &(pActiveDirectoryJoinInfo->alias));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "dn", (void**) &(pActiveDirectoryJoinInfo->dn));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "name",
        (void**) &(pActiveDirectoryJoinInfo->name));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "status",
        (void**) &(pActiveDirectoryJoinInfo->status));
    BAIL_ON_ERROR(e);

    *ppActiveDirectoryJoinInfo = pActiveDirectoryJoinInfo;

    error:

    if (e != SSOERROR_NONE)
    {
        AfdActiveDirectoryJoinInfoDataDelete(pActiveDirectoryJoinInfo);
    }

    return e;
}
