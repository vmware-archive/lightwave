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



/*
 * Module Name: Directory indexer
 *
 * Filename: init.c
 *
 * Abstract:
 *
 */

#include "includes.h"

/*
 * Sort function -
 * Array of VDIR_CFG_ATTR_INDEX_DESC
 * String compare VDIR_CFG_ATTR_INDEX_DESC.pszAttrName
 */
int
VdirAttrIndexNameCmp(
    const void *p1,
    const void *p2
    )
{
    PVDIR_CFG_ATTR_INDEX_DESC pDesc1 = (PVDIR_CFG_ATTR_INDEX_DESC) p1;
    PVDIR_CFG_ATTR_INDEX_DESC pDesc2 = (PVDIR_CFG_ATTR_INDEX_DESC) p2;

    if ((pDesc1 == NULL || pDesc1->pszAttrName == NULL) &&
        (pDesc2 == NULL || pDesc2->pszAttrName == NULL))
    {
        return 0;
    }

    if (pDesc1 == NULL || pDesc1->pszAttrName == NULL)
    {
        return -1;
    }

    if (pDesc2 == NULL || pDesc2->pszAttrName == NULL)
    {
        return 1;
    }

    return VmDirStringCompareA(pDesc1->pszAttrName, pDesc2->pszAttrName, FALSE);
}

/*
 * Convert attrIndexDesc attribute into a VDIR_ATTR_INDEX_INSTANCE
 * and enable gVdirAttrIndexCache with version 0.
 */
DWORD
VdirAttrIndexInitViaEntry(
    PVDIR_ENTRY  pEntry
    )
{
    DWORD                       dwError = 0;
    DWORD                       dwCnt = 0;
    BOOLEAN                     bInLock = FALSE;
    PVDIR_ATTRIBUTE             pAttr = NULL;
    PVDIR_ATTR_INDEX_INSTANCE   pAttrIdxCache = NULL;
    USHORT                      usLive = gVdirAttrIndexGlobals.usLive;
    PVDIR_SCHEMA_CTX            pSchemaCtx = NULL;

    assert(pEntry);

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);
    assert(pSchemaCtx);

    pAttr = VmDirEntryFindAttribute(
            ATTR_INDEX_DESC,
            pEntry);
    assert(pAttr);

    dwError = VdirAttrIndexCacheAllocate(
            &pAttrIdxCache,
            pAttr->numVals);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt = 0; dwCnt < pAttr->numVals; dwCnt++)
    {
        VDIR_CFG_ATTR_INDEX_DESC indexDesc = {0};

        dwError = VdirstrToAttrIndexDesc(
                pAttr->vals[dwCnt].lberbv.bv_val,
                &indexDesc);
        BAIL_ON_VMDIR_ERROR(dwError);

        // cache takes over indexDesc.pszAttrName
        pAttrIdxCache->pSortName[dwCnt].pszAttrName = indexDesc.pszAttrName;
        indexDesc.pszAttrName = NULL;

        pAttrIdxCache->pSortName[dwCnt].bIsUnique = indexDesc.bIsUnique;
        pAttrIdxCache->pSortName[dwCnt].iTypes = indexDesc.iTypes;
        pAttrIdxCache->pSortName[dwCnt].status = indexDesc.status;
        pAttrIdxCache->pSortName[dwCnt].bIsNumeric = VmDirSchemaAttrHasIntegerMatchingRule(
                                                            pSchemaCtx,
                                                            pAttrIdxCache->pSortName[dwCnt].pszAttrName);

        pAttrIdxCache->pSortName[dwCnt].iId = dwCnt;
    }

    qsort(pAttrIdxCache->pSortName,
          pAttrIdxCache->usNumIndex,
          sizeof(VDIR_CFG_ATTR_INDEX_DESC),
          VdirAttrIndexNameCmp);

    VMDIR_LOCK_MUTEX(bInLock, gVdirAttrIndexGlobals.mutex);

    // Set up a cache instance
    // In case there is no need to do bootstrape during server start up, usLive == 0
    // is not assigned (i.e.: schema entry is found)
    if ((usLive == 0 && gVdirAttrIndexGlobals.pCaches[usLive] != NULL) || usLive > 0)
    {
        usLive++;
    }
    gVdirAttrIndexGlobals.pCaches[usLive] = pAttrIdxCache;
    gVdirAttrIndexGlobals.usLive = usLive;

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVdirAttrIndexGlobals.mutex);

    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }

    return dwError;

