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
VOID
_VmDirFreeTxnLogs(
    PVMDIR_THREAD_TXN_CONTEXT pThreadTxnContext
    );

DWORD
VmDirMDBTxnBegin(
    PVDIR_BACKEND_CTX     pBECtx,
    VDIR_BACKEND_TXN_MODE txnMode,
    PBOOLEAN              pBnewTxn
    )
{
    DWORD           dwError = 0;
    PVDIR_DB_TXN    pParentTxn = NULL;
    PVDIR_DB_TXN    pChildTxn = NULL;
    PVDIR_MDB_DB    pDB = NULL;
    unsigned int    iMDBFlag = 0;
    PVMDIR_THREAD_TXN_CONTEXT pThreadTxnContext = NULL;
    PVMDIR_TXN_CONTEXT pTxnContext = NULL;
    PSTR    pszLocalErrMsg = NULL;
    BOOLEAN bNewTxn = FALSE;
    UINT64 txn_ms = {0};

    assert(pBECtx);
    assert(pBECtx->pBE);

    dwError = VmDirAllocAndSetThrTxnCtx(&pThreadTxnContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    assert(pThreadTxnContext);

    pDB = VmDirSafeDBFromCtx(pBECtx);
    assert(pDB);

    if (pDB->bIsMainDB)
    {
        pTxnContext = &pThreadTxnContext->mainDbTxnCtx;

        // This if -- else block only sets the new txn mode (read-only or write)
        // based on whether there is already a outstanding transaction and
        // the requested mode (txnMode); iDBFlag is target mode.
        // pTxnContext->txnMode tell the outstanding transaction read/write mode.
        // The block doesn't change the state in per thread transaction struct, nor the mode.
        // The rules are:
        //   The new mode can be set to read-only only iff there no any
        //   outstanding tranaction or there is only a regular transaction under
        //   which there is already in read-only mode.
        //   The logic also needs to error out invalid mode down grade.
        //   since MDB wouldn't support those operations.
        if (pTxnContext->txnState == TXN_NONE)
        {
            // Only regular ransaction can get here because user txn always get to TXN_PENDING first
            iMDBFlag = (txnMode == VDIR_BACKEND_TXN_READ)?MDB_RDONLY:BE_DB_FLAGS_ZERO;
        } else
        {
            if (pTxnContext->txnMode == VDIR_BACKEND_TXN_WRITE)
            {
                //Request txnMode is ignored since downgrade (from write to read) now allowed.
                iMDBFlag = BE_DB_FLAGS_ZERO;
            } else if (txnMode == VDIR_BACKEND_TXN_WRITE)
            {
               // Now the oustanding mdb transaction is in read-only mode
               // or there is no outstanding mdb transaction due to user txn pending.
               if (pTxnContext->txnState == TXN_PENDING_START)
               {
                   // User transaction always uses write mode
                   iMDBFlag = BE_DB_FLAGS_ZERO;
               } else
               {
                   //attemp to upgrade from read to write - allowed by MDB.
                   dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
                   BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg,
                                                "txn mode upgrade from read to write not allowed");
               }
            } else if (pTxnContext->txnState == TXN_PENDING_START ||
                       pTxnContext->txnState == TXN_USER_IN_PROGRESS)
            {
               //Search when an user transaction pending or in progress, alwasy use write txn mode 
               iMDBFlag = BE_DB_FLAGS_ZERO;
            } else
            {
                iMDBFlag = MDB_RDONLY;
            }
        }

        // This block will do per thread state transaction based on:
        // 1. the current state recorded in the per thread struct.
        // 2. whether the request has a match user-txn-id tracked in the per-thread struct.
        // 3. whether there is a outstanding parent or a child transaction (trakced in the per-thread struct)
        //    note that child transaction is used only for user defined transaction.
        switch(pTxnContext->txnState)
        {
          case TXN_PENDING_START:
            // A user transaction pending
            // The begin is invoked from the first client originated OP.
            assert(pTxnContext->pBE == NULL && pTxnContext->pTxnParent == NULL && pTxnContext->pTxnChild == NULL);
            pTxnContext->pBE = (PVDIR_GENERIC_BACKEND_INTERFACE)pBECtx->pBE;

            if (pBECtx->pszTxnId == NULL || VmDirStringCompareA(pBECtx->pszTxnId, pTxnContext->pszTxnId, FALSE)!=0)
            {
               /* The client is in user defined transaction, but the request has an txn-id different
                * from the previous request, or the request doesn't supply an txn-id.
                */
               dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
               BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "No txn id supplied by client while pending user txn");
            }

            VmDirWtxnOutstandingInc();
            dwError = mdb_txn_begin(pDB->mdbEnv, BE_DB_PARENT_TXN_NULL, iMDBFlag, &pParentTxn );
            VmDirWtxnOutstandingDec();
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "mdb_txn_begin failed");
            bNewTxn = TRUE;

            dwError = mdb_txn_begin(pDB->mdbEnv, pParentTxn, iMDBFlag, &pChildTxn);
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "mdb_txn_begin failed");

            pTxnContext->txnState = TXN_USER_IN_PROGRESS;
            pTxnContext->pTxnParent = (PVDIR_TXN_HANDLE)pParentTxn;
            pTxnContext->pTxnChild = (PVDIR_TXN_HANDLE)pChildTxn;
            pTxnContext->txnMode = VDIR_BACKEND_TXN_WRITE;
            pBECtx->pBEPrivate = pChildTxn;
            break;

          case TXN_USER_IN_PROGRESS:
            assert(pTxnContext->pTxnParent);
            assert(pTxnContext->pszTxnId);
            if (pTxnContext->txnMode == VDIR_BACKEND_TXN_READ && txnMode == VDIR_BACKEND_TXN_WRITE)
            {
               dwError = VMDIR_ERROR_INVALID_PARAMETER;
               BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg,
                 "invalid attempt to upgrade user txn from READ to WRITE - aborting user txn");
            }

            if (pTxnContext->pTxnChild)
            {
                pBECtx->pBEPrivate = pTxnContext->pTxnChild;
                break;
            }

            if (pBECtx->pszTxnId && VmDirStringCompareA(pBECtx->pszTxnId, pTxnContext->pszTxnId, FALSE) != 0)
            {
               /* The client is in user defined transaction, but the request has an txn-id different
                * from the previous request.
                */
               dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
               BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg,
                 "No txn id supplied by client within user txn - aborting user txn");
            }

            txn_ms = VmDirGetTimeInMilliSec() - pTxnContext->txnStartTime;
            if (txn_ms > gVmdirGlobals.dwLdapUserTxnMaxtimeMS)
            {
               dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
               BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "user transaction time (%d ms) exceeded (%d ms)",
                                            txn_ms,  gVmdirGlobals.dwLdapUserTxnMaxtimeMS);
            }

            //Get here during user transaction, but have no outstandig child transaction:
            // 1. pBECtx->pszTxnId match the TxnId for the user transaction.
            //    - i.e. the top level call to begin a (child) transaction, or
            // 2 pBECtx->pszTxnId is NULL - non top level call to begin a (child) transaction.
            //   e.g. called from post plugin, which allow to fail as long as it can be rolled back. 
            dwError = mdb_txn_begin(pDB->mdbEnv, pTxnContext->pTxnParent, iMDBFlag, &pChildTxn );
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "mdb_txn_begin failed");
            bNewTxn = TRUE;

            pTxnContext->pTxnChild = (PVDIR_TXN_HANDLE)pChildTxn;
            pBECtx->pBEPrivate = pChildTxn;
            break;

          case TXN_NONE:
            //there is no outstanding transaction context on mainDB
            assert(pTxnContext->pTxnParent == NULL);
            if (pBECtx->pszTxnId)
            {
               /* The client supplied with a txn-id (control), but has not start a user transaction.
                */
               dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
               BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "txn id supplied but no outstanding user txn");
            }

            if (iMDBFlag == BE_DB_FLAGS_ZERO)
            {
                VmDirWtxnOutstandingInc();
            }
            dwError = mdb_txn_begin(pDB->mdbEnv, BE_DB_PARENT_TXN_NULL, iMDBFlag, &pParentTxn );
            if (iMDBFlag == BE_DB_FLAGS_ZERO)
            {
                VmDirWtxnOutstandingDec();
            }
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "mdb_txn_begin failed");
            bNewTxn = TRUE;

            pTxnContext->pBE = (PVDIR_GENERIC_BACKEND_INTERFACE)pBECtx->pBE;
            pTxnContext->txnState = TXN_OP_IN_PROGRESS;
            pTxnContext->pTxnParent = (PVDIR_TXN_HANDLE)pParentTxn;
            pTxnContext->txnMode = txnMode;
            pBECtx->pBEPrivate = pParentTxn;
            break;

          case TXN_OP_IN_PROGRESS:
            //an standalone txn is in progress on the backend, simply pass the txn handle.
            assert(pTxnContext->pTxnChild == NULL && pTxnContext->pTxnParent);
            if (pTxnContext->txnMode == VDIR_BACKEND_TXN_READ && txnMode == VDIR_BACKEND_TXN_WRITE)
            {
               dwError = VMDIR_ERROR_INVALID_PARAMETER;
               BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg,
                 "invalid attempt to upgrade a standalone transaction from READ to WRITE");
            }
            pBECtx->pBEPrivate = pTxnContext->pTxnParent;
            break;

          default:
            dwError = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "Invalid transaction state on mainDb");
            break;
        }
    } else
    {
        //On log DB, the rules are the same as the main DB, but much simplified
        //  because there is no user-txn on log DB.
        pTxnContext = &pThreadTxnContext->logDbTxnCtx;

        if (pTxnContext->txnState == TXN_NONE)
        {
            iMDBFlag = (txnMode == VDIR_BACKEND_TXN_READ)?MDB_RDONLY:BE_DB_FLAGS_ZERO;
        } else
        {
            if (pTxnContext->txnMode == VDIR_BACKEND_TXN_WRITE)
            {
                //txnMode is ignored since MDB not allowed downgrade from write to read.
                iMDBFlag = BE_DB_FLAGS_ZERO;
            } else if (txnMode == VDIR_BACKEND_TXN_WRITE)
            {
               //upgrade from read to write is not allowed by MDB.
               dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
               BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "txn mode upgrade from read to write not allowed");
            } else
            {
                iMDBFlag = MDB_RDONLY;
            }
        }

        //Requring that pTxnContext->pBE set to NULL when any transaction on log db commmit or abort.
        assert(pTxnContext->pBE == NULL || pTxnContext->pBE == (PVDIR_GENERIC_BACKEND_INTERFACE)pBECtx->pBE);
        if (pTxnContext->txnState == TXN_NONE)
        {
            dwError = mdb_txn_begin(pDB->mdbEnv, BE_DB_PARENT_TXN_NULL, iMDBFlag, &pParentTxn );
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "mdb_txn_begin failed");
            bNewTxn = TRUE;
            pTxnContext->pBE = (PVDIR_GENERIC_BACKEND_INTERFACE)pBECtx->pBE;
            pTxnContext->pTxnParent = (PVDIR_TXN_HANDLE)pParentTxn;
            pTxnContext->txnMode = txnMode;
            pBECtx->pBEPrivate = pParentTxn;
            pTxnContext->txnState = TXN_OP_IN_PROGRESS;
        } else if (pTxnContext->txnState == TXN_OP_IN_PROGRESS)
        {
           assert(pTxnContext->pBE);
           assert(pTxnContext->pTxnParent);
           pBECtx->pBEPrivate = pTxnContext->pTxnParent;
        } else
        {
            dwError = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "Invalid transaction state on logDb");
        }
    }

