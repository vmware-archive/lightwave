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

static
DWORD
_LdapSchemaObjectDiffInit(
    PSTR                            pszCN,
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF*  ppObjDiff
    )
{
    DWORD   dwError = 0;
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF   pObjDiff = NULL;

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_LDAP_SCHEMA_OBJECT_DIFF),
            (PVOID*)&pObjDiff);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszCN, &pObjDiff->pszCN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pObjDiff->pszDN,
            "cn=%s,%s",
            pszCN, SCHEMA_NAMING_CONTEXT_DN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(&pObjDiff->mods,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppObjDiff = pObjDiff;

cleanup:
    return dwError;

error:
    VmDirFreeLdapSchemaObjectDiff(pObjDiff);
    goto cleanup;
}

static
DWORD
_LdapSchemaObjectDiffAddMod(
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF   pObjDiff,
    VDIR_LDAP_MOD_OP                op,
    PSTR                            type,
    PSTR                            pszVal,
    PSTR*                           ppszVals
    )
{
    DWORD   dwError = 0, i = 0;
    PVDIR_LDAP_MOD  pMod = NULL, pNewMod = NULL;

    if (!pObjDiff || !type || !(pszVal || ppszVals))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (LwRtlHashMapFindKey(pObjDiff->mods, (PVOID*)&pMod, type) != 0)
    {
        dwError = VmDirAllocateMemory(sizeof(VDIR_LDAP_MOD), (PVOID*)&pNewMod);
        BAIL_ON_VMDIR_ERROR(dwError);

        pNewMod->op = op;

        dwError = VmDirAllocateStringA(type, &pNewMod->pszType);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirStringListInitialize(&pNewMod->pVals, 16);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapInsert(
                pObjDiff->mods, pNewMod->pszType, pNewMod, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        pMod = pNewMod;
        pNewMod = NULL;
    }
    else if (pMod->op != op)
    {
        dwError = ERROR_INVALID_OPERATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pszVal)
    {
        dwError = VmDirStringListAddStrClone(pszVal, pMod->pVals);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        for (i = 0; ppszVals[i]; i++)
        {
            dwError = VmDirStringListAddStrClone(ppszVals[i], pMod->pVals);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    return dwError;

error:
    VmDirFreeLdapMod(pNewMod);
    goto cleanup;
}

DWORD
VmDirLdapAtGetDiff(
    PVDIR_LDAP_ATTRIBUTE_TYPE       pOldAt,
    PVDIR_LDAP_ATTRIBUTE_TYPE       pNewAt,
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF*  ppAtDiff
    )
{
    DWORD   dwError = 0;
    CHAR    pszVmwAttrUsage[3] = {0};
    CHAR    pszSearchFlags[4] = {0};
    PSTR*   ppszNewUniqueScopes = NULL;
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF   pAtDiff = NULL;

    if (!pNewAt || !ppAtDiff)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _LdapSchemaObjectDiffInit(pNewAt->pszName, &pAtDiff);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringNPrintFA(pszVmwAttrUsage,
            sizeof(pszVmwAttrUsage), sizeof(pszVmwAttrUsage),
            "%d",
            (1 << pNewAt->usage) >> 1 | (pNewAt->bNoUserMod ? 0x8 : 0));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringNPrintFA(pszSearchFlags,
            sizeof(pszSearchFlags), sizeof(pszSearchFlags),
            "%d",
            pNewAt->dwSearchFlags);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!pOldAt)
    {
        // Add AttributeSchema
        dwError = _LdapSchemaObjectDiffAddMod(pAtDiff, MOD_OP_ADD,
                ATTR_IS_SINGLE_VALUED,
                pNewAt->bSingleValue ? "TRUE" : "FALSE", NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _LdapSchemaObjectDiffAddMod(pAtDiff, MOD_OP_ADD,
                ATTR_OBJECT_CLASS,
                OC_ATTRIBUTE_SCHEMA, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _LdapSchemaObjectDiffAddMod(pAtDiff, MOD_OP_ADD,
                ATTR_CN,
                pNewAt->pszName, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _LdapSchemaObjectDiffAddMod(pAtDiff, MOD_OP_ADD,
                ATTR_ATTRIBUTE_SYNTAX,
                pNewAt->pszSyntaxOid, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _LdapSchemaObjectDiffAddMod(pAtDiff, MOD_OP_ADD,
                ATTR_VMW_ATTRIBUTE_USAGE,
                pszVmwAttrUsage, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _LdapSchemaObjectDiffAddMod(pAtDiff, MOD_OP_ADD,
                ATTR_ATTRIBUTE_ID,
                pNewAt->pszOid, NULL);          // TBD
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _LdapSchemaObjectDiffAddMod(pAtDiff, MOD_OP_ADD,
                ATTR_OMSYNTAX,
                "1", NULL);                     // TBD
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pNewAt->pszDesc)
        {
            dwError = _LdapSchemaObjectDiffAddMod(pAtDiff, MOD_OP_ADD,
                    ATTR_DESCRIPTION,
                    pNewAt->pszDesc, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (pNewAt->dwSearchFlags)
        {
            dwError = _LdapSchemaObjectDiffAddMod(pAtDiff, MOD_OP_ADD,
                    ATTR_SEARCH_FLAGS,
                    pszSearchFlags, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (pNewAt->ppszUniqueScopes)
        {
            dwError = _LdapSchemaObjectDiffAddMod(pAtDiff, MOD_OP_ADD,
                    ATTR_UNIQUENESS_SCOPE,
                    NULL, pNewAt->ppszUniqueScopes);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else
    {
        // Modify AttributeSchema
        if (pNewAt->bSingleValue != pOldAt->bSingleValue)
        {
            dwError = _LdapSchemaObjectDiffAddMod(pAtDiff, MOD_OP_REPLACE,
                    ATTR_IS_SINGLE_VALUED,
                    pNewAt->bSingleValue ? "TRUE" : "FALSE", NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (pNewAt->bNoUserMod != pOldAt->bNoUserMod
                || pNewAt->usage != pOldAt->usage)
        {
            dwError = _LdapSchemaObjectDiffAddMod(pAtDiff, MOD_OP_REPLACE,
                    ATTR_VMW_ATTRIBUTE_USAGE,
                    pszVmwAttrUsage, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (pNewAt->dwSearchFlags != pOldAt->dwSearchFlags)
        {
            dwError = _LdapSchemaObjectDiffAddMod(pAtDiff, MOD_OP_REPLACE,
                    ATTR_SEARCH_FLAGS,
                    pszSearchFlags, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        dwError = VmDirGetStrArrayDiffs(
                pOldAt->ppszUniqueScopes, pNewAt->ppszUniqueScopes,
                &ppszNewUniqueScopes, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (ppszNewUniqueScopes)
        {
            dwError = _LdapSchemaObjectDiffAddMod(pAtDiff, MOD_OP_ADD,
                    ATTR_UNIQUENESS_SCOPE,
                    NULL, ppszNewUniqueScopes);
            BAIL_ON_VMDIR_ERROR(dwError);
            VmDirFreeStrArray(ppszNewUniqueScopes);
        }

        if (pNewAt->pszDesc)
        {
            if (!pOldAt->pszDesc)
            {
                dwError = _LdapSchemaObjectDiffAddMod(pAtDiff, MOD_OP_ADD,
                        ATTR_DESCRIPTION,
                        pNewAt->pszDesc, NULL);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            else if (VmDirStringCompareA(
                    pNewAt->pszDesc, pOldAt->pszDesc, TRUE))
            {
                dwError = _LdapSchemaObjectDiffAddMod(pAtDiff, MOD_OP_REPLACE,
                        ATTR_DESCRIPTION,
                        pNewAt->pszDesc, NULL);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
    }

    if (LwRtlHashMapGetCount(pAtDiff->mods) == 0)
    {
        *ppAtDiff = NULL;
        goto error;
    }

    *ppAtDiff = pAtDiff;

cleanup:
    return dwError;

error:
    VmDirFreeLdapSchemaObjectDiff(pAtDiff);
    goto cleanup;
}

DWORD
VmDirLdapOcGetDiff(
    PVDIR_LDAP_OBJECT_CLASS         pOldOc,
    PVDIR_LDAP_OBJECT_CLASS         pNewOc,
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF*  ppOcDiff
    )
{
    DWORD   dwError = 0;
    PSTR*   ppszNewMay = NULL;
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF   pOcDiff = NULL;

    static PSTR ppszClassType[3] = { "1", "2", "3" };

    if (!pNewOc || !ppOcDiff)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _LdapSchemaObjectDiffInit(pNewOc->pszName, &pOcDiff);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!pOldOc)
    {
        // Add ClassSchema
        dwError = _LdapSchemaObjectDiffAddMod(pOcDiff, MOD_OP_ADD,
                ATTR_OBJECT_CLASS,
                OC_CLASS_SCHEMA, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _LdapSchemaObjectDiffAddMod(pOcDiff, MOD_OP_ADD,
                ATTR_CN,
                pNewOc->pszName, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _LdapSchemaObjectDiffAddMod(pOcDiff, MOD_OP_ADD,
                ATTR_OBJECTCLASS_CATEGORY,
                ppszClassType[pNewOc->type - 1], NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _LdapSchemaObjectDiffAddMod(pOcDiff, MOD_OP_ADD,
                ATTR_GOVERNSID,
                pNewOc->pszOid, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _LdapSchemaObjectDiffAddMod(pOcDiff, MOD_OP_ADD,
                ATTR_SUBCLASSOF,
                pNewOc->pszSup, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pNewOc->ppszMust)
        {
            dwError = _LdapSchemaObjectDiffAddMod(pOcDiff, MOD_OP_ADD,
                    ATTR_SYSTEMMUSTCONTAIN,
                    NULL, pNewOc->ppszMust);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (pNewOc->ppszMay)
        {
            dwError = _LdapSchemaObjectDiffAddMod(pOcDiff, MOD_OP_ADD,
                    ATTR_SYSTEMMAYCONTAIN,
                    NULL, pNewOc->ppszMay);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (pNewOc->pszDesc)
        {
            dwError = _LdapSchemaObjectDiffAddMod(pOcDiff, MOD_OP_ADD,
                    ATTR_DESCRIPTION,
                    pNewOc->pszDesc, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else
    {
        // Modify ClassSchema
        dwError = VmDirGetStrArrayDiffs(
                pOldOc->ppszMay, pNewOc->ppszMay,
                &ppszNewMay, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (ppszNewMay)
        {
            dwError = _LdapSchemaObjectDiffAddMod(pOcDiff, MOD_OP_ADD,
                    ATTR_SYSTEMMAYCONTAIN,
                    NULL, ppszNewMay);
            BAIL_ON_VMDIR_ERROR(dwError);
            VmDirFreeStrArray(ppszNewMay);
        }

        if (pNewOc->pszDesc)
        {
            if (!pOldOc->pszDesc)
            {
                dwError = _LdapSchemaObjectDiffAddMod(pOcDiff, MOD_OP_ADD,
                        ATTR_DESCRIPTION,
                        pNewOc->pszDesc, NULL);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            else if (VmDirStringCompareA(
                    pNewOc->pszDesc, pOldOc->pszDesc, TRUE))
            {
                dwError = _LdapSchemaObjectDiffAddMod(pOcDiff, MOD_OP_REPLACE,
                        ATTR_DESCRIPTION,
                        pNewOc->pszDesc, NULL);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
    }

    if (LwRtlHashMapGetCount(pOcDiff->mods) == 0)
    {
        *ppOcDiff = NULL;
        goto error;
    }

    *ppOcDiff = pOcDiff;

cleanup:
    return dwError;

error:
    VmDirFreeStrArray(ppszNewMay);
    VmDirFreeLdapSchemaObjectDiff(pOcDiff);
    goto cleanup;

}

DWORD
VmDirLdapCrGetDiff(
    PVDIR_LDAP_CONTENT_RULE         pOldCr,
    PVDIR_LDAP_CONTENT_RULE         pNewCr,
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF   pOcDiff,
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF*  ppCrDiff
    )
{
    DWORD   dwError = 0;
    PSTR*   ppszNewMay = NULL;
    PSTR*   ppszNewAux = NULL;
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF   pCrDiff = NULL;

    if (!pNewCr || !ppCrDiff)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pOcDiff)
    {
        pCrDiff = pOcDiff;
    }
    else
    {
        dwError = _LdapSchemaObjectDiffInit(pNewCr->pszName, &pCrDiff);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!pOldCr)
    {
        // Add Content rule
        if (pNewCr->ppszMust)
        {
            dwError = _LdapSchemaObjectDiffAddMod(pCrDiff, MOD_OP_ADD,
                    ATTR_MUSTCONTAIN,
                    NULL, pNewCr->ppszMust);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (pNewCr->ppszMay)
        {
            dwError = _LdapSchemaObjectDiffAddMod(pCrDiff, MOD_OP_ADD,
                    ATTR_MAYCONTAIN,
                    NULL, pNewCr->ppszMay);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (pNewCr->ppszAux)
        {
            dwError = _LdapSchemaObjectDiffAddMod(pCrDiff, MOD_OP_ADD,
                    ATTR_AUXILIARY_CLASS,
                    NULL, pNewCr->ppszAux);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else
    {
        // Modify Content rule
        dwError = VmDirGetStrArrayDiffs(
                pOldCr->ppszMay, pNewCr->ppszMay,
                &ppszNewMay, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirGetStrArrayDiffs(
                pOldCr->ppszAux, pNewCr->ppszAux,
                &ppszNewAux, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (ppszNewMay)
        {
            dwError = _LdapSchemaObjectDiffAddMod(pCrDiff, MOD_OP_ADD,
                    ATTR_MAYCONTAIN,
                    NULL, ppszNewMay);
            BAIL_ON_VMDIR_ERROR(dwError);
            VmDirFreeStrArray(ppszNewMay);
        }

        if (ppszNewAux)
        {
            dwError = _LdapSchemaObjectDiffAddMod(pCrDiff, MOD_OP_ADD,
                    ATTR_AUXILIARY_CLASS,
                    NULL, ppszNewAux);
            BAIL_ON_VMDIR_ERROR(dwError);
            VmDirFreeStrArray(ppszNewAux);
        }
    }

    if (LwRtlHashMapGetCount(pCrDiff->mods) == 0)
    {
        *ppCrDiff = NULL;
        goto error;
    }

    *ppCrDiff = pCrDiff;

cleanup:
    return dwError;

error:
    VmDirFreeStrArray(ppszNewMay);
    VmDirFreeStrArray(ppszNewAux);
    VmDirFreeLdapSchemaObjectDiff(pOcDiff ? NULL : pCrDiff);
    goto cleanup;
}

static
DWORD
_GetAllAttributeTypeDiffs(
    PVDIR_LDAP_SCHEMA_DIFF  pSchemaDiff,
    PVDIR_LDAP_SCHEMA       pOldSchema,
    PVDIR_LDAP_SCHEMA       pNewSchema
    )
{
    DWORD   dwError = 0;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};

    PVDIR_LDAP_ATTRIBUTE_TYPE pOldAt = NULL;
    PVDIR_LDAP_ATTRIBUTE_TYPE pNewAt = NULL;
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF pAtDiff = NULL;

    while (LwRtlHashMapIterate(pNewSchema->attributeTypes, &iter, &pair))
    {
        pOldAt = NULL;
        pNewAt = (PVDIR_LDAP_ATTRIBUTE_TYPE)pair.pValue;
        pAtDiff = NULL;

        if (pOldSchema)
        {
            LwRtlHashMapFindKey(pOldSchema->attributeTypes,
                    (PVOID*)&pOldAt, pNewAt->pszName);
        }

        dwError = VmDirLdapAtGetDiff(pOldAt, pNewAt, &pAtDiff);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pAtDiff)
        {
            if (pOldAt)
            {
                dwError = VmDirLinkedListInsertTail(
                        pSchemaDiff->attrToModify, pAtDiff, NULL);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            else
            {
                dwError = VmDirLinkedListInsertTail(
                        pSchemaDiff->attrToAdd, pAtDiff, NULL);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
    }

cleanup:
    return dwError;

error:
    VmDirFreeLdapSchemaObjectDiff(pAtDiff);
    goto cleanup;
}

static
DWORD
_GetAllObjectClassDiffs(
    PVDIR_LDAP_SCHEMA_DIFF  pSchemaDiff,
    PVDIR_LDAP_SCHEMA       pOldSchema,
    PVDIR_LDAP_SCHEMA       pNewSchema,
    PLW_HASHMAP             pClassDiffMap
    )
{
    DWORD   dwError = 0;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};

    PVDIR_LDAP_OBJECT_CLASS pOldOc = NULL;
    PVDIR_LDAP_OBJECT_CLASS pNewOc = NULL;
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF pOcDiff = NULL;

    while (LwRtlHashMapIterate(pNewSchema->objectClasses, &iter, &pair))
    {
        pOldOc = NULL;
        pNewOc = (PVDIR_LDAP_OBJECT_CLASS)pair.pValue;
        pOcDiff = NULL;

        if (pOldSchema)
        {
            LwRtlHashMapFindKey(pOldSchema->objectClasses,
                    (PVOID*)&pOldOc, pNewOc->pszName);
        }

        dwError = VmDirLdapOcGetDiff(pOldOc, pNewOc, &pOcDiff);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pOcDiff)
        {
            if (pOldOc)
            {
                dwError = VmDirLinkedListInsertTail(
                        pSchemaDiff->classToModify, pOcDiff, NULL);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            else
            {
                dwError = VmDirLinkedListInsertTail(
                        pSchemaDiff->classToAdd, pOcDiff, NULL);
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            dwError = LwRtlHashMapInsert(
                    pClassDiffMap, pOcDiff->pszCN, pOcDiff, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    return dwError;

error:
    VmDirFreeLdapSchemaObjectDiff(pOcDiff);
    goto cleanup;
}

static
DWORD
_GetAllContentRuleDiffs(
    PVDIR_LDAP_SCHEMA_DIFF  pSchemaDiff,
    PVDIR_LDAP_SCHEMA       pOldSchema,
    PVDIR_LDAP_SCHEMA       pNewSchema,
    PLW_HASHMAP             pClassDiffMap
    )
{
    DWORD   dwError = 0;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};

    PVDIR_LDAP_CONTENT_RULE pOldCr = NULL;
    PVDIR_LDAP_CONTENT_RULE pNewCr = NULL;
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF pOcDiff = NULL;
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF pCrDiff = NULL;

    while (LwRtlHashMapIterate(pNewSchema->contentRules, &iter, &pair))
    {
        pOldCr = NULL;
        pNewCr = (PVDIR_LDAP_CONTENT_RULE)pair.pValue;
        pOcDiff = NULL;
        pCrDiff = NULL;

        LwRtlHashMapFindKey(pClassDiffMap, (PVOID*)&pOcDiff, pNewCr->pszName);

        if (pOldSchema)
        {
            LwRtlHashMapFindKey(pOldSchema->contentRules,
                    (PVOID*)&pOldCr, pNewCr->pszName);
        }

        dwError = VmDirLdapCrGetDiff(pOldCr, pNewCr, pOcDiff, &pCrDiff);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pCrDiff && !pOcDiff)
        {
            if (pOldCr)
            {
                dwError = VmDirLinkedListInsertTail(
                        pSchemaDiff->classToModify, pCrDiff, NULL);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            else
            {
                dwError = VmDirLinkedListInsertTail(
                        pSchemaDiff->classToAdd, pCrDiff, NULL);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
    }

cleanup:
    return dwError;

error:
    if (!pOcDiff)
    {
        VmDirFreeLdapSchemaObjectDiff(pCrDiff);
    }
    goto cleanup;
}

static
DWORD
_AddClassFamilyDiffsInOrder(
    PVDIR_LINKED_LIST               pSortedDiffs,
    PVDIR_LDAP_SCHEMA               pSchema,
    PLW_HASHMAP                     pToSort,
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF   pDiff
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PVDIR_LDAP_CONTENT_RULE pCr = NULL;
    PVDIR_LDAP_OBJECT_CLASS pOc = NULL;
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF   pAuxDiff = NULL;
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF   pSupDiff = NULL;

    if (LwRtlHashMapFindKey(pToSort, NULL, pDiff->pszCN) != 0)
    {
        goto cleanup;
    }

    if (LwRtlHashMapFindKey(
            pSchema->contentRules, (PVOID*)&pCr, pDiff->pszCN) == 0)
    {
        for (i = 0; pCr->ppszAux && pCr->ppszAux[i]; i++)
        {
            if (LwRtlHashMapFindKey(
                    pToSort, (PVOID*)&pAuxDiff, pCr->ppszAux[i]) == 0)
            {
                dwError = _AddClassFamilyDiffsInOrder(
                        pSortedDiffs, pSchema, pToSort, pAuxDiff);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
    }

    dwError = LwRtlHashMapFindKey(
            pSchema->objectClasses, (PVOID*)&pOc, pDiff->pszCN);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pOc->pszSup &&
            VmDirStringCompareA(pOc->pszSup, OC_TOP, FALSE) != 0 &&
            LwRtlHashMapFindKey(pToSort, (PVOID*)&pSupDiff, pOc->pszSup) == 0)
    {
        dwError = _AddClassFamilyDiffsInOrder(
                pSortedDiffs, pSchema, pToSort, pSupDiff);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLinkedListInsertTail(pSortedDiffs, pDiff, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlHashMapRemove(pToSort, pDiff->pszCN, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_SortClassDiffsByHierarchy(
    PVDIR_LDAP_SCHEMA_DIFF  pSchemaDiff,
    PVDIR_LDAP_SCHEMA       pSchema
    )
{
    DWORD   dwError = 0;
    PLW_HASHMAP pToSort = NULL;
    PVDIR_LINKED_LIST   pSortedDiffs = NULL;
    PVDIR_LINKED_LIST_NODE  pNode = NULL;
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF   pDiff = NULL;

    dwError = LwRtlCreateHashMap(&pToSort,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pSortedDiffs);
    BAIL_ON_VMDIR_ERROR(dwError);

    pNode = pSchemaDiff->classToAdd->pHead;
    while (pNode)
    {
        pDiff = (PVDIR_LDAP_SCHEMA_OBJECT_DIFF)pNode->pElement;

        dwError = LwRtlHashMapInsert(pToSort, pDiff->pszCN, pDiff, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        pNode = pNode->pPrev;
    }

    pNode = pSchemaDiff->classToAdd->pHead;
    while (pNode)
    {
        pDiff = (PVDIR_LDAP_SCHEMA_OBJECT_DIFF)pNode->pElement;

        dwError = _AddClassFamilyDiffsInOrder(
                pSortedDiffs, pSchema, pToSort, pDiff);
        BAIL_ON_VMDIR_ERROR(dwError);

        pNode = pNode->pPrev;
    }

    VmDirFreeLinkedList(pSchemaDiff->classToAdd);
    pSchemaDiff->classToAdd = pSortedDiffs;

cleanup:
    LwRtlHashMapClear(pToSort, VmDirNoopHashMapPairFree, NULL);
    LwRtlFreeHashMap(&pToSort);
    return dwError;

error:
    VmDirFreeLinkedList(pSortedDiffs);
    goto cleanup;
}

DWORD
VmDirLdapSchemaGetDiff(
    PVDIR_LDAP_SCHEMA       pOldSchema,
    PVDIR_LDAP_SCHEMA       pNewSchema,
    PVDIR_LDAP_SCHEMA_DIFF* ppSchemaDiff
    )
{
    DWORD   dwError = 0;
    PLW_HASHMAP pClassDiffMap = NULL;
    PVDIR_LDAP_SCHEMA_DIFF  pSchemaDiff = NULL;

    if (!pNewSchema || !ppSchemaDiff)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = LwRtlCreateHashMap(&pClassDiffMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_LDAP_SCHEMA_DIFF),
            (PVOID*)&pSchemaDiff);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pSchemaDiff->attrToAdd);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pSchemaDiff->attrToModify);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pSchemaDiff->classToAdd);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pSchemaDiff->classToModify);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _GetAllAttributeTypeDiffs(pSchemaDiff, pOldSchema, pNewSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _GetAllObjectClassDiffs(
            pSchemaDiff, pOldSchema, pNewSchema, pClassDiffMap);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _GetAllContentRuleDiffs(
            pSchemaDiff, pOldSchema, pNewSchema, pClassDiffMap);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _SortClassDiffsByHierarchy(pSchemaDiff, pNewSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppSchemaDiff = pSchemaDiff;

cleanup:
    LwRtlHashMapClear(pClassDiffMap, VmDirNoopHashMapPairFree, NULL);
    LwRtlFreeHashMap(&pClassDiffMap);
    return dwError;

error:
    VmDirFreeLdapSchemaDiff(pSchemaDiff);
    goto cleanup;
}

static
VOID
_FreeLdapModMapPair(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    )
{
    PVDIR_LDAP_MOD pMod = (PVDIR_LDAP_MOD)pPair->pValue;
    VmDirFreeLdapMod(pMod);
}

static
VOID
_FreeLdapSchemaObjectDiffList(
    PVDIR_LINKED_LIST   pLinkedList
    )
{
    PVDIR_LINKED_LIST_NODE  pNode = NULL;
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF   pObjDiff = NULL;

    if (pLinkedList)
    {
        pNode = pLinkedList->pHead;
        while (pNode)
        {
            pObjDiff = (PVDIR_LDAP_SCHEMA_OBJECT_DIFF)pNode->pElement;
            VmDirFreeLdapSchemaObjectDiff(pObjDiff);
            pNode = pNode->pPrev;
        }
        VmDirFreeLinkedList(pLinkedList);
    }
}

VOID
VmDirFreeLdapMod(
    PVDIR_LDAP_MOD  pMod
    )
{
    if (pMod)
    {
        VmDirStringListFree(pMod->pVals);
        VMDIR_SAFE_FREE_MEMORY(pMod->pszType);
        VMDIR_SAFE_FREE_MEMORY(pMod);
    }
}

VOID
VmDirFreeLdapSchemaObjectDiff(
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF   pObjDiff
    )
{
    if (pObjDiff)
    {
        if (pObjDiff->mods)
        {
            LwRtlHashMapClear(pObjDiff->mods, _FreeLdapModMapPair, NULL);
            LwRtlFreeHashMap(&pObjDiff->mods);
        }
        VMDIR_SAFE_FREE_MEMORY(pObjDiff->pszCN);
        VMDIR_SAFE_FREE_MEMORY(pObjDiff->pszDN);
        VMDIR_SAFE_FREE_MEMORY(pObjDiff);
    }
}

VOID
VmDirFreeLdapSchemaDiff(
    PVDIR_LDAP_SCHEMA_DIFF  pSchemaDiff
    )
{
    if (pSchemaDiff)
    {
        _FreeLdapSchemaObjectDiffList(pSchemaDiff->attrToAdd);
        _FreeLdapSchemaObjectDiffList(pSchemaDiff->attrToModify);
        _FreeLdapSchemaObjectDiffList(pSchemaDiff->classToAdd);
        _FreeLdapSchemaObjectDiffList(pSchemaDiff->classToModify);
        VMDIR_SAFE_FREE_MEMORY(pSchemaDiff);
    }
}
