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
_BuildAllKeyStrs(
    PVDIR_INDEX_CFG pIndexCfg,
    PSTR*           ppszStatusKey,
    PSTR*           ppszInitOffsetKey,
    PSTR*           ppszScopesKey,
    PSTR*           ppszNewScopesKey,
    PSTR*           ppszDelScopesKey
    );

static
DWORD
_BuildAllValStrs(
    PVDIR_INDEX_CFG pIndexCfg,
    PSTR*           ppszStatusVal,
    PSTR*           ppszInitOffsetVal,
    PSTR*           ppszScopesVal,
    PSTR*           ppszNewScopesVal,
    PSTR*           ppszDelScopesVal
    );

DWORD
VmDirIndexCfgRecordProgress(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_INDEX_CFG     pIndexCfg
    )
{
    DWORD   dwError = 0;
    PSTR    pszStatusKey = NULL;
    PSTR    pszStatusVal = NULL;
    PSTR    pszInitOffsetKey = NULL;
    PSTR    pszInitOffsetVal = NULL;
    PSTR    pszScopesKey = NULL;
    PSTR    pszScopesVal = NULL;
    PSTR    pszNewScopesKey = NULL;
    PSTR    pszNewScopesVal = NULL;
    PSTR    pszDelScopesKey = NULL;
    PSTR    pszDelScopesVal = NULL;

    if (!pBECtx || !pIndexCfg)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _BuildAllKeyStrs(
            pIndexCfg,
            &pszStatusKey,
            &pszInitOffsetKey,
            &pszScopesKey,
            &pszNewScopesKey,
            &pszDelScopesKey);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _BuildAllValStrs(
            pIndexCfg,
            &pszStatusVal,
            &pszInitOffsetVal,
            &pszScopesVal,
            &pszNewScopesVal,
            &pszDelScopesVal);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = pBECtx->pBE->pfnBEUniqKeySetValue(
            pBECtx, pszStatusKey, VDIR_SAFE_STRING(pszStatusVal));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = pBECtx->pBE->pfnBEUniqKeySetValue(
            pBECtx, pszInitOffsetKey, VDIR_SAFE_STRING(pszInitOffsetVal));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = pBECtx->pBE->pfnBEUniqKeySetValue(
            pBECtx, pszScopesKey, VDIR_SAFE_STRING(pszScopesVal));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = pBECtx->pBE->pfnBEUniqKeySetValue(
            pBECtx, pszNewScopesKey, VDIR_SAFE_STRING(pszNewScopesVal));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = pBECtx->pBE->pfnBEUniqKeySetValue(
            pBECtx, pszDelScopesKey, VDIR_SAFE_STRING(pszDelScopesVal));
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszStatusKey);
    VMDIR_SAFE_FREE_MEMORY(pszStatusVal);
    VMDIR_SAFE_FREE_MEMORY(pszInitOffsetKey);
    VMDIR_SAFE_FREE_MEMORY(pszInitOffsetVal);
    VMDIR_SAFE_FREE_MEMORY(pszScopesKey);
    VMDIR_SAFE_FREE_MEMORY(pszScopesVal);
    VMDIR_SAFE_FREE_MEMORY(pszNewScopesKey);
    VMDIR_SAFE_FREE_MEMORY(pszNewScopesVal);
    VMDIR_SAFE_FREE_MEMORY(pszDelScopesKey);
    VMDIR_SAFE_FREE_MEMORY(pszDelScopesVal);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

