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
 * Module Name:  rpcmemory.c
 *
 * Abstract: Common rpc memory handling functions.
 *
 */

#include "includes.h"

DWORD
VmDnsRpcAllocateMemory(
    size_t   dwSize,
    PVOID*   ppMemory
    )
{
    DWORD dwError = 0;
    void* pMemory = NULL;

    if (!ppMemory || !dwSize)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pMemory = rpc_ss_allocate((idl_size_t)dwSize);
    if (!pMemory)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    memset(pMemory,0, dwSize);

    *ppMemory = pMemory;

cleanup:
    *ppMemory = pMemory;

    return dwError;

error:
    rpc_ss_free(pMemory);
    pMemory = NULL;

    goto cleanup;
}

DWORD
VmDnsRpcAllocateStringA(
    PCSTR   pszString,
    PSTR*   ppszString
    )
{
    DWORD  dwError = 0;
    PSTR   pszNewString = NULL;
    size_t dwLen = 0;

    if (!pszString || !ppszString)
    {
        if (ppszString) { *ppszString = NULL; }
        return 0;
    }

    dwLen = VmDnsStringLenA(pszString);
    // + 1 for \'0'
    dwError = VmDnsRpcAllocateMemory(dwLen + 1, (PVOID*)&pszNewString);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyMemory(pszNewString, (dwLen + 1), pszString, dwLen);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppszString = pszNewString;

cleanup:

    return dwError;

error:
    rpc_ss_free(pszNewString);
    goto cleanup;
}

VOID
VmDnsRpcFreeStringA(
    PSTR    pszString
    )
{
    DWORD  dwError = 0;
    if (pszString)
    {
        rpc_string_free((PBYTE*)&pszString, &dwError);
    }
}

VOID
VmDnsRpcFreeMemory(
    PVOID pMemory
    )
{
    if (pMemory)
    {
        rpc_ss_free(pMemory);
    }
}


VOID
VmDnsRpcServerFreeStringArrayA(
    PSTR*  ppszStrArray,
    DWORD  dwCount
    )
{
    DWORD iStr = 0;

    for (; iStr < dwCount; iStr++)
    {
        VmDnsRpcFreeStringA(ppszStrArray[iStr]);
    }
    VmDnsRpcFreeMemory(ppszStrArray);
}

