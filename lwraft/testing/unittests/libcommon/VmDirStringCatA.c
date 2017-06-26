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
_Test_VmDirStringCatA_NullSourceString(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    CHAR szDestination[16] = { 0 };

    dwError = VmDirStringCatA(
                &szDestination[0],
                VMDIR_ARRAY_SIZE(szDestination),
                NULL);
    TestAssertEquals(dwError, ERROR_INVALID_PARAMETER);
    TestAssert(szDestination[0] == 0);
}

VOID
_Test_VmDirStringCatA_EmptySourceString(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    CHAR szDestination[16] = { 'a', 'b', 'c', 0 };

    dwError = VmDirStringCatA(
                &szDestination[0],
                VMDIR_ARRAY_SIZE(szDestination),
                "");
    TestAssertEquals(dwError, 0);
    TestAssert(strcmp(szDestination, "abc") == 0);
}

VOID
_Test_VmDirStringCatA_NullDestinationString(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;

    dwError = VmDirStringCatA(NULL, 4, "test");
    TestAssertEquals(dwError, ERROR_INVALID_PARAMETER);
}

VOID
_Test_VmDirStringCatA_EmptyDestinationString(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    CHAR szString[16] = { 0 };

    dwError = VmDirStringCatA(
                szString,
                VMDIR_ARRAY_SIZE(szString),
                "test");
    TestAssertEquals(dwError, 0);
    TestAssertStrEquals(szString, "test");
}

VOID
_Test_VmDirStringCatA_SourceStringTooLong(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    CHAR szDestination[4] = { 0 };

    dwError = VmDirStringCatA(
                &szDestination[0],
                VMDIR_ARRAY_SIZE(szDestination),
                "Hello, world!");
    TestAssertEquals(dwError, ERROR_INVALID_PARAMETER);
    TestAssert(szDestination[0] == 0);
}

VOID
_Test_VmDirStringCatA_CallShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    CHAR szDestination[14] = { 0 };

    dwError = VmDirStringCatA(
                &szDestination[0],
                VMDIR_ARRAY_SIZE(szDestination),
                "Hello, world!");
    TestAssertEquals(dwError, 0);
    TestAssertStrEquals(szDestination, "Hello, world!");
}


VOID
TestVmDirStringCatA(
    PVMDIR_TEST_STATE pState
    )
{
    printf("Testing VmDirStringCatA ...");

    _Test_VmDirStringCatA_NullSourceString(pState);
    _Test_VmDirStringCatA_EmptySourceString(pState);
    _Test_VmDirStringCatA_NullDestinationString(pState);
    _Test_VmDirStringCatA_EmptyDestinationString(pState);
    _Test_VmDirStringCatA_SourceStringTooLong(pState);
    _Test_VmDirStringCatA_CallShouldSucceed(pState);

    printf(" PASSED\n");
}
