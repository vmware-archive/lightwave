/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
 * Module Name: ThinAppVMCAService
 *
 * Filename: rpcmemory.c
 *
 * Abstract:
 *
 * RPC Server Side Memory Allocation
 *
 */

#include "includes.h"

DWORD
VMCARpcAllocateMemory(
    size_t size,
    PVOID* ppMemory
    )
{
    DWORD dwError = 0;
    PVOID pMemory = NULL;

    if (size <= 0 || !ppMemory)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    pMemory = rpc_ss_allocate((idl_size_t)size);
    if (!pMemory)
    {
        dwError = ERROR_OUTOFMEMORY;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    memset(pMemory,0, size);
    *ppMemory = pMemory;

cleanup:

    return dwError;

error:

    if (ppMemory)
    {
        *ppMemory = NULL;
    }

    goto cleanup;
}

VOID
VMCARpcFreeMemory(
    PVOID pMemory
    )
{
    if (pMemory)
    {
        rpc_ss_free(pMemory);
    }
}

