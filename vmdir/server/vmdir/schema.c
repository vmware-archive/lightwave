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

/*
 * Examines the following options in order and use the first detected source
 * to initialize schema:
 *
 * 1. Individual schema entries (7.0)
 * 2. Subschema subentry (6.x)
 * 3. Schema file
 *
 * OUTPUT:
 * pbWriteSchemaEntry will be TRUE if option 3 was used
 * pbLegacyDataLoaded will be TRUE if option 2 was used
 */
DWORD
InitializeSchema(
    PBOOLEAN    pbWriteSchemaEntry,
    PBOOLEAN    pbLegacyDataLoaded
    )
{
    DWORD               dwError = 0;
    PVDIR_ENTRY_ARRAY   pAtEntries = NULL;
    PVDIR_ENTRY_ARRAY   pOcEntries = NULL;
    PVDIR_ENTRY         pSchemaEntry = NULL;

    assert(pbWriteSchemaEntry && pbLegacyDataLoaded);

    dwError = VmDirSchemaLibInit();
    BAIL_ON_VMDIR_ERROR(dwError);

    // legacy support
    dwError = VmDirSchemaLibInitLegacy();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirReadSchemaObjects(&pAtEntries, &pOcEntries);
    if (dwError == 0)
    {
        dwError = VmDirSchemaLibPrepareUpdateViaEntries(pAtEntries, pOcEntries);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (dwError == ERROR_BACKEND_ENTRY_NOTFOUND)
    {
        dwError = VmDirReadSubSchemaSubEntry(&pSchemaEntry);
        if (dwError == 0)
        {
            dwError = VmDirSchemaLibPrepareUpdateViaSubSchemaSubEntry(pSchemaEntry);
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

            dwError = VmDirSchemaLibPrepareUpdateViaFile(pszSchemaFilePath);
            BAIL_ON_VMDIR_ERROR(dwError);

            *pbWriteSchemaEntry = TRUE;
        }
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaLibUpdate(0);
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
 * During upgrade 7.0 or later, we can patch schema via this function.
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

    dwError = VmDirSchemaCtxAcquire(&pOldSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaLibPrepareUpdateViaFile(pszSchemaFilePath);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaLibUpdate(0);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaCtxAcquire(&pNewSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirPatchLocalSchemaObjects(pOldSchemaCtx, pNewSchemaCtx);
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
 * During upgrade 6.x, we can patch schema via this function.
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

    dwError = VmDirSchemaLibPrepareUpdateViaFile(pszSchemaFilePath);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaLibUpdate(0);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirPatchLocalSubSchemaSubEntry();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirWriteSchemaObjects();
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}
