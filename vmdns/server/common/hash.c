/*
 * Copyright (c) VMware Inc. 2011  All rights Reserved.
 *
 * Module Name:  hash.c
 *
 * Abstract: VMware Domain Name Service.
 *
 * Hash table
 */

#include "includes.h"

#define RESIZE_THRESHOLD(ulSize) ((ULONG) (((ULONG64) ulSize * 80) / 100))

static
DWORD
HashLookup(
    PCVMDNS_HASHTABLE pTable,
    PCVOID pKey,
    ULONG ulDigest,
    PVMDNS_HASHTABLE_NODE** pppNode
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_HASHTABLE_NODE* ppNode = NULL;
    PVMDNS_HASHTABLE_NODE pNode = NULL;
    PVOID pNodeKey = NULL;

    for (ppNode = &pTable->ppBuckets[ulDigest % pTable->ulSize];
        *ppNode;
        ppNode = &pNode->pNext)
    {
        pNode = *ppNode;
        pNodeKey = pTable->pfnGetKey(pNode, pTable->pUserData);
        if (pNode->ulDigest == ulDigest &&
            pTable->pfnEqual(
                pKey,
                pNodeKey,
                pTable->pUserData
            ))
        {
            goto error;
        }
        pTable->pfnFreeKey(pNodeKey);
        pNodeKey = NULL;
    }

    dwError = ERROR_NOT_FOUND;
    BAIL_ON_VMDNS_ERROR(dwError);

error:
    if (pTable && pNodeKey)
    {
        pTable->pfnFreeKey(pNodeKey);
        pNodeKey = NULL;
    }

    *pppNode = ppNode;

    return dwError;
}

static
DWORD
HashLocate(
    PCVMDNS_HASHTABLE pTable,
    PVMDNS_HASHTABLE_NODE pNode,
    PVMDNS_HASHTABLE_NODE** pppNode
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_HASHTABLE_NODE* ppNode = NULL;

    for (ppNode = &pTable->ppBuckets[pNode->ulDigest % pTable->ulSize];
        *ppNode;
        ppNode = &(*ppNode)->pNext)
    {
        if (*ppNode == pNode)
        {
            goto error;
        }
    }

    ppNode = NULL;
    dwError = ERROR_NOT_FOUND;
    BAIL_ON_VMDNS_ERROR(dwError);

error:

    *pppNode = ppNode;

    return dwError;
}

DWORD
VmDnsCreateHashTable(
    PVMDNS_HASHTABLE* ppTable,
    VMDNS_HASH_GET_KEY_FUNCTION pfnGetKey,
    VMDNS_HASH_FREE_KEY_FUNCTION pfnFreeKey,
    VMDNS_HASH_DIGEST_FUNCTION pfnDigest,
    VMDNS_HASH_EQUAL_FUNCTION pfnEqual,
    PVOID pUserData,
    ULONG ulSize
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_HASHTABLE pTable = NULL;

    if (!ppTable || !pfnGetKey || !pfnDigest || !pfnEqual || ulSize < 1)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(sizeof((*pTable)), (PVOID*)(&pTable));
    BAIL_ON_VMDNS_ERROR(dwError);

    pTable->pfnGetKey = pfnGetKey;
    pTable->pfnFreeKey = pfnFreeKey;
    pTable->pfnDigest = pfnDigest;
    pTable->pfnEqual = pfnEqual;
    pTable->pUserData = pUserData;
    pTable->ulSize = ulSize;
    pTable->ulThreshold = RESIZE_THRESHOLD(ulSize);

    dwError = VmDnsAllocateMemory(
        sizeof((*(pTable->ppBuckets)))*ulSize, (PVOID*)(&pTable->ppBuckets)
    );
    BAIL_ON_VMDNS_ERROR(dwError);

error:

    if (dwError != ERROR_SUCCESS)
    {
        VmDnsFreeHashTable(&pTable);
    }

    if (ppTable)
    {
        *ppTable = pTable;
    }

    return dwError;
}

