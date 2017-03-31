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

static
BOOLEAN
EventLogIsLocalHost(
    PCSTR pszHostname
    );

DWORD
EventLogCreateBindingHandleA(
    PCSTR pszNetworkAddress,
    PCSTR pszNetworkEndpoint,
    handle_t * ppBinding
    )
{
    DWORD dwError = 0;
    struct
    {
      PCSTR pszProtocolSequence;
      PCSTR pszEndPoint;
    }
    connection_info[] =
    {
        {
            VMEVENT_SF_INIT(.pszProtocolSequence, "ncalrpc"),
            VMEVENT_SF_INIT(.pszEndPoint, VMEVENT_NCALRPC_END_POINT),
        },
        {
            VMEVENT_SF_INIT(.pszProtocolSequence, "ncacn_ip_tcp"),
            VMEVENT_SF_INIT(.pszEndPoint, VMEVENT_RPC_TCP_END_POINT),
        }
    };
    PCSTR pszProtocolSequence = NULL;
    PCSTR pszEndpoint      = NULL;
    PSTR  pszStringBinding = NULL;
    BOOLEAN  bLocalHost = FALSE;
    handle_t pBinding   = NULL;
    PSTR pszServerPrincipalName = NULL;
    PSTR pszCanonicalHostName = NULL;

    if (!ppBinding)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMEVENT_ERROR(dwError);
    }

    bLocalHost = EventLogIsLocalHost(pszNetworkAddress);
    if (bLocalHost)
    {
        pszNetworkAddress = NULL;
        pszProtocolSequence = connection_info[0].pszProtocolSequence;

        pszEndpoint = pszNetworkEndpoint ?  pszNetworkEndpoint :
                                            connection_info[0].pszEndPoint;
    }
    else
    {
        pszProtocolSequence = connection_info[1].pszProtocolSequence;

        pszEndpoint = pszNetworkEndpoint ?  pszNetworkEndpoint :
                                            connection_info[1].pszEndPoint;
    }

    rpc_string_binding_compose(
                        NULL,
                        (PBYTE)pszProtocolSequence,
                        (PBYTE)pszNetworkAddress,
                        (PBYTE)pszEndpoint,
                        NULL,
                        (PBYTE*)&pszStringBinding,
                        &dwError);
    BAIL_ON_ERROR(dwError);

    rpc_binding_from_string_binding(
                        pszStringBinding,
                        &pBinding,
                        &dwError);
    BAIL_ON_ERROR(dwError);

    /* Set up authentication if we are connecting to a remote host */
    if (!bLocalHost)
    {
        dwError = EventLogGetCanonicalHostName(
                          pszNetworkAddress,
                          &pszCanonicalHostName);
        BAIL_ON_ERROR(dwError);

        dwError = EventLogAllocateStringPrintf(
                          &pszServerPrincipalName,
                          "host/%s",
                          pszCanonicalHostName);
        BAIL_ON_ERROR(dwError);

        rpc_binding_set_auth_info(
                        pBinding,
                        pszServerPrincipalName,
                        rpc_c_protect_level_pkt_privacy,
                        rpc_c_authn_gss_negotiate,
                        NULL, /* Auth Identity */
                        rpc_c_authz_name,
                        &dwError);

        BAIL_ON_ERROR(dwError);
        VMEVENT_LOG_INFO("RpcBindingSetAuthInfo done.");
    }

    *ppBinding = pBinding;

cleanup:

    if (pszStringBinding)
    {
        DWORD dwError1 = 0;

        rpc_string_free((PBYTE*)&pszStringBinding, &dwError1);
    }
    VMEVENT_SAFE_FREE_STRINGA(pszServerPrincipalName);
    VMEVENT_SAFE_FREE_STRINGA(pszCanonicalHostName);

    return dwError;

error:

    if (pBinding)
    {
        DWORD dwError1 = 0;

        rpc_binding_free(&pBinding, &dwError1);
    }

    if (ppBinding)
    {
        *ppBinding = NULL;
    }

    goto cleanup;
}

DWORD
EventLogFreeBindingHandle(
    handle_t * pBindingHandle
    )
{
    DWORD dwError = 0;

    rpc_binding_free(pBindingHandle, &dwError);

    return dwError;
}

static
BOOLEAN
EventLogIsLocalHost(
    PCSTR pszInputHostname
    )
{
    DWORD    dwError = 0;
    BOOLEAN  bResult = FALSE;
    PSTR     pszLocalHostname = NULL;
    PSTR     pszLocalHostnameCanon = NULL;
    PSTR     pszInputHostnameCanon = NULL;
    PCSTR    pszLocalName = NULL;
    PCSTR    pszInputName = NULL;
#ifdef _WIN32
    WSADATA wsaData = {0};
    BOOLEAN bWsaStartup = FALSE;
#endif

    if ( !strcmp(pszInputHostname, "localhost") ||
         !strcmp(pszInputHostname, "127.0.0.1") )
    {
        bResult = TRUE;
    }
    else
    {
#ifdef _WIN32
        dwError = WSAStartup(MAKEWORD(2, 2), &wsaData);
        BAIL_ON_ERROR(dwError);
        bWsaStartup = TRUE;
#endif
        // Convert the local host name to its canonical form and check against
        // the canonical form of the input host name
        dwError = EventLogGetHostName(&pszLocalHostname);
        BAIL_ON_ERROR(dwError);

        dwError = EventLogGetCanonicalHostName(
            pszLocalHostname, &pszLocalHostnameCanon);

        pszLocalName = dwError ? &pszLocalHostname[0] : pszLocalHostnameCanon;

        dwError = EventLogGetCanonicalHostName(
                          pszInputHostname,
                          &pszInputHostnameCanon);

        pszInputName = dwError ? pszInputHostname : pszInputHostnameCanon;

        bResult = !EventLogStringCompareA(pszLocalName, pszInputName, FALSE);
    }

cleanup:

   VMEVENT_SAFE_FREE_STRINGA(pszLocalHostname);
   VMEVENT_SAFE_FREE_STRINGA(pszLocalHostnameCanon);
   VMEVENT_SAFE_FREE_STRINGA(pszInputHostnameCanon);

#ifdef _WIN32
    if( bWsaStartup != FALSE )
    {
        WSACleanup();
    }
#endif

    return bResult;

error:

   bResult = FALSE;

   goto cleanup;
}
