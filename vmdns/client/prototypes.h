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
 * Module   : prototypes.h
 *
 * Abstract :
 *
 *            VMware dns Service
 *
 *            Client API
 *
 *            Core API
 */

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

DWORD
VmDnsAllocateFromRpcRecordArray(
    PVMDNS_RECORD_ARRAY pRecordArray,
    PVMDNS_RECORD_ARRAY *ppRecordArray
    );

VOID
VmDnsRpcClientFreeRpcRecordArray(
    PVMDNS_RECORD_ARRAY pRecordArray
    );

DWORD
VmDnsAllocateFromRpcZoneInfoArray(
    PVMDNS_ZONE_INFO_ARRAY pZoneInfoArray,
    PVMDNS_ZONE_INFO_ARRAY *ppZoneInfoArray
    );

VOID
VmDnsRpcClientFreeZoneInfo(
    PVMDNS_ZONE_INFO pZoneInfo
    );


VOID
VmDnsRpcClientFreeZoneInfoArray(
    PVMDNS_ZONE_INFO_ARRAY pZoneInfoArray
    );
