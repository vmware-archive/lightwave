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

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 256
#endif

static
BOOLEAN
VMCAIsLocalHost(
   PCSTR pszHostname
   );


DWORD
VMCARpcCreateSrpAuthIdentity(
    PCSTR user,
    PCSTR domain,
    PCSTR password,
    PSTR *retUpn,
    rpc_auth_identity_handle_t *rpc_identity_h
    )
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
 * Refactor VMCACreateBindingHandleA() / VMCACreateBindingHandleNoauthA() /
 * VMCACreateBindingHandleAuthA() into a single core utility function.
 */
static ULONG
VMCACreateBindingHandleUtilityA(
    PCSTR     pszNetworkAddress,
    PCSTR     pszNetworkEndpoint,
    PCSTR     pszUserName,
    PCSTR     pszDomain,
    PCSTR     pszPassword,
    BOOL      bBindAuth,
    handle_t* ppInBinding
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
            VMCA_SF_INIT(.pszProtocolSequence, "ncalrpc"),
            VMCA_SF_INIT(.pszEndPoint, VMCA_NCALRPC_END_POINT),
        },
        {
            VMCA_SF_INIT(.pszProtocolSequence, "ncacn_ip_tcp"),
            VMCA_SF_INIT(.pszEndPoint, VMCA_RPC_TCP_END_POINT),
        }
    };
    PCSTR pszProtocolSequence = NULL;
    PCSTR pszEndpoint      = NULL;
    PSTR  pszStringBinding = NULL;
    BOOLEAN  bLocalHost = FALSE;
    handle_t* ppBinding = (handle_t *) ppInBinding;
    handle_t pBinding   = NULL;
    PSTR  pszServerPrincipalName = NULL;
    PSTR  pszCanonicalHostName   = NULL;
    rpc_auth_identity_handle_t rpc_identity_h = NULL;

    if (!ppBinding || !pszNetworkAddress)
    {
        ulError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(ulError);
    }

    bLocalHost = VMCAIsLocalHost(pszNetworkAddress);
    if (bLocalHost)
    {
        pszNetworkAddress = NULL;
        pszProtocolSequence = connection_info[0].pszProtocolSequence;

        pszEndpoint = connection_info[0].pszEndPoint;
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
    BAIL_ON_VMCA_ERROR(ulError);

    VMCA_LOG_VERBOSE("VMCACreateBindingHandleUtilityA, string binding (%s)", pszStringBinding);

    rpc_binding_from_string_binding(
            (PBYTE)pszStringBinding,
            &pBinding,
            &ulError);
    BAIL_ON_VMCA_ERROR(ulError);

    if (!IsNullOrEmptyString(pszUserName) &&
        !IsNullOrEmptyString(pszPassword))
    {
        /* Setup SRP auth handle */
        ulError = VMCARpcCreateSrpAuthIdentity(
                      pszUserName,
                      pszDomain,
                      pszPassword,
                      &pszServerPrincipalName,
                      &rpc_identity_h);
        BAIL_ON_VMCA_ERROR(ulError);
    }
#ifdef VMCA_KRB_AUTHENTICATION
    else
    {
        DWORD dwCanonNameLen = 0;
        ulError = VMCAGetCanonicalHostName(
                      pszNetworkAddress,
                      &pszCanonicalHostName);
        BAIL_ON_VMCA_ERROR(ulError);

#if 1
        /*
         * Not worth porting VmDir string.c module to VMCA just for
         * VMCAAllocateStringPrintf()
         */
        dwCanonNameLen = (DWORD) strlen(pszCanonicalHostName) +
                             sizeof("host/") + 1;
        ulError = VMCAAllocateMemory(
                      dwCanonNameLen,
                      (PVOID) &pszServerPrincipalName);
        BAIL_ON_VMCA_ERROR(ulError);
        snprintf(pszServerPrincipalName,
                 dwCanonNameLen,
                 "host/%s",
                 pszCanonicalHostName);
#else
        ulError = VMCAAllocateStringPrintf(&pszServerPrincipalName,
                                            "host/%s",
                                            pszCanonicalHostName);
#endif
        BAIL_ON_VMCA_ERROR(ulError);

        VMCA_LOG_VERBOSE("VMCACreateBindingHandleUtilityA, SPN (%s)",
                         pszServerPrincipalName);
    }
#endif

    if (rpc_identity_h != NULL)
    {
        rpc_binding_set_auth_info(
                pBinding,
                pszServerPrincipalName,         /* Server Principal Name */
                VMCA_RPC_PROTECT_LEVEL_PKT_PRIVACY,
                VMCA_RPC_AUTHN_GSS_NEGOTIATE,
                rpc_identity_h,                 /* Auth Identity */
                VMCA_RPC_AUTHZN_NAME,
                &ulError);
        BAIL_ON_VMCA_ERROR(ulError);
    }

    *ppBinding = pBinding;

cleanup:

    if (pszStringBinding)
    {
        ULONG ulError1 = 0;

        ulError1 = VMCARpcFreeString(&pszStringBinding);
    }

    VMCA_SAFE_FREE_MEMORY( pszServerPrincipalName );
    VMCA_SAFE_FREE_MEMORY( pszCanonicalHostName) ;

    return ulError;

error:

    VMCA_LOG_ERROR("VMCACreateBindingHandleUtilityA, (%s)(%s) failed (%u)",
                   pszCanonicalHostName,
                   pszServerPrincipalName,
                   ulError);

    if (pBinding)
    {
        VMCAFreeBindingHandle(&pBinding);
    }

    if (ppBinding)
    {
        *ppBinding = NULL;
    }

    goto cleanup;
}

