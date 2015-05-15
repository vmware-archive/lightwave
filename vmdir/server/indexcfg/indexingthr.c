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
 * Filename: indexingthr.c
 *
 * Abstract:
 *
 */

#include "includes.h"

static
DWORD
vdirIndexingThrFun(
    PVOID   pArg);

static
DWORD
vdirIndexingWorkerThrFun(
    PVOID   pArg);

static
DWORD
vdirIndexingFinish(
    PVDIR_CFG_ATTR_INDEX_DESC* ppIndexDesc,
    BOOLEAN                    bCommit);

static
DWORD
vdirIndexingEntryFinish(
    BOOLEAN bCommit);

DWORD
VdirIndexingPreCheck(
    PVDIR_ATTR_INDEX_INSTANCE* ppCache)
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    USHORT  usVersion = 0;
    BOOLEAN bIndexInProgress = FALSE;
    PVDIR_ATTR_INDEX_INSTANCE pCache = NULL;

    VMDIR_LOCK_MUTEX(bInLock, gVdirAttrIndexGlobals.mutex);

    usVersion = gVdirAttrIndexGlobals.usLive;
    pCache = gVdirAttrIndexGlobals.pCaches[usVersion];
    bIndexInProgress = gVdirAttrIndexGlobals.bIndexInProgress;

    VMDIR_UNLOCK_MUTEX(bInLock, gVdirAttrIndexGlobals.mutex);

    // no more reserved cache slot available
    if (usVersion >= MAX_ATTR_INDEX_CACHE_INSTANCE - 1)
    {
        dwError = LDAP_UNWILLING_TO_PERFORM;
        //TODO, log more meaningful error
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // have out standing indexing job
    if (bIndexInProgress)
    {
        dwError = LDAP_UNWILLING_TO_PERFORM;
        //TODO, log more meaningful error
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (ppCache)
    {
        *ppCache = pCache;
    }

error:

    return dwError;
}

/*
 * Tweak indices entry to append building flag to newly added values.
 * e.g. user add a new index attribute "myuid eq unique"
 * we want to save it as "myuid eq unique (buildingflag)" instead.
 */
DWORD
VdirIndexingEntryAppendFlag(
    VDIR_MODIFICATION*   pMods,
    PVDIR_ENTRY          pEntry)
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    PSTR    pszNewStr = NULL;
    PVDIR_ATTRIBUTE  pAttr = NULL;

    assert(pMods && pEntry && pEntry->allocType == ENTRY_STORAGE_FORMAT_NORMAL);


    // unpack entry, so we can manipulate its contents.
    dwError = VmDirEntryUnpack(pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    pAttr = VmDirFindAttrByName(pEntry, ATTR_INDEX_DESC);
    if (!pAttr)
    {
        dwError = ERROR_INVALID_ENTRY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (dwCnt = 0; dwCnt < pAttr->numVals; dwCnt++)
    {
        DWORD iIdx = 0;
        for (iIdx = 0; iIdx < pMods->attr.numVals; iIdx++)
        {
            if (0 == VmDirStringCompareA(pAttr->vals[dwCnt].lberbv.bv_val, pMods->attr.vals[iIdx].lberbv.bv_val, FALSE))
            {
                dwError = VmDirAllocateStringAVsnprintf(
                        &pszNewStr,
                        "%s %s",
                        pAttr->vals[dwCnt].lberbv.bv_val,
                        ATTR_INDEX_BUILDING_FLAG);
                BAIL_ON_VMDIR_ERROR(dwError);

                VmDirFreeBervalArrayContent(&pAttr->vals[dwCnt], 1);

                pAttr->vals[dwCnt].lberbv.bv_val = pszNewStr;
                pszNewStr = NULL;
                pAttr->vals[dwCnt].bOwnBvVal = TRUE;
                pAttr->vals[dwCnt].lberbv.bv_len = VmDirStringLenA(pAttr->vals[dwCnt].lberbv.bv_val);

                dwError = VmDirSchemaBervalNormalize(
                        pEntry->pSchemaCtx,
                        pAttr->pATDesc,
                        &pAttr->vals[dwCnt]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
    }

cleanup:

    return dwError;

error:

    VMDIR_SAFE_FREE_MEMORY(pszNewStr);

    goto cleanup;
}

DWORD
InitializeIndexingThread(
    void)
{
    DWORD       dwError = 0;
    PVDIR_THREAD_INFO   pThrInfo = NULL;

    dwError = VmDirAllocateMemory(
            sizeof(*pThrInfo),
            (PVOID)&pThrInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSrvThrInit(
            pThrInfo,
            gVdirAttrIndexGlobals.mutex,       // alternative mutex
            gVdirAttrIndexGlobals.condition,   // alternative cond
            TRUE);                              // join by main thr

    dwError = VmDirCreateThread(
            &pThrInfo->tid,
            FALSE,
            vdirIndexingThrFun,
            pThrInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSrvThrAdd(pThrInfo);

cleanup:

    return dwError;

error:

    if (pThrInfo)
    {
        VmDirSrvThrFree(pThrInfo);
    }

    goto cleanup;
}

/*
 * This thread waits on gVdirAttrIndexGlobals.condition and spawns worker thread
 * to perform indexing in the back ground.
 */
static
DWORD
vdirIndexingThrFun(
    PVOID   pArg
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    PVDIR_CFG_ATTR_INDEX_DESC*  ppIndexDesc = NULL;
    PVDIR_THREAD_INFO           pThrInfo = (PVDIR_THREAD_INFO)pArg;

    while (1)
    {
        DWORD   dwCnt = 0;
        DWORD   dwSize = 0;
        VMDIR_THREAD  tid = {0};
        PVDIR_ATTR_INDEX_INSTANCE pCache = NULL;

        memset(&tid, 0, sizeof(tid));

        // wait till new indices are created
        VMDIR_LOCK_MUTEX(bInLock, gVdirAttrIndexGlobals.mutex);

        dwError = VmDirConditionWait(
                pThrInfo->conditionUsed,
                pThrInfo->mutexUsed);
        BAIL_ON_VMDIR_ERROR(dwError);

        // get new cache we want to enable
        pCache = gVdirAttrIndexGlobals.pNewCache;

        VMDIR_UNLOCK_MUTEX(bInLock, gVdirAttrIndexGlobals.mutex);

        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            break;
        }

        if (!pCache)
        {
            continue;
        }

        // get number of indices in building status
        for (dwCnt = 0, dwSize = 0; dwCnt < pCache->usNumIndex; dwCnt++)
        {
            if (pCache->pSortName[dwCnt].status == VDIR_CFG_ATTR_INDEX_BUILDING)
            {
                dwSize++;
            }
        }

        if (dwSize == 0)
        {
            continue;
        }

        dwError  = VmDirAllocateMemory(
                sizeof(PVDIR_CFG_ATTR_INDEX_DESC) * (dwSize + 1),  // +1 for NULL ending
                (PVOID)&ppIndexDesc);
        BAIL_ON_VMDIR_ERROR(dwError);

        // fill ppIndexDesc with building index
        // NOTE, ppIndexDesc does NOT own them; pCache does.
        for (dwCnt = 0, dwSize = 0; dwCnt < pCache->usNumIndex; dwCnt++)
        {
            if (pCache->pSortName[dwCnt].status == VDIR_CFG_ATTR_INDEX_BUILDING)
            {
                ppIndexDesc[dwSize] = &pCache->pSortName[dwCnt];
                dwSize++;
            }
        }

        // TODO, detach thr now, need to join to safely finish bdb io...
        dwError = VmDirCreateThread(
                &tid,
                TRUE,
                vdirIndexingWorkerThrFun,
                (PVOID)ppIndexDesc);
        BAIL_ON_VMDIR_ERROR(dwError);
        // worker takes over ppIndexDesc.
        ppIndexDesc = NULL;

        VmDirFreeVmDirThread(&tid);
    }

cleanup:

    VMDIR_UNLOCK_MUTEX(bInLock, gVdirAttrIndexGlobals.mutex);

    // TODO: should be dwError?
    return 0;

error:

    VMDIR_SAFE_FREE_MEMORY(ppIndexDesc);

    //TODO, should we stop vmdird?

    VmDirLog( LDAP_DEBUG_ANY, "VmdirIndexingThr: error stop" );

    goto cleanup;
}


/*
 * Main indexing worker thread.
 * Loop through entry db to create new indices.
 *
 * Now runs in single thr mode.  TODO, make # of sub worker configurable.
 */
static
DWORD
vdirIndexingWorkerThrFun(
    PVOID   pArg)
{
    //TODO, Disable indexing during backend interface reorg. need to re-think how we define indices
    vdirIndexingFinish(NULL, TRUE);
    return 0;
/*
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    DWORD   dwNumIndices = 0;
    BOOLEAN bInLock = FALSE;

    ENTRYID    ulMaxId = 0;
    ENTRYID    ulCurrentId = 0;

    // owns ppIndexDesc but not it contents.
    PVDIR_CFG_ATTR_INDEX_DESC* ppIndexDesc = (PVDIR_CFG_ATTR_INDEX_DESC*)pArg;

    // handle backend db file open/create for new index attribute
    for (dwCnt=0, dwNumIndices=0; ppIndexDesc[dwCnt]; dwCnt++, dwNumIndices++)
    {   // fill one gVdirBdbGlobals.bdbIndexDBs.pIndexDBs and create/open bdb data file
        dwError = VmDirBDBGlobalIndexStructAdd(ppIndexDesc[dwCnt]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // make new cache live after backend setup is done
    VMDIR_LOCK_MUTEX(bInLock, gVdirAttrIndexGlobals.mutex);

    assert(gVdirAttrIndexGlobals.pNewCache);
    gVdirAttrIndexGlobals.pCaches[gVdirAttrIndexGlobals.usLive+1] = gVdirAttrIndexGlobals.pNewCache;
    gVdirAttrIndexGlobals.pNewCache = NULL;
    gVdirAttrIndexGlobals.usLive++;

    VMDIR_UNLOCK_MUTEX(bInLock, gVdirAttrIndexGlobals.mutex);

    // get the max entry id in current data store
    dwError = BdbNextEntryId(&ulMaxId);
    BAIL_ON_VMDIR_ERROR(dwError);

    ulCurrentId = 0;
    while (VmDirdState() != VMDIR_SHUTDOWN && ulCurrentId <= ulMaxId)
    {
        //TODO, use data store interface to remove BDB tie
        // batch the job in ENTRY_INDEXING_BATCH_SIZE (25) entries size
        dwError = VmDirBdbIndicesCreate(
                ppIndexDesc,
                dwNumIndices,
                ulCurrentId,
                ENTRY_INDEXING_BATCH_SIZE);
        BAIL_ON_VMDIR_ERROR(dwError);

        ulCurrentId += ENTRY_INDEXING_BATCH_SIZE;
    }

    if (ulCurrentId > ulMaxId)
    {
        dwError = vdirIndexingFinish(ppIndexDesc, TRUE);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = ERROR_OPERATION_INTERRUPT;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:

    // free ppIndexDesc but not its contents, which owns by gVdirBdbGlobals.*
    VMDIR_SAFE_FREE_MEMORY(ppIndexDesc);

    return NULL;

error:

    vdirIndexingFinish(ppIndexDesc, FALSE);

    VmDirLog( LDAP_DEBUG_ANY, "VmdirIndexingWorkerThr: error %d", dwError );

    goto cleanup;
*/
}

/*
 * We are done indexing, make change to cache and indices entry.
 *
 * if indexing success -
 * 1. make status in cache ENABLED
 * 2. remove building flag in indices entry
 * if indexing fail -
 * 1. remove values in indices entry
 * 2. reset status in gVdirAttrIndexGlobals cache
 */
static
DWORD
vdirIndexingFinish(
    PVDIR_CFG_ATTR_INDEX_DESC*  ppIndexDesc,
    BOOLEAN                     bCommit)
{
    DWORD   dwError = 0;

//    DWORD   dwCnt = 0;
//    BOOLEAN bInLock = FALSE;

    // fix indices entry - remove build flag
    dwError = vdirIndexingEntryFinish(bCommit);
    //BAIL_ON_VMDIR_ERROR(dwError);  do NOT bail yet, continue to update cache
    if (dwError != 0)
    {
        bCommit = FALSE;
    }

/* TODO, disable indexing for now, need to re-think how to do index config
    VMDIR_LOCK_MUTEX(bInLock, gVdirAttrIndexGlobals.mutex);

    for (dwCnt=0; ppIndexDesc[dwCnt]; dwCnt++)
    {
        VmDirLog(LDAP_DEBUG_ANY,
                "VmdirIndexingWorkerThr: finish (%s)",
                VDIR_SAFE_STRING(ppIndexDesc[dwCnt]->pszAttrName));

        if (bCommit)
        {
            ppIndexDesc[dwCnt]->status = VDIR_CFG_ATTR_INDEX_ENABLED;
        }
        else
        {
            ppIndexDesc[dwCnt]->status = VDIR_CFG_ATTR_INDEX_ABORTED;
        }
    }

    // We have done indexing, reset progress
    gVdirAttrIndexGlobals.bIndexInProgress = FALSE;

    VMDIR_UNLOCK_MUTEX(bInLock, gVdirAttrIndexGlobals.mutex);

    VmDirLog( LDAP_DEBUG_ANY, "vdirIndexingCommit: error %d", dwError );
*/
    return dwError;
}

/*
 * Update indices entry after indexing done
 *
 * 1. if successful -> remove building flag off value
 * e.g. "myuid eq unique (B)" -> "myuid eq unique"
 *
 * 2. otherwise -> remove value with building flag
 * e.g. "myuid eq unique (B)" -> NULL
 */
static
DWORD
vdirIndexingEntryFinish(
    BOOLEAN bCommit)
{
//TODO, Disable indexing during backend interface reorg. need to re-think how we define indices
    return 0;
/*
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;

    DWORD   dwNumOrgVal = 0;
    DWORD   dwNumKeepVal = 0;
    VDIR_BERVALUE*   pOrgBerv = NULL;
    VDIR_BERVALUE*   pKeepBerv = NULL;

    PCSTR       pcszErrMsg = NULL;
    DB_TXN*     ptxn = NULL;
    PAttribute  pIndexAttr = NULL;
    Entry       indiceEntry = {0};

    // NOTE NOTE NOTE, read indices entry then write later in separate txn.
    // It is safe as indices entry write is also protected by VdirIndexingPreCheck.
    dwError = BDBSimpleEIdToEntry(
            CFG_INDEX_ENTRY_ID,
            &indiceEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    pIndexAttr = VmDirFindAttrByName(&indiceEntry, ATTR_INDEX_DESC);
    if (!pIndexAttr)
    {
        dwError = ERROR_INVALID_ENTRY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // use to manipulate indiceEntry to present final image
    dwError = VmDirAllocateMemory(
            sizeof(VDIR_BERVALUE) * (pIndexAttr->numVals + 1),
            (PVOID)&pKeepBerv);
    BAIL_ON_VMDIR_ERROR(dwError);

    // create Berval for values we want to delete or replace
    for (dwCnt = 0, dwNumKeepVal = 0; pIndexAttr->vals[dwCnt].lberbv.bv_val; dwCnt++)
    {
        char* pszStr = VmDirStringStrA(pIndexAttr->vals[dwCnt].lberbv.bv_val, ATTR_INDEX_BUILDING_FLAG);

        if (pszStr != NULL)
        {
            if (bCommit)
            {
                // remove building flag, which is always at the end of the string
                *(pszStr-1) = '\0';
                pIndexAttr->vals[dwCnt].lberbv.bv_len = VmDirStringLenA(pIndexAttr->vals[dwCnt].lberbv.bv_val);
            }
        }
        else
        {   // value w/o building flag are the one to be restored
            memcpy(&pKeepBerv[dwNumKeepVal], &pIndexAttr->vals[dwCnt], sizeof(VDIR_BERVALUE));
            dwNumKeepVal++;
        }
    }

    if (bCommit)
    {
        ;
    }
    else
    {
        //////////////////////////////////////////////////////////////
        // tweak entry to make it represent the final image we want
        // TODO do not like this but no easy option as now.
        //////////////////////////////////////////////////////////////
        // originally, pIndexAttr->vals points into Entry.bvs.  Redirect it to
        // pBackupBerv, which contains values we want to keep.
        pOrgBerv = pIndexAttr->vals;
        dwNumOrgVal = pIndexAttr->numVals;
        pIndexAttr->vals = pKeepBerv;
        pIndexAttr->numVals = dwNumKeepVal;

    }

    dwError = InternalDirectModifyEntry(
            &indiceEntry,
            &pcszErrMsg);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    if (ptxn)
    {
        ptxn->abort(ptxn);
    }

    VMDIR_SAFE_FREE_MEMORY(pKeepBerv);

    if (pOrgBerv)
    {   // restore original attribute before deleting entry
        pIndexAttr->vals  = pOrgBerv;
        pIndexAttr->numVals = dwNumOrgVal;
    }

    DeleteEntry(&indiceEntry);

    return dwError;

error:

    VmDirLog( LDAP_DEBUG_ANY, "vdirIndexingEntryFinish: error %d", dwError );

    goto cleanup;
*/
}


