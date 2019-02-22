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

static
DWORD
_VmDirIndexLibInitLogDB(
    VOID
    );

DWORD
VmDirIndexLibInit(
    PVMDIR_MUTEX    pModMutex
    )
{
    static VDIR_DEFAULT_INDEX_CFG defIdx[] = VDIR_INDEX_INITIALIZER;

    DWORD   dwError = 0;
    DWORD   i = 0;
    PSTR    pszLastOffset = NULL;
    VDIR_BACKEND_CTX    beCtx = {0};
    BOOLEAN             bHasTxn = FALSE;
    PVDIR_INDEX_CFG     pIndexCfg = NULL;
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;
    PVDIR_SCHEMA_AT_DESC    pATDesc = NULL;

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

    dwError = beCtx.pBE->pfnBETxnBegin(&beCtx, VDIR_BACKEND_TXN_WRITE, &bHasTxn);
    BAIL_ON_VMDIR_ERROR(dwError);

    // get fields to continue indexing from where it left last time
    dwError = beCtx.pBE->pfnBEUniqKeyGetValue(
            &beCtx, INDEX_LAST_OFFSET_KEY, &pszLastOffset);
    if (dwError)
    {
        gVdirIndexGlobals.bFirstboot = TRUE;

        // set index_last_offset = -1 to indicate indexing has started
        gVdirIndexGlobals.offset = -1;
        dwError = beCtx.pBE->pfnBEUniqKeySetValue(
                &beCtx, INDEX_LAST_OFFSET_KEY, "-1");
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VmDirStringToINT64(pszLastOffset, NULL, &gVdirIndexGlobals.offset);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (bHasTxn)
    {
        dwError = beCtx.pBE->pfnBETxnCommit(&beCtx);
        bHasTxn = FALSE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    // open default indices
    for (i = 0; defIdx[i].pszAttrName; i++)
    {
        dwError = VmDirDefaultIndexCfgInit(&defIdx[i], &pIndexCfg);
        BAIL_ON_VMDIR_ERROR(dwError);

        // update attribute types in schema cache with their index info
        dwError = VmDirSchemaAttrNameToDescriptor(
                pSchemaCtx, pIndexCfg->pszAttrName, &pATDesc);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirIndexCfgGetAllScopesInStrArray(
                pIndexCfg, &pATDesc->ppszUniqueScopes);
        BAIL_ON_VMDIR_ERROR(dwError);

        pATDesc->dwSearchFlags |= 1;

        // for free later
        pATDesc->pLdapAt->ppszUniqueScopes = pATDesc->ppszUniqueScopes;
        pATDesc->pLdapAt->dwSearchFlags = pATDesc->dwSearchFlags;

        dwError = VmDirIndexOpen(pIndexCfg);
        BAIL_ON_VMDIR_ERROR(dwError);
        pIndexCfg = NULL;
    }

    dwError = _VmDirIndexLibInitLogDB();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = InitializeIndexingThread();
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirBackendCtxContentFree(&beCtx);
    VmDirSchemaCtxRelease(pSchemaCtx);
    VMDIR_SAFE_FREE_MEMORY(pszLastOffset);
    return dwError;

error:
    if (bHasTxn)
    {
        beCtx.pBE->pfnBETxnAbort(&beCtx);
    }
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

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
    dwError = pBE->pfnBEIndexOpen(pBE, pIndexCfg);
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
}

/*
 * include logdb in init path.
 * note logdb is not considered for dynamic indexing at this time.
 * this is because multidb split is pending for indexcfg.
*/
static
DWORD
_VmDirIndexLibInitLogDB(
    VOID
    )
{
    DWORD dwError = 0;
    static VDIR_DEFAULT_INDEX_CFG defIdx[] = VDIR_INDEX_INITIALIZER;
    PVDIR_INDEX_CFG pIndexCfg = NULL;
    PVDIR_BACKEND_INTERFACE pBE = NULL;
    DWORD i = 0;

    pBE = VmDirBackendSelect(ALIAS_LOG_CURRENT);
    assert(pBE);

    // open default indices
    for (i = 0; defIdx[i].pszAttrName; i++)
    {
        dwError = VmDirIndexCfgAcquire(
                defIdx[i].pszAttrName, VDIR_INDEX_READ, &pIndexCfg);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = pBE->pfnBEIndexOpen(pBE, pIndexCfg);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:
    return dwError;
}
