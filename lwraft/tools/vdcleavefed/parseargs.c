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
    PSTR*   ppszRaftLeader,
    PSTR*   ppszServerToLeave,
    PSTR*   ppszUserName,
    PSTR*   ppszPassword
    )
{
    DWORD   dwError = ERROR_SUCCESS;

    PSTR    pszRaftLeader = NULL;
    PSTR    pszServerToLeave = NULL;
    PSTR    pszUserName = NULL;
    PSTR    pszPassword = NULL;
#ifndef _WIN32
	int     opt = 0;
#else
	int i=1;
	PSTR optarg = NULL;
#endif

    if (ppszRaftLeader == NULL || ppszServerToLeave == NULL || ppszUserName == NULL || ppszPassword == NULL )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

#ifndef _WIN32
    while ( (opt = getopt( argc, argv, VMDIR_OPTIONS_VALID)) != EOF )
    {
        switch ( opt )
        {
            case VMDIR_OPTION_RAFT_LEADER:
                pszRaftLeader = optarg;
                break;

            case VMDIR_OPTION_HOST_TO_REMOVE:
                pszServerToLeave = optarg;
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
            if (VmDirStringCompareA(VMDIR_OPTION_RAFT_LEADER, argv[i], TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszRaftLeader);
            } else if (VmDirStringCompareA(VMDIR_OPTION_RAFT_SERVR_TO_LEAVE, argv[i], TRUE) == 0)
            {
                VmDirGetCmdLineOption(argc, argv, &i, &pszServerToLeave);
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
    if (!pszUserName || !pszRaftLeader || !pszServerToLeave)
    {
         dwError = ERROR_INVALID_PARAMETER;
         BAIL_ON_VMDIR_ERROR(dwError);
    }
    *ppszRaftLeader = pszRaftLeader;
    *ppszServerToLeave = pszServerToLeave;
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
      "Usage: vdcleavefed -H <Raft leader in FQDN> -h <server to leave in FQDN>  -u <administrator user name> [-w <administrator password>]\n"
      "        server to remove must have been down\n");
}
