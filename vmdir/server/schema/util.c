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
VmDirReadAttributeSchemaObjects(
    PVDIR_ENTRY_ARRAY*  ppAtEntries
    )
{
    DWORD   dwError = 0;
    PVDIR_ENTRY_ARRAY   pAtEntries = NULL;

    if (!ppAtEntries)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_ENTRY_ARRAY),
            (PVOID*)&pAtEntries);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSimpleEqualFilterInternalSearch(
            SCHEMA_NAMING_CONTEXT_DN,
            LDAP_SCOPE_SUBTREE,
            ATTR_OBJECT_CLASS,
            OC_ATTRIBUTE_SCHEMA,
            pAtEntries);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pAtEntries->iSize == 0)
    {
        dwError = ERROR_BACKEND_ENTRY_NOTFOUND;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppAtEntries = pAtEntries;

cleanup:
    return dwError;

error:
    VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirFreeEntryArray(pAtEntries);
    goto cleanup;
}

DWORD
VmDirReadClassSchemaObjects(
    PVDIR_ENTRY_ARRAY*  ppOcEntries
    )
{
    DWORD   dwError = 0;
    PVDIR_ENTRY_ARRAY   pOcEntries = NULL;

    if (!ppOcEntries)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_ENTRY_ARRAY),
            (PVOID*)&pOcEntries);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSimpleEqualFilterInternalSearch(
            SCHEMA_NAMING_CONTEXT_DN,
            LDAP_SCOPE_SUBTREE,
            ATTR_OBJECT_CLASS,
            OC_CLASS_SCHEMA,
            pOcEntries);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pOcEntries->iSize == 0)
    {
        dwError = ERROR_BACKEND_ENTRY_NOTFOUND;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppOcEntries = pOcEntries;

cleanup:
    return dwError;

error:
    VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirFreeEntryArray(pOcEntries);
    goto cleanup;
}

DWORD
VmDirWriteSchemaObjects(
    VOID
    )
{
    DWORD   dwError = 0;
    PVDIR_SCHEMA_CTX pSchemaCtx = NULL;

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirPatchLocalSchemaObjects(NULL, pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirSchemaCtxRelease(pSchemaCtx);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}
