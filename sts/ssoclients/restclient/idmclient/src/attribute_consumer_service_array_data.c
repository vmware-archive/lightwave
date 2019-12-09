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
IdmAttributeConsumerServiceArrayDataNew(
    IDM_ATTRIBUTE_CONSUMER_SERVICE_ARRAY_DATA** ppAttributeConsumerServiceArray,
    IDM_ATTRIBUTE_CONSUMER_SERVICE_DATA** ppEntry,
    size_t length)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_ATTRIBUTE_CONSUMER_SERVICE_ARRAY_DATA* pAttributeConsumerServiceArray = NULL;
    size_t i = 0;

    if (ppAttributeConsumerServiceArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_ATTRIBUTE_CONSUMER_SERVICE_ARRAY_DATA), (void**) &pAttributeConsumerServiceArray);
    BAIL_ON_ERROR(e);

    if (ppEntry != NULL)
    {
        pAttributeConsumerServiceArray->length = length;

        e = SSOMemoryAllocateArray(
            length,
            sizeof(IDM_ATTRIBUTE_CONSUMER_SERVICE_DATA*),
            (void**) &(pAttributeConsumerServiceArray->ppEntry));
        BAIL_ON_ERROR(e);

        for (i = 0; i < length; i++)
        {
            e = IdmAttributeConsumerServiceDataNew(
                &(pAttributeConsumerServiceArray->ppEntry[i]),
                ppEntry[i]->name,
                ppEntry[i]->index,
                ppEntry[i]->attributes);
            BAIL_ON_ERROR(e);
        }
    }

    *ppAttributeConsumerServiceArray = pAttributeConsumerServiceArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmAttributeConsumerServiceArrayDataDelete(pAttributeConsumerServiceArray);
    }

    return e;
}

void
IdmAttributeConsumerServiceArrayDataDelete(
    IDM_ATTRIBUTE_CONSUMER_SERVICE_ARRAY_DATA* pAttributeConsumerServiceArray)
{
    if (pAttributeConsumerServiceArray != NULL)
    {
        SSOMemoryFreeArrayOfObjects(
            (void**) pAttributeConsumerServiceArray->ppEntry,
            pAttributeConsumerServiceArray->length,
            (GenericDestructorFunction) IdmAttributeConsumerServiceDataDelete);
        SSOMemoryFree(pAttributeConsumerServiceArray, sizeof(IDM_ATTRIBUTE_CONSUMER_SERVICE_ARRAY_DATA));
    }
}

SSOERROR
IdmAttributeConsumerServiceArrayDataToJson(
    const IDM_ATTRIBUTE_CONSUMER_SERVICE_ARRAY_DATA* pAttributeConsumerServiceArray,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pAttributeConsumerServiceArray != NULL)
    {
        e = RestArrayDataToJson(
            (const REST_GENERIC_ARRAY_DATA*) pAttributeConsumerServiceArray,
            (DataObjectToJsonFunc) IdmAttributeConsumerServiceDataToJson,
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToAttributeConsumerServiceArrayData(
    PCSSO_JSON pJson,
    IDM_ATTRIBUTE_CONSUMER_SERVICE_ARRAY_DATA** ppAttributeConsumerServiceArray)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_ATTRIBUTE_CONSUMER_SERVICE_ARRAY_DATA* pAttributeConsumerServiceArray = NULL;

    if (pJson == NULL || ppAttributeConsumerServiceArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestJsonToArrayData(
        pJson,
        (JsonToDataObjectFunc) IdmJsonToAttributeConsumerServiceData,
        (REST_GENERIC_ARRAY_DATA**) &pAttributeConsumerServiceArray);
    BAIL_ON_ERROR(e);

    *ppAttributeConsumerServiceArray = pAttributeConsumerServiceArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmAttributeConsumerServiceArrayDataDelete(pAttributeConsumerServiceArray);
    }

    return e;
}
