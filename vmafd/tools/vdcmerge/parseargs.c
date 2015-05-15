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
 * Module Name: vdcmerge
 *
 * Filename: parseargs.c
 *
 * Abstract:
 *
 * vdcmerge argument parsing functions
 *
 */

#include "includes.h"

DWORD
VmAfdParseArgs(
    int      argc,
    char*    argv[],
    PSTR*    ppszTargetHost,
    PSTR*    ppszSourceUserName,
    PSTR*    ppszTargetUserName,
    PSTR*    ppszSourcePassword,
    PSTR*    ppszTargetPassword
    )
{
    DWORD   dwError = ERROR_SUCCESS;
    PSTR    pszTargetHost = NULL;
    PSTR    pszSourceUserName = NULL;
    PSTR    pszTargetUserName = NULL;
    PSTR    pszSourcePassword = NULL;
    PSTR    pszTargetPassword = NULL;
#ifndef _WIN32
    int opt=0;
#else
    int i=1;
    PSTR optarg = NULL;
#endif

    if (ppszTargetHost == NULL || ppszSourceUserName ==NULL || ppszTargetUserName == NULL ||
            ppszSourcePassword == NULL || ppszTargetPassword ==NULL )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

#ifndef _WIN32
    while ( (opt = getopt( argc, argv, VMAFD_OPTIONS_VALID)) != EOF )
    {
        switch ( opt )
        {
            case VMAFD_OPTION_TARGET_HOST:
                pszTargetHost = optarg;
                break;

            case VMAFD_OPTION_SOURCE_USERNAME:
                pszSourceUserName = optarg;
                break;

            case VMAFD_OPTION_TARGET_USERNAME:
                pszTargetUserName = optarg;
                break;

            case VMAFD_OPTION_SOURCE_PASSWORD:
                pszSourcePassword = optarg;
                break;

            case VMAFD_OPTION_TARGET_PASSWORD:
                pszTargetPassword = optarg;
                break;

            default:
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR(dwError);
                break;
        }
    }
#else
    while (i < argc)
    {
        if (VmAfdIsCmdLineOption(argv[i]) != FALSE)
        {
            if (VmAfdStringCompareA(VMAFD_OPTION_TARGET_HOST, argv[i], TRUE) == 0)
            {
                VmAfdGetCmdLineOption(argc, argv, &i, &pszTargetHost);
            }
            else if (VmAfdStringCompareA(VMAFD_OPTION_SOURCE_USERNAME, argv[i], TRUE) == 0)
            {
                VmAfdGetCmdLineOption(argc, argv, &i, &pszSourceUserName);
            }
            else if (VmAfdStringCompareA(VMAFD_OPTION_TARGET_USERNAME, argv[i], TRUE) == 0)
            {
                VmAfdGetCmdLineOption(argc, argv, &i, &pszTargetUserName);
            }
            else if (VmAfdStringCompareA(VMAFD_OPTION_SOURCE_PASSWORD, argv[i], TRUE) == 0)
            {
                VmAfdGetCmdLineOption(argc, argv, &i, &pszSourcePassword);
            }
            else if (VmAfdStringCompareA(VMAFD_OPTION_TARGET_PASSWORD, argv[i], TRUE) == 0)
            {
                VmAfdGetCmdLineOption(argc, argv, &i, &pszTargetPassword);
            }
        }
        i++;
    }
#endif

    if (pszTargetHost == NULL || pszSourceUserName ==NULL || pszTargetUserName == NULL ||
            pszSourcePassword == NULL || pszTargetPassword ==NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppszTargetHost = pszTargetHost;
    *ppszSourceUserName = pszSourceUserName;
    *ppszTargetUserName = pszTargetUserName;
    *ppszSourcePassword = pszSourcePassword;
    *ppszTargetPassword = pszTargetPassword;

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
      "Usage: vdcmerge -u <source_username> -w <source_password>"
      " -H <target_host> -U <target_username> -W <target_password> \n"
      "Note: please run it on source machine with root privilege.\n");
}
