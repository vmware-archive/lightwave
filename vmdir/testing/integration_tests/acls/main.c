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
TestSetup(
    PVMDIR_TEST_STATE pState
    )
{
    return 0;
}


DWORD
TestCleanup(
    PVMDIR_TEST_STATE pState
    )
{
    return 0;
}

// TODO -- Test that specifies ACL and make sure that it's merged properly
//         with inherited ACLs.
// TODO -- Test that has a class-based ACL and make sure that it's merged
//         properly with inherited ACLs.
// TODO -- Tests that make sure that group permissions work.
// TODO -- Tests that make sure that nested groups work.
// TODO -- Tests that make sure domain admins/clients privileges don't
//         extend to secondary tenants.
// TODO -- Behavior of inheritance for containers but with OBJECT_INHERIT_ACE (https://msdn.microsoft.com/en-us/library/windows/desktop/aa374924(v=vs.85).aspx)
// TODO -- What rights, if any, do users in the CAAdmins group get in 6.5?
// TODO -- Make sure that the read/write permissions on the things under "cn=schemacontext" are correct.
// TODO -- Make sure we handle entries under cn=services,<domain> properly
// TODO -- Test various inheritance flags (e.g., no_propagate)
// TODO -- Test for anonymous access (make sure error value is correct).
// TODO -- Test with non-binding anonymous access (cf bug #1793712).
// TODO -- Verify appropriate "Deleted Objects" access (admins should be able to delete them, too).
// TODO -- Make sure no one can write to dse root.

DWORD
TestRunner(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;

    printf("Testing security descriptor code ...\n");

    dwError = TestProtectedEntries(pState);
    BAIL_ON_VMDIR_ERROR(dwError);


#if 0 // TODO
    dwError = TestStandardRightsForAdminUser(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestStandardRightsForAdminGroup(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestSecurityDescriptorInheritance(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestBadParameters(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestSecurityDescriptors(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestStandardRightsForDomainAdmin(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestStandardRightsForDomainClients(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestWellknownSids(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestSecurityDescriptorsSddl(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestLegacyAccessChecks(pState);
    BAIL_ON_VMDIR_ERROR(dwError);
#endif

    printf("Security descriptor tests finished successfully.\n");

cleanup:
    return dwError;
error:
    goto cleanup;
}
