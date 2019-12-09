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
RestDataToJson(
    const void* pData,
    REST_JSON_OBJECT_TYPE type,
    DataObjectToJsonFunc f,
    PCSTRING key,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;
    PSSO_JSON pJsonValue = NULL;

    if (pJson == NULL || IS_NULL_OR_EMPTY_STRING(key))
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pData != NULL)
    {
        if (type == REST_JSON_OBJECT_TYPE_STRING)
        {
            e = SSOJsonStringNew(&pJsonValue, pData);
            BAIL_ON_ERROR(e);
        }
        else if (type == REST_JSON_OBJECT_TYPE_INTEGER)
        {
            e = SSOJsonIntegerNew(&pJsonValue, *(INTEGER*) pData);
            BAIL_ON_ERROR(e);
        }
        else if (type == REST_JSON_OBJECT_TYPE_LONG)
        {
            e = SSOJsonLongNew(&pJsonValue, *(SSO_LONG*) pData);
            BAIL_ON_ERROR(e);
        }
        else if (type == REST_JSON_OBJECT_TYPE_BOOLEAN)
        {
            e = SSOJsonBooleanNew(&pJsonValue, *(bool*) pData);
            BAIL_ON_ERROR(e);
        }
        else
        {
            if (type == REST_JSON_OBJECT_TYPE_OBJECT)
            {
                e = SSOJsonObjectNew(&pJsonValue);
                BAIL_ON_ERROR(e);
            }
            else if (type == REST_JSON_OBJECT_TYPE_ARRAY)
            {
                e = SSOJsonArrayNew(&pJsonValue);
                BAIL_ON_ERROR(e);
            }
            else
            {
                e = SSOERROR_JSON_FAILURE;
                BAIL_ON_ERROR(e);
            }
            e = f(pData, pJsonValue);
            BAIL_ON_ERROR(e);
        }
        e = SSOJsonObjectSet(pJson, key, pJsonValue);
        BAIL_ON_ERROR(e);
    }

    error:

    SSOJsonDelete(pJsonValue);

    return e;
}

SSOERROR
RestJsonToData(
    PCSSO_JSON pJson,
    REST_JSON_OBJECT_TYPE type,
    JsonToDataObjectFunc f,
    PCSTRING key,
    void** ppData)
{
    SSOERROR e = SSOERROR_NONE;
    PSSO_JSON pJsonValue = NULL;
    bool isNull = false;
    void* pData = NULL;

    if (pJson == NULL || IS_NULL_OR_EMPTY_STRING(key) || ppData == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOJsonObjectGet(pJson, key, &pJsonValue);
    BAIL_ON_ERROR(e);

    e = SSOJsonIsNull(pJsonValue, &isNull);
    BAIL_ON_ERROR(e);

    if (!isNull)
    {
        if (type == REST_JSON_OBJECT_TYPE_STRING)
        {
            PSTRING value = NULL;
            e = SSOJsonStringValue(pJsonValue, &value);
            BAIL_ON_ERROR(e);

            e = RestStringDataNew((PSTRING*) &pData, value);
            BAIL_ON_ERROR(e);

            SSOStringFree(value);
        }
        else if (type == REST_JSON_OBJECT_TYPE_INTEGER)
        {
            INTEGER value = 0;
            e = SSOJsonIntegerValue(pJsonValue, &value);
            BAIL_ON_ERROR(e);

            e = RestIntegerDataNew((INTEGER**) &pData, value);
            BAIL_ON_ERROR(e);
        }
        else if (type == REST_JSON_OBJECT_TYPE_LONG)
        {
            SSO_LONG value = 0;
            e = SSOJsonLongValue(pJsonValue, &value);
            BAIL_ON_ERROR(e);

            e = RestLongDataNew((SSO_LONG**) &pData, value);
            BAIL_ON_ERROR(e);
        }
        else if (type == REST_JSON_OBJECT_TYPE_BOOLEAN)
        {
            bool value = false;
            e = SSOJsonBooleanValue(pJsonValue, &value);
            BAIL_ON_ERROR(e);

            e = RestBooleanDataNew((bool**) &pData, value);
            BAIL_ON_ERROR(e);
        }
        else if (type == REST_JSON_OBJECT_TYPE_OBJECT)
        {
            e = f(pJsonValue, &pData);
            BAIL_ON_ERROR(e);
        }
        else
        {
            e = SSOERROR_JSON_FAILURE;
            BAIL_ON_ERROR(e);
        }
    }

    *ppData = pData;

    error:

    if (e != SSOERROR_NONE)
    {
        SSOMemoryFree(pData, 0);
    }

    SSOJsonDelete(pJsonValue);

    return e;
}

SSOERROR
RestArrayDataToJson(
    const REST_GENERIC_ARRAY_DATA* pArray,
    DataObjectToJsonFunc f,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;
    PSSO_JSON pJsonValue = NULL;
    size_t i = 0;

    if (pArray != NULL)
    {
        for (i = 0; i < pArray->length; i++)
        {
            e = SSOJsonObjectNew(&pJsonValue);
            BAIL_ON_ERROR(e);

            e = f(pArray->ppEntry[i], pJsonValue);
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
RestJsonToArrayData(
    PCSSO_JSON pJson,
    JsonToDataObjectFunc f,
    REST_GENERIC_ARRAY_DATA** ppArray)
{
    SSOERROR e = SSOERROR_NONE;
    PSSO_JSON pJsonValue = NULL;
    REST_GENERIC_ARRAY_DATA* pArray = NULL;
    size_t i = 0;

    if (pJson == NULL || f == NULL || ppArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(REST_GENERIC_ARRAY_DATA), (void**) &pArray);
    BAIL_ON_ERROR(e);

    e = SSOJsonArraySize(pJson, &(pArray->length));
    BAIL_ON_ERROR(e);

    e = SSOMemoryAllocateArray(pArray->length, sizeof(void*), (void**) &(pArray->ppEntry));
    BAIL_ON_ERROR(e);

    for (i = 0; i < pArray->length; i++)
    {
        e = SSOJsonArrayGet(pJson, i, &pJsonValue);
        BAIL_ON_ERROR(e);

        e = f(pJsonValue, &(pArray->ppEntry[i]));
        BAIL_ON_ERROR(e);

        SSOJsonDelete(pJsonValue);
    }

    *ppArray = pArray;

    error:

    if (e != SSOERROR_NONE)
    {
        if (pArray != NULL)
        {
            for (i = 0; i < pArray->length; i++)
            {
                SSOMemoryFree(pArray->ppEntry[i], 0);
            }
            SSOMemoryFree(pArray, sizeof(REST_GENERIC_ARRAY_DATA));
        }
    }

    return e;
}
