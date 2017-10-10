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
VmDirMDBIndexIteratorInit(
    PVDIR_INDEX_CFG                 pIndexCfg,
    PSTR                            pszInitVal,
    PVDIR_BACKEND_INDEX_ITERATOR*   ppIterator
    )
{
    DWORD   dwError = 0;
    VDIR_DB mdbDBi = 0;
    PSTR            pszInitKey = NULL;
    VDIR_DB_DBT     key = {0};
    VDIR_DB_DBT     value = {0};
    PSTR            pszVal = NULL;
    ENTRYID         eId = 0;
    PVDIR_DB_TXN    pTxn = NULL;
    PVDIR_DB_DBC    pCursor = NULL;
    PVDIR_BACKEND_INDEX_ITERATOR    pIterator = NULL;
    PVDIR_MDB_INDEX_ITERATOR        pMdbIterator = NULL;

    if (!pIndexCfg || !ppIterator)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_BACKEND_INDEX_ITERATOR),
            (PVOID*)&pIterator);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_MDB_INDEX_ITERATOR),
            (PVOID*)&pMdbIterator);
    BAIL_ON_VMDIR_ERROR(dwError);

    pIterator->pIterator = (PVOID)pMdbIterator;

    dwError = VmDirMDBIndexGetDBi(pIndexCfg, &mdbDBi);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = mdb_txn_begin(gVdirMdbGlobals.mdbEnv, NULL, MDB_RDONLY, &pTxn);
    BAIL_ON_VMDIR_ERROR(dwError);

    pMdbIterator->pTxn = pTxn;

    dwError = mdb_cursor_open(pTxn, mdbDBi, &pCursor);
    BAIL_ON_VMDIR_ERROR(dwError);

    pMdbIterator->pCursor = pCursor;

    if (IsNullOrEmptyString(pszInitVal))
    {
        dwError = mdb_cursor_get(pCursor, &key, &value, MDB_FIRST);
    }
    else
    {
        dwError = VmDirAllocateStringPrintf(
                &pszInitKey, "%c%s", BE_INDEX_KEY_TYPE_FWD, pszInitVal);
        BAIL_ON_VMDIR_ERROR(dwError);

        key.mv_data = pszInitKey;
        key.mv_size = VmDirStringLenA(pszInitKey);

        dwError = mdb_cursor_get(pCursor, &key, &value, MDB_SET_RANGE);
    }

    if (dwError == 0 && *(char *)(key.mv_data) == BE_INDEX_KEY_TYPE_FWD)
    {
        dwError = VmDirAllocateMemory(key.mv_size, (PVOID*)&pszVal);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirCopyMemory(
                pszVal, key.mv_size, (char*)key.mv_data + 1, key.mv_size - 1);
        BAIL_ON_VMDIR_ERROR(dwError);

        MDBDBTToEntryId(&value, &eId);

        pIterator->bHasNext = TRUE;
    }
    else
    {
        pIterator->bHasNext = FALSE;
        dwError = dwError == MDB_NOTFOUND ? 0 : dwError;
        pMdbIterator->bAbort = dwError ? TRUE : FALSE;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    pMdbIterator->pszVal = pszVal;
    pMdbIterator->eId = eId;

    *ppIterator = pIterator;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszInitKey);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirMDBIndexIteratorFree(pIterator);
    VMDIR_SAFE_FREE_MEMORY(pszVal);
    goto cleanup;
}

