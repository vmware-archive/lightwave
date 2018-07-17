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
USN
VmDirBackendLeastOutstandingUSN(
    PVDIR_BACKEND_CTX   pBECtx,
    BOOLEAN             pending // looking for pending transactions only
    );

static
USN
VmDirBackendHighestCommittedUSN(
    PVDIR_BACKEND_CTX   pBECtx
    );

static
USN
VmDirBackendGetMaxOriginatingUSN(
    PVDIR_BACKEND_CTX   pBECtx
    );

static
VOID
VmDirBackendSetMaxOriginatingUSN(
    PVDIR_BACKEND_CTX   pBECtx,
    USN                 maxOriginatingUSN
    );

static
DWORD
VmDirAllocateUSNList(
    PVDIR_BACKEND_USN_LIST*      ppUSNList
    );

static
VOID
VmDirFreeUSNList(
    PVDIR_BACKEND_USN_LIST      pUSNList
    );

static
VOID
_VmDirFreeBackendInstance(
    PVDIR_BACKEND_INSTANCE pInstance
    );

/*
 * map a dn to a backend
 * map created here is used in VmDirBackendSelect to switch backends.
*/
static
DWORD
_VmDirMapDNToBackend(
    PCSTR pszDN,
    PVDIR_BACKEND_INTERFACE pBE
    )
{
    DWORD dwError = 0;

    if (!pszDN || !pBE)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    /* check if this dn is already mapped */
    if (LwRtlHashMapFindKey(gVdirBEGlobals.pDNToBEMap, NULL, pszDN) == 0)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_ENTRY_ALREADY_EXIST);
    }

    dwError = LwRtlHashMapInsert(
                  gVdirBEGlobals.pDNToBEMap,
                  (PVOID)pszDN,
                  pBE,
                  NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}

