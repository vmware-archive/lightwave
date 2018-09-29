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

    const struct CMUnitTest LwCASrvJSONUtils_Tests[] =
    {
        cmocka_unit_test_setup_teardown(
                LwCAJsonLoadObjectFromFile_ValidInput,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                LwCAJsonLoadObjectFromFile_InvalidInput,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                LwCAJsonGetObjectFromKey_Valid,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                LwCAJsonGetObjectFromKey_Invalid,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                LwCAJsonGetStringFromKey_Valid,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                LwCAJsonGetStringFromKey_Invalid,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                LwCAJsonGetStringArrayFromKey_Valid,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                LwCAJsonGetStringArrayFromKey_Invalid,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                LwCAJsonGetTimeFromKey_Valid,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                LwCAJsonGetTimeFromKey_Invalid,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                Test_LwCACreateCertificate_Valid,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                Test_LwCACreateCertificate_Invalid,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                Test_LwCACreateCertArray_Valid,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                Test_LwCACreateCertArray_Invalid,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                Test_LwCACreateKey_Valid,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                Test_LwCACreateKey_Invalid,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                Test_LwCACopyCertArray_Valid,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                Test_LwCACopyCertArray_Invalid,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                Test_LwCACopyKey_Valid,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                Test_LwCACopyKey_Invalid,
                NULL,
                NULL),
    };

    ret = cmocka_run_group_tests_name("MutentCA JSON Utils Tests", LwCASrvJSONUtils_Tests, NULL, NULL);
    if (ret)
    {
        fail_msg("%s", "MutentCA server JSON utils failed");
    }

    return 0;
}
