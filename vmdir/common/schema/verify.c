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
VmDirLdapAtVerify(
    PVDIR_LDAP_ATTRIBUTE_TYPE   pAt
    )
{
    DWORD   dwError = 0;

    if (!pAt)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!pAt->pszName)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: missing pszName.",
                __FUNCTION__);
        dwError = ERROR_INVALID_SCHEMA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!pAt->pSource)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: missing pSource (%s).",
                __FUNCTION__, pAt->pszName);
        dwError = ERROR_INVALID_SCHEMA;
    }

    if (!pAt->pszOid)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: missing pszOid (%s).",
                __FUNCTION__, pAt->pszName);
        dwError = ERROR_INVALID_SCHEMA;
    }

    if (!pAt->pszSyntaxOid)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: missing pszSyntaxOid(%s).",
                __FUNCTION__, pAt->pszName);
        dwError = ERROR_INVALID_SCHEMA;
    }

error:
    return dwError;
}

DWORD
VmDirLdapOcVerify(
    PVDIR_LDAP_SCHEMA       pSchema,
    PVDIR_LDAP_OBJECT_CLASS pOc
    )
{
    DWORD dwError = 0;
    DWORD i = 0;
    PVDIR_LDAP_OBJECT_CLASS pSupOc = NULL;
    PVDIR_LDAP_ATTRIBUTE_TYPE pAt = NULL;

    if (!pSchema || !pOc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!pOc->pszName)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: missing pszName.",
                __FUNCTION__);
        dwError = ERROR_INVALID_SCHEMA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!pOc->pSource)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: missing pSource (%s).",
                __FUNCTION__, pOc->pszName);
        dwError = ERROR_INVALID_SCHEMA;
    }

    if (!pOc->pszOid)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: missing pszOid (%s).",
                __FUNCTION__, pOc->pszName);
        dwError = ERROR_INVALID_SCHEMA;
    }

    if (!pOc->pszSup)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: missing pszSup (%s).",
                __FUNCTION__, pOc->pszName);
        dwError = ERROR_INVALID_SCHEMA;
    }
    else if (pOc->pSource->oc_sup_oids[1])
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: more than one sup (%s).",
                __FUNCTION__, pOc->pszName);
        dwError = ERROR_INVALID_SCHEMA;
    }

    for (i = 0; pOc->ppszMust && pOc->ppszMust[i]; i++)
    {
        if (LwRtlHashMapFindKey(pSchema->attributeTypes,
                (PVOID*)&pAt, pOc->ppszMust[i]))
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "%s: (%s) has an unknown must attribute (%s).",
                    __FUNCTION__, pOc->pszName, pOc->ppszMust[i]);
            dwError = ERROR_INVALID_SCHEMA;
        }
    }

    for (i = 0; pOc->ppszMay && pOc->ppszMay[i]; i++)
    {
        if (LwRtlHashMapFindKey(pSchema->attributeTypes,
                (PVOID*)&pAt, pOc->ppszMay[i]))
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "%s: (%s) has an unknown may attribute (%s).",
                    __FUNCTION__, pOc->pszName, pOc->ppszMay[i]);
            dwError = ERROR_INVALID_SCHEMA;
        }
    }

    // skip hierarchy check for top
    if (VmDirStringCompareA(OC_TOP, pOc->pszName, FALSE) == 0)
    {
        goto error;
    }

    if (LwRtlHashMapFindKey(pSchema->objectClasses,
            (PVOID*)&pSupOc, pOc->pszSup))
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s: (%s) has an unknown sup (%s).",
                __FUNCTION__, pOc->pszName, pOc->pszSup);
        dwError = ERROR_INVALID_SCHEMA;
    }
    else if (pOc->type == VDIR_LDAP_STRUCTURAL_CLASS)
    {
        if (pSupOc->type == VDIR_LDAP_AUXILIARY_CLASS)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "%s: (%s) can't have auxiliary (%s) sup.",
                    __FUNCTION__, pOc->pszName, pOc->pszSup);
            dwError = ERROR_INVALID_SCHEMA;
        }
    }
    else if (pOc->type == VDIR_LDAP_AUXILIARY_CLASS)
    {
        if (pSupOc->type == VDIR_LDAP_STRUCTURAL_CLASS)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "%s: (%s) can't have structural (%s) sup.",
                    __FUNCTION__, pOc->pszName, pOc->pszSup);
            dwError = ERROR_INVALID_SCHEMA;
        }
    }
    else    // VDIR_OC_ABSTRACT
    {
        if (pSupOc->type != VDIR_LDAP_ABSTRACT_CLASS)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "%s: (%s) can have only abstract sup.",
                    __FUNCTION__, pOc->pszName);
            dwError = ERROR_INVALID_SCHEMA;
        }
    }