/*
 * Initialize a database instance.
 * pszDbPath: Requires path exist on disk. database created on first use.
*/
DWORD
_VmDirInitInstance(
    PSTR pszDbPath,
    PVDIR_BACKEND_INSTANCE *ppInstance
    )
{
    DWORD dwError = 0;
    PVDIR_BACKEND_INSTANCE pNewInstance = NULL;

    if (IsNullOrEmptyString(pszDbPath) || !ppInstance)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    /* make sure this database is not yet initialized */
    if (LwRtlHashMapFindKey(
            gVdirBEGlobals.pInstanceMap,
            NULL,
            pszDbPath) == 0)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_ALREADY_INITIALIZED);
    }

    dwError = VmDirAllocateMemory(
                  sizeof(VDIR_BACKEND_INSTANCE),
                  ((PVOID*)&pNewInstance));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszDbPath, &pNewInstance->pszDbPath);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirMDBBEInterface(&pNewInstance->pBE);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* create path to instance entry */
    dwError = LwRtlHashMapInsert(
            gVdirBEGlobals.pInstanceMap,
            pszDbPath,
            pNewInstance,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* create pBE to instance entry */
    dwError = LwRtlHashMapInsert(
            gVdirBEGlobals.pBEToInstanceMap,
            pNewInstance->pBE,
            pNewInstance,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppInstance = pNewInstance;

cleanup:
    return dwError;

error:
    _VmDirFreeBackendInstance(pNewInstance);

    goto cleanup;
}

DWORD
VmDirIterateDBInstances(
    PFN_ITERATE_DB_INSTANCE_CB pfnCB,
    PVOID pUserData
    )
{
    DWORD dwError = 0;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};

    if (!pfnCB)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    while (gVdirBEGlobals.pInstanceMap &&
           LwRtlHashMapIterate(gVdirBEGlobals.pInstanceMap, &iter, &pair))
    {
        dwError = pfnCB((PVDIR_BACKEND_INSTANCE)pair.pValue, pUserData);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
_VmDirBackendGlobalsInit(
    VOID
    )
{
    DWORD dwError = 0;

    /* create map from path to db instances */
    dwError = LwRtlCreateHashMap(
            &gVdirBEGlobals.pInstanceMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* create map from pBE to instance. helps lookups. */
    dwError = LwRtlCreateHashMap(
            &gVdirBEGlobals.pBEToInstanceMap,
            LwRtlHashDigestPointer,
            LwRtlHashEqualPointer,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* create map from dn to backend. used in select backend */
    dwError = LwRtlCreateHashMap(
            &gVdirBEGlobals.pDNToBEMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    gVdirBEGlobals.usnFirstNext = USN_SEQ_INITIAL_VALUE;

error:
    return dwError;
}

static
DWORD
_HasLogRecords(
    PVDIR_BACKEND_INTERFACE pBE,
    BOOLEAN *pHasLogRecords
    )
{
    DWORD dwError = 0;
    BOOLEAN bHasLogRecords = FALSE;
    VDIR_OPERATION searchOp = {0};

    if (!pBE || !pHasLogRecords)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }


    dwError = VmDirInitStackOperation( &searchOp,
                                       VDIR_OPERATION_TYPE_INTERNAL,
                                       LDAP_REQ_SEARCH,
                                       NULL );
    BAIL_ON_VMDIR_ERROR(dwError);

    searchOp.pBEIF = pBE;
    searchOp.reqDn.lberbv.bv_val = RAFT_LOGS_CONTAINER_DN;
    searchOp.reqDn.lberbv.bv_len = VmDirStringLenA(RAFT_LOGS_CONTAINER_DN);
    searchOp.request.searchReq.scope = LDAP_SCOPE_ONE;
    searchOp.request.searchReq.sizeLimit = 1;

    dwError = StrFilterToFilter(
                  "objectClass=*",
                  &searchOp.request.searchReq.filter);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInternalSearch(&searchOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (searchOp.internalSearchEntryArray.iSize > 0)
    {
        bHasLogRecords = TRUE;
    }

    *pHasLogRecords = bHasLogRecords;

error:
    /* okay to accept entry not found errors */
    if(dwError == VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND)
    {
        dwError = 0;
    }
    VmDirFreeOperationContent(&searchOp);

    return dwError;
}

/*
 * Do post init log mapping. Requires schema init so is called
 * after base config
*/
DWORD
VmDirBackendMapPreviousLogs(
    VOID
    )
{
    DWORD dwError = 0;
    BOOLEAN bHasLogRecords = FALSE;

    PVDIR_BACKEND_INTERFACE pBE = VmDirBackendSelect(ALIAS_MAIN);

    /* if main database has logs, map it as the previous log database */
    dwError = _HasLogRecords(pBE, &bHasLogRecords);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (bHasLogRecords)
    {
        dwError = _VmDirMapDNToBackend(ALIAS_LOG_PREVIOUS, pBE);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:
    return dwError;
}

/*
 * Called during server start up to configure backends(s)
 * Currently, we only support ONE backend.
 */
DWORD
VmDirBackendConfig(
    VOID)
{
    DWORD                   dwError = 0;
    PVDIR_BACKEND_USN_LIST  pUSNList = NULL;
    PVDIR_BACKEND_INSTANCE  pMainInstance = NULL;
    PVDIR_BACKEND_INSTANCE  pLogInstance = NULL;

    /* call once to init global maps */
    dwError = _VmDirBackendGlobalsInit();
    BAIL_ON_VMDIR_ERROR(dwError);

    //Main db
    dwError = _VmDirInitInstance(LWRAFT_DB_DIR, &pMainInstance);
    BAIL_ON_VMDIR_ERROR(dwError);

    gVdirBEGlobals.pBE = pMainInstance->pBE;

    /* map main db */
    dwError = _VmDirMapDNToBackend(ALIAS_MAIN, pMainInstance->pBE);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* map raft persist state dn to main */
    dwError = _VmDirMapDNToBackend(RAFT_PERSIST_STATE_DN, pMainInstance->pBE);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* TODO: not applicable for post. remove */
    gVdirBEGlobals.pBE->pfnBEGetLeastOutstandingUSN = VmDirBackendLeastOutstandingUSN;

    gVdirBEGlobals.pBE->pfnBEGetHighestCommittedUSN = VmDirBackendHighestCommittedUSN;

    gVdirBEGlobals.pBE->pfnBEGetMaxOriginatingUSN = VmDirBackendGetMaxOriginatingUSN;

    gVdirBEGlobals.pBE->pfnBESetMaxOriginatingUSN = VmDirBackendSetMaxOriginatingUSN;

    dwError = VmDirAllocateUSNList(&pUSNList);
    BAIL_ON_VMDIR_ERROR(dwError);

    gVdirBEGlobals.pBE->pBEUSNList = pUSNList;
    pUSNList = NULL;


    if (gVmdirGlobals.bUseLogDB)
    {
        dwError = _VmDirInitInstance(LOG1_DB_PATH, &pLogInstance);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _VmDirMapDNToBackend(ALIAS_LOG_CURRENT, pLogInstance->pBE);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _VmDirMapDNToBackend(RAFT_LOGS_CONTAINER_DN, pLogInstance->pBE);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:

    return dwError;

error:

    VmDirFreeUSNList(pUSNList);

    goto cleanup;
}

static
PVDIR_BACKEND_INTERFACE
_LookupBE(
    PCSTR   pszDN
    )
{
    PVDIR_BACKEND_INTERFACE pBE = NULL;

    LwRtlHashMapFindKey(
            gVdirBEGlobals.pDNToBEMap,
            (PVOID*)&pBE,
            pszDN);

    return pBE;
}

/*
 * Select backend based on entry DN
 */
PVDIR_BACKEND_INTERFACE
VmDirBackendSelect(
    PCSTR   pszDN
    )
{
    PVDIR_BACKEND_INTERFACE pBE = gVdirBEGlobals.pBE;

    if (gVmdirGlobals.bUseLogDB)
    {
        if (!IsNullOrEmptyString(pszDN))
        {
            /* allow overrides before matching parts */
            PVDIR_BACKEND_INTERFACE pNewBE = _LookupBE(pszDN);

            if (!pNewBE)
            {
                if (VmDirStringEndsWith(pszDN, RAFT_CONTEXT_DN, FALSE))
                {
                    pszDN = RAFT_LOGS_CONTAINER_DN;
                    pNewBE = _LookupBE(pszDN);
                }
            }

            if (pNewBE)
            {
                pBE = pNewBE;
            }
        }
    }

    return pBE;
}

BOOLEAN
VmDirHasBackend(
    PCSTR pszDN
    )
{
    return _LookupBE(pszDN) ? TRUE : FALSE;
}

DWORD
VmDirInstanceFromBE(
    PVDIR_BACKEND_INTERFACE pBE,
    PVDIR_BACKEND_INSTANCE *ppInstance
    )
{
    DWORD dwError = 0;
    PVDIR_BACKEND_INSTANCE pInstanceFromBE = NULL;

    if (!pBE || !ppInstance)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (LwRtlHashMapFindKey(
            gVdirBEGlobals.pBEToInstanceMap,
            (PVOID*)&pInstanceFromBE,
            pBE) == 0)

    if (!pInstanceFromBE)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_BACKEND_INSTANCE_NOT_FOUND);
    }

    *ppInstance = pInstanceFromBE;

error:
    return dwError;
}


VOID
VmDirBackendContentFree(
    PVDIR_BACKEND_INTERFACE     pBE
    )
{
    if (pBE)
    {
        VmDirFreeUSNList(pBE->pBEUSNList);
        pBE->pBEUSNList = NULL;
    }
}

/*
 * Free PVDIR_BACKEND_CTX and it's content.
 */
VOID
VmDirBackendCtxFree(
    PVDIR_BACKEND_CTX   pBECtx
    )
{
    if ( pBECtx )
    {
        VmDirBackendCtxContentFree( pBECtx );
        VMDIR_SAFE_FREE_MEMORY( pBECtx );
    }

    return;
}

/*
 * Free backend specific resources
 */
VOID
VmDirBackendCtxContentFree(
    PVDIR_BACKEND_CTX   pBECtx)
{
    if ( pBECtx )
    {
        if (pBECtx->pBE && pBECtx->pBEPrivate)
        {
            pBECtx->pBE->pfnBETxnAbort(pBECtx);
        }

        if (pBECtx->pBE && pBECtx->wTxnUSN > 0)
        {
            // NOTE, one operation context may be used by multiple (or nested) txns.  However,
            // we add the first (smallest) UNS to the outstanding list during the very first USN acquire.
            // Now, we are done and hence remove the USN from the list.
            VmDirBackendRemoveOutstandingUSN(pBECtx);
            pBECtx->wTxnUSN = 0;
        }

        VMDIR_SAFE_FREE_MEMORY(pBECtx->pszBEErrorMsg);
    }

    return;
}

/*
 * call this function during server start up.
 * It prepares gVdirBEGlobals.pBE->pBEUSNList such that replication scan can
 *      still call VmDirBackendLeastOutstandingUSN to get a safe USN number even if there is NO write
 *      operations since server starts.
 */
DWORD
VmDirBackendInitUSNList(
    PVDIR_BACKEND_INTERFACE   pBE
    )
{
    DWORD               dwError = 0;
    VDIR_BACKEND_CTX    beCtx = {0};
    USN                 tmpUSN = 0;

    assert( pBE );

    beCtx.pBE = pBE;

    // acquire a USN and also add it to USNList
    dwError = pBE->pfnBEGetNextUSN(&beCtx, &tmpUSN);
    BAIL_ON_VMDIR_ERROR(dwError);

    gVdirBEGlobals.usnFirstNext = tmpUSN;
    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Next available local USN: %lu", tmpUSN);

error:

    VmDirBackendCtxContentFree(&beCtx);

    return dwError;
}

VOID
VmDirBackendGetFirstNextUSN(
    USN *pUSN
    )
{
    assert(pUSN != NULL);

    *pUSN = gVdirBEGlobals.usnFirstNext;
}

/*
 * For those USN not stored in pBEUSNList->pUSNAry (i.e. not the main USN we are tracking for
 * the least outstandind/pending USN per BECtx), we want to track the max.
 *
 * NOTE, this function is called only when we acquire multiple USN per BECTx.
 */
VOID
VmDirBackendSetMaxOutstandingUSN(
    PVDIR_BACKEND_CTX   pBECtx,
    USN                 maxOutstandingUSN
    )
{
    BOOLEAN     bInLock = FALSE;

    assert( pBECtx && pBECtx->pBE && pBECtx->pBE->pBEUSNList);

    VMDIR_LOCK_MUTEX(bInLock, pBECtx->pBE->pBEUSNList->pMutex);

    pBECtx->pBE->pBEUSNList->maxOutstandingUSN = maxOutstandingUSN;

    VMDIR_UNLOCK_MUTEX(bInLock, pBECtx->pBE->pBEUSNList->pMutex);

    VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "set max outstanding USN (%u)", maxOutstandingUSN);

    return;
}

/*
 * Add USN change number to PVDIR_BACKEND_USN_LIST
 */
DWORD
VmDirBackendAddOutstandingUSN(
    PVDIR_BACKEND_CTX      pBECtx
    )
{
    DWORD       dwError = 0;
    BOOLEAN     bFoundSlot = FALSE;
    BOOLEAN     bInLock = FALSE;
    size_t      iCnt = 0;
    PVDIR_BACKEND_USN_LIST  pUSNList = NULL;

    assert( pBECtx && pBECtx->pBE && pBECtx->pBE->pBEUSNList);

    if (pBECtx->wTxnUSN > 0)
    {
        pUSNList = pBECtx->pBE->pBEUSNList;

        VMDIR_LOCK_MUTEX(bInLock, pUSNList->pMutex);

        for (iCnt = 0; bFoundSlot == FALSE && iCnt < pUSNList->iSize; iCnt++)
        {
            if (pUSNList->pUSNAry[iCnt] == 0)
            {
                pUSNList->pUSNAry[iCnt] = pBECtx->wTxnUSN;
                bFoundSlot = TRUE;
            }
        }

        if (! bFoundSlot)
        {
            size_t  iCurrentSize = pUSNList->iSize;
            size_t  iNewSize     = iCurrentSize * 2;

            dwError = VmDirReallocateMemoryWithInit(
                                pUSNList->pUSNAry,
                                (PVOID*)&pUSNList->pUSNAry,
                                iNewSize * sizeof(USN),
                                iCurrentSize * sizeof(USN) );
            BAIL_ON_VMDIR_ERROR(dwError);

            pUSNList->pUSNAry[iCurrentSize] = pBECtx->wTxnUSN;
            pUSNList->iSize = iNewSize;

            if (iNewSize > BE_OUTSTANDING_USN_LIST_SIZE * 4)
            {   // we should not get here normally.  log message if we did.
                VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "Outstanding USN list size (%ld)", iNewSize);
            }
        }

        if (pBECtx->wTxnUSN > pUSNList->maxUsedMainUSN)
        {
            pUSNList->maxUsedMainUSN = pBECtx->wTxnUSN;
        }

        VMDIR_UNLOCK_MUTEX(bInLock, pUSNList->pMutex);

        VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "add outstanding USN (%u).",  pBECtx->wTxnUSN);
    }

cleanup:

    VMDIR_UNLOCK_MUTEX(bInLock, pUSNList->pMutex);

    return dwError;

error:

    goto cleanup;
}

/*
 * Remove USN change number from PVDIR_BACKEND_USN_LIST
 * Also, adjust USNList.maxOutstandingUSN if necessary
 */
VOID
VmDirBackendRemoveOutstandingUSN(
    PVDIR_BACKEND_CTX      pBECtx
    )
{
    BOOLEAN     bFoundTarget = FALSE;
    BOOLEAN     bInLock = FALSE;
    size_t      iCnt = 0;
    int         iPendingCnt = 0;
    USN         minPendingUSN = 0;
    USN         localMaxOutstandingUSN = 0;
    PVDIR_BACKEND_USN_LIST  pUSNList = NULL;

    assert( pBECtx && pBECtx->pBE && pBECtx->pBE->pBEUSNList);

    if (pBECtx->wTxnUSN > 0)
    {
        pUSNList = pBECtx->pBE->pBEUSNList;

        VMDIR_LOCK_MUTEX(bInLock, pUSNList->pMutex);

        localMaxOutstandingUSN = pUSNList->maxOutstandingUSN;

        for (iCnt = 0; iCnt < pUSNList->iSize; iCnt++)
        {
            if (pUSNList->pUSNAry[iCnt] > 0)
            {
                if (minPendingUSN == 0 || pUSNList->pUSNAry[iCnt] < minPendingUSN)
                {
                    minPendingUSN = pUSNList->pUSNAry[iCnt];
                }

                iPendingCnt++;
            }

            if (pUSNList->pUSNAry[iCnt] == pBECtx->wTxnUSN)
            {
                pUSNList->pUSNAry[iCnt] = 0;
                bFoundTarget = TRUE;
            }
        }

        if (bFoundTarget && (iPendingCnt == 1))
        {   // no more pending USN in pUSNAry after this call, adjust maxOutstandingUNS accordingly
            pUSNList->maxOutstandingUSN = VMDIR_MAX( pUSNList->maxUsedMainUSN, pUSNList->maxOutstandingUSN ) + 1;
            localMaxOutstandingUSN = pUSNList->maxOutstandingUSN;
        }

        VMDIR_UNLOCK_MUTEX(bInLock, pUSNList->pMutex);

        VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "rm outstanding USN (%u)(%u)(%u)(%u)",
                  pBECtx->wTxnUSN, minPendingUSN, iPendingCnt, localMaxOutstandingUSN);
    }

    if (! bFoundTarget)
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "Remove outstanding USN (%ld) not found", pBECtx->wTxnUSN);
    }

    // Do NOT reset pBECtx->wTxnUSN here because we could be retry loop.

    return;
}

