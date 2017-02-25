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

// https://<server>/openidconnect/<tenant>/.well-known/openid-configuration
static
SSOERROR
OidcServerMetadataConstructMetadataEndpoint(
    PCSTRING pszServer,
    int portNumber,
    PCSTRING pszTenant,
    PSTRING* ppszEndpoint)
{
    SSOERROR e = SSOERROR_NONE;

    PSTRING pszEndpoint = NULL;
    PSTRING pszPortNumber = NULL;
    PSSO_STRING_BUILDER pSB = NULL;

    e = SSOStringAllocateFromInt(portNumber, &pszPortNumber);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderNew(&pSB);
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderAppend(pSB, "https://");
    BAIL_ON_ERROR(e);
    e = SSOStringBuilderAppend(pSB, pszServer);
    BAIL_ON_ERROR(e);
    e = SSOStringBuilderAppend(pSB, ":");
    BAIL_ON_ERROR(e);
    e = SSOStringBuilderAppend(pSB, pszPortNumber);
    BAIL_ON_ERROR(e);
    e = SSOStringBuilderAppend(pSB, "/openidconnect/");
    BAIL_ON_ERROR(e);
    e = SSOStringBuilderAppend(pSB, pszTenant);
    BAIL_ON_ERROR(e);
    e = SSOStringBuilderAppend(pSB, "/.well-known/openid-configuration");
    BAIL_ON_ERROR(e);

    e = SSOStringBuilderGetString(pSB, &pszEndpoint);
    BAIL_ON_ERROR(e);

    *ppszEndpoint = pszEndpoint;

error:

    if (e != SSOERROR_NONE)
    {
        SSOStringFree(pszEndpoint);
    }

    SSOStringFree(pszPortNumber);
    SSOStringBuilderDelete(pSB);

    return e;
}

static
SSOERROR
OidcServerMetadataAcquireEndpoints(
    PCSSO_HTTP_CLIENT pHttpClient,
    PCSTRING pszMetadataEndpointUrl,
    PSTRING* ppszTokenEndpointUrl,
    PSTRING* ppszJwksEndpointUrl)
{
    SSOERROR e = SSOERROR_NONE;

    PSTRING pszTokenEndpointUrl = NULL;
    PSTRING pszJwksEndpointUrl = NULL;
    PSTRING pszResponse = NULL;
    long httpStatusCode = 0;
    PSSO_JSON pJson = NULL;
    PSSO_JSON pJsonTokenEndpoint = NULL;
    PSSO_JSON pJsonJwksEndpoint = NULL;

    e = SSOHttpClientSendGet(
        pHttpClient,
        pszMetadataEndpointUrl,
        NULL, // ppszHeaders
        0, // headerCount
        &pszResponse,
        &httpStatusCode);
    BAIL_ON_ERROR(e);

    e = SSOJsonParse(&pJson, pszResponse);
    BAIL_ON_ERROR(e);

    e = SSOJsonObjectGet(pJson, "token_endpoint", &pJsonTokenEndpoint);
    BAIL_ON_ERROR(e);
    e = SSOJsonStringValue(pJsonTokenEndpoint, &pszTokenEndpointUrl);
    BAIL_ON_ERROR(e);

    e = SSOJsonObjectGet(pJson, "jwks_uri", &pJsonJwksEndpoint);
    BAIL_ON_ERROR(e);
    e = SSOJsonStringValue(pJsonJwksEndpoint, &pszJwksEndpointUrl);
    BAIL_ON_ERROR(e);

    *ppszTokenEndpointUrl = pszTokenEndpointUrl;
    *ppszJwksEndpointUrl = pszJwksEndpointUrl;

error:

    if (e != SSOERROR_NONE)
    {
        SSOStringFree(pszTokenEndpointUrl);
        SSOStringFree(pszJwksEndpointUrl);
    }

    SSOStringFree(pszResponse);
    SSOJsonDelete(pJson);
    SSOJsonDelete(pJsonTokenEndpoint);
    SSOJsonDelete(pJsonJwksEndpoint);

    return e;
}