cleanup:
    *pBnewTxn = bNewTxn;
    VMDIR_SAFE_FREE_MEMORY(pBECtx->pszBEErrorMsg);
    return dwError;

error:
    if (pTxnContext)
    {
        _VmDirFreeTxnLogs(pThreadTxnContext);

        if (pTxnContext->pTxnParent)
        {
            mdb_txn_abort((PVDIR_DB_TXN)pTxnContext->pTxnParent);
        }
        bNewTxn = FALSE;
        pTxnContext->txnState = TXN_NONE;
        pTxnContext->pTxnParent = NULL;
        pTxnContext->pTxnChild = NULL;
        pBECtx->pBEPrivate = NULL;
        pTxnContext->pBE = NULL;
        VMDIR_SAFE_FREE_MEMORY(pTxnContext->pszTxnId);
    }

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: error: %s errcode: %d", __func__,
                    VDIR_SAFE_STRING(pszLocalErrMsg), dwError);

    pBECtx->pszBEErrorMsg = pszLocalErrMsg;
    goto cleanup;
}

DWORD
VmDirMDBTxnAbort(
    PVDIR_BACKEND_CTX   pBECtx
    )
{
    DWORD dwError = 0;
    PVMDIR_THREAD_TXN_CONTEXT pThreadTxnContext = NULL;
    PVMDIR_TXN_CONTEXT pTxnContext = NULL;
    PVDIR_MDB_DB pDB = NULL;
    PSTR pszLocalErrMsg = NULL;

    assert(pBECtx);
    assert(pBECtx->pBE);

    dwError = VmDirGetThreadTxnContextValue(&pThreadTxnContext);
    assert(pThreadTxnContext);

    pDB = VmDirSafeDBFromCtx(pBECtx);
    assert(pDB);

    if (pDB->bIsMainDB)
    {
        pTxnContext = &pThreadTxnContext->mainDbTxnCtx;
        if(pTxnContext->txnState == TXN_NONE)
        {
           //already aborted by lower call stack
           goto cleanup;
        }

        assert(pTxnContext->pBE);

        if (pTxnContext->pBE != (PVDIR_GENERIC_BACKEND_INTERFACE)pBECtx->pBE)
        {
            dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "abort on a wrong mainDb backend");
        }

        if (pTxnContext->txnState == TXN_USER_IN_PROGRESS)
        {
            if (pTxnContext->pTxnChild)
            {
                //Abort transaction for LDAP op within user transaction.
                mdb_txn_abort((PVDIR_DB_TXN)pTxnContext->pTxnChild);
                pTxnContext->pTxnChild = NULL;
                pBECtx->pBEPrivate = NULL;
            } else if (pTxnContext->pTxnParent)
            {
                //Tansaction abort is from extended op/txn-abort
                _VmDirFreeTxnLogs(pThreadTxnContext);

                mdb_txn_abort((PVDIR_DB_TXN)pTxnContext->pTxnParent);
                pTxnContext->pTxnParent = NULL;
                pTxnContext->pTxnChild = NULL;
                pBECtx->pBEPrivate = NULL;
                pTxnContext->txnState = TXN_NONE;
            } else
            {
                VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,  "%s: no outstanding transaction, igored", __func__);
            }
        } else if (pTxnContext->txnState == TXN_OP_IN_PROGRESS)
        {
            //Individual transaction abort.
            assert(pTxnContext->pTxnChild == NULL);
            assert(pTxnContext->pTxnParent);

            _VmDirFreeTxnLogs(pThreadTxnContext);

            mdb_txn_abort((PVDIR_DB_TXN)pTxnContext->pTxnParent);
            pTxnContext->pTxnParent = NULL;
            pTxnContext->pTxnChild = NULL;
            pBECtx->pBEPrivate = NULL;
            pTxnContext->txnState = TXN_NONE;
        } else
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,  "%s: no outstanding transaction, igored", __func__);
        }
        _VmDirFreeTxnLogs(pThreadTxnContext);
        goto cleanup;
    }

    // abort a logDb transaction.
    pTxnContext = &pThreadTxnContext->logDbTxnCtx;
    assert(pTxnContext->pBE);
    assert(pTxnContext->pTxnChild == NULL);
    assert(pTxnContext->pTxnParent);
    assert(pTxnContext->txnState == TXN_OP_IN_PROGRESS);
    mdb_txn_abort((PVDIR_DB_TXN)pTxnContext->pTxnParent);
    pTxnContext->pTxnParent = NULL;
    pBECtx->pBEPrivate = NULL;
    pTxnContext->txnState = TXN_NONE;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pBECtx->pszBEErrorMsg);
    return 0;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: error: %s errcode: %d", __func__,
                    VDIR_SAFE_STRING(pszLocalErrMsg), dwError);
    goto cleanup;
}

