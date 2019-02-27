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
VmDirMDBDupKeyGetValues(
    PVDIR_BACKEND_CTX   pBECtx,
    PCSTR               pszKey,
    PVMDIR_STRING_LIST* ppStrList
    )
{
    DWORD               dwError = 0;
    VDIR_DB_DBT         currKey = {0};
    VDIR_DB_DBT         value = {0};
    PVDIR_DB_DBC        pCursor = NULL;
    unsigned int        cursorFlags;
    PVMDIR_STRING_LIST  pStrList = NULL;
    PSTR                pValue = NULL;

    dwError = VmDirStringListInitialize(&pStrList, 128);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = mdb_cursor_open((PVDIR_DB_TXN)pBECtx->pBEPrivate,
            gVdirMdbGlobals.mdbGenericDupKeyDBi, &pCursor);
    BAIL_ON_VMDIR_ERROR(dwError);

    memset(&currKey, 0, sizeof(currKey));
    currKey.mv_size = VmDirStringLenA(pszKey);
    currKey.mv_data = (PVOID)pszKey;

    cursorFlags = MDB_SET;

    while (TRUE)
    {
        if ((dwError = mdb_cursor_get(pCursor, &currKey, &value, cursorFlags)))
        {
            dwError = 0;
            break;  // break loop if no more cursor data
        }

        dwError = VmDirAllocateAndCopyMemory(value.mv_data, value.mv_size,
                (PVOID*) &pValue);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirStringListAdd(pStrList, pValue);
        BAIL_ON_VMDIR_ERROR(dwError);
        pValue = NULL;

        cursorFlags = MDB_NEXT;
    }

    *ppStrList = pStrList;

cleanup:
    if (pCursor)
    {
        mdb_cursor_close(pCursor);
    }
    return dwError;

error:
    VMDIR_LOG_INFO( LDAP_DEBUG_BACKEND,
            "%s error (%d)", __FUNCTION__, dwError );

    VMDIR_SAFE_FREE_MEMORY(pValue);
    VmDirStringListFree(pStrList);
    goto cleanup;
}

DWORD
VmDirMDBDupKeySetValues(
    PVDIR_BACKEND_CTX   pBECtx,
    PCSTR               pszKey,
    PVMDIR_STRING_LIST  pStrList
    )
{
    DWORD       dwError = 0;
    VDIR_DB_DBT key = {0};
    VDIR_DB_DBT value = {0};
    PSTR        pszkeyData = NULL;
    DWORD       dwIdx = 0;

    dwError = VmDirAllocateStringA(pszKey, &pszkeyData);
    BAIL_ON_VMDIR_ERROR(dwError);

    key.mv_data = pszkeyData;
    key.mv_size = VmDirStringLenA(pszKey);

    dwError = VmDirMDBConfigureFsync(FALSE);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwIdx= 0; dwIdx < pStrList->dwCount; dwIdx++)
    {
        value.mv_data = (PVOID)pStrList->pStringList[dwIdx];
        value.mv_size = VmDirStringLenA(pStrList->pStringList[dwIdx]);

        dwError = mdb_put((PVDIR_DB_TXN)pBECtx->pBEPrivate,
                gVdirMdbGlobals.mdbGenericDupKeyDBi,
                &key, &value, BE_DB_FLAGS_ZERO);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszkeyData);
    dwError = VmDirMDBConfigureFsync(TRUE);
    return dwError;

