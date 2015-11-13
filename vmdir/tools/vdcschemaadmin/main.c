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
 * Module Name: vdcschemaadmin
 *
 * Filename: main.c
 *
 * Abstract:
 *
 * vdcschemaadmin main module entry point
 *
 * Author : arokade
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

    DWORD       dwError                             =   0 ;
    PSTR        pszUPN                              = NULL;
    PSTR        pszBaseHostName                     = NULL;
    PSTR        pszPartnerCurrPassword              = NULL;
    PSTR        pszVersionHostName                  = NULL;
    PSTR        pszPasswordBuf                      = NULL;
    PSTR        pszResult                           = NULL;
    CHAR        pszPath[MAX_PATH];


#ifndef _WIN32
    setlocale(LC_ALL,"");
#endif

    dwError = VmDirGetVmDirLogPath(pszPath, "vdcschemaadmin.log");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLogInitialize(pszPath, FALSE, NULL, VMDIR_LOG_INFO, VMDIR_LOG_MASK_ALL );
    BAIL_ON_VMDIR_ERROR(dwError);

    //get command line parameters
    dwError = VmDirParseArgs(
            argc,
            argv,
            &pszUPN,
            &pszBaseHostName,
            &pszVersionHostName,
            &pszPartnerCurrPassword);

    if (dwError)
    {
        ShowUsage();
        goto cleanup;
    }

    dwError = VmDirAllocateMemory(VMDIR_MAX_PWD_LEN+1, (PVOID *)&pszPasswordBuf);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pszPartnerCurrPassword != NULL )
    {
        dwError = VmDirStringCpyA(pszPasswordBuf, VMDIR_MAX_PWD_LEN, pszPartnerCurrPassword);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
        //no password, read password from stdin
    {
        VmDirReadString("password: ", pszPasswordBuf, VMDIR_MAX_PWD_LEN+1, TRUE);
    }

    if( pszVersionHostName != NULL )
    {

        dwError = VmDirSyncVersionsInFederation(
                                       pszVersionHostName,
                                       pszUPN,
                                       pszPasswordBuf,
                                       &pszResult
                                       );

        BAIL_ON_VMDIR_ERROR(dwError);

        printf("Version sync completed successfully.\n%s\n",
               pszResult ? pszResult:"No metadata version synchronization required." );
    }
    else
    {
        dwError = VMDirCheckSchemaEquality(
                  pszBaseHostName ,
                  pszUPN ,
                  pszPasswordBuf
                  );
        BAIL_ON_VMDIR_ERROR (dwError);

        printf("Schema in the whole federation is all in sync. \n");

     }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszPasswordBuf);
    VMDIR_SAFE_FREE_MEMORY(pszResult);
    VmDirLogTerminate();

    return dwError;

error:
    printf("Vdcschemaadmin failed. Error[%d]\n", dwError);
    goto cleanup;

}
