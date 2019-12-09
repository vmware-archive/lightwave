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
VmdirGroupDetailsDataNew(
    VMDIR_GROUP_DETAILS_DATA** ppGroupDetails,
    PCSTRING description)
{
    SSOERROR e = SSOERROR_NONE;
    VMDIR_GROUP_DETAILS_DATA* pGroupDetails = NULL;

    if (ppGroupDetails == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(VMDIR_GROUP_DETAILS_DATA), (void**) &pGroupDetails);
    BAIL_ON_ERROR(e);

    if (description != NULL)
    {
        e = RestStringDataNew(&(pGroupDetails->description), description);
        BAIL_ON_ERROR(e);
    }

    *ppGroupDetails = pGroupDetails;

    error:

    if (e != SSOERROR_NONE)
    {
        VmdirGroupDetailsDataDelete(pGroupDetails);
    }

    return e;
}

void
VmdirGroupDetailsDataDelete(
    VMDIR_GROUP_DETAILS_DATA* pGroupDetails)
{
    if (pGroupDetails != NULL)
    {
        RestStringDataDelete(pGroupDetails->description);
        SSOMemoryFree(pGroupDetails, sizeof(VMDIR_GROUP_DETAILS_DATA));
    }
}

SSOERROR
VmdirGroupDetailsDataToJson(
    const VMDIR_GROUP_DETAILS_DATA* pGroupDetails,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pGroupDetails != NULL)
    {
        e = RestDataToJson(pGroupDetails->description, REST_JSON_OBJECT_TYPE_STRING, NULL, "description", pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
VmdirJsonToGroupDetailsData(
    PCSSO_JSON pJson,
    VMDIR_GROUP_DETAILS_DATA** ppGroupDetails)
{
    SSOERROR e = SSOERROR_NONE;
    VMDIR_GROUP_DETAILS_DATA* pGroupDetails = NULL;

    if (pJson == NULL || ppGroupDetails == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(VMDIR_GROUP_DETAILS_DATA), (void**) &pGroupDetails);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "description",
        (void**) &(pGroupDetails->description));
    BAIL_ON_ERROR(e);

    *ppGroupDetails = pGroupDetails;

    error:

    if (e != SSOERROR_NONE)
    {
        VmdirGroupDetailsDataDelete(pGroupDetails);
    }

    return e;
}
