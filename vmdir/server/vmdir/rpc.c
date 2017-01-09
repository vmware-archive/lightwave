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

#if !defined(_WIN32) || defined(HAVE_DCERPC_WIN32)

#ifdef _WIN32
ULONG LwErrnoToWin32Error(int sts)
{
    /* TBD: Adam-Possibly need to map error to Win32 code */
    return (ULONG) sts;
}
#define RPC_STATUS ULONG
#define RPC_S_OK rpc_s_ok

#endif

static
PVOID
VmDirListenRpcServer(
    PVOID pInfo
    );

static
BOOLEAN
VmDirRpcCheckServerIsActive(
    VOID
    );

static
ULONG
VmDirStopRpcServer(
    VOID
);

static
VOID
VmDirRpcIfCallbackFn(
    VMDIR_IF_HANDLE_T InterfaceUuid,
    PVOID         Context,
    unsigned32*   status
);

ULONG
VmDirRpcServerStartListen(
    VOID
)
{
    ULONG ulError = 0;

    ulError = LwErrnoToWin32Error(
                    dcethread_create(
                            &gVmdirGlobals.pRPCServerThread,
                            NULL,
                            VmDirListenRpcServer,
                            NULL));
    BAIL_ON_VMDIR_ERROR(ulError);

    while (!VmDirRpcCheckServerIsActive())
    {
        VmDirSleep(1 * 1000); // sleep one second
    }

error:

    return ulError;
}

ULONG
VmDirRpcServerStopListen(
    VOID
)
{
    ULONG ulError = 0;

    ulError = VmDirStopRpcServer();
    BAIL_ON_VMDIR_ERROR(ulError);

    if (gVmdirGlobals.pRPCServerThread)
    {
        ulError = LwErrnoToWin32Error(
                        dcethread_interrupt(
                                gVmdirGlobals.pRPCServerThread));
        BAIL_ON_VMDIR_ERROR(ulError);

#if defined(_WIN32)
        // BUGBUG BUGBUG PR http://bugzilla.eng.vmware.com/show_bug.cgi?id=1219191
        // This is most likely a pthread issue due to signal lost.
        // We should update pthread to see if we can resolve this.
        // http://bugzilla.eng.vmware.com/show_bug.cgi?id=1224401 tracks this effort.

        // For now, force shutdonw w/o synchronize listener thead exiting.
        VmDirSleep(100);  // pause 100 ms
#else
        ulError = LwErrnoToWin32Error(
                        dcethread_join(
                                gVmdirGlobals.pRPCServerThread,
                                NULL));
        BAIL_ON_VMDIR_ERROR(ulError);
#endif
        gVmdirGlobals.pRPCServerThread = NULL;
    }

error:

    return ulError;
}

ULONG
VmDirRpcServerRegisterIf(
    VMDIR_IF_HANDLE_T pInterfaceSpec
)
{
    ULONG ulError = 0;

    DCETHREAD_TRY
    {
       rpc_server_register_if_ex(
               pInterfaceSpec,
               NULL,
               NULL,
               rpc_if_allow_secure_only,
               rpc_c_listen_max_calls_default,
               VmDirRpcIfCallbackFn,
               &ulError
               );
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
       if ( ulError == rpc_s_ok )
       {
           ulError = dcethread_exc_getstatus (THIS_CATCH);
          if (!ulError)
          {
              ulError = RPC_S_INTERNAL_ERROR;
          }
       }
    }
    DCETHREAD_ENDTRY;

    return ulError;
}

ULONG
VmDirRpcServerUnRegisterIf(
    VMDIR_IF_HANDLE_T pInterfaceSpec
)
{
    ULONG ulError = 0;

    DCETHREAD_TRY
    {
       rpc_server_unregister_if(
               pInterfaceSpec,
               NULL,
               &ulError
               );
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
       if ( ulError == rpc_s_ok )
       {
           ulError = dcethread_exc_getstatus (THIS_CATCH);
          if (!ulError)
          {
              ulError = RPC_S_INTERNAL_ERROR;
          }
       }
    }
    DCETHREAD_ENDTRY;

    return ulError;
}

