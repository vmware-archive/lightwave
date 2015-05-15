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
    char* argv[],
    PVMW_IC_SETUP_PARAMS* ppSetupParams
    );

static
DWORD
VmwDeployBuildParams(
    PCSTR pszDomainController,
    PCSTR pszDomain,
    PCSTR pszPassword,
    PVMW_IC_SETUP_PARAMS* ppSetupParams
    );

static
DWORD
VmwDeployReadPassword(
    PCSTR pszUser,
    PCSTR pszDomain, 
    PSTR* ppszPassword
    );

static
VOID
ShowUsage(
    VOID
    );

int main(int argc, char* argv[])
{
    DWORD dwError = 0;
    PVMW_IC_SETUP_PARAMS pSetupParams = NULL;
    PVMW_DEPLOY_LOG_CONTEXT pContext = NULL;

    setlocale(LC_ALL, "");

    dwError = VmwDeployInitialize();
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = ParseArgs(argc-1, &argv[1], &pSetupParams);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeployCreateLogContext(
                    VMW_DEPLOY_LOG_TARGET_FILE,
                    VMW_DEPLOY_LOG_LEVEL_INFO,
                    ".",
                    &pContext);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeploySetLogContext(pContext);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeploySetupInstance(pSetupParams);
    BAIL_ON_DEPLOY_ERROR(dwError);

    fprintf(stdout, "Domain Join was successful\n");

cleanup:

    if (pSetupParams)
    {
        VmwDeployFreeSetupParams(pSetupParams);
    }
    if (pContext)
    {
        VmwDeployReleaseLogContext(pContext);
    }
    VmwDeployShutdown();

    return dwError;

error:

    if (dwError == ERROR_INVALID_PARAMETER)
    {
        ShowUsage();
    }

    VMW_DEPLOY_LOG_ERROR("Domain Join failed. (Error code: %u)\n", dwError);

    goto cleanup;
}

static
DWORD
ParseArgs(
    int   argc,
    char* argv[],
    PVMW_IC_SETUP_PARAMS* ppSetupParams
    )
{
    DWORD dwError     = 0;
    PSTR  pszDomainController   = NULL;
    PSTR  pszDomain = NULL;
    PSTR  pszPassword = NULL;
    enum PARSE_MODE
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_DOMAIN_CONTROLLER,
        PARSE_MODE_DOMAIN,
        PARSE_MODE_PASSWORD,
    } parseMode = PARSE_MODE_OPEN;
    int iArg = 0;
    PVMW_IC_SETUP_PARAMS pSetupParams = NULL;

    for (; iArg < argc; iArg++)
    {
        char* pszArg = argv[iArg];

        switch (parseMode)
        {
            case PARSE_MODE_OPEN:
                if (!strcmp(pszArg, "--password"))
                {
                    parseMode = PARSE_MODE_PASSWORD;
                }
                else if (!strcmp(pszArg, "--domain-controller"))
                {
                    parseMode = PARSE_MODE_DOMAIN_CONTROLLER;
                }
                else if (!strcmp(pszArg, "--domain"))
                {
                    parseMode = PARSE_MODE_DOMAIN;
                }
                else if (!strcmp(pszArg, "--help"))
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_DEPLOY_ERROR(dwError);
                }
                else
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_DEPLOY_ERROR(dwError);
                }

                break;

            case PARSE_MODE_PASSWORD:

                if (pszPassword)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_DEPLOY_ERROR(dwError);
                }

                pszPassword = pszArg;

                parseMode = PARSE_MODE_OPEN;

                break;

            case PARSE_MODE_DOMAIN:

                if (pszDomain)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_DEPLOY_ERROR(dwError);
                }

                pszDomain = pszArg;

                parseMode = PARSE_MODE_OPEN;

                break;

            case PARSE_MODE_DOMAIN_CONTROLLER:

                if (pszDomainController)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_DEPLOY_ERROR(dwError);
                }

                pszDomainController = pszArg;

                parseMode = PARSE_MODE_OPEN;

                break;

            default:

                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_DEPLOY_ERROR(dwError);

                break;
        }
    }

    dwError = VmwDeployBuildParams(
                    pszDomainController,
                    pszDomain,
                    pszPassword,
                    &pSetupParams);
    BAIL_ON_DEPLOY_ERROR(dwError);

    *ppSetupParams = pSetupParams;

cleanup:

    return dwError;

error:

    *ppSetupParams = NULL;

    if (pSetupParams)
    {
        VmwDeployFreeSetupParams(pSetupParams);
    }

    goto cleanup;
}

