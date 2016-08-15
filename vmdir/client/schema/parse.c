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
VmDirLdapAtParseLDAPEntry(
    LDAP*                       pLd,
    LDAPMessage*                pEntry,
    PVDIR_LDAP_ATTRIBUTE_TYPE*  ppAt
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    DWORD   dwVmwAttrUsage = 0;
    DWORD   dwSearchFlags = 0;
    PSTR*   ppszUniqueScopes = NULL;
    BerValue**  ppBerVals = NULL;
    LDAPAttributeType*  pSource = NULL;
    PVDIR_LDAP_ATTRIBUTE_TYPE   pAt = NULL;

    if (!pEntry || !ppAt)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(LDAPAttributeType), (PVOID*)&pSource);
    BAIL_ON_VMDIR_ERROR(dwError);

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_IS_SINGLE_VALUED);
    if (ppBerVals && ppBerVals[0])
    {
        pSource->at_single_value =
                VmDirStringCompareA(
                        "TRUE", ppBerVals[0]->bv_val, FALSE) == 0;

        ldap_value_free_len(ppBerVals);
        ppBerVals = NULL;
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_VMW_ATTRIBUTE_USAGE);
    if (ppBerVals && ppBerVals[0])
    {
        dwVmwAttrUsage = VmDirStringToIA(ppBerVals[0]->bv_val);

        pSource->at_no_user_mod = dwVmwAttrUsage & 0x8 ? 1 : 0;

        pSource->at_usage = 0;
        dwVmwAttrUsage &= 0x7;
        while (dwVmwAttrUsage)
        {
            pSource->at_usage++;
            dwVmwAttrUsage >>= 1;
        }

        ldap_value_free_len(ppBerVals);
        ppBerVals = NULL;
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_CN);
    if (ppBerVals && ppBerVals[0])
    {
        dwError = VmDirAllocateMemory(
                sizeof(char*) * 2, (PVOID*)&pSource->at_names);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(
                ppBerVals[0]->bv_val, &pSource->at_names[0]);
        BAIL_ON_VMDIR_ERROR(dwError);

        ldap_value_free_len(ppBerVals);
        ppBerVals = NULL;
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_ATTRIBUTE_SYNTAX);
    if (ppBerVals && ppBerVals[0])
    {
        dwError = VmDirAllocateStringA(
                ppBerVals[0]->bv_val, &pSource->at_syntax_oid);
        BAIL_ON_VMDIR_ERROR(dwError);

        ldap_value_free_len(ppBerVals);
        ppBerVals = NULL;
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_ATTRIBUTE_ID);
    if (ppBerVals && ppBerVals[0])
    {
        dwError = VmDirAllocateStringA(
                ppBerVals[0]->bv_val, &pSource->at_oid);
        BAIL_ON_VMDIR_ERROR(dwError);

        ldap_value_free_len(ppBerVals);
        ppBerVals = NULL;
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_DESCRIPTION);
    if (ppBerVals && ppBerVals[0])
    {
        dwError = VmDirAllocateStringA(
                ppBerVals[0]->bv_val, &pSource->at_desc);
        BAIL_ON_VMDIR_ERROR(dwError);

        ldap_value_free_len(ppBerVals);
        ppBerVals = NULL;
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_SEARCH_FLAGS);
    if (ppBerVals && ppBerVals[0])
    {
        dwSearchFlags = VmDirStringToIA(ppBerVals[0]->bv_val);

        ldap_value_free_len(ppBerVals);
        ppBerVals = NULL;
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_UNIQUENESS_SCOPE);
    if (ppBerVals)
    {
        DWORD dwNumVals = ldap_count_values_len(ppBerVals);
        if (dwNumVals > 0)
        {
            dwError = VmDirAllocateMemory(
                    sizeof(char*) * (dwNumVals + 1),
                    (PVOID*)&ppszUniqueScopes);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (i = 0; i < dwNumVals; i++)
            {
                dwError = VmDirAllocateStringA(
                        ppBerVals[i]->bv_val, &ppszUniqueScopes[i]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }

        ldap_value_free_len(ppBerVals);
        ppBerVals = NULL;
    }

    dwError = VmDirLdapAtCreate(pSource, &pAt);
    BAIL_ON_VMDIR_ERROR(dwError);

    pAt->dwSearchFlags = dwSearchFlags;
    pAt->ppszUniqueScopes = ppszUniqueScopes;

    *ppAt = pAt;

cleanup:
    return dwError;

error:
    if (pSource)
    {
        ldap_attributetype_free(pSource);
    }
    if (ppBerVals)
    {
        ldap_value_free_len(ppBerVals);
    }
    VmDirFreeLdapAt(pAt);
    goto cleanup;
}

DWORD
VmDirLdapOcParseLDAPEntry(
    LDAP*                       pLd,
    LDAPMessage*                pEntry,
    PVDIR_LDAP_OBJECT_CLASS*    ppOc
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    BerValue**  ppBerVals = NULL;
    LDAPObjectClass*    pSource = NULL;

    if (!pEntry || !ppOc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(LDAPObjectClass), (PVOID*)&pSource);
    BAIL_ON_VMDIR_ERROR(dwError);

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_SUBCLASSOF);
    if (ppBerVals && ppBerVals[0])
    {
        dwError = VmDirAllocateMemory(
                sizeof(char*) * 2, (PVOID*)&pSource->oc_sup_oids);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(
                ppBerVals[0]->bv_val, &pSource->oc_sup_oids[0]);
        BAIL_ON_VMDIR_ERROR(dwError);

        ldap_value_free_len(ppBerVals);
        ppBerVals = NULL;
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_CN);
    if (ppBerVals && ppBerVals[0])
    {
        dwError = VmDirAllocateMemory(
                sizeof(char*) * 2, (PVOID*)&pSource->oc_names);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(
                ppBerVals[0]->bv_val, &pSource->oc_names[0]);
        BAIL_ON_VMDIR_ERROR(dwError);

        ldap_value_free_len(ppBerVals);
        ppBerVals = NULL;
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_GOVERNSID);
    if (ppBerVals && ppBerVals[0])
    {
        dwError = VmDirAllocateStringA(
                ppBerVals[0]->bv_val, &pSource->oc_oid);
        BAIL_ON_VMDIR_ERROR(dwError);

        ldap_value_free_len(ppBerVals);
        ppBerVals = NULL;
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_DESCRIPTION);
    if (ppBerVals && ppBerVals[0])
    {
        dwError = VmDirAllocateStringA(
                ppBerVals[0]->bv_val, &pSource->oc_desc);
        BAIL_ON_VMDIR_ERROR(dwError);

        ldap_value_free_len(ppBerVals);
        ppBerVals = NULL;
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_OBJECTCLASS_CATEGORY);
    if (ppBerVals && ppBerVals[0])
    {
        pSource->oc_kind = VmDirStringToIA(ppBerVals[0]->bv_val);

        ldap_value_free_len(ppBerVals);
        ppBerVals = NULL;
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_SYSTEMMUSTCONTAIN);
    if (ppBerVals)
    {
        DWORD dwNumVals = ldap_count_values_len(ppBerVals);
        if (dwNumVals > 0)
        {
            dwError = VmDirAllocateMemory(
                    sizeof(char*) * (dwNumVals + 1),
                    (PVOID*)&pSource->oc_at_oids_must);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (i = 0; i < dwNumVals; i++)
            {
                dwError = VmDirAllocateStringA(
                        ppBerVals[i]->bv_val, &pSource->oc_at_oids_must[i]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }

        ldap_value_free_len(ppBerVals);
        ppBerVals = NULL;
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_SYSTEMMAYCONTAIN);
    if (ppBerVals)
    {
        DWORD dwNumVals = ldap_count_values_len(ppBerVals);
        if (dwNumVals > 0)
        {
            dwError = VmDirAllocateMemory(
                    sizeof(char*) * (dwNumVals + 1),
                    (PVOID*)&pSource->oc_at_oids_may);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (i = 0; i < dwNumVals; i++)
            {
                dwError = VmDirAllocateStringA(
                        ppBerVals[i]->bv_val, &pSource->oc_at_oids_may[i]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }

        ldap_value_free_len(ppBerVals);
        ppBerVals = NULL;
    }

    dwError = VmDirLdapOcCreate(pSource, ppOc);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    if (pSource)
    {
        ldap_objectclass_free(pSource);
    }
    if (ppBerVals)
    {
        ldap_value_free_len(ppBerVals);
    }
    goto cleanup;
}

DWORD
VmDirLdapCrParseLDAPEntry(
    LDAP*                       pLd,
    LDAPMessage*                pEntry,
    PVDIR_LDAP_CONTENT_RULE*    ppCr
    )
{
    BOOLEAN bHasCr = FALSE;
    DWORD   dwError = 0;
    DWORD   i = 0, dwAux = 0;
    BerValue**  ppBerVals = NULL;
    LDAPContentRule*    pSource = NULL;

    if (!pEntry || !ppCr)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(LDAPContentRule), (PVOID*)&pSource);
    BAIL_ON_VMDIR_ERROR(dwError);

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_CN);
    if (ppBerVals && ppBerVals[0])
    {
        dwError = VmDirAllocateMemory(
                sizeof(char*) * 2, (PVOID*)&pSource->cr_names);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(
                ppBerVals[0]->bv_val, &pSource->cr_names[0]);
        BAIL_ON_VMDIR_ERROR(dwError);

        ldap_value_free_len(ppBerVals);
        ppBerVals = NULL;
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_GOVERNSID);
    if (ppBerVals && ppBerVals[0])
    {
        dwError = VmDirAllocateStringA(
                ppBerVals[0]->bv_val, &pSource->cr_oid);
        BAIL_ON_VMDIR_ERROR(dwError);

        ldap_value_free_len(ppBerVals);
        ppBerVals = NULL;
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_MUSTCONTAIN);
    if (ppBerVals)
    {
        DWORD dwNumVals = ldap_count_values_len(ppBerVals);
        if (dwNumVals > 0)
        {
            dwError = VmDirAllocateMemory(
                    sizeof(char*) * (dwNumVals + 1),
                    (PVOID*)&pSource->cr_at_oids_must);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (i = 0; i < dwNumVals; i++)
            {
                dwError = VmDirAllocateStringA(
                        ppBerVals[i]->bv_val, &pSource->cr_at_oids_must[i]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            bHasCr = TRUE;
        }

        ldap_value_free_len(ppBerVals);
        ppBerVals = NULL;
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_MAYCONTAIN);
    if (ppBerVals)
    {
        DWORD dwNumVals = ldap_count_values_len(ppBerVals);
        if (dwNumVals > 0)
        {
            dwError = VmDirAllocateMemory(
                    sizeof(char*) * (dwNumVals + 1),
                    (PVOID*)&pSource->cr_at_oids_may);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (i = 0; i < dwNumVals; i++)
            {
                dwError = VmDirAllocateStringA(
                        ppBerVals[i]->bv_val, &pSource->cr_at_oids_may[i]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            bHasCr = TRUE;
        }

        ldap_value_free_len(ppBerVals);
        ppBerVals = NULL;
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_AUXILIARY_CLASS);
    if (ppBerVals)
    {
        DWORD dwNumVals = ldap_count_values_len(ppBerVals);
        if (dwNumVals > 0)
        {
            dwError = VmDirAllocateMemory(
                    sizeof(char*) * (dwNumVals + 1),
                    (PVOID*)&pSource->cr_oc_oids_aux);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (i = 0; i < dwNumVals; i++)
            {
                dwError = VmDirAllocateStringA(
                        ppBerVals[i]->bv_val, &pSource->cr_oc_oids_aux[i]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            dwAux = dwNumVals;
            bHasCr = TRUE;
        }

        ldap_value_free_len(ppBerVals);
        ppBerVals = NULL;
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_SYSTEMAUXILIARY_CLASS);
    if (ppBerVals)
    {
        DWORD dwNumVals = ldap_count_values_len(ppBerVals);
        if (dwNumVals > 0)
        {
            dwError = VmDirReallocateMemoryWithInit(
                    (PVOID)pSource->cr_oc_oids_aux,
                    (PVOID*)&pSource->cr_oc_oids_aux,
                    sizeof(char*) * (dwAux + dwNumVals + 1),
                    dwAux ? sizeof(char*) * (dwAux + 1) : 0);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (i = 0; i < dwNumVals; i++, dwAux++)
            {
                dwError = VmDirAllocateStringA(
                        ppBerVals[i]->bv_val, &pSource->cr_oc_oids_aux[dwAux]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            bHasCr = TRUE;
        }

        ldap_value_free_len(ppBerVals);
        ppBerVals = NULL;
    }

    if (!bHasCr)
    {
        goto error;
    }

    dwError = VmDirLdapCrCreate(pSource, ppCr);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    if (pSource)
    {
        ldap_contentrule_free(pSource);
    }
    if (ppBerVals)
    {
        ldap_value_free_len(ppBerVals);
    }
    if (ppCr)
    {
        *ppCr = NULL;
    }
    goto cleanup;
}
