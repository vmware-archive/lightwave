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
VmDirLdapAtAreCompat(
    PVDIR_LDAP_ATTRIBUTE_TYPE   pPrevAt,
    PVDIR_LDAP_ATTRIBUTE_TYPE   pNewAt
    )
{
    DWORD   dwError = 0;

    if (!pPrevAt || !pNewAt)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (VmDirStringCompareA(pPrevAt->pszName, pNewAt->pszName, FALSE))
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s: cannot change attribute type name (current: %s, new: %s).",
                __FUNCTION__,
                pPrevAt->pszName,
                pNewAt->pszName);
        dwError = VMDIR_ERROR_SCHEMA_NOT_COMPATIBLE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (VmDirStringCompareA(pPrevAt->pszSyntaxOid, pNewAt->pszSyntaxOid, FALSE))
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s: cannot change attribute type syntax (current: %s, new: %s) (name: %s).",
                __FUNCTION__,
                pPrevAt->pszSyntaxOid,
                pNewAt->pszSyntaxOid,
                pNewAt->pszName);
        dwError = VMDIR_ERROR_SCHEMA_NOT_COMPATIBLE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!pPrevAt->bSingleValue && pNewAt->bSingleValue)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s: cannot convert multi-value attribute type to single-value (name: %s).",
                __FUNCTION__, pNewAt->pszName);
        dwError = VMDIR_ERROR_SCHEMA_NOT_COMPATIBLE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pPrevAt->bNoUserMod != pNewAt->bNoUserMod)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s: cannot change attribute type user mod permission (current: %s, new: %s) (name: %s).",
                __FUNCTION__,
                pPrevAt->bNoUserMod ? "TRUE" : "FALSE",
                        pNewAt->bNoUserMod ? "TRUE" : "FALSE",
                                pNewAt->pszName);
        dwError = VMDIR_ERROR_SCHEMA_NOT_COMPATIBLE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pPrevAt->usage && pPrevAt->usage != pNewAt->usage)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s: cannot change attribute type usage (current: %d, new: %d) (%s).",
                __FUNCTION__,
                pPrevAt->usage,
                pNewAt->usage,
                pNewAt->pszName);
        dwError = VMDIR_ERROR_SCHEMA_NOT_COMPATIBLE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:
    return dwError;
}

DWORD
VmDirLdapOcAreCompat(
    PVDIR_LDAP_OBJECT_CLASS pPrevOc,
    PVDIR_LDAP_OBJECT_CLASS pNewOc
    )
{
    DWORD   dwError = 0;
    PSTR*   ppszRemovedMust = NULL;
    PSTR*   ppszMinimumMay = NULL;

    if (!pPrevOc || !pNewOc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (VmDirStringCompareA(pPrevOc->pszName, pNewOc->pszName, FALSE))
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s: cannot change object class name (current: %s, new: %s).",
                __FUNCTION__,
                pPrevOc->pszName,
                pNewOc->pszName);
        dwError = VMDIR_ERROR_SCHEMA_NOT_COMPATIBLE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (VmDirStringCompareA(pPrevOc->pszSup, pNewOc->pszSup, FALSE))
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s: cannot change object class sup (current: %s, new: %s) (name: %s).",
                __FUNCTION__,
                pPrevOc->pszSup,
                pNewOc->pszSup,
                pNewOc->pszName);
        dwError = VMDIR_ERROR_SCHEMA_NOT_COMPATIBLE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pPrevOc->type != pNewOc->type)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s: cannot change object class type (current: %d, new: %d) (name: %s).",
                __FUNCTION__,
                pPrevOc->type,
                pNewOc->type,
                pNewOc->pszName);
        dwError = VMDIR_ERROR_SCHEMA_NOT_COMPATIBLE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (VmDirIsStrArrayIdentical(pNewOc->ppszMust, pPrevOc->ppszMust))
    {
        if (!VmDirIsStrArraySuperSet(pNewOc->ppszMay, pPrevOc->ppszMay))
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "%s: cannot remove maycontain attribute types (name: %s).",
                    __FUNCTION__, pPrevOc->pszName);
            dwError = VMDIR_ERROR_SCHEMA_NOT_COMPATIBLE;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else if (VmDirIsStrArraySuperSet(pPrevOc->ppszMust, pNewOc->ppszMust))
    {
        dwError = VmDirGetStrArrayDiffs(
                pNewOc->ppszMust, pPrevOc->ppszMust, &ppszRemovedMust, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirMergeStrArray(
                pPrevOc->ppszMay, ppszRemovedMust, &ppszMinimumMay);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (!VmDirIsStrArraySuperSet(pNewOc->ppszMay, ppszMinimumMay))
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "%s: cannot remove must contain attribute types (name: %s).",
                    __FUNCTION__, pPrevOc->pszName);
            dwError = VMDIR_ERROR_SCHEMA_NOT_COMPATIBLE;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s: cannot add must contain attribute types (name: %s).",
                __FUNCTION__, pPrevOc->pszName);
        dwError = VMDIR_ERROR_SCHEMA_NOT_COMPATIBLE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:
    VmDirFreeStrArray(ppszRemovedMust);
    VmDirFreeStrArray(ppszMinimumMay);
    return dwError;
}

