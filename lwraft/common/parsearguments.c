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

BOOLEAN
_VmDirIsCmdLineOption(
    PSTR pszArg
    )
{
    return (!IsNullOrEmptyString(pszArg) && pszArg[0] == '-');
}

BOOLEAN _VmDirMatchOption(
    PVMDIR_COMMAND_LINE_OPTION Option,
    PSTR pszArgument)
{
    if (strlen(pszArgument) <= 1)
    {
        return FALSE;
    }

    //
    // pszArgument should be of the form "-X" or "--longoption".
    //
    if (pszArgument[1] == '-')
    {
        if (Option->LongSwitch == NULL)
        {
            return FALSE;
        }
        else
        {
            return (VmDirStringCompareA(Option->LongSwitch, &pszArgument[2], TRUE) == 0);
        }
    }
    else
    {
        return Option->Switch == pszArgument[1];
    }
}

PVMDIR_COMMAND_LINE_OPTION
_VmDirFindOption(
    PVMDIR_COMMAND_LINE_OPTIONS Options,
    PSTR pszArgument
    )
{
    PVMDIR_COMMAND_LINE_OPTION Option = Options->Options;

    while (Option->Switch != 0)
    {
        if (_VmDirMatchOption(Option, pszArgument))
        {
            return Option;
        }

        Option += 1;
    }

    return NULL;
}

DWORD _VmDirCallArgumentCallback(
    PVMDIR_COMMAND_LINE_OPTION Option,
    PSTR Parameter,
    PVOID pvContext)
{
    DWORD dwError = 0;

    switch (Option->Type)
    {
        case CL_NO_PARAMETER:
        {
            COMMAND_PARAMETER_CALLBACK_NO_PARAM Callback = (COMMAND_PARAMETER_CALLBACK_NO_PARAM)Option->Callback;
            dwError = (*Callback)(pvContext);
            break;
        }

        case CL_STRING_PARAMETER:
        {
            COMMAND_PARAMETER_CALLBACK_STRING_PARAM Callback = (COMMAND_PARAMETER_CALLBACK_STRING_PARAM)Option->Callback;
            dwError = (*Callback)(pvContext, Parameter);
            break;
        }

        case CL_INTEGER_PARAMETER:
        {
            COMMAND_PARAMETER_CALLBACK_INTEGER_PARAM Callback = (COMMAND_PARAMETER_CALLBACK_INTEGER_PARAM)Option->Callback;
            char *pszEndPointer;
            int iValue = 0;

            iValue = strtol(Parameter, &pszEndPointer, 10);
            if (*pszEndPointer != '\0')
            {
                dwError = VMDIR_ERROR_INVALID_PARAMETER;
            }
            else
            {
                dwError = (*Callback)(pvContext, iValue);
            }

            break;
       }

       default:
       dwError = VMDIR_ERROR_INVALID_PARAMETER;
       break;
    }

    return dwError;
}

DWORD
VmDirParseArguments(
    PVMDIR_COMMAND_LINE_OPTIONS Options,
    PVOID pvContext,
    int argc,
    PSTR *argv
    )
{
    int i = 1;
    PSTR Parameter = NULL;
    DWORD dwError = 0;

    while (i < argc)
    {
        if (_VmDirIsCmdLineOption(argv[i]))
        {
            PVMDIR_COMMAND_LINE_OPTION Option = _VmDirFindOption(Options, argv[i]);

            if (Option == NULL)
            {
                dwError = VMDIR_ERROR_INVALID_PARAMETER;
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            //
            // If this option is supposed to have a parameter make sure that we got
            // enough data on the command line.
            //
            if (Option->Type != CL_NO_PARAMETER)
            {
                if (i == (argc - 1))
                {
                    dwError = VMDIR_ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMDIR_ERROR(dwError);
                }

                Parameter = argv[i + 1];
                i += 2;
            }
            else
            {
                Parameter = NULL;
                ++i;
            }

            dwError = _VmDirCallArgumentCallback(Option, Parameter, pvContext);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            //
            // We don't support naked parameters at the moment.
            //
            dwError = VMDIR_ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    if (Options->ValidationRoutine != NULL)
    {
        dwError = (*Options->ValidationRoutine)(pvContext);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    if (Options->ShowUsage != NULL)
    {
        (*Options->ShowUsage)(pvContext);
    }

    goto cleanup;
}