DWORD
VmDirMDBIndexIterate(
    PVDIR_BACKEND_INDEX_ITERATOR    pIterator,
    PSTR*                           ppszVal,
    ENTRYID*                        pEId
    )
{
    DWORD   dwError = 0;
    PVDIR_MDB_INDEX_ITERATOR    pMdbIterator = NULL;
    PVDIR_DB_DBC    pCursor = NULL;
    VDIR_DB_DBT     key = {0};
    VDIR_DB_DBT     value = {0};
    PSTR            pszVal = NULL;
    ENTRYID         eId = 0;

    if (!pIterator || !ppszVal || !pEId)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pMdbIterator = (PVDIR_MDB_INDEX_ITERATOR)pIterator->pIterator;

    pCursor = pMdbIterator->pCursor;
    if (pIterator->bHasNext)
    {
        *ppszVal = pMdbIterator->pszVal;
        *pEId = pMdbIterator->eId;

        dwError = mdb_cursor_get(pCursor, &key, &value, MDB_NEXT);
        if (dwError == 0 && *(char *)(key.mv_data) == BE_INDEX_KEY_TYPE_FWD)
        {
            dwError = VmDirAllocateMemory(key.mv_size, (PVOID*)&pszVal);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirCopyMemory(
                    pszVal, key.mv_size, (char*)key.mv_data + 1, key.mv_size - 1);
            BAIL_ON_VMDIR_ERROR(dwError);

            MDBDBTToEntryId(&value, &eId);

            pIterator->bHasNext = TRUE;
        }
        else
        {
            pIterator->bHasNext = FALSE;
            dwError = dwError == MDB_NOTFOUND ? 0 : dwError;
            pMdbIterator->bAbort = dwError ? TRUE : FALSE;
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        pMdbIterator->pszVal = pszVal;
        pMdbIterator->eId = eId;
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VMDIR_SAFE_FREE_MEMORY(pszVal);
    goto cleanup;
}

VOID
VmDirMDBIndexIteratorFree(
    PVDIR_BACKEND_INDEX_ITERATOR    pIterator
    )
{
    PVDIR_MDB_INDEX_ITERATOR    pMdbIterator = NULL;

    if (pIterator)
    {
        pMdbIterator = (PVDIR_MDB_INDEX_ITERATOR)pIterator->pIterator;
        if (pMdbIterator)
        {
            if (pMdbIterator->pCursor)
            {
                mdb_cursor_close(pMdbIterator->pCursor);
            }
            if (pMdbIterator->pTxn)
            {
                if (pMdbIterator->bAbort)
                {
                    mdb_txn_abort(pMdbIterator->pTxn);
                }
                else
                {
                    mdb_txn_commit(pMdbIterator->pTxn);
                }
            }
            VMDIR_SAFE_FREE_MEMORY(pMdbIterator->pszVal);
            VMDIR_SAFE_FREE_MEMORY(pMdbIterator);
        }
        VMDIR_SAFE_FREE_MEMORY(pIterator);
    }
}

DWORD
VmDirMDBParentIdIndexIteratorInit(
    ENTRYID                                 parentId,
    PVDIR_BACKEND_PARENT_ID_INDEX_ITERATOR* ppIterator
    )
{
    DWORD   dwError = 0;
    VDIR_DB mdbDBi = 0;
    VDIR_DB_DBT     key = {0};
    VDIR_DB_DBT     value = {0};
    PVDIR_DB_TXN    pTxn = NULL;
    PVDIR_DB_DBC    pCursor = NULL;
    PVDIR_INDEX_CFG pIndexCfg = NULL;
    PVDIR_BACKEND_PARENT_ID_INDEX_ITERATOR  pIterator = NULL;
    PVDIR_MDB_PARENT_ID_INDEX_ITERATOR      pMdbIterator = NULL;

    unsigned char   parentEIdBytes[sizeof( ENTRYID )] = {0};

    if (!ppIterator)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_BACKEND_PARENT_ID_INDEX_ITERATOR),
            (PVOID*)&pIterator);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_MDB_PARENT_ID_INDEX_ITERATOR),
            (PVOID*)&pMdbIterator);
    BAIL_ON_VMDIR_ERROR(dwError);

    pIterator->pIterator = (PVOID)pMdbIterator;

    dwError = VmDirIndexCfgAcquire(ATTR_PARENT_ID, VDIR_INDEX_WRITE, &pIndexCfg);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirMDBIndexGetDBi(pIndexCfg, &mdbDBi);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = mdb_txn_begin(gVdirMdbGlobals.mdbEnv, NULL, MDB_RDONLY, &pTxn);
    BAIL_ON_VMDIR_ERROR(dwError);

    pMdbIterator->pTxn = pTxn;

    dwError = mdb_cursor_open(pTxn, mdbDBi, &pCursor);
    BAIL_ON_VMDIR_ERROR(dwError);

    pMdbIterator->pCursor = pCursor;

    key.mv_data = &parentEIdBytes[0];
    MDBEntryIdToDBT(parentId, &key);

    dwError = mdb_cursor_get(pCursor, &key, &value, MDB_SET_RANGE);
    MDBDBTToEntryId(&key, &pMdbIterator->parentId);
    MDBDBTToEntryId(&value, &pMdbIterator->entryId);

    if (dwError == 0 && parentId == pMdbIterator->parentId)
    {
        pIterator->bHasNext = TRUE;
    }
    else
    {
        pIterator->bHasNext = FALSE;
        dwError = dwError == MDB_NOTFOUND ? 0 : dwError;
        pMdbIterator->bAbort = dwError ? TRUE : FALSE;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppIterator = pIterator;

cleanup:
    VmDirIndexCfgRelease(pIndexCfg);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirMDBParentIdIndexIteratorFree(pIterator);
    goto cleanup;
}

