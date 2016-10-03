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
VmDirLegacySchemaLoadRemoteSchema(
    PVDIR_LEGACY_SCHEMA pLegacySchema,
    LDAP*               pLd
    )
{
    DWORD           dwError = 0;
    int             i = 0;
    LDAPMessage*    pResult = NULL;
    LDAPMessage*    pEntry = NULL;
    struct berval** ppValues = NULL;

    PSTR    pszDef = NULL;
    PSTR    pszFixedDef = NULL;
    PVDIR_LDAP_ATTRIBUTE_TYPE   pAt = NULL;
    PVDIR_LDAP_OBJECT_CLASS     pOc = NULL;
    PVDIR_LDAP_CONTENT_RULE     pCr = NULL;

    dwError = VmDirLdapSearchSubSchemaSubEntry(pLd, &pResult, &pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    ppValues = ldap_get_values_len(pLd, pEntry, ATTR_ATTRIBUTETYPES);
    dwError = ppValues ? 0 : ERROR_INVALID_DATA;
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < ldap_count_values_len(ppValues); i++)
    {
        dwError = VmDirAllocateStringA(ppValues[i]->bv_val, &pszDef);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirFixLegacySchemaDefSyntaxErr(pszDef, &pszFixedDef);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapAtParseStr(pszFixedDef, &pAt);
        BAIL_ON_VMDIR_ERROR(dwError);
        VMDIR_SAFE_FREE_MEMORY(pszFixedDef);

        dwError = LwRtlHashMapInsert(pLegacySchema->pAtDefStrMap,
                pAt->pszName, pszDef, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
        pszDef = NULL;

        dwError = VmDirLdapSchemaAddAt(pLegacySchema->pSchema, pAt);
        BAIL_ON_VMDIR_ERROR(dwError);
        pAt = NULL;
    }
    ldap_value_free_len(ppValues);
    ppValues = NULL;

    ppValues = ldap_get_values_len(pLd, pEntry, ATTR_OBJECTCLASSES);
    dwError = ppValues ? 0 : ERROR_INVALID_DATA;
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < ldap_count_values_len(ppValues); i++)
    {
        dwError = VmDirAllocateStringA(ppValues[i]->bv_val, &pszDef);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirFixLegacySchemaDefSyntaxErr(pszDef, &pszFixedDef);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapOcParseStr(pszFixedDef, &pOc);
        BAIL_ON_VMDIR_ERROR(dwError);
        VMDIR_SAFE_FREE_MEMORY(pszFixedDef);

        dwError = LwRtlHashMapInsert(pLegacySchema->pOcDefStrMap,
                pOc->pszName, pszDef, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
        pszDef = NULL;

        dwError = VmDirLdapSchemaAddOc(pLegacySchema->pSchema, pOc);
        BAIL_ON_VMDIR_ERROR(dwError);
        pOc = NULL;
    }
    ldap_value_free_len(ppValues);
    ppValues = NULL;

    ppValues = ldap_get_values_len(pLd, pEntry, ATTR_DITCONTENTRULES);
    dwError = ppValues ? 0 : ERROR_INVALID_DATA;
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < ldap_count_values_len(ppValues); i++)
    {
        dwError = VmDirAllocateStringA(ppValues[i]->bv_val, &pszDef);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirFixLegacySchemaDefSyntaxErr(pszDef, &pszFixedDef);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapCrParseStr(pszFixedDef, &pCr);
        BAIL_ON_VMDIR_ERROR(dwError);
        VMDIR_SAFE_FREE_MEMORY(pszFixedDef);

        dwError = LwRtlHashMapInsert(pLegacySchema->pCrDefStrMap,
                pCr->pszName, pszDef, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
        pszDef = NULL;

        dwError = VmDirLdapSchemaAddCr(pLegacySchema->pSchema, pCr);
        BAIL_ON_VMDIR_ERROR(dwError);
        pCr = NULL;
    }
    ldap_value_free_len(ppValues);
    ppValues = NULL;

    dwError = VmDirLdapSchemaResolveAndVerifyAll(pLegacySchema->pSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pResult)
    {
        ldap_msgfree(pResult);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    if (ppValues)
    {
        ldap_value_free_len(ppValues);
        ppValues = NULL;
    }
    VmDirFreeLdapAt(pAt);
    VmDirFreeLdapOc(pOc);
    VmDirFreeLdapCr(pCr);
    VMDIR_SAFE_FREE_MEMORY(pszDef);
    VMDIR_SAFE_FREE_MEMORY(pszFixedDef);
    goto cleanup;
}
