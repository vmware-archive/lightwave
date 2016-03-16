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
 *
 * Module   : vecs-cli/main.c
 * Author   : Aishu Raghavan (araghavan@vmware.com)
 */



#include "includes.h"

static
DWORD
ParseArgs(
    int   argc,
    char* argv[]
    );

static
DWORD
VecsCliExecStoreRequest(
    int   argc,
    char* argv[]
    );

static
DWORD
VecsCliExecEntryRequest(
    int   argc,
    char* argv[]
    );

static
DWORD
VecsCliTriggerRefreshThread(
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
GetPassword(
    PSTR *ppszPassword
    );

static
void
PrintError (
    DWORD dwError
    );

#ifndef _WIN32
int  main(int argc, char* argv[])
#else
int _tmain(int argc, _TCHAR* targv[])
#endif
{
    DWORD dwError = 0;

#ifdef _WIN32

    char** allocArgv = NULL;
    PSTR* argv = NULL;
    char pathsep = '\\';

#ifdef UNICODE

    dwError = VmAfdAllocateArgsAFromArgsW( argc, targv, &allocArgv );
    BAIL_ON_VMAFD_ERROR(dwError);
    argv = allocArgv;

#else  /* ifndef UNICODE */

    argv = targv; // non-unicode => targv is char

#endif /* ifdef UNICODE */

#else /* ifndef _WIN32 */

    setlocale(LC_ALL, "");

#endif /* ifdef _WIN32 */

    dwError = ParseArgs(argc, argv);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    PrintError (dwError);
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
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    iArg++; // skip first argument
    dwArgsLeft--;

    if (!dwArgsLeft)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pszArg = argv[iArg++];
    dwArgsLeft--;

    if (!strcmp(pszArg, "help"))
    {
        ShowUsage();
    }
    else if (!strcmp(pszArg, "store"))
    {
        dwError = VecsCliExecStoreRequest(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL);
    }
    else if (!strcmp(pszArg, "entry"))
    {
        dwError = VecsCliExecEntryRequest(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL);
    }
    else if (!strcmp(pszArg, "force-refresh"))
    {
        dwError = VecsCliTriggerRefreshThread(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL);
    }
    else
    {
        dwError = ERROR_INVALID_PARAMETER;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    if (dwError == ERROR_INVALID_PARAMETER)
    {
        ShowUsage();
    }

    goto cleanup;
}

static
DWORD
VecsCliExecStoreRequest(
    int   argc,
    char* argv[]
    )
{
    DWORD dwError = 0;
    PSTR  pszStoreName = NULL;
    PSTR  pszPassword = NULL;
    PSTR  pszUserName = NULL;
    PSTR  pszServerName = NULL;
    PSTR  pszUPN = NULL;
    PSTR  pszLotusPassword = NULL;
    DWORD idx = 0;
    DWORD dwForceDelete = 0;
    DWORD dwAccessMask = 0;
    VECS_COMMAND command = VECS_COMMAND_UNKNOWN;
    VECS_PERMISSION_MODE permMode = VECS_PERMISSION_MODE_UNKNOWN;
    typedef enum
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_CREATE,
        PARSE_MODE_DELETE,
        PARSE_MODE_LIST,
        PARSE_MODE_PERMISSIONS,
        PARSE_MODE_GET_PERMISSIONS
    } PARSE_MODE;
    typedef enum
    {
        PARSE_SUB_MODE_OPEN = 0,
        PARSE_SUB_MODE_NAME,
        PARSE_SUB_MODE_PASSWORD,
        PARSE_SUB_MODE_USER,
        PARSE_SUB_MODE_MASK,
        PARSE_SUB_MODE_SERVER,
        PARSE_SUB_MODE_UPN
    } PARSE_SUB_MODE;
    PARSE_MODE mode = PARSE_MODE_OPEN;
    PARSE_SUB_MODE submode = PARSE_SUB_MODE_OPEN;
    PVMAFD_SERVER pServer = NULL;

    if (!argc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    for (; idx < argc; idx++)
    {
        PSTR pszArg = argv[idx];

        switch (mode)
        {
            case PARSE_MODE_OPEN:

                if (!strcmp(pszArg, "create"))
                {
                    command = VECS_COMMAND_STORE_CREATE;
                    mode = PARSE_MODE_CREATE;
                }
                else if (!strcmp(pszArg, "list"))
                {
                    command = VECS_COMMAND_STORE_LIST;
                    mode = PARSE_MODE_LIST;
                }
                else if (!strcmp(pszArg, "delete"))
                {
                    command = VECS_COMMAND_STORE_DELETE;
                    mode = PARSE_MODE_DELETE;
                }
                else if (!strcmp(pszArg, "permission"))
                {
                    command = VECS_COMMAND_STORE_PERMISSION;
                    mode = PARSE_MODE_PERMISSIONS;
                }
                else if (!strcmp(pszArg, "get-permissions"))
                {
                    command = VECS_COMMAND_STORE_GET_PERMISSIONS;
                    mode = PARSE_MODE_GET_PERMISSIONS;
                }
                else
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_CREATE:

                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                        if (!strcmp(pszArg, "--name"))
                        {
                            submode = PARSE_SUB_MODE_NAME;
                        }
                        else if (!strcmp(pszArg, "--server"))
                        {
                            submode = PARSE_SUB_MODE_SERVER;
                        }
                        else if (!strcmp(pszArg, "--upn"))
                        {
                            submode = PARSE_SUB_MODE_UPN;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        break;

                    case PARSE_SUB_MODE_NAME:

                        pszStoreName = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_SERVER:

                        pszServerName = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_UPN:

                        pszUPN = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    default:

                        dwError = ERROR_INVALID_PARAMETER;

                        BAIL_ON_VMAFD_ERROR (dwError);

                        break;
                }

                break;

            case PARSE_MODE_DELETE:

                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                        if (!strcmp(pszArg, "--name"))
                        {
                            submode = PARSE_SUB_MODE_NAME;
                        }
                        else if (!strcmp(pszArg, "-y"))
                        {
                            dwForceDelete = 1;
                            submode = PARSE_SUB_MODE_OPEN;
                        }
                        else if (!strcmp(pszArg, "--server"))
                        {
                            submode = PARSE_SUB_MODE_SERVER;
                        }
                        else if (!strcmp(pszArg, "--upn"))
                        {
                            submode = PARSE_SUB_MODE_UPN;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        break;

                    case PARSE_SUB_MODE_NAME:

                        pszStoreName = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_SERVER:

                        pszServerName = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_UPN:

                        pszUPN = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    default:

                        dwError = ERROR_INVALID_STATE;
                        BAIL_ON_VMAFD_ERROR(dwError);

                        break;
                }

                break;

            case PARSE_MODE_LIST:
                switch(submode)
                {
                    case PARSE_SUB_MODE_OPEN:
                      if (!strcmp(pszArg, "--server"))
                      {
                          submode = PARSE_SUB_MODE_SERVER;
                      }
                      else if (!strcmp(pszArg, "--upn"))
                      {
                          submode = PARSE_SUB_MODE_UPN;
                      }
                      else
                      {
                          dwError = ERROR_INVALID_PARAMETER;
                          BAIL_ON_VMAFD_ERROR (dwError);
                      }
                      break;

                    case PARSE_SUB_MODE_SERVER:
                      if (!pszServerName)
                      {
                          pszServerName = pszArg;
                      }

                      submode = PARSE_SUB_MODE_OPEN;

                      break;

                    case PARSE_SUB_MODE_UPN:
                      if (!pszUPN)
                      {
                          pszUPN = pszArg;
                      }

                      submode = PARSE_SUB_MODE_OPEN;

                      break;

                    default:
                      dwError = ERROR_INVALID_PARAMETER;
                      BAIL_ON_VMAFD_ERROR (dwError);

                      break;
                }

                break;

            case PARSE_MODE_PERMISSIONS:

                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                      if (!strcmp(pszArg, "--name"))
                      {
                          submode = PARSE_SUB_MODE_NAME;
                      }
                      else if (!strcmp(pszArg, "--grant"))
                      {
                          if (permMode != VECS_PERMISSION_MODE_UNKNOWN)
                          {
                              dwError = ERROR_INVALID_PARAMETER;
                              BAIL_ON_VMAFD_ERROR (dwError);
                          }
                          permMode = VECS_PERMISSION_MODE_GRANT;
                          submode = PARSE_SUB_MODE_MASK;
                      }
                      else if (!strcmp(pszArg, "--revoke"))
                      {
                          if (permMode != VECS_PERMISSION_MODE_UNKNOWN)
                          {
                              dwError = ERROR_INVALID_PARAMETER;
                              BAIL_ON_VMAFD_ERROR (dwError);
                          }
                          permMode = VECS_PERMISSION_MODE_REVOKE;
                          submode = PARSE_SUB_MODE_MASK;
                      }
                      else if (!strcmp (pszArg, "--user"))
                      {
                          submode = PARSE_SUB_MODE_USER;
                      }
                      else
                      {
                          dwError = ERROR_INVALID_PARAMETER;
                          BAIL_ON_VMAFD_ERROR (dwError);
                      }
                      break;

                    case PARSE_SUB_MODE_NAME:

                      if (!pszStoreName)
                      {
                          pszStoreName = pszArg;
                      }

                      submode = PARSE_SUB_MODE_OPEN;

                      break;

                    case PARSE_SUB_MODE_MASK:

                      if (!strcmp(pszArg, "read"))
                      {
                          dwAccessMask = dwAccessMask |
                                          READ_STORE;
                      }
                      else if (!strcmp (pszArg, "write"))
                      {
                          dwAccessMask = dwAccessMask |
                                         WRITE_STORE;
                      }
                      else
                      {
                          dwError = ERROR_INVALID_PARAMETER;
                          BAIL_ON_VMAFD_ERROR (dwError);
                      }

                      submode = PARSE_SUB_MODE_OPEN;
                      break;

                    case PARSE_SUB_MODE_USER:

                      if (!pszUserName)
                      {
                          pszUserName = pszArg;
                      }

                      submode = PARSE_SUB_MODE_OPEN;

                      break;

                    default:

                      dwError = ERROR_INVALID_PARAMETER;
                      BAIL_ON_VMAFD_ERROR (dwError);
                      break;

                }
                break;

            case PARSE_MODE_GET_PERMISSIONS:

                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                        if (!strcmp(pszArg, "--name"))
                        {
                            submode = PARSE_SUB_MODE_NAME;
                        }
                        else if (!strcmp(pszArg, "--server"))
                        {
                            submode = PARSE_SUB_MODE_SERVER;
                        }
                        else if (!strcmp(pszArg, "--upn"))
                        {
                            submode = PARSE_SUB_MODE_UPN;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        break;

                    case PARSE_SUB_MODE_NAME:

                        pszStoreName = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_SERVER:

                        pszServerName = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_UPN:

                        pszUPN = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    default:

                        dwError = ERROR_INVALID_STATE;
                        BAIL_ON_VMAFD_ERROR(dwError);

                        break;
                }

                break;
        }
    }

    if (IsNullOrEmptyString(pszServerName) ^
        IsNullOrEmptyString(pszUPN)
       )
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VECS_CLI_ERROR (
                        dwError,
                        "Error: You have to provide both server and upn \n"
                        );
    }

    if (!IsNullOrEmptyString(pszServerName))
    {
        fprintf (
            stdout,
            "Enter password:\t"
            );

        dwError = GetPassword(&pszLotusPassword);
        BAIL_ON_VECS_CLI_ERROR(
                          dwError,
                          "Failed to get password from user \n"
                          );
        fprintf (stdout, "\n");
    }

    dwError = VmAfdOpenServerA(
                  pszServerName,
                  pszUPN,
                  pszLotusPassword,
                  &pServer);
    BAIL_ON_VECS_CLI_ERROR(
                          dwError,
                          "Failed to establish connection to remote server \n"
                          );

    switch (command)
    {
        case VECS_COMMAND_STORE_CREATE:

            dwError = VecsCliCreateStoreA(pServer, pszStoreName);

            break;

        case VECS_COMMAND_STORE_LIST:

            dwError = VecsCliListStoreA(pServer);

            break;

        case VECS_COMMAND_STORE_DELETE:

            if (!dwForceDelete)
            {
                char input = 0;
                fprintf (stdout,
                         "Warning: This operation will delete store [%s]\n"
                         "Do you wish to continue? Y/N [N] \n",
                         pszStoreName
                        );
                scanf (
                        "%c",
                        &input
                        );

                if (input == 'Y' || input == 'y')
                {
                    dwForceDelete = 1;
                }
            }

            if (dwForceDelete)
            {
                dwError = VecsCliDeleteStoreA(pServer, pszStoreName, pszPassword);
            }

            break;

        case VECS_COMMAND_STORE_PERMISSION:

            dwError = VecsCliSetPermissionA (
                                      pszStoreName,
                                      pszUserName,
                                      permMode,
                                      dwAccessMask
                                      );
            break;

        case VECS_COMMAND_STORE_GET_PERMISSIONS:

            dwError = VecsCliGetPermissionsA (
                                     pServer,
                                     pszStoreName
                                     );
            break;

        default:

            dwError = ERROR_INVALID_STATE;

            break;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_STRINGA(pszLotusPassword);

    if (pServer)
    {
        VmAfdCloseServer(pServer);
    }

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VecsCliExecEntryRequest(
    int   argc,
    char* argv[]
    )
{
    DWORD dwError = 0;
    PSTR  pszStoreName = NULL;
    PSTR  pszPassword = NULL;
    PSTR  pszKeyPassword = NULL;
    PSTR  pszAlias = NULL;
    PSTR  pszCertFilePath = NULL;
    PSTR  pszKeyFilePath = NULL;
    PSTR  pszOutputFilePath = NULL;
    PSTR  pszServerName = NULL;
    PSTR  pszUPN = NULL;
    PSTR  pszLotusPassword = NULL;
    DWORD dwFormatAsText = 0;
    DWORD idx = 0;
    DWORD dwAliasesOnly = 0;
    DWORD dwForceDelete = 0;
    VECS_COMMAND command = VECS_COMMAND_UNKNOWN;
    typedef enum
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_CREATE,
        PARSE_MODE_LIST,
        PARSE_MODE_GET_ENTRY,
        PARSE_MODE_DELETE
    } PARSE_MODE;
    typedef enum
    {
        PARSE_SUB_MODE_OPEN = 0,
        PARSE_SUB_MODE_NAME,
        PARSE_SUB_MODE_ALIAS,
        PARSE_SUB_MODE_CERT_PATH,
        PARSE_SUB_MODE_KEY_PATH,
        PARSE_SUB_MODE_SERVER,
        PARSE_SUB_MODE_UPN,
        PARSE_SUB_MODE_PASSWORD
    } PARSE_SUB_MODE;
    PARSE_MODE mode = PARSE_MODE_OPEN;
    PARSE_SUB_MODE submode = PARSE_SUB_MODE_OPEN;
    PVMAFD_SERVER pServer = NULL;

    if (!argc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    for (; idx < argc; idx++)
    {
        PSTR pszArg = argv[idx];

        switch (mode)
        {
            case PARSE_MODE_OPEN:

                if (!strcmp(pszArg, "create"))
                {
                    command = VECS_COMMAND_ENTRY_CREATE;
                    mode = PARSE_MODE_CREATE;
                }
                else if (!strcmp(pszArg, "list"))
                {
                    command = VECS_COMMAND_ENTRY_LIST;
                    mode = PARSE_MODE_LIST;
                }
                else if (!strcmp(pszArg, "delete"))
                {
                    command = VECS_COMMAND_ENTRY_DELETE;
                    mode = PARSE_MODE_DELETE;
                }
                else if (!strcmp(pszArg, "getcert"))
                {
                    command = VECS_COMMAND_ENTRY_GETCERT;
                    mode = PARSE_MODE_GET_ENTRY;
                }
                else if (!strcmp(pszArg, "getkey"))
                {
                    command = VECS_COMMAND_ENTRY_GETKEY;
                    mode = PARSE_MODE_GET_ENTRY;
                }
                else
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_CREATE:

                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:
                      if (!strcmp(pszArg, "--store"))
                      {
                          submode = PARSE_SUB_MODE_NAME;
                      }
                      else if (!strcmp(pszArg, "--alias"))
                      {
                          submode = PARSE_SUB_MODE_ALIAS;
                      }
                      else if (!strcmp(pszArg, "--cert"))
                      {
                          submode = PARSE_SUB_MODE_CERT_PATH;
                      }
                      else if (!strcmp(pszArg, "--key"))
                      {
                          submode = PARSE_SUB_MODE_KEY_PATH;
                      }
                      else if (!strcmp(pszArg, "--server"))
                      {
                          submode = PARSE_SUB_MODE_SERVER;
                      }
                      else if (!strcmp(pszArg, "--upn"))
                      {
                          submode = PARSE_SUB_MODE_UPN;
                      }
                      else if (!strcmp(pszArg, "--password"))
                      {
                          submode = PARSE_SUB_MODE_PASSWORD;
                      }
                      else
                      {
                          dwError = ERROR_INVALID_PARAMETER;
                          BAIL_ON_VMAFD_ERROR (dwError);
                      }
                      break;

                    case PARSE_SUB_MODE_NAME:
                      if (!pszStoreName)
                      {
                          pszStoreName = pszArg;
                      }

                      submode = PARSE_SUB_MODE_OPEN;
                      break;
                    case PARSE_SUB_MODE_ALIAS:
                      if (!pszAlias)
                      {
                          pszAlias = pszArg;
                      }

                      submode = PARSE_SUB_MODE_OPEN;

                      break;
                    case PARSE_SUB_MODE_CERT_PATH:
                      if (!pszCertFilePath)
                      {
                          pszCertFilePath = pszArg;
                      }

                      submode = PARSE_SUB_MODE_OPEN;
                      break;

                    case PARSE_SUB_MODE_KEY_PATH:
                      if (!pszKeyFilePath)
                      {
                          pszKeyFilePath = pszArg;
                      }

                      submode = PARSE_SUB_MODE_OPEN;
                      break;

                    case PARSE_SUB_MODE_SERVER:
                      if (!pszServerName)
                      {
                          pszServerName = pszArg;
                      }

                      submode = PARSE_SUB_MODE_OPEN;
                      break;

                    case PARSE_SUB_MODE_UPN:
                      if (!pszUPN)
                      {
                          pszUPN = pszArg;
                      }
                      submode = PARSE_SUB_MODE_OPEN;
                      break;

                    case PARSE_SUB_MODE_PASSWORD:
                      if (!pszKeyPassword)
                      {
                          pszKeyPassword = pszArg;
                      }
                      submode = PARSE_SUB_MODE_OPEN;
                      break;

                    default:
                      dwError = ERROR_INVALID_PARAMETER;
                      BAIL_ON_VMAFD_ERROR (dwError);
                      break;
                }

                break;

            case PARSE_MODE_LIST:

                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                        if (!strcmp(pszArg, "--store"))
                        {
                            submode = PARSE_SUB_MODE_NAME;
                        }
                        else if (!strcmp(pszArg, "--text"))
                        {
                            dwFormatAsText = 1;
                            submode = PARSE_SUB_MODE_OPEN;
                        }
                        else if (!strcmp(pszArg, "--aliases"))
                        {
                            dwAliasesOnly = 1;
                            submode = PARSE_SUB_MODE_OPEN;
                        }
                        else if (!strcmp(pszArg, "--server"))
                        {
                            submode = PARSE_SUB_MODE_SERVER;
                        }
                        else if (!strcmp(pszArg, "--upn"))
                        {
                            submode = PARSE_SUB_MODE_UPN;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        break;

                    case PARSE_SUB_MODE_NAME:
                        if (!pszStoreName)
                        {

                            pszStoreName = pszArg;

                        }

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_SERVER:
                        if (!pszServerName)
                        {
                            pszServerName = pszArg;
                        }

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_UPN:
                        if (!pszUPN)
                        {
                            pszUPN = pszArg;
                        }

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    default:

                        dwError = ERROR_INVALID_PARAMETER;
                        BAIL_ON_VMAFD_ERROR(dwError);

                        break;
                }

                break;

            case PARSE_MODE_GET_ENTRY:

                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                      if (!strcmp(pszArg, "--store"))
                      {
                          submode = PARSE_SUB_MODE_NAME;
                      }
                      else if(!strcmp(pszArg, "--alias"))
                      {
                          submode = PARSE_SUB_MODE_ALIAS;
                      }
                      else if(!strcmp(pszArg, "--output"))
                      {
                          submode = PARSE_SUB_MODE_CERT_PATH;
                      }
                      else if(!strcmp(pszArg, "--text"))
                      {
                          submode = PARSE_SUB_MODE_OPEN;
                          dwFormatAsText = 1;
                      }
                      else if (!strcmp(pszArg, "--server"))
                      {
                          submode = PARSE_SUB_MODE_SERVER;
                      }
                      else if (!strcmp(pszArg, "--upn"))
                      {
                          submode = PARSE_SUB_MODE_UPN;
                      }
                      else if (!strcmp(pszArg, "--password"))
                      {
                          submode = PARSE_SUB_MODE_PASSWORD;
                      }
                      else
                      {
                          dwError = ERROR_INVALID_PARAMETER;
                          BAIL_ON_VMAFD_ERROR (dwError);
                      }
                      break;

                    case PARSE_SUB_MODE_NAME:
                      if (!pszStoreName)
                      {
                          pszStoreName = pszArg;
                      }
                      submode = PARSE_SUB_MODE_OPEN;
                      break;

                    case PARSE_SUB_MODE_ALIAS:
                      if (!pszAlias)
                      {
                          pszAlias = pszArg;
                      }
                      submode = PARSE_SUB_MODE_OPEN;
                      break;

                    case PARSE_SUB_MODE_CERT_PATH:
                      if (!pszOutputFilePath)
                      {
                          pszOutputFilePath = pszArg;
                      }
                      submode = PARSE_SUB_MODE_OPEN;
                      break;

                    case PARSE_SUB_MODE_SERVER:
                      if (!pszServerName)
                      {
                          pszServerName = pszArg;
                      }
                      submode = PARSE_SUB_MODE_OPEN;
                      break;

                    case PARSE_SUB_MODE_UPN:
                      if (!pszUPN)
                      {
                          pszUPN = pszArg;
                      }

                      submode = PARSE_SUB_MODE_OPEN;

                      break;

                    case PARSE_SUB_MODE_PASSWORD:
                      if (!pszKeyPassword)
                      {
                          pszKeyPassword = pszArg;
                      }
                      submode = PARSE_SUB_MODE_OPEN;
                      break;

                    default:
                      dwError = ERROR_INVALID_PARAMETER;
                      BAIL_ON_VMAFD_ERROR(dwError);

                }

                break;


            case PARSE_MODE_DELETE:

                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                      if (!strcmp(pszArg, "--store"))
                      {
                          submode = PARSE_SUB_MODE_NAME;
                      }
                      else if (!strcmp(pszArg, "--alias"))
                      {
                          submode = PARSE_SUB_MODE_ALIAS;
                      }
                      else if (!strcmp(pszArg, "-y"))
                      {
                          dwForceDelete = 1;
                          submode = PARSE_SUB_MODE_OPEN;
                      }
                      else if (!strcmp(pszArg, "--server"))
                      {
                          submode = PARSE_SUB_MODE_SERVER;
                      }
                      else if (!strcmp(pszArg, "--upn"))
                      {
                          submode = PARSE_SUB_MODE_UPN;
                      }
                      else
                      {
                          dwError = ERROR_INVALID_PARAMETER;
                          BAIL_ON_VMAFD_ERROR (dwError);
                      }
                      break;

                    case PARSE_SUB_MODE_NAME:
                      if (!pszStoreName)
                      {
                          pszStoreName = pszArg;
                      }
                      submode = PARSE_SUB_MODE_OPEN;

                      break;
                    case PARSE_SUB_MODE_ALIAS:
                      if (!pszAlias)
                      {
                          pszAlias = pszArg;
                      }
                      submode = PARSE_SUB_MODE_OPEN;
                      break;

                    case PARSE_SUB_MODE_SERVER:
                      if (!pszServerName)
                      {
                          pszServerName = pszArg;
                      }

                      submode = PARSE_SUB_MODE_OPEN;

                      break;
                    case PARSE_SUB_MODE_UPN:
                      if (!pszUPN)
                      {
                          pszUPN = pszArg;
                      }

                      submode = PARSE_SUB_MODE_OPEN;

                      break;
                    default:

                      dwError = ERROR_INVALID_PARAMETER;
                      BAIL_ON_VMAFD_ERROR (dwError);

                      break;
                }
                break;
        }
    }

    if (submode != PARSE_SUB_MODE_OPEN)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (IsNullOrEmptyString(pszServerName) ^
        IsNullOrEmptyString(pszUPN)
       )
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VECS_CLI_ERROR (
                        dwError,
                        "Error: You have to provide both server and upn \n"
                        );
    }

    if (!IsNullOrEmptyString(pszServerName))
    {
        fprintf (
            stdout,
            "Enter password:\t"
            );

        dwError = GetPassword(&pszLotusPassword);

        BAIL_ON_VECS_CLI_ERROR(
                          dwError,
                          "Failed to get password from user \n"
                          );

        fprintf (stdout, "\n");
    }

    dwError = VmAfdOpenServerA(
                  pszServerName,
                  pszUPN,
                  pszLotusPassword,
                  &pServer);
    BAIL_ON_VECS_CLI_ERROR (
                            dwError,
                            "Failed to establish connection to remote server \n"
                           );

    switch (command)
    {
        case VECS_COMMAND_ENTRY_CREATE:

            dwError = VecsCliAddEntryA(
                              pServer,
                              pszStoreName,
                              pszPassword,
                              pszAlias,
                              pszCertFilePath,
                              pszKeyFilePath,
                              pszKeyPassword
                              );
            break;

        case VECS_COMMAND_ENTRY_LIST:

            dwError = VecsCliListEntriesA(
                              pServer,
                              pszStoreName,
                              pszPassword,
                              dwFormatAsText,
                              dwAliasesOnly
                              );

            break;

        case VECS_COMMAND_ENTRY_GETCERT:
            dwError = VecsCliGetCertificateA(
                              pServer,
                              pszStoreName,
                              pszPassword,
                              pszAlias,
                              pszOutputFilePath,
                              dwFormatAsText
                              );
            break;

        case VECS_COMMAND_ENTRY_GETKEY:
            dwError = VecsCliGetKeyA(
                              pServer,
                              pszStoreName,
                              pszPassword,
                              pszAlias,
                              pszOutputFilePath,
                              dwFormatAsText,
                              pszKeyPassword
                              );
            break;

        case VECS_COMMAND_ENTRY_DELETE:

            if (!dwForceDelete)
            {
                char input = 0;
                fprintf (stdout,
                         "Warning: This operation will delete entry [%s] from store [%s]\n"
                         "Do you wish to continue? Y/N [N] \n",
                         pszAlias,
                         pszStoreName
                        );
                scanf (
                        "%c",
                        &input
                        );

                if (input == 'Y' || input == 'y')
                {
                    dwForceDelete = 1;
                }
            }

            if (dwForceDelete)
            {

                dwError = VecsCliDeleteEntryA(
                              pServer,
                              pszStoreName,
                              pszPassword,
                              pszAlias
                              );
            }

            break;

        default:

            dwError = ERROR_INVALID_STATE;

            break;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_STRINGA(pszLotusPassword);

    if (pServer)
    {
        VmAfdCloseServer(pServer);
    }

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VecsCliTriggerRefreshThread(
    int   argc,
    char* argv[]
    )
{
    DWORD dwError = 0;
    PSTR  pszServerName = NULL;
    PSTR  pszUPN = NULL;
    PSTR  pszLotusPassword = NULL;
    DWORD idx = 0;

    typedef enum
    {
        PARSE_SUB_MODE_OPEN = 0,
        PARSE_SUB_MODE_SERVER,
        PARSE_SUB_MODE_UPN
    } PARSE_SUB_MODE;
    PARSE_SUB_MODE submode = PARSE_SUB_MODE_OPEN;

    for (; idx < argc; idx++)
    {
        PSTR pszArg = argv[idx];

        switch (submode)
        {
            case PARSE_SUB_MODE_OPEN:

              if (!strcmp(pszArg, "--server"))
              {
                  submode = PARSE_SUB_MODE_SERVER;
              }
              else if (!strcmp(pszArg, "--upn"))
              {
                  submode = PARSE_SUB_MODE_UPN;
              }
              else
              {
                  dwError = ERROR_INVALID_PARAMETER;
                  BAIL_ON_VMAFD_ERROR (dwError);
              }
              break;

            case PARSE_SUB_MODE_SERVER:
              if (!pszServerName)
              {
                  pszServerName = pszArg;
              }

              submode = PARSE_SUB_MODE_OPEN;

              break;
            case PARSE_SUB_MODE_UPN:
              if (!pszUPN)
              {
                  pszUPN = pszArg;
              }

              submode = PARSE_SUB_MODE_OPEN;

              break;
            default:

              dwError = ERROR_INVALID_PARAMETER;
              BAIL_ON_VMAFD_ERROR (dwError);

              break;
        }
    }

    if (submode != PARSE_SUB_MODE_OPEN)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (IsNullOrEmptyString(pszServerName) ^
        IsNullOrEmptyString(pszUPN)
       )
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VECS_CLI_ERROR (
                        dwError,
                        "Error: You have to provide both server and upn \n"
                        );
    }

    if (!IsNullOrEmptyString(pszServerName))
    {
        fprintf (
            stdout,
            "Enter password:\t"
            );

        dwError = GetPassword(&pszLotusPassword);

        BAIL_ON_VECS_CLI_ERROR(
                          dwError,
                          "Failed to get password from user \n"
                          );

        fprintf (stdout, "\n");
    }

    dwError = VmAfdTriggerRootCertsRefresh(
                pszServerName,
                pszUPN,
                pszLotusPassword);
    BAIL_ON_VECS_CLI_ERROR (
                            dwError,
                            "Failed to trigger root cert refresh \n"
                           );

cleanup:
    VMAFD_SAFE_FREE_STRINGA(pszLotusPassword);
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
        "Usage: vecs-cli { arguments }\n\n"
        "Arguments:\n\n"
        "\tstore create --name <name> \n"
        "\t            [--server <server-name>]\n"
        "\t            [--upn <user-name>]\n"
        "\tstore list\n"
        "\t            [--server <server-name>]\n"
        "\t            [--upn <user-name>]\n"
        "\tstore delete --name <name> \n"
        "\t            [--server <server-name>]\n"
        "\t            [--upn <user-name>]\n"
        "\t            [-y]\n"
        "\tstore permission --name <name>\n"
        "\t                 --user <username>\n"
        "\t                 --grant|--revoke  read|write\n"
        "\tstore get-permissions --name <name>\n"
        "\t            [--server <server-name>]\n"
        "\t            [--upn <user-name>]\n"
        "\tentry create --store <store-name>\n"
        "\t             --alias <alias>\n"
        "\t             --cert <file-path>\n"
        "\t            [--key  <file-path>]\n"
        "\t            [--password  <password>]\n"
        "\t            [--text]\n"
        "\t            [--server <server-name>]\n"
        "\t            [--upn <user-name>]\n"
        "\tentry list   --store <store-name>\n"
        "\tentry getcert --store <store-name>\n"
        "\t              --alias <alias>\n"
        "\t            [--output <output-file-path>]\n"
        "\t            [--text]\n"
        "\t            [--server <server-name>]\n"
        "\t            [--upn <user-name>]\n"
        "\tentry getkey --store <store-name>\n"
        "\t             --alias <alias>\n"
        "\t            [--password  <password>]\n"
        "\t            [--output <output-file-path>]\n"
        "\t            [--text]\n"
        "\t            [--server <server-name>]\n"
        "\t            [--upn <user-name>]\n"
        "\tentry delete --store <store-name> --alias <alias>\n"
        "\t            [--server <server-name>]\n"
        "\t            [--upn <user-name>]\n"
        "\t            [-y]\n"
        "\tforce-refresh\n"
        "\t            [--server <server-name>]\n"
        "\t            [--upn <user-name>]\n"
        "\thelp\n");
}


