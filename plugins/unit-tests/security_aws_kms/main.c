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

VOID
test_success_empty(VOID **pState);

int main(VOID)
{
    int ret = 0;

    const struct CMUnitTest Security_Aws_Kms_Tests[] =
    {
        /* get and check version - this does not require load interface */
        cmocka_unit_test_setup(
                Security_Aws_Kms_Tests_Check_Version,
                Security_Aws_Kms_Tests_Get_Version),

        /* load and validate interface - this interface is used in all tests */
        cmocka_unit_test_setup(
                Security_Aws_Kms_Tests_Validate_Interface,
                Security_Aws_Kms_Tests_Load_Interface),

        /* validate config file parsing */
        cmocka_unit_test(Security_Aws_Kms_Tests_Config_Parse_Good),
        cmocka_unit_test(Security_Aws_Kms_Tests_Config_Parse_Bad_No_Root),
        cmocka_unit_test(Security_Aws_Kms_Tests_Config_Parse_Bad_No_CMKId),
        cmocka_unit_test(Security_Aws_Kms_Tests_Config_Parse_Bad_No_KeySpec),

        /* unload interface */
        cmocka_unit_test_teardown(
                test_success_empty,
                Security_Aws_Kms_Tests_Unload_Interface),

        /* ensure interface is cleared after unload */
        cmocka_unit_test(Security_Aws_Kms_Tests_Validate_Interface_Cleared),
    };

    /*
     * before running tests, load implementation library
     * unload library after tests
    */
    ret = cmocka_run_group_tests(
              Security_Aws_Kms_Tests,
              Security_Aws_Kms_Tests_Load,
              Security_Aws_Kms_Tests_Unload);
    if (ret)
    {
        fail_msg("%s", "security aws kms tests failed");
    }

    return ret;
}

VOID
test_success_empty(VOID **pState)
{
}
