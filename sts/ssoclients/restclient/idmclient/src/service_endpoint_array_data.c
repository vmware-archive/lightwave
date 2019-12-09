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
IdmServiceEndpointArrayDataNew(
    IDM_SERVICE_ENDPOINT_ARRAY_DATA** ppServiceEndpointArray,
    IDM_SERVICE_ENDPOINT_DATA** ppEntry,
    size_t length)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_SERVICE_ENDPOINT_ARRAY_DATA* pServiceEndpointArray = NULL;
    size_t i = 0;

    if (ppServiceEndpointArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_SERVICE_ENDPOINT_ARRAY_DATA), (void**) &pServiceEndpointArray);
    BAIL_ON_ERROR(e);

    if (ppEntry != NULL)
    {
        pServiceEndpointArray->length = length;

        e = SSOMemoryAllocateArray(length, sizeof(IDM_SERVICE_ENDPOINT_DATA*), (void**) &(pServiceEndpointArray->ppEntry));
        BAIL_ON_ERROR(e);

        for (i = 0; i < length; i++)
        {
            e = IdmServiceEndpointDataNew(
                &(pServiceEndpointArray->ppEntry[i]),
                ppEntry[i]->name,
                ppEntry[i]->endpoint,
                ppEntry[i]->binding);
            BAIL_ON_ERROR(e);
        }
    }

    *ppServiceEndpointArray = pServiceEndpointArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmServiceEndpointArrayDataDelete(pServiceEndpointArray);
    }

    return e;
}

void
IdmServiceEndpointArrayDataDelete(
    IDM_SERVICE_ENDPOINT_ARRAY_DATA* pServiceEndpointArray)
{
    if (pServiceEndpointArray != NULL)
    {
        SSOMemoryFreeArrayOfObjects(
            (void**) pServiceEndpointArray->ppEntry,
            pServiceEndpointArray->length,
            (GenericDestructorFunction) IdmServiceEndpointDataDelete);
        SSOMemoryFree(pServiceEndpointArray, sizeof(IDM_SERVICE_ENDPOINT_ARRAY_DATA));
    }
}

SSOERROR
IdmServiceEndpointArrayDataToJson(
    const IDM_SERVICE_ENDPOINT_ARRAY_DATA* pServiceEndpointArray,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pServiceEndpointArray != NULL)
    {
        e = RestArrayDataToJson(
            (const REST_GENERIC_ARRAY_DATA*) pServiceEndpointArray,
            (DataObjectToJsonFunc) IdmServiceEndpointDataToJson,
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToServiceEndpointArrayData(
    PCSSO_JSON pJson,
    IDM_SERVICE_ENDPOINT_ARRAY_DATA** ppServiceEndpointArray)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_SERVICE_ENDPOINT_ARRAY_DATA* pServiceEndpointArray = NULL;

    if (pJson == NULL || ppServiceEndpointArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestJsonToArrayData(
        pJson,
        (JsonToDataObjectFunc) IdmJsonToServiceEndpointData,
        (REST_GENERIC_ARRAY_DATA**) &pServiceEndpointArray);
    BAIL_ON_ERROR(e);

    *ppServiceEndpointArray = pServiceEndpointArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmServiceEndpointArrayDataDelete(pServiceEndpointArray);
    }

    return e;
}
