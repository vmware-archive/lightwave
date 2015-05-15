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
 * Module Name: vdcleavefed
 *
 * Filename: parseargs.c
 *
 * Abstract:
 *
 * vdcleavefed argument parsing functions
 *
 */

#include "includes.h"

DWORD
VmDirParseArgs(
    int     argc,
    char*   argv[],
    PSTR*   ppszServerName,
    PSTR*   ppszUserName,
    PSTR*   ppszPassword
    )
{
    DWORD   dwError = ERROR_SUCCESS;

    PSTR    pszServerName = NULL;
    PSTR    pszUserName = NULL;
    PSTR    pszPassword = NULL;
#ifndef _WIN32
	int     opt = 0;
#else
	int i=1;
	PSTR optarg = NULL;
#endif

    if (ppszServerName == NULL || ppszUserName == NULL || ppszPassword == NULL )
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
                pszServerName = optarg;
                break;

            case VMDIR_OPTION_USER_LOGIN:
                pszUserName = optarg;
                break;

            case VMDIR_OPTION_PASSWORD_LOGIN:
                pszPassword = optarg;
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
                VmDirGetCmdLineOption(argc, argv, &i, &pszServerName);
            }
            else if (VmDirStringCompareA(VMDIR_OPTION_USER_LOGIN, argv[i], TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszUserName);
            }
            else if (VmDirStringCompareA(VMDIR_OPTION_PASSWORD_LOGIN, argv[i], TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszPassword);
            } else
                    {
                            BAIL_ON_VMDIR_ERROR(dwError);
                    }
            }
        i++;
    }
#endif
    if (!pszUserName)
    {
         dwError = ERROR_INVALID_PARAMETER;
         BAIL_ON_VMDIR_ERROR(dwError);
    }
    *ppszServerName = pszServerName;
    *ppszUserName = pszUserName;
    *ppszPassword = pszPassword;

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
      "Usage: vdcleavefed [ -h <server name in FQDN> ] -u <administrator user name> [-w <administrator password>]\n"
      "        implying offline mode if <server name> is provided, and the server must have been down.\n"
      "        implying online mode if <server name> is not provided\n" );
}
