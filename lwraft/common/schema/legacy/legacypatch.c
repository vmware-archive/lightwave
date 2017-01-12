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
VmDirPatchRemoteSubSchemaSubEntry(
    LDAP*               pLd,
    PVDIR_LDAP_SCHEMA   pNewSchema
    )
{
    DWORD   dwError = 0;
    PVDIR_LEGACY_SCHEMA pLegacySchema = NULL;
    PVDIR_LDAP_SCHEMA   pMergedSchema = NULL;
    PVDIR_LEGACY_SCHEMA_MOD pLegacySchemaMod = NULL;

    if (!pLd || !pNewSchema)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLegacySchemaInit(&pLegacySchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLegacySchemaLoadRemoteSchema(pLegacySchema, pLd);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapSchemaMerge(
            pLegacySchema->pSchema, pNewSchema, &pMergedSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLegacySchemaModInit(&pLegacySchemaMod);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLegacySchemaModPopulate(
            pLegacySchemaMod, pLegacySchema, pMergedSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapModifySubSchemaSubEntry(pLd, pLegacySchemaMod);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirFreeLegacySchema(pLegacySchema);
    VmDirFreeLdapSchema(pMergedSchema);
    VmDirFreeLegacySchemaMod(pLegacySchemaMod);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}