DWORD
VmDirMDBParentIdIndexIterate(
    PVDIR_BACKEND_PARENT_ID_INDEX_ITERATOR  pIterator,
    ENTRYID*                                pEntryId
    )
{
    DWORD   dwError = 0;
    PVDIR_MDB_PARENT_ID_INDEX_ITERATOR  pMdbIterator = NULL;
    PVDIR_DB_DBC    pCursor = NULL;
    VDIR_DB_DBT     key = {0};
    VDIR_DB_DBT     value = {0};
    ENTRYID         parentId = 0;

    if (!pIterator || !pEntryId)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pMdbIterator = (PVDIR_MDB_PARENT_ID_INDEX_ITERATOR)pIterator->pIterator;

    pCursor = pMdbIterator->pCursor;
    if (pIterator->bHasNext)
    {
        *pEntryId = pMdbIterator->entryId;

        dwError = mdb_cursor_get(pCursor, &key, &value, MDB_NEXT);
        MDBDBTToEntryId(&key, &parentId);
        MDBDBTToEntryId(&value, &pMdbIterator->entryId);

        if (dwError == 0 && parentId == pMdbIterator->parentId)
        {
            pIterator->bHasNext = TRUE;
        }
        else
        {
            pIterator->bHasNext = FALSE;
            dwError = dwError == MDB_NOTFOUND ? 0 : dwError;
            pMdbIterator->bAbort = dwError ? TRUE : FALSE;
        }
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

VOID
VmDirMDBParentIdIndexIteratorFree(
    PVDIR_BACKEND_PARENT_ID_INDEX_ITERATOR  pIterator
    )
{
    PVDIR_MDB_PARENT_ID_INDEX_ITERATOR  pMdbIterator = NULL;

    if (pIterator)
    {
        pMdbIterator = (PVDIR_MDB_PARENT_ID_INDEX_ITERATOR)pIterator->pIterator;
        if (pMdbIterator)
        {
            if (pMdbIterator->pCursor)
            {
                mdb_cursor_close(pMdbIterator->pCursor);
            }
            if (pMdbIterator->pTxn)
            {
                if (pMdbIterator->bAbort)
                {
                    mdb_txn_abort(pMdbIterator->pTxn);
                }
                else
                {
                    mdb_txn_commit(pMdbIterator->pTxn);
                }
            }
            VMDIR_SAFE_FREE_MEMORY(pMdbIterator);
        }
        VMDIR_SAFE_FREE_MEMORY(pIterator);
    }
}

DWORD
VmDirMDBEntryBlobIteratorInit(
    ENTRYID                                 EId,
    PVDIR_BACKEND_ENTRYBLOB_ITERATOR*       ppIterator
    )
{
    DWORD   dwError = 0;
    VDIR_DB mdbDBi = 0;
    VDIR_DB_DBT     key = {0};
    VDIR_DB_DBT     value = {0};
    PVDIR_DB_TXN    pTxn = NULL;
    PVDIR_DB_DBC    pCursor = NULL;
    unsigned char   EIdBytes[sizeof( ENTRYID )] = {0};
    PVDIR_BACKEND_ENTRYBLOB_ITERATOR  pIterator = NULL;
    PVDIR_MDB_ENTRYBLOB_ITERATOR      pMdbIterator = NULL;

    if (!ppIterator)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_BACKEND_ENTRYBLOB_ITERATOR),
            (PVOID*)&pIterator);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_MDB_ENTRYBLOB_ITERATOR),
            (PVOID*)&pMdbIterator);
    BAIL_ON_VMDIR_ERROR(dwError);

    pIterator->pIterator = (PVOID)pMdbIterator;

    mdbDBi = gVdirMdbGlobals.mdbEntryDB.pMdbDataFiles[0].mdbDBi;

    dwError = mdb_txn_begin(gVdirMdbGlobals.mdbEnv, NULL, MDB_RDONLY, &pTxn);
    BAIL_ON_VMDIR_ERROR(dwError);

    pMdbIterator->pTxn = pTxn;

    dwError = mdb_cursor_open(pTxn, mdbDBi, &pCursor);
    BAIL_ON_VMDIR_ERROR(dwError);

    pMdbIterator->pCursor = pCursor;

    // get the last record - max eid
    dwError = mdb_cursor_get(pCursor, &key, &value, MDB_LAST);
    if (dwError != 0)
    {
        pIterator->bHasNext = FALSE;
        dwError = MDB_NOTFOUND;
        pMdbIterator->bAbort = TRUE;
    }
    MDBDBTToEntryId(&key, &pIterator->maxEID);

    key.mv_data = &EIdBytes[0];
    MDBEntryIdToDBT(EId, &key);

    // locate next record that is >= key
    dwError = mdb_cursor_get(pCursor, &key, &value, MDB_SET_RANGE);
    if (dwError == 0)
    {
        MDBDBTToEntryId(&key, &pMdbIterator->entryId);
        pIterator->startEID = pMdbIterator->entryId;
        pIterator->bHasNext = TRUE;
    }
    else
    {
        pIterator->bHasNext = FALSE;
        dwError = dwError == MDB_NOTFOUND ? 0 : dwError;
        pMdbIterator->bAbort = dwError ? TRUE : FALSE;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppIterator = pIterator;

cleanup:

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirMDBEntryBlobIteratorFree(pIterator);
    goto cleanup;
}

