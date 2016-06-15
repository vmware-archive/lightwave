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
VmDirLdapAtCreate(
    LDAPAttributeType*          pSource,
    PVDIR_LDAP_ATTRIBUTE_TYPE*  ppAt
    )
{
    DWORD   dwError = 0;
    PVDIR_LDAP_DEFINITION   pDef = NULL;
    PVDIR_LDAP_ATTRIBUTE_TYPE   pAt = NULL;

    if (!pSource || !ppAt)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_LDAP_DEFINITION), (PVOID*)&pDef);
    BAIL_ON_VMDIR_ERROR(dwError);

    pAt = &pDef->data.at;
    pAt->pszName = pSource->at_names[0];
    pAt->pszOid = pSource->at_oid;
    pAt->pszDesc = pSource->at_desc;
    pAt->ppszAliases = pSource->at_names;
    pAt->pszSyntaxOid = pSource->at_syntax_oid;
    pAt->bSingleValue = pSource->at_single_value;
    pAt->bCollective = pSource->at_collective;
    pAt->bNoUserMod = pSource->at_no_user_mod;
    pAt->bObsolete = pSource->at_obsolete;
    pAt->usage = pSource->at_usage;
    pAt->pSource = pSource;
    pDef->type = VDIR_LDAP_DEFINITION_TYPE_AT;
    pDef->pszName = pAt->pszName;
    *ppAt = pAt;

cleanup:
    return dwError;

error:
    VmDirFreeLdapAt(pAt);
    goto cleanup;
}

DWORD
VmDirLdapOcCreate(
    LDAPObjectClass*            pSource,
    PVDIR_LDAP_OBJECT_CLASS*    ppOc
    )
{
    DWORD   dwError = 0;
    PVDIR_LDAP_DEFINITION   pDef = NULL;
    PVDIR_LDAP_OBJECT_CLASS pOc = NULL;

    if (!pSource || !ppOc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_LDAP_DEFINITION), (PVOID*)&pDef);
    BAIL_ON_VMDIR_ERROR(dwError);

    pOc = &pDef->data.oc;
    pOc->pszName = pSource->oc_names[0];
    pOc->pszOid = pSource->oc_oid;
    pOc->pszDesc = pSource->oc_desc;
    pOc->pszSup = pSource->oc_sup_oids ? pSource->oc_sup_oids[0] : NULL;
    pOc->ppszMust = pSource->oc_at_oids_must;
    pOc->ppszMay = pSource->oc_at_oids_may;
    pOc->bObsolete = pSource->oc_obsolete;
    pOc->type = pSource->oc_kind;
    pOc->pSource = pSource;
    pDef->type = VDIR_LDAP_DEFINITION_TYPE_OC;
    pDef->pszName = pOc->pszName;
    *ppOc = pOc;

cleanup:
    return dwError;

error:
    VmDirFreeLdapOc(pOc);
    goto cleanup;
}

DWORD
VmDirLdapCrCreate(
    LDAPContentRule*            pSource,
    PVDIR_LDAP_CONTENT_RULE*    ppCr
    )
{
    DWORD   dwError = 0;
    PVDIR_LDAP_DEFINITION   pDef = NULL;
    PVDIR_LDAP_CONTENT_RULE pCr = NULL;

    if (!pSource || !ppCr)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_LDAP_DEFINITION), (PVOID*)&pDef);
    BAIL_ON_VMDIR_ERROR(dwError);

    pCr = &pDef->data.cr;
    pCr->pszName = pSource->cr_names[0];
    pCr->pszOid = pSource->cr_oid;
    pCr->ppszMust = pSource->cr_at_oids_must;
    pCr->ppszMay = pSource->cr_at_oids_may;
    pCr->ppszNot = pSource->cr_at_oids_not;
    pCr->ppszAux = pSource->cr_oc_oids_aux;
    pCr->bObsolete = pSource->cr_obsolete;
    pCr->pSource = pSource;
    pDef->type = VDIR_LDAP_DEFINITION_TYPE_CR;
    pDef->pszName = pCr->pszName;
    *ppCr = pCr;

cleanup:
    return dwError;

error:
    VmDirFreeLdapCr(pCr);
    goto cleanup;
}

