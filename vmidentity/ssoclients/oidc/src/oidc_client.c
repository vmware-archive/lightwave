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

SSOERROR
OidcClientGlobalInit()
{
    return SSOHttpClientGlobalInit();
}

void
OidcClientGlobalCleanup()
{
    SSOHttpClientGlobalCleanup();
}

SSOERROR
OidcClientBuild(
    POIDC_CLIENT* pp,
    PCSTRING pszServer, // OPT: null means use HA to get affinitized host
    int portNumber,
    PCSTRING pszTenant,
    SSO_LONG clockToleranceInSeconds)
{
    SSOERROR e = SSOERROR_NONE;
    POIDC_CLIENT p = NULL;
    POIDC_SERVER_METADATA pServerMetadata = NULL;

    BAIL_ON_NULL_ARGUMENT(pp);
    BAIL_ON_NULL_ARGUMENT(pszTenant);

    e = SSOMemoryAllocate(sizeof(OIDC_CLIENT), (void**) &p);
    BAIL_ON_ERROR(e);

    p->clockToleranceInSeconds = clockToleranceInSeconds;

    if (NULL == pszServer)
    {
        e = SSOCdcNew(&p->pClientDCCache);
        BAIL_ON_ERROR(e);

        e = SSOCdcGetAffinitizedHost(
            p->pClientDCCache,
            NULL, // PCSTRING domainName
            0, // int cdcFlags,
            &p->pszServer);
        BAIL_ON_ERROR(e);
    }
    else
    {
        e = SSOStringAllocate(pszServer, &p->pszServer);
        BAIL_ON_ERROR(e);
    }

    e = OidcServerMetadataAcquire(&pServerMetadata, p->pszServer, portNumber, pszTenant);
    BAIL_ON_ERROR(e);

    e = SSOStringAllocate(OidcServerMetadataGetTokenEndpointUrl(pServerMetadata), &p->pszTokenEndpointUrl);
    BAIL_ON_ERROR(e);

    e = SSOStringAllocate(OidcServerMetadataGetSigningCertificatePEM(pServerMetadata), &p->pszSigningCertificatePEM);
    BAIL_ON_ERROR(e);

    *pp = p;

error:

    if (e != SSOERROR_NONE)
    {
        OidcClientDelete(p);
    }

    OidcServerMetadataDelete(pServerMetadata);

    return e;
}

void
OidcClientDelete(
    POIDC_CLIENT p)
{
    if (p != NULL)
    {
        SSOStringFree(p->pszServer);
        SSOStringFree(p->pszTokenEndpointUrl);
        SSOStringFree(p->pszSigningCertificatePEM);
        SSOCdcDelete(p->pClientDCCache);
        SSOMemoryFree(p, sizeof(OIDC_CLIENT));
    }
}

