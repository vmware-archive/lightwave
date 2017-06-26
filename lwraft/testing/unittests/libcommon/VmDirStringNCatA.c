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
_Test_VmDirStringNCatA_NullSourceString(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    CHAR szDestination[16] = { 0 };

    dwError = VmDirStringNCatA(
                &szDestination[0],
                VMDIR_ARRAY_SIZE(szDestination),
                NULL,
                0);
    TestAssertEquals(dwError, ERROR_INVALID_PARAMETER);
    TestAssert(szDestination[0] == 0);
}

VOID
_Test_VmDirStringNCatA_EmptySourceString(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    CHAR szDestination[16] = { 0 };

    dwError = VmDirStringNCatA(
                &szDestination[0],
                VMDIR_ARRAY_SIZE(szDestination),
                "",
                0);
    TestAssertEquals(dwError, 0);
    TestAssertEquals(szDestination[0], '\0');
}

VOID
_Test_VmDirStringNCatA_NullDestinationString(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;

    dwError = VmDirStringNCatA(NULL, 4, "test", 3);
    TestAssertEquals(dwError, ERROR_INVALID_PARAMETER);
}

VOID
_Test_VmDirStringNCatA_EmptyDestinationString(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    CHAR szString[16] = { 0 };

    dwError = VmDirStringNCatA(
                szString,
                VMDIR_ARRAY_SIZE(szString),
                "test",
                3);
    TestAssertEquals(dwError, 0);
    TestAssertStrEquals(szString, "tes");
}

VOID
_Test_VmDirStringNCatA_SourceStringTooLong(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    CHAR szDestination[4] = { 0 };

    dwError = VmDirStringNCatA(
                &szDestination[0],
                VMDIR_ARRAY_SIZE(szDestination),
                "Hello, world!",
                strlen("Hello, world!"));
    TestAssertEquals(dwError, ERROR_INVALID_PARAMETER);
    TestAssertEquals(szDestination[0], 0);
}

VOID
_Test_VmDirStringNCatA_CallShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    CHAR szDestination[128] = { 'H', 'e', 'l', 'l', 'o', 0 };

    dwError = VmDirStringNCatA(
                szDestination,
                VMDIR_ARRAY_SIZE(szDestination),
                ", world!",
                8);
    TestAssertEquals(dwError, 0);
    TestAssertStrEquals(szDestination, "Hello, world!");
}

VOID
TestVmDirStringNCatA(
    PVMDIR_TEST_STATE pState
    )
{
    printf("Testing VmDirStringNCatA ...");

    _Test_VmDirStringNCatA_NullSourceString(pState);
    _Test_VmDirStringNCatA_EmptySourceString(pState);
    _Test_VmDirStringNCatA_NullDestinationString(pState);
    _Test_VmDirStringNCatA_EmptyDestinationString(pState);
    _Test_VmDirStringNCatA_SourceStringTooLong(pState);
    _Test_VmDirStringNCatA_CallShouldSucceed(pState);

    printf(" PASSED\n");
}
