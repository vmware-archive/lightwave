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
 * Filename: api.c
 *
 * Abstract:
 *
 * indexer api
 *
 */

#include "includes.h"

static
DWORD
vdirIndexModifyContentCheck(
    VDIR_MODIFICATION*               pMods,
    PVDIR_CFG_ATTR_INDEX_DESC*  ppIndexDesc,
    USHORT*                     pdwSize
    );

static
PVDIR_CFG_ATTR_INDEX_DESC
vdirAttrNameToIndexDesc(
    PCSTR       pszName,        // name of the attribute
    USHORT      usVersion,      // version of index cache to use
    USHORT*     pusVersion,     // version of index cached used in his call
    VDIR_CFG_ATTR_INDEX_USAGE_CTX    idxType
    );

/*
 * Bootstrap attribute index cache to initially open BDB index data files.
 * (During first time server startup, we need this function to create basic
 *  AttrIndexCache in order to write Schema Entry correctly.)
 */
DWORD
VmDirAttrIndexBootStrap(
    VOID
    )
{
    static VDIR_CFG_ATTR_INDEX_DESC idxTbl[] = VDIR_CFG_INDEX_INITIALIZER;

    BOOLEAN bInLock = FALSE;
    DWORD   dwError = 0;
    USHORT  usCnt = 0;
    PVDIR_ATTR_INDEX_INSTANCE   pAttrIdxCache = NULL;
    USHORT usLive = gVdirAttrIndexGlobals.usLive;

    pBootStrapIdxAttrDesc = idxTbl;

    dwError = VdirAttrIndexCacheAllocate(
            &pAttrIdxCache,
            (sizeof(idxTbl)/sizeof(idxTbl[0])) - 1);    // -1 to ignore last dummy entry
    BAIL_ON_VMDIR_ERROR(dwError);

    for (usCnt=0; usCnt < pAttrIdxCache->usNumIndex; usCnt++)
    {
        pAttrIdxCache->pSortName[usCnt].bIsUnique = idxTbl[usCnt].bIsUnique;
        pAttrIdxCache->pSortName[usCnt].bIsNumeric = idxTbl[usCnt].bIsNumeric;
        pAttrIdxCache->pSortName[usCnt].iTypes = idxTbl[usCnt].iTypes;
        pAttrIdxCache->pSortName[usCnt].status = idxTbl[usCnt].status;
        pAttrIdxCache->pSortName[usCnt].iId = usCnt;

        dwError = VmDirAllocateStringA(
                idxTbl[usCnt].pszAttrName,
                &(pAttrIdxCache->pSortName[usCnt].pszAttrName));
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    qsort(pAttrIdxCache->pSortName,
          pAttrIdxCache->usNumIndex,
          sizeof(VDIR_CFG_ATTR_INDEX_DESC),
          VdirAttrIndexNameCmp);

    VMDIR_LOCK_MUTEX(bInLock, gVdirAttrIndexGlobals.mutex);

    // Bootstrap is called for the very first time server starts
    assert(usLive == 0 && !gVdirAttrIndexGlobals.pCaches[usLive]);
    gVdirAttrIndexGlobals.pCaches[usLive] = pAttrIdxCache;

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVdirAttrIndexGlobals.mutex);

    return dwError;

error:
    if (pAttrIdxCache)
    {
        VdirAttrIdxCacheFree(pAttrIdxCache);
    }

    goto cleanup;
}

PVDIR_CFG_ATTR_INDEX_DESC
VmDirAttrNameToReadIndexDesc(
    PCSTR       pszName,        // name of the attribute
    USHORT      usVersion,      // version of index cache to use
    USHORT*     pusVersion      // version of index cached used in his call
    )
{
    return vdirAttrNameToIndexDesc(
                                pszName,
                                usVersion,
                                pusVersion,
                                VDIR_CFG_ATTR_INDEX_READ);
}

PVDIR_CFG_ATTR_INDEX_DESC
VmDirAttrNameToWriteIndexDesc(
    PCSTR       pszName,        // name of the attribute
    USHORT      usVersion,      // version of index cache to use
    USHORT*     pusVersion      // version of index cached used in his call
    )
{
    return vdirAttrNameToIndexDesc(
                                pszName,
                                usVersion,
                                pusVersion,
                                VDIR_CFG_ATTR_INDEX_WRITE);
}

/*
 * Get a list of attribute index in BDB to either (initialize/shutdown)
 * 1. open  index data files or
 * 2. close index data files
 *
 * Caller does NOT own *ppAttrIdxDesc
 */
