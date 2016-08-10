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
 *            Worker Thread Management
 */

#include "includes.h"

static
DWORD
VmDnsRegisterRpcServer(
    VOID
    );

static
DWORD
VmDnsBindServer(
    rpc_binding_vector_p_t * server_binding,
    PVMDNS_RPC_ENDPOINT pEndPoints,
    ULONG             ulCount
    );

DWORD
VmDnsRpcServerInit(
    VOID
    )
{
    DWORD dwError = 0;

    dwError  = VmDnsRegisterRpcServer();
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRpcServerStartListen();
    BAIL_ON_VMDNS_ERROR(dwError);

error:

    return dwError;
}

VOID
VmDnsRpcServerShutdown(
    VOID
    )
{
    DWORD dwError = 0;

    dwError = VmDnsRpcServerStopListen();
    BAIL_ON_VMDNS_ERROR(dwError);

error:

    return;
}

static
DWORD
VmDnsRegisterRpcServer(
    VOID
    )
{
    DWORD dwError = 0;
    VMDNS_RPC_ENDPOINT endpoints[] =
    {
        { "ncalrpc", VMDNS_NCALRPC_END_POINT },
        { "ncacn_ip_tcp", VMDNS_RPC_TCP_END_POINT },
    };
    DWORD dwEpCount = sizeof(endpoints) / sizeof(endpoints[0]);
    rpc_if_handle_t pInterfaceSpec = vmdns_v1_0_s_ifspec;
    rpc_binding_vector_p_t pServerBinding = NULL;

    dwError = VmDnsRpcServerRegisterIf(pInterfaceSpec);
    BAIL_ON_VMDNS_ERROR(dwError);

    VmDnsLog(VMDNS_LOG_LEVEL_INFO,
        "VMware dns Service registered successfully.");

    dwError = VmDnsBindServer(
                        &pServerBinding,
                        endpoints,
                        dwEpCount);
    BAIL_ON_VMDNS_ERROR(dwError);

    VmDnsLog(VMDNS_LOG_LEVEL_INFO, "VMware dns Service bound successfully.");

#ifndef _WIN32
    dwError = VmDnsRpcEpRegister(
                        pServerBinding,
                        pInterfaceSpec,
                        "VMware dns Service"
                        );
    if (dwError == 0)
    {
        VmDnsLog(VMDNS_LOG_LEVEL_INFO, "RPC Endpoint registered successfully.");
    }
#endif

#ifndef _WIN32
/*
 * This does not work yet with DCERPC-WIN32 for release builds
 */

    dwError = VmDnsRpcServerRegisterAuthInfo();
    BAIL_ON_VMDNS_ERROR(dwError);
#endif

error:

    if( pServerBinding != NULL )
    {
        VmDnsRpcBindingVectorFree( &pServerBinding );
    }

    return dwError;
}

static
DWORD
VmDnsBindServer(
    rpc_binding_vector_p_t * server_binding,
    PVMDNS_RPC_ENDPOINT pEndPoints,
    ULONG             ulCount
    )
{
    DWORD dwError = 0;
    DWORD iEP;

    /*
     * Prepare the server binding handle
     * use all avail protocols (UDP and TCP). This basically allocates
     * new sockets for us and associates the interface UUID and
     * object UUID of with those communications endpoints.
     */
    for (iEP = 0; iEP < ulCount; iEP++)
    {
        if (!pEndPoints[iEP].pszEndPointName)
        {
            dwError = VmDnsRpcServerUseProtSeq(
                pEndPoints[iEP].pszEndPointType
            );
            BAIL_ON_VMDNS_ERROR(dwError);
        }
        else
        {
            dwError = VmDnsRpcServerUseProtSeqEp(
                pEndPoints[iEP].pszEndPointType,
                pEndPoints[iEP].pszEndPointName
            );
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }

    dwError = VmDnsRpcServerInqBindings( server_binding );
    BAIL_ON_VMDNS_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmDnsRpcAuthCallback(
    PVOID         Context
    )
{
    rpc_authz_handle_t hPrivs;
    DWORD dwAuthnLevel = 0;
    DWORD dwAuthnSvc = 0;
    DWORD dwAuthzSvc = 0;
    DWORD rpcStatus = rpc_s_ok;

    rpcStatus = VmDnsRpcBindingInqAuthClient(
            Context,
            &hPrivs, // The data referenced by this parameter is read-only,
                     // and therefore should not be modified/freed.
            NULL,    // ServerPrincipalName - we don't need it
            &dwAuthnLevel,
            &dwAuthnSvc,
            &dwAuthzSvc);
    BAIL_ON_VMDNS_ERROR(rpcStatus);

    VmDnsLog(
            VMDNS_LOG_LEVEL_INFO,
            "Authentication Level = %d, Authentication Service = %d,"
            "Authorization Service = %d.",
            dwAuthnLevel,
            dwAuthnSvc,
            dwAuthzSvc);

    // Now check the authentication level. We require at least packet-level
    // authentication.
    if (dwAuthnLevel < rpc_c_authn_level_pkt)
    {
        VmDnsLog(VMDNS_LOG_LEVEL_INFO,
            "Attempt by client to use weak authentication.");

        rpcStatus = ERROR_ACCESS_DENIED;
        BAIL_ON_VMDNS_ERROR(rpcStatus);
    }

error:

    return rpcStatus;
}
