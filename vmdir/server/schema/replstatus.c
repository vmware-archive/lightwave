/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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
_VdirEntryCoreInit(
    PVDIR_ENTRY         pEntry,
    PVDIR_SCHEMA_CTX    pSchemaCtx
    );

static
DWORD
_VdirEntryLoadGlobal(
    PVDIR_ENTRY         pEntry,
    PVDIR_SCHEMA_CTX    pSchemaCtx
    );

static
DWORD
_VdirEntryLoadReplState(
    PVDIR_ENTRY             pEntry,
    PVDIR_SCHEMA_CTX        pSchemaCtx,
    PVDIR_SCHEMA_REPL_STATE pReplState
    );

DWORD
VmDirSchemaReplStatusGlobalsInit(
    VOID
    )
{
    DWORD   dwError = 0;

    dwError = VmDirAllocateRWLock(&gVdirSchemaReplStatusGlobals.rwlock);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&gVdirSchemaReplStatusGlobals.mutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateCondition(&gVdirSchemaReplStatusGlobals.cond);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&gVdirSchemaReplStatusGlobals.pReplStates);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

VOID
VmDirSchemaReplStatusGlobalsShutdown(
    VOID
    )
{
    if (gVdirSchemaReplStatusGlobals.pThrInfo)
    {
        VmDirSrvThrShutdown(gVdirSchemaReplStatusGlobals.pThrInfo);
        gVdirSchemaReplStatusGlobals.pThrInfo = NULL;
    }

    VmDirSchemaReplStatusEntriesClear();
    VmDirFreeLinkedList(gVdirSchemaReplStatusGlobals.pReplStates);
    gVdirSchemaReplStatusGlobals.pReplStates = NULL;

    VMDIR_SAFE_FREE_CONDITION(gVdirSchemaReplStatusGlobals.cond);
    gVdirSchemaReplStatusGlobals.cond = NULL;

    VMDIR_SAFE_FREE_MUTEX(gVdirSchemaReplStatusGlobals.mutex);
    gVdirSchemaReplStatusGlobals.mutex = NULL;

    VMDIR_SAFE_FREE_RWLOCK(gVdirSchemaReplStatusGlobals.rwlock);
    gVdirSchemaReplStatusGlobals.rwlock = NULL;
}

