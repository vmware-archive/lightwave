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

void
SSOJsonDelete(
    PSSO_JSON pJson)
{
    if (pJson != NULL)
    {
        json_decref(pJson->pJson_t);
        SSOMemoryFree(pJson, sizeof(SSO_JSON));
    }
}

SSOERROR
SSOJsonObjectNew(
    PSSO_JSON* ppJson)
{
    SSOERROR e = SSOERROR_NONE;
    SSO_JSON* pJson = NULL;

    if (ppJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(SSO_JSON), (void**) &pJson);
    BAIL_ON_ERROR(e);

    pJson->pJson_t = json_object();

    if (pJson->pJson_t == NULL)
    {
        e = SSOERROR_JSON_FAILURE;
        BAIL_ON_ERROR(e);
    }

    *ppJson = pJson;

    error:

    if (e != SSOERROR_NONE)
    {
        SSOJsonDelete(pJson);
    }

    return e;
}

SSOERROR
SSOJsonArrayNew(
    PSSO_JSON* ppJson)
{
    SSOERROR e = SSOERROR_NONE;
    SSO_JSON* pJson = NULL;

    if (ppJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(SSO_JSON), (void**) &pJson);
    BAIL_ON_ERROR(e);

    pJson->pJson_t = json_array();

    if (pJson->pJson_t == NULL)
    {
        e = SSOERROR_JSON_FAILURE;
        BAIL_ON_ERROR(e);
    }

    *ppJson = pJson;

    error:

    if (e != SSOERROR_NONE)
    {
        SSOJsonDelete(pJson);
    }

    return e;
}

SSOERROR
SSOJsonStringNew(
    PSSO_JSON* ppJson,
    PCSTRING value)
{
    SSOERROR e = SSOERROR_NONE;
    SSO_JSON* pJson = NULL;

    if (ppJson == NULL || value == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(SSO_JSON), (void**) &pJson);
    BAIL_ON_ERROR(e);

    pJson->pJson_t = json_string(value);

    if (pJson->pJson_t == NULL)
    {
        e = SSOERROR_JSON_FAILURE;
        BAIL_ON_ERROR(e);
    }

    *ppJson = pJson;

    error:

    if (e != SSOERROR_NONE)
    {
        SSOJsonDelete(pJson);
    }

    return e;
}

SSOERROR
SSOJsonIntegerNew(
    PSSO_JSON* ppJson,
    INTEGER value)
{
    SSOERROR e = SSOERROR_NONE;
    SSO_JSON* pJson = NULL;

    if (ppJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(SSO_JSON), (void**) &pJson);
    BAIL_ON_ERROR(e);

    pJson->pJson_t = json_integer(value);

    if (pJson->pJson_t == NULL)
    {
        e = SSOERROR_JSON_FAILURE;
        BAIL_ON_ERROR(e);
    }

    *ppJson = pJson;

    error:

    if (e != SSOERROR_NONE)
    {
        SSOJsonDelete(pJson);
    }

    return e;
}

SSOERROR
SSOJsonLongNew(
    PSSO_JSON* ppJson,
    SSO_LONG value)
{
    SSOERROR e = SSOERROR_NONE;
    SSO_JSON* pJson = NULL;

    if (ppJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(SSO_JSON), (void**) &pJson);
    BAIL_ON_ERROR(e);

    pJson->pJson_t = json_integer(value);

    if (pJson->pJson_t == NULL)
    {
        e = SSOERROR_JSON_FAILURE;
        BAIL_ON_ERROR(e);
    }

    *ppJson = pJson;

    error:

    if (e != SSOERROR_NONE)
    {
        SSOJsonDelete(pJson);
    }

    return e;
}

