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
    { "TestStringAllocate",                 &TestStringAllocate },
    { "TestStringAllocateFromInt",          &TestStringAllocateFromInt },
    { "TestStringAllocateSubstring",        &TestStringAllocateSubstring },
    { "TestStringConcatenate",              &TestStringConcatenate },
    { "TestStringReplace",                  &TestStringReplace },
    { "TestStringBuilder",                  &TestStringBuilder },
    { "TestKeyValuePair",                   &TestKeyValuePair },
    { "TestBase64UrlEncodeToString",        &TestBase64UrlEncodeToString },
    { "TestBase64UrlDecodeToString",        &TestBase64UrlDecodeToString } ,
    { "TestJwtParseSuccess",                &TestJwtParseSuccess },
    { "TestJwtParseFail",                   &TestJwtParseFail },
    { "TestJwtSignatureVerifySuccess",      &TestJwtSignatureVerifySuccess },
    { "TestJwtSignatureVerifyFail",         &TestJwtSignatureVerifyFail },
    { "TestJwtCreateSignedJwtString",       &TestJwtCreateSignedJwtString },
    { "TestJwkParseSuccessWithCert",        &TestJwkParseSuccessWithCert },
    { "TestJwkParseSuccessWithoutCert",     &TestJwkParseSuccessWithoutCert } ,
    { "TestJwkParseFail",                   &TestJwkParseFail },
    { "TestSignatureVerifySuccess",         &TestSignatureVerifySuccess },
    { "TestSignatureVerifyFail",            &TestSignatureVerifyFail },
};

int
main(
    int argc,
    PSTRING* argv)
{
    bool allSucceeded = true;
    size_t i = 0;
    size_t numTestCases = sizeof(testCases) / sizeof(testCases[0]);
    bool success = false;
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
