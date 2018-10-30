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
        cmocka_unit_test(Test_LwCACreateRootCA_Valid),
        cmocka_unit_test(Test_LwCACreateRootCA_Invalid),
        cmocka_unit_test(Test_LwCAGetCACertificates_Valid),
        cmocka_unit_test(Test_LwCAGetCACertificates_Invalid),
        cmocka_unit_test(Test_LwCAGetSignedCertificate_Valid),
        cmocka_unit_test(Test_LwCAGetSignedCertificate_Invalid),
        cmocka_unit_test(Test_LwCACreateIntermediateCA_Valid),
        cmocka_unit_test(Test_LwCACreateIntermediateCA_Invalid),
        cmocka_unit_test(Test_LwCARevokeCertificate_Valid),
        cmocka_unit_test(Test_LwCARevokeCertificate_Invalid),
        cmocka_unit_test(Test_LwCARevokeIntermediateCA_Valid),
        cmocka_unit_test(Test_LwCARevokeIntermediateCA_Invalid),
        cmocka_unit_test(Test_LwCAGetCACrl_Valid),
        cmocka_unit_test(Test_LwCAGetCACrl_Invalid),
    };

    ret = cmocka_run_group_tests(tests, TestLwCACreateRequestContext, TestLwCAFreeRequestContext);

    if (ret)
    {
        fail_msg("%s", "MutentCA API tests failed");
    }

    return ret;
}
