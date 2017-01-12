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
    PSTR pszString;
    DWORD dwInteger;
    BOOLEAN bTriggered;
    BOOLEAN bShowUsageTriggered;
    BOOLEAN bPostValidationCallbackTriggered;
    BOOLEAN bReturnFailure;
    DWORD dwStringCallbackCount;
    DWORD dwIntegerCallbackCount;
    DWORD dwNoneCallbackCount;
} COMMAND_LINE_PARAMETER_STATE, *PCOMMAND_LINE_PARAMETER_STATE;


DWORD
PostValidateParameters(
    PVOID pvParameter
    )
{
    PCOMMAND_LINE_PARAMETER_STATE State = (PCOMMAND_LINE_PARAMETER_STATE)pvParameter;

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
    PCOMMAND_LINE_PARAMETER_STATE State = (PCOMMAND_LINE_PARAMETER_STATE)pvParameter;

    State->bShowUsageTriggered = TRUE;
}

DWORD
HandleStringParameter(
    PVOID pContext,
    PCSTR pValue
    )
{
    PCOMMAND_LINE_PARAMETER_STATE ParameterState = (PCOMMAND_LINE_PARAMETER_STATE)pContext;

    ParameterState->pszString = (PSTR)pValue;
    ParameterState->dwStringCallbackCount++;
    return 0;
}

DWORD
HandleIntegerParameter(
    PVOID pContext,
    DWORD dwValue
    )
{
    PCOMMAND_LINE_PARAMETER_STATE ParameterState = (PCOMMAND_LINE_PARAMETER_STATE)pContext;

    ParameterState->dwInteger = dwValue;
    ParameterState->dwIntegerCallbackCount++;
    return 0;
}

DWORD
HandleNoParameter(
    PVOID pContext
    )
{
    PCOMMAND_LINE_PARAMETER_STATE ParameterState = (PCOMMAND_LINE_PARAMETER_STATE)pContext;

    ParameterState->bTriggered = TRUE;
    ParameterState->dwNoneCallbackCount++;

    return 0;
}

VMDIR_COMMAND_LINE_OPTIONS CommandLineOptions =
{
    ShowUsage,
    PostValidateParameters,
    {
        {'s', "string1", CL_STRING_PARAMETER, HandleStringParameter},
        {'t', "string2", CL_STRING_PARAMETER, HandleStringParameter},
        {'u', "string3", CL_STRING_PARAMETER, HandleStringParameter},
        {'i', "integer1", CL_INTEGER_PARAMETER, HandleIntegerParameter},
        {'j', "integer2", CL_INTEGER_PARAMETER, HandleIntegerParameter},
        {'k', "integer3", CL_INTEGER_PARAMETER, HandleIntegerParameter},
        {'n', "noparameter1", CL_NO_PARAMETER, HandleNoParameter},
        {'o', "noparameter2", CL_NO_PARAMETER, HandleNoParameter},
        {'p', "noparameter3", CL_NO_PARAMETER, HandleNoParameter},
        {0, 0, 0, 0}
    }
};

VOID
_Test_VmDirParseArgumentsWithInvalidEnumValueFails(
    VOID)
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app", "-s"};
    static VMDIR_COMMAND_LINE_OPTIONS Options =
    {
        ShowUsage,
        PostValidateParameters,
        {
            {'s', NULL, 0xFFFFFFFF, HandleStringParameter}
        }
    };

    dwError = VmDirParseArguments(&Options, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == VMDIR_ERROR_INVALID_PARAMETER);
}

VOID
_Test_VmDirParseArgumentsWithNullLongFlagDoesntCrash(
    VOID)
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app", "--invalid"};
    static VMDIR_COMMAND_LINE_OPTIONS Options =
    {
        ShowUsage,
        PostValidateParameters,
        {
            {'s', NULL, CL_STRING_PARAMETER, HandleStringParameter},
        }
    };

    dwError = VmDirParseArguments(&Options, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == VMDIR_ERROR_INVALID_PARAMETER);
}

