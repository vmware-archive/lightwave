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

// global variables instantiation
int tests_run = 0;
PREST_CLIENT pBearerTokenClient = NULL;
PREST_CLIENT pBearerTokenHAClient = NULL;
PREST_CLIENT pHOKTokenClient = NULL;

static
PCSTRING
run_all_tests()
{
    // OIDC Bearer token tests
//    mu_run_test(IdmTenantCreateTest);
//    mu_run_test(IdmTenantGetTest);
//    mu_run_test(IdmTenantDeleteTest);
//    mu_run_test(IdmTenantGetConfigTest);
//    mu_run_test(IdmTenantUpdateConfigTest);
//    mu_run_test(IdmTenantSearchTest);

//    mu_run_test(IdmDiagnosticsClearEventLogTest);
//    mu_run_test(IdmDiagnosticsGetEventLogTest);
//    mu_run_test(IdmDiagnosticsGetEventLogStatusTest);
//    mu_run_test(IdmDiagnosticsStartEventLogTest);
//    mu_run_test(IdmDiagnosticsStopEventLogTest);

//    mu_run_test(IdmIdentityProviderCreateTest);
//    mu_run_test(IdmIdentityProviderProbeTest);
//    mu_run_test(IdmIdentityProviderGetAllTest);
//    mu_run_test(IdmIdentityProviderGetTest);
//    mu_run_test(IdmIdentityProviderUpdateTest);
//    mu_run_test(IdmIdentityProviderDeleteTest);

//    mu_run_test(IdmCertificateGetTest);
//    mu_run_test(IdmCertificateDeleteTest);
//    mu_run_test(IdmCertificateGetPrivateKeyTest);
//    mu_run_test(IdmCertificateSetCredentialsTest);

//    mu_run_test(IdmExternalIdpRegisterTest);
//    mu_run_test(IdmExternalIdpRegisterByMetadataTest);    // this does not work since metadata is not set yet.
//    mu_run_test(IdmExternalIdpGetAllTest);
//    mu_run_test(IdmExternalIdpGetTest);
//    mu_run_test(IdmExternalIdpDeleteTest);

//    mu_run_test(IdmGroupGetTest);
//    mu_run_test(IdmGroupGetMembersTest);
//    mu_run_test(IdmGroupGetParentsTest);

//    mu_run_test(IdmOidcClientRegisterTest);
//    mu_run_test(IdmOidcClientGetAllTest);
//    mu_run_test(IdmOidcClientGetTest);
//    mu_run_test(IdmOidcClientUpdateTest);
//    mu_run_test(IdmOidcClientDeleteTest);

//    mu_run_test(IdmRelyingPartyRegisterTest);
//    mu_run_test(IdmRelyingPartyGetAllTest);
//    mu_run_test(IdmRelyingPartyGetTest);
//    mu_run_test(IdmRelyingPartyUpdateTest);
//    mu_run_test(IdmRelyingPartyDeleteTest);

//    mu_run_test(IdmResourceServerRegisterTest);
//    mu_run_test(IdmResourceServerGetAllTest);
//    mu_run_test(IdmResourceServerGetTest);
//    mu_run_test(IdmResourceServerUpdateTest);
//    mu_run_test(IdmResourceServerDeleteTest);

//    mu_run_test(IdmServerGetComputersTest);

//    mu_run_test(IdmSolutionUserGetTest);

//    mu_run_test(IdmUserGetTest);
//    mu_run_test(IdmUserGetGroupsTest);

//    mu_run_test(IdmUserCreateTest); // IDM external IDP user
//    mu_run_test(IdmUserDeleteTest); // IDM external IDP user

    // OIDC HOK token tests
//    mu_run_test(IdmUserGetTestByHOKToken);

//    mu_run_test(IdmUserCreateTestByHOKToken); // IDM external IDP user
//    mu_run_test(IdmUserDeleteTestByHOKToken); // IDM external IDP user

    // OIDC Bearer token HA tests
    mu_run_test(IdmUserGetTestHA);

    return 0;
}

int
main(
    int argc,
    PSTRING* argv)
{
    SSOERROR e = SSOERROR_NONE;
    PCSTRING ret = NULL;

    if (argc == 3)
    {
        e = RestTestSetup(argv[1], argv[2]);
        BAIL_ON_ERROR(e);

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

        error:

        // cleanup
        RestTestCleanup();
    }
    else
    {
        fprintf(stdout, "%s\n", "Command to run test: idmclienttest testConfigureFile hokPrivateKeyFile");
    }

    return e;
}
