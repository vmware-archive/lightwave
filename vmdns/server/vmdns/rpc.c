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
 * Module   : service.c
 *
 * Abstract :
 *
 *            VMware dns Service
 *
 *            Service
 *
 *            RPC common functions
 *
 */

#include "includes.h"

static
PVOID
VmDnsListenRpcServer(
    PVOID pInfo
    );

static
BOOLEAN
VmDnsRpcCheckServerIsActive(
    VOID
    );

static
VOID
VmDnsRpcIfCallbackFn(
    rpc_if_handle_t InterfaceUuid,
    PVOID         Context,
    unsigned32*   status
);

DWORD
VmDnsRpcServerStartListen(
    VOID
    )
{
    DWORD dwError = 0;
    int status = 0;

    status = dcethread_create(
                            &gVmdnsGlobals.pRPCServerThread,
                            NULL,
                            VmDnsListenRpcServer,
                            NULL);
#ifndef _WIN32
    dwError = LwErrnoToWin32Error(status);
#else
    dwError = status;
#endif

    BAIL_ON_VMDNS_ERROR(dwError);

    while (!VmDnsRpcCheckServerIsActive())
    {
        // Wait for RPC Server to come up.
    }

error:

    return dwError;
}

DWORD
VmDnsRpcServerStopListen(
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
        if(!dwError)
        {
            dwError = RPC_S_INTERNAL_ERROR;
        }
    }
    DCETHREAD_ENDTRY;

    BAIL_ON_VMDNS_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmDnsRpcServerRegisterIf(
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
               VmDnsRpcIfCallbackFn,
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
VmDnsRpcServerUseProtSeq(
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

    BAIL_ON_VMDNS_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmDnsRpcServerUseProtSeqEp(
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

    BAIL_ON_VMDNS_ERROR(dwError);
error:
    return dwError;
}

DWORD
VmDnsRpcServerInqBindings(
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

    BAIL_ON_VMDNS_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmDnsRpcEpRegister(
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
VmDnsRpcServerRegisterAuthInfo(
    VOID
    )
{
    DWORD dwError = 0;

    DCETHREAD_TRY
    {
       rpc_server_register_auth_info (
                  NULL, // Server principal name
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

    return dwError;
}

DWORD
VmDnsRpcBindingInqAuthClient(
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

    BAIL_ON_VMDNS_ERROR(rpcStatus);

error:

    return rpcStatus;
}

DWORD
VmDnsRpcBindingVectorFree(
    rpc_binding_vector_p_t* ppServerBindings
    )
{
    error_status_t rpcStatus = rpc_s_ok;

    if ( (ppServerBindings != NULL) && ((*ppServerBindings) != NULL) )
    {
        rpc_binding_vector_free(ppServerBindings, &rpcStatus);
    }
    BAIL_ON_VMDNS_ERROR(rpcStatus);

error:

    return rpcStatus;
}

static
PVOID
VmDnsListenRpcServer(
    PVOID pInfo
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
        if(!dwError)
        {
            dwError = RPC_S_INTERNAL_ERROR;
        }
    }
    DCETHREAD_ENDTRY;

    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

#ifndef _WIN32
    raise(SIGTERM); // indicate that process must terminate
#endif

    return NULL;

error:

    goto cleanup;
}

static
BOOLEAN
VmDnsRpcCheckServerIsActive(
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

    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return bIsActive;

error:

    bIsActive = FALSE;

    goto cleanup;
}

static
VOID
VmDnsRpcIfCallbackFn(
    rpc_if_handle_t InterfaceUuid,
    PVOID           Context,
    unsigned32*     status
    )
{
    unsigned32 sts = 0;
    *status = sts;
}