ULONG
VmDirRpcServerUseProtSeq(
    PCSTR pszProtSeq
)
{
    ULONG ulError = 0;

    DCETHREAD_TRY
    {
        rpc_server_use_protseq(
                (unsigned char*) pszProtSeq,
                rpc_c_protseq_max_calls_default,
                (unsigned32*)&ulError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        if ( ulError == rpc_s_ok )
        {
            ulError = dcethread_exc_getstatus (THIS_CATCH);
            if (!ulError)
            {
                ulError = RPC_S_INTERNAL_ERROR;
            }
        }
    }
    DCETHREAD_ENDTRY;

    BAIL_ON_VMDIR_ERROR(ulError);

error:

    return ulError;
}

ULONG
VmDirRpcServerUseProtSeqEp(
    PCSTR pszProtSeq,
    PCSTR pszEndpoint
)
{
    ULONG   ulError = 0;
    int     iCnt = 0;

    for (iCnt = 0; iCnt < 6; iCnt++)
    {
        DCETHREAD_TRY
        {
            rpc_server_use_protseq_ep(
                    (unsigned char*) pszProtSeq,
                    rpc_c_protseq_max_calls_default,
                    (unsigned char*) pszEndpoint,
                    (unsigned32*)&ulError);
        }
        DCETHREAD_CATCH_ALL(THIS_CATCH)
        {
            if ( ulError == rpc_s_ok )
            {
                ulError = dcethread_exc_getstatus (THIS_CATCH);
                if (!ulError)
                {
                    ulError = RPC_S_INTERNAL_ERROR;
                }
            }
        }
        DCETHREAD_ENDTRY;

        if (ulError == 0)
        {
            break;
        }
        VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL, "VmDirRpcServerUseProtSeqEp() failed (%u), protoSeq (%s), endPoint(%s). "
                           "Sleeping for 5 seconds before retrying ...", ulError, pszProtSeq, pszEndpoint );

        VmDirSleep(5 * 1000); // sleep five seconds
    }

    BAIL_ON_VMDIR_ERROR(ulError);

error:
    if (ulError)
    {
       VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirRpcServerUseProtSeqEp() failed (%u), protoSeq (%s), endPoint(%s)",
                        ulError, pszProtSeq, pszEndpoint  );
    }
    return ulError;
}

ULONG
VmDirRpcServerInqBindings(
    VMDIR_RPC_BINDING_VECTOR_P_T* ppServerBindings
)
{
    ULONG ulError = 0;

    DCETHREAD_TRY
    {
        rpc_server_inq_bindings(ppServerBindings, (unsigned32*)&ulError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        if ( ulError == rpc_s_ok )
        {
            ulError = dcethread_exc_getstatus (THIS_CATCH);
            if (!ulError)
            {
                ulError = RPC_S_INTERNAL_ERROR;
            }
        }
     }
     DCETHREAD_ENDTRY;

     BAIL_ON_VMDIR_ERROR(ulError);

error:

     return ulError;
}

ULONG
VmDirRpcEpRegister(
    VMDIR_RPC_BINDING_VECTOR_P_T pServerBinding,
    VMDIR_IF_HANDLE_T            pInterfaceSpec,
    PCSTR                        pszAnnotation
)
{
    ULONG ulError = 0;

    DCETHREAD_TRY
    {
       rpc_ep_register(
                pInterfaceSpec,
                pServerBinding,
                NULL,
                (idl_char*)pszAnnotation,
                (unsigned32*)&ulError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        if (ulError == rpc_s_ok )
        {
            ulError = dcethread_exc_getstatus (THIS_CATCH);
            if (!ulError)
            {
               ulError = RPC_S_INTERNAL_ERROR;
            }
        }
    }
    DCETHREAD_ENDTRY;

    return ulError;
}

ULONG
VmDirRpcServerRegisterAuthInfo(
    VOID
)
{
    ULONG   ulError = 0;
    char    szHostName[VMDIR_MAX_HOSTNAME_LEN] = {0};
    PSTR    pszCanonicalHostName = NULL;
    PSTR    pszServerPrincipalName = NULL;
    PSTR    pszLowerCaseHostName = NULL;

    ulError = VmDirGetHostName( szHostName, sizeof(szHostName)-1);
    BAIL_ON_VMDIR_ERROR(ulError);

    ulError = VmDirGetCanonicalHostName( szHostName, &pszCanonicalHostName );
    BAIL_ON_VMDIR_ERROR(ulError);

    ulError = VmDirAllocASCIIUpperToLower( pszCanonicalHostName, &pszLowerCaseHostName );
    BAIL_ON_VMDIR_ERROR(ulError);

    ulError = VmDirAllocateStringPrintf(  &pszServerPrincipalName,
                                          "host/%s",
                                          pszLowerCaseHostName);
    BAIL_ON_VMDIR_ERROR(ulError);

    DCETHREAD_TRY
    {
       rpc_server_register_auth_info (
                  pszServerPrincipalName,       // Server principal name
                  rpc_c_authn_gss_negotiate,    // Authentication service
                  NULL,                         // Use default key function
                  NULL,
                  &ulError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
       if (ulError == rpc_s_ok )
       {
            ulError = dcethread_exc_getstatus (THIS_CATCH);
            if (!ulError)
            {
              ulError = RPC_S_INTERNAL_ERROR;
            }
       }
    }
    DCETHREAD_ENDTRY;

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "RPC server register auth info SPN (%s)", pszServerPrincipalName);

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszCanonicalHostName);
    VMDIR_SAFE_FREE_MEMORY(pszServerPrincipalName);
    VMDIR_SAFE_FREE_MEMORY(pszLowerCaseHostName);

    return ulError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirRpcServerRegisterAuthInfo, (%s)(%s) failed (%u)",
                                         VDIR_SAFE_STRING(pszCanonicalHostName),
                                         VDIR_SAFE_STRING(pszServerPrincipalName),
                                         ulError);
    goto cleanup;
}