static
SSOERROR
OidcClientBuildSolutionUserAssertionPayload(
    PCOIDC_CLIENT p,
    PCSTRING pszCertificateSubjectDN,
    PSTRING* ppsz /* OUT */)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING psz = NULL;
    PSSO_JSON pJson = NULL;
    PSSO_JSON pJsonValue = NULL; // do not delete this
    PSTRING pszJwtID = NULL;
    time_t currentTime = time(NULL);

    // use current time as the jwt-id (jti), not as good as uuid but simpler to generate
    e = SSOStringAllocateFromInt(currentTime, &pszJwtID);
    BAIL_ON_ERROR(e);

    e = SSOJsonObjectNew(&pJson);
    BAIL_ON_ERROR(e);

    e = SSOJsonStringNew(&pJsonValue, "solution_user_assertion");
    BAIL_ON_ERROR(e);
    e = SSOJsonObjectSet(pJson, "token_class", pJsonValue);
    BAIL_ON_ERROR(e);
    SSOJsonDelete(pJsonValue);
    pJsonValue = NULL;

    e = SSOJsonStringNew(&pJsonValue, "Bearer");
    BAIL_ON_ERROR(e);
    e = SSOJsonObjectSet(pJson, "token_type", pJsonValue);
    BAIL_ON_ERROR(e);
    SSOJsonDelete(pJsonValue);
    pJsonValue = NULL;

    e = SSOJsonStringNew(&pJsonValue, pszJwtID);
    BAIL_ON_ERROR(e);
    e = SSOJsonObjectSet(pJson, "jti", pJsonValue);
    BAIL_ON_ERROR(e);
    SSOJsonDelete(pJsonValue);
    pJsonValue = NULL;

    e = SSOJsonStringNew(&pJsonValue, pszCertificateSubjectDN);
    BAIL_ON_ERROR(e);
    e = SSOJsonObjectSet(pJson, "iss", pJsonValue);
    BAIL_ON_ERROR(e);
    SSOJsonDelete(pJsonValue);
    pJsonValue = NULL;

    e = SSOJsonStringNew(&pJsonValue, pszCertificateSubjectDN);
    BAIL_ON_ERROR(e);
    e = SSOJsonObjectSet(pJson, "sub", pJsonValue);
    BAIL_ON_ERROR(e);
    SSOJsonDelete(pJsonValue);
    pJsonValue = NULL;

    e = SSOJsonStringNew(&pJsonValue, p->pszTokenEndpointUrl);
    BAIL_ON_ERROR(e);
    e = SSOJsonObjectSet(pJson, "aud", pJsonValue);
    BAIL_ON_ERROR(e);
    SSOJsonDelete(pJsonValue);
    pJsonValue = NULL;

    e = SSOJsonLongNew(&pJsonValue, currentTime);
    BAIL_ON_ERROR(e);
    e = SSOJsonObjectSet(pJson, "iat", pJsonValue);
    BAIL_ON_ERROR(e);
    SSOJsonDelete(pJsonValue);
    pJsonValue = NULL;

    e = SSOJsonToString(pJson, &psz);
    BAIL_ON_ERROR(e);

    *ppsz = psz;

error:

    if (e != SSOERROR_NONE)
    {
        SSOStringFree(psz);
    }

    SSOJsonDelete(pJson); // pJsonValue's will be deleted by the this call
    SSOJsonDelete(pJsonValue);
    SSOStringFree(pszJwtID);

    return e;
}

static
SSOERROR
OidcClientAcquireTokens(
    PCOIDC_CLIENT p,
    PSSO_KEY_VALUE_PAIR* ppPairs,
    size_t parameterCount,
    POIDC_TOKEN_SUCCESS_RESPONSE* ppOutTokenSuccessResponse, /* OUT */
    POIDC_ERROR_RESPONSE*   ppOutTokenErrorResponse /* OUT */)
{
    SSOERROR e = SSOERROR_NONE;

    POIDC_TOKEN_SUCCESS_RESPONSE pOutTokenSuccessResponse = NULL;
    POIDC_ERROR_RESPONSE   pOutTokenErrorResponse = NULL;

    PSSO_HTTP_CLIENT pHttpClient = NULL;
    PSTRING pszJsonResponse = NULL;
    PSTRING pszAffinitizedTokenEndpointUrl = NULL;
    PSTRING pszAffinitizedHost = NULL;
    long httpStatusCode = 0;

    if (p->pClientDCCache != NULL) // highAvailabilityEnabled
    {
        e = SSOCdcGetAffinitizedHost(
            p->pClientDCCache,
            NULL, // PCSTRING domainName
            0, // int cdcFlags,
            &pszAffinitizedHost);
        BAIL_ON_ERROR(e);

        if (!SSOStringEqual(pszAffinitizedHost, p->pszServer))
        {
            e = SSOStringReplace(p->pszTokenEndpointUrl, p->pszServer, pszAffinitizedHost, &pszAffinitizedTokenEndpointUrl);
            BAIL_ON_ERROR(e);
        }
    }

    e = SSOHttpClientNew(&pHttpClient);
    BAIL_ON_ERROR(e);
    e = SSOHttpClientSendPostForm(
        pHttpClient,
        (pszAffinitizedTokenEndpointUrl != NULL) ? pszAffinitizedTokenEndpointUrl : p->pszTokenEndpointUrl,
        NULL, // pszHeaders
        0, // headerCount
        ppPairs,
        parameterCount,
        &pszJsonResponse,
        &httpStatusCode);
    BAIL_ON_ERROR(e);

    if (200 == httpStatusCode)
    {
        e = OidcTokenSuccessResponseParse(&pOutTokenSuccessResponse, pszJsonResponse, p->pszSigningCertificatePEM, p->clockToleranceInSeconds);
        BAIL_ON_ERROR(e);
    }
    else
    {
        e = OidcErrorResponseParse(&pOutTokenErrorResponse, pszJsonResponse);
        BAIL_ON_ERROR(e);
    }

    *ppOutTokenSuccessResponse = pOutTokenSuccessResponse;
    *ppOutTokenErrorResponse = pOutTokenErrorResponse;
    if (pOutTokenErrorResponse != NULL)
    {
        // if server return error response, we translate that into an SSOERROR code
        e = OidcErrorResponseGetSSOErrorCode(pOutTokenErrorResponse);
    }

error:
    SSOHttpClientDelete(pHttpClient);
    SSOStringFree(pszAffinitizedTokenEndpointUrl);
    SSOStringFree(pszAffinitizedHost);
    SSOStringFree(pszJsonResponse);
    return e;
}

