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
IdmSchemaObjectMappingDataNew(
    IDM_SCHEMA_OBJECT_MAPPING_DATA** ppSchemaObjectMapping,
    PCSTRING objectClass,
    const IDM_STRING_MAP_DATA* attributeMappings)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_SCHEMA_OBJECT_MAPPING_DATA* pSchemaObjectMapping = NULL;

    if (ppSchemaObjectMapping == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_SCHEMA_OBJECT_MAPPING_DATA), (void**) &pSchemaObjectMapping);
    BAIL_ON_ERROR(e);

    if (objectClass != NULL)
    {
        e = RestStringDataNew(&(pSchemaObjectMapping->objectClass), objectClass);
        BAIL_ON_ERROR(e);
    }

    if (attributeMappings != NULL)
    {
        e = IdmStringMapDataNew(
            &(pSchemaObjectMapping->attributeMappings),
            attributeMappings->ppEntry,
            attributeMappings->length);
        BAIL_ON_ERROR(e);
    }

    *ppSchemaObjectMapping = pSchemaObjectMapping;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmSchemaObjectMappingDataDelete(pSchemaObjectMapping);
    }

    return e;
}

void
IdmSchemaObjectMappingDataDelete(
    IDM_SCHEMA_OBJECT_MAPPING_DATA* pSchemaObjectMapping)
{
    if (pSchemaObjectMapping != NULL)
    {
        RestStringDataDelete(pSchemaObjectMapping->objectClass);
        IdmStringMapDataDelete(pSchemaObjectMapping->attributeMappings);
        SSOMemoryFree(pSchemaObjectMapping, sizeof(IDM_SCHEMA_OBJECT_MAPPING_DATA));
    }
}

SSOERROR
IdmSchemaObjectMappingDataToJson(
    const IDM_SCHEMA_OBJECT_MAPPING_DATA* pSchemaObjectMapping,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pSchemaObjectMapping != NULL)
    {
        e = RestDataToJson(
            pSchemaObjectMapping->objectClass,
            REST_JSON_OBJECT_TYPE_STRING,
            NULL,
            "objectClass",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pSchemaObjectMapping->attributeMappings,
            REST_JSON_OBJECT_TYPE_OBJECT,
            (DataObjectToJsonFunc) IdmStringMapDataToJson,
            "attributeMappings",
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToSchemaObjectMappingData(
    PCSSO_JSON pJson,
    IDM_SCHEMA_OBJECT_MAPPING_DATA** ppSchemaObjectMapping)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_SCHEMA_OBJECT_MAPPING_DATA* pSchemaObjectMapping = NULL;

    if (pJson == NULL || ppSchemaObjectMapping == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_SCHEMA_OBJECT_MAPPING_DATA), (void**) &pSchemaObjectMapping);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "objectClass",
        (void**) &(pSchemaObjectMapping->objectClass));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_OBJECT,
        (JsonToDataObjectFunc) IdmJsonToStringMapData,
        "attributeMappings",
        (void**) &(pSchemaObjectMapping->attributeMappings));
    BAIL_ON_ERROR(e);

    *ppSchemaObjectMapping = pSchemaObjectMapping;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmSchemaObjectMappingDataDelete(pSchemaObjectMapping);
    }

    return e;
}
