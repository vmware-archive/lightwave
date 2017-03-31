/*
 * Copyright (C) 2011 VMware, Inc. All rights reserved.
 *
 * Module   : service.c
 *
 * Abstract :
 *
 *            VMware Directory Service
 *
 *            Service
 *
 *            RPC common functions
 *
 */

#include "..\includes.h"


#ifdef _WIN32

#define END_POINT_BUF_LEN 128
#define HOST_NAME_LEN 256


// Security-callback function.


// TBD: Why is the following callback function NOT being called on
// Linux/Likewise env?
static
RPC_STATUS CALLBACK RpcIfCallbackFn(
    RPC_IF_HANDLE  InterfaceUuid,
    PVOID Context
)
{
    RPC_AUTHZ_HANDLE hPrivs;
    unsigned char * pszServerPrincName=NULL;
    DWORD dwAuthnLevel;
    DWORD dwAuthnSvc;
    DWORD dwAuthzSvc;
    RPC_STATUS rpcStatus = RPC_S_OK;

    VMCA_LOG_DEBUG("In RpcIfCallbackFn.\n");

    rpcStatus = RpcBindingInqAuthClient(
        Context,
        &hPrivs, // The data referenced by this parameter is read-only,
                 // and therefore should not be modified/freed.
        &pszServerPrincName,
        &dwAuthnLevel,
        &dwAuthnSvc,
        &dwAuthzSvc);

    if (rpcStatus != RPC_S_OK) {
        VMCA_LOG_ERROR("RpcBindingInqAuthClient returned: 0x%x\n", rpcStatus);
        BAIL_ON_VMCA_ERROR(rpcStatus);
    }

    VMCA_LOG_DEBUG("Authentication Level = %d, Authentication Service = %d,"
             "Authorization Service = %d.\n", dwAuthnLevel, dwAuthnSvc,
                 dwAuthzSvc);

    // Now check the authentication level. We require at least packet-level
    // authentication.
    if (dwAuthnLevel < RPC_C_AUTHN_LEVEL_PKT) {
        VMCA_LOG_ERROR("Attempt by client to use weak authentication.\n");
        rpcStatus = ERROR_ACCESS_DENIED;
        BAIL_ON_VMCA_ERROR(rpcStatus);
    }

    return 0;

error:
    if (pszServerPrincName != NULL) {
        RpcStringFree(&pszServerPrincName);
    }
    return 0;
}

static
DWORD
VMCARegisterRpcServerIf(
    VMCA_IF_HANDLE_T hInterfaceSpec
)
{
    DWORD dwError = 0;


    dwError = RpcServerRegisterIfEx(
               hInterfaceSpec,
               NULL,
               NULL,
               RPC_IF_ALLOW_SECURE_ONLY,
               RPC_C_LISTEN_MAX_CALLS_DEFAULT,
               RpcIfCallbackFn
    );

    BAIL_ON_VMCA_ERROR(dwError);

error:

    return dwError;
}


static
DWORD
VMCABindServer(
    VMCA_RPC_BINDING_VECTOR_P_T *pServerBinding,
    RPC_IF_HANDLE ifSpec,
    PVMCA_ENDPOINT pEndPoints,
    DWORD dwCount
    )
{
    DWORD dwError = 0;
    DWORD i;

    /*
     * Prepare the server binding handle
     * use all avail protocols (UDP and TCP). This basically allocates
     * new sockets for us and associates the interface UUID and
     * object UUID of with those communications endpoints.
     */
    for (i = 0; i<dwCount; i++)
    {
        if (!pEndPoints[i].endpoint)
        {
            RpcTryExcept
            {

               dwError = RpcServerUseProtseq((unsigned char*) pEndPoints[i].protocol,
                                      RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                                      NULL);
            }
            RpcExcept (RpcExceptionCode())
            {
               dwError = RpcExceptionCode();
            }
            RpcEndExcept;

            BAIL_ON_VMCA_ERROR(dwError);
        }
        else
        {
            RpcTryExcept
            {
                dwError = RpcServerUseProtseqEp((unsigned char*) pEndPoints[i].protocol,
                                        RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                                        (unsigned char*) pEndPoints[i].endpoint,
                                        NULL);
            }
            RpcExcept (RpcExceptionCode())
            {
                dwError = RpcExceptionCode();
            }
            RpcEndExcept;

            BAIL_ON_VMCA_ERROR(dwError);
        }
    }

    RpcTryExcept
    {
        dwError = RpcServerInqBindings(pServerBinding);
    }
    RpcExcept (RpcExceptionCode())
    {
        dwError = RpcExceptionCode();
    }
    RpcEndExcept;

    BAIL_ON_VMCA_ERROR(dwError);

error:

    return dwError;
}

static
DWORD
VMCAFreeBindingVector(
    VMCA_RPC_BINDING_VECTOR_P_T pServerBinding
)
{
    DWORD dwError = 0;
    RpcTryExcept
    {
        dwError = RpcBindingVectorFree( &pServerBinding);
    }
    RpcExcept (RpcExceptionCode())
    {
        dwError = RpcExceptionCode();
    }
    RpcEndExcept;

    BAIL_ON_VMCA_ERROR(dwError);

error:
    return dwError;
}

