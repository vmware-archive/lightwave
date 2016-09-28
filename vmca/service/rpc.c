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
#include "includes.h"

#ifndef _WIN32
#include <lwrpcrt/lwrpcrt.h>
#endif

#include <dce/rpc.h>

DWORD
VMCARegisterAuthInfo(
    VOID
)
{
    #define VMCA_MAX_HOSTNAME_LEN         256
    ULONG   ulError = 0;
    PSTR    pszHostName = NULL;
    PSTR    pszCanonicalHostName = NULL;
    PSTR    pszServerPrincipalName = NULL;
    PSTR    pszLowerCaseHostName = NULL;

    ulError = VMCAGetHostName( &pszHostName);
    BAIL_ON_VMCA_ERROR(ulError);

    ulError = VMCAGetCanonicalHostName( pszHostName,
                                &pszCanonicalHostName );
    BAIL_ON_VMCA_ERROR(ulError);

    ulError = VMCAStringToLower(pszCanonicalHostName,
                                    &pszLowerCaseHostName);
    BAIL_ON_VMCA_ERROR(ulError);

    ulError = VMCAAllocateStringPrintfA( &pszServerPrincipalName,
                                        "vmca/%s",
                                        pszLowerCaseHostName);

    rpc_server_register_auth_info(
        pszServerPrincipalName, // Server principal name
        rpc_c_authn_gss_negotiate, // Authentication service
        NULL, // Use default key function
        NULL,
        &ulError
    );

    if (ulError == rpc_s_ok){
        ulError = 0;
    }
    BAIL_ON_VMCA_ERROR(ulError);

error:
    VMCA_SAFE_FREE_MEMORY(pszHostName);
    VMCA_SAFE_FREE_MEMORY(pszCanonicalHostName);
    VMCA_SAFE_FREE_MEMORY(pszServerPrincipalName);
    VMCA_SAFE_FREE_MEMORY(pszLowerCaseHostName);
    return (DWORD) ulError;
}

DWORD
VMCAStartRpcServer()
{
    DWORD   dwError = 0;
    int     i = 0;
    int     iCnt = 0;

    VMCA_ENDPOINT endpoints[] =
        {
            {"ncalrpc",      VMCA_NCALRPC_END_POINT},
            {"ncacn_ip_tcp", VMCA_RPC_TCP_END_POINT}
        };

    rpc_server_register_if(
            vmca_v3_0_s_ifspec,
            NULL,
            NULL,
            &dwError);
    BAIL_ON_VMCA_ERROR(dwError);

    for (i=0; i<sizeof(endpoints)/sizeof(VMCA_ENDPOINT); i++)
    {
        for (iCnt = 0; iCnt < 6; iCnt++)
        {
            rpc_server_use_protseq_ep(
                (unsigned char *) endpoints[i].pszProtSeq,
                rpc_c_protseq_max_calls_default,
                (unsigned char *) endpoints[i].pszEndpoint,
                &dwError);

            if (dwError == 0)
            {
                VMCA_LOG_INFO( "rpc_server_use_protseq_ep() succeeded, protoSeq (%s), endPoint(%s).",
                               endpoints[i].pszProtSeq, endpoints[i].pszEndpoint );
                break;
            }

            VMCA_LOG_INFO( "rpc_server_use_protseq_ep() failed (%d), protoSeq (%s), endPoint(%s). Sleeping for 5 "
                           "seconds before retrying ...", dwError, endpoints[i].pszProtSeq, endpoints[i].pszEndpoint );

            VMCASleep(5); // sleep five seconds
        }
        BAIL_ON_VMCA_ERROR(dwError);
    }

// disbale KRB until vCake is ready for it.
#ifdef VMCA_KRB
    dwError = VMCARegisterAuthInfo();
    BAIL_ON_VMCA_ERROR(dwError);
#endif

error:
    if (dwError)
    {
        VMCA_LOG_ERROR( "VMCAStartRpcServer failed (%d)", dwError );
    }

    return dwError;
}



void *
VMCAListenRpcServer(
    void * pArg
    )
{
    DWORD dwError = 0;
    PVMAFD_HB_HANDLE pHandle = NULL;

    dwError = VMCAHeartbeatInit(&pHandle);
    BAIL_ON_VMCA_ERROR(dwError);

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

    BAIL_ON_VMCA_ERROR(dwError);

cleanup:

    if (pHandle)
    {
        VMCAStopHeartbeat(pHandle);
    }
    VMCA_LOG_INFO ("VMCAListenRpcServer is exiting\n");
#ifndef _WIN32
    raise(SIGTERM);
#endif
    return NULL;

error:

    VMCA_LOG_ERROR ("VMCAListenRpcServer failed (%d)", dwError);
    goto cleanup;
}

DWORD
VMCAStopRpcServer(
    VOID
    )
{
    DWORD dwError = 0;

    rpc_mgmt_stop_server_listening(NULL, &dwError);
    BAIL_ON_VMCA_ERROR(dwError);

error:
    return dwError;

}
