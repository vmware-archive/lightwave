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
 * Module Name: dns main
 *
 * Filename: prototypes.h
 *
 * Abstract:
 *
 * dns main module prototypes
 *
 */


#ifdef __cplusplus
extern "C" {
#endif

// init.c
DWORD
VmDnsInit(
    VOID
    );

int
LoadServerGlobals(
    VOID
    );

// TODO: remove ifndef for regconfig.c once registry config
// is ported to windows...
#ifndef _WIN32

// regconfig.c

DWORD
VmDnsSrvUpdateConfig(
    VOID
    );
#endif

DWORD
VmDnsConfigSetAdminCredentials(
    PCSTR pszUserDN,
    PCSTR pszPassword
    );

// rpcmemory.c

DWORD
VmDnsRpcAllocateMemory(
    size_t size,
    PVOID* ppMemory
    );

VOID
VmDnsRpcFreeMemory(
    PVOID pMemory
    );

// rpcstring.c

DWORD
VmDnsRpcAllocateStringW(
    PWSTR  pwszSrc,
    PWSTR* ppwszDst
    );

/* service.c */

DWORD
VmDnsRpcServerInit(
    VOID
    );

VOID
VmDnsRpcServerShutdown(
    VOID
    );

DWORD
VmDnsRpcAuthCallback(
    PVOID         Context
    );

/* rpc.c */

DWORD
VmDnsRpcServerStartListen(
    VOID
    );

DWORD
VmDnsRpcServerStopListen(
    VOID
    );

DWORD
VmDnsRpcServerRegisterIf(
    rpc_if_handle_t pInterfaceSpec
    );

DWORD VmDnsRpcServerUseProtSeq(
    PCSTR pszProtSeq
    );

DWORD VmDnsRpcServerUseProtSeqEp(
    PCSTR pszProtSeq,
    PCSTR pszEndpoint
    );

DWORD
VmDnsRpcServerInqBindings(
    rpc_binding_vector_p_t* ppServerBindings
    );

DWORD
VmDnsRpcEpRegister(
    rpc_binding_vector_p_t pServerBinding,
    rpc_if_handle_t        pInterfaceSpec,
    PCSTR                  pszAnnotation
    );

DWORD
VmDnsRpcServerRegisterAuthInfo(
    VOID
    );

DWORD
VmDnsRpcBindingInqAuthClient(
    rpc_binding_handle_t hClientBinding,
    rpc_authz_handle_t* pPrivs,
    PSTR* ppServerPrincName,
    DWORD* pAuthnLevel,
    DWORD* pAuthnSvc,
    DWORD* pAuthzSvc
    );

DWORD
VmDnsRpcBindingVectorFree(
    rpc_binding_vector_p_t* ppServerBindings
    );

#ifndef _WIN32

// signals.c
VOID
VmDnsBlockSelectedSignals(
    VOID
    );

DWORD
VmDnsHandleSignals(
    VOID
    );
#endif

/*parseargs.c*/

DWORD
VmDnsParseArgs(
    int argc,
    char* argv[],
    int* pLoggingLevel,
    PCSTR* ppszLogFileName,
    int* pLdapPort,
    PBOOLEAN pbEnableSysLog,
    PBOOLEAN pbConsoldeMode
    );

VOID
ShowUsage(
    PSTR pName
    );

/* utils.c */
VOID
VmDnsdStateSet(
    VMDNS_SERVER_STATE   state
);

/* auth.c */
DWORD
VmDnsCheckAccess(
    handle_t IDL_handle,
    BOOL bNeedAdminPrivilage
    );

/* dirsync.c*/
DWORD
VmDnsStartDirectorySync(
    );

DWORD
VmDnsStopDirectorySync(
    );

#ifdef __cplusplus
}
#endif
