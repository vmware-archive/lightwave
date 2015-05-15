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
    PCSTR pszDomain,
    PCSTR pszPassword,
    PCSTR pszPartner,
    PCSTR pszSite,
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

    fprintf(stdout, "Domain Controller setup was successful\n");

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

    VMW_DEPLOY_LOG_ERROR("Domain controller setup failed. Error code: %u", dwError);

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
ParseArgs(
    int   argc,
    char* argv[],
    PVMW_IC_SETUP_PARAMS* ppSetupParams
    )
{
    DWORD dwError     = 0;
    PSTR  pszDomain   = NULL;
    PSTR  pszPartner  = NULL;
    PSTR  pszPassword = NULL;
    PSTR  pszSite     = NULL;
    enum PARSE_MODE
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_DOMAIN,
        PARSE_MODE_PARTNER,
        PARSE_MODE_PASSWORD,
        PARSE_MODE_SITE
    } parseMode = PARSE_MODE_OPEN;
    int iArg = 0;
    PVMW_IC_SETUP_PARAMS pSetupParams = NULL;

    for (; iArg < argc; iArg++)
    {
        char* pszArg = argv[iArg];

        switch (parseMode)
        {
            case PARSE_MODE_OPEN:
                if (!strcmp(pszArg, "--domain"))
                {
                    parseMode = PARSE_MODE_DOMAIN;
                }
                else if (!strcmp(pszArg, "--password"))
                {
                    parseMode = PARSE_MODE_PASSWORD;
                }
                else if (!strcmp(pszArg, "--partner"))
                {
                    parseMode = PARSE_MODE_PARTNER;
                }
                else if (!strcmp(pszArg, "--site"))
                {
                    parseMode = PARSE_MODE_SITE;
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

            case PARSE_MODE_DOMAIN:

                if (pszDomain)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_DEPLOY_ERROR(dwError);
                }

                pszDomain = pszArg;

                parseMode = PARSE_MODE_OPEN;

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

            case PARSE_MODE_PARTNER:

                if (pszPartner)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_DEPLOY_ERROR(dwError);
                }

                pszPartner = pszArg;

                parseMode = PARSE_MODE_OPEN;

                break;

            case PARSE_MODE_SITE:

                if (pszSite)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_DEPLOY_ERROR(dwError);
                }

                pszSite = pszArg;

                parseMode = PARSE_MODE_OPEN;

                break;

            default:

                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_DEPLOY_ERROR(dwError);

                break;
        }
    }

    dwError = VmwDeployBuildParams(
                    pszDomain,
                    pszPassword,
                    pszPartner,
                    pszSite,
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
    PCSTR pszDomain,
    PCSTR pszPassword,
    PCSTR pszPartner,
    PCSTR pszSite,
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

    if (IsNullOrEmptyString(pszPartner))
    {
        pSetupParams->dir_svc_mode = VMW_DIR_SVC_MODE_STANDALONE;
    }
    else
    {
        pSetupParams->dir_svc_mode = VMW_DIR_SVC_MODE_PARTNER;

        dwError = VmwDeployAllocateStringA(
                        pszPartner,
                        &pSetupParams->pszServer);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VmwDeployGetHostname(&pSetupParams->pszHostname);
    BAIL_ON_DEPLOY_ERROR(dwError);

    if (IsNullOrEmptyString(pszDomain))
    {
        pszDomain = VMW_DEFAULT_DOMAIN_NAME;
    }

    if (!pszPassword)
    {
        dwError = VmwDeployReadPassword(
                        "administrator",
                        pszDomain,
                        &pszPassword1);
        BAIL_ON_DEPLOY_ERROR(dwError);

        pszPassword = pszPassword1;
    }

    dwError = VmwDeployAllocateStringA(pszDomain, &pSetupParams->pszDomainName);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeployAllocateStringA(pszPassword, &pSetupParams->pszPassword);
    BAIL_ON_DEPLOY_ERROR(dwError);

    if (pszSite)
    {
        dwError = VmwDeployAllocateStringA(pszSite, &pSetupParams->pszSite);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

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
    printf("Usage : ic-promote { arguments }\n"
           "Arguments:\n"
           "[--domain   <fully qualified domain name. Default : vsphere.local>]\n"
           "--password  <password to administrator account>\n"
           "[--partner  <partner domain controller's hostname or IP Address>]\n"
           "[--site     <infra site name>]\n\n");
}
