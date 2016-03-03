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
 * Filename: parseargs.c
 *
 * Abstract:
 *
 * vdcschemaadmin  argument parsing functions
 *
 */

#include "includes.h"

DWORD
VmDirParseArgs(
    int      argc,
    char*    argv[],
    PSTR*    ppszUPN,
    PSTR*    ppszPartnerHostName,
    PSTR*    ppszVersionHostName,
    PSTR*    ppszUpgradeHostName,
    PSTR*    ppszSchemaFile,
    BOOLEAN* pDryRun,
    PSTR*    ppszPartnerCurrPassword
    )
{
    DWORD       dwError                 = ERROR_SUCCESS;
    PSTR        pszUPN                  = NULL;
    PSTR        pszPartnerHostName      = NULL;
    PSTR        pszVersionHostName      = NULL;
    PSTR        pszUpgradeHostName      = NULL;
    PSTR        pszSchemaFile           = NULL;
    PSTR        pszDryRun               = NULL;
    PSTR        pszPartnerCurrPassword  = NULL;

#ifndef _WIN32
    int opt = 0;
#else
    int i=1;
    PSTR optarg = NULL;
#endif

    if ( ppszUPN  == NULL               ||
         ppszPartnerHostName == NULL    ||
         ppszPartnerCurrPassword == NULL ||
         ppszUpgradeHostName == NULL    ||
         ppszSchemaFile == NULL         ||
         pDryRun == NULL
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

#ifndef _WIN32
    while ( (opt = getopt( argc, argv, VMDIR_OPTIONS_VALID)) != EOF )
    {
        switch ( opt )
        {
            case VMDIR_OPTION_UPN:
                pszUPN = optarg;
                break;

            case VMDIR_OPTION_TARGET_HOSTNAME:
                pszPartnerHostName = optarg;
                break;

            case VMDIR_OPTION_SOURCE_PASSWORD:
                pszPartnerCurrPassword = optarg;
                break;
            case VMDIR_OPTION_SOURCE_HOSTNAME:
                pszVersionHostName = optarg;
                break;
            case VMDIR_OPTION_UPGRADE_HOSTNAME:
                pszUpgradeHostName = optarg;
                break;
            case VMDIR_OPTION_UPGRADE_SCHEMAFILE:
                pszSchemaFile = optarg;
                break;
            case VMDIR_OPTION_UPGRADE_DRYRUN:
                pszDryRun = optarg;
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
            if (VmDirStringCompareA(VMDIR_OPTION_UPN, argv[i], TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszUPN);
            }
            else if (VmDirStringCompareA(VMDIR_OPTION_TARGET_HOSTNAME, argv[i], TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszPartnerHostName);
            }
            else if (VmDirStringCompareA(VMDIR_OPTION_SOURCE_HOSTNAME, argv[i], TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszVersionHostName);
            }
            else if (VmDirStringCompareA(VMDIR_OPTION_UPGRADE_HOSTNAME, argv[i], TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszUpgradeHostName);
            }
            else if (VmDirStringCompareA(VMDIR_OPTION_UPGRADE_SCHEMAFILE, argv[i], TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszSchemaFile);
            }
            else if (VmDirStringCompareA(VMDIR_OPTION_UPGRADE_DRYRUN, argv[i], TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszDryRun);
            }
            else if (VmDirStringCompareA(VMDIR_OPTION_SOURCE_PASSWORD, argv[i], TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszPartnerCurrPassword);
            }
        }
        i++;
    }
#endif

    if ( pszUPN  == NULL ||
         ( pszPartnerHostName  == NULL &&  pszVersionHostName == NULL &&  pszUpgradeHostName == NULL) ||
         ( pszUpgradeHostName != NULL && pszSchemaFile == NULL)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppszUPN                    = pszUPN;
    *ppszPartnerHostName        = pszPartnerHostName;
    *ppszVersionHostName        = pszVersionHostName;
    *ppszUpgradeHostName        = pszUpgradeHostName;
    *ppszSchemaFile             = pszSchemaFile;
    if (pszDryRun && VmDirStringCompareA(pszDryRun,"FALSE",FALSE)==0)
    {
        *pDryRun                = FALSE;
    }
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
            "Usage: vdcschemaadmin -D <Bind UPN> [-H|-V|-U] <Host> [-w <current password>] [-f SchemaFileFullPath]\n"
            " -H: To examine and report federation schema state.\n\n"
            " -V: If only metadata vesrion is out of sync, use this option to force sync metadata version.\n"
            "     Force sync takes time to converge in the federation.\n"
            "     So -H option could still report schema out of sync for a while after -V run.\n\n"
            " -U: Upgrade host schema.\n"
            "     -f specify full path to new schema definition file\n"
            "     [-d TRUE|FALSE] dry run default to TRUE\n\n"
            " -D: BIND UPN.\n"
            "if -w is not specified, read password from stdin\n"
          );
}


