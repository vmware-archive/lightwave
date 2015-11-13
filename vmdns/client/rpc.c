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
 * Module   : binding.c
 *
 * Abstract :
 *
 *            VMware dns Service
 *
 *            system RPC functions
 *
 */

#include "includes.h"

DWORD
VmDnsRpcStringBindingCompose(
    PCSTR pszProtSeq,
    PCSTR pszNetworkAddr,
    PCSTR pszEndPoint,
    PSTR* ppszStringBinding
)
{
    DWORD dwError = ERROR_SUCCESS;

    rpc_string_binding_compose(
        NULL,
        (PBYTE)pszProtSeq,
        (PBYTE)pszNetworkAddr,
        (PBYTE)pszEndPoint,
        NULL,
        (PBYTE*)ppszStringBinding,
        &dwError);

    return dwError;
}

DWORD
VmDnsRpcBindingFromStringBinding(
  PCSTR pszStringBinding,
  handle_t* pBinding
)
{
    DWORD dwError = ERROR_SUCCESS;

    rpc_binding_from_string_binding(
        (PBYTE)pszStringBinding,
        pBinding,
        &dwError);

    return dwError;
}

DWORD
VmDnsRpcBindingSetAuthInfo(
  handle_t pBinding,
  unsigned long ulAuthnLevel,
  unsigned long ulAuthnSvc,
  unsigned long ulAuthzService
)
{
    DWORD dwError = ERROR_SUCCESS;

    rpc_binding_set_auth_info(
        pBinding,
        NULL,         /* Server Principal Name */
        ulAuthnLevel,
        ulAuthnSvc,
        NULL,         /* Auth Identity */
        ulAuthzService,
        &dwError);

    return dwError;
}

DWORD
VmDnsRpcFreeString(
    PSTR* ppszString
)
{
    DWORD dwError = ERROR_SUCCESS;

    rpc_string_free((PBYTE*)ppszString, &dwError);

    return dwError;
}

DWORD
VmDnsRpcFreeBinding(
    handle_t* pBinding
)
{
    DWORD dwError = ERROR_SUCCESS;

    rpc_binding_free(pBinding, &dwError);

    return dwError;
}

VOID
VmDnsRpcClientFreeMemory(
PVOID pMemory
)
{
    if (pMemory)
    {
        DWORD rpcStatus = ERROR_SUCCESS;
        rpc_sm_client_free(pMemory, &rpcStatus);
    }
}

VOID
VmDnsRpcClientFreeStringArrayA(
PSTR*  ppszStrArray,
DWORD  dwCount
)
{
    DWORD iStr = 0;

    for (; iStr < dwCount; iStr++)
    {
        VmDnsRpcClientFreeStringA(ppszStrArray[iStr]);
    }
    VmDnsRpcClientFreeMemory(ppszStrArray);
}

VOID
VmDnsRpcClientFreeStringA(
PSTR pszStr
)
{
    if (pszStr)
    {
        VmDnsRpcClientFreeMemory(pszStr);
    }
}
