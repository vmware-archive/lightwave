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


DWORD
VmDirSrvInitializeTenant(
    PWSTR    pwszDomainName,
    PWSTR    pwszUsername,
    PWSTR    pwszPassword
    )
{
    DWORD dwError = 0;
    PSTR  pszDomainName = NULL;
    PSTR  pszUsername = NULL;
    PSTR  pszPassword = NULL;

    if (IsNullOrEmptyString(pwszDomainName) ||
        IsNullOrEmptyString(pwszUsername) ||
        IsNullOrEmptyString(pwszPassword))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringAFromW(pwszDomainName, &pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAFromW(pwszUsername, &pszUsername);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAFromW(pwszPassword, &pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateTenant(pszDomainName, pszUsername, pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "VmDirSrvInitializeTenant (%u)(%s)(%s)",
                                       dwError,
                                       VDIR_SAFE_STRING(pszDomainName),
                                       VDIR_SAFE_STRING(pszUsername));

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszDomainName);
    VMDIR_SAFE_FREE_MEMORY(pszUsername);
    VMDIR_SAFE_FREE_MEMORY(pszPassword);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirSrvInitializeTenant failed (%u)(%s)(%s)",
                                      dwError,
                                      VDIR_SAFE_STRING(pszDomainName),
                                      VDIR_SAFE_STRING(pszUsername));
    goto cleanup;
}

DWORD
VmDirSrvCreateTenant(
    PCSTR pszFQDomainName,
    PCSTR pszUsername,
    PCSTR pszPassword
    )
{
    DWORD dwError = 0;
    PSTR  pszDomainDN = NULL;
    PVDIR_SCHEMA_CTX pSchemaCtx = NULL;

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
                   "Setting up a tenant instance (%s).",
                   VDIR_SAFE_STRING(pszFQDomainName));

    dwError = VmDirDomainNameToDN(pszFQDomainName, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    // can not create tenant below and above system domain
    // e.g. if sytem tenant is "taipei.lw.local",
    // this function should fail pszDomainName 
    // "center.taipei.lw.local" || "taipei.lw.local" || "lw.local" || "local"
    if (VmDirStringEndsWith(pszDomainDN, gVmdirServerGlobals.systemDomainDN.lberbv_val, FALSE) ||
        VmDirStringEndsWith(gVmdirServerGlobals.systemDomainDN.lberbv_val, pszDomainDN, FALSE))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INSUFFICIENT_ACCESS);
    }

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvSetupDomainInstance(
                    pSchemaCtx,
                    FALSE,
                    FALSE,
                    pszFQDomainName,
                    pszDomainDN,
                    pszUsername,
                    pszPassword,
                    NULL,
                    NULL,
                    NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);

    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }

    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
        "%s attempt to create tenant %s failed, error (%u)",
        __FUNCTION__, VDIR_SAFE_STRING(pszFQDomainName), dwError);
    goto cleanup;
}

DWORD
VmDirSrvDeleteTenant(
    PCSTR pszDomainName
    )
{
    DWORD dwError = 0;
    PSTR pszDomainDn = NULL;
    VDIR_ENTRY_ARRAY entryArray = {0};
    int iIdx = 0;
    int iCnt = 0;

    dwError = VmDirFQDNToDN(pszDomainName, &pszDomainDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    // can not touch system domain subtree including its ancestor nodes.
    // e.g. if sytem tenant is "taipei.lw.local",
    // this function should fail pszDomainName "taipei.lw.local" || "lw.local" || "local"
    if (VmDirStringEndsWith(gVmdirServerGlobals.systemDomainDN.lberbv_val, pszDomainDn, FALSE))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INSUFFICIENT_ACCESS);
    }

    // ================================================================================
    // Tenant delete IPC makes sure the caller has system domain administrator password
    // and has root permission on this VM.
    // We need a more controlled way to enforce tenant deletion privilege.
    // ================================================================================
    dwError = VmDirFilterInternalSearch(pszDomainDn,
                                        LDAP_SCOPE_SUBTREE,
                                        "objectClass=*",
                                        0,
                                        NULL,
                                        &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (iCnt = 0, iIdx = (int)entryArray.iSize - 1; iCnt < entryArray.iSize; iCnt++, iIdx--)
    {
        dwError = VmDirDeleteEntry(&entryArray.pEntry[iIdx]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDomainDn);
    VmDirFreeEntryArrayContent(&entryArray);
    return dwError;
error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
        "%s attempt to delete tenant %s failed, error (%u)",
        __FUNCTION__, VDIR_SAFE_STRING(pszDomainName), dwError);

    goto cleanup;
}

static
DWORD
_VmDirSrvEnumerateDomains(
    PVMDIR_STRING_LIST pTenantList,
    PCSTR pszBaseDn
    )
{
    DWORD dwError = 0;
    VDIR_ENTRY_ARRAY entryArray = {0};
    int iCnt = 0;
    PSTR pszDomainName = NULL;

    dwError = VmDirFilterInternalSearch(pszBaseDn,
                                        LDAP_SCOPE_ONELEVEL,
                                        "objectClass=dcObject",
                                        0,
                                        NULL,
                                        &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (iCnt = 0; iCnt < entryArray.iSize; ++iCnt)
    {
        dwError = _VmDirSrvEnumerateDomains(
                    pTenantList,
                    entryArray.pEntry[iCnt].dn.lberbv.bv_val);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (entryArray.iSize == 0)
    {
        dwError = VmDirDomainDNToName(pszBaseDn, &pszDomainName);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirStringListAdd(pTenantList, pszDomainName);
        BAIL_ON_VMDIR_ERROR(dwError);

        pszDomainName = NULL;
    }

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDomainName);
    VmDirFreeEntryArrayContent(&entryArray);
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDirSrvEnumerateTenants(
    PVMDIR_STRING_LIST pTenantList
    )
{
    DWORD dwError = 0;

    dwError = _VmDirSrvEnumerateDomains(pTenantList, "");
    BAIL_ON_VMDIR_ERROR(dwError);
cleanup:
    return dwError;
error:
    goto cleanup;
}
