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
 * Module Name:  hashtable.c
 *
 * Abstract: VMware Domain Name Service.
 *
 * Hash table
 */

#include "includes.h"

#define RESIZE_THRESHOLD(ulSize) ((ULONG) (((ULONG64) ulSize * 80) / 100))

static
ULONG
VmDnsGetHash(
    PCSTR pszKey
    );

static
DWORD
VmDnsGetNode(
    PCVMDNS_HASH_TABLE pTable,
    PCSTR pszKey,
    ULONG ulDigest,
    PVMDNS_HASH_TABLE_NODE* pNode,
    PVMDNS_HASH_TABLE_NODE* pPrevNode
    );

static
BOOLEAN
VmDnsCompareKey(
    PCSTR pszKey1,
    PCSTR pszKey2
    );

static
VOID
VmDnsHashTableClearData(
    PVMDNS_HASH_TABLE pTable
    );

static
DWORD
VmDnsHashTableResize(
    PVMDNS_HASH_TABLE pTable,
    ULONG ulSize
    );

static
void
VmDnsFreeHashTableNode(
    PVMDNS_HASH_TABLE_NODE pNode
    );

VOID
VmDnsHashTableFree(
    PVMDNS_HASH_TABLE pTable
    )
{
    if (pTable)
    {
        VmDnsHashTableClearData(pTable);
        VMDNS_SAFE_FREE_MEMORY(pTable->ppData);
        VMDNS_SAFE_FREE_MEMORY(pTable);
    }
}

DWORD
VmDnsHashTableAllocate(
    PVMDNS_HASH_TABLE* ppTable,
    ULONG ulSize
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_HASH_TABLE pTable = NULL;

    if (!ppTable || ulSize < 1)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(
                    sizeof((*pTable)),
                    (PVOID*)(&pTable));
    BAIL_ON_VMDNS_ERROR(dwError);

    pTable->ulSize = ulSize;
    pTable->ulThreshold = RESIZE_THRESHOLD(ulSize);

    dwError = VmDnsAllocateMemory(
                sizeof((*(pTable->ppData)))*ulSize,
                (PVOID*)(&pTable->ppData));
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppTable = pTable;

cleanup:
    return dwError;

error:
    VmDnsHashTableFree(pTable);

    if (ppTable)
    {
        *ppTable = NULL;
    }

    goto cleanup;
}

DWORD
VmDnsHashTableGet(
    PCVMDNS_HASH_TABLE pTable,
    PCSTR pszKey,
    PVOID* ppValue
    )
{
    DWORD dwError = ERROR_SUCCESS;
    ULONG ulDigest;
    PVMDNS_HASH_TABLE_NODE pNode = NULL;
    PVMDNS_HASH_TABLE_NODE pPrevNode = NULL;

    if (!pTable || IsNullOrEmptyString(pszKey) || !ppValue)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    ulDigest = VmDnsGetHash(pszKey);

    dwError = VmDnsGetNode(
                    pTable,
                    pszKey,
                    ulDigest,
                    &pNode,
                    &pPrevNode);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppValue = pNode->pData;

cleanup:
    return dwError;

error:
    if (ppValue)
    {
        *ppValue = NULL;
    }
    goto cleanup;
}

