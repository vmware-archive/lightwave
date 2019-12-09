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
IdmServerDetailsArrayDataNew(
    IDM_SERVER_DETAILS_ARRAY_DATA** ppServerDetailsArray,
    IDM_SERVER_DETAILS_DATA** ppEntry,
    size_t length)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_SERVER_DETAILS_ARRAY_DATA* pServerDetailsArray = NULL;
    size_t i = 0;

    if (ppServerDetailsArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_SERVER_DETAILS_ARRAY_DATA), (void**) &pServerDetailsArray);
    BAIL_ON_ERROR(e);

    if (ppEntry != NULL)
    {
        pServerDetailsArray->length = length;

        e = SSOMemoryAllocateArray(length, sizeof(IDM_SERVER_DETAILS_DATA*), (void**) &(pServerDetailsArray->ppEntry));
        BAIL_ON_ERROR(e);

        for (i = 0; i < length; i++)
        {
            e = IdmServerDetailsDataNew(
                &(pServerDetailsArray->ppEntry[i]),
                ppEntry[i]->hostname,
                ppEntry[i]->domainController);
            BAIL_ON_ERROR(e);
        }
    }

    *ppServerDetailsArray = pServerDetailsArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmServerDetailsArrayDataDelete(pServerDetailsArray);
    }

    return e;
}

void
IdmServerDetailsArrayDataDelete(
    IDM_SERVER_DETAILS_ARRAY_DATA* pServerDetailsArray)
{
    if (pServerDetailsArray != NULL)
    {
        SSOMemoryFreeArrayOfObjects(
            (void**) pServerDetailsArray->ppEntry,
            pServerDetailsArray->length,
            (GenericDestructorFunction) IdmServerDetailsDataDelete);
        SSOMemoryFree(pServerDetailsArray, sizeof(IDM_SERVER_DETAILS_ARRAY_DATA));
    }
}

SSOERROR
IdmServerDetailsArrayDataToJson(
    const IDM_SERVER_DETAILS_ARRAY_DATA* pServerDetailsArray,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pServerDetailsArray != NULL)
    {
        e = RestArrayDataToJson(
            (const REST_GENERIC_ARRAY_DATA*) pServerDetailsArray,
            (DataObjectToJsonFunc) IdmServerDetailsDataToJson,
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToServerDetailsArrayData(
    PCSSO_JSON pJson,
    IDM_SERVER_DETAILS_ARRAY_DATA** ppServerDetailsArray)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_SERVER_DETAILS_ARRAY_DATA* pServerDetailsArray = NULL;

    if (pJson == NULL || ppServerDetailsArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestJsonToArrayData(
        pJson,
        (JsonToDataObjectFunc) IdmJsonToServerDetailsData,
        (REST_GENERIC_ARRAY_DATA**) &pServerDetailsArray);
    BAIL_ON_ERROR(e);

    *ppServerDetailsArray = pServerDetailsArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmServerDetailsArrayDataDelete(pServerDetailsArray);
    }

    return e;
}
