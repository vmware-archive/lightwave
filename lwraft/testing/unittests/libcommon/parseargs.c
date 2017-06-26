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

typedef struct
{
    PSTR pszString1;
    PSTR pszString2;
    PSTR pszString3;
    DWORD dwInteger1;
    DWORD dwInteger2;
    DWORD dwInteger3;
    BOOLEAN bBoolean1;
    BOOLEAN bBoolean2;
    BOOLEAN bBoolean3;
    BOOLEAN bShowUsageTriggered;
    BOOLEAN bPostValidationCallbackTriggered;
    BOOLEAN bReturnFailure;
} COMMAND_LINE_STATE, *PCOMMAND_LINE_STATE;

DWORD
PostValidateParameters(
    PVOID pvParameter
    )
{
    PCOMMAND_LINE_STATE State = (PCOMMAND_LINE_STATE)pvParameter;

    State->bPostValidationCallbackTriggered = TRUE;

    if (State->bReturnFailure)
    {
        return VMDIR_ERROR_INVALID_PARAMETER;
    }
    else
    {
        return 0;
    }
}

VOID
ShowUsage(
    PVOID pvParameter
    )
{
    PCOMMAND_LINE_STATE State = (PCOMMAND_LINE_STATE)pvParameter;

    State->bShowUsageTriggered = TRUE;
}

VOID
FreeCLStateContent(
    PCOMMAND_LINE_STATE pState
    )
{
    if (pState)
    {
        VMDIR_SAFE_FREE_MEMORY(pState->pszString1);
        VMDIR_SAFE_FREE_MEMORY(pState->pszString2);
        VMDIR_SAFE_FREE_MEMORY(pState->pszString3);
    }
}

DWORD
VmDirParseArgumentsTestWrapper(
    PCOMMAND_LINE_STATE pState,
    int argc,
    PSTR* argv
    )
{
    VMDIR_COMMAND_LINE_OPTION Options[] =
    {
            {'s', "string1", CL_STRING_PARAMETER, &pState->pszString1},
            {'t', "string2", CL_STRING_PARAMETER, &pState->pszString2},
            {'u', "string3", CL_STRING_PARAMETER, &pState->pszString3},
            {'i', "integer1", CL_INTEGER_PARAMETER, &pState->dwInteger1},
            {'j', "integer2", CL_INTEGER_PARAMETER, &pState->dwInteger2},
            {'k', "integer3", CL_INTEGER_PARAMETER, &pState->dwInteger3},
            {'n', "noparameter1", CL_NO_PARAMETER, &pState->bBoolean1},
            {'o', "noparameter2", CL_NO_PARAMETER, &pState->bBoolean2},
            {'p', "noparameter3", CL_NO_PARAMETER, &pState->bBoolean3},
            {0, 0, 0, 0}
    };

    VMDIR_PARSE_ARG_CALLBACKS Callbacks =
    {
            PostValidateParameters,
            ShowUsage,
            pState
    };

    return VmDirParseArguments(Options, &Callbacks, argc, argv);
}

