/*
 * Copyright ©2019 VMware, Inc.  All Rights Reserved.
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
char
_VmDirGetKeyPrefix(
    int     searchType,
    BOOLEAN bReverseSearch
    );

static
BOOLEAN
_VmDirMDBIndexIterKeyNotMatch(
    PVDIR_ITERATOR_CONTEXT  pIterContext,
    PVDIR_DB_DBT            pKey
    );

static
DWORD
_VmDirMDBIndexIteratorInitContent(
    PVDIR_INDEX_CFG             pIndexCfg,
    PVDIR_ITERATOR_CONTEXT      pIterContext,
    PVDIR_MDB_INDEX_ITERATOR    pMdbIterator
    );

static
DWORD
_VmDirCloneDbt2BV(
    PVDIR_DB_DBT    pDBT,
    PVDIR_BERVALUE  pBerV
    );

static
DWORD
_VmDirMDBIndexIterNewTxn(
    PVDIR_BACKEND_INDEX_ITERATOR pIterator,
    PVDIR_ITERATOR_CONTEXT       pIterContext
    );

/*
 * transform filter value into MDB key format:
 * 1. value should be in normalized form
 * 2. handle key prefix (FWD/REV)
 * 3. handle reverse key (DN only for now)
 *
 * TODO, should expose as a BE interface?
 */
DWORD
VmDirMDBIndexIteratorInitKey(
    PVDIR_ITERATOR_CONTEXT  pIteratorContext,
    PCSTR                   pszNormValue
    )
{
    DWORD   dwError = 0;
    CHAR    keyPrefix = {0};
    PSTR    pszKey = NULL;
    DWORD   dwInitLen = 0;

    if (!pIteratorContext || !pszNormValue)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwInitLen = VmDirStringLenA(pszNormValue);
    keyPrefix = _VmDirGetKeyPrefix(pIteratorContext->iSearchType, pIteratorContext->bReverseSearch);

    // length = keyPrefix = pszInit + NULL
    dwError = VmDirAllocateMemory(dwInitLen + 1 + 1, (PVOID*)&pszKey);
    BAIL_ON_VMDIR_ERROR(dwError);

    pszKey[0] = keyPrefix;
    dwError = VmDirCopyMemory(pszKey+1, dwInitLen, pszNormValue, dwInitLen);

    if (pIteratorContext->bReverseSearch)
    {
        VmDirStringReverseA(pszKey+1);
    }

    pIteratorContext->bvFilterValue.lberbv_val = pszKey;
    pIteratorContext->bvFilterValue.lberbv_len = VmDirStringLenA(pszKey);
    pIteratorContext->bvFilterValue.bOwnBvVal = TRUE;
    pszKey = NULL;

    dwError = VmDirBervalContentDup(
            &pIteratorContext->bvFilterValue,
            &pIteratorContext->bvCurrentKey);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszKey);
    return dwError;

error:
    goto cleanup;
}

/*
 * convert pszNormValue, a parent id string, to parient id table key format
 */
