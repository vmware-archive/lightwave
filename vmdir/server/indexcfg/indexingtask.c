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
VmDirIndexingTaskInit(
    PVDIR_INDEXING_TASK*    ppTask
    )
{
    DWORD   dwError = 0;
    PVDIR_INDEXING_TASK pTask = NULL;

    if (!ppTask)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_INDEXING_TASK),
            (PVOID*)&pTask);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pTask->pIndicesToPopulate);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pTask->pIndicesToValidate);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pTask->pIndicesToDelete);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pTask->pIndicesCompleted);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppTask = pTask;

cleanup:
    return dwError;

error:
    VmDirFreeIndexingTask(pTask);
    goto cleanup;
}

DWORD
VmDirIndexingTaskCompute(
    PVDIR_INDEXING_TASK*    ppTask
    )
{
    DWORD   dwError = 0;
    ENTRYID maxEId = 0;
    PVDIR_BACKEND_INTERFACE pBE = NULL;
    PVDIR_INDEXING_TASK     pTask = NULL;

    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};
    PVDIR_INDEX_CFG pIndexCfg = NULL;

    if (!ppTask)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // compute offset for new task
    pBE = VmDirBackendSelect(NULL);
    if (gVdirIndexGlobals.offset < 0)
    {
        gVdirIndexGlobals.offset = 0;
    }
    else
    {
        dwError = pBE->pfnBEMaxEntryId(&maxEId);
        BAIL_ON_VMDIR_ERROR(dwError);

        gVdirIndexGlobals.offset += INDEXING_BATCH_SIZE;
        if (gVdirIndexGlobals.offset > maxEId)
        {
            gVdirIndexGlobals.offset = 0;
        }
    }

    dwError = VmDirIndexingTaskInit(&pTask);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (LwRtlHashMapIterate(gVdirIndexGlobals.pIndexCfgMap, &iter, &pair))
    {
        pIndexCfg = (PVDIR_INDEX_CFG)pair.pValue;
        if (pIndexCfg->status == VDIR_INDEXING_SCHEDULED)
        {
            pIndexCfg->status = VDIR_INDEXING_IN_PROGRESS;
            pIndexCfg->initOffset =  gVdirIndexGlobals.offset;

            dwError = VmDirLinkedListInsertHead(
                    pTask->pIndicesToPopulate, pIndexCfg, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (pIndexCfg->status == VDIR_INDEXING_IN_PROGRESS)
        {
            if (pIndexCfg->initOffset ==  gVdirIndexGlobals.offset)
            {
                pIndexCfg->status = VDIR_INDEXING_VALIDATING_SCOPES;

                dwError = VmDirLinkedListInsertHead(
                        pTask->pIndicesToValidate, pIndexCfg, NULL);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            else
            {
                dwError = VmDirLinkedListInsertHead(
                        pTask->pIndicesToPopulate, pIndexCfg, NULL);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
        else if (pIndexCfg->status == VDIR_INDEXING_VALIDATING_SCOPES)
        {
            if (VmDirLinkedListIsEmpty(pIndexCfg->pNewUniqScopes) &&
                VmDirLinkedListIsEmpty(pIndexCfg->pDelUniqScopes))
            {
                pIndexCfg->status = VDIR_INDEXING_COMPLETE;

                dwError = VmDirLinkedListInsertHead(
                        pTask->pIndicesCompleted, pIndexCfg, NULL);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            else
            {
                dwError = VmDirLinkedListInsertHead(
                        pTask->pIndicesToValidate, pIndexCfg, NULL);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
        else if (pIndexCfg->status == VDIR_INDEXING_COMPLETE)
        {
            if (!VmDirLinkedListIsEmpty(pIndexCfg->pNewUniqScopes) ||
                !VmDirLinkedListIsEmpty(pIndexCfg->pDelUniqScopes))
            {
                pIndexCfg->status = VDIR_INDEXING_VALIDATING_SCOPES;

                dwError = VmDirLinkedListInsertHead(
                        pTask->pIndicesToValidate, pIndexCfg, NULL);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
        else if (pIndexCfg->status == VDIR_INDEXING_DISABLED)
        {
            if (pIndexCfg->usRefCnt == 1)
            {
                dwError = VmDirLinkedListInsertHead(
                        pTask->pIndicesToDelete, pIndexCfg, NULL);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
    }

    *ppTask = pTask;

cleanup:
    return dwError;

error:
    if (dwError == VMDIR_ERROR_BACKEND_ERROR &&
            VmDirdState() != VMDIRD_STATE_NORMAL)
    {
        dwError = ERROR_INVALID_STATE;
    }
    else
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                "%s failed, error (%d)", __FUNCTION__, dwError );
    }
    VmDirFreeIndexingTask(pTask);
    goto cleanup;
}

DWORD
VmDirIndexingTaskPopulateIndices(
    PVDIR_INDEXING_TASK pTask
    )
{
    DWORD   dwError = 0;
    PVDIR_LINKED_LIST_NODE  pNode = NULL;
    PVDIR_BACKEND_INTERFACE pBE = NULL;
    PLW_HASHMAP pIndexCfgs  = NULL;

    if (!pTask)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pBE = VmDirBackendSelect(NULL);

    dwError = LwRtlCreateHashMap(
            &pIndexCfgs,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    pNode = pTask->pIndicesToPopulate->pTail;
    while (pNode)
    {
        PVDIR_INDEX_CFG pIndexCfg = (PVDIR_INDEX_CFG)pNode->pElement;

        // open index db first if it's new
        if (pIndexCfg->initOffset == gVdirIndexGlobals.offset &&
                !pBE->pfnBEIndexExist(pIndexCfg))
        {
            dwError = pBE->pfnBEIndexOpen(pIndexCfg);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        dwError = LwRtlHashMapInsert(
                pIndexCfgs, pIndexCfg->pszAttrName, pIndexCfg, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        pNode = pNode->pNext;
    }

    dwError = pBE->pfnBEIndexPopulate(
            pIndexCfgs,  gVdirIndexGlobals.offset, INDEXING_BATCH_SIZE);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    LwRtlHashMapClear(pIndexCfgs, VmDirNoopHashMapPairFree, NULL);
    LwRtlFreeHashMap(&pIndexCfgs);
    return dwError;

error:
    if (dwError == VMDIR_ERROR_BACKEND_ERROR &&
            VmDirdState() != VMDIRD_STATE_NORMAL)
    {
        dwError = ERROR_INVALID_STATE;
    }
    else
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                "%s failed, error (%d)", __FUNCTION__, dwError );
    }
    goto cleanup;
}

DWORD
VmDirIndexingTaskValidateScopes(
    PVDIR_INDEXING_TASK pTask
    )
{
    DWORD   dwError = 0;
    PVDIR_LINKED_LIST_NODE  pNode = NULL;

    if (!pTask)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pNode = pTask->pIndicesToValidate->pTail;
    while (pNode)
    {
        PVDIR_INDEX_CFG pIndexCfg = (PVDIR_INDEX_CFG)pNode->pElement;

        dwError = VmDirIndexCfgValidateUniqueScopeMods(pIndexCfg);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirIndexCfgApplyUniqueScopeMods(pIndexCfg);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirIndexCfgRevertBadUniqueScopeMods(pIndexCfg);
        BAIL_ON_VMDIR_ERROR(dwError);

        pNode = pNode->pNext;
    }

cleanup:
    return dwError;

error:
    if (dwError == VMDIR_ERROR_BACKEND_ERROR &&
            VmDirdState() != VMDIRD_STATE_NORMAL)
    {
        dwError = ERROR_INVALID_STATE;
    }
    else if (dwError != ERROR_INVALID_STATE)
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                "%s failed, error (%d)", __FUNCTION__, dwError );
    }
    goto cleanup;
}

DWORD
VmDirIndexingTaskDeleteIndices(
    PVDIR_INDEXING_TASK pTask
    )
{
    DWORD   dwError = 0;
    PVDIR_BACKEND_INTERFACE pBE = NULL;
    PVDIR_LINKED_LIST_NODE  pNode = NULL;

    if (!pTask)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pBE = VmDirBackendSelect(NULL);
    pNode = pTask->pIndicesToDelete->pTail;
    while (pNode)
    {
        PVDIR_INDEX_CFG pIndexCfg = (PVDIR_INDEX_CFG)pNode->pElement;

        // in case of resume, it may be already deleted
        if (pIndexCfg->status == VDIR_INDEXING_DISABLED)
        {
            dwError = pBE->pfnBEIndexDelete(pIndexCfg);
            BAIL_ON_VMDIR_ERROR(dwError);

            pIndexCfg->status = VDIR_INDEXING_DELETED;
            VmDirIndexCfgClear(pIndexCfg);
        }

        pNode = pNode->pNext;
    }

cleanup:
    return dwError;

error:
    if (dwError == VMDIR_ERROR_BACKEND_ERROR &&
            VmDirdState() != VMDIRD_STATE_NORMAL)
    {
        dwError = ERROR_INVALID_STATE;
    }
    else
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                "%s failed, error (%d)", __FUNCTION__, dwError );
    }
    goto cleanup;
}

DWORD
VmDirIndexingTaskRecordProgress(
    PVDIR_INDEXING_TASK pTask,
    PVDIR_INDEX_UPD     pIndexUpd
    )
{
    DWORD   dwError = 0;
    PSTR    pszOffset = NULL;
    VDIR_BACKEND_CTX    beCtx = {0};
    BOOLEAN             bHasTxn = FALSE;
    PLW_HASHMAP             pUpdCfgMap = NULL;
    PVDIR_LINKED_LIST_NODE  pNode = NULL;
    PSTR                    pszStatus = NULL;

    if (VmDirIndexingTaskIsNoop(pTask))
    {
        // nothing to record
        goto cleanup;
    }

    if (pIndexUpd)
    {
        // always check pUpdCfgMap to avoid overwriting progress records
        // from VmDirIndexUpdateCommit
        pUpdCfgMap = pIndexUpd->pUpdIndexCfgMap;
    }

    beCtx.pBE = VmDirBackendSelect(NULL);
    dwError = beCtx.pBE->pfnBETxnBegin(&beCtx, VDIR_BACKEND_TXN_WRITE);
    BAIL_ON_VMDIR_ERROR(dwError);
    bHasTxn = TRUE;

    // record offset to continue from in case of restart
    dwError = VmDirAllocateStringPrintf(
            &pszOffset, "%u",  gVdirIndexGlobals.offset);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = beCtx.pBE->pfnBEUniqKeySetValue(
            &beCtx, INDEX_LAST_OFFSET_KEY, pszOffset);
    BAIL_ON_VMDIR_ERROR(dwError);

    // record all indexing progresses
    pNode = pTask->pIndicesToPopulate->pTail;
    while (pNode)
    {
        PVDIR_INDEX_CFG pCurCfg = (PVDIR_INDEX_CFG)pNode->pElement;
        PVDIR_INDEX_CFG pUpdCfg = NULL;
        PSTR pszAttrName = pCurCfg->pszAttrName;

        if (LwRtlHashMapFindKey(pUpdCfgMap, (PVOID*)&pUpdCfg, pszAttrName))
        {
            dwError = VmDirIndexCfgRecordProgress(&beCtx, pCurCfg);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        // log populate progress every 10000
        if (gVdirIndexGlobals.offset % 10000 == 0 &&
            (!pUpdCfg || pUpdCfg->status == VDIR_INDEXING_IN_PROGRESS))
        {
            VMDIR_SAFE_FREE_MEMORY(pszStatus);
            dwError = VmDirIndexCfgStatusStringfy(pCurCfg, &pszStatus);
            BAIL_ON_VMDIR_ERROR(dwError);

            VMDIR_LOG_INFO( LDAP_DEBUG_INDEX,
                    "%s (%ld)", pszStatus, gVdirIndexGlobals.offset );
        }

        pNode = pNode->pNext;
    }

    pNode = pTask->pIndicesToValidate->pTail;
    while (pNode)
    {
        PVDIR_INDEX_CFG pCurCfg = (PVDIR_INDEX_CFG)pNode->pElement;
        PVDIR_INDEX_CFG pUpdCfg = NULL;
        PSTR pszAttrName = pCurCfg->pszAttrName;

        if (LwRtlHashMapFindKey(pUpdCfgMap, (PVOID*)&pUpdCfg, pszAttrName))
        {
            dwError = VmDirIndexCfgRecordProgress(&beCtx, pCurCfg);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        pNode = pNode->pNext;
    }

    pNode = pTask->pIndicesToDelete->pTail;
    while (pNode)
    {
        PVDIR_INDEX_CFG pCurCfg = (PVDIR_INDEX_CFG)pNode->pElement;
        PVDIR_INDEX_CFG pUpdCfg = NULL;
        PSTR pszAttrName = pCurCfg->pszAttrName;

        if (LwRtlHashMapFindKey(pUpdCfgMap, (PVOID*)&pUpdCfg, pszAttrName))
        {
            dwError = VmDirIndexCfgRecordProgress(&beCtx, pCurCfg);
            BAIL_ON_VMDIR_ERROR(dwError);

            VMDIR_SAFE_FREE_MEMORY(pszStatus);
            dwError = VmDirIndexCfgStatusStringfy(pCurCfg, &pszStatus);
            BAIL_ON_VMDIR_ERROR(dwError);

            VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, pszStatus );
        }

        pNode = pNode->pNext;
    }

    pNode = pTask->pIndicesCompleted->pTail;
    while (pNode)
    {
        PVDIR_INDEX_CFG pCurCfg = (PVDIR_INDEX_CFG)pNode->pElement;
        PVDIR_INDEX_CFG pUpdCfg = NULL;
        PSTR pszAttrName = pCurCfg->pszAttrName;

        if (LwRtlHashMapFindKey(pUpdCfgMap, (PVOID*)&pUpdCfg, pszAttrName))
        {
            dwError = VmDirIndexCfgRecordProgress(&beCtx, pCurCfg);
            BAIL_ON_VMDIR_ERROR(dwError);

            VMDIR_SAFE_FREE_MEMORY(pszStatus);
            dwError = VmDirIndexCfgStatusStringfy(pCurCfg, &pszStatus);
            BAIL_ON_VMDIR_ERROR(dwError);

            VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, pszStatus );
        }

        pNode = pNode->pNext;
    }

    dwError = beCtx.pBE->pfnBETxnCommit(&beCtx);
    BAIL_ON_VMDIR_ERROR(dwError);
    bHasTxn = FALSE;

cleanup:
    if (bHasTxn)
    {
        beCtx.pBE->pfnBETxnAbort(&beCtx);
    }
    VmDirBackendCtxContentFree(&beCtx);
    VMDIR_SAFE_FREE_MEMORY(pszOffset);
    VMDIR_SAFE_FREE_MEMORY(pszStatus);
    return dwError;

error:
    if (dwError == VMDIR_ERROR_BACKEND_ERROR &&
            VmDirdState() != VMDIRD_STATE_NORMAL)
    {
        dwError = ERROR_INVALID_STATE;
    }
    else
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                "%s failed, error (%d)", __FUNCTION__, dwError );
    }
    goto cleanup;
}

BOOLEAN
VmDirIndexingTaskIsNoop(
    PVDIR_INDEXING_TASK pTask
    )
{
    BOOLEAN bNoop = TRUE;
    if (pTask)
    {
        if (!VmDirLinkedListIsEmpty(pTask->pIndicesToPopulate) ||
            !VmDirLinkedListIsEmpty(pTask->pIndicesToValidate) ||
            !VmDirLinkedListIsEmpty(pTask->pIndicesToDelete) ||
            !VmDirLinkedListIsEmpty(pTask->pIndicesCompleted))
        {
            bNoop = FALSE;
        }
    }
    return bNoop;
}

VOID
VmDirFreeIndexingTask(
    PVDIR_INDEXING_TASK pTask
    )
{
    if (pTask)
    {
        VmDirFreeLinkedList(pTask->pIndicesToPopulate);
        VmDirFreeLinkedList(pTask->pIndicesToValidate);
        VmDirFreeLinkedList(pTask->pIndicesToDelete);
        VmDirFreeLinkedList(pTask->pIndicesCompleted);
        VMDIR_SAFE_FREE_MEMORY(pTask);
    }
}
