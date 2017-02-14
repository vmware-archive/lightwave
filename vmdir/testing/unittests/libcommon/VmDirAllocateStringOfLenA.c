/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
_Test_VmDirAllocateStringOfLenA_NullSourceString(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PSTR pszString = (PSTR)0xDEADBEEF;

    dwError = VmDirAllocateStringOfLenA(NULL, 0, &pszString);
    TestAssertEquals(dwError, 0);
    TestAssert(pszString == NULL);
}

VOID
_Test_VmDirAllocateStringOfLenA_EmptySourceString(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PSTR pszString = NULL;

    dwError = VmDirAllocateStringOfLenA("", 0, &pszString);
    TestAssertEquals(dwError, 0);
    TestAssert(*pszString == '\0');
}

VOID
_Test_VmDirAllocateStringOfLenA_NullDestinationString(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;

    dwError = VmDirAllocateStringOfLenA("test", 2, NULL);
    TestAssertEquals(dwError, 0);
}

VOID
_Test_VmDirAllocateStringOfLenA_TooManyCharactersRequestedShouldFail(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PSTR pszString = NULL;

    dwError = VmDirAllocateStringOfLenA("Hello, world!", 20, &pszString);
    TestAssert(dwError == VMDIR_ERROR_INVALID_PARAMETER);
}

VOID
_Test_VmDirAllocateStringOfLenA_CallShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PSTR pszString = NULL;

    dwError = VmDirAllocateStringOfLenA("Hello, world!", 5, &pszString);
    TestAssertEquals(dwError, 0);
    TestAssertStrEquals(pszString, "Hello");
}


VOID
TestVmDirAllocateStringOfLenA(
    PVMDIR_TEST_STATE pState
    )
{
    printf("Testing VmDirAllocateStringOfLenA ...");

    _Test_VmDirAllocateStringOfLenA_NullSourceString(pState);
    _Test_VmDirAllocateStringOfLenA_EmptySourceString(pState);
    _Test_VmDirAllocateStringOfLenA_NullDestinationString(pState);
    _Test_VmDirAllocateStringOfLenA_TooManyCharactersRequestedShouldFail(pState);
    _Test_VmDirAllocateStringOfLenA_CallShouldSucceed(pState);

    printf(" PASSED\n");
}
