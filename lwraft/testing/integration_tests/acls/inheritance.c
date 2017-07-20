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
TestProtectedSecurityDescriptor(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PSTR pszSecurityDescriptor = NULL;
    PSTR pszObjectSD = NULL;
    PSTR pszUserName = NULL;
    PSTR pszUserDn = NULL;
    PSTR pszDomainSid = NULL;

    dwError = VmDirTestGetGuid(&pszUserName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetDomainSid(pState, pState->pszBaseDN, &pszDomainSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    //
    // The permissions are duplicated oddly due to a likewise bug; this ensures
    // that the string looks the same on the way back "out".
    //
    dwError = VmDirAllocateStringPrintf(
                &pszSecurityDescriptor,
                "O:%s-500G:BAD:P(A;;RCSDSDRCRP;;;%s-500)",
                pszDomainSid,
                pszDomainSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestCreateUser(pState, "testcontainer", pszUserName, pszSecurityDescriptor);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                &pszUserDn,
                "cn=%s,cn=testcontainer,%s",
                pszUserName,
                pState->pszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirTestGetAttributeValueString(
                pState->pLd,
                pszUserDn,
                LDAP_SCOPE_BASE,
                "(objectClass=*)",
                ATTR_ACL_STRING,
                &pszObjectSD);
    BAIL_ON_VMDIR_ERROR(dwError);

    TestAssertStrEquals(pszSecurityDescriptor, pszObjectSD);

cleanup:
    VmDirTestDeleteUser(pState, "testcontainer", pszUserName);
    VMDIR_SAFE_FREE_STRINGA(pszSecurityDescriptor);
    VMDIR_SAFE_FREE_STRINGA(pszObjectSD);
    VMDIR_SAFE_FREE_STRINGA(pszUserName);
    VMDIR_SAFE_FREE_STRINGA(pszUserDn);
    VMDIR_SAFE_FREE_STRINGA(pszDomainSid);

    return dwError;
error:
    TestAssert(false);
    goto cleanup;
}

// TODO -- Test that specifies ACL and make sure that it's merged properly
//         with inherited ACLs (make sure ALLOW/DENY ACEs are arranged sensibly).
DWORD
TestSecurityDescriptorInheritance(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;

    dwError = TestProtectedSecurityDescriptor(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}