DWORD
VmDirLdapSrCreate(
    LDAPStructureRule*          pSource,
    PVDIR_LDAP_STRUCTURE_RULE*  ppSr
    )
{
    DWORD   dwError = 0;
    PVDIR_LDAP_DEFINITION   pDef = NULL;
    PVDIR_LDAP_STRUCTURE_RULE   pSr = NULL;

    if (!pSource || !ppSr)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_LDAP_DEFINITION), (PVOID*)&pDef);
    BAIL_ON_VMDIR_ERROR(dwError);

    pSr = &pDef->data.sr;
    pSr->pszName = pSource->sr_names ? pSource->sr_names[0] : NULL;
    // TODO
    pSr->pSource = pSource;
    pDef->type = VDIR_LDAP_DEFINITION_TYPE_SR;
    pDef->pszName = pSr->pszName;
    *ppSr = pSr;

cleanup:
    return dwError;

error:
    VmDirFreeLdapSr(pSr);
    goto cleanup;
}

DWORD
VmDirLdapNfCreate(
    LDAPNameForm*           pSource,
    PVDIR_LDAP_NAME_FORM*   ppNf
    )
{
    DWORD   dwError = 0;
    PVDIR_LDAP_DEFINITION   pDef = NULL;
    PVDIR_LDAP_NAME_FORM    pNf = NULL;

    if (!pSource || !ppNf)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_LDAP_DEFINITION), (PVOID*)&pDef);
    BAIL_ON_VMDIR_ERROR(dwError);

    pNf = &pDef->data.nf;
    pNf->pszName = pSource->nf_names ? pSource->nf_names[0] : NULL;
    // TODO
    pNf->pSource = pSource;
    pDef->type = VDIR_LDAP_DEFINITION_TYPE_NF;
    pDef->pszName = pNf->pszName;
    *ppNf = pNf;

cleanup:
    return dwError;

error:
    VmDirFreeLdapNf(pNf);
    goto cleanup;
}

DWORD
VmDirLdapAtParseStr(
    PCSTR                       pcszStr,
    PVDIR_LDAP_ATTRIBUTE_TYPE*  ppAt
    )
{
    DWORD   dwError = 0;
    int     iCode = 0;
    const char* pErr = NULL;
    const int   flags = LDAP_SCHEMA_ALLOW_ALL;
    LDAPAttributeType*  pSource = NULL;

    if (!pcszStr || !ppAt)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (IS_ATTRIBUTETYPES_TAG(pcszStr))
    {
        pcszStr += ATTRIBUTETYPS_TAG_LEN;
        while (isspace(*pcszStr)) pcszStr++;
    }

    pSource = ldap_str2attributetype(pcszStr, &iCode, &pErr, flags);

    if (!pSource)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s: ldap_str2attributetype failed (code:%d) (err:%s) %s",
                __FUNCTION__, iCode, ldap_scherr2str(iCode), pErr);
        dwError = ERROR_INVALID_SCHEMA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLdapAtCreate(pSource, ppAt);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    if (pSource)
    {
        ldap_attributetype_free(pSource);
    }
    goto cleanup;
}

DWORD
VmDirLdapOcParseStr(
    PCSTR                       pcszStr,
    PVDIR_LDAP_OBJECT_CLASS*    ppOc
    )
{
    DWORD   dwError = 0;
    int     iCode = 0;
    const char* pErr = NULL;
    const int   flags = LDAP_SCHEMA_ALLOW_ALL;
    LDAPObjectClass*    pSource = NULL;

    if (!pcszStr || !ppOc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (IS_OBJECTCLASSES_TAG(pcszStr))
    {
        pcszStr += OBJECTCLASSES_TAG_LEN;
        while (isspace(*pcszStr)) pcszStr++;
    }

    pSource = ldap_str2objectclass(pcszStr, &iCode, &pErr, flags);

    if (!pSource)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s: ldap_str2objectclass failed (code:%d) (err:%s) %s",
                __FUNCTION__, iCode, ldap_scherr2str(iCode), pErr);
        dwError = ERROR_INVALID_SCHEMA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pSource->oc_kind = gVdirOpenLdapToADClassType[pSource->oc_kind];

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
VmDirLdapCrParseStr(
    PCSTR                       pcszStr,
    PVDIR_LDAP_CONTENT_RULE*    ppCr
    )
{
    DWORD   dwError = 0;
    int     iCode = 0;
    const char* pErr = NULL;
    const int   flags = LDAP_SCHEMA_ALLOW_ALL;
    LDAPContentRule*    pSource = NULL;

    if (!pcszStr || !ppCr)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (IS_CONTENTRULES_TAG(pcszStr))
    {
        pcszStr += CONTENTRULES_TAG_LEN;
        while (isspace(*pcszStr)) pcszStr++;
    }

    pSource = ldap_str2contentrule(pcszStr, &iCode, &pErr, flags);

    if (!pSource)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s: ldap_str2contentrule failed (code:%d) (err:%s) %s",
                __FUNCTION__, iCode, ldap_scherr2str(iCode), pErr);
        dwError = ERROR_INVALID_SCHEMA;
        BAIL_ON_VMDIR_ERROR(dwError);
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
    goto cleanup;
}

