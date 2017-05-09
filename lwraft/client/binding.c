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

#ifdef _WIN32
#define snprintf _snprintf
#endif

static
DWORD
VmDirRpcCreateSrpAuthIdentity(
    PCSTR user,
    PCSTR domain,
    PCSTR password,
    PSTR *retUpn,
    rpc_auth_identity_handle_t *rpc_identity_h)
{
    OM_uint32 min = 0;
    OM_uint32 maj = 0;
    const gss_OID_desc gss_srp_password_oid =
        {GSSAPI_SRP_CRED_OPT_PW_LEN, (void *) GSSAPI_SRP_CRED_OPT_PW};
    const gss_OID_desc spnego_mech_oid =
        {SPNEGO_OID_LENGTH, (void *) SPNEGO_OID};
    gss_buffer_desc name_buf = {0};
    gss_name_t gss_name_buf = NULL;
    gss_buffer_desc gss_pwd = {0};
    size_t upn_len = 0;
    char *upn = NULL;
    gss_cred_id_t cred_handle = NULL;
    gss_OID_desc mech_oid_array[1];
    gss_OID_set_desc desired_mech = {0};

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
        snprintf(upn, upn_len, "%s@%s", user, domain);
    }
    else
    {
        /* Assume a UPN-like name form when no domain is provided */
        upn = strdup((char *) user);
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

    gss_pwd.value = (char *) password;
    gss_pwd.length = strlen(gss_pwd.value);
    maj = gss_set_cred_option(
              &min,
              &cred_handle,
              (gss_OID) &gss_srp_password_oid,
              &gss_pwd);
    if (maj)
    {
        goto error;
    }

    *retUpn = upn;
    upn = NULL;
    *rpc_identity_h = (rpc_auth_identity_handle_t) cred_handle;
    cred_handle = NULL;

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

    return (DWORD) maj;
}

/*
 * Refactor VmDirCreateBindingHandleA() / VmDirCreateBindingHandleNoauthA() /
 * VmDirCreateBindingHandleAuthA() into a single core utility function.
 *
 * TODO cleanup, pszUserName could be username or UPN format depends on pszDomain input
 * per implementation of VmDirRpcCreateSrpAuthIdentity. This is awkward and confusion.
 */
static
ULONG
VmDirCreateBindingHandleUtilityA(
    PCSTR     pszNetworkAddress,
    PCSTR     pszNetworkEndpoint,
    PCSTR     pszUserName,
    PCSTR     pszDomain,
    PCSTR     pszPassword,
    BOOL      bBindAuth,
    handle_t* ppBinding
    )
{
    ULONG ulError = 0;
    struct
    {
      PCSTR pszProtocolSequence;
      PCSTR pszEndPoint;
    }
    connection_info[] =
    {
        {
            VMDIR_SF_INIT(.pszProtocolSequence, "ncalrpc"),
            VMDIR_SF_INIT(.pszEndPoint, LWRAFT_NCALRPC_END_POINT),
        },
        {
            VMDIR_SF_INIT(.pszProtocolSequence, "ncacn_ip_tcp"),
            VMDIR_SF_INIT(.pszEndPoint, LWRAFT_RPC_TCP_END_POINT),
        }
    };
    PCSTR pszProtocolSequence = NULL;
    PCSTR pszEndpoint      = NULL;
    PSTR  pszStringBinding = NULL;
    BOOLEAN  bLocalHost = FALSE;
    handle_t pBinding   = NULL;
    PSTR  pszServerPrincipalName = NULL;
    PSTR  pszCanonicalHostName   = NULL;
    rpc_auth_identity_handle_t rpc_identity_h = NULL;

    if (!ppBinding)
    {
        ulError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(ulError);
    }
    if (!pszNetworkAddress)
    {
        pszNetworkAddress = "localhost";
        pszNetworkEndpoint = NULL;
    }

    bLocalHost = VmDirIsLocalHost(pszNetworkAddress);
    if (bLocalHost)
    {
#if defined(_WIN32) && !defined(HAVE_DCERPC_WIN32)  /* TBD: NCACN_IP_TCP needs pszNetworkAddress */
        /* on windows no hostname should be there in the binding
        for localhost case*/
        pszNetworkAddress = NULL;
#endif
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
            &ulError);
    BAIL_ON_VMDIR_ERROR(ulError);

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL,
                      "VmDirCreateBindingHandleUtilityA, string binding (%s)",
                      VDIR_SAFE_STRING(pszStringBinding));

    rpc_binding_from_string_binding(
            (PBYTE)pszStringBinding,
            &pBinding,
            &ulError);
    BAIL_ON_VMDIR_ERROR(ulError);

    if (pszUserName && pszPassword)
    {
        /* Setup SRP auth handle */
        ulError = VmDirRpcCreateSrpAuthIdentity(
                      pszUserName,
                      pszDomain,
                      pszPassword,
                      &pszServerPrincipalName,
                      &rpc_identity_h);
        BAIL_ON_VMDIR_ERROR(ulError);

        if (bBindAuth)
        {
            rpc_binding_set_auth_info(
                    pBinding,
                    pszServerPrincipalName,         /* Server Principal Name */
                    VMDIR_RPC_PROTECT_LEVEL_PKT_PRIVACY,
                    VMDIR_RPC_AUTHN_GSS_NEGOTIATE,
                    rpc_identity_h,                 /* Auth Identity */
                    VMDIR_RPC_AUTHZN_NAME,
                    &ulError);
            BAIL_ON_VMDIR_ERROR(ulError);
        }
    }

    *ppBinding = pBinding;

