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

#ifdef HAVE_DCERPC_WIN32
#define RPC_STATUS ULONG
#define RPC_S_OK rpc_s_ok
#endif

static BOOLEAN bRpcRegistered = FALSE;
static BOOLEAN bRpcListening = FALSE;

static
ULONG
VmDirRegisterRpcServer(
    VOID
    );

static
ULONG
VmDirUnRegisterRpcServer(
    VOID
    );

static
BOOLEAN
VmDirRegisterForTcpEndpoint(
    VOID
    );

static
ULONG
VmDirBindServer(
    VMDIR_RPC_BINDING_VECTOR_P_T * server_binding,
    PVMDIR_RPC_ENDPOINT pEndPoints,
    ULONG             ulCount
    );

ULONG
VmDirRpcServerInit(
    VOID
    )
{
    ULONG   ulError = 0;

    ulError  = VmDirRegisterRpcServer();
    BAIL_ON_VMDIR_ERROR(ulError);
    bRpcRegistered = TRUE;

    ulError = VmDirRpcServerStartListen();
    BAIL_ON_VMDIR_ERROR(ulError);
    bRpcListening = TRUE;

    /* Allow time for RPC service to really be listening */
    VmDirSleep(1000);

cleanup:

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Lotus RPC service status listening");
    return ulError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirRpcServerInit failed (%u)", ulError);

    goto cleanup;
}

VOID
VmDirRpcServerShutdown(
    VOID
    )
{
    ULONG ulError = 0;

    if (bRpcListening)
    {
        ulError = VmDirRpcServerStopListen();
        BAIL_ON_VMDIR_ERROR(ulError);
        bRpcListening = FALSE;
    }

    if (bRpcRegistered)
    {
        ulError = VmDirUnRegisterRpcServer();
        BAIL_ON_VMDIR_ERROR(ulError);
        bRpcRegistered = FALSE;
    }

error:

    return;
}

static
ULONG
VmDirRegisterRpcServer(
    VOID
    )
{
    ULONG ulError = 0;
    VMDIR_RPC_ENDPOINT endpoints[] =
        {
            {"ncalrpc",      LWRAFT_NCALRPC_END_POINT},
            {"ncacn_ip_tcp", LWRAFT_RPC_TCP_END_POINT}
        };
    DWORD dwEpCount = sizeof(endpoints)/sizeof(endpoints[0]);
    VMDIR_IF_HANDLE_T pVmDirInterfaceSpec    = vmdir_v1_4_s_ifspec;
    VMDIR_IF_HANDLE_T pVmDirFtpInterfaceSpec = vmdirftp_v1_0_s_ifspec; // IDL compiler will generate Srv_ prefix
    VMDIR_IF_HANDLE_T pSrpVerifierInterfaceSpec = rpc_srp_verifier_v1_0_s_ifspec; // IDL compiler will generate Srv_ prefix
    VMDIR_IF_HANDLE_T pSuperLogInterfaceSpec = vmdirsuperlog_v1_0_s_ifspec; // IDL compiler will generate Srv_ prefix
    VMDIR_IF_HANDLE_T pVmDirDbcpInterfaceSpec = vmdirdbcp_v1_0_s_ifspec; // IDL compiler will generate Srv_ prefix
    VMDIR_IF_HANDLE_T pVmDirUrgentReplInterfaceSpec = vmdirurgentrepl_v1_0_s_ifspec; // IDL compiler will generate Srv_ prefix
    VMDIR_IF_HANDLE_T pVmDirRaftInterfaceSpec = vmdirraft_v1_0_s_ifspec;
    VMDIR_RPC_BINDING_VECTOR_P_T pServerBinding = NULL;
    BOOLEAN bEndpointsRegistered = TRUE;

    ulError = VmDirRpcServerRegisterIf(pVmDirInterfaceSpec);
    BAIL_ON_VMDIR_ERROR(ulError);

    ulError = VmDirRpcServerRegisterIf(pVmDirFtpInterfaceSpec);
    BAIL_ON_VMDIR_ERROR(ulError);

    ulError = VmDirRpcServerRegisterIf(pSrpVerifierInterfaceSpec);
    BAIL_ON_VMDIR_ERROR(ulError);

    ulError = VmDirRpcServerRegisterIf(pSuperLogInterfaceSpec);
    BAIL_ON_VMDIR_ERROR(ulError);

    ulError = VmDirRpcServerRegisterIf(pVmDirDbcpInterfaceSpec);
    BAIL_ON_VMDIR_ERROR(ulError);

    ulError = VmDirRpcServerRegisterIf(pVmDirUrgentReplInterfaceSpec);
    BAIL_ON_VMDIR_ERROR(ulError);

    ulError = VmDirRpcServerRegisterIf(pVmDirRaftInterfaceSpec);
    BAIL_ON_VMDIR_ERROR(ulError);

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "Lightwave Raft Service registered successfully.");

    ulError = VmDirBindServer( &pServerBinding, endpoints, VmDirRegisterForTcpEndpoint() ? dwEpCount : dwEpCount - 1);
    BAIL_ON_VMDIR_ERROR(ulError);

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "Lightwave Raft Service bound successfully.");