DWORD
VmDirSchemaReplStatusEntriesInit(
    VOID
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInWriteLock = FALSE;
    BOOLEAN bInReplArgLock = FALSE;
    PVDIR_LINKED_LIST   pReplStates = NULL;
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr = NULL;
    PVDIR_SCHEMA_REPL_STATE pReplState = NULL;

    VmDirSchemaReplStatusEntriesClear();

    VMDIR_RWLOCK_WRITELOCK(bInWriteLock, gVdirSchemaReplStatusGlobals.rwlock, 0);
    VMDIR_LOCK_MUTEX(bInReplArgLock, gVmdirGlobals.replAgrsMutex);

    pReplStates = gVdirSchemaReplStatusGlobals.pReplStates;

    for (pReplAgr = gVmdirReplAgrs; pReplAgr; pReplAgr = pReplAgr->next)
    {
        dwError = VmDirSchemaReplStateCreateFromReplAgr(pReplAgr, &pReplState);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLinkedListInsertHead(pReplStates, pReplState, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
        pReplState = NULL;
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bInReplArgLock, gVmdirGlobals.replAgrsMutex);
    VMDIR_RWLOCK_UNLOCK(bInWriteLock, gVdirSchemaReplStatusGlobals.rwlock);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirFreeSchemaReplState(pReplState);
    goto cleanup;
}

VOID
VmDirSchemaReplStatusEntriesClear(
    VOID
    )
{
    BOOLEAN bInLock = FALSE;
    PVDIR_LINKED_LIST   pReplStates = NULL;
    PVDIR_SCHEMA_REPL_STATE pReplState = NULL;

    VMDIR_RWLOCK_WRITELOCK(bInLock, gVdirSchemaReplStatusGlobals.rwlock, 0);

    pReplStates = gVdirSchemaReplStatusGlobals.pReplStates;
    while (!VmDirLinkedListIsEmpty(pReplStates))
    {
        pReplState = (PVDIR_SCHEMA_REPL_STATE)pReplStates->pHead->pElement;
        VmDirLinkedListRemove(pReplStates, pReplStates->pHead);
        VmDirFreeSchemaReplState(pReplState);
    }

    VMDIR_RWLOCK_UNLOCK(bInLock, gVdirSchemaReplStatusGlobals.rwlock);
}

DWORD
VmDirSchemaReplStatusEntriesRetrieve(
    PVDIR_ENTRY_ARRAY   pEntries,
    int                 scope
    )
{
    DWORD   dwError = 0;
    size_t  size = 0;
    BOOLEAN bInLock = FALSE;
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;
    PVDIR_LINKED_LIST_NODE  pNode = NULL;

    if (!pEntries)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_RWLOCK_READLOCK(bInLock, gVdirSchemaReplStatusGlobals.rwlock, 0);

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    // allocate entry array
    size = VmDirLinkedListGetSize(gVdirSchemaReplStatusGlobals.pReplStates);

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_ENTRY) * (size + 1),
            (PVOID*)&pEntries->pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    // return pseudo parent entry if scope is BASE or SUB
    if (scope == LDAP_SCOPE_BASE || scope == LDAP_SCOPE_SUB)
    {
        dwError = _VdirEntryLoadGlobal(
                &pEntries->pEntry[pEntries->iSize++], pSchemaCtx);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    //  return status entries if scope is SUB or ONE
    if (scope == LDAP_SCOPE_SUB || scope == LDAP_SCOPE_ONE)
    {
        // for n in nodes
        //   convert n to vdir entry
        pNode = gVdirSchemaReplStatusGlobals.pReplStates->pTail;
        while (pNode)
        {
            dwError = _VdirEntryLoadReplState(
                    &pEntries->pEntry[pEntries->iSize++],
                    pSchemaCtx,
                    (PVDIR_SCHEMA_REPL_STATE)pNode->pElement);
            BAIL_ON_VMDIR_ERROR(dwError);

            pNode = pNode->pNext;
        }
    }

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, gVdirSchemaReplStatusGlobals.rwlock);
    VmDirSchemaCtxRelease(pSchemaCtx);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

DWORD
VmDirSchemaReplStatusEntriesRefresh(
    VOID
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;

    VMDIR_RWLOCK_WRITELOCK(bInLock, gVdirSchemaReplStatusGlobals.rwlock, 0);

    // check for existing thread
    // start a thread if none
    if (!gVdirSchemaReplStatusGlobals.bRefreshInProgress)
    {
        VmDirSrvThrFree(gVdirSchemaReplStatusGlobals.pThrInfo);
        gVdirSchemaReplStatusGlobals.pThrInfo = NULL;

        dwError = VmDirSrvThrInit(
                &gVdirSchemaReplStatusGlobals.pThrInfo,
                gVdirSchemaReplStatusGlobals.mutex,
                gVdirSchemaReplStatusGlobals.cond,
                TRUE);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirCreateThread(
                &gVdirSchemaReplStatusGlobals.pThrInfo->tid,
                gVdirSchemaReplStatusGlobals.pThrInfo->bJoinThr,
                VmDirSchemaReplStatusEntriesRefreshThread,
                gVdirSchemaReplStatusGlobals.pThrInfo);
        BAIL_ON_VMDIR_ERROR(dwError);

        gVdirSchemaReplStatusGlobals.bRefreshInProgress = TRUE;
    }

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, gVdirSchemaReplStatusGlobals.rwlock);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

DWORD
VmDirSchemaReplStatusEntriesRefreshThread(
    PVOID   pArg
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInWaitLock = FALSE;
    BOOLEAN bInWriteLock = FALSE;
    PVDIR_SCHEMA_CTX    pCtx = NULL;
    PVDIR_LINKED_LIST_NODE  pNode = NULL;
    PVDIR_SCHEMA_REPL_STATE pReplState = NULL;

    // log start info
    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "%s starts", __FUNCTION__ );

    // get partners and init their status structs
    dwError = VmDirSchemaReplStatusEntriesInit();
    BAIL_ON_VMDIR_ERROR(dwError);

    // sleep for repl interval + 10 secs
    VMDIR_LOCK_MUTEX(bInWaitLock, gVdirSchemaReplStatusGlobals.mutex);

    dwError = VmDirConditionTimedWait(
            gVdirSchemaReplStatusGlobals.cond,
            gVdirSchemaReplStatusGlobals.mutex,
            (gVmdirServerGlobals.replInterval + 10) * 1000);
    dwError = dwError == ETIMEDOUT ? 0 : dwError;
    BAIL_ON_VMDIR_ERROR(dwError);

    if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
    {
        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "%s halting due to shutdown", __FUNCTION__ );
        goto cleanup;
    }

    VMDIR_RWLOCK_WRITELOCK(bInWriteLock, gVdirSchemaReplStatusGlobals.rwlock, 0);

    dwError = VmDirSchemaCtxAcquire(&pCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    // for n in nodes
    //   connect to n and get repl status
    pNode = gVdirSchemaReplStatusGlobals.pReplStates->pTail;
    while (pNode)
    {
        pReplState = (PVDIR_SCHEMA_REPL_STATE)pNode->pElement;

        dwError = VmDirSchemaReplStateCheck(pReplState, pCtx->pLdapSchema);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirSchemaReplStateLog(pReplState);
        BAIL_ON_VMDIR_ERROR(dwError);

        pNode = pNode->pNext;
    }

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "%s completed", __FUNCTION__ );

