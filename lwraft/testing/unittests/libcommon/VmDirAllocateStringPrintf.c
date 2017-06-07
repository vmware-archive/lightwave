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


DWORD
_TestVmDirAllocateStringPrintfWithBadParameters(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PSTR pszString = NULL;

    dwError = VmDirAllocateStringPrintf(
                NULL,
                "dword ==> %d, string ==> '%s'",
                42,
                "Hello, world!");
    TestAssertEquals(dwError, ERROR_INVALID_PARAMETER);

    dwError = VmDirAllocateStringPrintf(
                &pszString,
                NULL,
                42,
                "Hello, world!");
    TestAssertEquals(dwError, ERROR_INVALID_PARAMETER);

    return 0;
}

DWORD
_TestVmDirAllocateStringPrintfWithGoodParameters(
    PVMDIR_TEST_STATE pState
    )
{
    PSTR pszString = NULL;
    DWORD dwError = 0;

    dwError = VmDirAllocateStringPrintf(
                &pszString,
                "dword ==> %d, string ==> '%s'",
                (DWORD)42,
                "Hello, world!");
    TestAssertEquals(dwError, ERROR_SUCCESS);
    TestAssertStrEquals(pszString, "dword ==> 42, string ==> 'Hello, world!'");

    return 0;
}

DWORD
TestVmDirAllocateStringPrintf(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;

    dwError = _TestVmDirAllocateStringPrintfWithBadParameters(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _TestVmDirAllocateStringPrintfWithGoodParameters(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}
