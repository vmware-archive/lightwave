/*
 * Copyright (C) 2011 VMware, Inc. All rights reserved.
 *
 * Module   : prototypes.h
 *
 * Abstract :
 *
 *            VMware dns Service
 *
 *            Client API
 *
 *            Core API
 *
 * Authors  : Sriram Nambakam (snambakam@vmware.com)
 *
 */

/* binding.c */

struct _VMDNS_SERVER_CONTEXT
{
    handle_t hBinding;
};

DWORD
VmDnsCreateBindingHandleA(
    PCSTR pszNetworkAddress,
    PCSTR pszNetworkEndpoint,
    PCSTR pszUserName,
    PCSTR pszDomain,
    PCSTR pszPassword,
    handle_t* ppInBinding
    );

DWORD
VmDnsCreateBindingHandleW(
    PCWSTR pwszNetworkAddress,
    PCWSTR pwszNetworkEndpoint,
    PCWSTR pwszUserName,
    PCWSTR pwszDomain,
    PCWSTR pwszPassword,
    handle_t * ppBinding
    );

#ifdef UNICODE
    #define VmDnsCreateBindingHandle VmDnsCreateBindingHandleW
#else // non- UNICODE
    #define VmDnsCreateBindingHandle VmDnsCreateBindingHandleA
#endif // #ifdef UNICODE

VOID
VmDnsFreeBindingHandle(
    handle_t * ppBinding
    );

/* rpc.c */
DWORD
VmDnsRpcStringBindingCompose(
    PCSTR pszProtSeq,
    PCSTR pszNetworkAddr,
    PCSTR pszEndPoint,
    PSTR* ppszStringBinding
);

DWORD
VmDnsRpcBindingFromStringBinding(
  PCSTR pszStringBinding,
  handle_t* pBinding
);

DWORD
VmDnsRpcFreeString(
    PSTR* ppszString
);

DWORD
VmDnsRpcFreeBinding(
    handle_t* pBinding
);

VOID
VmDnsRpcClientFreeMemory(
PVOID pMemory
);

VOID
VmDnsRpcClientFreeStringArrayA(
PSTR*  ppszStrArray,
DWORD  dwCount
);

VOID
VmDnsRpcClientFreeStringA(
PSTR pszStr
);