cleanup:
    gVdirSchemaReplStatusGlobals.bRefreshInProgress = FALSE;
    VMDIR_RWLOCK_UNLOCK(bInWriteLock, gVdirSchemaReplStatusGlobals.rwlock);
    VMDIR_UNLOCK_MUTEX(bInWaitLock, gVdirSchemaReplStatusGlobals.mutex);
    VmDirSchemaCtxRelease(pCtx);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

DWORD
VmDirSchemaReplStateCreateFromReplAgr(
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr,
    PVDIR_SCHEMA_REPL_STATE*        ppReplState
    )
{
    DWORD   dwError = 0;
    PSTR    pszHostName = NULL;
    PSTR    pszDomainName = NULL;
    PVDIR_SCHEMA_REPL_STATE pReplState = NULL;

    if (!pReplAgr || !ppReplState)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // get host name and domain name from repl agr
    dwError = VmDirReplURIToHostname(pReplAgr->ldapURI, &pszHostName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirDomainDNToName(
            BERVAL_NORM_VAL(gVmdirServerGlobals.systemDomainDN),
            &pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaReplStateCreate(
            pszHostName, pszDomainName, &pReplState);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppReplState = pReplState;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirFreeSchemaReplState(pReplState);
    goto cleanup;
}

static
DWORD
_VdirEntryCoreInit(
    PVDIR_ENTRY         pEntry,
    PVDIR_SCHEMA_CTX    pSchemaCtx
    )
{
    DWORD   dwError = 0;
    VMDIR_SECURITY_DESCRIPTOR   vsd = {0};

    if (!pEntry || !pSchemaCtx)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pEntry->allocType = ENTRY_STORAGE_FORMAT_NORMAL;
    pEntry->pSchemaCtx = VmDirSchemaCtxClone(pSchemaCtx);

    dwError = VmDirCreateTransientSecurityDescriptor(FALSE, &vsd);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirEntryCacheSecurityDescriptor(
            pEntry, vsd.pSecDesc, vsd.ulSecDesc);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirEntryAddSingleValueStrAttribute(
            pEntry, ATTR_OBJECT_CLASS, OC_SERVER_STATUS);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(vsd.pSecDesc);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

static
DWORD
_VdirEntryLoadGlobal(
    PVDIR_ENTRY         pEntry,
    PVDIR_SCHEMA_CTX    pSchemaCtx
    )
{
    DWORD   dwError = 0;
    PSTR    pszDN = NULL;
    PSTR    pszAttrVal = NULL;

    if (!pEntry || !pSchemaCtx)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VdirEntryCoreInit(pEntry, pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pszDN, "cn=%s", SCHEMA_REPL_STATUS_CN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToBervalContent(pszDN, &pEntry->dn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirEntryAddSingleValueStrAttribute(
            pEntry, ATTR_CN, SCHEMA_REPL_STATUS_CN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pszAttrVal,
            "%s: %s",
            SCHEMA_REPL_STATUS_REFRESH_IN_PROGRESS,
            gVdirSchemaReplStatusGlobals.bRefreshInProgress ?
                    "TRUE" : "FALSE");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirEntryAddSingleValueStrAttribute(
            pEntry, ATTR_SERVER_RUNTIME_STATUS, pszAttrVal);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszAttrVal);
    VMDIR_SAFE_FREE_MEMORY(pszDN);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

static
DWORD
_VdirEntryLoadReplState(
    PVDIR_ENTRY             pEntry,
    PVDIR_SCHEMA_CTX        pSchemaCtx,
    PVDIR_SCHEMA_REPL_STATE pReplState
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PSTR    pszDN = NULL;
    PSTR    pszAttrVal = NULL;

    if (!pEntry || !pSchemaCtx || !pReplState)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VdirEntryCoreInit(pEntry, pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pszDN,
            "cn=%s,cn=%s",
            pReplState->pszHostName,
            SCHEMA_REPL_STATUS_CN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToBervalContent(pszDN, &pEntry->dn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirEntryAddSingleValueStrAttribute(
            pEntry, ATTR_CN, pReplState->pszHostName);
    BAIL_ON_VMDIR_ERROR(dwError);

    {
        PSTR pszStrKeys[] = {
                SCHEMA_REPL_STATUS_HOST_NAME,
                SCHEMA_REPL_STATUS_DOMAIN_NAME,
                NULL
        };

        PSTR pszStrVals[] = {
                pReplState->pszHostName,
                pReplState->pszDomainName
        };

        for (i = 0; pszStrKeys[i]; i++)
        {
            VMDIR_SAFE_FREE_MEMORY(pszAttrVal);
            dwError = VmDirAllocateStringPrintf(
                    &pszAttrVal, "%s: %s", pszStrKeys[i], pszStrVals[i]);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirEntryAddSingleValueStrAttribute(
                    pEntry, ATTR_SERVER_RUNTIME_STATUS, pszAttrVal);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    {
        PSTR pszBoolKeys[] = {
                SCHEMA_REPL_STATUS_CHECK_INITIATED,
                SCHEMA_REPL_STATUS_CHECK_SUCCEEDED,
                SCHEMA_REPL_STATUS_TREE_IN_SYNC,
                NULL
        };

        PSTR pszBoolVals[] = {
                pReplState->bCheckInitiated ? "TRUE" : "FALSE",
                pReplState->bCheckSucceeded ? "TRUE" : "FALSE",
                pReplState->bTreeInSync ? "TRUE" : "FALSE"
        };

        for (i = 0; pszBoolKeys[i]; i++)
        {
            VMDIR_SAFE_FREE_MEMORY(pszAttrVal);
            dwError = VmDirAllocateStringPrintf(
                    &pszAttrVal, "%s: %s", pszBoolKeys[i], pszBoolVals[i]);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirEntryAddSingleValueStrAttribute(
                    pEntry, ATTR_SERVER_RUNTIME_STATUS, pszAttrVal);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    {
        PSTR pszDwordKeys[] = {
                SCHEMA_REPL_STATUS_ATTR_MISSING_IN_TREE,
                SCHEMA_REPL_STATUS_ATTR_MISMATCH_IN_TREE,
                SCHEMA_REPL_STATUS_CLASS_MISSING_IN_TREE,
                SCHEMA_REPL_STATUS_CLASS_MISMATCH_IN_TREE,
                NULL
        };

        DWORD dwDwordVals[] = {
                pReplState->dwAttrMissingInTree,
                pReplState->dwAttrMismatchInTree,
                pReplState->dwClassMissingInTree,
                pReplState->dwClassMismatchInTree
        };

        for (i = 0; pszDwordKeys[i]; i++)
        {
            VMDIR_SAFE_FREE_MEMORY(pszAttrVal);
            dwError = VmDirAllocateStringPrintf(
                    &pszAttrVal, "%s: %d", pszDwordKeys[i], dwDwordVals[i]);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirEntryAddSingleValueStrAttribute(
                    pEntry, ATTR_SERVER_RUNTIME_STATUS, pszAttrVal);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszAttrVal);
    VMDIR_SAFE_FREE_MEMORY(pszDN);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}