VOID
_Test_VmDirParseArgumentsWithEmptyLongFlagDoesntCrash(
    VOID)
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app", "--invalid"};
    static VMDIR_COMMAND_LINE_OPTIONS Options =
    {
        ShowUsage,
        PostValidateParameters,
        {
            {'s', "", CL_STRING_PARAMETER, HandleStringParameter},
        }
    };

    dwError = VmDirParseArguments(&Options, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == VMDIR_ERROR_INVALID_PARAMETER);
}

VOID
_Test_VmDirParseArguments_StringParameterWithNoParameterShouldFail(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app", "-s"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == VMDIR_ERROR_INVALID_PARAMETER);
}

VOID
_Test_VmDirParseArguments_IntegerParameterWithNoParameterShouldFail(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app", "-i"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, 2, argv);
    ASSERT(dwError == VMDIR_ERROR_INVALID_PARAMETER);
}

VOID
_Test_VmDirParseArguments_IntegerParameterWithStringParameterShouldFail(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app", "-i", "hello"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == VMDIR_ERROR_INVALID_PARAMETER);
}

VOID
_Test_VmDirParseArguments_NoParameterWithParameterShouldFail(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app", "-n", "extraparameter"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == VMDIR_ERROR_INVALID_PARAMETER);
}

VOID
_Test_VmDirParseArguments_ShortStringParameterWithStringParameterShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app", "-s", "hello"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(strcmp(Parameters.pszString, "hello") == 0);
}

VOID
_Test_VmDirParseArguments_ShortIntegerParameterWithIntegerParameterShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app", "-i", "42"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwInteger = 42);
}

VOID
_Test_VmDirParseArguments_ShortNoParameterWithNoParameterShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app", "-n"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.bTriggered);
}

VOID
_Test_VmDirParseArguments_LongStringParameterWithStringParameterShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app", "--string1", "hello, world!"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(strcmp(Parameters.pszString, "hello, world!") == 0);
}

VOID
_Test_VmDirParseArguments_LongIntegerParameterWithIntegerParameterShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app", "--integer1", "-37"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwInteger == (DWORD)-37);
}

VOID
_Test_VmDirParseArguments_LongNoParameterWithNoParameterShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app", "--noparameter1"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.bTriggered);
}

VOID
_Test_VmDirParseArguments_InvalidParametersShowUsageShouldBeCalled(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app", "--invalid-parameter"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == VMDIR_ERROR_INVALID_PARAMETER);
    ASSERT(Parameters.bShowUsageTriggered);
}

VOID
_Test_VmDirParseArguments_ValidParametersPostValidtionShouldBeCalled(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app", "--noparameter1"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.bPostValidationCallbackTriggered);
}

VOID
_Test_VmDirParseArguments_ValidParametersPostValidtionShouldBeCalledAndShowUsage(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app", "--noparameter1"};

    Parameters.bReturnFailure = TRUE;
    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == VMDIR_ERROR_INVALID_PARAMETER);
    ASSERT(Parameters.bPostValidationCallbackTriggered);
    ASSERT(Parameters.bShowUsageTriggered);
}

VOID
_Test_VmDirParseArgumentsShortStringStringStringWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-s", "string1", "-t", "string2", "-u", "string3"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 3);
    ASSERT(Parameters.dwIntegerCallbackCount == 0);
    ASSERT(Parameters.dwNoneCallbackCount == 0);

}

VOID
_Test_VmDirParseArgumentsLongStringStringStringWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--string1", "string1", "--string2", "string2", "--string3", "string3"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 3);
    ASSERT(Parameters.dwIntegerCallbackCount == 0);
    ASSERT(Parameters.dwNoneCallbackCount == 0);

}

