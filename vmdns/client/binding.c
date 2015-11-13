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
BOOLEAN
VmDnsIsLocalHost(
    PCSTR pszHostname
    );

static
DWORD
VmDnsGetCanonicalHostName(
    PCSTR pszHostname,
    PSTR* ppszCanonicalHostName
    );

static
DWORD
VmDnsGetHostName(
    PSTR* pszHostName
);

static
DWORD
VmDnsGetAddrInfo(
    PCSTR pszHostname,
    struct addrinfo** ppResult
);

static
DWORD
VmDnsGetNameInfo(
    const struct sockaddr* pSockaddr,
    socklen_t sockaddrLength,
    PCHAR pHostName,
    DWORD dwBufferSize
);

static
DWORD
VmDnsRpcCreateSrpAuthIdentity(
    PCSTR user,
    PCSTR domain,
    PCSTR password,
    PSTR *retUpn,
    rpc_auth_identity_handle_t *rpc_identity_h
    )
{
    OM_uint32 min = 0;
    OM_uint32 maj = 0;
    const gss_OID_desc gss_srp_password_oid = { GSSAPI_SRP_CRED_OPT_PW_LEN,
                                                (void *) GSSAPI_SRP_CRED_OPT_PW};
    const gss_OID_desc spnego_mech_oid = { SPNEGO_OID_LENGTH, (void *)SPNEGO_OID };
    gss_buffer_desc name_buf = { 0 };
    gss_name_t gss_name_buf = NULL;
    gss_buffer_desc gss_pwd = { 0 };
    size_t upn_len = 0;
    char *upn = NULL;
    gss_cred_id_t cred_handle = NULL;
    gss_OID_desc mech_oid_array[1];
    gss_OID_set_desc desired_mech = { 0 };

    if (domain)
    {
        /* user@DOMAIN\0 */
        upn_len = strlen(user) + 1 + strlen(domain) + 1;
        upn = calloc(upn_len, sizeof(char));
        if (!upn)
        {
            maj = GSS_S_FAILURE;
            min = ENOMEM;
        }
#ifndef _WIN32
        snprintf(upn, upn_len, "%s@%s", user, domain);
#else
        _snprintf_s(upn, upn_len, upn_len, "%s@%s", user, domain);
#endif
    }
    else
    {
        /* Assume a UPN-like name form when no domain is provided */
        upn = strdup((char *)user);
        if (!upn)
        {
            maj = GSS_S_FAILURE;
            min = ENOMEM;
        }
    }
    name_buf.value = upn;
    name_buf.length = strlen(name_buf.value);
    maj = gss_import_name(
        &min,
        &name_buf,
        GSS_C_NT_USER_NAME,
        &gss_name_buf);
    if (maj)
    {
        goto error;
    }

    /*
    * Hard code desired mech OID to SRP
    */
    desired_mech.count = 1;
    desired_mech.elements = mech_oid_array;
    desired_mech.elements[0] = spnego_mech_oid;
    maj = gss_acquire_cred(
        &min,
        gss_name_buf,
        0,
        &desired_mech,
        GSS_C_INITIATE,
        &cred_handle,
        NULL,
        NULL);
    if (maj)
    {
        goto error;
    }

    gss_pwd.value = (char *)password;
    gss_pwd.length = strlen(gss_pwd.value);
    maj = gss_set_cred_option(
        &min,
        &cred_handle,
        (gss_OID)&gss_srp_password_oid,
        &gss_pwd);
    if (maj)
    {
        goto error;
    }

    *retUpn = upn;
    upn = NULL;
    *rpc_identity_h = (rpc_auth_identity_handle_t)cred_handle;

error:
    if (maj)
    {
        maj = min ? min : maj;
    }

    if (upn)
    {
        free(upn);
    }
    if (gss_name_buf)
    {
        gss_release_name(&min, &gss_name_buf);
    }

    return (DWORD)maj;
}