SSOERROR
SSOJsonBooleanNew(
    PSSO_JSON* ppJson,
    bool value)
{
    SSOERROR e = SSOERROR_NONE;
    SSO_JSON* pJson = NULL;

    if (ppJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(SSO_JSON), (void**) &pJson);
    BAIL_ON_ERROR(e);

    pJson->pJson_t = json_boolean(value);

    if (pJson->pJson_t == NULL)
    {
        e = SSOERROR_JSON_FAILURE;
        BAIL_ON_ERROR(e);
    }

    *ppJson = pJson;

    error:

    if (e != SSOERROR_NONE)
    {
        SSOJsonDelete(pJson);
    }

    return e;
}

SSOERROR
SSOJsonObjectSet(
    PSSO_JSON pJson,
    PCSTRING key,
    PCSSO_JSON pJsonValue)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL || json_is_object(pJson->pJson_t) == 0 || IS_NULL_OR_EMPTY_STRING(key) || pJsonValue == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (json_object_set(pJson->pJson_t, key, pJsonValue->pJson_t) != 0)
    {
        e = SSOERROR_JSON_FAILURE;
        BAIL_ON_ERROR(e);
    };

    error:

    return e;
}

SSOERROR
SSOJsonIsObject(
    PCSSO_JSON pJson,
    bool* pBool)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL || pBool == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    *pBool = json_is_object(pJson->pJson_t) != 0;

    error:

    return e;
}

SSOERROR
SSOJsonObjectSize(
    PCSSO_JSON pJson,
    size_t* pSize)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL || json_is_object(pJson->pJson_t) == 0 || pSize == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    *pSize = json_object_size(pJson->pJson_t);

    error:

    return e;
}

SSOERROR
SSOJsonObjectGet(
    PCSSO_JSON pJson,
    PCSTRING key,
    PSSO_JSON* ppJsonValue)
{
    SSOERROR e = SSOERROR_NONE;
    SSO_JSON* pJsonValue = NULL;

    if (pJson == NULL || json_is_object(pJson->pJson_t) == 0 || IS_NULL_OR_EMPTY_STRING(key) || ppJsonValue == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(SSO_JSON), (void**) &pJsonValue);
    BAIL_ON_ERROR(e);

    pJsonValue->pJson_t = json_deep_copy(json_object_get(pJson->pJson_t, key));

    if (pJsonValue->pJson_t == NULL)
    {
        // NULL is ok, means no value is found for this key.
    }

    *ppJsonValue = pJsonValue;

    error:

    if (e != SSOERROR_NONE)
    {
        SSOJsonDelete(pJsonValue);
    }

    return e;
}

SSOERROR
SSOJsonStringValue(
    PCSSO_JSON pJson,
    PSTRING* pValue)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING value;

    if (pJson == NULL || json_is_string(pJson->pJson_t) == 0 || pValue == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOStringAllocate(json_string_value(pJson->pJson_t), &value);

    if (value == NULL)
    {
        e = SSOERROR_JSON_FAILURE;
        BAIL_ON_ERROR(e);
    }

    *pValue = value;

    error:

    return e;
}

SSOERROR
SSOJsonIntegerValue(
    PCSSO_JSON pJson,
    INTEGER* pValue)
{
    SSOERROR e = SSOERROR_NONE;
    INTEGER value;

    if (pJson == NULL || json_is_integer(pJson->pJson_t) == 0 || pValue == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    value = json_integer_value(pJson->pJson_t);

    *pValue = value;

    error:

    return e;
}

SSOERROR
SSOJsonLongValue(
    PCSSO_JSON pJson,
    SSO_LONG* pValue)
{
    SSOERROR e = SSOERROR_NONE;
    SSO_LONG value;

    if (pJson == NULL || json_is_integer(pJson->pJson_t) == 0 || pValue == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    value = json_integer_value(pJson->pJson_t);

    *pValue = value;

    error:

    return e;
}

SSOERROR
SSOJsonBooleanValue(
    PCSSO_JSON pJson,
    bool* pValue)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL || json_is_boolean(pJson->pJson_t) == 0 || pValue == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    *pValue = json_is_true(pJson->pJson_t) != 0;

    error:

    return e;
}

SSOERROR
SSOJsonArrayAppend(
    PSSO_JSON pJson,
    PCSSO_JSON pJsonValue)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL || json_is_array(pJson->pJson_t) == 0 || pJsonValue == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (json_array_append(pJson->pJson_t, pJsonValue->pJson_t) != 0)
    {
        e = SSOERROR_JSON_FAILURE;
        BAIL_ON_ERROR(e);
    };

    error:

    return e;
}

