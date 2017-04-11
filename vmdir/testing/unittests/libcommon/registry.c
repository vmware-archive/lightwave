/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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
TestDwordRoundTrip(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwTestValue = 0;
    DWORD dwComparisonValue = 0;
    DWORD dwError = 0;

    dwTestValue = 42;
    dwError = VmDirSetRegKeyValueDword(
                VMDIR_CONFIG_PARAMETER_KEY_PATH,
                "TestValue",
                dwTestValue);
    TestAssertEquals(dwError, 0);

    dwError = VmDirGetRegKeyValueDword(
                VMDIR_CONFIG_PARAMETER_KEY_PATH,
                "TestValue",
                &dwComparisonValue,
                0);
    TestAssertEquals(dwError, 0);
    TestAssertEquals(dwTestValue, dwComparisonValue);
}

VOID
TestDwordDefaultValue(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwTestValue = 0;
    DWORD dwComparisonValue = 0;
    DWORD dwError = 0;

    dwTestValue = 42;
    dwError = VmDirGetRegKeyValueDword(
                VMDIR_CONFIG_PARAMETER_KEY_PATH,
                "TestValueDoesNotExist",
                &dwComparisonValue,
                dwTestValue);
    TestAssertNotEquals(dwError, 0);
    TestAssert(dwTestValue == dwComparisonValue);
}

VOID
TestMaxDwordValueRoundTrip(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwTestValue = 0;
    DWORD dwComparisonValue = 0;
    DWORD dwError = 0;

    dwTestValue = 0xFFFFFFFF; // Biggest possible DWORD
    dwError = VmDirSetRegKeyValueDword(
                VMDIR_CONFIG_PARAMETER_KEY_PATH,
                "TestMaxValue",
                dwTestValue);
    TestAssertEquals(dwError, 0);

    dwError = VmDirGetRegKeyValueDword(
                VMDIR_CONFIG_PARAMETER_KEY_PATH,
                "TestMaxValue",
                &dwComparisonValue,
                0);
    TestAssertEquals(dwError, 0);
    TestAssertEquals(dwTestValue, dwComparisonValue);
}

VOID
TestRegistryCode(
    PVMDIR_TEST_STATE pState
    )
{
    printf("Testing registry code ...");

    TestDwordRoundTrip(pState);
    TestDwordDefaultValue(pState);
    TestMaxDwordValueRoundTrip(pState);

    printf(" PASSED\n");
}