static
VOID
_VmDirFreeTxnLogs(PVMDIR_THREAD_TXN_CONTEXT pThreadTxnContext)
{
    PVOID pChgLog = NULL;

    if (pThreadTxnContext == NULL)
    {
        goto done;
    }

    if (pThreadTxnContext->chgLogs.iSize > 0)
    {
        while(!dequeIsEmpty(&pThreadTxnContext->chgLogs))
        {
            dequePopLeft(&pThreadTxnContext->chgLogs, (PVOID*)&pChgLog);
            VmDirChgLogFree(pChgLog, TRUE);
        }
    }

done:
    return;
}

/*
 * This is called when the thread exits to abort all left over transactions
 * which were started but not commited/aborted explictly by the thread,
 * e.g. a client disconnected but not sent commit/abort its user transaction
 */
VOID
VmDirMDBTxnAbortAll(
    )
{
    PVMDIR_THREAD_TXN_CONTEXT pThreadTxnContext = NULL;
    PVMDIR_TXN_CONTEXT pTxnContext = NULL;

    VmDirGetThreadTxnContextValue(&pThreadTxnContext);
    assert(pThreadTxnContext);

    _VmDirFreeTxnLogs(pThreadTxnContext);

    pTxnContext = &pThreadTxnContext->mainDbTxnCtx;

    if (pTxnContext->pTxnParent)
    {
        mdb_txn_abort((PVDIR_DB_TXN)pTxnContext->pTxnParent);
        pTxnContext->pTxnParent = NULL;
        pTxnContext->pTxnChild = NULL;
        pTxnContext->txnState = TXN_NONE;
        pTxnContext->pBE = NULL;
    }
    VMDIR_SAFE_FREE_MEMORY(pTxnContext->pszTxnId);

    pTxnContext = &pThreadTxnContext->logDbTxnCtx;

    if (pTxnContext->pTxnParent)
    {
        mdb_txn_abort((PVDIR_DB_TXN)pTxnContext->pTxnParent);
        pTxnContext->pTxnParent = NULL;
        pTxnContext->pTxnChild = NULL;
        pTxnContext->txnState = TXN_NONE;
        pTxnContext->pBE = NULL;
    }
}

