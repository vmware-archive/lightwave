/*
 * Copyright (C) 2015 VMware, Inc. All rights reserved.
 *
 * Module   : main.c
 *
 * Abstract :
 *
 */
#include "includes.h"

static
DWORD
ProcessArgs(
    int   argc,
    char* argv[]
    );

static
DWORD
ProcessJoin(
    int   argc,
    char* argv[]
    );

static
DWORD
ReadPassword(
    PSTR* ppszPassword
    );

static
DWORD
ProcessLeave(
    int   argc,
    char* argv[]
    );

static
DWORD
ProcessInfo(
    int   argc,
    char* argv[]
    );

static
void
ShowUsage(
    VOID
    );

#ifndef _WIN32
int  main(int argc, char* argv[])
#else
int _tmain(int argc, _TCHAR* targv[])
#endif
{
    DWORD dwError = 0;
    int retCode = 0;
    PCSTR pszErrorMsg = NULL;
    PSTR  pszErrorDesc = NULL;

#ifdef _WIN32

    char** allocArgv = NULL;
    PSTR* argv = NULL;

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

    dwError = ProcessArgs(argc, argv);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_STRINGA(pszErrorDesc);
    return dwError;

error:

    switch (dwError)
    {
        case ERROR_CANNOT_CONNECT_VMAFD:
            retCode = 20;
            pszErrorMsg = "Could not connect to the local service VMware AFD.\nVerify VMware AFD is running.";
            break;
        case VMDIR_ERROR_CANNOT_CONNECT_VMDIR:
            retCode = 21;
            pszErrorMsg = "Could not connect to the local service VMware Directory Service.\nVerify VMware Directory Service is running.";
            break;
        case ERROR_INVALID_CONFIGURATION:
            retCode = 22;
            pszErrorMsg = "Configuration is not correct.\nFirst boot scripts need to be executed.";
            break;
        case VMDIR_ERROR_SERVER_DOWN:
            retCode = 23;
            pszErrorMsg = "Could not connect to VMware Directory Service via LDAP.\nVerify VMware Directory Service is running on the appropriate system and is reachable from this host.";
            break;
        case VMDIR_ERROR_USER_INVALID_CREDENTIAL:
            retCode = 24;
            pszErrorMsg = "Authentication to VMware Directory Service failed.\nVerify the username and password.";
            break;
        case ERROR_ACCESS_DENIED:
            retCode = 25;
            pszErrorMsg = "Authorization failed.\nVerify account has proper administrative privileges.";
            break;
        case ERROR_INVALID_DOMAINNAME:
            retCode = 26;
            pszErrorMsg = "Failed to join the domain.\nThe domain name specified is invalid.";
            break;
        case ERROR_NO_SUCH_DOMAIN:
            retCode = 27;
            pszErrorMsg = "Failed to join the domain.\nA domain controller for the domain could not be located. Verify the DNS settings pertaining to this domain name.";
            break;
        case ERROR_PASSWORD_RESTRICTION:
            retCode = 28;
            pszErrorMsg = "Failed to join the domain.\nA required password was not specified or did not match complexity requirements.";
            break;
        case ERROR_HOST_DOWN:
            retCode = 29;
            pszErrorMsg = "Failed to join the domain.\nThe required service on the domain controller is unreachable.";
            break;
        default:
            retCode = 1;
    }

    if (retCode != 1)
    {
        fprintf(
            stderr,
            "domain-join failed, error= %s %u\n",
            pszErrorMsg,
            dwError);
    }
    else
    {
        if (!VmAfdGetErrorString(dwError, &pszErrorDesc))
        {
            fprintf(stderr, "domain-join failed. Error %u: %s \n", dwError, pszErrorDesc);
        }
        else
        {
            fprintf(stderr, "domain-join failed with error: %u\n", dwError);
        }
    }

    goto cleanup;
}

