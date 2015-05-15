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
 * Module Name: vdcsetupldu
 *
 * Filename: main.c
 *
 * Abstract:
 *
 * vdcsetupldu main module entry point
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
    DWORD   dwError = 0;

    PSTR        pszHostURI = NULL;
    PSTR        pszDomain = NULL;
    PSTR        pszLoginUser = NULL;
    PSTR        pszLoginPassword = NULL;
    PSTR        pszPwdFile = NULL;
    PSTR        pszErrorMessage = NULL;
    BOOLEAN     bVerbose = FALSE;
    CHAR        pszGuid[VMDIR_GUID_STR_LEN] = {0};
    CHAR        pszPath[MAX_PATH];
    PSTR        pszPasswordBuf = NULL;
    FILE *      fpPwdFile;
#ifndef _WIN32
    setlocale(LC_ALL, "");
#endif

    dwError = VmDirGetVmDirLogPath(pszPath, "vdcsetupldu.log");
    BAIL_ON_VMDIR_ERROR(dwError);
    dwError = VmDirLogInitialize(pszPath, FALSE, NULL, VMDIR_LOG_INFO, VMDIR_LOG_MASK_ALL );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirParseArgs(
                        argc, argv,
                        &pszHostURI,
                        &pszDomain,
                        &pszLoginUser,
                        &pszLoginPassword,
                        &bVerbose,
			&pszPwdFile);

    if (dwError != ERROR_SUCCESS)
    {
        ShowUsage();
        goto cleanup;
    }

    if (bVerbose)
    {
        VmDirSetLogLevel( "VERBOSE" );
    }

    dwError = VmDirAllocateMemory(VMDIR_MAX_PWD_LEN+1, (PVOID *)&pszPasswordBuf);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pszLoginPassword == NULL && pszPwdFile != NULL)
    {
       fpPwdFile = fopen(pszPwdFile, "rb");
       if (fpPwdFile == NULL)
       {
           dwError = VMDIR_ERROR_NO_SUCH_FILE_OR_DIRECTORY;
           VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "cannot open password file %s", pszPwdFile);
           BAIL_ON_VMDIR_ERROR(dwError);
       }
       if (fread(pszPasswordBuf, 1, VMDIR_MAX_PWD_LEN, fpPwdFile) == 0)
        {
           dwError = VMDIR_ERROR_VDCPROMO;
           VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "Invalid contents in password file %s", pszPwdFile);
           BAIL_ON_VMDIR_ERROR(dwError);
       }
    }
    else if (pszLoginPassword != NULL && pszPwdFile == NULL)
    {
       dwError = VmDirStringCpyA(pszPasswordBuf, VMDIR_MAX_PWD_LEN, pszLoginPassword);
       BAIL_ON_VMDIR_ERROR(dwError);
    }
    else //no password nor password-file, read password from stdin
    {
        VmDirReadString("password: ", pszPasswordBuf, VMDIR_MAX_PWD_LEN+1, FALSE);
    }

    dwError = VmDirSetupLdu(
                                pszHostURI,
                                pszDomain,
                                pszLoginUser,
                                pszPasswordBuf);

    BAIL_ON_VMDIR_ERROR(dwError);

    //first is SiteGuid, second is LduGuid. first boot script will get them and publish install parameters
    dwError = VmDirGetLocalSiteGuid(pszGuid);
    BAIL_ON_VMDIR_ERROR(dwError);
    printf("%s ", pszGuid);

    dwError = VmDirGetLocalLduGuid(pszGuid);
    BAIL_ON_VMDIR_ERROR(dwError);
    printf("%s\n", pszGuid);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszPasswordBuf);
    VMDIR_SAFE_FREE_MEMORY(pszErrorMessage);
    VmDirLogTerminate();

    return dwError;

error:
    VmDirGetErrorMessage(dwError, &pszErrorMessage);
    printf("Vdcsetupldu failed. Error[%d] - %s\n",
        dwError, ( pszErrorMessage ) ? pszErrorMessage : "");

    goto cleanup;
}