DWORD
CreateBindingHandleSharedKeyA(
    PCSTR pszNetworkAddress,
    PCSTR pszNetworkEndpoint,
    handle_t * pBindingHandleSharedKey
    )
{

    DWORD dwError = 0;
    DWORD dwTempError = 0;
    PCSTR pszProtocolSequence = NULL;
    PCSTR pszEndpoint    = NULL;
    unsigned char* pszStringBinding    = NULL;
    BOOLEAN  bLocalHost = FALSE;
    handle_t BindingHandle = NULL;

    struct
    {
      PCSTR pszProtocolSequence;
      PCSTR pszEndPoint;
    }
    connection_info[] =
    {
        {
            VMCA_SF_INIT(.pszProtocolSequence, "ncalrpc"),
            VMCA_SF_INIT(.pszEndPoint, VMCA_NCALRPC_END_POINT),
        }
    };

    bLocalHost = VMCAIsLocalHost(pszNetworkAddress);
    if (!bLocalHost)
    {
        goto error;
    }

    /* on windows no hostname should be there in the binding
    for localhost case*/
    pszNetworkAddress = NULL;
    pszProtocolSequence = connection_info[0].pszProtocolSequence;

    pszEndpoint = pszNetworkEndpoint ?  pszNetworkEndpoint :
                                        connection_info[0].pszEndPoint;


    rpc_string_binding_compose(
        NULL,
        (unsigned char*)pszProtocolSequence,
        (unsigned char*)pszNetworkAddress,
        (unsigned char*)pszEndpoint,
        NULL,
        &pszStringBinding,
        &dwError
    );
    BAIL_ON_ERROR(dwError);

    rpc_binding_from_string_binding(
        pszStringBinding,
        &BindingHandle,
        &dwError
    );
    BAIL_ON_ERROR(dwError);

    *pBindingHandleSharedKey = BindingHandle;

cleanup:

    if (pszStringBinding) {
        rpc_string_free(&pszStringBinding, &dwTempError);
    }

    return dwError;

error:
    if (BindingHandle)
    {
        rpc_binding_free(&BindingHandle, &dwError);
    }
    goto cleanup;
}

