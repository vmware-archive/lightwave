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
 * Module Name: lwraftpromo
 *
 * Filename: main.c
 *
 * Abstract:
 *
 * lwraftpromo main module entry point
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

static
int VmDirMain(int argc, char* argv[])
{
    DWORD   dwError = 0;

    PSTR   pszDomain = NULL;
    PSTR   pszUserName = NULL;
    PSTR   pszPassword = NULL;
    PSTR   pszPwdFile = NULL;
    PSTR   pszSiteName = NULL;
    PSTR   pszPartnerHostName = NULL;
    PSTR   pszSystemDomainAdminName = "Administrator";   // for default system domain Admin
    PSTR   pszPartnerHostNameCanon = NULL;
    PSTR   pszLotusServerName = NULL;
    PSTR   pszPasswordBuf = NULL;
    PSTR   pszErrorMessage = NULL;

    BOOLEAN bHost = TRUE;
    CHAR    pszPath[MAX_PATH];
    VMDIR_FIRST_REPL_CYCLE_MODE firstReplCycleMode = FIRST_REPL_CYCLE_MODE_COPY_DB;
    FILE *fpPwdFile = NULL;

#ifndef _WIN32
    setlocale(LC_ALL, "");
#endif

    dwError = VmDirGetVmDirLogPath(pszPath, "lwraftpromo.log");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLogInitialize( pszPath, FALSE, NULL, VMDIR_LOG_INFO, VMDIR_LOG_MASK_ALL );
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirLogSetLevel( VMDIR_LOG_VERBOSE );

    dwError = VmDirParseArgs( argc,
                              argv,
                              &pszDomain,           // system domain name
                              &pszUserName,         // administrator user - default to "Administrator"
                              &pszPassword,         // administrator password
                              &pszSiteName,         // default to "Default-First-Site"
                              &pszPartnerHostName,  // replication partner host to join federaion
                              &pszLotusServerName,  // preferred lotus server name
                              &bHost,
                              &firstReplCycleMode,
                              &pszPwdFile);
    if (dwError)
    {
        ShowUsage();
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(VMDIR_MAX_PWD_LEN+1, (PVOID *)&pszPasswordBuf);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pszPassword == NULL && pszPwdFile != NULL)
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
    else if (pszPassword != NULL && pszPwdFile == NULL)
    {
	dwError = VmDirStringCpyA(pszPasswordBuf, VMDIR_MAX_PWD_LEN, pszPassword);
	BAIL_ON_VMDIR_ERROR(dwError);
    } else //no password nor password-file, read password from stdin
    {
       VmDirReadString("password: ", pszPasswordBuf, VMDIR_MAX_PWD_LEN+1, FALSE);
    }

    if (bHost)
    {
        if ( !IsNullOrEmptyString(pszUserName)
             &&
             VmDirStringCompareA( pszUserName, pszSystemDomainAdminName, FALSE) != 0
           )
        {
            VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL,
                               "For system domain creation, overrides -u option using (%s)",
                               VDIR_SAFE_STRING(pszSystemDomainAdminName) );
        }

        printf("Initializing Directory server instance ... \n");

        if (IsNullOrEmptyString(pszPartnerHostName))
        {
            dwError = VmDirSetupHostInstance(
                                        pszDomain,
                                        pszLotusServerName ? pszLotusServerName : "localhost",
                                        pszSystemDomainAdminName,
                                        pszPasswordBuf,
                                        pszSiteName);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            dwError = VmDirGetCanonicalHostName(pszPartnerHostName, &pszPartnerHostNameCanon);
            BAIL_ON_VMDIR_ERROR(dwError);
            VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "Canon partner host name (%s)", pszPartnerHostNameCanon);

            dwError = VmDirJoin(
                            pszLotusServerName ? pszLotusServerName : "localhost",
                            pszSystemDomainAdminName,
                            pszPasswordBuf,
                            pszSiteName,
                            pszPartnerHostNameCanon,
                            firstReplCycleMode);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        printf("Directory host instance created successfully\n");
    }
    else
    {
        dwError = VmDirSetupTenantInstance(
                        pszDomain,
                        pszUserName,
                        pszPasswordBuf);
        BAIL_ON_VMDIR_ERROR(dwError);

        printf("Directory tenant instance created successfully\n");
    }

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "--- lwraftpromo succeeded ---");

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszPasswordBuf);
    VMDIR_SAFE_FREE_MEMORY(pszErrorMessage);
    VmDirLogTerminate();
    VMDIR_SAFE_FREE_MEMORY(pszPartnerHostNameCanon);

    return dwError > 255 ? 255 : dwError;

error:
    VmDirGetErrorMessage(dwError, &pszErrorMessage);
    printf("Vdcpromo failed. Error[%d] - %s\n",
        dwError, ( pszErrorMessage ) ? pszErrorMessage : "");
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "--- lwraftpromo failed (%u)(%s)---",
        dwError, ( pszErrorMessage ) ? pszErrorMessage : "");

    goto cleanup;
}

