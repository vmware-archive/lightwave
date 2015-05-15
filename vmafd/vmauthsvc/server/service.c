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
VmAuthsvcRegisterRpcServer(
    VOID
    );

static
DWORD
VmAuthsvcBindServer(
    rpc_binding_vector_p_t * server_binding,
    PVMAUTHSVC_RPC_ENDPOINT pEndPoints,
    ULONG             ulCount
    );

DWORD
VmAuthsvcRpcServerInit(
    VOID
    )
{
    DWORD dwError = 0;

    dwError  = VmAuthsvcRegisterRpcServer();
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    dwError = VmAuthsvcRpcServerStartListen();
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

error:

    return dwError;
}

VOID
VmAuthsvcRpcServerShutdown(
    VOID
    )
{
    DWORD dwError = 0;

    dwError = VmAuthsvcRpcServerStopListen();
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

error:

    return;
}

static
DWORD
VmAuthsvcRegisterRpcServer(
    VOID
    )
{
    DWORD dwError = 0;
    VMAUTHSVC_RPC_ENDPOINT endpoints[] =
        {
            {"ncacn_ip_tcp", VMAUTHSVC_RPC_TCP_END_POINT},
        };
    DWORD dwEpCount = sizeof(endpoints)/sizeof(endpoints[0]);
    rpc_if_handle_t pInterfaceSpec = vmauthsvc_v1_0_s_ifspec;
    rpc_binding_vector_p_t pServerBinding = NULL;

    dwError = VmAuthsvcRpcServerRegisterIf(pInterfaceSpec);
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    VmAuthsvcLog(VMAUTHSVC_DEBUG_TRACE,
        "VMware Authsvc Service registered successfully.");

    dwError = VmAuthsvcBindServer(
        &pServerBinding,
        endpoints,
        dwEpCount
    );
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

    VmAuthsvcLog(VMAUTHSVC_DEBUG_TRACE, "VMware Authsvc Service bound successfully.");

#ifndef _WIN32
    dwError = VmAuthsvcRpcEpRegister(
        pServerBinding,
        pInterfaceSpec,
        "VMware Authsvc Service"
    );
    BAIL_ON_VMAUTHSVC_ERROR(dwError);
#endif

#ifndef _WIN32
/*
 * This does not work yet with DCERPC-WIN32 for release builds
 */
    VmAuthsvcLog(VMAUTHSVC_DEBUG_TRACE, "RPC Endpoints registered successfully.");

    dwError = VmAuthsvcRpcServerRegisterAuthInfo();
    BAIL_ON_VMAUTHSVC_ERROR(dwError);
#endif

error:

    if( pServerBinding != NULL )
    {
        VmAuthsvcRpcBindingVectorFree( &pServerBinding );
    }

    return dwError;
}

static
DWORD
VmAuthsvcBindServer(
    rpc_binding_vector_p_t * server_binding,
    PVMAUTHSVC_RPC_ENDPOINT pEndPoints,
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
            dwError = VmAuthsvcRpcServerUseProtSeq(
                pEndPoints[iEP].pszEndPointType
            );
            BAIL_ON_VMAUTHSVC_ERROR(dwError);
        }
        else
        {
            dwError = VmAuthsvcRpcServerUseProtSeqEp(
                pEndPoints[iEP].pszEndPointType,
                pEndPoints[iEP].pszEndPointName
            );
            BAIL_ON_VMAUTHSVC_ERROR(dwError);
        }
    }

    dwError = VmAuthsvcRpcServerInqBindings( server_binding );
    BAIL_ON_VMAUTHSVC_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmAuthsvcRpcAuthCallback(
    PVOID         Context
    )
{
    rpc_authz_handle_t hPrivs;
    DWORD dwAuthnLevel = 0;
    DWORD dwAuthnSvc = 0;
    DWORD dwAuthzSvc = 0;
    DWORD rpcStatus = rpc_s_ok;

    rpcStatus = VmAuthsvcRpcBindingInqAuthClient(
            Context,
            &hPrivs, // The data referenced by this parameter is read-only,
                     // and therefore should not be modified/freed.
            NULL,    // ServerPrincipalName - we don't need it
            &dwAuthnLevel,
            &dwAuthnSvc,
            &dwAuthzSvc);
    BAIL_ON_VMAUTHSVC_ERROR(rpcStatus);

    VmAuthsvcLog(
            VMAUTHSVC_DEBUG_ANY,
            "Authentication Level = %d, Authentication Service = %d,"
            "Authorization Service = %d.",
            dwAuthnLevel,
            dwAuthnSvc,
            dwAuthzSvc);

    // Now check the authentication level. We require at least packet-level
    // authentication.
    if (dwAuthnLevel < rpc_c_authn_level_pkt_privacy)
    {
        VmAuthsvcLog(VMAUTHSVC_DEBUG_ANY,
            "Attempt by client to use weak authentication level.");

        rpcStatus = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAUTHSVC_ERROR(rpcStatus);
    }

error:

    return rpcStatus;
}