VOID
_Test_VmDirParseArgumentsShortStringStringIntegerWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-s", "string1", "-t", "string2", "-i", "1"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 2);
    ASSERT(Parameters.dwIntegerCallbackCount == 1);
    ASSERT(Parameters.dwNoneCallbackCount == 0);

}

VOID
_Test_VmDirParseArgumentsLongStringStringIntegerWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--string1", "string1", "--string2", "string2", "--integer1", "1"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 2);
    ASSERT(Parameters.dwIntegerCallbackCount == 1);
    ASSERT(Parameters.dwNoneCallbackCount == 0);

}

VOID
_Test_VmDirParseArgumentsShortStringStringNoneWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-s", "string1", "-t", "string2", "-n"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 2);
    ASSERT(Parameters.dwIntegerCallbackCount == 0);
    ASSERT(Parameters.dwNoneCallbackCount == 1);

}

VOID
_Test_VmDirParseArgumentsLongStringStringNoneWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--string1", "string1", "--string2", "string2", "--noparameter1"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 2);
    ASSERT(Parameters.dwIntegerCallbackCount == 0);
    ASSERT(Parameters.dwNoneCallbackCount == 1);

}

VOID
_Test_VmDirParseArgumentsShortStringIntegerStringWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-s", "string1", "-i", "1", "-t", "string2"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 2);
    ASSERT(Parameters.dwIntegerCallbackCount == 1);
    ASSERT(Parameters.dwNoneCallbackCount == 0);

}

VOID
_Test_VmDirParseArgumentsLongStringIntegerStringWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--string1", "string1", "--integer1", "1", "--string2", "string2"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 2);
    ASSERT(Parameters.dwIntegerCallbackCount == 1);
    ASSERT(Parameters.dwNoneCallbackCount == 0);

}

VOID
_Test_VmDirParseArgumentsShortStringIntegerIntegerWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-s", "string1", "-i", "1", "-j", "2"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 1);
    ASSERT(Parameters.dwIntegerCallbackCount == 2);
    ASSERT(Parameters.dwNoneCallbackCount == 0);

}

VOID
_Test_VmDirParseArgumentsLongStringIntegerIntegerWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--string1", "string1", "--integer1", "1", "--integer2", "2"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 1);
    ASSERT(Parameters.dwIntegerCallbackCount == 2);
    ASSERT(Parameters.dwNoneCallbackCount == 0);

}

VOID
_Test_VmDirParseArgumentsShortStringIntegerNoneWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-s", "string1", "-i", "1", "-n"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 1);
    ASSERT(Parameters.dwIntegerCallbackCount == 1);
    ASSERT(Parameters.dwNoneCallbackCount == 1);

}

VOID
_Test_VmDirParseArgumentsLongStringIntegerNoneWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--string1", "string1", "--integer1", "1", "--noparameter1"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 1);
    ASSERT(Parameters.dwIntegerCallbackCount == 1);
    ASSERT(Parameters.dwNoneCallbackCount == 1);

}

VOID
_Test_VmDirParseArgumentsShortStringNoneStringWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-s", "string1", "-n", "-t", "string2"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 2);
    ASSERT(Parameters.dwIntegerCallbackCount == 0);
    ASSERT(Parameters.dwNoneCallbackCount == 1);

}

VOID
_Test_VmDirParseArgumentsLongStringNoneStringWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--string1", "string1", "--noparameter1", "--string2", "string2"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 2);
    ASSERT(Parameters.dwIntegerCallbackCount == 0);
    ASSERT(Parameters.dwNoneCallbackCount == 1);

}

VOID
_Test_VmDirParseArgumentsShortStringNoneIntegerWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-s", "string1", "-n", "-i", "1"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 1);
    ASSERT(Parameters.dwIntegerCallbackCount == 1);
    ASSERT(Parameters.dwNoneCallbackCount == 1);

}