DWORD
VmDirMDBIndexIteratorInitParentIdKey(
    PVDIR_ITERATOR_CONTEXT  pIteratorContext,
    PCSTR                   pszNormValue
    )
{
    DWORD   dwError = 0;
    ENTRYID parentEId = 0;
    VDIR_BERVALUE   bvValue = {0};
    unsigned char   eIdBytes[sizeof(ENTRYID) + 1] = {0};

    if (!pIteratorContext || !pszNormValue)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirStringToINT64(pszNormValue, NULL, &parentEId);
    BAIL_ON_VMDIR_ERROR(dwError);

    bvValue.lberbv_val = (PSTR) &eIdBytes[0];
    bvValue.lberbv_len = sizeof(ENTRYID);
    dwError = VmDirEntryIdToBV(parentEId, &bvValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirBervalContentDup(
            &bvValue,
            &pIteratorContext->bvFilterValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirBervalContentDup(
            &pIteratorContext->bvFilterValue,
            &pIteratorContext->bvCurrentKey);
    BAIL_ON_VMDIR_ERROR(dwError);

#if 0
    {
        unsigned char* p = (unsigned char*)pIteratorContext->bvFilterValue.lberbv.bv_val;
        VMDIR_LOG_DEBUG(VMDIR_LOG_MASK_ALL,
            "%s key size %d key %02X %02X %02X %02X %02X %02X %02X %02X",
            __FUNCTION__,
            pIteratorContext->bvFilterValue.lberbv.bv_len, p[0],p[1],p[2],p[3],p[4],p[5],p[6], p[7]);
    }
#endif

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirMDBIndexIteratorInit(
    PVDIR_INDEX_CFG                  pIndexCfg,
    PVDIR_ITERATOR_CONTEXT           pIterContext,
    PVDIR_BACKEND_INDEX_ITERATOR*    ppIterator
    )
{
    DWORD           dwError = 0;
    VDIR_DB_DBT     key = {0};
    VDIR_DB_DBT     value = {0};
    unsigned char   EIDBytes[sizeof( ENTRYID )] = {0};

    PVDIR_MDB_INDEX_ITERATOR          pMdbIterator = NULL;
    PVDIR_BACKEND_INDEX_ITERATOR      pIterator = NULL;

    if (!pIndexCfg || !ppIterator || !pIterContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    assert(pIterContext->bInit);

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_BACKEND_INDEX_ITERATOR),
            (PVOID*)&pIterator);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_MDB_INDEX_ITERATOR),
            (PVOID*)&pMdbIterator);
    BAIL_ON_VMDIR_ERROR(dwError);

    pIterator->pIterator = (PVOID)pMdbIterator;

    dwError = _VmDirMDBIndexIteratorInitContent(
            pIndexCfg,
            pIterContext,
            pMdbIterator);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pMdbIterator->dwInitCursorFlag == MDB_FIRST)
    {
        dwError = mdb_cursor_get(pMdbIterator->pCursor, &key, &value, MDB_FIRST);

        // only the very first call use MDB_FIRST, all subsequent calls or _VmDirMDBIndexIterNewTxn
        // should use the right cursor flag
        pMdbIterator->dwInitCursorFlag = pMdbIterator->dwOverrideInitCursorFlag;
    }
    else
    {
        key.mv_data = pIterContext->bvCurrentKey.lberbv_val;
        key.mv_size = pIterContext->bvCurrentKey.lberbv_len;

        if (pIterContext->eId != 0)
        {
            value.mv_data = &EIDBytes[0];
            MDBEntryIdToDBT(pIterContext->eId, &value);
        }

        dwError = mdb_cursor_get(
                pMdbIterator->pCursor,
                &key,
                &value,
                // use MDB_SET_RANGE if there is NO value. i.e. first call
                // use dwInitCursorFlag for subsequent call where we have value/eid
                (pIterContext->eId == 0) ? MDB_SET_RANGE : pMdbIterator->dwInitCursorFlag);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    if (_VmDirMDBIndexIterKeyNotMatch(pIterContext, &key))
    {
        pIterator->bHasNext = FALSE;
        goto done;
    }

    // if cursor positioned to a different place, mdb changes key content;
    // otherwise, no need to clone key into bvCurrentKey. (they point to same address actually)
    if (key.mv_size != pIterContext->bvCurrentKey.lberbv_len ||
        key.mv_data != pIterContext->bvCurrentKey.lberbv_val)
    {
        dwError = _VmDirCloneDbt2BV(&key, &pIterContext->bvCurrentKey);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    MDBDBTToEntryId(&value, &pIterContext->eId);
    pIterator->bHasNext = TRUE;
    pIterContext->iIterCount++;

done:
    *ppIterator = pIterator;

cleanup:
    return dwError;

error:
    if (dwError == MDB_NOTFOUND)
    {
        pIterator->bHasNext = FALSE;
        dwError = 0;
        goto done;
    }
    else
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                "%s failed, error (%d)", __FUNCTION__, dwError );
        VmDirMDBIndexIteratorFree(pIterator);
    }
    goto cleanup;
}

DWORD
VmDirMDBIndexIterate(
    PVDIR_BACKEND_INDEX_ITERATOR    pIterator,
    PVDIR_ITERATOR_CONTEXT          pIterContext
    )
{
    DWORD   dwError = 0;
    PVDIR_MDB_INDEX_ITERATOR    pMdbIterator = NULL;
    PVDIR_DB_DBC    pCursor = NULL;
    VDIR_DB_DBT     key = {0};
    VDIR_DB_DBT     value = {0};

    if (!pIterator || !pIterator->pIterator || !pIterContext)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pMdbIterator = (PVDIR_MDB_INDEX_ITERATOR)pIterator->pIterator;

    pCursor = pMdbIterator->pCursor;
    if (pIterator->bHasNext)
    {
        dwError = mdb_cursor_get(
                pCursor,
                &key,
                &value,
                pMdbIterator->dwCursorFlag);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (_VmDirMDBIndexIterKeyNotMatch(pIterContext, &key))
        {
            pIterator->bHasNext = FALSE;
            goto cleanup;
        }

        dwError = _VmDirCloneDbt2BV(&key, &pIterContext->bvCurrentKey);
        BAIL_ON_VMDIR_ERROR(dwError);

        MDBDBTToEntryId(&value, &pIterContext->eId);
        pIterator->bHasNext = TRUE;
        pIterContext->iIterCount++;

        if (gVmdirGlobals.dwMaxSearchIterationTxn &&
            ++(pMdbIterator->iIterCount) > gVmdirGlobals.dwMaxSearchIterationTxn)
        {
           dwError = _VmDirMDBIndexIterNewTxn(pIterator, pIterContext);
           BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    return dwError;

error:
    if (dwError == MDB_NOTFOUND)
    {
        pIterator->bHasNext = FALSE;
        dwError = 0;
    }
    else
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                "%s failed, error (%d)", __FUNCTION__, dwError );
    }
    goto cleanup;
}