/**
* To insert new item into hash table.
* To use chaining in the case of collision.
**/
DWORD
VmDnsHashTableInsert(
    PVMDNS_HASH_TABLE pTable,
    PCSTR pszKey,
    PVOID pValue
    )
{
    DWORD dwError = ERROR_SUCCESS;
    DWORD dwFindError = ERROR_SUCCESS;
    ULONG ulDigest;
    DWORD dwIdx;
    PVMDNS_HASH_TABLE_NODE pExistNode = NULL;
    PVMDNS_HASH_TABLE_NODE pNewNode = NULL;
    PVMDNS_HASH_TABLE_NODE pPrevNode = NULL;

    if (!pTable || IsNullOrEmptyString(pszKey) || !pValue)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    ulDigest = VmDnsGetHash(pszKey);

    dwFindError = VmDnsGetNode(
                    pTable,
                    pszKey,
                    ulDigest,
                    &pExistNode,
                    &pPrevNode);

    if (dwFindError == ERROR_SUCCESS)
    {
        dwError = ERROR_ALREADY_EXISTS;
        BAIL_ON_VMDNS_ERROR(dwFindError);
    }

    if (pTable->ulCount >= pTable->ulThreshold)
    {
        dwError = VmDnsHashTableResize(pTable, pTable->ulSize * 2);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(
                sizeof(VMDNS_HASH_TABLE_NODE),
                (PVOID*)(&pNewNode));
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringA(
                pszKey,
                &(pNewNode->pKey));
    BAIL_ON_VMDNS_ERROR(dwError);

    dwIdx = ulDigest % pTable->ulSize;
    pNewNode->ulDigest = ulDigest;
    pNewNode->pData = pValue;
    pTable->ulCount++;
    pNewNode->pNext = pTable->ppData[dwIdx];
    pTable->ppData[dwIdx] = pNewNode;

cleanup:
    return dwError;

error:
    VmDnsFreeHashTableNode(pNewNode);
    goto cleanup;
}


DWORD
VmDnsHashTableRemove(
    PVMDNS_HASH_TABLE pTable,
    PCSTR pszKey
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_HASH_TABLE_NODE pNode = NULL;
    PVMDNS_HASH_TABLE_NODE pPrevNode = NULL;
    PVMDNS_HASH_TABLE_NODE pNextNode = NULL;
    ULONG ulDigest = 0;

    if (!pTable || IsNullOrEmptyString(pszKey))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    ulDigest = VmDnsGetHash(pszKey);

    dwError = VmDnsGetNode(
                pTable,
                pszKey,
                ulDigest,
                &pNode,
                &pPrevNode);
    BAIL_ON_VMDNS_ERROR(dwError);

    pNextNode = pNode->pNext;
    VmDnsFreeHashTableNode(pNode);

    if (pPrevNode)
    {
        pPrevNode->pNext = pNextNode;
    }
    else
    {
        pTable->ppData[ulDigest%pTable->ulSize] = pNextNode;
    }

    pTable->ulCount--;

cleanup:
    return dwError;

error:
    goto cleanup;
}


ULONG
VmDnsHashTableGetSize(
    PCVMDNS_HASH_TABLE pTable
    )
{
    return pTable->ulSize;
}

ULONG
VmDnsHashTableGetCount(
    PCVMDNS_HASH_TABLE pTable
    )
{
    return pTable->ulCount;
}

VOID
VmDnsHashTableResetIter(
    PVMDNS_HASH_TABLE_ITER pIter
)
{
    pIter->pNext = NULL;
    pIter->ulIndex = 0;
}

PVMDNS_HASH_TABLE_NODE
VmDnsHashTableIterate(
    PCVMDNS_HASH_TABLE pTable,
    PVMDNS_HASH_TABLE_ITER pIter
    )
{
    PVMDNS_HASH_TABLE_NODE pNode = NULL;
    DWORD dwIdx = 0;

    if (pIter && pIter->pNext)
    {
        pNode = pIter->pNext;
        pIter->pNext = pNode->pNext;
    }
    else if (pTable->ppData)
    {
        for (dwIdx = pIter->ulIndex; dwIdx < pTable->ulSize; dwIdx++)
        {
            if (pTable->ppData[dwIdx])
            {
                pNode = pTable->ppData[dwIdx];
                pIter->pNext = pNode->pNext;
                pIter->ulIndex = dwIdx+1;
                break;
            }
        }
    }

    return pNode;
}

VOID
VmDnsHashTableClearData(
    PVMDNS_HASH_TABLE pTable
    )
{
    PVMDNS_HASH_TABLE_NODE pNode = NULL;
    PVMDNS_HASH_TABLE_NODE pNodeNext = NULL;
    DWORD dwIdx;

    if (!pTable || !pTable->ppData)
    {
        return;
    }

    for (dwIdx = 0; dwIdx < pTable->ulSize; dwIdx++)
    {
        pNode = pTable->ppData[dwIdx];
        while (pNode)
        {
            pNodeNext = pNode->pNext;
            VMDNS_SAFE_FREE_MEMORY(pNode->pKey);
            VMDNS_SAFE_FREE_MEMORY(pNode);
            pNode = pNodeNext;
        }
    }

    memset(pTable->ppData, 0, sizeof(*pTable->ppData) * pTable->ulSize);
    pTable->ulCount = 0;
}

static
ULONG
VmDnsGetHash(
    PCSTR pszKey
    )
{
    ULONG code = 0;

    if (pszKey)
    {
        while (*pszKey)
        {
            code = code * 31 + *(pszKey++);
        }
    }

    return code;
}

static
BOOLEAN
VmDnsCompareKey(
    PCSTR pszKey1,
    PCSTR pszKey2
    )
{
    return ((pszKey1 == NULL && pszKey2 == NULL) ||
        (pszKey1 != NULL && pszKey2 != NULL &&
        (VmDnsStringCompareA(pszKey1, pszKey2, TRUE) == 0)));
}

static
DWORD
VmDnsGetNode(
    PCVMDNS_HASH_TABLE pTable,
    PCSTR pszKey,
    ULONG ulDigest,
    PVMDNS_HASH_TABLE_NODE* pData,
    PVMDNS_HASH_TABLE_NODE* pPrevNode
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_HASH_TABLE_NODE pCurrent = NULL;

    *pPrevNode = NULL;
    *pData = NULL;
    dwError = ERROR_NOT_FOUND;

    for (pCurrent = pTable->ppData[ulDigest % pTable->ulSize];
        pCurrent;)
    {
        if (pCurrent->ulDigest == ulDigest
                && VmDnsCompareKey(pszKey, pCurrent->pKey))
        {
            *pData = pCurrent;
            dwError = ERROR_SUCCESS;
            break;
        }
        *pPrevNode = pCurrent;
        pCurrent = pCurrent->pNext;
    }
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    *pData = NULL;
    *pPrevNode = NULL;

    goto cleanup;
}

static
DWORD
VmDnsHashTableResize(
    PVMDNS_HASH_TABLE pTable,
    ULONG ulSize
    )
{
    DWORD dwError = ERROR_SUCCESS;
    ULONG ulOldSize = 0;
    ULONG ulOldCount = 0;
    ULONG ulIndex = 0;
    PVMDNS_HASH_TABLE_NODE* ppOldData = NULL;
    PVMDNS_HASH_TABLE_NODE* ppTempData = NULL;
    PVMDNS_HASH_TABLE_NODE pCurrent = NULL;
    PVMDNS_HASH_TABLE_NODE pNextNode = NULL;

    if (!pTable || ulSize < 1)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    ulOldCount = pTable->ulCount;
    ulOldSize = pTable->ulSize;
    ppOldData = pTable->ppData;

    dwError = VmDnsAllocateMemory(
        sizeof((*ppTempData))*ulSize, (PVOID*)(&ppTempData)
        );
    BAIL_ON_VMDNS_ERROR(dwError);

    pTable->ppData = ppTempData;
    pTable->ulSize = ulSize;

    for (ulIndex = 0; ulIndex < ulOldSize; ulIndex++)
    {
        if (ppOldData[ulIndex])
        {
            for (pCurrent = ppOldData[ulIndex]; pCurrent;)
            {
                dwError = VmDnsHashTableInsert(pTable,
                            pCurrent->pKey,
                            pCurrent->pData);
                BAIL_ON_VMDNS_ERROR(dwError);
                pCurrent = pCurrent->pNext;
            }
        }
    }

    pTable->ulThreshold = RESIZE_THRESHOLD(ulSize);

cleanup:
    VMDNS_SAFE_FREE_MEMORY(ppOldData);

    return dwError;

error:
    for (ulIndex = 0; ulIndex < ulSize; ulIndex++)
    {
        if (ppTempData[ulIndex])
        {
            for (pCurrent = ppTempData[ulIndex]; pCurrent;)
            {
                pNextNode = pCurrent->pNext;
                VmDnsFreeHashTableNode(pCurrent);
                pCurrent = pNextNode;
            }
        }
    }

    VMDNS_SAFE_FREE_MEMORY(ppTempData);

    pTable->ppData = ppOldData;
    pTable->ulSize = ulOldSize;
    pTable->ulCount = ulOldCount;

    goto cleanup;
}

static
void
VmDnsFreeHashTableNode(
    PVMDNS_HASH_TABLE_NODE pNode
    )
{
    VMDNS_SAFE_FREE_MEMORY(pNode->pKey);
    VMDNS_SAFE_FREE_MEMORY(pNode);
}
