/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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
    int             argc,
    char*           argv[],
    PVMW_CA_PARAMS* ppCaParams
    );

static
DWORD
VmwCaBuildParams(
    PCSTR           pcszConfigFilePath,
    PCSTR           pcszPrivKeyFilePath,
    PCSTR           pcszCertFilePath,
    PCSTR           pcszServer,
    PCSTR           pcszDomain,
    PCSTR           pcszUsername,
    PCSTR           pcszPassword,
    PCSTR           pcszCAServer,
    PCSTR           pcszCAId,
    PCSTR           pcszKeySize,
    PCSTR           pcszDuration,
    BOOL            bInsecure,
    PVMW_CA_PARAMS* ppCaParams
    );

static
VOID
ShowUsage(
    VOID
    );

static
DWORD
VmwCaGetSignedCertRestRequest(
    PVMW_CA_PARAMS pCaParams,
    PVMCA_CSR      pCSR,
    PSTR*          ppszCert
    );

static
DWORD
VmwMutentCaGetSignedCertRestRequest(
    PVMW_CA_PARAMS pCaParams,
    PVMCA_CSR      pCSR,
    PSTR*          ppszCert
    );

static
DWORD
VmwCaGetUrl(
    PCSTR   pcszServer,
    PCSTR   pcszPort,
    PCSTR   pcszEndpoint,
    PSTR*   ppszUrl
    );

static
DWORD
VmwCaParseRestOutput(
    PCSTR pcszRestOutput,
    PSTR* ppszCert
    );

