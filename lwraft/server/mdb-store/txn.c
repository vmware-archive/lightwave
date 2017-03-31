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
VmDirMDBTxnBegin(
    PVDIR_BACKEND_CTX        pBECtx,
    VDIR_BACKEND_TXN_MODE    txnMode
    )
{
    DWORD           dwError = 0;
    PVDIR_DB_TXN    pTxn = NULL;
    unsigned int    iMDBFlag = 0;

    assert(pBECtx);

    if (pBECtx->pBEPrivate)
    {
        pBECtx->iBEPrivateRef++;
        goto cleanup;
    }

    if (txnMode == VDIR_BACKEND_TXN_READ)
    {
        iMDBFlag |= MDB_RDONLY;
    }

    dwError = mdb_txn_begin( gVdirMdbGlobals.mdbEnv, BE_DB_PARENT_TXN_NULL, iMDBFlag, &pTxn );
    BAIL_ON_VMDIR_ERROR(dwError);

    pBECtx->pBEPrivate = (PVOID) pTxn;
    pBECtx->iBEPrivateRef++;

cleanup:

    return dwError;

error:

    if (pTxn)
    {
        mdb_txn_abort(pTxn);
    }

    pBECtx->pBEPrivate = NULL;
    pBECtx->iBEPrivateRef = 0;

    dwError = MDBToBackendError(dwError, 0, ERROR_BACKEND_ERROR, pBECtx, "TxnBegin");

    VMDIR_SAFE_FREE_MEMORY(pBECtx->pszBEErrorMsg);
    VmDirAllocateStringA(mdb_strerror(pBECtx->dwBEErrorCode), &pBECtx->pszBEErrorMsg);

    goto cleanup;
}

DWORD
VmDirMDBTxnAbort(
    PVDIR_BACKEND_CTX   pBECtx
    )
{
    PVDIR_DB_TXN    pTxn = NULL;

    assert(pBECtx);

    if (pBECtx->pBEPrivate)
    {
        assert(pBECtx->iBEPrivateRef >= 1);
        pBECtx->iBEPrivateRef--;

        if (pBECtx->iBEPrivateRef == 0)
        {
            pTxn = (PVDIR_DB_TXN)pBECtx->pBEPrivate;
            pBECtx->pBEPrivate = NULL;

            mdb_txn_abort(pTxn);
        }
    }

    return 0;
}

DWORD
VmDirMDBTxnCommit(
    PVDIR_BACKEND_CTX   pBECtx
    )
{
    DWORD           dwError = 0;
    PVDIR_DB_TXN    pTxn = NULL;

    assert(pBECtx);

    if (pBECtx->pBEPrivate)
    {
        assert(pBECtx->iBEPrivateRef >= 1);
        pBECtx->iBEPrivateRef--;

        if (pBECtx->iBEPrivateRef == 0)
        {
            pTxn = (PVDIR_DB_TXN)pBECtx->pBEPrivate;
            pBECtx->pBEPrivate = NULL;

            dwError = mdb_txn_commit(pTxn);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:

    return dwError;

error:

    dwError = MDBToBackendError(dwError, 0, ERROR_BACKEND_ERROR, pBECtx, "TxnCommit");

    VMDIR_SAFE_FREE_MEMORY(pBECtx->pszBEErrorMsg);
    VmDirAllocateStringA(mdb_strerror(pBECtx->dwBEErrorCode), &pBECtx->pszBEErrorMsg);

    goto cleanup;
}
