/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ?~@~\License?~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ?~@~\AS IS?~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "includes.h"

DWORD
VmAllocateMemory(
    size_t   dwSize,
    PVOID*   ppMemory
    )
{
    DWORD dwError = 0;
    void* pMemory = NULL;

    if (!ppMemory || !dwSize)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    pMemory = calloc(1, dwSize);

    if (!pMemory)
    {
        dwError = VM_COMMON_ERROR_NO_MEMORY;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

cleanup:
    *ppMemory = pMemory;

    return dwError;

error:
    VM_COMMON_SAFE_FREE_MEMORY(pMemory);
    pMemory = NULL;

    goto cleanup;
}

DWORD
VmReallocateMemory(
    PVOID        pMemory,
    PVOID*       ppNewMemory,
    size_t       dwSize
    )
{
    DWORD dwError = 0;
    void*    pNewMemory = NULL;

    if (!ppNewMemory)
    {
        dwError = VM_COMMON_ERROR_INVALID_PARAMETER;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    if (pMemory)
    {
        pNewMemory = realloc(pMemory, dwSize);
    }
    else
    {
        dwError = VmAllocateMemory(dwSize, &pNewMemory);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    if (!pNewMemory)
    {
        dwError = VM_COMMON_ERROR_NO_MEMORY;
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    *ppNewMemory = pNewMemory;

cleanup:
    return dwError;

error:
    goto cleanup;
}

VOID
VmFreeMemory(
    PVOID   pMemory
    )
{
    if (pMemory)
    {
        free(pMemory);
    }

    return;
}
