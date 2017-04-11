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
IdmGroupDataNew(
    IDM_GROUP_DATA** ppGroup,
    PCSTRING name,
    PCSTRING domain,
    const IDM_GROUP_DETAILS_DATA* details,
    const IDM_PRINCIPAL_DATA* alias,
    PCSTRING objectId)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_GROUP_DATA* pGroup = NULL;

    if (ppGroup == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_GROUP_DATA), (void**) &pGroup);
    BAIL_ON_ERROR(e);

    if (name != NULL)
    {
        e = RestStringDataNew(&(pGroup->name), name);
        BAIL_ON_ERROR(e);
    }

    if (domain != NULL)
    {
        e = RestStringDataNew(&(pGroup->domain), domain);
        BAIL_ON_ERROR(e);
    }

    if (details != NULL)
    {
        e = IdmGroupDetailsDataNew(&(pGroup->details), details->description);
        BAIL_ON_ERROR(e);
    }

    if (alias != NULL)
    {
        e = IdmPrincipalDataNew(&(pGroup->alias), alias->name, alias->domain);
        BAIL_ON_ERROR(e);
    }

    if (objectId != NULL)
    {
        e = RestStringDataNew(&(pGroup->objectId), objectId);
        BAIL_ON_ERROR(e);
    }

    *ppGroup = pGroup;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmGroupDataDelete(pGroup);
    }

    return e;
}

void
IdmGroupDataDelete(
    IDM_GROUP_DATA* pGroup)
{
    if (pGroup != NULL)
    {
        RestStringDataDelete(pGroup->name);
        RestStringDataDelete(pGroup->domain);
        IdmGroupDetailsDataDelete(pGroup->details);
        IdmPrincipalDataDelete(pGroup->alias);
        RestStringDataDelete(pGroup->objectId);
        SSOMemoryFree(pGroup, sizeof(IDM_GROUP_DATA));
    }
}

SSOERROR
IdmGroupDataToJson(
    const IDM_GROUP_DATA* pGroup,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pGroup != NULL)
    {
        e = RestDataToJson(pGroup->name, REST_JSON_OBJECT_TYPE_STRING, NULL, "name", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pGroup->domain, REST_JSON_OBJECT_TYPE_STRING, NULL, "domain", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pGroup->details,
            REST_JSON_OBJECT_TYPE_OBJECT,
            (DataObjectToJsonFunc) IdmGroupDetailsDataToJson,
            "details",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pGroup->alias,
            REST_JSON_OBJECT_TYPE_OBJECT,
            (DataObjectToJsonFunc) IdmPrincipalDataToJson,
            "alias",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pGroup->objectId, REST_JSON_OBJECT_TYPE_STRING, NULL, "objectId", pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToGroupData(
    PCSSO_JSON pJson,
    IDM_GROUP_DATA** ppGroup)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_GROUP_DATA* pGroup = NULL;

    if (pJson == NULL || ppGroup == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_GROUP_DATA), (void**) &pGroup);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "name", (void**) &(pGroup->name));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "domain", (void**) &(pGroup->domain));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToGroupDetailsData,
        "details",
        (void**) &(pGroup->details));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToPrincipalData,
        "alias",
        (void**) &(pGroup->alias));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "objectId", (void**) &(pGroup->objectId));
    BAIL_ON_ERROR(e);

    *ppGroup = pGroup;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmGroupDataDelete(pGroup);
    }

    return e;
}
