/*
 * Copyright © 2016 VMware, Inc.  All Rights Reserved.
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
VmDirLdapSearchSubSchemaSubEntry(
    LDAP*           pLd,
    LDAPMessage**   ppResult,
    LDAPMessage**   ppEntry
    )
{
    static PSTR ppszSubSchemaSubEntryAttrs[] =
    {
        ATTR_ATTRIBUTETYPES,
        ATTR_OBJECTCLASSES,
        ATTR_DITCONTENTRULES,
        ATTR_LDAPSYNTAXES,
        NULL
    };

    DWORD           dwError = 0;
    PCSTR           pcszFilter = "(objectclass=*)";
    LDAPMessage*    pResult = NULL;
    LDAPMessage*    pEntry = NULL;

    if (!ppResult)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = ldap_search_ext_s(
                pLd,
                SUB_SCHEMA_SUB_ENTRY_DN,
                LDAP_SCOPE_BASE,
                pcszFilter,
                ppszSubSchemaSubEntryAttrs,
                FALSE, /* get values      */
                NULL,  /* server controls */
                NULL,  /* client controls */
                NULL,  /* timeout         */
                0,     /* size limit      */
                &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (ldap_count_entries(pLd, pResult) != 1)
    {
        dwError = VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pEntry = ldap_first_entry(pLd, pResult);

    *ppResult = pResult;
    *ppEntry = pEntry;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    if (pResult)
    {
        ldap_msgfree(pResult);
    }
    goto cleanup;
}

DWORD
VmDirFixLegacySchemaDefSyntaxErr(
    PSTR    pszDef,
    PSTR*   ppszFixedDef
    )
{
    static PCSTR    ppcszDefFixes[] =
    {
        "( VMWare.LKUP.attribute.27 NAME vmwLKUPLegacyIds DESC 'VMware Lookup Service - service identifier' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{256} )",
        "( VMWare.LKUP.attribute.27 NAME 'vmwLKUPLegacyIds' DESC 'VMware Lookup Service - service identifier' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{256} )",
        "( VMWare.STS.objectclass.25 NAME 'vmwExternalIdpUser' DESC 'VMWare external idp user' AUXILIARY MUST ( vmwSTSEntityId vmwSTSExternalIdpUserId ) )",
        "( VMWare.STS.objectclass.25 NAME 'vmwExternalIdpUser' DESC 'VMWare external idp user' AUXILIARY MUST ( vmwSTSEntityId $ vmwSTSExternalIdpUserId ) )",
        NULL
    };

    DWORD   dwError = 0;
    DWORD   i = 0;
    PSTR    pszFixedDef = NULL;

    for (i = 0; ppcszDefFixes[i]; i += 2)
    {
        if (VmDirStringCompareA(pszDef, ppcszDefFixes[i], FALSE) == 0)
        {
            dwError = VmDirAllocateStringA(ppcszDefFixes[i+1], &pszFixedDef);
            BAIL_ON_VMDIR_ERROR(dwError);
            break;
        }
    }

    if (!pszFixedDef)
    {
        dwError = VmDirAllocateStringA(pszDef, &pszFixedDef);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppszFixedDef = pszFixedDef;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszFixedDef);
    goto cleanup;
}

BOOLEAN
VmDirIsMultiNameAttribute(
    PSTR    pszName
    )
{
    static PCSTR    ppcszMultiNameAttrs[] =
    {
        "emailAddress",
        "email",
        "pkcs9email",
        "aliasedEntryName",
        "aliasedObjectName",
        NULL
    };

    DWORD   i = 0;

    assert(pszName);
    for (i = 0; ppcszMultiNameAttrs[i]; i++)
    {
        if (VmDirStringCompareA(pszName, ppcszMultiNameAttrs[i], FALSE) == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

DWORD
VmDirLdapModifySubSchemaSubEntry(
    LDAP*                   pLd,
    PVDIR_LEGACY_SCHEMA_MOD pLegacySchemaMod
    )
{
    DWORD   dwError = 0;
    int     i = 0;
    LDAPMod*    mods[7] = {0};
    LDAPMod     modAtDel = {0};
    LDAPMod     modAtAdd = {0};
    LDAPMod     modOcDel = {0};
    LDAPMod     modOcAdd = {0};
    LDAPMod     modCrDel = {0};
    LDAPMod     modCrAdd = {0};

    if (pLegacySchemaMod->pDelAt->dwCount > 0)
    {
        modAtDel.mod_op = LDAP_MOD_DELETE;
        modAtDel.mod_type = (PSTR)ATTR_ATTRIBUTETYPES;
        modAtDel.mod_vals.modv_strvals =
                (PSTR*)pLegacySchemaMod->pDelAt->pStringList;
        mods[i++] = &modAtDel;
    }

    if (pLegacySchemaMod->pAddAt->dwCount > 0)
    {
        modAtAdd.mod_op = LDAP_MOD_ADD;
        modAtAdd.mod_type = (PSTR)ATTR_ATTRIBUTETYPES;
        modAtAdd.mod_vals.modv_strvals =
                (PSTR*)pLegacySchemaMod->pAddAt->pStringList;
        mods[i++] = &modAtAdd;
    }

    if (pLegacySchemaMod->pDelOc->dwCount > 0)
    {
        modOcDel.mod_op = LDAP_MOD_DELETE;
        modOcDel.mod_type = (PSTR)ATTR_OBJECTCLASSES;
        modOcDel.mod_vals.modv_strvals =
                (PSTR*)pLegacySchemaMod->pDelOc->pStringList;
        mods[i++] = &modOcDel;
    }

    if (pLegacySchemaMod->pAddOc->dwCount > 0)
    {
        modOcAdd.mod_op = LDAP_MOD_ADD;
        modOcAdd.mod_type = (PSTR)ATTR_OBJECTCLASSES;
        modOcAdd.mod_vals.modv_strvals =
                (PSTR*)pLegacySchemaMod->pAddOc->pStringList;
        mods[i++] = &modOcAdd;
    }

    if (pLegacySchemaMod->pDelCr->dwCount > 0)
    {
        modCrDel.mod_op = LDAP_MOD_DELETE;
        modCrDel.mod_type = (PSTR)ATTR_DITCONTENTRULES;
        modCrDel.mod_vals.modv_strvals =
                (PSTR*)pLegacySchemaMod->pDelCr->pStringList;
        mods[i++] = &modCrDel;
    }

    if (pLegacySchemaMod->pAddCr->dwCount > 0)
    {
        modCrAdd.mod_op = LDAP_MOD_ADD;
        modCrAdd.mod_type = (PSTR)ATTR_DITCONTENTRULES;
        modCrAdd.mod_vals.modv_strvals =
                (PSTR*)pLegacySchemaMod->pAddCr->pStringList;
        mods[i++] = &modCrAdd;
    }

    if (i > 0)
    {
        dwError = ldap_modify_ext_s(
                pLd,
                SUB_SCHEMA_SUB_ENTRY_DN,
                mods,
                NULL,
                NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL,
                    "%s Updated 6.x partner schema",
                    __FUNCTION__ );
    }
    else
    {
        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL,
                    "%s 6.x partner schema is up-to-date",
                    __FUNCTION__ );
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}
