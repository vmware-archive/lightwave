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
_Test_VmDirStringToTokenList_NullSourceString(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PVMDIR_STRING_LIST pList = NULL;

    dwError = VmDirStringToTokenList( NULL, ",", &pList);

    TestAssertEquals(dwError, VMDIR_ERROR_INVALID_PARAMETER);
}

VOID
_Test_VmDirStringToTokenList_NullDelimiterString(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PVMDIR_STRING_LIST pList = NULL;

    dwError = VmDirStringListInitialize(&pList, 10);
    TestAssertEquals(dwError, 0);

    dwError = VmDirStringToTokenList( "abcdefghijklmnop", NULL, &pList);
    TestAssertEquals(dwError, VMDIR_ERROR_INVALID_PARAMETER);
}

VOID
_Test_VmDirStringToTokenList_NullStringList(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;

    dwError = VmDirStringToTokenList( "abcdefghijklmnop", ",", NULL);
    TestAssertEquals(dwError, VMDIR_ERROR_INVALID_PARAMETER);
}

VOID
_Test_VmDirStringToTokenList_StringIsDelimiter(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PVMDIR_STRING_LIST pList = NULL;

    dwError = VmDirStringToTokenList( ",", ",", &pList);
    TestAssertEquals(dwError, 0);
    TestAssertEquals(pList->dwCount, 0);
}

VOID
_Test_VmDirStringToTokenList_NoDelimiters(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PVMDIR_STRING_LIST pList = NULL;

    dwError = VmDirStringListInitialize(&pList, 10);
    TestAssertEquals(dwError, 0);

    dwError = VmDirStringToTokenList( "abcdefghijklmnopqrstuvwxyz1234567890", ",", &pList);
    TestAssertEquals(dwError, 0);

    TestAssertEquals(pList->dwCount,  1);
    TestAssertStrEquals(pList->pStringList[0], "abcdefghijklmnopqrstuvwxyz1234567890");
}

VOID
_Test_VmDirStringToTokenList_StartWithDelimiter(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PVMDIR_STRING_LIST pList = NULL;

    dwError = VmDirStringListInitialize(&pList, 10);
    TestAssertEquals(dwError, 0);

    dwError = VmDirStringToTokenList(",abcdefghijklmnopqrstuvwxyz1234567890", ",", &pList);
    TestAssertEquals(dwError, 0);

    TestAssertEquals(pList->dwCount, 1);
    TestAssertStrEquals(pList->pStringList[0], "abcdefghijklmnopqrstuvwxyz1234567890");
}

VOID
_Test_VmDirStringToTokenList_EndWithDelimiter(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PVMDIR_STRING_LIST pList = NULL;

    dwError = VmDirStringListInitialize(&pList, 10);
    TestAssertEquals(dwError, 0);

    dwError = VmDirStringToTokenList( "abcdefghijklmnopqrstuvwxyz1234567890,", ",", &pList);
    TestAssertEquals(dwError, 0);

    TestAssertEquals(pList->dwCount, 1);
    TestAssertStrEquals(pList->pStringList[0], "abcdefghijklmnopqrstuvwxyz1234567890");
}

VOID
_Test_VmDirStringToTokenList_DelimiterInMiddle(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PVMDIR_STRING_LIST pList = NULL;

    dwError = VmDirStringListInitialize(&pList, 10);
    TestAssertEquals(dwError, 0);

    dwError = VmDirStringToTokenList( "abcdefghijklmnopqrstuvwxyz,1234567890", ",", &pList);
    TestAssertEquals(dwError, 0);

    TestAssertEquals(pList->dwCount, 2);
    TestAssertStrEquals(pList->pStringList[0], "abcdefghijklmnopqrstuvwxyz");
    TestAssertStrEquals(pList->pStringList[1], "1234567890");
}

VOID
_Test_VmDirStringToTokenList_MultipleDelimiters(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PVMDIR_STRING_LIST pList = NULL;

    dwError = VmDirStringListInitialize(&pList, 10);
    TestAssertEquals(dwError, 0);

    dwError = VmDirStringToTokenList( ",abcdefghijklm,nopqrstuvwxyz,1234567,890,", ",", &pList);
    TestAssertEquals(dwError, 0);

    TestAssertEquals(pList->dwCount, 4);
    TestAssertStrEquals(pList->pStringList[0], "abcdefghijklm");
    TestAssertStrEquals(pList->pStringList[1], "nopqrstuvwxyz");
    TestAssertStrEquals(pList->pStringList[2], "1234567");
    TestAssertStrEquals(pList->pStringList[3], "890");
}

VOID
_Test_VmDirStringToTokenList_MulticharDelimiterInMiddle(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    PVMDIR_STRING_LIST pList = NULL;

    dwError = VmDirStringListInitialize(&pList, 10);
    TestAssertEquals(dwError, 0);

    dwError = VmDirStringToTokenList( "abcdefghijklmnopqrstuvwxyz:;:1234567890", ":;:", &pList);
    TestAssertEquals(dwError, 0);

    TestAssertEquals(pList->dwCount, 2);
    TestAssertStrEquals(pList->pStringList[0], "abcdefghijklmnopqrstuvwxyz");
    TestAssertStrEquals(pList->pStringList[1], "1234567890");
}

VOID
TestVmDirStringToTokenList(
    PVMDIR_TEST_STATE pState
    )
{
    printf("Testing VmDirStringToTokenList ...");

    _Test_VmDirStringToTokenList_NullSourceString(pState);
    _Test_VmDirStringToTokenList_NullDelimiterString(pState);
    _Test_VmDirStringToTokenList_NullStringList(pState);
    _Test_VmDirStringToTokenList_StringIsDelimiter(pState);
    _Test_VmDirStringToTokenList_NoDelimiters(pState);
    _Test_VmDirStringToTokenList_StartWithDelimiter(pState);
    _Test_VmDirStringToTokenList_EndWithDelimiter(pState);
    _Test_VmDirStringToTokenList_DelimiterInMiddle(pState);
    _Test_VmDirStringToTokenList_MultipleDelimiters(pState);
    _Test_VmDirStringToTokenList_MulticharDelimiterInMiddle(pState);

    printf(" PASSED\n");
}
