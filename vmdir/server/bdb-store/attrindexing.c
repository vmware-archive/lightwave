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
 * Module Name: Directory bdb-store
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
    PVDIR_ENTRY  pEntry,
    PVDIR_CFG_ATTR_INDEX_DESC*  ppIndexDesc,
    DWORD   dwNumIndices);

/*
 * Create indices start from dwStartEntryId and next dwBatchSize entries.
 */
DWORD
VmDirBdbIndicesCreate(
    PVDIR_CFG_ATTR_INDEX_DESC*  ppIndexDesc,
    DWORD   dwNumIndices,
    DWORD   dwStartEntryId,
    DWORD   dwBatchSize)
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;

    DB_TXN* ptxn = NULL;
    VDIR_ENTRY   targetEntry = {0};
    PVDIR_ENTRY  pEntry = NULL;
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;
    VDIR_BACKEND_CTX    bdbBECtx = {0};

	dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
	BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt = 0; dwCnt < dwBatchSize; dwCnt ++)
    {
        DWORD   dwRetries = 0;
        BOOLEAN bIndexDone = FALSE;

        // indexing is not simple and cheap to restart, so give it more chances
        // to retry deadlock.
        while (dwRetries < MAX_DEADLOCK_RETRIES * 2)
        {

            PVDIR_ATTRIBUTE  pAttr = NULL;
            BOOLEAN     bRetry = FALSE;

            if (pEntry)
            {
                VmDirFreeEntryContent(pEntry);
                pEntry = NULL;

                bdbBECtx.pBEPrivate = NULL;
                VmDirBackendCtxContentFree(&bdbBECtx);
            }
            memset(&targetEntry, 0 , sizeof(targetEntry));

            assert(!ptxn);

            dwError = gVdirBdbGlobals.bdbEnv->txn_begin (
                    gVdirBdbGlobals.bdbEnv,
                    BDB_PARENT_TXN_NULL,
                    &ptxn,
                    BDB_FLAGS_ZERO);
            BAIL_ON_VMDIR_ERROR(dwError);

            // use bdbBECtx just to make BdbEIdToEntry happy
            bdbBECtx.pBEPrivate = ptxn;
            dwError = BdbEIdToEntry(
                    &bdbBECtx,
                    pSchemaCtx,
                    dwStartEntryId + dwCnt,
                    &targetEntry,
                    VDIR_BACKEND_ENTRY_LOCK_WRITE);  // acquire write lock
            if (dwError == ERROR_BACKEND_ENTRY_NOTFOUND)
            {
                dwError = 0;
                ptxn->abort(ptxn);
                ptxn = NULL;
                break;  // exit retry while loop, move on to next entryid
            }
            BAIL_ON_VMDIR_ERROR(dwError);

            pEntry = &targetEntry;

            // normalize only newly indexed attributes
            dwError = vmdirNewIndexedAttrValueNormalize(
                    pEntry,
                    ppIndexDesc,
                    dwNumIndices);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (pAttr = pEntry->attrs; pAttr != NULL && !bRetry; pAttr = pAttr->next)
            {
                DWORD   dwNum = 0;
                for (dwNum = 0; dwNum < dwNumIndices && !bRetry; dwNum++)
                {
                    if (0 == VmDirStringCompareA(pAttr->type.lberbv.bv_val, ppIndexDesc[dwNum]->pszAttrName, FALSE))
                    {
                        // create indices
                        dwError = UpdateIndicesForAttribute(
                                 ptxn,
                                 &pAttr->type,
                                 pAttr->vals,
                                 pAttr->numVals,
                                 pEntry->eId,
                                 BDB_INDEX_OP_TYPE_UPDATE);

                         if (dwError != 0)
                         {
                             ptxn->abort(ptxn);
                             ptxn = NULL;

                             switch (dwError)
                             {
                             case DB_LOCK_DEADLOCK:
                                 bRetry = TRUE;
                                 dwRetries++;
                                 break;

                             case DB_KEYEXIST:
                                 dwError = ERROR_DATA_CONSTRAIN_VIOLATION;
                                 BAIL_ON_VMDIR_ERROR(dwError);
                                 break;

                             default:
                                 dwError = ERROR_BACKEND_ERROR;
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

        if (ptxn)
        {
            if (bIndexDone)
            {   // we have created indices for this entry, commit changes.
                dwError = ptxn->commit (ptxn, DB_TXN_SYNC);
                ptxn = NULL;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            else
            {   // reach retry limit, bail.
                ptxn->abort(ptxn);
                ptxn = NULL;

                dwError = ERROR_BACKEND_MAX_RETRY;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
    }

cleanup:

    if (pEntry)
    {
        VmDirFreeEntryContent(pEntry);
    }

    if (ptxn)
    {
        ptxn->abort(ptxn);
    }

    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }

    bdbBECtx.pBEPrivate = NULL;
    VmDirBackendCtxContentFree(&bdbBECtx);

    return dwError;

error:

    VmDirLog( LDAP_DEBUG_ANY, "VmDirBdbIndicesCreate: error " );

    goto cleanup;
}

static
DWORD
vmdirNewIndexedAttrValueNormalize(
    PVDIR_ENTRY  pEntry,
    PVDIR_CFG_ATTR_INDEX_DESC*  ppIndexDesc,
    DWORD   dwNumIndices)
{
    DWORD   dwError = 0;
    VDIR_ATTRIBUTE* pAttr = NULL;

    assert (pEntry && ppIndexDesc);

    for (pAttr = pEntry->attrs; pAttr; pAttr = pAttr->next)
    {
        int iCnt = 0;
        for (iCnt = 0; iCnt < dwNumIndices; iCnt++)
        {
            if (0 == VmDirStringCompareA(pAttr->type.lberbv.bv_val, ppIndexDesc[iCnt]->pszAttrName, FALSE))
            {
                int iNum = 0;
                for (iNum=0; iNum < pAttr->numVals; iNum++)
                {
                    dwError = VmDirSchemaBervalNormalize(
                            pEntry->pSchemaCtx,
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