VOID
_Test_VmDirParseArgumentsLongStringNoneIntegerWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--string1", "string1", "--noparameter1", "--integer1", "1"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 1);
    ASSERT(Parameters.dwIntegerCallbackCount == 1);
    ASSERT(Parameters.dwNoneCallbackCount == 1);

}

VOID
_Test_VmDirParseArgumentsShortStringNoneNoneWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-s", "string1", "-n", "-o"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 1);
    ASSERT(Parameters.dwIntegerCallbackCount == 0);
    ASSERT(Parameters.dwNoneCallbackCount == 2);

}

VOID
_Test_VmDirParseArgumentsLongStringNoneNoneWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--string1", "string1", "--noparameter1", "--noparameter2"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 1);
    ASSERT(Parameters.dwIntegerCallbackCount == 0);
    ASSERT(Parameters.dwNoneCallbackCount == 2);

}

VOID
_Test_VmDirParseArgumentsShortIntegerStringStringWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-i", "1", "-s", "string1", "-t", "string2"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 2);
    ASSERT(Parameters.dwIntegerCallbackCount == 1);
    ASSERT(Parameters.dwNoneCallbackCount == 0);

}

VOID
_Test_VmDirParseArgumentsLongIntegerStringStringWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--integer1", "1", "--string1", "string1", "--string2", "string2"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 2);
    ASSERT(Parameters.dwIntegerCallbackCount == 1);
    ASSERT(Parameters.dwNoneCallbackCount == 0);

}

VOID
_Test_VmDirParseArgumentsShortIntegerStringIntegerWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-i", "1", "-s", "string1", "-j", "2"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 1);
    ASSERT(Parameters.dwIntegerCallbackCount == 2);
    ASSERT(Parameters.dwNoneCallbackCount == 0);

}

VOID
_Test_VmDirParseArgumentsLongIntegerStringIntegerWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--integer1", "1", "--string1", "string1", "--integer2", "2"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 1);
    ASSERT(Parameters.dwIntegerCallbackCount == 2);
    ASSERT(Parameters.dwNoneCallbackCount == 0);

}

VOID
_Test_VmDirParseArgumentsShortIntegerStringNoneWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-i", "1", "-s", "string1", "-n"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 1);
    ASSERT(Parameters.dwIntegerCallbackCount == 1);
    ASSERT(Parameters.dwNoneCallbackCount == 1);

}

VOID
_Test_VmDirParseArgumentsLongIntegerStringNoneWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--integer1", "1", "--string1", "string1", "--noparameter1"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 1);
    ASSERT(Parameters.dwIntegerCallbackCount == 1);
    ASSERT(Parameters.dwNoneCallbackCount == 1);

}

VOID
_Test_VmDirParseArgumentsShortIntegerIntegerStringWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-i", "1", "-j", "2", "-s", "string1"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 1);
    ASSERT(Parameters.dwIntegerCallbackCount == 2);
    ASSERT(Parameters.dwNoneCallbackCount == 0);

}

VOID
_Test_VmDirParseArgumentsLongIntegerIntegerStringWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--integer1", "1", "--integer2", "2", "--string1", "string1"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 1);
    ASSERT(Parameters.dwIntegerCallbackCount == 2);
    ASSERT(Parameters.dwNoneCallbackCount == 0);

}

VOID
_Test_VmDirParseArgumentsShortIntegerIntegerIntegerWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-i", "1", "-j", "2", "-k", "3"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 0);
    ASSERT(Parameters.dwIntegerCallbackCount == 3);
    ASSERT(Parameters.dwNoneCallbackCount == 0);

}

VOID
_Test_VmDirParseArgumentsLongIntegerIntegerIntegerWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--integer1", "1", "--integer2", "2", "--integer3", "3"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 0);
    ASSERT(Parameters.dwIntegerCallbackCount == 3);
    ASSERT(Parameters.dwNoneCallbackCount == 0);

}