cleanup:

    if (pszStringBinding)
    {
        ULONG ulError1 = 0;

        ulError1 = VmDirRpcFreeString(&pszStringBinding);
    }

    VMDIR_SAFE_FREE_MEMORY( pszServerPrincipalName );
    VMDIR_SAFE_FREE_MEMORY( pszCanonicalHostName) ;

    return ulError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirCreateBindingHandleUtilityA, (%s)(%s) failed (%u)",
                                          VDIR_SAFE_STRING(pszCanonicalHostName),
                                          VDIR_SAFE_STRING(pszServerPrincipalName),
                                          ulError);

    if (pBinding)
    {
        VmDirFreeBindingHandle(&pBinding);
    }

    if (ppBinding)
    {
        *ppBinding = NULL;
    }

    goto cleanup;
}

ULONG
VmDirCreateBindingHandleA(
    PCSTR     pszNetworkAddress,
    PCSTR     pszNetworkEndpoint,
    handle_t* ppBinding
    )
{
    ULONG ulError = 0;


    ulError = VmDirCreateBindingHandleUtilityA(
              pszNetworkAddress,
              pszNetworkEndpoint,
              NULL,
              NULL,
              NULL,
              TRUE,
              ppBinding);
    return ulError;

}

/****************************************************
** FIXME: this is a temporary function for allowing
** unauthenticated RPC calls.  It must be removed
** when SRP authentication is available for RPC.
*****************************************************
**/
ULONG
VmDirCreateBindingHandleNoauthA(
    PCSTR     pszNetworkAddress,
    PCSTR     pszNetworkEndpoint,
    handle_t* ppBinding
    )
{
    ULONG ulError = 0;

    ulError = VmDirCreateBindingHandleUtilityA(
              pszNetworkAddress,
              pszNetworkEndpoint,
              NULL,
              NULL,
              NULL,
              FALSE,
              ppBinding);
    return ulError;
}

/*
 * Implement binding using user/domain/pass; only SRP supported now
 */
ULONG
VmDirCreateBindingHandleAuthA(
    PCSTR     pszNetworkAddress,
    PCSTR     pszNetworkEndpoint,
    PCSTR     pszUserName,
    PCSTR     pszDomain,
    PCSTR     pszPassword,
    handle_t  *ppBinding
    )
{
    ULONG ulError = 0;

    ulError = VmDirCreateBindingHandleUtilityA(
              pszNetworkAddress,
              pszNetworkEndpoint,
              pszUserName,
              pszDomain,
              pszPassword,
              TRUE,
              ppBinding);
    return ulError;
}

