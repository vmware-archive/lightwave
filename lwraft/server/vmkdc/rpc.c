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

static
PVOID
VmKdcListenRpcServer(
    PVOID pInfo
    );

static
BOOLEAN
VmKdcRpcCheckServerIsActive(
    VOID
    );

static
DWORD
VmKdcStopRpcServer(
    VOID
    );

static
VOID
VmKdcRpcIfCallbackFn(
    rpc_if_handle_t InterfaceUuid,
    PVOID           Context,
    unsigned32*     status
    );

DWORD
VmKdcRpcServerStartListen(
    VOID
    )
{
    DWORD dwError = ERROR_SUCCESS;
    int status = 0;

    status = dcethread_create(
                            &gVmkdcGlobals.pRPCServerThread,
                            NULL,
                            VmKdcListenRpcServer,
                            NULL);

#ifndef _WIN32
    dwError = LwErrnoToWin32Error(status);
#else
    dwError = status;
#endif
    BAIL_ON_VMKDC_ERROR(dwError);

    while (!VmKdcRpcCheckServerIsActive())
    {
        // Wait for RPC Server to come up.
        VmKdcSleep(1000);
    }

error:

    return dwError;
}

DWORD
VmKdcRpcServerStopListen(
    VOID
    )
{
    DWORD dwError = ERROR_SUCCESS;
    int status = 0;

    dwError = VmKdcStopRpcServer();
    BAIL_ON_VMKDC_ERROR(dwError);

    if (gVmkdcGlobals.pRPCServerThread)
    {
        status = dcethread_interrupt(
                        gVmkdcGlobals.pRPCServerThread);

#ifndef _WIN32
        dwError = LwErrnoToWin32Error(status);
#else
        dwError = status;
#endif
        BAIL_ON_VMKDC_ERROR(dwError);

        status = dcethread_join(
                        gVmkdcGlobals.pRPCServerThread,
                        NULL);

#ifndef _WIN32
        dwError = LwErrnoToWin32Error(status);
#else
        dwError = status;
#endif
        BAIL_ON_VMKDC_ERROR(dwError);

        gVmkdcGlobals.pRPCServerThread = NULL;
    }

error:

    return dwError;
}

DWORD
VmKdcRpcServerRegisterIf(
    rpc_if_handle_t pInterfaceSpec
    )
{
    DWORD dwError = ERROR_SUCCESS;
    error_status_t rpcStatus = rpc_s_ok;

    rpc_server_register_if_ex(
            pInterfaceSpec,
            NULL,
            NULL,
            rpc_if_allow_secure_only,
            rpc_c_listen_max_calls_default,
            VmKdcRpcIfCallbackFn,
            &rpcStatus);

    dwError = rpcStatus;

    return dwError;
}

DWORD
VmKdcRpcServerUseProtSeq(
    PCSTR pszProtSeq
    )
{
    DWORD dwError = ERROR_SUCCESS;
    error_status_t rpcStatus = rpc_s_ok;

    rpc_server_use_protseq(
            (unsigned char*) pszProtSeq,
            rpc_c_protseq_max_calls_default,
            (unsigned32*)&rpcStatus);

    dwError = rpcStatus;

    return dwError;
}

DWORD
VmKdcRpcServerUseProtSeqEp(
    PCSTR pszProtSeq,
    PCSTR pszEndpoint
    )
{
    DWORD dwError = ERROR_SUCCESS;
    error_status_t rpcStatus = rpc_s_ok;

    rpc_server_use_protseq_ep(
            (unsigned char*) pszProtSeq,
            rpc_c_protseq_max_calls_default,
            (unsigned char*) pszEndpoint,
            (unsigned32*)&dwError);

    dwError = rpcStatus;

    return dwError;
}

DWORD
VmKdcRpcServerInqBindings(
    rpc_binding_vector_p_t* ppServerBindings
    )
{
    DWORD dwError = ERROR_SUCCESS;
    error_status_t rpcStatus = rpc_s_ok;

    rpc_server_inq_bindings(
            ppServerBindings,
            (unsigned32*)&rpcStatus);

    dwError = rpcStatus;

     return dwError;
}

