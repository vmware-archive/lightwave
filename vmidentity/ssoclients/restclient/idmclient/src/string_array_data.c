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
IdmStringArrayDataNew(
    IDM_STRING_ARRAY_DATA** ppStringArray,
    PSTRING* ppEntry,
    size_t length)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_STRING_ARRAY_DATA* pStringArray = NULL;
    size_t i = 0;

    if (ppStringArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_STRING_ARRAY_DATA), (void**) &pStringArray);
    BAIL_ON_ERROR(e);

    if (ppEntry != NULL)
    {
        pStringArray->length = length;

        e = SSOMemoryAllocateArray(length, sizeof(PSTRING), (void**) &(pStringArray->ppEntry));
        BAIL_ON_ERROR(e);

        for (i = 0; i < length; i++)
        {
            e = RestStringDataNew(&(pStringArray->ppEntry[i]), ppEntry[i]);
            BAIL_ON_ERROR(e);
        }
    }

    *ppStringArray = pStringArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmStringArrayDataDelete(pStringArray);
    }

    return e;
}

void
IdmStringArrayDataDelete(
    IDM_STRING_ARRAY_DATA* pStringArray)
{
    if (pStringArray != NULL)
    {
        SSOMemoryFreeArrayOfObjects(
            (void**) pStringArray->ppEntry,
            pStringArray->length,
            (GenericDestructorFunction) RestStringDataDelete);
        SSOMemoryFree(pStringArray, sizeof(IDM_STRING_ARRAY_DATA));
    }
}

SSOERROR
IdmStringArrayDataToJson(
    const IDM_STRING_ARRAY_DATA* pStringArray,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;
    size_t i = 0;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pStringArray != NULL)
    {
        for (i = 0; i < pStringArray->length; i++)
        {
            PSSO_JSON pJsonValue = NULL;

            e = SSOJsonStringNew(&pJsonValue, pStringArray->ppEntry[i]);
            BAIL_ON_ERROR(e);

            e = SSOJsonArrayAppend(pJson, pJsonValue);
            BAIL_ON_ERROR(e);

            SSOJsonDelete(pJsonValue);
        }
    }

    error:

    return e;
}

SSOERROR
IdmJsonToStringArrayData(
    PCSSO_JSON pJson,
    IDM_STRING_ARRAY_DATA** ppStringArray)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_STRING_ARRAY_DATA* pStringArray = NULL;

    if (pJson == NULL || ppStringArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestJsonToArrayData(pJson, (JsonToDataObjectFunc) RestJsonToStringData, (REST_GENERIC_ARRAY_DATA**) &pStringArray);
    BAIL_ON_ERROR(e);

    *ppStringArray = pStringArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmStringArrayDataDelete(pStringArray);
    }

    return e;
}
