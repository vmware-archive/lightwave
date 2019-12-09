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
IdmTokenClaimGroupDataNew(
    IDM_TOKEN_CLAIM_GROUP_DATA** ppTokenClaimGroup,
    PCSTRING claimName,
    PCSTRING claimValue,
    const IDM_STRING_ARRAY_DATA* groups)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_TOKEN_CLAIM_GROUP_DATA* pTokenClaimGroup = NULL;

    if (ppTokenClaimGroup == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_TOKEN_CLAIM_GROUP_DATA), (void**) &pTokenClaimGroup);
    BAIL_ON_ERROR(e);

    if (claimName != NULL)
    {
        e = RestStringDataNew(&(pTokenClaimGroup->claimName), claimName);
        BAIL_ON_ERROR(e);
    }

    if (claimValue != NULL)
    {
        e = RestStringDataNew(&(pTokenClaimGroup->claimValue), claimValue);
        BAIL_ON_ERROR(e);
    }

    if (groups != NULL)
    {
        e = IdmStringArrayDataNew(&(pTokenClaimGroup->groups), groups->ppEntry, groups->length);
        BAIL_ON_ERROR(e);
    }

    *ppTokenClaimGroup = pTokenClaimGroup;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmTokenClaimGroupDataDelete(pTokenClaimGroup);
    }

    return e;
}

void
IdmTokenClaimGroupDataDelete(
    IDM_TOKEN_CLAIM_GROUP_DATA* pTokenClaimGroup)
{
    if (pTokenClaimGroup != NULL)
    {
        RestStringDataDelete(pTokenClaimGroup->claimName);
        RestStringDataDelete(pTokenClaimGroup->claimValue);
        IdmStringArrayDataDelete(pTokenClaimGroup->groups);
        SSOMemoryFree(pTokenClaimGroup, sizeof(IDM_TOKEN_CLAIM_GROUP_DATA));
    }
}

SSOERROR
IdmTokenClaimGroupDataToJson(
    const IDM_TOKEN_CLAIM_GROUP_DATA* pTokenClaimGroup,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pTokenClaimGroup != NULL)
    {
        e = RestDataToJson(pTokenClaimGroup->claimName, REST_JSON_OBJECT_TYPE_STRING, NULL, "claimName", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pTokenClaimGroup->claimValue, REST_JSON_OBJECT_TYPE_STRING, NULL, "claimValue", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pTokenClaimGroup->groups,
            REST_JSON_OBJECT_TYPE_ARRAY,
            (DataObjectToJsonFunc) IdmStringArrayDataToJson,
            "groups",
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToTokenClaimGroupData(
    PCSSO_JSON pJson,
    IDM_TOKEN_CLAIM_GROUP_DATA** ppTokenClaimGroup)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_TOKEN_CLAIM_GROUP_DATA* pTokenClaimGroup = NULL;

    if (pJson == NULL || ppTokenClaimGroup == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_TOKEN_CLAIM_GROUP_DATA), (void**) &pTokenClaimGroup);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "claimName",
        (void**) &(pTokenClaimGroup->claimName));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "claimValue",
        (void**) &(pTokenClaimGroup->claimValue));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToStringArrayData,
        "groups",
        (void**) &(pTokenClaimGroup->groups));
    BAIL_ON_ERROR(e);

    *ppTokenClaimGroup = pTokenClaimGroup;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmTokenClaimGroupDataDelete(pTokenClaimGroup);
    }

    return e;
}
