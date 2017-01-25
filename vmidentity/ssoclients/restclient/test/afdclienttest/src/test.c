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
PREST_CLIENT pHOKTokenClient = NULL;

static
PCSTRING
run_all_tests()
{
    // OIDC Bearer token tests
    mu_run_test(AfdVecsGetSSLCertificatesTest);

//    mu_run_test(AfdAdProviderJoinTest);

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
        fprintf(stdout, "%s\n", "Command to run test: afdclienttest testConfigureFile hokPrivateKeyFile");
    }

    return e;
}
