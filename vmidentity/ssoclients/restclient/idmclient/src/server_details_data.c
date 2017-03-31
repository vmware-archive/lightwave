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
IdmServerDetailsDataNew(
    IDM_SERVER_DETAILS_DATA** ppServerDetails,
    PCSTRING hostname,
    const bool* domainController)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_SERVER_DETAILS_DATA* pServerDetails = NULL;

    if (ppServerDetails == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_SERVER_DETAILS_DATA), (void**) &pServerDetails);
    BAIL_ON_ERROR(e);

    if (hostname != NULL)
    {
        e = RestStringDataNew(&(pServerDetails->hostname), hostname);
        BAIL_ON_ERROR(e);
    }

    if (domainController != NULL)
    {
        e = RestBooleanDataNew(&(pServerDetails->domainController), *domainController);
        BAIL_ON_ERROR(e);
    }

    *ppServerDetails = pServerDetails;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmServerDetailsDataDelete(pServerDetails);
    }

    return e;
}

void
IdmServerDetailsDataDelete(
    IDM_SERVER_DETAILS_DATA* pServerDetails)
{
    if (pServerDetails != NULL)
    {
        RestStringDataDelete(pServerDetails->hostname);
        RestBooleanDataDelete(pServerDetails->domainController);
        SSOMemoryFree(pServerDetails, sizeof(IDM_SERVER_DETAILS_DATA));
    }
}

SSOERROR
IdmServerDetailsDataToJson(
    const IDM_SERVER_DETAILS_DATA* pServerDetails,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pServerDetails != NULL)
    {
        e = RestDataToJson(pServerDetails->hostname, REST_JSON_OBJECT_TYPE_STRING, NULL, "hostname", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pServerDetails->domainController,
            REST_JSON_OBJECT_TYPE_BOOLEAN,
            NULL,
            "domainController",
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToServerDetailsData(
    PCSSO_JSON pJson,
    IDM_SERVER_DETAILS_DATA** ppServerDetails)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_SERVER_DETAILS_DATA* pServerDetails = NULL;

    if (pJson == NULL || ppServerDetails == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_SERVER_DETAILS_DATA), (void**) &pServerDetails);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "hostname",
        (void**) &(pServerDetails->hostname));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_BOOLEAN,
        NULL,
        "domainController",
        (void**) &(pServerDetails->domainController));
    BAIL_ON_ERROR(e);

    *ppServerDetails = pServerDetails;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmServerDetailsDataDelete(pServerDetails);
    }

    return e;
}
