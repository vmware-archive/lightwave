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
DWORD
_populateHashMapWithSuperlogEntries(
        PLW_HASHMAP pHashMap,
        PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY pEntries,
        PVMDIR_SUPERLOG_TABLE_COLUMN_SET pCols
        );

static
DWORD
_generateHashMapKeyString(
        PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION pEntry,
        PVMDIR_SUPERLOG_TABLE_COLUMN_SET pCols,
        PSTR *ppszKey
        );

static
DWORD
_allocateSuperlogTableRow(
        PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION pEntry,
        PVMDIR_SUPERLOG_TABLE_COLUMN_SET pCols,
        PVMDIR_SUPERLOG_TABLE_ROW *ppRow
        );

static
DWORD
_convertHashMapToSuperlogTable(
        PLW_HASHMAP pHashMap,
        PVMDIR_SUPERLOG_TABLE_COLUMN_SET pCols,
        PVMDIR_SUPERLOG_TABLE *ppTable
        );

static
DWORD
_allocateSuperlogTable(
        DWORD tableSize,
        PVMDIR_SUPERLOG_TABLE_COLUMN_SET pCols,
        PVMDIR_SUPERLOG_TABLE *ppTable
        );

static
int
_compareSuperlogTableRows(
        const void *a,
        const void *b
        );

static
VOID
_hashMapPairFree(
        PLW_HASHMAP_PAIR pPair,
        PVOID pUnused
        );


DWORD
VmDirSuperLogGetTable(
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY pEntries,
    PVMDIR_SUPERLOG_TABLE_COLUMN_SET pColumnSet,
    PVMDIR_SUPERLOG_TABLE *ppTable
    )
{
    DWORD                   dwError     = 0;
    PLW_HASHMAP             pHashMap    = NULL;
    PVMDIR_SUPERLOG_TABLE   pTable      = NULL;

    dwError = LwRtlCreateHashMap(
            &pHashMap,
            LwRtlHashDigestPstr,
            LwRtlHashEqualPstr,
            NULL
            );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _populateHashMapWithSuperlogEntries(
            pHashMap,
            pEntries,
            pColumnSet
            );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _convertHashMapToSuperlogTable(
            pHashMap,
            pColumnSet,
            &pTable
            );
    BAIL_ON_VMDIR_ERROR(dwError);

    qsort(  pTable->rows,
            pTable->numRows,
            sizeof(VMDIR_SUPERLOG_TABLE_ROW),
            _compareSuperlogTableRows);

    *ppTable = pTable;

cleanup:
    LwRtlHashMapClear(pHashMap, _hashMapPairFree, NULL);
    LwRtlFreeHashMap(&pHashMap);
    return dwError;

error:
    VmDirFreeSuperLogTable(pTable);
    goto cleanup;
}

static
VOID
_FreeSearchInformation(
        PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION pEntry)
{
    if (pEntry)
    {
        VMDIR_SAFE_FREE_MEMORY(pEntry->opInfo.searchInfo.pwszAttributes);
        VMDIR_SAFE_FREE_MEMORY(pEntry->opInfo.searchInfo.pwszBaseDN);
        VMDIR_SAFE_FREE_MEMORY(pEntry->opInfo.searchInfo.pwszScope);
        VMDIR_SAFE_FREE_MEMORY(pEntry->opInfo.searchInfo.pwszIndexResults);
    }
}

VOID
VmDirFreeSuperLogEntryLdapOperationArray(
        PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY pEntries
        )
{
    unsigned int i;
    if (pEntries)
    {
        for (i = 0; i < pEntries->dwCount; i++)
        {
            VMDIR_SAFE_FREE_MEMORY(pEntries->entries[i].pwszLoginDN);
            VMDIR_SAFE_FREE_MEMORY(pEntries->entries[i].pwszClientIP);
            VMDIR_SAFE_FREE_MEMORY(pEntries->entries[i].pwszServerIP);
            VMDIR_SAFE_FREE_MEMORY(pEntries->entries[i].pwszOperation);
            VMDIR_SAFE_FREE_MEMORY(pEntries->entries[i].pwszString);
            switch (pEntries->entries[i].opType)
            {
            case LDAP_REQ_SEARCH:
                _FreeSearchInformation(&pEntries->entries[i]);
                break;
            default:
                break;
            }
        }
        VMDIR_SAFE_FREE_MEMORY(pEntries->entries);
        VMDIR_SAFE_FREE_MEMORY(pEntries);
    }
}

