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

    const struct CMUnitTest Security_Tests[] =
    {
        /* get and check version */
        cmocka_unit_test_setup_teardown(
                Security_Tests_Check_Version,
                Security_Tests_Get_Version,
                NULL),
        /* load interface and validate fn ptrs */
        cmocka_unit_test_setup_teardown(
                Security_Tests_Validate_Interface,
                Security_Tests_Load_Interface,
                Security_Tests_Unload_Interface),
        /* check cap */
        cmocka_unit_test_setup_teardown(
                Security_Tests_Check_Caps,
                Security_Tests_Initialize,
                Security_Tests_Unload_Interface)
    };

    ret = cmocka_run_group_tests(Security_Tests, Security_Tests_Load, Security_Tests_Unload);
    if (ret)
    {
        fail_msg("%s", "ca security plugin tests failed");
    }

    return ret;
}