VOID
VmDnsHashTableInsert(
    PVMDNS_HASHTABLE pTable,
    PVMDNS_HASHTABLE_NODE pNode,
    PVMDNS_HASHTABLE_NODE* ppPrevNode
    )
{
    PVOID pKey = pTable->pfnGetKey(pNode, pTable->pUserData);
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_HASHTABLE_NODE* ppNode = NULL;

    pNode->ulDigest = pTable->pfnDigest(pKey, pTable->pUserData);

    dwError = HashLookup(pTable, pKey, pNode->ulDigest, &ppNode);
    if (dwError == ERROR_SUCCESS)
    {
        if (ppPrevNode)
        {
            *ppPrevNode = *ppNode;
        }

        pNode->pNext = (*ppNode)->pNext;
        *ppNode = pNode;
    }
    else
    {
        pNode->pNext = NULL;
        *ppNode = pNode;
        pTable->ulCount++;
    }

    VMDNS_SAFE_FREE_MEMORY(pKey);
}

VOID
VmDnsHashTableResizeAndInsert(
    PVMDNS_HASHTABLE pTable,
    PVMDNS_HASHTABLE_NODE pNode,
    PVMDNS_HASHTABLE_NODE* ppPrevNode
    )
{
    if (pTable->ulCount >= pTable->ulThreshold)
    {
        VmDnsHashTableResize(pTable, pTable->ulSize * 2);
    }

    VmDnsHashTableInsert(pTable, pNode, ppPrevNode);
}

DWORD
VmDnsHashTableRemove(
    PVMDNS_HASHTABLE pTable,
    PVMDNS_HASHTABLE_NODE pNode
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_HASHTABLE_NODE* ppNode = NULL;

    if (!pTable || !pNode)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = HashLocate(pTable, pNode, &ppNode);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppNode = pNode->pNext;
    pTable->ulCount--;

error:

    return dwError;
}

DWORD
VmDnsHashTableFindKey(
    PCVMDNS_HASHTABLE pTable,
    PVMDNS_HASHTABLE_NODE* ppNode,
    PCVOID pKey
    )
{
    DWORD dwError = ERROR_SUCCESS;
    ULONG ulDigest = 0;
    PVMDNS_HASHTABLE_NODE* ppFound = NULL;
    PVMDNS_HASHTABLE_NODE pNode = NULL;

    if (!pTable)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    ulDigest = pTable->pfnDigest(pKey, pTable->pUserData);

    dwError = HashLookup(pTable, pKey, ulDigest, &ppFound);
    BAIL_ON_VMDNS_ERROR(dwError);

    pNode = *ppFound;

error:

    if (ppNode)
    {
        *ppNode = pNode;
    }

    return dwError;
}

VOID
VmDnsHashTableResetIter(
    PVMDNS_HASHTABLE_ITER pIter
    )
{
    pIter->pNext = NULL;
    pIter->ulIndex = 0;
}

PVMDNS_HASHTABLE_NODE
VmDnsHashTableIterate(
    PCVMDNS_HASHTABLE pTable,
    PVMDNS_HASHTABLE_ITER pIter
    )
{
    PVMDNS_HASHTABLE_NODE pNode = NULL;

    if (pIter->pNext)
    {
        pNode = pIter->pNext;
        pIter->pNext = pNode->pNext;
    }
    else if (pTable->ppBuckets)
    {
        for (; pIter->ulIndex < pTable->ulSize; pIter->ulIndex++)
        {
            if (pTable->ppBuckets[pIter->ulIndex])
            {
                pNode = pTable->ppBuckets[pIter->ulIndex++];
                pIter->pNext = pNode->pNext;
                break;
            }
        }
    }

    return pNode;
}

ULONG
VmDnsHashTableGetSize(
    PCVMDNS_HASHTABLE pTable
    )
{
    return pTable->ulSize;
}

