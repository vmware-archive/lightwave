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
 * 1. Schema subtree (6.6 and higher)
 * 2. Subschema subentry (6.5 and lower) : out dated
 * 3. Schema file
 *
 * OUTPUT:
 * pbWriteSchemaEntry will be TRUE if option 3 was used
 */
DWORD
VmDirLoadSchema(
    PBOOLEAN    pbWriteSchemaEntry
    )
{
    DWORD               dwError = 0;
    PVDIR_ENTRY_ARRAY   pAtEntries = NULL;
    PVDIR_ENTRY_ARRAY   pOcEntries = NULL;
    PVDIR_ENTRY         pSchemaEntry = NULL;

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
        dwError = VmDirReadSubSchemaSubEntry(&pSchemaEntry);
        if (dwError == 0)
        {
            assert(FALSE);  // no longer support single schema entry scheme
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
    }
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirFreeEntryArray(pAtEntries);
    VmDirFreeEntryArray(pOcEntries);
    VmDirFreeEntry(pSchemaEntry);
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
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
VmDirSchemaSetSystemDefaultSecurityDescriptors(
    VOID
    )
{
    DWORD   dwError = 0;
    PSTR    pszBuiltInUsersGroupSid = NULL;
    PSTR    pszDomainClientsGroupSid = NULL;
    PSTR    pszSTSAccountsGroupSid = NULL;
    PSTR    pszDaclTemplate = NULL;

    // create builtin users group and domain clients group SID templates
    dwError = VmDirGenerateWellknownSid(
            NULL, VMDIR_DOMAIN_ALIAS_RID_USERS, &pszBuiltInUsersGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGenerateWellknownSid(
            NULL, VMDIR_DOMAIN_CLIENTS_RID, &pszDomainClientsGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pszSTSAccountsGroupSid, "%s-%u",
            VMDIR_SYSTEM_DOMAIN_NULL_SID_TEMPLATE, VMDIR_DOMAIN_STS_ACCOUNTS_RID);
    BAIL_ON_VMDIR_ERROR(dwError);

    // grant built-in users group rights to read control & property of
    // any group/computer/certificates object
    dwError = VmDirAllocateStringPrintf(
            &pszDaclTemplate,
            "D:(A;;RCRP;;;%s)(A;;RCRP;;;%s)",
            pszBuiltInUsersGroupSid,
            pszSTSAccountsGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSetDefaultSecurityDescriptorForClass(
            OC_GROUP, pszDaclTemplate);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSetDefaultSecurityDescriptorForClass(
            OC_COMPUTER, pszDaclTemplate);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSetDefaultSecurityDescriptorForClass(
            OC_VMW_CERTIFICATION_AUTHORITY, pszDaclTemplate);
    BAIL_ON_VMDIR_ERROR(dwError);

    // grant built-in users group to read control of any user object
    // grant domain clients group to read property of any user object
    VMDIR_SAFE_FREE_MEMORY(pszDaclTemplate);
    dwError = VmDirAllocateStringPrintf(
            &pszDaclTemplate,
            "D:(A;;RC;;;%s)(A;;RP;;;%s)(A;;RP;;;%s)",
            pszBuiltInUsersGroupSid,
            pszDomainClientsGroupSid,
            pszSTSAccountsGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSetDefaultSecurityDescriptorForClass(
            OC_USER, pszDaclTemplate);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszBuiltInUsersGroupSid);
    VMDIR_SAFE_FREE_MEMORY(pszDomainClientsGroupSid);
    VMDIR_SAFE_FREE_MEMORY(pszSTSAccountsGroupSid);
    VMDIR_SAFE_FREE_MEMORY(pszDaclTemplate);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
VmDirReadSubSchemaSubEntry(
    PVDIR_ENTRY*    ppSubSchemaSubEntry
    )
{
    DWORD   dwError = 0;
    PVDIR_BACKEND_INTERFACE pBE = NULL;
    PVDIR_ENTRY pEntry = NULL;

    pBE = VmDirBackendSelect(NULL);
    assert(pBE);

    dwError = VmDirAllocateMemory(sizeof(VDIR_ENTRY), (PVOID*)&pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = pBE->pfnBESimpleIdToEntry(SUB_SCEHMA_SUB_ENTRY_ID, pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppSubSchemaSubEntry = pEntry;

cleanup:
    return dwError;

error:
    // fix EID lookup found but not the same DN. it must be deleted tombstone entry.
    // such as "cn=aggregate#objectGUID:187ac1c1-924d-4b16-a40d-fcea4ab35e7b,cn=Deleted Objects,dc=vsphere,dc=local"
    // in vmdir log file, you will see ERROR: DecodeEntry failed (9602) line. you can ignore this.
    // unfortunately, we would not have exact DN at this layer. thus, rely on error code to determine the case.
    if (dwError == VMDIR_ERROR_BACKEND_ERROR)
    {   // treats as if this entry does not exit.
        dwError = ERROR_BACKEND_ENTRY_NOTFOUND;
    }
    else
    {
        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL,
            "%s subschema subentry not found (%d)", __FUNCTION__, dwError );
    }

    VmDirFreeEntry(pEntry);
    goto cleanup;
}