DWORD
VmDnsCreateBindingHandleA(
    PCSTR     pszNetworkAddress,
    PCSTR     pszNetworkEndpoint,
    PCSTR     pszUserName,
    PCSTR     pszDomain,
    PCSTR     pszPassword,
    handle_t* ppInBinding
    )
{
    ULONG ulError = 0;
    PCSTR pszProtocolSequence = NULL;
    PCSTR pszEndpoint = NULL;
    PSTR  pszStringBinding = NULL;
    BOOLEAN  bLocalHost = FALSE;
    handle_t* ppBinding = (handle_t *)ppInBinding;
    handle_t pBinding = NULL;
    PSTR  pszServerPrincipalName = NULL;
    PSTR  pszCanonicalHostName = NULL;
    rpc_auth_identity_handle_t rpc_identity_h = NULL;

    struct
    {
        PCSTR pszProtocolSequence;
        PCSTR pszEndPoint;
    }
    connection_info[] =
    {
        {
            VMDNS_SF_INIT(.pszProtocolSequence, "ncalrpc"),
            VMDNS_SF_INIT(.pszEndPoint, VMDNS_NCALRPC_END_POINT),
        },
        {
            VMDNS_SF_INIT(.pszProtocolSequence, "ncacn_ip_tcp"),
            VMDNS_SF_INIT(.pszEndPoint, VMDNS_RPC_TCP_END_POINT),
        }
    };

    if (!ppBinding || !pszNetworkAddress)
    {
        ulError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(ulError);
    }

    bLocalHost = VmDnsIsLocalHost(pszNetworkAddress);
    if (bLocalHost)
    {
        pszNetworkAddress = NULL;
        pszProtocolSequence = connection_info[0].pszProtocolSequence;
        pszEndpoint = connection_info[0].pszEndPoint;
    }
    else
    {
        pszProtocolSequence = connection_info[1].pszProtocolSequence;
        pszEndpoint = pszNetworkEndpoint ? pszNetworkEndpoint :
                                           connection_info[1].pszEndPoint;
    }

    rpc_string_binding_compose(
                    NULL,
                    (PBYTE)pszProtocolSequence,
                    (PBYTE)pszNetworkAddress,
                    (PBYTE)pszEndpoint,
                    NULL,
                    (PBYTE*)&pszStringBinding,
                    &ulError);
    BAIL_ON_VMDNS_ERROR(ulError);

    rpc_binding_from_string_binding(
                    (PBYTE)pszStringBinding,
                    &pBinding,
                    &ulError);
    BAIL_ON_VMDNS_ERROR(ulError);

    if (!IsNullOrEmptyString(pszUserName) &&
        !IsNullOrEmptyString(pszPassword))
    {
        /* Setup SRP auth handle */
        ulError = VmDnsRpcCreateSrpAuthIdentity(
                            pszUserName,
                            pszDomain,
                            pszPassword,
                            &pszServerPrincipalName,
                            &rpc_identity_h);
        BAIL_ON_VMDNS_ERROR(ulError);
    }

    if (rpc_identity_h != NULL)
    {
        rpc_binding_set_auth_info(
                            pBinding,
                            pszServerPrincipalName,         /* Server Principal Name */
                            VMDNS_RPC_PROTECT_LEVEL_PKT_PRIVACY,
                            VMDNS_RPC_AUTHN_GSS_NEGOTIATE,
                            rpc_identity_h,                 /* Auth Identity */
                            VMDNS_RPC_AUTHZN_NAME,
                            &ulError);
        BAIL_ON_VMDNS_ERROR(ulError);
    }

    *ppBinding = pBinding;

cleanup:

    if (pszStringBinding)
    {
        ULONG ulError1 = 0;

        ulError1 = VmDnsRpcFreeString(&pszStringBinding);
    }

    VMDNS_SAFE_FREE_MEMORY(pszServerPrincipalName);
    VMDNS_SAFE_FREE_MEMORY(pszCanonicalHostName);

    return ulError;

error:

    if (pBinding)
    {
        VmDnsFreeBindingHandle(&pBinding);
    }

    if (ppBinding)
    {
        *ppBinding = NULL;
    }

    goto cleanup;
}