DWORD
VmDirLdapSrParseStr(
    PCSTR                       pcszStr,
    PVDIR_LDAP_STRUCTURE_RULE*  ppSr
    )
{
    DWORD   dwError = 0;
    int     iCode = 0;
    const char* pErr = NULL;
    const int   flags = LDAP_SCHEMA_ALLOW_ALL;
    LDAPStructureRule*  pSource = NULL;

    if (!pcszStr || !ppSr)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (IS_STRUCTURERULES_TAG(pcszStr))
    {
        pcszStr += STRUCTURERULES_TAG_LEN;
        while (isspace(*pcszStr)) pcszStr++;
    }

    pSource = ldap_str2structurerule(pcszStr, &iCode, &pErr, flags);

    if (!pSource)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s: ldap_str2structurerule failed (code:%d) (err:%s) %s",
                __FUNCTION__, iCode, ldap_scherr2str(iCode), pErr);
        dwError = ERROR_INVALID_SCHEMA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLdapSrCreate(pSource, ppSr);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    if (pSource)
    {
        ldap_structurerule_free(pSource);
    }
    goto cleanup;
}

DWORD
VmDirLdapNfParseStr(
    PCSTR                   pcszStr,
    PVDIR_LDAP_NAME_FORM*   ppNf
    )
{
    DWORD   dwError = 0;
    int     iCode = 0;
    const char* pErr = NULL;
    const int   flags = LDAP_SCHEMA_ALLOW_ALL;
    LDAPNameForm*   pSource = NULL;

    if (!pcszStr || !ppNf)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (IS_NAMEFORM_TAG(pcszStr))
    {
        pcszStr += NAMEFORM_TAG_LEN;
        while (isspace(*pcszStr)) pcszStr++;
    }

    pSource = ldap_str2nameform(pcszStr, &iCode, &pErr, flags);

    if (!pSource)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s: ldap_str2nameform failed (code:%d) (err:%s) %s",
                __FUNCTION__, iCode, ldap_scherr2str(iCode), pErr);
        dwError = ERROR_INVALID_SCHEMA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLdapNfCreate(pSource, ppNf);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    if (pSource)
    {
        ldap_nameform_free(pSource);
    }
    goto cleanup;
}

DWORD
VmDirLdapAtToStr(
    PVDIR_LDAP_ATTRIBUTE_TYPE   pAt,
    PSTR*                       ppszStr
    )
{
    DWORD   dwError = 0;
    PSTR    pszStr = NULL;

    if (!pAt || !ppszStr)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pszStr = ldap_attributetype2str(pAt->pSource);
    dwError = pszStr ? 0 : ERROR_INVALID_SCHEMA;
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszStr = pszStr;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszStr);
    goto cleanup;
}

DWORD
VmDirLdapOcToStr(
    PVDIR_LDAP_OBJECT_CLASS pOc,
    PSTR*                   ppszStr
    )
{
    DWORD   dwError = 0;
    PSTR    pszStr = NULL;

    if (!pOc || !ppszStr)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pOc->pSource->oc_kind = gVdirADToOpenLdapClassType[pOc->type - 1];

    pszStr = ldap_objectclass2str(pOc->pSource);
    dwError = pszStr ? 0 : ERROR_INVALID_SCHEMA;
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszStr = pszStr;

cleanup:
    pOc->pSource->oc_kind = pOc->type;
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszStr);
    goto cleanup;
}

