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
    PVMDIR_TEST_STATE pState
    )
{
    static DWORD i = 0;
    PSTR pszString = NULL;
    DWORD dwError = 0;

    //
    // This is a unit test so we assume that the allocation succeeds.
    //
    dwError = VmDirAllocateStringPrintf(
                &pszString,
                "Test String #%d",
                i++);
    TestAssertEquals(dwError, 0);

    return pszString;
}

VOID
TestStringListInitialization(
    PVMDIR_TEST_STATE pState,
    PVMDIR_STRING_LIST *ppStringList
    )
{
    PVMDIR_STRING_LIST pStringList;
    DWORD dwError = 0;

    dwError = VmDirStringListInitialize(&pStringList, 10);
    TestAssertEquals(dwError, 0);
    TestAssert(pStringList != NULL);
    TestAssert(pStringList->dwCount == 0);
    TestAssert(pStringList->dwSize == 10);

    *ppStringList = pStringList;
}

VOID
TestStringListInitializationCountTooBig(
    PVMDIR_TEST_STATE pState
    )
{
    PVMDIR_STRING_LIST pStringList = NULL;
    DWORD dwError = 0;

    dwError = VmDirStringListInitialize(&pStringList, 0xFFFFFFFF);
    TestAssert(dwError == VMDIR_ERROR_INVALID_PARAMETER);
    TestAssert(pStringList == NULL);
}

VOID
TestStringListAdd(
    PVMDIR_TEST_STATE pState,
    PVMDIR_STRING_LIST pStringList
    )
{
    DWORD dwError = 0;
    PCSTR pszString = GenerateString(pState);

    dwError = VmDirStringListAdd(pStringList, pszString);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringListContains(pStringList, pszString));
}

VOID
TestStringListAddWithReallocation(
    PVMDIR_TEST_STATE pState,
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
                    GenerateString(pState));
        TestAssertEquals(dwError, 0);
    }

    TestAssert(pStringList->dwSize > pStringList->dwCount);
    TestAssert(pStringList->dwSize > dwMaxSize);
    TestAssert(pStringList->dwCount >= dwMaxSize);
}

VOID
TestStringListAddLayout(
    PVMDIR_TEST_STATE pState
    )
{
    PSTR ppszStrings[5];
    DWORD dwError = 0;
    DWORD i = 0;
    PVMDIR_STRING_LIST pStringList;

    dwError = VmDirStringListInitialize(&pStringList, 10);
    TestAssertEquals(dwError, 0);

    for (i = 0; i < VMDIR_ARRAY_SIZE(ppszStrings); ++i)
    {
        ppszStrings[i] = GenerateString(pState);
        dwError = VmDirStringListAdd(pStringList, ppszStrings[i]);
        TestAssertEquals(dwError, 0);
    }

    TestAssert(pStringList->dwCount == VMDIR_ARRAY_SIZE(ppszStrings));

    for (i = 0; i < VMDIR_ARRAY_SIZE(ppszStrings); ++i)
    {
        TestAssert(pStringList->pStringList[i] == ppszStrings[i]);
    }

    VmDirStringListFree(pStringList);
}

VOID
TestStringListRemoveShouldSucceed(
    PVMDIR_TEST_STATE pState,
    PVMDIR_STRING_LIST pStringList
    )
{
    DWORD dwError = 0;
    PCSTR pszString = GenerateString(pState);
    DWORD dwCount = 0;

    VmDirStringListAdd(pStringList, pszString);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringListContains(pStringList, pszString));
    dwCount = pStringList->dwCount;

    dwError = VmDirStringListRemove(pStringList, pszString);
    TestAssertEquals(dwError, 0);
    TestAssert(!VmDirStringListContains(pStringList, pszString));
    TestAssert(dwCount == pStringList->dwCount + 1);
}

VOID
TestStringListRemoveShouldHaveCorrectLayout(
    PVMDIR_TEST_STATE pState
    )
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
    TestAssertEquals(dwError, 0);

    for (i = 0; i < VMDIR_ARRAY_SIZE(ppszStrings); ++i)
    {
        dwError = VmDirStringListAdd(pStringList, ppszStrings[i]);
        TestAssertEquals(dwError, 0);
    }

    dwError = VmDirStringListRemove(pStringList, ppszStrings[2]);
    TestAssertEquals(dwError, 0);
    TestAssert(pStringList->dwCount == VMDIR_ARRAY_SIZE(ppszStrings) - 1);
    TestAssert(pStringList->pStringList[0] == ppszStrings[0]);
    TestAssert(pStringList->pStringList[1] == ppszStrings[1]);
    TestAssert(pStringList->pStringList[2] == ppszStrings[3]);
    TestAssert(pStringList->pStringList[3] == ppszStrings[4]);
}

