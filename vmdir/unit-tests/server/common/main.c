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
    const struct CMUnitTest tests[] =
    {
        //metadata.c
        cmocka_unit_test_setup_teardown(
                VmDirMetaDataDeserialize_ValidInput,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                VmDirMetaDataDeserialize_InvalidInput,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                VmDirMetaDataSerialize_ValidInput,
                VmDirSetupMetaData,
                VmDirMetaDataFree),
        cmocka_unit_test_setup_teardown(
                VmDirMetaDataSerialize_InvalidInput,
                VmDirSetupMetaData,
                VmDirMetaDataFree),
        cmocka_unit_test_setup_teardown(
                VmDirMetaDataCopyContent_ValidInput,
                VmDirSetupMetaData,
                VmDirMetaDataFree),
        cmocka_unit_test_setup_teardown(
                VmDirMetaDataCopyContent_InvalidInput,
                VmDirSetupMetaData,
                VmDirMetaDataFree),
        cmocka_unit_test_setup_teardown(
                VmDirMetaDataCreate_ValidInput,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                VmDirMetaDataCreate_InvalidInput,
                NULL,
                NULL),
        //valuemetadata.c
        cmocka_unit_test_setup_teardown(
                VmDirValueMetaDataDeserialize_ValidInput,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                VmDirValueMetaDataDeserialize_InvalidInput,
                NULL,
                NULL),
        cmocka_unit_test_setup_teardown(
                VmDirValueMetaDataSerialize_ValidInput,
                VmDirSetupValueMetaData,
                VmDirValueMetaDataFree),
        cmocka_unit_test_setup_teardown(
                VmDirValueMetaDataSerialize_InvalidInput,
                VmDirSetupValueMetaData,
                VmDirValueMetaDataFree),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
