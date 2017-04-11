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
VmdirUserDetailsDataNew(
    VMDIR_USER_DETAILS_DATA** ppUserDetails,
    PCSTRING email,
    PCSTRING upn,
    PCSTRING firstName,
    PCSTRING lastName,
    PCSTRING description)
{
    SSOERROR e = SSOERROR_NONE;
    VMDIR_USER_DETAILS_DATA* pUserDetails = NULL;

    if (ppUserDetails == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(VMDIR_USER_DETAILS_DATA), (void**) &pUserDetails);
    BAIL_ON_ERROR(e);

    if (email != NULL)
    {
        e = RestStringDataNew(&(pUserDetails->email), email);
        BAIL_ON_ERROR(e);
    }

    if (upn != NULL)
    {
        e = RestStringDataNew(&(pUserDetails->upn), upn);
        BAIL_ON_ERROR(e);
    }

    if (firstName != NULL)
    {
        e = RestStringDataNew(&(pUserDetails->firstName), firstName);
        BAIL_ON_ERROR(e);
    }

    if (lastName != NULL)
    {
        e = RestStringDataNew(&(pUserDetails->lastName), lastName);
        BAIL_ON_ERROR(e);
    }

    if (description != NULL)
    {
        e = RestStringDataNew(&(pUserDetails->description), description);
        BAIL_ON_ERROR(e);
    }

    *ppUserDetails = pUserDetails;

    error:

    if (e != SSOERROR_NONE)
    {
        VmdirUserDetailsDataDelete(pUserDetails);
    }

    return e;
}

void
VmdirUserDetailsDataDelete(
    VMDIR_USER_DETAILS_DATA* pUserDetails)
{
    if (pUserDetails != NULL)
    {
        RestStringDataDelete(pUserDetails->email);
        RestStringDataDelete(pUserDetails->upn);
        RestStringDataDelete(pUserDetails->firstName);
        RestStringDataDelete(pUserDetails->lastName);
        RestStringDataDelete(pUserDetails->description);
        SSOMemoryFree(pUserDetails, sizeof(VMDIR_USER_DETAILS_DATA));
    }
}

SSOERROR
VmdirUserDetailsDataToJson(
    const VMDIR_USER_DETAILS_DATA* pUserDetails,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pUserDetails != NULL)
    {
        e = RestDataToJson(pUserDetails->email, REST_JSON_OBJECT_TYPE_STRING, NULL, "email", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pUserDetails->upn, REST_JSON_OBJECT_TYPE_STRING, NULL, "upn", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pUserDetails->firstName, REST_JSON_OBJECT_TYPE_STRING, NULL, "firstName", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pUserDetails->lastName, REST_JSON_OBJECT_TYPE_STRING, NULL, "lastName", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pUserDetails->description, REST_JSON_OBJECT_TYPE_STRING, NULL, "description", pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
VmdirJsonToUserDetailsData(
    PCSSO_JSON pJson,
    VMDIR_USER_DETAILS_DATA** ppUserDetails)
{
    SSOERROR e = SSOERROR_NONE;
    VMDIR_USER_DETAILS_DATA* pUserDetails = NULL;

    if (pJson == NULL || ppUserDetails == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(VMDIR_USER_DETAILS_DATA), (void**) &pUserDetails);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "email", (void**) &(pUserDetails->email));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "upn", (void**) &(pUserDetails->upn));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "firstName",
        (void**) &(pUserDetails->firstName));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "lastName", (void**) &(pUserDetails->lastName));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "description",
        (void**) &(pUserDetails->description));
    BAIL_ON_ERROR(e);

    *ppUserDetails = pUserDetails;

    error:

    if (e != SSOERROR_NONE)
    {
        VmdirUserDetailsDataDelete(pUserDetails);
    }

    return e;
}
