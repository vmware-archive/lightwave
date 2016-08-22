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
VmDirIndexCfgMap(
    PLW_HASHMAP*    ppIndexCfgMap
    )
{
    DWORD   dwError = 0;
    VDIR_SERVER_STATE   vmdirState = VmDirdState();

    if (!ppIndexCfgMap)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (vmdirState != VMDIRD_STATE_STARTUP)
    {
        dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppIndexCfgMap = gVdirIndexGlobals.pIndexCfgMap;

error:
    return dwError;
}

DWORD
VmDirIndexCfgAcquire(
    PCSTR               pszAttrName,
    VDIR_INDEX_USAGE    usage,
    PVDIR_INDEX_CFG*    ppIndexCfg
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    PVDIR_INDEX_CFG pIndexCfg = NULL;
    PVMDIR_MUTEX    pMutex = NULL;

    if (IsNullOrEmptyString(pszAttrName) || !ppIndexCfg)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppIndexCfg = NULL;

    if (LwRtlHashMapFindKey(
            gVdirIndexGlobals.pIndexCfgMap, (PVOID*)&pIndexCfg, pszAttrName))
    {
        goto cleanup;
    }

    pMutex = pIndexCfg->mutex;
    VMDIR_LOCK_MUTEX(bInLock, pMutex);

    if (pIndexCfg->status == VDIR_INDEXING_SCHEDULED &&
            usage == VDIR_INDEX_READ)
    {
        dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
    }
    else if (pIndexCfg->status == VDIR_INDEXING_IN_PROGRESS &&
            usage == VDIR_INDEX_READ)
    {
        dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
    }
    else if (pIndexCfg->status == VDIR_INDEXING_DISABLED ||
            pIndexCfg->status == VDIR_INDEXING_DELETED)
    {
        goto cleanup;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    pIndexCfg->usRefCnt++;
    *ppIndexCfg = pIndexCfg;

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, pMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

VOID
VmDirIndexCfgRelease(
    PVDIR_INDEX_CFG pIndexCfg
    )
{
    BOOLEAN bInLock = FALSE;

    if (pIndexCfg)
    {
        VMDIR_LOCK_MUTEX(bInLock, pIndexCfg->mutex);
        pIndexCfg->usRefCnt--;
        VMDIR_UNLOCK_MUTEX(bInLock, pIndexCfg->mutex);
    }
}

BOOLEAN
VmDirIndexExist(
    PCSTR   pszAttrName
    )
{
    BOOLEAN bExist = FALSE;
    PVDIR_INDEX_CFG pIndexCfg = NULL;

    if (!IsNullOrEmptyString(pszAttrName) &&
            LwRtlHashMapFindKey(
                    gVdirIndexGlobals.pIndexCfgMap,
                    (PVOID*)&pIndexCfg,
                    pszAttrName) == 0)
    {
        if (pIndexCfg->status != VDIR_INDEXING_DISABLED &&
            pIndexCfg->status != VDIR_INDEXING_DELETED)
        {
            bExist = TRUE;
        }
    }

    return bExist;
}

BOOLEAN
VmDirIndexIsDefault(
    PCSTR   pszAttrName
    )
{
    BOOLEAN bIsDefault = FALSE;
    PVDIR_INDEX_CFG pIndexCfg = NULL;

    if (!IsNullOrEmptyString(pszAttrName) &&
            LwRtlHashMapFindKey(
                    gVdirIndexGlobals.pIndexCfgMap,
                    (PVOID*)&pIndexCfg,
                    pszAttrName) == 0)
    {
        bIsDefault = pIndexCfg->bDefaultIndex;
    }

    return bIsDefault;
}

/*
 * run-time
 */
DWORD
VmDirIndexSchedule(
    PVDIR_BACKEND_CTX   pBECtx,
    PCSTR               pszAttrName,
    PCSTR               pszAttrSyntaxOid
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    PVDIR_INDEX_CFG pIndexCfg = NULL;
    PSTR            pszIdxStatus = NULL;

    if (!pBECtx || IsNullOrEmptyString(pszAttrName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_LOCK_MUTEX(bInLock, gVdirIndexGlobals.mutex);

    if (LwRtlHashMapFindKey(
            gVdirIndexGlobals.pIndexCfgMap, (PVOID*)&pIndexCfg, pszAttrName))
    {
        dwError = VmDirIndexCfgCreate(pszAttrName, &pIndexCfg);
        BAIL_ON_VMDIR_ERROR(dwError);

        pIndexCfg->bIsNumeric = VmDirSchemaSyntaxIsNumeric(pszAttrSyntaxOid);

        dwError = LwRtlHashMapInsert(
                gVdirIndexGlobals.pIndexCfgMap,
                pIndexCfg->pszAttrName,
                pIndexCfg,
                NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (pIndexCfg->status == VDIR_INDEXING_DELETED)
    {
        pIndexCfg->status = VDIR_INDEXING_SCHEDULED;
    }
    else
    {
        dwError = ERROR_ALREADY_EXISTS;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirIndexCfgRecordProgress(pBECtx, pIndexCfg);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirIndexCfgStatusStringfy(pIndexCfg, &pszIdxStatus);
    BAIL_ON_VMDIR_ERROR(dwError);
    pIndexCfg = NULL;

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, pszIdxStatus );

    dwError = VmDirConditionSignal(gVdirIndexGlobals.cond);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVdirIndexGlobals.mutex);
    VMDIR_SAFE_FREE_MEMORY(pszIdxStatus);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirFreeIndexCfg(pIndexCfg);
    goto cleanup;
}

DWORD
VmDirIndexDelete(
    PVDIR_BACKEND_CTX   pBECtx,
    PCSTR               pszAttrName
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    PVDIR_INDEX_CFG pIndexCfg = NULL;
    PSTR            pszIdxStatus = NULL;

    if (!pBECtx || IsNullOrEmptyString(pszAttrName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_LOCK_MUTEX(bInLock, gVdirIndexGlobals.mutex);

    if (LwRtlHashMapFindKey(
            gVdirIndexGlobals.pIndexCfgMap, (PVOID*)&pIndexCfg, pszAttrName) ||
            pIndexCfg->status == VDIR_INDEXING_DISABLED ||
            pIndexCfg->status == VDIR_INDEXING_DELETED)
    {
        dwError = VMDIR_ERROR_NOT_FOUND;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pIndexCfg->bDefaultIndex)
    {
        dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pIndexCfg->status = VDIR_INDEXING_DISABLED;

    dwError = VmDirIndexCfgRecordProgress(pBECtx, pIndexCfg);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirIndexCfgStatusStringfy(pIndexCfg, &pszIdxStatus);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, pszIdxStatus );

    dwError = VmDirConditionSignal(gVdirIndexGlobals.cond);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVdirIndexGlobals.mutex);
    VMDIR_SAFE_FREE_MEMORY(pszIdxStatus);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

DWORD
VmDirIndexAddUniquenessScope(
    PVDIR_BACKEND_CTX   pBECtx,
    PCSTR               pszAttrName,
    PCSTR*              ppszUniqScopes
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    BOOLEAN bInLock = FALSE;
    PSTR    pszUniqScope = NULL;
    PVDIR_INDEX_CFG pIndexCfg = NULL;

    if (!pBECtx || IsNullOrEmptyString(pszAttrName) || !ppszUniqScopes)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_LOCK_MUTEX(bInLock, gVdirIndexGlobals.mutex);

    if (LwRtlHashMapFindKey(
            gVdirIndexGlobals.pIndexCfgMap, (PVOID*)&pIndexCfg, pszAttrName) ||
            pIndexCfg->status == VDIR_INDEXING_DISABLED ||
            pIndexCfg->status == VDIR_INDEXING_DELETED)
    {
        dwError = VMDIR_ERROR_NOT_FOUND;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!pIndexCfg->bScopeEditable ||
         pIndexCfg->status == VDIR_INDEXING_VALIDATING_SCOPES)
    {
        dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (i = 0; ppszUniqScopes[i]; i++)
    {
        if (LwRtlHashMapFindKey(
                pIndexCfg->pUniqScopes, NULL, ppszUniqScopes[i]) == 0)
        {
            dwError = ERROR_ALREADY_EXISTS;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    for (i = 0; ppszUniqScopes[i]; i++)
    {
        dwError = VmDirAllocateStringA(ppszUniqScopes[i], &pszUniqScope);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLinkedListInsertHead(
                pIndexCfg->pNewUniqScopes, pszUniqScope, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
        pszUniqScope = NULL;
    }

    dwError = VmDirIndexCfgRecordProgress(pBECtx, pIndexCfg);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirConditionSignal(gVdirIndexGlobals.cond);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVdirIndexGlobals.mutex);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VMDIR_SAFE_FREE_MEMORY(pszUniqScope);
    goto cleanup;
}

DWORD
VmDirIndexDeleteUniquenessScope(
    PVDIR_BACKEND_CTX   pBECtx,
    PCSTR               pszAttrName,
    PCSTR*              ppszUniqScopes
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    BOOLEAN bInLock = FALSE;
    PSTR    pszUniqScope = NULL;
    PVDIR_INDEX_CFG pIndexCfg = NULL;

    if (!pBECtx || IsNullOrEmptyString(pszAttrName) || !ppszUniqScopes)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_LOCK_MUTEX(bInLock, gVdirIndexGlobals.mutex);

    if (LwRtlHashMapFindKey(
            gVdirIndexGlobals.pIndexCfgMap, (PVOID*)&pIndexCfg, pszAttrName) ||
            pIndexCfg->status == VDIR_INDEXING_DISABLED ||
            pIndexCfg->status == VDIR_INDEXING_DELETED)
    {
        dwError = VMDIR_ERROR_NOT_FOUND;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!pIndexCfg->bScopeEditable ||
         pIndexCfg->status == VDIR_INDEXING_VALIDATING_SCOPES)
    {
        dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (i = 0; ppszUniqScopes[i]; i++)
    {
        if (LwRtlHashMapFindKey(
                pIndexCfg->pUniqScopes, NULL, ppszUniqScopes[i]) != 0)
        {
            dwError = VMDIR_ERROR_NOT_FOUND;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    for (i = 0; ppszUniqScopes[i]; i++)
    {
        dwError = VmDirAllocateStringA(ppszUniqScopes[i], &pszUniqScope);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLinkedListInsertHead(
                pIndexCfg->pDelUniqScopes, pszUniqScope, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
        pszUniqScope = NULL;
    }

    dwError = VmDirIndexCfgRecordProgress(pBECtx, pIndexCfg);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirConditionSignal(gVdirIndexGlobals.cond);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVdirIndexGlobals.mutex);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VMDIR_SAFE_FREE_MEMORY(pszUniqScope);
    goto cleanup;
}
