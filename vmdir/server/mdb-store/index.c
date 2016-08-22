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
    LW_PVOID            pUnused
    );

static
DWORD
_NewIndexedAttrValueNormalize(
    PVDIR_ENTRY pEntry,
    PLW_HASHMAP pIndexCfgs
    );

DWORD
VmDirMDBInitializeIndexDB(
    VOID
    )
{
    DWORD   dwError = 0;

    dwError = LwRtlCreateHashMap(
            &gVdirMdbGlobals.mdbIndexDBs,
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
    VOID
    )
{
    if (gVdirMdbGlobals.mdbIndexDBs)
    {
        LwRtlHashMapClear(gVdirMdbGlobals.mdbIndexDBs, _FreeIdxDBMapPair, NULL);
        LwRtlFreeHashMap(&gVdirMdbGlobals.mdbIndexDBs);
        gVdirMdbGlobals.mdbIndexDBs = NULL;
    }
}

DWORD
VmDirMDBIndexOpen(
    PVDIR_INDEX_CFG     pIndexCfg
    )
{
    DWORD   dwError = 0;
    unsigned int    iDbFlags = 0;
    PVDIR_MDB_INDEX_DATABASE    pMdbIndexDB = NULL;

    if (!pIndexCfg)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = MdbIndexDescInit(pIndexCfg, &pMdbIndexDB);
    BAIL_ON_VMDIR_ERROR(dwError);

    iDbFlags |= (pMdbIndexDB->pMdbDataFiles[0].bIsUnique) ?
            BE_DB_FLAGS_ZERO : MDB_DUPSORT;

    dwError = MDBOpenDB(
            &pMdbIndexDB->pMdbDataFiles[0].mdbDBi,
            pMdbIndexDB->pMdbDataFiles[0].pszDBName,
            pMdbIndexDB->pMdbDataFiles[0].pszDBFile,
            pMdbIndexDB->btKeyCmpFcn,
            iDbFlags);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlHashMapInsert(
            gVdirMdbGlobals.mdbIndexDBs,
            pMdbIndexDB->pszAttrName,
            pMdbIndexDB,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    MDBCloseDB(pMdbIndexDB->pMdbDataFiles[0].mdbDBi);
    MdbIndexDescFree(pMdbIndexDB);
    goto cleanup;
}

BOOLEAN
VmDirMDBIndexExist(
    PVDIR_INDEX_CFG     pIndexCfg
    )
{
    int     iError = 0;
    size_t  i = 0;
    BOOLEAN bExist = FALSE;
    PSTR    pszDBName = NULL;
    MDB_txn*    pTxn = NULL;
    VDIR_DB     mdbDBi  = 0;

    if (pIndexCfg)
    {
        iError = VmDirAllocateStringA(pIndexCfg->pszAttrName, &pszDBName);
        BAIL_ON_VMDIR_ERROR(iError);

        for (i = 0; i < VmDirStringLenA(pszDBName); i++)
        {
            pszDBName[i] = tolower(pszDBName[i]);
        }

        iError = mdb_txn_begin(
                gVdirMdbGlobals.mdbEnv, NULL, MDB_RDONLY, &pTxn);
        BAIL_ON_VMDIR_ERROR(iError);

        iError = mdb_open(pTxn, pszDBName, 0, &mdbDBi);
        if (iError == 0)
        {
            bExist = TRUE;
        }
    }

error:
    if (pTxn)
    {
        mdb_txn_abort(pTxn);
    }
    VMDIR_SAFE_FREE_MEMORY(pszDBName);
    return bExist;
}

DWORD
VmDirMDBIndexDelete(
    PVDIR_INDEX_CFG     pIndexCfg
    )
{
    DWORD   dwError = 0;
    LW_HASHMAP_PAIR pair = {NULL, NULL};
    PVDIR_MDB_INDEX_DATABASE    pMdbIndexDB = NULL;

    if (!pIndexCfg)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = LwRtlHashMapRemove(
            gVdirMdbGlobals.mdbIndexDBs,
            pIndexCfg->pszAttrName,
            &pair);
    BAIL_ON_VMDIR_ERROR(dwError);

    pMdbIndexDB = (PVDIR_MDB_INDEX_DATABASE)pair.pValue;
    (VOID)MDBDropDB(pMdbIndexDB->pMdbDataFiles[0].mdbDBi);
    MdbIndexDescFree(pMdbIndexDB);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

/*
 * Create indices start from dwStartEntryId and next dwBatchSize entries.
 */
DWORD
VmDirMDBIndicesPopulate(
    PLW_HASHMAP pIndexCfgs,
    ENTRYID     startEntryId,
    DWORD       dwBatchSize
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;

    PVDIR_DB_TXN        pTxn = NULL;
    PVDIR_ENTRY         pEntry = NULL;
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;
    VDIR_BACKEND_CTX    mdbBECtx = {0};

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = mdb_txn_begin(gVdirMdbGlobals.mdbEnv, BE_DB_PARENT_TXN_NULL, BE_DB_FLAGS_ZERO, &pTxn);
    BAIL_ON_VMDIR_ERROR(dwError);

    // use mdbBECtx just to make VmDirMDBEIdToEntry happy
    mdbBECtx.pBEPrivate = pTxn;

    for (dwCnt = 0; dwCnt < dwBatchSize; dwCnt ++)
    {
        VDIR_ENTRY entry = {0};
        PVDIR_ATTRIBUTE pAttr = NULL;

        dwError = VmDirMDBEIdToEntry(   &mdbBECtx,
                                        pSchemaCtx,
                                        startEntryId + dwCnt,
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
                dwError = MdbUpdateIndicesForAttr(  pTxn,
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

    dwError = mdb_txn_commit(pTxn);
    pTxn = NULL;
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    mdbBECtx.pBEPrivate = NULL;
    VmDirBackendCtxContentFree(&mdbBECtx);
    VmDirSchemaCtxRelease(pSchemaCtx);
    return dwError;

error:
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

    mdb_txn_abort(pTxn);
    VmDirFreeEntryContent(pEntry);
    goto cleanup;
}

DWORD
VmDirMDBIndexGetDBi(
    PVDIR_INDEX_CFG pIndexCfg,
    VDIR_DB*        pDBi
    )
{
    DWORD   dwError = 0;
    PVDIR_MDB_INDEX_DATABASE    pMdbIndexDB = NULL;

    *pDBi = 0;

    if (pIndexCfg)
    {
        dwError = LwRtlHashMapFindKey(
                gVdirMdbGlobals.mdbIndexDBs,
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
    LW_PVOID            pUnused
    )
{
    PVDIR_MDB_INDEX_DATABASE pMdbIndexDB = NULL;

    pMdbIndexDB = (PVDIR_MDB_INDEX_DATABASE)pPair->pValue;
    MDBCloseDB(pMdbIndexDB->pMdbDataFiles[0].mdbDBi);
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