DWORD
VmDirBackendUniqKeyGetValue(
    PCSTR       pKey,
    PSTR*       ppValue
    )
{
    DWORD               dwError = 0;
    VDIR_BACKEND_CTX    beCtx = {0};
    BOOLEAN             bHasTxn = FALSE;
    PSTR                pValue = NULL;

    if (!pKey || !ppValue)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    beCtx.pBE = VmDirBackendSelect(ALIAS_MAIN);
    dwError = beCtx.pBE->pfnBETxnBegin(&beCtx, VDIR_BACKEND_TXN_READ);
    BAIL_ON_VMDIR_ERROR(dwError);
    bHasTxn = TRUE;

    dwError = beCtx.pBE->pfnBEUniqKeyGetValue(
                            &beCtx, pKey, &pValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppValue = pValue;
    pValue = NULL;

cleanup:
    if (bHasTxn)
    {
        beCtx.pBE->pfnBETxnCommit(&beCtx);
    }
    VMDIR_SAFE_FREE_MEMORY(pValue);
    VmDirBackendCtxContentFree(&beCtx);

    return dwError;

error:
    VMDIR_LOG_INFO( LDAP_DEBUG_BACKEND,
                    "%s error (%d)", __FUNCTION__, dwError );
    goto cleanup;
}

DWORD
VmDirBackendUniqKeySetValue(
    PCSTR       pKey,
    PCSTR       pValue,
    BOOLEAN     bForce
    )
{
    DWORD               dwError = 0;
    VDIR_BACKEND_CTX    beCtx = {0};
    BOOLEAN             bHasTxn = FALSE;
    PSTR                pLocalValue = NULL;

    if (!pKey || !pValue)
    {
        dwError = VMDIR_ERROR_GENERIC;
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    beCtx.pBE = VmDirBackendSelect(ALIAS_MAIN);
    dwError = beCtx.pBE->pfnBETxnBegin(&beCtx, VDIR_BACKEND_TXN_WRITE);
    BAIL_ON_VMDIR_ERROR(dwError);
    bHasTxn = TRUE;

    if (!bForce)
    {
        // Maybe MDB has option to force set already?
        // for now, query to see if key exists.
        dwError = beCtx.pBE->pfnBEUniqKeyGetValue(
                                &beCtx, pKey, &pLocalValue);
        if (dwError == 0)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_TYPE_OR_VALUE_EXISTS);
        }

        if (dwError == VMDIR_ERROR_NOT_FOUND)
        {
            dwError = 0;
        }
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = beCtx.pBE->pfnBEUniqKeySetValue(
                            &beCtx, pKey, pValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    beCtx.pBE->pfnBETxnCommit(&beCtx);
    bHasTxn = FALSE;

cleanup:
    if (bHasTxn)
    {
        beCtx.pBE->pfnBETxnAbort(&beCtx);
    }
    VMDIR_SAFE_FREE_MEMORY(pLocalValue);
    VmDirBackendCtxContentFree(&beCtx);

    return dwError;

error:
    VMDIR_LOG_INFO( LDAP_DEBUG_BACKEND,
                    "%s error (%d)", __FUNCTION__, dwError );
    goto cleanup;
}

/*
 * Least outstanding USN change number.
 * Replication is safe to search USN below this number
 */
static
USN
VmDirBackendLeastOutstandingUSN(
    PVDIR_BACKEND_CTX   pBECtx,
    BOOLEAN             pending // looking for pending transactions only
    )
{
    BOOLEAN     bInLock = FALSE;
    USN         minPendingUSN = 0;
    USN         maxOutstandingUSN = 0;
    size_t      iCnt = 0;
    PVDIR_BACKEND_USN_LIST  pUSNList = NULL;

    assert( pBECtx && pBECtx->pBE && pBECtx->pBE->pBEUSNList );

    pUSNList = pBECtx->pBE->pBEUSNList;

    VMDIR_LOCK_MUTEX(bInLock, pUSNList->pMutex);

    for (iCnt = 0; iCnt < pUSNList->iSize; iCnt++)
    {
        if (pUSNList->pUSNAry[iCnt] > 0)
        {
            if (minPendingUSN == 0 || pUSNList->pUSNAry[iCnt] < minPendingUSN)
            {
                minPendingUSN = pUSNList->pUSNAry[iCnt];
            }
        }
    }

    maxOutstandingUSN = pUSNList->maxOutstandingUSN;

    VMDIR_UNLOCK_MUTEX(bInLock, pUSNList->pMutex);

    return (pending || (minPendingUSN > 0)) ? minPendingUSN : maxOutstandingUSN;
}

/*
 * Highest Committed USN.
 */
static
USN
VmDirBackendHighestCommittedUSN(
    PVDIR_BACKEND_CTX   pBECtx
    )
{
    BOOLEAN     bInLock = FALSE;
    USN         minPendingUSN = 0;
    USN         maxOutstandingUSN = 0;
    size_t      iCnt = 0;
    PVDIR_BACKEND_USN_LIST  pUSNList = NULL;

    assert( pBECtx && pBECtx->pBE && pBECtx->pBE->pBEUSNList );

    pUSNList = pBECtx->pBE->pBEUSNList;

    VMDIR_LOCK_MUTEX(bInLock, pUSNList->pMutex);

    for (iCnt = 0; iCnt < pUSNList->iSize; iCnt++)
    {
        if (pUSNList->pUSNAry[iCnt] > 0)
        {
            if (minPendingUSN == 0 || pUSNList->pUSNAry[iCnt] < minPendingUSN)
            {
                minPendingUSN = pUSNList->pUSNAry[iCnt];
            }
        }
    }

    maxOutstandingUSN = pUSNList->maxOutstandingUSN;

    VMDIR_UNLOCK_MUTEX(bInLock, pUSNList->pMutex);

    return ((minPendingUSN > 0) ? minPendingUSN - 1 : maxOutstandingUSN - 1);
}

static
VOID
VmDirBackendSetMaxOriginatingUSN(
    PVDIR_BACKEND_CTX   pBECtx,
    USN                 maxOriginatingUSN
    )
{
    BOOLEAN     bInLock = FALSE;

    assert( pBECtx && pBECtx->pBE && pBECtx->pBE->pBEUSNList);

    VMDIR_LOCK_MUTEX(bInLock, pBECtx->pBE->pBEUSNList->pMutex);

    pBECtx->pBE->pBEUSNList->maxOriginatingUSN = maxOriginatingUSN;

    VMDIR_UNLOCK_MUTEX(bInLock, pBECtx->pBE->pBEUSNList->pMutex);

    VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE,
                     "Set max originating USN (%u)",
                     maxOriginatingUSN);

    return;
}

static
USN
VmDirBackendGetMaxOriginatingUSN(
    PVDIR_BACKEND_CTX   pBECtx
    )
{
    BOOLEAN     bInLock = FALSE;
    USN maxOriginatingUSN = 0;

    assert( pBECtx && pBECtx->pBE && pBECtx->pBE->pBEUSNList);

    VMDIR_LOCK_MUTEX(bInLock, pBECtx->pBE->pBEUSNList->pMutex);

    maxOriginatingUSN = pBECtx->pBE->pBEUSNList->maxOriginatingUSN;

    VMDIR_UNLOCK_MUTEX(bInLock, pBECtx->pBE->pBEUSNList->pMutex);

    VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE,
                     "Get max originating USN (%u)",
                     maxOriginatingUSN);

    return maxOriginatingUSN;
}

