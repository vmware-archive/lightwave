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
_Test_VmdirAllocateStringOfLenA_NullSourceString(
    VOID
    )
{
    DWORD dwError = 0;
    PSTR pszString = (PSTR)0xDEADBEEF;

    dwError = VmDirAllocateStringOfLenA(NULL, 0, &pszString);
    ASSERT(dwError == 0);
    ASSERT(pszString == NULL);
}

VOID
_Test_VmdirAllocateStringOfLenA_EmptySourceString(
    VOID
    )
{
    DWORD dwError = 0;
    PSTR pszString = NULL;

    dwError = VmDirAllocateStringOfLenA("", 0, &pszString);
    ASSERT(dwError == 0);
    ASSERT(*pszString == '\0');
}

VOID
_Test_VmdirAllocateStringOfLenA_NullDestinationString(
    VOID
    )
{
    DWORD dwError = 0;

    dwError = VmDirAllocateStringOfLenA("test", 2, NULL);
    ASSERT(dwError == 0);
}

VOID
_Test_VmdirAllocateStringOfLenA_TooManyCharactersRequestedShouldFail(
    VOID
    )
{
    DWORD dwError = 0;
    PSTR pszString = NULL;

    dwError = VmDirAllocateStringOfLenA("Hello, world!", 20, &pszString);
    ASSERT(dwError == VMDIR_ERROR_INVALID_PARAMETER);
}

VOID
_Test_VmdirAllocateStringOfLenA_CallShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    PSTR pszString = NULL;

    dwError = VmDirAllocateStringOfLenA("Hello, world!", 5, &pszString);
    ASSERT(dwError == 0);
    ASSERT(strcmp(pszString, "Hello") == 0);
}


VOID
TestVmDirAllocateStringOfLenA(
    VOID
    )
{
    printf("Testing VmDirAllocateStringOfLenA ...\n");
    _Test_VmdirAllocateStringOfLenA_NullSourceString();
    _Test_VmdirAllocateStringOfLenA_EmptySourceString();
    _Test_VmdirAllocateStringOfLenA_NullDestinationString();
    _Test_VmdirAllocateStringOfLenA_TooManyCharactersRequestedShouldFail();
    _Test_VmdirAllocateStringOfLenA_CallShouldSucceed();
}
