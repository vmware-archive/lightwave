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
 * Module Name: common
 *
 * Filename: deque.c
 *
 * Abstract:
 *
 * deque implementation
 *
 */

#include "includes.h"

DWORD
dequeCreate(
    PDEQUE* ppDeque
    )
{
    if (!ppDeque)
        return ERROR_INVALID_PARAMETER;
    else
        return VmDirAllocateMemory(sizeof(DEQUE), (PVOID*) ppDeque);
}

VOID
dequeFree(
    PDEQUE pDeque
    )
{
    VmDirFreeMemory(pDeque);
}

PDEQUE_NODE
dequeHeadNode(
    PDEQUE pDeque
    )
{
    if (pDeque)
        return pDeque->pHead;
    else
        return NULL;
}

PVOID
dequeHead(
    PDEQUE pDeque
    )
{
    if (pDeque && pDeque->pHead)
        return pDeque->pHead->pElement;
    else
        return NULL;
}

PVOID
dequeTail(
    PDEQUE pDeque
    )
{
    if (pDeque && pDeque->pTail)
        return pDeque->pTail->pElement;
    else
        return NULL;
}

DWORD
dequePush(
    PDEQUE pDeque, PVOID pElement
    )
{
    DWORD       dwError = 0;
    PDEQUE_NODE pNode   = NULL;

    if (!pDeque || !pElement)
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    dwError = VmDirAllocateMemory(sizeof(DEQUE_NODE), (PVOID*) &pNode);
    BAIL_ON_VMDIR_ERROR(dwError);

    pNode->pElement = pElement;
    //enqueue first element
    if (!pDeque->pHead)
    {
        pDeque->pHead = pNode;
        pDeque->pTail = pNode;
    }
    else
    {
        pDeque->pTail->pNext = pNode;
        pNode->pPrev = pDeque->pTail;
        pDeque->pTail = pNode;
    }
    pDeque->iSize++;

cleanup:
    return dwError;

error:
    if (pNode)
    {
        VmDirFreeMemory((PVOID)pNode);
    }
    VmDirLog(LDAP_DEBUG_TRACE, "queue_enqueue failed. Error(%u)", dwError);
    goto cleanup;
}

DWORD
dequePop(
    PDEQUE pDeque,
    PVOID* ppElement
    )
{
    DWORD dwError = 0;

    if (!pDeque || !ppElement)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pDeque->pTail)
    {
        PDEQUE_NODE  pNode = pDeque->pTail;
        *ppElement = pDeque->pTail->pElement;
        //dequeue last element
        if(pDeque->pHead == pDeque->pTail)
        {
            pDeque->pHead = NULL;
            pDeque->pTail = NULL;
        }
        else
        {
            pDeque->pTail = pDeque->pTail->pPrev;
            pDeque->pTail->pNext = NULL;
        }

        VmDirFreeMemory(pNode);
        pDeque->iSize--;
    }
    else
    {
        dwError = ERROR_NO_MORE_ITEMS;
        goto error;
    }

cleanup:
    return dwError;

error:
    if (ppElement)
    {
        *ppElement  = NULL;
    }
    VmDirLog(LDAP_DEBUG_TRACE, "queue_dequeue failed. Error(%u)", dwError);
    goto cleanup;
}

DWORD
dequePopLeft(
    PDEQUE pDeque,
    PVOID* ppElement
    )
{
    DWORD dwError = 0;

    if (!pDeque || !ppElement)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pDeque->pHead)
    {
        PDEQUE_NODE  pNode = pDeque->pHead;
        *ppElement = pDeque->pHead->pElement;
        //dequeue last element
        if(pDeque->pHead == pDeque->pTail)
        {
            pDeque->pHead = NULL;
            pDeque->pTail = NULL;
        }
        else
        {
            pDeque->pHead = pDeque->pHead->pNext;
            pDeque->pHead->pPrev = NULL;
        }

        VmDirFreeMemory(pNode);
        pDeque->iSize--;
    }
    else
    {
        dwError = ERROR_NO_MORE_ITEMS;
        goto error;
    }

cleanup:
    return dwError;

error:
    if (ppElement)
    {
        *ppElement  = NULL;
    }
    VmDirLog(LDAP_DEBUG_TRACE, "queue_dequeue failed. Error(%u)", dwError);
    goto cleanup;
}

size_t
dequeGetSize(
    PDEQUE pDeque
    )
{
    size_t    iRtn = 0;

    if (pDeque)
    {
        iRtn = pDeque->iSize;
    }
    return iRtn;
}

BOOLEAN
dequeIsEmpty(
    PDEQUE pDeque
    )
{
    return pDeque == NULL || pDeque->pHead == NULL;
}

VOID
dequeFreeStringContents(
    PDEQUE pDeque
    )
{
    PSTR pItem = NULL;

    while(!dequeIsEmpty(pDeque))
    {
        dequePopLeft(pDeque, (PVOID*)&pItem);
        VMDIR_SAFE_FREE_MEMORY(pItem);
    }
}