#ifndef _WIN32
static
DWORD
GetPassword(
    PSTR *ppszPassword
    )
{
    CHAR pszPasswordBuff[100] = {0};
    PSTR pszPassword = NULL;
    DWORD dwError = 0;
    struct termios tp, save;

    fflush(stdout);

    tcgetattr(0, &tp) ;
    memcpy (&save, &tp, sizeof (struct termios));
    save.c_lflag &= ~ECHO;                /* ECHO off, other bits unchanged */
    tcsetattr(0, TCSANOW, &save);

    if (!fgets(pszPasswordBuff, 100, stdin) && ferror(stdin))
    {
        dwError = LwErrnoToWin32Error(ferror(stdin));
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (pszPasswordBuff[strlen(pszPasswordBuff)-1] == '\n')
    {
        pszPasswordBuff[strlen(pszPasswordBuff)-1] = '\0';
    }

    dwError = VmAfdAllocateStringPrintf(
                                        &pszPassword,
                                        "%s",
                                        pszPasswordBuff
                                       );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppszPassword = pszPassword;

cleanup:

    tcsetattr(0, TCSANOW, &tp);

    fflush (stdin);

    return dwError;

error:
    if (ppszPassword)
    {
        *ppszPassword = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY (pszPassword);

    goto cleanup;
}

#else
static
DWORD
GetPassword(
    PSTR *ppszPassword
    )
{
    CHAR pszPasswordBuff[100] = {0};
    PSTR pszPassword = NULL;
    DWORD dwError = 0;
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    fflush(stdout);
    if (!GetConsoleMode(hStdin, &mode))
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    if (!SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT)))
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    if (!fgets(pszPasswordBuff, 100, stdin) && ferror(stdin))
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (pszPasswordBuff[strlen(pszPasswordBuff)-1] == '\n')
    {
        pszPasswordBuff[strlen(pszPasswordBuff)-1] = '\0';
    }

    dwError = VmAfdAllocateStringPrintf(
                                        &pszPassword,
                                        "%s",
                                        pszPasswordBuff
                                       );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppszPassword = pszPassword;

cleanup:

    if (!SetConsoleMode(hStdin, mode))
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    fflush (stdin);

    return dwError;

error:
    if (ppszPassword)
    {
        *ppszPassword = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY (pszPassword);

    goto cleanup;
}
#endif


static
void
PrintError(
    DWORD dwError
    )
{
    PSTR pszErrorMsg = NULL;

    if (!VmAfdGetErrorString(dwError, &pszErrorMsg))
    {
        fprintf(stderr, "vecs-cli failed. Error %u: %s \n", dwError, pszErrorMsg);
    }
    else
    {
        fprintf(stderr, "vecs-cli failed with error %u\n", dwError);
    }

    VMAFD_SAFE_FREE_STRINGA(pszErrorMsg);
}
