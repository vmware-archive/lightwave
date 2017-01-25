/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

#define MAXBUF 8192

static PCSTRING const DELIMETER = "=";

static
bool
IdmStringStartsWith(
    PCSTRING a,
    PCSTRING b)
{
    if (strncmp(a, b, strlen(b)) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

SSOERROR
RestTestSetup(
    PCSTRING testConfigureFile,
    PCSTRING hokPrivateKeyFile)
{
    SSOERROR e = SSOERROR_NONE;

    REST_SCHEME_TYPE schemeType = REST_SCHEME_TYPE_HTTPS;
    PSTRING serverHost = NULL;
    size_t serverPort = 0;
    PSTRING bearerAccessToken = NULL;
    PSTRING hokAccessToken = NULL;
    PSTRING privateKey = NULL;

    PREST_ACCESS_TOKEN pAccessToken = NULL;
    PSSO_STRING_BUILDER sb = NULL;

    // read configuration file
    FILE* file = fopen(testConfigureFile, "r");

    if (file != NULL)
    {
        char line[MAXBUF];

        while (fgets(line, sizeof(line), file) != NULL)
        {
            PSTRING cfline;
            cfline = strstr((PSTRING) line, DELIMETER);
            cfline = cfline + strlen(DELIMETER);
            if (cfline[strlen(cfline) - 1] == '\n')
            {
                cfline[strlen(cfline) - 1] = 0;
            }

            if (IdmStringStartsWith(line, "server_ip"))
            {
                strtol(line, (PSTRING*) NULL, 10);
                e = SSOStringAllocate(cfline, &serverHost);
                BAIL_ON_ERROR(e)
            }

            if (IdmStringStartsWith(line, "server_port"))
            {
                serverPort = strtol(cfline, (PSTRING*) NULL, 10);
            }

            if (IdmStringStartsWith(line, "bearer_token"))
            {
                e = SSOStringAllocate(cfline, &bearerAccessToken);
                BAIL_ON_ERROR(e)
            }

            if (IdmStringStartsWith(line, "hok_token"))
            {
                e = SSOStringAllocate(cfline, &hokAccessToken);
                BAIL_ON_ERROR(e)
            }
        }
        fclose(file);
    }

    // read private key from file
    e = SSOStringBuilderNew(&sb);
    BAIL_ON_ERROR(e)

    file = fopen(hokPrivateKeyFile, "r");

    if (file != NULL)
    {
        char line[MAXBUF];

        while (fgets(line, sizeof(line), file))
        {
            e = SSOStringBuilderAppend(sb, line);
            BAIL_ON_ERROR(e);
        };
        fclose(file);
    }

    e = SSOStringBuilderGetString(sb, &privateKey);
    BAIL_ON_ERROR(e);

    // CURL global initialization
    e = RestClientGlobalInit();
    BAIL_ON_ERROR(e);

    // create bearer access token
    e = RestAccessTokenNew(&pAccessToken, bearerAccessToken, REST_ACCESS_TOKEN_TYPE_JWT, NULL);
    BAIL_ON_ERROR(e);

    // create bearer token client
    e = RestClientNew(&pBearerTokenClient, serverHost, false, serverPort, schemeType, NULL);
    BAIL_ON_ERROR(e);

    // test set access token
    e = RestClientSetAccessToken(pBearerTokenClient, pAccessToken);
    BAIL_ON_ERROR(e);

    // create bearer token HA client
    e = RestClientNew(&pBearerTokenHAClient, serverHost, true, serverPort, schemeType, NULL);
    BAIL_ON_ERROR(e);

    // test set access token
    e = RestClientSetAccessToken(pBearerTokenHAClient, pAccessToken);
    BAIL_ON_ERROR(e);

    // free access token
    RestAccessTokenDelete(pAccessToken);

    // create HOK access token
    e = RestAccessTokenNew(&pAccessToken, hokAccessToken, REST_ACCESS_TOKEN_TYPE_JWT_HOK, privateKey);
    BAIL_ON_ERROR(e);

    // create hok token client
    e = RestClientNew(&pHOKTokenClient, serverHost, false, serverPort, schemeType, NULL);
    BAIL_ON_ERROR(e);

    // test set access token
    e = RestClientSetAccessToken(pHOKTokenClient, pAccessToken);
    BAIL_ON_ERROR(e);

    // free access token
    RestAccessTokenDelete(pAccessToken);

    error:

    return e;
}

void
RestTestCleanup()
{
    RestClientGlobalCleanup();
    RestClientDelete(pBearerTokenClient);
    RestClientDelete(pHOKTokenClient);
}

PCSTRING
RestTestGenerateErrorMessage(
    PCSTRING testName,
    const SSOERROR testError,
    const REST_SERVER_ERROR* pTestServerError)
{
    PSTRING message = NULL;

    SSOERROR e = SSOERROR_NONE;
    PSSO_STRING_BUILDER sb = NULL;

    if (testError != SSOERROR_NONE)
    {
        e = SSOStringBuilderNew(&sb);
        BAIL_ON_ERROR(e);

        e = SSOStringBuilderAppend(sb, "Test case failed: ");
        BAIL_ON_ERROR(e);

        e = SSOStringBuilderAppend(sb, testName);
        BAIL_ON_ERROR(e);

        e = SSOStringBuilderAppend(sb, "\n");
        BAIL_ON_ERROR(e);

        e = SSOStringBuilderAppend(sb, "Error code returned: ");
        BAIL_ON_ERROR(e);

        e = SSOStringBuilderAppend(sb, SSOErrorToString(testError));
        BAIL_ON_ERROR(e);

        e = SSOStringBuilderAppend(sb, "\n");
        BAIL_ON_ERROR(e);

        if (pTestServerError != NULL)
        {
            char serverErrorMessage[500];

            sprintf(
                serverErrorMessage,
                " HTTP status code: %ld \n error: %s \n details: %s \n cause: %s \n",
                pTestServerError->httpStatusCode,
                pTestServerError->error,
                pTestServerError->details,
                pTestServerError->cause);

            e = SSOStringBuilderAppend(sb, serverErrorMessage);
            BAIL_ON_ERROR(e);

            e = SSOStringBuilderAppend(sb, "\n");
            BAIL_ON_ERROR(e);
        }

        e = SSOStringBuilderGetString(sb, &message);
        BAIL_ON_ERROR(e);
    }

    error:

    if (e != SSOERROR_NONE)
    {
        message = "Failure encountered in testing code\n";
    }

    // cleanup
    SSOStringBuilderDelete(sb);

    return message;
}
