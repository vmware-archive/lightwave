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
BdbTxnBegin(
    PVDIR_BACKEND_CTX        pBECtx,
    VDIR_BACKEND_TXN_MODE    txnMode
    )
{
    DWORD   dwError = 0;
    DB_TXN* pTxn = NULL;

    assert(pBECtx);

    if (pBECtx->pBEPrivate)
    {
        pBECtx->iBEPrivateRef++;
        goto cleanup;
    }

    dwError = gVdirBdbGlobals.bdbEnv->txn_begin( gVdirBdbGlobals.bdbEnv, BDB_PARENT_TXN_NULL, &pTxn, BDB_FLAGS_ZERO );
    BAIL_ON_VMDIR_ERROR(dwError);

    pBECtx->pBEPrivate = (PVOID) pTxn;
	pBECtx->iBEPrivateRef++;

cleanup:

    return dwError;

error:

    if (pTxn)
    {
        pTxn->abort(pTxn);
    }

    pBECtx->pBEPrivate = NULL;
	pBECtx->iBEPrivateRef = 0;

    dwError = BdbToBackendError(dwError, 0, ERROR_BACKEND_ERROR, pBECtx);

    VMDIR_SAFE_FREE_MEMORY(pBECtx->pszBEErrorMsg);
    VmDirAllocateStringA(db_strerror(pBECtx->dwBEErrorCode), &pBECtx->pszBEErrorMsg);

    goto cleanup;
}

DWORD
BdbTxnAbort(
    PVDIR_BACKEND_CTX   pBECtx
    )
{
    DWORD   dwError = 0;
    DB_TXN* pTxn = NULL;

    assert(pBECtx);

    if (pBECtx->pBEPrivate)
    {
        assert(pBECtx->iBEPrivateRef >= 1);
        pBECtx->iBEPrivateRef--;

        if (pBECtx->iBEPrivateRef == 0)
        {
            pTxn = (DB_TXN*)pBECtx->pBEPrivate;
            pBECtx->pBEPrivate = NULL;

            dwError = pTxn->abort(pTxn);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:

    return dwError;

error:

    dwError = BdbToBackendError(dwError, 0, ERROR_BACKEND_ERROR, pBECtx);

    VMDIR_SAFE_FREE_MEMORY(pBECtx->pszBEErrorMsg);
    VmDirAllocateStringA(db_strerror(pBECtx->dwBEErrorCode), &pBECtx->pszBEErrorMsg);

    goto cleanup;
}

DWORD
BdbTxnCommit(
    PVDIR_BACKEND_CTX   pBECtx
    )
{
    DWORD   dwError = 0;
    DB_TXN* pTxn = NULL;

    assert(pBECtx);

    if (pBECtx->pBEPrivate)
    {
        assert(pBECtx->iBEPrivateRef >= 1);
        pBECtx->iBEPrivateRef--;

        if (pBECtx->iBEPrivateRef == 0)
        {
            pTxn = (DB_TXN*)pBECtx->pBEPrivate;
            pBECtx->pBEPrivate = NULL;

            dwError = pTxn->commit(pTxn, BDB_FLAGS_ZERO);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:

    return dwError;

error:

    dwError = BdbToBackendError(dwError, 0, ERROR_BACKEND_ERROR, pBECtx);

    VMDIR_SAFE_FREE_MEMORY(pBECtx->pszBEErrorMsg);
    VmDirAllocateStringA(db_strerror(pBECtx->dwBEErrorCode), &pBECtx->pszBEErrorMsg);

    goto cleanup;
}
