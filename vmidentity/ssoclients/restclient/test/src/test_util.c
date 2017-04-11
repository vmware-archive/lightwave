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

SSOERROR
RestTestBearerTokenClientNew(
    PCSTRING pscHost,
    size_t pscPort,
    PCSTRING tenant,
    PCSTRING username,
    PCSTRING password,
    bool useHA,
    PREST_CLIENT* ppRestBearerTokenClient)
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestBearerTokenClient = NULL;

    POIDC_CLIENT pOIDCClient = NULL;
    POIDC_TOKEN_SUCCESS_RESPONSE pTokenSuccessResponse = NULL;
    POIDC_ERROR_RESPONSE pTokenErrorResponse = NULL;
    PREST_ACCESS_TOKEN pBearerAccessToken = NULL;

    // create OIDC client to get token
    e = OidcClientGlobalInit();
    BAIL_ON_ERROR(e);

    e = OidcClientBuild(&pOIDCClient, pscHost, pscPort, tenant, 5 * 60L);
    BAIL_ON_ERROR(e);

    e = OidcClientAcquireTokensByPassword(
        pOIDCClient,
        username,
        password,
        "openid rs_admin_server",
        &pTokenSuccessResponse,
        &pTokenErrorResponse);
    BAIL_ON_ERROR(e);

    // create bearer access token
    e = RestAccessTokenNew(&pBearerAccessToken, OidcTokenSuccessResponseGetAccessToken(pTokenSuccessResponse), REST_ACCESS_TOKEN_TYPE_JWT, NULL);
    BAIL_ON_ERROR(e);

    // create bearer token client
    e = RestClientNew(&pRestBearerTokenClient, pscHost, useHA, pscPort, REST_SCHEME_TYPE_HTTPS, pBearerAccessToken);
    BAIL_ON_ERROR(e);

    *ppRestBearerTokenClient = pRestBearerTokenClient;

    error:

    if (e != SSOERROR_NONE)
    {
        RestClientDelete(pRestBearerTokenClient);
    }

    // cleanup REST client data
    RestAccessTokenDelete(pBearerAccessToken);

    // cleanup OIDC client data
    OidcTokenSuccessResponseDelete(pTokenSuccessResponse);
    OidcErrorResponseDelete(pTokenErrorResponse);
    OidcClientDelete(pOIDCClient);
    OidcClientGlobalCleanup();

    return e;
}

