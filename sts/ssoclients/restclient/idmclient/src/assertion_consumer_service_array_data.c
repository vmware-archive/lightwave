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
IdmAssertionConsumerServiceArrayDataNew(
    IDM_ASSERTION_CONSUMER_SERVICE_ARRAY_DATA** ppAssertionConsumerServiceArray,
    IDM_ASSERTION_CONSUMER_SERVICE_DATA** ppEntry,
    size_t length)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_ASSERTION_CONSUMER_SERVICE_ARRAY_DATA* pAssertionConsumerServiceArray = NULL;
    size_t i = 0;

    if (ppAssertionConsumerServiceArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_ASSERTION_CONSUMER_SERVICE_ARRAY_DATA), (void**) &pAssertionConsumerServiceArray);
    BAIL_ON_ERROR(e);

    if (ppEntry != NULL)
    {
        pAssertionConsumerServiceArray->length = length;

        e = SSOMemoryAllocateArray(
            length,
            sizeof(IDM_ASSERTION_CONSUMER_SERVICE_DATA*),
            (void**) &(pAssertionConsumerServiceArray->ppEntry));
        BAIL_ON_ERROR(e);

        for (i = 0; i < length; i++)
        {
            e = IdmAssertionConsumerServiceDataNew(
                &(pAssertionConsumerServiceArray->ppEntry[i]),
                ppEntry[i]->name,
                ppEntry[i]->endpoint,
                ppEntry[i]->binding,
                ppEntry[i]->index);
            BAIL_ON_ERROR(e);
        }
    }

    *ppAssertionConsumerServiceArray = pAssertionConsumerServiceArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmAssertionConsumerServiceArrayDataDelete(pAssertionConsumerServiceArray);
    }

    return e;
}

void
IdmAssertionConsumerServiceArrayDataDelete(
    IDM_ASSERTION_CONSUMER_SERVICE_ARRAY_DATA* pAssertionConsumerServiceArray)
{
    if (pAssertionConsumerServiceArray != NULL)
    {
        SSOMemoryFreeArrayOfObjects(
            (void**) pAssertionConsumerServiceArray->ppEntry,
            pAssertionConsumerServiceArray->length,
            (GenericDestructorFunction) IdmAssertionConsumerServiceDataDelete);
        SSOMemoryFree(pAssertionConsumerServiceArray, sizeof(IDM_ASSERTION_CONSUMER_SERVICE_ARRAY_DATA));
    }
}

SSOERROR
IdmAssertionConsumerServiceArrayDataToJson(
    const IDM_ASSERTION_CONSUMER_SERVICE_ARRAY_DATA* pAssertionConsumerServiceArray,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pAssertionConsumerServiceArray != NULL)
    {
        e = RestArrayDataToJson(
            (const REST_GENERIC_ARRAY_DATA*) pAssertionConsumerServiceArray,
            (DataObjectToJsonFunc) IdmAssertionConsumerServiceDataToJson,
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToAssertionConsumerServiceArrayData(
    PCSSO_JSON pJson,
    IDM_ASSERTION_CONSUMER_SERVICE_ARRAY_DATA** ppAssertionConsumerServiceArray)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_ASSERTION_CONSUMER_SERVICE_ARRAY_DATA* pAssertionConsumerServiceArray = NULL;

    if (pJson == NULL || ppAssertionConsumerServiceArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestJsonToArrayData(
        pJson,
        (JsonToDataObjectFunc) IdmJsonToAssertionConsumerServiceData,
        (REST_GENERIC_ARRAY_DATA**) &pAssertionConsumerServiceArray);
    BAIL_ON_ERROR(e);

    *ppAssertionConsumerServiceArray = pAssertionConsumerServiceArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmAssertionConsumerServiceArrayDataDelete(pAssertionConsumerServiceArray);
    }

    return e;
}
