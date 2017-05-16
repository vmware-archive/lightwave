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

// both NULL || both have same string
#define VMDIR_TWO_STRING_COMPATIBLE( pszNewString, pszOldString )                                               \
    ( ( !pszNewString && !pszOldString )  ||                                                                    \
      ( ( pszNewString && pszOldString ) && (VmDirStringCompareA( pszNewString, pszOldString, FALSE) == 0) )    \
    )

// e.g. single value tag from TRUE to FALSE
#define VMDIR_TWO_BOOL_COMPATILBE_T2F( bONE, bTWO )     \
    ( (bTWO == bONE) || ( bTWO == TRUE && bONE == FALSE ) )

// e.g. obsolete tag from FALSE to TRUE
#define VMDIR_TWO_BOOL_COMPATILBE_F2T( bONE, bTWO )     \
    ( (bONE == bTWO) || ( bONE == TRUE && bTWO == FALSE ) )

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

    if (!VMDIR_TWO_STRING_COMPATIBLE(
            pNewAt->pszName, pPrevAt->pszName) ||
        !VMDIR_TWO_STRING_COMPATIBLE(
            pNewAt->pszSyntaxOid, pPrevAt->pszSyntaxOid) ||
        !VMDIR_TWO_BOOL_COMPATILBE_T2F(
            pNewAt->bSingleValue, pPrevAt->bSingleValue) ||
        pNewAt->bNoUserMod != pPrevAt->bNoUserMod ||
        (pPrevAt->usage && pNewAt->usage != pPrevAt->usage))
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s: cannot accept backward incompatible defn (%s).",
                __FUNCTION__, pPrevAt->pszName);
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

    if (!VMDIR_TWO_STRING_COMPATIBLE(
            pNewOc->pszName, pPrevOc->pszName) ||
        !VMDIR_TWO_STRING_COMPATIBLE(
            pNewOc->pszSup, pPrevOc->pszSup) ||
        pNewOc->type != pPrevOc->type)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s: cannot accept backward incompatible defn (%s).",
                __FUNCTION__, pPrevOc->pszName);
        dwError = VMDIR_ERROR_SCHEMA_NOT_COMPATIBLE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (VmDirIsStrArrayIdentical(pNewOc->ppszMust, pPrevOc->ppszMust))
    {
        if (!VmDirIsStrArraySuperSet(pNewOc->ppszMay, pPrevOc->ppszMay))
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "%s: cannot remove maycontain attribute (%s).",
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
                    "%s: cannot remove must contain attribute (%s).",
                    __FUNCTION__, pPrevOc->pszName);
            dwError = VMDIR_ERROR_SCHEMA_NOT_COMPATIBLE;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s: cannot add must contain attribute (%s).",
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

    if (!VMDIR_TWO_STRING_COMPATIBLE(
            pNewCr->pszName, pPrevCr->pszName) ||
        !VmDirIsStrArraySuperSet(
            pNewCr->ppszAux, pPrevCr->ppszAux))
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s: cannot accept backward incompatible defn (%s).",
                __FUNCTION__, pPrevCr->pszName);
        dwError = VMDIR_ERROR_SCHEMA_NOT_COMPATIBLE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (VmDirIsStrArrayIdentical(pNewCr->ppszMust, pPrevCr->ppszMust))
    {
        if (!VmDirIsStrArraySuperSet(pNewCr->ppszMay, pPrevCr->ppszMay))
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "%s: cannot remove maycontain attribute (%s).",
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
                    "%s: cannot remove must contain attribute (%s).",
                    __FUNCTION__, pPrevCr->pszName);
            dwError = VMDIR_ERROR_SCHEMA_NOT_COMPATIBLE;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s: cannot add must contain attribute (%s).",
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
