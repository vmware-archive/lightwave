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
 * Module Name: vdcsetldu
 *
 * Filename: parseargs.c
 *
 * Abstract:
 *
 * vdcsetldu argument parsing functions
 *
 */

#include "includes.h"

DWORD
VmDirParseArgs(
    int     argc,
    char*   argv[],
    PSTR*   ppszHostURI,
    PSTR*   ppszDomain,
    PSTR*   ppszLoginUser,
    PSTR*   ppszLoginPassword,
    PBOOLEAN pbVerbose,
    PSTR*   ppszPwdFile
    )
{
    DWORD   dwError = ERROR_SUCCESS;

    PSTR    pszHostURI = NULL;
    PSTR    pszDomain = NULL;
    PSTR    pszLoginUser = NULL;
    PSTR    pszLoginPassword = NULL;
    BOOLEAN bVerbose = FALSE;
    PSTR    pszPwdFile = NULL;
#ifndef _WIN32
	int     opt = 0;
#else
	int i=1;
	PSTR optarg = NULL;
#endif

    if (ppszHostURI == NULL || ppszDomain == NULL || ppszLoginUser == NULL || ppszLoginPassword == NULL || ppszPwdFile == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

#ifndef _WIN32
    while ( (opt = getopt( argc, argv, VMDIR_OPTIONS_VALID)) != EOF )
    {
        switch ( opt )
        {
            case VMDIR_OPTION_HOST:
                pszHostURI = optarg;
                break;

            case VMDIR_OPTION_DOMAIN:
                pszDomain = optarg;
                break;

            case VMDIR_OPTION_USER_LOGIN:
                pszLoginUser = optarg;
                break;

            case VMDIR_OPTION_PASSWORD_LOGIN:
                pszLoginPassword = optarg;
                break;

            case VMDIR_OPTION_VERBOSE:
                bVerbose = TRUE;
                break;

	    case VMDIR_OPTION_PWD_FILE:
                pszPwdFile = optarg;
                break;

            default:
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMDIR_ERROR(dwError);
                break;
        }
    }
#else
	while (i < argc)
	{
		if (VmDirIsCmdLineOption(argv[i]) != FALSE)
		{
			if (VmDirStringCompareA(VMDIR_OPTION_HOST, argv[i], TRUE) == 0)
			{
				VmDirGetCmdLineOption(argc, argv, &i, &pszHostURI);
			}
			else if (VmDirStringCompareA(VMDIR_OPTION_DOMAIN, argv[i], TRUE) == 0)
			{
				VmDirGetCmdLineOption(argc, argv, &i, &pszDomain);
			}
			else if (VmDirStringCompareA(VMDIR_OPTION_USER_LOGIN, argv[i], TRUE) == 0)
			{
				VmDirGetCmdLineOption(argc, argv, &i, &pszLoginUser);
			}
			else if (VmDirStringCompareA(VMDIR_OPTION_PASSWORD_LOGIN, argv[i], TRUE) == 0)
			{
				VmDirGetCmdLineOption(argc, argv, &i, &pszLoginPassword);
			}
			else if (VmDirStringCompareA(VMDIR_OPTION_VERBOSE, argv[i], TRUE) == 0)
			{
				bVerbose = TRUE;
			} else if ( VmDirStringCompareA(VMDIR_OPTION_PWD_FILE, argv[i], TRUE ) == 0 )
		        {
		                VmDirGetCmdLineOption( argc, argv, &i, &pszPwdFile );
                        }
                }
		i++;
	}
#endif

    if (pszHostURI == NULL || pszDomain == NULL || pszLoginUser == NULL ||
	(pszLoginPassword != NULL && pszPwdFile != NULL)) //if both specified
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppszHostURI = pszHostURI;
    *ppszDomain = pszDomain;
    *ppszLoginUser = pszLoginUser;
    *ppszLoginPassword = pszLoginPassword;
    *pbVerbose = bVerbose;
    *ppszPwdFile = pszPwdFile;
cleanup:
    return dwError;

error:
    goto cleanup;
}

VOID
ShowUsage(
    VOID
    )
{
    printf(
      "Usage: vdcsetldu -h <host URI> -d <domain name> -u <user DN> [-w <user password>|-x <password-file>]\n");
}
