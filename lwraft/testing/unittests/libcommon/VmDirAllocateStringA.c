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
_Test_VmDirAllocateStringA_NullSourceString(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PSTR pszString = (PSTR)0xDEADBEEF;

    dwError = VmDirAllocateStringA(NULL, &pszString);
    TestAssertEquals(dwError, 0);
    TestAssert(pszString == NULL);
}

VOID
_Test_VmDirAllocateStringA_EmptySourceString(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PSTR pszString = NULL;

    dwError = VmDirAllocateStringA("", &pszString);
    TestAssertEquals(dwError, 0);
    TestAssert(*pszString == '\0');
}

VOID
_Test_VmDirAllocateStringA_NullDestinationString(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;

    dwError = VmDirAllocateStringA("test", NULL);
    TestAssertEquals(dwError, 0);
}

VOID
_Test_VmDirAllocateStringA_CallShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PSTR pszString = NULL;

    dwError = VmDirAllocateStringA("Hello, world!", &pszString);
    TestAssertEquals(dwError, 0);
    TestAssertStrEquals(pszString, "Hello, world!");
}


VOID
TestVmDirAllocateStringA(
    PVMDIR_TEST_STATE pState
    )
{
    printf("Testing VmDirAllocateStringA ...");

    _Test_VmDirAllocateStringA_NullSourceString(pState);
    _Test_VmDirAllocateStringA_EmptySourceString(pState);
    _Test_VmDirAllocateStringA_NullDestinationString(pState);
    _Test_VmDirAllocateStringA_CallShouldSucceed(pState);

    printf(" PASSED\n");
}
