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

    const struct CMUnitTest policyUtilTests[] = {
        cmocka_unit_test(Test_LwCAPolicyCfgObjInit),
        cmocka_unit_test(Test_LwCAPolicyCfgObjArrayInit),
        cmocka_unit_test(Test_LwCAPoliciesInit),
        cmocka_unit_test(Test_LwCAPoliciesInit_Empty),
        cmocka_unit_test(Test_LwCAPolicyCfgObjCopy),
        cmocka_unit_test(Test_LwCAPolicyCfgObjArrayCopy),
    };

    ret = cmocka_run_group_tests(policyUtilTests, NULL, NULL);

    if (ret)
    {
        fail_msg("%s", "MutentCA Policy Util tests failed");
    }

    return ret;
}
