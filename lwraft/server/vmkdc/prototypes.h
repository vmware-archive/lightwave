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
 * Module Name: Kdc main
 *
 * Filename: prototypes.h
 *
 * Abstract:
 *
 * Kdc main module prototypes
 *
 */


#ifdef __cplusplus
extern "C" {
#endif

/* auth.c */

ULONG
ConstructSDForVmKdcServ(
    PSECURITY_DESCRIPTOR_ABSOLUTE * ppSD
    );

BOOL
VmKdcIsRpcOperationAllowed(
    handle_t pBinding,
    PSECURITY_DESCRIPTOR_ABSOLUTE pSD,
    ULONG    ulAccessDesired
    );

/* init.c */

DWORD
VmKdcInit(
    VOID
    );

DWORD
VmKdcInitLoop(
    PVMKDC_GLOBALS pGlobals
    );

int
LoadServerGlobals();

/* instance.c */

DWORD
VmKdcSrvSetupHostInstance(
    PCSTR pszDomainName,
    PCSTR pszUsername,
    PCSTR pszPassword,
    PCSTR pszSiteName,
    PCSTR pszServerId,
    PCSTR pszReplURI,
    PCSTR pszReplBindDN,
    PCSTR pszReplBindPassword,
    PCSTR pszReplBase
    );

DWORD
VmKdcSrvSetupTenantInstance(
    PCSTR pszDomainName,
    PCSTR pszUsername,
    PCSTR pszPassword
    );

/* main.c */
/*
DWORD
VmKdcServiceStartup(
    VOID
);

VOID
VmKdcServiceShutdown(
    VOID
);
*/

/* regconfig.c */

DWORD
VmKdcSrvUpdateConfig(
    PVMKDC_GLOBALS pGlobals
    );

/* rpcmemory.c */

DWORD
VmKdcRpcAllocateMemory(
    size_t size,
    PVOID* ppMemory
    );

/* rpcstring.c */

DWORD
VmKdcRpcAllocateStringW(
    PWSTR  pwszSrc,
    PWSTR* ppwszDst
    );

/* service.c */

DWORD
VmKdcRpcServerInit(
    VOID
    );

VOID
VmKdcRpcServerShutdown(
    VOID
    );

DWORD
VmKdcRpcAuthCallback(
    PVOID         Context
);

/* rpc.c */

DWORD
VmKdcRpcServerStartListen(
    VOID
);

DWORD
VmKdcRpcServerStopListen(
    VOID
);

DWORD
VmKdcRpcServerRegisterIf(
    rpc_if_handle_t pInterfaceSpec
);

DWORD
VmKdcRpcServerUseProtSeq(
    PCSTR pszProtSeq
);

DWORD
VmKdcRpcServerUseProtSeqEp(
    PCSTR pszProtSeq,
    PCSTR pszEndpoint
);

DWORD
VmKdcRpcServerInqBindings(
    rpc_binding_vector_p_t* ppServerBindings
);

DWORD
VmKdcRpcEpRegister(
    rpc_binding_vector_p_t pServerBinding,
    rpc_if_handle_t        pInterfaceSpec,
    PCSTR                  pszAnnotation
);

DWORD
VmKdcRpcServerRegisterAuthInfo(
    VOID
);

DWORD
VmKdcRpcBindingInqAuthClient(
    rpc_binding_handle_t hClientBinding,
    rpc_authz_handle_t* pPrivs,
    PSTR* ppServerPrincName,
    DWORD* pAuthnLevel,
    DWORD* pAuthnSvc,
    DWORD* pAuthzSvc
);

DWORD
VmKdcRpcBindingVectorFree(
    rpc_binding_vector_p_t* ppServerBindings
);

#ifndef _WIN32

/* signal.c */

VOID
VmKdcBlockSelectedSignals(
    VOID
    );

DWORD
VmKdcHandleSignals(
    VOID
    );

DWORD
VmKdcInitSignalThread(
    PVMKDC_GLOBALS pGlobals
    );

#endif /* ifndef _WIN32 */

VOID
VmKdcRpcFreeMemory(
    PVOID pMemory
    );

/* parseargs.c */

DWORD
VmKdcParseArgs(
    int argc,
    char* argv[],
    int* pLoggingLevel,
    PBOOLEAN pbEnableSysLog,
    PBOOLEAN pbEnableConsole
);

VOID
ShowUsage(
    PSTR pName
);

/* utils.c */

VOID
VmKdcdStateSet(
    VMKDC_SERVER_STATE   state
);

VMKDC_SERVER_STATE
VmKdcdState(
    VOID
);

#ifdef _WIN32
DWORD
VmKdcGetMasterKeyStashFile(
    _TCHAR *lpMasterKeyStashFile
);
#endif


#ifdef __cplusplus
}
#endif
