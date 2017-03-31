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

#include "test_cases.h"

static const TEST_CASE testCases[] =
{
    { "TestPasswordGrantSuccessResponse",                   &TestPasswordGrantSuccessResponse },
    { "TestPasswordGrantSuccessResponseNoRefreshToken",     &TestPasswordGrantSuccessResponseNoRefreshToken },
    { "TestPasswordGrantErrorResponse",                     &TestPasswordGrantErrorResponse },
    { "TestRefreshTokenGrantSuccessResponse",               &TestRefreshTokenGrantSuccessResponse },
    { "TestRefreshTokenGrantErrorResponse",                 &TestRefreshTokenGrantErrorResponse },
    { "TestIDTokenParseSuccessWithGroups",                  &TestIDTokenParseSuccessWithGroups },
    { "TestIDTokenParseSuccessWithoutGroups",               &TestIDTokenParseSuccessWithoutGroups },
    { "TestIDTokenParseFail",                               &TestIDTokenParseFail },
    { "TestIDTokenBuildFailInvalidSignature",               &TestIDTokenBuildFailInvalidSignature },
    { "TestIDTokenBuildFailExpired",                        &TestIDTokenBuildFailExpired },
    { "TestAccessTokenParseSuccessWithGroups",              &TestAccessTokenParseSuccessWithGroups },
    { "TestAccessTokenParseSuccessWithoutGroups",           &TestAccessTokenParseSuccessWithoutGroups },
    { "TestAccessTokenParseFail",                           &TestAccessTokenParseFail },
    { "TestAccessTokenBuildFailInvalidSignature",           &TestAccessTokenBuildFailInvalidSignature },
    { "TestAccessTokenBuildFailInvalidAudience",            &TestAccessTokenBuildFailInvalidAudience },
    { "TestAccessTokenBuildFailExpired",                    &TestAccessTokenBuildFailExpired },
};

int
main(
    int argc,
    PSTRING* argv)
{
    PCSTRING pszServer = NULL;
    PCSTRING pszTenant = NULL;
    PCSTRING pszUsername = NULL;
    PCSTRING pszPassword = NULL;
    bool highAvailabilityEnabled = false;
    SSOERROR e = SSOERROR_NONE;
    bool allSucceeded = true;
    size_t i = 0;
    size_t numTestCases = 0;
    bool success = false;

    if (argc != 6)
    {
        printf("usage: >oidctest server tenant username password enableHA\n");
        return -1;
    }

    pszServer = argv[1];
    pszTenant = argv[2];
    pszUsername = argv[3];
    pszPassword = argv[4];
    highAvailabilityEnabled = SSOStringEqual(argv[5], "true");

    e = TestInit(pszServer, pszTenant, pszUsername, pszPassword, highAvailabilityEnabled);
    if (e != SSOERROR_NONE)
    {
        printf("failed with %s\n", SSOErrorToString(e));
        return e;
    }

    numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (i = 0; i < numTestCases; i++)
    {
        printf("\n %s ", testCases[i].pszFunctionName);
        success = testCases[i].function();
        allSucceeded = allSucceeded && success;
        printf("\n\t%s", success ? "success" : "FAIL");
    }
    printf("\n\n%s\n\n", allSucceeded ? "--success--" : "--FAIL--");

    return 0;
}