SSOERROR
OidcClientAcquireTokensByPassword(
    PCOIDC_CLIENT p,
    PCSTRING pszUsername,
    PCSTRING pszPassword,
    PCSTRING pszScope,
    POIDC_TOKEN_SUCCESS_RESPONSE* ppOutTokenSuccessResponse, /* OUT */
    POIDC_ERROR_RESPONSE*   ppOutTokenErrorResponse /* OUT */)
{
    SSOERROR e = SSOERROR_NONE;
    PSSO_KEY_VALUE_PAIR* ppPairs = NULL;
    const int parameterCount = 4;

    BAIL_ON_NULL_ARGUMENT(p);
    BAIL_ON_NULL_ARGUMENT(pszUsername);
    BAIL_ON_NULL_ARGUMENT(pszPassword);
    BAIL_ON_NULL_ARGUMENT(pszScope);
    BAIL_ON_NULL_ARGUMENT(ppOutTokenSuccessResponse);
    BAIL_ON_NULL_ARGUMENT(ppOutTokenErrorResponse);

    e = SSOMemoryAllocateArray(parameterCount, sizeof(PSSO_KEY_VALUE_PAIR), (void**) &ppPairs);
    BAIL_ON_ERROR(e);
    e = SSOKeyValuePairNew(&ppPairs[0], "grant_type", "password");
    BAIL_ON_ERROR(e);
    e = SSOKeyValuePairNew(&ppPairs[1], "username", pszUsername);
    BAIL_ON_ERROR(e);
    e = SSOKeyValuePairNew(&ppPairs[2], "password", pszPassword);
    BAIL_ON_ERROR(e);
    e = SSOKeyValuePairNew(&ppPairs[3], "scope", pszScope);
    BAIL_ON_ERROR(e);

    e = OidcClientAcquireTokens(p, ppPairs, parameterCount, ppOutTokenSuccessResponse, ppOutTokenErrorResponse);
    BAIL_ON_ERROR(e);

error:
    SSOMemoryFreeArrayOfObjects((void**) ppPairs, parameterCount, (GenericDestructorFunction) SSOKeyValuePairDelete);
    return e;
}

