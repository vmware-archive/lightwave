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
IdmStringMapDataNew(
    IDM_STRING_MAP_DATA** ppStringMap,
    IDM_STRING_MAP_ENTRY_DATA** ppEntry,
    size_t length)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_STRING_MAP_DATA* pStringMap = NULL;
    size_t i = 0;

    if (ppStringMap == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_STRING_MAP_DATA), (void**) &pStringMap);
    BAIL_ON_ERROR(e);

    if (ppEntry != NULL)
    {
        pStringMap->length = length;

        e = SSOMemoryAllocateArray(length, sizeof(IDM_STRING_MAP_ENTRY_DATA*), (void**) &(pStringMap->ppEntry));
        BAIL_ON_ERROR(e);

        for (i = 0; i < length; i++)
        {
            e = IdmStringMapEntryDataNew(&(pStringMap->ppEntry[i]), ppEntry[i]->key, ppEntry[i]->value);
            BAIL_ON_ERROR(e);
        }
    }

    *ppStringMap = pStringMap;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmStringMapDataDelete(pStringMap);
    }

    return e;
}

void
IdmStringMapDataDelete(
    IDM_STRING_MAP_DATA* pStringMap)
{
    if (pStringMap != NULL)
    {
        SSOMemoryFreeArrayOfObjects(
            (void**) pStringMap->ppEntry,
            pStringMap->length,
            (GenericDestructorFunction) IdmStringMapEntryDataDelete);
        SSOMemoryFree(pStringMap, sizeof(IDM_STRING_MAP_DATA));
    }
}

SSOERROR
IdmStringMapDataToJson(
    const IDM_STRING_MAP_DATA* pStringMap,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;
    size_t i = 0;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pStringMap != NULL)
    {
        for (i = 0; i < pStringMap->length; i++)
        {
            e = RestDataToJson(
                pStringMap->ppEntry[i]->value,
                REST_JSON_OBJECT_TYPE_STRING,
                NULL,
                pStringMap->ppEntry[i]->key,
                pJson);
            BAIL_ON_ERROR(e);
        }
    }

    error:

    return e;
}

SSOERROR
IdmJsonToStringMapData(
    PCSSO_JSON pJson,
    IDM_STRING_MAP_DATA** ppStringMap)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_STRING_MAP_DATA* pStringMap = NULL;
    PSSO_JSON_ITERATOR pJsonIter = NULL;
    PSSO_JSON_ITERATOR pJsonIterNext = NULL;
    bool jsonIterHasNext = false;
    PCSTRING key = NULL;
    PSSO_JSON pJsonValue = NULL;
    PSTRING value = NULL;
    size_t i = 0;

    if (pJson == NULL || ppStringMap == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_STRING_MAP_DATA), (void**) &pStringMap);
    BAIL_ON_ERROR(e);

    e = SSOJsonObjectSize(pJson, &(pStringMap->length));
    BAIL_ON_ERROR(e);

    e = SSOMemoryAllocateArray(pStringMap->length, sizeof(IDM_STRING_MAP_ENTRY_DATA*), (void**) &(pStringMap->ppEntry));
    BAIL_ON_ERROR(e);

    e = SSOJsonObjectIterator(pJson, &pJsonIter);
    BAIL_ON_ERROR(e);

    e = SSOJsonObjectIteratorHasNext(pJsonIter, &jsonIterHasNext);
    BAIL_ON_ERROR(e);

    while (jsonIterHasNext)
    {
        e = SSOJsonObjectIteratorKey(pJsonIter, &key);
        BAIL_ON_ERROR(e);

        e = SSOJsonObjectIteratorValue(pJsonIter, &pJsonValue);
        BAIL_ON_ERROR(e);

        e = SSOJsonStringValue(pJsonValue, &value);
        BAIL_ON_ERROR(e);

        SSOJsonDelete(pJsonValue);

        e = IdmStringMapEntryDataNew(&(pStringMap->ppEntry[i]), key, value);
        BAIL_ON_ERROR(e);

        SSOStringFree(value);

        i = i + 1;

        e = SSOJsonObjectIteratorNext(pJson, pJsonIter, &pJsonIterNext);
        BAIL_ON_ERROR(e);

        // cleanup the old pJsonIter
        SSOJsonIteratorDelete(pJsonIter);

        pJsonIter = pJsonIterNext;

        e = SSOJsonObjectIteratorHasNext(pJsonIter, &jsonIterHasNext);
        BAIL_ON_ERROR(e);
    }

    *ppStringMap = pStringMap;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmStringMapDataDelete(pStringMap);
    }

    // cleanup
    SSOJsonIteratorDelete(pJsonIter);

    return e;
}