#if !defined(HAVE_DCERPC_WIN32)
    ulError = VmDirRpcEpRegister( pServerBinding, pVmDirInterfaceSpec, "Lightwave Raft Service");
    if (ulError)
    {
        bEndpointsRegistered = FALSE;
    }

    ulError = VmDirRpcEpRegister( pServerBinding, pVmDirFtpInterfaceSpec, "Lightwave Raft Service FTP");
    if (ulError)
    {
        bEndpointsRegistered = FALSE;
    }

    ulError = VmDirRpcEpRegister( pServerBinding, pVmDirDbcpInterfaceSpec, "Lightwave Raft Service dbcp");
    if (ulError)
    {
        bEndpointsRegistered = FALSE;
    }

    ulError = VmDirRpcEpRegister( pServerBinding, pVmDirUrgentReplInterfaceSpec, "Lightwave Raft Service Urgent Repl");
    if (ulError)
    {
        bEndpointsRegistered = FALSE;
    }

    ulError = VmDirRpcEpRegister( pServerBinding, pVmDirRaftInterfaceSpec, "Lightwave Raft Service Raft");
    if (ulError)
    {
        bEndpointsRegistered = FALSE;
    }

    if (bEndpointsRegistered)
    {
        VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "RPC Endpoints registered successfully.");
    }
#endif

    ulError = VmDirRpcServerRegisterAuthInfo();
    BAIL_ON_VMDIR_ERROR(ulError);

error:

    if( pServerBinding != NULL )
    {
        VmDirRpcBindingVectorFree( &pServerBinding );
    }
    if (ulError)
    {
       VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirRegisterRpcServer() failed (%u)", ulError);
    }

    return ulError;
}

static
ULONG
VmDirUnRegisterRpcServer(
    VOID
    )
{
    ULONG ulError = 0;
    VMDIR_IF_HANDLE_T pVmDirInterfaceSpec    = vmdir_v1_4_s_ifspec;
    VMDIR_IF_HANDLE_T pVmDirFtpInterfaceSpec = vmdirftp_v1_0_s_ifspec; // IDL compiler will generate Srv_ prefix
    VMDIR_IF_HANDLE_T pVmDirSuperLogInterfaceSpec = vmdirsuperlog_v1_0_s_ifspec; // IDL compiler will generate Srv_ prefix
    VMDIR_IF_HANDLE_T pVmDirDbcpInterfaceSpec = vmdirdbcp_v1_0_s_ifspec; // IDL compiler will generate Srv_ prefix
    VMDIR_IF_HANDLE_T pVmDirUrgentReplInterfaceSpec = vmdirurgentrepl_v1_0_s_ifspec; // IDL compiler will generate Srv_ prefix
    VMDIR_IF_HANDLE_T pVmDirRaftInterfaceSpec = vmdirraft_v1_0_s_ifspec;

    ulError = VmDirRpcServerUnRegisterIf(pVmDirInterfaceSpec);
    BAIL_ON_VMDIR_ERROR(ulError);

    ulError = VmDirRpcServerUnRegisterIf(pVmDirFtpInterfaceSpec);
    BAIL_ON_VMDIR_ERROR(ulError);

    ulError = VmDirRpcServerUnRegisterIf(pVmDirSuperLogInterfaceSpec);
    BAIL_ON_VMDIR_ERROR(ulError);

    ulError = VmDirRpcServerUnRegisterIf(pVmDirDbcpInterfaceSpec);
    BAIL_ON_VMDIR_ERROR(ulError);

    ulError = VmDirRpcServerUnRegisterIf(pVmDirUrgentReplInterfaceSpec);
    BAIL_ON_VMDIR_ERROR(ulError);

    ulError = VmDirRpcServerUnRegisterIf(pVmDirRaftInterfaceSpec);
    BAIL_ON_VMDIR_ERROR(ulError);

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "Lightwave Raft Service unregistered successfully.");

