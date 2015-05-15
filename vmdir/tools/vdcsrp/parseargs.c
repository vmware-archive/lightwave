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

DWORD
VmDirParseArgs(
    int     argc,
    char*   argv[],
    PSTR*   ppszUPN,
    PSTR*   ppszSecret,
    PSTR*   ppszSecret_file
    )
{
    DWORD   dwError = ERROR_SUCCESS;
#ifndef _WIN32
    int     opt = 0;
#else
    int i = 1;
    PSTR optarg = NULL;
#endif
    PSTR    pszUPN = NULL;
    PSTR    pszSecret = NULL;
    PSTR    pszSecret_file = NULL;

    if (ppszUPN == NULL ||
        ppszSecret == NULL ||
        ppszSecret_file == NULL)
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

            case VMDIR_OPTION_SECRET:
                pszSecret = optarg;
                break;

            case VMDIR_OPTION_SECRET_FILE:
                pszSecret_file = optarg;
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
            else if (VmDirStringCompareA(VMDIR_OPTION_SECRET, argv[i], TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszSecret);
            }
            else if (VmDirStringCompareA(VMDIR_OPTION_SECRET_FILE, argv[i], TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszSecret_file);
            }
        }
        i++;
    }
#endif

    if (pszUPN == NULL ||
	(pszSecret != NULL && pszSecret_file != NULL))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppszUPN = pszUPN;
    *ppszSecret = pszSecret;
    *ppszSecret_file = pszSecret_file;

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
      "Usage: vdcsrp -D <Bind UPN> [-W <current password>|-x <current password file>]\n"
      "Note: setting SRP secret needs administrator privilege.\n");
}
