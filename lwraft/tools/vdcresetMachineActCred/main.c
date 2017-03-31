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
* Module Name: vdcresetMachineActCred
*
* Filename: main.c
*
* Abstract:
*
* vdcresetMachineActCred main module entry point
*
*/

#include "includes.h"

static
int
VmDirMain(
    int argc,
    char* argv[]
    );

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

int
VmDirMain(int argc, char* argv[])
    {
    DWORD   dwError              = 0;

    PSTR    pszUserName          = NULL;
    PSTR    pszPartnerHost       = NULL;
    PSTR    pszPassword          = NULL;
    PSTR    pszPasswordBuf       = NULL;

    CHAR    pszPath[MAX_PATH];
    CHAR    pszLocalHostName[VMDIR_MAX_HOSTNAME_LEN] = {0};
#ifndef _WIN32
    setlocale(LC_ALL,"");
#endif

    dwError = VmDirGetVmDirLogPath(pszPath, "vdcresetMachineActCred.log");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLogInitialize(pszPath, FALSE, NULL, VMDIR_LOG_INFO, VMDIR_LOG_MASK_ALL );
    BAIL_ON_VMDIR_ERROR(dwError);

    //get commandline parameters
    dwError = VmDirParseArgs(argc,
                             argv,
                             &pszUserName,
                             &pszPartnerHost,
                             &pszPassword);

    if ( dwError )
    {
        ShowUsage();
        goto cleanup;
    }

    dwError = VmDirAllocateMemory(VMDIR_MAX_PWD_LEN+1, (PVOID *)&pszPasswordBuf);
    BAIL_ON_VMDIR_ERROR(dwError);

    if ( pszPassword != NULL )
    {
        dwError = VmDirStringCpyA(pszPasswordBuf, VMDIR_MAX_PWD_LEN, pszPassword);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    //no password, read password from stdin
    {
        VmDirReadString("password: ", pszPasswordBuf, VMDIR_MAX_PWD_LEN+1, TRUE);
    }

    dwError = VmDirGetHostName(pszLocalHostName, sizeof(pszLocalHostName)-1);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirResetMachineActCred(pszLocalHostName,
                                       pszPartnerHost,
                                       pszUserName,
                                       pszPasswordBuf);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("vdcresetMachineActCred completed.\n");

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "vdcresetMachineActCred completed.");

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszPasswordBuf);
    VmDirLogTerminate();

    return dwError;

error:
    printf("vdcresetMachineActCred failed. Error[%d]\n", dwError);
    goto cleanup;
    }