ULONG
VmDirCreateBindingHandleW(
    PCWSTR     pwszNetworkAddress,
    PCWSTR     pwszNetworkEndpoint,
    handle_t* ppBinding
    )
{
    DWORD dwError = 0;
    PSTR pszNetworkAddress = NULL;
    PSTR pszNetworkEndpoint = NULL;

    dwError = VmDirAllocateStringAFromW(pwszNetworkAddress,
                                        &pszNetworkAddress);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAFromW(pwszNetworkEndpoint,
                                        &pszNetworkEndpoint);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateBindingHandleA(pszNetworkAddress,
                                        pszNetworkEndpoint,
                                        ppBinding);
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    VMDIR_SAFE_FREE_MEMORY(pszNetworkAddress);
    VMDIR_SAFE_FREE_MEMORY(pszNetworkEndpoint);

    return dwError;
}

ULONG
VmDirCreateBindingHandleMachineAccountA(
    PCSTR     pszNetworkAddress,
    PCSTR     pszNetworkEndpoint,
    handle_t* ppBinding
    )
{
    DWORD dwError = 0;
    handle_t hBinding = NULL;
    PSTR pszDCAccount = NULL;
    PSTR pszPassword = NULL;
    PSTR pszDomainName = NULL;

    dwError = VmDirRegReadDCAccount(&pszDCAccount);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirReadDCAccountPassword(&pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetDomainName(pszNetworkAddress, &pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateBindingHandleAuthA(
                    pszNetworkAddress,
                    pszNetworkEndpoint,
                    pszDCAccount,
                    pszDomainName,
                    pszPassword,
                    &hBinding);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppBinding = hBinding;

cleanup:

    VMDIR_SAFE_FREE_STRINGA(pszDCAccount);
    VMDIR_SAFE_FREE_STRINGA(pszPassword);
    VMDIR_SAFE_FREE_STRINGA(pszDomainName);

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "VmDirCreateBindingHandleMachineAccountA failed (%u)",
                     dwError);

    goto cleanup;
}

VOID
VmDirFreeBindingHandle(
    handle_t * ppBinding
    )
{
    if (ppBinding)
    {
        ULONG ulError = 0;

        ulError = VmDirRpcFreeBinding(ppBinding);
    }
}

BOOLEAN
VmDirIsLocalHost(
   PCSTR pszHostname
   )
{
    ULONG    ulError = 0;
    BOOLEAN  bResult = FALSE;
    CHAR     localHostName[HOST_NAME_MAX] = {0};
    PSTR     pszLocalHostnameCanon = NULL;
    PSTR     pszInputHostnameCanon = NULL;
    PCSTR    pszLocalName = NULL;
    PCSTR    pszInputName = NULL;
#ifdef _WIN32
    WSADATA wsaData = {0};
    BOOLEAN bWsaStartup = FALSE;

#endif

    if ( !VmDirStringCompareA(pszHostname, "localhost", FALSE) ||
         !VmDirStringCompareA(pszHostname, "127.0.0.1", TRUE) ||
         !VmDirStringCompareA(pszHostname, "::1", TRUE) )
    {
        bResult = TRUE;
    }
    else
    {
#ifdef _WIN32
        ulError = WSAStartup(MAKEWORD(2, 2), &wsaData);
        BAIL_ON_VMDIR_ERROR(ulError);
        bWsaStartup = TRUE;
#endif
        // Convert the local host name to its canonical form and check against
        // the canonical form of the input host name
        ulError = VmDirGetHostName(localHostName, sizeof(localHostName) - 1);
        BAIL_ON_VMDIR_ERROR(ulError);

        ulError = VmDirGetCanonicalHostName(
            localHostName, &pszLocalHostnameCanon);

        pszLocalName = ulError ? &localHostName[0] : pszLocalHostnameCanon;

        ulError = VmDirGetCanonicalHostName(
                    pszHostname,
                    &pszInputHostnameCanon);

        pszInputName = ulError ? pszHostname : pszInputHostnameCanon;

        bResult = !VmDirStringCompareA(pszLocalName, pszInputName, FALSE);
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszLocalHostnameCanon);
    VMDIR_SAFE_FREE_MEMORY(pszInputHostnameCanon);

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

static
DWORD
_VmDirCloseServerInternalH(
    PVMDIR_SERVER_CONTEXT pServerContext
    )
{
    DWORD dwError = 0;

    if (!pServerContext)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pServerContext->hBinding)
    {
        rpc_binding_free(&pServerContext->hBinding, &dwError);
        pServerContext->hBinding = NULL;
    }
    free(pServerContext);
error:

    return dwError;
}

static
DWORD
__VmDirOpenServerA(
    PCSTR pszNetworkAddress,
    PCSTR pszUserName,
    PCSTR pszDomainName,
    PCSTR pszPassword,
    PVMDIR_SERVER_CONTEXT *ppServerContext
    )
{
    DWORD dwError = 0;
    handle_t hBinding = NULL;
    PVMDIR_SERVER_CONTEXT pServerContext = NULL;
    BOOLEAN bBindingAuth = TRUE;

    dwError = VmDirAllocateMemory(sizeof(*pServerContext),
                                 (PVOID) &pServerContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateBindingHandleUtilityA(
                  pszNetworkAddress,
                  NULL,
                  pszUserName,
                  pszDomainName,
                  pszPassword,
                  bBindingAuth,
                  &hBinding);
    BAIL_ON_VMDIR_ERROR(dwError);

    pServerContext->hBinding = hBinding, hBinding = NULL;
    *ppServerContext = pServerContext, pServerContext = NULL;

error:
    if (dwError)
    {
        _VmDirCloseServerInternalH(pServerContext);
    }
    return dwError;
}

DWORD
VmDirOpenServerA(
    PCSTR pszNetworkAddress,
    PCSTR pszUserName,
    PCSTR pszDomain,
    PCSTR pszPassword,
    DWORD dwFlags,
    PVOID pReserved,
    PVMDIR_SERVER_CONTEXT *ppServerContext
    )
{
    DWORD dwError = 0;
    PVMDIR_SERVER_CONTEXT pServerContext = NULL;

    dwError = __VmDirOpenServerA(
                  pszNetworkAddress,
                  pszUserName,
                  pszDomain,
                  pszPassword,
                  &pServerContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppServerContext = pServerContext;
error:
    return dwError;
}

DWORD
VmDirOpenServerW(
    PCWSTR pwszNetworkAddress,
    PCWSTR pwszUserName,
    PCWSTR pwszDomain,
    PCWSTR pwszPassword,
    DWORD dwFlags,
    PVOID pReserved,
    PVMDIR_SERVER_CONTEXT *ppServerContext
    )
{
    DWORD dwError = 0;
    PSTR pszNetworkAddress = NULL;
    PSTR pszUserName = NULL;
    PSTR pszDomainName = NULL;
    PSTR pszPassword = NULL;
    PVMDIR_SERVER_CONTEXT pServerContext = NULL;

    dwError = VmDirAllocateStringAFromW(
                    pwszNetworkAddress,
                    &pszNetworkAddress);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAFromW(pwszUserName, &pszUserName);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* Assume username is a UPN format when domain is NULL */
    if (pszDomainName)
    {
        dwError = VmDirAllocateStringAFromW(pwszDomain, &pszDomainName);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringAFromW(pwszPassword, &pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = __VmDirOpenServerA(
                  pszNetworkAddress,
                  pszUserName,
                  pszDomainName,
                  pszPassword,
                  &pServerContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppServerContext = pServerContext;

error:
    VMDIR_SAFE_FREE_MEMORY(pszNetworkAddress);
    if (dwError)
    {
        _VmDirCloseServerInternalH(pServerContext);
    }
    return dwError;
}


DWORD
VmDirCloseServer(
    PVMDIR_SERVER_CONTEXT pServerContext
    )
{
    DWORD dwError = 0;

    dwError = _VmDirCloseServerInternalH(pServerContext);
    return dwError;
}
