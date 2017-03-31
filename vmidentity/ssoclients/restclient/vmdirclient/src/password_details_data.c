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
VmdirPasswordDetailsDataNew(
    VMDIR_PASSWORD_DETAILS_DATA** ppPasswordDetails,
    PCSTRING password,
    const SSO_LONG* lastSet,
    const SSO_LONG* lifetime)
{
    SSOERROR e = SSOERROR_NONE;
    VMDIR_PASSWORD_DETAILS_DATA* pPasswordDetails = NULL;

    if (ppPasswordDetails == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(VMDIR_PASSWORD_DETAILS_DATA), (void**) &pPasswordDetails);
    BAIL_ON_ERROR(e);

    if (password != NULL)
    {
        e = RestStringDataNew(&(pPasswordDetails->password), password);
        BAIL_ON_ERROR(e);
    }

    if (lastSet != NULL)
    {
        e = RestLongDataNew(&(pPasswordDetails->lastSet), *lastSet);
        BAIL_ON_ERROR(e);
    }

    if (lifetime != NULL)
    {
        e = RestLongDataNew(&(pPasswordDetails->lifetime), *lifetime);
        BAIL_ON_ERROR(e);
    }

    *ppPasswordDetails = pPasswordDetails;

    error:

    if (e != SSOERROR_NONE)
    {
        VmdirPasswordDetailsDataDelete(pPasswordDetails);
    }

    return e;
}

void
VmdirPasswordDetailsDataDelete(
    VMDIR_PASSWORD_DETAILS_DATA* pPasswordDetails)
{
    if (pPasswordDetails != NULL)
    {
        RestStringDataDelete(pPasswordDetails->password);
        RestLongDataDelete(pPasswordDetails->lastSet);
        RestLongDataDelete(pPasswordDetails->lifetime);
        SSOMemoryFree(pPasswordDetails, sizeof(VMDIR_PASSWORD_DETAILS_DATA));
    }
}

SSOERROR
VmdirPasswordDetailsDataToJson(
    const VMDIR_PASSWORD_DETAILS_DATA* pPasswordDetails,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pPasswordDetails != NULL)
    {
        e = RestDataToJson(pPasswordDetails->password, REST_JSON_OBJECT_TYPE_STRING, NULL, "password", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pPasswordDetails->lastSet, REST_JSON_OBJECT_TYPE_LONG, NULL, "lastSet", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pPasswordDetails->lifetime, REST_JSON_OBJECT_TYPE_LONG, NULL, "lifetime", pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
VmdirJsonToPasswordDetailsData(
    PCSSO_JSON pJson,
    VMDIR_PASSWORD_DETAILS_DATA** ppPasswordDetails)
{
    SSOERROR e = SSOERROR_NONE;
    VMDIR_PASSWORD_DETAILS_DATA* pPasswordDetails = NULL;

    if (pJson == NULL || ppPasswordDetails == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(VMDIR_PASSWORD_DETAILS_DATA), (void**) &pPasswordDetails);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "password",
        (void**) &(pPasswordDetails->password));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_LONG, NULL, "lastSet", (void**) &(pPasswordDetails->lastSet));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_LONG,
        NULL,
        "lifetime",
        (void**) &(pPasswordDetails->lifetime));
    BAIL_ON_ERROR(e);

    *ppPasswordDetails = pPasswordDetails;

    error:

    if (e != SSOERROR_NONE)
    {
        VmdirPasswordDetailsDataDelete(pPasswordDetails);
    }

    return e;
}