VOID
_Test_WithInvalidEnumValueFails(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app", "-s"};

    VMDIR_COMMAND_LINE_OPTION Options[] =
    {
            {'s', NULL, 0xFFFFFFFF, &Parameters.pszString1},
            {0, 0, 0, 0}
    };

    dwError = VmDirParseArguments(Options, NULL, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssert(dwError == VMDIR_ERROR_INVALID_PARAMETER);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_WithNullLongFlagDoesntCrash(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app", "--invalid"};

    VMDIR_COMMAND_LINE_OPTION Options[] =
    {
            {'s', NULL, CL_STRING_PARAMETER, &Parameters.pszString1},
            {0, 0, 0, 0}
    };

    dwError = VmDirParseArguments(Options, NULL, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssert(dwError == VMDIR_ERROR_INVALID_PARAMETER);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_WithEmptyLongFlagDoesntCrash(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app", "--invalid"};

    VMDIR_COMMAND_LINE_OPTION Options[] =
    {
            {'s', "", CL_STRING_PARAMETER, &Parameters.pszString1},
            {0, 0, 0, 0}
    };

    dwError = VmDirParseArguments(Options, NULL, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssert(dwError == VMDIR_ERROR_INVALID_PARAMETER);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_StringParameterWithNoParameterShouldFail(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app", "-s"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssert(dwError == VMDIR_ERROR_INVALID_PARAMETER);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_IntegerParameterWithNoParameterShouldFail(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app", "-i"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssert(dwError == VMDIR_ERROR_INVALID_PARAMETER);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_IntegerParameterWithStringParameterShouldFail(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app", "-i", "hello"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssert(dwError == VMDIR_ERROR_INVALID_PARAMETER);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_NoParameterWithParameterShouldFail(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app", "-n", "extraparameter"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssert(dwError == VMDIR_ERROR_INVALID_PARAMETER);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortStringParameterWithStringParameterShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app", "-s", "hello"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(strcmp(Parameters.pszString1, "hello") == 0);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortIntegerParameterWithIntegerParameterShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app", "-i", "42"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(Parameters.dwInteger1 = 42);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortNoParameterWithNoParameterShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app", "-n"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongStringParameterWithStringParameterShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app", "--string1", "hello, world!"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(strcmp(Parameters.pszString1, "hello, world!") == 0);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongIntegerParameterWithIntegerParameterShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app", "--integer1", "-37"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(Parameters.dwInteger1 == (DWORD)-37);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongNoParameterWithNoParameterShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app", "--noparameter1"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_InvalidParametersShowUsageShouldBeCalled(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app", "--invalid-parameter"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssert(dwError == VMDIR_ERROR_INVALID_PARAMETER);
    TestAssert(Parameters.bShowUsageTriggered);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ValidParametersPostValidtionShouldBeCalled(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app", "--noparameter1"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(Parameters.bPostValidationCallbackTriggered);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ValidParametersPostValidtionShouldBeCalledAndShowUsage(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app", "--noparameter1"};

    Parameters.bReturnFailure = TRUE;
    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssert(dwError == VMDIR_ERROR_INVALID_PARAMETER);
    TestAssert(Parameters.bPostValidationCallbackTriggered);
    TestAssert(Parameters.bShowUsageTriggered);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortStringStringStringWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-s", "string1", "-t", "string2", "-u", "string3"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString2, "string2", TRUE) == 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString3, "string3", TRUE) == 0);
    TestAssert(Parameters.dwInteger1 == 0);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == FALSE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongStringStringStringWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--string1", "string1", "--string2", "string2", "--string3", "string3"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString2, "string2", TRUE) == 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString3, "string3", TRUE) == 0);
    TestAssert(Parameters.dwInteger1 == 0);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == FALSE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortStringStringIntegerWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-s", "string1", "-t", "string2", "-i", "1"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString2, "string2", TRUE) == 0);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == FALSE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongStringStringIntegerWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--string1", "string1", "--string2", "string2", "--integer1", "1"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString2, "string2", TRUE) == 0);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == FALSE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortStringStringNoneWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-s", "string1", "-t", "string2", "-n"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString2, "string2", TRUE) == 0);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 0);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongStringStringNoneWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--string1", "string1", "--string2", "string2", "--noparameter1"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString2, "string2", TRUE) == 0);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 0);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortStringIntegerStringWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-s", "string1", "-i", "1", "-t", "string2"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString2, "string2", TRUE) == 0);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == FALSE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongStringIntegerStringWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--string1", "string1", "--integer1", "1", "--string2", "string2"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString2, "string2", TRUE) == 0);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == FALSE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortStringIntegerIntegerWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-s", "string1", "-i", "1", "-j", "2"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 2);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == FALSE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongStringIntegerIntegerWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--string1", "string1", "--integer1", "1", "--integer2", "2"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 2);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == FALSE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortStringIntegerNoneWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-s", "string1", "-i", "1", "-n"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongStringIntegerNoneWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--string1", "string1", "--integer1", "1", "--noparameter1"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortStringNoneStringWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-s", "string1", "-n", "-t", "string2"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString2, "string2", TRUE) == 0);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 0);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongStringNoneStringWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--string1", "string1", "--noparameter1", "--string2", "string2"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString2, "string2", TRUE) == 0);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 0);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortStringNoneIntegerWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-s", "string1", "-n", "-i", "1"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongStringNoneIntegerWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--string1", "string1", "--noparameter1", "--integer1", "1"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortStringNoneNoneWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-s", "string1", "-n", "-o"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 0);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == TRUE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongStringNoneNoneWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--string1", "string1", "--noparameter1", "--noparameter2"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 0);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == TRUE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortIntegerStringStringWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-i", "1", "-s", "string1", "-t", "string2"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString2, "string2", TRUE) == 0);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == FALSE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongIntegerStringStringWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--integer1", "1", "--string1", "string1", "--string2", "string2"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString2, "string2", TRUE) == 0);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == FALSE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortIntegerStringIntegerWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-i", "1", "-s", "string1", "-j", "2"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 2);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == FALSE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongIntegerStringIntegerWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--integer1", "1", "--string1", "string1", "--integer2", "2"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 2);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == FALSE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortIntegerStringNoneWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-i", "1", "-s", "string1", "-n"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongIntegerStringNoneWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--integer1", "1", "--string1", "string1", "--noparameter1"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortIntegerIntegerStringWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-i", "1", "-j", "2", "-s", "string1"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 2);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == FALSE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongIntegerIntegerStringWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--integer1", "1", "--integer2", "2", "--string1", "string1"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 2);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == FALSE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortIntegerIntegerIntegerWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-i", "1", "-j", "2", "-k", "3"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(Parameters.pszString1 == NULL);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 2);
    TestAssert(Parameters.dwInteger3 == 3);
    TestAssert(Parameters.bBoolean1 == FALSE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongIntegerIntegerIntegerWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--integer1", "1", "--integer2", "2", "--integer3", "3"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(Parameters.pszString1 == NULL);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 2);
    TestAssert(Parameters.dwInteger3 == 3);
    TestAssert(Parameters.bBoolean1 == FALSE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortIntegerIntegerNoneWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-i", "1", "-j", "2", "-n"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(Parameters.pszString1 == NULL);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 2);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongIntegerIntegerNoneWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--integer1", "1", "--integer2", "2", "--noparameter1"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(Parameters.pszString1 == NULL);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 2);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortIntegerNoneStringWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-i", "1", "-n", "-s", "string1"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongIntegerNoneStringWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--integer1", "1", "--noparameter1", "--string1", "string1"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortIntegerNoneIntegerWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-i", "1", "-n", "-j", "2"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(Parameters.pszString1 == NULL);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 2);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongIntegerNoneIntegerWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--integer1", "1", "--noparameter1", "--integer2", "2"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(Parameters.pszString1 == NULL);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 2);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortIntegerNoneNoneWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-i", "1", "-n", "-o"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(Parameters.pszString1 == NULL);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == TRUE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongIntegerNoneNoneWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--integer1", "1", "--noparameter1", "--noparameter2"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(Parameters.pszString1 == NULL);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == TRUE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortNoneStringStringWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-n", "-s", "string1", "-t", "string2"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString2, "string2", TRUE) == 0);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 0);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongNoneStringStringWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--noparameter1", "--string1", "string1", "--string2", "string2"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString2, "string2", TRUE) == 0);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 0);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortNoneStringIntegerWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-n", "-s", "string1", "-i", "1"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongNoneStringIntegerWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--noparameter1", "--string1", "string1", "--integer1", "1"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortNoneStringNoneWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-n", "-s", "string1", "-o"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 0);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == TRUE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongNoneStringNoneWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--noparameter1", "--string1", "string1", "--noparameter2"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 0);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == TRUE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortNoneIntegerStringWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-n", "-i", "1", "-s", "string1"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongNoneIntegerStringWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--noparameter1", "--integer1", "1", "--string1", "string1"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortNoneIntegerIntegerWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-n", "-i", "1", "-j", "2"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(Parameters.pszString1 == NULL);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 2);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongNoneIntegerIntegerWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--noparameter1", "--integer1", "1", "--integer2", "2"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(Parameters.pszString1 == NULL);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 2);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == FALSE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortNoneIntegerNoneWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-n", "-i", "1", "-o"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(Parameters.pszString1 == NULL);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == TRUE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongNoneIntegerNoneWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--noparameter1", "--integer1", "1", "--noparameter2"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(Parameters.pszString1 == NULL);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == TRUE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortNoneNoneStringWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-n", "-o", "-s", "string1"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 0);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == TRUE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongNoneNoneStringWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--noparameter1", "--noparameter2", "--string1", "string1"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(VmDirStringCompareA(Parameters.pszString1, "string1", TRUE) == 0);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 0);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == TRUE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortNoneNoneIntegerWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-n", "-o", "-i", "1"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(Parameters.pszString1 == NULL);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == TRUE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongNoneNoneIntegerWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--noparameter1", "--noparameter2", "--integer1", "1"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(Parameters.pszString1 == NULL);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 1);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == TRUE);
    TestAssert(Parameters.bBoolean3 == FALSE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_ShortNoneNoneNoneWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-n", "-o", "-p"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(Parameters.pszString1 == NULL);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 0);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == TRUE);
    TestAssert(Parameters.bBoolean3 == TRUE);
    FreeCLStateContent(&Parameters);
}

