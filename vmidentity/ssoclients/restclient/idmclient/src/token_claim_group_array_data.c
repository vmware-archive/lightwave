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
IdmTokenClaimGroupArrayDataNew(
    IDM_TOKEN_CLAIM_GROUP_ARRAY_DATA** ppTokenClaimGroupArray,
    IDM_TOKEN_CLAIM_GROUP_DATA** ppEntry,
    size_t length)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_TOKEN_CLAIM_GROUP_ARRAY_DATA* pTokenClaimGroupArray = NULL;
    size_t i = 0;

    if (ppTokenClaimGroupArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_TOKEN_CLAIM_GROUP_ARRAY_DATA), (void**) &pTokenClaimGroupArray);
    BAIL_ON_ERROR(e);

    if (ppEntry != NULL)
    {
        pTokenClaimGroupArray->length = length;

        e = SSOMemoryAllocateArray(length, sizeof(IDM_TOKEN_CLAIM_GROUP_DATA*), (void**) &(pTokenClaimGroupArray->ppEntry));
        BAIL_ON_ERROR(e);

        for (i = 0; i < length; i++)
        {
            e = IdmTokenClaimGroupDataNew(
                &(pTokenClaimGroupArray->ppEntry[i]),
                ppEntry[i]->claimName,
                ppEntry[i]->claimValue,
                ppEntry[i]->groups);
            BAIL_ON_ERROR(e);
        }
    }

    *ppTokenClaimGroupArray = pTokenClaimGroupArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmTokenClaimGroupArrayDataDelete(pTokenClaimGroupArray);
    }

    return e;
}

void
IdmTokenClaimGroupArrayDataDelete(
    IDM_TOKEN_CLAIM_GROUP_ARRAY_DATA* pTokenClaimGroupArray)
{
    if (pTokenClaimGroupArray != NULL)
    {
        SSOMemoryFreeArrayOfObjects(
            (void**) pTokenClaimGroupArray->ppEntry,
            pTokenClaimGroupArray->length,
            (GenericDestructorFunction) IdmTokenClaimGroupDataDelete);
        SSOMemoryFree(pTokenClaimGroupArray, sizeof(IDM_TOKEN_CLAIM_GROUP_ARRAY_DATA));
    }
}

SSOERROR
IdmTokenClaimGroupArrayDataToJson(
    const IDM_TOKEN_CLAIM_GROUP_ARRAY_DATA* pTokenClaimGroupArray,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pTokenClaimGroupArray != NULL)
    {
        e = RestArrayDataToJson(
            (const REST_GENERIC_ARRAY_DATA*) pTokenClaimGroupArray,
            (DataObjectToJsonFunc) IdmTokenClaimGroupDataToJson,
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToTokenClaimGroupArrayData(
    PCSSO_JSON pJson,
    IDM_TOKEN_CLAIM_GROUP_ARRAY_DATA** ppTokenClaimGroupArray)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_TOKEN_CLAIM_GROUP_ARRAY_DATA* pTokenClaimGroupArray = NULL;

    if (pJson == NULL || ppTokenClaimGroupArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestJsonToArrayData(
        pJson,
        (JsonToDataObjectFunc) IdmJsonToTokenClaimGroupData,
        (REST_GENERIC_ARRAY_DATA**) &pTokenClaimGroupArray);
    BAIL_ON_ERROR(e);

    *ppTokenClaimGroupArray = pTokenClaimGroupArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmTokenClaimGroupArrayDataDelete(pTokenClaimGroupArray);
    }

    return e;
}