SSOERROR
SSOJsonArraySize(
    PCSSO_JSON pJson,
    size_t* pSize)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL || json_is_array(pJson->pJson_t) == 0 || pSize == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    *pSize = json_array_size(pJson->pJson_t);

    error:

    return e;
}

SSOERROR
SSOJsonArrayGet(
    PCSSO_JSON pJson,
    size_t index,
    PSSO_JSON* ppJsonValue)
{
    SSOERROR e = SSOERROR_NONE;
    SSO_JSON* pJsonValue = NULL;

    if (pJson == NULL || json_is_array(pJson->pJson_t) == 0 || ppJsonValue == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(SSO_JSON), (void**) &pJsonValue);
    BAIL_ON_ERROR(e);

    pJsonValue->pJson_t = json_deep_copy(json_array_get(pJson->pJson_t, index));

    if (pJsonValue->pJson_t == NULL)
    {
        e = SSOERROR_JSON_FAILURE;
        BAIL_ON_ERROR(e);
    }

    *ppJsonValue = pJsonValue;

    error:

    if (e != SSOERROR_NONE)
    {
        SSOJsonDelete(pJsonValue);
    }

    return e;
}

SSOERROR
SSOJsonToString(
    PCSSO_JSON pJson,
    PSTRING* ppString)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING pString = NULL;

    if (pJson == NULL || ppString == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    pString = json_dumps(pJson->pJson_t, JSON_ENCODE_ANY);

    if (pString == NULL)
    {
        e = SSOERROR_JSON_FAILURE;
        BAIL_ON_ERROR(e);
    }

    *ppString = pString;

    error:

    if (e != SSOERROR_NONE)
    {
        SSOStringFree(pString);
    }

    return e;
}

void
SSOJsonIteratorDelete(
    PSSO_JSON_ITERATOR pJsonIter)
{
    if (pJsonIter != NULL)
    {
        SSOMemoryFree(pJsonIter, sizeof(SSO_JSON_ITERATOR));
    }
}

SSOERROR
SSOJsonObjectIterator(
    PCSSO_JSON pJson,
    PSSO_JSON_ITERATOR* ppJsonIter)
{
    SSOERROR e = SSOERROR_NONE;
    SSO_JSON_ITERATOR* pJsonIter = NULL;
    void* p = NULL;

    if (pJson == NULL || json_is_object(pJson->pJson_t) == 0 || ppJsonIter == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(SSO_JSON_ITERATOR), (void**) &pJsonIter);
    BAIL_ON_ERROR(e);

    p = json_object_iter(pJson->pJson_t);

    if (p == NULL)
    {
        // NULL is ok, means no key-value pair left for iteration.
    }

    pJsonIter->pIter = p;

    *ppJsonIter = pJsonIter;

    error:

    if (e != SSOERROR_NONE)
    {
        SSOJsonIteratorDelete(pJsonIter);
    }

    return e;
}

SSOERROR
SSOJsonObjectIteratorHasNext(
    PCSSO_JSON_ITERATOR pJsonIter,
    bool* pBool)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJsonIter == NULL || pBool == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    *pBool = pJsonIter->pIter != NULL;

    error:

    return e;
}

