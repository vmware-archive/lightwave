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
IdmUserDataNew(
    IDM_USER_DATA** ppUser,
    PCSTRING name,
    PCSTRING domain,
    const IDM_PRINCIPAL_DATA* alias,
    const IDM_USER_DETAILS_DATA* details,
    const bool* disabled,
    const bool* locked,
    PCSTRING objectId,
    const IDM_PASSWORD_DETAILS_DATA* passwordDetails)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_USER_DATA* pUser = NULL;

    if (ppUser == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_USER_DATA), (void**) &pUser);
    BAIL_ON_ERROR(e);

    if (name != NULL)
    {
        e = RestStringDataNew(&(pUser->name), name);
        BAIL_ON_ERROR(e);
    }

    if (domain != NULL)
    {
        e = RestStringDataNew(&(pUser->domain), domain);
        BAIL_ON_ERROR(e);
    }

    if (alias != NULL)
    {
        e = IdmPrincipalDataNew(&(pUser->alias), alias->name, alias->domain);
        BAIL_ON_ERROR(e);
    }

    if (details != NULL)
    {
        e = IdmUserDetailsDataNew(
            &(pUser->details),
            details->email,
            details->upn,
            details->firstName,
            details->lastName,
            details->description);
        BAIL_ON_ERROR(e);
    }

    if (disabled != NULL)
    {
        e = RestBooleanDataNew(&(pUser->disabled), *disabled);
        BAIL_ON_ERROR(e);
    }

    if (locked != NULL)
    {
        e = RestBooleanDataNew(&(pUser->locked), *locked);
        BAIL_ON_ERROR(e);
    }

    if (objectId != NULL)
    {
        e = RestStringDataNew(&(pUser->objectId), objectId);
        BAIL_ON_ERROR(e);
    }

    if (passwordDetails != NULL)
    {
        e = IdmPasswordDetailsDataNew(
            &(pUser->passwordDetails),
            passwordDetails->password,
            passwordDetails->lastSet,
            passwordDetails->lifetime);
        BAIL_ON_ERROR(e);
    }

    *ppUser = pUser;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmUserDataDelete(pUser);
    }

    return e;
}

void
IdmUserDataDelete(
    IDM_USER_DATA* pUser)
{
    if (pUser != NULL)
    {
        RestStringDataDelete(pUser->name);
        RestStringDataDelete(pUser->domain);
        IdmPrincipalDataDelete(pUser->alias);
        IdmUserDetailsDataDelete(pUser->details);
        RestBooleanDataDelete(pUser->disabled);
        RestBooleanDataDelete(pUser->locked);
        RestStringDataDelete(pUser->objectId);
        IdmPasswordDetailsDataDelete(pUser->passwordDetails);
        SSOMemoryFree(pUser, sizeof(IDM_USER_DATA));
    }
}

SSOERROR
IdmUserDataToJson(
    const IDM_USER_DATA* pUser,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pUser != NULL)
    {
        e = RestDataToJson(pUser->name, REST_JSON_OBJECT_TYPE_STRING, NULL, "name", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pUser->domain, REST_JSON_OBJECT_TYPE_STRING, NULL, "domain", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pUser->alias,
            REST_JSON_OBJECT_TYPE_OBJECT,
            (DataObjectToJsonFunc) IdmPrincipalDataToJson,
            "alias",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pUser->details,
            REST_JSON_OBJECT_TYPE_OBJECT,
            (DataObjectToJsonFunc) IdmUserDetailsDataToJson,
            "details",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pUser->disabled, REST_JSON_OBJECT_TYPE_BOOLEAN, NULL, "disabled", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pUser->locked, REST_JSON_OBJECT_TYPE_BOOLEAN, NULL, "locked", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pUser->objectId, REST_JSON_OBJECT_TYPE_STRING, NULL, "objectId", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pUser->passwordDetails,
            REST_JSON_OBJECT_TYPE_OBJECT,
            (DataObjectToJsonFunc) IdmPasswordDetailsDataToJson,
            "passwordDetails",
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToUserData(
    PCSSO_JSON pJson,
    IDM_USER_DATA** ppUser)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_USER_DATA* pUser = NULL;

    if (pJson == NULL || ppUser == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_USER_DATA), (void**) &pUser);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "name", (void**) &(pUser->name));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "domain", (void**) &(pUser->domain));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToPrincipalData,
        "alias",
        (void**) &(pUser->alias));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToUserDetailsData,
        "details",
        (void**) &(pUser->details));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_BOOLEAN, NULL, "disabled", (void**) &(pUser->disabled));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_BOOLEAN, NULL, "locked", (void**) &(pUser->locked));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "objectId", (void**) &(pUser->objectId));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToPasswordDetailsData,
        "passwordDetails",
        (void**) &(pUser->passwordDetails));
    BAIL_ON_ERROR(e);

    *ppUser = pUser;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmUserDataDelete(pUser);
    }

    return e;
}
