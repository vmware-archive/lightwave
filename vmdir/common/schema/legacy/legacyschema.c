/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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

DWORD
VmDirLegacySchemaInit(
    PVDIR_LEGACY_SCHEMA*    ppLegacySchema
    )
{
    DWORD   dwError = 0;
    PVDIR_LEGACY_SCHEMA pLegacySchema = NULL;

    if (!ppLegacySchema)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_LEGACY_SCHEMA),
            (PVOID*)&pLegacySchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(&pLegacySchema->pAtDefStrMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(&pLegacySchema->pOcDefStrMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(&pLegacySchema->pCrDefStrMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapSchemaInit(&pLegacySchema->pSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppLegacySchema = pLegacySchema;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirFreeLegacySchema(pLegacySchema);
    goto cleanup;
}

static
VOID
_FreeDefStrMapPair(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    )
{
    if (pPair)
    {
        VMDIR_SAFE_FREE_MEMORY(pPair->pValue);
    }
}

VOID
VmDirFreeLegacySchema(
    PVDIR_LEGACY_SCHEMA pLegacySchema
    )
{
    if (pLegacySchema)
    {
        if (pLegacySchema->pAtDefStrMap)
        {
            LwRtlHashMapClear(pLegacySchema->pAtDefStrMap, _FreeDefStrMapPair, NULL);
            LwRtlFreeHashMap(&pLegacySchema->pAtDefStrMap);
        }

        if (pLegacySchema->pOcDefStrMap)
        {
            LwRtlHashMapClear(pLegacySchema->pOcDefStrMap, _FreeDefStrMapPair, NULL);
            LwRtlFreeHashMap(&pLegacySchema->pOcDefStrMap);
        }

        if (pLegacySchema->pCrDefStrMap)
        {
            LwRtlHashMapClear(pLegacySchema->pCrDefStrMap, _FreeDefStrMapPair, NULL);
            LwRtlFreeHashMap(&pLegacySchema->pCrDefStrMap);
        }

        if (pLegacySchema->pSchema)
        {
            VmDirFreeLdapSchema(pLegacySchema->pSchema);
        }

        VMDIR_SAFE_FREE_MEMORY(pLegacySchema);
    }
}