static
DWORD
ProcessArgs(
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

    if (!VmAfdStringCompareA(pszArg, "help", TRUE))
    {
        ShowUsage();
    }
    else if (!VmAfdStringCompareA(pszArg, "join", TRUE))
    {
        dwError = ProcessJoin(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL);
    }
    else if (!VmAfdStringCompareA(pszArg, "leave", TRUE))
    {
        dwError = ProcessLeave(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL);
    }
    else if (!VmAfdStringCompareA(pszArg, "info", TRUE))
    {
        dwError = ProcessInfo(
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

    if (dwError == ERROR_INVALID_PARAMETER ||
        dwError == ERROR_INVALID_COMMAND_LINE)
    {
        ShowUsage();
    }

    goto cleanup;
}

static
DWORD
ProcessJoin(
    int   argc,
    char* argv[]
    )
{
    typedef enum
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_ACCOUNT,
        PARSE_MODE_PASSWORD,
        PARSE_MODE_ORGUNIT
    } PARSE_MODE;

    DWORD dwError = 0;
    DWORD idx = 0;
    PSTR pszLogin = NULL;
    PSTR pszPassword = NULL;
    PSTR pszPasswordNew = NULL;
    PSTR pszDomain = NULL;
    PSTR pszOrgUnit = NULL;
    PARSE_MODE mode = PARSE_MODE_OPEN;

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

                if (!VmAfdStringCompareA(pszArg, "--username", TRUE))
                {
                    mode = PARSE_MODE_ACCOUNT;
                }
                else if (!VmAfdStringCompareA(pszArg, "--password", TRUE))
                {
                    mode = PARSE_MODE_PASSWORD;
                }
                else if (!VmAfdStringCompareA(pszArg, "--orgunit", TRUE))
                {
                    mode = PARSE_MODE_ORGUNIT;
                }
                else
                {
                    if (pszDomain)
                    {
                        dwError = ERROR_INVALID_COMMAND_LINE;
                        BAIL_ON_VMAFD_ERROR(dwError);
                    }

                    pszDomain = pszArg;
                }
                break;

            case PARSE_MODE_ACCOUNT:

                pszLogin = pszArg;

                mode = PARSE_MODE_OPEN;

                break;

            case PARSE_MODE_PASSWORD:

                pszPassword = pszArg;

                mode = PARSE_MODE_OPEN;

                break;

            case PARSE_MODE_ORGUNIT:

                pszOrgUnit = pszArg;

                mode = PARSE_MODE_OPEN;

                break;

            default:

                dwError = ERROR_INVALID_STATE;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

    if (!pszPassword)
    {
        dwError = ReadPassword(&pszPasswordNew);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszPassword = pszPasswordNew;
    }

    if (!pszDomain)
    {
        dwError = ERROR_NO_SUCH_DOMAIN;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!pszLogin)
    {
        pszLogin = "Administrator";
    }
    else if (strchr(pszLogin, (int)'@') != NULL)
    {
        fprintf(stderr, "Error: Username may not include domain\n");
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdJoinDomain(pszDomain, pszLogin, pszPassword, pszOrgUnit);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszPasswordNew);

    return dwError;

error:

    goto cleanup;
}

#ifndef _WIN32
static
DWORD
ReadPassword(
    PSTR* ppszPassword
    )
{
    DWORD dwError = ERROR_SUCCESS;
    struct termios orig, nonecho;
    sigset_t sig, osig;
    CHAR  szPassword[33] = "";
    PSTR  pszPassword = NULL;

    fprintf(stdout, "Password: ");
    fflush(stdout);

    sigemptyset(&sig);
    sigaddset(&sig,SIGINT);
    sigaddset(&sig,SIGTSTP);
    sigprocmask(SIG_BLOCK,&sig,&osig);

    tcgetattr(0, &orig); // get current settings
    memcpy(&nonecho, &orig, sizeof(struct termios)); // copy settings
    nonecho.c_lflag &= ~(ECHO); // don't echo password characters
    tcsetattr(0, TCSANOW, &nonecho); // set current settings to not echo

    // Read up to 32 characters of password

    if (fgets(szPassword,32,stdin) == NULL)
    {
        szPassword[0] = '\0';
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    szPassword[strcspn(szPassword, "\n")] = '\0';

    if (IsNullOrEmptyString(szPassword))
    {
        dwError = ERROR_PASSWORD_RESTRICTION;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringA(szPassword, &pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszPassword = pszPassword;

cleanup:

    tcsetattr(0, TCSANOW, &orig);
    sigprocmask(SIG_SETMASK,&osig,NULL);

    return dwError;

error:

    *ppszPassword = NULL;

    goto cleanup;
}
#else
static
DWORD
ReadPassword(
    PSTR* ppszPassword
    )
{
    return ERROR_PASSWORD_RESTRICTION;
}
#endif

static
DWORD
ProcessLeave(
    int   argc,
    char* argv[]
    )
{
    typedef enum
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_ACCOUNT,
        PARSE_MODE_PASSWORD
    } PARSE_MODE;

    DWORD dwError = 0;
    DWORD idx = 0;
    DWORD dwLeaveFlags = 0;
    PSTR pszLogin = NULL;
    PSTR pszPassword = NULL;
    PSTR pszPasswordNew = NULL;
    PARSE_MODE mode = PARSE_MODE_OPEN;

    for (; idx < argc; idx++)
    {
        PSTR pszArg = argv[idx];

        switch (mode)
        {
            case PARSE_MODE_OPEN:

                if (!VmAfdStringCompareA(pszArg, "--username", TRUE))
                {
                    mode = PARSE_MODE_ACCOUNT;
                }
                else if (!VmAfdStringCompareA(pszArg, "--password", TRUE))
                {
                    mode = PARSE_MODE_PASSWORD;
                }
                else if (!VmAfdStringCompareA(pszArg, "--force", TRUE))
                {
                    dwLeaveFlags = dwLeaveFlags | VMAFD_DOMAIN_LEAVE_FLAGS_FORCE;
                    mode = PARSE_MODE_OPEN;
                }
                else
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_ACCOUNT:

                pszLogin = pszArg;

                mode = PARSE_MODE_OPEN;

                break;

            case PARSE_MODE_PASSWORD:

                pszPassword = pszArg;

                mode = PARSE_MODE_OPEN;

                break;

            default:

                dwError = ERROR_INVALID_STATE;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

    if (pszLogin && !pszPassword)
    {
        dwError = ReadPassword(&pszPasswordNew);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszPassword = pszPasswordNew;
    }

    dwError = VmAfdLeaveDomain( pszLogin, pszPassword, dwLeaveFlags );
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszPasswordNew);

    return dwError;

error:

    goto cleanup;
}

static
DWORD
ProcessInfo(
    int   argc,
    char* argv[]
    )
{
    DWORD dwError = 0;
    PSTR pszDomain = NULL;
    BOOLEAN bIsDC = FALSE;

    dwError = VmAfdGetJoinStatus(&pszDomain, &bIsDC);
    if (dwError == ERROR_NOT_JOINED)
    {
        dwError = ERROR_SUCCESS;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    printf("Domain : %s\n", VMAFD_SAFE_STRING(pszDomain));
    if (bIsDC)
    {
        printf("Domain controller : TRUE\n");
    }

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszDomain);

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
        "Usage: domain-join { arguments }\n\n"
        "Arguments:\n\n"
        "\tjoin <domain-name>\n"
        "\t     [ --orgunit <organizational unit> ]\n"
        "\t     [ --username <account name> ]\n"
        "\t     [ --password <password> ]\n"
        "\tleave\n"
        "\t     [ --username <account name> ]\n"
        "\t     [ --password <password> ]\n"
        "\t     [ --force ]\n"
        "\tinfo\n"
        "\thelp\n");
}