static
DWORD
VmDirAllocateUSNList(
    PVDIR_BACKEND_USN_LIST*      ppUSNList
    )
{
    DWORD       dwError = 0;
    PVDIR_BACKEND_USN_LIST  pUSNList = NULL;

    dwError = VmDirAllocateMemory(sizeof(*pUSNList), (PVOID)&pUSNList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&pUSNList->pMutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(sizeof(USN) * BE_OUTSTANDING_USN_LIST_SIZE, (PVOID)&pUSNList->pUSNAry);
    BAIL_ON_VMDIR_ERROR(dwError);

    pUSNList->iSize = BE_OUTSTANDING_USN_LIST_SIZE;
    *ppUSNList = pUSNList;
    pUSNList = NULL;

cleanup:

    return dwError;

error:

    VmDirFreeUSNList(pUSNList);

    goto cleanup;
}

static
VOID
VmDirFreeUSNList(
    PVDIR_BACKEND_USN_LIST      pUSNList
    )
{
    if (pUSNList != NULL)
    {
        VMDIR_SAFE_FREE_MUTEX(pUSNList->pMutex);
        VMDIR_SAFE_FREE_MEMORY(pUSNList->pUSNAry);
        VMDIR_SAFE_FREE_MEMORY(pUSNList);
    }

    return;
}

/*
  Calls shutdown on backend interface.
  Free all related resources.
  Safe to set pBE to NULL after this.
*/
static
VOID
_VmDirShutdownAndFreeBE(
    PVDIR_BACKEND_INTERFACE pBE,
    PVDIR_DB_HANDLE hDB
    )
{
    if(pBE)
    {
        pBE->pfnBEShutdown(hDB);
        VmDirBackendContentFree(pBE);
        VMDIR_SAFE_FREE_MEMORY(pBE);
    }
}

static
VOID
_VmDirFreeBackendInstance(
    PVDIR_BACKEND_INSTANCE pInstance
    )
{
    if (pInstance != NULL)
    {
        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "%s shutdown: %s", __func__, pInstance->pszDbPath);
        VMDIR_SAFE_FREE_MEMORY(pInstance->pszDbPath);
        _VmDirShutdownAndFreeBE(pInstance->pBE, pInstance->hDB);
        pInstance->pBE = NULL;
        VMDIR_SAFE_FREE_MEMORY(pInstance);
        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "%s shutdown backend complete.", __func__);
    }
}