DWORD
VmDnsCreateBindingHandleW(
    PCWSTR pwszNetworkAddress,
    PCWSTR pwszNetworkEndpoint,
    PCWSTR pwszUserName,
    PCWSTR pwszDomain,
    PCWSTR pwszPassword,
    handle_t * ppBinding
    )
{
    PSTR pszNetworkAddress = NULL;
    PSTR pszNetworkEndpoint = NULL;
    PSTR pszUserName = NULL;
    PSTR pszDomain = NULL;
    PSTR pszPassword = NULL;

    DWORD dwError = 0;

    if (pwszNetworkAddress)
    {
        dwError = VmDnsAllocateStringAFromW(
                            pwszNetworkAddress,
                            &pszNetworkAddress);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (pwszNetworkEndpoint)
    {
        dwError = VmDnsAllocateStringAFromW(
                            pwszNetworkEndpoint,
                            &pszNetworkEndpoint);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (pwszUserName)
    {
        dwError = VmDnsAllocateStringAFromW(
                            pwszUserName,
                            &pszUserName);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (pwszDomain)
    {
        dwError = VmDnsAllocateStringAFromW(
                            pwszDomain,
                            &pszDomain);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (pwszPassword)
    {
        dwError = VmDnsAllocateStringAFromW(
                            pwszPassword,
                            &pszPassword);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCreateBindingHandleA(
                            pszNetworkAddress,
                            pszNetworkEndpoint,
                            pszUserName,
                            pszDomain,
                            pszPassword,
                            ppBinding);

cleanup:

    VMDNS_SAFE_FREE_MEMORY(pszNetworkAddress);
    VMDNS_SAFE_FREE_MEMORY(pszNetworkEndpoint);
    VMDNS_SAFE_FREE_MEMORY(pszUserName);
    VMDNS_SAFE_FREE_MEMORY(pszDomain);
    VMDNS_SAFE_FREE_MEMORY(pszPassword);

    return dwError;

error:

    goto cleanup;

}

VOID
VmDnsFreeBindingHandle(
    handle_t * ppBinding
    )
{
    if (ppBinding)
    {
        DWORD dwError = 0;

        dwError = VmDnsRpcFreeBinding(ppBinding);
    }
}

static
BOOLEAN
VmDnsIsLocalHost(
   PCSTR pszHostname
   )
{
    ULONG    ulError = 0;
    BOOLEAN  bResult = FALSE;
    PSTR     localHostName = NULL;
    PSTR     pszLocalHostnameCanon = NULL;
    PSTR     pszInputHostnameCanon = NULL;
    PCSTR    pszLocalName = NULL;
    PCSTR    pszInputName = NULL;
#ifdef _WIN32
    WSADATA wsaData = { 0 };
    BOOLEAN bWsaStartup = FALSE;

#endif

    if (!VmDnsStringCompareA(pszHostname, "localhost", FALSE) ||
        !VmDnsStringCompareA(pszHostname, "127.0.0.1", TRUE) ||
        !VmDnsStringCompareA(pszHostname, "::1", TRUE))
    {
        bResult = TRUE;
    }
    else
    {
#ifdef _WIN32
        ulError = WSAStartup(MAKEWORD(2, 2), &wsaData);
        BAIL_ON_VMDNS_ERROR(ulError);
        bWsaStartup = TRUE;
#endif
        // Convert the local host name to its canonical form and check against
        // the canonical form of the input host name
        ulError = VmDnsGetHostName(&localHostName);
        BAIL_ON_VMDNS_ERROR(ulError);

        ulError = VmDnsGetCanonicalHostName(localHostName, &pszLocalHostnameCanon);

        pszLocalName = ulError ? &localHostName[0] : pszLocalHostnameCanon;

        ulError = VmDnsGetCanonicalHostName(
                            pszHostname,
                            &pszInputHostnameCanon);

        pszInputName = ulError ? pszHostname : pszInputHostnameCanon;

        bResult = !VmDnsStringCompareA(pszLocalName, pszInputName, FALSE);
    }

cleanup:

    VMDNS_SAFE_FREE_MEMORY(pszLocalHostnameCanon);
    VMDNS_SAFE_FREE_MEMORY(pszInputHostnameCanon);
    VMDNS_SAFE_FREE_MEMORY(localHostName);

#ifdef _WIN32
    if (bWsaStartup != FALSE)
    {
        WSACleanup();
    }
#endif

    return bResult;

error:

    bResult = FALSE;

    goto cleanup;
}

static
DWORD
VmDnsGetCanonicalHostName(
    PCSTR pszHostname,
    PSTR* ppszCanonicalHostname
    )
{
    DWORD  dwError = 0;
    struct addrinfo* pHostInfo = NULL;
    CHAR   szCanonicalHostname[NI_MAXHOST+1] = "";
    PSTR   pszCanonicalHostname = NULL;

    dwError = VmDnsGetAddrInfo(pszHostname, &pHostInfo);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsGetNameInfo(
                    pHostInfo->ai_addr,
                    (socklen_t)(pHostInfo->ai_addrlen),
                    szCanonicalHostname,
                    NI_MAXHOST);
    BAIL_ON_VMDNS_ERROR(dwError);

    if (!IsNullOrEmptyString(&szCanonicalHostname[0]))
    {
        dwError = VmDnsAllocateStringA(
                    &szCanonicalHostname[0],
                    &pszCanonicalHostname);
    }
    else
    {
        dwError = ERROR_NO_DATA;
    }
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppszCanonicalHostname = pszCanonicalHostname;

cleanup:

    if (pHostInfo)
    {
        freeaddrinfo(pHostInfo);
    }

    return dwError;

error:

    *ppszCanonicalHostname = NULL;

    VMDNS_SAFE_FREE_MEMORY(pszCanonicalHostname);

    goto cleanup;
}

DWORD
VmDnsGetHostName(
    PSTR *ppszHostName
    )
{
    DWORD dwError = ERROR_SUCCESS;
    char hostBuf[NI_MAXHOST + 1];
    DWORD dwBufLen = sizeof(hostBuf)-1;
    PSTR pszHostName = NULL;

#ifdef _WIN32
    WSADATA wsaData = {0};
    BOOLEAN bWsaStartup = FALSE;
#endif

#ifndef _WIN32
    if (gethostname(hostBuf, dwBufLen) < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
    }
#else

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        dwError = WSAGetLastError();
    }
    else
    {
        /*
        MSDN:
        If no error occurs, gethostname returns zero.
        Otherwise, it returns SOCKET_ERROR and a specific error code
        can be retrieved by calling WSAGetLastError.
        */
        if (gethostname(hostBuf, dwBufLen) != 0)
        {
            dwError = WSAGetLastError();
        }

        WSACleanup();
    }

#endif

    dwError = VmDnsAllocateStringA(hostBuf, &pszHostName);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppszHostName = pszHostName;
error:
    return dwError;
}


DWORD
VmDnsGetAddrInfo(
  PCSTR pszHostname,
  struct addrinfo** ppHostInfo
)
{
    DWORD dwError = ERROR_SUCCESS;
#ifndef _WIN32
    dwError = getaddrinfo(pszHostname, NULL, NULL, ppHostInfo);
#else
    /*
    MSDN:
    Success returns zero. Failure returns a nonzero Windows Sockets error code,
    as found in the Windows Sockets Error Codes.

    It is recommended that the WSA error codes be used, as they offer familiar
    and comprehensive error information for Winsock programmers.
    */
    if( getaddrinfo(pszHostname, NULL, NULL, ppHostInfo) != 0 )
    {
        dwError = WSAGetLastError();
    }
#endif
    return dwError;
}

DWORD
VmDnsGetNameInfo(
    const struct sockaddr*     pSockaddr,
    socklen_t           sockaddrLength,
    PCHAR               pHostName,
    DWORD               dwBufferSize
)
{
    DWORD dwError = ERROR_SUCCESS;
#ifndef _WIN32
    dwError = getnameinfo(
                pSockaddr,
                sockaddrLength,
                pHostName,
                dwBufferSize,
                NULL,
                0,
                0);
#else
    /*
    MSDN:
    On success, getnameinfo returns zero. Any nonzero return value indicates
    failure and a specific error code can be retrieved
    by calling WSAGetLastError.
    */
    if ( getnameinfo(
             pSockaddr, sockaddrLength, pHostName, dwBufferSize,
             NULL, 0,0 ) != 0 )
    {
        dwError = WSAGetLastError();
    }
#endif
    return dwError;
}

