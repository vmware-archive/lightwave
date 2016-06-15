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
_getOCAttr(
    PVDIR_SCHEMA_CTX    pCtx,           // IN
    PVDIR_ENTRY         pEntry,         // IN
    PVDIR_ATTRIBUTE*    ppOCAttr        // OUT
    )
{
    DWORD dwError = 0;
    PVDIR_ATTRIBUTE pAttr = NULL;
    PVDIR_SCHEMA_AT_DESC pObjectClassATDesc = NULL;

    assert(pCtx && pEntry && ppOCAttr);

    dwError = VmDirSchemaAttrNameToDescriptor(
            pCtx, "objectClass", &pObjectClassATDesc);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (pAttr = pEntry->attrs; pAttr; pAttr = pAttr->next)
    {
        if (pAttr->pATDesc->usAttrID == pObjectClassATDesc->usAttrID)
        {
            break;
        }
    }

    if (!pAttr || pAttr->numVals < 1)
    {
        VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
        VmDirAllocateStringAVsnprintf(&pCtx->pszErrorMsg,
                "Entry has no objectclass");

        dwError = pCtx->dwErrorCode = ERROR_INVALID_ENTRY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppOCAttr = pAttr;

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_getOCDescs(
    PVDIR_SCHEMA_CTX        pCtx,           // IN
    PVDIR_ATTRIBUTE         pOCAttr,        // IN
    PVDIR_SCHEMA_OC_DESC**  pppStrOCs,      // OUT
    PVDIR_SCHEMA_OC_DESC**  pppAuxOCs,      // OUT
    PVDIR_SCHEMA_OC_DESC**  pppAbsOCs       // OUT
    )
{
    DWORD dwError = 0;
    DWORD i = 0, dwNumStrOC = 0, dwNumAuxOC = 0, dwNumAbsOC = 0;
    PVDIR_SCHEMA_OC_DESC* ppStrOCs = NULL;
    PVDIR_SCHEMA_OC_DESC* ppAuxOCs = NULL;
    PVDIR_SCHEMA_OC_DESC* ppAbsOCs = NULL;
    PVDIR_SCHEMA_OC_DESC pOCDesc = NULL;

    dwError = VmDirAllocateMemory(
            sizeof(PVDIR_SCHEMA_OC_DESC) * (pOCAttr->numVals + 1),
            (PVOID*)&ppStrOCs);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
            sizeof(PVDIR_SCHEMA_OC_DESC) * (pOCAttr->numVals + 1),
            (PVOID*)&ppAuxOCs);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
            sizeof(PVDIR_SCHEMA_OC_DESC) * (pOCAttr->numVals + 1),
            (PVOID*)&ppAbsOCs);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < pOCAttr->numVals; i++)
    {
        PSTR pszOCName = VDIR_SAFE_STRING(pOCAttr->vals[i].lberbv.bv_val);
        dwError = VmDirSchemaOCNameToDescriptor(pCtx, pszOCName, &pOCDesc);
        if (dwError)
        {
            VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
            VmDirAllocateStringAVsnprintf(&pCtx->pszErrorMsg,
                    "Objectclass (%s) is not defined in schema",
                    pszOCName);

            dwError = pCtx->dwErrorCode = ERROR_INVALID_ENTRY;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (pOCDesc->type == VDIR_LDAP_STRUCTURAL_CLASS)
        {
            ppStrOCs[dwNumStrOC++] = pOCDesc;
        }
        else if (pOCDesc->type == VDIR_LDAP_AUXILIARY_CLASS)
        {
            ppAuxOCs[dwNumAuxOC++] = pOCDesc;
        }
        else    // VDIR_LDAP_ABSTRACT_CLASS
        {
            ppAbsOCs[dwNumAbsOC++] = pOCDesc;
        }
    }

    *pppStrOCs = ppStrOCs;
    *pppAuxOCs = ppAuxOCs;
    *pppAbsOCs = ppAbsOCs;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(ppStrOCs);
    VMDIR_SAFE_FREE_MEMORY(ppAuxOCs);
    VMDIR_SAFE_FREE_MEMORY(ppAbsOCs);
    goto cleanup;
}

