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

#include "../includes.h"

/*
 * Auxiliary function to tune schema library compatible with legacy data
 */
DWORD
VmDirSchemaLibInitLegacy(
    VOID
    )
{
    DWORD   dwError = 0;
    PVDIR_ENTRY pSchemaEntry = NULL;
    VDIR_SCHEMA_BOOTSTRAP_TABLE ATTable[] =
            VDIR_SCHEMA_BOOTSTRP_ATTR_INITIALIZER;

    // Take attrIdMap if subschema subentry is found because
    // attrIds in subschema subentry should prevail in order
    // to be able to read entries from legacy db
    dwError = VmDirReadSubSchemaSubEntry(&pSchemaEntry);
    if (dwError == 0)
    {
        dwError = VmDirSchemaAttrIdMapLoadSubSchemaSubEntry(
                gVdirSchemaGlobals.pAttrIdMap, pSchemaEntry);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (dwError == ERROR_BACKEND_ENTRY_NOTFOUND)
    {
        dwError = 0;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    // VmDirSchemaLibInit() loads legacy bootstrap table to avoid
    // possible conflict with subschema subentry. After attempt
    // taking attrIdMap from subschema subentry, it is time to
    // load the new bootstrap table
    dwError = VmDirLdapSchemaCopy(
            gVdirSchemaGlobals.pLdapSchema,
            &gVdirSchemaGlobals.pPendingLdapSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaLibLoadBootstrapTable(ATTable);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaLibUpdate(0);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirFreeEntry(pSchemaEntry);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

DWORD
VmDirSchemaLibPrepareUpdateViaSubSchemaSubEntry(
    PVDIR_ENTRY pSchemaEntry
    )
{
    DWORD   dwError = 0;
    PVDIR_LDAP_SCHEMA   pCurLdapSchema = NULL;
    PVDIR_LDAP_SCHEMA   pTmpLdapSchema = NULL;
    PVDIR_LDAP_SCHEMA   pNewLdapSchema = NULL;
    PVDIR_SCHEMA_INSTANCE   pNewVdirSchema = NULL;

    pCurLdapSchema = gVdirSchemaGlobals.pLdapSchema;

    dwError = VmDirLdapSchemaInit(&pTmpLdapSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapSchemaLoadSubSchemaSubEntry(pTmpLdapSchema, pSchemaEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapSchemaMerge(
            pCurLdapSchema, pTmpLdapSchema, &pNewLdapSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaInstanceCreate(pNewLdapSchema, &pNewVdirSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    gVdirSchemaGlobals.pPendingLdapSchema = pNewLdapSchema;
    gVdirSchemaGlobals.pPendingVdirSchema = pNewVdirSchema;

cleanup:
    VmDirFreeLdapSchema(pTmpLdapSchema);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirFreeLdapSchema(pNewLdapSchema);
    VmDirFreeSchemaInstance(pNewVdirSchema);
    goto cleanup;
}