VOID
TestStringListRemoveShouldFail(
    PVMDIR_TEST_STATE pState,
    PVMDIR_STRING_LIST pStringList
    )
{
    DWORD dwError = 0;
    PSTR pszString = GenerateString(pState);

    dwError = VmDirStringListRemove(pStringList, pszString);
    TestAssert(dwError == VMDIR_ERROR_NOT_FOUND);
    TestAssert(!VmDirStringListContains(pStringList, pszString));
}

VOID
TestStringListRemoveNullShouldFail(
    PVMDIR_TEST_STATE pState,
    PVMDIR_STRING_LIST pStringList
    )
{
    DWORD dwError = 0;

    dwError = VmDirStringListRemove(pStringList, NULL);
    TestAssert(dwError == VMDIR_ERROR_NOT_FOUND);
    TestAssert(!VmDirStringListContains(pStringList, NULL));
}

VOID
TestStringListContainsNullShouldFail(
    PVMDIR_TEST_STATE pState,
    PVMDIR_STRING_LIST pStringList
    )
{
    TestAssert(!VmDirStringListContains(pStringList, NULL));
}

VOID
TestStringListContainsShouldFail(
    PVMDIR_TEST_STATE pState,
    PVMDIR_STRING_LIST pStringList
    )
{
    TestAssert(!VmDirStringListContains(pStringList, GenerateString(pState)));
}

VOID
TestStringListFree(
    PVMDIR_TEST_STATE pState,
    PVMDIR_STRING_LIST pStringList
    )
{
    VmDirStringListFree(pStringList);
}

VOID
TestStringListFreeWithNull(
    PVMDIR_TEST_STATE pState
    )
{
    VmDirStringListFree(NULL);
}

VOID
TestStringListMultiStringRoutines(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    DWORD dwByteCount = 0;
    PVMDIR_STRING_LIST pStringList = NULL;
    PSTR pszMultiStringResult = NULL;
    BYTE pbMultiString[] = {
        'l', 'o', 't', 'u', 's', '\0',
        'v', 'm', 'd', 'i', 'r', '\0',
        'v', 'm', 'w', 'a', 'r', 'e', '\0', '\0'};

    dwError = VmDirStringListFromMultiString(
                (PCSTR)pbMultiString,
                0,
                &pStringList);
    TestAssertEquals(dwError, 0);
    TestAssertEquals(pStringList->dwCount, 3);
    TestAssertStrEquals(pStringList->pStringList[0], "lotus");
    TestAssertStrEquals(pStringList->pStringList[1], "vmdir");
    TestAssertStrEquals(pStringList->pStringList[2], "vmware");

    dwError = VmDirMultiStringFromStringList(
                pStringList,
                &pszMultiStringResult,
                &dwByteCount);
    TestAssertEquals(dwError, 0);

    TestAssertEquals(dwByteCount, sizeof(pbMultiString));
    TestAssert(memcmp(pbMultiString, pszMultiStringResult, dwByteCount) == 0);
}


VOID TestVmDirStringList(
    PVMDIR_TEST_STATE pState
    )
{
    PVMDIR_STRING_LIST pStringList;

    printf("Testing VmDirStringList code ...");
    TestStringListInitialization(pState, &pStringList);
    TestStringListInitializationCountTooBig(pState);
    TestStringListAdd(pState, pStringList);
    TestStringListAddWithReallocation(pState, pStringList);
    TestStringListAddLayout(pState);
    TestStringListRemoveShouldSucceed(pState, pStringList);
    TestStringListRemoveShouldHaveCorrectLayout(pState);
    TestStringListRemoveShouldFail(pState, pStringList);
    TestStringListRemoveNullShouldFail(pState, pStringList);
    TestStringListContainsNullShouldFail(pState, pStringList);
    TestStringListContainsShouldFail(pState, pStringList);
    TestStringListFree(pState, pStringList);
    TestStringListFreeWithNull(pState);
    TestStringListMultiStringRoutines(pState);

    printf(" PASSED\n");
}
