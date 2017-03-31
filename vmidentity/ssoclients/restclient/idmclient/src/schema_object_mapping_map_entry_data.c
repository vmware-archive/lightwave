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
IdmSchemaObjectMappingMapEntryDataNew(
    IDM_SCHEMA_OBJECT_MAPPING_MAP_ENTRY_DATA** ppSchemaObjectMappingMapEntry,
    PCSTRING key,
    const IDM_SCHEMA_OBJECT_MAPPING_DATA* value)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_SCHEMA_OBJECT_MAPPING_MAP_ENTRY_DATA* pSchemaObjectMappingMapEntry = NULL;

    if (ppSchemaObjectMappingMapEntry == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_SCHEMA_OBJECT_MAPPING_MAP_ENTRY_DATA), (void**) &pSchemaObjectMappingMapEntry);
    BAIL_ON_ERROR(e);

    if (key != NULL)
    {
        e = RestStringDataNew(&(pSchemaObjectMappingMapEntry->key), key);
        BAIL_ON_ERROR(e);
    }

    if (value != NULL)
    {
        e = IdmSchemaObjectMappingDataNew(
            &(pSchemaObjectMappingMapEntry->value),
            value->objectClass,
            value->attributeMappings);
        BAIL_ON_ERROR(e);
    }

    *ppSchemaObjectMappingMapEntry = pSchemaObjectMappingMapEntry;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmSchemaObjectMappingMapEntryDataDelete(pSchemaObjectMappingMapEntry);
    }

    return e;
}

void
IdmSchemaObjectMappingMapEntryDataDelete(
    IDM_SCHEMA_OBJECT_MAPPING_MAP_ENTRY_DATA* pSchemaObjectMappingMapEntry)
{
    if (pSchemaObjectMappingMapEntry != NULL)
    {
        RestStringDataDelete(pSchemaObjectMappingMapEntry->key);
        IdmSchemaObjectMappingDataDelete(pSchemaObjectMappingMapEntry->value);
        SSOMemoryFree(pSchemaObjectMappingMapEntry, sizeof(IDM_SCHEMA_OBJECT_MAPPING_MAP_ENTRY_DATA));
    }
}