DWORD
VmKdcRpcEpRegister(
    rpc_binding_vector_p_t pServerBinding,
    rpc_if_handle_t        pInterfaceSpec,
    PCSTR                  pszAnnotation
    )
{
    DWORD dwError = ERROR_SUCCESS;
    error_status_t rpcStatus = rpc_s_ok;

    rpc_ep_register(
            pInterfaceSpec,
            pServerBinding,
            NULL,
            (idl_char*)pszAnnotation,
            (unsigned32*)&rpcStatus);

    dwError = rpcStatus;

    return dwError;
}

DWORD
VmKdcRpcServerRegisterAuthInfo(
    VOID
    )
{
    DWORD dwError = ERROR_SUCCESS;
    error_status_t rpcStatus = rpc_s_ok;

    rpc_server_register_auth_info (
            NULL, // Server principal name
            rpc_c_authn_gss_negotiate, // Authentication service
            NULL, // Use default key function
            NULL,
            &rpcStatus);

    dwError = rpcStatus;

    return dwError;
}

DWORD
VmKdcRpcBindingInqAuthClient(
    rpc_binding_handle_t hClientBinding,
    rpc_authz_handle_t* pPrivs,
    PSTR* ppServerPrincName,
    DWORD* pAuthnLevel,
    DWORD* pAuthnSvc,
    DWORD* pAuthzSvc
    )
{
    DWORD dwError = ERROR_SUCCESS;
    error_status_t rpcStatus = rpc_s_ok;

    rpc_binding_inq_auth_client(
            hClientBinding,
            pPrivs, // The data referenced by this parameter is read-only,
                    // and therefore should not be modified/freed.
            (unsigned_char_p_t*)ppServerPrincName,
            pAuthnLevel,
            pAuthnSvc,
            pAuthzSvc,
            &rpcStatus);

    dwError = rpcStatus;

    return dwError;
}

DWORD
VmKdcRpcBindingVectorFree(
    rpc_binding_vector_p_t* ppServerBindings
    )
{
    DWORD dwError = ERROR_SUCCESS;
    error_status_t rpcStatus = rpc_s_ok;

    if ( (ppServerBindings == NULL) || ((*ppServerBindings) == NULL) )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    rpc_binding_vector_free(
            ppServerBindings,
            &rpcStatus);

    dwError = rpcStatus;
    BAIL_ON_VMKDC_ERROR(dwError);

error:

    return dwError;
}

static
PVOID
VmKdcListenRpcServer(
    PVOID pInfo
    )
{
    error_status_t rpcStatus = rpc_s_ok;

    rpc_server_listen(
        rpc_c_listen_max_calls_default,
        (unsigned32*)&rpcStatus);

    raise(SIGTERM); // indicate that process must terminate

    return NULL;
}

static
BOOLEAN
VmKdcRpcCheckServerIsActive(
    VOID
    )
{
    BOOLEAN bIsActive = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    error_status_t rpcStatus = rpc_s_ok;

    bIsActive = rpc_mgmt_is_server_listening(
                        NULL,
                        (unsigned32*)&rpcStatus);

    dwError = rpcStatus;
    BAIL_ON_VMKDC_ERROR(dwError);

cleanup:

    return bIsActive;

error:

    bIsActive = FALSE;

    goto cleanup;
}

static
DWORD
VmKdcStopRpcServer(
    VOID
    )
{
    DWORD dwError = ERROR_SUCCESS;
    error_status_t rpcStatus = rpc_s_ok;

    rpc_mgmt_stop_server_listening(
            NULL,
            (unsigned32*)&rpcStatus);

    dwError = rpcStatus;

    return dwError;
}

static
VOID
VmKdcRpcIfCallbackFn(
    rpc_if_handle_t InterfaceUuid,
    PVOID           Context,
    unsigned32*     status
    )
{
    unsigned32 sts = 0;
    *status = sts;
}
