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
TestEntriesProtectedByEid(
    PVMDIR_TEST_STATE pState,
    PCSTR pszDomain
    )
{
    DWORD dwError = 0;
    DWORD i = 0;
    PSTR pszObjectDn = NULL;
    PSTR ppszObjects[] = {
        "cn=DSE Root",
        "cn=attributeMetaData,cn=schemacontext",
        "cn=config",
        "cn=organization,cn=config",
        "cn=Deleted Objects,%s",
        "cn=Administrator,cn=Users,%s"
    };

    for (i = 0; i < VMDIR_ARRAY_SIZE(ppszObjects); ++i)
    {
        dwError = VmDirAllocateStringPrintf(
                    &pszObjectDn,
                    ppszObjects[i],
                    pszDomain);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = ldap_delete_ext_s(pState->pLd, pszObjectDn, NULL, NULL);
        //
        // This previously returned LDAP_UNWILLING_TO_PERFORM but we probably don't
        // need to maintain that (access denied is better).
        //
        TestAssert(dwError == LDAP_UNWILLING_TO_PERFORM || dwError == LDAP_INSUFFICIENT_ACCESS);

        VMDIR_SAFE_FREE_STRINGA(pszObjectDn);
    }

    dwError = 0;
cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszObjectDn);
    return dwError;
error:
    goto cleanup;
}


// TODO -- Call this for tenant domains.
DWORD
TestEntriesProtectedByName(
    PVMDIR_TEST_STATE pState,
    PCSTR pszDomain
    )
{
    DWORD dwError = 0;
    DWORD i = 0;
    PSTR pszObjectDn = NULL;
    PSTR ppszObjects[] = {
        "cn=Administrators,cn=Builtin",
        "cn=CAAdmins,cn=Builtin",
        "cn=DCAdmins,cn=Builtin",
        "cn=Users,cn=Builtin",
        "cn=Administrator,cn=Users",
        "cn=DCClients,cn=Builtin"
    };

    for (i = 0; i < VMDIR_ARRAY_SIZE(ppszObjects); ++i)
    {
        dwError = VmDirAllocateStringPrintf(
                    &pszObjectDn,
                    "%s,%s",
                    ppszObjects[i],
                    pszDomain);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = ldap_delete_ext_s(pState->pLd, pszObjectDn, NULL, NULL);
        TestAssertEquals(dwError, LDAP_INSUFFICIENT_ACCESS);

        VMDIR_SAFE_FREE_STRINGA(pszObjectDn);
    }

    dwError = 0;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszObjectDn);
    return dwError;
error:
    goto cleanup;
}

//
// Prior to 7.0 we had code that prevented any entry under "cn=schemacontext"
// from being deleted (by doing a string comparison). In 7.0 this is due via
// ACLs.
//
DWORD
TestProtectedSchemaEntries(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;

    dwError = ldap_delete_ext_s(pState->pLd, "cn=msTSLSProperty01,cn=schemacontext", NULL, NULL);
    // TODO
    // Version specific error code asserts? 6.5 will return the former, 7.0
    // the latter
    TestAssert(dwError == LDAP_UNWILLING_TO_PERFORM || dwError == LDAP_INSUFFICIENT_ACCESS);

    return 0;
}

//
// Make sure that we can create a random user under the builtin container
// and then delete it (there are other entries under builtin that shouldn't be
// deletable, so we want to make sure that doesn't "leak" out onto other objects
// created under it).
//
DWORD
TestBuiltinContainerDeletion(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PSTR pszUserName = NULL;

    dwError = VmDirTestGetGuid(&pszUserName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateUser(pState, "builtin", pszUserName, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestDeleteUser(pState, "builtin", pszUserName);
    TestAssertEquals(dwError, 0);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszUserName);
    return dwError;
error:
    goto cleanup;
}

DWORD
TestProtectedEntries(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;

    dwError = TestEntriesProtectedByEid(pState, pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestEntriesProtectedByName(pState, pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestProtectedSchemaEntries(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestBuiltinContainerDeletion(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("Protected entries tests succceeded!\n");

cleanup:
    return dwError;

error:
    printf("Security Descriptor tests failed with error 0n%d\n", dwError);
    goto cleanup;
}
