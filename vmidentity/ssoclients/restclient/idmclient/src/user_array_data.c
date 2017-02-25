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
IdmUserArrayDataNew(
    IDM_USER_ARRAY_DATA** ppUserArray,
    IDM_USER_DATA** ppEntry,
    size_t length)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_USER_ARRAY_DATA* pUserArray = NULL;
    size_t i = 0;

    if (ppUserArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_USER_ARRAY_DATA), (void**) &pUserArray);
    BAIL_ON_ERROR(e);

    if (ppEntry != NULL)
    {
        pUserArray->length = length;

        e = SSOMemoryAllocateArray(length, sizeof(IDM_USER_DATA*), (void**) &(pUserArray->ppEntry));
        BAIL_ON_ERROR(e);

        for (i = 0; i < length; i++)
        {
            e = IdmUserDataNew(
                &(pUserArray->ppEntry[i]),
                ppEntry[i]->name,
                ppEntry[i]->domain,
                ppEntry[i]->alias,
                ppEntry[i]->details,
                ppEntry[i]->disabled,
                ppEntry[i]->locked,
                ppEntry[i]->objectId,
                ppEntry[i]->passwordDetails);
            BAIL_ON_ERROR(e);
        }
    }

    *ppUserArray = pUserArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmUserArrayDataDelete(pUserArray);
    }

    return e;
}

void
IdmUserArrayDataDelete(
    IDM_USER_ARRAY_DATA* pUserArray)
{
    if (pUserArray != NULL)
    {
        SSOMemoryFreeArrayOfObjects(
            (void**) pUserArray->ppEntry,
            pUserArray->length,
            (GenericDestructorFunction) IdmUserDataDelete);
        SSOMemoryFree(pUserArray, sizeof(IDM_USER_ARRAY_DATA));
    }
}

SSOERROR
IdmUserArrayDataToJson(
    const IDM_USER_ARRAY_DATA* pUserArray,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pUserArray != NULL)
    {
        e = RestArrayDataToJson(
            (const REST_GENERIC_ARRAY_DATA*) pUserArray,
            (DataObjectToJsonFunc) IdmUserDataToJson,
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToUserArrayData(
    PCSSO_JSON pJson,
    IDM_USER_ARRAY_DATA** ppUserArray)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_USER_ARRAY_DATA* pUserArray = NULL;

    if (pJson == NULL || ppUserArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestJsonToArrayData(pJson, (JsonToDataObjectFunc) IdmJsonToUserData, (REST_GENERIC_ARRAY_DATA**) &pUserArray);
    BAIL_ON_ERROR(e);

    *ppUserArray = pUserArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmUserArrayDataDelete(pUserArray);
    }

    return e;
}
