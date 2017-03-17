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
VmDirIndexLibInit(
    PVMDIR_MUTEX    pModMutex
    )
{
    static VDIR_DEFAULT_INDEX_CFG defIdx[] = VDIR_INDEX_INITIALIZER;

    DWORD   dwError = 0;
    DWORD   i = 0;
    PSTR    pszLastOffset = NULL;
    ENTRYID maxEId = 0;
    VDIR_BACKEND_CTX    beCtx = {0};
    BOOLEAN             bHasTxn = FALSE;
    PVDIR_INDEX_CFG     pIndexCfg = NULL;

    if (!pModMutex)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // pModMutex refers to gVdirSchemaGlobals.cacheModMutex,
    // so do not free it during shutdown
    gVdirIndexGlobals.mutex = pModMutex;

    dwError = VmDirAllocateCondition(&gVdirIndexGlobals.cond);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(
            &gVdirIndexGlobals.pIndexCfgMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    beCtx.pBE = VmDirBackendSelect(NULL);

    dwError = beCtx.pBE->pfnBETxnBegin(&beCtx, VDIR_BACKEND_TXN_WRITE);
    BAIL_ON_VMDIR_ERROR(dwError);
    bHasTxn = TRUE;

    // get fields to continue indexing from where it left last time
    dwError = beCtx.pBE->pfnBEUniqKeyGetValue(
            &beCtx, INDEX_LAST_OFFSET_KEY, &pszLastOffset);
    if (dwError)
    {
        dwError = beCtx.pBE->pfnBEMaxEntryId(&maxEId);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (maxEId == ENTRY_ID_SEQ_INITIAL_VALUE)
        {
            gVdirIndexGlobals.bFirstboot = TRUE;
        }
        else
        {
            gVdirIndexGlobals.bLegacyDB = TRUE;
        }

        // set index_last_offset = -1 to indicate indexing has started
        gVdirIndexGlobals.offset = -1;
        dwError = beCtx.pBE->pfnBEUniqKeySetValue(
                &beCtx, INDEX_LAST_OFFSET_KEY, "-1");
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        gVdirIndexGlobals.offset = VmDirStringToIA(pszLastOffset);
    }

    dwError = beCtx.pBE->pfnBETxnCommit(&beCtx);
    BAIL_ON_VMDIR_ERROR(dwError);
    bHasTxn = FALSE;

    // open default indices
    for (i = 0; defIdx[i].pszAttrName; i++)
    {
        dwError = VmDirDefaultIndexCfgInit(&defIdx[i], &pIndexCfg);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirIndexOpen(pIndexCfg);
        BAIL_ON_VMDIR_ERROR(dwError);
        pIndexCfg = NULL;
    }

    // VMIT support
    dwError = VmDirIndexLibInitVMIT();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = InitializeIndexingThread();
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (bHasTxn)
    {
        beCtx.pBE->pfnBETxnAbort(&beCtx);
    }
    VmDirBackendCtxContentFree(&beCtx);
    VMDIR_SAFE_FREE_MEMORY(pszLastOffset);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirFreeIndexCfg(pIndexCfg);
    goto cleanup;
}

/*
 * should only be used during bootstrap
 * maybe add state check?
 */
DWORD
VmDirIndexOpen(
    PVDIR_INDEX_CFG pIndexCfg
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    PVDIR_BACKEND_INTERFACE pBE = NULL;

    if (!pIndexCfg)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_LOCK_MUTEX(bInLock, gVdirIndexGlobals.mutex);

    if (LwRtlHashMapFindKey(
            gVdirIndexGlobals.pIndexCfgMap, NULL, pIndexCfg->pszAttrName) == 0)
    {
        dwError = ERROR_ALREADY_INITIALIZED;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = LwRtlHashMapInsert(
            gVdirIndexGlobals.pIndexCfgMap,
            pIndexCfg->pszAttrName,
            pIndexCfg,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    pBE = VmDirBackendSelect(NULL);
    dwError = pBE->pfnBEIndexOpen(pIndexCfg);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVdirIndexGlobals.mutex);
    return dwError;

error:
    goto cleanup;
}

VOID
VmDirIndexLibShutdown(
    VOID
    )
{
    if (gVdirIndexGlobals.pThrInfo)
    {
        VmDirSrvThrShutdown(gVdirIndexGlobals.pThrInfo);
        gVdirIndexGlobals.pThrInfo = NULL;
    }

    if (gVdirIndexGlobals.pIndexCfgMap)
    {
        LwRtlHashMapClear(gVdirIndexGlobals.pIndexCfgMap,
                VmDirFreeIndexCfgMapPair, NULL);
        LwRtlFreeHashMap(&gVdirIndexGlobals.pIndexCfgMap);
        gVdirIndexGlobals.pIndexCfgMap = NULL;
    }

    VmDirIndexUpdFree(gVdirIndexGlobals.pIndexUpd);
    gVdirIndexGlobals.pIndexUpd = NULL;

    VMDIR_SAFE_FREE_CONDITION(gVdirIndexGlobals.cond);
    gVdirIndexGlobals.cond = NULL;

    gVdirIndexGlobals.mutex = NULL;
    gVdirIndexGlobals.bFirstboot = FALSE;
    gVdirIndexGlobals.bLegacyDB = FALSE;
}
