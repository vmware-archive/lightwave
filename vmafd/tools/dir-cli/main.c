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



#include "includes.h"

static
DWORD
ParseArgs(
    int   argc,
    char* argv[]
    );

static
DWORD
DirCliExecServicePrincipalRequest(
    int   argc,
    char* argv[]
    );

static
DWORD
DirCliExecCertificateRequest(
    int   argc,
    char* argv[]
    );

static
DWORD
DirCliExecGroupPrincipalRequest(
    int   argc,
    char* argv[]
    );

static
DWORD
DirCliExecUserRequest(
    int   argc,
    char* argv[]
    );

static
DWORD
DirCliExecGroupRequest(
    int   argc,
    char* argv[]
    );

static
DWORD
DirCliExecPasswordRequest(
    int   argc,
    char* argv[]
    );

static
DWORD
DirCliExecFuncLvlRequest(
    int   argc,
    char* argv[]
    );

DWORD
DirCliExecNodesVersionRequest(
    int   argc,
    char* argv[]
    );

static
DWORD
DirCliExecTopologyRequest(
    int   argc,
    char* argv[]
    );

static
DWORD
DirCliExecMachineAccountReset(
    int   argc,
    char* argv[]
    );

static
DWORD
DirCliExecStateRequest(
    int   argc,
    char* argv[]
    );

static
DWORD
DirCliExecTenantRequest(
    int   argc,
    char* argv[]
    );

