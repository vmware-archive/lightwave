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

    const struct CMUnitTest VMCASrvPolicy_Tests[] =
    {
        cmocka_unit_test_setup_teardown(
                VMCAPolicyInit_ValidInput,
                NULL,
                NULL),
    };

    const struct CMUnitTest VMCASrvJSONUtils_Tests[] =
    {
        cmocka_unit_test_setup_teardown(
                VMCAJsonLoadObjectFromFile_ValidInput,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                VMCAJsonLoadObjectFromFile_InvalidInput,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                VMCAJsonGetObjectFromKey_Valid,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                VMCAJsonGetObjectFromKey_Invalid,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                VMCAJsonGetStringFromKey_Valid,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                VMCAJsonGetStringFromKey_Invalid,
                NULL,
                NULL),
    };

    ret = cmocka_run_group_tests_name("VMCA Policy Tests", VMCASrvPolicy_Tests, NULL, NULL);
    if (ret)
    {
        fail_msg("%s", "VMCA service policy tests failed");
    }

    ret = cmocka_run_group_tests_name("VMCA JSON Utils Tests", VMCASrvJSONUtils_Tests, NULL, NULL);
    if (ret)
    {
        fail_msg("%s", "VMCA service JSON utils failed");
    }

    return 0;
}