static
VOID
_VmDirFreeBackendInstancePair(
    PLW_HASHMAP_PAIR pPair,
    LW_PVOID         pUnused
    )
{
    PVDIR_BACKEND_INSTANCE pInstance = (PVDIR_BACKEND_INSTANCE)pPair->pValue;
    _VmDirFreeBackendInstance(pInstance);
}

static
VOID
_VmDirFreeMapPair(
    PLW_HASHMAP_PAIR pPair,
    LW_PVOID         pUnused
    )
{
}

/*
  Shuts down main db, free and set main db refs to NULL.
  Shuts down all additional dbs. Free and set instances to NULL.
*/
VOID
VmDirShutdownAndFreeAllBackends(
    )
{
    PVDIR_BACKEND_INSTANCE pMainInstance = NULL;

    /* find main db, so that it can be freed at the end */
    LwRtlHashMapFindKey(
        gVdirBEGlobals.pInstanceMap,
        (PVOID*)&pMainInstance,
        LWRAFT_DB_DIR);

    /* remove main instance from instance map before iterating */
    LwRtlHashMapRemove(gVdirBEGlobals.pInstanceMap, LWRAFT_DB_DIR, NULL);

    /* Shutdown and free additional instances */
    LwRtlHashMapClear(
        gVdirBEGlobals.pInstanceMap,
        _VmDirFreeBackendInstancePair,
        NULL);
    LwRtlFreeHashMap(&gVdirBEGlobals.pInstanceMap);

    /* Shutdown and free main db */
    _VmDirFreeBackendInstance(pMainInstance);

    /* lookup map made up of pointers already freed above */
    LwRtlHashMapClear(gVdirBEGlobals.pBEToInstanceMap, _VmDirFreeMapPair, NULL);
    LwRtlFreeHashMap(&gVdirBEGlobals.pBEToInstanceMap);

    /* lookup map made up of pointers already freed above */
    LwRtlHashMapClear(gVdirBEGlobals.pDNToBEMap, _VmDirFreeMapPair, NULL);
    LwRtlFreeHashMap(&gVdirBEGlobals.pDNToBEMap);

    gVdirBEGlobals.pInstanceMap = NULL;
    gVdirBEGlobals.pBEToInstanceMap = NULL;
    gVdirBEGlobals.pDNToBEMap = NULL;
}