DWORD
CreateBindingHandleSrpA(
    PCSTR pszNetworkAddress,
    PCSTR pszNetworkEndpoint,
    handle_t * ppBinding
    )
{
    DWORD dwError = 0;
    PSTR pszAccount = NULL;
    PSTR pszPassword = NULL;
    PSTR pszUpn = NULL;

    dwError = VMCAGetMachineAccountInfoA(
                  &pszAccount,
                  &pszPassword);
    if (dwError == 0)
    {
        dwError = VMCAAccountDnToUpn(pszAccount, &pszUpn);
        if (dwError != 0)
        {
            /* Don't use registry entries if UPN conversion fails */
            VMCA_SAFE_FREE_MEMORY(pszAccount);
            VMCA_SAFE_FREE_MEMORY(pszUpn);
        }
    }

    dwError = VMCACreateBindingHandleUtilityA(
                  pszNetworkAddress,
                  pszNetworkEndpoint,
                  pszUpn,
                  NULL,
                  pszPassword,
                  TRUE,
                  ppBinding);
    VMCA_SAFE_FREE_MEMORY(pszAccount);
    VMCA_SAFE_FREE_MEMORY(pszPassword);
    VMCA_SAFE_FREE_MEMORY(pszUpn);
    return dwError;
}

/*
 * TBD: Note CreateBindingHandleKrb() always generates an SRP binding handle
 * in this implementation.  This will change in the future to support both
 * KRB5/SRP transparently.
 */
DWORD
CreateBindingHandleKrbA(
    PCSTR pszNetworkAddress,
    PCSTR pszNetworkEndpoint,
    handle_t * ppBinding
    )
{
    ULONG ulError = 0;

    /*
     * TBD: Adam- Force CreateBindingHandleKrb() to create an SRP
     * authenticate binding handle for this release. This is the minimum
     * change that can be easily reverted.
     */

#if 1
    ulError = CreateBindingHandleSrpA(
                  pszNetworkAddress,
                  pszNetworkEndpoint,
                  ppBinding);
#else
    ulError = VMCACreateBindingHandleUtilityA(
                  pszNetworkAddress,
                  pszNetworkEndpoint,
                  NULL,
                  NULL,
                  NULL,
                  TRUE,
                  ppBinding);
#endif
    return ulError;
}

