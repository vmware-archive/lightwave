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

    gVdirBEGlobals.pszBERootDN = "";
    gVdirBEGlobals.usnFirstNext = USN_SEQ_INITIAL_VALUE;
    gVdirBEGlobals.pBE = NULL;

#ifdef HAVE_DB_H
    gVdirBEGlobals.pBE = BdbBEInterface();
#endif

#ifdef HAVE_TCBDB_H
    gVdirBEGlobals.pBE = VmDirTCBEInterface();
#endif

#ifdef HAVE_LMDB_H
    gVdirBEGlobals.pBE = VmDirMDBBEInterface();
#endif

    gVdirBEGlobals.pBE->pfnBEGetLeastOutstandingUSN = VmDirBackendLeastOutstandingUSN;

    gVdirBEGlobals.pBE->pfnBEGetHighestCommittedUSN = VmDirBackendHighestCommittedUSN;

    gVdirBEGlobals.pBE->pfnBEGetMaxOriginatingUSN = VmDirBackendGetMaxOriginatingUSN;

    gVdirBEGlobals.pBE->pfnBESetMaxOriginatingUSN = VmDirBackendSetMaxOriginatingUSN;

    dwError = VmDirAllocateUSNList(&pUSNList);
    BAIL_ON_VMDIR_ERROR(dwError);
    gVdirBEGlobals.pBE->pBEUSNList = pUSNList;
    pUSNList = NULL;

cleanup:

    return dwError;

error:

    VmDirFreeUSNList(pUSNList);

    goto cleanup;
}

/*
 * Select backend based on entry DN
 */
PVDIR_BACKEND_INTERFACE
VmDirBackendSelect(
    PCSTR   pszDN)
{
    // only have one backend for now
    return gVdirBEGlobals.pBE;
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