error:
    if (pAttrIdxCache)
    {
        VdirAttrIdxCacheFree(pAttrIdxCache);
    }

    goto cleanup;
}

/*
 * Create new cache from another cache + an array of VDIR_CFG_ATTR_INDEX_DESC
 */
DWORD
VdirAttrIndexInitViaCacheAndDescs(
    PVDIR_ATTR_INDEX_INSTANCE   pFromCache,
    PVDIR_CFG_ATTR_INDEX_DESC   pIndexDesc,
    USHORT                      dwDescSize)
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    int     iMaxId = 0;
    BOOLEAN bInLock = FALSE;

    PVDIR_ATTR_INDEX_INSTANCE pCache = NULL;
    PVDIR_ATTR_INDEX_INSTANCE pJunkCache = NULL;

    assert(pFromCache && pIndexDesc);

    dwError = VdirAttrIndexCacheAllocate(
            &pCache,
            (pFromCache->usNumIndex + dwDescSize));
    BAIL_ON_VMDIR_ERROR(dwError);

    // copy all existing data from current cache
    // TODO, need to handle status, so we should only copy ENABLED ones. however,
    // need to be careful for ABORT ones if it appears in pIndexDesc again.  in such
    // case, we need to pick up its old pFromCache->pSortName[dwCnt].iId (bdb db)
    for (dwCnt = 0; dwCnt < pFromCache->usNumIndex; dwCnt++)
    {
        dwError = VmDirAllocateStringA(
                pFromCache->pSortName[dwCnt].pszAttrName,
                &pCache->pSortName[dwCnt].pszAttrName);
        BAIL_ON_VMDIR_ERROR(dwError);

        pCache->pSortName[dwCnt].bIsUnique = pFromCache->pSortName[dwCnt].bIsUnique;
        pCache->pSortName[dwCnt].iTypes = pFromCache->pSortName[dwCnt].iTypes;
        pCache->pSortName[dwCnt].status = pFromCache->pSortName[dwCnt].status;
        pCache->pSortName[dwCnt].iId = pFromCache->pSortName[dwCnt].iId;

        if (pCache->pSortName[dwCnt].iId > iMaxId)
        {
            iMaxId = pCache->pSortName[dwCnt].iId;
        }
    }

    // add new contents from pIndexDesc
    for (dwCnt = 0; dwCnt < dwDescSize; dwCnt++)
    {
        int iIdx = dwCnt + pFromCache->usNumIndex;

        dwError = VmDirAllocateStringA(
                pIndexDesc[dwCnt].pszAttrName,
                &pCache->pSortName[iIdx].pszAttrName);
        BAIL_ON_VMDIR_ERROR(dwError);

        pCache->pSortName[iIdx].bIsUnique = pIndexDesc[dwCnt].bIsUnique;
        pCache->pSortName[iIdx].iTypes = pIndexDesc[dwCnt].iTypes;
        pCache->pSortName[iIdx].status = pIndexDesc[dwCnt].status;
        pCache->pSortName[iIdx].iId = iMaxId + 1 + dwCnt;
    }

    pCache->usNumIndex = pFromCache->usNumIndex + dwDescSize;

    qsort(pCache->pSortName,
          pCache->usNumIndex,
          sizeof(VDIR_CFG_ATTR_INDEX_DESC),
          VdirAttrIndexNameCmp);

    // done create pCache, add it to gVdirAttrIndexGlobals
    VMDIR_LOCK_MUTEX(bInLock, gVdirAttrIndexGlobals.mutex);

    pJunkCache = gVdirAttrIndexGlobals.pNewCache;
    gVdirAttrIndexGlobals.pNewCache = pCache;

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVdirAttrIndexGlobals.mutex);

    if (pJunkCache)
    {
        VdirAttrIdxCacheFree(pJunkCache);
    }

    return dwError;

error:
    if (pCache)
    {
        VdirAttrIdxCacheFree(pCache);
    }

    goto cleanup;
}