VOID
_Test_VmDirParseArgumentsShortIntegerIntegerNoneWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-i", "1", "-j", "2", "-n"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 0);
    ASSERT(Parameters.dwIntegerCallbackCount == 2);
    ASSERT(Parameters.dwNoneCallbackCount == 1);

}

VOID
_Test_VmDirParseArgumentsLongIntegerIntegerNoneWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--integer1", "1", "--integer2", "2", "--noparameter1"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 0);
    ASSERT(Parameters.dwIntegerCallbackCount == 2);
    ASSERT(Parameters.dwNoneCallbackCount == 1);

}

VOID
_Test_VmDirParseArgumentsShortIntegerNoneStringWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-i", "1", "-n", "-s", "string1"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 1);
    ASSERT(Parameters.dwIntegerCallbackCount == 1);
    ASSERT(Parameters.dwNoneCallbackCount == 1);

}

VOID
_Test_VmDirParseArgumentsLongIntegerNoneStringWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--integer1", "1", "--noparameter1", "--string1", "string1"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 1);
    ASSERT(Parameters.dwIntegerCallbackCount == 1);
    ASSERT(Parameters.dwNoneCallbackCount == 1);

}

VOID
_Test_VmDirParseArgumentsShortIntegerNoneIntegerWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-i", "1", "-n", "-j", "2"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 0);
    ASSERT(Parameters.dwIntegerCallbackCount == 2);
    ASSERT(Parameters.dwNoneCallbackCount == 1);

}

VOID
_Test_VmDirParseArgumentsLongIntegerNoneIntegerWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--integer1", "1", "--noparameter1", "--integer2", "2"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 0);
    ASSERT(Parameters.dwIntegerCallbackCount == 2);
    ASSERT(Parameters.dwNoneCallbackCount == 1);

}

VOID
_Test_VmDirParseArgumentsShortIntegerNoneNoneWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-i", "1", "-n", "-o"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 0);
    ASSERT(Parameters.dwIntegerCallbackCount == 1);
    ASSERT(Parameters.dwNoneCallbackCount == 2);

}

VOID
_Test_VmDirParseArgumentsLongIntegerNoneNoneWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--integer1", "1", "--noparameter1", "--noparameter2"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 0);
    ASSERT(Parameters.dwIntegerCallbackCount == 1);
    ASSERT(Parameters.dwNoneCallbackCount == 2);

}

VOID
_Test_VmDirParseArgumentsShortNoneStringStringWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-n", "-s", "string1", "-t", "string2"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 2);
    ASSERT(Parameters.dwIntegerCallbackCount == 0);
    ASSERT(Parameters.dwNoneCallbackCount == 1);

}

VOID
_Test_VmDirParseArgumentsLongNoneStringStringWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--noparameter1", "--string1", "string1", "--string2", "string2"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 2);
    ASSERT(Parameters.dwIntegerCallbackCount == 0);
    ASSERT(Parameters.dwNoneCallbackCount == 1);

}

VOID
_Test_VmDirParseArgumentsShortNoneStringIntegerWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-n", "-s", "string1", "-i", "1"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 1);
    ASSERT(Parameters.dwIntegerCallbackCount == 1);
    ASSERT(Parameters.dwNoneCallbackCount == 1);

}

VOID
_Test_VmDirParseArgumentsLongNoneStringIntegerWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--noparameter1", "--string1", "string1", "--integer1", "1"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 1);
    ASSERT(Parameters.dwIntegerCallbackCount == 1);
    ASSERT(Parameters.dwNoneCallbackCount == 1);

}

VOID
_Test_VmDirParseArgumentsShortNoneStringNoneWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-n", "-s", "string1", "-o"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 1);
    ASSERT(Parameters.dwIntegerCallbackCount == 0);
    ASSERT(Parameters.dwNoneCallbackCount == 2);

}

VOID
_Test_VmDirParseArgumentsLongNoneStringNoneWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--noparameter1", "--string1", "string1", "--noparameter2"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 1);
    ASSERT(Parameters.dwIntegerCallbackCount == 0);
    ASSERT(Parameters.dwNoneCallbackCount == 2);

}