int
LightwaveCaGetSignedCert(
    int argc,
    char* argv[]
    )
{
    int             retCode        = 0;
    DWORD           dwError        = 0;
    DWORD           dwError2       = 0;
    PSTR            pszErrorMsg    = NULL;
    PVMW_CA_PARAMS  pCaParams      = NULL;
    PVMCA_CSR       pCSR           = NULL;
    PSTR            pszCert        = NULL;

    if (argc == 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    if (!strcmp(argv[0], "--help"))
    {
        ShowUsage();
        goto cleanup;
    }

    dwError = ParseArgs(argc, argv, &pCaParams);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwCaGenerateCertSigningRequest(pCaParams, &pCSR);
    BAIL_ON_DEPLOY_ERROR(dwError);

    if (IsNullOrEmptyString(pCaParams->pszCAServer))
    {
        dwError = VmwCaGetSignedCertRestRequest(pCaParams, pCSR, &pszCert);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }
    else
    {
        dwError = VmwMutentCaGetSignedCertRestRequest(pCaParams, pCSR, &pszCert);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    if (pCaParams->pszCertFilePath)
    {
        dwError = VmwCaWriteToFile(pCaParams->pszCertFilePath, pszCert);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }
    else
    {
        fprintf(stdout, "%s", pszCert);
    }

cleanup:
    VmwDeployFreeMemory(pszErrorMsg);
    VMCAFreeCSR(pCSR);
    VmwDeployFreeMemory(pszCert);

    if (pCaParams)
    {
        VmwCaFreeParams(pCaParams);
    }

    return retCode;

error:
    if (dwError != VMW_CA_DEFAULT_ERROR)
    {
        dwError2 = VmwDeployGetError(
                     dwError,
                     &pszErrorMsg,
                     &retCode);
        if (dwError2)
        {
            fprintf(stderr, "Failed to get signed certificate. Error %d: %s\n", dwError, pszErrorMsg);
        }
        else
        {
            fprintf(stderr, "Failed to get signed certificate with error: %d\n", dwError);
        }
    }

    if (retCode == 0)
    {
        retCode = VMW_CA_DEFAULT_ERROR;
    }

    goto cleanup;
}

static
DWORD
VmwCaGetSignedCertRestRequest(
    PVMW_CA_PARAMS pCaParams,
    PVMCA_CSR      pCSR,
    PSTR*          ppszCert
    )
{
    DWORD                 dwError        = 0;
    PSTR                  pszEpochTime   = 0;
    PSTR                  pszAccessToken = NULL;
    PSTR                  pszUrl         = NULL;
    PSTR                  pszHeader      = NULL;
    PSTR                  pszData        = NULL;
    json_t*               pJsonData      = NULL;
    PSTR                  pszRestOutput  = NULL;
    PSTR                  pszCert        = NULL;

    if (!ppszCert || !pCSR || !pCaParams)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VmwCaAcquireOidcToken(pCaParams, VMW_OIDC_CA_SCOPE, &pszAccessToken);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwCaGetUrl(
                pCaParams->pszServer,
                VMW_CA_HTTPS_PORT,
                VMW_CA_CERT_REST_ENDPOINT,
                &pszUrl);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeployAllocateStringPrintf(
                        &pszHeader,
                        VMW_CA_BEARER_AUTH_FORMAT,
                        pszAccessToken);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeployAllocateStringPrintf(
                        &pszEpochTime,
                        "%ld",
                        (DWORD)time(NULL) - VMW_CA_CERT_EXPIRY_START_LAG);
    BAIL_ON_DEPLOY_ERROR(dwError);

    pJsonData = json_pack("{ssssss}", "csr", pCSR, "notBefore", pszEpochTime, "duration", pCaParams->pszCertDuration);
    if (!pJsonData)
    {
        fprintf(stderr, "Could not create JSON body for REST request\n");
        dwError = VMW_CA_DEFAULT_ERROR;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    pszData = json_dumps(pJsonData, 0);
    if (!pszData)
    {
        fprintf(stderr, "Could not dump JSON REST request body data into string\n");
        dwError = VMW_CA_DEFAULT_ERROR;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VmwCaMakeRestRequest(
                    pszUrl,
                    pszHeader,
                    pszData,
                    VMW_CA_CERT_REST_REQUEST_METHOD,
                    pCaParams->bInsecure,
                    &pszRestOutput);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwCaParseRestOutput(pszRestOutput, &pszCert);
    BAIL_ON_DEPLOY_ERROR(dwError);

    *ppszCert = pszCert;

cleanup:
    VmwDeployFreeMemory(pszEpochTime);
    VmwDeployFreeMemory(pszAccessToken);
    VmwDeployFreeMemory(pszUrl);
    VmwDeployFreeMemory(pszHeader);
    VmwDeployFreeMemory(pszData);
    VmwDeployFreeMemory(pszRestOutput);

    if (pJsonData)
    {
        json_decref(pJsonData);
        pJsonData = NULL;
    }

    return dwError;

error:
    VmwDeployFreeMemory(pszCert);
    pszCert = NULL;

    goto cleanup;
}

static
DWORD
VmwMutentCaGetSignedCertRestRequest(
    PVMW_CA_PARAMS pCaParams,
    PVMCA_CSR      pCSR,
    PSTR*          ppszCert
    )
{
    DWORD                 dwError        = 0;
    PSTR                  pszEndpoint    = NULL;
    PSTR                  pszAccessToken = NULL;
    PSTR                  pszUrl         = NULL;
    PSTR                  pszHeader      = NULL;
    PSTR                  pszData        = NULL;
    json_t*               pJsonData      = NULL;
    PSTR                  pszRestOutput  = NULL;
    PSTR                  pszCert        = NULL;

    if (!ppszCert || !pCSR || !pCaParams)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VmwCaAcquireOidcToken(pCaParams, VMW_OIDC_MUTENTCA_SCOPE, &pszAccessToken);
    BAIL_ON_DEPLOY_ERROR(dwError);

    if (!IsNullOrEmptyString(pCaParams->pszCAId))
    {
        dwError = VmwDeployAllocateStringPrintf(
                            &pszEndpoint,
                            VMW_MUTENTCA_INTERMEDIATE_CERT_REST_ENDPOINT,
                            pCaParams->pszCAId);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }
    else
    {
        dwError = VmwDeployAllocateStringA(
                            VMW_MUTENTCA_ROOT_CERT_REST_ENDPOINT,
                            &pszEndpoint);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VmwCaGetUrl(
                pCaParams->pszCAServer,
                VMW_MUTENTCA_HTTPS_PORT,
                pszEndpoint,
                &pszUrl);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeployAllocateStringPrintf(
                        &pszHeader,
                        VMW_CA_BEARER_AUTH_FORMAT,
                        pszAccessToken);
    BAIL_ON_DEPLOY_ERROR(dwError);

    // TODO: Add Cert Duration to json input data if provided
    pJsonData = json_pack("{ss}", "csr", pCSR);
    if (!pJsonData)
    {
        fprintf(stderr, "Could not create JSON body for REST request\n");
        dwError = VMW_CA_DEFAULT_ERROR;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    pszData = json_dumps(pJsonData, 0);
    if (!pszData)
    {
        fprintf(stderr, "Could not dump JSON REST request body data into string\n");
        dwError = VMW_CA_DEFAULT_ERROR;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VmwCaMakeRestRequest(
                    pszUrl,
                    pszHeader,
                    pszData,
                    VMW_MUTENTCA_CERT_REST_REQUEST_METHOD,
                    pCaParams->bInsecure,
                    &pszRestOutput);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwCaParseRestOutput(pszRestOutput, &pszCert);
    BAIL_ON_DEPLOY_ERROR(dwError);

    *ppszCert = pszCert;

cleanup:
    VmwDeployFreeMemory(pszEndpoint);
    VmwDeployFreeMemory(pszAccessToken);
    VmwDeployFreeMemory(pszUrl);
    VmwDeployFreeMemory(pszHeader);
    VmwDeployFreeMemory(pszData);
    VmwDeployFreeMemory(pszRestOutput);

    if (pJsonData)
    {
        json_decref(pJsonData);
        pJsonData = NULL;
    }

    return dwError;

error:
    VmwDeployFreeMemory(pszCert);
    pszCert = NULL;

    goto cleanup;
}

static
DWORD
VmwCaGetUrl(
    PCSTR   pcszServer,
    PCSTR   pcszPort,
    PCSTR   pcszEndpoint,
    PSTR*   ppszUrl
    )
{
    DWORD dwError = 0;
    PSTR pszUrl = NULL;

    if (VmwCaIsIPV6AddrFormat(pcszServer))
    {
        dwError = VmwDeployAllocateStringPrintf(
                            &pszUrl,
                            VMW_CA_REST_URL_FORMAT_IPV6,
                            pcszServer,
                            pcszPort,
                            pcszEndpoint);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }
    else
    {
        dwError = VmwDeployAllocateStringPrintf(
                            &pszUrl,
                            VMW_CA_REST_URL_FORMAT_IPV4,
                            pcszServer,
                            pcszPort,
                            pcszEndpoint);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    *ppszUrl = pszUrl;

cleanup:
    return dwError;

error:
    VmwDeployFreeMemory(pszUrl);
    pszUrl = NULL;

    goto cleanup;
}

static
DWORD
VmwCaParseRestOutput(
    PCSTR pcszRestOutput,
    PSTR* ppszCert
    )
{
    DWORD           dwError         = 0;
    PCSTR           pcszCert        = NULL;
    json_t*         pJsonResponse   = NULL;
    json_t*         pJsonCert       = NULL;
    json_error_t    jsonError       = {0};
    PSTR            pszCert         = NULL;

    pJsonResponse = json_loads(pcszRestOutput, 0, &jsonError);
    if (!pJsonResponse)
    {
        fprintf(stderr, "Failed to load json response. JSON Error: %s\n", jsonError.text);
        dwError = VMW_CA_DEFAULT_ERROR;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    pJsonCert = json_object_get(pJsonResponse, "cert");
    if (!pJsonCert)
    {
        fprintf(stderr, "Unexpected Response from Server. No certificate in json response\n");
        dwError = VMW_CA_DEFAULT_ERROR;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    pcszCert = json_string_value(pJsonCert);
    if (!pcszCert)
    {
        fprintf(stderr, "Failed to get certificate string from json response\n");
        dwError = VMW_CA_DEFAULT_ERROR;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VmwDeployAllocateStringA(
                    pcszCert,
                    &pszCert);
    BAIL_ON_DEPLOY_ERROR(dwError);

    *ppszCert = pszCert;

cleanup:
    if (pJsonResponse)
    {
        json_decref(pJsonResponse);
        pJsonResponse = NULL;
    }

    return dwError;

error:
    VmwDeployFreeMemory(pszCert);
    pszCert = NULL;

    goto cleanup;
}

static
DWORD
ParseArgs(
    int             argc,
    char*           argv[],
    PVMW_CA_PARAMS* ppCaParams
    )
{
    DWORD           dwError            = 0;
    int             iArg               = 0;
    PSTR            pszConfigFilePath  = NULL;
    PSTR            pszPrivKeyFilePath = NULL;
    PSTR            pszCertFilePath    = NULL;
    PSTR            pszServer          = NULL;
    PSTR            pszDomain          = NULL;
    PSTR            pszUsername        = NULL;
    PSTR            pszPassword        = NULL;
    PSTR            pszCAServer        = NULL;
    PSTR            pszCAId            = NULL;
    PSTR            pszKeySize         = NULL;
    PSTR            pszDuration        = NULL;
    BOOL            bInsecure          = false;
    PVMW_CA_PARAMS  pCaParams          = NULL;

    enum PARSE_MODE
    {
        PARSE_MODE_OPEN = 0,
        PARSE_MODE_CONFIG,
        PARSE_MODE_PRIVKEY,
        PARSE_MODE_CERT,
        PARSE_MODE_SERVER,
        PARSE_MODE_DOMAIN,
        PARSE_MODE_USERNAME,
        PARSE_MODE_PASSWORD,
        PARSE_MODE_CASERVER,
        PARSE_MODE_CAID,
        PARSE_MODE_KEYSIZE,
        PARSE_MODE_DURATION,
    } parseMode = PARSE_MODE_OPEN;

    for (; iArg < argc; iArg++)
    {
        char* pszArg = argv[iArg];

        switch (parseMode)
        {
            case PARSE_MODE_OPEN:
                if (!strcmp(pszArg, "--config"))
                {
                    parseMode = PARSE_MODE_CONFIG;
                }
                else if (!strcmp(pszArg, "--privkey"))
                {
                    parseMode = PARSE_MODE_PRIVKEY;
                }
                else if (!strcmp(pszArg, "--cert"))
                {
                    parseMode = PARSE_MODE_CERT;
                }
                else if (!strcmp(pszArg, "--server"))
                {
                    parseMode = PARSE_MODE_SERVER;
                }
                else if (!strcmp(pszArg, "--domain"))
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
                else if (!strcmp(pszArg, "--caserver"))
                {
                    parseMode = PARSE_MODE_CASERVER;
                }
                else if (!strcmp(pszArg, "--caid"))
                {
                    parseMode = PARSE_MODE_CAID;
                }
                else if (!strcmp(pszArg, "--keysize"))
                {
                    parseMode = PARSE_MODE_KEYSIZE;
                }
                else if (!strcmp(pszArg, "--duration"))
                {
                    parseMode = PARSE_MODE_DURATION;
                }
                else if (!strcmp(pszArg, "--insecure"))
                {
                    bInsecure = true;
                }
                else
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_DEPLOY_ERROR(dwError);
                }

                break;

            case PARSE_MODE_CONFIG:
                if (pszConfigFilePath)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_DEPLOY_ERROR(dwError);
                }

                pszConfigFilePath = pszArg;
                parseMode = PARSE_MODE_OPEN;

                break;

            case PARSE_MODE_PRIVKEY:
                if (pszPrivKeyFilePath)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_DEPLOY_ERROR(dwError);
                }

                pszPrivKeyFilePath = pszArg;
                parseMode = PARSE_MODE_OPEN;

                break;

             case PARSE_MODE_CERT:
                if (pszCertFilePath)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_DEPLOY_ERROR(dwError);
                }

                pszCertFilePath = pszArg;
                parseMode = PARSE_MODE_OPEN;

                break;

            case PARSE_MODE_SERVER:
                if (pszServer)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_DEPLOY_ERROR(dwError);
                }

                pszServer = pszArg;
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

            case PARSE_MODE_USERNAME:
                if (pszUsername)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_DEPLOY_ERROR(dwError);
                }

                pszUsername = pszArg;
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

            case PARSE_MODE_CASERVER:
                if (pszCAServer)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_DEPLOY_ERROR(dwError);
                }

                pszCAServer = pszArg;
                parseMode = PARSE_MODE_OPEN;

                break;

            case PARSE_MODE_CAID:
                if (pszCAId)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_DEPLOY_ERROR(dwError);
                }

                pszCAId = pszArg;
                parseMode = PARSE_MODE_OPEN;

                break;

            case PARSE_MODE_KEYSIZE:
                if (pszKeySize)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_DEPLOY_ERROR(dwError);
                }

                pszKeySize = pszArg;
                parseMode = PARSE_MODE_OPEN;

                break;

            case PARSE_MODE_DURATION:
                if (pszDuration)
                {
                    dwError = ERROR_INVALID_PARAMETER;
                    BAIL_ON_DEPLOY_ERROR(dwError);
                }

                pszDuration = pszArg;
                parseMode = PARSE_MODE_OPEN;

                break;

            default:
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_DEPLOY_ERROR(dwError);

                break;
        }
    }

    dwError = VmwCaBuildParams(
                    pszConfigFilePath,
                    pszPrivKeyFilePath,
                    pszCertFilePath,
                    pszServer,
                    pszDomain,
                    pszUsername,
                    pszPassword,
                    pszCAServer,
                    pszCAId,
                    pszKeySize,
                    pszDuration,
                    bInsecure,
                    &pCaParams
                    );
    BAIL_ON_DEPLOY_ERROR(dwError);

    *ppCaParams = pCaParams;

cleanup:
    return dwError;

error:
    if (dwError == ERROR_INVALID_PARAMETER)
    {
        fprintf(stderr, "Invalid Parameter in Arguments\n\n");
        ShowUsage();
        dwError = VMW_CA_DEFAULT_ERROR;
    }

    *ppCaParams = NULL;
    if (pCaParams)
    {
        VmwCaFreeParams(pCaParams);
    }

    goto cleanup;
}

static
DWORD
VmwCaBuildParams(
    PCSTR           pcszConfigFilePath,
    PCSTR           pcszPrivKeyFilePath,
    PCSTR           pcszCertFilePath,
    PCSTR           pcszServer,
    PCSTR           pcszDomain,
    PCSTR           pcszUsername,
    PCSTR           pcszPassword,
    PCSTR           pcszCAServer,
    PCSTR           pcszCAId,
    PCSTR           pcszKeySize,
    PCSTR           pcszDuration,
    BOOL            bInsecure,
    PVMW_CA_PARAMS* ppCaParams
    )
{
    DWORD           dwError    = 0;
    DWORD           dwDuration = 0;
    PVMW_CA_PARAMS  pCaParams  = NULL;
    json_error_t    jsonError  = {0};

    if (IsNullOrEmptyString(pcszConfigFilePath))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    if (IsNullOrEmptyString(pcszPrivKeyFilePath))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VmwDeployAllocateMemory(
                    sizeof(*pCaParams),
                    (VOID*)&pCaParams
                    );
    BAIL_ON_DEPLOY_ERROR(dwError);

    pCaParams->pJsonConfig = json_load_file(pcszConfigFilePath, 0, &jsonError);
    if (!pCaParams->pJsonConfig)
    {
        fprintf(
            stderr,
            "Failed to load config file. FilePath: %s, Line: %d, Error: %s\n",
            pcszConfigFilePath,
            jsonError.line,
            jsonError.text);

        dwError = VMW_CA_DEFAULT_ERROR;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pcszPrivKeyFilePath))
    {
        dwError = VmwDeployAllocateStringA(
                        pcszPrivKeyFilePath,
                        &pCaParams->pszPrivKeyFilePath
                        );
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pcszCertFilePath))
    {
        dwError = VmwDeployAllocateStringA(
                        pcszCertFilePath,
                        &pCaParams->pszCertFilePath
                        );
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pcszServer))
    {
        dwError = VmwDeployAllocateStringA(
                        pcszServer,
                        &pCaParams->pszServer
                        );
        BAIL_ON_DEPLOY_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdGetDCNameA(
                        VMW_DEFAULT_AFD_SERVER,
                        &pCaParams->pszServer
                        );
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pcszDomain))
    {
        dwError = VmwDeployAllocateStringA(
                        pcszDomain,
                        &pCaParams->pszDomain
                        );
        BAIL_ON_DEPLOY_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdGetDomainNameA(
                        VMW_DEFAULT_AFD_SERVER,
                        &pCaParams->pszDomain
                        );
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pcszUsername) && !IsNullOrEmptyString(pcszPassword))
    {
        dwError = VmwDeployAllocateStringA(
                        pcszUsername,
                        &pCaParams->pszUsername
                        );
        BAIL_ON_DEPLOY_ERROR(dwError);

        dwError = VmwDeployAllocateStringA(
                        pcszPassword,
                        &pCaParams->pszPassword
                        );
        BAIL_ON_DEPLOY_ERROR(dwError);
    }
    else if (!IsNullOrEmptyString(pcszUsername))
    {
        dwError = VmwDeployAllocateStringA(
                        pcszUsername,
                        &pCaParams->pszUsername
                        );
        BAIL_ON_DEPLOY_ERROR(dwError);

        dwError = VmwDeployReadPassword(
                        pcszUsername,
                        pCaParams->pszDomain,
                        &pCaParams->pszPassword);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdGetMachineAccountInfoA(
                        VMW_DEFAULT_AFD_SERVER,
                        &pCaParams->pszUsername,
                        &pCaParams->pszPassword
                        );
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pcszCAServer))
    {
        dwError = VmwDeployAllocateStringA(
                        pcszCAServer,
                        &pCaParams->pszCAServer
                        );
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pcszCAId))
    {
        if (IsNullOrEmptyString(pcszCAServer))
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_DEPLOY_ERROR(dwError);
        }

        dwError = VmwDeployAllocateStringA(
                        pcszCAId,
                        &pCaParams->pszCAId
                        );
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    if (IsNullOrEmptyString(pcszKeySize))
    {
        pCaParams->privKeySize = VMW_CA_DEFAULT_KEYSIZE;
    }
    else
    {
        dwError = VmwGetKeySize(pcszKeySize, &pCaParams->privKeySize);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pcszDuration))
    {
        dwDuration = atoi(pcszDuration);
        if (dwDuration == 0)
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_DEPLOY_ERROR(dwError);
        }

        dwError = VmwDeployAllocateStringPrintf(
                        &pCaParams->pszCertDuration,
                        "%d",
                        dwDuration + VMW_CA_CERT_EXPIRY_START_LAG
                        );
        BAIL_ON_DEPLOY_ERROR(dwError);
    }
    else
    {
        dwError = VmwDeployAllocateStringPrintf(
                        &pCaParams->pszCertDuration,
                        "%d",
                        VMW_CA_DEFAULT_CERT_VALIDITY
                        );
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    pCaParams->bInsecure = bInsecure;

    *ppCaParams = pCaParams;

cleanup:
    return dwError;

error:
    *ppCaParams = NULL;
    if (pCaParams)
    {
        VmwCaFreeParams(pCaParams);
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
            "Usage: lightwave ca get-cert { arguments }\n"
            "Arguments:\n"
            "   --config <path to .json config file>\n"
            "   --privkey <path to get or store private key>\n"
            "           (If privkey file exists, private key from the file will be used)\n"
            "           (If privkey file does not exists, new private key will be created and stored in the file)\n"
            "   [--cert <path to store certificate>] (overwrites if exists)\n"
            "   [--duration <cert duration in seconds>] (default: 10 years in seconds)\n"
            "   [--keysize <private key size>] (default: 2k, options: 2k/4k/8k)\n"
            "   [--server <server-ip/hostname>] (default: dc-name)\n"
            "   [--domain <domain name>] (default: machine account domain)\n"
            "   [--username <username>] (default: machine account username)\n"
            "   [--password <password>] (default: machine account password)\n"
            "   [--caserver <mutentca server ip/hostname>] (default: uses vmca)]\n"
            "   [--caid <root/intermediate ca id>] (default: root ca id) (NOTE: Can only be used with --caserver option)\n"
            "   [--insecure] (skip certificate validation)\n";

    printf("%s", pszUsageText);
}
