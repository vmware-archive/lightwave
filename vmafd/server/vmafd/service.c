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
DWORD
VmAfdRegisterRpcServer(
    VOID
    );

static
DWORD
VmAfdBindServer(
    rpc_binding_vector_p_t * server_binding,
    PVMAFD_RPC_ENDPOINT      pEndPoints,
    ULONG                    ulCount
    );

DWORD
VmAfdRpcServerInit(
    VOID
    )
{
    DWORD dwError = 0;
    int iCnt = 0;
    BOOLEAN bRPCReady = FALSE;

    dwError = EventLogInitialize();
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRegisterRpcServer();
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRpcServerStartListen();
    BAIL_ON_VMAFD_ERROR(dwError);

    ////////////////////////////////////////////////////////////////////////
    // This is a hack. The right solution should be in RPC runtime, where it
    // does not return until it is listening on the RPC port.
    // Wait (max 75 seconds) until RPC port is ready before continue.
    ////////////////////////////////////////////////////////////////////////
    for (iCnt = 1; iCnt < 76; iCnt++)
    {
        VMAFD_STATUS afdStatus = 0;
        DWORD dwLocalError = 0;

        dwLocalError = VmAfdGetStatusRPC(NULL, &afdStatus);
        if (dwLocalError == 0)
        {
            bRPCReady = TRUE;
            break;
        }

        VmAfdSleep(1000); // sleep one second

        if ( (iCnt % 10) == 0 )
        {
            VmAfdLog(VMAFD_DEBUG_ANY,
                     "waiting for RPC service (%d) seconds passed", iCnt);
        }
    }

cleanup:

    VmAfdLog(VMAFD_DEBUG_ANY, "RPC service status (%s)",
             bRPCReady ? "listening" : "not yet listening");

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfdRpcServerInit failed (%u)", dwError);

    goto cleanup;
}

VOID
VmAfdRpcServerShutdown(
    VOID
    )
{
    DWORD dwError = 0;

    dwError = VmAfdRpcServerStopListen();
    BAIL_ON_VMAFD_ERROR(dwError);

error:

    return;
}

static
DWORD
VmAfdRegisterRpcServer(
    VOID
    )
{
    DWORD dwError = 0;
    VMAFD_RPC_ENDPOINT endpoints[] =
        {
            {"ncalrpc", VMAFD_NCALRPC_END_POINT},
            {"ncacn_ip_tcp", VMAFD_RPC_TCP_END_POINT},
        };
    DWORD dwEpCount = sizeof(endpoints)/sizeof(endpoints[0]);
    rpc_if_handle_t pInterfaceSpec65 = vmafd_v1_5_s_ifspec;
    rpc_if_handle_t pInterfaceSpec60 = vmafd_v1_4_s_ifspec;
    rpc_if_handle_t pInterfaceSpecSL = vmafdsuperlog_v1_0_s_ifspec;

    rpc_binding_vector_p_t pServerBinding = NULL;

    dwError = VmAfdRpcServerRegisterIf(pInterfaceSpec65);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRpcServerRegisterIf(pInterfaceSpec60);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRpcServerRegisterIf(pInterfaceSpecSL);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = EventLogRegisterRpcServerIf();
    BAIL_ON_VMAFD_ERROR(dwError);

    VmAfdLog(VMAFD_DEBUG_TRACE,
        "VMware afd Service registered successfully.");

    dwError = VmAfdBindServer(
                  &pServerBinding,
                  endpoints,
                  dwEpCount);
    BAIL_ON_VMAFD_ERROR(dwError);

    VmAfdLog(VMAFD_DEBUG_TRACE, "VMware afd Service bound successfully.");

#if !defined(_WIN32) && !defined(PLATFORM_VMWARE_ESX)
    dwError = EventLogEpRegister(pServerBinding);
    BAIL_ON_VMAFD_ERROR(dwError);
#endif

    dwError = VmAfdRpcServerRegisterAuthInfo();
    BAIL_ON_VMAFD_ERROR(dwError);

error:

    if( pServerBinding != NULL )
    {
        VmAfdRpcBindingVectorFree( &pServerBinding );
    }

    return dwError;
}

static
DWORD
VmAfdBindServer(
    rpc_binding_vector_p_t * server_binding,
    PVMAFD_RPC_ENDPOINT      pEndPoints,
    ULONG                   ulCount
    )
{
    DWORD dwError = 0;
    ULONG iEP = 0;
    int iCnt = 0;

    /*
     * Prepare the server binding handle
     * use all avail protocols (UDP and TCP). This basically allocates
     * new sockets for us and associates the interface UUID and
     * object UUID of with those communications endpoints.
     */
    for (iEP = 0; iEP < ulCount; iEP++)
    {
        for (iCnt = 0; iCnt < 10; iCnt++)
        {
            if (!pEndPoints[iEP].pszEndPointName)
            {
                dwError = VmAfdRpcServerUseProtSeq(
                    pEndPoints[iEP].pszEndPointType
                );
            }
            else
            {
                dwError = VmAfdRpcServerUseProtSeqEp(
                    pEndPoints[iEP].pszEndPointType,
                    pEndPoints[iEP].pszEndPointName
                );
            }
            if (dwError == 0)
            {
                break;
            }
            else if (dwError != rpc_s_no_addrs)
            {
                BAIL_ON_VMAFD_ERROR(dwError);
            }

            /*
             * wait a second and retry if the error is rpc_s_no_addrs
             */
            VmAfdLog(VMAFD_DEBUG_ANY,
                     "VmAfdRpcServerUseProtSeqEp failed, retrying");
            VmAfdSleep(1000);
        }
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdRpcServerInqBindings( server_binding );
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY,
             "VmAfdBindServer failed. Error(%u)",
             dwError);

    goto cleanup;
}

DWORD
VmAfdRpcAuthCallback(
    PVOID Context
)
{
    rpc_authz_handle_t hPrivs;
    DWORD dwAuthnLevel = 0;
    DWORD dwAuthnSvc = 0;
    DWORD dwAuthzSvc = 0;
    DWORD rpcStatus = rpc_s_ok;

    rpcStatus = VmAfdRpcBindingInqAuthClient(
            Context,
            &hPrivs, // The data referenced by this parameter is read-only,
                     // and therefore should not be modified/freed.
            NULL,    // ServerPrincipalName - we don't need it
            &dwAuthnLevel,
            &dwAuthnSvc,
            &dwAuthzSvc);
    BAIL_ON_VMAFD_ERROR(rpcStatus);

    VmAfdLog(
            VMAFD_DEBUG_ANY,
            "Authentication Level = %d, Authentication Service = %d,"
            "Authorization Service = %d.",
            dwAuthnLevel,
            dwAuthnSvc,
            dwAuthzSvc);

    // Now check the authentication level. We require at least packet-level
    // authentication.
    if (dwAuthnLevel < rpc_c_authn_level_pkt)
    {
        VmAfdLog(VMAFD_DEBUG_ANY,
            "Attempt by client to use weak authentication.");

        rpcStatus = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR(rpcStatus);
    }

error:

    return rpcStatus;
}