DWORD
VmDirMDBTxnCommit(
    PVDIR_BACKEND_CTX   pBECtx
    )
{
    DWORD dwError = 0;
    PVMDIR_THREAD_TXN_CONTEXT pThreadTxnContext = NULL;
    PVMDIR_TXN_CONTEXT pTxnContext = NULL;
    PVDIR_MDB_DB pDB = NULL;
    PSTR pszLocalErrMsg = NULL;

    assert(pBECtx);
    assert(pBECtx->pBE);

    dwError = VmDirGetThreadTxnContextValue(&pThreadTxnContext);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrMsg), "VmDirGetThreadTxnContextValue");

    assert(pThreadTxnContext);
    pDB = VmDirSafeDBFromCtx(pBECtx);
    assert(pDB);

    if (pDB->bIsMainDB)
    {
        pTxnContext = &pThreadTxnContext->mainDbTxnCtx;
        if(pTxnContext->txnState == TXN_NONE)
        {
           //already aborted by lower call stack
           dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
           BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "transaction has been aborted");
        }

        assert(pTxnContext->pBE);

        if (pTxnContext->pBE != (PVDIR_GENERIC_BACKEND_INTERFACE)pBECtx->pBE)
        {
            dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "commit on a wrong mainDb backend");
        }

        if (pTxnContext->txnState == TXN_USER_IN_PROGRESS)
        {
            if (pTxnContext->pTxnChild)
            {
               assert(pTxnContext->pTxnParent);
               dwError = mdb_txn_commit((PVDIR_DB_TXN)pTxnContext->pTxnChild);
               pTxnContext->pTxnChild = NULL;
               pBECtx->pBEPrivate = NULL;
               BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrMsg), "mdb_txn_commit on child txn");
            } else if (pTxnContext->pTxnParent)
            {
                //Transaction commit is from extended op/txn-abort, need a validation
                dwError = VmDirEncodeLogEntry();
                BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrMsg), "VmDirEncodeLogEntry error during user transction");

                dwError = mdb_txn_commit((PVDIR_DB_TXN)pTxnContext->pTxnParent);
                pTxnContext->pTxnParent = NULL;
                pBECtx->pBEPrivate = NULL;
                pTxnContext->txnState = TXN_NONE;
                pTxnContext->pBE = NULL;
                BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrMsg), "mdb_txn_commit on parent during user transction");
                VMDIR_LOG_INFO(LDAP_DEBUG_BACKEND, "%s: commit user txn parent succeeded", __func__);
            } else
            {
               dwError = LDAP_OPERATIONS_ERROR;
               BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrMsg),
                  "try to commit a user transaction but there is no outstanding user transction");
            }
        } else if (pTxnContext->txnState == TXN_OP_IN_PROGRESS)
        {
            //Commit individual transaction
            assert(pTxnContext->pTxnChild == NULL);
            assert(pTxnContext->pTxnParent);

            dwError = VmDirEncodeLogEntry();
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrMsg), "VmDirEncodeLogEntry error during single op");

            dwError = mdb_txn_commit((PVDIR_DB_TXN)pTxnContext->pTxnParent);
            pTxnContext->pTxnParent = NULL;
            pBECtx->pBEPrivate = NULL;
            pTxnContext->txnState = TXN_NONE;
            pTxnContext->pBE = NULL;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrMsg), "mdb_txn_commit on parent txn during single op");
        } else
        {
            dwError = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrMsg),
                  "try to commit stanalone transaction on mainDB, but there is no outstanding transction");
        }
        goto cleanup;
    }

    // commit logDB
    pTxnContext = &pThreadTxnContext->logDbTxnCtx;
    assert(pTxnContext->pBE);
    assert(pTxnContext->pTxnChild == NULL);
    assert(pTxnContext->pTxnParent);
    assert(pTxnContext->txnState == TXN_OP_IN_PROGRESS);
    dwError = mdb_txn_commit((PVDIR_DB_TXN)pTxnContext->pTxnParent);
    pTxnContext->pTxnParent = NULL;
    pBECtx->pBEPrivate = NULL;
    pTxnContext->txnState = TXN_NONE;
    pTxnContext->pBE = NULL;
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrMsg), "mdb_txn_commit on logDB");
    if (pTxnContext->txnMode == VDIR_BACKEND_TXN_WRITE)
    {
       VMDIR_LOG_INFO(LDAP_DEBUG_BACKEND, "%s: commit log for write succeeded", __func__);
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);
    return dwError;

