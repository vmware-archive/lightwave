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
IdmSchemaObjectMappingMapDataNew(
    IDM_SCHEMA_OBJECT_MAPPING_MAP_DATA** ppSchemaObjectMappingMap,
    IDM_SCHEMA_OBJECT_MAPPING_MAP_ENTRY_DATA** ppEntry,
    size_t length)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_SCHEMA_OBJECT_MAPPING_MAP_DATA* pSchemaObjectMappingMap = NULL;
    size_t i = 0;

    if (ppSchemaObjectMappingMap == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_SCHEMA_OBJECT_MAPPING_MAP_DATA), (void**) &pSchemaObjectMappingMap);
    BAIL_ON_ERROR(e);

    if (ppEntry != NULL)
    {
        pSchemaObjectMappingMap->length = length;

        e = SSOMemoryAllocateArray(
            length,
            sizeof(IDM_SCHEMA_OBJECT_MAPPING_MAP_ENTRY_DATA*),
            (void**) &(pSchemaObjectMappingMap->ppEntry));
        BAIL_ON_ERROR(e);

        for (i = 0; i < length; i++)
        {
            e = IdmSchemaObjectMappingMapEntryDataNew(
                &(pSchemaObjectMappingMap->ppEntry[i]),
                ppEntry[i]->key,
                ppEntry[i]->value);
            BAIL_ON_ERROR(e);
        }
    }

    *ppSchemaObjectMappingMap = pSchemaObjectMappingMap;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmSchemaObjectMappingMapDataDelete(pSchemaObjectMappingMap);
    }

    return e;
}

void
IdmSchemaObjectMappingMapDataDelete(
    IDM_SCHEMA_OBJECT_MAPPING_MAP_DATA* pSchemaObjectMappingMap)
{
    if (pSchemaObjectMappingMap != NULL)
    {
        SSOMemoryFreeArrayOfObjects(
            (void**) pSchemaObjectMappingMap->ppEntry,
            pSchemaObjectMappingMap->length,
            (GenericDestructorFunction) IdmSchemaObjectMappingMapEntryDataDelete);
        SSOMemoryFree(pSchemaObjectMappingMap, sizeof(IDM_SCHEMA_OBJECT_MAPPING_MAP_DATA));
    }
}

SSOERROR
IdmSchemaObjectMappingMapDataToJson(
    const IDM_SCHEMA_OBJECT_MAPPING_MAP_DATA* pSchemaObjectMappingMap,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;
    size_t i = 0;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pSchemaObjectMappingMap != NULL)
    {
        for (i = 0; i < pSchemaObjectMappingMap->length; i++)
        {
            e = RestDataToJson(
                pSchemaObjectMappingMap->ppEntry[i]->value,
                REST_JSON_OBJECT_TYPE_OBJECT,
                (DataObjectToJsonFunc) IdmSchemaObjectMappingDataToJson,
                pSchemaObjectMappingMap->ppEntry[i]->key,
                pJson);
            BAIL_ON_ERROR(e);
        }
    }

    error:

    return e;
}

SSOERROR
IdmJsonToSchemaObjectMappingMapData(
    PCSSO_JSON pJson,
    IDM_SCHEMA_OBJECT_MAPPING_MAP_DATA** ppSchemaObjectMappingMap)
{
    SSOERROR e = SSOERROR_NONE;
    bool isObject = false;
    IDM_SCHEMA_OBJECT_MAPPING_MAP_DATA* pSchemaObjectMappingMap = NULL;
    IDM_SCHEMA_OBJECT_MAPPING_DATA* pSchemaObjectMapping = NULL;
    PSSO_JSON_ITERATOR pJsonIter = NULL;
    PSSO_JSON_ITERATOR pJsonIterNext = NULL;
    bool jsonIterHasNext = false;
    PCSTRING key = NULL;
    PSSO_JSON pJsonValue = NULL;
    size_t i = 0;

    if (pJson == NULL || ppSchemaObjectMappingMap == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_SCHEMA_OBJECT_MAPPING_MAP_DATA), (void**) &pSchemaObjectMappingMap);
    BAIL_ON_ERROR(e);

    e = SSOJsonObjectSize(pJson, &(pSchemaObjectMappingMap->length));
    BAIL_ON_ERROR(e);

    e = SSOMemoryAllocateArray(
        pSchemaObjectMappingMap->length,
        sizeof(IDM_SCHEMA_OBJECT_MAPPING_MAP_ENTRY_DATA*),
        (void**) &(pSchemaObjectMappingMap->ppEntry));
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

        e = SSOJsonIsObject(pJsonValue, &isObject);
        BAIL_ON_ERROR(e);

        if (isObject)
        {
            e = IdmJsonToSchemaObjectMappingData(pJsonValue, &pSchemaObjectMapping);
            BAIL_ON_ERROR(e);

            e = IdmSchemaObjectMappingMapEntryDataNew(
                &(pSchemaObjectMappingMap->ppEntry[i]),
                key,
                pSchemaObjectMapping);
            BAIL_ON_ERROR(e);

            IdmSchemaObjectMappingDataDelete(pSchemaObjectMapping);
        }
        else
        {
            e = SSOERROR_JSON_FAILURE;
            BAIL_ON_ERROR(e);
        }

        SSOJsonDelete(pJsonValue);

        i = i + 1;

        e = SSOJsonObjectIteratorNext(pJson, pJsonIter, &pJsonIterNext);
        BAIL_ON_ERROR(e);

        // cleanup the old pJsonIter
        SSOJsonIteratorDelete(pJsonIter);

        pJsonIter = pJsonIterNext;

        e = SSOJsonObjectIteratorHasNext(pJsonIter, &jsonIterHasNext);
        BAIL_ON_ERROR(e);
    }

    *ppSchemaObjectMappingMap = pSchemaObjectMappingMap;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmSchemaObjectMappingMapDataDelete(pSchemaObjectMappingMap);
    }

    // cleanup
    SSOJsonIteratorDelete(pJsonIter);

    return e;
}
