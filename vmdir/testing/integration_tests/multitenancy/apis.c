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
_VmDirTestAdminConnectionFromDomain(
    PVMDIR_TEST_STATE pState,
    PCSTR pszDomain,
    LDAP **ppLd
    )
{
    DWORD dwError = 0;
    PSTR pszUserUPN = NULL;
    LDAP *pLd;

    dwError = VmDirAllocateStringPrintf(
                &pszUserUPN,
                "administrator@%s",
                pszDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSafeLDAPBind(
                &pLd,
                pState->pszServerName,
                pszUserUPN,
                pState->pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppLd = pLd;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszUserUPN);
    return dwError;
error:
    goto cleanup;
}

//
// Quick check to make sure that the tenant domain was not only created but
// looks good.
//
DWORD
SanityCheckTenantDomain(
    PVMDIR_TEST_STATE pState,
    PCSTR pszTenantName, // "foo.bar"
    PCSTR pszTenantDn // dc=foo,dc=bar
    )
{
    DWORD dwError = 0;
    LDAP *pLd = NULL;
    PSTR pszUserDn = NULL;
    PSTR pszUserSid = NULL;

    dwError = _VmDirTestAdminConnectionFromDomain(pState, pszTenantName, &pLd);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                &pszUserDn,
                "cn=Administrator,cn=Users,%s",
                pszTenantDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetAttributeValueString(
                pLd,
                pszUserDn,
                LDAP_SCOPE_BASE,
                "(objectclass=*)",
                "objectSid",
                &pszUserSid);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszUserDn);
    VMDIR_SAFE_FREE_STRINGA(pszUserSid);
    return dwError;
error:
    goto cleanup;
}

DWORD
ShouldBeAbleToCreateTenants(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;

    dwError = VmDirCreateTenant(
                pState->pszUserName,
                pState->pszPassword,
                "secondary.local",
                "administrator",
                pState->pszPassword);
    TestAssertEquals(dwError, 0);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = SanityCheckTenantDomain(pState, "secondary.local", "dc=secondary,dc=local");
    TestAssertEquals(dwError, 0);

    dwError = VmDirCreateTenant(
                pState->pszUserName,
                pState->pszPassword,
                "tertiary.com",
                "administrator",
                pState->pszPassword);
    TestAssertEquals(dwError, 0);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = SanityCheckTenantDomain(pState, "tertiary.com", "dc=tertiary,dc=com");
    TestAssertEquals(dwError, 0);

    dwError = VmDirCreateTenant(
                pState->pszUserName,
                pState->pszPassword,
                "quad.com",
                "administrator",
                pState->pszPassword);
    TestAssertEquals(dwError, 0);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = SanityCheckTenantDomain(pState, "quad.com", "dc=quad,dc=com");
    TestAssertEquals(dwError, 0);

cleanup:
    return dwError;
error:
    TestAssertEquals(dwError, 0);
    goto cleanup;
}

DWORD
ShouldNotBeAbleToCreateTenantsOfACertainLength(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;

    dwError = VmDirCreateTenant(
                pState->pszUserName,
                pState->pszPassword,
                "marketing.pepsi.com",
                "administrator",
                pState->pszPassword);
    TestAssertEquals(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
ShouldBeAbleToEnumerateTenants(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PSTR *ppszTenants = NULL;
    DWORD dwTenantCount = 0;

    dwError = VmDirEnumerateTenants(
                pState->pszUserName,
                pState->pszPassword,
                &ppszTenants,
                &dwTenantCount);
    TestAssert(dwError == 0);
    TestAssert(dwTenantCount == 4);
    VmDirFreeStringArray(ppszTenants, dwTenantCount);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
ShouldBeAbleToDeleteTenants(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;

    dwError = VmDirDeleteTenant(
                pState->pszUserName,
                pState->pszPassword,
                "secondary.local");
    TestAssertEquals(dwError, 0);

    dwError = VmDirDeleteTenant(
                pState->pszUserName,
                pState->pszPassword,
                "tertiary.com");
    TestAssertEquals(dwError, 0);

    dwError = VmDirDeleteTenant(
                pState->pszUserName,
                pState->pszPassword,
                "quad.com");
    TestAssertEquals(dwError, 0);

    return dwError;
}

VOID
NullParametersShouldFail(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PSTR *ppszTenants = NULL;
    DWORD dwTenantCount = 0;

    dwError = VmDirCreateTenant(NULL, pState->pszPassword, "domain.com", "administrator", pState->pszPassword);
    TestAssertEquals(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirCreateTenant(pState->pszUserName, NULL, "domain.com", "administrator", pState->pszPassword);
    TestAssertEquals(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirCreateTenant(pState->pszUserName, pState->pszPassword, NULL, "administrator", pState->pszPassword);
    TestAssertEquals(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirCreateTenant(pState->pszUserName, pState->pszPassword, "domain.com", NULL, pState->pszPassword);
    TestAssertEquals(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirCreateTenant(pState->pszUserName, pState->pszPassword, "domain.com", "administrator", NULL);
    TestAssertEquals(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirDeleteTenant(NULL, pState->pszPassword, "domain.com");
    TestAssertEquals(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirDeleteTenant(pState->pszUserName, NULL, "domain.com");
    TestAssertEquals(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirDeleteTenant(pState->pszUserName, pState->pszPassword, NULL);
    TestAssertEquals(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirEnumerateTenants(NULL, pState->pszPassword, &ppszTenants, &dwTenantCount);
    TestAssertEquals(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirEnumerateTenants(pState->pszUserName, NULL, &ppszTenants, &dwTenantCount);
    TestAssertEquals(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirEnumerateTenants(pState->pszUserName, pState->pszPassword, NULL, &dwTenantCount);
    TestAssertEquals(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirEnumerateTenants(pState->pszUserName, pState->pszPassword, &ppszTenants, NULL);
    TestAssertEquals(dwError, VMDIR_ERROR_INVALID_PARAMETER);
}

VOID
InvalidCredentialsShouldFail(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PSTR *ppszTenants = NULL;
    DWORD dwTenantCount = 0;

    dwError = VmDirCreateTenant("no_such_user@vsphere.local", pState->pszPassword, "domain.com", "administrator", pState->pszPassword);
    TestAssertEquals(dwError, VMDIR_ERROR_ENTRY_NOT_FOUND);

    dwError = VmDirCreateTenant(pState->pszUserName, "not the password", "domain.com", "administrator", pState->pszPassword);
    TestAssertEquals(dwError, VMDIR_ERROR_USER_INVALID_CREDENTIAL);

    dwError = VmDirDeleteTenant("no_such_user@vsphere.local", pState->pszPassword, "domaintodelete.com");
    TestAssertEquals(dwError, VMDIR_ERROR_ENTRY_NOT_FOUND);

    dwError = VmDirDeleteTenant(pState->pszUserName, "not the password", "domaintodelete.com");
    TestAssertEquals(dwError, VMDIR_ERROR_USER_INVALID_CREDENTIAL);

    dwError = VmDirEnumerateTenants("no_such_user@vsphere.local", pState->pszPassword, &ppszTenants, &dwTenantCount);
    TestAssertEquals(dwError, VMDIR_ERROR_ENTRY_NOT_FOUND);

    dwError = VmDirEnumerateTenants(pState->pszUserName, "not the password", &ppszTenants, &dwTenantCount);
    TestAssertEquals(dwError, VMDIR_ERROR_USER_INVALID_CREDENTIAL);
}
