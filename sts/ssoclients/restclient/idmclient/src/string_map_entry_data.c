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
IdmStringMapEntryDataNew(
    IDM_STRING_MAP_ENTRY_DATA** ppStringMapEntry,
    PCSTRING key,
    PCSTRING value)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_STRING_MAP_ENTRY_DATA* pStringMapEntry = NULL;

    if (ppStringMapEntry == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_STRING_MAP_ENTRY_DATA), (void**) &pStringMapEntry);
    BAIL_ON_ERROR(e);

    if (key != NULL)
    {
        e = RestStringDataNew(&(pStringMapEntry->key), key);
        BAIL_ON_ERROR(e);
    }

    if (value != NULL)
    {
        e = RestStringDataNew(&(pStringMapEntry->value), value);
        BAIL_ON_ERROR(e);
    }

    *ppStringMapEntry = pStringMapEntry;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmStringMapEntryDataDelete(pStringMapEntry);
    }

    return e;
}

void
IdmStringMapEntryDataDelete(
    IDM_STRING_MAP_ENTRY_DATA* pStringMapEntry)
{
    if (pStringMapEntry != NULL)
    {
        RestStringDataDelete(pStringMapEntry->key);
        RestStringDataDelete(pStringMapEntry->value);
        SSOMemoryFree(pStringMapEntry, sizeof(IDM_STRING_MAP_ENTRY_DATA));
    }
}
