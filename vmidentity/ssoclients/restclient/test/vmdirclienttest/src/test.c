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
//    mu_run_test(VmdirGroupCreateTest);
//    mu_run_test(VmdirGroupGetTest);
//    mu_run_test(VmdirGroupUpdateTest);
//    mu_run_test(VmdirGroupDeleteTest);
//    mu_run_test(VmdirGroupAddMembersTest);
//    mu_run_test(VmdirGroupGetMembersTest);
//    mu_run_test(VmdirGroupRemoveMembersTest);
//    mu_run_test(VmdirGroupGetParentsTest);

//    mu_run_test(VmdirSolutionUserCreateTest);
//    mu_run_test(VmdirSolutionUserGetTest);
//    mu_run_test(VmdirSolutionUserUpdateTest);
//    mu_run_test(VmdirSolutionUserDeleteTest);

    mu_run_test(VmdirUserCreateTest);
    mu_run_test(VmdirUserGetTest);
    mu_run_test(VmdirUserGetGroupsTest);
    mu_run_test(VmdirUserUpdateTest);
    mu_run_test(VmdirUserUpdatePasswordTest);
    mu_run_test(VmdirUserResetPasswordTest);
    mu_run_test(VmdirUserDeleteTest);

    // OIDC HOK token tests
    mu_run_test(VmdirUserCreateTestByHOKToken);
    mu_run_test(VmdirUserGetTestByHOKToken);
    mu_run_test(VmdirUserDeleteTestByHOKToken);

    // OIDC Bearer token HA tests
    mu_run_test(VmdirUserGetTestHA);

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
        fprintf(stdout, "%s\n", "Command to run test: vmdirclienttest testConfigureFile hokPrivateKeyFile");
    }

    return e;
}