error:
    dwError = MDBToBackendError(dwError, 0, ERROR_BACKEND_ERROR, pBECtx, "TxnCommit");
    if (pTxnContext)
    {
        _VmDirFreeTxnLogs(pThreadTxnContext);

        if(pTxnContext->pTxnParent)
        {
            mdb_txn_abort((PVDIR_DB_TXN)pTxnContext->pTxnParent);
        }
        pTxnContext->txnState = TXN_NONE;
        pTxnContext->pTxnChild = NULL;
        pTxnContext->pTxnParent = NULL;
        pBECtx->pBEPrivate = NULL;
        pTxnContext->pBE = NULL;
    }
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,  "%s: %s error %d", __func__, VDIR_SAFE_STRING(pszLocalErrMsg), dwError);
    VMDIR_SAFE_FREE_MEMORY(pBECtx->pszBEErrorMsg);
    VmDirAllocateStringA(mdb_strerror(pBECtx->dwBEErrorCode), &pBECtx->pszBEErrorMsg);
    goto cleanup;
}

/* Called from ldap-head only, not for calling from internal add/modify/delete
 * If there is any outstanding txn context,
 * it is invalid if a new operation from exnternal operation doens't
 * supply txn-id (in control) or that txn-id doesn't match
 * the ones from previous operations.
 */
