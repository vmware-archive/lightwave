/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

#include "stdafx.h"

#define END_POINT_BUF_LEN 128
#define HOST_NAME_LEN 256

// Security-callback function.
DWORD
EventLogRpcStringFree(
    unsigned_char_p_t *ppszString
    )
{
    DWORD dwError = 0;

    if (ppszString)
    {
        rpc_string_free((PBYTE*)ppszString, &dwError);
    }

    return dwError;
}


// TBD: Why is the following callback function NOT being called on
// Linux/Likewise env?
static void
RpcIfCallbackFn(
    rpc_if_handle_t  InterfaceUuid,
    void *Context,
    unsigned32 *status
    )
{
	rpc_authz_handle_t hPrivs;
	unsigned_char_p_t pszServerPrincName=NULL;
	DWORD dwAuthnLevel;
	DWORD dwAuthnSvc;
	DWORD dwAuthzSvc;
	error_status_t rpcStatus = rpc_s_ok;

	rpc_binding_inq_auth_client(
		Context,
		&hPrivs, // The data referenced by this parameter is read-only,
		         // and therefore should not be modified/freed.
		&pszServerPrincName,
		&dwAuthnLevel,
		&dwAuthnSvc,
		&dwAuthzSvc,
		&rpcStatus);

	if (rpcStatus != rpc_s_ok) {
		BAIL_ON_VMEVENT_ERROR(rpcStatus);
	}

	// Now check the authentication level. We require at least packet-level
	// authentication.
	if (dwAuthnLevel < rpc_c_authn_level_pkt_privacy) {
		rpcStatus = ERROR_ACCESS_DENIED;
		BAIL_ON_VMEVENT_ERROR(rpcStatus);
	}

error:
	if (pszServerPrincName != NULL) {
		EventLogRpcStringFree(&pszServerPrincName);
	}
	*status = rpcStatus;
}

DWORD
EventLogRegisterRpcServerIf()
{
    DWORD dwError = 0;

    DCETHREAD_TRY
    {
       rpc_server_register_if_ex(
               eventlog_v1_0_s_ifspec,
               NULL,
               NULL,
               rpc_if_allow_secure_only, // 1st line of defence.
               rpc_c_listen_max_calls_default,
               RpcIfCallbackFn,
               &dwError
               );
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
       if ( dwError == rpc_s_ok )
       {
          dwError = dcethread_exc_getstatus (THIS_CATCH);
          if (!dwError)
          {
            dwError = RPC_S_INTERNAL_ERROR;
          }
       }
    }
    DCETHREAD_ENDTRY;

    return dwError;
}

static
DWORD
EventLogBindServer(
    rpc_binding_vector_p_t * server_binding,
    rpc_if_handle_t interface_spec,
    PEVENTLOG_ENDPOINT pEndPoints,
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
            DCETHREAD_TRY
            {
               rpc_server_use_protseq((unsigned char*) pEndPoints[i].protocol,
                                      rpc_c_protseq_max_calls_default,
                                      (unsigned32*)&dwError);
            }
            DCETHREAD_CATCH_ALL(THIS_CATCH)
            {
               if ( dwError == rpc_s_ok )
               {
                  dwError = dcethread_exc_getstatus (THIS_CATCH);
                  if (!dwError)
                  {
                    dwError = RPC_S_INTERNAL_ERROR;
                  }
               }
            }
            DCETHREAD_ENDTRY;

            BAIL_ON_VMEVENT_ERROR(dwError);
        }
        else
        {
            DCETHREAD_TRY
            {
              rpc_server_use_protseq_ep((unsigned char*) pEndPoints[i].protocol,
                                        rpc_c_protseq_max_calls_default,
                                        (unsigned char*) pEndPoints[i].endpoint,
                                        (unsigned32*)&dwError);
            }
            DCETHREAD_CATCH_ALL(THIS_CATCH)
            {
               if ( dwError == rpc_s_ok )
               {
                  dwError = dcethread_exc_getstatus (THIS_CATCH);
                  if (!dwError)
                  {
                    dwError = RPC_S_INTERNAL_ERROR;
                  }
               }
             }
             DCETHREAD_ENDTRY;

            BAIL_ON_VMEVENT_ERROR(dwError);
        }
    }

    DCETHREAD_TRY
    {
        rpc_server_inq_bindings(server_binding, (unsigned32*)&dwError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        if ( dwError == rpc_s_ok )
        {
           dwError = dcethread_exc_getstatus (THIS_CATCH);
           if (!dwError)
           {
            dwError = RPC_S_INTERNAL_ERROR;
           }
        }
     }
     DCETHREAD_ENDTRY;

    BAIL_ON_VMEVENT_ERROR(dwError);

error:

    return dwError;
}