/* helper function to safely return be from ctx */
PVDIR_BACKEND_INTERFACE
VmDirSafeBEFromCtx(
    PVDIR_BACKEND_CTX pBECtx
    )
{
    PVDIR_BACKEND_INTERFACE pBE = NULL;

    if (pBECtx && pBECtx->pBE)
    {
        pBE = pBECtx->pBE;
    }

    return pBE;
}

/*
 * helper function to safely return hDB given a pBECtx
*/
PVDIR_DB_HANDLE
VmDirSafeDBFromCtx(
    PVDIR_BACKEND_CTX pBECtx
    )
{
    PVDIR_DB_HANDLE hDB = NULL;

    if (pBECtx && pBECtx->pBE)
    {
        hDB = VmDirSafeDBFromBE(pBECtx->pBE);
        assert(hDB);
    }

    return hDB;
}

/*
 * helper function to safely return hDB from a pBE
*/
PVDIR_DB_HANDLE
VmDirSafeDBFromBE(
    PVDIR_BACKEND_INTERFACE pBE
    )
{
    PVDIR_DB_HANDLE hDB = NULL;
    PVDIR_BACKEND_INSTANCE pInstance = NULL;

    if (pBE)
    {
        VmDirInstanceFromBE(pBE, &pInstance);
        if (pInstance)
        {
            hDB = pInstance->hDB;
        }
    }

    return hDB;
}

