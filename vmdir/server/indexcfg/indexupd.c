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
VmDirIndexUpdInit(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_INDEX_UPD*    ppIndexUpd
    )
{
    DWORD   dwError = 0;
    PVDIR_INDEX_UPD pIndexUpd = NULL;

    if (!ppIndexUpd)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_INDEX_UPD),
            (PVOID*)&pIndexUpd);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(
            &pIndexUpd->pUpdIndexCfgMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    pIndexUpd->pBECtx = pBECtx;
    pIndexUpd->bOwnBECtx = FALSE;
    pIndexUpd->bHasBETxn = FALSE;
    pIndexUpd->bInLock = FALSE;

    if (!pIndexUpd->pBECtx)
    {
        dwError = VmDirAllocateMemory(
                sizeof(VDIR_BACKEND_CTX),
                (PVOID*)&pIndexUpd->pBECtx);
        BAIL_ON_VMDIR_ERROR(dwError);
        pIndexUpd->bOwnBECtx = TRUE;
    }

    *ppIndexUpd = pIndexUpd;

cleanup:
    return dwError;

error:
    VmDirIndexUpdFree(pIndexUpd);
    goto cleanup;
}

DWORD
VmDirIndexUpdCopy(
    PVDIR_INDEX_UPD     pSrcIdxUpd,
    PVDIR_INDEX_UPD     pTgtIdxUpd
    )
{
    DWORD   dwError = 0;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};
    PVDIR_INDEX_CFG pIndexCfg = NULL;
    PVDIR_INDEX_CFG pIndexCfgCpy = NULL;

    if (!pTgtIdxUpd)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pSrcIdxUpd)
    {
        while (LwRtlHashMapIterate(pSrcIdxUpd->pUpdIndexCfgMap, &iter, &pair))
        {
            pIndexCfg = (PVDIR_INDEX_CFG)pair.pValue;

            dwError = VmDirIndexCfgCopy(pIndexCfg, &pIndexCfgCpy);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = LwRtlHashMapInsert(
                    pTgtIdxUpd->pUpdIndexCfgMap,
                    pIndexCfgCpy->pszAttrName,
                    pIndexCfgCpy,
                    NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
            pIndexCfgCpy = NULL;
        }
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirFreeIndexCfg(pIndexCfgCpy);
    goto cleanup;
}

DWORD
VmDirIndexUpdApply(
    PVDIR_INDEX_UPD     pIndexUpd
    )
{
    DWORD   dwError = 0;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};

    if (pIndexUpd)
    {
        PLW_HASHMAP pCurCfgMap = gVdirIndexGlobals.pIndexCfgMap;
        PLW_HASHMAP pUpdCfgMap = pIndexUpd->pUpdIndexCfgMap;

        while (LwRtlHashMapIterate(pUpdCfgMap, &iter, &pair))
        {
            PVDIR_INDEX_CFG pCurCfg = NULL;
            PVDIR_INDEX_CFG pUpdCfg = (PVDIR_INDEX_CFG)pair.pValue;

            dwError = LwRtlHashMapInsert(
                    pCurCfgMap, pUpdCfg->pszAttrName, pUpdCfg, &pair);
            BAIL_ON_VMDIR_ERROR(dwError);

            pCurCfg = (PVDIR_INDEX_CFG)pair.pValue;
            VmDirIndexCfgRelease(pCurCfg);
        }

        LwRtlHashMapClear(pUpdCfgMap, VmDirNoopHashMapPairFree, NULL);
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

VOID
VmDirIndexUpdFree(
    PVDIR_INDEX_UPD     pIndexUpd
    )
{
    if (pIndexUpd)
    {
        if (pIndexUpd->pUpdIndexCfgMap)
        {
            LwRtlHashMapClear(pIndexUpd->pUpdIndexCfgMap,
                    VmDirFreeIndexCfgMapPair, NULL);
            LwRtlFreeHashMap(&pIndexUpd->pUpdIndexCfgMap);
            pIndexUpd->pUpdIndexCfgMap = NULL;
        }

        if (pIndexUpd->bOwnBECtx)
        {
            VmDirBackendCtxFree(pIndexUpd->pBECtx);
            pIndexUpd->pBECtx = NULL;
        }

        VMDIR_SAFE_FREE_MEMORY(pIndexUpd);
    }
}
