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
    PCSTR pszDomain,
    PCSTR pszUsername,
    PCSTR pszPassword,
    PCSTR pszPartner,
    PCSTR pszSite,
    PCSTR pszSubjectAltName,
    PCSTR pszParentDomain,
    PCSTR pszParentDC,
    PCSTR pszParentUserName,
    PCSTR pszParentPassword,
    PVMW_IC_SETUP_PARAMS* ppSetupParams
    );

static
VOID
ShowUsage(
    VOID
    );

int
LightwaveDomainPromote(
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

    if (argc == 0 || argv[0] == NULL || !strcmp(argv[0], "--help"))
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
            fprintf(stderr, "Domain controller setup failed. Error %u: %s \n", dwError, pszErrorDesc);
        }
        else
        {
            fprintf(stderr, "Domain controller setup failed with error: %u\n", dwError);
        }
    }
    else
    {
        fprintf(
            stderr,
            "Domain controller setup failed, error= %s %u\n",
            pszErrorMsg,
            dwError);
    }

    VMW_DEPLOY_LOG_ERROR("Domain controller setup failed. Error code: %u", dwError);

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
    DWORD dwError     = 0;
    PSTR  pszDomain   = NULL;
    PSTR  pszPartner  = NULL;
    PSTR  pszUsername = NULL;
    PSTR  pszPassword = NULL;
    PSTR  pszSite     = NULL;
    PSTR  pszSubjectAltName = NULL;
    PSTR  pszParentDomain = NULL;
    PSTR  pszParentDC = NULL;
    PSTR  pszParentUserName = NULL;
    PSTR  pszParentPassword = NULL;
    PSTR  pszFQDomainName = NULL;
    enum PARSE_MODE
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_DOMAIN,
        PARSE_MODE_PARTNER,
        PARSE_MODE_USERNAME,
        PARSE_MODE_PASSWORD,
        PARSE_MODE_SITE,
        PARSE_MODE_SSL_SUBJECT_ALT_NAME,
        PARSE_MODE_PARENT_DOMAIN,
        PARSE_MODE_PARENT_DC,
        PARSE_MODE_PARENT_USERNAME,
        PARSE_MODE_PARENT_PASSWORD,
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
                else if (!strcmp(pszArg, "--username"))
                {
                    parseMode = PARSE_MODE_USERNAME;
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
                else if (!strcmp(pszArg, "--ssl-subject-alt-name"))
                {
                    parseMode = PARSE_MODE_SSL_SUBJECT_ALT_NAME;
                }
                else if (!strcmp(pszArg, "--parent-domain"))
                {
                    parseMode = PARSE_MODE_PARENT_DOMAIN;
                }
                else if (!strcmp(pszArg, "--parent-dc"))
                {
                    parseMode = PARSE_MODE_PARENT_DC;
                }
                else if (!strcmp(pszArg, "--parent-username"))
                {
                    parseMode = PARSE_MODE_PARENT_USERNAME;
                }
                else if (!strcmp(pszArg, "--parent-password"))
                {
                    parseMode = PARSE_MODE_PARENT_PASSWORD;
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

            case PARSE_MODE_SSL_SUBJECT_ALT_NAME:

                if (pszSubjectAltName)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_DEPLOY_ERROR(dwError);
                }

                pszSubjectAltName = pszArg;

                parseMode = PARSE_MODE_OPEN;

                break;

            case PARSE_MODE_PARENT_DOMAIN:

                if (pszParentDomain)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_DEPLOY_ERROR(dwError);
                }

                pszParentDomain = pszArg;

                parseMode = PARSE_MODE_OPEN;

                break;

            case PARSE_MODE_PARENT_DC:

                if (pszParentDC)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_DEPLOY_ERROR(dwError);
                }

                pszParentDC = pszArg;

                parseMode = PARSE_MODE_OPEN;

                break;

            case PARSE_MODE_PARENT_USERNAME:

                if (pszParentUserName)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_DEPLOY_ERROR(dwError);
                }

                pszParentUserName = pszArg;

                parseMode = PARSE_MODE_OPEN;

                break;

            case PARSE_MODE_PARENT_PASSWORD:

                if (pszParentPassword)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_DEPLOY_ERROR(dwError);
                }

                pszParentPassword = pszArg;

                parseMode = PARSE_MODE_OPEN;

                break;

            default:

                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_DEPLOY_ERROR(dwError);

                break;
        }
    }

    if (pszParentDomain && pszDomain)
    {
        dwError = VmwDeployAllocateStringPrintf(
                        &pszFQDomainName,
                        "%s.%s",
                        pszDomain,
                        pszParentDomain);
        BAIL_ON_DEPLOY_ERROR(dwError);

        pszDomain = pszFQDomainName;
    }

    dwError = VmwDeployBuildParams(
                    pszDomain,
                    pszUsername,
                    pszPassword,
                    pszPartner,
                    pszSite,
                    pszSubjectAltName,
                    pszParentDomain,
                    pszParentDC,
                    pszParentUserName,
                    pszParentPassword,
                    &pSetupParams);
    BAIL_ON_DEPLOY_ERROR(dwError);

    *ppSetupParams = pSetupParams;