static
DWORD
DirCliExecOrgunitRequest(
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

    dwError = VmAfCfgInit();
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = ParseArgs(argc, argv);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_STRINGA(pszErrorDesc);
    return dwError;

error:

    switch (dwError)
    {
        case ERROR_LOCAL_OPTION_UNKNOWN:
            retCode = 2;
            pszErrorMsg = "An unknown option was present on the command line.";
            break;
        case ERROR_LOCAL_OPTION_INVALID:
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
        case VMDIR_ERROR_NO_FUNC_LVL:
            retCode = 26;
            pszErrorMsg = "Domain Functional Level was not found\n";
            break;
        case VMDIR_ERROR_INVALID_FUNC_LVL:
            retCode = 27;
            pszErrorMsg = "Invalid Domain Functional Level\n"
                "Verify that level is valid for domain.";
            break;
        case VMDIR_ERROR_INCOMPLETE_MAX_DFL:
            retCode = 28;
            pszErrorMsg = "Maximum Domain Functional Level could not be determined\n"
                "Verify that all nodes in the domain are online and reachable.";
            break;
        default:
            retCode = 1;
    }

    if (retCode != 1)
    {
        fprintf(
            stderr,
            "dir-cli failed, error= %s %u\n",
            pszErrorMsg,
            dwError);
    }
    else
    {
        if (!VmAfdGetErrorString(dwError, &pszErrorDesc))
        {
            fprintf(stderr, "dir-cli failed. Error %u: %s \n", dwError, pszErrorDesc);
        }
        else
        {
            fprintf(stderr, "dir-cli failed with error: %u\n", dwError);
        }
    }

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
        dwError = ERROR_LOCAL_OPTION_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pszArg = argv[iArg++];
    dwArgsLeft--;

    if (!VmAfdStringCompareA(pszArg, "help", TRUE))
    {
        ShowUsage();
    }
    else if (!VmAfdStringCompareA(pszArg, "service", TRUE))
    {
        dwError = DirCliExecServicePrincipalRequest(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL);
    }
    else if (!VmAfdStringCompareA(pszArg, "user", TRUE))
    {
        dwError = DirCliExecUserRequest(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL);
    }
    else if (!VmAfdStringCompareA(pszArg, "group", TRUE))
    {
        dwError = DirCliExecGroupRequest(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL);
    }
    else if (!VmAfdStringCompareA(pszArg, "trustedcert", TRUE))
    {
        dwError = DirCliExecCertificateRequest(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL);
    }
    else if (!VmAfdStringCompareA(pszArg, "ssogroup", TRUE))
    {
        dwError = DirCliExecGroupPrincipalRequest(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL);
    }
    else if (!VmAfdStringCompareA(pszArg, "password", TRUE))
    {
        dwError = DirCliExecPasswordRequest(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL);
    }
    else if (!VmAfdStringCompareA(pszArg, "domain-functional-level", TRUE))
    {
        dwError = DirCliExecFuncLvlRequest(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL);
    }
    else if (!VmAfdStringCompareA(pszArg, "list-domain-versions", TRUE))
    {
        dwError = DirCliExecNodesVersionRequest(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL);
    }
    else if (!VmAfdStringCompareA(pszArg, "nodes", TRUE))
    {
        dwError = DirCliExecTopologyRequest(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL);
    }
    else if (!VmAfdStringCompareA(pszArg, "computer", TRUE))
    {
        dwError = DirCliExecMachineAccountReset(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL);
    }
    else if (!VmAfdStringCompareA(pszArg, "state", TRUE))
    {
        dwError = DirCliExecStateRequest(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL);
    }
    else if (!VmAfdStringCompareA(pszArg, "tenant", TRUE))
    {
        dwError = DirCliExecTenantRequest(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL);
    }
    else if (!VmAfdStringCompareA(pszArg, "orgunit", TRUE))
    {
        dwError = DirCliExecOrgunitRequest(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL);
    }
    else
    {
        dwError = ERROR_LOCAL_OPTION_UNKNOWN;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    if (dwError == ERROR_INVALID_PARAMETER ||
        dwError == ERROR_LOCAL_OPTION_UNKNOWN ||
        dwError == ERROR_LOCAL_OPTION_INVALID)
    {
        ShowUsage();
    }

    goto cleanup;
}

static
DWORD
DirCliExecServicePrincipalRequest(
    int   argc,
    char* argv[]
    )
{
    DWORD dwError = 0;
    DWORD idx = 0;

    PSTR pszServiceName = NULL;
    PSTR pszCertPath = NULL;
    PSTR pszLogin = NULL;
    PSTR pszPassword = NULL;
    PSTR pszSsoGroups = NULL;
    SSO_ADMIN_ROLE ssoAdminRole = SSO_ROLE_UNKNOWN;
    BOOL bTrustedUserGroup = FALSE;

    typedef enum
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_CREATE,
        PARSE_MODE_LIST,
        PARSE_MODE_DELETE,
        PARSE_MODE_UPDATE
    } PARSE_MODE;
    typedef enum
    {
        PARSE_SUB_MODE_OPEN = 0,
        PARSE_SUB_MODE_NAME,
        PARSE_SUB_MODE_CERT,
        PARSE_SUB_MODE_SSOGROUPS,
        PARSE_SUB_MODE_WSTRUSTEDROLE,
        PARSE_SUB_MODE_SSOADMINROLE,
        PARSE_SUB_MODE_LOGIN,
        PARSE_SUB_MODE_PASSWORD
    } PARSE_SUB_MODE;
    PARSE_MODE mode = PARSE_MODE_OPEN;
    PARSE_SUB_MODE submode = PARSE_SUB_MODE_OPEN;
    DIR_COMMAND command = DIR_COMMAND_UNKNOWN;

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

                if (!VmAfdStringCompareA(pszArg, "create", TRUE))
                {
                    command = DIR_COMMAND_SERVICE_CREATE;
                    mode = PARSE_MODE_CREATE;
                }
                else if (!VmAfdStringCompareA(pszArg, "list", TRUE))
                {
                    command = DIR_COMMAND_SERVICE_LIST;
                    mode = PARSE_MODE_LIST;
                }
                else if (!VmAfdStringCompareA(pszArg, "update", TRUE))
                {
                    command = DIR_COMMAND_SERVICE_UPDATE;
                    mode = PARSE_MODE_UPDATE;
                }
                else if (!VmAfdStringCompareA(pszArg, "delete", TRUE))
                {
                    command = DIR_COMMAND_SERVICE_DELETE;
                    mode = PARSE_MODE_DELETE;
                }
                else
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_CREATE:
            case PARSE_MODE_UPDATE:

                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                        if (!VmAfdStringCompareA(pszArg, "--name", TRUE))
                        {
                            submode = PARSE_SUB_MODE_NAME;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--cert", TRUE))
                        {
                            submode = PARSE_SUB_MODE_CERT;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--ssogroups", TRUE))
                        {
                            submode = PARSE_SUB_MODE_SSOGROUPS;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--wstrustrole", TRUE))
                        {
                            submode = PARSE_SUB_MODE_WSTRUSTEDROLE;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--ssoadminrole", TRUE))
                        {
                            submode = PARSE_SUB_MODE_SSOADMINROLE;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--login", TRUE))
                        {
                            submode = PARSE_SUB_MODE_LOGIN;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        break;

                    case PARSE_SUB_MODE_NAME:

                        pszServiceName = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_CERT:

                        pszCertPath = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_SSOGROUPS:

                        pszSsoGroups = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_WSTRUSTEDROLE:

                        if (!VmAfdStringCompareA(pszArg, "ActAsUser", TRUE))
                        {
                            bTrustedUserGroup = TRUE;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_SSOADMINROLE:

                        if (!VmAfdStringCompareA(pszArg, "Administrator", TRUE))
                        {
                            ssoAdminRole = SSO_ROLE_ADMINISTRATOR;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "User", TRUE))
                        {
                            ssoAdminRole = SSO_ROLE_USER;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_LOGIN:

                        pszLogin = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_PASSWORD:

                        pszPassword = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;
                }

                break;

            case PARSE_MODE_LIST:

                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                        if (!VmAfdStringCompareA(pszArg, "--login", TRUE))
                        {
                            submode = PARSE_SUB_MODE_LOGIN;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        break;

                    case PARSE_SUB_MODE_LOGIN:

                        pszLogin = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_PASSWORD:

                        pszPassword = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    default:

                        dwError = ERROR_INVALID_STATE;
                        BAIL_ON_VMAFD_ERROR(dwError);

                        break;
                }

                break;

            case PARSE_MODE_DELETE:

                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                        if (!VmAfdStringCompareA(pszArg, "--name", TRUE))
                        {
                            submode = PARSE_SUB_MODE_NAME;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--login", TRUE))
                        {
                            submode = PARSE_SUB_MODE_LOGIN;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        break;

                    case PARSE_SUB_MODE_NAME:

                        pszServiceName = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_LOGIN:

                        pszLogin = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_PASSWORD:

                        pszPassword = pszArg;

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

    switch (command)
    {
        case DIR_COMMAND_SERVICE_CREATE:

            dwError = DirCliCreateServiceA(
                           pszServiceName,
                           pszCertPath,
                           pszSsoGroups,
                           bTrustedUserGroup,
                           ssoAdminRole,
                           pszLogin,
                           pszPassword);
            BAIL_ON_VMAFD_ERROR(dwError);

            fprintf(
                stdout,
                "Service [%s] created successfully\n",
                pszServiceName);

            break;

        case DIR_COMMAND_SERVICE_UPDATE:

            dwError = DirCliUpdateServiceA(
                           pszServiceName,
                           pszCertPath,
                           pszLogin,
                           pszPassword);
            BAIL_ON_VMAFD_ERROR(dwError);

            fprintf(
                stdout,
                "Service [%s] updated successfully\n",
                pszServiceName);

            break;

        case DIR_COMMAND_SERVICE_LIST:

            dwError = DirCliListServiceA(pszLogin, pszPassword);
            BAIL_ON_VMAFD_ERROR(dwError);

            break;

        case DIR_COMMAND_SERVICE_DELETE:

            fprintf(
                stdout,
                "Confirm deleting service [%s] - [Y/N] : ",
                pszServiceName);

            switch (getchar())
            {
                case 'y':
                case 'Y':

                    dwError = DirCliDeleteServiceA(
                                   pszServiceName,
                                   pszLogin,
                                   pszPassword);
                    BAIL_ON_VMAFD_ERROR(dwError);

                    fprintf(
                        stdout,
                        "Service [%s] deleted successfully\n",
                        pszServiceName);

                    break;

                default:

                    fprintf(
                        stdout,
                        "Service [%s] was not deleted\n",
                        pszServiceName);

                    break;
            }

            break;

        default:

            dwError = ERROR_INVALID_STATE;

            break;
    }

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
DirCliExecCertificateRequest(
    int   argc,
    char* argv[]
    )
{
    DWORD dwError = 0;
    DWORD idx = 0;
    BOOL bPublishChain = FALSE;

    typedef enum
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_PUBLISH_CERT,
        PARSE_MODE_PUBLISH_CRL,
        PARSE_MODE_UNPUBLISH_CERT,
        PARSE_MODE_LIST,
        PARSE_MODE_GET,
    } PARSE_MODE;

    typedef enum
    {
        PARSE_SUB_MODE_OPEN = 0,
        PARSE_SUB_MODE_ID,
        PARSE_SUB_MODE_CERT,
        PARSE_SUB_MODE_CRL,
        PARSE_SUB_MODE_LOGIN,
        PARSE_SUB_MODE_PASSWORD
    } PARSE_SUB_MODE;

    PARSE_MODE mode = PARSE_MODE_OPEN;
    PARSE_SUB_MODE submode = PARSE_SUB_MODE_OPEN;
    DIR_COMMAND command = DIR_COMMAND_UNKNOWN;

    PSTR pszCACN = NULL;
    PSTR pszCertFile = NULL;
    PSTR pszCrlFile = NULL;
    PSTR pszLogin = NULL;
    PSTR pszPassword = NULL;

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

                if (!VmAfdStringCompareA(pszArg, "publish", TRUE))
                {
                    command = DIR_COMMAND_CERTIFICATE_PUB_CERT;
                    mode = PARSE_MODE_PUBLISH_CERT;
                }
                else if (!VmAfdStringCompareA(pszArg, "publish-crl", TRUE))
                {
                    command = DIR_COMMAND_CERTIFICATE_PUB_CRL;
                    mode = PARSE_MODE_PUBLISH_CRL;
                }
                else if (!VmAfdStringCompareA(pszArg, "unpublish", TRUE))
                {
                    command = DIR_COMMAND_CERTIFICATE_UNPUB_CERT;
                    mode = PARSE_MODE_UNPUBLISH_CERT;
                }
                else if (!VmAfdStringCompareA(pszArg, "get", TRUE))
                {
                    command = DIR_COMMAND_CERTIFICATE_GET;
                    mode = PARSE_MODE_GET;
                }
                else if (!VmAfdStringCompareA(pszArg, "list", TRUE))
                {
                    command = DIR_COMMAND_CERTIFICATE_LIST;
                    mode = PARSE_MODE_LIST;
                }
                else
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_PUBLISH_CERT:

                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:
                        if (!VmAfdStringCompareA(pszArg, "--cert", TRUE))
                        {
                            submode = PARSE_SUB_MODE_CERT;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--crl", TRUE))
                        {
                            submode = PARSE_SUB_MODE_CRL;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--login", TRUE))
                        {
                            submode = PARSE_SUB_MODE_LOGIN;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--chain", TRUE))
                        {
                            bPublishChain = TRUE;
                            submode = PARSE_SUB_MODE_OPEN;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        break;

                    case PARSE_SUB_MODE_CERT:

                        pszCertFile = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;

                    case PARSE_SUB_MODE_CRL:

                        pszCrlFile = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;

                    case PARSE_SUB_MODE_LOGIN:

                        pszLogin = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;

                    case PARSE_SUB_MODE_PASSWORD:

                        pszPassword = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;
                    default:
                        dwError = ERROR_INVALID_PARAMETER;
                        BAIL_ON_VMAFD_ERROR(dwError);
                        break;
                }
                break;

            case PARSE_MODE_PUBLISH_CRL:

                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:
                        if (!VmAfdStringCompareA(pszArg, "--crl", TRUE))
                        {
                            submode = PARSE_SUB_MODE_CRL;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--login", TRUE))
                        {
                            submode = PARSE_SUB_MODE_LOGIN;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        break;

                    case PARSE_SUB_MODE_CRL:

                        pszCrlFile = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;

                    case PARSE_SUB_MODE_LOGIN:

                        pszLogin = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;

                    case PARSE_SUB_MODE_PASSWORD:

                        pszPassword = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;
                    default:
                        dwError = ERROR_INVALID_PARAMETER;
                        BAIL_ON_VMAFD_ERROR(dwError);
                        break;
                }
                break;

            case PARSE_MODE_GET:
                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:
                        if (!VmAfdStringCompareA(pszArg, "--outcert", TRUE))
                        {
                            submode = PARSE_SUB_MODE_CERT;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--outcrl", TRUE))
                        {
                            submode = PARSE_SUB_MODE_CRL;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--id", TRUE))
                        {
                            submode = PARSE_SUB_MODE_ID;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--login", TRUE))
                        {
                            submode = PARSE_SUB_MODE_LOGIN;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        break;

                    case PARSE_SUB_MODE_CERT:

                        pszCertFile = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;

                    case PARSE_SUB_MODE_CRL:

                        pszCrlFile = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;

                    case PARSE_SUB_MODE_ID:

                        pszCACN = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;

                    case PARSE_SUB_MODE_LOGIN:

                        pszLogin = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;

                    case PARSE_SUB_MODE_PASSWORD:

                        pszPassword = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;
                    default:
                        dwError = ERROR_INVALID_PARAMETER;
                        BAIL_ON_VMAFD_ERROR(dwError);
                        break;
                }
                break;

            case PARSE_MODE_UNPUBLISH_CERT:

                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:
                        if (!VmAfdStringCompareA(pszArg, "--cert", TRUE))
                        {
                            submode = PARSE_SUB_MODE_CERT;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--login", TRUE))
                        {
                            submode = PARSE_SUB_MODE_LOGIN;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        break;

                    case PARSE_SUB_MODE_CERT:

                        pszCertFile = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;

                    case PARSE_SUB_MODE_LOGIN:

                        pszLogin = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;

                    case PARSE_SUB_MODE_PASSWORD:

                        pszPassword = pszArg;
                        submode = PARSE_SUB_MODE_OPEN;
                        break;
                    default:
                        dwError = ERROR_INVALID_PARAMETER;
                        BAIL_ON_VMAFD_ERROR(dwError);
                        break;
                }
                break;

            case PARSE_MODE_LIST:
                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                        if (!VmAfdStringCompareA(pszArg, "--login", TRUE))
                        {
                            submode = PARSE_SUB_MODE_LOGIN;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        break;

                    case PARSE_SUB_MODE_LOGIN:

                        pszLogin = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_PASSWORD:

                        pszPassword = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    default:

                        dwError = ERROR_INVALID_STATE;
                        BAIL_ON_VMAFD_ERROR(dwError);

                        break;
                }

            default:
                dwError = ERROR_INVALID_STATE;
                break;
        }
    }

    switch (command)
    {
        case DIR_COMMAND_CERTIFICATE_PUB_CERT:
            BAIL_ON_VMAFD_INVALID_POINTER(pszCertFile, dwError);
            dwError = DirCliPublishCertA(pszCertFile, pszCrlFile, pszLogin, pszPassword, bPublishChain);
            BAIL_ON_VMAFD_ERROR(dwError);

            fprintf(stdout,
                "Certificate pubished successfully\n");

            break;

        case DIR_COMMAND_CERTIFICATE_PUB_CRL:
            BAIL_ON_VMAFD_INVALID_POINTER(pszCrlFile, dwError);
            dwError = DirCliPublishCrlA(NULL, pszCrlFile, pszLogin, pszPassword);
            BAIL_ON_VMAFD_ERROR(dwError);

            fprintf(stdout,
                    "CRL pubished successfully\n");

            break;

        case DIR_COMMAND_CERTIFICATE_UNPUB_CERT:
            BAIL_ON_VMAFD_INVALID_POINTER(pszCertFile, dwError);
            dwError = DirCliUnpublishCertA(pszCertFile, pszLogin, pszPassword);
            BAIL_ON_VMAFD_ERROR(dwError);

            fprintf(stdout,
                    "Certificate unpubished successfully\n");

            break;

        case DIR_COMMAND_CERTIFICATE_GET:
            dwError = DirCliGetCertificationAuthoritiesA(
                    pszCACN, pszCertFile, pszCrlFile, pszLogin, pszPassword);
            BAIL_ON_VMAFD_ERROR(dwError);

            fprintf(stdout,
                    "Certificate retrieved successfully\n");

            break;

        case DIR_COMMAND_CERTIFICATE_LIST:
            dwError = DirCliListCertificationAuthoritiesA(pszLogin, pszPassword);
            BAIL_ON_VMAFD_ERROR(dwError);
            break;

        default:
            dwError = ERROR_INVALID_STATE;
            break;
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
DirCliExecTopologyRequest(
    int   argc,
    char* argv[]
    )
{
    DWORD dwError = 0;
    DWORD idx = 0;
    PSTR pszServerName = NULL;

    typedef enum
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_LIST
    } PARSE_MODE;

    typedef enum
    {
        PARSE_SUB_MODE_OPEN = 0,
        PARSE_SUB_MODE_SERVER_NAME,
        PARSE_SUB_MODE_LOGIN,
        PARSE_SUB_MODE_PASSWORD
    } PARSE_SUB_MODE;

    PARSE_MODE mode = PARSE_MODE_OPEN;
    PARSE_SUB_MODE submode = PARSE_SUB_MODE_OPEN;
    DIR_COMMAND command = DIR_COMMAND_UNKNOWN;

    PSTR pszLogin = NULL;
    PSTR pszPassword = NULL;

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

                if (!VmAfdStringCompareA(pszArg, "list", TRUE))
                {
                    command = DIR_COMMAND_NODES_LIST;
                    mode = PARSE_MODE_LIST;
                }
                else
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_LIST:
                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                        if (!strcmp(pszArg, "--server-name"))
                        {
                            submode = PARSE_SUB_MODE_SERVER_NAME;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--login", TRUE))
                        {
                            submode = PARSE_SUB_MODE_LOGIN;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        break;

                    case PARSE_SUB_MODE_SERVER_NAME:

                        if (pszServerName)
                        {
                            dwError = ERROR_LOCAL_OPTION_INVALID;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }

                        pszServerName = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_LOGIN:

                        pszLogin = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_PASSWORD:

                        pszPassword = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    default:

                        dwError = ERROR_INVALID_STATE;
                        BAIL_ON_VMAFD_ERROR(dwError);

                        break;
                }

            default:
                dwError = ERROR_INVALID_STATE;
                break;
        }
    }

    switch (command)
    {
        case DIR_COMMAND_NODES_LIST:
            dwError = DirCliListNodesA(pszServerName, pszLogin, pszPassword);
            BAIL_ON_VMAFD_ERROR(dwError);
            break;

        default:
            dwError = ERROR_INVALID_STATE;
            break;
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
DirCliExecMachineAccountReset(
    int   argc,
    char* argv[]
    )
{
    DWORD dwError = 0;
    DIR_COMMAND command = DIR_COMMAND_UNKNOWN;
    DWORD idx = 0;
    PSTR pszLogin = NULL;
    PSTR pszPassword = NULL;
    PSTR pszServerName = NULL;

    typedef enum
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_PASSWORD_RESET
    } PARSE_MODE;

    typedef enum
    {
        PARSE_SUB_MODE_OPEN = 0,
        PARSE_SUB_MODE_SERVER_NAME,
        PARSE_SUB_MODE_LOGIN,
        PARSE_SUB_MODE_PASSWORD
    } PARSE_SUB_MODE;

    PARSE_MODE mode = PARSE_MODE_OPEN;
    PARSE_SUB_MODE submode = PARSE_SUB_MODE_OPEN;

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

                if (!VmAfdStringCompareA(pszArg, "password-reset", TRUE))
                {
                    command = DIR_COMMAND_COMPUTER_PASSWORD_RESET;
                    mode = PARSE_MODE_PASSWORD_RESET;
                }
                else
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_PASSWORD_RESET:
                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                        if (!VmAfdStringCompareA(pszArg, "--live-dc-hostname", TRUE))
                        {
                            submode = PARSE_SUB_MODE_SERVER_NAME;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--login", TRUE))
                        {
                            submode = PARSE_SUB_MODE_LOGIN;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        break;

                    case PARSE_SUB_MODE_SERVER_NAME:

                        if (pszServerName)
                        {
                            dwError = ERROR_LOCAL_OPTION_INVALID;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }

                        pszServerName = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_LOGIN:

                        pszLogin = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_PASSWORD:

                        pszPassword = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    default:

                        dwError = ERROR_INVALID_STATE;
                        BAIL_ON_VMAFD_ERROR(dwError);

                        break;
                }
                break;

            default:
                dwError = ERROR_INVALID_STATE;
                break;
        }
    }

    switch (command)
    {
        case DIR_COMMAND_COMPUTER_PASSWORD_RESET:
            dwError = DirCliMachineAccountReset(
                        pszServerName,
                        pszLogin,
                        pszPassword);
            BAIL_ON_VMAFD_ERROR(dwError);
            break;

        default:
            dwError = ERROR_INVALID_STATE;
            break;
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
DirCliExecGroupPrincipalRequest(
    int   argc,
    char* argv[]
    )
{
    DWORD dwError = 0;
    DWORD idx = 0;

    PSTR pszGroupName = NULL;
    PSTR pszDescription = NULL;
    PSTR pszLogin = NULL;
    PSTR pszPassword = NULL;

    typedef enum
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_CREATE,
    } PARSE_MODE;
    typedef enum
    {
        PARSE_SUB_MODE_OPEN = 0,
        PARSE_SUB_MODE_NAME,
        PARSE_SUB_MODE_DESCRIPTION,
        PARSE_SUB_MODE_LOGIN,
        PARSE_SUB_MODE_PASSWORD
    } PARSE_SUB_MODE;

    PARSE_MODE mode = PARSE_MODE_OPEN;
    PARSE_SUB_MODE submode = PARSE_SUB_MODE_OPEN;
    DIR_COMMAND command = DIR_COMMAND_UNKNOWN;

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

                if (!VmAfdStringCompareA(pszArg, "create", TRUE))
                {
                    command = DIR_COMMAND_GROUP_CREATE;
                    mode = PARSE_MODE_CREATE;
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

                        if (!VmAfdStringCompareA(pszArg, "--name", TRUE))
                        {
                            submode = PARSE_SUB_MODE_NAME;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--description", TRUE))
                        {
                            submode = PARSE_SUB_MODE_DESCRIPTION;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--login", TRUE))
                        {
                            submode = PARSE_SUB_MODE_LOGIN;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        break;

                    case PARSE_SUB_MODE_NAME:

                        pszGroupName = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_DESCRIPTION:

                        pszDescription = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_LOGIN:

                        pszLogin = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_PASSWORD:

                        pszPassword = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;
                }

                break;
        }
    }

    switch (command)
    {
        case DIR_COMMAND_GROUP_CREATE:

            dwError = DirCliCreateGroupA(
                            pszGroupName,
                            pszDescription,
                            pszLogin,
                            pszPassword);
            BAIL_ON_VMAFD_ERROR(dwError);

            fprintf(
                stdout,
                "Service [%s] created successfully\n",
                pszGroupName);

            break;

        default:

            dwError = ERROR_INVALID_STATE;

            break;
    }

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
DirCliExecUserRequest(
    int   argc,
    char* argv[]
    )
{
    DWORD dwError = 0;
    DWORD idx = 0;

    PSTR pszAcctName = NULL;
    PSTR pszUserPassword = NULL;
    PSTR pszFirstname = NULL;
    PSTR pszLastname = NULL;
    PSTR pszLogin = NULL;
    PSTR pszPassword = NULL;
    USER_INFO_LEVEL userInfoLevel = USER_INFO_LEVEL_DEFAULT;
    USER_MODIFY_OPT userModifyOpt = USER_MODIFY_NO_OPT;

    typedef enum
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_CREATE,
        PARSE_MODE_MODIFY,
        PARSE_MODE_DELETE,
        PARSE_MODE_FIND
    } PARSE_MODE;
    typedef enum
    {
        PARSE_SUB_MODE_OPEN = 0,
        PARSE_SUB_MODE_ACCT,
        PARSE_SUB_MODE_FIRST_NAME,
        PARSE_SUB_MODE_LAST_NAME,
        PARSE_SUB_MODE_USER_PASSWORD,
        PARSE_SUB_MODE_LOGIN,
        PARSE_SUB_MODE_PASSWORD,
        PARSE_SUB_MODE_INFO_LEVEL
    } PARSE_SUB_MODE;

    PARSE_MODE mode = PARSE_MODE_OPEN;
    PARSE_SUB_MODE submode = PARSE_SUB_MODE_OPEN;
    DIR_COMMAND command = DIR_COMMAND_UNKNOWN;

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

                if (!VmAfdStringCompareA(pszArg, "create", TRUE))
                {
                    command = DIR_COMMAND_USER_CREATE;
                    mode = PARSE_MODE_CREATE;
                }
                else if (!VmAfdStringCompareA(pszArg, "modify", TRUE))
                {
                    command = DIR_COMMAND_USER_MODIFY;
                    mode = PARSE_MODE_MODIFY;
                }
                else if (!VmAfdStringCompareA(pszArg, "delete", TRUE))
                {
                    command = DIR_COMMAND_USER_DELETE;
                    mode = PARSE_MODE_DELETE;
                }
                else if (!VmAfdStringCompareA(pszArg, "find-by-name", TRUE))
                {
                    command = DIR_COMMAND_USER_FIND_BY_NAME;
                    mode = PARSE_MODE_FIND;
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

                        if (!VmAfdStringCompareA(pszArg, "--account", TRUE))
                        {
                            submode = PARSE_SUB_MODE_ACCT;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--first-name", TRUE))
                        {
                            submode = PARSE_SUB_MODE_FIRST_NAME;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--last-name", TRUE))
                        {
                            submode = PARSE_SUB_MODE_LAST_NAME;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--user-password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_USER_PASSWORD;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--login", TRUE))
                        {
                            submode = PARSE_SUB_MODE_LOGIN;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        break;

                    case PARSE_SUB_MODE_ACCT:

                        pszAcctName = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_FIRST_NAME:

                        pszFirstname = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_LAST_NAME:

                        pszLastname = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_USER_PASSWORD:

                        pszUserPassword = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_LOGIN:

                        pszLogin = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_PASSWORD:

                        pszPassword = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    default:

                        dwError = ERROR_INVALID_STATE;
                        BAIL_ON_VMAFD_ERROR(dwError);
                }

                break;

            case PARSE_MODE_MODIFY:

                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                        if (!VmAfdStringCompareA(pszArg, "--account", TRUE))
                        {
                            submode = PARSE_SUB_MODE_ACCT;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--password-never-expires", TRUE))
                        {
                            if ((userModifyOpt == USER_MODIFY_NO_OPT) ||
                            !(userModifyOpt & USER_MODIFY_PWD_EXPIRE))
                            {
                                userModifyOpt |= USER_MODIFY_PWD_NEVER_EXP;
                            }
                            else{
                                dwError = ERROR_INVALID_PARAMETER;
                                BAIL_ON_VMAFD_ERROR(dwError);
                            }
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--password-expires", TRUE))
                        {

                            if ((userModifyOpt == USER_MODIFY_NO_OPT) ||
                            !(userModifyOpt & USER_MODIFY_PWD_NEVER_EXP))
                            {
                                userModifyOpt |= USER_MODIFY_PWD_EXPIRE;
                            }
                            else{
                                dwError = ERROR_INVALID_PARAMETER;
                                BAIL_ON_VMAFD_ERROR(dwError);
                            }
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--login", TRUE))
                        {
                            submode = PARSE_SUB_MODE_LOGIN;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        break;

                    case PARSE_SUB_MODE_ACCT:

                        pszAcctName = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_LOGIN:

                        pszLogin = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_PASSWORD:

                        pszPassword = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    default:

                        dwError = ERROR_INVALID_STATE;
                        BAIL_ON_VMAFD_ERROR(dwError);
                }

                break;

            case PARSE_MODE_DELETE:

                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                        if (!VmAfdStringCompareA(pszArg, "--account", TRUE))
                        {
                            submode = PARSE_SUB_MODE_ACCT;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--login", TRUE))
                        {
                            submode = PARSE_SUB_MODE_LOGIN;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        break;

                    case PARSE_SUB_MODE_ACCT:

                        pszAcctName = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_LOGIN:

                        pszLogin = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_PASSWORD:

                        pszPassword = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    default:

                        dwError = ERROR_INVALID_STATE;
                        BAIL_ON_VMAFD_ERROR(dwError);
                }

                break;

            case PARSE_MODE_FIND:

                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                        if (!VmAfdStringCompareA(pszArg, "--level", TRUE))
                        {
                          submode = PARSE_SUB_MODE_INFO_LEVEL;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--account", TRUE))
                        {
                          submode = PARSE_SUB_MODE_ACCT;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--login", TRUE))
                        {
                          submode = PARSE_SUB_MODE_LOGIN;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--password", TRUE))
                        {
                          submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        else
                        {
                          dwError = ERROR_INVALID_PARAMETER;
                          BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        break;

                    case PARSE_SUB_MODE_INFO_LEVEL:

                        if (!VmAfdStringCompareA(pszArg, "0", TRUE))
                        {
                          userInfoLevel = USER_INFO_LEVEL_DEFAULT;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "1", TRUE))
                        {
                          userInfoLevel = USER_INFO_LEVEL_ONE;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "2", TRUE))
                        {
                          userInfoLevel = USER_INFO_LEVEL_TWO;
                        }
                        else
                        {
                          dwError = ERROR_INVALID_PARAMETER;
                          BAIL_ON_VMAFD_ERROR(dwError);
                        }

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_ACCT:

                        pszAcctName = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_LOGIN:

                        pszLogin = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_PASSWORD:

                        pszPassword = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    default:

                        dwError = ERROR_INVALID_STATE;
                        BAIL_ON_VMAFD_ERROR(dwError);
                }

                break;

        }
    }

    switch (command)
    {
        case DIR_COMMAND_USER_CREATE:

            dwError = DirCliCreateUserA(
                            pszAcctName,
                            pszFirstname,
                            pszLastname,
                            pszUserPassword,
                            pszLogin,
                            pszPassword);
            BAIL_ON_VMAFD_ERROR(dwError);

            fprintf(
                stdout,
                "User account [%s] created successfully\n",
                VMAFD_SAFE_STRING(pszAcctName));

            break;

        case DIR_COMMAND_USER_MODIFY:

            dwError = DirCliModifyAttributeUserA(
                            pszAcctName,
                            pszLogin,
                            pszPassword,
                            userModifyOpt);
            BAIL_ON_VMAFD_ERROR(dwError);

            break;

        case DIR_COMMAND_USER_DELETE:

            dwError = DirCliDeleteUserA(pszAcctName, pszLogin, pszPassword);
            BAIL_ON_VMAFD_ERROR(dwError);

            fprintf(
                stdout,
                "User account [%s] deleted successfully\n",
                VMAFD_SAFE_STRING(pszAcctName));

            break;

        case DIR_COMMAND_USER_FIND_BY_NAME:

            dwError = DirCliFindByNameUserA(
                            pszAcctName,
                            pszLogin,
                            pszPassword,
                            userInfoLevel);
            BAIL_ON_VMAFD_ERROR(dwError);

            break;

        default:

            dwError = ERROR_INVALID_STATE;

            break;
    }

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
DirCliExecGroupRequest(
    int   argc,
    char* argv[]
    )
{
    DWORD dwError = 0;
    DWORD idx = 0;

    PSTR pszGroupName = NULL;
    PSTR pszAcctName = NULL;
    PSTR pszLogin = NULL;
    PSTR pszPassword = NULL;
    BOOLEAN bAddMembership = FALSE;

    typedef enum
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_ADD,
        PARSE_MODE_DELETE,
        PARSE_MODE_MODIFY,
        PARSE_MODE_LIST
    } PARSE_MODE;
    typedef enum
    {
        PARSE_SUB_MODE_OPEN = 0,
        PARSE_SUB_MODE_NAME,
        PARSE_SUB_MODE_ADD,
        PARSE_SUB_MODE_REMOVE,
        PARSE_SUB_MODE_LOGIN,
        PARSE_SUB_MODE_PASSWORD
    } PARSE_SUB_MODE;

    PARSE_MODE mode = PARSE_MODE_OPEN;
    PARSE_SUB_MODE submode = PARSE_SUB_MODE_OPEN;
    DIR_COMMAND command = DIR_COMMAND_UNKNOWN;

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

                if (!VmAfdStringCompareA(pszArg, "add", TRUE))
                {
                    command = DIR_COMMAND_GROUP_CREATE;
                    mode = PARSE_MODE_ADD;
                }
                else if (!VmAfdStringCompareA(pszArg, "delete", TRUE))
                {
                    command = DIR_COMMAND_GROUP_DELETE;
                    mode = PARSE_MODE_DELETE;
                }
                else if (!VmAfdStringCompareA(pszArg, "modify", TRUE))
                {
                    command = DIR_COMMAND_GROUP_MODIFY;
                    mode = PARSE_MODE_MODIFY;
                }
                else if (!VmAfdStringCompareA(pszArg, "list", TRUE))
                {
                    command = DIR_COMMAND_GROUP_LIST;
                    mode = PARSE_MODE_LIST;
                }
                else
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_ADD:

                /* TBD */
                dwError = ERROR_NOT_SUPPORTED;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;

            case PARSE_MODE_DELETE:

                /* TBD */
                dwError = ERROR_NOT_SUPPORTED;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;

            case PARSE_MODE_MODIFY:

                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                        if (!VmAfdStringCompareA(pszArg, "--name", TRUE))
                        {
                            submode = PARSE_SUB_MODE_NAME;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--add", TRUE))
                        {
                            submode = PARSE_SUB_MODE_ADD;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--remove", TRUE))
                        {
                            submode = PARSE_SUB_MODE_REMOVE;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--login", TRUE))
                        {
                            submode = PARSE_SUB_MODE_LOGIN;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        break;

                    case PARSE_SUB_MODE_NAME:

                        pszGroupName = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_ADD:

                        pszAcctName = pszArg;
                        bAddMembership = TRUE;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_REMOVE:

                        pszAcctName = pszArg;
                        bAddMembership = FALSE;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_LOGIN:

                        pszLogin = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_PASSWORD:

                        pszPassword = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    default:

                        dwError = ERROR_INVALID_STATE;
                        BAIL_ON_VMAFD_ERROR(dwError);
                }

                break;

            case PARSE_MODE_LIST:

                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                        if (!VmAfdStringCompareA(pszArg, "--name", TRUE))
                        {
                            submode = PARSE_SUB_MODE_NAME;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--login", TRUE))
                        {
                            submode = PARSE_SUB_MODE_LOGIN;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        break;

                    case PARSE_SUB_MODE_NAME:

                        pszGroupName = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_LOGIN:

                        pszLogin = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_PASSWORD:

                        pszPassword = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    default:

                        dwError = ERROR_INVALID_STATE;
                        BAIL_ON_VMAFD_ERROR(dwError);
                }

                break;

        }
    }

    switch (command)
    {
        case DIR_COMMAND_GROUP_CREATE:

            /* TBD */
            dwError = ERROR_NOT_SUPPORTED;

            break;

        case DIR_COMMAND_GROUP_MODIFY:

            if (bAddMembership)
            {
                dwError = DirCliAddGroupMemberA(
                                pszGroupName,
                                pszAcctName,
                                pszLogin,
                                pszPassword);
                BAIL_ON_VMAFD_ERROR(dwError);

                fprintf(
                    stdout,
                    "Group member [%s] added successfully\n",
                    VMAFD_SAFE_STRING(pszAcctName));

            }
            else
            {
                dwError = DirCliRemoveGroupMemberA(
                                pszGroupName,
                                pszAcctName,
                                pszLogin,
                                pszPassword);
                BAIL_ON_VMAFD_ERROR(dwError);

                fprintf(
                    stdout,
                    "Group member [%s] removed successfully\n",
                    VMAFD_SAFE_STRING(pszAcctName));

            }

            break;

        case DIR_COMMAND_GROUP_DELETE:

            /* TBD */
            dwError = ERROR_NOT_SUPPORTED;

            break;

        case DIR_COMMAND_GROUP_LIST:

            dwError = DirCliListGroupA(
                                pszGroupName,
                                pszLogin,
                                pszPassword);
            BAIL_ON_VMAFD_ERROR(dwError);

            break;

        default:

            dwError = ERROR_INVALID_STATE;

            break;
    }

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
DirCliExecPasswordRequest(
    int   argc,
    char* argv[]
    )
{
    DWORD dwError = 0;
    DWORD idx = 0;
    PSTR pszAccount = NULL;
    PSTR pszLogin = NULL;
    PSTR pszPassword = NULL;
    PSTR pszPasswordNew = NULL;
    PSTR pszPasswordGenerated = NULL;

    typedef enum
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_CREATE,
        PARSE_MODE_CHANGE,
        PARSE_MODE_RESET
    } PARSE_MODE;
    typedef enum
    {
        PARSE_SUB_MODE_OPEN = 0,
        PARSE_SUB_MODE_LOGIN,
        PARSE_SUB_MODE_PASSWORD,
        PARSE_SUB_MODE_NEW_PASSWORD,
        PARSE_SUB_MODE_ACCOUNT
    } PARSE_SUB_MODE;

    PARSE_MODE mode = PARSE_MODE_OPEN;
    PARSE_SUB_MODE submode = PARSE_SUB_MODE_OPEN;
    DIR_COMMAND command = DIR_COMMAND_UNKNOWN;

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

                if (!VmAfdStringCompareA(pszArg, "create", TRUE))
                {
                    command = DIR_COMMAND_PASSWORD_CREATE;
                    mode = PARSE_MODE_CREATE;
                }
                else if (!VmAfdStringCompareA(pszArg, "change", TRUE))
                {
                    command = DIR_COMMAND_PASSWORD_CHANGE;
                    mode = PARSE_MODE_CHANGE;
                }
                else if (!VmAfdStringCompareA(pszArg, "reset", TRUE))
                {
                    command = DIR_COMMAND_PASSWORD_RESET;
                    mode = PARSE_MODE_RESET;
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

                        if (!VmAfdStringCompareA(pszArg, "--login", TRUE))
                        {
                            submode = PARSE_SUB_MODE_LOGIN;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        break;

                    case PARSE_SUB_MODE_LOGIN:

                        pszLogin = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_PASSWORD:

                        pszPassword = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    default:

                        dwError = ERROR_INVALID_STATE;
                        BAIL_ON_VMAFD_ERROR(dwError);
                }

                break;

            case PARSE_MODE_CHANGE:

                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                        if (!VmAfdStringCompareA(pszArg, "--account", TRUE))
                        {
                            submode = PARSE_SUB_MODE_ACCOUNT;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--current", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--new", TRUE))
                        {
                            submode = PARSE_SUB_MODE_NEW_PASSWORD;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        break;

                    case PARSE_SUB_MODE_ACCOUNT:

                        pszAccount = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_PASSWORD:

                        pszPassword = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_NEW_PASSWORD:

                        pszPasswordNew = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    default:

                        dwError = ERROR_INVALID_STATE;
                        BAIL_ON_VMAFD_ERROR(dwError);
                }

                break;

            case PARSE_MODE_RESET:

                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                        if (!VmAfdStringCompareA(pszArg, "--account", TRUE))
                        {
                            submode = PARSE_SUB_MODE_ACCOUNT;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--login", TRUE))
                        {
                            submode = PARSE_SUB_MODE_LOGIN;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--new", TRUE))
                        {
                            submode = PARSE_SUB_MODE_NEW_PASSWORD;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        break;

                    case PARSE_SUB_MODE_ACCOUNT:

                        pszAccount = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_LOGIN:

                        pszLogin = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_PASSWORD:

                        pszPassword = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_NEW_PASSWORD:

                        pszPasswordNew = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    default:

                        dwError = ERROR_INVALID_STATE;
                        BAIL_ON_VMAFD_ERROR(dwError);
                }

                break;
        }
    }

    switch (command)
    {
        case DIR_COMMAND_PASSWORD_CREATE:

            dwError = DirCliGeneratePassword(
                            pszLogin,
                            pszPassword,
                            &pszPasswordGenerated);
            BAIL_ON_VMAFD_ERROR(dwError);

            fprintf(stdout, "%s\n", pszPasswordGenerated);

            break;

        case DIR_COMMAND_PASSWORD_CHANGE:

            dwError = DirCliChangePassword(
                            pszAccount,
                            pszPassword,
                            pszPasswordNew);
            BAIL_ON_VMAFD_ERROR(dwError);

            fprintf(
                stdout,
                "Password was changed successfully for [%s]\n",
                VMAFD_SAFE_STRING(pszAccount));

            break;

        case DIR_COMMAND_PASSWORD_RESET:

            dwError = DirCliResetPassword(
                            pszAccount,
                            pszPasswordNew,
                            pszLogin,
                            pszPassword);
            BAIL_ON_VMAFD_ERROR(dwError);

            fprintf(
                stdout,
                "Password was reset successfully for [%s]\n",
                VMAFD_SAFE_STRING(pszAccount));

            break;

        default:

            dwError = ERROR_INVALID_STATE;

            break;
    }

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszPasswordGenerated);

    return dwError;

error:

    goto cleanup;
}

static
DWORD
DirCliExecStateRequest(
    int   argc,
    char* argv[]
    )
{

    DWORD dwError = 0;
    DWORD iArg = 0;
    PSTR  pszState = NULL;
    DWORD dwState = 0;
    PSTR  pszServerName = NULL;
    PSTR  pszDomainName = NULL;
    PSTR  pszUserName = NULL;
    PSTR  pszPassword = NULL;
    PSTR  pszStateStr = NULL;
    typedef enum
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_GET,
        PARSE_MODE_SET
    } PARSE_MODE;


    typedef enum
    {
        PARSE_SUB_MODE_OPEN = 0,
        PARSE_SUB_MODE_SERVER_NAME,
        PARSE_SUB_MODE_DOMAIN_NAME,
        PARSE_SUB_MODE_USER_NAME,
        PARSE_SUB_MODE_PASSWORD,
        PARSE_SUB_MODE_NEW_STATE
    } PARSE_SUB_MODE;

    PARSE_MODE parseMode = PARSE_MODE_OPEN;
    PARSE_SUB_MODE parseSubMode = PARSE_SUB_MODE_OPEN;
    DIR_COMMAND command = DIR_COMMAND_UNKNOWN;

    if (!argc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    for (iArg = 0; iArg < argc; iArg++)
    {
        PSTR pszArg = argv[iArg];
        switch (parseMode)
        {
        case PARSE_MODE_OPEN:

            if (!VmAfdStringCompareA(pszArg, "get", TRUE))
            {
                command = DIR_COMMAND_STATE_GET;
                parseMode = PARSE_MODE_GET;
            }
            else if (!VmAfdStringCompareA(pszArg, "set", TRUE))
            {
                command = DIR_COMMAND_STATE_SET;
                parseMode = PARSE_MODE_SET;
            }
            else
            {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR(dwError);
            }
            break;

        case PARSE_MODE_GET:

            switch (parseSubMode)
            {
            case PARSE_SUB_MODE_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseSubMode = PARSE_SUB_MODE_SERVER_NAME;
                }
                else if(!strcmp(pszArg, "--domain-name"))
                {
                    parseSubMode = PARSE_SUB_MODE_DOMAIN_NAME;
                }
                else if(!strcmp(pszArg, "--login"))
                {
                    parseSubMode = PARSE_SUB_MODE_USER_NAME;
                }
                else if(!strcmp(pszArg, "--password"))
                {
                    parseSubMode = PARSE_SUB_MODE_PASSWORD;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_SUB_MODE_SERVER_NAME:

                if (pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                pszServerName = pszArg;

                parseSubMode = PARSE_SUB_MODE_OPEN;

                break;
            case PARSE_SUB_MODE_DOMAIN_NAME:
                if (pszDomainName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                pszDomainName = pszArg;

                parseSubMode = PARSE_SUB_MODE_OPEN;

                break;

            case PARSE_SUB_MODE_USER_NAME:
                if (pszUserName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg,
                                               &pszUserName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseSubMode = PARSE_SUB_MODE_OPEN;

                break;

            case PARSE_SUB_MODE_PASSWORD:
                if (pszPassword)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg,
                                               &pszPassword);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseSubMode = PARSE_SUB_MODE_OPEN;

                break;

            default:

                dwError = ERROR_LOCAL_OPTION_INVALID;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
            }

            break;

        case PARSE_MODE_SET:

            switch (parseSubMode)
            {
            case PARSE_SUB_MODE_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseSubMode = PARSE_SUB_MODE_SERVER_NAME;
                }
                else if(!strcmp(pszArg, "--domain-name"))
                {
                    parseSubMode = PARSE_SUB_MODE_DOMAIN_NAME;
                }
                else if(!strcmp(pszArg, "--login"))
                {
                    parseSubMode = PARSE_SUB_MODE_USER_NAME;
                }
                else if(!strcmp(pszArg, "--password"))
                {
                    parseSubMode = PARSE_SUB_MODE_PASSWORD;
                }
                else if (!strcmp(pszArg, "--state"))
                {
                    parseSubMode = PARSE_SUB_MODE_NEW_STATE;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                break;

            case PARSE_SUB_MODE_SERVER_NAME:

                if (pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                pszServerName = pszArg;

                parseSubMode = PARSE_SUB_MODE_OPEN;

                break;
            case PARSE_SUB_MODE_DOMAIN_NAME:
                if (pszDomainName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                pszDomainName = pszArg;

                parseSubMode = PARSE_SUB_MODE_OPEN;

                break;

            case PARSE_SUB_MODE_USER_NAME:
                if (pszUserName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg,
                                               &pszUserName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseSubMode = PARSE_SUB_MODE_OPEN;

                break;

            case PARSE_SUB_MODE_PASSWORD:
                if (pszPassword)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg,
                                               &pszPassword);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseSubMode = PARSE_SUB_MODE_OPEN;

                break;

            case PARSE_SUB_MODE_NEW_STATE:
                if (pszState)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg,
                                               &pszState);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseSubMode = PARSE_SUB_MODE_OPEN;

                break;

            default:

                dwError = ERROR_LOCAL_OPTION_INVALID;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
            }

            break;

        default:

            dwError = ERROR_INVALID_STATE;

            BAIL_ON_VMAFD_ERROR(dwError);

            break;
        }
    }

    switch (command)
    {
        case DIR_COMMAND_STATE_GET:

            dwError = DirCliGetState(
                                pszServerName,
                                pszUserName,
                                pszPassword,
                                pszDomainName,
                                &dwState
                                );
            BAIL_ON_VMAFD_ERROR(dwError);

            switch (dwState)
            {
            case VMDIRD_STATE_UNDEFINED:
                pszStateStr = "Undefined";
                break;
            case VMDIRD_STATE_STARTUP:
                pszStateStr = "Startup";
                break;
            case VMDIRD_STATE_READ_ONLY:
                pszStateStr = "Read only";
                break;
            case VMDIRD_STATE_NORMAL:
                pszStateStr = "Normal" ;
                break;
            case VMDIRD_STATE_SHUTDOWN:
                pszStateStr = "Shutdown";
                break;
            case VMDIRD_STATE_FAILURE:
                pszStateStr = "Failure";
                break;
            case VMDIRD_STATE_READ_ONLY_DEMOTE:
                pszStateStr = "Demote";
                break;
            case VMDIRD_STATE_STANDALONE:
                pszStateStr = "Standalone";
                break;
            case VMDIRD_STATE_RESTORE:
                pszStateStr = "Restore";
                break;
            default:
                pszStateStr = "Unknown";
                break;
            }

            fprintf(
                stdout,
                "Directory Server State: %s (%d)\n",
                pszStateStr,
                dwState
                );

            break;

        case DIR_COMMAND_STATE_SET:

            if (pszState)
            {
                if (!VmAfdStringCompareA(pszState, "NORMAL", FALSE))
                {
                    dwState = VMDIRD_STATE_NORMAL;
                }
                else if (!VmAfdStringCompareA(pszState, "READONLY", FALSE))
                {
                    dwState = VMDIRD_STATE_READ_ONLY;
                }
                else if (!VmAfdStringCompareA(pszState, "STANDALONE", FALSE))
                {
                    dwState = VMDIRD_STATE_STANDALONE;
                }
                else
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

            }

            dwError = DirCliSetState(
                                pszServerName,
                                pszUserName,
                                pszPassword,
                                pszDomainName,
                                dwState
                                );
            BAIL_ON_VMAFD_ERROR(dwError);

            fprintf(
                stdout,
                "Directory Server State set to: %s (%d)\n",
                pszState,
                dwState
                );

            break;

        default:

            dwError = ERROR_INVALID_STATE;

            break;
    }

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszUserName);
    VMAFD_SAFE_FREE_MEMORY(pszPassword);
    VMAFD_SAFE_FREE_MEMORY(pszState);

    return dwError;

error:

    goto cleanup;
}


static
DWORD
DirCliExecFuncLvlRequest(
    int   argc,
    char* argv[]
    )
{

    DWORD dwError = 0;
    DWORD iArg = 0;
    PSTR  pszFuncLvl = NULL;
    DWORD dwFuncLvl = 0;
    PSTR  pszServerName = NULL;
    PSTR  pszDomainName = NULL;
    PSTR  pszUserName = NULL;
    PSTR  pszPassword = NULL;

    typedef enum
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_GET,
        PARSE_MODE_SET
    } PARSE_MODE;


    typedef enum
    {
        PARSE_SUB_MODE_OPEN = 0,
        PARSE_SUB_MODE_SERVER_NAME,
        PARSE_SUB_MODE_DOMAIN_NAME,
        PARSE_SUB_MODE_USER_NAME,
        PARSE_SUB_MODE_PASSWORD,
        PARSE_SUB_MODE_NEW_LEVEL
    } PARSE_SUB_MODE;

    PARSE_MODE parseMode = PARSE_MODE_OPEN;
    PARSE_SUB_MODE parseSubMode = PARSE_SUB_MODE_OPEN;
    DIR_COMMAND command = DIR_COMMAND_UNKNOWN;

    if (!argc)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    for (iArg = 0; iArg < argc; iArg++)
    {
        PSTR pszArg = argv[iArg];
        switch (parseMode)
        {
        case PARSE_MODE_OPEN:

            if (!VmAfdStringCompareA(pszArg, "get", TRUE))
            {
                command = DIR_COMMAND_FUNCLVL_GET;
                parseMode = PARSE_MODE_GET;
            }
            else if (!VmAfdStringCompareA(pszArg, "set", TRUE))
            {
                command = DIR_COMMAND_FUNCLVL_SET;
                parseMode = PARSE_MODE_SET;
            }
            else
            {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR(dwError);
            }
            break;

        case PARSE_MODE_GET:

            switch (parseSubMode)
            {
            case PARSE_SUB_MODE_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseSubMode = PARSE_SUB_MODE_SERVER_NAME;
                }
                else if(!strcmp(pszArg, "--domain-name"))
                {
                    parseSubMode = PARSE_SUB_MODE_DOMAIN_NAME;
                }
                else if(!strcmp(pszArg, "--login"))
                {
                    parseSubMode = PARSE_SUB_MODE_USER_NAME;
                }
                else if(!strcmp(pszArg, "--password"))
                {
                    parseSubMode = PARSE_SUB_MODE_PASSWORD;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_SUB_MODE_SERVER_NAME:

                if (pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                pszServerName = pszArg;

                parseSubMode = PARSE_SUB_MODE_OPEN;

                break;
            case PARSE_SUB_MODE_DOMAIN_NAME:
                if (pszDomainName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                pszDomainName = pszArg;

                parseSubMode = PARSE_SUB_MODE_OPEN;

                break;

            case PARSE_SUB_MODE_USER_NAME:
                if (pszUserName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg,
                                               &pszUserName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseSubMode = PARSE_SUB_MODE_OPEN;

                break;

            case PARSE_SUB_MODE_PASSWORD:
                if (pszPassword)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg,
                                               &pszPassword);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseSubMode = PARSE_SUB_MODE_OPEN;

                break;

            default:

                dwError = ERROR_LOCAL_OPTION_INVALID;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
            }

            break;

        case PARSE_MODE_SET:

            switch (parseSubMode)
            {
            case PARSE_SUB_MODE_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseSubMode = PARSE_SUB_MODE_SERVER_NAME;
                }
                else if(!strcmp(pszArg, "--domain-name"))
                {
                    parseSubMode = PARSE_SUB_MODE_DOMAIN_NAME;
                }
                else if(!strcmp(pszArg, "--login"))
                {
                    parseSubMode = PARSE_SUB_MODE_USER_NAME;
                }
                else if(!strcmp(pszArg, "--password"))
                {
                    parseSubMode = PARSE_SUB_MODE_PASSWORD;
                }
                else if (!strcmp(pszArg, "--level"))
                {
                    parseSubMode = PARSE_SUB_MODE_NEW_LEVEL;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                break;

            case PARSE_SUB_MODE_SERVER_NAME:

                if (pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                pszServerName = pszArg;

                parseSubMode = PARSE_SUB_MODE_OPEN;

                break;
            case PARSE_SUB_MODE_DOMAIN_NAME:
                if (pszDomainName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                pszDomainName = pszArg;

                parseSubMode = PARSE_SUB_MODE_OPEN;

                break;

            case PARSE_SUB_MODE_USER_NAME:
                if (pszUserName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg,
                                               &pszUserName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseSubMode = PARSE_SUB_MODE_OPEN;

                break;

            case PARSE_SUB_MODE_PASSWORD:
                if (pszPassword)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg,
                                               &pszPassword);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseSubMode = PARSE_SUB_MODE_OPEN;

                break;

            case PARSE_SUB_MODE_NEW_LEVEL:
                if (pszFuncLvl)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg,
                                               &pszFuncLvl);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseSubMode = PARSE_SUB_MODE_OPEN;

                break;

            default:

                dwError = ERROR_LOCAL_OPTION_INVALID;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
            }

            break;

        default:

            dwError = ERROR_INVALID_STATE;

            BAIL_ON_VMAFD_ERROR(dwError);

            break;
        }
    }

    switch (command)
    {
        case DIR_COMMAND_FUNCLVL_GET:

            dwError = DirCliGetFuncLvl(pszServerName,
                                       pszUserName,
                                       pszPassword,
                                       pszDomainName,
                                       &dwFuncLvl);
            BAIL_ON_VMAFD_ERROR(dwError);

            fprintf(
                stdout,
                "Domain Functional Level: %d\n",
                dwFuncLvl);

            break;

        case DIR_COMMAND_FUNCLVL_SET:

            if (pszFuncLvl)
            {
                dwFuncLvl = atoi(pszFuncLvl);
            }

            dwError = DirCliSetFuncLvl(pszServerName,
                                       pszUserName,
                                       pszPassword,
                                       pszDomainName,
                                       &dwFuncLvl);
            BAIL_ON_VMAFD_ERROR(dwError);

            fprintf(
                stdout,
                "Domain Functional Level set to %d\n",
                dwFuncLvl);

            break;

        default:

            dwError = ERROR_INVALID_STATE;

            break;
    }

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszUserName);
    VMAFD_SAFE_FREE_MEMORY(pszPassword);
    VMAFD_SAFE_FREE_MEMORY(pszFuncLvl);

    return dwError;

error:

    goto cleanup;
}

DWORD
DirCliExecNodesVersionRequest(
    int   argc,
    char* argv[]
    )
{
    DWORD dwError = 0;
    DWORD iArg = 0;
    PSTR pszServerName = NULL;
    PSTR pszDomainName = NULL;
    PSTR pszUserName = NULL;
    PSTR pszPassword = NULL;
    PVMDIR_DC_VERSION_INFO pDCVerInfo = NULL;
    DWORD dwCnt = 0;

    typedef enum
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_SERVER_NAME,
        PARSE_MODE_DOMAIN_NAME,
        PARSE_MODE_USER_NAME,
        PARSE_MODE_PASSWORD,
        PARSE_MODE_NEW_LEVEL
    } PARSE_MODE;

    PARSE_MODE parseMode = PARSE_MODE_OPEN;

    for (iArg = 0; iArg < argc; iArg++)
    {
        PSTR pszArg = argv[iArg];
        switch (parseMode)
        {
        case PARSE_MODE_OPEN:
            if (!strcmp(pszArg, "--server-name"))
            {
                parseMode = PARSE_MODE_SERVER_NAME;
            }
            else if(!strcmp(pszArg, "--domain-name"))
            {
                parseMode = PARSE_MODE_DOMAIN_NAME;
            }
            else if(!strcmp(pszArg, "--login"))
            {
                parseMode = PARSE_MODE_USER_NAME;
            }
            else if(!strcmp(pszArg, "--password"))
            {
                parseMode = PARSE_MODE_PASSWORD;
            }
            else
            {
                dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                BAIL_ON_VMAFD_ERROR(dwError);
            }
            break;

        case PARSE_MODE_SERVER_NAME:

            if (pszServerName)
            {
                dwError = ERROR_LOCAL_OPTION_INVALID;
                BAIL_ON_VMAFD_ERROR(dwError);
            }

            pszServerName = pszArg;

            parseMode = PARSE_MODE_OPEN;

            break;
        case PARSE_MODE_DOMAIN_NAME:
            if (pszDomainName)
            {
                dwError = ERROR_LOCAL_OPTION_INVALID;
                BAIL_ON_VMAFD_ERROR(dwError);
            }

            pszDomainName = pszArg;

            parseMode = PARSE_MODE_OPEN;

            break;

        case PARSE_MODE_USER_NAME:
            if (pszUserName)
            {
                dwError = ERROR_LOCAL_OPTION_INVALID;
                BAIL_ON_VMAFD_ERROR(dwError);
            }

            dwError = VmAfdAllocateStringA(pszArg,
                                           &pszUserName);
            BAIL_ON_VMAFD_ERROR(dwError);

            parseMode = PARSE_MODE_OPEN;

            break;

        case PARSE_MODE_PASSWORD:
            if (pszPassword)
            {
                dwError = ERROR_LOCAL_OPTION_INVALID;
                BAIL_ON_VMAFD_ERROR(dwError);
            }

            dwError = VmAfdAllocateStringA(pszArg,
                                           &pszPassword);
            BAIL_ON_VMAFD_ERROR(dwError);

            parseMode = PARSE_MODE_OPEN;

            break;

        default:

            dwError = ERROR_LOCAL_OPTION_INVALID;
            BAIL_ON_VMAFD_ERROR(dwError);

            break;
        }
    }

    dwError = DirCliGetDCNodesVersion(
        pszServerName ,
        pszUserName ,
        pszPassword ,
        pszDomainName,
        &pDCVerInfo
        );
    BAIL_ON_VMAFD_ERROR(dwError);

    if(!pDCVerInfo)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    fprintf(
        stdout,
        "\nPSC Version list:\n");

    fprintf(
        stdout,
        "Max Domain Functional Level: %d\n\n",
        pDCVerInfo->dwMaxDomainFuncLvl
        );

    fprintf(
        stdout,
        "Server                               PSC Version\n");
    fprintf(
        stdout,
        "------                               -----------\n");

    for(dwCnt = 0; dwCnt < pDCVerInfo->dwSize; dwCnt++)
    {
        fprintf(
            stdout,
            "%-36s %s\n",
            VMAFD_SAFE_STRING(pDCVerInfo->ppszServer[dwCnt]),
            VMAFD_SAFE_STRING(pDCVerInfo->ppszVersion[dwCnt]));
    }

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszUserName);
    VMAFD_SAFE_FREE_MEMORY(pszPassword);
    VmDirFreeDCVersionInfo(pDCVerInfo);

    return dwError;

error:

    goto cleanup;
}


static
DWORD
DirCliExecTenantRequest(
    int   argc,
    char* argv[]
    )
{
    DWORD dwError = 0;
    DIR_COMMAND command = DIR_COMMAND_UNKNOWN;
    DWORD idx = 0;
    PSTR pszLogin = NULL;
    PSTR pszPassword = NULL;
    PSTR pszUserName = NULL;
    PSTR pszUserPassword = NULL;
    PSTR pszDomainName = NULL;

    typedef enum
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_TENANT_CREATE,
        PARSE_MODE_TENANT_DELETE,
        PARSE_MODE_TENANT_LIST,
    } PARSE_MODE;

    typedef enum
    {
        PARSE_SUB_MODE_OPEN = 0,
        PARSE_SUB_MODE_DOMAIN_NAME,
        PARSE_SUB_MODE_LOGIN,
        PARSE_SUB_MODE_PASSWORD,
        PARSE_SUB_MODE_USER_NAME,
        PARSE_SUB_MODE_USER_PASSWORD
    } PARSE_SUB_MODE;

    PARSE_MODE mode = PARSE_MODE_OPEN;
    PARSE_SUB_MODE submode = PARSE_SUB_MODE_OPEN;

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

                if (!VmAfdStringCompareA(pszArg, "create", TRUE))
                {
                    command = DIR_COMMAND_TENANT_CREATE;
                    mode = PARSE_MODE_TENANT_CREATE;
                }
                else if (!VmAfdStringCompareA(pszArg, "delete", TRUE))
                {
                    command = DIR_COMMAND_TENANT_DELETE;
                    mode = PARSE_MODE_TENANT_DELETE;
                }
                else if (!VmAfdStringCompareA(pszArg, "list", TRUE))
                {
                    command = DIR_COMMAND_TENANT_LIST;
                    mode = PARSE_MODE_TENANT_LIST;
                }
                else
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_TENANT_CREATE:
                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                        if (!VmAfdStringCompareA(pszArg, "--domain-name", TRUE))
                        {
                            submode = PARSE_SUB_MODE_DOMAIN_NAME;
                        }
                        else if(!strcmp(pszArg, "--login"))
                        {
                            submode = PARSE_SUB_MODE_LOGIN;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--user-name", TRUE))
                        {
                            submode = PARSE_SUB_MODE_USER_NAME;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--user-password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_USER_PASSWORD;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        break;

                    case PARSE_SUB_MODE_LOGIN:
                        if (pszLogin)
                        {
                            dwError = ERROR_LOCAL_OPTION_INVALID;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        pszLogin = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_PASSWORD:
                        if (pszPassword)
                        {
                            dwError = ERROR_LOCAL_OPTION_INVALID;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        pszPassword = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_DOMAIN_NAME:

                        if (pszDomainName)
                        {
                            dwError = ERROR_LOCAL_OPTION_INVALID;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }

                        pszDomainName = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_USER_NAME:

                        if (pszUserName)
                        {
                            dwError = ERROR_LOCAL_OPTION_INVALID;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        pszUserName = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_USER_PASSWORD:

                        if (pszUserPassword)
                        {
                            dwError = ERROR_LOCAL_OPTION_INVALID;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        pszUserPassword = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    default:

                        dwError = ERROR_INVALID_STATE;
                        BAIL_ON_VMAFD_ERROR(dwError);

                        break;
                }
                break;

            case PARSE_MODE_TENANT_DELETE:
                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:
                        if(!strcmp(pszArg, "--login"))
                        {
                            submode = PARSE_SUB_MODE_LOGIN;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--domain-name", TRUE))
                        {
                            submode = PARSE_SUB_MODE_DOMAIN_NAME;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        break;

                    case PARSE_SUB_MODE_LOGIN:
                        if (pszLogin)
                        {
                            dwError = ERROR_LOCAL_OPTION_INVALID;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        pszLogin = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_PASSWORD:
                        if (pszPassword)
                        {
                            dwError = ERROR_LOCAL_OPTION_INVALID;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        pszPassword = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_DOMAIN_NAME:

                        if (pszDomainName)
                        {
                            dwError = ERROR_LOCAL_OPTION_INVALID;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }

                        pszDomainName = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    default:

                        dwError = ERROR_INVALID_STATE;
                        BAIL_ON_VMAFD_ERROR(dwError);
                        break;
                }
                break;

            case PARSE_MODE_TENANT_LIST:
                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:
                        if(!strcmp(pszArg, "--login"))
                        {
                            submode = PARSE_SUB_MODE_LOGIN;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        break;

                    case PARSE_SUB_MODE_LOGIN:
                        if (pszLogin)
                        {
                            dwError = ERROR_LOCAL_OPTION_INVALID;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        pszLogin = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_PASSWORD:
                        if (pszPassword)
                        {
                            dwError = ERROR_LOCAL_OPTION_INVALID;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        pszPassword = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;
                    default:
                        dwError = ERROR_INVALID_STATE;
                        break;
            }
            break;

            default:
                dwError = ERROR_INVALID_STATE;
                break;
        }
    }

    switch (command)
    {
        case DIR_COMMAND_TENANT_CREATE:
            dwError = DirCliCreateTenant(
                        pszLogin,
                        pszPassword,
                        pszDomainName,
                        pszUserName,
                        pszUserPassword);
            BAIL_ON_VMAFD_ERROR(dwError);

            break;

        case DIR_COMMAND_TENANT_DELETE:
            dwError = DirCliDeleteTenant(pszLogin, pszPassword, pszDomainName);
            BAIL_ON_VMAFD_ERROR(dwError);
            break;

        case DIR_COMMAND_TENANT_LIST:
            dwError = DirCliEnumerateTenants(pszLogin, pszPassword);
            BAIL_ON_VMAFD_ERROR(dwError);
            break;

        default:
            dwError = ERROR_INVALID_STATE;
            break;
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
DirCliExecOrgunitRequest(
    int   argc,
    char* argv[]
    )
{
    DWORD dwError = 0;
    DIR_COMMAND command = DIR_COMMAND_UNKNOWN;
    DWORD idx = 0;
    PSTR pszLogin = NULL;
    PSTR pszPassword = NULL;
    PSTR pszOrgunit = NULL;
    PSTR pszParentDN = NULL;
    PSTR pszContainerDN = NULL;

    typedef enum
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_ORGUNIT_CREATE,
        PARSE_MODE_ORGUNIT_LIST,
    } PARSE_MODE;

    typedef enum
    {
        PARSE_SUB_MODE_OPEN = 0,
        PARSE_SUB_MODE_LOGIN,
        PARSE_SUB_MODE_PASSWORD,
        PARSE_SUB_MODE_ORGUNIT,
        PARSE_SUB_MODE_PARENT_DN,
        PARSE_SUB_MODE_CONTAINER_DN,
    } PARSE_SUB_MODE;

    PARSE_MODE mode = PARSE_MODE_OPEN;
    PARSE_SUB_MODE submode = PARSE_SUB_MODE_OPEN;

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

                if (!VmAfdStringCompareA(pszArg, "create", TRUE))
                {
                    command = DIR_COMMAND_ORGUNIT_CREATE;
                    mode = PARSE_MODE_ORGUNIT_CREATE;
                }
                else if (!VmAfdStringCompareA(pszArg, "list", TRUE))
                {
                    command = DIR_COMMAND_ORGUNIT_LIST;
                    mode = PARSE_MODE_ORGUNIT_LIST;
                }
                else
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_ORGUNIT_CREATE:
                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:

                        if (!VmAfdStringCompareA(pszArg, "--login", TRUE))
                        {
                            submode = PARSE_SUB_MODE_LOGIN;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--orgunit-name", TRUE))
                        {
                            submode = PARSE_SUB_MODE_ORGUNIT;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--parent-dn", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PARENT_DN;
                        }
                        else
                        {
                            dwError = ERROR_INVALID_PARAMETER;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        break;

                    case PARSE_SUB_MODE_LOGIN:
                        if (pszLogin)
                        {
                            dwError = ERROR_LOCAL_OPTION_INVALID;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        pszLogin = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_PASSWORD:
                        if (pszPassword)
                        {
                            dwError = ERROR_LOCAL_OPTION_INVALID;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        pszPassword = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_ORGUNIT:

                        if (pszOrgunit)
                        {
                            dwError = ERROR_LOCAL_OPTION_INVALID;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        pszOrgunit = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_PARENT_DN:

                        if (pszParentDN)
                        {
                            dwError = ERROR_LOCAL_OPTION_INVALID;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        pszParentDN = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    default:

                        dwError = ERROR_INVALID_STATE;
                        BAIL_ON_VMAFD_ERROR(dwError);

                        break;
                }
                break;

            case PARSE_MODE_ORGUNIT_LIST:
                switch (submode)
                {
                    case PARSE_SUB_MODE_OPEN:
                        if(!strcmp(pszArg, "--login"))
                        {
                            submode = PARSE_SUB_MODE_LOGIN;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--password", TRUE))
                        {
                            submode = PARSE_SUB_MODE_PASSWORD;
                        }
                        else if (!VmAfdStringCompareA(pszArg, "--container-dn", TRUE))
                        {
                            submode = PARSE_SUB_MODE_CONTAINER_DN;
                        }
                        break;

                    case PARSE_SUB_MODE_LOGIN:
                        if (pszLogin)
                        {
                            dwError = ERROR_LOCAL_OPTION_INVALID;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        pszLogin = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_PASSWORD:
                        if (pszPassword)
                        {
                            dwError = ERROR_LOCAL_OPTION_INVALID;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        pszPassword = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    case PARSE_SUB_MODE_CONTAINER_DN:
                        if (pszContainerDN)
                        {
                            dwError = ERROR_LOCAL_OPTION_INVALID;
                            BAIL_ON_VMAFD_ERROR(dwError);
                        }
                        pszContainerDN = pszArg;

                        submode = PARSE_SUB_MODE_OPEN;

                        break;

                    default:
                        dwError = ERROR_INVALID_STATE;
                        break;
            }
            break;

            default:
                dwError = ERROR_INVALID_STATE;
                break;
        }
    }

    switch (command)
    {
        case DIR_COMMAND_ORGUNIT_CREATE:
            dwError = DirCliCreateOrgunit(
                          pszLogin,
                          pszPassword,
                          pszOrgunit,
                          pszParentDN);
            BAIL_ON_VMAFD_ERROR(dwError);
            break;

        case DIR_COMMAND_ORGUNIT_LIST:
            dwError = DirCliEnumerateOrgunits(
                          pszLogin,
                          pszPassword,
                          pszContainerDN);
            BAIL_ON_VMAFD_ERROR(dwError);
            break;

        default:
            dwError = ERROR_INVALID_STATE;
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
        "Usage: dir-cli { arguments }\n\n"
        "Arguments:\n\n"
        "\tservice create --name <name>\n"
        "\t               --cert <path to cert file>\n"
        "\t             [ --ssogroups <','separated group names>]\n"
        "\t             [ --wstrustrole <ActAsUser>             ]\n"
        "\t             [ --ssoadminrole <Administrator/User>   ]\n"
        "\t             [ --login    <admin user id>            ]\n"
        "\t             [ --password <password>                 ]\n"
        "\tservice list\n"
        "\t             [ --login    <admin user id>            ]\n"
        "\t             [ --password <password>                 ]\n"
        "\tservice delete --name <name>\n"
        "\t             [ --login    <admin user id>            ]\n"
        "\t             [ --password <password>                 ]\n"
        "\tservice update --name <name>\n"
        "\t               --cert <path to cert file>\n"
        "\t             [ --login    <admin user id>            ]\n"
        "\t             [ --password <password>                 ]\n"
        "\tuser create --account <account name>\n"
        "\t            --user-password <password>\n"
        "\t            --first-name    <first name>\n"
        "\t            --last-name     <last name>\n"
        "\t             [ --login    <admin user id>            ]\n"
        "\t             [ --password <password>                 ]\n"
        "\tuser modify --account <account name>\n"
        "\t             [ --password-never-expires              ]\n"
        "\t             [ --password-expires                    ]\n"
        "\t             [ --login    <admin user id>            ]\n"
        "\t             [ --password <password>                 ]\n"
        "\tuser delete --account <account name>\n"
        "\t             [ --login    <admin user id>            ]\n"
        "\t             [ --password <password>                 ]\n"
        "\tuser find-by-name --account <account name>\n"
        "\t             [ --level    <info level 0|1|2>         ]\n"
        "\t             [ --login    <admin user id>            ]\n"
        "\t             [ --password <password>                 ]\n"
        "\tgroup modify --name <group name>\n"
        "\t             [ --add <user or group name>            ]\n"
        "\t             [ --login    <admin user id>            ]\n"
        "\t             [ --password <password>                 ]\n"
        "\tgroup list --name <group name>\n"
        "\t             [ --login    <admin user id>            ]\n"
        "\t             [ --password <password>                 ]\n"
        "\tssogroup create --name <name>\n"
        "\t             [ --description  <description>          ]\n"
        "\t             [ --login    <admin user id>            ]\n"
        "\t             [ --password <password>                 ]\n"
        "\ttrustedcert publish\n"
        "\t               --cert <path to cert file>\n"
        "\t             [ --crl  <path to crl file>             ]\n"
        "\t             [ --login    <admin user id>            ]\n"
        "\t             [ --password <password>                 ]\n"
        "\t             [ --chain                               ]\n"
        "\ttrustedcert publish-crl <path to crl file>\n"
        "\t               --crl <path to crl file>\n"
        "\t             [ --login    <admin user id>            ]\n"
        "\t             [ --password <password>                 ]\n"
        "\ttrustedcert unpublish\n"
        "\t               --cert <path to cert file>\n"
        "\t             [ --login    <admin user id>            ]\n"
        "\t             [ --password <password>                 ]\n"
        "\ttrustedcert list\n"
        "\t             [ --login    <admin user id>            ]\n"
        "\t             [ --password <password>                 ]\n"
        "\ttrustedcert get\n"
        "\t               --id <CN of CA cert from list command>\n"
        "\t             [--outcert <path to output cert file>\n"
        "\t             [--outcrl <path to output crl file>\n"
        "\t             [ --login    <admin user id>            ]\n"
        "\t             [ --password <password>                 ]\n"
        "\tpassword create\n"
        "\t             [ --login    <admin user id>            ]\n"
        "\t             [ --password <password>                 ]\n"
        "\tpassword reset --account  <account>\n"
        "\t             [ --new      <new password>             ]\n"
        "\t             [ --login    <admin user id>            ]\n"
        "\t             [ --password <password>                 ]\n"
        "\tpassword change --account <account>\n"
        "\t             [ --current  <current password>         ]\n"
        "\t             [ --new      <new password>             ]\n"
        "\tnodes list\n"
        "\t             [ --login    <admin user id>            ]\n"
        "\t             [ --password <password>                 ]\n"
        "\t             [ --server-name <server name>           ]\n"
        "\tdomain-functional-level get\n"
        "\t             [ --login       <admin user id>         ]\n"
        "\t             [ --password    <password>              ]\n"
        "\t             [ --server-name <server name>           ]\n"
        "\t             [ --domain-name <domain name>           ]\n"
        "\tdomain-functional-level set\n"
        "\t             [ --level       <level>                 ]\n"
        "\t             [ --login       <admin user id>         ]\n"
        "\t             [ --password    <password>              ]\n"
        "\t             [ --server-name <server name>           ]\n"
        "\t             [ --domain-name <domain name>           ]\n"
        "\tlist-domain-versions\n"
        "\t             [ --login       <admin user id>         ]\n"
        "\t             [ --password    <password>              ]\n"
        "\t             [ --server-name <server name>           ]\n"
        "\t             [ --domain-name <domain name>           ]\n"
        "\tcomputer password-reset\n"
        "\t             [ --login       <admin user id>         ]\n"
        "\t             [ --password    <password>              ]\n"
        "\t             [ --live-dc-hostname <server name>      ]\n"
        "\tstate get\n"
        "\t             [ --login       <admin user id>         ]\n"
        "\t             [ --password    <password>              ]\n"
        "\t             [ --server-name <server name>           ]\n"
        "\t             [ --domain-name <domain name>           ]\n"
        "\tstate set\n"
        "\t               --state  (NORMAL|READONLY|STANDALONE)  \n"
        "\t             [ --login       <admin user id>         ]\n"
        "\t             [ --password    <password>              ]\n"
        "\t             [ --server-name <server name>           ]\n"
        "\t             [ --domain-name <domain name>           ]\n"
        "\ttenant create\n"
        "\t             [ --login       <admin user id>         ]\n"
        "\t             [ --password    <password>              ]\n"
        "\t               --domain-name <domain name>            \n"
        "\t               --user-name   <user name>              \n"
        "\t               --user-password <user password>        \n"
        "\ttenant list\n"
        "\t             [ --login       <admin user id>         ]\n"
        "\t             [ --password    <password>              ]\n"
        "\ttenant delete\n"
        "\t             [ --login       <admin user id>         ]\n"
        "\t             [ --password    <password>              ]\n"
        "\t               --domain-name <domain name>            \n"
        "\torgunit create\n"
        "\t             [ --login       <admin user id>         ]\n"
        "\t             [ --password    <password>              ]\n"
        "\t               --orgunit-name <orgunit name>          \n"
        "\t             [ --parent-dn   <parent DN>             ]\n"
        "\torgunit list\n"
        "\t             [ --login       <admin user id>         ]\n"
        "\t             [ --password    <password>              ]\n"
        "\t             [ --container-dn <container DN>         ]\n"
        "\thelp\n");
}
