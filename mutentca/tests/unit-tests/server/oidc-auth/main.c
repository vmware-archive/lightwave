/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

int main(VOID)
{
    int ret = 0;

    const struct CMUnitTest LwCAOIDCTests[] = {
        cmocka_unit_test(Test_LwCAOIDCTokenAuthenticate_BearerToken_Valid),
        cmocka_unit_test(Test_LwCAOIDCTokenAuthenticate_BearerToken_InvalidIssuer),
        cmocka_unit_test(Test_LwCAOIDCTokenAuthenticate_BearerToken_InvalidAud),
        cmocka_unit_test(Test_LwCAOIDCTokenAuthenticate_BearerToken_Expired),
        cmocka_unit_test(Test_LwCAOIDCTokenAuthenticate_BearerToken_NotInHeader),
        cmocka_unit_test(Test_LwCAOIDCTokenAuthenticate_HOTKToken_Valid),
        cmocka_unit_test(Test_LwCAOIDCTokenAuthenticate_HOTKToken_InvalidIssuer),
        cmocka_unit_test(Test_LwCAOIDCTokenAuthenticate_HOTKToken_InvalidAud),
        cmocka_unit_test(Test_LwCAOIDCTokenAuthenticate_HOTKToken_Expired),
        cmocka_unit_test(Test_LwCAOIDCTokenAuthenticate_HOTKToken_NoPOP),
        cmocka_unit_test(Test_LwCAOIDCTokenAuthenticate_HOTKToken_InvalidPOPData),
        cmocka_unit_test(Test_LwCAOIDCTokenAuthenticate_UnknownTokType),
    };

    ret = cmocka_run_group_tests(LwCAOIDCTests, NULL, NULL);
    if (ret)
    {
        fail_msg("%s", "MutentCA Lightwave OIDC Auth Tests Failed");
    }

    return ret;
}