SSOERROR
RestTestHOKTokenClientNew(
    PCSTRING pscHost,
    size_t pscPort,
    PCSTRING tenant,
    PCSTRING username,
    PCSTRING password,
    bool useHA,
    PREST_CLIENT* ppRestHOKTokenClient)
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestBearerTokenClient = NULL;
    PREST_CLIENT pRestHOKTokenClient = NULL;

    POIDC_CLIENT pOIDCClient = NULL;
    POIDC_TOKEN_SUCCESS_RESPONSE pTokenSuccessResponse = NULL;
    POIDC_ERROR_RESPONSE pTokenErrorResponse = NULL;
    PREST_ACCESS_TOKEN pBearerAccessToken = NULL;
    PREST_ACCESS_TOKEN pHOKAccessToken = NULL;

    REST_SERVER_ERROR* pServerError = NULL;
    VMDIR_SOLUTION_USER_DATA* pSolutionUserReturn = NULL;
    PSTRING description = "description";
    VMDIR_PRINCIPAL_DATA* pAlias = NULL;
    REST_CERTIFICATE_DATA* pCertificate = NULL;
    PCSTRING privateKey =
        "-----BEGIN RSA PRIVATE KEY-----\nMIICXAIBAAKBgQDKLBEMCHLGaiYfnnCzNeid+o+pMyZ+Jl5ncze7cXEQfXxzhp8M\nWIDJHwp0np7G+CwN6zFYYiFx0jpHEUISPlaThsX1B54uhJlQY5EpPic2nwEfvKGj\nGzluN2cZ3LpDu2vArCmwJfu8cVpU8jHImp4wregfWCEZPvZp4XuWDvs67QIDAQAB\nAoGAQClyE46O1neTt88x1z5Rn+mINFueaMTGizBezKc5CdG5cHSV/3YLEUk/qTk6\nDM1wNc6hr3odQSFGC43nmSCdms1VOVIbeh9KUmTYjsbb3765Ed5cWxUVWCb9guMr\ndeiZnw7sy1P0Ltv9qBEQAUH67Sh9pBuGdgGLeltSN1ex04UCQQD5X04L2hzJowRA\nXU+MN2tYTtSmiaHaaV/lNQoRHwq9SebHySP+jSIuXKrDofJDkrmIj5ZvqOQWpIEO\naLR4uJbHAkEAz4ueSiwSSXyPYCdcswYs5esv7c5VmLBJDVW4ZexexB3kXz2Ar5FT\nmQHTAw0bZuhsCeadQGrlVpSyB87Fc4lcqwJAE0qqMQSIoZi45XnSg5ht94DxtoiD\nOvBX+NVnl/d8zzP+ZIpM/I5SjJ+inkvTACNDsyrYin7YVmAXk9PJ0mUFVQJAWYOU\nT9VWRc+tXwFbH/On5bpcP6rgjCxsNY4lLACYlul1mAZEvHRno/R/eC9tBCN2hYar\niB8SrxcO+gMackxqZQJBAJaFqetj07pZz71WcTxfH5vqsbKdyDZm+K+VoDKO2dhk\nqEl1+fMVgigMpq9W2VPZvb/5lKLN27xzxEzbYFey2Y4=\n-----END RSA PRIVATE KEY-----";
    PCSTRING certificateSubjectDN = "CN=oidc-client-test-723693c9-bff6-4bce-ad25-e654179f7448";
    PCSTRING x509Certificate =
        "-----BEGIN CERTIFICATE-----\nMIIB+zCCAWSgAwIBAgIIXtT2OC3pDLUwDQYJKoZIhvcNAQEFBQAwQDE+MDwGA1UEAww1b2lkYy1jbGllbnQtdGVzdC03MjM2OTNjOS1iZmY2LTRiY2UtYWQyNS1lNjU0MTc5Zjc0NDgwHhcNMTcwMjAyMjM0NTAwWhcNMTcwMjIxMDAyNTI5WjBAMT4wPAYDVQQDDDVvaWRjLWNsaWVudC10ZXN0LTcyMzY5M2M5LWJmZjYtNGJjZS1hZDI1LWU2NTQxNzlmNzQ0ODCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEAyiwRDAhyxmomH55wszXonfqPqTMmfiZeZ3M3u3FxEH18c4afDFiAyR8KdJ6exvgsDesxWGIhcdI6RxFCEj5Wk4bF9QeeLoSZUGORKT4nNp8BH7yhoxs5bjdnGdy6Q7trwKwpsCX7vHFaVPIxyJqeMK3oH1ghGT72aeF7lg77Ou0CAwEAATANBgkqhkiG9w0BAQUFAAOBgQCb7oIP6tSmF/ygrnPp39SLZEugdUixfk5Ol3BaLJpO9c3wzW7DvHecrwn6O9XtEwAI1MeqEWF0zelz8A/iNUmQwW95E8DglYumDG3H4LjRA+c8Y7/qOdYve0JGHUD0dY4s+jert3weCAgXShzJ6fNga2719E8X9mYMta14hVleaQ==-----END CERTIFICATE-----";

    bool disabled = false;
    PSTRING objectId = "objectId";
    VMDIR_SOLUTION_USER_DATA* pSolutionUser = NULL;
    PCSTRING name = "test_solution_user_rest_hok_c_client";
    PCSTRING domain = testTenant;

    PSTRING pMemberArray[] =
    {
        "oidc-client-test-723693c9-bff6-4bce-ad25-e654179f7448@test_tenant_name"
    };
    VMDIR_STRING_ARRAY_DATA* pMembers = NULL;
    VMDIR_MEMBER_TYPE memberType = VMDIR_MEMBER_TYPE_SOLUTIONUSER;
    PCSTRING groupName = "Administrators";

    // create OIDC client to get token
    e = OidcClientGlobalInit();
    BAIL_ON_ERROR(e);

    e = OidcClientBuild(&pOIDCClient, pscHost, pscPort, tenant, 5 * 60L);
    BAIL_ON_ERROR(e);

    e = OidcClientAcquireTokensByPassword(
        pOIDCClient,
        username,
        password,
        "openid rs_admin_server",
        &pTokenSuccessResponse,
        &pTokenErrorResponse);
    BAIL_ON_ERROR(e);

    // create bearer access token
    e = RestAccessTokenNew(&pBearerAccessToken, OidcTokenSuccessResponseGetAccessToken(pTokenSuccessResponse), REST_ACCESS_TOKEN_TYPE_JWT, NULL);
    BAIL_ON_ERROR(e);

    // create bearer token client
    e = RestClientNew(&pRestBearerTokenClient, pscHost, useHA, pscPort, REST_SCHEME_TYPE_HTTPS, pBearerAccessToken);
    BAIL_ON_ERROR(e);

    // create solution user which is used to obtain HOK token
    e = VmdirPrincipalDataNew(&pAlias, "name", "domain");
    BAIL_ON_ERROR(e);

    e = RestCertificateDataNew(&pCertificate, x509Certificate);
    BAIL_ON_ERROR(e);

    e = VmdirSolutionUserDataNew(&pSolutionUser, name, domain, description, pAlias, pCertificate, &disabled, objectId);
    BAIL_ON_ERROR(e);

    e = VmdirSolutionUserCreate(pRestBearerTokenClient, testTenant, pSolutionUser, &pSolutionUserReturn, &pServerError);
    BAIL_ON_ERROR(e);

    // add solution user to regular user group
    e = VmdirStringArrayDataNew(&pMembers, pMemberArray, (size_t) (sizeof(pMemberArray) / sizeof(PSTRING)));
    BAIL_ON_ERROR(e);

    e = VmdirGroupAddMembers(pRestBearerTokenClient, testTenant, groupName, domain, pMembers, memberType, &pServerError);
    BAIL_ON_ERROR(e);

    // OIDC client get HOK token by solution user
    e = OidcClientAcquireTokensBySolutionUserCredentials(
        pOIDCClient,
        certificateSubjectDN,
        privateKey,
        "openid rs_admin_server",
        &pTokenSuccessResponse,
        &pTokenErrorResponse);
    BAIL_ON_ERROR(e);

    // create HOK access token
    e = RestAccessTokenNew(&pHOKAccessToken, OidcTokenSuccessResponseGetAccessToken(pTokenSuccessResponse), REST_ACCESS_TOKEN_TYPE_JWT_HOK, privateKey);
    BAIL_ON_ERROR(e);

    // create HOK token client
    e = RestClientNew(&pRestHOKTokenClient, pscHost, useHA, pscPort, REST_SCHEME_TYPE_HTTPS, pHOKAccessToken);
    BAIL_ON_ERROR(e);

    *ppRestHOKTokenClient = pRestHOKTokenClient;

    error:

    if (e != SSOERROR_NONE)
    {
        RestClientDelete(pRestHOKTokenClient);
    }

    // cleanup REST client data
    RestAccessTokenDelete(pBearerAccessToken);
    RestClientDelete(pRestBearerTokenClient);
    RestAccessTokenDelete(pHOKAccessToken);

    // cleanup OIDC client data
    OidcTokenSuccessResponseDelete(pTokenSuccessResponse);
    OidcErrorResponseDelete(pTokenErrorResponse);
    OidcClientDelete(pOIDCClient);
    OidcClientGlobalCleanup();

    return e;
}