error:
    VMDIR_LOG_INFO( LDAP_DEBUG_BACKEND,
            "%s error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

DWORD
VmDirMDBUniqKeyGetValue(
    PVDIR_BACKEND_CTX   pBECtx,
    PCSTR               pszKey,
    PSTR*               ppszValue
    )
{
    DWORD       dwError = 0;
    VDIR_DB_DBT key = {0};
    VDIR_DB_DBT value = {0};
    PSTR        pszValue = NULL;

    key.mv_size = VmDirStringLenA(pszKey);
    key.mv_data = (PVOID)pszKey;

    dwError =  mdb_get((PVDIR_DB_TXN)pBECtx->pBEPrivate,
            gVdirMdbGlobals.mdbGenericUniqKeyDBi,
            &key, &value);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (value.mv_size > 0)
    {
        dwError = VmDirAllocateAndCopyMemory(
                value.mv_data, value.mv_size, (PVOID*)&pszValue);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppszValue = pszValue;

cleanup:
    return dwError;

error:
    VMDIR_LOG_INFO( LDAP_DEBUG_BACKEND,
            "%s error (%d)", __FUNCTION__, dwError );

    dwError = MDBToBackendError(dwError, MDB_NOTFOUND, VMDIR_ERROR_NOT_FOUND, pBECtx, NULL);
    VMDIR_SAFE_FREE_MEMORY(pszValue);
    goto cleanup;
}

DWORD
VmDirMDBUniqKeySetValue(
    PVDIR_BACKEND_CTX   pBECtx,
    PCSTR               pszKey,
    PCSTR               pszValue
    )
{
    DWORD       dwError = 0;
    VDIR_DB_DBT key = {0};
    VDIR_DB_DBT value = {0};

    key.mv_data = (PVOID)pszKey;
    key.mv_size = VmDirStringLenA(pszKey);

    value.mv_data = (PVOID)pszValue;
    value.mv_size = VmDirStringLenA(pszValue);

    dwError = mdb_put((PVDIR_DB_TXN)pBECtx->pBEPrivate,
            gVdirMdbGlobals.mdbGenericUniqKeyDBi,
            &key, &value, BE_DB_FLAGS_ZERO);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_INFO( LDAP_DEBUG_BACKEND,
            "%s error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

DWORD
VmDirMDBGetDBi(
    PSTR        pszDBName,
    PVDIR_DB    pDBi
    )
{
    DWORD                       dwError = 0;
    PVDIR_MDB_INDEX_DATABASE    pMdbIndexDB = NULL;

    if (IsNullOrEmptyString(pszDBName) || !pDBi)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    *pDBi = 0;

    if (VmDirStringCompareA(pszDBName, BE_MDB_SEQ_DB_NAME, FALSE) == 0)
    {
        *pDBi = gVdirMdbGlobals.mdbSeqDBi;
    }
    else if (VmDirStringCompareA(pszDBName, BE_MDB_GENERIC_DUPKEY_DB_NAME, FALSE) == 0)
    {
        *pDBi = gVdirMdbGlobals.mdbGenericDupKeyDBi;
    }
    else if (VmDirStringCompareA(pszDBName, BE_MDB_GENERIC_UNIQKEY_DB_NAME, FALSE) == 0)
    {
        *pDBi = gVdirMdbGlobals.mdbGenericUniqKeyDBi;
    }
    else if (VmDirStringCompareA(pszDBName, VMDIR_ENTRY_DB, FALSE) == 0)
    {
        *pDBi = gVdirMdbGlobals.mdbEntryDB.pMdbDataFiles[0].mdbDBi;
    }
    else
    {
        dwError = LwRtlHashMapFindKey(
                gVdirMdbGlobals.mdbIndexDBs,
                (PVOID*)&pMdbIndexDB,
                pszDBName);
        BAIL_ON_VMDIR_ERROR(dwError);

        *pDBi = pMdbIndexDB->pMdbDataFiles[0].mdbDBi;
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%u)", dwError);
    VMDIR_SET_BACKEND_ERROR(dwError);
    goto cleanup;
}

DWORD
VmDirMDBGetAllDBNames(
    PVMDIR_STRING_LIST*    ppDBList
    )
{
    PSTR            pszDBName = NULL;
    DWORD           dwError = 0;
    int             retVal = 0;
    VDIR_DB         mdbDBi = 0;
    BOOLEAN         bTxn = FALSE;
    BOOLEAN         bTxnAbort = FALSE;
    VDIR_DB_DBT     key = {0};
    PVDIR_DB_TXN    pTxn = NULL;
    PVDIR_DB_DBC    pCursor = NULL;
    PVMDIR_STRING_LIST    pDBList = NULL;

    if (ppDBList == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirStringListInitialize(&pDBList, 10);
    BAIL_ON_VMDIR_ERROR(dwError);

    retVal = mdb_txn_begin(gVdirMdbGlobals.mdbEnv, NULL, MDB_RDONLY, &pTxn);
    BAIL_ON_VMDIR_ERROR(retVal);

    bTxn = TRUE;

    retVal = mdb_open(pTxn, NULL, 0, &mdbDBi);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = mdb_cursor_open(pTxn, mdbDBi, &pCursor);
    BAIL_ON_VMDIR_ERROR(retVal);

    while((retVal = mdb_cursor_get(pCursor, &key, NULL, MDB_NEXT)) == 0)
    {
        retVal = VmDirAllocateMemory(key.mv_size+1, (PVOID*) &pszDBName);
        BAIL_ON_VMDIR_ERROR(retVal);

        retVal = VmDirCopyMemory(pszDBName, key.mv_size, key.mv_data, key.mv_size);
        BAIL_ON_VMDIR_ERROR(retVal);

        retVal = VmDirStringListAdd(pDBList, pszDBName);
        BAIL_ON_VMDIR_ERROR(retVal);

        pszDBName = NULL;
    }
    retVal = retVal == MDB_NOTFOUND ? 0 : retVal;
    BAIL_ON_VMDIR_ERROR(retVal);

    *ppDBList = pDBList;
    pDBList = NULL;

cleanup:
    VmDirStringListFree(pDBList);
    if (pCursor)
    {
        mdb_cursor_close(pCursor);
    }

    if (mdbDBi != 0)
    {
        mdb_close(gVdirMdbGlobals.mdbEnv, mdbDBi);
    }

    if (bTxn)
    {
        if (bTxnAbort)
        {
            mdb_txn_abort(pTxn);
        }
        else
        {
            mdb_txn_commit(pTxn);
        }
    }

    VMDIR_SAFE_FREE_MEMORY(pszDBName);
    return dwError;

error:
    bTxnAbort = TRUE;
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", retVal);
    VMDIR_SET_BACKEND_ERROR(dwError);
    goto cleanup;
}

DWORD
VmDirMDBGetDBKeysCount(
    PSTR     pszDBName,
    PDWORD   pdwKeysCount
    )
{
    DWORD           dwError = 0;
    int             retVal = 0;
    VDIR_DB         mdbDBi = 0;
    BOOLEAN         bTxn = FALSE;
    BOOLEAN         bTxnAbort = FALSE;
    PVDIR_DB_TXN    pTxn = NULL;
    VDIR_DB_STAT    mdbStat = {0};

    if (pszDBName == NULL || pdwKeysCount == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_PARAMETER);
    }

    retVal = mdb_txn_begin(gVdirMdbGlobals.mdbEnv, NULL, MDB_RDONLY, &pTxn);
    BAIL_ON_VMDIR_ERROR(retVal);

    bTxn = TRUE;

    retVal = VmDirMDBGetDBi(pszDBName, &mdbDBi);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = mdb_stat(pTxn, mdbDBi, &mdbStat);
    BAIL_ON_VMDIR_ERROR(retVal);

    *pdwKeysCount = mdbStat.ms_entries;

cleanup:
    if (bTxn)
    {
        if (bTxnAbort)
        {
            mdb_txn_abort(pTxn);
        }
        else
        {
            mdb_txn_commit(pTxn);
        }
    }

    return dwError;

error:
    bTxnAbort = TRUE;
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", retVal);
    VMDIR_SET_BACKEND_ERROR(dwError);
    goto cleanup;
}