VOID
_Test_LongNoneNoneNoneWithValidParametersShouldSucceed(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--noparameter1", "--noparameter2", "--noparameter3"};

    dwError = VmDirParseArgumentsTestWrapper(&Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    TestAssertEquals(dwError, 0);
    TestAssert(Parameters.pszString1 == NULL);
    TestAssert(Parameters.pszString2 == NULL);
    TestAssert(Parameters.pszString3 == NULL);
    TestAssert(Parameters.dwInteger1 == 0);
    TestAssert(Parameters.dwInteger2 == 0);
    TestAssert(Parameters.dwInteger3 == 0);
    TestAssert(Parameters.bBoolean1 == TRUE);
    TestAssert(Parameters.bBoolean2 == TRUE);
    TestAssert(Parameters.bBoolean3 == TRUE);
    FreeCLStateContent(&Parameters);
}

VOID
TestVmDirParseArguments(
    PVMDIR_TEST_STATE pState
    )
{
    printf("Testing VmDirParseArguments ...");

    _Test_WithInvalidEnumValueFails(pState);
    _Test_WithNullLongFlagDoesntCrash(pState);
    _Test_WithEmptyLongFlagDoesntCrash(pState);

    _Test_StringParameterWithNoParameterShouldFail(pState);
    _Test_IntegerParameterWithNoParameterShouldFail(pState);
    _Test_IntegerParameterWithStringParameterShouldFail(pState);
    _Test_NoParameterWithParameterShouldFail(pState);
    _Test_ShortStringParameterWithStringParameterShouldSucceed(pState);
    _Test_ShortIntegerParameterWithIntegerParameterShouldSucceed(pState);
    _Test_ShortNoParameterWithNoParameterShouldSucceed(pState);
    _Test_LongStringParameterWithStringParameterShouldSucceed(pState);
    _Test_LongIntegerParameterWithIntegerParameterShouldSucceed(pState);
    _Test_LongNoParameterWithNoParameterShouldSucceed(pState);

    _Test_InvalidParametersShowUsageShouldBeCalled(pState);
    _Test_ValidParametersPostValidtionShouldBeCalled(pState);
    _Test_ValidParametersPostValidtionShouldBeCalledAndShowUsage(pState);

    _Test_LongStringStringStringWithValidParametersShouldSucceed(pState);
    _Test_ShortStringStringStringWithValidParametersShouldSucceed(pState);
    _Test_LongStringStringIntegerWithValidParametersShouldSucceed(pState);
    _Test_ShortStringStringIntegerWithValidParametersShouldSucceed(pState);
    _Test_LongStringStringNoneWithValidParametersShouldSucceed(pState);
    _Test_ShortStringStringNoneWithValidParametersShouldSucceed(pState);
    _Test_LongStringIntegerStringWithValidParametersShouldSucceed(pState);
    _Test_ShortStringIntegerStringWithValidParametersShouldSucceed(pState);
    _Test_LongStringIntegerIntegerWithValidParametersShouldSucceed(pState);
    _Test_ShortStringIntegerIntegerWithValidParametersShouldSucceed(pState);
    _Test_LongStringIntegerNoneWithValidParametersShouldSucceed(pState);
    _Test_ShortStringIntegerNoneWithValidParametersShouldSucceed(pState);
    _Test_LongStringNoneStringWithValidParametersShouldSucceed(pState);
    _Test_ShortStringNoneStringWithValidParametersShouldSucceed(pState);
    _Test_LongStringNoneIntegerWithValidParametersShouldSucceed(pState);
    _Test_ShortStringNoneIntegerWithValidParametersShouldSucceed(pState);
    _Test_LongStringNoneNoneWithValidParametersShouldSucceed(pState);
    _Test_ShortStringNoneNoneWithValidParametersShouldSucceed(pState);
    _Test_LongIntegerStringStringWithValidParametersShouldSucceed(pState);
    _Test_ShortIntegerStringStringWithValidParametersShouldSucceed(pState);
    _Test_LongIntegerStringIntegerWithValidParametersShouldSucceed(pState);
    _Test_ShortIntegerStringIntegerWithValidParametersShouldSucceed(pState);
    _Test_LongIntegerStringNoneWithValidParametersShouldSucceed(pState);
    _Test_ShortIntegerStringNoneWithValidParametersShouldSucceed(pState);
    _Test_LongIntegerIntegerStringWithValidParametersShouldSucceed(pState);
    _Test_ShortIntegerIntegerStringWithValidParametersShouldSucceed(pState);
    _Test_LongIntegerIntegerIntegerWithValidParametersShouldSucceed(pState);
    _Test_ShortIntegerIntegerIntegerWithValidParametersShouldSucceed(pState);
    _Test_LongIntegerIntegerNoneWithValidParametersShouldSucceed(pState);
    _Test_ShortIntegerIntegerNoneWithValidParametersShouldSucceed(pState);
    _Test_LongIntegerNoneStringWithValidParametersShouldSucceed(pState);
    _Test_ShortIntegerNoneStringWithValidParametersShouldSucceed(pState);
    _Test_LongIntegerNoneIntegerWithValidParametersShouldSucceed(pState);
    _Test_ShortIntegerNoneIntegerWithValidParametersShouldSucceed(pState);
    _Test_LongIntegerNoneNoneWithValidParametersShouldSucceed(pState);
    _Test_ShortIntegerNoneNoneWithValidParametersShouldSucceed(pState);
    _Test_LongNoneStringStringWithValidParametersShouldSucceed(pState);
    _Test_ShortNoneStringStringWithValidParametersShouldSucceed(pState);
    _Test_LongNoneStringIntegerWithValidParametersShouldSucceed(pState);
    _Test_ShortNoneStringIntegerWithValidParametersShouldSucceed(pState);
    _Test_LongNoneStringNoneWithValidParametersShouldSucceed(pState);
    _Test_ShortNoneStringNoneWithValidParametersShouldSucceed(pState);
    _Test_LongNoneIntegerStringWithValidParametersShouldSucceed(pState);
    _Test_ShortNoneIntegerStringWithValidParametersShouldSucceed(pState);
    _Test_LongNoneIntegerIntegerWithValidParametersShouldSucceed(pState);
    _Test_ShortNoneIntegerIntegerWithValidParametersShouldSucceed(pState);
    _Test_LongNoneIntegerNoneWithValidParametersShouldSucceed(pState);
    _Test_ShortNoneIntegerNoneWithValidParametersShouldSucceed(pState);
    _Test_LongNoneNoneStringWithValidParametersShouldSucceed(pState);
    _Test_ShortNoneNoneStringWithValidParametersShouldSucceed(pState);
    _Test_LongNoneNoneIntegerWithValidParametersShouldSucceed(pState);
    _Test_ShortNoneNoneIntegerWithValidParametersShouldSucceed(pState);
    _Test_LongNoneNoneNoneWithValidParametersShouldSucceed(pState);
    _Test_ShortNoneNoneNoneWithValidParametersShouldSucceed(pState);

    printf(" PASSED\n");
}