DWORD
VmDirLdapCrAreCompat(
    PVDIR_LDAP_CONTENT_RULE pPrevCr,
    PVDIR_LDAP_CONTENT_RULE pNewCr
    )
{
    DWORD   dwError = 0;
    PSTR*   ppszRemovedMust = NULL;
    PSTR*   ppszMinimumMay = NULL;

    if (!pPrevCr || !pNewCr)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (VmDirStringCompareA(pPrevCr->pszName, pNewCr->pszName, FALSE))
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s: cannot change content rule name (current: %s, new: %s).",
                __FUNCTION__,
                pPrevCr->pszName,
                pNewCr->pszName);
        dwError = VMDIR_ERROR_SCHEMA_NOT_COMPATIBLE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!VmDirIsStrArraySuperSet(pNewCr->ppszAux, pPrevCr->ppszAux))
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s: cannot remove auxiliary class(es) (name: %s).",
                __FUNCTION__, pPrevCr->pszName);
        dwError = VMDIR_ERROR_SCHEMA_NOT_COMPATIBLE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (VmDirIsStrArrayIdentical(pNewCr->ppszMust, pPrevCr->ppszMust))
    {
        if (!VmDirIsStrArraySuperSet(pNewCr->ppszMay, pPrevCr->ppszMay))
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "%s: cannot remove maycontain attribute (name: %s).",
                    __FUNCTION__, pPrevCr->pszName);
            dwError = VMDIR_ERROR_SCHEMA_NOT_COMPATIBLE;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else if (VmDirIsStrArraySuperSet(pPrevCr->ppszMust, pNewCr->ppszMust))
    {
        dwError = VmDirGetStrArrayDiffs(
                pNewCr->ppszMust, pPrevCr->ppszMust, &ppszRemovedMust, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirMergeStrArray(
                pPrevCr->ppszMay, ppszRemovedMust, &ppszMinimumMay);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (!VmDirIsStrArraySuperSet(pNewCr->ppszMay, ppszMinimumMay))
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "%s: cannot remove must contain attribute (name: %s).",
                    __FUNCTION__, pPrevCr->pszName);
            dwError = VMDIR_ERROR_SCHEMA_NOT_COMPATIBLE;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s: cannot add must contain attribute (name: %s).",
                __FUNCTION__, pPrevCr->pszName);
        dwError = VMDIR_ERROR_SCHEMA_NOT_COMPATIBLE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:
    VmDirFreeStrArray(ppszRemovedMust);
    VmDirFreeStrArray(ppszMinimumMay);
    return dwError;
}

DWORD
VmDirLdapSrAreCompat(
    PVDIR_LDAP_STRUCTURE_RULE   pPrevSr,
    PVDIR_LDAP_STRUCTURE_RULE   pNewSr
    )
{
    DWORD   dwError = 0;

    if (!pPrevSr || !pNewSr)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    // TODO

error:
    return dwError;
}

DWORD
VmDirLdapNfAreCompat(
    PVDIR_LDAP_NAME_FORM    pPrevNf,
    PVDIR_LDAP_NAME_FORM    pNewNf
    )
{
    DWORD   dwError = 0;

    if (!pPrevNf || !pNewNf)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    // TODO

error:
    return dwError;
}

DWORD
VmDirLdapDefAreCompat(
    PVDIR_LDAP_DEFINITION   pPrevDef,
    PVDIR_LDAP_DEFINITION   pNewDef
    )
{
    DWORD   dwError = 0;

    if (!pPrevDef || !pNewDef || pPrevDef->type != pNewDef->type)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pNewDef->type == VDIR_LDAP_DEFINITION_TYPE_AT)
    {
        dwError = VmDirLdapAtAreCompat(&pPrevDef->data.at, &pNewDef->data.at);
    }
    else if (pNewDef->type == VDIR_LDAP_DEFINITION_TYPE_OC)
    {
        dwError = VmDirLdapOcAreCompat(&pPrevDef->data.oc, &pNewDef->data.oc);
    }
    else if (pNewDef->type == VDIR_LDAP_DEFINITION_TYPE_CR)
    {
        dwError = VmDirLdapCrAreCompat(&pPrevDef->data.cr, &pNewDef->data.cr);
    }
    else if (pNewDef->type == VDIR_LDAP_DEFINITION_TYPE_SR)
    {
        dwError = VmDirLdapSrAreCompat(&pPrevDef->data.sr, &pNewDef->data.sr);
    }
    else if (pNewDef->type == VDIR_LDAP_DEFINITION_TYPE_NF)
    {
        dwError = VmDirLdapNfAreCompat(&pPrevDef->data.nf, &pNewDef->data.nf);
    }
    else
    {
        dwError = ERROR_INVALID_DATATYPE;
    }

error:
    return dwError;
}
