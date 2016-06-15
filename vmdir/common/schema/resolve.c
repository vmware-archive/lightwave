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
VmDirLdapAtResolveAliases(
    PVDIR_LDAP_ATTRIBUTE_TYPE   pAt,
    PVDIR_LDAP_ATTRIBUTE_TYPE** ppAtList
    )
{
    DWORD   dwError = 0;
    DWORD   dwListSize = 0, i = 0;
    PSTR    pszName = NULL;
    PVDIR_LDAP_DEFINITION   pDef = NULL;
    PVDIR_LDAP_ATTRIBUTE_TYPE*  pAtList = NULL;

    if (!pAt || !ppAtList)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (; pAt->ppszAliases && pAt->ppszAliases[dwListSize]; dwListSize++);

    dwError = VmDirAllocateMemory(
            sizeof(PVDIR_LDAP_ATTRIBUTE_TYPE) * (dwListSize + 1),
            (PVOID*)&pAtList);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < dwListSize; i++)
    {
        dwError = VmDirLdapAtDeepCopy(pAt, &pAtList[i]);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(
                pAtList[i]->pSource->at_names[i], &pszName);
        BAIL_ON_VMDIR_ERROR(dwError);

        VmDirFreeStrArray(pAtList[i]->pSource->at_names);
        dwError = VmDirAllocateMemory(
                sizeof(PSTR) * 2,
                (PVOID*)&pAtList[i]->pSource->at_names);
        BAIL_ON_VMDIR_ERROR(dwError);

        pAtList[i]->pSource->at_names[0] = pszName;
        pAtList[i]->ppszAliases = pAtList[i]->pSource->at_names;
        pAtList[i]->pszName = pAtList[i]->ppszAliases[0];

        pDef = (PVDIR_LDAP_DEFINITION)pAtList[i];
        pDef->pszName = pAtList[i]->pszName;
    }

    *ppAtList = pAtList;

cleanup:
    return dwError;

error:
    for (i = 0; pAtList && pAtList[i]; i++)
    {
        VmDirFreeLdapAt(pAtList[i]);
    }
    VMDIR_SAFE_FREE_MEMORY(pAtList);
    goto cleanup;
}

DWORD
VmDirLdapAtResolveSup(
    PVDIR_LDAP_SCHEMA           pSchema,
    PVDIR_LDAP_ATTRIBUTE_TYPE   pAt
    )
{
    DWORD   dwError = 0;

    if (!pSchema || !pAt)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pAt->pszSyntaxOid == NULL)
    {
        PVDIR_LDAP_ATTRIBUTE_TYPE pParentAt = pAt;
        while (pParentAt->pszSyntaxOid == NULL)
        {
            PSTR pszSup = VDIR_SAFE_STRING(pParentAt->pSource->at_sup_oid);
            if (LwRtlHashMapFindKey(
                    pSchema->attributeTypes, (PVOID*)&pParentAt, pszSup))
            {
                VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                        "%s: (%s) cannot resolve syntax, unknown sup (%s).",
                        __FUNCTION__, pAt->pszName, pszSup);
                dwError = ERROR_INVALID_SCHEMA;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }

        dwError = VmDirAllocateStringA(
                pParentAt->pszSyntaxOid, &pAt->pszSyntaxOid);
        BAIL_ON_VMDIR_ERROR(dwError);

        // for free later
        pAt->pSource->at_syntax_oid = pAt->pszSyntaxOid;
    }

error:
    return dwError;
}

DWORD
VmDirLdapOcResolveSup(
    PVDIR_LDAP_SCHEMA           pSchema,
    PVDIR_LDAP_OBJECT_CLASS     pOc
    )
{
    DWORD   dwError = 0;

    if (!pSchema || !pOc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // if sup is empty, default to top
    if (pOc->pszSup == NULL)
    {
        dwError = VmDirAllocateStringA(OC_TOP, &pOc->pszSup);
        BAIL_ON_VMDIR_ERROR(dwError);

        // for free later
        dwError = VmDirAllocateMemory(
                sizeof(PSTR) * 2, (PVOID*)&pOc->pSource->oc_sup_oids);
        BAIL_ON_VMDIR_ERROR(dwError);

        pOc->pSource->oc_sup_oids[0] = pOc->pszSup;
    }

error:
    return dwError;
}

DWORD
VmDirLdapCrResolveOid(
    PVDIR_LDAP_SCHEMA       pSchema,
    PVDIR_LDAP_CONTENT_RULE pCr
    )
{
    DWORD   dwError = 0;
    PVDIR_LDAP_OBJECT_CLASS pOc = NULL;

    if (!pSchema || !pCr)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (LwRtlHashMapFindKey(
            pSchema->objectClasses, (PVOID*)&pOc, pCr->pszName))
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s: (%s) cannot resolve content rule, no such class.",
                __FUNCTION__, pCr->pszName);
        dwError = ERROR_INVALID_SCHEMA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (VmDirStringCompareA(pOc->pszOid, pCr->pszOid, TRUE) != 0)
    {
        VMDIR_SAFE_FREE_MEMORY(pCr->pszOid);

        dwError = VmDirAllocateStringA(pOc->pszOid, &pCr->pszOid);
        BAIL_ON_VMDIR_ERROR(dwError);

        // for free later
        pCr->pSource->cr_oid = pCr->pszOid;
    }

error:
    return dwError;
}
