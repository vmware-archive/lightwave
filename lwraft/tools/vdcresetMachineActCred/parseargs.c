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
* Filename: parseargs.c
*
* Abstract:
*
* vdcresetMachineActCred argument parsing functions
*
*/

#include "includes.h"

DWORD
VmDirParseArgs(
    int      argc,
    char*    argv[],
    PSTR*    ppszUserName,
    PSTR*    ppszPartnerHostName,
    PSTR*    ppszPartnerCurrPassword
    )
{
    DWORD   dwError                 = ERROR_SUCCESS;
    PSTR    pszUserName             = NULL;
    PSTR    pszPartnerHostName      = NULL;
    PSTR    pszPartnerCurrPassword  = NULL;

#ifndef _WIN32
    int opt = 0;
#else
    int i=1;
    PSTR optarg = NULL;
#endif

    if ( ppszUserName  == NULL ||
        ppszPartnerHostName == NULL ||
        ppszPartnerCurrPassword == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

#ifndef _WIN32
    while ( (opt = getopt( argc, argv, VMDIR_OPTIONS_VALID)) != EOF )
    {
        switch ( opt )
        {
            case VMDIR_OPTION_USER_NAME:
                pszUserName = optarg;
                break;

            case VMDIR_OPTION_TARGET_HOSTNAME:
                pszPartnerHostName = optarg;
                break;

            case VMDIR_OPTION_SOURCE_PASSWORD:
                pszPartnerCurrPassword = optarg;
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
            if (VmDirStringCompareA(VMDIR_OPTION_USER_NAME, argv[i], TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszUserName);
            }
            else if (VmDirStringCompareA(VMDIR_OPTION_TARGET_HOSTNAME, argv[i], TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszPartnerHostName);
            }
            else if (VmDirStringCompareA(VMDIR_OPTION_SOURCE_PASSWORD, argv[i], TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszPartnerCurrPassword);
            }
        }
        i++;
    }
#endif

    if ( pszUserName  == NULL )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppszUserName               = pszUserName;
    *ppszPartnerHostName        = pszPartnerHostName;
    *ppszPartnerCurrPassword    = pszPartnerCurrPassword;

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
        "Usage: vdcresetMachineActCred -u <UserName> [-H <PartnerHost>] [-w <current password>]\n"
        "if -H is not specified, the local machine password will be modified\n"
        "if -w is not specified, read password from stdin\n"
        );
}
