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

PSTR GenerateString(
    VOID
    )
{
    static DWORD i = 0;
    PSTR pszString = NULL;
    DWORD dwError = 0;

    //
    // This is a unit test so we assume that the allocation succeeds.
    //
    dwError = VmDirAllocateStringAVsnprintf(
                &pszString,
                "Test String #%d",
                i++);
    ASSERT(dwError == 0);

    return pszString;
}

VOID
TestStringListInitialization(
    PVMDIR_STRING_LIST *ppStringList
    )
{
    PVMDIR_STRING_LIST pStringList;
    DWORD dwError = 0;

    dwError = VmDirStringListInitialize(&pStringList, 10);
    ASSERT(dwError == 0);
    ASSERT(pStringList != NULL);
    ASSERT(pStringList->dwCount == 0);
    ASSERT(pStringList->dwSize == 10);

    *ppStringList = pStringList;
}

VOID
TestStringListInitializationCountTooBig(
    VOID
    )
{
    PVMDIR_STRING_LIST pStringList = NULL;
    DWORD dwError = 0;

    dwError = VmDirStringListInitialize(&pStringList, 0xFFFFFFFF);
    ASSERT(dwError == VMDIR_ERROR_INVALID_PARAMETER);
    ASSERT(pStringList == NULL);
}

VOID
TestStringListAdd(
    PVMDIR_STRING_LIST pStringList
    )
{
    DWORD dwError = 0;
    PCSTR pszString = GenerateString();

    dwError = VmDirStringListAdd(pStringList, pszString);
    ASSERT(dwError == 0);
    ASSERT(VmDirStringListContains(pStringList, pszString));
}

VOID
TestStringListAddWithReallocation(
    PVMDIR_STRING_LIST pStringList
    )
{
    UINT i = 0;
    DWORD dwMaxSize = 0;
    DWORD dwError = 0;

    dwMaxSize = pStringList->dwSize + 5;
    for (i = pStringList->dwCount; i < dwMaxSize; ++i)
    {
        dwError = VmDirStringListAdd(
                    pStringList,
                    GenerateString());
        ASSERT(dwError == 0);
    }

    ASSERT(pStringList->dwSize > pStringList->dwCount);
    ASSERT(pStringList->dwSize > dwMaxSize);
    ASSERT(pStringList->dwCount >= dwMaxSize);
}

VOID
TestStringListAddLayout(
    VOID
    )
{
    PSTR ppszStrings[5];
    DWORD dwError = 0;
    DWORD i = 0;
    PVMDIR_STRING_LIST pStringList;

    dwError = VmDirStringListInitialize(&pStringList, 10);
    ASSERT(dwError == 0);

    for (i = 0; i < VMDIR_ARRAY_SIZE(ppszStrings); ++i)
    {
        ppszStrings[i] = GenerateString();
        dwError = VmDirStringListAdd(pStringList, ppszStrings[i]);
        ASSERT(dwError == 0);
    }

    ASSERT(pStringList->dwCount == VMDIR_ARRAY_SIZE(ppszStrings));

    for (i = 0; i < VMDIR_ARRAY_SIZE(ppszStrings); ++i)
    {
        ASSERT(pStringList->pStringList[i] == ppszStrings[i]);
    }

    VmDirStringListFree(pStringList);
}

VOID
TestStringListRemoveShouldSucceed(
    PVMDIR_STRING_LIST pStringList
    )
{
    DWORD dwError = 0;
    PCSTR pszString = GenerateString();
    DWORD dwCount = 0;

    VmDirStringListAdd(pStringList, pszString);
    ASSERT(dwError == 0);
    ASSERT(VmDirStringListContains(pStringList, pszString));
    dwCount = pStringList->dwCount;

    dwError = VmDirStringListRemove(pStringList, pszString);
    ASSERT(dwError == 0);
    ASSERT(!VmDirStringListContains(pStringList, pszString));
    ASSERT(dwCount == pStringList->dwCount + 1);
}

VOID
TestStringListRemoveShouldHaveCorrectLayout(
    VOID)
{
    PCSTR ppszStrings[] = {
        "Test 1",
        "Test 2",
        "Test 3",
        "Test 4",
        "Test 5"
    };
    PVMDIR_STRING_LIST pStringList = NULL;
    DWORD dwError = 0;
    DWORD i = 0;

    dwError = VmDirStringListInitialize(&pStringList, 10);
    ASSERT(dwError == 0);

    for (i = 0; i < VMDIR_ARRAY_SIZE(ppszStrings); ++i)
    {
        dwError = VmDirStringListAdd(pStringList, ppszStrings[i]);
        ASSERT(dwError == 0);
    }

    dwError = VmDirStringListRemove(pStringList, ppszStrings[2]);
    ASSERT(dwError == 0);
    ASSERT(pStringList->dwCount == VMDIR_ARRAY_SIZE(ppszStrings) - 1);
    ASSERT(pStringList->pStringList[0] == ppszStrings[0]);
    ASSERT(pStringList->pStringList[1] == ppszStrings[1]);
    ASSERT(pStringList->pStringList[2] == ppszStrings[3]);
    ASSERT(pStringList->pStringList[3] == ppszStrings[4]);
}

VOID
TestStringListRemoveShouldFail(
    PVMDIR_STRING_LIST pStringList
    )
{
    DWORD dwError = 0;
    PSTR pszString = GenerateString();

    dwError = VmDirStringListRemove(pStringList, pszString);
    ASSERT(dwError == VMDIR_ERROR_NOT_FOUND);
    ASSERT(!VmDirStringListContains(pStringList, pszString));
}

VOID
TestStringListRemoveNullShouldFail(
    PVMDIR_STRING_LIST pStringList
    )
{
    DWORD dwError = 0;

    dwError = VmDirStringListRemove(pStringList, NULL);
    ASSERT(dwError == VMDIR_ERROR_NOT_FOUND);
    ASSERT(!VmDirStringListContains(pStringList, NULL));
}

VOID
TestStringListContainsNullShouldFail(
    PVMDIR_STRING_LIST pStringList
    )
{
    ASSERT(!VmDirStringListContains(pStringList, NULL));
}

VOID
TestStringListContainsShouldFail(
    PVMDIR_STRING_LIST pStringList
    )
{
    ASSERT(!VmDirStringListContains(pStringList, GenerateString()));
}

VOID
TestStringListFree(
    PVMDIR_STRING_LIST pStringList
    )
{
    VmDirStringListFree(pStringList);
}

VOID
TestStringListFreeWithNull(
    VOID
    )
{
    VmDirStringListFree(NULL);
}

VOID TestVmDirStringList(
    VOID
    )
{
    PVMDIR_STRING_LIST pStringList;

    printf("Testing VmDirStringList code ...\n");
    TestStringListInitialization(&pStringList);
    TestStringListInitializationCountTooBig();
    TestStringListAdd(pStringList);
    TestStringListAddWithReallocation(pStringList);
    TestStringListAddLayout();
    TestStringListRemoveShouldSucceed(pStringList);
    TestStringListRemoveShouldHaveCorrectLayout();
    TestStringListRemoveShouldFail(pStringList);
    TestStringListRemoveNullShouldFail(pStringList);
    TestStringListContainsNullShouldFail(pStringList);
    TestStringListContainsShouldFail(pStringList);
    TestStringListFree(pStringList);
    TestStringListFreeWithNull();
}