static
DWORD
_VmDirMDBIndexIterNewTxn(
    PVDIR_BACKEND_INDEX_ITERATOR pIterator,
    PVDIR_ITERATOR_CONTEXT       pIterContext
    )
{
    DWORD           dwError = 0;
    VDIR_DB_DBT     key = {0};
    VDIR_DB_DBT     value = {0};
    PVDIR_DB_TXN    pTxn = NULL;
    PVDIR_DB_DBC    pCursor = NULL;
    PVDIR_MDB_INDEX_ITERATOR pMdbIterator = NULL;
    unsigned char   EIDBytes[sizeof( ENTRYID )] = {0};

    pMdbIterator = pIterator->pIterator;

    mdb_cursor_close(pMdbIterator->pCursor);
    mdb_txn_abort(pMdbIterator->pTxn);
    pMdbIterator->pCursor = NULL;
    pMdbIterator->pTxn = NULL;

    dwError = mdb_txn_begin(gVdirMdbGlobals.mdbEnv, NULL, MDB_RDONLY, &pTxn);
    BAIL_ON_VMDIR_ERROR(dwError);

    pMdbIterator->pTxn = pTxn;
    pMdbIterator->iIterCount = 0;

    dwError = mdb_cursor_open(pTxn, pMdbIterator->mdbDBi, &pCursor);
    BAIL_ON_VMDIR_ERROR(dwError);

    pMdbIterator->pCursor = pCursor;

    key.mv_data = pIterContext->bvCurrentKey.lberbv_val;
    key.mv_size = pIterContext->bvCurrentKey.lberbv_len;

    value.mv_data = &EIDBytes[0];
    MDBEntryIdToDBT(pIterContext->eId, &value);

    dwError = mdb_cursor_get(
            pMdbIterator->pCursor,
            &key,
            &value,
            pMdbIterator->dwInitCursorFlag);
    BAIL_ON_VMDIR_ERROR(dwError);

    // potentially, we may not reposition to the exact cursor position before this call
    //   if there were writes happened in between.
    // this, however, is benign issue that could happen, say between two page search call.
    if (_VmDirMDBIndexIterKeyNotMatch(pIterContext, &key))
    {
        pIterator->bHasNext = FALSE;
        goto cleanup;
    }

    // if cursor positioned to a different place, mdb changes key content;
    // otherwise, no need to clone key into bvCurrentKey. (they point to same address actually)
    if (key.mv_size != pIterContext->bvCurrentKey.lberbv_len ||
        key.mv_data != pIterContext->bvCurrentKey.lberbv_val)
    {
        dwError = _VmDirCloneDbt2BV(&key, &pIterContext->bvCurrentKey);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    MDBDBTToEntryId(&value, &pIterContext->eId);
    pIterator->bHasNext = TRUE;

cleanup:
    return dwError;

error:
    if (dwError == MDB_NOTFOUND)
    {
        pIterator->bHasNext = FALSE;
        dwError = 0;
    }
    else
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                "%s failed, error (%d)", __FUNCTION__, dwError );
    }
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
            VMDIR_SAFE_FREE_MEMORY(pMdbIterator);
        }
        VMDIR_SAFE_FREE_MEMORY(pIterator);
    }
}

static
char
_VmDirGetKeyPrefix(
    int     searchType,
    BOOLEAN bReverseSearch
    )
{
    char keyType = BE_INDEX_KEY_TYPE_FWD;

    if (searchType == LDAP_FILTER_SUBSTRINGS && bReverseSearch)
    {
        keyType = BE_INDEX_KEY_TYPE_REV;
    }
    // else if TODO, add GE/LE support

    return keyType;
}

/*
 * Verify if we can terminate iterator or should continue
 */
