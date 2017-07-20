/*
 * Copyright © 2012-2017 VMware, Inc.  All Rights Reserved.
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
VmDirLdapAtMerge(
    PVDIR_LDAP_ATTRIBUTE_TYPE   pOldAt,
    PVDIR_LDAP_ATTRIBUTE_TYPE   pNewAt,
    PVDIR_LDAP_ATTRIBUTE_TYPE*  ppMergedAt
    )
{
    DWORD   dwError = 0;
    PVDIR_LDAP_ATTRIBUTE_TYPE   pMergedAt = NULL;

    if (!ppMergedAt || !(pOldAt || pNewAt))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pNewAt)
    {
        dwError = VmDirLdapAtDeepCopy(pNewAt, &pMergedAt);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pOldAt && !pMergedAt)
    {
        dwError = VmDirLdapAtDeepCopy(pOldAt, &pMergedAt);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (pOldAt)
    {
        // keep old description if there isn't new description
        if (IsNullOrEmptyString(pNewAt->pszDesc) &&
            !IsNullOrEmptyString(pOldAt->pszDesc))
        {
            dwError = VmDirAllocateStringA(
                    pOldAt->pszDesc, &pMergedAt->pszDesc);
            BAIL_ON_VMDIR_ERROR(dwError);

            // for free later
            pMergedAt->pSource->at_desc = pMergedAt->pszDesc;
        }

        // combine old and new search flags
        if (pOldAt->dwSearchFlags != pNewAt->dwSearchFlags)
        {
            pMergedAt->dwSearchFlags =
                    pOldAt->dwSearchFlags | pNewAt->dwSearchFlags;
        }

        // combine old and new uniqueness scopes
        VmDirFreeStrArray(pMergedAt->ppszUniqueScopes);
        dwError = VmDirMergeStrArray(
                pOldAt->ppszUniqueScopes,
                pNewAt->ppszUniqueScopes,
                &pMergedAt->ppszUniqueScopes);
        BAIL_ON_VMDIR_ERROR(dwError);

        // legacy support
        if (pOldAt->pSource->at_sup_oid)
        {
            VMDIR_SAFE_FREE_MEMORY(pMergedAt->pSource->at_sup_oid);

            dwError = VmDirAllocateStringA(
                    pOldAt->pSource->at_sup_oid,
                    &pMergedAt->pSource->at_sup_oid);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        // legacy support
        if (pOldAt->pSource->at_equality_oid)
        {
            VMDIR_SAFE_FREE_MEMORY(pMergedAt->pSource->at_equality_oid);

            dwError = VmDirAllocateStringA(
                    pOldAt->pSource->at_equality_oid,
                    &pMergedAt->pSource->at_equality_oid);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        // legacy support
        if (pOldAt->pSource->at_ordering_oid)
        {
            VMDIR_SAFE_FREE_MEMORY(pMergedAt->pSource->at_ordering_oid);

            dwError = VmDirAllocateStringA(
                    pOldAt->pSource->at_ordering_oid,
                    &pMergedAt->pSource->at_ordering_oid);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        // legacy support
        if (pOldAt->pSource->at_substr_oid)
        {
            VMDIR_SAFE_FREE_MEMORY(pMergedAt->pSource->at_substr_oid);

            dwError = VmDirAllocateStringA(
                    pOldAt->pSource->at_substr_oid,
                    &pMergedAt->pSource->at_substr_oid);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        // legacy support
        pMergedAt->pSource->at_syntax_len = pOldAt->pSource->at_syntax_len;
    }

    *ppMergedAt = pMergedAt;

cleanup:
    return dwError;

error:
    VmDirFreeLdapAt(pMergedAt);
    goto cleanup;
}

DWORD
VmDirLdapOcMerge(
    PVDIR_LDAP_OBJECT_CLASS     pOldOc,
    PVDIR_LDAP_OBJECT_CLASS     pNewOc,
    PVDIR_LDAP_OBJECT_CLASS*    ppMergedOc
    )
{
    DWORD   dwError = 0;
    PVDIR_LDAP_OBJECT_CLASS pMergedOc = NULL;

    if (!ppMergedOc || !(pOldOc || pNewOc))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pNewOc)
    {
        dwError = VmDirLdapOcDeepCopy(pNewOc, &pMergedOc);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pOldOc && !pMergedOc)
    {
        dwError = VmDirLdapOcDeepCopy(pOldOc, &pMergedOc);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (pOldOc)
    {
        // keep old description if there isn't new description
        if (IsNullOrEmptyString(pNewOc->pszDesc) &&
            !IsNullOrEmptyString(pOldOc->pszDesc))
        {
            dwError = VmDirAllocateStringA(
                    pOldOc->pszDesc, &pMergedOc->pszDesc);
            BAIL_ON_VMDIR_ERROR(dwError);

            // for free later
            pMergedOc->pSource->oc_desc = pMergedOc->pszDesc;
        }

        // merged may = old may + new may
        VmDirFreeStrArray(pMergedOc->ppszMay);
        dwError = VmDirMergeStrArray(
                pOldOc->ppszMay, pNewOc->ppszMay, &pMergedOc->ppszMay);
        BAIL_ON_VMDIR_ERROR(dwError);

        // for free later
        pMergedOc->pSource->oc_at_oids_may = pMergedOc->ppszMay;
    }

    *ppMergedOc = pMergedOc;

cleanup:
    return dwError;

error:
    VmDirFreeLdapOc(pMergedOc);
    goto cleanup;
}

DWORD
VmDirLdapCrMerge(
    PVDIR_LDAP_CONTENT_RULE     pOldCr,
    PVDIR_LDAP_CONTENT_RULE     pNewCr,
    PVDIR_LDAP_CONTENT_RULE*    ppMergedCr
    )
{
    DWORD   dwError = 0;
    PVDIR_LDAP_CONTENT_RULE pMergedCr = NULL;

    if (!ppMergedCr || !(pOldCr || pNewCr))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pNewCr)
    {
        dwError = VmDirLdapCrDeepCopy(pNewCr, &pMergedCr);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pOldCr && !pMergedCr)
    {
        dwError = VmDirLdapCrDeepCopy(pOldCr, &pMergedCr);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (pOldCr)
    {
        // merged may = old may + new may
        VmDirFreeStrArray(pMergedCr->ppszMay);
        dwError = VmDirMergeStrArray(
                pOldCr->ppszMay, pNewCr->ppszMay, &pMergedCr->ppszMay);
        BAIL_ON_VMDIR_ERROR(dwError);

        // for free later
        pMergedCr->pSource->cr_at_oids_may = pMergedCr->ppszMay;

        // merged aux = old aux + new aux
        VmDirFreeStrArray(pMergedCr->ppszAux);
        dwError = VmDirMergeStrArray(
                pOldCr->ppszAux, pNewCr->ppszAux, &pMergedCr->ppszAux);
        BAIL_ON_VMDIR_ERROR(dwError);

        // for free later
        pMergedCr->pSource->cr_oc_oids_aux = pMergedCr->ppszAux;
    }

    *ppMergedCr = pMergedCr;

cleanup:
    return dwError;

error:
    VmDirFreeLdapCr(pMergedCr);
    goto cleanup;
}

DWORD
VmDirLdapSchemaMerge(
    PVDIR_LDAP_SCHEMA   pOldSchema,
    PVDIR_LDAP_SCHEMA   pNewSchema,
    PVDIR_LDAP_SCHEMA*  ppMergedSchema
    )
{
    DWORD   dwError = 0;
    LW_HASHMAP_ITER atIter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_ITER ocIter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_ITER crIter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};

    PVDIR_LDAP_ATTRIBUTE_TYPE   pOldAt = NULL;
    PVDIR_LDAP_ATTRIBUTE_TYPE   pNewAt = NULL;
    PVDIR_LDAP_ATTRIBUTE_TYPE   pMergedAt = NULL;

    PVDIR_LDAP_OBJECT_CLASS pOldOc = NULL;
    PVDIR_LDAP_OBJECT_CLASS pNewOc = NULL;
    PVDIR_LDAP_OBJECT_CLASS pMergedOc = NULL;

    PVDIR_LDAP_CONTENT_RULE pOldCr = NULL;
    PVDIR_LDAP_CONTENT_RULE pNewCr = NULL;
    PVDIR_LDAP_CONTENT_RULE pMergedCr = NULL;

    PVDIR_LDAP_SCHEMA   pMergedSchema = NULL;

    if (!pOldSchema || !pNewSchema || !ppMergedSchema)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLdapSchemaCopy(pOldSchema, &pMergedSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (LwRtlHashMapIterate(pNewSchema->attributeTypes, &atIter, &pair))
    {
        pOldAt = NULL;
        pNewAt = (PVDIR_LDAP_ATTRIBUTE_TYPE)pair.pValue;
        pMergedAt = NULL;

        LwRtlHashMapFindKey(pOldSchema->attributeTypes,
                (PVOID*)&pOldAt, pNewAt->pszName);

        dwError = VmDirLdapAtMerge(pOldAt, pNewAt, &pMergedAt);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapSchemaAddAt(pMergedSchema, pMergedAt);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    pMergedAt = NULL;

    while (LwRtlHashMapIterate(pNewSchema->objectClasses, &ocIter, &pair))
    {
        pOldOc = NULL;
        pNewOc = (PVDIR_LDAP_OBJECT_CLASS)pair.pValue;
        pMergedOc = NULL;

        LwRtlHashMapFindKey(pOldSchema->objectClasses,
                (PVOID*)&pOldOc, pNewOc->pszName);

        dwError = VmDirLdapOcMerge(pOldOc, pNewOc, &pMergedOc);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapSchemaAddOc(pMergedSchema, pMergedOc);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    pMergedOc = NULL;

    while (LwRtlHashMapIterate(pNewSchema->contentRules, &crIter, &pair))
    {
        pOldCr = NULL;
        pNewCr = (PVDIR_LDAP_CONTENT_RULE)pair.pValue;
        pMergedCr = NULL;

        LwRtlHashMapFindKey(pOldSchema->contentRules,
                (PVOID*)&pOldCr, pNewCr->pszName);

        dwError = VmDirLdapCrMerge(pOldCr, pNewCr, &pMergedCr);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapSchemaAddCr(pMergedSchema, pMergedCr);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    pMergedCr = NULL;

    dwError = VmDirLdapSchemaResolveAndVerifyAll(pMergedSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppMergedSchema = pMergedSchema;

cleanup:
    return dwError;

error:
    VmDirFreeLdapAt(pMergedAt);
    VmDirFreeLdapOc(pMergedOc);
    VmDirFreeLdapCr(pMergedCr);
    VmDirFreeLdapSchema(pMergedSchema);
    goto cleanup;
}
