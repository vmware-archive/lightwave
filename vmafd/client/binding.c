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
VmAfdRpcCreateSrpAuthIdentity(
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
        {VMAFD_SPNEGO_OID_LENGTH, (void *) VMAFD_SPNEGO_OID};
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
#ifdef _WIN32
#define snprintf _snprintf
#endif
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

DWORD
VmAfdCreateBindingHandleUtilityA(
    PCSTR     pszNetworkAddress,
    PCSTR     pszNetworkEndpoint,
    PCSTR     pszUserName,
    PCSTR     pszDomain,
    PCSTR     pszPassword,
    BOOL      bBindAuth,
    handle_t* ppBinding
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
            VMAFD_SF_INIT(.pszProtocolSequence, "ncalrpc"),
            VMAFD_SF_INIT(.pszEndPoint, VMAFD_NCALRPC_END_POINT),
        },
        {
            VMAFD_SF_INIT(.pszProtocolSequence, "ncacn_ip_tcp"),
            VMAFD_SF_INIT(.pszEndPoint, VMAFD_RPC_TCP_END_POINT),
        }
    };
    PCSTR pszProtocolSequence = NULL;
    PCSTR pszEndpoint      = NULL;
    PSTR  pszStringBinding = NULL;
    BOOLEAN  bLocalHost = FALSE;
    handle_t pBinding   = NULL;
    PSTR pszServerPrincipalName = NULL;
    rpc_auth_identity_handle_t rpc_identity_h = NULL;

    if (!ppBinding)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!pszNetworkAddress)
    {
        pszNetworkAddress = "localhost";
    }

    bLocalHost = VmAfdIsLocalHost(pszNetworkAddress);
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
    BAIL_ON_VMAFD_ERROR(dwError);

    rpc_binding_from_string_binding(
            (PBYTE)pszStringBinding,
            &pBinding,
            &dwError);
    BAIL_ON_VMAFD_ERROR(dwError);

    /* Set up authentication if we are connecting to a remote host */
    if (!bLocalHost)
    {
        if (pszUserName && pszPassword)
        {
            /* Setup SRP auth handle */
            dwError = VmAfdRpcCreateSrpAuthIdentity(
                          pszUserName,
                          NULL,
                          pszPassword,
                          &pszServerPrincipalName,
                          &rpc_identity_h);
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        if (bBindAuth)
        {
            rpc_binding_set_auth_info(
                    pBinding,
                    pszServerPrincipalName,
                    rpc_c_protect_level_pkt_privacy,
                    rpc_c_authn_gss_negotiate,
                    rpc_identity_h,
                    rpc_c_authz_name,
                    &dwError);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }

    *ppBinding = pBinding;

cleanup:

    if (pszStringBinding)
    {
        DWORD dwError1 = 0;

        rpc_string_free((PBYTE*)&pszStringBinding, &dwError1);
    }
    VMAFD_SAFE_FREE_STRINGA(pszServerPrincipalName);

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
VmAfdCreateBindingHandleA(
    PCSTR     pszNetworkAddress,
    PCSTR     pszNetworkEndpoint,
    handle_t* ppBinding
    )
{
    DWORD dwError = 0;

    dwError = VmAfdCreateBindingHandleUtilityA(
                  pszNetworkAddress,
                  pszNetworkEndpoint,
                  NULL,
                  NULL,
                  NULL,
                  TRUE,
                  ppBinding);

    return dwError;
}

/*
 * Implement binding using user/domain/pass; only SRP supported now
 */
DWORD
VmAfdCreateBindingHandleAuthA(
    PCSTR     pszNetworkAddress,
    PCSTR     pszNetworkEndpoint,
    PCSTR     pszUserName,
    PCSTR     pszDomain,
    PCSTR     pszPassword,
    handle_t  *ppBinding
    )
{
    DWORD dwError = 0;

    dwError = VmAfdCreateBindingHandleUtilityA(
                  pszNetworkAddress,
                  pszNetworkEndpoint,
                  pszUserName,
                  pszDomain,
                  pszPassword,
                  TRUE,
                  ppBinding);

    return dwError;
}

DWORD
VmAfdCreateBindingHandleW(
    PCWSTR     pwszNetworkAddress,
    PCWSTR     pwszNetworkEndpoint,
    handle_t* ppBinding
    )
{
    DWORD dwError = 0;
    PSTR pszNetworkAddress = NULL;
    PSTR pszNetworkEndpoint = NULL;

    if (pwszNetworkAddress)
    {
        dwError = VmAfdAllocateStringAFromW(
                          pwszNetworkAddress,
                          &pszNetworkAddress);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pwszNetworkEndpoint)
    {
        dwError = VmAfdAllocateStringAFromW(
                          pwszNetworkEndpoint,
                          &pszNetworkEndpoint);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdCreateBindingHandleA(
                      pszNetworkAddress,
                      pszNetworkEndpoint,
                      ppBinding);
    BAIL_ON_VMAFD_ERROR(dwError);

error:

    VMAFD_SAFE_FREE_MEMORY(pszNetworkAddress);
    VMAFD_SAFE_FREE_MEMORY(pszNetworkEndpoint);

    return dwError;
}

VOID
VmAfdFreeBindingHandle(
    handle_t * ppBinding
    )
{
    if (ppBinding)
    {
        DWORD dwError = 0;

        dwError = VmAfdRpcFreeBinding(ppBinding);
    }
}
