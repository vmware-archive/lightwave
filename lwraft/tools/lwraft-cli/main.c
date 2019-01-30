/*
 * Copyright �  2017 VMware, Inc.  All Rights Reserved.
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

#include "includes.h"

static
DWORD
ParseArgs(
    int   argc,
    char* argv[]
    );

static
void
ShowUsage(
    VOID
    );

static
DWORD
RaftCliExecNodeRequest(
    int   argc,
    char* argv[]
    );

static
DWORD
RaftCliExecDBBackupRequest(
    int   argc,
    char* argv[]
    );

static
DWORD
RaftCliExecDisasterRecoveryRequest(
    int   argc,
    char* argv[]
    );

static
DWORD
_RaftCliExecProcessRequest(
    int   argc,
    char* argv[]
    );

int
main(int argc, char* argv[])
{
    DWORD   dwError = 0;
    int     retCode = 0;
    PCSTR   pszErrorMsg = NULL;
    PSTR    pszErrorDesc = NULL;

    setlocale(LC_ALL, "");

    // TODO, should switch to use VmDirParseArguments.  See vmdir/tool/vdcschema/parseargs.c for example.
    dwError = ParseArgs(argc, argv);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    VMDIR_SAFE_FREE_STRINGA(pszErrorDesc);
    return dwError;

error:

    switch (dwError)
    {
        case VMDIR_ERROR_OPTION_UNKNOWN:
            retCode = 2;
            pszErrorMsg = "An unknown option was present on the command line.";
            break;
        case VMDIR_ERROR_OPTION_INVALID:
            retCode = 3;
            pszErrorMsg = "The options present on the command line are not valid.";
            break;
        case ERROR_LOCAL_PASSWORDFILE_CANNOT_OPEN:
            retCode = 4;
            pszErrorMsg = "Could not open password file.\nVerify the path is correct.";
            break;
        case ERROR_LOCAL_PASSWORDFILE_CANNOT_READ:
            retCode = 5;
            pszErrorMsg = "Problem reading password file.\nVerify contents of password file.";
            break;
        case ERROR_LOCAL_PASSWORD_EMPTY:
            retCode = 6;
            pszErrorMsg = "Invalid password; password cannot be empty.";
            break;
        case VMDIR_ERROR_CANNOT_CONNECT_VMDIR:
            retCode = 21;
            pszErrorMsg = "Could not connect to the local Persistent Objectstore Service.";
            break;
        case VMDIR_ERROR_SERVER_DOWN:
            retCode = 23;
            pszErrorMsg = "Could not connect to Persistent Objectstore Service.";
            break;
        case VMDIR_ERROR_USER_INVALID_CREDENTIAL:
            retCode = 24;
            pszErrorMsg = "Authentication to Persistent Objectstore Service failed.";
            break;
        case VMDIR_ERROR_ACCESS_DENIED:
            retCode = 25;
            pszErrorMsg = "Authorization failed.";
            break;
        case VMDIR_ERROR_NO_LEADER:
            retCode = 26;
            pszErrorMsg = "No raft leader.";
            break;
        case VMDIR_ERROR_ALREADY_PROMOTED:
            retCode = 26;
            pszErrorMsg = "Node already promoted.";
            break;
        case VMDIR_ERROR_UNWILLING_TO_PERFORM:
            retCode = 27;
            pszErrorMsg = "Server is unwilling to perform request.";
            break;
        case VMDIR_ERROR_UNAVAILABLE:
            retCode = 28;
            pszErrorMsg = "Server is not available.";
            break;
        default:
            VmDirGetErrorMessage(dwError, &pszErrorDesc);
            retCode = 1;
    }

    fprintf(
        stderr,
        "\n%s failed, error=%d %s %u\n",
        argv[0],
        retCode,
        pszErrorMsg ? pszErrorMsg : VDIR_SAFE_STRING(pszErrorDesc),
        dwError);


    goto cleanup;
}

static
DWORD
ParseArgs(
    int   argc,
    char* argv[]
    )
{
    DWORD dwError = 0;
    DWORD iArg = 0;
    DWORD dwArgsLeft = argc;
    PSTR  pszArg = NULL;

    if (!argc || !argv)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    /*
     * Skipping the first argument
     */
    iArg++;
    dwArgsLeft--;

    /*
     * case : No arguments provided
     */
    if (!dwArgsLeft)
    {
        dwError = VMDIR_ERROR_OPTION_INVALID;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    /*
     * pszArg represents current argument that is being scanned and processed
     */
    pszArg = argv[iArg++];
    dwArgsLeft--;

    /*
     * The first argument is the principal argument. Depending on this argument
     * appropriate handler is called
     */

    if (!VmDirStringCompareA(pszArg, "help", TRUE))
    {
        ShowUsage();
    }
    else if (!VmDirStringNCompareA(pszArg, "node", VmDirStringLenA("node"), TRUE))
    {
        dwError = RaftCliExecNodeRequest(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL);
    }
    else if (!VmDirStringNCompareA(pszArg, "database-backup", VmDirStringLenA("database-backup"), TRUE))
    {
        dwError = RaftCliExecDBBackupRequest(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL);
    }
    else if (!VmDirStringNCompareA(pszArg, "disaster-recovery", VmDirStringLenA("disaster-recovery"), TRUE))
    {
        dwError = RaftCliExecDisasterRecoveryRequest(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL);
    }
    else if (!VmDirStringNCompareA(pszArg, "process", VmDirStringLenA("process"), TRUE))
    {
        dwError = _RaftCliExecProcessRequest(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL);
    }
    else
    {
        dwError = VMDIR_ERROR_OPTION_UNKNOWN;
    }

    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    return dwError;

error:

    if (dwError == VMDIR_ERROR_INVALID_PARAMETER ||
        dwError == VMDIR_ERROR_OPTION_UNKNOWN ||
        dwError == VMDIR_ERROR_OPTION_INVALID)
    {
        ShowUsage();
    }

    goto cleanup;
}

