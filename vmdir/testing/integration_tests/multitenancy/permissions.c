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
_VmDirTestRecursiveDeleteContainer(
    LDAP *pLd,
    PCSTR pszContainerDn
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    PVMDIR_STRING_LIST pObjectList;

    dwError = VmDirTestGetObjectList(
                pLd,
                pszContainerDn,
                &pObjectList);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwIndex = 0; dwIndex < pObjectList->dwCount; ++dwIndex)
    {
        dwError = ldap_delete_ext_s(pLd, pObjectList->pStringList[dwIndex], NULL, NULL);
        if (dwError == LDAP_NOT_ALLOWED_ON_NONLEAF)
        {
            dwError = _VmDirTestRecursiveDeleteContainer(
                        pLd,
                        pObjectList->pStringList[dwIndex]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    dwError = ldap_delete_ext_s(pLd, pszContainerDn, NULL, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    //
    // dwError can only be LDAP_NOT_ALLOWED_ON_NONLEAF if we are given a DN
    // to delete but we don't RP permissions on that entry so we can't list
    // any of its children so we go to delete the object itself and get back
    // LDAP_NOT_ALLOWED_ON_NONLEAF. In that case, the "real" error is that
    // we don't have permission, so fixup dwError.
    //
    if (dwError == LDAP_NOT_ALLOWED_ON_NONLEAF)
    {
        dwError = LDAP_INSUFFICIENT_ACCESS;
    }
    return dwError;
error:
    goto cleanup;
}

DWORD
_VmDirTestGetDomains(
    LDAP *pLd,
    PCSTR pszDn,
    PVMDIR_STRING_LIST *ppObjectList /* OPTIONAL */
    )
{
    DWORD dwError = 0;
    DWORD dwObjectCount = 0;
    PCSTR ppszAttrs[] = {NULL};
    LDAPMessage *pResult = NULL;
    PVMDIR_STRING_LIST pObjectList = NULL;

    dwError = ldap_search_ext_s(
                pLd,
                pszDn,
                LDAP_SCOPE_SUBTREE,
                "(objectClass=dcObject)",
                (PSTR*)ppszAttrs,
                0,
                NULL,
                NULL,
                NULL,
                -1,
                &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (ppObjectList != NULL)
    {
        dwObjectCount = ldap_count_entries(pLd, pResult);
        dwError = VmDirStringListInitialize(&pObjectList, dwObjectCount);
        if (dwObjectCount > 0)
        {
            LDAPMessage* pEntry = ldap_first_entry(pLd, pResult);

            //
            // Grab the next entry. The first one will be the base DN itself.
            //
            pEntry = ldap_next_entry(pLd, pEntry);
            for (; pEntry != NULL; pEntry = ldap_next_entry(pLd, pEntry))
            {
                dwError = VmDirStringListAddStrClone(ldap_get_dn(pLd, pEntry), pObjectList);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }

        *ppObjectList = pObjectList;
    }

cleanup:
    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    return dwError;

error:
    VmDirStringListFree(pObjectList);
    goto cleanup;
}

//
// Takes a domain like "foo.bar", and returns (an allocated) string like 
// "dc=bar".
//
DWORD
_TestGetRootDNFromDomain(
    PCSTR pszDomainName,
    PSTR *ppszRootDomainDN
    )
{
    DWORD dwError = 0;
    PSTR pszRootDomainPtr = NULL;
    PSTR pszRootDomainDN = NULL;

    pszRootDomainPtr = strrchr(pszDomainName, '.');
    if (pszRootDomainPtr == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateStringPrintf(
                &pszRootDomainDN,
                "dc=%s",
                pszRootDomainPtr + 1);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszRootDomainDN = pszRootDomainDN;

cleanup:
    return dwError;
error:
    goto cleanup;
}


DWORD
TestTenantPermissions(
    PVMDIR_TEST_STATE pState,
    LDAP *pLdToUse,
    PCSTR pszDomain,
    DWORD dwExpectedTenantCount
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    BOOLEAN bDomainFound = FALSE;
    PVMDIR_STRING_LIST pTenantList = NULL;
    PSTR pszDN = NULL;

    dwError = _TestGetRootDNFromDomain(pszDomain, &pszDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirTestGetDomains(pLdToUse, pszDN, &pTenantList);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwIndex = 0; dwIndex < pTenantList->dwCount; ++dwIndex)
    {
        if (VmDirStringCompareA(pTenantList->pStringList[dwIndex], pszDomain, FALSE))
        {
            bDomainFound = TRUE;
        }
    }

    TestAssert(bDomainFound);
    TestAssertEquals(pTenantList->dwCount, dwExpectedTenantCount);

cleanup:
    VmDirStringListFree(pTenantList);
    return dwError;
error:
    goto cleanup;
}

VOID
CrossTenancyDeletionsShouldFail(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;

    dwError = _VmDirTestRecursiveDeleteContainer(
                pState->pLd,
                "dc=customer,dc=com");
    TestAssertEquals(dwError, LDAP_INSUFFICIENT_ACCESS);

    dwError = _VmDirTestRecursiveDeleteContainer(
                pState->pLd,
                "dc=foobar,dc=net");
    TestAssertEquals(dwError, LDAP_INSUFFICIENT_ACCESS);

    dwError = _VmDirTestRecursiveDeleteContainer(
                pState->pLd,
                "dc=testing,dc=local");
    TestAssertEquals(dwError, LDAP_INSUFFICIENT_ACCESS);
}

DWORD
TestCreateUserInTenant(
    LDAP *pLd,
    PCSTR pszUserName,
    PCSTR pszTenantDn
    )
{
    DWORD dwError = 0;
    PCSTR valsCn[] = {pszUserName, NULL};
    PCSTR valssAMActName[] = {pszUserName, NULL};
    PCSTR valsClass[] = {OC_USER, OC_PERSON, OC_TOP, OC_ORGANIZATIONAL_PERSON, NULL};
    PCSTR valsPNE[] = {"TRUE", NULL};
    PCSTR valsPN[] = {NULL, NULL};
    PCSTR valsPass[] = {"Admin!23", NULL};
    PSTR pszUPN = NULL;
    PSTR pszDN = NULL;
    LDAPMod mod[]={
        {LDAP_MOD_ADD, ATTR_CN, {(PSTR*)valsCn}},
        {LDAP_MOD_ADD, ATTR_SAM_ACCOUNT_NAME, {(PSTR*)valssAMActName}},
        {LDAP_MOD_ADD, ATTR_OBJECT_CLASS, {(PSTR*)valsClass}},
        {LDAP_MOD_ADD, ATTR_PASSWORD_NEVER_EXPIRES, {(PSTR*)valsPNE}},
        {LDAP_MOD_ADD, ATTR_KRB_UPN, {(PSTR*)valsPN}},
        {LDAP_MOD_ADD, ATTR_USER_PASSWORD, {(PSTR*)valsPass}},
        {LDAP_MOD_ADD, ATTR_SN, {(PSTR*)valsCn}},
    };
    LDAPMod *attrs[] = {&mod[0], &mod[1], &mod[2], &mod[3], &mod[4], &mod[5], &mod[6], NULL};

    dwError = VmDirAllocateStringPrintf(&pszUPN, "%s@%s", pszUserName, pszTenantDn);
    BAIL_ON_VMDIR_ERROR(dwError);
    valsPN[0] = pszUPN;

    dwError = VmDirAllocateStringPrintf(
                &pszDN,
                "cn=%s,cn=Users,%s",
                pszUserName,
                pszTenantDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_add_ext_s(
                pLd,
                pszDN,
                attrs,
                NULL,
                NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDN);
    VMDIR_SAFE_FREE_STRINGA(pszUPN);
    return dwError;

error:
    goto cleanup;
}

//
//    "dc=testing,dc=local");
DWORD
TestCrossTenantCreateFails(
    PVMDIR_TEST_STATE pState,
    PCSTR pszTenantDn
    )
{
    DWORD dwError = 0;
    PSTR pszUserName = NULL;

    dwError = VmDirTestGetGuid(&pszUserName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestCreateUserInTenant(pState->pLd, pszUserName, pszTenantDn);
    TestAssertEquals(dwError, LDAP_INSUFFICIENT_ACCESS);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszUserName);
    return dwError;
error:
    goto cleanup;
}

// "dc=foobar,dc=net");
DWORD
TestCrossTenantDeleteFails(
    PVMDIR_TEST_STATE pState,
    LDAP *pLdTenant,
    PCSTR pszTenantDn
    )
{
    DWORD dwError = 0;
    PSTR pszUserName = NULL;
    PSTR pszDn = NULL;

    dwError = VmDirTestGetGuid(&pszUserName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestCreateUserInTenant(pLdTenant, pszUserName, pszTenantDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                &pszDn,
                "cn=%s,cn=Users,%s",
                pszUserName,
                pszTenantDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_delete_ext_s(pState->pLd, pszDn, NULL, NULL);
    TestAssertEquals(dwError, LDAP_INSUFFICIENT_ACCESS);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszUserName);
    VMDIR_SAFE_FREE_STRINGA(pszDn);
    return dwError;
error:
    goto cleanup;
}

//
// Test that tenants have appropriate permissions. We'll create a few domains:
// vsphere.local (assumed, pre-existing, primary tenant)
// testing.local
// customer.com
// foobar.net
//
//
DWORD
TestMultiTenancyPermissions(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    LDAP *pLdTestingDotLocal = NULL;
    LDAP *pLdCustomerDotCom = NULL;
    LDAP *pLdFoobarDotNet = NULL;

    dwError = VmDirCreateTenant(
                pState->pszUserName,
                pState->pszPassword,
                "testing.local",
                "administrator",
                pState->pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateTenant(
                pState->pszUserName,
                pState->pszPassword,
                "customer.com",
                "administrator",
                pState->pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateTenant(
                pState->pszUserName,
                pState->pszPassword,
                "foobar.net",
                "administrator",
                pState->pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirTestAdminConnectionFromDomain(
                pState,
                "testing.local",
                &pLdTestingDotLocal);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirTestAdminConnectionFromDomain(
                pState,
                "customer.com",
                &pLdCustomerDotCom);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirTestAdminConnectionFromDomain(
                pState,
                "foobar.net",
                &pLdFoobarDotNet);
    BAIL_ON_VMDIR_ERROR(dwError);

    TestTenantPermissions(pState, pLdTestingDotLocal, "testing.local", 1);
    TestTenantPermissions(pState, pLdCustomerDotCom, "customer.com", 1);
    TestTenantPermissions(pState, pLdFoobarDotNet, "foobar.net", 1);
    TestTenantPermissions(pState, pState->pLd, pState->pszDomain, 2);

    //
    // Make sure that we can't add or delete entries from tenants even using
    // the primary admin's credentials.
    //
    TestCrossTenantCreateFails(pState, "dc=testing,dc=local");
    TestCrossTenantDeleteFails(pState, pLdFoobarDotNet, "dc=foobar,dc=net");
    TestCrossTenantDeleteFails(pState, pState->pLd, "dc=foobar,dc=net");

    CrossTenancyDeletionsShouldFail(pState);

cleanup:
    VmDirDeleteTenant(pState->pszUserName, pState->pszPassword, "testing.local");
    VmDirDeleteTenant(pState->pszUserName, pState->pszPassword, "customer.com");
    VmDirDeleteTenant(pState->pszUserName, pState->pszPassword, "foobar.net");

    VmDirTestLdapUnbind(pLdTestingDotLocal);
    VmDirTestLdapUnbind(pLdCustomerDotCom);
    VmDirTestLdapUnbind(pLdFoobarDotNet);
    return dwError;
error:
    TestAssertEquals(dwError, 0); // TODO
    goto cleanup;
}
