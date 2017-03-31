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



/*
 * Module Name: vdcpass
 *
 * Filename: main.c
 *
 * Abstract:
 *
 * vdcpass main module entry point
 *
 */

#include "includes.h"

static
int
VmDirMain(int argc, char* argv[])
{
    DWORD   dwError = 0;
    CHAR    pszPasswordBuf[VMDIR_MAX_PWD_LEN + 1] = {0};
    COMMAND_LINE_STATE State = { 0 };

    VMDIR_COMMAND_LINE_OPTION Options[] =
    {
            {'h', "host", CL_STRING_PARAMETER, &State.pszHostName},
            {'u', "username", CL_STRING_PARAMETER, &State.pszLoginUserUPN},
            {'U', "newuser", CL_STRING_PARAMETER, &State.pszUserUPN},
            {'w', "password", CL_STRING_PARAMETER, &State.pszLoginPassword},
            {'W', "newpass", CL_STRING_PARAMETER, &State.pszNewPassword},
            {0, 0, 0, 0}
    };

    VMDIR_PARSE_ARG_CALLBACKS Callbacks =
    {
            PostValidationRoutine,
            ShowUsage,
            &State
    };

    dwError = VmDirParseArguments(
                Options,
                &Callbacks,
                argc,
                argv);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (State.pszLoginPassword == NULL)
    {
        // read password from stdin
        VmDirReadString(
            "password: ",
            pszPasswordBuf,
            sizeof(pszPasswordBuf),
            TRUE);
        State.pszLoginPassword = pszPasswordBuf;
    }

    if (State.pszUserUPN) //set password
    {
        dwError = VmDirSetPassword(
                            State.pszHostName,
                            State.pszLoginUserUPN,
                            State.pszLoginPassword,
                            State.pszUserUPN,
                            State.pszNewPassword);
        BAIL_ON_VMDIR_ERROR(dwError);

        printf("password was set successfully.\n");
    }
    else //change password
    {
        dwError = VmDirChangePassword(
                            State.pszHostName,
                            State.pszLoginUserUPN,
                            State.pszLoginPassword,
                            State.pszNewPassword);
        BAIL_ON_VMDIR_ERROR(dwError);

        printf("password was changed successfully.\n");
    }

cleanup:

    memset(pszPasswordBuf, 0, sizeof(pszPasswordBuf));
    FreeCLStateContent(&State);
    return dwError;

error:
    printf("Vdcpass failed with error code %d.\n", dwError);

    goto cleanup;
}

#ifdef _WIN32

int wmain(int argc, wchar_t* argv[])
{
    DWORD dwError = 0;
    PSTR* ppszArgs = NULL;
    int   iArg = 0;

    dwError = VmDirAllocateMemory(sizeof(PSTR) * argc, (PVOID*)&ppszArgs);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (; iArg < argc; iArg++)
    {
        dwError = VmDirAllocateStringAFromW(argv[iArg], &ppszArgs[iArg]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirMain(argc, ppszArgs);
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    if (ppszArgs)
    {
        for (iArg = 0; iArg < argc; iArg++)
        {
            VMDIR_SAFE_FREE_MEMORY(ppszArgs[iArg]);
        }
        VmDirFreeMemory(ppszArgs);
    }

    return dwError;
}
#else

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "");

    return VmDirMain(argc, argv);
}

#endif