ULONG
VmDirRpcBindingInqAuthClient(
    VMDIR_RPC_BINDING_HANDLE    hClientBinding,
    VMDIR_RPC_AUTHZ_HANDLE*     pPrivs,
    PSTR*                       ppServerPrincName,
    DWORD*                      pAuthnLevel,
    DWORD*                      pAuthnSvc,
    DWORD*                      pAuthzSvc
)
{
    RPC_STATUS rpcStatus = rpc_s_ok;

    rpc_binding_inq_auth_client(
            hClientBinding,
            pPrivs, // The data referenced by this parameter is read-only,
                    // and therefore should not be modified/freed.
            (unsigned_char_p_t*)ppServerPrincName,
            pAuthnLevel,
            pAuthnSvc,
            pAuthzSvc,
            &rpcStatus);

    BAIL_ON_VMDIR_ERROR(rpcStatus);

error:

    return rpcStatus;
}

ULONG
VmDirRpcBindingVectorFree(
    VMDIR_RPC_BINDING_VECTOR_P_T* ppServerBindings
)
{
    RPC_STATUS rpcStatus = rpc_s_ok;

    if ( (ppServerBindings != NULL) && ((*ppServerBindings) != NULL) )
    {
        rpc_binding_vector_free(ppServerBindings, &rpcStatus);
    }
    BAIL_ON_VMDIR_ERROR(rpcStatus);

error:

    return rpcStatus;
}

static
PVOID
VmDirListenRpcServer(
    PVOID pInfo
    )
{
    volatile ULONG ulError = 0;
    BOOLEAN bHeartbeat = TRUE;

    ulError  = VmDirCreateHeartbeatThread();
    if (ulError)
    {
        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL,
                "Was not able to start vmafd heartbeat. Skipping." );

        bHeartbeat = FALSE;
        ulError = 0;
    }

    DCETHREAD_TRY
    {
        rpc_server_listen(
            rpc_c_listen_max_calls_default, (unsigned32*)&ulError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        if (!ulError)
        {
            ulError = dcethread_exc_getstatus (THIS_CATCH);
        }
        if(!ulError)
        {
            ulError = RPC_S_INTERNAL_ERROR;
        }
    }
    DCETHREAD_ENDTRY;

    BAIL_ON_VMDIR_ERROR(ulError);

cleanup:

    if (bHeartbeat)
    {
        VmDirKillHeartbeatThread();
    }

#if !defined(HAVE_DCERPC_WIN32)
    raise(SIGTERM); // indicate that process must terminate
#endif

    return NULL;

error:

    goto cleanup;
}

static
BOOLEAN
VmDirRpcCheckServerIsActive(
    VOID
    )
{
    volatile ULONG ulError = 0;
    BOOLEAN bIsActive = FALSE;

    DCETHREAD_TRY
    {
        bIsActive = rpc_mgmt_is_server_listening(NULL, (unsigned32*)&ulError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        if (!ulError)
        {
            ulError = dcethread_exc_getstatus (THIS_CATCH);
        }
        if (!ulError)
        {
            ulError = RPC_S_INTERNAL_ERROR;
        }
    }
    DCETHREAD_ENDTRY;

    BAIL_ON_VMDIR_ERROR(ulError);

cleanup:

    return bIsActive;

error:

    bIsActive = FALSE;

    goto cleanup;
}

static
ULONG
VmDirStopRpcServer(
    VOID
    )
{
    volatile ULONG ulError = 0;

    DCETHREAD_TRY
    {
        rpc_mgmt_stop_server_listening(NULL, (unsigned32*)&ulError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        if (!ulError)
        {
            ulError = dcethread_exc_getstatus (THIS_CATCH);
        }
        if(!ulError)
        {
            ulError = RPC_S_INTERNAL_ERROR;
        }
    }
    DCETHREAD_ENDTRY;

    BAIL_ON_VMDIR_ERROR(ulError);

cleanup:

    return ulError;

error:

    goto cleanup;
}

static
VOID
VmDirRpcIfCallbackFn(
    VMDIR_IF_HANDLE_T InterfaceUuid,
    PVOID         Context,
    unsigned32*   status
    )
{
    *status = VmDirRpcAuthCallback(Context);
}

#endif
