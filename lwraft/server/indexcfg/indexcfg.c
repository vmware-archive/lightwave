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
VmDirIndexCfgCreate(
    PCSTR               pszAttrName,
    PVDIR_INDEX_CFG*    ppIndexCfg
    )
{
    DWORD   dwError = 0;
    PVDIR_INDEX_CFG pIndexCfg = NULL;

    if (IsNullOrEmptyString(pszAttrName) || !ppIndexCfg)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_INDEX_CFG),
            (PVOID*)&pIndexCfg);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszAttrName, &pIndexCfg->pszAttrName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(
            &pIndexCfg->pUniqScopes,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pIndexCfg->pNewUniqScopes);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pIndexCfg->pDelUniqScopes);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pIndexCfg->pBadUniqScopes);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&pIndexCfg->mutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    // default values
    pIndexCfg->bDefaultIndex = FALSE;
    pIndexCfg->bScopeEditable = TRUE;
    pIndexCfg->bGlobalUniq = FALSE;
    pIndexCfg->bIsNumeric = FALSE;
    pIndexCfg->iTypes = INDEX_TYPE_EQUALITY;
    pIndexCfg->usRefCnt = 1;

    *ppIndexCfg = pIndexCfg;

cleanup:
    return dwError;

error:
    VmDirFreeIndexCfg(pIndexCfg);
    goto cleanup;
}

DWORD
VmDirDefaultIndexCfgInit(
    PVDIR_DEFAULT_INDEX_CFG pDefIdxCfg,
    PVDIR_INDEX_CFG*        ppIndexCfg
    )
{
    DWORD   dwError = 0;
    PSTR    pszScope = NULL;
    BOOLEAN bRestore = FALSE;
    PVDIR_INDEX_CFG pIndexCfg = NULL;
    PSTR            pszIdxStatus = NULL;
    VDIR_BACKEND_CTX    beCtx = {0};
    BOOLEAN             bHasTxn = FALSE;

    if (!pDefIdxCfg || !ppIndexCfg)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirIndexCfgCreate(pDefIdxCfg->pszAttrName, &pIndexCfg);
    BAIL_ON_VMDIR_ERROR(dwError);

    pIndexCfg->bDefaultIndex = TRUE;
    pIndexCfg->bScopeEditable = pDefIdxCfg->bScopeEditable;
    pIndexCfg->bGlobalUniq = pDefIdxCfg->bGlobalUniq;
    pIndexCfg->bIsNumeric = pDefIdxCfg->bIsNumeric;
    pIndexCfg->iTypes = pDefIdxCfg->iTypes;

    beCtx.pBE = VmDirBackendSelect(NULL);
    dwError = beCtx.pBE->pfnBETxnBegin(&beCtx, VDIR_BACKEND_TXN_WRITE);
    BAIL_ON_VMDIR_ERROR(dwError);
    bHasTxn = TRUE;

    dwError = VmDirIndexCfgRestoreProgress(&beCtx, pIndexCfg, &bRestore);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!bRestore && pIndexCfg->bGlobalUniq)
    {
        dwError = VmDirAllocateStringA(PERSISTED_DSE_ROOT_DN, &pszScope);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pIndexCfg->status == VDIR_INDEXING_COMPLETE)
        {
            dwError = LwRtlHashMapInsert(
                    pIndexCfg->pUniqScopes, pszScope, NULL, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            dwError = VmDirLinkedListInsertHead(
                    pIndexCfg->pNewUniqScopes, pszScope, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        pszScope = NULL;
    }

    dwError = VmDirIndexCfgRecordProgress(&beCtx, pIndexCfg);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirIndexCfgStatusStringfy(pIndexCfg, &pszIdxStatus);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, pszIdxStatus );

    dwError = beCtx.pBE->pfnBETxnCommit(&beCtx);
    BAIL_ON_VMDIR_ERROR(dwError);
    bHasTxn = FALSE;

    *ppIndexCfg = pIndexCfg;

cleanup:
    if (bHasTxn)
    {
        beCtx.pBE->pfnBETxnAbort(&beCtx);
    }
    VMDIR_SAFE_FREE_MEMORY(pszIdxStatus);
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszScope);
    VmDirFreeIndexCfg(pIndexCfg);
    goto cleanup;
}