VOID
VmDirFreeSuperLogTable(
    PVMDIR_SUPERLOG_TABLE pTable
    )
{
    unsigned int i, j;
    if (pTable)
    {
        for (i = 0; i < pTable->numRows; i++)
        {
            for (j = 0; j < VMDIR_SUPERLOG_TABLE_COL_NUM; j++)
            {
                VMDIR_SAFE_FREE_MEMORY(pTable->rows[i].colVals[j]);
            }
        }
        VMDIR_SAFE_FREE_MEMORY(pTable->rows);
        VMDIR_SAFE_FREE_MEMORY(pTable->cols);
        VMDIR_SAFE_FREE_MEMORY(pTable);
    }
}

static
DWORD
_populateHashMapWithSuperlogEntries(
        PLW_HASHMAP pHashMap,
        PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY pEntries,
        PVMDIR_SUPERLOG_TABLE_COLUMN_SET pCols
        )
{
    DWORD                       dwError = 0;
    PSTR                        pszKey  = NULL;
    PVMDIR_SUPERLOG_TABLE_ROW   pValue  = NULL;
    unsigned int    i;

    for (i = 0; i < pEntries->dwCount; i++)
    {
        dwError = _generateHashMapKeyString(&pEntries->entries[i], pCols, &pszKey);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapFindKey(pHashMap, (PVOID*)&pValue, pszKey);
        if (dwError == 0)
        {
            pValue->count += 1;
            pValue->totalTime += pEntries->entries[i].iEndTime - pEntries->entries[i].iStartTime;
            VMDIR_SAFE_FREE_MEMORY(pszKey);
        }
        else
        {
            dwError = _allocateSuperlogTableRow(&pEntries->entries[i], pCols, &pValue);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = LwRtlHashMapInsert(pHashMap, pszKey, pValue, NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_generateHashMapKeyString(
        PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION pEntry,
        PVMDIR_SUPERLOG_TABLE_COLUMN_SET pCols,
        PSTR *ppszKey
        )
{
    static PCSTR pcszEmptyColumn       = "--";
    static PCSTR pcszColumnDelimeter   = "||";
    DWORD   dwError         = 0;
    PSTR    pszKey          = NULL;
    PSTR    pszLoginDN      = NULL;
    PSTR    pszIP           = NULL;
    PSTR    pszPort         = NULL;
    PSTR    pszOperation    = NULL;
    PSTR    pszString       = NULL;
    PSTR    pszErrorCode    = NULL;

    if (pCols->isColumnSet[LOGIN_DN])
    {
        dwError = VmDirAllocateStringAFromW(pEntry->pwszLoginDN, &pszLoginDN);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VmDirAllocateStringA(pcszEmptyColumn, &pszLoginDN);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pCols->isColumnSet[IP])
    {
        dwError = VmDirAllocateStringAFromW(pEntry->pwszClientIP, &pszIP);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VmDirAllocateStringA(pcszEmptyColumn, &pszIP);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pCols->isColumnSet[PORT])
    {
        dwError = VmDirAllocateStringPrintf(&pszPort, "%u", pEntry->dwClientPort);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VmDirAllocateStringA(pcszEmptyColumn, &pszPort);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pCols->isColumnSet[OPERATION])
    {
        dwError = VmDirAllocateStringAFromW(pEntry->pwszOperation, &pszOperation);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VmDirAllocateStringA(pcszEmptyColumn, &pszOperation);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pCols->isColumnSet[STRING])
    {
        dwError = VmDirAllocateStringAFromW(pEntry->pwszString, &pszString);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VmDirAllocateStringA(pcszEmptyColumn, &pszString);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pCols->isColumnSet[ERROR_CODE])
    {
        dwError = VmDirAllocateStringPrintf(&pszErrorCode, "%u", pEntry->dwErrorCode);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VmDirAllocateStringA(pcszEmptyColumn, &pszErrorCode);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringPrintf(
            &pszKey,
            "%s%s%s%s%s%s%s%s%s%s%s",
            pszLoginDN, pcszColumnDelimeter,
            pszIP, pcszColumnDelimeter,
            pszPort, pcszColumnDelimeter,
            pszOperation, pcszColumnDelimeter,
            pszString, pcszColumnDelimeter,
            pszErrorCode
            );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszKey = pszKey;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLoginDN);
    VMDIR_SAFE_FREE_MEMORY(pszIP);
    VMDIR_SAFE_FREE_MEMORY(pszPort);
    VMDIR_SAFE_FREE_MEMORY(pszOperation);
    VMDIR_SAFE_FREE_MEMORY(pszString);
    VMDIR_SAFE_FREE_MEMORY(pszErrorCode);
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszKey);
    goto cleanup;
}

static
DWORD
_allocateSuperlogTableRow(
        PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION pEntry,
        PVMDIR_SUPERLOG_TABLE_COLUMN_SET pCols,
        PVMDIR_SUPERLOG_TABLE_ROW *ppRow
        )
{
    DWORD dwError = 0;
    PVMDIR_SUPERLOG_TABLE_ROW pRow = NULL;

    dwError = VmDirAllocateMemory(
            sizeof(VMDIR_SUPERLOG_TABLE_ROW),
            (PVOID*)&pRow);
    BAIL_ON_VMDIR_ERROR(dwError);

    memset(pRow->colVals, 0, sizeof(pRow->colVals));

    if (pCols->isColumnSet[LOGIN_DN])
    {
        dwError = VmDirAllocateStringAFromW(
                pEntry->pwszLoginDN,
                &pRow->colVals[LOGIN_DN]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pCols->isColumnSet[IP])
    {
        dwError = VmDirAllocateStringAFromW(
                pEntry->pwszClientIP,
                &pRow->colVals[IP]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pCols->isColumnSet[PORT])
    {
        dwError = VmDirAllocateStringPrintf(
                &pRow->colVals[PORT],
                "%u",
                pEntry->dwClientPort
        );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pCols->isColumnSet[OPERATION])
    {
        dwError = VmDirAllocateStringAFromW(
                pEntry->pwszOperation,
                &pRow->colVals[OPERATION]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pCols->isColumnSet[STRING])
    {
        dwError = VmDirAllocateStringAFromW(
                pEntry->pwszString,
                &pRow->colVals[STRING]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pCols->isColumnSet[ERROR_CODE])
    {
        dwError = VmDirAllocateStringPrintf(
                &pRow->colVals[ERROR_CODE],
                "%u",
                pEntry->dwErrorCode
        );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pRow->count = 1;
    pRow->totalTime = pEntry->iEndTime - pEntry->iStartTime;

    *ppRow = pRow;

cleanup:
    return dwError;

error:
    if (pRow)
    {
        VMDIR_SAFE_FREE_MEMORY(pRow->colVals[LOGIN_DN]);
        VMDIR_SAFE_FREE_MEMORY(pRow->colVals[IP]);
        VMDIR_SAFE_FREE_MEMORY(pRow->colVals[PORT]);
        VMDIR_SAFE_FREE_MEMORY(pRow->colVals[OPERATION]);
        VMDIR_SAFE_FREE_MEMORY(pRow->colVals[STRING]);
        VMDIR_SAFE_FREE_MEMORY(pRow->colVals[ERROR_CODE]);
        VMDIR_SAFE_FREE_MEMORY(pRow);
    }
    goto cleanup;
}

static
DWORD
_convertHashMapToSuperlogTable(
        PLW_HASHMAP pHashMap,
        PVMDIR_SUPERLOG_TABLE_COLUMN_SET pCols,
        PVMDIR_SUPERLOG_TABLE *ppTable
        )
{
    DWORD                   dwError = 0;
    DWORD                   dwCount = 0;
    PVMDIR_SUPERLOG_TABLE   pTable  = NULL;

    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};
    unsigned int i = 0;
    unsigned int j;

    PVMDIR_SUPERLOG_TABLE_ROW pSrcRow = NULL;
    PVMDIR_SUPERLOG_TABLE_ROW pDstRow = NULL;

    dwCount = LwRtlHashMapGetCount(pHashMap);

    dwError = _allocateSuperlogTable(dwCount, pCols, &pTable);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (LwRtlHashMapIterate(pHashMap, &iter, &pair))
    {
        pSrcRow = pair.pValue;
        pDstRow = (PVMDIR_SUPERLOG_TABLE_ROW)&pTable->rows[i++];
        pDstRow->count = pSrcRow->count;
        pDstRow->totalTime = pSrcRow->totalTime;
        for (j = 0; j < VMDIR_SUPERLOG_TABLE_COL_NUM; j++)
        {
            if (pSrcRow->colVals[j])
            {
                dwError = VmDirAllocateStringA(pSrcRow->colVals[j], &pDstRow->colVals[j]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
        dwError = VmDirAllocateStringPrintf(
                &pDstRow->colVals[AVG_TIME],
                "%lu",
                pDstRow->totalTime / pDstRow->count
                );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppTable = pTable;

cleanup:
    return dwError;

error:
    VmDirFreeSuperLogTable(pTable);
    goto cleanup;
}

static
DWORD
_allocateSuperlogTable(
        DWORD tableSize,
        PVMDIR_SUPERLOG_TABLE_COLUMN_SET pCols,
        PVMDIR_SUPERLOG_TABLE *ppTable
        )
{
    DWORD dwError = 0;
    PVMDIR_SUPERLOG_TABLE pTable = NULL;

    dwError = VmDirAllocateMemory(
            sizeof(VMDIR_SUPERLOG_TABLE),
            (PVOID*)&pTable
            );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateAndCopyMemory(
            pCols,
            sizeof(VMDIR_SUPERLOG_TABLE_COLUMN_SET),
            (PVOID*)&pTable->cols
            );
    BAIL_ON_VMDIR_ERROR(dwError);

    pTable->numRows = tableSize;

    if (tableSize > 0)
    {
        dwError = VmDirAllocateMemory(
                sizeof(VMDIR_SUPERLOG_TABLE_ROW)*tableSize,
                (PVOID*)&pTable->rows
                );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppTable = pTable;

cleanup:
    return dwError;

error:
    if (pTable)
    {
        VMDIR_SAFE_FREE_MEMORY(pTable->rows);
        VMDIR_SAFE_FREE_MEMORY(pTable->cols);
        VMDIR_SAFE_FREE_MEMORY(pTable);
    }
    goto cleanup;
}

static
int
_compareSuperlogTableRows(
        const void *a,
        const void *b
        )
{
    PVMDIR_SUPERLOG_TABLE_ROW pA = (PVMDIR_SUPERLOG_TABLE_ROW)a;
    PVMDIR_SUPERLOG_TABLE_ROW pB = (PVMDIR_SUPERLOG_TABLE_ROW)b;
    if (pB->count > pA->count)
    {
        return 1;
    }
    else if (pB->count < pA->count)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

static
VOID
_hashMapPairFree(
        PLW_HASHMAP_PAIR pPair,
        PVOID pUnused
)
{
    unsigned int i;

    VMDIR_SAFE_FREE_MEMORY(pPair->pKey);
    for (i = 0; i < VMDIR_SUPERLOG_TABLE_COL_NUM; i++)
    {
        VMDIR_SAFE_FREE_MEMORY(((PVMDIR_SUPERLOG_TABLE_ROW)pPair->pValue)->colVals[i]);
        ((PVMDIR_SUPERLOG_TABLE_ROW)pPair->pValue)->colVals[i] = NULL;
    }
    VMDIR_SAFE_FREE_MEMORY(pPair->pValue);
    pPair->pValue = NULL;
}