static
BOOLEAN
_VmDirMDBIndexIterKeyNotMatch(
    PVDIR_ITERATOR_CONTEXT  pIterContext,
    PVDIR_DB_DBT            pKey
    )
{
    BOOLEAN bNotMatch = FALSE;

    if (pIterContext->iSearchType == LDAP_FILTER_PRESENT)
    {
        goto cleanup;
    }
    else if (pIterContext->iSearchType == LDAP_FILTER_SUBSTRINGS)
    {
        if (pIterContext->bvFilterValue.lberbv_len > pKey->mv_size      ||
            memcmp(pKey->mv_data, pIterContext->bvFilterValue.lberbv_val,
                   pIterContext->bvFilterValue.lberbv_len) != 0)
        {
            bNotMatch = TRUE;
        }
    }
    else if (pIterContext->iSearchType == LDAP_FILTER_EQUALITY      ||
             pIterContext->iSearchType == FILTER_ONE_LEVEL_SEARCH)
    {
        if (pIterContext->bvFilterValue.lberbv_len != pKey->mv_size     ||
            memcmp(pKey->mv_data, pIterContext->bvFilterValue.lberbv_val,
                    pIterContext->bvFilterValue.lberbv_len) != 0 )
        {
            bNotMatch = TRUE;
        }
    }
    // else if TODO add LG/LE support

cleanup:

    if (bNotMatch)
    {
        VMDIR_LOG_VERBOSE(LDAP_DEBUG_FILTER,
            "%s table (%s) value (%.*s), actual(%.*s)",
            __FUNCTION__,
            pIterContext->pszIterTable,
            pIterContext->bvFilterValue.lberbv_len,
            pIterContext->bvFilterValue.lberbv_val,
            pKey->mv_size,
            pKey->mv_data);
    }
    return bNotMatch;
}

static
DWORD
_VmDirCloneDbt2BV(
    PVDIR_DB_DBT    pDBT,
    PVDIR_BERVALUE  pBerV
    )
{
    DWORD   dwError = 0;
    PSTR    pszKey = NULL;

    VmDirFreeBervalContent(pBerV);

    dwError = VmDirAllocateAndCopyMemory(
            pDBT->mv_data,
            pDBT->mv_size,
            (PVOID*)&pszKey);
    BAIL_ON_VMDIR_ERROR(dwError);

    pBerV->lberbv_val = pszKey;
    pszKey = NULL;
    pBerV->lberbv_len = pDBT->mv_size;
    pBerV->bOwnBvVal = TRUE;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszKey);
    return dwError;

error:
    goto cleanup;
}

/*
 * Initialized pMdbIterator for every MDB level index iterate
 *   via longer lasting upper layer pIterContext (e.g. PageSearchContext)
 *
 * TODO, add GE/LE support
 */
static
DWORD
_VmDirMDBIndexIteratorInitContent(
    PVDIR_INDEX_CFG             pIndexCfg,
    PVDIR_ITERATOR_CONTEXT      pIterContext,
    PVDIR_MDB_INDEX_ITERATOR    pMdbIterator
    )
{
    DWORD   dwError = 0;
    DWORD   dwDBFlags = 0;
    VDIR_DB mdbDBi = 0;
    PVDIR_DB_DBC    pCursor = NULL;
    PVDIR_DB_TXN    pTxn = NULL;

    pMdbIterator->iIterCount = 0;

    dwError = VmDirMDBIndexGetDBi(pIndexCfg, &mdbDBi);
    BAIL_ON_VMDIR_ERROR(dwError);
    pMdbIterator->mdbDBi = mdbDBi;

    dwError = mdb_txn_begin(gVdirMdbGlobals.mdbEnv, NULL, MDB_RDONLY, &pTxn);
    BAIL_ON_VMDIR_ERROR(dwError);
    pMdbIterator->pTxn = pTxn;

    dwError = mdb_dbi_flags(pTxn, mdbDBi, &dwDBFlags);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (dwDBFlags & MDB_DUPSORT)
    {
        pMdbIterator->dwInitCursorFlag = MDB_GET_BOTH_RANGE;
        pMdbIterator->dwOverrideInitCursorFlag = MDB_GET_BOTH_RANGE;
    }
    else
    {
        pMdbIterator->dwInitCursorFlag = MDB_SET_RANGE;
        pMdbIterator->dwOverrideInitCursorFlag = MDB_SET_RANGE;
    }

    if (pIterContext->iSearchType == LDAP_FILTER_PRESENT && pIterContext->iIterCount == 0)
    {   // for the very first LDAP_FILTER_PRESENT search to position cursor
        pMdbIterator->dwInitCursorFlag = MDB_FIRST;
    }

    pMdbIterator->dwCursorFlag = MDB_NEXT;

    dwError = mdb_cursor_open(pTxn, mdbDBi, &pCursor);
    BAIL_ON_VMDIR_ERROR(dwError);
    pMdbIterator->pCursor = pCursor;

cleanup:
    return dwError;

error:
    if (pMdbIterator->pCursor)
    {
        mdb_cursor_close(pMdbIterator->pCursor);
    }
    if (pMdbIterator->pTxn)
    {
        mdb_txn_abort(pMdbIterator->pTxn);
    }
    goto cleanup;
}
