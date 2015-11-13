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

#define VMDNS_ADMINISTRATORS_GROUP "cn=Administrators,cn=Builtin"

static BOOL
VmDnsIsMemberOf(
    PSTR* ppszMemberships,
    DWORD dwMemberships,
    PCSTR pszGroupName
    );

static
VOID
VmDnsFreeMemberShips(
    PSTR* ppszMemberships,
    DWORD dwMemberships
    );

DWORD
VmDnsLdapAccessCheck(
    PCSTR szAuthPrinc,
    VMDNS_USER_TYPE userType
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_DIR_CONTEXT pLd = NULL;
    PSTR* ppszMemberships = NULL;
    PSTR szGroupName = NULL;
    PSTR pszDomainName = NULL;
    DWORD dwMemberships = 0;

    if (IsNullOrEmptyString(szAuthPrinc) ||
        (userType != VMDNS_ADMINISTRATORS))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsDirConnect("localhost", &pLd);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsLdapGetMemberships(
                    pLd,
                    szAuthPrinc,
                    &ppszMemberships,
                    &dwMemberships
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsGetDefaultDomainName(pLd, &pszDomainName);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringPrintfA(
                &szGroupName,
                "%s,%s",
                VMDNS_ADMINISTRATORS_GROUP,
                pszDomainName);
    BAIL_ON_VMDNS_ERROR(dwError);

    VMDNS_LOG_INFO("Checking upn: %s against DNS admin group: %s ",
                    szGroupName, szAuthPrinc);

    if (!VmDnsIsMemberOf(
                    ppszMemberships,
                    dwMemberships,
                    szGroupName
                    ))
    {
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:
    VMDNS_SAFE_FREE_STRINGA(pszDomainName);
    VMDNS_SAFE_FREE_STRINGA(szGroupName);

    if (ppszMemberships)
    {
        VmDnsFreeMemberShips(ppszMemberships, dwMemberships);
        ppszMemberships = NULL;
    }

    if (pLd)
    {
        VmDnsDirClose(pLd);
        pLd = NULL;
    }

    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsCheckAccess(
    handle_t IDL_handle,
    BOOL bNeedAdminPrivilage
    )
{
    DWORD dwError = 0;
    rpc_authz_cred_handle_t hPriv = { 0 };
    DWORD dwProtectLevel = 0;
    ULONG rpc_status = rpc_s_ok;
    unsigned char *authPrinc = NULL;

    rpc_binding_inq_auth_caller(
                IDL_handle,
                &hPriv,
                &authPrinc,
                &dwProtectLevel,
                NULL, /* unsigned32 *authn_svc, */
                NULL, /* unsigned32 *authz_svc, */
                &rpc_status);

    /* Deny if connection is not encrypted */
    if (dwProtectLevel < rpc_c_protect_level_pkt_privacy)
    {
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    /* Deny if no auth identity is provided.  */
    if (rpc_status == rpc_s_binding_has_no_auth || !authPrinc || !*authPrinc)
    {
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (bNeedAdminPrivilage)
    {
        dwError = VmDnsLdapAccessCheck(authPrinc, VMDNS_ADMINISTRATORS);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

error:
    if (authPrinc)
    {
        rpc_string_free((unsigned_char_t **)&authPrinc, &rpc_status);
    }

    return dwError;
}

static
BOOL
VmDnsIsMemberOf(
    PSTR* ppszMemberships,
    DWORD dwMemberships,
    PCSTR pszGroupName
    )
{
    BOOL bRetVal = FALSE;
    int nCmpRet = 0;
    DWORD i = 0;

    for(i = 0; i < dwMemberships; ++i)
    {
        nCmpRet = VmDnsStringCompareA(ppszMemberships[i], pszGroupName, FALSE);
        if (nCmpRet == 0)
        {
            bRetVal = TRUE;
            break;
        }
    }

    return bRetVal;
}

static
VOID
VmDnsFreeMemberShips(
    PSTR* ppszMemberships,
    DWORD dwMemberships
    )
{
    DWORD i = 0;
    for(i = 0; i < dwMemberships; ++i)
    {
        VMDNS_SAFE_FREE_STRINGA(ppszMemberships[i]);
    }

    VMDNS_SAFE_FREE_MEMORY(ppszMemberships);
}