static
SSOERROR
OidcServerMetadataAcquireSigningCertificatePEM(
    PCSSO_HTTP_CLIENT pHttpClient,
    PCSTRING pszJwksEndpoint,
    PSTRING* ppsz)
{
    SSOERROR e = SSOERROR_NONE;

    PSTRING psz = NULL;
    PSSO_JWK pJwk = NULL;
    PSTRING pszResponse = NULL;
    long httpStatusCode = 0;

    e = SSOHttpClientSendGet(
        pHttpClient,
        pszJwksEndpoint,
        NULL, // ppszHeaders
        0, // headerCount
        &pszResponse,
        &httpStatusCode);
    BAIL_ON_ERROR(e);

    e = SSOJwkParseFromSet(&pJwk, pszResponse);
    BAIL_ON_ERROR(e);

    e = SSOJwkToCertificatePEM(pJwk, &psz);
    BAIL_ON_ERROR(e);

    *ppsz = psz;

error:

    if (e != SSOERROR_NONE)
    {
        SSOStringFree(psz);
    }

    SSOJwkDelete(pJwk);
    SSOStringFree(pszResponse);

    return e;
}

SSOERROR
OidcServerMetadataAcquire(
    POIDC_SERVER_METADATA* pp,
    PCSTRING pszServer,
    int portNumber,
    PCSTRING pszTenant)
{
    SSOERROR e = SSOERROR_NONE;
    POIDC_SERVER_METADATA p = NULL;
    PSSO_HTTP_CLIENT pHttpClient = NULL;
    PSTRING pszMetadataEndpoint = NULL;
    PSTRING pszJwksEndpoint = NULL;

    BAIL_ON_NULL_ARGUMENT(pp);
    BAIL_ON_NULL_ARGUMENT(pszServer);
    BAIL_ON_NULL_ARGUMENT(pszTenant);

    e = SSOMemoryAllocate(sizeof(OIDC_SERVER_METADATA), (void**) &p);
    BAIL_ON_ERROR(e);

    e = SSOHttpClientNew(&pHttpClient);
    BAIL_ON_ERROR(e);

    e = OidcServerMetadataConstructMetadataEndpoint(pszServer, portNumber, pszTenant, &pszMetadataEndpoint);
    BAIL_ON_ERROR(e);

    e = OidcServerMetadataAcquireEndpoints(pHttpClient, pszMetadataEndpoint, &p->pszTokenEndpointUrl, &pszJwksEndpoint);
    BAIL_ON_ERROR(e);

    e = OidcServerMetadataAcquireSigningCertificatePEM(pHttpClient, pszJwksEndpoint, &p->pszSigningCertificatePEM);
    BAIL_ON_ERROR(e);

    *pp = p;

error:

    SSOHttpClientDelete(pHttpClient);
    SSOStringFree(pszMetadataEndpoint);
    SSOStringFree(pszJwksEndpoint);

    if (e != SSOERROR_NONE)
    {
        OidcServerMetadataDelete(p);
    }

    return e;
}

void
OidcServerMetadataDelete(
    POIDC_SERVER_METADATA p)
{
    if (p != NULL)
    {
        SSOStringFree(p->pszTokenEndpointUrl);
        SSOStringFree(p->pszSigningCertificatePEM);
        SSOMemoryFree(p, sizeof(OIDC_SERVER_METADATA));
    }
}

PCSTRING
OidcServerMetadataGetTokenEndpointUrl(
    PCOIDC_SERVER_METADATA p)
{
    ASSERT_NOT_NULL(p);
    return p->pszTokenEndpointUrl;
}

PCSTRING
OidcServerMetadataGetSigningCertificatePEM(
    PCOIDC_SERVER_METADATA p)
{
    ASSERT_NOT_NULL(p);
    return p->pszSigningCertificatePEM;
}
