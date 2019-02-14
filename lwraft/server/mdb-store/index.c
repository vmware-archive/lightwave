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
_FreeIdxDBMapPair(
    PLW_HASHMAP_PAIR    pPair,
    LW_PVOID            pUserData
    );

static
DWORD
_NewIndexedAttrValueNormalize(
    PVDIR_ENTRY pEntry,
    PLW_HASHMAP pIndexCfgs
    );

DWORD
VmDirMDBInitializeIndexDB(
    PVDIR_DB_HANDLE hDB
    )
{
    DWORD   dwError = 0;
    PVDIR_MDB_DB pDB = (PVDIR_MDB_DB)hDB;

    if (!pDB)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = LwRtlCreateHashMap(
            &pDB->mdbIndexDBs,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

VOID
VmDirMDBShutdownIndexDB(
    PVDIR_DB_HANDLE hDB
    )
{
    PVDIR_MDB_DB pDB = (PVDIR_MDB_DB)hDB;

    if (pDB && pDB->mdbIndexDBs)
    {
        LwRtlHashMapClear(pDB->mdbIndexDBs, _FreeIdxDBMapPair, pDB);
        LwRtlFreeHashMap(&pDB->mdbIndexDBs);
        pDB->mdbIndexDBs = NULL;
    }
}

DWORD
VmDirMDBIndexOpen(
    PVDIR_BACKEND_INTERFACE pBE,
    PVDIR_INDEX_CFG         pIndexCfg
    )
{
    DWORD   dwError = 0;
    unsigned int    iDbFlags = 0;
    PVDIR_MDB_INDEX_DATABASE    pMdbIndexDB = NULL;
    PVDIR_MDB_DB pDB = VmDirSafeDBFromBE(pBE);

    if (!pDB || !pIndexCfg)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = MdbIndexDescInit(pIndexCfg, &pMdbIndexDB);
    BAIL_ON_VMDIR_ERROR(dwError);

    iDbFlags |= (pMdbIndexDB->pMdbDataFiles[0].bIsUnique) ?
            BE_DB_FLAGS_ZERO : MDB_DUPSORT;

    dwError = MDBOpenDB(
            pDB,
            &pMdbIndexDB->pMdbDataFiles[0].mdbDBi,
            pMdbIndexDB->pMdbDataFiles[0].pszDBName,
            pMdbIndexDB->pMdbDataFiles[0].pszDBFile,
            pMdbIndexDB->btKeyCmpFcn,
            iDbFlags);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlHashMapInsert(
            pDB->mdbIndexDBs,
            pMdbIndexDB->pszAttrName,
            pMdbIndexDB,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    if (pDB && pMdbIndexDB)
    {
        MDBCloseDB(pDB, pMdbIndexDB->pMdbDataFiles[0].mdbDBi);
        MdbIndexDescFree(pMdbIndexDB);
    }
    goto cleanup;
}

BOOLEAN
VmDirMDBIndexExist(
    PVDIR_BACKEND_INTERFACE pBE,
    PVDIR_INDEX_CFG         pIndexCfg
    )
{
    int     iError = 0;
    size_t  i = 0;
    BOOLEAN bExist = FALSE;
    PSTR    pszDBName = NULL;
    VDIR_DB     mdbDBi  = 0;
    PVDIR_MDB_DB pDB = VmDirSafeDBFromBE(pBE);
    BOOLEAN bHasTxn = FALSE;
    VDIR_BACKEND_CTX mdbBECtx = {0};

    if (pDB && pIndexCfg)
    {
        iError = VmDirAllocateStringA(pIndexCfg->pszAttrName, &pszDBName);
        BAIL_ON_VMDIR_ERROR(iError);

        for (i = 0; i < VmDirStringLenA(pszDBName); i++)
        {
            pszDBName[i] = tolower(pszDBName[i]);
        }


        mdbBECtx.pBE = pBE;
        iError = VmDirMDBTxnBegin(&mdbBECtx, VDIR_BACKEND_TXN_READ, &bHasTxn);

        iError = mdb_open(mdbBECtx.pBEPrivate, pszDBName, 0, &mdbDBi);
        if (iError == 0)
        {
            bExist = TRUE;
        }
    }

error:
    if (bHasTxn)
    {
        VmDirMDBTxnAbort(&mdbBECtx);
    }
    VMDIR_SAFE_FREE_MEMORY(pszDBName);
    return bExist;
}

DWORD
VmDirMDBIndexDelete(
    PVDIR_BACKEND_INTERFACE pBE,
    PVDIR_INDEX_CFG         pIndexCfg
    )
{
    DWORD   dwError = 0;
    LW_HASHMAP_PAIR pair = {NULL, NULL};
    PVDIR_MDB_INDEX_DATABASE    pMdbIndexDB = NULL;
    PVDIR_MDB_DB pDB = VmDirSafeDBFromBE(pBE);

    if (!pDB || !pIndexCfg)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = LwRtlHashMapRemove(
            pDB->mdbIndexDBs,
            pIndexCfg->pszAttrName,
            &pair);
    BAIL_ON_VMDIR_ERROR(dwError);

    pMdbIndexDB = (PVDIR_MDB_INDEX_DATABASE)pair.pValue;
    (VOID)MDBDropDB(pDB, pMdbIndexDB->pMdbDataFiles[0].mdbDBi);
    MdbIndexDescFree(pMdbIndexDB);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

static
DWORD
_VmDirGetEIDBatch(
    PVDIR_BACKEND_INTERFACE pBE,
    ENTRYID     eId,
    DWORD       dwBatchSize,
    ENTRYID*    pEidArray
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    PVDIR_BACKEND_ENTRYBLOB_ITERATOR pIterator = NULL;

    dwError = pBE->pfnBEEntryBlobIteratorInit(pBE, eId, &pIterator);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (pIterator->bHasNext && dwCnt < dwBatchSize)
    {
        dwError = pBE->pfnBEEntryBlobIterate(pIterator, &(pEidArray[dwCnt]));
        BAIL_ON_VMDIR_ERROR(dwError);

        dwCnt++;
    }

cleanup:
    pBE->pfnBEEntryBlobIteratorFree(pIterator);

    return dwError;

error:
    goto cleanup;
}

/*
 * Create indices start from dwStartEntryId and next dwBatchSize entries.
 */
DWORD
VmDirMDBIndicesPopulate(
    PVDIR_BACKEND_INTERFACE pBE,
    PLW_HASHMAP             pIndexCfgs,
    PVMDIR_INDEXING_BATCH   pIndexingBatch
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;

    PVDIR_ENTRY         pEntry = NULL;
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;
    VDIR_BACKEND_CTX    mdbBECtx = {0};
    PVDIR_MDB_DB pDB = VmDirSafeDBFromBE(pBE);
    BOOLEAN bHasTxn = FALSE;
    ENTRYID*            pEidArray = NULL;

    if (!pDB || !pIndexCfgs || !pIndexingBatch)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(
        sizeof(ENTRYID)*pIndexingBatch->dwBatchSize,
        (PVOID) &pEidArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirGetEIDBatch(
        pBE,
        pIndexingBatch->startEID,
        pIndexingBatch->dwBatchSize,
        pEidArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    mdbBECtx.pBE = pBE;
    dwError = VmDirMDBTxnBegin(&mdbBECtx, VDIR_BACKEND_TXN_WRITE, &bHasTxn);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt = 0; dwCnt < pIndexingBatch->dwBatchSize && pEidArray[dwCnt] != 0; dwCnt ++)
    {
        VDIR_ENTRY entry = {0};
        PVDIR_ATTRIBUTE pAttr = NULL;

        dwError = VmDirMDBEIdToEntry(   &mdbBECtx,
                                        pSchemaCtx,
                                        pEidArray[dwCnt],
                                        &entry,
                                        VDIR_BACKEND_ENTRY_LOCK_WRITE);  // acquire write lock

        if (dwError == VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND)
        {
            dwError = 0;
            continue;  // move on to next entryid
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        pEntry = &entry;

        // normalize only newly indexed attributes
        dwError = _NewIndexedAttrValueNormalize(pEntry, pIndexCfgs);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (pAttr = pEntry->attrs; pAttr; pAttr = pAttr->next)
        {
            PSTR pszAttrName = pAttr->type.lberbv.bv_val;

            if (LwRtlHashMapFindKey(pIndexCfgs, NULL, pszAttrName) == 0)
            {
                // create indices
                dwError = MdbUpdateIndicesForAttr(  pBE,
                                                    mdbBECtx.pBEPrivate,
                                                    &pEntry->dn,
                                                    &pAttr->type,
                                                    pAttr->vals,
                                                    pAttr->numVals,
                                                    pEntry->eId,
                                                    BE_INDEX_OP_TYPE_UPDATE);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }

        VmDirFreeEntryContent(pEntry);
        pEntry = NULL;
    }

    if (dwCnt > 0)
    {
        pIndexingBatch->endEID = pEidArray[dwCnt-1];
        pIndexingBatch->dwNumEntryIndexed = dwCnt;
    }

    if (bHasTxn)
    {
        dwError = VmDirMDBTxnCommit(&mdbBECtx);
        bHasTxn = FALSE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    mdbBECtx.pBE = NULL;
    VmDirBackendCtxContentFree(&mdbBECtx);
    VmDirSchemaCtxRelease(pSchemaCtx);
    VMDIR_SAFE_FREE_MEMORY(pEidArray);

    return dwError;

error:
    if (bHasTxn)
    {
        VmDirMDBTxnAbort(&mdbBECtx);
    }

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    if (dwError == MDB_KEYEXIST)
    {
        dwError = VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION;
    }
    else
    {
        dwError = VMDIR_ERROR_BACKEND_ERROR;
    }

    VmDirFreeEntryContent(pEntry);
    goto cleanup;
}

DWORD
VmDirMDBIndexGetDBi(
    PVDIR_BACKEND_INTERFACE pBE,
    PVDIR_INDEX_CFG         pIndexCfg,
    VDIR_DB*                pDBi
    )
{
    DWORD   dwError = 0;
    PVDIR_MDB_INDEX_DATABASE    pMdbIndexDB = NULL;
    PVDIR_MDB_DB pDB = VmDirSafeDBFromBE(pBE);

    if (!pDB || !pDBi)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    *pDBi = 0;

    if (pIndexCfg)
    {
        dwError = LwRtlHashMapFindKey(
                pDB->mdbIndexDBs,
                (PVOID*)&pMdbIndexDB,
                pIndexCfg->pszAttrName);
        BAIL_ON_VMDIR_ERROR(dwError);

        *pDBi = pMdbIndexDB->pMdbDataFiles[0].mdbDBi;
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%u)", __FUNCTION__, dwError );

    VMDIR_SET_BACKEND_ERROR(dwError);
    goto cleanup;
}

static
VOID
_FreeIdxDBMapPair(
    PLW_HASHMAP_PAIR    pPair,
    LW_PVOID            pUserData
    )
{
    PVDIR_MDB_DB pDB = (PVDIR_MDB_DB)pUserData;
    PVDIR_MDB_INDEX_DATABASE pMdbIndexDB = NULL;

    pMdbIndexDB = (PVDIR_MDB_INDEX_DATABASE)pPair->pValue;
    MDBCloseDB(pDB, pMdbIndexDB->pMdbDataFiles[0].mdbDBi);
    MdbIndexDescFree(pMdbIndexDB);
}

static
DWORD
_NewIndexedAttrValueNormalize(
    PVDIR_ENTRY pEntry,
    PLW_HASHMAP pIndexCfgs
    )
{
    DWORD   dwError = 0;
    VDIR_ATTRIBUTE* pAttr = NULL;

    assert (pEntry && pIndexCfgs);

    for (pAttr = pEntry->attrs; pAttr; pAttr = pAttr->next)
    {
        PSTR pszAttrName = pAttr->type.lberbv.bv_val;

        if (LwRtlHashMapFindKey(pIndexCfgs, NULL, pszAttrName) == 0)
        {
            unsigned int iNum = 0;
            for (iNum=0; iNum < pAttr->numVals; iNum++)
            {
                dwError = VmDirSchemaBervalNormalize(   pEntry->pSchemaCtx,
                                                        pAttr->pATDesc,
                                                        &pAttr->vals[iNum]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
    }

error:
    return dwError;
}
