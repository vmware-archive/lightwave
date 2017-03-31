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
VmDirSchemaATDescCreate(
    PVDIR_LDAP_ATTRIBUTE_TYPE   pLdapAt,
    PVDIR_SCHEMA_AT_DESC*       ppATDesc
    )
{
    DWORD dwError = 0;
    PVDIR_SCHEMA_AT_DESC pATDesc = NULL;

    if (!pLdapAt || !ppATDesc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_SCHEMA_AT_DESC),
            (PVOID*)&pATDesc);
    BAIL_ON_VMDIR_ERROR(dwError);

    pATDesc->pszName = pLdapAt->pszName;
    pATDesc->pszOid = pLdapAt->pszOid;
    pATDesc->pszSyntaxOid = pLdapAt->pszSyntaxOid;
    pATDesc->bSingleValue = pLdapAt->bSingleValue;
    pATDesc->bCollective = pLdapAt->bCollective;
    pATDesc->bNoUserModifiable = pLdapAt->bNoUserMod;
    pATDesc->bObsolete = pLdapAt->bObsolete;
    pATDesc->usage = pLdapAt->usage;
    pATDesc->dwSearchFlags = pLdapAt->dwSearchFlags;
    pATDesc->ppszUniqueScopes = pLdapAt->ppszUniqueScopes;
    pATDesc->pLdapAt = pLdapAt;

    dwError = VdirSyntaxLookupByOid(pATDesc->pszSyntaxOid, &pATDesc->pSyntax);
    BAIL_ON_VMDIR_ERROR(dwError);

    pATDesc->pEqualityMR =
            VdirEqualityMRLookupBySyntaxOid(pATDesc->pszSyntaxOid);
    pATDesc->pOrderingMR =
            VdirOrderingMRLookupBySyntaxOid(pATDesc->pszSyntaxOid);
    pATDesc->pSubStringMR =
            VdirSubstrMRLookupBySyntaxOid(pATDesc->pszSyntaxOid);

    *ppATDesc = pATDesc;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pATDesc);
    goto cleanup;
}

DWORD
VmDirSchemaOCDescCreate(
    PVDIR_LDAP_OBJECT_CLASS pLdapOc,
    PVDIR_SCHEMA_OC_DESC*   ppOCDesc
    )
{
    DWORD dwError = 0;
    PVDIR_SCHEMA_OC_DESC pOCDesc = NULL;

    if (!pLdapOc || !ppOCDesc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_SCHEMA_OC_DESC),
            (PVOID*)&pOCDesc);
    BAIL_ON_VMDIR_ERROR(dwError);

    pOCDesc->pszName = pLdapOc->pszName;
    pOCDesc->pszOid = pLdapOc->pszOid;
    pOCDesc->pszSup = pLdapOc->pszSup;
    pOCDesc->ppszMustATs = pLdapOc->ppszMust;
    pOCDesc->ppszMayATs = pLdapOc->ppszMay;
    pOCDesc->bObsolete = pLdapOc->bObsolete;
    pOCDesc->type = pLdapOc->type;
    pOCDesc->pLdapOc = pLdapOc;

    *ppOCDesc = pOCDesc;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pOCDesc);
    goto cleanup;
}

DWORD
VmDirSchemaCRDescCreate(
    PVDIR_LDAP_CONTENT_RULE pLdapCr,
    PVDIR_SCHEMA_CR_DESC*   ppCRDesc
    )
{
    DWORD dwError = 0;
    PVDIR_SCHEMA_CR_DESC pCRDesc = NULL;

    if (!pLdapCr || !ppCRDesc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_SCHEMA_CR_DESC),
            (PVOID*)&pCRDesc);
    BAIL_ON_VMDIR_ERROR(dwError);

    pCRDesc->pszName = pLdapCr->pszName;
    pCRDesc->pszOid = pLdapCr->pszOid;
    pCRDesc->ppszMustATs = pLdapCr->ppszMust;
    pCRDesc->ppszMayATs = pLdapCr->ppszMay;
    pCRDesc->ppszNotATs = pLdapCr->ppszNot;
    pCRDesc->ppszAuxOCs = pLdapCr->ppszAux;
    pCRDesc->bObsolete = pLdapCr->bObsolete;
    pCRDesc->pLdapCr = pLdapCr;

    *ppCRDesc = pCRDesc;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pCRDesc);
    goto cleanup;
}

