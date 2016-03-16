/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : main.c
 *
 * Abstract :
 *
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
CdcCliExecCacheRequest(
    int   argc,
    char* argv[]
    );

static
DWORD
CdcCliExecClientAffinity(
    int   argc,
    char* argv[]
    );

static
DWORD
CdcCliExecGetDCName(
    int   argc,
    char* argv[]
    );

static
DWORD
CdcCliExecGetDCStatus(
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
DWORD
OpenServer(
    PSTR pszServerName,
    PSTR pszUPN,
    PVMAFD_SERVER *pServer
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
    else if (!strcmp(pszArg, "cache"))
    {
        dwError = CdcCliExecCacheRequest(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL);
    }
    else if (!strcmp(pszArg, "client-affinity"))
    {
        dwError = CdcCliExecClientAffinity(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL);
    }
    else if (!strcmp(pszArg, "get-affinitized-psc"))
    {
        dwError = CdcCliExecGetDCName(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL);
    }
    else if (!strcmp(pszArg, "get-psc-status"))
    {
        dwError = CdcCliExecGetDCStatus(
                        dwArgsLeft,
                        dwArgsLeft > 0? &argv[iArg] : NULL);
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
CdcCliExecCacheRequest(
    int   argc,
    char* argv[]
    )
{
    DWORD dwError = 0;
    PSTR  pszServerName = NULL;
    PSTR  pszUPN = NULL;
    DWORD idx = 0;
    CDC_COMMAND command = CDC_COMMAND_UNKNOWN;
    typedef enum
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_LIST,
        PARSE_MODE_REFRESH
    } PARSE_MODE;
    typedef enum
    {
        PARSE_SUB_MODE_OPEN = 0,
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
                if (!strcmp(pszArg, "list"))
                {
                    command = CDC_COMMAND_CACHE_LIST;
                    mode = PARSE_MODE_LIST;
                }
                else if (!strcmp(pszArg, "refresh"))
                {
                    command = CDC_COMMAND_CACHE_REFRESH;
                    mode = PARSE_MODE_REFRESH;
                }
                else
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_LIST:
            case PARSE_MODE_REFRESH:
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
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
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
        }
    }

    dwError = OpenServer(pszServerName, pszUPN, &pServer);
    BAIL_ON_VMAFD_ERROR(dwError);

    switch (command)
    {
        case CDC_COMMAND_CACHE_LIST:

            dwError = CdcCliDcCacheList(pServer);

            break;

        case CDC_COMMAND_CACHE_REFRESH:

            dwError = CdcCliDcCacheRefresh(pServer);

            break;

        default:

            dwError = ERROR_INVALID_STATE;

            break;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
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
CdcCliExecClientAffinity(
    int   argc,
    char* argv[]
    )
{
    DWORD dwError = 0;
    PSTR  pszServerName = NULL;
    PSTR  pszUPN = NULL;
    DWORD idx = 0;
    CDC_COMMAND command = CDC_COMMAND_UNKNOWN;
    typedef enum
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_ENABLE,
        PARSE_MODE_DISABLE,
        PARSE_MODE_GET_STATE
    } PARSE_MODE;
    typedef enum
    {
        PARSE_SUB_MODE_OPEN = 0,
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

                if (!strcmp(pszArg, "enable"))
                {
                    command = CDC_COMMAND_ENABLE_DEFAULT_HA;
                    mode = PARSE_MODE_ENABLE;
                }
                else if (!strcmp(pszArg, "disable"))
                {
                    command = CDC_COMMAND_ENABLE_LEGACY_HA;
                    mode = PARSE_MODE_DISABLE;
                }
                else if (!strcmp(pszArg, "default"))
                {
                    command = CDC_COMMAND_ENABLE_DEFAULT_HA;
                    mode = PARSE_MODE_ENABLE;
                }
                else if (!strcmp(pszArg, "legacy"))
                {
                    command = CDC_COMMAND_ENABLE_LEGACY_HA;
                    mode = PARSE_MODE_DISABLE;
                }
                else if (!strcmp(pszArg, "state"))
                {
                    command = CDC_COMMAND_GETSTATE_CLIENT_AFFINITY;
                    mode = PARSE_MODE_GET_STATE;
                }
                else
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_ENABLE:
            case PARSE_MODE_DISABLE:
            case PARSE_MODE_GET_STATE:

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

                break;
        }
    }

    if (submode != PARSE_SUB_MODE_OPEN)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = OpenServer(pszServerName, pszUPN, &pServer);
    BAIL_ON_VMAFD_ERROR(dwError);

    switch (command)
    {
        case CDC_COMMAND_ENABLE_DEFAULT_HA:

            dwError = CdcCliEnableDefaultHA(
                              pServer
                              );
            break;

        case CDC_COMMAND_ENABLE_LEGACY_HA:

            dwError = CdcCliEnableLegacyHA(
                              pServer
                              );

            break;

        case CDC_COMMAND_GETSTATE_CLIENT_AFFINITY:

            dwError = CdcCliGetStateofClientAffinity(
                              pServer
                              );
            break;

        default:

            dwError = ERROR_INVALID_STATE;

            break;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

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
CdcCliExecGetDCName(
    int   argc,
    char* argv[]
    )
{
    DWORD dwError = 0;
    PSTR  pszServerName = NULL;
    PSTR  pszUPN = NULL;
    PVMAFD_SERVER pServer = NULL;
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

    dwError = OpenServer(pszServerName, pszUPN, &pServer);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcCliGetDCName(
                pServer
                );
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
CdcCliExecGetDCStatus(
    int   argc,
    char* argv[]
    )
{
    DWORD dwError = 0;
    PSTR  pszServerName = NULL;
    PSTR  pszUPN = NULL;
    PSTR  pszPSC = NULL;
    PVMAFD_SERVER pServer = NULL;
    DWORD idx = 0;

    typedef enum
    {
        PARSE_SUB_MODE_OPEN = 0,
        PARSE_SUB_MODE_SERVER,
        PARSE_SUB_MODE_UPN,
        PARSE_SUB_MODE_PSC
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
              else if (!strcmp(pszArg, "--psc"))
              {
                  submode = PARSE_SUB_MODE_PSC;
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
            case PARSE_SUB_MODE_PSC:
              if (!pszPSC)
              {
                  pszPSC = pszArg;
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

    dwError = OpenServer(pszServerName, pszUPN, &pServer);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcCliGetDCStatus(
                pServer,
                pszPSC
                );
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if (pServer)
    {
        VmAfdCloseServer(pServer);
    }
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
        "Usage: cdc-cli { arguments }\n\n"
        "Arguments:\n\n"
        "\tclient-affinity default\n"
        "\t            [--server <server-name>]\n"
        "\t            [--upn <user-name>]\n"
        "\tclient-affinity legacy\n"
        "\t            [--server <server-name>]\n"
        "\t            [--upn <user-name>]\n"
        "\tclient-affinity state\n"
        "\t            [--server <server-name>]\n"
        "\t            [--upn <user-name>]\n"
        "\tget-affinitized-psc \n"
        "\t            [--server <server-name>]\n"
        "\t            [--upn <user-name>]\n"
        "\tget-psc-status \n"
        "\t            --psc <psc-name>\n"
        "\t            [--server <server-name>]\n"
        "\t            [--upn <user-name>]\n"
        "\tcache list\n"
        "\t            [--server <server-name>]\n"
        "\t            [--upn <user-name>]\n"
        "\tcache refresh\n"
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
DWORD
OpenServer(
    PSTR pszServerName,
    PSTR pszUPN,
    PVMAFD_SERVER *ppServer
    )
{
    DWORD dwError = 0;
    PVMAFD_SERVER pServer = NULL;
    PSTR pszLotusPassword = NULL;

    if (!ppServer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (IsNullOrEmptyString(pszServerName) ^
        IsNullOrEmptyString(pszUPN)
       )
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_CDC_CLI_ERROR (
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

        BAIL_ON_CDC_CLI_ERROR(
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
    BAIL_ON_CDC_CLI_ERROR (
                            dwError,
                            "Failed to establish connection to remote server \n"
                           );

    *ppServer = pServer;

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszLotusPassword);

    return dwError;

error:
    if (ppServer)
    {
        *ppServer = NULL;
    }
    if (pServer)
    {
        VmAfdCloseServer(pServer);
    }

    goto cleanup;
}

static
void
PrintError(
    DWORD dwError
    )
{
    PSTR pszErrorMsg = NULL;

    if (!VmAfdGetErrorString(dwError, &pszErrorMsg))
    {
        fprintf(stderr, "cdc-cli failed. Error %u: %s \n", dwError, pszErrorMsg);
    }
    else
    {
        fprintf(stderr, "cdc-cli failed with error %u\n", dwError);
    }

    VMAFD_SAFE_FREE_STRINGA(pszErrorMsg);
}
