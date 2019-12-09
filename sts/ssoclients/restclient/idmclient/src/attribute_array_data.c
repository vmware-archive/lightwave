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
IdmAttributeArrayDataNew(
    IDM_ATTRIBUTE_ARRAY_DATA** ppAttributeArray,
    IDM_ATTRIBUTE_DATA** ppEntry,
    size_t length)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_ATTRIBUTE_ARRAY_DATA* pAttributeArray = NULL;
    size_t i;

    if (ppAttributeArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_ATTRIBUTE_ARRAY_DATA), (void**) &pAttributeArray);
    BAIL_ON_ERROR(e);

    if (ppEntry != NULL)
    {
        pAttributeArray->length = length;

        e = SSOMemoryAllocateArray(length, sizeof(IDM_ATTRIBUTE_DATA*), (void**) &(pAttributeArray->ppEntry));
        BAIL_ON_ERROR(e);

        for (i = 0; i < length; i++)
        {
            e = IdmAttributeDataNew(
                &(pAttributeArray->ppEntry[i]),
                ppEntry[i]->name,
                ppEntry[i]->friendlyName,
                ppEntry[i]->nameFormat);
            BAIL_ON_ERROR(e);
        }
    }

    *ppAttributeArray = pAttributeArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmAttributeArrayDataDelete(pAttributeArray);
    }

    return e;
}

void
IdmAttributeArrayDataDelete(
    IDM_ATTRIBUTE_ARRAY_DATA* pAttributeArray)
{
    if (pAttributeArray != NULL)
    {
        SSOMemoryFreeArrayOfObjects(
            (void**) pAttributeArray->ppEntry,
            pAttributeArray->length,
            (GenericDestructorFunction) IdmAttributeDataDelete);
        SSOMemoryFree(pAttributeArray, sizeof(IDM_ATTRIBUTE_ARRAY_DATA));
    }
}

SSOERROR
IdmAttributeArrayDataToJson(
    const IDM_ATTRIBUTE_ARRAY_DATA* pAttributeArray,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pAttributeArray != NULL)
    {
        e = RestArrayDataToJson(
            (const REST_GENERIC_ARRAY_DATA*) pAttributeArray,
            (DataObjectToJsonFunc) IdmAttributeDataToJson,
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToAttributeArrayData(
    PCSSO_JSON pJson,
    IDM_ATTRIBUTE_ARRAY_DATA** ppAttributeArray)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_ATTRIBUTE_ARRAY_DATA* pAttributeArray = NULL;

    if (pJson == NULL || ppAttributeArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestJsonToArrayData(
        pJson,
        (JsonToDataObjectFunc) IdmJsonToAttributeData,
        (REST_GENERIC_ARRAY_DATA**) &pAttributeArray);
    BAIL_ON_ERROR(e);

    *ppAttributeArray = pAttributeArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmAttributeArrayDataDelete(pAttributeArray);
    }

    return e;
}
