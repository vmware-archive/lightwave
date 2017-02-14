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
    (VOID)VmDirDeleteTenant(
                pState->pszUserName,
                pState->pszPassword,
                "secondary.local");

    (VOID)VmDirDeleteTenant(
                pState->pszUserName,
                pState->pszPassword,
                "tertiary.com");

    (VOID)VmDirDeleteTenant(
                pState->pszUserName,
                pState->pszPassword,
                "quad.com");

    (VOID)VmDirDeleteTenant(
                pState->pszUserName,
                pState->pszPassword,
                "testing.local");

    (VOID)VmDirDeleteTenant(
                pState->pszUserName,
                pState->pszPassword,
                "customer.com");

    (VOID)VmDirDeleteTenant(
                pState->pszUserName,
                pState->pszPassword,
                "foobar.net");

    return 0;
}

//
// Tests the ability to create and manage tenant domains. To do this we will
// constuct a few tenant domains such that we have a domain structure that
// looks like this:
//      vsphere.local
//      secondary.local
//      tertiary.com
//      quad.com
//
DWORD
TestRunner(
    PVMDIR_TEST_STATE pState
    )
{
    printf("Testing multi-tenancy code ...\n");

    NullParametersShouldFail(pState);
    InvalidCredentialsShouldFail(pState);

    ShouldBeAbleToCreateTenants(pState);
    ShouldNotBeAbleToCreateTenantsOfACertainLength(pState);

    ShouldBeAbleToEnumerateTenants(pState);

    ShouldBeAbleToDeleteTenants(pState);

    TestMultiTenancyPermissions(pState);

    printf("Multi-tenancy testing completed successfully.\n");

    return 0;
}
