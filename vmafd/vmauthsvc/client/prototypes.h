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



/* binding.c */

ULONG
VmAuthsvcCreateBindingHandleA(
    PCSTR      pszNetworkAddress,
    PCSTR      pszNetworkEndpoint,
    handle_t * ppBinding
    );

ULONG
VmAuthsvcCreateBindingHandleW(
    PCWSTR      pwszNetworkAddress,
    PCWSTR      pwszNetworkEndpoint,
    handle_t * ppBinding
    );

#ifdef UNICODE
    #define VmAuthsvcCreateBindingHandle VmAuthsvcCreateBindingHandleW
#else // non- UNICODE
    #define VmAuthsvcCreateBindingHandle VmAuthsvcCreateBindingHandleA
#endif // #ifdef UNICODE

VOID
VmAuthsvcFreeBindingHandle(
    handle_t * ppBinding
    );

/* rpc.c */
DWORD
VmAuthsvcRpcStringBindingCompose(
    PCSTR pszProtSeq,
    PCSTR pszNetworkAddr,
    PCSTR pszEndPoint,
    PSTR* ppszStringBinding
);

DWORD
VmAuthsvcRpcBindingFromStringBinding(
  PCSTR pszStringBinding,
  handle_t* pBinding
);

DWORD
VmAuthsvcRpcBindingSetAuthInfo(
  handle_t pBinding,
  unsigned long AuthnLevel,
  unsigned long AuthnSvc,
  unsigned long AuthzService
);

DWORD
VmAuthsvcRpcFreeString(
    PSTR* ppszString
);

DWORD
VmAuthsvcRpcFreeBinding(
    handle_t* pBinding
);
