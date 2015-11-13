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
 * Filename: parseargs.c
 *
 * Abstract:
 *
 * vdcpass argument parsing functions
 *
 */

#include "includes.h"

#ifndef _WIN32

DWORD
VmDirParseArgs(
    int     argc,
    char*   argv[],
    PSTR*   ppszHostURI,
    PSTR*   ppszLoginUserDN,
    PSTR*   ppszLoginPassword,
    PSTR*   ppszNewPassword,
    PSTR*   ppszUserDN
    )
{
    DWORD   dwError = ERROR_SUCCESS;
    int     opt = 0;
    PSTR    pszHostURI = NULL;
    PSTR    pszLoginUserDN = NULL;
    PSTR    pszLoginPassword = NULL;
    PSTR    pszNewPassword = NULL;
    PSTR    pszUserDN = NULL;

    if (ppszHostURI == NULL || ppszLoginUserDN == NULL || ppszLoginPassword == NULL || ppszNewPassword == NULL || ppszUserDN == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    while ( (opt = getopt( argc, argv, VMDIR_OPTIONS_VALID)) != EOF )
    {
        switch ( opt )
        {
            case VMDIR_OPTION_HOST:
                pszHostURI = optarg;
                break;

            case VMDIR_OPTION_USER_LOGIN:
                pszLoginUserDN = optarg;
                break;

            case VMDIR_OPTION_PASSWORD_LOGIN:
                pszLoginPassword = optarg;
                break;

            case VMDIR_OPTION_PASSWORD_CHANGE:
                pszNewPassword = optarg;
                break;

            case VMDIR_OPTION_USER_CHANGE:
                pszUserDN = optarg;
                break;

            default:
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMDIR_ERROR(dwError);
                break;
        }
    }

    if (pszHostURI == NULL || pszLoginUserDN == NULL || pszNewPassword ==NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppszHostURI = pszHostURI;
    *ppszLoginUserDN = pszLoginUserDN;
    *ppszLoginPassword = pszLoginPassword;
    *ppszNewPassword = pszNewPassword;
    *ppszUserDN = pszUserDN;

cleanup:
    return dwError;

error:
    goto cleanup;
}

#else

DWORD
VmDirParseArgs(
    int     argc,
    char*   argv[],
    PSTR*   ppszHostURI,
    PSTR*   ppszLoginUserDN,
    PSTR*   ppszLoginPassword,
    PSTR*   ppszNewPassword,
    PSTR*   ppszUserDN
    )
{
    DWORD dwError = ERROR_SUCCESS;
    //TBD
    return dwError;
}

#endif

VOID
ShowUsage(
    VOID
    )
{
    printf(
      "Usage: vdcmerge -h <host URI> -u <user DN> -w <user password> -W <new password>\n"
      "    [-U <user DN for password change>]\n"
      "Note: change password needs administrator privilege.\n");
}