DWORD
VmDirIndexCfgRestoreProgress(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_INDEX_CFG     pIndexCfg,
    PBOOLEAN            pbRestore
    )
{
    DWORD   dwError = 0;
    PSTR    pszStatusKey = NULL;
    PSTR    pszStatusVal = NULL;
    PSTR    pszInitOffsetKey = NULL;
    PSTR    pszInitOffsetVal = NULL;
    PSTR    pszScopesKey = NULL;
    PSTR    pszScopesVal = NULL;
    PSTR    pszNewScopesKey = NULL;
    PSTR    pszNewScopesVal = NULL;
    PSTR    pszDelScopesKey = NULL;
    PSTR    pszDelScopesVal = NULL;
    PSTR    pszScope = NULL;

    if (!pBECtx || !pIndexCfg || !pbRestore)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _BuildAllKeyStrs(
            pIndexCfg,
            &pszStatusKey,
            &pszInitOffsetKey,
            &pszScopesKey,
            &pszNewScopesKey,
            &pszDelScopesKey);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (gVdirIndexGlobals.bFirstboot)
    {
        pIndexCfg->status = VDIR_INDEXING_COMPLETE;
    }
    else if (gVdirIndexGlobals.bLegacyDB)
    {
        if (pBECtx->pBE->pfnBEIndexExist(pIndexCfg))
        {
            pIndexCfg->status = VDIR_INDEXING_COMPLETE;
        }
        else
        {
            pIndexCfg->status = VDIR_INDEXING_SCHEDULED;
        }
    }
    else
    {
        PSTR pszToken = NULL;
        char* save = NULL;

        dwError = pBECtx->pBE->pfnBEUniqKeyGetValue(
                pBECtx, pszStatusKey, &pszStatusVal);
        BAIL_ON_VMDIR_ERROR(dwError);

        pIndexCfg->status = VmDirStringToIA(pszStatusVal);

        dwError = pBECtx->pBE->pfnBEUniqKeyGetValue(
                pBECtx, pszInitOffsetKey, &pszInitOffsetVal);
        BAIL_ON_VMDIR_ERROR(dwError);

        pIndexCfg->initOffset = VmDirStringToIA(pszInitOffsetVal);

        dwError = pBECtx->pBE->pfnBEUniqKeyGetValue(
                pBECtx, pszScopesKey, &pszScopesVal);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (!IsNullOrEmptyString(pszScopesVal))
        {
            pszToken = VmDirStringTokA(pszScopesVal, INDEX_SEP, &save);
            while (pszToken)
            {
                dwError = VmDirAllocateStringA(pszToken, &pszScope);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = LwRtlHashMapInsert(
                        pIndexCfg->pUniqScopes, pszScope, NULL, NULL);
                BAIL_ON_VMDIR_ERROR(dwError);
                pszScope = NULL;

                pszToken = VmDirStringTokA(NULL, INDEX_SEP, &save);
            }
        }

        dwError = pBECtx->pBE->pfnBEUniqKeyGetValue(
                pBECtx, pszNewScopesKey, &pszNewScopesVal);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (!IsNullOrEmptyString(pszNewScopesVal))
        {
            pszToken = VmDirStringTokA(pszNewScopesVal, INDEX_SEP, &save);
            while (pszToken)
            {
                dwError = VmDirAllocateStringA(pszToken, &pszScope);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = VmDirLinkedListInsertHead(
                        pIndexCfg->pNewUniqScopes, pszScope, NULL);
                BAIL_ON_VMDIR_ERROR(dwError);
                pszScope = NULL;

                pszToken = VmDirStringTokA(NULL, INDEX_SEP, &save);
            }
        }

        dwError = pBECtx->pBE->pfnBEUniqKeyGetValue(
                pBECtx, pszDelScopesKey, &pszDelScopesVal);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (!IsNullOrEmptyString(pszDelScopesVal))
        {
            pszToken = VmDirStringTokA(pszDelScopesVal, INDEX_SEP, &save);
            while (pszToken)
            {
                dwError = VmDirAllocateStringA(pszToken, &pszScope);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = VmDirLinkedListInsertHead(
                        pIndexCfg->pDelUniqScopes, pszScope, NULL);
                BAIL_ON_VMDIR_ERROR(dwError);
                pszScope = NULL;

                pszToken = VmDirStringTokA(NULL, INDEX_SEP, &save);
            }
        }

        *pbRestore = TRUE;
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszStatusKey);
    VMDIR_SAFE_FREE_MEMORY(pszStatusVal);
    VMDIR_SAFE_FREE_MEMORY(pszInitOffsetKey);
    VMDIR_SAFE_FREE_MEMORY(pszInitOffsetVal);
    VMDIR_SAFE_FREE_MEMORY(pszScopesKey);
    VMDIR_SAFE_FREE_MEMORY(pszScopesVal);
    VMDIR_SAFE_FREE_MEMORY(pszNewScopesKey);
    VMDIR_SAFE_FREE_MEMORY(pszNewScopesVal);
    VMDIR_SAFE_FREE_MEMORY(pszDelScopesKey);
    VMDIR_SAFE_FREE_MEMORY(pszDelScopesVal);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VMDIR_SAFE_FREE_MEMORY(pszScope);
    goto cleanup;
}

