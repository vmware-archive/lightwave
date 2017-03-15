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

#define VDIR_SCHEMA_NAMING_CONTEXT_ENTRY_INITIALIZER    \
{                                                       \
    "objectclass",  "dmd",                              \
    "cn",           "schemacontext",                    \
    NULL                                                \
}

static
DWORD
_MarkDefaultIndices(
    VOID
    );

/*
 * Examines the following options in order and use the first detected source
 * to initialize schema:
 *
 * 1. Schema subtree (6.6 and higher)
 * 2. Subschema subentry (6.5 and lower)
 * 3. Schema file
 *
 * OUTPUT:
 * pbWriteSchemaEntry will be TRUE if option 3 was used
 * pbLegacyDataLoaded will be TRUE if option 2 was used
 */
DWORD
VmDirLoadSchema(
    PBOOLEAN    pbWriteSchemaEntry,
    PBOOLEAN    pbLegacyDataLoaded
    )
{
    DWORD               dwError = 0;
    PVDIR_ENTRY_ARRAY   pAtEntries = NULL;
    PVDIR_ENTRY_ARRAY   pOcEntries = NULL;
    PVDIR_ENTRY         pSchemaEntry = NULL;

    assert(pbWriteSchemaEntry && pbLegacyDataLoaded);

    dwError = VmDirReadAttributeSchemaObjects(&pAtEntries);
    if (dwError == 0)
    {
        dwError = VmDirSchemaLibLoadAttributeSchemaEntries(pAtEntries);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirReadClassSchemaObjects(&pOcEntries);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirSchemaLibLoadClassSchemaEntries(pOcEntries);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (dwError == ERROR_BACKEND_ENTRY_NOTFOUND)
    {
        dwError = VmDirReadSubSchemaSubEntry(&pSchemaEntry);
        if (dwError == 0)
        {
            dwError = VmDirSchemaLibLoadSubSchemaSubEntry(pSchemaEntry);
            BAIL_ON_VMDIR_ERROR(dwError);

            *pbLegacyDataLoaded = TRUE;
        }
        else if (dwError == ERROR_BACKEND_ENTRY_NOTFOUND)
        {
            PSTR pszSchemaFilePath = gVmdirGlobals.pszBootStrapSchemaFile;
            if (!pszSchemaFilePath)
            {
                dwError = ERROR_NO_SCHEMA;
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            dwError = VmDirSchemaLibLoadFile(pszSchemaFilePath);
            BAIL_ON_VMDIR_ERROR(dwError);

            *pbWriteSchemaEntry = TRUE;
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _MarkDefaultIndices();
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirFreeEntryArray(pAtEntries);
    VmDirFreeEntryArray(pOcEntries);
    VmDirFreeEntry(pSchemaEntry);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

/*
 * Initialize schemacontext subtree entries
 * Should be called if InitializeSchema() results pbWriteSchemaEntry = TRUE
 */
DWORD
InitializeSchemaEntries(
    PVDIR_SCHEMA_CTX    pSchemaCtx
    )
{
    DWORD dwError = 0;

    static PSTR ppszSchemaContext[] =
            VDIR_SCHEMA_NAMING_CONTEXT_ENTRY_INITIALIZER;

    dwError = VmDirSimpleEntryCreate(
            pSchemaCtx,
            ppszSchemaContext,
            SCHEMA_NAMING_CONTEXT_DN,
            SCHEMA_NAMING_CONTEXT_ID);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirPatchLocalSchemaObjects(NULL, pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );
    goto cleanup;
}

/*
 * If upgrading from 6.6 or higher, use this function to patch schema.
 *
 * INPUT:
 * new version of Lotus schema file
 */
DWORD
VmDirSchemaPatchViaFile(
    PCSTR       pszSchemaFilePath
    )
{
    DWORD    dwError = 0;
    PVDIR_SCHEMA_CTX    pOldSchemaCtx = NULL;
    PVDIR_SCHEMA_CTX    pNewSchemaCtx = NULL;
    PVDIR_BACKEND_INTERFACE pBE = NULL;

    dwError = VmDirSchemaCtxAcquire(&pOldSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaLibLoadFile(pszSchemaFilePath);
    BAIL_ON_VMDIR_ERROR(dwError);

    // support for mixed version (with 6.5 or lower) federation upgrade scenario
    dwError = VmDirPatchLocalSubSchemaSubEntry();
    dwError = dwError == ERROR_BACKEND_ENTRY_NOTFOUND ? 0 : dwError;
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaCtxAcquire(&pNewSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirPatchLocalSchemaObjects(pOldSchemaCtx, pNewSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    pBE = VmDirBackendSelect(NULL);
    dwError = pBE->pfnBEApplyIndicesNewMR();
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirSchemaCtxRelease(pOldSchemaCtx);
    VmDirSchemaCtxRelease(pNewSchemaCtx);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

/*
 * If upgrading from 6.5 or lower, use this function to patch schema.
 * Should be called if InitializeSchema() results pbLegacyDataLoaded = TRUE
 *
 * INPUT:
 * new version of Lotus schema file
 */
DWORD
VmDirSchemaPatchLegacyViaFile(
    PCSTR       pszSchemaFilePath
    )
{
    DWORD    dwError = 0;
    PVDIR_BACKEND_INTERFACE pBE = NULL;

    dwError = VmDirSchemaLibLoadFile(pszSchemaFilePath);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirPatchLocalSubSchemaSubEntry();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirWriteSchemaObjects();
    BAIL_ON_VMDIR_ERROR(dwError);

    pBE = VmDirBackendSelect(NULL);
    dwError = pBE->pfnBEApplyIndicesNewMR();
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

static
DWORD
_MarkDefaultIndices(
    VOID
    )
{
    DWORD   dwError = 0;
    PLW_HASHMAP pIndexCfgMap = NULL;
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};

    dwError = VmDirIndexCfgMap(&pIndexCfgMap);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (LwRtlHashMapIterate(pIndexCfgMap, &iter, &pair))
    {
        PVDIR_INDEX_CFG pIndexCfg = (PVDIR_INDEX_CFG)pair.pValue;
        PVDIR_SCHEMA_AT_DESC pATDesc = NULL;

        dwError = VmDirSchemaAttrNameToDescriptor(
                pSchemaCtx, pIndexCfg->pszAttrName, &pATDesc);

        // VMIT support
        if (dwError == VMDIR_ERROR_NO_SUCH_ATTRIBUTE)
        {
            VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                    "%s detected index for unknown attribute %s, "
                    "the index will be deleted",
                    __FUNCTION__, pIndexCfg->pszAttrName, dwError );

            pIndexCfg->status = VDIR_INDEXING_DISABLED;
            continue;
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirIndexCfgGetAllScopesInStrArray(
                pIndexCfg, &pATDesc->ppszUniqueScopes);
        BAIL_ON_VMDIR_ERROR(dwError);

        pATDesc->dwSearchFlags |= 1;

        // for free later
        pATDesc->pLdapAt->ppszUniqueScopes = pATDesc->ppszUniqueScopes;
        pATDesc->pLdapAt->dwSearchFlags = pATDesc->dwSearchFlags;
    }

cleanup:
    VmDirSchemaCtxRelease(pSchemaCtx);
    return dwError;

error:
    goto cleanup;
}