DWORD
CreateBindingHandleKrbW(
    PCWSTR pwszNetworkAddress,
    PCWSTR pwszNetworkEndpoint,
    handle_t * ppBinding
    )
{
    DWORD dwError = 0;
    PSTR pszNetworkAddress = NULL;
    PSTR pszNetworkEndpoint = NULL;

    if (pwszNetworkAddress)
    {
        dwError = VMCAAllocateStringAFromW(
                        pwszNetworkAddress,
                        &pszNetworkAddress);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (pwszNetworkEndpoint)
    {
        dwError = VMCAAllocateStringAFromW(
                        pwszNetworkEndpoint,
                        &pszNetworkEndpoint);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = CreateBindingHandleKrbA(
                        pszNetworkAddress,
                        pszNetworkEndpoint,
                        ppBinding);

    BAIL_ON_VMCA_ERROR(dwError);

error:

    VMCA_SAFE_FREE_MEMORY(pszNetworkAddress);
    VMCA_SAFE_FREE_MEMORY(pszNetworkEndpoint);

    return dwError;
 }

DWORD
CreateBindingHandleSharedKeyW(
    PCWSTR pwszNetworkAddress,
    PCWSTR pwszNetworkEndpoint,
    handle_t * pBindingHandleSharedKey
    )
{
    DWORD dwError = 0;
    PSTR pszNetworkAddress = NULL;
    PSTR pszNetworkEndpoint = NULL;

    if (pwszNetworkAddress)
    {
        dwError = VMCAAllocateStringAFromW(
                        pwszNetworkAddress,
                        &pszNetworkAddress);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (pwszNetworkEndpoint)
    {
        dwError = VMCAAllocateStringAFromW(
                        pwszNetworkEndpoint,
                        &pszNetworkEndpoint);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = CreateBindingHandleSharedKeyA(
                        pszNetworkAddress,
                        pszNetworkEndpoint,
                        pBindingHandleSharedKey);
    BAIL_ON_VMCA_ERROR(dwError);
error:

    VMCA_SAFE_FREE_MEMORY(pszNetworkAddress);
    VMCA_SAFE_FREE_MEMORY(pszNetworkEndpoint);

    return dwError;
}


DWORD
CreateBindingHandleAuthW(
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
        dwError = VMCAAllocateStringAFromW(
                        pwszNetworkAddress,
                        &pszNetworkAddress);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (pwszNetworkEndpoint)
    {
        dwError = VMCAAllocateStringAFromW(
                        pwszNetworkEndpoint,
                        &pszNetworkEndpoint);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (pwszUserName)
    {
        dwError = VMCAAllocateStringAFromW(
                        pwszUserName,
                        &pszUserName);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (pwszDomain)
    {
        dwError = VMCAAllocateStringAFromW(
                        pwszDomain,
                        &pszDomain);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (pwszPassword)
    {
        dwError = VMCAAllocateStringAFromW(
                        pwszPassword,
                        &pszPassword);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = CreateBindingHandleAuthA(
                        pszNetworkAddress,
                        pszNetworkEndpoint,
                        pszUserName,
                        pszDomain,
                        pszPassword,
                        ppBinding);

error:

    VMCA_SAFE_FREE_MEMORY(pszNetworkAddress);
    VMCA_SAFE_FREE_MEMORY(pszNetworkEndpoint);
    VMCA_SAFE_FREE_MEMORY(pszUserName);
    VMCA_SAFE_FREE_MEMORY(pszDomain);
    VMCA_SAFE_FREE_MEMORY(pszPassword);

    return dwError;
}

DWORD
CreateBindingHandleAuthA(
    PCSTR pszNetworkAddress,
    PCSTR pszNetworkEndpoint,
    PCSTR pszUserName,
    PCSTR pszDomain,
    PCSTR pszPassword,
    handle_t *ppBinding
    )
{
    ULONG ulError = 0;

    ulError = VMCACreateBindingHandleUtilityA(
                  pszNetworkAddress,
                  pszNetworkEndpoint,
                  pszUserName,
                  pszDomain,
                  pszPassword,
                  TRUE,
                  ppBinding);
    return ulError;
}

DWORD
_VMCACloseServerInternalH(
    PVMCA_SERVER_CONTEXT pServerContext
    )
{
    DWORD dwError = 0;

    if (!pServerContext)
    {
        dwError = VMCA_ARGUMENT_ERROR;
        BAIL_ON_ERROR(dwError);
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

static DWORD
_VMCAOpenServerAInternal(
    PCSTR pszNetworkAddress,
    PCSTR pszUserName,
    PCSTR pszDomainName,
    PCSTR pszPassword,
    PVMCA_SERVER_CONTEXT *ppServerContext
    )
{
    DWORD dwError = 0;
    handle_t hBinding = NULL;
    PVMCA_SERVER_CONTEXT pServerContext = NULL;
    BOOLEAN bBindingAuth = FALSE;

    dwError = VMCAAllocateMemory(sizeof(*pServerContext),
                                 (PVOID) &pServerContext);
    BAIL_ON_VMCA_ERROR(dwError);

    /*
     * Authenticated RPC is possible, so set the bBindingAuth flag
     * TBD: Is this right for Kerberos?
     */
    if (!IsNullOrEmptyString(pszUserName) &&
        !IsNullOrEmptyString(pszPassword))
    {
        bBindingAuth = TRUE;
    }
    dwError = VMCACreateBindingHandleUtilityA(
                  pszNetworkAddress,
                  VMCA_RPC_TCP_END_POINT,
                  pszUserName,
                  pszDomainName,
                  pszPassword,
                  bBindingAuth,
                  &hBinding);
    BAIL_ON_VMCA_ERROR(dwError);

    pServerContext->hBinding = hBinding, hBinding = NULL;
    *ppServerContext = pServerContext, pServerContext = NULL;

error:
    if (dwError)
    {
        _VMCACloseServerInternalH(pServerContext);
    }
    return dwError;
}


DWORD
VMCAOpenServerA(
    PCSTR pszNetworkAddress,
    PCSTR pszUserName,
    PCSTR pszDomain,
    PCSTR pszPassword,
    DWORD dwFlags,
    PVOID pReserved,
    PVMCA_SERVER_CONTEXT *ppServerContext
    )
{
    DWORD dwError = 0;
    PVMCA_SERVER_CONTEXT pServerContext = NULL;

    dwError = _VMCAOpenServerAInternal(
                  pszNetworkAddress,
                  pszUserName,
                  pszDomain,
                  pszPassword,
                  &pServerContext);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppServerContext = pServerContext;
error:
    return dwError;
}

DWORD
VMCAOpenServerW(
    PCWSTR pwszNetworkAddress,
    PCWSTR pwszUserName,
    PCWSTR pwszDomain,
    PCWSTR pwszPassword,
    DWORD dwFlags,
    PVOID pReserved,
    PVMCA_SERVER_CONTEXT *ppServerContext
    )
{
    DWORD dwError = 0;
    PSTR pszNetworkAddress = NULL;
    PSTR pszUserName = NULL;
    PSTR pszDomainName = NULL;
    PSTR pszPassword = NULL;
    PVMCA_SERVER_CONTEXT pServerContext = NULL;

    dwError = VMCAAllocateStringAFromW(
                    pwszNetworkAddress,
                    &pszNetworkAddress);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAAllocateStringAFromW(pwszUserName, &pszUserName);
    BAIL_ON_ERROR(dwError);

    /* Assume username is a UPN format when domain is NULL */
    if (pszDomainName)
    {
        dwError = VMCAAllocateStringAFromW(pwszDomain, &pszDomainName);
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCAAllocateStringAFromW(pwszPassword, &pszPassword);
    BAIL_ON_ERROR(dwError);

    dwError = _VMCAOpenServerAInternal(
                  pszNetworkAddress,
                  pszUserName,
                  pszDomainName,
                  pszPassword,
                  &pServerContext);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppServerContext = pServerContext;

error:
    VMCA_SAFE_FREE_MEMORY(pszNetworkAddress);
    VMCA_SAFE_FREE_STRINGA(pszUserName);
    VMCA_SAFE_FREE_STRINGA(pszDomainName);
    VMCA_SAFE_FREE_STRINGA(pszPassword);

    if (dwError)
    {
        _VMCACloseServerInternalH(pServerContext);
    }
    return dwError;
}


DWORD
VMCACloseServer(
    PVMCA_SERVER_CONTEXT pServerContext
    )
{
    DWORD dwError = 0;

    dwError = _VMCACloseServerInternalH(pServerContext);
    return dwError;
}

DWORD
VMCAFreeBindingHandle(
    handle_t *pBindingHandle
    )
{
    DWORD dwError = 0;

    rpc_binding_free(pBindingHandle, &dwError);

    return dwError;
}

static
BOOLEAN
VMCAIsLocalHost(
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
    WSADATA wsaData = {0};
    BOOLEAN bWsaStartup = FALSE;

#endif

    if ( !VMCAStringCompareA(pszHostname, "localhost", FALSE) ||
         !VMCAStringCompareA(pszHostname, "127.0.0.1", TRUE) ||
         !VMCAStringCompareA(pszHostname, "::1", TRUE) )
    {
        bResult = TRUE;
    }
    else
    {
#ifdef _WIN32
        ulError = WSAStartup(MAKEWORD(2, 2), &wsaData);
        BAIL_ON_VMCA_ERROR(ulError);
        bWsaStartup = TRUE;
#endif
        // Convert the local host name to its canonical form and check against
        // the canonical form of the input host name
        ulError = VMCAGetHostName(&localHostName);
        BAIL_ON_VMCA_ERROR(ulError);

        ulError = VMCAGetCanonicalHostName(
            localHostName, &pszLocalHostnameCanon);

        pszLocalName = ulError ? &localHostName[0] : pszLocalHostnameCanon;

        ulError = VMCAGetCanonicalHostName(
                    pszHostname,
                    &pszInputHostnameCanon);

        pszInputName = ulError ? pszHostname : pszInputHostnameCanon;

        bResult = !VMCAStringCompareA(pszLocalName, pszInputName, FALSE);
    }

cleanup:

    VMCA_SAFE_FREE_MEMORY(pszLocalHostnameCanon);
    VMCA_SAFE_FREE_MEMORY(pszInputHostnameCanon);
    VMCA_SAFE_FREE_MEMORY(localHostName);

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
