/*
 * Copyright © 2019 VMware, Inc.  All Rights Reserved.
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
_VmDirSetSearchPriorityMap(
    PLW_HASHMAP     pSearchTypePriMap,
    PLW_HASHMAP     pAttrTypePriMap
    );

static
VOID
_VmDirFreeFilterTypeArray(
    PVDIR_FILTER_TYPE_PRI   pArray
    );

static
VOID
_VmDirFreeAttrTypeArray(
    PVDIR_ATTR_TYPE_PRI     pArray
    );

// Input string format, e.g. "163:5"
// TODO, like to see "LDAP_FILTER_EQUALITY:5" instead
static
DWORD
_VmDirParseSearchTypes(
    PSTR    pszInput,
    int *   piSearchType,
    int *   piPri
    )
{
    PSTR    pszPos = NULL;
    DWORD   dwError = 0;

    pszPos = VmDirStringChrA(pszInput, ':');
    if (!pszPos)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_CONFIGURATION);
    }

    *pszPos = '\0';
    *piSearchType = VmDirStringToIA(pszInput);
    pszPos++;

    *piPri = VmDirStringToIA(pszPos);

cleanup:
    return dwError;

error:
    goto cleanup;
}

// Input string format, e.g. "sAMAccountName:1"
static
DWORD
_VmDirParseAttrTypes(
    PSTR    pszInput,
    PSTR*   ppszAt,
    int *   piPri
    )
{
    PSTR    pszPos = NULL;
    DWORD   dwError = 0;
    PSTR    pszAt = NULL;

    *ppszAt = NULL;
    *piPri = 0;

    pszPos = VmDirStringChrA(pszInput, ':');
    if (!pszPos)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_CONFIGURATION);
    }

    *pszPos = '\0';
    dwError = VmDirAllocateStringA(pszInput, &pszAt);
    BAIL_ON_VMDIR_ERROR(dwError);
    *ppszAt = pszAt;

    pszPos++;
    *piPri = VmDirStringToIA(pszPos);
cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmDirReadIteratorPriMapEntry(
    PVDIR_FILTER_TYPE_PRI*  ppSearchTypesPri,
    PVDIR_ATTR_TYPE_PRI*    ppAttrTypesPri
    )
{
    VDIR_ENTRY_ARRAY        entryArray = {0};
    PVDIR_ATTRIBUTE         pAttr = NULL;
    PVDIR_FILTER_TYPE_PRI   pSearchTypesPri = NULL;
    PVDIR_ATTR_TYPE_PRI     pAttrTypesPri = NULL;
    DWORD   dwError = 0;
    int     i = 0;

    dwError = VmDirSimpleEqualFilterInternalSearch(
            CFG_ITERATION_MAP_DN,
            LDAP_SCOPE_BASE,
            ATTR_OBJECT_CLASS,
            OC_ITERATOR_PRI_MAP,
            &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (entryArray.iSize == 0)
    {
       BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND);
    }

    pAttr =  VmDirEntryFindAttribute(ATTR_SEARCH_TYPE_PRI, entryArray.pEntry);
    if (pAttr)
    {
        dwError = VmDirAllocateMemory((pAttr->numVals+1) * sizeof(VDIR_FILTER_TYPE_PRI), (PVOID*)&pSearchTypesPri);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (i = 0; i < pAttr->numVals; i++)
        {
            dwError = _VmDirParseSearchTypes(
                        pAttr->vals[i].lberbv.bv_val,
                        &pSearchTypesPri[i].iFilterType,
                        &pSearchTypesPri[i].iPri);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    pAttr =  VmDirEntryFindAttribute(ATTR_ATTR_TYPE_PRI, entryArray.pEntry);
    if (pAttr)
    {
        dwError = VmDirAllocateMemory((pAttr->numVals+1) * sizeof(VDIR_ATTR_TYPE_PRI), (PVOID*)&pAttrTypesPri);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (i = 0; i < pAttr->numVals; i++)
        {
            dwError = _VmDirParseAttrTypes(
                      pAttr->vals[i].lberbv.bv_val,
                      &pAttrTypesPri[i].pszAttrType,
                      &pAttrTypesPri[i].iPri);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    *ppSearchTypesPri = pSearchTypesPri;
    *ppAttrTypesPri = pAttrTypesPri;

cleanup:
    VmDirFreeEntryArrayContent(&entryArray);
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pSearchTypesPri);
    VMDIR_SAFE_FREE_MEMORY(pAttrTypesPri);
    goto cleanup;
}

static
DWORD
_VmDirAddDefaultIteratorPriMapEntry(
    VOID
    )
{
    DWORD   dwError = 0;
    PVDIR_SCHEMA_CTX pSchemaCtx = NULL;
    int     iAttrCnt = 0;
    PSTR *  ppAttributes = NULL;
    int     i = 0;
    int     j = 0;

    // The final priority is the matching value in this table, plus the base value UNIQ_ATTR_BASE_PRI
    //   when the index is globally unique or, NON_UNIQ_ATTR_BASE_PRI. if not, the base value is overridden if the
    //   attribute type is found in attrTypesPri.
    VDIR_FILTER_TYPE_PRI searchTypesPri[] =
        {
            {LDAP_FILTER_EQUALITY   /* 163 */,  5},
            {LDAP_FILTER_LE         /* 166 */,  -20},   /* disable */
            {LDAP_FILTER_GE         /* 165 */,  3},
            {FILTER_ONE_LEVEL_SEARCH /* 0 */,   1},
            {LDAP_FILTER_SUBSTRINGS /* 164 */,  1},
            {LDAP_FILTER_PRESENT    /* 135 */,  -20},   /* disable, use one/sub instead */
            {0,                         0}
        };

    VDIR_ATTR_TYPE_PRI attrTypesPri[] =
        {
                {ATTR_PARENT_ID,        2},
                {ATTR_SAM_ACCOUNT_NAME, 1},
                {ATTR_OBJECT_CLASS,     -20},   /* disable objectclass on iterator */
                {NULL,                  0}
        };

    PSTR ppDefaultAttrs[] =
        {
             ATTR_OBJECT_CLASS, OC_TOP,
             ATTR_OBJECT_CLASS, OC_ITERATOR_PRI_MAP,
             ATTR_CN,           CFG_ITERATION_MAP,
             NULL
        };

    iAttrCnt = sizeof(ppDefaultAttrs)/sizeof(PSTR) + \
                 2 * sizeof(searchTypesPri)/sizeof(VDIR_FILTER_TYPE_PRI) + \
                 2 * sizeof(attrTypesPri)/sizeof(VDIR_ATTR_TYPE_PRI) + 2;

    dwError = VmDirAllocateMemory(iAttrCnt*sizeof(PSTR), (PVOID*)&ppAttributes);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (j=0; ppDefaultAttrs[j]; j++)
    {
        dwError = VmDirAllocateStringPrintf(&ppAttributes[i++], "%s", ppDefaultAttrs[j]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (j=0; searchTypesPri[j].iPri; j++)
    {
        dwError = VmDirAllocateStringPrintf(&ppAttributes[i++], "%s", ATTR_SEARCH_TYPE_PRI);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringPrintf(&ppAttributes[i++],
                "%d:%d", searchTypesPri[j].iFilterType, searchTypesPri[j].iPri);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (j=0; attrTypesPri[j].iPri; j++)
    {
        dwError = VmDirAllocateStringPrintf(&ppAttributes[i++], "%s", ATTR_ATTR_TYPE_PRI);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringPrintf(&ppAttributes[i++], "%s:%d",
                attrTypesPri[j].pszAttrType, attrTypesPri[j].iPri);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    ppAttributes[i] = NULL;

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSimpleEntryCreate(
                    pSchemaCtx,
                    ppAttributes,
                    CFG_ITERATION_MAP_DN,
                    0);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "%s: succeeded", __func__);

cleanup:
    for (i=0; ppAttributes[i]; i++)
    {
       VMDIR_SAFE_FREE_MEMORY(ppAttributes[i]);
    }
    VMDIR_SAFE_FREE_MEMORY(ppAttributes);

    VmDirSchemaCtxRelease(pSchemaCtx);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s: error %d", __func__, dwError);
    goto cleanup;
}

static
DWORD
_VmDirSetSearchPriorityMap(
    PLW_HASHMAP     pSearchTypePriMap,
    PLW_HASHMAP     pAttrTypePriMap
    )
{
    DWORD   dwError = 0;
    DWORD   dwIndex = 0;
    PLW_HASHMAP     pLocalSearchMap = NULL;
    PLW_HASHMAP     pLocalAttrMap   = NULL;

    if (!gVmdirServerGlobals.searchOptMap.bMapLoaded)
    {
        memset(&gVmdirServerGlobals.searchOptMap, 0, sizeof(gVmdirServerGlobals.searchOptMap));

        dwError = VmDirAllocateMemory(
                sizeof(PVOID) * VMDIR_SEARCH_MAP_CACHE_SIZE,
                (PVOID)&gVmdirServerGlobals.searchOptMap.ppSearchTypePriMap);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateMemory(
                sizeof(PVOID) * VMDIR_SEARCH_MAP_CACHE_SIZE,
                (PVOID)&gVmdirServerGlobals.searchOptMap.ppAttrTypePriMap);
        BAIL_ON_VMDIR_ERROR(dwError);

        gVmdirServerGlobals.searchOptMap.ppSearchTypePriMap[0] = pSearchTypePriMap;
        gVmdirServerGlobals.searchOptMap.ppAttrTypePriMap[0] = pAttrTypePriMap;
        gVmdirServerGlobals.searchOptMap.bMapLoaded = TRUE;
    }
    else
    {
        PLW_HASHMAP *ppSearchTypePriMap = gVmdirServerGlobals.searchOptMap.ppSearchTypePriMap;
        PLW_HASHMAP *ppAttrTypePriMap   = gVmdirServerGlobals.searchOptMap.ppAttrTypePriMap;

        {   // section to atomically point cache to latest version
            LwInterlockedIncrement64(&gVmdirServerGlobals.searchOptMap.iNext);

            dwIndex = gVmdirServerGlobals.searchOptMap.iNext % VMDIR_SEARCH_MAP_CACHE_SIZE;
            assert(dwIndex != gVmdirServerGlobals.searchOptMap.iCurrent);

            pLocalSearchMap = ppSearchTypePriMap[dwIndex];
            pLocalAttrMap   = ppAttrTypePriMap[dwIndex];

            ppSearchTypePriMap[dwIndex] = pSearchTypePriMap;
            ppAttrTypePriMap[dwIndex]   = pAttrTypePriMap;

            LwInterlockedExchange64(&gVmdirServerGlobals.searchOptMap.iCurrent, dwIndex);
        }

        if (pLocalSearchMap)
        {
            LwRtlHashMapClear(pLocalSearchMap, VmDirSimpleHashMapPairFree, NULL);
            LwRtlFreeHashMap(&pLocalSearchMap);
        }

        if (pLocalAttrMap)
        {
            LwRtlHashMapClear(pLocalAttrMap, VmDirSimpleHashMapPairFree, NULL);
            LwRtlFreeHashMap(&pLocalAttrMap);
        }
    }

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: curent serach priority map index (%d)", __func__, dwIndex);

error:
    return dwError;
}

/*
 * Start up time function to load "cn=iteratorprimap,cn=config" into cache for iterator based
 * cost evaluation.
 */
DWORD
VmDirLoadSearchPriorityMap(
    VOID
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    DWORD   dwPropertyCnt = 0;
    PVDIR_INDEX_PROPERTY    pIndexProperty = NULL;
    PVDIR_FILTER_TYPE_PRI   pSearchTypesPri = NULL;
    PVDIR_ATTR_TYPE_PRI     pAttrTypesPri = NULL;
    PLW_HASHMAP         pSearchTypePriMap = NULL;
    PLW_HASHMAP         pAttrTypePriMap = NULL;
    PSTR                pszKey = NULL;
    PSTR                pszVal = NULL;

    if (gVmdirGlobals.dwEnableSearchOptimization == 0)
    {
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: Search optimization disabled", __func__);
        goto cleanup;
    }

    dwError = LwRtlCreateHashMap(
            &pSearchTypePriMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(
            &pAttrTypePriMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirReadIteratorPriMapEntry(&pSearchTypesPri, &pAttrTypesPri);
    if (dwError == VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND)
    {
        dwError = _VmDirAddDefaultIteratorPriMapEntry();
        if (dwError == VMDIR_ERROR_NO_SUCH_ATTRIBUTE)
        {
            VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL, "%s: iterator search optimization needs schema upgrade.", __func__);
            dwError = 0;    // TODO, should just load default value into cache instead?
            goto cleanup;
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _VmDirReadIteratorPriMapEntry(&pSearchTypesPri, &pAttrTypesPri);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirIndexMapGetProperty(&pIndexProperty);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwPropertyCnt= 0; pIndexProperty[dwPropertyCnt].pszName; dwPropertyCnt++)
    {
        PSTR    pszAttr = NULL;
        int     iPriVal = 0;

        pszAttr = pIndexProperty[dwPropertyCnt].pszName;
        iPriVal = pIndexProperty[dwPropertyCnt].bGlobalUnique ? UNIQ_ATTR_BASE_PRI : NON_UNIQ_ATTR_BASE_PRI;

        for (dwCnt=0; (pAttrTypesPri && pAttrTypesPri[dwCnt].pszAttrType); dwCnt++)
        {
            if (VmDirStringCompareA(pszAttr, pAttrTypesPri[dwCnt].pszAttrType, FALSE) == 0)
            {
                iPriVal += pAttrTypesPri[dwCnt].iPri;
                break;
            }
        }

        dwError = VmDirAllocateStringPrintf(&pszKey, "%s", pszAttr);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringPrintf(&pszVal, "%d", iPriVal);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapInsert(pAttrTypePriMap, pszKey, pszVal, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: add key:value %s: %s onto AttrTypePriMap", __func__, pszKey, pszVal);
    }

    for (dwCnt=0;
         pSearchTypesPri && (pSearchTypesPri[dwCnt].iFilterType || pSearchTypesPri[dwCnt].iPri) ;
         dwCnt++)
    {
        dwError = VmDirAllocateStringPrintf(&pszKey, "%d", pSearchTypesPri[dwCnt].iFilterType);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringPrintf(&pszVal, "%d", pSearchTypesPri[dwCnt].iPri);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapInsert(pSearchTypePriMap, pszKey, pszVal, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: add key:value %s: %s onto SearchTypePriMap", __func__, pszKey, pszVal);
    }

    dwError = _VmDirSetSearchPriorityMap(pSearchTypePriMap, pAttrTypePriMap);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    _VmDirFreeFilterTypeArray(pSearchTypesPri);
    _VmDirFreeAttrTypeArray(pAttrTypesPri);
    VmDirIndexMapFreeProperty(pIndexProperty);

    return dwError;

error:
    if (pSearchTypePriMap)
    {
        LwRtlHashMapClear(pSearchTypePriMap, VmDirSimpleHashMapPairFree, NULL);
        LwRtlFreeHashMap(&pSearchTypePriMap);
    }

    if (pAttrTypePriMap)
    {
        LwRtlHashMapClear(pAttrTypePriMap, VmDirSimpleHashMapPairFree, NULL);
        LwRtlFreeHashMap(&pAttrTypePriMap);
    }

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: error %d", __func__, dwError);
    goto cleanup;
}

/*
 * Search global map that is currently loaded with static rules.
 */
int
VmDirGetSearchPri(
    PVDIR_SEARCHOPT_PARAM pOptParm
    )
{
    int     iTotalPri = 0;
    PSTR    pszFixedPri = NULL;
    char    key[VMDIR_SIZE_128]= {0};
    DWORD   dwError = 0;

    if (!pOptParm)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (pOptParm->bPagedSearch || pOptParm->iSizeLimit|| pOptParm->iTimeLimit)
    {
        iTotalPri = PAGED_OR_LIMIT_RAISED_PRI;
    }

    dwError = VmDirStringNPrintFA(key, sizeof(key), sizeof(key)-1,  "%d", pOptParm->iSearchType);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (LwRtlHashMapFindKey(
            gVmdirServerGlobals.searchOptMap.ppSearchTypePriMap[
                gVmdirServerGlobals.searchOptMap.iCurrent],
            (PVOID*)&pszFixedPri,
            key) == 0)
    {
        iTotalPri += atoi(pszFixedPri);
    }

    dwError = VmDirStringNPrintFA(key, sizeof(key), sizeof(key)-1,  "%s", pOptParm->pszAttrType);
    BAIL_ON_VMDIR_ERROR(dwError);

    pszFixedPri = NULL;
    if (LwRtlHashMapFindKey(
            gVmdirServerGlobals.searchOptMap.ppAttrTypePriMap[
                gVmdirServerGlobals.searchOptMap.iCurrent],
            (PVOID*)&pszFixedPri,
            key) == 0)
    {
        iTotalPri += atoi(pszFixedPri);
    }

cleanup:
    return iTotalPri;

error:
    iTotalPri = 0;
    goto cleanup;
}

static
VOID
_VmDirFreeFilterTypeArray(
    PVDIR_FILTER_TYPE_PRI   pArray
    )
{
    if (pArray)
    {
        VMDIR_SAFE_FREE_MEMORY(pArray);
    }
}

static
VOID
_VmDirFreeAttrTypeArray(
    PVDIR_ATTR_TYPE_PRI     pArray
    )
{
    PVDIR_ATTR_TYPE_PRI   pBase = pArray;

    if (pArray)
    {
        while (pBase->pszAttrType)
        {
            VMDIR_SAFE_FREE_MEMORY(pBase->pszAttrType);
            pBase++;
        }
        VMDIR_SAFE_FREE_MEMORY(pArray);
    }
}
