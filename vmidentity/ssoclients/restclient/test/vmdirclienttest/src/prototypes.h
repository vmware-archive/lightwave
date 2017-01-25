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

#ifndef PROTOTYPES_H_
#define PROTOTYPES_H_

PCSTRING VmdirGroupCreateTest();
PCSTRING VmdirGroupGetTest();
PCSTRING VmdirGroupUpdateTest();
PCSTRING VmdirGroupDeleteTest();
PCSTRING VmdirGroupAddMembersTest();
PCSTRING VmdirGroupGetMembersTest();
PCSTRING VmdirGroupRemoveMembersTest();
PCSTRING VmdirGroupGetParentsTest();

PCSTRING VmdirSolutionUserCreateTest();
PCSTRING VmdirSolutionUserGetTest();
PCSTRING VmdirSolutionUserUpdateTest();
PCSTRING VmdirSolutionUserDeleteTest();

PCSTRING VmdirUserCreateTest();
PCSTRING VmdirUserGetTest();
PCSTRING VmdirUserGetGroupsTest();
PCSTRING VmdirUserUpdateTest();
PCSTRING VmdirUserUpdatePasswordTest();
PCSTRING VmdirUserResetPasswordTest();
PCSTRING VmdirUserDeleteTest();

PCSTRING VmdirUserCreateTestByHOKToken();
PCSTRING VmdirUserGetTestByHOKToken();
PCSTRING VmdirUserDeleteTestByHOKToken();

PCSTRING VmdirUserGetTestHA();

SSOERROR
RestTestSetup(
    PCSTRING testConfigureFile,
    PCSTRING hokPrivateKeyFile);

void
RestTestCleanup();

PCSTRING
RestTestGenerateErrorMessage(
    PCSTRING testName,
    const SSOERROR testError,
    const REST_SERVER_ERROR* pTestServerError);

#endif /* PROTOTYPES_H_ */
