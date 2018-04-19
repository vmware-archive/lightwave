/*
 * Copyright © 2012-2018 VMware, Inc.  All Rights Reserved.
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
    PCSTR pszUsername,
    PCSTR pszPassword,
    PCSTR pszDomain,
    PCSTR pszClient,
    DWORD dwLeaveFlags,
    PVMW_IC_SETUP_PARAMS* ppSetupParams
    );

static
VOID
ShowUsage(
    VOID
    );

int
LightwaveDomainLeave(
    int argc,
    char* argv[])
{
    DWORD dwError = 0;
    PVMW_IC_SETUP_PARAMS pSetupParams = NULL;
    PVMW_DEPLOY_LOG_CONTEXT pContext = NULL;
    int retCode = 0;
    PSTR pszErrorMsg = NULL;
    PSTR pszErrorDesc = NULL;
    DWORD dwError2 = 0;

    if (!strcmp(argv[0], "--help"))
    {
        ShowUsage();
        goto cleanup;
    }

    setlocale(LC_ALL, "");

    dwError = VmwDeployInitialize();
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = ParseArgs(argc, argv, &pSetupParams);
    if (dwError)
    {
        ShowUsage();
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VmwDeployCreateLogContext(
                    VMW_DEPLOY_LOG_TARGET_FILE,
                    VMW_DEPLOY_LOG_LEVEL_INFO,
                    ".",
                    &pContext);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeploySetLogContext(pContext);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeployDeleteInstance(pSetupParams);
    BAIL_ON_DEPLOY_ERROR(dwError);

    fprintf(stdout, "Domain Leave was successful\n");

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

    dwError2 = VmwDeployGetError(
                     dwError,
                     &pszErrorMsg,
                     &retCode);
    if (dwError2 || retCode == 1)
    {
        if (!VmAfdGetErrorMsgByCode(dwError, &pszErrorDesc))
        {
            fprintf(stderr, "Domain leave failed. Error %u: %s \n", dwError, pszErrorDesc);
        }
        else
        {
            fprintf(stderr, "Domain leave failed with error: %u\n", dwError);
        }
    }
    else
    {
        fprintf(
            stderr,
            "Domain leave failed, error= %s %u\n",
            pszErrorMsg,
            dwError);
    }

    VMW_DEPLOY_LOG_ERROR("Domain leave failed. Error code: %u", dwError);

    if (pszErrorMsg)
    {
        VmwDeployFreeMemory(pszErrorMsg);
        pszErrorMsg = NULL;
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
    DWORD dwError = 0;
    PSTR  pszUsername = NULL;
    PSTR  pszPassword = NULL;
    PSTR  pszDomain = NULL;
    PSTR  pszClient = NULL;
    DWORD dwLeaveFlags = 0;

    enum PARSE_MODE
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_USERNAME,
        PARSE_MODE_PASSWORD,
        PARSE_MODE_DOMAIN,
        PARSE_MODE_CLIENT,
    } parseMode = PARSE_MODE_OPEN;
    int iArg = 0;
    PVMW_IC_SETUP_PARAMS pSetupParams = NULL;

    for (; iArg < argc; iArg++)
    {
        char* pszArg = argv[iArg];

        switch (parseMode)
        {
            case PARSE_MODE_OPEN:
                if (!strcmp(pszArg, "--username"))
                {
                    parseMode = PARSE_MODE_USERNAME;
                }
                else if (!strcmp(pszArg, "--password"))
                {
                    parseMode = PARSE_MODE_PASSWORD;
                }
                else if (!strcmp(pszArg, "--domain"))
                {
                    parseMode = PARSE_MODE_DOMAIN;
                }
                else if (!strcmp(pszArg, "--client"))
                {
                    parseMode = PARSE_MODE_CLIENT;
                }
                else if (!strcmp(pszArg, "--force"))
                {
                    dwLeaveFlags = VMAFD_DOMAIN_LEAVE_FLAGS_FORCE;
                    parseMode = PARSE_MODE_OPEN;
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

            case PARSE_MODE_USERNAME:

                if (pszUsername)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_DEPLOY_ERROR(dwError);
                }

                pszUsername = pszArg;

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

            case PARSE_MODE_CLIENT:

                if (pszClient)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_DEPLOY_ERROR(dwError);
                }

                pszClient = pszArg;

                parseMode = PARSE_MODE_OPEN;

                break;

            default:

                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_DEPLOY_ERROR(dwError);

                break;
        }
    }

    dwError = VmwDeployBuildParams(
                    pszUsername,
                    pszPassword,
                    pszDomain,
                    pszClient,
                    dwLeaveFlags,
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
    PCSTR pszUsername,
    PCSTR pszPassword,
    PCSTR pszDomain,
    PCSTR pszClient,
    DWORD dwLeaveFlags,
    PVMW_IC_SETUP_PARAMS* ppSetupParams
    )
{
    DWORD dwError = 0;
    PVMW_IC_SETUP_PARAMS pSetupParams = NULL;
    PSTR pszPassword1 = NULL;
    PCSTR pszUserNamePrompt = NULL;

    dwError = VmwDeployAllocateMemory(
                    sizeof(*pSetupParams),
                    (VOID*)&pSetupParams);
    BAIL_ON_DEPLOY_ERROR(dwError);

    if (IsNullOrEmptyString(pszDomain))
    {
        pszDomain = VMW_DEFAULT_DOMAIN_NAME;
    }

    pSetupParams->dir_svc_mode = VMW_DIR_SVC_MODE_CLIENT;

    if (IsNullOrEmptyString(pszUsername))
    {
        pszUsername = VMW_ADMIN_NAME;
    }

    dwError = VmwDeployAllocateStringA(
                    pszUsername,
                    &pSetupParams->pszUsername);
    BAIL_ON_DEPLOY_ERROR(dwError);

    if (!pszPassword)
    {
        pszUserNamePrompt = pszUsername;

        dwError = VmwDeployReadPassword(
                        pszUserNamePrompt,
                        pszDomain,
                        &pszPassword1);
        BAIL_ON_DEPLOY_ERROR(dwError);

        pszPassword = pszPassword1;
    }

    if (!IsNullOrEmptyString(pszDomain))
    {
        dwError = VmwDeployAllocateStringA(
                        pszDomain,
                        &pSetupParams->pszDomainName);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pszClient))
    {
        dwError = VmwDeployAllocateStringA(
                        pszClient,
                        &pSetupParams->pszMachineAccount);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VmwDeployAllocateStringA(pszPassword, &pSetupParams->pszPassword);
    BAIL_ON_DEPLOY_ERROR(dwError);

    pSetupParams->dwLeaveFlags = dwLeaveFlags;

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
VOID
ShowUsage(
    VOID
    )
{
    PSTR pszUsageText =
           "Usage: lightwave domain leave { arguments }\n"
           "Arguments:\n"
           "    [--username <acccount name>]\n"
           "    --password <password>\n"
           "    [--domain <domain name>]\n"
           "    [--client <hostname of client machine>]\n"
           "    [--force]\n";

    printf("%s", pszUsageText);
}