VOID
_Test_VmDirParseArgumentsShortNoneIntegerStringWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-n", "-i", "1", "-s", "string1"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 1);
    ASSERT(Parameters.dwIntegerCallbackCount == 1);
    ASSERT(Parameters.dwNoneCallbackCount == 1);

}

VOID
_Test_VmDirParseArgumentsLongNoneIntegerStringWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--noparameter1", "--integer1", "1", "--string1", "string1"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 1);
    ASSERT(Parameters.dwIntegerCallbackCount == 1);
    ASSERT(Parameters.dwNoneCallbackCount == 1);

}

VOID
_Test_VmDirParseArgumentsShortNoneIntegerIntegerWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-n", "-i", "1", "-j", "2"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 0);
    ASSERT(Parameters.dwIntegerCallbackCount == 2);
    ASSERT(Parameters.dwNoneCallbackCount == 1);

}

VOID
_Test_VmDirParseArgumentsLongNoneIntegerIntegerWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--noparameter1", "--integer1", "1", "--integer2", "2"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 0);
    ASSERT(Parameters.dwIntegerCallbackCount == 2);
    ASSERT(Parameters.dwNoneCallbackCount == 1);

}

VOID
_Test_VmDirParseArgumentsShortNoneIntegerNoneWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-n", "-i", "1", "-o"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 0);
    ASSERT(Parameters.dwIntegerCallbackCount == 1);
    ASSERT(Parameters.dwNoneCallbackCount == 2);

}

VOID
_Test_VmDirParseArgumentsLongNoneIntegerNoneWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--noparameter1", "--integer1", "1", "--noparameter2"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 0);
    ASSERT(Parameters.dwIntegerCallbackCount == 1);
    ASSERT(Parameters.dwNoneCallbackCount == 2);

}

VOID
_Test_VmDirParseArgumentsShortNoneNoneStringWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-n", "-o", "-s", "string1"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 1);
    ASSERT(Parameters.dwIntegerCallbackCount == 0);
    ASSERT(Parameters.dwNoneCallbackCount == 2);

}

VOID
_Test_VmDirParseArgumentsLongNoneNoneStringWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--noparameter1", "--noparameter2", "--string1", "string1"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 1);
    ASSERT(Parameters.dwIntegerCallbackCount == 0);
    ASSERT(Parameters.dwNoneCallbackCount == 2);

}

VOID
_Test_VmDirParseArgumentsShortNoneNoneIntegerWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-n", "-o", "-i", "1"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 0);
    ASSERT(Parameters.dwIntegerCallbackCount == 1);
    ASSERT(Parameters.dwNoneCallbackCount == 2);

}

VOID
_Test_VmDirParseArgumentsLongNoneNoneIntegerWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--noparameter1", "--noparameter2", "--integer1", "1"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 0);
    ASSERT(Parameters.dwIntegerCallbackCount == 1);
    ASSERT(Parameters.dwNoneCallbackCount == 2);

}

VOID
_Test_VmDirParseArgumentsShortNoneNoneNoneWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "-n", "-o", "-p"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 0);
    ASSERT(Parameters.dwIntegerCallbackCount == 0);
    ASSERT(Parameters.dwNoneCallbackCount == 3);

}

VOID
_Test_VmDirParseArgumentsLongNoneNoneNoneWithValidParametersShouldSucceed(
    VOID
    )
{
    DWORD dwError = 0;
    COMMAND_LINE_PARAMETER_STATE Parameters = { 0 };
    char *argv[] = {"app_name", "--noparameter1", "--noparameter2", "--noparameter3"};

    dwError = VmDirParseArguments(&CommandLineOptions, &Parameters, VMDIR_ARRAY_SIZE(argv), argv);
    ASSERT(dwError == 0);
    ASSERT(Parameters.dwStringCallbackCount == 0);
    ASSERT(Parameters.dwIntegerCallbackCount == 0);
    ASSERT(Parameters.dwNoneCallbackCount == 3);

}

