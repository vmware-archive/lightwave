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
VmDirValidateUserCreateParamsA(
    PVMDIR_USER_CREATE_PARAMS_A pCreateParams
    );

static
DWORD
VmDirValidateUserCreateParamsW(
    PVMDIR_USER_CREATE_PARAMS_W pCreateParams
    );

DWORD
VmDirCreateUser(
    PSTR pszUserName,
    PSTR pszPassword,
    PSTR pszUPNName,
    BOOLEAN bRandKey
    )
{
    DWORD dwError = 0;
    PCSTR pszServerName = "localhost";
    PCSTR pszServerEndpoint = NULL;
    PWSTR pwszUserName = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszUPNName = NULL;
    handle_t hBinding = NULL;

    if (
         IsNullOrEmptyString(pszUserName)
      || IsNullOrEmptyString(pszUPNName)
      || ( IsNullOrEmptyString(pszPassword) && !bRandKey )
       )
    {
        dwError =  ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirCreateBindingHandleMachineAccountA(
                    pszServerName,
                    pszServerEndpoint,
                    &hBinding
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringWFromA(
                        pszUserName,
                        &pwszUserName
                        );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!bRandKey)
    {
       dwError = VmDirAllocateStringWFromA(
                           pszPassword,
                           &pwszPassword
                           );
       BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringWFromA(
                        pszUPNName,
                        &pwszUPNName
                        );
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_RPC_TRY
    {
        dwError = RpcVmDirCreateUser(
                        hBinding,
                        pwszUserName,
                        (bRandKey ? NULL : pwszPassword),
                        pwszUPNName,
                        (unsigned char)bRandKey
                        );
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pwszUserName);
    VMDIR_SAFE_FREE_MEMORY(pwszPassword);
    VMDIR_SAFE_FREE_MEMORY(pwszUPNName);
    return dwError;
error:

    if (hBinding)
    {
        VmDirFreeBindingHandle(&hBinding);
    }

    goto cleanup;
}

DWORD
VmDirCreateUserA(
    PVMDIR_SERVER_CONTEXT       pServerContext,
    PVMDIR_USER_CREATE_PARAMS_A pCreateParams
    )
{
    DWORD dwError = 0;
    PVMDIR_USER_CREATE_PARAMS_W pCreateParamsW = NULL;

    if (!pServerContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirValidateUserCreateParamsA(pCreateParams);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateUserCreateParamsWFromA(pCreateParams, &pCreateParamsW);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateUserW(pServerContext, pCreateParamsW);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    if (pCreateParamsW)
    {
        VmDirFreeUserCreateParamsW(pCreateParamsW);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDirCreateUserW(
    PVMDIR_SERVER_CONTEXT       pServerContext,
    PVMDIR_USER_CREATE_PARAMS_W pCreateParams
    )
{
    DWORD dwError = 0;
    VMDIR_USER_CREATE_PARAMS_RPC createParams = {0};

    if (!pServerContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirValidateUserCreateParamsW(pCreateParams);
    BAIL_ON_VMDIR_ERROR(dwError);

    createParams.pwszName      = pCreateParams->pwszName;
    createParams.pwszAccount   = pCreateParams->pwszAccount;
    createParams.pwszFirstname = pCreateParams->pwszFirstname;
    createParams.pwszLastname  = pCreateParams->pwszLastname;
    createParams.pwszPassword  = pCreateParams->pwszPassword;
    createParams.pwszUPN       = pCreateParams->pwszUPN;

    VMDIR_RPC_TRY
    {
        dwError = RpcVmDirCreateUserEx(pServerContext->hBinding, &createParams);
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

error:

     return dwError;
}

DWORD _VmDirFindUserDN(
    LDAP *pLd,
    PCSTR pszUserUPN,
    PSTR *ppszUserDN
    )
{
    DWORD dwError = 0;
    LDAPMessage *searchRes = NULL;
    LDAPMessage *entry = NULL;
    PSTR pszSearchFilter = NULL;
    PSTR pszUserDN = NULL;
    PSTR pszDN = NULL;

    dwError = VmDirAllocateStringPrintf(
                &pszSearchFilter,
                "(%s=%s)",
                ATTR_KRB_UPN,
                pszUserUPN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_search_ext_s(pLd,
                                "",
                                LDAP_SCOPE_SUBTREE,
                                pszSearchFilter,
                                NULL,
                                TRUE,
                                NULL,
                                NULL,
                                NULL,
                                0,
                                &searchRes);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (ldap_count_entries(pLd, searchRes) != 1)
    {
        dwError = ERROR_INVALID_DATA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    entry = ldap_first_entry(pLd, searchRes);
    if (!entry)
    {
        dwError = ERROR_INVALID_ENTRY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pszDN = ldap_get_dn(pLd, entry);
    if (IsNullOrEmptyString(pszDN))
    {
        dwError = ERROR_INVALID_DN;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringA(pszDN, &pszUserDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszUserDN = pszUserDN;
    pszUserDN = NULL;

cleanup:
    ldap_memfree(pszDN);
    ldap_msgfree(searchRes);
    VMDIR_SAFE_FREE_MEMORY(pszSearchFilter);
    VMDIR_SAFE_FREE_MEMORY(pszUserDN);
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirSetPassword(
    PCSTR pszHostName,
    PCSTR pszAdminUPN,
    PCSTR pszAdminPassword,
    PCSTR pszUserUPN,
    PCSTR pszNewPassword
    )
{
    DWORD       dwError = 0;

    LDAP*       pLd = NULL;
    LDAPMod     mod = {0};
    LDAPMod*    mods[2] = {&mod, NULL};
    PSTR        vals[2] = {(PSTR)pszNewPassword, NULL};
    PSTR        pszUserDN = NULL;

    if (IsNullOrEmptyString(pszHostName) ||
        IsNullOrEmptyString(pszAdminUPN) ||
        IsNullOrEmptyString(pszAdminPassword) ||
        IsNullOrEmptyString(pszUserUPN) ||
        IsNullOrEmptyString(pszNewPassword))
    {
        dwError =  ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSafeLDAPBind(
                &pLd,
                pszHostName,
                pszAdminUPN,
                pszAdminPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirFindUserDN(
                pLd,
                pszUserUPN,
                &pszUserDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    mod.mod_op = LDAP_MOD_REPLACE;
    mod.mod_type = ATTR_USER_PASSWORD;
    mod.mod_vals.modv_strvals = vals;

    dwError = ldap_modify_ext_s(
                            pLd,
                            pszUserDN,
                            mods,
                            NULL,
                            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszUserDN);
    if (pLd)
    {
        ldap_unbind_ext_s(pLd, NULL, NULL);
    }
    return dwError;

error:
    VmDirLog(LDAP_DEBUG_TRACE, "VmDirSetPassword failed with error (%u)\n", dwError);
    goto cleanup;
}

DWORD
VmDirChangePassword(
    PCSTR pszHostName,
    PCSTR pszUserUPN,
    PCSTR pszOldPassword,
    PCSTR pszNewPassword)
{
    DWORD       dwError = 0;

    LDAP*       pLd = NULL;
    LDAPMod     mod[2] = {{0}};
    LDAPMod*    mods[3] = {&mod[0], &mod[1], NULL};
    PSTR        vals_new[2] = {(PSTR)pszNewPassword, NULL};
    PSTR        vals_old[2] = {(PSTR)pszOldPassword, NULL};
    PSTR        pszUserDN = NULL;

    if (IsNullOrEmptyString(pszHostName) ||
        IsNullOrEmptyString(pszUserUPN) ||
        IsNullOrEmptyString(pszOldPassword) ||
        IsNullOrEmptyString(pszNewPassword))
    {
        dwError =  ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSafeLDAPBind(
                &pLd,
                pszHostName,
                pszUserUPN,
                pszOldPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirFindUserDN(
                pLd,
                pszUserUPN,
                &pszUserDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    mod[0].mod_op = LDAP_MOD_ADD;
    mod[0].mod_type = ATTR_USER_PASSWORD;
    mod[0].mod_vals.modv_strvals = vals_new;

    mod[1].mod_op = LDAP_MOD_DELETE;
    mod[1].mod_type = ATTR_USER_PASSWORD;
    mod[1].mod_vals.modv_strvals = vals_old;

    dwError = ldap_modify_ext_s(
                            pLd,
                            pszUserDN,
                            mods,
                            NULL,
                            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszUserDN);
    if (pLd)
    {
        ldap_unbind_ext_s(pLd, NULL, NULL);
    }
    return dwError;

error:
    VmDirLog(LDAP_DEBUG_TRACE, "VmDirChangePassword failed with error (%u)\n", dwError);
    goto cleanup;
}

DWORD
VmDirCreateService(
    PCSTR pszSvcname,          /* IN              */
    PCSTR pszPassword,          /* IN     OPTIONAL */
    PSTR* ppszUPN,          /*    OUT OPTIONAL */
    PSTR* ppszPassword      /*    OUT OPTIONAL */
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMDIR_INVALID_POINTER(pszSvcname, dwError);
    BAIL_ON_VMDIR_INVALID_POINTER(pszPassword, dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDirValidateUserCreateParamsA(
    PVMDIR_USER_CREATE_PARAMS_A pCreateParams
    )
{
    DWORD dwError = 0;

    if (!pCreateParams ||
        IsNullOrEmptyString(pCreateParams->pszAccount) ||
        IsNullOrEmptyString(pCreateParams->pszFirstname) ||
        IsNullOrEmptyString(pCreateParams->pszLastname) ||
        IsNullOrEmptyString(pCreateParams->pszPassword))
    {
        dwError = ERROR_INVALID_PARAMETER;
    }

    return dwError;
}

static
DWORD
VmDirValidateUserCreateParamsW(
    PVMDIR_USER_CREATE_PARAMS_W pCreateParams
    )
{
    DWORD dwError = 0;

    if (!pCreateParams ||
        IsNullOrEmptyString(pCreateParams->pwszAccount) ||
        IsNullOrEmptyString(pCreateParams->pwszFirstname) ||
        IsNullOrEmptyString(pCreateParams->pwszLastname) ||
        IsNullOrEmptyString(pCreateParams->pwszPassword))
    {
        dwError = ERROR_INVALID_PARAMETER;
    }

    return dwError;
}

