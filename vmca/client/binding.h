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
 * Module Name: ThinAppRepoClient
 *
 * Filename: prototypes.h
 *
 * Abstract:
 *
 * Private function prototypes
 *
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include <vmcacommon.h>

/* binding.c */

DWORD
CreateBindingHandleSharedKeyA(
    PCSTR pwszNetworkAddress,
    PCSTR pwszNetworkEndpoint,
    handle_t * pBindingHandleSharedKey
    );

DWORD
CreateBindingHandleSharedKeyW(
    PCWSTR pwszNetworkAddress,
    PCWSTR pwszNetworkEndpoint,
    handle_t * pBindingHandleSharedKey
    );


DWORD
CreateBindingHandleKrbA(
    PCSTR pwszNetworkAddress,
    PCSTR pwszNetworkEndpoint,
    handle_t * pBindingHandle
    );

DWORD
CreateBindingHandleKrbW(
    PCWSTR pwszNetworkAddress,
    PCWSTR pwszNetworkEndpoint,
    handle_t * pBindingHandle
    );

/*
 * Same as CreateBindingHandleKrb() but performs SRP when ser/pass is provided.
 */
DWORD
CreateBindingHandleAuthA(
    PCSTR pszNetworkAddress,
    PCSTR pszNetworkEndpoint,
    PCSTR     pszUserName,
    PCSTR     pszDomain,
    PCSTR     pszPassword,
    handle_t * ppBinding
    );

DWORD
CreateBindingHandleAuthW(
    PCWSTR pszNetworkAddress,
    PCWSTR pszNetworkEndpoint,
    PCWSTR pszUserName,
    PCWSTR pszDomain,
    PCWSTR pszPassword,
    handle_t * ppBinding
    );

DWORD
VMCAFreeBindingHandle(
    handle_t * pBindingHandle
    );

#ifdef UNICODE
#define CreateBindingHandleSharedKey      CreateBindingHandleSharedKeyW
#define CreateBindingHandleKrb            CreateBindingHandleKrbW
#define CreateBindingHandleAuth           CreateBindingHandleAuthW
#else
#define CreateBindingHandleSharedKey      CreateBindingHandleSharedKeyA
#define CreateBindingHandleKrb            CreateBindingHandleKrbA
#define CreateBindingHandleAuth           CreateBindingHandleAuthA
#endif

#ifdef __cplusplus
}
#endif