VOID
TestVmDirParseArguments(
    VOID
    )
{
    printf("Testing VmDirParseArguments ...\n");

    _Test_VmDirParseArgumentsWithInvalidEnumValueFails();
    _Test_VmDirParseArgumentsWithNullLongFlagDoesntCrash();
    _Test_VmDirParseArgumentsWithEmptyLongFlagDoesntCrash();

    _Test_VmDirParseArguments_StringParameterWithNoParameterShouldFail();
    _Test_VmDirParseArguments_IntegerParameterWithNoParameterShouldFail();
    _Test_VmDirParseArguments_IntegerParameterWithStringParameterShouldFail();
    _Test_VmDirParseArguments_NoParameterWithParameterShouldFail();
    _Test_VmDirParseArguments_ShortStringParameterWithStringParameterShouldSucceed();
    _Test_VmDirParseArguments_ShortIntegerParameterWithIntegerParameterShouldSucceed();
    _Test_VmDirParseArguments_ShortNoParameterWithNoParameterShouldSucceed();
    _Test_VmDirParseArguments_LongStringParameterWithStringParameterShouldSucceed();
    _Test_VmDirParseArguments_LongIntegerParameterWithIntegerParameterShouldSucceed();
    _Test_VmDirParseArguments_LongNoParameterWithNoParameterShouldSucceed();

    _Test_VmDirParseArguments_InvalidParametersShowUsageShouldBeCalled();
    _Test_VmDirParseArguments_ValidParametersPostValidtionShouldBeCalled();
    _Test_VmDirParseArguments_ValidParametersPostValidtionShouldBeCalledAndShowUsage();

    _Test_VmDirParseArgumentsLongStringStringStringWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsShortStringStringStringWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsLongStringStringIntegerWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsShortStringStringIntegerWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsLongStringStringNoneWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsShortStringStringNoneWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsLongStringIntegerStringWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsShortStringIntegerStringWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsLongStringIntegerIntegerWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsShortStringIntegerIntegerWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsLongStringIntegerNoneWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsShortStringIntegerNoneWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsLongStringNoneStringWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsShortStringNoneStringWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsLongStringNoneIntegerWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsShortStringNoneIntegerWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsLongStringNoneNoneWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsShortStringNoneNoneWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsLongIntegerStringStringWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsShortIntegerStringStringWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsLongIntegerStringIntegerWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsShortIntegerStringIntegerWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsLongIntegerStringNoneWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsShortIntegerStringNoneWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsLongIntegerIntegerStringWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsShortIntegerIntegerStringWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsLongIntegerIntegerIntegerWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsShortIntegerIntegerIntegerWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsLongIntegerIntegerNoneWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsShortIntegerIntegerNoneWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsLongIntegerNoneStringWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsShortIntegerNoneStringWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsLongIntegerNoneIntegerWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsShortIntegerNoneIntegerWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsLongIntegerNoneNoneWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsShortIntegerNoneNoneWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsLongNoneStringStringWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsShortNoneStringStringWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsLongNoneStringIntegerWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsShortNoneStringIntegerWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsLongNoneStringNoneWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsShortNoneStringNoneWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsLongNoneIntegerStringWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsShortNoneIntegerStringWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsLongNoneIntegerIntegerWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsShortNoneIntegerIntegerWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsLongNoneIntegerNoneWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsShortNoneIntegerNoneWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsLongNoneNoneStringWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsShortNoneNoneStringWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsLongNoneNoneIntegerWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsShortNoneNoneIntegerWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsLongNoneNoneNoneWithValidParametersShouldSucceed();
    _Test_VmDirParseArgumentsShortNoneNoneNoneWithValidParametersShouldSucceed();
}
