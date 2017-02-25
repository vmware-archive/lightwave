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
IdmResourceServerArrayDataNew(
    IDM_RESOURCE_SERVER_ARRAY_DATA** ppResourceServerArray,
    IDM_RESOURCE_SERVER_DATA** ppEntry,
    size_t length)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_RESOURCE_SERVER_ARRAY_DATA* pResourceServerArray = NULL;
    size_t i = 0;

    if (ppResourceServerArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_RESOURCE_SERVER_ARRAY_DATA), (void**) &pResourceServerArray);
    BAIL_ON_ERROR(e);

    if (ppEntry != NULL)
    {
        pResourceServerArray->length = length;

        e = SSOMemoryAllocateArray(length, sizeof(IDM_RESOURCE_SERVER_DATA*), (void**) &(pResourceServerArray->ppEntry));
        BAIL_ON_ERROR(e);

        for (i = 0; i < length; i++)
        {
            e = IdmResourceServerDataNew(
                &(pResourceServerArray->ppEntry[i]),
                ppEntry[i]->name,
                ppEntry[i]->groupFilter);
            BAIL_ON_ERROR(e);
        }
    }

    *ppResourceServerArray = pResourceServerArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmResourceServerArrayDataDelete(pResourceServerArray);
    }

    return e;
}

void
IdmResourceServerArrayDataDelete(
    IDM_RESOURCE_SERVER_ARRAY_DATA* pResourceServerArray)
{
    if (pResourceServerArray != NULL)
    {
        SSOMemoryFreeArrayOfObjects(
            (void**) pResourceServerArray->ppEntry,
            pResourceServerArray->length,
            (GenericDestructorFunction) IdmResourceServerDataDelete);
        SSOMemoryFree(pResourceServerArray, sizeof(IDM_RESOURCE_SERVER_ARRAY_DATA));
    }
}

SSOERROR
IdmResourceServerArrayDataToJson(
    const IDM_RESOURCE_SERVER_ARRAY_DATA* pResourceServerArray,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pResourceServerArray != NULL)
    {
        e = RestArrayDataToJson(
            (const REST_GENERIC_ARRAY_DATA*) pResourceServerArray,
            (DataObjectToJsonFunc) IdmResourceServerDataToJson,
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToResourceServerArrayData(
    PCSSO_JSON pJson,
    IDM_RESOURCE_SERVER_ARRAY_DATA** ppResourceServerArray)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_RESOURCE_SERVER_ARRAY_DATA* pResourceServerArray = NULL;

    if (pJson == NULL || ppResourceServerArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestJsonToArrayData(
        pJson,
        (JsonToDataObjectFunc) IdmJsonToResourceServerData,
        (REST_GENERIC_ARRAY_DATA**) &pResourceServerArray);
    BAIL_ON_ERROR(e);

    *ppResourceServerArray = pResourceServerArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmResourceServerArrayDataDelete(pResourceServerArray);
    }

    return e;
}
