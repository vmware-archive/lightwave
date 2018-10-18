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

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(Test_LwCADbAddCA),
        cmocka_unit_test(Test_LwCADbAddCertData),
        cmocka_unit_test(Test_LwCADbCheckCA),
        cmocka_unit_test(Test_LwCADbCheckCertData),
        cmocka_unit_test(Test_LwCADbGetCA),
        cmocka_unit_test(Test_LwCADbGetCACertificates),
        cmocka_unit_test(Test_LwCADbGetCertData),
        cmocka_unit_test(Test_LwCADbGetCACRLNumber),
        cmocka_unit_test(Test_LwCADbGetParentCAId),
        cmocka_unit_test(Test_LwCADbUpdateCA),
        cmocka_unit_test(Test_LwCADbUpdateCAStatus),
        cmocka_unit_test(Test_LwCADbUpdateCertData),
        cmocka_unit_test(Test_LwCADbUpdateCACRLNumber),
    };

    ret = cmocka_run_group_tests(tests, Test_LwCADbInitCtx, Test_LwCADbFreeCtx);
    if (ret)
    {
        fail_msg("%s", "MutentCA DB tests failed");
    }

    const struct CMUnitTest tests1[] = {
        cmocka_unit_test(Test_LwCADbCAData),
        cmocka_unit_test(Test_LwCADbCAData_Invalid),
        cmocka_unit_test(Test_LwCADbCertData),
        cmocka_unit_test(Test_LwCADbCertData_Invalid),
        cmocka_unit_test_setup_teardown(Test_LwCADbInitCtx_Invalid, Test_LwCADbInitCtx, Test_LwCADbFreeCtx),
        cmocka_unit_test(Test_LwCADbAddCA_Invalid),
        cmocka_unit_test(Test_LwCADbAddCertData_Invalid),
        cmocka_unit_test(Test_LwCADbCheckCA_Invalid),
        cmocka_unit_test(Test_LwCADbCheckCertData_Invalid),
        cmocka_unit_test(Test_LwCADbGetCA_Invalid),
        cmocka_unit_test(Test_LwCADbGetCACertificates_Invalid),
        cmocka_unit_test(Test_LwCADbGetCertData_Invalid),
        cmocka_unit_test(Test_LwCADbGetCACRLNumber_Invalid),
        cmocka_unit_test(Test_LwCADbGetParentCAId_Invalid),
        cmocka_unit_test(Test_LwCADbUpdateCA_Invalid),
        cmocka_unit_test(Test_LwCADbUpdateCAStatus_Invalid),
        cmocka_unit_test(Test_LwCADbUpdateCertData_Invalid),
        cmocka_unit_test(Test_LwCADbUpdateCACRLNumber_Invalid),
    };

    ret = cmocka_run_group_tests(tests1, NULL, NULL);
    if (ret)
    {
        fail_msg("%s", "MutentCA DB tests failed");
    }

    return ret;
}
