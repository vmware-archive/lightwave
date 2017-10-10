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
_Test_VmDirStringNCpyA_NullSourceString(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    CHAR szDestination[16] = { 'a' };

    dwError = VmDirStringNCpyA(
                szDestination,
                VMDIR_ARRAY_SIZE(szDestination),
                NULL,
                10);
    TestAssertEquals(dwError, ERROR_INVALID_PARAMETER);
    TestAssertEquals(szDestination[0], 'a');
}

VOID
_Test_VmDirStringNCpyA_EmptySourceStringRightCount(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    CHAR szDestination[16] = { '\0' };

    //
    // This call will yield different results on windows and linux. On
    // the former we call the _s version of strncpy so the string's first byte
    // will be NULL, regardless of what it is coming into the call. However,
    // on linux the string will be untouched after the call.
    //
    dwError = VmDirStringNCpyA(
                szDestination,
                VMDIR_ARRAY_SIZE(szDestination),
                "",
                0);
    TestAssertEquals(dwError, 0);
    TestAssertEquals(szDestination[0], '\0');
}

VOID
_Test_VmDirStringNCpyA_EmptySourceStringWrongCount(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    CHAR szDestination[16] = { 'a' };

    dwError = VmDirStringNCpyA(
                szDestination,
                VMDIR_ARRAY_SIZE(szDestination),
                "",
                10);
    TestAssertEquals(dwError, 0);
    TestAssertEquals(szDestination[0], '\0');
}

VOID
_Test_VmDirStringNCpyA_NullDestinationString(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;

    dwError = VmDirStringNCpyA(NULL, 4, "test", 4);
    TestAssertEquals(dwError, ERROR_INVALID_PARAMETER);
}

VOID
_Test_VmDirStringNCpyA_SourceStringTooLong(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    CHAR szDestination[4] = { 'a' };

    dwError = VmDirStringNCpyA(
                szDestination,
                VMDIR_ARRAY_SIZE(szDestination),
                "Hello, world!",
                7);
    TestAssertEquals(dwError, ERROR_INVALID_PARAMETER);
    TestAssertEquals(szDestination[0], 'a');
}

VOID
_Test_VmDirStringNCpyA_CallShouldSucceedWithoutTruncation(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    CHAR szDestination[128] = { 'a' };

    dwError = VmDirStringNCpyA(
                szDestination,
                VMDIR_ARRAY_SIZE(szDestination),
                "Hello, world!",
                strlen("Hello, world!"));
    TestAssertEquals(dwError, 0);
    TestAssertStrEquals(szDestination, "Hello, world!");
}

VOID
_Test_VmDirStringNCpyA_CallShouldSucceedExactlyRightSize(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    CHAR szDestination[14] = { 'a' };

    dwError = VmDirStringNCpyA(
                szDestination,
                VMDIR_ARRAY_SIZE(szDestination),
                "Hello, world!",
                VMDIR_ARRAY_SIZE(szDestination) - 1);
    TestAssertEquals(dwError, 0);
    TestAssertStrEquals(szDestination, "Hello, world!");
}

VOID
_Test_VmDirStringNCpyA_CallShouldSucceedWithTruncation(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    CHAR szDestination[6] = { 'a' };

    dwError = VmDirStringNCpyA(
                szDestination,
                VMDIR_ARRAY_SIZE(szDestination),
                "Hello, world!",
                VMDIR_ARRAY_SIZE(szDestination) - 1);
    TestAssertEquals(dwError, 0);
    TestAssertStrEquals(szDestination, "Hello");
}

VOID
_Test_VmDirStringNCpyA_CallShouldFailCountMatchesSize(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    CHAR szBuffer[4] = { 'a' };

    dwError = VmDirStringNCpyA(
                szBuffer,
                VMDIR_ARRAY_SIZE(szBuffer),
                "abcd",
                4);
    TestAssertEquals(dwError, ERROR_INVALID_PARAMETER);
    TestAssertEquals(szBuffer[0], 'a');
}

VOID
TestVmDirStringNCpyA(
    PVMDIR_TEST_STATE pState
    )
{
    printf("Testing VmDirStringNCpyA ...");

    _Test_VmDirStringNCpyA_NullSourceString(pState);
    _Test_VmDirStringNCpyA_EmptySourceStringRightCount(pState);
    _Test_VmDirStringNCpyA_EmptySourceStringWrongCount(pState);
    _Test_VmDirStringNCpyA_NullDestinationString(pState);
    _Test_VmDirStringNCpyA_SourceStringTooLong(pState);
    _Test_VmDirStringNCpyA_CallShouldFailCountMatchesSize(pState);
    _Test_VmDirStringNCpyA_CallShouldSucceedWithTruncation(pState);
    _Test_VmDirStringNCpyA_CallShouldSucceedWithoutTruncation(pState);
    _Test_VmDirStringNCpyA_CallShouldSucceedExactlyRightSize(pState);

    printf(" PASSED\n");
}
