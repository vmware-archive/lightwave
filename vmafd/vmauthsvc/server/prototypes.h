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
 * Module Name: Authsvc main
 *
 * Filename: prototypes.h
 *
 * Abstract:
 *
 * Authsvc main module prototypes
 *
 */


#ifdef __cplusplus
extern "C" {
#endif

/* init.c */

DWORD
VmAuthsvcInit(
    VOID
    );

int
LoadServerGlobals(
    VOID
    );

// TODO: remove ifndef for regconfig.c once registry config
// is ported to windows...
#ifndef _WIN32

/* regconfig.c */

DWORD
VmAuthsvcSrvUpdateConfig(
    VOID
    );
#endif

DWORD
VmAuthsvcConfigSetAdminCredentials(
    PCSTR pszUserDN,
    PCSTR pszPassword
    );

/* rpcmemory.c */

DWORD
VmAuthsvcRpcAllocateMemory(
    size_t size,
    PVOID* ppMemory
    );

VOID
VmAuthsvcRpcFreeMemory(
    PVOID pMemory
    );

/* rpcstring.c */

DWORD
VmAuthsvcRpcAllocateStringW(
    PWSTR  pwszSrc,
    PWSTR* ppwszDst
    );

/* service.c */

DWORD
VmAuthsvcRpcServerInit(
    VOID
    );

VOID
VmAuthsvcRpcServerShutdown(
    VOID
    );

DWORD
VmAuthsvcRpcAuthCallback(
    PVOID         Context
);

/* rpc.c */

DWORD
VmAuthsvcRpcServerStartListen(
    VOID
);

DWORD
VmAuthsvcRpcServerStopListen(
    VOID
);

DWORD
VmAuthsvcRpcServerRegisterIf(
    rpc_if_handle_t pInterfaceSpec
);

DWORD VmAuthsvcRpcServerUseProtSeq(
    PCSTR pszProtSeq
);

DWORD VmAuthsvcRpcServerUseProtSeqEp(
    PCSTR pszProtSeq,
    PCSTR pszEndpoint
);

DWORD
VmAuthsvcRpcServerInqBindings(
    rpc_binding_vector_p_t* ppServerBindings
);

DWORD
VmAuthsvcRpcEpRegister(
    rpc_binding_vector_p_t pServerBinding,
    rpc_if_handle_t        pInterfaceSpec,
    PCSTR                  pszAnnotation
);

DWORD
VmAuthsvcRpcServerRegisterAuthInfo(
    VOID
);

DWORD
VmAuthsvcRpcBindingInqAuthClient(
    rpc_binding_handle_t hClientBinding,
    rpc_authz_handle_t* pPrivs,
    PSTR* ppServerPrincName,
    DWORD* pAuthnLevel,
    DWORD* pAuthnSvc,
    DWORD* pAuthzSvc
);

DWORD
VmAuthsvcRpcBindingVectorFree(
    rpc_binding_vector_p_t* ppServerBindings
);

#ifndef _WIN32

/* signals.c */

VOID
VmAuthsvcBlockSelectedSignals(
    VOID
    );

DWORD
VmAuthsvcHandleSignals(
    VOID
    );
#endif

/* parseargs.c */

DWORD
VmAuthsvcParseArgs(
    int argc,
    char* argv[],
    int* pLoggingLevel,
    PCSTR* ppszLogFileName,
    int* pLdapPort,
    PBOOLEAN pbEnableSysLog
);

VOID
ShowUsage(
    PSTR pName
);

/* utils.c */

VOID
VmAuthsvcdStateSet(
    VMAUTHSVC_SERVER_STATE   state
);

#ifdef __cplusplus
}
#endif
