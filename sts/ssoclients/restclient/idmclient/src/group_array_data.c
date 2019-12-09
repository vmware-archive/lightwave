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
IdmGroupArrayDataNew(
    IDM_GROUP_ARRAY_DATA** ppGroupArray,
    IDM_GROUP_DATA** ppEntry,
    size_t length)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_GROUP_ARRAY_DATA* pGroupArray = NULL;
    size_t i = 0;

    if (ppGroupArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_GROUP_ARRAY_DATA), (void**) &pGroupArray);
    BAIL_ON_ERROR(e);

    if (ppEntry != NULL)
    {
        pGroupArray->length = length;

        e = SSOMemoryAllocateArray(length, sizeof(IDM_GROUP_DATA*), (void**) &(pGroupArray->ppEntry));
        BAIL_ON_ERROR(e);

        for (i = 0; i < length; i++)
        {
            e = IdmGroupDataNew(
                &(pGroupArray->ppEntry[i]),
                ppEntry[i]->name,
                ppEntry[i]->domain,
                ppEntry[i]->details,
                ppEntry[i]->alias,
                ppEntry[i]->objectId);
            BAIL_ON_ERROR(e);
        }
    }

    *ppGroupArray = pGroupArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmGroupArrayDataDelete(pGroupArray);
    }

    return e;
}

void
IdmGroupArrayDataDelete(
    IDM_GROUP_ARRAY_DATA* pGroupArray)
{
    if (pGroupArray != NULL)
    {
        SSOMemoryFreeArrayOfObjects(
            (void**) pGroupArray->ppEntry,
            pGroupArray->length,
            (GenericDestructorFunction) IdmGroupDataDelete);
        SSOMemoryFree(pGroupArray, sizeof(IDM_GROUP_ARRAY_DATA));
    }
}

SSOERROR
IdmGroupArrayDataToJson(
    const IDM_GROUP_ARRAY_DATA* pGroupArray,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pGroupArray != NULL)
    {
        e = RestArrayDataToJson(
            (const REST_GENERIC_ARRAY_DATA*) pGroupArray,
            (DataObjectToJsonFunc) IdmGroupDataToJson,
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToGroupArrayData(
    PCSSO_JSON pJson,
    IDM_GROUP_ARRAY_DATA** ppGroupArray)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_GROUP_ARRAY_DATA* pGroupArray = NULL;

    if (pJson == NULL || ppGroupArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestJsonToArrayData(pJson, (JsonToDataObjectFunc) IdmJsonToGroupData, (REST_GENERIC_ARRAY_DATA**) &pGroupArray);
    BAIL_ON_ERROR(e);

    *ppGroupArray = pGroupArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmGroupArrayDataDelete(pGroupArray);
    }

    return e;
}
