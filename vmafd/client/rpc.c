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

DWORD
VmAfdRpcFreeString(
    PSTR* ppszString
)
{
    DWORD dwError = ERROR_SUCCESS;

     rpc_string_free((PBYTE*)ppszString, &dwError);

    return dwError;
}

DWORD
VmAfdRpcFreeBinding(
    handle_t* pBinding
)
{
    DWORD dwError = ERROR_SUCCESS;

    rpc_binding_free(pBinding, &dwError);

    return dwError;
}

VOID
VmAfdRpcClientFreeMemory(
    PVOID pMemory
    )
{
    if (pMemory)
    {
        DWORD rpcStatus = rpc_s_ok;
        rpc_sm_client_free(pMemory, &rpcStatus);
    }
}

VOID
VmAfdRpcClientFreeStringArrayA(
    PSTR*  ppszStrArray,
    DWORD  dwCount
    )
{
    DWORD iStr = 0;

    for (; iStr < dwCount; iStr++)
    {
        VmAfdRpcClientFreeStringA(ppszStrArray[iStr]);
    }
    VmAfdRpcClientFreeMemory(ppszStrArray);
}

VOID
VmAfdRpcClientFreeStringArrayW(
    PWSTR* ppwszStrArray,
    DWORD  dwCount
    )
{
    DWORD iStr = 0;

    for (; iStr < dwCount; iStr++)
    {
        VmAfdRpcClientFreeStringW(ppwszStrArray[iStr]);
    }
    VmAfdRpcClientFreeMemory(ppwszStrArray);
}

VOID
VmAfdRpcClientFreeStringA(
    PSTR pszStr
    )
{
    if (pszStr)
    {
        VmAfdRpcClientFreeMemory(pszStr);
    }
}

VOID
VmAfdRpcClientFreeStringW(
    PWSTR pwszStr
    )
{
    if (pwszStr)
    {
        VmAfdRpcClientFreeMemory(pwszStr);
    }
}