DWORD
VmDirLdapCrToStr(
    PVDIR_LDAP_CONTENT_RULE pCr,
    PSTR*                   ppszStr
    )
{
    DWORD   dwError = 0;
    PSTR    pszStr = NULL;

    if (!pCr || !ppszStr)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pszStr = ldap_contentrule2str(pCr->pSource);
    dwError = pszStr ? 0 : ERROR_INVALID_SCHEMA;
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszStr = pszStr;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszStr);
    goto cleanup;
}

DWORD
VmDirLdapSrToStr(
    PVDIR_LDAP_STRUCTURE_RULE   pSr,
    PSTR*                       ppszStr
    )
{
    DWORD   dwError = 0;
    PSTR    pszStr = NULL;

    if (!pSr || !ppszStr)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pszStr = ldap_structurerule2str(pSr->pSource);
    dwError = pszStr ? 0 : ERROR_INVALID_SCHEMA;
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszStr = pszStr;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszStr);
    goto cleanup;
}

DWORD
VmDirLdapNfToStr(
    PVDIR_LDAP_NAME_FORM    pNf,
    PSTR*                   ppszStr
    )
{
    DWORD   dwError = 0;
    PSTR    pszStr = NULL;

    if (!pNf || !ppszStr)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pszStr = ldap_nameform2str(pNf->pSource);
    dwError = pszStr ? 0 : ERROR_INVALID_SCHEMA;
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszStr = pszStr;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszStr);
    goto cleanup;
}

VOID
VmDirFreeLdapAt(
    PVDIR_LDAP_ATTRIBUTE_TYPE   pAt
    )
{
    if (pAt)
    {
        if (pAt->pSource)
        {
            ldap_attributetype_free(pAt->pSource);
            pAt->pSource = NULL;
        }
        VMDIR_SAFE_FREE_MEMORY(pAt);
    }
}

VOID
VmDirFreeLdapOc(
    PVDIR_LDAP_OBJECT_CLASS pOc
    )
{
    if (pOc)
    {
        if (pOc->pSource)
        {
            ldap_objectclass_free(pOc->pSource);
            pOc->pSource = NULL;
        }
        VMDIR_SAFE_FREE_MEMORY(pOc);
    }
}

VOID
VmDirFreeLdapCr(
    PVDIR_LDAP_CONTENT_RULE pCr
    )
{
    if (pCr)
    {
        if (pCr->pSource)
        {
            ldap_contentrule_free(pCr->pSource);
            pCr->pSource = NULL;
        }
        VMDIR_SAFE_FREE_MEMORY(pCr);
    }
}

VOID
VmDirFreeLdapSr(
    PVDIR_LDAP_STRUCTURE_RULE   pSr
    )
{
    if (pSr)
    {
        if (pSr->pSource)
        {
            ldap_structurerule_free(pSr->pSource);
            pSr->pSource = NULL;
        }
        VMDIR_SAFE_FREE_MEMORY(pSr);
    }
}

VOID
VmDirFreeLdapNf(
    PVDIR_LDAP_NAME_FORM    pNf
    )
{
    if (pNf)
    {
        if (pNf->pSource)
        {
            ldap_nameform_free(pNf->pSource);
            pNf->pSource = NULL;
        }
        VMDIR_SAFE_FREE_MEMORY(pNf);
    }
}

VOID
VmDirFreeLdapDef(
    PVDIR_LDAP_DEFINITION  pDef
    )
{
    if (pDef)
    {
        if (pDef->type == VDIR_LDAP_DEFINITION_TYPE_AT)
        {
            VmDirFreeLdapAt(&pDef->data.at);
        }
        else if (pDef->type == VDIR_LDAP_DEFINITION_TYPE_OC)
        {
            VmDirFreeLdapOc(&pDef->data.oc);
        }
        else if (pDef->type == VDIR_LDAP_DEFINITION_TYPE_CR)
        {
            VmDirFreeLdapCr(&pDef->data.cr);
        }
        else if (pDef->type == VDIR_LDAP_DEFINITION_TYPE_SR)
        {
            VmDirFreeLdapSr(&pDef->data.sr);
        }
        else if (pDef->type == VDIR_LDAP_DEFINITION_TYPE_NF)
        {
            VmDirFreeLdapNf(&pDef->data.nf);
        }
    }
}
