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
VmDirRpcFreeString(
    PSTR* ppszString
)
{
    DWORD dwError = ERROR_SUCCESS;
#if !defined(_WIN32) || defined(HAVE_DCERPC_WIN32)
     rpc_string_free((PBYTE*)ppszString, &dwError);
#else
    dwError = RpcStringFreeA((RPC_CSTR*)ppszString);
#endif
    return dwError;
}

DWORD
VmDirRpcFreeBinding(
    handle_t* pBinding
)
{
    DWORD dwError = ERROR_SUCCESS;
#if !defined(_WIN32) || defined(HAVE_DCERPC_WIN32)
     rpc_binding_free(pBinding, &dwError);
#else
    dwError = RpcBindingFree(pBinding);
#endif
    return dwError;
}