DWORD
VdirAttrIndexCacheAllocate(
    PVDIR_ATTR_INDEX_INSTANCE* ppAttrIdxCache,
    USHORT  usIdxSize
    )
{
    DWORD dwError = 0;
    PVDIR_ATTR_INDEX_INSTANCE pAttrIdxCache = NULL;

    assert(ppAttrIdxCache);

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_ATTR_INDEX_INSTANCE),
            (PVOID*)&pAttrIdxCache);
    BAIL_ON_VMDIR_ERROR(dwError);

    pAttrIdxCache->usNumIndex = usIdxSize;

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_CFG_ATTR_INDEX_DESC) * pAttrIdxCache->usNumIndex,
            (PVOID*)&pAttrIdxCache->pSortName);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppAttrIdxCache = pAttrIdxCache;

cleanup:

    return dwError;

error:

    if (pAttrIdxCache)
    {
        VdirAttrIdxCacheFree(pAttrIdxCache);
    }

    *ppAttrIdxCache = NULL;

    goto cleanup;
}


/*
 * Free Attribute Index Cache.
 */
VOID
VdirAttrIdxCacheFree(
    PVDIR_ATTR_INDEX_INSTANCE   pAttrIdxCache
    )
{
    DWORD dwCnt = 0;

    if (pAttrIdxCache)
    {
        for (dwCnt=0; dwCnt < pAttrIdxCache->usNumIndex; dwCnt++)
        {
            VMDIR_SAFE_FREE_MEMORY(pAttrIdxCache->pSortName[dwCnt].pszAttrName);
        }

        VMDIR_SAFE_FREE_MEMORY(pAttrIdxCache->pSortName);
        VMDIR_SAFE_FREE_MEMORY(pAttrIdxCache);
    }

    return;
}

/*
 * Convert AttrIndexDesc content into VDIR_CFG_ATTR_INDEX_DECS
 *
 * NOTE: pszStr content will be change
 */
DWORD
VdirstrToAttrIndexDesc(
    PSTR    pszStr,
    PVDIR_CFG_ATTR_INDEX_DESC   pDesc)
{
    DWORD   dwError = 0;
    PSTR    pToken = NULL;
    PSTR    pRest = NULL;
    PSTR    pSep = " ";
    PSTR    pszLocal = NULL;

    assert (pszStr && pDesc);

    // make a local copy
    dwError = VmDirAllocateStringA(
            pszStr,
            &pszLocal);
    BAIL_ON_VMDIR_ERROR(dwError);

    pDesc->status = VDIR_CFG_ATTR_INDEX_ENABLED;

    pToken = VmDirStringTokA(pszLocal, pSep, &pRest);
    if (!pToken)
    {
        dwError = ERROR_INVALID_CONFIGURATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringA(
            pToken,
            &(pDesc->pszAttrName));
    BAIL_ON_VMDIR_ERROR(dwError);

    for (pToken=VmDirStringTokA(NULL, pSep, &pRest);
        pToken;
        pToken=VmDirStringTokA(NULL, pSep, &pRest))
    {
        if (VmDirStringCompareA(pToken, "eq", FALSE) == 0)
        {
            pDesc->iTypes |= INDEX_TYPE_EQUALITY;
        }
        else if (VmDirStringCompareA(pToken, "sub", FALSE) == 0)
        {
            pDesc->iTypes |= INDEX_TYPE_SUBSTR;
        }
        else if (VmDirStringCompareA(pToken, "pres", FALSE) == 0)
        {
            pDesc->iTypes |= INDEX_TYPE_PRESENCE;
        }
        else if (VmDirStringCompareA(pToken, "approx", FALSE) == 0)
        {
            pDesc->iTypes |= INDEX_TYPE_APPROX;
        }
        else if (VmDirStringCompareA(pToken, "unique", FALSE) == 0)
        {
            pDesc->bIsUnique = TRUE;
        }
        else if (VmDirStringCompareA(pToken, ATTR_INDEX_BUILDING_FLAG, FALSE) == 0)
        {
            //TODO, need to remove building flag off indices entry
            //if crash or power off during indexing, we will get here
            pDesc->status = VDIR_CFG_ATTR_INDEX_ABORTED;
        }
        else
        {
            dwError = ERROR_INVALID_CONFIGURATION;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }



cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszLocal);

    return dwError;

error:

    VMDIR_SAFE_FREE_MEMORY(pDesc->pszAttrName);

    goto cleanup;
}
