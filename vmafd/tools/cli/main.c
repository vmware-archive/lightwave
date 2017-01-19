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

#define VMAFD_MAX_PWD_LEN 128

static
DWORD
ParseArgs(
    int                  argc,
    char*                argv[],
    PVM_AFD_CLI_CONTEXT* ppContext
    );

static
DWORD
ParseArgsGetStatus(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsGetDomainName(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsSetDomainName(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsGetDomainState(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsGetLDU(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsSetLDU(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsGetRHTTPProxyPort(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsSetRHTTPProxyPort(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsSetDCPort(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsGetCMLocation(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsGetLSLocation(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsGetDCName(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsSetDCName(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsGetPNID(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsSetPNID(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsGetSiteGUID(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsGetSiteName(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsGetMachineID(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsSetMachineID(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );


static
DWORD
ParseArgsAddPasswordEntry(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsGetMachineAccountInfo(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsSetMachineAccountInfo(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsGetMachineSSLCertificates(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsSetMachineSSLCertificates(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsPromoteVmDir(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsDemoteVmDir(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsJoinVmDir(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsLeaveVmDir(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsJoinAD(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsLeaveAD(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsQueryAD(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsGetHbStatus(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsChangePNID(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
DWORD
ParseArgsGetDCList(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    );

static
void
ShowUsage(
    VOID
    );

static
DWORD
GetPasswordFromFile(PVM_AFD_CLI_CONTEXT pContext, PSTR passwordFile)
{
    DWORD dwError = 0;
    FILE *fpPwdFile = NULL;
    PSTR   pszPasswordBuf = NULL;

    fpPwdFile = fopen(passwordFile, "r");
    if (fpPwdFile == NULL)
    {
           dwError = ERROR_LOCAL_PASSWORDFILE_CANNOT_OPEN;
           BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(VMAFD_MAX_PWD_LEN+1, (PVOID *)&pszPasswordBuf);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!fread(pszPasswordBuf, VMAFD_MAX_PWD_LEN, 1, fpPwdFile))
    {
           dwError = ERROR_LOCAL_PASSWORDFILE_CANNOT_READ;
           BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdStringCpyA(pContext->pszPassword, VMAFD_MAX_PWD_LEN, pszPasswordBuf);
    BAIL_ON_VMAFD_ERROR(dwError);

error:
    VMAFD_SAFE_FREE_MEMORY(pszPasswordBuf);
    return dwError;
}

#ifndef _WIN32
int main(int argc, char* argv[])
#else
int _tmain(int argc, _TCHAR* targv[])
#endif
{
    DWORD dwError = 0;
    int retCode = 0;
    PVM_AFD_CLI_CONTEXT pContext = NULL;
    PCSTR pszErrorMsg = NULL;
    PSTR pszErrDesc = NULL;

#ifdef _WIN32

    char** allocArgv = NULL;
    PSTR* argv = NULL;
    char pathsep = '\\';

#ifdef UNICODE
    dwError = VmAfdAllocateArgsAFromArgsW( argc, targv, &allocArgv );
    BAIL_ON_VMAFD_ERROR(dwError);
    argv = allocArgv;
#else
    argv = targv; // non-unicode => targv is char
#endif

#else
    setlocale(LC_ALL, "");
#endif

    dwError = VmAfCfgInit();
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = ParseArgs(argc, argv, &pContext);
    if (dwError == ERROR_INVALID_PARAMETER ||
        dwError == ERROR_LOCAL_OPTION_UNKNOWN ||
        dwError == ERROR_LOCAL_OPTION_INVALID)
    {
        ShowUsage();
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdCliExecute(pContext);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_STRINGA(pszErrDesc);
    if (pContext)
    {
        VmAfdCliFreeContext(pContext);
    }

    return retCode;

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
        case ERROR_FILE_NOT_FOUND:
            retCode = 26;
            pszErrorMsg = "Object/Configuration not found.\nVerify configuration value has been set.";
            break;
        default:
            retCode = 1;
    }

    if (pszErrorMsg)
    {
        fprintf(stderr, "Error %u: %s\n", dwError, pszErrorMsg);
    }
    else
    {
        if (!VmAfdGetErrorString(dwError, &pszErrDesc))
        {
            fprintf(stderr, "Error %u: %s\n", dwError, pszErrDesc);
        }
        else
        {
            fprintf(stderr, "Error %u\n", dwError);
        }
    }
    goto cleanup;
}

static
DWORD
ParseArgs(
    int                  argc,
    char*                argv[],
    PVM_AFD_CLI_CONTEXT* ppContext
    )
{
    DWORD dwError = 0;
    PVM_AFD_CLI_CONTEXT pContext = NULL;
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

    dwError = VmAfdAllocateMemory(sizeof(*pContext), (PVOID*)&pContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!strcmp(pszArg, "help"))
    {
        dwError = ERROR_INVALID_PARAMETER;
    }

    if (!strcmp(pszArg, "get-status"))
    {
        pContext->action = VM_AFD_ACTION_GET_STATUS;

        dwError = ParseArgsGetStatus(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "get-domain-name"))
    {
        pContext->action = VM_AFD_ACTION_GET_DOMAIN_NAME;

        dwError = ParseArgsGetDomainName(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "set-domain-name"))
    {
        pContext->action = VM_AFD_ACTION_SET_DOMAIN_NAME;

        dwError = ParseArgsSetDomainName(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "get-domain-state"))
    {
        pContext->action = VM_AFD_ACTION_GET_DOMAIN_STATE;

        dwError = ParseArgsGetDomainState(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "get-ldu"))
    {
        pContext->action = VM_AFD_ACTION_GET_LDU;

        dwError = ParseArgsGetLDU(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "set-ldu"))
    {
        pContext->action = VM_AFD_ACTION_SET_LDU;

        dwError = ParseArgsSetLDU(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "get-rhttpproxy-port"))
    {
        pContext->action = VM_AFD_ACTION_GET_RHTTPPROXY_PORT;

        dwError = ParseArgsGetRHTTPProxyPort(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "set-rhttpproxy-port"))
    {
        pContext->action = VM_AFD_ACTION_SET_RHTTPPROXY_PORT;

        dwError = ParseArgsSetRHTTPProxyPort(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "set-dc-port"))
    {
        pContext->action = VM_AFD_ACTION_SET_DC_PORT;

        dwError = ParseArgsSetDCPort(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "get-cm-location"))
    {
        pContext->action = VM_AFD_ACTION_GET_CM_LOCATION;

        dwError = ParseArgsGetCMLocation(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "get-ls-location"))
    {
        pContext->action = VM_AFD_ACTION_GET_LS_LOCATION;

        dwError = ParseArgsGetLSLocation(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "get-dc-name"))
    {
        pContext->action = VM_AFD_ACTION_GET_DC_NAME;

        dwError = ParseArgsGetDCName(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "set-dc-name"))
    {
        pContext->action = VM_AFD_ACTION_SET_DC_NAME;

        dwError = ParseArgsSetDCName(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "get-pnid"))
    {
        pContext->action = VM_AFD_ACTION_GET_PNID;

        dwError = ParseArgsGetPNID(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "get-pnid-url"))
    {
        pContext->action = VM_AFD_ACTION_GET_PNID_URL;

        dwError = ParseArgsGetPNID(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "set-pnid"))
    {
        pContext->action = VM_AFD_ACTION_SET_PNID;

        dwError = ParseArgsSetPNID(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "get-site-guid"))
    {
        pContext->action = VM_AFD_ACTION_GET_SITE_GUID;

        dwError = ParseArgsGetSiteGUID(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "get-site-name"))
    {
        pContext->action = VM_AFD_ACTION_GET_SITE_NAME;

        dwError = ParseArgsGetSiteName(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "refresh-site-name"))
    {
        pContext->action = VM_AFD_ACTION_REFRESH_SITE_NAME;
    }
    else if (!strcmp(pszArg, "get-machine-id"))
    {
        pContext->action = VM_AFD_ACTION_GET_MACHINE_ID;

        dwError = ParseArgsGetMachineID(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "set-machine-id"))
    {
        pContext->action = VM_AFD_ACTION_SET_MACHINE_ID;

        dwError = ParseArgsSetMachineID(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "add-password-entry"))
    {
        pContext->action = VM_AFD_ACTION_ADD_PASSWORD_ENTRY;

        dwError = ParseArgsAddPasswordEntry(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "get-machine-account-info"))
    {
        pContext->action = VM_AFD_ACTION_GET_MACHINE_ACCOUNT_INFO;

        dwError = ParseArgsGetMachineAccountInfo(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "set-machine-account-info"))
    {
        pContext->action = VM_AFD_ACTION_SET_MACHINE_ACCOUNT_INFO;

        dwError = ParseArgsSetMachineAccountInfo(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "get-machine-ssl-certificate"))
    {
        pContext->action = VM_AFD_ACTION_GET_MACHINE_SSL_CERTIFICATES;

        dwError = ParseArgsGetMachineSSLCertificates(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "set-machine-ssl-certificate"))
    {
        pContext->action = VM_AFD_ACTION_SET_MACHINE_SSL_CERTIFICATES;

        dwError = ParseArgsSetMachineSSLCertificates(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "promote-vmdir"))
    {
        pContext->action = VM_AFD_ACTION_PROMOTE_VM_DIR;

        dwError = ParseArgsPromoteVmDir(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "demote-vmdir"))
    {
        pContext->action = VM_AFD_ACTION_DEMOTE_VM_DIR;

        dwError = ParseArgsDemoteVmDir(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "join-vmdir"))
    {
        pContext->action = VM_AFD_ACTION_JOIN_VM_DIR;

        dwError = ParseArgsJoinVmDir(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "leave-vmdir"))
    {
        pContext->action = VM_AFD_ACTION_LEAVE_VM_DIR;

        dwError = ParseArgsLeaveVmDir(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "join-ad"))
    {
        pContext->action = VM_AFD_ACTION_JOIN_AD;

        dwError = ParseArgsJoinAD(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "leave-ad"))
    {
        pContext->action = VM_AFD_ACTION_LEAVE_AD;

        dwError = ParseArgsLeaveAD(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "query-ad"))
    {
        pContext->action = VM_AFD_ACTION_QUERY_AD;

        dwError = ParseArgsQueryAD(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "get-heartbeat-status"))
    {
        pContext->action = VM_AFD_ACTION_GET_HEARTBEAT_STATUS;

        dwError = ParseArgsGetHbStatus(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "change-pnid"))
    {
        pContext->action = VM_AFD_ACTION_CHANGE_PNID;

        dwError = ParseArgsChangePNID(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
    }
    else if (!strcmp(pszArg, "get-dc-list"))
    {
        pContext->action = VM_AFD_ACTION_GET_DC_LIST;

        dwError = ParseArgsGetDCList(
                        dwArgsLeft,
                        dwArgsLeft > 0 ? &argv[iArg] : NULL,
                        pContext);
                        }
    else
    {
        dwError = ERROR_LOCAL_OPTION_UNKNOWN;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppContext = pContext;

cleanup:

    return dwError;

error:

    *ppContext = NULL;

    if (pContext)
    {
        VmAfdCliFreeContext(pContext);
    }

    goto cleanup;
}

static
DWORD
ParseArgsGetStatus(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_GET_STATUS_OPEN = 0,
        PARSE_MODE_GET_STATUS_SERVER_NAME,
    } PARSE_MODE_GET_STATUS;
    PARSE_MODE_GET_STATUS parseMode = PARSE_MODE_GET_STATUS_OPEN;
    DWORD iArg = 0;

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
            case PARSE_MODE_GET_STATUS_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseMode = PARSE_MODE_GET_STATUS_SERVER_NAME;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_GET_STATUS_SERVER_NAME:

                if (pContext->pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszServerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_GET_STATUS_OPEN;

                break;

            default:

                dwError = ERROR_LOCAL_OPTION_INVALID;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

    if (!pContext->pszServerName)
    {
        dwError = ERROR_LOCAL_OPTION_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
ParseArgsGetDomainName(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_GET_DOMAIN_NAME_OPEN = 0,
        PARSE_MODE_GET_DOMAIN_NAME_SERVER_NAME,
    } PARSE_MODE_GET_DOMAIN_NAME;
    PARSE_MODE_GET_DOMAIN_NAME parseMode = PARSE_MODE_GET_DOMAIN_NAME_OPEN;
    DWORD iArg = 0;

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
            case PARSE_MODE_GET_DOMAIN_NAME_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseMode = PARSE_MODE_GET_DOMAIN_NAME_SERVER_NAME;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_GET_DOMAIN_NAME_SERVER_NAME:

                if (pContext->pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszServerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_GET_DOMAIN_NAME_OPEN;

                break;

            default:

                dwError = ERROR_LOCAL_OPTION_INVALID;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

    if (!pContext->pszServerName)
    {
        dwError = ERROR_LOCAL_OPTION_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
ParseArgsGetDomainState(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_GET_DOMAIN_STATE_OPEN = 0,
        PARSE_MODE_GET_DOMAIN_STATE_SERVER_NAME,
    } PARSE_MODE_GET_DOMAIN_STATE;
    PARSE_MODE_GET_DOMAIN_STATE parseMode = PARSE_MODE_GET_DOMAIN_STATE_OPEN;
    DWORD iArg = 0;

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
            case PARSE_MODE_GET_DOMAIN_STATE_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseMode = PARSE_MODE_GET_DOMAIN_STATE_SERVER_NAME;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_GET_DOMAIN_STATE_SERVER_NAME:

                if (pContext->pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszServerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_GET_DOMAIN_STATE_OPEN;

                break;

            default:

                dwError = ERROR_LOCAL_OPTION_INVALID;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

    if (!pContext->pszServerName)
    {
        dwError = ERROR_LOCAL_OPTION_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
ParseArgsSetDomainName(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_SET_DOMAIN_NAME_OPEN = 0,
        PARSE_MODE_SET_DOMAIN_NAME_SERVER_NAME,
        PARSE_MODE_SET_DOMAIN_NAME_DOMAIN_NAME,
    } PARSE_MODE_SET_DOMAIN_NAME;
    PARSE_MODE_SET_DOMAIN_NAME parseMode = PARSE_MODE_SET_DOMAIN_NAME_OPEN;
    DWORD iArg = 0;

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
            case PARSE_MODE_SET_DOMAIN_NAME_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseMode = PARSE_MODE_SET_DOMAIN_NAME_SERVER_NAME;
                }
                else if (!strcmp(pszArg, "--domain-name"))
                {
                    parseMode = PARSE_MODE_SET_DOMAIN_NAME_DOMAIN_NAME;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_SET_DOMAIN_NAME_SERVER_NAME:

                if (pContext->pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszServerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_SET_DOMAIN_NAME_OPEN;

                break;

            case PARSE_MODE_SET_DOMAIN_NAME_DOMAIN_NAME:

                if (pContext->pszDomainName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszDomainName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_SET_DOMAIN_NAME_OPEN;

                break;

            default:

                dwError = ERROR_LOCAL_OPTION_INVALID;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

    if (!pContext->pszServerName ||
        !pContext->pszDomainName)
    {
        dwError = ERROR_LOCAL_OPTION_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
ParseArgsGetLDU(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_GET_LDU_OPEN = 0,
        PARSE_MODE_GET_LDU_SERVER_NAME,
    } PARSE_MODE_GET_LDU;
    PARSE_MODE_GET_LDU parseMode = PARSE_MODE_GET_LDU_OPEN;
    DWORD iArg = 0;

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
            case PARSE_MODE_GET_LDU_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseMode = PARSE_MODE_GET_LDU_SERVER_NAME;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_GET_LDU_SERVER_NAME:

                if (pContext->pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszServerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_GET_LDU_OPEN;

                break;

            default:

                dwError = ERROR_LOCAL_OPTION_INVALID;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

    if (!pContext->pszServerName)
    {
        dwError = ERROR_LOCAL_OPTION_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
ParseArgsSetLDU(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_SET_LDU_OPEN = 0,
        PARSE_MODE_SET_LDU_SERVER_NAME,
        PARSE_MODE_SET_LDU_LDU,
    } PARSE_MODE_SET_LDU;
    PARSE_MODE_SET_LDU parseMode = PARSE_MODE_SET_LDU_OPEN;
    DWORD iArg = 0;

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
            case PARSE_MODE_SET_LDU_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseMode = PARSE_MODE_SET_LDU_SERVER_NAME;
                }
                else if (!strcmp(pszArg, "--ldu"))
                {
                    parseMode = PARSE_MODE_SET_LDU_LDU;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_SET_LDU_SERVER_NAME:

                if (pContext->pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszServerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_SET_LDU_OPEN;

                break;

            case PARSE_MODE_SET_LDU_LDU:

                if (pContext->pszLDU)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszLDU);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_SET_LDU_OPEN;

                break;

            default:

                dwError = ERROR_INTERNAL_ERROR;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

    if (!pContext->pszServerName ||
        !pContext->pszLDU)
    {
        dwError = ERROR_LOCAL_OPTION_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
ParseArgsGetCMLocation(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_GET_CM_LOCATION_OPEN = 0,
        PARSE_MODE_GET_CM_LOCATION_SERVER_NAME,
    } PARSE_MODE_GET_CM_LOCATION;
    PARSE_MODE_GET_CM_LOCATION parseMode = PARSE_MODE_GET_CM_LOCATION_OPEN;
    DWORD iArg = 0;

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
            case PARSE_MODE_GET_CM_LOCATION_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseMode = PARSE_MODE_GET_CM_LOCATION_SERVER_NAME;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_GET_CM_LOCATION_SERVER_NAME:

                if (pContext->pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszServerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_GET_CM_LOCATION_OPEN;

                break;

            default:

                dwError = ERROR_INTERNAL_ERROR;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

    if (!pContext->pszServerName)
    {
        dwError = ERROR_LOCAL_OPTION_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
ParseArgsGetLSLocation(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_GET_LS_LOCATION_OPEN = 0,
        PARSE_MODE_GET_LS_LOCATION_SERVER_NAME,
    } PARSE_MODE_GET_LS_LOCATION;
    PARSE_MODE_GET_LS_LOCATION parseMode = PARSE_MODE_GET_LS_LOCATION_OPEN;
    DWORD iArg = 0;

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
            case PARSE_MODE_GET_LS_LOCATION_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseMode = PARSE_MODE_GET_LS_LOCATION_SERVER_NAME;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_GET_LS_LOCATION_SERVER_NAME:

                if (pContext->pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszServerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_GET_LS_LOCATION_OPEN;

                break;

            default:

                dwError = ERROR_INTERNAL_ERROR;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

    if (!pContext->pszServerName)
    {
        dwError = ERROR_LOCAL_OPTION_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
ParseArgsGetRHTTPProxyPort(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_GET_STATUS_OPEN = 0,
        PARSE_MODE_GET_STATUS_SERVER_NAME,
    } PARSE_MODE_GET_STATUS;
    PARSE_MODE_GET_STATUS parseMode = PARSE_MODE_GET_STATUS_OPEN;
    DWORD iArg = 0;

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
            case PARSE_MODE_GET_STATUS_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseMode = PARSE_MODE_GET_STATUS_SERVER_NAME;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_GET_STATUS_SERVER_NAME:

                if (pContext->pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszServerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_GET_STATUS_OPEN;

                break;

            default:

                dwError = ERROR_LOCAL_OPTION_INVALID;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

    if (!pContext->pszServerName)
    {
        dwError = ERROR_LOCAL_OPTION_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
ParseArgsSetRHTTPProxyPort(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_SET_RHTTPPROXY_PORT_OPEN = 0,
        PARSE_MODE_SET_RHTTPPROXY_PORT_SERVER_NAME,
        PARSE_MODE_SET_RHTTPPROXY_PORT_RHTTPPROXY_PORT,
    } PARSE_MODE_SET_RHTTPPROXY_PORT;
    PARSE_MODE_SET_RHTTPPROXY_PORT parseMode = PARSE_MODE_SET_RHTTPPROXY_PORT_OPEN;
    DWORD iArg = 0;

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
            case PARSE_MODE_SET_RHTTPPROXY_PORT_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseMode = PARSE_MODE_SET_RHTTPPROXY_PORT_SERVER_NAME;
                }
                else if (!strcmp(pszArg, "--rhttpproxy-port"))
                {
                    parseMode = PARSE_MODE_SET_RHTTPPROXY_PORT_RHTTPPROXY_PORT;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_SET_RHTTPPROXY_PORT_SERVER_NAME:

                if (pContext->pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszServerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_SET_RHTTPPROXY_PORT_OPEN;

                break;

            case PARSE_MODE_SET_RHTTPPROXY_PORT_RHTTPPROXY_PORT:

                pContext->dwPort = atoi(pszArg);

                parseMode = PARSE_MODE_SET_RHTTPPROXY_PORT_OPEN;

                break;

            default:

                dwError = ERROR_INTERNAL_ERROR;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

    if (!pContext->dwPort)
    {
        dwError = ERROR_LOCAL_OPTION_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
ParseArgsSetDCPort(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_SET_DC_PORT_OPEN = 0,
        PARSE_MODE_SET_DC_PORT_SERVER_NAME,
        PARSE_MODE_SET_DC_PORT_DC_PORT,
    } PARSE_MODE_SET_DC_PORT;
    PARSE_MODE_SET_DC_PORT parseMode = PARSE_MODE_SET_DC_PORT_OPEN;
    DWORD iArg = 0;

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
            case PARSE_MODE_SET_DC_PORT_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseMode = PARSE_MODE_SET_DC_PORT_SERVER_NAME;
                }
                else if (!strcmp(pszArg, "--dc-port"))
                {
                    parseMode = PARSE_MODE_SET_DC_PORT_DC_PORT;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_SET_DC_PORT_SERVER_NAME:

                if (pContext->pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszServerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_SET_DC_PORT_OPEN;

                break;

            case PARSE_MODE_SET_DC_PORT_DC_PORT:

                pContext->dwPort = atoi(pszArg);

                parseMode = PARSE_MODE_SET_DC_PORT_OPEN;

                break;

            default:

                dwError = ERROR_INTERNAL_ERROR;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

    if (!pContext->dwPort)
    {
        dwError = ERROR_LOCAL_OPTION_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
ParseArgsGetPNID(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_GET_PNID_OPEN = 0,
        PARSE_MODE_GET_PNID_SERVER_NAME,
    } PARSE_MODE_GET_PNID;
    PARSE_MODE_GET_PNID parseMode = PARSE_MODE_GET_PNID_OPEN;
    DWORD iArg = 0;

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
            case PARSE_MODE_GET_PNID_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseMode = PARSE_MODE_GET_PNID_SERVER_NAME;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_GET_PNID_SERVER_NAME:

                if (pContext->pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszServerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_GET_PNID_OPEN;

                break;

            default:

                dwError = ERROR_INTERNAL_ERROR;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

    if (!pContext->pszServerName)
    {
        dwError = ERROR_LOCAL_OPTION_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
ParseArgsSetPNID(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_SET_PNID_OPEN = 0,
        PARSE_MODE_SET_PNID_SERVER_NAME,
        PARSE_MODE_SET_PNID_PNID,
    } PARSE_MODE_SET_PNID;
    PARSE_MODE_SET_PNID parseMode = PARSE_MODE_SET_PNID_OPEN;
    DWORD iArg = 0;

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
            case PARSE_MODE_SET_PNID_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseMode = PARSE_MODE_SET_PNID_SERVER_NAME;
                }
                else if (!strcmp(pszArg, "--pnid"))
                {
                    parseMode = PARSE_MODE_SET_PNID_PNID;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_SET_PNID_SERVER_NAME:

                if (pContext->pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszServerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_SET_PNID_OPEN;

                break;

            case PARSE_MODE_SET_PNID_PNID:

                if (pContext->pszPNID)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszPNID);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_SET_PNID_OPEN;

                break;

            default:

                dwError = ERROR_INTERNAL_ERROR;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

    if (!pContext->pszServerName ||
        !pContext->pszPNID)
    {
        dwError = ERROR_LOCAL_OPTION_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
ParseArgsGetSiteGUID(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_GET_SITE_GUID_OPEN = 0,
        PARSE_MODE_GET_SITE_GUID_SERVER_NAME,
    } PARSE_MODE_GET_SITE_GUID;
    PARSE_MODE_GET_SITE_GUID parseMode = PARSE_MODE_GET_SITE_GUID_OPEN;
    DWORD iArg = 0;

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
            case PARSE_MODE_GET_SITE_GUID_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseMode = PARSE_MODE_GET_SITE_GUID_SERVER_NAME;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_GET_SITE_GUID_SERVER_NAME:

                if (pContext->pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszServerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_GET_SITE_GUID_OPEN;

                break;

            default:

                dwError = ERROR_INTERNAL_ERROR;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

    if (!pContext->pszServerName)
    {
        dwError = ERROR_LOCAL_OPTION_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
ParseArgsGetSiteName(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_GET_SITE_NAME_OPEN = 0,
        PARSE_MODE_GET_SITE_NAME_SERVER_NAME,
    } PARSE_MODE_GET_SITE_NAME;
    PARSE_MODE_GET_SITE_NAME parseMode = PARSE_MODE_GET_SITE_NAME_OPEN;
    DWORD iArg = 0;

    for (iArg = 0; iArg < argc; iArg++)
    {
        PSTR pszArg = argv[iArg];

        switch (parseMode)
        {
            case PARSE_MODE_GET_SITE_NAME_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseMode = PARSE_MODE_GET_SITE_NAME_SERVER_NAME;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_GET_SITE_NAME_SERVER_NAME:

                if (pContext->pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszServerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_GET_SITE_NAME_OPEN;

                break;

            default:

                dwError = ERROR_INTERNAL_ERROR;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

    if (!pContext->pszServerName)
    {
        dwError = VmAfdAllocateStringA("localhost", &pContext->pszServerName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
ParseArgsGetMachineID(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_GET_MACHINE_ID_OPEN = 0,
        PARSE_MODE_GET_MACHINE_ID_SERVER_NAME,
    } PARSE_MODE_GET_MACHINE_ID;
    PARSE_MODE_GET_MACHINE_ID parseMode = PARSE_MODE_GET_MACHINE_ID_OPEN;
    DWORD iArg = 0;

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
            case PARSE_MODE_GET_MACHINE_ID_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseMode = PARSE_MODE_GET_MACHINE_ID_SERVER_NAME;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_GET_MACHINE_ID_SERVER_NAME:

                if (pContext->pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszServerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_GET_MACHINE_ID_OPEN;

                break;

            default:

                dwError = ERROR_INTERNAL_ERROR;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

    if (!pContext->pszServerName)
    {
        dwError = ERROR_LOCAL_OPTION_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
ParseArgsSetMachineID(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_SET_MACHINE_ID_OPEN = 0,
        PARSE_MODE_SET_MACHINE_ID_VALUE,
        PARSE_MODE_SET_MACHINE_ID_SERVER_NAME,
    } PARSE_MODE_SET_MACHINE_ID;
    PARSE_MODE_SET_MACHINE_ID parseMode = PARSE_MODE_SET_MACHINE_ID_OPEN;
    DWORD iArg = 0;

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
            case PARSE_MODE_SET_MACHINE_ID_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseMode = PARSE_MODE_SET_MACHINE_ID_SERVER_NAME;
                }
                else if (!strcmp(pszArg, "--id"))
                {
                    parseMode = PARSE_MODE_SET_MACHINE_ID_VALUE;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_SET_MACHINE_ID_SERVER_NAME:

                if (pContext->pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszServerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_SET_MACHINE_ID_OPEN;

                break;

            case PARSE_MODE_SET_MACHINE_ID_VALUE:

                if (pContext->pszMachineId)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszMachineId);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_SET_MACHINE_ID_OPEN;

                break;

            default:

                dwError = ERROR_INTERNAL_ERROR;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

    if (!pContext->pszServerName || IsNullOrEmptyString(pContext->pszMachineId))
    {
        dwError = ERROR_LOCAL_OPTION_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
ParseArgsPromoteVmDir(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_PROMOTE_VM_DIR_OPEN = 0,
        PARSE_MODE_PROMOTE_VM_DIR_SERVER_NAME,
        PARSE_MODE_PROMOTE_VM_DIR_DOMAIN_NAME,
        PARSE_MODE_PROMOTE_VM_DIR_USER_NAME,
        PARSE_MODE_PROMOTE_VM_DIR_PASSWORD,
        PARSE_MODE_PROMOTE_VM_DIR_SITE_NAME,
        PARSE_MODE_PROMOTE_VM_DIR_PARTNER_NAME,
    } PARSE_MODE_PROMOTE_VM_DIR;
    PARSE_MODE_PROMOTE_VM_DIR parseMode = PARSE_MODE_PROMOTE_VM_DIR_OPEN;
    DWORD iArg = 0;

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
            case PARSE_MODE_PROMOTE_VM_DIR_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseMode = PARSE_MODE_PROMOTE_VM_DIR_SERVER_NAME;
                }
                else if (!strcmp(pszArg, "--domain-name"))
                {
                    parseMode = PARSE_MODE_PROMOTE_VM_DIR_DOMAIN_NAME;
                }
                else if (!strcmp(pszArg, "--user-name"))
                {
                    parseMode = PARSE_MODE_PROMOTE_VM_DIR_USER_NAME;
                }
                else if (!strcmp(pszArg, "--password"))
                {
                    parseMode = PARSE_MODE_PROMOTE_VM_DIR_PASSWORD;
                }
                else if (!strcmp(pszArg, "--site-name"))
                {
                    parseMode = PARSE_MODE_PROMOTE_VM_DIR_SITE_NAME;
                }
                else if (!strcmp(pszArg, "--partner-name"))
                {
                    parseMode = PARSE_MODE_PROMOTE_VM_DIR_PARTNER_NAME;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_PROMOTE_VM_DIR_SERVER_NAME:

                if (pContext->pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszServerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_PROMOTE_VM_DIR_OPEN;

                break;

            case PARSE_MODE_PROMOTE_VM_DIR_DOMAIN_NAME:

                if (pContext->pszDomainName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszDomainName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_PROMOTE_VM_DIR_OPEN;

                break;

            case PARSE_MODE_PROMOTE_VM_DIR_USER_NAME:

                if (pContext->pszUserName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszUserName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_PROMOTE_VM_DIR_OPEN;

                break;

            case PARSE_MODE_PROMOTE_VM_DIR_PASSWORD:

                if (pContext->pszPassword)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszPassword);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_PROMOTE_VM_DIR_OPEN;

                break;

            case PARSE_MODE_PROMOTE_VM_DIR_SITE_NAME:

                if (pContext->pszSiteName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszSiteName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_PROMOTE_VM_DIR_OPEN;

                break;

            case PARSE_MODE_PROMOTE_VM_DIR_PARTNER_NAME:

                if (pContext->pszPartnerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszPartnerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_PROMOTE_VM_DIR_OPEN;

                break;


            default:

                dwError = ERROR_INTERNAL_ERROR;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

    if (!pContext->pszServerName ||
        !pContext->pszUserName ||
        !pContext->pszPassword)
    {
        dwError = ERROR_LOCAL_OPTION_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
ParseArgsDemoteVmDir(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_DEMOTE_VM_DIR_OPEN = 0,
        PARSE_MODE_DEMOTE_VM_DIR_USER_NAME,
        PARSE_MODE_DEMOTE_VM_DIR_PASSWORD,
    } PARSE_MODE_DEMOTE_VM_DIR;
    PARSE_MODE_DEMOTE_VM_DIR parseMode = PARSE_MODE_DEMOTE_VM_DIR_OPEN;
    DWORD iArg = 0;

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
            case PARSE_MODE_DEMOTE_VM_DIR_OPEN:
		if (!strcmp(pszArg, "--user-name"))
                {
                    parseMode = PARSE_MODE_DEMOTE_VM_DIR_USER_NAME;
                }
                else if (!strcmp(pszArg, "--password"))
                {
                    parseMode = PARSE_MODE_DEMOTE_VM_DIR_PASSWORD;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_DEMOTE_VM_DIR_USER_NAME:

                if (pContext->pszUserName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszUserName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_DEMOTE_VM_DIR_OPEN;

                break;

            case PARSE_MODE_DEMOTE_VM_DIR_PASSWORD:

                if (pContext->pszPassword)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszPassword);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_DEMOTE_VM_DIR_OPEN;

                break;

            default:

                dwError = ERROR_INTERNAL_ERROR;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

    if (!pContext->pszUserName ||
	!pContext->pszPassword)
    {
        dwError = ERROR_LOCAL_OPTION_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
ParseArgsJoinVmDir(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_JOIN_VM_DIR_OPEN = 0,
        PARSE_MODE_JOIN_VM_DIR_SERVER_NAME,
        PARSE_MODE_JOIN_VM_DIR_USER_NAME,
        PARSE_MODE_JOIN_VM_DIR_PASSWORD,
        PARSE_MODE_JOIN_VM_DIR_MACHINE_NAME,
        PARSE_MODE_JOIN_VM_DIR_DOMAIN_NAME,
        PARSE_MODE_JOIN_VM_DIR_ORG_UNIT,
        PARSE_MODE_JOIN_VM_DIR_PASSWORD_FILE,
    } PARSE_MODE_JOIN_VM_DIR;
    PARSE_MODE_JOIN_VM_DIR parseMode = PARSE_MODE_JOIN_VM_DIR_OPEN;
    DWORD iArg = 0;
    PSTR pszPassword = NULL;
    PSTR pszPassword_file = NULL;

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
            case PARSE_MODE_JOIN_VM_DIR_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseMode = PARSE_MODE_JOIN_VM_DIR_SERVER_NAME;
                }
                else if (!strcmp(pszArg, "--user-name"))
                {
                    parseMode = PARSE_MODE_JOIN_VM_DIR_USER_NAME;
                }
                else if (!strcmp(pszArg, "--password"))
                {
                    parseMode = PARSE_MODE_JOIN_VM_DIR_PASSWORD;
                }
                else if (!strcmp(pszArg, "--password-file"))
                {
                    parseMode = PARSE_MODE_JOIN_VM_DIR_PASSWORD_FILE;
                }
                else if (!strcmp(pszArg, "--machine-name"))
                {
                    parseMode = PARSE_MODE_JOIN_VM_DIR_MACHINE_NAME;
                }
                else if (!strcmp(pszArg, "--domain-name"))
                {
                    parseMode = PARSE_MODE_JOIN_VM_DIR_DOMAIN_NAME;
                }
                else if (!strcmp(pszArg, "--org-unit"))
                {
                    parseMode = PARSE_MODE_JOIN_VM_DIR_ORG_UNIT;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_JOIN_VM_DIR_SERVER_NAME:

                if (pContext->pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszServerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_JOIN_VM_DIR_OPEN;

                break;

            case PARSE_MODE_JOIN_VM_DIR_MACHINE_NAME:

                if (pContext->pszMachineName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszMachineName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_JOIN_VM_DIR_OPEN;

                break;

            case PARSE_MODE_JOIN_VM_DIR_DOMAIN_NAME:

                if (pContext->pszDomainName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszDomainName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_JOIN_VM_DIR_OPEN;

                break;

            case PARSE_MODE_JOIN_VM_DIR_ORG_UNIT:

                if (pContext->pszOrgUnit)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszOrgUnit);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_JOIN_VM_DIR_OPEN;

                break;

            case PARSE_MODE_JOIN_VM_DIR_USER_NAME:

                if (pContext->pszUserName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszUserName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_JOIN_VM_DIR_OPEN;

                break;

            case PARSE_MODE_JOIN_VM_DIR_PASSWORD:

                if (pContext->pszPassword)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pszPassword);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_JOIN_VM_DIR_OPEN;

                break;

            case PARSE_MODE_JOIN_VM_DIR_PASSWORD_FILE:

                if (pszPassword)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pszPassword_file);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_JOIN_VM_DIR_OPEN;

                break;

            default:

                dwError = ERROR_INTERNAL_ERROR;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

    if (!pContext->pszServerName ||
        !pContext->pszDomainName ||
        !pContext->pszUserName ||
        (pszPassword && pszPassword_file))
    {
        dwError = ERROR_LOCAL_OPTION_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(VMAFD_MAX_PWD_LEN+1, (PVOID *)&pContext->pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!pszPassword && pszPassword_file) {
       dwError = GetPasswordFromFile(pContext, pszPassword_file);
       BAIL_ON_VMAFD_ERROR(dwError);
    } else if (pszPassword && !pszPassword_file) {
       dwError = VmAfdStringCpyA(pContext->pszPassword, VMAFD_MAX_PWD_LEN, pszPassword);
       BAIL_ON_VMAFD_ERROR(dwError);
    } else //no password nor password-file, read password from stdin
    {
       VmAfdReadString("password: ", pContext->pszPassword, VMAFD_MAX_PWD_LEN+1, FALSE);
    }

error:

    return dwError;
}

static
DWORD
ParseArgsLeaveVmDir(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_LEAVE_VM_DIR_OPEN = 0,
        PARSE_MODE_LEAVE_VM_DIR_SERVER_NAME,
        PARSE_MODE_LEAVE_VM_DIR_USER_NAME,
        PARSE_MODE_LEAVE_VM_DIR_PASSWORD,
    } PARSE_MODE_LEAVE_VM_DIR;
    PARSE_MODE_LEAVE_VM_DIR parseMode = PARSE_MODE_LEAVE_VM_DIR_OPEN;
    DWORD iArg = 0;

    for (iArg = 0; iArg < argc; iArg++)
    {
        PSTR pszArg = argv[iArg];

        switch (parseMode)
        {
            case PARSE_MODE_LEAVE_VM_DIR_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseMode = PARSE_MODE_LEAVE_VM_DIR_SERVER_NAME;
                }
                else if (!strcmp(pszArg, "--user-name"))
                {
                    parseMode = PARSE_MODE_LEAVE_VM_DIR_USER_NAME;
                }
                else if (!strcmp(pszArg, "--password"))
                {
                    parseMode = PARSE_MODE_LEAVE_VM_DIR_PASSWORD;
                }
                else if (!strcmp(pszArg, "--force"))
                {
                    pContext->dwLeaveFlags = 1;
                    parseMode = PARSE_MODE_LEAVE_VM_DIR_OPEN;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_LEAVE_VM_DIR_SERVER_NAME:

                if (pContext->pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszServerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_LEAVE_VM_DIR_OPEN;

                break;

            case PARSE_MODE_LEAVE_VM_DIR_USER_NAME:

                if (pContext->pszUserName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszUserName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_LEAVE_VM_DIR_OPEN;

                break;

            case PARSE_MODE_LEAVE_VM_DIR_PASSWORD:

                if (pContext->pszPassword)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszPassword);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_LEAVE_VM_DIR_OPEN;

                break;

            default:

                dwError = ERROR_INTERNAL_ERROR;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }
    if (pContext->pszPassword == NULL)
    {
        dwError = VmAfdAllocateMemory(VMAFD_MAX_PWD_LEN+1, (PVOID *)&pContext->pszPassword);
        BAIL_ON_VMAFD_ERROR(dwError);
        VmAfdReadString("password: ", pContext->pszPassword, VMAFD_MAX_PWD_LEN+1, FALSE);
    }
error:

    return dwError;
}

static
DWORD
ParseArgsJoinAD(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_JOIN_AD_OPEN = 0,
        PARSE_MODE_JOIN_AD_SERVER_NAME,
        PARSE_MODE_JOIN_AD_USER_NAME,
        PARSE_MODE_JOIN_AD_PASSWORD,
        PARSE_MODE_JOIN_AD_MACHINE_NAME,
        PARSE_MODE_JOIN_AD_DOMAIN_NAME,
        PARSE_MODE_JOIN_AD_ORG_UNIT,
    } PARSE_MODE_JOIN_AD;
    PARSE_MODE_JOIN_AD parseMode = PARSE_MODE_JOIN_AD_OPEN;
    DWORD iArg = 0;

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
            case PARSE_MODE_JOIN_AD_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseMode = PARSE_MODE_JOIN_AD_SERVER_NAME;
                }
                else if (!strcmp(pszArg, "--user-name"))
                {
                    parseMode = PARSE_MODE_JOIN_AD_USER_NAME;
                }
                else if (!strcmp(pszArg, "--password"))
                {
                    parseMode = PARSE_MODE_JOIN_AD_PASSWORD;
                }
                else if (!strcmp(pszArg, "--machine-name"))
                {
                    parseMode = PARSE_MODE_JOIN_AD_MACHINE_NAME;
                }
                else if (!strcmp(pszArg, "--domain-name"))
                {
                    parseMode = PARSE_MODE_JOIN_AD_DOMAIN_NAME;
                }
                else if (!strcmp(pszArg, "--org-unit"))
                {
                    parseMode = PARSE_MODE_JOIN_AD_ORG_UNIT;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_JOIN_AD_SERVER_NAME:

                if (pContext->pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszServerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_JOIN_AD_OPEN;

                break;

            case PARSE_MODE_JOIN_AD_USER_NAME:

                if (pContext->pszUserName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszUserName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_JOIN_AD_OPEN;

                break;

            case PARSE_MODE_JOIN_AD_PASSWORD:

                if (pContext->pszPassword)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszPassword);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_JOIN_AD_OPEN;

                break;

            case PARSE_MODE_JOIN_AD_MACHINE_NAME:

                if (pContext->pszMachineName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszMachineName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_JOIN_AD_OPEN;

                break;

            case PARSE_MODE_JOIN_AD_DOMAIN_NAME:

                if (pContext->pszDomainName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszDomainName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_JOIN_AD_OPEN;

                break;

            case PARSE_MODE_JOIN_AD_ORG_UNIT:

                if (pContext->pszOrgUnit)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszOrgUnit);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_JOIN_AD_OPEN;

                break;

            default:

                dwError = ERROR_INTERNAL_ERROR;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

    if (!pContext->pszServerName ||
        !pContext->pszDomainName ||
        !pContext->pszUserName ||
        !pContext->pszPassword)
    {
        dwError = ERROR_LOCAL_OPTION_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
ParseArgsLeaveAD(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_LEAVE_AD_OPEN = 0,
        PARSE_MODE_LEAVE_AD_SERVER_NAME,
        PARSE_MODE_LEAVE_AD_USER_NAME,
        PARSE_MODE_LEAVE_AD_PASSWORD,
    } PARSE_MODE_LEAVE_AD;
    PARSE_MODE_LEAVE_AD parseMode = PARSE_MODE_LEAVE_AD_OPEN;
    DWORD iArg = 0;

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
            case PARSE_MODE_LEAVE_AD_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseMode = PARSE_MODE_LEAVE_AD_SERVER_NAME;
                }
                else if (!strcmp(pszArg, "--user-name"))
                {
                    parseMode = PARSE_MODE_LEAVE_AD_USER_NAME;
                }
                else if (!strcmp(pszArg, "--password"))
                {
                    parseMode = PARSE_MODE_LEAVE_AD_PASSWORD;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_LEAVE_AD_SERVER_NAME:

                if (pContext->pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszServerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_LEAVE_AD_OPEN;

                break;

            case PARSE_MODE_LEAVE_AD_USER_NAME:

                if (pContext->pszUserName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszUserName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_LEAVE_AD_OPEN;

                break;

            case PARSE_MODE_LEAVE_AD_PASSWORD:

                if (pContext->pszPassword)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszPassword);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_LEAVE_AD_OPEN;

                break;

            default:

                dwError = ERROR_INTERNAL_ERROR;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

    if (!pContext->pszServerName ||
        !pContext->pszUserName ||
        !pContext->pszPassword)
    {
        dwError = ERROR_LOCAL_OPTION_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
ParseArgsQueryAD(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_QUERY_AD_OPEN = 0,
        PARSE_MODE_QUERY_AD_SERVER_NAME,
    } PARSE_MODE_QUERY_AD;
    PARSE_MODE_QUERY_AD parseMode = PARSE_MODE_QUERY_AD_OPEN;
    DWORD iArg = 0;

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
            case PARSE_MODE_QUERY_AD_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseMode = PARSE_MODE_QUERY_AD_SERVER_NAME;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_QUERY_AD_SERVER_NAME:

                if (pContext->pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszServerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_QUERY_AD_OPEN;

                break;

            default:

                dwError = ERROR_INTERNAL_ERROR;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

    if (!pContext->pszServerName)
    {
        dwError = ERROR_LOCAL_OPTION_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
ParseArgsGetDCName(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{

    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_GET_DC_NAME_OPEN = 0,
        PARSE_MODE_GET_DC_NAME_SERVER_NAME,
    } PARSE_MODE_GET_DC_NAME;
    PARSE_MODE_GET_DC_NAME parseMode = PARSE_MODE_GET_DC_NAME_OPEN;
    DWORD iArg = 0;

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
            case PARSE_MODE_GET_DC_NAME_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseMode = PARSE_MODE_GET_DC_NAME_SERVER_NAME;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_GET_DC_NAME_SERVER_NAME:

                if (pContext->pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszServerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_GET_DC_NAME_OPEN;

                break;

            default:

                dwError = ERROR_INTERNAL_ERROR;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

    if (!pContext->pszServerName)
    {
        dwError = ERROR_LOCAL_OPTION_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
ParseArgsSetDCName(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_SET_DC_NAME_OPEN = 0,
        PARSE_MODE_SET_DC_NAME_SERVER_NAME,
        PARSE_MODE_SET_DC_NAME_DC_NAME,
    } PARSE_MODE_SET_DC_NAME;
    PARSE_MODE_SET_DC_NAME parseMode = PARSE_MODE_SET_DC_NAME_OPEN;
    DWORD iArg = 0;

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
            case PARSE_MODE_SET_DC_NAME_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseMode = PARSE_MODE_SET_DC_NAME_SERVER_NAME;
                }
                else if (!strcmp(pszArg, "--dc-name"))
                {
                    parseMode = PARSE_MODE_SET_DC_NAME_DC_NAME;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_SET_DC_NAME_SERVER_NAME:

                if (pContext->pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszServerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_SET_DC_NAME_OPEN;

                break;

            case PARSE_MODE_SET_DC_NAME_DC_NAME:

                if (pContext->pszDCName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszDCName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_SET_DC_NAME_OPEN;

                break;

            default:

                dwError = ERROR_INTERNAL_ERROR;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

    if (!pContext->pszServerName ||
        !pContext->pszDCName)
    {
        dwError = ERROR_LOCAL_OPTION_INVALID;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
ParseArgsAddPasswordEntry(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}

static
DWORD
ParseArgsGetMachineAccountInfo(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}

static
DWORD
ParseArgsSetMachineAccountInfo(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}

static
DWORD
ParseArgsGetMachineSSLCertificates(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}

static
DWORD
ParseArgsSetMachineSSLCertificates(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}

static
DWORD
ParseArgsGetHbStatus(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_QUERY_OPEN = 0,
        PARSE_MODE_QUERY_SERVER_NAME,
        PARSE_MODE_QUERY_USER_NAME,
        PARSE_MODE_QUERY_PASSWORD,
    } PARSE_MODE_QUERY_AD;
    PARSE_MODE_QUERY_AD parseMode = PARSE_MODE_QUERY_OPEN;
    DWORD iArg = 0;

    for (iArg = 0; iArg < argc; iArg++)
    {
        PSTR pszArg = argv[iArg];

        switch (parseMode)
        {
            case PARSE_MODE_QUERY_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseMode = PARSE_MODE_QUERY_SERVER_NAME;
                }
                else if (!strcmp(pszArg, "--user-name"))
                {
                    parseMode = PARSE_MODE_QUERY_USER_NAME;
                }
                else if (!strcmp(pszArg, "--password"))
                {
                    parseMode = PARSE_MODE_QUERY_PASSWORD;
                }
               else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_QUERY_SERVER_NAME:

                if (pContext->pszServerName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszServerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_QUERY_OPEN;

                break;

            case PARSE_MODE_QUERY_USER_NAME:

                if (pContext->pszUserName)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszUserName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_QUERY_OPEN;

                break;

            case PARSE_MODE_QUERY_PASSWORD:

                if (pContext->pszPassword)
                {
                    dwError = ERROR_LOCAL_OPTION_INVALID;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszPassword);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_QUERY_OPEN;

                break;

            default:

                dwError = ERROR_INTERNAL_ERROR;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

error:

    return dwError;
}

static
DWORD
ParseArgsGetDCList(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_GET_DC_LIST_OPEN = 0,
        PARSE_MODE_GET_DC_LIST_DOMAIN_NAME,
        PARSE_MODE_GET_DC_LIST_SERVER_NAME,
    } PARSE_MODE_GET_DC_NAME;
    PARSE_MODE_GET_DC_NAME parseMode = PARSE_MODE_GET_DC_LIST_OPEN;
    DWORD iArg = 0;

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
            case PARSE_MODE_GET_DC_LIST_OPEN:
                if (!strcmp(pszArg, "--server-name"))
                {
                    parseMode = PARSE_MODE_GET_DC_LIST_SERVER_NAME;
                }
                else if (!strcmp(pszArg,"--domain-name"))
                {
                    parseMode = PARSE_MODE_GET_DC_LIST_DOMAIN_NAME;
                }
                else
                {
                    dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                    BAIL_ON_VMAFD_ERROR(dwError);
                }
                break;

            case PARSE_MODE_GET_DC_LIST_SERVER_NAME:

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszServerName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_GET_DC_LIST_OPEN;

                break;
            case PARSE_MODE_GET_DC_LIST_DOMAIN_NAME:

                dwError = VmAfdAllocateStringA(pszArg, &pContext->pszDomainName);
                BAIL_ON_VMAFD_ERROR(dwError);

                parseMode = PARSE_MODE_GET_DC_LIST_OPEN;
                break;
            default:
                dwError = ERROR_INTERNAL_ERROR;
                BAIL_ON_VMAFD_ERROR(dwError);

                break;
        }
    }

error:
    return dwError;
}


static
DWORD
ParseArgsChangePNID(
    int                 argc,
    char*               argv[],
    PVM_AFD_CLI_CONTEXT pContext
    )
{
    DWORD dwError = 0;
    typedef enum
    {
        PARSE_MODE_CHANGE_PNID_OPEN = 0,
        PARSE_MODE_CHANGE_PNID_PNID,
        PARSE_MODE_CHANGE_PNID_USER_NAME,
        PARSE_MODE_CHANGE_PNID_PASSWORD,
    } PARSE_MODE_CHANGE_PNID;
    PARSE_MODE_CHANGE_PNID parseMode = PARSE_MODE_CHANGE_PNID_OPEN;
    DWORD iArg = 0;

    for (iArg = 0; iArg < argc; iArg++)
    {
        PSTR pszArg = argv[iArg];

        switch (parseMode)
        {
        case PARSE_MODE_CHANGE_PNID_OPEN:
            if (!strcmp(pszArg, "--pnid"))
            {
                parseMode = PARSE_MODE_CHANGE_PNID_PNID;
            }
            else if (!strcmp(pszArg, "--user-name"))
            {
                parseMode = PARSE_MODE_CHANGE_PNID_USER_NAME;
            }
            else if (!strcmp(pszArg, "--password"))
            {
                parseMode = PARSE_MODE_CHANGE_PNID_PASSWORD;
            }
            else
            {
                dwError = ERROR_LOCAL_OPTION_UNKNOWN;
                BAIL_ON_VMAFD_ERROR(dwError);
            }
            break;

        case PARSE_MODE_CHANGE_PNID_PNID:

            if (pContext->pszPNID)
            {
                dwError = ERROR_LOCAL_OPTION_INVALID;
                BAIL_ON_VMAFD_ERROR(dwError);
            }

            dwError = VmAfdAllocateStringA(pszArg, &pContext->pszPNID);
            BAIL_ON_VMAFD_ERROR(dwError);

            parseMode = PARSE_MODE_CHANGE_PNID_OPEN;

            break;

        case PARSE_MODE_CHANGE_PNID_USER_NAME:

            if (pContext->pszUserName)
            {
                dwError = ERROR_LOCAL_OPTION_INVALID;
                BAIL_ON_VMAFD_ERROR(dwError);
            }

            dwError = VmAfdAllocateStringA(pszArg, &pContext->pszUserName);
            BAIL_ON_VMAFD_ERROR(dwError);

            parseMode = PARSE_MODE_CHANGE_PNID_OPEN;

            break;

        case PARSE_MODE_CHANGE_PNID_PASSWORD:

            if (pContext->pszPassword)
            {
                dwError = ERROR_LOCAL_OPTION_INVALID;
                BAIL_ON_VMAFD_ERROR(dwError);
            }

            dwError = VmAfdAllocateStringA(pszArg, &pContext->pszPassword);
            BAIL_ON_VMAFD_ERROR(dwError);

            parseMode = PARSE_MODE_CHANGE_PNID_OPEN;

            break;

        default:

            dwError = ERROR_INTERNAL_ERROR;
            BAIL_ON_VMAFD_ERROR(dwError);

            break;
        }
    }

error:
    return dwError;
}

static
void
ShowUsage(
    VOID
    )
{
    fprintf(
        stdout,
        "Usage: vmafd-cli { arguments }\n\n"
        "Arguments:\n\n"
        "\ttrigger-rootcerts-refresh\n"
        "\tget-status --server-name <server name>\n"
        "\tget-domain-state --server-name <server name>\n"
        "\tget-domain-name --server-name <server name>\n"
        "\tset-domain-name --server-name <server name> --domain-name <domain name>\n"
        "\tget-ldu --server-name <server name>\n"
        "\tset-ldu --server-name <server name> --ldu <ldu>\n"
        "\tset-rhttpproxy-port --server-name <server name> --rhttpproxy-port <port>\n"
        "\tget-rhttpproxy-port --server-name <server name>\n"
        "\tset-dc-port --server-name <server name> --dc-port <port>\n"
        "\tget-cm-location --server-name <server name>\n"
        "\tget-ls-location --server-name <server name>\n"
        "\tget-dc-name --server-name <server name>\n"
        "\tset-dc-name --server-name <server name> --dc-name <dc name>\n"
        "\tget-pnid-url --server-name <server name>\n"
        "\tget-pnid --server-name <server name>\n"
        "\tset-pnid --server-name <server name> --pnid <pnid>\n"
        "\tget-site-guid --server-name <server name>\n"
        "\tget-site-name --server-name <server name>\n"
        "\tget-machine-id --server-name <server name>\n"
        "\tset-machine-id --server-name <server name> --id <GUID>\n"
        "\tadd-password-entry --server-name <server name>\n"
        "\tget-machine-account-info --server-name <server name>\n"
        "\tset-machine-account-info --server-name <server name> --account-info <account info>\n"
        "\tget-machine-ssl-certificate --server-name <server name>\n"
        "\tset-machine-ssl-certificate --server-name <server name> --ssl-certification <ssl certificate>\n"
        "\tpromote-vmdir --server-name <server name> --domain-name <domain name> --user-name <user-name> --password <password> --site-name <site-name> --partner-name <partner name>\n"
        "\tdemote-vmdir --user-name <user-name> --password <password>\n"
        "\tjoin-vmdir --server-name <server name> --user-name <user-name> [--password <password>|--password-file <password-file>] --machine-name <machine name> --domain-name <domain name>\n"
        "\tleave-vmdir [--server-name <server name>] [--user-name <user-name> --password <password>]\n"
        "\tjoin-ad --server-name <server name> --user-name <user-name> --password <password> --machine-name <machine name> --domain-name <domain name>\n"
        "\tleave-ad --server-name <server name> --user-name <user-name> --password <password> --domain-name <domain name>\n"
        "\tquery-ad --server-name <server name>\n"
        "\tget-dc-list --domain-name <domain name> --server-name <server name>\n"
        "\thelp\n");
}
