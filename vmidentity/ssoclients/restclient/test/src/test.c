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

// global variables
int tests_run = 0;

PSTRING pscHost;
size_t pscPort;
PSTRING systemTenant;
PSTRING systemTenantUsername;
PSTRING systemTenantPassword;
PCSTRING testTenant = "test_tenant_name";
PCSTRING testTenantUsername = "Administrator@test_tenant_name";
PCSTRING testTenantPassword = "Admin!23";

static
PCSTRING
run_all_tests()
{
    // first step, always create a test tenant
    mu_run_test(IdmTenantCreateTest);

    // OIDC Bearer token tests
    mu_run_test(IdmTenantGetTest);
//    mu_run_test(IdmTenantGetConfigTest);
//    mu_run_test(IdmTenantUpdateConfigTest);
    mu_run_test(IdmTenantSearchTest);

    mu_run_test(IdmDiagnosticsClearEventLogTest);
    mu_run_test(IdmDiagnosticsGetEventLogTest);
    mu_run_test(IdmDiagnosticsGetEventLogStatusTest);
    mu_run_test(IdmDiagnosticsStartEventLogTest);
    mu_run_test(IdmDiagnosticsStopEventLogTest);

    mu_run_test(IdmIdentityProviderCreateTest);
    mu_run_test(IdmIdentityProviderProbeTest);
    mu_run_test(IdmIdentityProviderGetAllTest);
    mu_run_test(IdmIdentityProviderGetTest);
    mu_run_test(IdmIdentityProviderUpdateTest);
    mu_run_test(IdmIdentityProviderDeleteTest);

    mu_run_test(IdmCertificateGetTest);
//    mu_run_test(IdmCertificateDeleteTest);     // need to provide finger print to work
    mu_run_test(IdmCertificateGetPrivateKeyTest);
    mu_run_test(IdmCertificateSetCredentialsTest);

    mu_run_test(IdmExternalIdpRegisterTest);
//    mu_run_test(IdmExternalIdpRegisterByMetadataTest);     // need to provide metadata to work
    mu_run_test(IdmExternalIdpGetAllTest);
    mu_run_test(IdmExternalIdpGetTest);
    mu_run_test(IdmExternalIdpDeleteTest);

    mu_run_test(VmdirGroupCreateTest);
    mu_run_test(IdmGroupGetTest);
    mu_run_test(IdmGroupGetMembersTest);
    mu_run_test(IdmGroupGetParentsTest);
    mu_run_test(VmdirGroupGetTest);
    mu_run_test(VmdirGroupUpdateTest);
    mu_run_test(VmdirGroupAddMembersTest);
    mu_run_test(VmdirGroupGetMembersTest);
    mu_run_test(VmdirGroupRemoveMembersTest);
    mu_run_test(VmdirGroupGetParentsTest);
    mu_run_test(VmdirGroupDeleteTest);

    mu_run_test(IdmOidcClientRegisterTest);
    mu_run_test(IdmOidcClientGetAllTest);
    mu_run_test(IdmOidcClientGetTest);
    mu_run_test(IdmOidcClientUpdateTest);
    mu_run_test(IdmOidcClientDeleteTest);

    mu_run_test(IdmRelyingPartyRegisterTest);
    mu_run_test(IdmRelyingPartyGetAllTest);
    mu_run_test(IdmRelyingPartyGetTest);
    mu_run_test(IdmRelyingPartyUpdateTest);
    mu_run_test(IdmRelyingPartyDeleteTest);

    mu_run_test(IdmResourceServerRegisterTest);
    mu_run_test(IdmResourceServerGetAllTest);
    mu_run_test(IdmResourceServerGetTest);
    mu_run_test(IdmResourceServerUpdateTest);
    mu_run_test(IdmResourceServerDeleteTest);

    mu_run_test(IdmServerGetComputersTest);

    mu_run_test(VmdirSolutionUserCreateTest);
    mu_run_test(IdmSolutionUserGetTest);
    mu_run_test(VmdirSolutionUserGetTest);
    mu_run_test(VmdirSolutionUserUpdateTest);
    mu_run_test(VmdirSolutionUserDeleteTest);

    mu_run_test(VmdirUserCreateTest);
    mu_run_test(IdmUserGetTest);
    mu_run_test(IdmUserGetGroupsTest);
    mu_run_test(VmdirUserGetTest);
    mu_run_test(VmdirUserGetGroupsTest);
    mu_run_test(VmdirUserUpdateTest);
    mu_run_test(VmdirUserUpdatePasswordTest);
    mu_run_test(VmdirUserResetPasswordTest);
    mu_run_test(VmdirUserDeleteTest);

//    mu_run_test(IdmExternalUserCreateTest);
//    mu_run_test(IdmExternalUserDeleteTest);

    mu_run_test(AfdVecsGetSSLCertificatesTest);
//    mu_run_test(AfdAdProviderJoinTest);

    // OIDC HOK token tests
//    mu_run_test(IdmTenantGetByHOKClientTest);

    // OIDC Bearer token HA tests
//    mu_run_test(IdmTenantGetByHAClientTest);

    // last step, always delete test tenant.
    mu_run_test(IdmTenantDeleteTest);

    return 0;
}

int
main(
    int argc,
    PSTRING* argv)
{
    SSOERROR e = SSOERROR_NONE;
    PCSTRING ret = NULL;

    if (argc == 6)
    {
        e = SSOStringAllocate(argv[1], &pscHost);
        BAIL_ON_ERROR(e)

        pscPort = strtol(argv[2], (PSTRING*) NULL, 10);

        e = SSOStringAllocate(argv[3], &systemTenant);
        BAIL_ON_ERROR(e)

        e = SSOStringAllocate(argv[4], &systemTenantUsername);
        BAIL_ON_ERROR(e)

        e = SSOStringAllocate(argv[5], &systemTenantPassword);
        BAIL_ON_ERROR(e)

        ret = run_all_tests();

        if (ret)
        {
            fprintf(stdout, "%s\n", ret);
        }
        else
        {
            fprintf(stdout, "%s\n", "ALL TESTS PASSED");
        }

        fprintf(stdout, "%s %d\n", "TESTS RUN:", tests_run);
    }
    else
    {
        fprintf(stdout, "%s\n", "Command to run test: ssorestclienttest ip port systemTenant systemTenantAdminUsername systemTenantAdminPassword");
        fprintf(stdout, "%s\n", "        for example: ssorestclienttest 1.2.3.4 7444 coke admin@coke cokeword");
    }

    error:

    return e;
}
