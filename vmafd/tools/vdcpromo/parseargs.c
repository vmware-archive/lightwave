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
 * Module Name: vdcpromo
 *
 * Filename: parseargs.c
 *
 * Abstract:
 *
 * vdcpromo argument parsing functions
 *
 */

#include "includes.h"

DWORD
VmAfdParseArgs(
    int   argc,
    char* argv[],
    PSTR* ppszDomain,
    PSTR* ppszUserName,
    PSTR* ppszPassword,
    PSTR* ppszSiteName,
    PSTR* ppszReplHostName,
    PSTR* ppszLotusServerName,
    PSTR* ppszPwdFile,
    DNS_INIT_FLAG* pDnsInitFlag
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
    DNS_INIT_FLAG dnsInitFlag = FALSE;

#ifndef _WIN32
    int opt=0;
#else
    int     i=1;
    PSTR    optarg = NULL;
#endif

    if (ppszDomain == NULL          ||
        ppszUserName == NULL        ||
        ppszPassword == NULL        ||
        ppszReplHostName == NULL    ||
        ppszLotusServerName == NULL ||
        ppszSiteName == NULL        ||
        ppszPwdFile == NULL         ||
        pDnsInitFlag == NULL
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

#ifndef _WIN32
    while ( (opt = getopt( argc, argv, VMAFD_OPTIONS_VALID)) != EOF )
    {
        switch ( opt )
        {
            case VMAFD_OPTION_INIT_DNS:
                dnsInitFlag = DNS_INIT;
                break;

            case VMAFD_OPTION_DOMAIN:
                pszDomain = optarg;
                break;

            case VMAFD_OPTION_USER_NAME:
                pszUserName = optarg;
                break;

            case VMAFD_OPTION_PASSWORD:
                pszPassword = optarg;
                break;

            case VMAFD_OPTION_REPL_HOST_NAME:
                pszReplHostName = optarg;
                break;

            case VMDIR_OPTION_LOTUS_SERVER_NAME:
                pszLotusServerName = optarg;
                break;

            case VMAFD_OPTION_SITE_NAME:
                pszSiteName = optarg;
                break;

            case VMAFD_OPTION_PWD_FILE:
                pszPwdFile = optarg;
                break;

            default:
                dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                BAIL_ON_VMAFD_ERROR(dwError);
                break;
        }
    }
#else
    while( i < argc )
    {
        if( VmAfdIsCmdLineOption( argv[i] ) != FALSE )
        {
            if ( VmAfdStringCompareA(VMAFD_OPTION_INIT_DNS, argv[i], TRUE ) == 0 )
            {
                dnsInitFlag = DNS_INIT;
            }
            if ( VmAfdStringCompareA(VMAFD_OPTION_DOMAIN, argv[i], TRUE ) == 0 )
            {
                VmAfdGetCmdLineOption( argc, argv, &i, &pszDomain );
            }
            else if ( VmAfdStringCompareA(VMAFD_OPTION_USER_NAME, argv[i], TRUE ) == 0 )
            {
                VmAfdGetCmdLineOption( argc, argv, &i, &pszUserName );
            }
            else if ( VmAfdStringCompareA(VMAFD_OPTION_PASSWORD, argv[i], TRUE ) == 0 )
            {
                VmAfdGetCmdLineOption( argc, argv, &i, &pszPassword );
            }
            else if ( VmAfdStringCompareA(VMAFD_OPTION_SITE_NAME, argv[i], TRUE ) == 0 )
            {
                VmAfdGetCmdLineOption( argc, argv, &i, &pszSiteName );
            }
            else if ( VmAfdStringCompareA(VMAFD_OPTION_REPL_HOST_NAME, argv[i], TRUE ) == 0 )
            {
                VmAfdGetCmdLineOption( argc, argv, &i, &pszReplHostName );
            }
            else if ( VmAfdStringCompareA(VMDIR_OPTION_LOTUS_SERVER_NAME, argv[i], TRUE ) == 0 )
            {
                VmAfdGetCmdLineOption( argc, argv, &i, &pszLotusServerName );
            }
            else if ( VmAfdStringCompareA(VMAFD_OPTION_PWD_FILE, argv[i], TRUE ) == 0 )
            {
                VmAfdGetCmdLineOption( argc, argv, &i, &pszPwdFile );
            }
        }

        i++;
    } // while

#endif

    if ((IsNullOrEmptyString(pszDomain) && IsNullOrEmptyString(pszReplHostName)) ||     //if neither specified
        (!IsNullOrEmptyString(pszDomain) && !IsNullOrEmptyString(pszReplHostName)) ||   //if both specified
        IsNullOrEmptyString(pszUserName) ||
        (!IsNullOrEmptyString(pszPassword) && !IsNullOrEmptyString(pszPwdFile))) //if both specified
    {
        dwError = ERROR_LOCAL_OPTION_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppszDomain = pszDomain;
    *ppszUserName = pszUserName;
    *ppszPassword = pszPassword;
    *ppszSiteName = pszSiteName;
    *ppszReplHostName = pszReplHostName;
    *ppszLotusServerName = pszLotusServerName;
    *ppszPwdFile = pszPwdFile;
    *pDnsInitFlag = dnsInitFlag;

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
      "Usage: vdcpromo -u <username> [-w <password>|-x <password-file>] \n"
      "       [-s <site name. Default is Default-First-Site-Name>]\n"
      "       [-d <domain name, e.g. vsphere.local>]\n"
      "       [-h <optional preferred lotus server name, can be FQDN or IP format.>]\n"
      "       [-H <replication partner host>\n"
      "       [-n]\n"
      "Note : Specify -d for first Ldu\n"
      "       Specify -H for subsequent Ldus\n"
      "       Specify -n to initialize local DNS of an already promoted DC.\n");
}