cleanup:

    if (pszFQDomainName)
    {
        VmwDeployFreeMemory(pszFQDomainName);
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
VmwDeployBuildParams(
    PCSTR pszDomain,
    PCSTR pszUsername,
    PCSTR pszPassword,
    PCSTR pszPartner,
    PCSTR pszSite,
    PCSTR pszSubjectAltName,
    PCSTR pszParentDomain,
    PCSTR pszParentDC,
    PCSTR pszParentUserName,
    PCSTR pszParentPassword,
    PVMW_IC_SETUP_PARAMS* ppSetupParams
    )
{
    DWORD dwError = 0;
    PVMW_IC_SETUP_PARAMS pSetupParams = NULL;
    PSTR pszPassword1 = NULL;
    PSTR pszHostname = NULL;

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

    dwError = VmwDeployGetHostname(&pszHostname);
    BAIL_ON_DEPLOY_ERROR(dwError);

    if (IsNullOrEmptyString(pszDomain))
    {
        pszDomain = VMW_DEFAULT_DOMAIN_NAME;
    }

    if (!strchr(pszHostname, '.'))
    {
        dwError = VmwDeployAllocateStringPrintf(
                        &pSetupParams->pszHostname,
                        "%s.%s",
                        pszHostname,
                        pszDomain);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }
    else
    {
        dwError = VmwDeployAllocateStringA(
                        pszHostname,
                        &pSetupParams->pszHostname);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    if (!pszUsername)
    {
        pszUsername = VMW_ADMIN_NAME;
    }

    if (IsNullOrEmptyString(pszPassword))
    {
        dwError = VmwDeployReadPassword(
                        pszUsername,
                        pszDomain,
                        &pszPassword1);
        BAIL_ON_DEPLOY_ERROR(dwError);

        pszPassword = pszPassword1;
    }

    if (!IsNullOrEmptyString(pszSubjectAltName))
    {
        dwError = VmwDeployAllocateStringA(
                        pszSubjectAltName,
                        &pSetupParams->pszSubjectAltName);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VmwDeployAllocateStringA(pszDomain, &pSetupParams->pszDomainName);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeployAllocateStringA(pszUsername, &pSetupParams->pszUsername);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeployAllocateStringA(pszPassword, &pSetupParams->pszPassword);
    BAIL_ON_DEPLOY_ERROR(dwError);

    if (pszSite)
    {
        dwError = VmwDeployAllocateStringA(pszSite, &pSetupParams->pszSite);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    if (pszParentDomain)
    {
        dwError = VmwDeployAllocateStringA(pszParentDomain, &pSetupParams->pszParentDomainName);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    if (pszParentDC)
    {
        dwError = VmwDeployAllocateStringA(pszParentDC, &pSetupParams->pszParentDC);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    if (pszParentUserName)
    {
        dwError = VmwDeployAllocateStringA(pszParentUserName, &pSetupParams->pszParentUserName);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    if (pszParentPassword)
    {
        dwError = VmwDeployAllocateStringA(pszParentPassword, &pSetupParams->pszParentPassword);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    *ppSetupParams = pSetupParams;

cleanup:

    if (pszPassword1)
    {
        VmwDeployFreeMemory(pszPassword1);
    }
    if (pszHostname)
    {
        VmwDeployFreeMemory(pszHostname);
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
           "Usage: lightwave domain promote { arguments }\n"
           "Arguments:\n"
           "    [--domain   <fully qualified domain name. Default : vsphere.local>]\n"
           "    [--username <account name>]\n"
           "    --password  <password>\n"
           "    [--partner  <partner domain controller's hostname or IP Address>]\n"
           "    [--ssl-subject-alt-name <subject alternate name on generated SSL certificate. Default: hostname>]\n"
           "    [--site     <infra site name>]\n"
           "    [--parent-domain <parent domain name>]\n"
           "    [--parent-dc <parent domain controller>]\n"
           "    [--parent-username <parent domain admin username>]\n"
           "    [--parent-password <password for admin user in parent domain>]\n";

    printf("%s", pszUsageText);
}
