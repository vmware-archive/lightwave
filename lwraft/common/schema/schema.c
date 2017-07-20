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
VmDirLdapSchemaInit(
    PVDIR_LDAP_SCHEMA*  ppSchema
    )
{
    DWORD   dwError = 0;
    PVDIR_LDAP_SCHEMA   pSchema = NULL;

    if (!ppSchema)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_LDAP_SCHEMA),
            (PVOID*)&pSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(&pSchema->attributeTypes,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(&pSchema->objectClasses,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(&pSchema->contentRules,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(&pSchema->structureRules,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(&pSchema->nameForms,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppSchema = pSchema;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirFreeLdapSchema(pSchema);
    goto cleanup;
}

static
DWORD
VmDirLdapSchemaAddDef(
    PLW_HASHMAP             pDefMap,
    PVDIR_LDAP_DEFINITION   pDef
    )
{
    DWORD   dwError = 0;
    BOOLEAN bRemoveOnError = FALSE;
    LW_HASHMAP_PAIR pair = {NULL, NULL};
    PVDIR_LDAP_DEFINITION   pPrevDef = NULL;
    PVDIR_LDAP_DEFINITION_LIST  pDefList = NULL;
    PVDIR_LDAP_DEFINITION_LIST  pNewDefList = NULL;

    if (!pDefMap || !pDef)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = LwRtlHashMapInsert(pDefMap, pDef->pszName, pDef, &pair);
    BAIL_ON_VMDIR_ERROR(dwError);

    pPrevDef = (PVDIR_LDAP_DEFINITION)pair.pValue;
    if (pPrevDef)
    {
        assert(!pDef->pList);   // linked list conflict
        pDefList = pPrevDef->pList;
    }
    else
    {
        pDefList = pDef->pList;
        bRemoveOnError = TRUE;
    }

    if (!pDefList)
    {
        dwError = VmDirLdapDefListCreate(&pNewDefList);
        BAIL_ON_VMDIR_ERROR(dwError);
        pDefList = pNewDefList;
    }

    dwError = VmDirLdapDefListUpdateHead(pDefList, pDef);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    if (pPrevDef)
    {
        (VOID)LwRtlHashMapInsert(pDefMap, pPrevDef->pszName, pPrevDef, NULL);
    }
    else if (bRemoveOnError)
    {
        (VOID)LwRtlHashMapRemove(pDefMap, pDef->pszName, NULL);
    }
    VmDirFreeLdapDefList(pNewDefList);
    goto cleanup;
}

DWORD
VmDirLdapSchemaAddAt(
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

    dwError = VmDirLdapSchemaAddDef(
            pSchema->attributeTypes, (PVDIR_LDAP_DEFINITION)pAt);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}

DWORD
VmDirLdapSchemaAddOc(
    PVDIR_LDAP_SCHEMA       pSchema,
    PVDIR_LDAP_OBJECT_CLASS pOc
    )
{
    DWORD   dwError = 0;
    DWORD   dwNumMust = 0;
    DWORD   dwNumMay = 0;

    if (!pSchema || !pOc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (; pOc->ppszMust && pOc->ppszMust[dwNumMust]; dwNumMust++);
    qsort(pOc->ppszMust, dwNumMust, sizeof(PSTR),
            VmDirQsortCaseIgnoreCompareString);

    for (; pOc->ppszMay && pOc->ppszMay[dwNumMay]; dwNumMay++);
    qsort(pOc->ppszMay, dwNumMay, sizeof(PSTR),
            VmDirQsortCaseIgnoreCompareString);

    dwError = VmDirLdapSchemaAddDef(
            pSchema->objectClasses, (PVDIR_LDAP_DEFINITION)pOc);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}

DWORD
VmDirLdapSchemaAddCr(
    PVDIR_LDAP_SCHEMA       pSchema,
    PVDIR_LDAP_CONTENT_RULE pCr
    )
{
    DWORD   dwError = 0;
    DWORD   dwNumAux = 0;
    DWORD   dwNumMust = 0;
    DWORD   dwNumMay = 0;

    if (!pSchema || !pCr)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (; pCr->ppszAux && pCr->ppszAux[dwNumAux]; dwNumAux++);
    qsort(pCr->ppszAux, dwNumAux, sizeof(PSTR),
            VmDirQsortCaseIgnoreCompareString);

    for (; pCr->ppszMust && pCr->ppszMust[dwNumMust]; dwNumMust++);
    qsort(pCr->ppszMust, dwNumMust, sizeof(PSTR),
            VmDirQsortCaseIgnoreCompareString);

    for (; pCr->ppszMay && pCr->ppszMay[dwNumMay]; dwNumMay++);
    qsort(pCr->ppszMay, dwNumMay, sizeof(PSTR),
            VmDirQsortCaseIgnoreCompareString);

    dwError = VmDirLdapSchemaAddDef(
            pSchema->contentRules, (PVDIR_LDAP_DEFINITION)pCr);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}

DWORD
VmDirLdapSchemaAddSr(
    PVDIR_LDAP_SCHEMA           pSchema,
    PVDIR_LDAP_STRUCTURE_RULE   pSr
    )
{
    DWORD   dwError = 0;

    if (!pSchema || !pSr)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLdapSchemaAddDef(
            pSchema->structureRules, (PVDIR_LDAP_DEFINITION)pSr);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}

DWORD
VmDirLdapSchemaAddNf(
    PVDIR_LDAP_SCHEMA       pSchema,
    PVDIR_LDAP_NAME_FORM    pNf
    )
{
    DWORD   dwError = 0;

    if (!pSchema || !pNf)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLdapSchemaAddDef(
            pSchema->nameForms, (PVDIR_LDAP_DEFINITION)pNf);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}

DWORD
VmDirLdapSchemaResolveAndVerifyAll(
    PVDIR_LDAP_SCHEMA   pSchema
    )
{
    DWORD dwError = 0;
    LW_HASHMAP_ITER atIter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_ITER ocIter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_ITER crIter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};

    if (!pSchema)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    while (LwRtlHashMapIterate(pSchema->attributeTypes, &atIter, &pair))
    {
        PVDIR_LDAP_ATTRIBUTE_TYPE pAt = (PVDIR_LDAP_ATTRIBUTE_TYPE)pair.pValue;

        dwError = VmDirLdapAtResolveSup(pSchema, pAt);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapAtVerify(pAt);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    while (LwRtlHashMapIterate(pSchema->objectClasses, &ocIter, &pair))
    {
        PVDIR_LDAP_OBJECT_CLASS pOc = (PVDIR_LDAP_OBJECT_CLASS)pair.pValue;

        dwError = VmDirLdapOcResolveSup(pSchema, pOc);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapOcVerify(pSchema, pOc);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    while (LwRtlHashMapIterate(pSchema->contentRules, &crIter, &pair))
    {
        PVDIR_LDAP_CONTENT_RULE pCr = (PVDIR_LDAP_CONTENT_RULE)pair.pValue;

        dwError = VmDirLdapCrResolveOid(pSchema, pCr);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapCrVerify(pSchema, pCr);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:
    return dwError;
}

/*
 * When LdapSchema is created from file or subschema subentry, it contains
 * data which is no longer needed. Only relevant data should remain in order
 * to keep LdapSchema instance always consistent.
 */
DWORD
VmDirLdapSchemaRemoveNoopData(
    PVDIR_LDAP_SCHEMA   pSchema
    )
{
    DWORD dwError = 0;
    LW_HASHMAP_ITER atIter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_ITER crIter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};

    if (!pSchema)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    while (LwRtlHashMapIterate(pSchema->attributeTypes, &atIter, &pair))
    {
        PVDIR_LDAP_ATTRIBUTE_TYPE pAt = (PVDIR_LDAP_ATTRIBUTE_TYPE)pair.pValue;
        VMDIR_SAFE_FREE_MEMORY(pAt->pSource->at_equality_oid);
        VMDIR_SAFE_FREE_MEMORY(pAt->pSource->at_ordering_oid);
        VMDIR_SAFE_FREE_MEMORY(pAt->pSource->at_substr_oid);
        VMDIR_SAFE_FREE_MEMORY(pAt->pSource->at_sup_oid);
        pAt->pSource->at_syntax_len = 0;
    }

    while (LwRtlHashMapIterate(pSchema->contentRules, &crIter, &pair))
    {
        PVDIR_LDAP_CONTENT_RULE pCr = (PVDIR_LDAP_CONTENT_RULE)pair.pValue;
        VMDIR_SAFE_FREE_MEMORY(pCr->pSource->cr_desc);
    }

error:
    return dwError;
}

static
VOID
_FreeDefMapPair(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    )
{
    PVDIR_LDAP_DEFINITION   pDef = NULL;

    if (pPair && pPair->pValue)
    {
        pDef = (PVDIR_LDAP_DEFINITION)pPair->pValue;
        VmDirLdapDefListRelease(pDef->pList, pDef);
    }
}

BOOLEAN
VmDirLdapSchemaIsEmpty(
    PVDIR_LDAP_SCHEMA   pSchema
    )
{
    BOOLEAN bEmpty = TRUE;

    if (pSchema)
    {
        if (LwRtlHashMapGetCount(pSchema->attributeTypes) ||
            LwRtlHashMapGetCount(pSchema->objectClasses) ||
            LwRtlHashMapGetCount(pSchema->contentRules) ||
            LwRtlHashMapGetCount(pSchema->structureRules) ||
            LwRtlHashMapGetCount(pSchema->nameForms))
        {
            bEmpty = FALSE;
        }
    }

    return bEmpty;
}

VOID
VmDirFreeLdapSchema(
    PVDIR_LDAP_SCHEMA   pSchema
    )
{
    if (pSchema)
    {
        if (pSchema->attributeTypes)
        {
            LwRtlHashMapClear(pSchema->attributeTypes, _FreeDefMapPair, NULL);
            LwRtlFreeHashMap(&pSchema->attributeTypes);
        }

        if (pSchema->objectClasses)
        {
            LwRtlHashMapClear(pSchema->objectClasses, _FreeDefMapPair, NULL);
            LwRtlFreeHashMap(&pSchema->objectClasses);
        }

        if (pSchema->contentRules)
        {
            LwRtlHashMapClear(pSchema->contentRules, _FreeDefMapPair, NULL);
            LwRtlFreeHashMap(&pSchema->contentRules);
        }

        if (pSchema->structureRules)
        {
            LwRtlHashMapClear(pSchema->structureRules, _FreeDefMapPair, NULL);
            LwRtlFreeHashMap(&pSchema->structureRules);
        }

        if (pSchema->nameForms)
        {
            LwRtlHashMapClear(pSchema->nameForms, _FreeDefMapPair, NULL);
            LwRtlFreeHashMap(&pSchema->nameForms);
        }

        VMDIR_SAFE_FREE_MEMORY(pSchema);
    }
}