/*
 * helper function to safely return hDB from path
*/
PVDIR_DB_HANDLE
VmDirSafeDBFromPath(
    PCSTR pszDbPath
    )
{
    PVDIR_DB_HANDLE hDB = NULL;
    PVDIR_BACKEND_INSTANCE pInstance = NULL;

    if (!IsNullOrEmptyString(pszDbPath))
    {
        if (LwRtlHashMapFindKey(
            gVdirBEGlobals.pInstanceMap,
            (PVOID*)&pInstance,
            pszDbPath) == 0)
        {
            if (pInstance)
            {
                hDB = pInstance->hDB;
            }
        }
    }

    return hDB;
}

/*
 * util function to transfer context contents
*/
DWORD
VmDirBackendCtxMoveContents(
    PVDIR_BACKEND_CTX pBECtxFrom,
    PVDIR_BACKEND_CTX pBECtxTo
    )
{
    DWORD dwError = 0;

    if (!pBECtxFrom ||
        !pBECtxTo ||
        !pBECtxFrom->pBE ||
        pBECtxTo->pBE)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pBECtxTo->pBE = pBECtxFrom->pBE;
    pBECtxTo->iBEPrivateRef = pBECtxFrom->iBEPrivateRef;
    pBECtxTo->pBEPrivate = pBECtxFrom->pBEPrivate;
    pBECtxTo->dwBEErrorCode = pBECtxFrom->dwBEErrorCode;
    pBECtxTo->pszBEErrorMsg = pBECtxFrom->pszBEErrorMsg;
    pBECtxTo->wTxnUSN = pBECtxFrom->wTxnUSN;
    pBECtxTo->iMaxScanForSizeLimit = pBECtxFrom->iMaxScanForSizeLimit;
    pBECtxTo->iPartialCandidates = pBECtxFrom->iPartialCandidates;

    memset(pBECtxFrom, 0, sizeof(VDIR_BACKEND_CTX));
error:
    return dwError;
}
