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
 * Module Name: vdcsrp
 *
 * Filename: main.c
 *
 * Abstract:
 *
 * vdcsrp module entry point
 *
 */

#include "includes.h"

static
int
VmDirMain(int argc, char* argv[])
{
    DWORD   dwError = 0;

    PSTR    pszUPN = NULL;
    PSTR    pszSecret = NULL;
    PSTR    pszSecret_file = NULL;
    PSTR    pszErrorMessage = NULL;
    CHAR    pszSecretBuf[VMDIR_MAX_PWD_LEN+1];

#ifndef _WIN32
    setlocale(LC_ALL, "");
#endif

    dwError = VmDirParseArgs(
                        argc, argv,
                        &pszUPN,
                        &pszSecret,
                        &pszSecret_file);

    if (dwError != ERROR_SUCCESS)
    {
        ShowUsage();
        goto cleanup;
    }

    memset(pszSecretBuf, 0, sizeof(pszSecretBuf));

    if (pszSecret == NULL && pszSecret_file != NULL)
    {
       dwError = VmDirReadStringFromFile(pszSecret_file, pszSecretBuf, sizeof(pszSecretBuf));
       BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (pszSecret != NULL && pszSecret_file == NULL)
    {
       dwError = VmDirStringCpyA(pszSecretBuf, VMDIR_MAX_PWD_LEN, pszSecret);
       BAIL_ON_VMDIR_ERROR(dwError);
    } else //no password nor password-file, read password from stdin
    {
        VmDirReadString("password: ", pszSecretBuf, VMDIR_MAX_PWD_LEN+1, FALSE);
    }

    {
        dwError = VmDirSetSRPSecret(pszUPN, pszSecretBuf);
        if ( dwError == 0 )
        {
            printf("SRP secret was set successfully.\n");
        }
        else if ( dwError == VMDIR_ERROR_ENTRY_ALREADY_EXIST )
        {
            dwError = 0;
            printf("SRP secret exists already.\n");
            // TODO, do a SRP bind to make sure  it works?
        }
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszErrorMessage);
    memset(pszSecretBuf, 0, sizeof(pszSecretBuf));
    return dwError;

error:
    VmDirGetErrorMessage(dwError, &pszErrorMessage);
    printf("Vdcsrp failed. Error[%d] - %s\n",
        dwError, ( pszErrorMessage ) ? pszErrorMessage : "");
    printf("Make sure the UPN and password are correct\n");

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
    return VmDirMain(argc, argv);
}

#endif