DWORD
VmDirMDBEntryBlobIterate(
    PVDIR_BACKEND_ENTRYBLOB_ITERATOR    pIterator,
    ENTRYID*                            pEntryId
    )
{
    DWORD   dwError = 0;
    PVDIR_MDB_ENTRYBLOB_ITERATOR  pMdbIterator = NULL;
    PVDIR_DB_DBC    pCursor = NULL;
    VDIR_DB_DBT     key = {0};
    VDIR_DB_DBT     value = {0};

    if (!pIterator || !pEntryId)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pMdbIterator = (PVDIR_MDB_ENTRYBLOB_ITERATOR)pIterator->pIterator;

    pCursor = pMdbIterator->pCursor;
    if (pIterator->bHasNext)
    {
        *pEntryId = pMdbIterator->entryId;

        dwError = mdb_cursor_get(pCursor, &key, &value, MDB_NEXT);
        MDBDBTToEntryId(&key, &pMdbIterator->entryId);

        if (dwError == 0)
        {
            pIterator->bHasNext = TRUE;
        }
        else
        {
            pIterator->bHasNext = FALSE;
            dwError = dwError == MDB_NOTFOUND ? 0 : dwError;
            pMdbIterator->bAbort = dwError ? TRUE : FALSE;
        }
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

VOID
VmDirMDBEntryBlobIteratorFree(
    PVDIR_BACKEND_ENTRYBLOB_ITERATOR  pIterator
    )
{
    PVDIR_MDB_ENTRYBLOB_ITERATOR  pMdbIterator = NULL;

    if (pIterator)
    {
        pMdbIterator = (PVDIR_MDB_ENTRYBLOB_ITERATOR)pIterator->pIterator;
        if (pMdbIterator)
        {
            if (pMdbIterator->pCursor)
            {
                mdb_cursor_close(pMdbIterator->pCursor);
            }
            if (pMdbIterator->pTxn)
            {
                if (pMdbIterator->bAbort)
                {
                    mdb_txn_abort(pMdbIterator->pTxn);
                }
                else
                {
                    mdb_txn_commit(pMdbIterator->pTxn);
                }
            }
            VMDIR_SAFE_FREE_MEMORY(pMdbIterator);
        }
        VMDIR_SAFE_FREE_MEMORY(pIterator);
    }
}
