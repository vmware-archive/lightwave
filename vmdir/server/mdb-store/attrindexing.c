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



/*
 * Module Name: Directory mdb-store
 *
 * Filename: attrindexing.c
 *
 * Abstract: create index for attribute
 *
 */

#include "includes.h"

static
DWORD
vmdirNewIndexedAttrValueNormalize(
    PVDIR_ENTRY                 pEntry,
    PVDIR_CFG_ATTR_INDEX_DESC*  ppIndexDesc,
    DWORD                       dwNumIndices);

/*
 * Create indices start from dwStartEntryId and next dwBatchSize entries.
 */
DWORD
VmDirMDBIndicesCreate(
    PVDIR_CFG_ATTR_INDEX_DESC*  ppIndexDesc,
    DWORD   dwNumIndices,
    DWORD   dwStartEntryId,
    DWORD   dwBatchSize)
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
                                            dwStartEntryId + dwCnt,
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
            dwError = vmdirNewIndexedAttrValueNormalize(pEntry, ppIndexDesc, dwNumIndices);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (pAttr = pEntry->attrs; pAttr != NULL && !bRetry; pAttr = pAttr->next)
            {
                DWORD   dwNum = 0;
                for (dwNum = 0; dwNum < dwNumIndices && !bRetry; dwNum++)
                {
                    if (0 == VmDirStringCompareA(pAttr->type.lberbv.bv_val, ppIndexDesc[dwNum]->pszAttrName, FALSE))
                    {
                        // create indices
                        dwError = MdbUpdateIndicesForAttr(  pTxn,
                                                            &pAttr->type,
                                                            pAttr->vals,
                                                            pAttr->numVals,
                                                            pEntry->eId,
                                                            BE_INDEX_OP_TYPE_UPDATE);
                         if (dwError != 0)
                         {
                             mdb_txn_abort(pTxn);
                             pTxn = NULL;

                             switch (dwError)
                             {
                             case MDB_KEYEXIST:
                                 dwError = VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION;
                                 BAIL_ON_VMDIR_ERROR(dwError);
                                 break;

                             default:
                                 dwError = VMDIR_ERROR_BACKEND_ERROR;
                                 BAIL_ON_VMDIR_ERROR(dwError);
                                 break;
                             }
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

    if (pEntry)
    {
        VmDirFreeEntryContent(pEntry);
    }

    if (pTxn)
    {
        mdb_txn_abort(pTxn);
    }

    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }

    mdbBECtx.pBEPrivate = NULL;
    VmDirBackendCtxContentFree(&mdbBECtx);

    return dwError;

error:

    VmDirLog( LDAP_DEBUG_ANY, "VmDirMdbIndicesCreate: error " );

    goto cleanup;
}

static
DWORD
vmdirNewIndexedAttrValueNormalize(
    PVDIR_ENTRY                 pEntry,
    PVDIR_CFG_ATTR_INDEX_DESC*  ppIndexDesc,
    DWORD                       dwNumIndices)
{
    DWORD   dwError = 0;
    VDIR_ATTRIBUTE* pAttr = NULL;

    assert (pEntry && ppIndexDesc);

    for (pAttr = pEntry->attrs; pAttr; pAttr = pAttr->next)
    {
        unsigned int iCnt = 0;
        for (iCnt = 0; iCnt < dwNumIndices; iCnt++)
        {
            if (0 == VmDirStringCompareA(pAttr->type.lberbv.bv_val, ppIndexDesc[iCnt]->pszAttrName, FALSE))
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

cleanup:

    return dwError;

error:

    goto cleanup;
}
