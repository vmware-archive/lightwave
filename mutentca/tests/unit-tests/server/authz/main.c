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

    const struct CMUnitTest LwCAAuthZTests[] = {
        cmocka_unit_test(Test_LwCAAuthZInitialize_NoPlugin_Valid),
        cmocka_unit_test(Test_LwCAAuthZInitialize_PluginPath_Invalid),
    };

    ret = cmocka_run_group_tests(LwCAAuthZTests, NULL, NULL);
    if (ret)
    {
        fail_msg("%s", "MutentCA AuthZ Tests Failed");
    }

    const struct CMUnitTest LwCAAuthZLWTests[] = {
        cmocka_unit_test(Test_LwCAAuthZLWCheckCACreate_Valid),
        cmocka_unit_test(Test_LwCAAuthZLWCheckCACreate_InValid),
        cmocka_unit_test(Test_LwCAAuthZLWCheckCARevoke_Valid),
        cmocka_unit_test(Test_LwCAAuthZLWCheckCARevoke_InValid),
        cmocka_unit_test(Test_LwCAAuthZLWCheckCSR_Valid),
        cmocka_unit_test(Test_LwCAAuthZLWCheckCSR_InValid),
        cmocka_unit_test(Test_LwCAAuthZLWCheckCRL_Valid),
        cmocka_unit_test(Test_LwCAAuthZLWCheckCRL_InValid),
    };

    ret = cmocka_run_group_tests(LwCAAuthZLWTests, Test_LwCAAuthZLW_Setup, Test_LwCAAuthZLW_Teardown);
    if (ret)
    {
        fail_msg("%s", "MutentCA Lightwave AuthZ Tests Failed");
    }

    return ret;
}
