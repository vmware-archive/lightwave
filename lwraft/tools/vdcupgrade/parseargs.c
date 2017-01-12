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
 * Module Name: vdcupgrade
 *
 * Filename: parseargs.c
 *
 * Abstract:
 *
 * vdcupgrade argument parsing functions
 *
 */

#include "includes.h"

DWORD
VmDirParseArgs(
    int     argc,
    char*   argv[],
    PSTR*   ppszServerName,
    PSTR*   ppszAdminUPN,
    PSTR*   ppszPassword,
    PSTR*   ppszPasswordFile,
    PBOOLEAN pbAclOnly,
    PSTR*   ppszPnidFixDcAccountName,
    PSTR*   ppszPnidFixNewSamAccount
    )
{
    DWORD   dwError = ERROR_SUCCESS;
#ifndef _WIN32
    int     opt = 0;
#else
    int i = 1;
    PSTR optarg = NULL;
#endif
    PSTR    pszServerName = NULL;
    PSTR    pszAdminUPN = NULL;
    PSTR    pszPassword = NULL;
    PSTR    pszPasswordFile = NULL;
    PSTR    pszPnidFixDcAccountName = NULL;
    PSTR    pszPnidFixNewSamAccount = NULL;
    PSTR    pszServerNameAlloc = NULL;
    PSTR    pszAdminUPNAlloc = NULL;
    PSTR    pszPasswordAlloc = NULL;
    PSTR    pszPasswordFileAlloc = NULL;
    PSTR    pszPnidFixDcAccountNameAlloc = NULL;
    PSTR    pszPnidFixNewSamAccountAlloc = NULL;
    BOOLEAN bAclOnly = FALSE;

    if (ppszServerName == NULL ||
        ppszAdminUPN == NULL  ||
        ppszPassword == NULL ||
        ppszPasswordFile == NULL ||
        pbAclOnly == NULL )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

#ifndef _WIN32
    while ( (opt = getopt( argc, argv, VMDIR_OPTIONS_VALID)) != EOF )
    {
        switch ( opt )
        {
            case VMDIR_OPTION_SERVER_NAME:
                pszServerName = optarg;
                break;

            case VMDIR_OPTION_ADMIN_UPN:
                pszAdminUPN = optarg;
                break;

            case VMDIR_OPTION_PASSWORD:
                pszPassword = optarg;
                break;

            case VMDIR_OPTION_PASSWORD_FILE:
                pszPasswordFile = optarg;
                break;
            case VMDIR_OPTION_ACLONLY:
                bAclOnly = TRUE;
                break;
            case VMDIR_OPTION_PNIDFIX_DCACCOUNT:
                pszPnidFixDcAccountName = optarg;
                break;
            case VMDIR_OPTION_PNIDFIX_SAMACCOUNT:
                pszPnidFixNewSamAccount = optarg;
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
            if (VmDirStringCompareA(VMDIR_OPTION_SERVER_NAME, argv[i], TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszServerName);
            }
            else if (VmDirStringCompareA(VMDIR_OPTION_ADMIN_UPN, argv[i], TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszAdminUPN);
            }
            else if (VmDirStringCompareA(VMDIR_OPTION_PASSWORD, argv[i], TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszPassword);
            }
            else if (VmDirStringCompareA(VMDIR_OPTION_PASSWORD_FILE, argv[i], TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszPasswordFile);
            }
            else if (VmDirStringCompareA(VMDIR_OPTION_ACLONLY, argv[i], TRUE) == 0)
            {
                bAclOnly = TRUE;
            }
            else if (VmDirStringCompareA(VMDIR_OPTION_PNIDFIX_DCACCOUNT, argv[i], TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszPnidFixDcAccountName);
            }
            else if (VmDirStringCompareA(VMDIR_OPTION_PNIDFIX_SAMACCOUNT, argv[i], TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszPnidFixNewSamAccount);
            }
        }
        i++;
    }
#endif

    dwError = VmDirAllocateStringA(pszServerName, &pszServerNameAlloc);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszAdminUPN, &pszAdminUPNAlloc);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pszPassword)
    {
        dwError = VmDirAllocateStringA(pszPassword, &pszPasswordAlloc);
        BAIL_ON_VMDIR_ERROR(dwError);
    } else if (pszPasswordFile)
    {
        dwError = VmDirAllocateStringA(pszPasswordFile, &pszPasswordFileAlloc);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pszPnidFixDcAccountName)
    {
        dwError = VmDirAllocateStringA(pszPnidFixDcAccountName, &pszPnidFixDcAccountNameAlloc);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pszPnidFixNewSamAccount)
    {
        dwError = VmDirAllocateStringA(pszPnidFixNewSamAccount, &pszPnidFixNewSamAccountAlloc);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppszServerName = pszServerNameAlloc;
    *ppszAdminUPN = pszAdminUPNAlloc;
    *ppszPassword = pszPasswordAlloc;
    *ppszPasswordFile = pszPasswordFileAlloc;
    *pbAclOnly = bAclOnly;
    *ppszPnidFixDcAccountName = pszPnidFixDcAccountNameAlloc;
    *ppszPnidFixNewSamAccount = pszPnidFixNewSamAccountAlloc;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_STRINGA(pszServerNameAlloc);
    VMDIR_SAFE_FREE_STRINGA(pszAdminUPNAlloc);
    VMDIR_SAFE_FREE_STRINGA(pszPasswordAlloc);
    VMDIR_SAFE_FREE_STRINGA(pszPasswordFileAlloc);
    VMDIR_SAFE_FREE_STRINGA(pszPnidFixDcAccountNameAlloc);
    VMDIR_SAFE_FREE_STRINGA(pszPnidFixNewSamAccountAlloc);
    goto cleanup;
}

VOID
ShowUsage(
    VOID
    )
{
    printf(
      "Usage: vdcupgrade -H <host> -D <AdminUPN> [-W <password>|-x <password-file>] -a\n"
      "Note: -a for ACL upgrade only.\n");
}