error:
    return dwError;
}

DWORD
VmDirLdapCrVerify(
    PVDIR_LDAP_SCHEMA       pSchema,
    PVDIR_LDAP_CONTENT_RULE pCr
    )
{
    DWORD dwError = 0;
    DWORD i = 0;
    PVDIR_LDAP_OBJECT_CLASS pOc = NULL;
    PVDIR_LDAP_ATTRIBUTE_TYPE pAt = NULL;

    if (!pSchema || !pCr)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!pCr->pszName)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: missing pszName.",
                __FUNCTION__);
        dwError = ERROR_INVALID_SCHEMA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!pCr->pSource)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: missing pSource (%s).",
                __FUNCTION__, pCr->pszName);
        dwError = ERROR_INVALID_SCHEMA;
    }

    if (!pCr->pszOid)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: missing pszOid (%s).",
                __FUNCTION__, pCr->pszName);
        dwError = ERROR_INVALID_SCHEMA;
    }

    if (LwRtlHashMapFindKey(pSchema->objectClasses,
            (PVOID*)&pOc, pCr->pszName))
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s: (%s) is an unknown object class.",
                __FUNCTION__, pCr->pszName);
        dwError = ERROR_INVALID_SCHEMA;
    }
    else if (pOc->type != VDIR_LDAP_STRUCTURAL_CLASS)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s: (%s) is not a str class but has a content rule.",
                __FUNCTION__, pOc->pszName);
        dwError = ERROR_INVALID_SCHEMA;
    }

    for (i = 0; pCr->ppszAux && pCr->ppszAux[i]; i++)
    {
        if (LwRtlHashMapFindKey(pSchema->objectClasses,
                (PVOID*)&pOc, pCr->ppszAux[i]))
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "%s: (%s) has an unknown aux (%s).",
                    __FUNCTION__, pCr->pszName, pCr->ppszAux[i]);
            dwError = ERROR_INVALID_SCHEMA;
        }
        else if (pOc->type != VDIR_LDAP_AUXILIARY_CLASS)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "%s: (%s) is not an aux class but is in aux list for %s.",
                    __FUNCTION__, pOc->pszName, pCr->pszName);
            dwError = ERROR_INVALID_SCHEMA;
        }
    }

    for (i = 0; pCr->ppszMust && pCr->ppszMust[i]; i++)
    {
        if (LwRtlHashMapFindKey(pSchema->attributeTypes,
                (PVOID*)&pAt, pCr->ppszMust[i]))
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "%s: (%s) has an unknown must attribute (%s).",
                    __FUNCTION__, pCr->pszName, pCr->ppszMust[i]);
            dwError = ERROR_INVALID_SCHEMA;
        }
    }

    for (i = 0; pCr->ppszMay && pCr->ppszMay[i]; i++)
    {
        if (LwRtlHashMapFindKey(pSchema->attributeTypes,
                (PVOID*)&pAt, pCr->ppszMay[i]))
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "%s: (%s) has an unknown may attribute (%s).",
                    __FUNCTION__, pCr->pszName, pCr->ppszMay[i]);
            dwError = ERROR_INVALID_SCHEMA;
        }
    }

error:
    return dwError;
}
