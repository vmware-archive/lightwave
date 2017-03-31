/*
 * Copyright © 2012-2017 VMware, Inc.  All Rights Reserved.
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

static
RPC_STATUS CALLBACK VmDirRpcIfCallbackFn(
  RPC_IF_HANDLE Interface,
  PVOID Context
);

ULONG
VmDirRpcServerStartListen(
    VOID
)
{
    DWORD dwError = 0;
    unsigned int    cMinCalls = 1;
    unsigned int    fDontWait = TRUE;

    dwError = RpcServerListen(
        cMinCalls, RPC_C_LISTEN_MAX_CALLS_DEFAULT, fDontWait
    );
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    return dwError;
}

ULONG
VmDirRpcServerStopListen(
    VOID
)
{
    ULONG dwError = 0;

    /*
    MSDN:  If the server is not listening, the function fails.
    */
    dwError = RpcMgmtStopServerListening(NULL);
    if(dwError == NO_ERROR)
    {
        // if we successfully called RpcMgmtStopServerListening
        // wait for rpc calls to complete....
        dwError = RpcMgmtWaitServerListen();
    }

    return dwError;
}

ULONG
VmDirRpcServerRegisterIf(
    VMDIR_IF_HANDLE_T pInterfaceSpec
)
{
    DWORD dwError = 0;

    dwError = RpcServerRegisterIfEx(
               pInterfaceSpec,
               NULL,
               NULL,
               RPC_IF_ALLOW_SECURE_ONLY,
               RPC_C_LISTEN_MAX_CALLS_DEFAULT,
               VmDirRpcIfCallbackFn
    );

    BAIL_ON_VMDIR_ERROR(dwError);

error:

    return dwError;
}

ULONG
VmDirRpcServerUseProtSeq(
    PCSTR pszProtSeq
)
{
    DWORD dwError = 0;

    dwError = RpcServerUseProtseqA(
                (unsigned char*)pszProtSeq,
                RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                NULL
    );
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    return dwError;
}

ULONG
VmDirRpcServerUseProtSeqEp(
    PCSTR pszProtSeq,
    PCSTR pszEndpoint
)
{
    DWORD dwError = 0;

    dwError = RpcServerUseProtseqEpA(
                (unsigned char*) pszProtSeq,
                RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                (unsigned char*) pszEndpoint,
                NULL
    );
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    return dwError;
}

ULONG
VmDirRpcServerInqBindings(
    VMDIR_RPC_BINDING_VECTOR_P_T* ppServerBindings
)
{
    ULONG dwError = 0;

    dwError = RpcServerInqBindings( ppServerBindings );
    BAIL_ON_VMDIR_ERROR(dwError);

error:

     return dwError;
}

ULONG
VmDirRpcEpRegister(
    VMDIR_RPC_BINDING_VECTOR_P_T pServerBinding,
    VMDIR_IF_HANDLE_T            pInterfaceSpec,
    PCSTR                        pszAnnotation
)
{
    DWORD dwError = 0;

    dwError = RpcEpRegisterA(
        pInterfaceSpec,
        pServerBinding,
        NULL,
        (RPC_CSTR)pszAnnotation
    );
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    return dwError;
}

ULONG
VmDirRpcServerRegisterAuthInfo(
    VOID
)
{
    DWORD dwError = 0;

    dwError = RpcServerRegisterAuthInfoA(
        NULL, // Server principal name
        RPC_C_AUTHN_GSS_NEGOTIATE, // Authentication service
        NULL, // Use default key function
        NULL
    );
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    return dwError;
}

ULONG
VmDirRpcBindingInqAuthClient(
    VMDIR_RPC_BINDING_HANDLE hClientBinding,
    VMDIR_RPC_AUTHZ_HANDLE* pPrivs,
    PSTR* ppServerPrincName,
    DWORD* pAuthnLevel,
    DWORD* pAuthnSvc,
    DWORD* pAuthzSvc
)
{
    DWORD dwError = 0;

    dwError = RpcBindingInqAuthClientA(
        hClientBinding,
        pPrivs,
        (RPC_CSTR*)ppServerPrincName,
        pAuthnLevel,
        pAuthnSvc,
        pAuthzSvc
    );
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    return dwError;
}

ULONG
VmDirRpcBindingVectorFree(
    VMDIR_RPC_BINDING_VECTOR_P_T* ppServerBindings
)
{
    DWORD dwError = 0;

    if ( ( ppServerBindings != NULL ) && ( (*ppServerBindings) != NULL ) )
    {
        dwError = RpcBindingVectorFree(ppServerBindings);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

error:

    return dwError;
}

static
RPC_STATUS CALLBACK VmDirRpcIfCallbackFn(
  RPC_IF_HANDLE Interface,
  PVOID Context
)
{
    return VmDirRpcAuthCallback(Context);
}