DWORD
VmDirSchemaSRDescCreate(
    PVDIR_LDAP_STRUCTURE_RULE   pLdapSr,
    PVDIR_SCHEMA_SR_DESC*       ppSRDesc
    )
{
    DWORD dwError = 0;
    PVDIR_SCHEMA_SR_DESC pSRDesc = NULL;

    if (!pLdapSr || !ppSRDesc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_SCHEMA_SR_DESC),
            (PVOID*)&pSRDesc);
    BAIL_ON_VMDIR_ERROR(dwError);

    pSRDesc->pszName = pLdapSr->pszName;
    // TODO
    pSRDesc->pLdapSr = pLdapSr;

    *ppSRDesc = pSRDesc;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pSRDesc);
    goto cleanup;
}

DWORD
VmDirSchemaNFDescCreate(
    PVDIR_LDAP_NAME_FORM    pLdapNf,
    PVDIR_SCHEMA_NF_DESC*   ppNFDesc
    )
{
    DWORD dwError = 0;
    PVDIR_SCHEMA_NF_DESC pNFDesc = NULL;

    if (!pLdapNf || !ppNFDesc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_SCHEMA_NF_DESC),
            (PVOID*)&pNFDesc);
    BAIL_ON_VMDIR_ERROR(dwError);

    pNFDesc->pszName = pLdapNf->pszName;
    // TODO
    pNFDesc->pLdapNf = pLdapNf;

    *ppNFDesc = pNFDesc;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pNFDesc);
    goto cleanup;
}

