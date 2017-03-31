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
int
_MDBIntegerCompareFunction(
    const VDIR_DB_DBT * pDbt1,
    const VDIR_DB_DBT * pDbt2
    );

DWORD
MdbIndexDescInit(
    PVDIR_INDEX_CFG             pIndexCfg,
    PVDIR_MDB_INDEX_DATABASE*   ppMdbIndexDB
    )
{
    DWORD   dwError = 0;
    size_t  len = 0, i = 0;
    PVDIR_MDB_INDEX_DATABASE    pMdbIndexDB = NULL;

    if (!pIndexCfg)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    len = VmDirStringLenA(pIndexCfg->pszAttrName);

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_MDB_INDEX_DATABASE),
            (PVOID*)&pMdbIndexDB);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_CFG_MDB_DATAFILE_DESC),
            (PVOID)&pMdbIndexDB->pMdbDataFiles);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
            pIndexCfg->pszAttrName,
            &pMdbIndexDB->pszAttrName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
            VMDIR_DB_FILE_NAME,
            &pMdbIndexDB->pMdbDataFiles[0].pszDBFile);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < len; i++)
    {
        pMdbIndexDB->pszAttrName[i] = tolower(pMdbIndexDB->pszAttrName[i]);
    }

    pMdbIndexDB->usNumDataFiles = 1;
    pMdbIndexDB->pMdbDataFiles[0].pszDBName = pMdbIndexDB->pszAttrName;
    pMdbIndexDB->pMdbDataFiles[0].bIsUnique = pIndexCfg->bGlobalUniq;
    pMdbIndexDB->btKeyCmpFcn = pIndexCfg->bIsNumeric ?
            _MDBIntegerCompareFunction : NULL;

    *ppMdbIndexDB = pMdbIndexDB;

cleanup:
    return dwError;

error:
    MdbIndexDescFree(pMdbIndexDB);
    goto cleanup;
}

VOID
MdbIndexDescFree(
    PVDIR_MDB_INDEX_DATABASE    pMdbIndexDB
    )
{
    if (pMdbIndexDB)
    {
        PVDIR_CFG_MDB_DATAFILE_DESC pMdbDataFiles = NULL;
        pMdbDataFiles = pMdbIndexDB->pMdbDataFiles;
        if (pMdbDataFiles)
        {
            VMDIR_SAFE_FREE_MEMORY(pMdbDataFiles[0].pszDBFile);
            VMDIR_SAFE_FREE_MEMORY(pMdbDataFiles);
        }
        VMDIR_SAFE_FREE_MEMORY(pMdbIndexDB->pszAttrName);
        VMDIR_SAFE_FREE_MEMORY(pMdbIndexDB);
    }
}

/* The bt_compare_fcn function must return an integer value less than, equal to, or greater than zero if the first key
 * parameter is considered to be respectively less than, equal to, or greater than the second key parameter. In
 * addition, the comparison function must cause the keys in the database to be well-ordered. The comparison function
 * must correctly handle any key values used by the application (possibly including zero-length keys). In addition,
 * when Btree key prefix comparison is being performed (see DB->set_bt_prefix() for more information), the comparison
 * routine may be passed a prefix of any database key. The data and size  fields of the DBT are the only fields that
 * may be used for the purposes of this comparison, and no particular alignment of the memory to which by the data
 * field refers may be assumed.
 */
static
int
_MDBIntegerCompareFunction(
    const VDIR_DB_DBT * pDbt1,
    const VDIR_DB_DBT * pDbt2
    )
{
//TODO, do we store number data in mv_data field?  sign or unsign?
//this only work with unsigned number.
    if (pDbt1->mv_size < pDbt2->mv_size)
    {
        return -1;
    }
    if (pDbt1->mv_size > pDbt2->mv_size)
    {
        return 1;
    }
    return (memcmp( pDbt1->mv_data, pDbt2->mv_data, pDbt1->mv_size));
}
