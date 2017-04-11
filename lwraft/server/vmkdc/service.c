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
VmKdcRegisterRpcServer(
    VOID
    );

static
DWORD
VmKdcBindServer(
    rpc_binding_vector_p_t* server_binding,
    PVMKDC_RPC_ENDPOINT     pEndPoints,
    ULONG                   ulCount
    );

DWORD
VmKdcRpcServerInit(
    VOID
    )
{
    DWORD dwError = 0;

    dwError  = VmKdcRegisterRpcServer();
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcRpcServerStartListen();
    BAIL_ON_VMKDC_ERROR(dwError);

error:

    return dwError;
}

VOID
VmKdcRpcServerShutdown(
    VOID
    )
{
    DWORD dwError = 0;

    dwError = VmKdcRpcServerStopListen();
    BAIL_ON_VMKDC_ERROR(dwError);

error:

    return;
}

static
DWORD
VmKdcRegisterRpcServer(
    VOID
    )
{
    DWORD dwError = 0;
    VMKDC_RPC_ENDPOINT endpoints[] =
        {
#if 0
            {"ncalrpc", VMKDC_NCALRPC_END_POINT},
#endif
            {"ncacn_ip_tcp", VMKDC_RPC_TCP_END_POINT}
        };
    DWORD dwEpCount = sizeof(endpoints)/sizeof(endpoints[0]);
    rpc_if_handle_t pInterfaceSpec = vmkdc_v1_0_s_ifspec;
    rpc_binding_vector_p_t pServerBinding = NULL;

    dwError = VmKdcRpcServerRegisterIf(pInterfaceSpec);
    BAIL_ON_VMKDC_ERROR(dwError);

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL,
        "VMware Kdc Service registered successfully.");

    dwError = VmKdcBindServer(
                      &pServerBinding,
                      endpoints,
                      dwEpCount);
    BAIL_ON_VMKDC_ERROR(dwError);

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL,
        "VMware Kdc Service bound successfully.");

#ifndef _WIN32
    dwError = VmKdcRpcEpRegister(
        pServerBinding,
        pInterfaceSpec,
        "VMware Kdc Service"
    );
    BAIL_ON_VMKDC_ERROR(dwError);
#endif

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL,
        "RPC Endpoints registered successfully.");

#ifndef _WIN32
/*
 * XXX
 * TODO: this does not work yet with DCERPC-WIN32 for release builds
 * FIXME
 */
    dwError = VmKdcRpcServerRegisterAuthInfo();
    BAIL_ON_VMKDC_ERROR(dwError);
#endif

error:

    if( pServerBinding != NULL )
    {
        VmKdcRpcBindingVectorFree( &pServerBinding );
    }

    return dwError;
}

static
DWORD
VmKdcBindServer(
    rpc_binding_vector_p_t* server_binding,
    PVMKDC_RPC_ENDPOINT     pEndPoints,
    ULONG                   ulCount
    )
{
    DWORD dwError = 0;
    DWORD iEP = 0;

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
            dwError = VmKdcRpcServerUseProtSeq(
                pEndPoints[iEP].pszEndPointType);
            BAIL_ON_VMKDC_ERROR(dwError);
        }
        else
        {
            dwError = VmKdcRpcServerUseProtSeqEp(
                pEndPoints[iEP].pszEndPointType,
                pEndPoints[iEP].pszEndPointName);
            BAIL_ON_VMKDC_ERROR(dwError);
        }
    }

    dwError = VmKdcRpcServerInqBindings( server_binding );
    BAIL_ON_VMKDC_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmKdcRpcAuthCallback(
    PVOID         Context
    )
{
    rpc_authz_handle_t hPrivs = NULL;
    DWORD dwAuthnLevel = 0;
    DWORD dwAuthnSvc = 0;
    DWORD dwAuthzSvc = 0;
    DWORD dwError = ERROR_SUCCESS;

    dwError = VmKdcRpcBindingInqAuthClient(
            Context,
            &hPrivs, // The data referenced by this parameter is read-only,
                     // and therefore should not be modified/freed.
            NULL,    // ServerPrincipalName - we don't need it
            &dwAuthnLevel,
            &dwAuthnSvc,
            &dwAuthzSvc);
    BAIL_ON_VMKDC_ERROR(dwError);

    VMDIR_LOG_INFO(
            VMDIR_LOG_MASK_ALL,
            "Authentication Level = %d, Authentication Service = %d,"
            "Authorization Service = %d.",
            dwAuthnLevel,
            dwAuthnSvc,
            dwAuthzSvc);

    // Now check the authentication level. We require at least packet-level
    // authentication.
    if (dwAuthnLevel < rpc_c_authn_level_pkt)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
            "Attempt by client to use weak authentication.");

        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

error:

    return dwError;
}