error:

    if (ulError)
    {
       VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirUnRegisterRpcServer() failed (%u)", ulError);
    }

    return ulError;
}

static
BOOLEAN
VmDirRegisterForTcpEndpoint(
    VOID
    )
{
    BOOLEAN bRegister = FALSE;
    BOOLEAN bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirGlobals.mutex);

    bRegister = gVmdirGlobals.bRegisterTcpEndpoint;

    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirGlobals.mutex);

    return bRegister;
}

static
ULONG
VmDirBindServer(
    VMDIR_RPC_BINDING_VECTOR_P_T *  server_binding,
    PVMDIR_RPC_ENDPOINT             pEndPoints,
    ULONG                           ulCount
    )
{
    ULONG ulError = 0;
    ULONG iEP;

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
            ulError = VmDirRpcServerUseProtSeq(
                pEndPoints[iEP].pszEndPointType
            );
            BAIL_ON_VMDIR_ERROR(ulError);
        }
        else
        {
            ulError = VmDirRpcServerUseProtSeqEp(
                pEndPoints[iEP].pszEndPointType,
                pEndPoints[iEP].pszEndPointName
            );
            BAIL_ON_VMDIR_ERROR(ulError);
        }
        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "VmDirBindServer() end-point type (%s), end-point name (%s) "
                        "VmDirRpcServerUseProtSeq() succeeded.", VDIR_SAFE_STRING(pEndPoints[iEP].pszEndPointType),
                        VDIR_SAFE_STRING(pEndPoints[iEP].pszEndPointName) );
    }

    ulError = VmDirRpcServerInqBindings( server_binding );
    BAIL_ON_VMDIR_ERROR(ulError);

error:

    if (ulError)
    {
       VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirBindServer() failed (%u)", ulError);
    }

    return ulError;
}

ULONG
VmDirRpcAuthCallback(
    PVOID         Context
)
{
    VMDIR_RPC_AUTHZ_HANDLE hPrivs;
    DWORD dwAuthnLevel = 0;
    DWORD dwAuthnSvc = 0;
    DWORD dwAuthzSvc = 0;
    RPC_STATUS rpcStatus = RPC_S_OK;

    rpcStatus = VmDirRpcBindingInqAuthClient(
            Context,
            &hPrivs, // The data referenced by this parameter is read-only,
                     // and therefore should not be modified/freed.
            NULL,    // ServerPrincipalName - we don't need it
            &dwAuthnLevel,
            &dwAuthnSvc,
            &dwAuthzSvc);

    BAIL_ON_VMDIR_ERROR(rpcStatus);

    // Now check the authentication level. We require at least packet-level
    // authentication.
    if (dwAuthnLevel < VMDIR_RPC_C_AUTHN_LEVEL_PKT)
    {
        VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL, "Attempt by client to use weak authentication.");

        rpcStatus = ERROR_ACCESS_DENIED;
        BAIL_ON_VMDIR_ERROR(rpcStatus);
    }

    VMDIR_LOG_VERBOSE(  LDAP_DEBUG_RPC,
                        "RPC auth callback: Authn Level = %d,  Authn Service = %d, Authz Service = %d",
                        dwAuthnLevel, dwAuthnSvc, dwAuthzSvc);

cleanup:

    return rpcStatus;

error:

    VMDIR_LOG_ERROR(  LDAP_DEBUG_RPC, "VmDirRpcAuthCallback failed (%u)", rpcStatus);

    goto cleanup;
}

BOOLEAN
VmDirHaveLegacy(
    VOID
    )
{
    DWORD    i = 0;
    PDWORD pdwPorts = NULL;
    DWORD  dwPorts = 0;

    VmDirGetLdapListenPorts(&pdwPorts, &dwPorts);
    for ( i = 0; i < dwPorts; i++)
    {
        if (pdwPorts[i] == LEGACY_DEFAULT_LDAP_PORT_NUM)
        {
            return TRUE;
        }
    }

    VmDirGetLdapsListenPorts(&pdwPorts, &dwPorts);
    for ( i = 0; i < dwPorts; i++)
    {
        if (pdwPorts[i] == LEGACY_DEFAULT_LDAPS_PORT_NUM)
        {
            return TRUE;
        }
    }
    return FALSE;
}