DWORD
EventLogEpRegister(
      rpc_binding_vector_p_t pServerBinding)
{
    DWORD dwError = 0;

    DCETHREAD_TRY
    {
       rpc_ep_register(
              eventlog_v1_0_s_ifspec,
              pServerBinding,
              NULL,
              (idl_char*)"EventLogService Service",
              (unsigned32*)&dwError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        if (dwError == rpc_s_ok )
        {
           dwError = dcethread_exc_getstatus (THIS_CATCH);
           if (!dwError)
           {
             dwError = RPC_S_INTERNAL_ERROR;
           }
        }
    }
    DCETHREAD_ENDTRY;

    return dwError;
}

static
DWORD
EventLogRegisterAuthInfo()
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
          dwError = dcethread_exc_getstatus (THIS_CATCH);
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
EventLogStartRpcServer()
{
    DWORD dwError = 0;
    char  npEndpointBuf[END_POINT_BUF_LEN] = {VMEVENT_NCALRPC_END_POINT};
    EVENTLOG_ENDPOINT endpoints[] =
	{
		{"ncalrpc", NULL},
        {"ncacn_ip_tcp", VMEVENT_RPC_TCP_END_POINT}
	};
    DWORD dwEpCount = sizeof(endpoints)/sizeof(endpoints[0]);

    rpc_binding_vector_p_t pServerBinding = NULL;
	
	endpoints[0].endpoint = npEndpointBuf;

    dwError = EventLogRegisterRpcServerIf();
    BAIL_ON_VMEVENT_ERROR(dwError);


    dwError = EventLogBindServer(
                      &pServerBinding,
                      eventlog_v1_0_s_ifspec,
                      endpoints,
                      dwEpCount);

    BAIL_ON_VMEVENT_ERROR(dwError);

    dwError = EventLogEpRegister(pServerBinding);
    BAIL_ON_VMEVENT_ERROR(dwError);


    dwError = EventLogRegisterAuthInfo();
    BAIL_ON_VMEVENT_ERROR(dwError);

error:

    return dwError;
}



PVOID
EventLogListenRpcServer(
    PVOID pInfo
    )
{
    volatile DWORD dwError = 0;

    DCETHREAD_TRY
    {
        rpc_server_listen(
                        rpc_c_listen_max_calls_default,
                        (unsigned32*)&dwError
                        );
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        if (!dwError)
        {
            dwError = dcethread_exc_getstatus (THIS_CATCH);
        }
        if(!dwError)
        {
           dwError = RPC_S_INTERNAL_ERROR;
        }
    }
    DCETHREAD_ENDTRY;

    BAIL_ON_VMEVENT_ERROR(dwError);

cleanup:

    raise(SIGTERM); // indicate that process must terminate

    return NULL;

error:

    goto cleanup;
}

DWORD
EventLogStopRpcServer(
    VOID
    )
{
    volatile DWORD dwError = 0;

    DCETHREAD_TRY
    {
        rpc_mgmt_stop_server_listening(NULL, (unsigned32*)&dwError);
    }
    DCETHREAD_CATCH_ALL(THIS_CATCH)
    {
        if (!dwError)
        {
            dwError = dcethread_exc_getstatus (THIS_CATCH);
        }
        if(!dwError)
        {
          dwError = RPC_S_INTERNAL_ERROR;
        }
    }
    DCETHREAD_ENDTRY;

    BAIL_ON_VMEVENT_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}