static
DWORD
VMCAEpRegister(
    VMCA_RPC_BINDING_VECTOR_P_T pServerBinding,
    VMCA_IF_HANDLE_T            pInterfaceSpec,
    PCSTR                        pszAnnotation  // "VMCAService Service"
)
{
    DWORD dwError = 0;
    RpcTryExcept
    {
        dwError = RpcEpRegisterA(
            pInterfaceSpec,
            pServerBinding,
            NULL,
            (RPC_CSTR)pszAnnotation
        );
    }
    RpcExcept (RpcExceptionCode())
    {
        dwError = RpcExceptionCode();
    }
    RpcEndExcept;

    BAIL_ON_VMCA_ERROR(dwError);

error:

    return dwError;
}


static DWORD
VMCARegisterAuthInfo(
    VOID
)
{
    DWORD dwError = 0;
    RpcTryExcept
    {
        dwError = RpcServerRegisterAuthInfoA(
            NULL, // Server principal name
            RPC_C_AUTHN_GSS_NEGOTIATE, // Authentication service
            NULL, // Use default key function
            NULL
        );
    }
    RpcExcept (RpcExceptionCode())
    {
        dwError = RpcExceptionCode();
    }
    RpcEndExcept;
    BAIL_ON_VMCA_ERROR(dwError);

error:

    return dwError;
}


DWORD
VMCAStartRpcServer()
{
    DWORD dwError = 0;
    char  npEndpointBuf[END_POINT_BUF_LEN] = {VMCA_NCALRPC_END_POINT};

#ifndef _WIN32
    VMCA_ENDPOINT endpoints[] =
    {
        {"ncalrpc", NULL}
    };
#else
    VMCA_ENDPOINT endpoints[] =
    {
        {"ncalrpc", VMCA_NCALRPC_END_POINT}
    };
#endif

    DWORD dwEpCount = sizeof(endpoints)/sizeof(endpoints[0]);

    VMCA_RPC_BINDING_VECTOR_P_T pServerBinding = NULL;

    endpoints[0].endpoint = npEndpointBuf;

    // Register RPC server
    dwError = VMCARegisterRpcServerIf(vmca_v1_0_s_ifspec);

    BAIL_ON_VMCA_ERROR(dwError);

    VMCA_LOG_INFO("VMCAService Service registered successfully.");

    // Bind RPC server
    dwError = VMCABindServer(
                      &pServerBinding,
                      vmca_v1_0_s_ifspec,
                      endpoints,
                      dwEpCount);

    BAIL_ON_VMCA_ERROR(dwError);

    VMCA_LOG_INFO("VMCAService Service bound successfully.");

    // The RpcEpRegister function adds or replaces entries in the local host's endpoint-map database.
    // For an existing database entry that matches the provided interface specification, binding handle, and object UUID,
    // this function replaces the entry's endpoint with the endpoint in the provided binding handle.

    dwError = VMCAEpRegister(pServerBinding, vmca_v1_0_s_ifspec, "VMCAService Service");
    BAIL_ON_VMCA_ERROR(dwError);

    VMCA_LOG_INFO("RPC Endpoints registered successfully.");

    // free binding vector to avoid resourc leaking
    dwError = VMCAFreeBindingVector(pServerBinding);
    BAIL_ON_VMCA_ERROR(dwError);
    VMCA_LOG_INFO("RPC free vector binding successfully.");

    // A server application calls RpcServerRegisterAuthInfo to register an authentication service to use
    // for authenticating remote procedure calls. A server calls this routine once for each authentication service the server wants to register.
    // If the server calls this function more than once for a given authentication service, the results are undefined.
    // dwError = VMCARegisterAuthInfo();

    // BAIL_ON_VMCA_ERROR(dwError);

    VMCA_LOG_INFO("VMCAService Service is listening on local named-piped port on [%s]\n",
                  endpoints[0].endpoint);

error:

    return dwError;
}


PVOID
VMCAListenRpcServer(
    PVOID pInfo
)
{
    DWORD dwError = 0;
    unsigned int    cMinCalls = 1;
    unsigned int    fDontWait = TRUE;
    PVMAFD_HB_HANDLE pHandle = NULL;

    dwError = VMCAHeartbeatInit(&pHandle);
    BAIL_ON_VMCA_ERROR(dwError);

    RpcTryExcept
    {
        dwError = RpcServerListen(
                    cMinCalls,
                    RPC_C_LISTEN_MAX_CALLS_DEFAULT,
                    fDontWait);
    }
    RpcExcept (RpcExceptionCode())
    {
        dwError = RpcExceptionCode();
    }
    RpcEndExcept;

    BAIL_ON_VMCA_ERROR(dwError);

cleanup:

    if (pHandle)
    {
        VMCAStopHeartbeat(pHandle);
    }
    VMCA_LOG_INFO ("VMCAListenRpcServer is exiting\n");
    return NULL;
error:
    VMCA_LOG_ERROR ("VMCAListenRpcServer failed [%d]\n", dwError);
    goto cleanup;
}

DWORD
VMCAStopRpcServer(
    VOID
)
{
    DWORD dwError = 0;

    RpcTryExcept
    {
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
    }
    RpcExcept (RpcExceptionCode())
    {
        dwError = RpcExceptionCode();
    }
    RpcEndExcept;

    return dwError;
}

#endif _WIN32