BOOLEAN
VmDirValidTxnState(
    PVDIR_BACKEND_CTX pBECtx,
    ber_tag_t reqCode
    )
{
    BOOLEAN bValidOp = TRUE;
    PVMDIR_THREAD_TXN_CONTEXT pThreadTxnContext = NULL;
    PVMDIR_TXN_CONTEXT pTxnContext = NULL;
    PVDIR_MDB_DB pDB = NULL;

    assert(pBECtx);
    assert(pBECtx->pBE);
    pDB = VmDirSafeDBFromCtx(pBECtx);
    assert(pDB);

    if (reqCode != LDAP_REQ_SEARCH && !pDB->bIsMainDB)
    {
       //Only search is allowed on log db for external operation.
       bValidOp = FALSE;
       goto done;
    }

    VmDirGetThreadTxnContextValue(&pThreadTxnContext);

    if (pThreadTxnContext == NULL)
    {
       //It is OK if there is no txn context yet.
       goto done;
    }

    pTxnContext = &pThreadTxnContext->mainDbTxnCtx;

    if(pTxnContext->txnState != TXN_USER_IN_PROGRESS)
    {
      goto done;
    }

    assert(pTxnContext->pszTxnId);

    if(pBECtx->pszTxnId == NULL ||
       VmDirStringCompareA(pBECtx->pszTxnId, pTxnContext->pszTxnId, FALSE) != 0)
    {
        bValidOp = FALSE;
    }

done:
    return bValidOp;
}

BOOLEAN
VmDirIsInUserTxn(
    PVDIR_BACKEND_CTX pBECtx
    )
{
    BOOLEAN bIsInUserTxn = FALSE;
    PVMDIR_THREAD_TXN_CONTEXT pThreadTxnContext = NULL;
    PVMDIR_TXN_CONTEXT pTxnContext = NULL;

    assert(pBECtx);

    VmDirGetThreadTxnContextValue(&pThreadTxnContext);

    if (pThreadTxnContext == NULL)
    {
       //It is OK if there is no txn context yet.
       goto cleanup;
    }

    pTxnContext = &pThreadTxnContext->mainDbTxnCtx;

    if(pTxnContext->txnState == TXN_USER_IN_PROGRESS ||
       pTxnContext->txnState == TXN_PENDING_START)
    {
        bIsInUserTxn = TRUE;
    }

cleanup:
    return bIsInUserTxn;
}