SSOERROR
OidcClientAcquireTokensByRefreshToken(
    PCOIDC_CLIENT p,
    PCSTRING pszRefreshToken,
    POIDC_TOKEN_SUCCESS_RESPONSE* ppOutTokenSuccessResponse, /* OUT */
    POIDC_ERROR_RESPONSE*   ppOutTokenErrorResponse /* OUT */)
{
    SSOERROR e = SSOERROR_NONE;
    PSSO_KEY_VALUE_PAIR* ppPairs = NULL;
    const int parameterCount = 2;

    BAIL_ON_NULL_ARGUMENT(p);
    BAIL_ON_NULL_ARGUMENT(pszRefreshToken);
    BAIL_ON_NULL_ARGUMENT(ppOutTokenSuccessResponse);
    BAIL_ON_NULL_ARGUMENT(ppOutTokenErrorResponse);

    e = SSOMemoryAllocateArray(parameterCount, sizeof(PSSO_KEY_VALUE_PAIR), (void**) &ppPairs);
    BAIL_ON_ERROR(e);
    e = SSOKeyValuePairNew(&ppPairs[0], "grant_type", "refresh_token");
    BAIL_ON_ERROR(e);
    e = SSOKeyValuePairNew(&ppPairs[1], "refresh_token", pszRefreshToken);
    BAIL_ON_ERROR(e);

    e = OidcClientAcquireTokens(p, ppPairs, parameterCount, ppOutTokenSuccessResponse, ppOutTokenErrorResponse);
    BAIL_ON_ERROR(e);

error:
    SSOMemoryFreeArrayOfObjects((void**) ppPairs, parameterCount, (GenericDestructorFunction) SSOKeyValuePairDelete);
    return e;
}

SSOERROR
OidcClientAcquireTokensBySolutionUserCredentials(
    PCOIDC_CLIENT p,
    PCSTRING pszCertificateSubjectDN,
    PCSTRING pszPrivateKeyPEM,
    PCSTRING pszScope,
    POIDC_TOKEN_SUCCESS_RESPONSE* ppOutTokenSuccessResponse, /* OUT */
    POIDC_ERROR_RESPONSE* ppOutTokenErrorResponse /* OUT */)
{
    SSOERROR e = SSOERROR_NONE;
    PSSO_KEY_VALUE_PAIR* ppPairs = NULL;
    const int parameterCount = 3;
    PSTRING pszAssertionPayload = NULL;
    PSTRING pszAssertionJwtString = NULL;

    BAIL_ON_NULL_ARGUMENT(p);
    BAIL_ON_NULL_ARGUMENT(pszCertificateSubjectDN);
    BAIL_ON_NULL_ARGUMENT(pszPrivateKeyPEM);
    BAIL_ON_NULL_ARGUMENT(pszScope);
    BAIL_ON_NULL_ARGUMENT(ppOutTokenSuccessResponse);
    BAIL_ON_NULL_ARGUMENT(ppOutTokenErrorResponse);

    e = OidcClientBuildSolutionUserAssertionPayload(p, pszCertificateSubjectDN, &pszAssertionPayload);
    BAIL_ON_ERROR(e);

    e = SSOJwtCreateSignedJwtString(pszAssertionPayload, pszPrivateKeyPEM, &pszAssertionJwtString);
    BAIL_ON_ERROR(e);

    e = SSOMemoryAllocateArray(parameterCount, sizeof(PSSO_KEY_VALUE_PAIR), (void**) &ppPairs);
    BAIL_ON_ERROR(e);
    e = SSOKeyValuePairNew(&ppPairs[0], "grant_type", "urn:vmware:grant_type:solution_user_credentials");
    BAIL_ON_ERROR(e);
    e = SSOKeyValuePairNew(&ppPairs[1], "solution_user_assertion", pszAssertionJwtString);
    BAIL_ON_ERROR(e);
    e = SSOKeyValuePairNew(&ppPairs[2], "scope", pszScope);
    BAIL_ON_ERROR(e);

    e = OidcClientAcquireTokens(p, ppPairs, parameterCount, ppOutTokenSuccessResponse, ppOutTokenErrorResponse);
    BAIL_ON_ERROR(e);

error:

    SSOMemoryFreeArrayOfObjects((void**) ppPairs, parameterCount, (GenericDestructorFunction) SSOKeyValuePairDelete);
    SSOStringFree(pszAssertionPayload);
    SSOStringFree(pszAssertionJwtString);

    return e;
}

PCSTRING
OidcClientGetSigningCertificatePEM(
    PCOIDC_CLIENT p)
{
    ASSERT_NOT_NULL(p);
    return p->pszSigningCertificatePEM;
}