static
DWORD
_BuildAllKeyStrs(
    PVDIR_INDEX_CFG pIndexCfg,
    PSTR*           ppszStatusKey,
    PSTR*           ppszInitOffsetKey,
    PSTR*           ppszScopesKey,
    PSTR*           ppszNewScopesKey,
    PSTR*           ppszDelScopesKey
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PSTR    pszAttrName = NULL;
    PSTR    pszStatusKey = NULL;
    PSTR    pszInitOffsetKey = NULL;
    PSTR    pszScopesKey = NULL;
    PSTR    pszNewScopesKey = NULL;
    PSTR    pszDelScopesKey = NULL;

    if (!pIndexCfg ||
            !ppszStatusKey ||
            !ppszInitOffsetKey ||
            !ppszScopesKey ||
            !ppszNewScopesKey ||
            !ppszDelScopesKey)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringA(pIndexCfg->pszAttrName, &pszAttrName);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < VmDirStringLenA(pszAttrName); i++)
    {
        pszAttrName[i] = tolower(pszAttrName[i]);
    }

    dwError = VmDirAllocateStringPrintf(
            &pszStatusKey,
            "%s%s%s%s%s",
            INDEX_TKN,
            INDEX_SEP,
            pszAttrName,
            INDEX_SEP,
            INDEX_STATUS_TKN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pszInitOffsetKey,
            "%s%s%s%s%s",
            INDEX_TKN,
            INDEX_SEP,
            pszAttrName,
            INDEX_SEP,
            INDEX_INIT_OFFSET_TKN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pszScopesKey,
            "%s%s%s%s%s",
            INDEX_TKN,
            INDEX_SEP,
            pszAttrName,
            INDEX_SEP,
            INDEX_SCOPES_TKN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pszNewScopesKey,
            "%s%s%s%s%s%s",
            INDEX_TKN,
            INDEX_SEP,
            pszAttrName,
            INDEX_SEP,
            INDEX_NEW_TKN,
            INDEX_SCOPES_TKN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pszDelScopesKey,
            "%s%s%s%s%s%s",
            INDEX_TKN,
            INDEX_SEP,
            pszAttrName,
            INDEX_SEP,
            INDEX_DEL_TKN,
            INDEX_SCOPES_TKN);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszStatusKey = pszStatusKey;
    *ppszInitOffsetKey = pszInitOffsetKey;
    *ppszScopesKey = pszScopesKey;
    *ppszNewScopesKey = pszNewScopesKey;
    *ppszDelScopesKey = pszDelScopesKey;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszAttrName);
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszStatusKey);
    VMDIR_SAFE_FREE_MEMORY(pszInitOffsetKey);
    VMDIR_SAFE_FREE_MEMORY(pszScopesKey);
    VMDIR_SAFE_FREE_MEMORY(pszNewScopesKey);
    VMDIR_SAFE_FREE_MEMORY(pszDelScopesKey);
    goto cleanup;
}

static
DWORD
_BuildAllValStrs(
    PVDIR_INDEX_CFG pIndexCfg,
    PSTR*           ppszStatusVal,
    PSTR*           ppszInitOffsetVal,
    PSTR*           ppszScopesVal,
    PSTR*           ppszNewScopesVal,
    PSTR*           ppszDelScopesVal
    )
{
    DWORD   dwError = 0;
    PSTR    pszStatusVal = NULL;
    PSTR    pszInitOffsetVal = NULL;
    PSTR    pszScopesVal = NULL;
    PSTR    pszNewScopesVal = NULL;
    PSTR    pszDelScopesVal = NULL;

    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};
    PVDIR_LINKED_LIST_NODE  pNode = NULL;

    if (!pIndexCfg ||
            !ppszStatusVal ||
            !ppszInitOffsetVal ||
            !ppszScopesVal ||
            !ppszNewScopesVal ||
            !ppszDelScopesVal)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringPrintf(
            &pszStatusVal, "%d", pIndexCfg->status);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pszInitOffsetVal, "%u", pIndexCfg->initOffset);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (LwRtlHashMapIterate(pIndexCfg->pUniqScopes, &iter, &pair))
    {
        PSTR pszScope = (PSTR)pair.pKey;
        PSTR pszTmp = pszScopesVal;

        dwError = VmDirAllocateStringPrintf(
                &pszScopesVal,
                "%s%s%s",
                pszTmp ? pszTmp : "",
                pszTmp ? INDEX_SEP : "",
                pszScope);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_SAFE_FREE_MEMORY(pszTmp);
    }

    pNode = pIndexCfg->pNewUniqScopes->pHead;
    while (pNode)
    {
        PSTR pszScope = (PSTR)pNode->pElement;
        PSTR pszTmp = pszNewScopesVal;

        dwError = VmDirAllocateStringPrintf(
                &pszNewScopesVal,
                "%s%s%s",
                pszTmp ? pszTmp : "",
                pszTmp ? INDEX_SEP : "",
                pszScope);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_SAFE_FREE_MEMORY(pszTmp);
        pNode = pNode->pPrev;
    }

    pNode = pIndexCfg->pDelUniqScopes->pHead;
    while (pNode)
    {
        PSTR pszScope = (PSTR)pNode->pElement;
        PSTR pszTmp = pszDelScopesVal;

        dwError = VmDirAllocateStringPrintf(
                &pszDelScopesVal,
                "%s%s%s",
                pszTmp ? pszTmp : "",
                pszTmp ? INDEX_SEP : "",
                pszScope);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_SAFE_FREE_MEMORY(pszTmp);
        pNode = pNode->pPrev;
    }

    *ppszStatusVal = pszStatusVal;
    *ppszInitOffsetVal = pszInitOffsetVal;
    *ppszScopesVal = pszScopesVal;
    *ppszNewScopesVal = pszNewScopesVal;
    *ppszDelScopesVal = pszDelScopesVal;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszStatusVal);
    VMDIR_SAFE_FREE_MEMORY(pszInitOffsetVal);
    VMDIR_SAFE_FREE_MEMORY(pszScopesVal);
    VMDIR_SAFE_FREE_MEMORY(pszNewScopesVal);
    VMDIR_SAFE_FREE_MEMORY(pszDelScopesVal);
    goto cleanup;
}
