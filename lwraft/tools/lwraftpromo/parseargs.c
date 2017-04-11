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
 * Filename: parseargs.c
 *
 * Abstract:
 *
 * lwraftpromo argument parsing functions
 *
 */

#include "includes.h"

static
DWORD
_MapFirstReplCycleStrToEnum(
    PSTR                            pszFirstReplCycleMode,
    VMDIR_FIRST_REPL_CYCLE_MODE *   pEnumFirstReplCycleMode);

DWORD
VmDirParseArgs(
    int                             argc,
    char*                           argv[],
    PSTR*                           ppszDomain,
    PSTR*                           ppszUserName,
    PSTR*                           ppszPassword,
    PSTR*                           ppszSiteName,
    PSTR*                           ppszReplHostName,
    PSTR*                           ppszLotusServerName,
    PBOOLEAN                        pbHost,
    VMDIR_FIRST_REPL_CYCLE_MODE *   pEnumFirstReplCycleMode,
    PSTR*                           ppszPwdFile
)
{
    DWORD   dwError = ERROR_SUCCESS;
    PSTR    pszDomain = NULL;
    PSTR    pszUserName = NULL;
    PSTR    pszPassword = NULL;
    PSTR    pszSiteName = NULL;
    PSTR    pszReplHostName = NULL;
    PSTR    pszLotusServerName = NULL;
    PSTR    pszPwdFile = NULL;
    BOOLEAN bHost = TRUE;

#ifndef _WIN32
    int opt=0;
#else
    int     i=1;
    PSTR    optarg = NULL;
    PSTR    pszFirstReplCycleMode = NULL;
#endif

    if (ppszDomain == NULL              ||
        ppszUserName == NULL            ||
        ppszPassword == NULL            ||
        ppszSiteName == NULL            ||
        ppszReplHostName == NULL        ||
        ppszLotusServerName == NULL     ||
        pbHost == NULL                  ||
        pEnumFirstReplCycleMode == NULL ||
        ppszPwdFile == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

#ifndef _WIN32
    while ( (opt = getopt( argc, argv, VMDIR_OPTIONS_VALID)) != EOF )
    {
        switch ( opt )
        {
            case VMDIR_OPTION_DOMAIN:
                pszDomain = optarg;
                break;

            case VMDIR_OPTION_USER_NAME:
                pszUserName = optarg;
                break;

            case VMDIR_OPTION_PASSWORD:
                pszPassword = optarg;
                break;

            case VMDIR_OPTION_SITE_NAME:
                pszSiteName = optarg;
                break;

            case VMDIR_OPTION_REPL_HOST_NAME:
                pszReplHostName = optarg;
                break;

            case VMDIR_OPTION_LOTUS_SERVER_NAME:
                pszLotusServerName = optarg;
                break;

            case VMDIR_OPTION_TENANT:
                bHost = FALSE;
                break;

            case VMDIR_OPTION_FIRST_REPL_MODE:
                dwError = _MapFirstReplCycleStrToEnum( optarg, pEnumFirstReplCycleMode );
                BAIL_ON_VMDIR_ERROR(dwError);
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
    while( i < argc )
    {
        if( VmDirIsCmdLineOption( argv[i] ) != FALSE )
        {
            if ( VmDirStringCompareA(VMDIR_OPTION_DOMAIN, argv[i], TRUE ) == 0 )
            {
                VmDirGetCmdLineOption( argc, argv, &i, &pszDomain );
            }
            else if ( VmDirStringCompareA(VMDIR_OPTION_USER_NAME, argv[i], TRUE ) == 0 )
            {
                VmDirGetCmdLineOption( argc, argv, &i, &pszUserName );
            }
            else if ( VmDirStringCompareA(VMDIR_OPTION_PASSWORD, argv[i], TRUE ) == 0 )
            {
                VmDirGetCmdLineOption( argc, argv, &i, &pszPassword );
            }
            else if ( VmDirStringCompareA(VMDIR_OPTION_SITE_NAME, argv[i], TRUE ) == 0 )
            {
                VmDirGetCmdLineOption( argc, argv, &i, &pszSiteName );
            }
            else if ( VmDirStringCompareA(VMDIR_OPTION_REPL_HOST_NAME, argv[i], TRUE ) == 0 )
            {
                VmDirGetCmdLineOption( argc, argv, &i, &pszReplHostName );
            }
            else if ( VmDirStringCompareA(VMDIR_OPTION_LOTUS_SERVER_NAME, argv[i], TRUE ) == 0 )
            {
                VmDirGetCmdLineOption( argc, argv, &i, &pszLotusServerName );
            }
            else if ( VmDirStringCompareA(VMDIR_OPTION_TENANT, argv[i], TRUE ) == 0 )
            {
                bHost = FALSE;
            }
            else if ( VmDirStringCompareA(VMDIR_OPTION_FIRST_REPL_MODE, argv[i], TRUE ) == 0 )
            {
                VmDirGetCmdLineOption( argc, argv, &i, &pszFirstReplCycleMode );
                dwError = _MapFirstReplCycleStrToEnum( pszFirstReplCycleMode, pEnumFirstReplCycleMode );
                BAIL_ON_VMDIR_ERROR(dwError);
            } else if ( VmDirStringCompareA(VMDIR_OPTION_PWD_FILE, argv[i], TRUE ) == 0 )
            {
                VmDirGetCmdLineOption( argc, argv, &i, &pszPwdFile );
            }

        }

        i++;
    } // while

#endif

    if ((IsNullOrEmptyString(pszDomain) && IsNullOrEmptyString(pszReplHostName))   ||   // if neither specified
        (!IsNullOrEmptyString(pszDomain) && !IsNullOrEmptyString(pszReplHostName)) ||   // if both specified
        (IsNullOrEmptyString(pszUserName) && !bHost)                               ||   // require username for -t (tenant creation)
        (!IsNullOrEmptyString(pszPassword) && !IsNullOrEmptyString(pszPwdFile)))        // if both specified
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppszDomain = pszDomain;
    *ppszUserName = pszUserName;
    *ppszPassword = pszPassword;
    *ppszSiteName = pszSiteName;
    *ppszReplHostName = pszReplHostName;
    *ppszLotusServerName = pszLotusServerName;
    *pbHost = bHost;
    *ppszPwdFile = pszPwdFile;

cleanup:
    return dwError;
error:
    goto cleanup;
}

static
DWORD
_MapFirstReplCycleStrToEnum(
    PSTR                            pszFirstReplCycleMode,
    VMDIR_FIRST_REPL_CYCLE_MODE *   pEnumFirstReplCycleMode)
{
    DWORD   dwError = 0;

    if (VmDirStringCompareA(pszFirstReplCycleMode, "copyDB", FALSE) == 0)
    {
        *pEnumFirstReplCycleMode = FIRST_REPL_CYCLE_MODE_COPY_DB;
    }
    else if (VmDirStringCompareA(pszFirstReplCycleMode, "useCopiedDB", FALSE) == 0)
    {
        *pEnumFirstReplCycleMode = FIRST_REPL_CYCLE_MODE_USE_COPIED_DB;
    }
    else if (VmDirStringCompareA(pszFirstReplCycleMode, "replObjects", FALSE) == 0)
    {
        *pEnumFirstReplCycleMode = FIRST_REPL_CYCLE_MODE_OBJECT_BY_OBJECT;
    }
    else
    {
        dwError = ERROR_INVALID_PARAMETER;
    }

    return dwError;
}

VOID
ShowUsage(
    VOID
    )
{
    printf(
      "Usage: lwraftpromo [-w <password>|-x <password-file>] \n"
      "       [-u <username> Valid only if -t is also specified]\n"
      "       [-s <site name. Default is Default-First-Site>]\n"
      "       [-d <domain name, e.g. vsphere.local>]\n"
      "       [-h <optional preferred lotus server name, can be FQDN or IP format.>]\n"
      "       [-H <replication partner host>  "
      "       [-R <first replication cycle mode: copyDB/useCopiedDB/replObjects>]]\n"
      "       [-t]\n"
      "Note : Specify -d for first Ldu\n"
      "       Specify -H for subsequent Ldus\n"
      "       Specify -R to specify first replication cycle mode\n"
      "       Specify -t to create a tenant domain.\n"
      "       Otherwise, the host domain will be created.\n");
}