DWORD
VmDirCustomIndexCfgInit(
    PVDIR_SCHEMA_AT_DESC    pATDesc,
    PVDIR_INDEX_CFG*        ppIndexCfg
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PSTR    pszScope = NULL;
    BOOLEAN bRestore = FALSE;
    PVDIR_INDEX_CFG pIndexCfg = NULL;
    PSTR            pszIdxStatus = NULL;
    VDIR_BACKEND_CTX    beCtx = {0};
    BOOLEAN             bHasTxn = FALSE;

    if (!pATDesc || !ppIndexCfg)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirIndexCfgCreate(pATDesc->pszName, &pIndexCfg);
    BAIL_ON_VMDIR_ERROR(dwError);

    pIndexCfg->bIsNumeric = VmDirSchemaAttrIsNumeric(pATDesc);

    beCtx.pBE = VmDirBackendSelect(NULL);
    dwError = beCtx.pBE->pfnBETxnBegin(&beCtx, VDIR_BACKEND_TXN_WRITE);
    BAIL_ON_VMDIR_ERROR(dwError);
    bHasTxn = TRUE;

    dwError = VmDirIndexCfgRestoreProgress(&beCtx, pIndexCfg, &bRestore);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!bRestore && pATDesc->ppszUniqueScopes)
    {
        for (i = 0; pATDesc->ppszUniqueScopes[i]; i++)
        {
            dwError = VmDirAllocateStringA(
                    pATDesc->ppszUniqueScopes[i], &pszScope);
            BAIL_ON_VMDIR_ERROR(dwError);

            if (pIndexCfg->status == VDIR_INDEXING_COMPLETE)
            {
                dwError = LwRtlHashMapInsert(
                        pIndexCfg->pUniqScopes, pszScope, NULL, NULL);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            else
            {
                dwError = VmDirLinkedListInsertHead(
                        pIndexCfg->pNewUniqScopes, pszScope, NULL);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            pszScope = NULL;
        }
    }

    dwError = VmDirIndexCfgRecordProgress(&beCtx, pIndexCfg);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirIndexCfgStatusStringfy(pIndexCfg, &pszIdxStatus);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, pszIdxStatus );

    dwError = beCtx.pBE->pfnBETxnCommit(&beCtx);
    BAIL_ON_VMDIR_ERROR(dwError);
    bHasTxn = FALSE;

    *ppIndexCfg = pIndexCfg;

cleanup:
    if (bHasTxn)
    {
        beCtx.pBE->pfnBETxnAbort(&beCtx);
    }
    VMDIR_SAFE_FREE_MEMORY(pszIdxStatus);
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszScope);
    VmDirFreeIndexCfg(pIndexCfg);
    goto cleanup;
}

