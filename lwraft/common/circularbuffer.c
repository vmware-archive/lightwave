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

VOID VmDirCircularBufferFree(
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer
    )
{
    if (pCircularBuffer != NULL)
    {
        VMDIR_SAFE_FREE_MUTEX(pCircularBuffer->mutex);
        VMDIR_SAFE_FREE_MEMORY(pCircularBuffer->CircularBuffer);

        VmDirFreeMemory(pCircularBuffer);
    }
}

DWORD
VmDirCircularBufferCreate(
    DWORD dwCapacity,
    DWORD dwElementSize,
    PVMDIR_CIRCULAR_BUFFER *ppCircularBuffer
    )
{
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer = NULL;
    DWORD dwError = 0;
    DWORD dwBufferSize = 0;

    dwError = VmDirAllocateMemory(sizeof(*pCircularBuffer), (PVOID)&pCircularBuffer);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&pCircularBuffer->mutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    //
    // Calculate the size of the buffer. We need to prevent overflow, as well as
    // disallow a zero-sized buffer.
    //
    dwBufferSize = dwCapacity * dwElementSize;
    if (dwBufferSize != 0 && dwBufferSize > dwCapacity)
    {
        dwError = VmDirAllocateMemory(dwCapacity * dwElementSize, (PVOID)&pCircularBuffer->CircularBuffer);
    }
    else
    {
        dwError = ERROR_INVALID_PARAMETER;
    }

    pCircularBuffer->dwCapacity = dwCapacity;
    pCircularBuffer->dwElementSize = dwElementSize;
    pCircularBuffer->dwSize = pCircularBuffer->dwHead = 0;
    *ppCircularBuffer = pCircularBuffer;

cleanup:
    return dwError;

error:
    VmDirCircularBufferFree(pCircularBuffer);

    goto cleanup;
}

static
VOID
_VmDirCircularBufferLock(
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer
    )
{
    VmDirLockMutex(pCircularBuffer->mutex);
}

static
VOID
_VmDirCircularBufferUnlock(
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer
    )
{
    VmDirUnLockMutex(pCircularBuffer->mutex);
}

DWORD
VmDirCircularBufferReset(
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer
    )
{
    _VmDirCircularBufferLock(pCircularBuffer);
    pCircularBuffer->dwHead = pCircularBuffer->dwSize = 0;
    _VmDirCircularBufferUnlock(pCircularBuffer);

    return ERROR_SUCCESS;
}

DWORD
VmDirCircularBufferGetSize(
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer,
    PDWORD pSize
    )
{
    *pSize = pCircularBuffer->dwSize;

    return ERROR_SUCCESS;
}

DWORD
VmDirCircularBufferGetCapacity(
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer,
    PDWORD pdwCapacity
    )
{
    *pdwCapacity = pCircularBuffer->dwCapacity;

    return ERROR_SUCCESS;
}

DWORD
VmDirCircularBufferSetCapacity(
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer,
    DWORD dwCapacity
    )
{
    DWORD dwError = 0;
    DWORD dwBufferSize = 0;

    _VmDirCircularBufferLock(pCircularBuffer);

    //
    // Check for overflow.
    //
    dwBufferSize = dwCapacity * pCircularBuffer->dwElementSize;
    if (dwBufferSize > dwCapacity)
    {
        dwError = VmDirReallocateMemory(pCircularBuffer->CircularBuffer, (PVOID)&pCircularBuffer->CircularBuffer, dwBufferSize);
        BAIL_ON_VMDIR_ERROR(dwError);

        //
        // Set the new capacity for the buffer and then reset our buffer.
        //
        pCircularBuffer->dwCapacity = dwCapacity;
        pCircularBuffer->dwHead = pCircularBuffer->dwSize = 0;
    }
    else
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:
    _VmDirCircularBufferUnlock(pCircularBuffer);

    return dwError;
}

static
PVOID
_VmDirCircularBufferGetElementAtIndex(
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer,
    DWORD dwIndex
    )
{
    return &pCircularBuffer->CircularBuffer[dwIndex * pCircularBuffer->dwElementSize];
}

PVOID
VmDirCircularBufferGetNextEntry(
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer
    )
{
    DWORD dwIndex = 0;

    //
    // Pre-conditions.
    //
    assert(pCircularBuffer->dwHead < pCircularBuffer->dwCapacity);
    assert(pCircularBuffer->dwSize <= pCircularBuffer->dwCapacity);

    dwIndex = pCircularBuffer->dwHead;

    pCircularBuffer->dwHead = (pCircularBuffer->dwHead + 1) % pCircularBuffer->dwCapacity;
    pCircularBuffer->dwSize = VMDIR_MIN((pCircularBuffer->dwSize + 1), pCircularBuffer->dwCapacity);

    assert(pCircularBuffer->dwHead < pCircularBuffer->dwCapacity);
    assert(pCircularBuffer->dwSize <= pCircularBuffer->dwCapacity);
    assert(dwIndex < pCircularBuffer->dwCapacity);
    assert(dwIndex < pCircularBuffer->dwSize);

    return _VmDirCircularBufferGetElementAtIndex(pCircularBuffer, dwIndex);
}

static
DWORD
_VmDirCircularBufferGetStartIndex(
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer
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
VmDirCircularBufferSelectElements(
    PVMDIR_CIRCULAR_BUFFER pCircularBuffer,
    DWORD dwCount, // 0 implies that we want all elements.
    CIRCULAR_BUFFER_SELECT_CALLBACK Callback,
    PVOID pContext
    )
{
    DWORD dwBufferIndex = 0;

    _VmDirCircularBufferLock(pCircularBuffer);

    if (dwCount == 0)
    {
        dwCount = pCircularBuffer->dwSize;
    }
    else
    {
        dwCount = VMDIR_MIN(dwCount, pCircularBuffer->dwSize);
    }

    dwBufferIndex  = _VmDirCircularBufferGetStartIndex(pCircularBuffer);
    for (; dwCount > 0; dwCount--)
    {
        PVOID Element = _VmDirCircularBufferGetElementAtIndex(pCircularBuffer, dwBufferIndex++);

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

    _VmDirCircularBufferUnlock(pCircularBuffer);

    return ERROR_SUCCESS;
}