DWORD
VmDirAttrIndexDescList(
    USHORT*                     pusSize,    // size of *ppAttrIdxDesc
    PVDIR_CFG_ATTR_INDEX_DESC*  ppAttrIdxDesc
    )
{
    DWORD   dwError = 0;
    USHORT  usLive = 0;
    BOOLEAN bInLock = FALSE;
    VDIR_SERVER_STATE vmdirState = VmDirdState();

    assert(pusSize || ppAttrIdxDesc);

    VMDIR_UNLOCK_MUTEX(bInLock, gVdirAttrIndexGlobals.mutex);
    usLive = gVdirAttrIndexGlobals.usLive;

    if (vmdirState != VMDIRD_STATE_STARTUP && vmdirState != VMDIRD_STATE_SHUTDOWN)
    {   // Operation not allowed.
        // This can only be called during startup or shutdown phase.
        dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *pusSize = gVdirAttrIndexGlobals.pCaches[usLive]->usNumIndex;
    *ppAttrIdxDesc = gVdirAttrIndexGlobals.pCaches[usLive]->pSortName;

cleanup:

    VMDIR_UNLOCK_MUTEX(bInLock, gVdirAttrIndexGlobals.mutex);

    return dwError;

error:

    *pusSize = 0;
    *ppAttrIdxDesc = NULL;

    goto cleanup;
}



/*
 * 1. Verify modify contents
 * 2. Create new cache version including new attribute with building flag
 *    (but not yet make this version of cache live)
 * 3. if existing job out standing (i.e. live cache has building flag), reject modify
 */
DWORD
VmDirCFGAttrIndexModifyPrepare(
    VDIR_MODIFICATION*   pMods,
    PVDIR_ENTRY          pEntry
    )
{
    DWORD   dwError = 0;
    USHORT  usModSize = 0;
    DWORD   dwCnt = 0;
    USHORT  usVersion = 0;

    PVDIR_ATTR_INDEX_INSTANCE pCache = NULL;
    PVDIR_CFG_ATTR_INDEX_DESC pIndexDesc = NULL;

    // have free cache slot? have out standing indexing job?
    dwError = VdirIndexingPreCheck(&pCache);
    BAIL_ON_VMDIR_ERROR(dwError);

    // verify pMods has good contents
    dwError = vdirIndexModifyContentCheck(
            pMods,
            &pIndexDesc,  // owns pIndexDesc and its contents
            &usModSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    // verify pMods does not contain existing index
    for (dwCnt = 0; dwCnt < usModSize; dwCnt++)
    {
        PVDIR_CFG_ATTR_INDEX_DESC pCheckDesc = vdirAttrNameToIndexDesc(
                pIndexDesc[dwCnt].pszAttrName,
                usVersion,
                &usVersion,
                VDIR_CFG_ATTR_INDEX_ALL);

        //TODO, need to allow user to re-modify indices entry in case the first
        // modify failed (e.g. constrain violation).  Currently, those failed attribute
        // has status ABORT. so subsequent try to add same attribute will bail here.
        if (pCheckDesc)
        {
            dwError = LDAP_UNWILLING_TO_PERFORM;
            //TODO, log reason at least
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    // append building flag to the end of attribute values
    dwError = VdirIndexingEntryAppendFlag(
            pMods,
            pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    // create new cache (but not yet make it live)
    dwError = VdirAttrIndexInitViaCacheAndDescs(
            pCache,
            pIndexDesc,
            usModSize);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    for (dwCnt = 0; dwCnt < usModSize; dwCnt++)
    {
        VMDIR_SAFE_FREE_MEMORY(pIndexDesc[dwCnt].pszAttrName);
    }
    VMDIR_SAFE_FREE_MEMORY(pIndexDesc);

    return dwError;

error:

    goto cleanup;
}

/*
 * Make new version of cache live
 *
 * NOTE, Modify check and commit calls are serialized at the entry modify level.
 */
VOID
VmDirCFGAttrIndexModifyCommit(
    VOID
    )
{
    BOOLEAN bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVdirAttrIndexGlobals.mutex);

    gVdirAttrIndexGlobals.bIndexInProgress = TRUE;
    // wake up indexing thread, which will enable new cache after necessary db setup
    VmDirConditionSignal(gVdirAttrIndexGlobals.condition);

    VMDIR_UNLOCK_MUTEX(bInLock, gVdirAttrIndexGlobals.mutex);
}

/*
 * allow modify with
 *  - MOD_OP_ADD (i.e. only add new value)
 *  - attribute must be ATTR_INEX_DESC only
 */
static
DWORD
vdirIndexModifyContentCheck(
    VDIR_MODIFICATION*               pMods,
    PVDIR_CFG_ATTR_INDEX_DESC*  ppIndexDesc,
    USHORT*                      pdwSize
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    PVDIR_CFG_ATTR_INDEX_DESC pIndexDesc = NULL;
    PVDIR_SCHEMA_CTX pSchemaCtx = NULL;
    VDIR_MODIFICATION * pIndexMod = NULL;
    VDIR_MODIFICATION * pMod = NULL;

    assert(pMods && ppIndexDesc && pdwSize);

	dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
	BAIL_ON_VMDIR_ERROR(dwError);

    // Make it very restrictive to modify indices entry. Look for THE ONLY index mod
    for (pMod = pMods; pMod != NULL; pMod = pMod->next)
    {
        if (pMod->operation == MOD_OP_ADD && (0 == VmDirStringCompareA(pMod->attr.type.lberbv.bv_val, ATTR_INDEX_DESC, FALSE)))
        {
            if (pIndexMod != NULL)
            {
                dwError = LDAP_UNWILLING_TO_PERFORM;
                //TODO, log reason at least
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            else
            {
                pIndexMod = pMod;
            }
        }
    }

    if (pIndexMod == NULL)
    {
        dwError = LDAP_UNWILLING_TO_PERFORM;
        //TODO, log reason at least
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_CFG_ATTR_INDEX_DESC) * (pIndexMod->attr.numVals),
            (PVOID)&pIndexDesc);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt = 0; dwCnt < pIndexMod->attr.numVals; dwCnt++)
    {
        // convert value to VDIR_CFG_ATTR_INDEX_DESC
        dwError = VdirstrToAttrIndexDesc(
                pIndexMod->attr.vals[dwCnt].lberbv.bv_val,
                &pIndexDesc[dwCnt]);
        BAIL_ON_VMDIR_ERROR(dwError);

        // set status to building
        pIndexDesc[dwCnt].status = VDIR_CFG_ATTR_INDEX_BUILDING;

        if (!VmDirSchemaAttrNameToDesc(pSchemaCtx, pIndexDesc[dwCnt].pszAttrName))
        {
            dwError = ERROR_NO_SUCH_ATTRIBUTE;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    *ppIndexDesc = pIndexDesc;
    *pdwSize = pIndexMod->attr.numVals;

cleanup:

    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }

    return dwError;

error:

    if (pIndexMod)
    {
        for (dwCnt = 0; dwCnt < pIndexMod->attr.numVals; dwCnt++)
        {
            VMDIR_SAFE_FREE_MEMORY(pIndexDesc[dwCnt].pszAttrName);
        }
    }
    VMDIR_SAFE_FREE_MEMORY(pIndexDesc);

    goto cleanup;
}

/*
 * 1. Query attribute index cache based on pszName.
 * 2. Use cache version "usVersion" to perform the query
 * 3. (*pusVersion) is set if usVersion=0, so subsequent call can use it as the
 *    second parameter - usVersion.  (to minimize mutex lock)
 */
static
PVDIR_CFG_ATTR_INDEX_DESC
vdirAttrNameToIndexDesc(
    PCSTR       pszName,        // name of the attribute
    USHORT      usVersion,      // version of index cache to use
    USHORT*     pusVersion,     // version of index cached used in his call
    VDIR_CFG_ATTR_INDEX_USAGE_CTX    idxType
    )
{
    BOOLEAN     bInLock = FALSE;
    USHORT      usLive  = usVersion;
    PVDIR_CFG_ATTR_INDEX_DESC   pAttrIdxDesc = NULL;
    VDIR_CFG_ATTR_INDEX_DESC    key = {0};

    if (usVersion == 0)
    {   // Caller does not know the current live version, query for them.
        VMDIR_LOCK_MUTEX(bInLock, gVdirAttrIndexGlobals.mutex);
        usLive = gVdirAttrIndexGlobals.usLive;
        VMDIR_UNLOCK_MUTEX(bInLock, gVdirAttrIndexGlobals.mutex);
    }

    assert(gVdirAttrIndexGlobals.pCaches[usLive]);

    key.pszAttrName = (PSTR)pszName;
    pAttrIdxDesc = (PVDIR_CFG_ATTR_INDEX_DESC) bsearch(
            &key,
            gVdirAttrIndexGlobals.pCaches[usLive]->pSortName,
            gVdirAttrIndexGlobals.pCaches[usLive]->usNumIndex,
            sizeof(VDIR_CFG_ATTR_INDEX_DESC),
            VdirAttrIndexNameCmp);

    if (pAttrIdxDesc)
    {
        if (idxType == VDIR_CFG_ATTR_INDEX_READ &&
            pAttrIdxDesc->status != VDIR_CFG_ATTR_INDEX_ENABLED)
        {
            pAttrIdxDesc = NULL;
        }

        if (idxType == VDIR_CFG_ATTR_INDEX_WRITE                &&
            pAttrIdxDesc->status != VDIR_CFG_ATTR_INDEX_ENABLED &&
            pAttrIdxDesc->status != VDIR_CFG_ATTR_INDEX_BUILDING)
        {
            pAttrIdxDesc = NULL;
        }
    }

    if (pusVersion)
    {
        *pusVersion = usLive;
    }

    return pAttrIdxDesc;
}