DWORD
VmDirIndexCfgCopy(
    PVDIR_INDEX_CFG     pIndexCfg,
    PVDIR_INDEX_CFG*    ppIndexCfgCpy
    )
{
    DWORD   dwError = 0;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};
    PVDIR_LINKED_LIST_NODE  pNode = NULL;
    PSTR    pszOrg = NULL;
    PSTR    pszCpy = NULL;
    PVDIR_INDEX_CFG pIndexCfgCpy = NULL;

    if (!pIndexCfg || !ppIndexCfgCpy)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirIndexCfgCreate(pIndexCfg->pszAttrName, &pIndexCfgCpy);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (LwRtlHashMapIterate(pIndexCfg->pUniqScopes, &iter, &pair))
    {
        pszOrg = (PSTR)pair.pKey;

        dwError = VmDirAllocateStringA(pszOrg, &pszCpy);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapInsert(
                pIndexCfgCpy->pUniqScopes, pszCpy, NULL, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
        pszCpy = NULL;
    }

    pNode = pIndexCfg->pNewUniqScopes->pTail;
    while (pNode)
    {
        PVDIR_LINKED_LIST_NODE pNextNode = pNode->pNext;
        pszOrg = (PSTR)pNode->pElement;

        dwError = VmDirAllocateStringA(pszOrg, &pszCpy);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLinkedListInsertHead(
                pIndexCfgCpy->pNewUniqScopes, pszCpy, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
        pszCpy = NULL;

        pNode = pNextNode;
    }

    pNode = pIndexCfg->pDelUniqScopes->pTail;
    while (pNode)
    {
        PVDIR_LINKED_LIST_NODE pNextNode = pNode->pNext;
        pszOrg = (PSTR)pNode->pElement;

        dwError = VmDirAllocateStringA(pszOrg, &pszCpy);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLinkedListInsertHead(
                pIndexCfgCpy->pDelUniqScopes, pszCpy, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
        pszCpy = NULL;

        pNode = pNextNode;
    }

    pNode = pIndexCfg->pBadUniqScopes->pTail;
    while (pNode)
    {
        PVDIR_LINKED_LIST_NODE pNextNode = pNode->pNext;
        pszOrg = (PSTR)pNode->pElement;

        dwError = VmDirAllocateStringA(pszOrg, &pszCpy);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLinkedListInsertHead(
                pIndexCfgCpy->pBadUniqScopes, pszCpy, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
        pszCpy = NULL;

        pNode = pNextNode;
    }

    pIndexCfgCpy->bDefaultIndex = pIndexCfg->bDefaultIndex;
    pIndexCfgCpy->bScopeEditable = pIndexCfg->bScopeEditable;
    pIndexCfgCpy->bGlobalUniq = pIndexCfg->bGlobalUniq;
    pIndexCfgCpy->bIsNumeric = pIndexCfg->bIsNumeric;
    pIndexCfgCpy->iTypes = pIndexCfg->iTypes;
    pIndexCfgCpy->status = pIndexCfg->status;
    pIndexCfgCpy->initOffset = pIndexCfg->initOffset;

    *ppIndexCfgCpy = pIndexCfgCpy;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VMDIR_SAFE_FREE_MEMORY(pszCpy);
    VmDirFreeIndexCfg(pIndexCfgCpy);
    goto cleanup;
}

DWORD
VmDirIndexCfgValidateUniqueScopeMods(
    PVDIR_INDEX_CFG pIndexCfg
    )
{
    DWORD   dwError = 0;
    int64_t iCnt = 0;
    PVDIR_BACKEND_INTERFACE pBE = NULL;
    PVDIR_BACKEND_INDEX_ITERATOR  pIterator = NULL;
    PVDIR_LINKED_LIST       pNewScopes = NULL;
    PVDIR_LINKED_LIST       pBadScopes = NULL;
    PVDIR_LINKED_LIST_NODE  pNode = NULL;
    PLW_HASHMAP pDetectedScopes = NULL;
    PSTR        pszLastVal = NULL;
    PSTR        pszVal = NULL;
    ENTRYID     eId = 0;
    VDIR_ENTRY  entry = {0};
    PSTR        pszIdxStatus = NULL;

    if (!pIndexCfg)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pNewScopes = pIndexCfg->pNewUniqScopes;
    pBadScopes = pIndexCfg->pBadUniqScopes;

    dwError = LwRtlCreateHashMap(&pDetectedScopes,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirIndexCfgStatusStringfy(pIndexCfg, &pszIdxStatus);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, pszIdxStatus );

    pBE = VmDirBackendSelect(NULL);
    dwError = pBE->pfnBEIndexIteratorInit(pIndexCfg, NULL, &pIterator);
    BAIL_ON_VMDIR_ERROR(dwError);

    /*
     * Iterate the whole index DB and validate all scopes in pNewScopes
     */
    while (pIterator->bHasNext && !VmDirLinkedListIsEmpty(pNewScopes))
    {
        // check vmdir state and log validate progress at every 10000
        if (iCnt % 10000 == 0)
        {
            if (VmDirdState() != VMDIRD_STATE_NORMAL)
            {
                dwError = ERROR_INVALID_STATE;
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            VMDIR_LOG_INFO( LDAP_DEBUG_INDEX, "%s (%ld)", pszIdxStatus, iCnt );
        }
        iCnt++;

        dwError = pBE->pfnBEIndexIterate(pIterator, &pszVal, &eId);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (!pszLastVal || VmDirStringCompareA(pszLastVal, pszVal, FALSE))
        {
            LwRtlHashMapClear(pDetectedScopes, VmDirNoopHashMapPairFree, NULL);
            VMDIR_SAFE_FREE_MEMORY(pszLastVal);
            pszLastVal = pszVal;
            pszVal = NULL;
        }
        else
        {
            VMDIR_SAFE_FREE_MEMORY(pszVal);
        }

        dwError = pBE->pfnBESimpleIdToEntry(eId, &entry);
        BAIL_ON_VMDIR_ERROR(dwError);

        pNode = pNewScopes->pTail;
        while (pNode)
        {
            PVDIR_LINKED_LIST_NODE pNextNode = pNode->pNext;
            PSTR pszScope = (PSTR)pNode->pElement;
            PSTR pszDn = entry.dn.lberbv.bv_val;

            /*
             * If the scope cannot be enforced, move the scope to
             * pBadScope for post-processing
             */
            if (VmDirStringCompareA(PERSISTED_DSE_ROOT_DN, pszScope, FALSE) == 0 ||
                VmDirStringEndsWith(pszDn, pszScope, FALSE))
            {
                if (LwRtlHashMapFindKey(pDetectedScopes, NULL, pszScope) == 0)
                {
                    dwError = VmDirLinkedListInsertHead(
                            pBadScopes, pszScope, NULL);
                    BAIL_ON_VMDIR_ERROR(dwError);

                    dwError = VmDirLinkedListRemove(pNewScopes, pNode);
                    BAIL_ON_VMDIR_ERROR(dwError);        

                    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                            "%s will revert the scope '%s' for attr '%s' "
                            "because it detected multiple of value '%s' ",
                            __FUNCTION__,
                            pszScope,
                            pIndexCfg->pszAttrName,
                            pszLastVal );
                }
                else
                {
                    dwError = LwRtlHashMapInsert(
                            pDetectedScopes, pszScope, NULL, NULL);
                    BAIL_ON_VMDIR_ERROR(dwError);
                }
            }
            pNode = pNextNode;
        }
        VmDirFreeEntryContent(&entry);
    }

cleanup:
    pBE->pfnBEIndexIteratorFree(pIterator);
    LwRtlHashMapClear(pDetectedScopes, VmDirNoopHashMapPairFree, NULL);
    LwRtlFreeHashMap(&pDetectedScopes);
    VmDirFreeEntryContent(&entry);
    VMDIR_SAFE_FREE_MEMORY(pszLastVal);
    VMDIR_SAFE_FREE_MEMORY(pszVal);
    VMDIR_SAFE_FREE_MEMORY(pszIdxStatus);
    return dwError;

error:
    if (dwError != ERROR_INVALID_STATE)
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                "%s failed, error (%d)", __FUNCTION__, dwError );
    }

    goto cleanup;
}

DWORD
VmDirIndexCfgApplyUniqueScopeMods(
    PVDIR_INDEX_CFG pIndexCfg
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    PVDIR_LINKED_LIST       pNewScopes = NULL;
    PVDIR_LINKED_LIST       pDelScopes = NULL;
    PVDIR_LINKED_LIST_NODE  pNode = NULL;
    LW_HASHMAP_PAIR pair = {NULL, NULL};
    PVMDIR_MUTEX    pMutex = NULL;

    if (!pIndexCfg)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pNewScopes = pIndexCfg->pNewUniqScopes;
    pDelScopes = pIndexCfg->pDelUniqScopes;

    pMutex = pIndexCfg->mutex;
    VMDIR_LOCK_MUTEX(bInLock, pMutex);

    /*
     * Move all good scopes to pIndexCfg->pUniqScopes
     */
    pNode = pNewScopes->pTail;
    while (pNode)
    {
        PVDIR_LINKED_LIST_NODE pNextNode = pNode->pNext;
        PSTR pszScope = (PSTR)pNode->pElement;

        dwError = LwRtlHashMapInsert(
                pIndexCfg->pUniqScopes, pszScope, NULL, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLinkedListRemove(pNewScopes, pNode);
        BAIL_ON_VMDIR_ERROR(dwError);

        pNode = pNextNode;
    }

    /*
     * Remove all del scopes from pIndexCfg->pUniqScopes
     */
    pNode = pDelScopes->pTail;
    while (pNode)
    {
        PVDIR_LINKED_LIST_NODE pNextNode = pNode->pNext;
        PSTR pszScope = (PSTR)pNode->pElement;

        dwError = LwRtlHashMapRemove(pIndexCfg->pUniqScopes, pszScope, &pair);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_SAFE_FREE_MEMORY(pair.pKey);
        VMDIR_SAFE_FREE_MEMORY(pszScope);

        dwError = VmDirLinkedListRemove(pDelScopes, pNode);
        BAIL_ON_VMDIR_ERROR(dwError);

        pNode = pNextNode;
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, pMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

DWORD
VmDirIndexCfgRevertBadUniqueScopeMods(
    PVDIR_INDEX_CFG pIndexCfg
    )
{
    DWORD   dwError = 0;
    PSTR    pszDn = NULL;
    PVDIR_LINKED_LIST       pBadScopes = NULL;
    PVDIR_LINKED_LIST_NODE  pNode = NULL;
    VDIR_OPERATION  ldapOp = {0};

    if (!pIndexCfg)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pBadScopes = pIndexCfg->pBadUniqScopes;

    if (VmDirLinkedListIsEmpty(pBadScopes))
    {
        goto cleanup;
    }

    dwError = VmDirAllocateStringPrintf(&pszDn,
            "cn=%s,%s",
            pIndexCfg->pszAttrName,
            SCHEMA_NAMING_CONTEXT_DN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInitStackOperation(&ldapOp,
            VDIR_OPERATION_TYPE_INTERNAL,
            LDAP_REQ_MODIFY,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    ldapOp.bNoRaftLog = TRUE;

    ldapOp.pBEIF = VmDirBackendSelect(NULL);
    ldapOp.reqDn.lberbv_val = pszDn;
    ldapOp.reqDn.lberbv_len = VmDirStringLenA(pszDn);

    ldapOp.request.modifyReq.dn.lberbv_val = ldapOp.reqDn.lberbv_val;
    ldapOp.request.modifyReq.dn.lberbv_len = ldapOp.reqDn.lberbv_len;

    pNode = pBadScopes->pTail;
    while (pNode)
    {
        PSTR pszScope = (PSTR)pNode->pElement;

        dwError = VmDirAppendAMod(&ldapOp,
                MOD_OP_DELETE,
                ATTR_UNIQUENESS_SCOPE,
                ATTR_UNIQUENESS_SCOPE_LEN,
                pszScope,
                VmDirStringLenA(pszScope));
        BAIL_ON_VMDIR_ERROR(dwError);

        pNode = pNode->pNext;
    }

    dwError = VmDirInternalModifyEntry(&ldapOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (!VmDirLinkedListIsEmpty(pBadScopes))
    {
        VMDIR_SAFE_FREE_MEMORY(pBadScopes->pHead->pElement);
        dwError = VmDirLinkedListRemove(pBadScopes, pBadScopes->pHead);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VmDirFreeOperationContent(&ldapOp);
    VMDIR_SAFE_FREE_MEMORY(pszDn);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

DWORD
VmDirIndexCfgGetAllScopesInStrArray(
    PVDIR_INDEX_CFG pIndexCfg,
    PSTR**          pppszScopes
    )
{
    DWORD   dwError = 0;
    PVMDIR_STRING_LIST  pScopes = NULL;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};
    PVDIR_LINKED_LIST_NODE  pNode = NULL;

    if (!pIndexCfg || !pppszScopes)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirStringListInitialize(&pScopes, 0);
    BAIL_ON_VMDIR_ERROR(dwError);

    // pScope = pUniqScopes (validated) + pNewUniqueScopes (not validated yet)
    while (LwRtlHashMapIterate(pIndexCfg->pUniqScopes, &iter, &pair))
    {
        dwError = VmDirStringListAddStrClone((PSTR)pair.pKey, pScopes);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pNode = pIndexCfg->pNewUniqScopes->pHead;
    while (pNode)
    {
        dwError = VmDirStringListAddStrClone((PSTR)pNode->pElement, pScopes);
        BAIL_ON_VMDIR_ERROR(dwError);
        pNode = pNode->pPrev;
    }

    // hand over string array
    if (pScopes->dwCount > 0)
    {
        *pppszScopes = (PSTR*)pScopes->pStringList;
        pScopes->pStringList = NULL;
        pScopes->dwCount = 0;
    }

cleanup:
    VmDirStringListFree(pScopes);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

DWORD
VmDirIndexCfgStatusStringfy(
    PVDIR_INDEX_CFG pIndexCfg,
    PSTR*           ppszStatus
    )
{
    DWORD   dwError = 0;
    PSTR    pszStatus = NULL;

    static PCSTR    ppcszStatuses[] = {
            "SCHEDULED",
            "IN_PROGRESS",
            "VALIDATING_SCOPES",
            "COMPLETE",
            "DISABLED",
            "DELETED"
    };

    if (!pIndexCfg || !ppszStatus)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringPrintf(&pszStatus,
            "Indexing Progress: Attribute = %s, Status = %s",
            pIndexCfg->pszAttrName,
            ppcszStatuses[pIndexCfg->status]);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszStatus = pszStatus;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VMDIR_SAFE_FREE_MEMORY(pszStatus);
    goto cleanup;
}

VOID
VmDirIndexCfgClear(
    PVDIR_INDEX_CFG pIndexCfg
    )
{
    PVDIR_LINKED_LIST   pList = NULL;

    if (pIndexCfg)
    {
        LwRtlHashMapClear(pIndexCfg->pUniqScopes,
                VmDirSimpleHashMapPairFree, NULL);

        pList = pIndexCfg->pNewUniqScopes;
        while (!VmDirLinkedListIsEmpty(pList))
        {
            VMDIR_SAFE_FREE_MEMORY(pList->pHead->pElement);
            VmDirLinkedListRemove(pList, pList->pHead);
        }

        pList = pIndexCfg->pDelUniqScopes;
        while (!VmDirLinkedListIsEmpty(pList))
        {
            VMDIR_SAFE_FREE_MEMORY(pList->pHead->pElement);
            VmDirLinkedListRemove(pList, pList->pHead);
        }

        pList = pIndexCfg->pBadUniqScopes;
        while (!VmDirLinkedListIsEmpty(pList))
        {
            VMDIR_SAFE_FREE_MEMORY(pList->pHead->pElement);
            VmDirLinkedListRemove(pList, pList->pHead);
        }
    }
}

VOID
VmDirFreeIndexCfg(
    PVDIR_INDEX_CFG pIndexCfg
    )
{
    if (pIndexCfg)
    {
        VmDirIndexCfgClear(pIndexCfg);

        VMDIR_SAFE_FREE_MEMORY(pIndexCfg->pszAttrName);
        LwRtlFreeHashMap(&pIndexCfg->pUniqScopes);
        VmDirFreeLinkedList(pIndexCfg->pNewUniqScopes);
        VmDirFreeLinkedList(pIndexCfg->pDelUniqScopes);
        VmDirFreeLinkedList(pIndexCfg->pBadUniqScopes);

        VMDIR_SAFE_FREE_MUTEX(pIndexCfg->mutex);

        VMDIR_SAFE_FREE_MEMORY(pIndexCfg);
    }
}

VOID
VmDirFreeIndexCfgMapPair(
    PLW_HASHMAP_PAIR    pPair,
    LW_PVOID            pUnused
    )
{
    PVDIR_INDEX_CFG pIndexCfg = (PVDIR_INDEX_CFG)pPair->pValue;
    VmDirIndexCfgRelease(pIndexCfg);
}