DWORD
VmDirLdapAtParseVdirEntry(
    PVDIR_ENTRY                 pEntry,
    PVDIR_LDAP_ATTRIBUTE_TYPE*  ppAt
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    DWORD   dwVmwAttrUsage = 0;
    DWORD   dwSearchFlags = 0;
    PSTR*   ppszUniqueScopes = NULL;
    PVDIR_ATTRIBUTE     pAttr = NULL;
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

    pAttr = pEntry->attrs;
    while (pAttr)
    {
        if (VmDirStringCompareA(ATTR_IS_SINGLE_VALUED,
                pAttr->type.lberbv_val, FALSE) == 0)
        {
            pSource->at_single_value =
                    VmDirStringCompareA(
                            "TRUE", pAttr->vals[0].lberbv_val, FALSE)
                            == 0;
        }
        else if (VmDirStringCompareA(ATTR_VMW_ATTRIBUTE_USAGE,
                pAttr->type.lberbv_val, FALSE) == 0)
        {
            dwVmwAttrUsage = VmDirStringToIA(pAttr->vals[0].lberbv_val);

            pSource->at_no_user_mod = dwVmwAttrUsage & 0x8 ? 1 : 0;

            pSource->at_usage = 0;
            dwVmwAttrUsage &= 0x7;
            while (dwVmwAttrUsage)
            {
                pSource->at_usage++;
                dwVmwAttrUsage >>= 1;
            }
        }
        else if (VmDirStringCompareA(ATTR_CN,
                pAttr->type.lberbv_val, FALSE) == 0)
        {
            dwError = VmDirAllocateMemory(
                    sizeof(char*) * 2,
                    (PVOID*)&pSource->at_names);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirAllocateStringA(
                    pAttr->vals[0].lberbv_val,
                    &pSource->at_names[0]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(ATTR_ATTRIBUTE_SYNTAX,
                pAttr->type.lberbv_val, FALSE) == 0)
        {
            dwError = VmDirAllocateStringA(
                    pAttr->vals[0].lberbv_val,
                    &pSource->at_syntax_oid);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(ATTR_ATTRIBUTE_ID,
                pAttr->type.lberbv_val, FALSE) == 0)
        {
            dwError = VmDirAllocateStringA(
                    pAttr->vals[0].lberbv_val,
                    &pSource->at_oid);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(ATTR_DESCRIPTION,
                pAttr->type.lberbv_val, FALSE) == 0)
        {
            dwError = VmDirAllocateStringA(
                    pAttr->vals[0].lberbv_val,
                    &pSource->at_desc);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(ATTR_SEARCH_FLAGS,
                pAttr->type.lberbv_val, FALSE) == 0)
        {
            dwSearchFlags = VmDirStringToIA(pAttr->vals[0].lberbv_val);
        }
        else if (VmDirStringCompareA(ATTR_UNIQUENESS_SCOPE,
                pAttr->type.lberbv_val, FALSE) == 0)
        {
            dwError = VmDirAllocateMemory(
                    sizeof(char*)*(pAttr->numVals+1),
                    (PVOID*)&ppszUniqueScopes);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (i = 0; i < pAttr->numVals; i++)
            {
                dwError = VmDirAllocateStringA(
                        pAttr->vals[i].lberbv_val,
                        &ppszUniqueScopes[i]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
        pAttr = pAttr->next;
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
    VmDirFreeLdapAt(pAt);
    goto cleanup;
}

DWORD
VmDirLdapOcParseVdirEntry(
    PVDIR_ENTRY                 pEntry,
    PVDIR_LDAP_OBJECT_CLASS*    ppOc
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PVDIR_ATTRIBUTE     pAttr = NULL;
    LDAPObjectClass*    pSource = NULL;

    if (!pEntry || !ppOc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(LDAPObjectClass), (PVOID*)&pSource);
    BAIL_ON_VMDIR_ERROR(dwError);

    pAttr = pEntry->attrs;
    while (pAttr)
    {
        if (VmDirStringCompareA(ATTR_SUBCLASSOF,
                pAttr->type.lberbv_val, FALSE) == 0)
        {
            dwError = VmDirAllocateMemory(
                    sizeof(char*) * 2,
                    (PVOID*)&pSource->oc_sup_oids);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirAllocateStringA(
                    pAttr->vals[0].lberbv_val,
                    &pSource->oc_sup_oids[0]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(ATTR_CN,
                pAttr->type.lberbv_val, FALSE) == 0)
        {
            dwError = VmDirAllocateMemory(
                    sizeof(char*) * 2,
                    (PVOID*)&pSource->oc_names);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirAllocateStringA(
                    pAttr->vals[0].lberbv_val,
                    &pSource->oc_names[0]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(ATTR_GOVERNSID,
                pAttr->type.lberbv_val, FALSE) == 0)
        {
            dwError = VmDirAllocateStringA(
                    pAttr->vals[0].lberbv_val,
                    &pSource->oc_oid);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(ATTR_DESCRIPTION,
                pAttr->type.lberbv_val, FALSE) == 0)
        {
            dwError = VmDirAllocateStringA(
                    pAttr->vals[0].lberbv_val,
                    &pSource->oc_desc);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(ATTR_OBJECTCLASS_CATEGORY,
                pAttr->type.lberbv_val, FALSE) == 0)
        {
            pSource->oc_kind = VmDirStringToIA(pAttr->vals[0].lberbv_val);
        }
        else if (VmDirStringCompareA(ATTR_SYSTEMMUSTCONTAIN,
                pAttr->type.lberbv_val, FALSE) == 0)
        {
            dwError = VmDirAllocateMemory(
                    sizeof(char*)*(pAttr->numVals+1),
                    (PVOID*)&pSource->oc_at_oids_must);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (i = 0; i < pAttr->numVals; i++)
            {
                dwError = VmDirAllocateStringA(
                        pAttr->vals[i].lberbv_val,
                        &pSource->oc_at_oids_must[i]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
        else if (VmDirStringCompareA(ATTR_SYSTEMMAYCONTAIN,
                pAttr->type.lberbv_val, FALSE) == 0)
        {
            dwError = VmDirAllocateMemory(
                    sizeof(char*)*(pAttr->numVals+1),
                    (PVOID*)&pSource->oc_at_oids_may);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (i = 0; i < pAttr->numVals; i++)
            {
                dwError = VmDirAllocateStringA(
                        pAttr->vals[i].lberbv_val,
                        &pSource->oc_at_oids_may[i]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
        pAttr = pAttr->next;
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
    goto cleanup;
}

DWORD
VmDirLdapCrParseVdirEntry(
    PVDIR_ENTRY                 pEntry,
    PVDIR_LDAP_CONTENT_RULE*    ppCr
    )
{
    BOOLEAN             bHasCr = FALSE;
    DWORD               dwError = 0;
    DWORD               i = 0, dwAux = 0;
    PVDIR_ATTRIBUTE     pAttr = NULL;
    LDAPContentRule*    pSource = NULL;

    if (!pEntry || !ppCr)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(LDAPContentRule), (PVOID*)&pSource);
    BAIL_ON_VMDIR_ERROR(dwError);

    pAttr = pEntry->attrs;
    while (pAttr)
    {
        if (VmDirStringCompareA(ATTR_CN,
                pAttr->type.lberbv_val, FALSE) == 0)
        {
            dwError = VmDirAllocateMemory(
                    sizeof(char*) * 2,
                    (PVOID*)&pSource->cr_names);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirAllocateStringA(
                    pAttr->vals[0].lberbv_val,
                    &pSource->cr_names[0]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(ATTR_GOVERNSID,
                pAttr->type.lberbv_val, FALSE) == 0)
        {
            dwError = VmDirAllocateStringA(
                    pAttr->vals[0].lberbv_val,
                    &pSource->cr_oid);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(ATTR_MUSTCONTAIN,
                pAttr->type.lberbv_val, FALSE) == 0)
        {
            dwError = VmDirAllocateMemory(
                    sizeof(char*)*(pAttr->numVals+1),
                    (PVOID*)&pSource->cr_at_oids_must);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (i = 0; i < pAttr->numVals; i++)
            {
                dwError = VmDirAllocateStringA(
                        pAttr->vals[i].lberbv_val,
                        &pSource->cr_at_oids_must[i]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            bHasCr = TRUE;
        }
        else if (VmDirStringCompareA(ATTR_MAYCONTAIN,
                pAttr->type.lberbv_val, FALSE) == 0)
        {
            dwError = VmDirAllocateMemory(
                    sizeof(char*)*(pAttr->numVals+1),
                    (PVOID*)&pSource->cr_at_oids_may);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (i = 0; i < pAttr->numVals; i++)
            {
                dwError = VmDirAllocateStringA(
                        pAttr->vals[i].lberbv_val,
                        &pSource->cr_at_oids_may[i]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            bHasCr = TRUE;
        }
        else if (VmDirStringCompareA(ATTR_AUXILIARY_CLASS,
                    pAttr->type.lberbv_val, FALSE) == 0 ||
                 VmDirStringCompareA(ATTR_SYSTEMAUXILIARY_CLASS,
                    pAttr->type.lberbv_val, FALSE) == 0)
        {
            dwError = VmDirReallocateMemoryWithInit(
                    (PVOID)pSource->cr_oc_oids_aux,
                    (PVOID*)&pSource->cr_oc_oids_aux,
                    sizeof(char*) * (dwAux + pAttr->numVals + 1),
                    dwAux ? sizeof(char*) * (dwAux + 1) : 0);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (i = 0; i < pAttr->numVals; i++, dwAux++)
            {
                dwError = VmDirAllocateStringA(
                        pAttr->vals[i].lberbv_val,
                        &pSource->cr_oc_oids_aux[dwAux]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            bHasCr = TRUE;
        }
        pAttr = pAttr->next;
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
    if (ppCr)
    {
        *ppCr = NULL;
    }
    goto cleanup;
}
