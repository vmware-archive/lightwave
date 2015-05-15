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
BOOLEAN
VmAuthsvcRpcCheckServerIsActive(
    VOID
    );

static
VOID
VmAuthsvcRpcIfCallbackFn(
    rpc_if_handle_t InterfaceUuid,
    PVOID           Context,
    unsigned32*     status
    );

static
PVOID
VmAuthsvcRpcListen(
    PVOID pData
    );

DWORD
VmAuthsvcRpcServerStartListen(
    VOID
    )
{
    DWORD dwError = 0;
    int status = 0;

    status = dcethread_create(
                            &gVmauthsvcGlobals.pRPCServerThread,
                            NULL,
                            VmAuthsvcRpcListen,
                            NULL);
#ifndef _WIN32
    dwError = LwErrnoToWin32Error(status);
#else
    dwError = status;
#endif

    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    while (!VmAuthsvcRpcCheckServerIsActive())
    {
        // Wait for RPC Server to come up.
        VmAuthsvcSleep(1000);
    }

error:

    return dwError;
}

DWORD
VmAuthsvcRpcServerStopListen(
    VOID
    )
{
    DWORD dwError = 0;

    DCETHREAD_TRY
    {
        rpc_mgmt_stop_server_listening(NULL, (unsigned32*)&dwError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        if (!dwError)
        {
            dwError = dcethread_exc_getstatus(THIS_CATCH);
        }
        if (!dwError)
        {
            dwError = RPC_S_INTERNAL_ERROR;
        }
    }
    DCETHREAD_ENDTRY;

    BAIL_ON_VMAUTHSVC_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmAuthsvcRegisterRpcServerIf(
    VOID
    )
{
    DWORD dwError = 0;
    rpc_if_handle_t pInterfaceSpec = vmauthsvc_v1_0_s_ifspec;

    dwError = VmAuthsvcRpcServerRegisterIf(pInterfaceSpec);

    return dwError;
}

DWORD
VmAuthsvcRpcServerRegisterIf(
    rpc_if_handle_t pInterfaceSpec
    )
{
    DWORD dwError = 0;

    DCETHREAD_TRY
    {
       rpc_server_register_if_ex(
               pInterfaceSpec,
               NULL,
               NULL,
               rpc_if_allow_secure_only,
               rpc_c_listen_max_calls_default,
               VmAuthsvcRpcIfCallbackFn,
               &dwError
               );
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
       if ( dwError == rpc_s_ok )
       {
           dwError = dcethread_exc_getstatus(THIS_CATCH);
          if (!dwError)
          {
              dwError = RPC_S_INTERNAL_ERROR;
          }
       }
    }
    DCETHREAD_ENDTRY;

    return dwError;
}

DWORD
VmAuthsvcRpcServerUseProtSeq(
    PCSTR pszProtSeq
    )
{
    DWORD dwError = 0;

    DCETHREAD_TRY
    {
        rpc_server_use_protseq(
                (unsigned char*) pszProtSeq,
                rpc_c_protseq_max_calls_default,
                (unsigned32*)&dwError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        if ( dwError == rpc_s_ok )
        {
            dwError = dcethread_exc_getstatus(THIS_CATCH);
            if (!dwError)
            {
                dwError = RPC_S_INTERNAL_ERROR;
            }
        }
    }
    DCETHREAD_ENDTRY;

    BAIL_ON_VMAUTHSVC_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmAuthsvcRpcServerUseProtSeqEp(
    PCSTR pszProtSeq,
    PCSTR pszEndpoint
    )
{
    DWORD dwError = 0;

    DCETHREAD_TRY
    {
        rpc_server_use_protseq_ep(
                (unsigned char*) pszProtSeq,
                rpc_c_protseq_max_calls_default,
                (unsigned char*) pszEndpoint,
                (unsigned32*)&dwError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        if ( dwError == rpc_s_ok )
        {
            dwError = dcethread_exc_getstatus(THIS_CATCH);
            if (!dwError)
            {
                dwError = RPC_S_INTERNAL_ERROR;
            }
        }
    }
    DCETHREAD_ENDTRY;

    BAIL_ON_VMAUTHSVC_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmAuthsvcRpcServerInqBindings(
    rpc_binding_vector_p_t* ppServerBindings
    )
{
    DWORD dwError = 0;

    DCETHREAD_TRY
    {
        rpc_server_inq_bindings(ppServerBindings, (unsigned32*)&dwError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        if ( dwError == rpc_s_ok )
        {
            dwError = dcethread_exc_getstatus(THIS_CATCH);
            if (!dwError)
            {
                dwError = RPC_S_INTERNAL_ERROR;
            }
        }
    }
    DCETHREAD_ENDTRY;

    BAIL_ON_VMAUTHSVC_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmAuthsvcEpRegister(
    rpc_binding_vector_p_t pServerBinding
    )
{
    DWORD dwError = 0;
    rpc_if_handle_t pInterfaceSpec = vmauthsvc_v1_0_s_ifspec;
    PCSTR pszAnnotation = "VmAuthsvc Service";

    dwError = VmAuthsvcRpcEpRegister(
                      pServerBinding,
                      pInterfaceSpec,
                      pszAnnotation);

    return dwError;
}

DWORD
VmAuthsvcRpcEpRegister(
    rpc_binding_vector_p_t pServerBinding,
    rpc_if_handle_t        pInterfaceSpec,
    PCSTR                  pszAnnotation
    )
{
    DWORD dwError = 0;

    DCETHREAD_TRY
    {
       rpc_ep_register(
                pInterfaceSpec,
                pServerBinding,
                NULL,
                (idl_char*)pszAnnotation,
                (unsigned32*)&dwError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        if (dwError == rpc_s_ok )
        {
            dwError = dcethread_exc_getstatus(THIS_CATCH);
            if (!dwError)
            {
               dwError = RPC_S_INTERNAL_ERROR;
            }
        }
    }
    DCETHREAD_ENDTRY;

    return dwError;
}

DWORD
VmAuthsvcRpcServerRegisterAuthInfo(
    VOID
    )
{
    DWORD dwError = 0;
    PSTR pszHostName = NULL;
    PSTR pszCanonicalHostName = NULL;
    PSTR pszServerPrincipalName = NULL;

    dwError = VmAuthsvcGetHostName(&pszHostName);
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    dwError = VmAuthsvcGetCanonicalHostName(
                      pszHostName,
                      &pszCanonicalHostName);
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    dwError = VmAuthsvcAllocateStringPrintf(
                      &pszServerPrincipalName,
                      "host/%s",
                      pszCanonicalHostName);
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    DCETHREAD_TRY
    {
       rpc_server_register_auth_info (
                  pszServerPrincipalName, // Server principal name
                  rpc_c_authn_gss_negotiate, // Authentication service
                  NULL, // Use default key function
                  NULL,
                  &dwError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
       if (dwError == rpc_s_ok )
       {
            dwError = dcethread_exc_getstatus(THIS_CATCH);
            if (!dwError)
            {
              dwError = RPC_S_INTERNAL_ERROR;
            }
       }
    }
    DCETHREAD_ENDTRY;

error:

    VMAUTHSVC_SAFE_FREE_STRINGA(pszHostName);
    VMAUTHSVC_SAFE_FREE_STRINGA(pszCanonicalHostName);
    VMAUTHSVC_SAFE_FREE_STRINGA(pszServerPrincipalName);

    return dwError;
}

DWORD
VmAuthsvcRpcBindingInqAuthClient(
    rpc_binding_handle_t hClientBinding,
    rpc_authz_handle_t* pPrivs,
    PSTR* ppServerPrincName,
    DWORD* pAuthnLevel,
    DWORD* pAuthnSvc,
    DWORD* pAuthzSvc
    )
{
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

    BAIL_ON_VMAUTHSVC_ERROR(rpcStatus);

error:

    return rpcStatus;
}

DWORD
VmAuthsvcRpcBindingVectorFree(
    rpc_binding_vector_p_t* ppServerBindings
    )
{
    error_status_t rpcStatus = rpc_s_ok;

    if ( (ppServerBindings != NULL) && ((*ppServerBindings) != NULL) )
    {
        rpc_binding_vector_free(ppServerBindings, &rpcStatus);
    }
    BAIL_ON_VMAUTHSVC_ERROR(rpcStatus);

error:

    return rpcStatus;
}

static
PVOID
VmAuthsvcRpcListen(
    PVOID pData
    )
{
    DWORD dwError = 0;

    DCETHREAD_TRY
    {
        rpc_server_listen(
            rpc_c_listen_max_calls_default, (unsigned32*)&dwError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        if (!dwError)
        {
            dwError = dcethread_exc_getstatus(THIS_CATCH);
        }
        if (!dwError)
        {
            dwError = RPC_S_INTERNAL_ERROR;
        }
    }
    DCETHREAD_ENDTRY;

    BAIL_ON_VMAUTHSVC_ERROR(dwError);

cleanup:

    raise(SIGTERM); // indicate that process must terminate

    return NULL;

error:

    goto cleanup;
}

static
BOOLEAN
VmAuthsvcRpcCheckServerIsActive(
    VOID
    )
{
    DWORD dwError = 0;
    BOOLEAN bIsActive = FALSE;

    DCETHREAD_TRY
    {
        bIsActive = rpc_mgmt_is_server_listening(NULL, (unsigned32*)&dwError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        if (!dwError)
        {
            dwError = dcethread_exc_getstatus(THIS_CATCH);
        }
        if (!dwError)
        {
            dwError = RPC_S_INTERNAL_ERROR;
        }
    }
    DCETHREAD_ENDTRY;

    BAIL_ON_VMAUTHSVC_ERROR(dwError);

cleanup:

    return bIsActive;

error:

    bIsActive = FALSE;

    goto cleanup;
}

static
VOID
VmAuthsvcRpcIfCallbackFn(
    rpc_if_handle_t InterfaceUuid,
    PVOID           Context,
    unsigned32*     status
    )
{
    *status = VmAuthsvcRpcAuthCallback(Context);
}
