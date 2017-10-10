/*
 * Copyright © 2012-2017 VMware, Inc.  All Rights Reserved.
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
 * 1. Schema entries
 * 2. Schema file
 *
 * OUTPUT:
 * pbWriteSchemaEntry will be TRUE if option 2 was used
 */
DWORD
VmDirLoadSchema(
    PBOOLEAN    pbWriteSchemaEntry
    )
{
    DWORD   dwError = 0;
    PVDIR_ENTRY_ARRAY   pAtEntries = NULL;
    PVDIR_ENTRY_ARRAY   pOcEntries = NULL;

    assert(pbWriteSchemaEntry);

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
        PCSTR pszSchemaFilePath = gVmdirGlobals.pszBootStrapSchemaFile;
        if (IsNullOrEmptyString(pszSchemaFilePath))
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NO_SCHEMA);
        }

        dwError = VmDirSchemaLibLoadFile(pszSchemaFilePath);
        BAIL_ON_VMDIR_ERROR(dwError);

        *pbWriteSchemaEntry = TRUE;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirFreeEntryArray(pAtEntries);
    VmDirFreeEntryArray(pOcEntries);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

/*
 * Initialize schemacontext subtree entries
 * Should be called if VmDirLoadSchema() results pbWriteSchemaEntry = TRUE
 */
DWORD
VmDirSchemaInitializeSubtree(
    PVDIR_SCHEMA_CTX    pSchemaCtx
    )
{
    DWORD   dwError = 0;

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
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);
    goto cleanup;
}