static
DWORD
VmwDeployBuildParams(
    PCSTR pszDomainController,
    PCSTR pszDomain,
    PCSTR pszPassword,
    PVMW_IC_SETUP_PARAMS* ppSetupParams
    )
{
    DWORD dwError = 0;
    PVMW_IC_SETUP_PARAMS pSetupParams = NULL;
    PSTR pszPassword1 = NULL;

    dwError = VmwDeployAllocateMemory(
                    sizeof(*pSetupParams),
                    (VOID*)&pSetupParams);
    BAIL_ON_DEPLOY_ERROR(dwError);

    if (IsNullOrEmptyString(pszDomain))
    {
        pszDomain = VMW_DEFAULT_DOMAIN_NAME;
    }

    pSetupParams->dir_svc_mode = VMW_DIR_SVC_MODE_CLIENT;

    if (IsNullOrEmptyString(pszDomainController))
    {
        VMW_DEPLOY_LOG_ERROR("Error : Missing or invalid domain controller.\n");

        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VmwDeployAllocateStringA(
                        pszDomainController,
                        &pSetupParams->pszServer);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeployGetHostname(&pSetupParams->pszHostname);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeployAllocateStringA(
                        pszDomain,
                        &pSetupParams->pszDomainName);
    BAIL_ON_DEPLOY_ERROR(dwError);

    if (!pszPassword)
    {
        dwError = VmwDeployReadPassword(
                        "administrator",
                        pszDomain,
                        &pszPassword1);
        BAIL_ON_DEPLOY_ERROR(dwError);

        pszPassword = pszPassword1;
    }

    dwError = VmwDeployAllocateStringA(pszPassword, &pSetupParams->pszPassword);
    BAIL_ON_DEPLOY_ERROR(dwError);

    *ppSetupParams = pSetupParams;

cleanup:

    if (pszPassword1)
    {
        VmwDeployFreeMemory(pszPassword1);
    }

    return dwError;

error:

    *ppSetupParams = NULL;

    if (pSetupParams)
    {
        VmwDeployFreeSetupParams(pSetupParams);
    }

    goto cleanup;
}

static
DWORD
VmwDeployReadPassword(
    PCSTR pszUser,
    PCSTR pszDomain,
    PSTR* ppszPassword
    )
{
    DWORD dwError = 0;
    struct termios orig, nonecho;
    CHAR  szPassword[33] = "";
    PSTR  pszPassword = NULL;
    DWORD iChar = 0;

    memset(szPassword, 0, sizeof(szPassword));

    fprintf(stdout, "Password (%s@%s): ", pszUser, pszDomain);
    fflush(stdout);

    tcgetattr(0, &orig); // get current settings
    memcpy(&nonecho, &orig, sizeof(struct termios)); // copy settings
    nonecho.c_lflag &= ~(ECHO); // don't echo password characters
    tcsetattr(0, TCSANOW, &nonecho); // set current settings to not echo

    // Read up to 32 characters of password

    for (; iChar < sizeof(szPassword); iChar++)
    {
        CHAR ch;

        if (read(STDIN_FILENO, &ch, 1) < 0)
        {
            dwError = LwErrnoToWin32Error(errno);
            BAIL_ON_DEPLOY_ERROR(dwError);
        }

        if (ch == '\n')
        {
            fprintf(stdout, "\n");
            fflush(stdout);
            break;
        }
        else if (ch == '\b') /* backspace */
        {
            if (iChar > 0)
            {
                iChar--;
                szPassword[iChar] = '\0';
            }
        }
        else
        {
            szPassword[iChar] = ch;
        }
    }

    if (IsNullOrEmptyString(&szPassword[0]))
    {
        dwError = ERROR_PASSWORD_RESTRICTION;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VmwDeployAllocateStringA(szPassword, &pszPassword);
    BAIL_ON_DEPLOY_ERROR(dwError);

    *ppszPassword = pszPassword;

cleanup:

    tcsetattr(0, TCSANOW, &orig);

    return dwError;

error:

    *ppszPassword = NULL;

    goto cleanup;
}


static
VOID
ShowUsage(
    VOID
    )
{
    printf("Usage : ic-domain-join { arguments }\n"
           "Arguments:\n"
           "--domain-controller <domain controller's hostname or IP Address>\n"
           "[--domain    <fully qualified domain name. default: vsphere.local>]\n"
           "[--password  <password to administrator account>]\n");
}
