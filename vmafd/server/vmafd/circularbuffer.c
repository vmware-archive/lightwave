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
 * Module Name:  circularbuffer.c
 *
 * Abstract: VMware Cloud Directory Platform.
 *
 * Implements a generic circular buffer data structure that uses pre-allocated memory for the buffer to ensure
 * that all future operations succeed in the face of memory pressure.
 */

#include "includes.h"

pthread_mutex_t g_mutex;


VOID VmAfdCircularBufferFree(
    PVMAFD_CIRCULAR_BUFFER pCircularBuffer
    )
{
    if (pCircularBuffer != NULL)
    {
        //pthread_mutex_destroy(&(pCircularBuffer->mutex));
        VMAFD_SAFE_FREE_MEMORY(pCircularBuffer->CircularBuffer);

        VmAfdFreeMemory(pCircularBuffer);
    }
}

DWORD
VmAfdCircularBufferCreate(
    DWORD dwCapacity,
    DWORD dwElementSize,
    PVMAFD_CIRCULAR_BUFFER *ppCircularBuffer
    )
{
    PVMAFD_CIRCULAR_BUFFER pCircularBuffer = NULL;
    DWORD dwError = 0;
    DWORD dwBufferSize = 0;

    dwError = VmAfdAllocateMemory(sizeof(*pCircularBuffer), (PVOID)&pCircularBuffer);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = pthread_mutex_init(&g_mutex, NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

    //
    // Calculate the size of the buffer. We need to prevent overflow, as well as
    // disallow a zero-sized buffer.
    //
    dwBufferSize = dwCapacity * dwElementSize;
    if (dwBufferSize != 0 && dwBufferSize > dwCapacity)
    {
        dwError = VmAfdAllocateMemory(dwCapacity * dwElementSize, (PVOID)&pCircularBuffer->CircularBuffer);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pCircularBuffer->dwCapacity = dwCapacity;
    pCircularBuffer->dwElementSize = dwElementSize;
    pCircularBuffer->dwSize = pCircularBuffer->dwHead = 0;
    *ppCircularBuffer = pCircularBuffer;

cleanup:
    return dwError;

error:
    VmAfdCircularBufferFree(pCircularBuffer);

    goto cleanup;
}

static
VOID
_VmAfdCircularBufferLock(
    PVMAFD_CIRCULAR_BUFFER pCircularBuffer
    )
{
    //pthread_mutex_lock(&(pCircularBuffer->mutex));
    pthread_mutex_lock(&g_mutex);
}

static
VOID
_VmAfdCircularBufferUnlock(
    PVMAFD_CIRCULAR_BUFFER pCircularBuffer
    )
{
    pthread_mutex_unlock(&g_mutex);
}

DWORD
VmAfdCircularBufferReset(
    PVMAFD_CIRCULAR_BUFFER pCircularBuffer
    )
{
    _VmAfdCircularBufferLock(pCircularBuffer);
    pCircularBuffer->dwHead = pCircularBuffer->dwSize = 0;
    _VmAfdCircularBufferUnlock(pCircularBuffer);

    return ERROR_SUCCESS;
}

DWORD
VmAfdCircularBufferGetSize(
    PVMAFD_CIRCULAR_BUFFER pCircularBuffer,
    PDWORD pSize
    )
{
    _VmAfdCircularBufferLock(pCircularBuffer);
    *pSize = pCircularBuffer->dwSize;
    _VmAfdCircularBufferUnlock(pCircularBuffer);

    return ERROR_SUCCESS;
}

DWORD
VmAfdCircularBufferGetCapacity(
    PVMAFD_CIRCULAR_BUFFER pCircularBuffer,
    PDWORD pdwCapacity
    )
{
    _VmAfdCircularBufferLock(pCircularBuffer);
    *pdwCapacity = pCircularBuffer->dwCapacity;
    _VmAfdCircularBufferUnlock(pCircularBuffer);

    return ERROR_SUCCESS;
}

DWORD
VmAfdCircularBufferSetCapacity(
    PVMAFD_CIRCULAR_BUFFER pCircularBuffer,
    DWORD dwCapacity
    )
{
    DWORD dwError = 0;
    DWORD dwBufferSize = 0;

    _VmAfdCircularBufferLock(pCircularBuffer);

    //
    // Check if new capacity is bigger than current one
    //
    if (dwCapacity > pCircularBuffer->dwCapacity)
    {
        dwBufferSize = dwCapacity * pCircularBuffer->dwElementSize;
        dwError = VmAfdReallocateMemory(pCircularBuffer->CircularBuffer, (PVOID)&pCircularBuffer->CircularBuffer, dwBufferSize);
        BAIL_ON_VMAFD_ERROR(dwError);

        //
        // Set the new capacity for the buffer and then reset our buffer.
        //
        pCircularBuffer->dwCapacity = dwCapacity;
        pCircularBuffer->dwHead = pCircularBuffer->dwSize = 0;
    }
    else
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    _VmAfdCircularBufferUnlock(pCircularBuffer);

    goto cleanup;
}

static
PVOID
_VmAfdCircularBufferGetElementAtIndex(
    PVMAFD_CIRCULAR_BUFFER pCircularBuffer,
    DWORD dwIndex
    )
{
    return &pCircularBuffer->CircularBuffer[dwIndex * pCircularBuffer->dwElementSize];
}

PVOID
VmAfdCircularBufferGetNextEntry(
    PVMAFD_CIRCULAR_BUFFER pCircularBuffer
    )
{
    DWORD dwIndex = 0;

    //
    // Pre-conditions.
    //
    //assert(pCircularBuffer->dwHead < pCircularBuffer->dwCapacity);
    //assert(pCircularBuffer->dwSize <= pCircularBuffer->dwCapacity);

    dwIndex = pCircularBuffer->dwHead;

    pCircularBuffer->dwHead = (pCircularBuffer->dwHead + 1) % pCircularBuffer->dwCapacity;
    pCircularBuffer->dwSize = VMAFD_MIN((pCircularBuffer->dwSize + 1), pCircularBuffer->dwCapacity);

    //assert(pCircularBuffer->dwHead < pCircularBuffer->dwCapacity);
    //assert(pCircularBuffer->dwSize <= pCircularBuffer->dwCapacity);
    //assert(dwIndex < pCircularBuffer->dwCapacity);
    //assert(dwIndex < pCircularBuffer->dwSize);

    return _VmAfdCircularBufferGetElementAtIndex(pCircularBuffer, dwIndex);
}

static
DWORD
_VmAfdCircularBufferGetStartIndex(
    PVMAFD_CIRCULAR_BUFFER pCircularBuffer
    )
{
    if (pCircularBuffer->dwSize < pCircularBuffer->dwCapacity)
    {
        return 0;
    }
    else
    {
        return pCircularBuffer->dwHead;
    }
}

DWORD
VmAfdCircularBufferSelectElements(
    PVMAFD_CIRCULAR_BUFFER pCircularBuffer,
    DWORD dwCount, // 0 implies that we want all elements.
    CIRCULAR_BUFFER_SELECT_CALLBACK Callback,
    PVOID pContext
    )
{
    DWORD dwBufferIndex = 0;

    _VmAfdCircularBufferLock(pCircularBuffer);

    if (dwCount == 0)
    {
        dwCount = pCircularBuffer->dwSize;
    }
    else
    {
        dwCount = VMAFD_MIN(dwCount, pCircularBuffer->dwSize);
    }

    dwBufferIndex  = _VmAfdCircularBufferGetStartIndex(pCircularBuffer);
    for (; dwCount > 0; dwCount--)
    {
        PVOID Element = _VmAfdCircularBufferGetElementAtIndex(pCircularBuffer, dwBufferIndex++);

        //
        // Start over at the beginning of the circle when we get to the end.
        //
        if (dwBufferIndex == pCircularBuffer->dwCapacity)
        {
            dwBufferIndex = 0;
        }

        if (!Callback(Element, pContext))
        {
            break;
        }
    }

    _VmAfdCircularBufferUnlock(pCircularBuffer);

    return ERROR_SUCCESS;
}