static
DWORD
RaftCliExecNodeRequest(
    int   argc,
    char* argv[]
    )
{
    DWORD dwError= 0;
    DWORD idx = 0;
    PSTR pszServerName = NULL;
    PSTR pszLogin = NULL;
    PSTR pszPassword = NULL;
    PSTR pszDomain = NULL;
    PSTR pszPartnerName = NULL;
    PSTR pszDemoteName = NULL;
    PSTR pszPreferredHostName = NULL;

    typedef enum
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_LIST,
        PARSE_MODE_DEMOTE,
        PARSE_MODE_PROMOTE,
        PARSE_MODE_STATE,
        PARSE_MODE_STARTVOTE
    } PARSE_MODE;

    typedef enum
    {
        PARSE_SUB_MODE_OPEN = 0,
        PARSE_SUB_MODE_SERVER_NAME,
        PARSE_SUB_MODE_USER_NAME,
        PARSE_SUB_MODE_PASSWORD,
        PARSE_SUB_MODE_DOMAIN_NAME,
        PARSE_SUB_MODE_PARTNER_NAME,
        PARSE_SUB_MODE_DEMOTE_NAME,
        PARSE_SUB_MODE_PREFERRED_HOST_NAME
    } PARSE_SUB_MODE;

    /*
     * Initializing to default values
     */
    PARSE_MODE          mode    = PARSE_MODE_OPEN;
    PARSE_SUB_MODE      submode = PARSE_SUB_MODE_OPEN;
    LWRAFT_NODE_COMMAND  command = LWRAFT_DIR_COMMAND_UNKNOWN;

    /*
     * @Todo:Add unit test for error message
     */
    if (!argc)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    /*
     * Iterate over all arguments recursively.
     *  1. Scan the first level - List, Demote, Promote, Show
     *  2. Gather required sub-arguments
     */
    for (; idx < argc; idx++)
    {
        PSTR pszArg = argv[idx];

        switch (mode)
        {
            case PARSE_MODE_OPEN:

                if (!VmDirStringCompareA(pszArg, "demote", TRUE))
                {
                    command = LWRAFT_DIR_COMMAND_NODE_DEMOTE;
                    mode = PARSE_MODE_DEMOTE;
                }
                else if (!VmDirStringCompareA(pszArg, "list", TRUE))
                {
                    command = LWRAFT_DIR_COMMAND_NODE_LIST;
                    mode = PARSE_MODE_LIST;
                }
                else if (!VmDirStringCompareA(pszArg, "promote", TRUE))
                {
                    command = LWRAFT_DIR_COMMAND_NODE_PROMOTE;
                    mode = PARSE_MODE_PROMOTE;
                }
                else if (!VmDirStringCompareA(pszArg, "state", TRUE))
                {
                    command = LWRAFT_DIR_COMMAND_NODE_STATE;
                    mode = PARSE_MODE_STATE;
                }
                else if(!VmDirStringCompareA(pszArg, "startvote", TRUE))
                {
                    command = LWRAFT_DIR_COMMAND_NODE_STARTVOTE;
                    mode = PARSE_MODE_STARTVOTE;
                }
                else
                {
                    BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_OPTION_INVALID);
                }
                break;


            case PARSE_MODE_LIST:

                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                        if (!VmDirStringCompareA(pszArg, "--server-name", TRUE))
                        {
                            submode = PARSE_SUB_MODE_SERVER_NAME;
                        }
                        else
                        {
                            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
                        }
                        break;

                    case PARSE_SUB_MODE_SERVER_NAME:

                        pszServerName = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;

                    default:

                        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_OPTION_INVALID);
                        break;
                }

                break;

            case PARSE_MODE_STATE:

                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                        if (!VmDirStringCompareA(pszArg, "--login", TRUE))
                        {
                            submode = PARSE_SUB_MODE_USER_NAME;
                        }
                        else if (!VmDirStringCompareA(pszArg, "--password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        else if (!VmDirStringCompareA(pszArg, "--server-name", TRUE))
                        {
                            submode = PARSE_SUB_MODE_SERVER_NAME;
                        }
                        else
                        {
                            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
                        }
                        break;

                    case PARSE_SUB_MODE_SERVER_NAME:

                        pszServerName = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;

                    case PARSE_SUB_MODE_USER_NAME:

                        pszLogin = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;

                    case PARSE_SUB_MODE_PASSWORD:

                        pszPassword = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;

                    default:

                        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_OPTION_INVALID);
                        break;
                }

                break;


            case PARSE_MODE_PROMOTE:

                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                        if (!VmDirStringCompareA(pszArg, "--password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        //else if (!VmDirStringCompareA(pszArg, "--administrator", TRUE))
                        //{
                        //    submode = PARSE_SUB_MODE_USER_NAME;
                        //}
                        else if (!VmDirStringCompareA(pszArg, "--domain-name", TRUE))
                        {
                            submode = PARSE_SUB_MODE_DOMAIN_NAME;
                        }
                        else if (!VmDirStringCompareA(pszArg, "--partner-name", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PARTNER_NAME;
                        }
                        else if (!VmDirStringCompareA(pszArg, "--host-name", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PREFERRED_HOST_NAME;
                        }
                        else
                        {
                            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
                        }
                        break;

                    case PARSE_SUB_MODE_DOMAIN_NAME:

                        pszDomain = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;

                    case PARSE_SUB_MODE_USER_NAME:

                        pszLogin = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;

                    case PARSE_SUB_MODE_PASSWORD:

                        pszPassword = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;

                    case PARSE_SUB_MODE_PARTNER_NAME:

                        pszPartnerName = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;

                    case PARSE_SUB_MODE_PREFERRED_HOST_NAME:

                        pszPreferredHostName = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;

                    default:

                        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_OPTION_INVALID);
                        break;

                }

                break;


            case PARSE_MODE_DEMOTE:

                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                        if (!VmDirStringCompareA(pszArg, "--login", TRUE))
                        {
                            submode = PARSE_SUB_MODE_USER_NAME;
                        }
                        else if (!VmDirStringCompareA(pszArg, "--password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        else if (!VmDirStringCompareA(pszArg, "--server-name", TRUE))
                        {
                            submode = PARSE_SUB_MODE_SERVER_NAME;
                        }
                        else if (!VmDirStringCompareA(pszArg, "--demote-host-name", TRUE))
                        {
                            submode = PARSE_SUB_MODE_DEMOTE_NAME;
                        }
                        else
                        {
                            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
                        }
                        break;

                    case PARSE_SUB_MODE_SERVER_NAME:

                        pszServerName = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;

                    case PARSE_SUB_MODE_USER_NAME:

                        pszLogin = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;

                    case PARSE_SUB_MODE_PASSWORD:

                        pszPassword = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;
                    case PARSE_SUB_MODE_DEMOTE_NAME:

                        pszDemoteName = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;

                    default:

                        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_OPTION_INVALID);
                        break;

                }

                break;

            case PARSE_MODE_STARTVOTE:

                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                        if (!VmDirStringCompareA(pszArg, "--login", TRUE))
                        {
                            submode = PARSE_SUB_MODE_USER_NAME;
                        }
                        else if (!VmDirStringCompareA(pszArg, "--password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        else if (!VmDirStringCompareA(pszArg, "--server-name", TRUE))
                        {
                            submode = PARSE_SUB_MODE_SERVER_NAME;
                        }
                        else
                        {
                            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
                        }
                        break;

                    case PARSE_SUB_MODE_SERVER_NAME:

                        pszServerName = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;

                    case PARSE_SUB_MODE_USER_NAME:

                        pszLogin = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;

                    case PARSE_SUB_MODE_PASSWORD:

                        pszPassword = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;

                    default:

                        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_OPTION_INVALID);
                        break;
                }
                break;

            default:
                dwError = VMDIR_ERROR_INVALID_STATE;
                BAIL_ON_VMDIR_ERROR(dwError);
                break;

        }
    }

    switch (command)
    {
        case LWRAFT_DIR_COMMAND_NODE_STATE:

            dwError = RaftCliShowNodesA(pszServerName, pszLogin, pszPassword);
            BAIL_ON_VMDIR_ERROR(dwError);
            break;

        case LWRAFT_DIR_COMMAND_NODE_LIST:

            dwError = RaftCliListNodesA(pszServerName);
            BAIL_ON_VMDIR_ERROR(dwError);
            break;

        case LWRAFT_DIR_COMMAND_NODE_DEMOTE:

            dwError = RaftCliDemoteA(pszServerName, pszLogin, pszPassword, pszDemoteName);
            BAIL_ON_VMDIR_ERROR(dwError);

            printf("Persistent Objectstore Service instance %s is removed from cluster successfully.\n", pszDemoteName);
            break;

        case LWRAFT_DIR_COMMAND_NODE_PROMOTE:

            printf("Initializing Persistent Objectstore Service instance ... \n");

            if ((pszDomain && pszPartnerName) || (!pszDomain && !pszPartnerName))
            {
                BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_OPTION_INVALID);
            }
            else if (pszDomain)
            {
                dwError = RaftCliPromoteA(pszPreferredHostName, pszDomain, pszLogin, pszPassword);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            else if (pszPartnerName)
            {
                dwError = RaftCliPromotePartnerA(pszPreferredHostName, pszPartnerName, pszLogin, pszPassword);
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            printf("Persistent Objectstore Service instance created successfully\n");
            break;

        case LWRAFT_DIR_COMMAND_NODE_STARTVOTE:

            printf("Initializing vote on followers ... \n");

            dwError = RaftCliStartVoteA(pszLogin, pszPassword, pszServerName);
            BAIL_ON_VMDIR_ERROR(dwError);

            printf("New vote completed successfully\n");
            break;

        default:

            dwError = VMDIR_ERROR_INVALID_STATE;
            break;
    }

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
RaftCliExecDBBackupRequest(
    int   argc,
    char* argv[]
    )
{
    DWORD dwError= 0;
    DWORD iArg = 0;

    PSTR pszUserName = NULL;
    PSTR pszPassword = NULL;
    PSTR pszBackupPath = NULL;
    PSTR pszServerName = NULL;

    typedef enum
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_SERVER_NAME,
        PARSE_MODE_USER_NAME,
        PARSE_MODE_PASSWORD,
        PARSE_MODE_BACKUP_FOLDER_PATH
    } PARSE_MODE;

    /*
     * Initializing to default values
     */
    PARSE_MODE  parseMode    = PARSE_MODE_OPEN;

    for (iArg = 0; iArg < argc; iArg++)
    {
        PSTR pszArg = argv[iArg];
        switch (parseMode)
        {
        case PARSE_MODE_OPEN:
            if(!strcmp(pszArg, "--login"))
            {
                parseMode = PARSE_MODE_USER_NAME;
            }
            else if(!strcmp(pszArg, "--password"))
            {
                parseMode = PARSE_MODE_PASSWORD;
            }
            else if(!strcmp(pszArg, "--backuppath"))
            {
                parseMode = PARSE_MODE_BACKUP_FOLDER_PATH;
            }
            else if(!strcmp(pszArg, "--server-name"))
            {
                parseMode = PARSE_MODE_SERVER_NAME;
            }
            else
            {
                dwError = VMDIR_ERROR_OPTION_UNKNOWN;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            break;

        case PARSE_MODE_USER_NAME:
            if (pszUserName)
            {
                dwError = VMDIR_ERROR_OPTION_INVALID;
                BAIL_ON_VMDIR_ERROR(dwError);
            };
            pszUserName = pszArg;

            parseMode = PARSE_MODE_OPEN;

            break;

        case PARSE_MODE_PASSWORD:
            if (pszPassword)
            {
                dwError = VMDIR_ERROR_OPTION_INVALID;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            pszPassword = pszArg;

            parseMode = PARSE_MODE_OPEN;

            break;

        case PARSE_MODE_SERVER_NAME:
            if (pszPassword)
            {
                dwError = VMDIR_ERROR_OPTION_INVALID;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            pszServerName = pszArg;

            parseMode = PARSE_MODE_OPEN;

            break;

        case PARSE_MODE_BACKUP_FOLDER_PATH:
            if (pszBackupPath)
            {
                dwError = VMDIR_ERROR_OPTION_INVALID;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            pszBackupPath = pszArg;

            parseMode = PARSE_MODE_OPEN;

            break;

        default:

            dwError = VMDIR_ERROR_OPTION_INVALID;
            BAIL_ON_VMDIR_ERROR(dwError);

            break;
        }
    }

    dwError = RaftCliDBBackup(
        pszServerName,
        pszUserName ,
        pszPassword,
        pszBackupPath);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    return dwError;

error:
    goto cleanup;
}

static
DWORD
RaftCliExecDisasterRecoveryRequest(
    int   argc,
    char* argv[]
    )
{
    DWORD dwError= 0;
    DWORD iArg = 0;

    PSTR pszUserName = NULL;
    PSTR pszPassword = NULL;

    typedef enum
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_USER_NAME,
        PARSE_MODE_PASSWORD
    } PARSE_MODE;

    /*
     * Initializing to default values
     */
    PARSE_MODE  parseMode    = PARSE_MODE_OPEN;

    for (iArg = 0; iArg < argc; iArg++)
    {
        PSTR pszArg = argv[iArg];
        switch (parseMode)
        {
        case PARSE_MODE_OPEN:
            if(!strcmp(pszArg, "--login"))
            {
                parseMode = PARSE_MODE_USER_NAME;
            }
            else if(!strcmp(pszArg, "--password"))
            {
                parseMode = PARSE_MODE_PASSWORD;
            }
            else
            {
                dwError = VMDIR_ERROR_OPTION_UNKNOWN;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            break;

        case PARSE_MODE_USER_NAME:
            if (pszUserName)
            {
                dwError = VMDIR_ERROR_OPTION_INVALID;
                BAIL_ON_VMDIR_ERROR(dwError);
            };
            pszUserName = pszArg;

            parseMode = PARSE_MODE_OPEN;

            break;

        case PARSE_MODE_PASSWORD:
            if (pszPassword)
            {
                dwError = VMDIR_ERROR_OPTION_INVALID;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            pszPassword = pszArg;

            parseMode = PARSE_MODE_OPEN;

            break;

        default:

            dwError = VMDIR_ERROR_OPTION_INVALID;
            BAIL_ON_VMDIR_ERROR(dwError);

            break;
        }
    }

    dwError = RaftCliDRNodeRestoreFromDB(
        pszUserName,
        pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    return dwError;

error:
    goto cleanup;
}

static
DWORD
_RaftCliExecProcessRequest(
    int   argc,
    char* argv[]
    )
{
    DWORD   dwError= 0;
    DWORD   idx = 0;
    DWORD   dwGroupId = -1;

    typedef enum
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_STOP_PROCESS,
        PARSE_MODE_START_PROCESS,
        PARSE_MODE_LIST_PROCESSES
    } PARSE_MODE;

    typedef enum
    {
        PARSE_SUB_MODE_OPEN = 0,
        PARSE_SUB_MODE_GROUP_ID
    } PARSE_SUB_MODE;

    /*
     * Initializing to default values
     */
    PARSE_MODE          mode    = PARSE_MODE_OPEN;
    PARSE_SUB_MODE      submode = PARSE_SUB_MODE_OPEN;
    LWRAFT_NODE_COMMAND command = LWRAFT_DIR_COMMAND_UNKNOWN;

    if (!argc)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    /*
     * Iterate over all arguments recursively.
     *  1. Scan the first level - start, stop, list
     *  2. Gather required sub-arguments
     */
    for (; idx < argc; idx++)
    {
        PSTR pszArg = argv[idx];

        switch (mode)
        {
            case PARSE_MODE_OPEN:

                if (!VmDirStringCompareA(pszArg, "start", TRUE))
                {
                    command = LWRAFT_DIR_COMMAND_PROCESS_START;
                    mode = PARSE_MODE_START_PROCESS;
                }
                else if (!VmDirStringCompareA(pszArg, "stop", TRUE))
                {
                    command = LWRAFT_DIR_COMMAND_PROCESS_STOP;
                    mode = PARSE_MODE_STOP_PROCESS;
                }
                else if (!VmDirStringCompareA(pszArg, "list", TRUE))
                {
                    command = LWRAFT_DIR_COMMAND_PROCESS_LIST;
                    mode = PARSE_MODE_LIST_PROCESSES;
                }
                else
                {
                    BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_OPTION_INVALID);
                }
                break;

            case PARSE_MODE_STOP_PROCESS:
            case PARSE_MODE_START_PROCESS:

                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                        if (!VmDirStringCompareA(pszArg, "--group", TRUE))
                        {
                            submode = PARSE_SUB_MODE_GROUP_ID;
                        }
                        else
                        {
                            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
                        }
                        break;

                    case PARSE_SUB_MODE_GROUP_ID:

                        dwGroupId = atoi(pszArg);
                        submode = PARSE_SUB_MODE_OPEN;
                        break;

                    default:

                        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_OPTION_INVALID);
                        break;
                }
                break;

            default:
                dwError = VMDIR_ERROR_INVALID_STATE;
                BAIL_ON_VMDIR_ERROR(dwError);
                break;
        }
    }

    switch (command)
    {
        case LWRAFT_DIR_COMMAND_PROCESS_STOP:

            dwError = RaftCliStopProcessA(dwGroupId);
            BAIL_ON_VMDIR_ERROR(dwError);

            printf("Process stopped successfully\n");
            break;

        case LWRAFT_DIR_COMMAND_PROCESS_START:

            dwError = RaftCliStartProcessA(dwGroupId);
            BAIL_ON_VMDIR_ERROR(dwError);

            printf("Process started successfully\n");
            break;

        case LWRAFT_DIR_COMMAND_PROCESS_LIST:

            dwError = RaftCliListProcessesA();
            BAIL_ON_VMDIR_ERROR(dwError);

            break;

        default:

            dwError = VMDIR_ERROR_INVALID_STATE;
            break;
    }

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
void
ShowUsage(
    VOID
    )
{
    fprintf(
        stdout,
        "Usage: post-cli { arguments }\n\n"
        "Arguments:\n\n"
        "\tnode list        --server-name      <host name>\n\n"

        "\tnode state       --server-name      <host name>\n"
        "\t                 --login            <user@domain>\n"
        "\t                 --password         <password>\n\n"

        "\tnode promote     --password         <password>\n"
 //       "\t                [--administrator    <user name> default to \"administrator\"]\n"
        "\t                 [--host-name        <host name> preferred Lightwave POST host name, can be FQDN or IP]\n"
        "\t                 [--domain-name      <domain name>      (for first node deployment)\n"
        "\t                 or \n"
        "\t                 --partner-name     <host of partner>  (for other nodes deployment)]\n\n"

        "\tnode demote      --server-name      <host name>\n"
        "\t                 --login            <user@domain>\n"
        "\t                 --password         <password>\n"
        "\t                 --demote-host-name <host to demote>]\n\n"

        "\tnode startvote   [--server-name      <host name> ]\n"
        "\t                 --login            <user@domain>\n"
        "\t                 --password         <password>\n\n"
        "\tdisaster-recovery\n"
        "\t                 [ --login          <admin user id> ]\n"
        "\t                 [ --password       <password> ]\n\n"
        "\tdatabase-backup\n"
        "\t                 [ --server-name    <server name> ]\n"
        "\t                 [ --login          <admin user id> ]\n"
        "\t                 [ --password       <password> ]\n"
        "\t                 --backuppath       <ABS path to store backup>\n\n"
        "\tprocess start    --group            <group id>\n\n"
        "\tprocess stop     --group            <group id>\n\n"
        "\tprocess list\n\n"
        "\thelp\n");
}
