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
    PVDIR_ENTRY         pEntry,
    PVDIR_INDEX_CFG*    ppIndexCfgs
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
    PVDIR_INDEX_CFG*    ppIndexCfgs,
    ENTRYID             startEntryId,
    DWORD               dwBatchSize
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;

    PVDIR_DB_TXN        pTxn = NULL;
    VDIR_ENTRY          targetEntry = {0};
    PVDIR_ENTRY         pEntry = NULL;
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;
    VDIR_BACKEND_CTX    mdbBECtx = {0};

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt = 0; dwCnt < dwBatchSize; dwCnt ++)
    {
        DWORD   dwRetries = 0;
        BOOLEAN bIndexDone = FALSE;

        // indexing is not simple and cheap to restart, so give it more chances
        // to retry deadlock.
        // MDB has NO DEADLOCK scenario, keep this to match with MDB code
        while (dwRetries < MAX_DEADLOCK_RETRIES * 2)
        {
            PVDIR_ATTRIBUTE  pAttr = NULL;
            BOOLEAN          bRetry = FALSE;

            if (pEntry)
            {
                VmDirFreeEntryContent(pEntry);
                pEntry = NULL;

                mdbBECtx.pBEPrivate = NULL;
                VmDirBackendCtxContentFree(&mdbBECtx);
            }
            memset(&targetEntry, 0 , sizeof(targetEntry));

            assert(!pTxn);

            dwError = mdb_txn_begin( gVdirMdbGlobals.mdbEnv, BE_DB_PARENT_TXN_NULL, BE_DB_FLAGS_ZERO, &pTxn );
            BAIL_ON_VMDIR_ERROR(dwError);

            // use mdbBECtx just to make VmDirMDBEIdToEntry happy
            mdbBECtx.pBEPrivate = pTxn;
            dwError = VmDirMDBEIdToEntry(   &mdbBECtx,
                                            pSchemaCtx,
                                            startEntryId + dwCnt,
                                            &targetEntry,
                                            VDIR_BACKEND_ENTRY_LOCK_WRITE);  // acquire write lock
            if (dwError == VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND)
            {
                dwError = 0;
                mdb_txn_abort(pTxn);
                pTxn = NULL;
                break;  // exit retry while loop, move on to next entryid
            }
            BAIL_ON_VMDIR_ERROR(dwError);

            pEntry = &targetEntry;

            // normalize only newly indexed attributes
            dwError = _NewIndexedAttrValueNormalize(pEntry, ppIndexCfgs);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (pAttr = pEntry->attrs; pAttr != NULL && !bRetry; pAttr = pAttr->next)
            {
                DWORD   dwNum = 0;
                for (dwNum = 0; ppIndexCfgs[dwNum] && !bRetry; dwNum++)
                {
                    if (0 == VmDirStringCompareA(pAttr->type.lberbv.bv_val, ppIndexCfgs[dwNum]->pszAttrName, FALSE))
                    {
                        // create indices
                        dwError = MdbUpdateIndicesForAttr(  pTxn,
                                                            &pEntry->dn,
                                                            &pAttr->type,
                                                            pAttr->vals,
                                                            pAttr->numVals,
                                                            pEntry->eId,
                                                            BE_INDEX_OP_TYPE_UPDATE);
                        if (dwError != 0)
                        {
                            mdb_txn_abort(pTxn);
                            pTxn = NULL;
                            BAIL_ON_VMDIR_ERROR(dwError);
                        }
                    }
                }
            }   // for (pAttr...

            if (!bRetry)
            {
                bIndexDone = TRUE;
                break;  // retry while loop
            }

            // reset dwError in while loop
            dwError = 0;

        }   // while (retries...

        if (pTxn)
        {
            if (bIndexDone)
            {
                dwError = mdb_txn_commit(pTxn);
                pTxn = NULL;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            else
            {   // reach retry limit, bail.
                mdb_txn_abort(pTxn);
                pTxn = NULL;

                dwError = VMDIR_ERROR_BACKEND_MAX_RETRY;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
    }

cleanup:
    mdb_txn_abort(pTxn);
    VmDirFreeEntryContent(pEntry);
    VmDirSchemaCtxRelease(pSchemaCtx);
    mdbBECtx.pBEPrivate = NULL;
    VmDirBackendCtxContentFree(&mdbBECtx);
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
    PVDIR_ENTRY         pEntry,
    PVDIR_INDEX_CFG*    ppIndexCfgs
    )
{
    DWORD   dwError = 0;
    VDIR_ATTRIBUTE* pAttr = NULL;

    assert (pEntry && ppIndexCfgs);

    for (pAttr = pEntry->attrs; pAttr; pAttr = pAttr->next)
    {
        unsigned int iCnt = 0;
        for (iCnt = 0; ppIndexCfgs[iCnt]; iCnt++)
        {
            if (0 == VmDirStringCompareA(pAttr->type.lberbv.bv_val, ppIndexCfgs[iCnt]->pszAttrName, FALSE))
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
    }

error:
    return dwError;
}