ULONG
VmDnsHashTableGetCount(
    PCVMDNS_HASHTABLE pTable
    )
{
    return pTable->ulCount;
}


DWORD
VmDnsHashTableResize(
    PVMDNS_HASHTABLE pTable,
    ULONG ulSize
    )
{
    DWORD dwError = ERROR_SUCCESS;
    ULONG ulOldSize = 0;
    ULONG ulOldCount = 0;
    ULONG ulIndex = 0;
    PVMDNS_HASHTABLE_NODE* ppOldBuckets = NULL;
    PVMDNS_HASHTABLE_NODE pNode = NULL;
    PVMDNS_HASHTABLE_NODE pNext = NULL;

    if (!pTable || ulSize < 1)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    ulOldCount = pTable->ulCount;
    ulOldSize = pTable->ulSize;
    ppOldBuckets = pTable->ppBuckets;

    pTable->ppBuckets = NULL;
    pTable->ulSize = ulSize;
    pTable->ulCount = 0;

    dwError = VmDnsAllocateMemory(
        sizeof((*(pTable->ppBuckets)))*ulSize, (PVOID*)(&pTable->ppBuckets)
    );
    if (dwError != ERROR_SUCCESS)
    {
        pTable->ppBuckets = ppOldBuckets;
        pTable->ulSize = ulOldSize;
        pTable->ulCount = ulOldCount;
    }
    BAIL_ON_VMDNS_ERROR(dwError);

    for (ulIndex = 0; ulIndex < ulOldSize; ulIndex++)
    {
        for (pNode = ppOldBuckets[ulIndex]; pNode; pNode = pNext)
        {
            pNext = pNode->pNext;
            VmDnsHashTableInsert(pTable, pNode, NULL);
        }
    }

    pTable->ulThreshold = RESIZE_THRESHOLD(ulSize);

    VMDNS_SAFE_FREE_MEMORY(ppOldBuckets);

error:

    return dwError;
}

VOID
VmDnsHashTableClear(
    PVMDNS_HASHTABLE pTable,
    VMDNS_HASHNODE_FREE_FUNCTION pFree,
    PVOID pUserData
    )
{
    VMDNS_HASHTABLE_ITER iter = VMDNS_HASHTABLE_ITER_INIT;
    PVMDNS_HASHTABLE_NODE pNode = NULL;

    if (pFree)
    {
        while ((pNode = VmDnsHashTableIterate(pTable, &iter)))
        {
            pFree(pNode, pUserData);
        }
    }

    memset(pTable->ppBuckets, 0, sizeof(*pTable->ppBuckets) * pTable->ulSize);
    pTable->ulCount = 0;
}

VOID
VmDnsFreeHashTable(
    PVMDNS_HASHTABLE* ppTable
    )
{
    if (ppTable)
    {
        if (*ppTable)
        {
            VMDNS_SAFE_FREE_MEMORY((*ppTable)->ppBuckets);
        }
        VMDNS_SAFE_FREE_MEMORY((*ppTable));
    }
}

ULONG
VmDnsHashDigestPstr(
    PCVOID pKey,
    PVOID pUnused
    )
{
    PCSTR pszStr = (PCSTR) pKey;
    ULONG ulDigest = 0;

    if (pszStr)
    {
        while (*pszStr)
        {
            ulDigest = ulDigest * 31 + *(pszStr++);
        }
    }

    return ulDigest;
}

BOOLEAN
VmDnsHashEqualPstr(
    PCVOID pKey1,
    PCVOID pKey2,
    PVOID pUnused
    )
{
    PCSTR pszStr1 = (PCSTR) pKey1;
    PCSTR pszStr2 = (PCSTR) pKey2;

    return ((pszStr1 == NULL && pszStr2 == NULL) ||
            (pszStr1 != NULL && pszStr2 != NULL &&
            (VmDnsStringCompareA(pszStr1, pszStr2, TRUE) == 0 )));
}