SSOERROR
SSOJsonObjectIteratorNext(
    PCSSO_JSON pJson,
    PCSSO_JSON_ITERATOR pJsonIter,
    PSSO_JSON_ITERATOR* ppJsonIterNext)
{
    SSOERROR e = SSOERROR_NONE;
    SSO_JSON_ITERATOR* pJsonIterNext = NULL;
    void* p = NULL;

    if (pJson == NULL || json_is_object(pJson->pJson_t) == 0 || pJsonIter == NULL || ppJsonIterNext == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(SSO_JSON_ITERATOR), (void**) &pJsonIterNext);
    BAIL_ON_ERROR(e);

    p = json_object_iter_next(pJson->pJson_t, pJsonIter->pIter);

    if (p == NULL)
    {
        // NULL is ok, means no key-value pair left for iteration.
    }

    pJsonIterNext->pIter = p;

    *ppJsonIterNext = pJsonIterNext;

    error:

    if (e != SSOERROR_NONE)
    {
        SSOJsonIteratorDelete(pJsonIterNext);
    }

    return e;
}

SSOERROR
SSOJsonObjectIteratorKey(
    PCSSO_JSON_ITERATOR pJsonIter,
    PCSTRING* ppKey)
{
    SSOERROR e = SSOERROR_NONE;
    PCSTRING pKey;

    if (pJsonIter == NULL || ppKey == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    pKey = json_object_iter_key(pJsonIter->pIter);

    if (pKey == NULL)
    {
        e = SSOERROR_JSON_FAILURE;
        BAIL_ON_ERROR(e);
    }

    *ppKey = pKey;

    error:

    return e;
}

SSOERROR
SSOJsonObjectIteratorValue(
    PCSSO_JSON_ITERATOR pJsonIter,
    PSSO_JSON* ppJsonValue)
{
    SSOERROR e = SSOERROR_NONE;
    SSO_JSON* pJsonValue;

    if (pJsonIter == NULL || ppJsonValue == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(SSO_JSON), (void**) &pJsonValue);
    BAIL_ON_ERROR(e);

    pJsonValue->pJson_t = json_deep_copy(json_object_iter_value(pJsonIter->pIter));

    if (pJsonValue->pJson_t == NULL)
    {
        // NULL is ok, value can be NULL.
    }

    *ppJsonValue = pJsonValue;

    error:

    if (e != SSOERROR_NONE)
    {
        SSOJsonDelete(pJsonValue);
    }

    return e;
}

SSOERROR
SSOJsonParse(
    PSSO_JSON* ppJson,
    PCSTRING pData)
{
    SSOERROR e = SSOERROR_NONE;
    SSO_JSON* pJson = NULL;
    json_error_t error;

    if (ppJson == NULL || pData == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(SSO_JSON), (void**) &pJson);
    BAIL_ON_ERROR(e);

    pJson->pJson_t = json_loads(pData, JSON_DECODE_ANY, &error);

    if (pJson->pJson_t == NULL)
    {
        e = SSOERROR_JSON_PARSE_FAILURE;
        BAIL_ON_ERROR(e);
    }

    *ppJson = pJson;

    error:

    if (e != SSOERROR_NONE)
    {
        SSOJsonDelete(pJson);
    }

    return e;
}

SSOERROR
SSOJsonReset(
    PSSO_JSON pJson,
    PCSSO_JSON pIn)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL || pIn == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pJson->pJson_t != NULL)
    {
        json_decref(pJson->pJson_t);
    }

    pJson->pJson_t = json_deep_copy(pIn->pJson_t);

    if (pJson->pJson_t == NULL)
    {
        e = SSOERROR_JSON_FAILURE;
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
SSOJsonEquals(
    PCSSO_JSON pJson1,
    PCSSO_JSON pJson2,
    bool* pBool)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson1 == NULL || pJson2 == NULL || pBool == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    *pBool = json_equal(pJson1->pJson_t, pJson2->pJson_t) == 1;

    error:

    return e;
}

SSOERROR
SSOJsonIsNull(
    PCSSO_JSON pJson,
    bool* pBool)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL || pBool == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    *pBool = pJson->pJson_t == NULL;

    error:

    return e;
}
