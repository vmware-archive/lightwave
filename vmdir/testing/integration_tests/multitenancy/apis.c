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
    VDIR_SAFE_UNBIND_EXT_S(pLd);
    return dwError;
error:
    goto cleanup;
}

//
// Test System Domain Administrator Permission in Tenant tree
// 1. It can read tenant top tree DC entry
// 2. It can not create object/container in tenant tree
// 3. It can not read object/container created by tenant admin in tenant tree
//
DWORD
TestSystemAdminTenantTreePermission(
    PVMDIR_TEST_STATE pState,
    PCSTR pszTenantName, // "foo.bar"
    PCSTR pszTenantDn // dc=foo,dc=bar
    )
{
    DWORD dwError = 0;
    LDAP *pLd = NULL;
    PSTR pszUserDn = NULL;
    PSTR pszContainerDn = NULL;

    dwError = _VmDirTestAdminConnectionFromDomain(pState, pszTenantName, &pLd);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                &pszUserDn,
                "cn=test-user-1-cn,%s",
                pszTenantDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                &pszContainerDn,
                "cn=test-container-1-cn,%s",
                pszTenantDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    // use system domain admin cred
    dwError = VmDirTestCreateSimpleUser(pState->pLd, "cn=test-user-1-cn", pszUserDn);
    if (dwError != LDAP_INSUFFICIENT_ACCESS)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_ACL_VIOLATION);
    }
    dwError = 0;

    dwError = VmDirTestCreateSimpleContainer(pState->pLd, "cn=test-container-1-cn", pszContainerDn);
    if (dwError != LDAP_INSUFFICIENT_ACCESS)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_ACL_VIOLATION);
    }
    dwError = 0;

    // use tenant domain admin cred
    dwError = VmDirTestCreateSimpleUser(pLd, "cn=test-user-1-cn", pszUserDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateSimpleContainer(pLd, "cn=test-container-1-cn", pszContainerDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    // system domain admin could not read created user/container
    if (VmDirTestCanReadSingleEntry(pState->pLd, pszUserDn) == TRUE)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_ACL_VIOLATION);
    }

    if (VmDirTestCanReadSingleEntry(pState->pLd, pszContainerDn) == TRUE)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_ACL_VIOLATION);
    }

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszUserDn);
    VMDIR_SAFE_FREE_STRINGA(pszContainerDn);

    VDIR_SAFE_UNBIND_EXT_S(pLd);
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
                pState->pszUserUPN,
                pState->pszPassword,
                "secondary.local",
                "administrator",
                pState->pszPassword);
    TestAssertEquals(dwError, 0);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = SanityCheckTenantDomain(pState, "secondary.local", "dc=secondary,dc=local");
    TestAssertEquals(dwError, 0);

    dwError = VmDirCreateTenant(
                pState->pszUserUPN,
                pState->pszPassword,
                "tertiary.com",
                "administrator",
                pState->pszPassword);
    TestAssertEquals(dwError, 0);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = SanityCheckTenantDomain(pState, "tertiary.com", "dc=tertiary,dc=com");
    TestAssertEquals(dwError, 0);

    dwError = VmDirCreateTenant(
                pState->pszUserUPN,
                pState->pszPassword,
                "quad.com",
                "administrator",
                pState->pszPassword);
    TestAssertEquals(dwError, 0);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = SanityCheckTenantDomain(pState, "quad.com", "dc=quad,dc=com");
    TestAssertEquals(dwError, 0);

    dwError = TestSystemAdminTenantTreePermission(pState, "quad.com", "dc=quad,dc=com");
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
                pState->pszUserUPN,
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
                pState->pszUserUPN,
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
                pState->pszUserUPN,
                pState->pszPassword,
                "secondary.local");
    TestAssertEquals(dwError, 0);

    dwError = VmDirDeleteTenant(
                pState->pszUserUPN,
                pState->pszPassword,
                "tertiary.com");
    TestAssertEquals(dwError, 0);

    dwError = VmDirDeleteTenant(
                pState->pszUserUPN,
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

    dwError = VmDirCreateTenant(pState->pszUserUPN, NULL, "domain.com", "administrator", pState->pszPassword);
    TestAssertEquals(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirCreateTenant(pState->pszUserUPN, pState->pszPassword, NULL, "administrator", pState->pszPassword);
    TestAssertEquals(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirCreateTenant(pState->pszUserUPN, pState->pszPassword, "domain.com", NULL, pState->pszPassword);
    TestAssertEquals(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirCreateTenant(pState->pszUserUPN, pState->pszPassword, "domain.com", "administrator", NULL);
    TestAssertEquals(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirDeleteTenant(NULL, pState->pszPassword, "domain.com");
    TestAssertEquals(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirDeleteTenant(pState->pszUserUPN, NULL, "domain.com");
    TestAssertEquals(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirDeleteTenant(pState->pszUserUPN, pState->pszPassword, NULL);
    TestAssertEquals(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirEnumerateTenants(NULL, pState->pszPassword, &ppszTenants, &dwTenantCount);
    TestAssertEquals(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirEnumerateTenants(pState->pszUserUPN, NULL, &ppszTenants, &dwTenantCount);
    TestAssertEquals(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirEnumerateTenants(pState->pszUserUPN, pState->pszPassword, NULL, &dwTenantCount);
    TestAssertEquals(dwError, VMDIR_ERROR_INVALID_PARAMETER);

    dwError = VmDirEnumerateTenants(pState->pszUserUPN, pState->pszPassword, &ppszTenants, NULL);
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

    dwError = VmDirCreateTenant(pState->pszUserUPN, "not the password", "domain.com", "administrator", pState->pszPassword);
    TestAssertEquals(dwError, VMDIR_ERROR_USER_INVALID_CREDENTIAL);

    dwError = VmDirDeleteTenant("no_such_user@vsphere.local", pState->pszPassword, "domaintodelete.com");
    TestAssertEquals(dwError, VMDIR_ERROR_ENTRY_NOT_FOUND);

    dwError = VmDirDeleteTenant(pState->pszUserUPN, "not the password", "domaintodelete.com");
    TestAssertEquals(dwError, VMDIR_ERROR_USER_INVALID_CREDENTIAL);

    dwError = VmDirEnumerateTenants("no_such_user@vsphere.local", pState->pszPassword, &ppszTenants, &dwTenantCount);
    TestAssertEquals(dwError, VMDIR_ERROR_ENTRY_NOT_FOUND);

    dwError = VmDirEnumerateTenants(pState->pszUserUPN, "not the password", &ppszTenants, &dwTenantCount);
    TestAssertEquals(dwError, VMDIR_ERROR_USER_INVALID_CREDENTIAL);
}