static
DWORD
_getCRDescs(
    PVDIR_SCHEMA_CTX        pCtx,           // IN
    PVDIR_SCHEMA_OC_DESC    pBottomOC,      // IN
    PVDIR_SCHEMA_CR_DESC**  pppCRs          // OUT
    )
{
    DWORD dwError = 0;
    PVDIR_SCHEMA_OC_DESC pOCDesc = pBottomOC;
    PVDIR_SCHEMA_CR_DESC pCRDesc = NULL;
    PVDIR_SCHEMA_CR_DESC* ppCRs = NULL;
    PDEQUE pCRDescDeque = NULL;
    DWORD i = 0;

    dwError = dequeCreate(&pCRDescDeque);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (pOCDesc)
    {
        if (pOCDesc->type != VDIR_LDAP_STRUCTURAL_CLASS)
        {
            break;
        }

        dwError = VmDirSchemaCRNameToDescriptor(
                pCtx, pOCDesc->pszName, &pCRDesc);

        if (!dwError)
        {
            dwError = dequePush(pCRDescDeque, pCRDesc);
        }
        else if (dwError == ERROR_NO_SUCH_DITCONTENTRULES)
        {
            dwError = 0;
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirSchemaOCNameToDescriptor(
                pCtx, pOCDesc->pszSup, &pOCDesc);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(PVDIR_SCHEMA_CR_DESC) * (dequeGetSize(pCRDescDeque) + 1),
            (PVOID*)&ppCRs);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (!dequeIsEmpty(pCRDescDeque))
    {
        dwError = dequePopLeft(pCRDescDeque, (PVOID*)&ppCRs[i++]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *pppCRs = ppCRs;

cleanup:
    dequeFree(pCRDescDeque);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_getAllMayAttributes(
    PVDIR_SCHEMA_CTX        pCtx,           // IN
    PVDIR_SCHEMA_OC_DESC    pOCDesc,        // IN
    PLW_HASHMAP             pAllMayAttrMap  // IN
    )
{
    DWORD dwError = 0;
    DWORD i = 0;
    PVDIR_SCHEMA_OC_DESC pCurOC = NULL;
    PVDIR_SCHEMA_CR_DESC pCR = NULL;

    if (!pCtx || !pOCDesc || !pAllMayAttrMap)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pCurOC = pOCDesc;
    while (pCurOC)
    {
        for (i = 0; pCurOC->ppszMayATs && pCurOC->ppszMayATs[i]; i++)
        {
            dwError = LwRtlHashMapInsert(pAllMayAttrMap,
                    pCurOC->ppszMayATs[i], pCurOC->ppszMayATs[i], NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (pCurOC->type == VDIR_LDAP_STRUCTURAL_CLASS)
        {
            dwError = VmDirSchemaCRNameToDescriptor(
                    pCtx, pCurOC->pszName, &pCR);

            if (dwError == 0)
            {
                for (i = 0; pCR->ppszMayATs && pCR->ppszMayATs[i]; i++)
                {
                    dwError = LwRtlHashMapInsert(pAllMayAttrMap,
                            pCR->ppszMayATs[i], pCR->ppszMayATs[i], NULL);
                    BAIL_ON_VMDIR_ERROR(dwError);
                }
            }
            else if (dwError != ERROR_NO_SUCH_DITCONTENTRULES)
            {
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }

        if (VmDirStringCompareA(OC_TOP, pCurOC->pszName, FALSE) == 0)
        {
            pCurOC = NULL;
        }
        else
        {
            dwError = VmDirSchemaOCNameToDescriptor(
                    pCtx, pCurOC->pszSup, &pCurOC);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

error:
    return dwError;
}

static
DWORD
_getAllMustAttributes(
    PVDIR_SCHEMA_CTX        pCtx,           // IN
    PVDIR_SCHEMA_OC_DESC    pOCDesc,        // IN
    PLW_HASHMAP             pAllMustAttrMap // IN
    )
{
    DWORD dwError = 0;
    DWORD i = 0;
    PVDIR_SCHEMA_OC_DESC pCurOC = NULL;
    PVDIR_SCHEMA_CR_DESC pCR = NULL;

    if (!pCtx || !pOCDesc || !pAllMustAttrMap)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pCurOC = pOCDesc;
    while (pCurOC)
    {
        for (i = 0; pCurOC->ppszMustATs && pCurOC->ppszMustATs[i]; i++)
        {
            dwError = LwRtlHashMapInsert(pAllMustAttrMap,
                    pCurOC->ppszMustATs[i], pCurOC->ppszMustATs[i], NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (pCurOC->type == VDIR_LDAP_STRUCTURAL_CLASS)
        {
            dwError = VmDirSchemaCRNameToDescriptor(
                    pCtx, pCurOC->pszName, &pCR);

            if (dwError == 0)
            {
                for (i = 0; pCR->ppszMustATs && pCR->ppszMustATs[i]; i++)
                {
                    dwError = LwRtlHashMapInsert(pAllMustAttrMap,
                            pCR->ppszMustATs[i], pCR->ppszMustATs[i], NULL);
                    BAIL_ON_VMDIR_ERROR(dwError);
                }
            }
            else if (dwError != ERROR_NO_SUCH_DITCONTENTRULES)
            {
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }

        if (VmDirStringCompareA(OC_TOP, pCurOC->pszName, FALSE) == 0)
        {
            pCurOC = NULL;
        }
        else
        {
            dwError = VmDirSchemaOCNameToDescriptor(
                    pCtx, pCurOC->pszSup, &pCurOC);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

error:
    return dwError;
}

static
DWORD
_checkAttributeSyntax(
    PVDIR_SCHEMA_CTX    pCtx,           // IN
    PVDIR_ENTRY         pEntry          // IN
    )
{
    DWORD           dwError = 0;
    VDIR_ATTRIBUTE* pAttr = NULL;

    for (pAttr = pEntry->attrs; pAttr; pAttr = pAttr->next)
    {
        USHORT usCnt = 0;
        for (usCnt = 0; usCnt < pAttr->numVals; usCnt++)
        {
            dwError = VmDirSchemaBervalSyntaxCheck(
                    pCtx, pAttr->pATDesc, &pAttr->vals[usCnt]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

error:
    return dwError;
}

static
DWORD
_checkAttributeDimension(
    PVDIR_SCHEMA_CTX    pCtx,           // IN
    PVDIR_ENTRY         pEntry          // IN
    )
{
    DWORD           dwError = 0;
    VDIR_ATTRIBUTE* pAttr = NULL;

    for (pAttr = pEntry->attrs; pAttr; pAttr = pAttr->next)
    {
        if (pAttr->pATDesc->bSingleValue && pAttr->numVals != 1)
        {
            VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
            VmDirAllocateStringAVsnprintf(&pCtx->pszErrorMsg,
                    "Attribute (%s) can have at most one value",
                    VDIR_SAFE_STRING(pAttr->type.lberbv.bv_val));

            dwError = pCtx->dwErrorCode = ERROR_INVALID_ENTRY;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

error:
    return dwError;
}

static
DWORD
_checkObjectClassHierarchy(
    PVDIR_SCHEMA_CTX        pCtx,           // IN
    PVDIR_SCHEMA_OC_DESC*   ppStrOCs,       // IN
    PVDIR_SCHEMA_OC_DESC*   ppAbsOCs,       // IN
    PVDIR_SCHEMA_OC_DESC*   ppBottomOC      // OUT
    )
{
    DWORD dwError = 0;
    PVDIR_SCHEMA_OC_DESC pBottomOC = NULL;
    DWORD i = 0;

    for (i = 0; ppStrOCs && ppStrOCs[i]; i++)
    {
        if (!pBottomOC ||
                VmDirSchemaIsAncestorOC(pCtx, ppStrOCs[i], pBottomOC))
        {
            pBottomOC = ppStrOCs[i];
        }
        else if (!VmDirSchemaIsAncestorOC(pCtx, pBottomOC, ppStrOCs[i]))
        {
            VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
            VmDirAllocateStringAVsnprintf(&pCtx->pszErrorMsg,
                    "Entry has incompatible structural objectclass (%s) (%s)",
                    pBottomOC->pszName, ppStrOCs[i]->pszName);

            dwError = pCtx->dwErrorCode = ERROR_INVALID_ENTRY;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    if (!pBottomOC)
    {
        VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
        VmDirAllocateStringAVsnprintf(&pCtx->pszErrorMsg,
                "Entry has no structural objectclass");

        dwError = pCtx->dwErrorCode = ERROR_INVALID_ENTRY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (i = 0; ppAbsOCs && ppAbsOCs[i]; i++)
    {
       if (!VmDirSchemaIsAncestorOC(pCtx, pBottomOC, ppAbsOCs[i]))
       {
           VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
           VmDirAllocateStringAVsnprintf(&pCtx->pszErrorMsg,
                   "Entry has invalid abstract objectclass (%s)",
                   ppAbsOCs[i]->pszName);

           dwError = pCtx->dwErrorCode = ERROR_INVALID_ENTRY;
           BAIL_ON_VMDIR_ERROR(dwError);
       }
    }

    *ppBottomOC = pBottomOC;

error:
    return dwError;
}

static
DWORD
_checkAuxContentRules(
    PVDIR_SCHEMA_CTX        pCtx,           // IN
    PVDIR_SCHEMA_CR_DESC*   pCRs,           // IN
    PVDIR_SCHEMA_OC_DESC*   ppAuxOCs        // IN
    )
{
    DWORD dwError = 0;
    PLW_HASHMAP pAllowedAuxOCMap = NULL;
    DWORD i = 0, j = 0;

    dwError = LwRtlCreateHashMap(&pAllowedAuxOCMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; pCRs && pCRs[i]; i++)
    {
        for (j = 0; pCRs[i]->ppszAuxOCs && pCRs[i]->ppszAuxOCs[j]; j++)
        {
            dwError = LwRtlHashMapInsert(pAllowedAuxOCMap,
                    pCRs[i]->ppszAuxOCs[j], pCRs[i]->ppszAuxOCs[j], NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    for (i = 0; ppAuxOCs && ppAuxOCs[i]; i++)
    {
        if (LwRtlHashMapFindKey(pAllowedAuxOCMap, NULL, ppAuxOCs[i]->pszName))
        {
            VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
            VmDirAllocateStringAVsnprintf(&pCtx->pszErrorMsg,
                    "Aux objectclass (%s) is not allowed.",
                    ppAuxOCs[i]->pszName);

            dwError = pCtx->dwErrorCode = ERROR_INVALID_ENTRY;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    LwRtlHashMapClear(pAllowedAuxOCMap, VmDirNoopHashMapPairFree, NULL);
    LwRtlFreeHashMap(&pAllowedAuxOCMap);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_checkAttributePresences(
    PVDIR_SCHEMA_CTX        pCtx,           // IN
    PVDIR_ENTRY             pEntry,         // IN
    PVDIR_SCHEMA_OC_DESC    pBottomOC,      // IN
    PVDIR_SCHEMA_OC_DESC*   ppAuxOCs        // IN
    )
{
    DWORD dwError = 0;
    PLW_HASHMAP pAllMayAttrMap = NULL;
    PLW_HASHMAP pAllMustAttrMap = NULL;
    PVDIR_ATTRIBUTE pAttr = NULL;

    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};
    DWORD i = 0;

    dwError = LwRtlCreateHashMap(&pAllMayAttrMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(&pAllMustAttrMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _getAllMayAttributes(pCtx, pBottomOC, pAllMayAttrMap);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _getAllMustAttributes(pCtx, pBottomOC, pAllMustAttrMap);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; ppAuxOCs && ppAuxOCs[i]; i++)
    {
        dwError = _getAllMayAttributes(pCtx, ppAuxOCs[i], pAllMayAttrMap);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _getAllMustAttributes(pCtx, ppAuxOCs[i], pAllMustAttrMap);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (pAttr = pEntry->attrs; pAttr; pAttr = pAttr->next)
    {
        if (LwRtlHashMapRemove(pAllMustAttrMap, pAttr->pATDesc->pszName, NULL) &&
            LwRtlHashMapFindKey(pAllMayAttrMap, NULL, pAttr->pATDesc->pszName) &&
            pAttr->pATDesc->usage == VDIR_LDAP_USER_APPLICATIONS_ATTRIBUTE)
        {
            VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
            VmDirAllocateStringAVsnprintf(&pCtx->pszErrorMsg,
                    "Attribute (%s) is not allowed.",
                    pAttr->pATDesc->pszName);

            dwError = pCtx->dwErrorCode = ERROR_INVALID_ENTRY;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    while (LwRtlHashMapIterate(pAllMustAttrMap, &iter, &pair))
    {
        PSTR pszAttrName = (PSTR)pair.pKey;

        // ignore missing "nTSecurityDescriptor" must attribute for now.
        // ADSI needs it to be a must attribute. However, it is NOT
        // easy/clean to make and enforce this change in Lotus.
        // (e.g. in VmDirInteralAddEntry, schema check is called
        //   prior to SD generation currently.)
        // TODO, clean up SD generation in bootstratp/promo/normal paths.
        if (VmDirStringCompareA(
                pszAttrName, ATTR_OBJECT_SECURITY_DESCRIPTOR, FALSE) == 0)
        {
            continue;
        }

        VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
        VmDirAllocateStringAVsnprintf(&pCtx->pszErrorMsg,
                "Missing must attribute (%s)",
                pszAttrName);

        dwError = pCtx->dwErrorCode = ERROR_INVALID_ENTRY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    LwRtlHashMapClear(pAllMayAttrMap, VmDirNoopHashMapPairFree, NULL);
    LwRtlFreeHashMap(&pAllMayAttrMap);
    LwRtlHashMapClear(pAllMustAttrMap, VmDirNoopHashMapPairFree, NULL);
    LwRtlFreeHashMap(&pAllMustAttrMap);
    return dwError;

error:
    goto cleanup;
}

/*
 * reorder value of objectclass in pOCAttr->vals
 * 1. structureOC->parentOC->parentOC->....->TOP(not included)
 *    expand parent objectclass if not supplied.
 * 2. aux OCs
 * We then store its values in this order in backend.
 */
static
DWORD
_reorderObjectClassAttr(
        PVDIR_SCHEMA_CTX        pCtx,
        PVDIR_ATTRIBUTE         pOCAttr,
        PVDIR_SCHEMA_OC_DESC    pStructureOCDesc,
        PVDIR_SCHEMA_OC_DESC*   ppAuxOCDesc)
{
    DWORD dwError = 0;
    PDEQUE pOCDescDeque = NULL;
    PVDIR_SCHEMA_OC_DESC pOCDesc = pStructureOCDesc;
    PVDIR_BERVALUE pBerv = NULL;
    DWORD dwCnt = 0, i = 0;

    dwError = dequeCreate(&pOCDescDeque);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (pOCDesc)
    {
        if (pOCDesc->type != VDIR_LDAP_STRUCTURAL_CLASS)
        {
            break;
        }

        dwError = dequePush(pOCDescDeque, pOCDesc);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirSchemaOCNameToDescriptor(
                pCtx, pOCDesc->pszSup, &pOCDesc);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (i = 0; ppAuxOCDesc && ppAuxOCDesc[i]; i++)
    {
        dwError = dequePush(pOCDescDeque, ppAuxOCDesc[i]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_BERVALUE) * (dequeGetSize(pOCDescDeque) + 1),
            (PVOID*)&pBerv);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (!dequeIsEmpty(pOCDescDeque))
    {
        PCSTR pszOrgName = NULL;

        dwError = dequePopLeft(pOCDescDeque, (PVOID*)&pOCDesc);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (i = 0; i < pOCAttr->numVals; i++)
        {
            if (VmDirStringCompareA(
                    pOCDesc->pszName, pOCAttr->vals[i].lberbv_val, FALSE) == 0)
            {   // keep whatever value provided from clients
                pszOrgName = pOCAttr->vals[i].lberbv_val;
                break;
            }
        }

        dwError = VmDirAllocateStringA(
                pszOrgName ? pszOrgName : pOCDesc->pszName,
                &(pBerv[dwCnt].lberbv_val));
        BAIL_ON_VMDIR_ERROR(dwError);

        pBerv[dwCnt].lberbv_len = VmDirStringLenA(pBerv[dwCnt].lberbv_val);
        pBerv[dwCnt].bOwnBvVal = TRUE;
        dwCnt++;
        // TODO, Do we need to normalize value here?
    }

    VmDirFreeBervalArrayContent(pOCAttr->vals, pOCAttr->numVals);
    VMDIR_SAFE_FREE_MEMORY(pOCAttr->vals);

    pOCAttr->vals = pBerv;
    pOCAttr->numVals = dwCnt;

cleanup:
    dequeFree(pOCDescDeque);
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pBerv);
    goto cleanup;
}

DWORD
VmDirSchemaCheck(
    PVDIR_ENTRY     pEntry
    )
{
    DWORD                   dwError = 0;
    PVDIR_SCHEMA_CTX        pCtx = NULL;
    PVDIR_ATTRIBUTE         pOCAttr = NULL;
    PVDIR_SCHEMA_OC_DESC*   ppStrOCs = NULL;
    PVDIR_SCHEMA_OC_DESC*   ppAuxOCs = NULL;
    PVDIR_SCHEMA_OC_DESC*   ppAbsOCs = NULL;
    PVDIR_SCHEMA_OC_DESC    pBottomOC = NULL;
    PVDIR_SCHEMA_CR_DESC*   ppCRs = NULL;

    if ( !pEntry || !pEntry->pSchemaCtx )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pCtx = pEntry->pSchemaCtx;

    // check all attributes
    dwError = VmDirSchemaCheckSetAttrDesc(pCtx, pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _checkAttributeSyntax(pCtx, pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _checkAttributeDimension(pCtx, pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    // check objectclass attribute
    dwError = _getOCAttr(pCtx, pEntry, &pOCAttr);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _getOCDescs(pCtx, pOCAttr, &ppStrOCs, &ppAuxOCs, &ppAbsOCs);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _checkObjectClassHierarchy(pCtx, ppStrOCs, ppAbsOCs, &pBottomOC);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _getCRDescs(pCtx, pBottomOC, &ppCRs);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _checkAuxContentRules(pCtx, ppCRs, ppAuxOCs);
    BAIL_ON_VMDIR_ERROR(dwError);

    // check attribute presence
    dwError = _checkAttributePresences(pCtx, pEntry, pBottomOC, ppAuxOCs);
    BAIL_ON_VMDIR_ERROR(dwError);

    // reorder objectclass attribute values for optimization
    dwError = _reorderObjectClassAttr(pCtx, pOCAttr, pBottomOC, ppAuxOCs);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(ppStrOCs);
    VMDIR_SAFE_FREE_MEMORY(ppAuxOCs);
    VMDIR_SAFE_FREE_MEMORY(ppAbsOCs);
    VMDIR_SAFE_FREE_MEMORY(ppCRs);
    return dwError;

error:
    goto cleanup;
}

/*
 * Entry schema check dit structure rule
 */
DWORD
VmDirSchemaCheckDITStructure(
    PVDIR_SCHEMA_CTX    pCtx,
    PVDIR_ENTRY         pParentEntry,
    PVDIR_ENTRY         pEntry
    )
{
    DWORD dwError = 0;

    assert(pCtx && pEntry);

    // TODO Not supported yet

    return dwError;
}

DWORD
VmDirSchemaCheckSetAttrDesc(
    PVDIR_SCHEMA_CTX    pCtx,
    PVDIR_ENTRY         pEntry
    )
{
    DWORD dwError = 0;
    VDIR_ATTRIBUTE* pAttr = NULL;

    assert(pCtx && pEntry);

    for (pAttr = pEntry->attrs; pAttr; pAttr = pAttr->next)
    {
        if (!pAttr->pATDesc)
        {
            if (VmDirStringCompareA( pAttr->type.lberbv.bv_val, ATTR_VMW_ORGANIZATION_GUID, FALSE ) == 0)
            {
                continue; // going to delete this attribute any way.
            }
            if (VmDirStringCompareA( pAttr->type.lberbv.bv_val, ATTR_VMW_OBJECT_SECURITY_DESCRIPTOR, FALSE ) == 0)
            {
                pAttr->pATDesc = VmDirSchemaAttrNameToDesc(pCtx, ATTR_OBJECT_SECURITY_DESCRIPTOR);
            }
            else
            {
                pAttr->pATDesc = VmDirSchemaAttrNameToDesc(pCtx, pAttr->type.lberbv.bv_val);
            }
            if (!pAttr->pATDesc)
            {
                VMDIR_SAFE_FREE_MEMORY(pCtx->pszErrorMsg);
                VmDirAllocateStringAVsnprintf(&pCtx->pszErrorMsg,
                        "Attribute (%s) is not defined in schema",
                        VDIR_SAFE_STRING(pAttr->type.lberbv.bv_val));

                dwError = pCtx->dwErrorCode = ERROR_INVALID_ENTRY;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
    }

error:
    return dwError;
}

/*
 * Find the structure objectclass of an entry and get its objectclass descriptor
 * If error, pEntry->pSchemaCtx errorcode/message could have more info.
 */
DWORD
VmDirSchemaGetEntryStructureOCDesc(
    PVDIR_ENTRY             pEntry,
    PVDIR_SCHEMA_OC_DESC*   ppStructureOCDesc       // caller does not own *ppStructureOCDesc
    )
{
    DWORD                   dwError = 0;
    PVDIR_ATTRIBUTE         pObjectClassAttr = NULL;
    PVDIR_SCHEMA_OC_DESC    pOCDesc = NULL;

    if (!pEntry || !pEntry->pSchemaCtx)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _getOCAttr(pEntry->pSchemaCtx, pEntry, &pObjectClassAttr);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Baesd on Objectclass value storing convention, the first
    // objectclass value is leaf structure objectclass.
    // See _reorderObjectClassAttr for details.
    dwError = VmDirSchemaOCNameToDescriptor(
            pEntry->pSchemaCtx,
            pObjectClassAttr->vals[0].lberbv.bv_val,
            &pOCDesc);

    if (dwError == ERROR_NO_SUCH_OBJECTCLASS ||
            pOCDesc->type != VDIR_LDAP_STRUCTURAL_CLASS)
    {
        dwError = ERROR_INVALID_ENTRY;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    // pszStructureOC point into pEntry->attrs content.
    pEntry->pszStructureOC = pObjectClassAttr->vals[0].lberbv.bv_val;
    if (ppStructureOCDesc)
    {
        *ppStructureOCDesc = pOCDesc;
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}
