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

static const SSO_LONG MAX_CLOCK_TOLERANCE_IN_SECONDS = 10 * 60; // 10 minutes

SSOERROR
OidcTokenBuild(
    POIDC_TOKEN* pp,
    PCSTRING psz,
    PCSTRING pszSigningCertificatePEM,
    PCSTRING pszIssuer, // not used for now
    PCSTRING pszResourceServerName, /* OPT */
    SSO_LONG clockToleranceInSeconds,
    PCSTRING pszExpectedTokenClass)
{
    SSOERROR e = SSOERROR_NONE;
    POIDC_TOKEN p = NULL;
    time_t now = time(NULL);
    bool validSignature = false;
    bool resourceServerFound = false;
    size_t i = 0;

    BAIL_ON_NULL_ARGUMENT(pp);
    BAIL_ON_NULL_ARGUMENT(psz);
    BAIL_ON_NULL_ARGUMENT(pszSigningCertificatePEM);
    // pszResourceServerName is nullable
    BAIL_ON_NULL_ARGUMENT(pszExpectedTokenClass);

    if (clockToleranceInSeconds < 0 || clockToleranceInSeconds > MAX_CLOCK_TOLERANCE_IN_SECONDS)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = OidcTokenParse(&p, psz, pszExpectedTokenClass);
    BAIL_ON_ERROR(e);

    // validate signature
    e = SSOJwtVerifySignature(p->pJwt, pszSigningCertificatePEM, &validSignature);
    BAIL_ON_ERROR(e);
    if (!validSignature)
    {
        e = SSOERROR_TOKEN_INVALID_SIGNATURE;
        BAIL_ON_ERROR(e);
    }

    // validate resource server name appears in audience
    if (pszResourceServerName != NULL)
    {
        for (i = 0; i < p->audienceSize; i++)
        {
            if (SSOStringEqual(p->ppszAudience[i], pszResourceServerName))
            {
                resourceServerFound = true;
                break;
            }
        }
        if (!resourceServerFound)
        {
            e = SSOERROR_TOKEN_INVALID_AUDIENCE;
            BAIL_ON_ERROR(e);
        }
    }

    // check for notBefore and notAfter
    if (now < p->issueTime - clockToleranceInSeconds || now > p->expirationTime + clockToleranceInSeconds)
    {
        e = SSOERROR_TOKEN_EXPIRED;
        BAIL_ON_ERROR(e);
    }

    *pp = p;

error:

    if (e != SSOERROR_NONE)
    {
        OidcTokenDelete(p);
    }

    return e;
}

SSOERROR
OidcTokenParse(
    POIDC_TOKEN* pp,
    PCSTRING psz,
    PCSTRING pszExpectedTokenClass)
{
    SSOERROR e = SSOERROR_NONE;
    POIDC_TOKEN p = NULL;
    PSTRING pszTokenClass = NULL;
    PSTRING pszTokenType = NULL;
    PSTRING pszHolderOfKeyJWKS = NULL;
    PSSO_JWK pJwk = NULL;
    bool hasAudienceArrayClaim = false;
    bool hasHokJwksClaim = false;
    bool hasTokenClassClaim = false;
    bool hasGroupsClaim = false;

    BAIL_ON_NULL_ARGUMENT(pp);
    BAIL_ON_NULL_ARGUMENT(psz);
    BAIL_ON_NULL_ARGUMENT(pszExpectedTokenClass);

    e = SSOMemoryAllocate(sizeof(OIDC_TOKEN), (void**) &p);
    BAIL_ON_ERROR(e);

    e = SSOJwtParse(&p->pJwt, psz);
    BAIL_ON_ERROR(e);

    e = SSOJwtHasClaim(p->pJwt, "lightwave_token_class", &hasTokenClassClaim);
    BAIL_ON_ERROR(e);
    if (hasTokenClassClaim)
    {
        e = SSOJwtGetStringClaim(p->pJwt, "lightwave_token_class", &pszTokenClass);
        BAIL_ON_ERROR(e);
        if (!SSOStringEqual(pszTokenClass, pszExpectedTokenClass))
        {
            e = SSOERROR_INVALID_ARGUMENT;
            BAIL_ON_ERROR(e);
        }
    }

    e = SSOJwtGetStringClaim(p->pJwt, "token_type", &pszTokenType);
    BAIL_ON_ERROR(e);
    if (SSOStringEqual(pszTokenType, "Bearer"))
    {
        p->tokenType = OIDC_TOKEN_TYPE_BEARER;
    }
    else if (SSOStringEqual(pszTokenType, "hotk-pk"))
    {
        p->tokenType = OIDC_TOKEN_TYPE_HOK;
    }
    else
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOJwtGetStringClaim(p->pJwt, "iss", &p->pszIssuer);
    BAIL_ON_ERROR(e);
    e = SSOJwtGetStringClaim(p->pJwt, "sub", &p->pszSubject);
    BAIL_ON_ERROR(e);

    // aud claim might be a string or an array of strings
    e = SSOJwtHasArrayClaim(p->pJwt, "aud", &hasAudienceArrayClaim);
    BAIL_ON_ERROR(e);
    if (hasAudienceArrayClaim)
    {
        e = SSOJwtGetStringArrayClaim(p->pJwt, "aud", &p->ppszAudience, &p->audienceSize);
        BAIL_ON_ERROR(e);
    }
    else
    {
        e = SSOMemoryAllocateArray(1, sizeof(PSTRING), (void**) &p->ppszAudience);
        BAIL_ON_ERROR(e);
        p->audienceSize = 1;

        e = SSOJwtGetStringClaim(p->pJwt, "aud", &p->ppszAudience[0]);
        BAIL_ON_ERROR(e);
    }

    e = SSOJwtGetLongClaim(p->pJwt, "iat", &p->issueTime);
    BAIL_ON_ERROR(e);
    e = SSOJwtGetLongClaim(p->pJwt, "exp", &p->expirationTime);
    BAIL_ON_ERROR(e);

    e = SSOJwtHasClaim(p->pJwt, "hotk", &hasHokJwksClaim);
    BAIL_ON_ERROR(e);
    if (hasHokJwksClaim)
    {
        e = SSOJwtGetJsonClaim(p->pJwt, "hotk", &pszHolderOfKeyJWKS);
        BAIL_ON_ERROR(e);

        e = SSOJwkParseFromSet(&pJwk, pszHolderOfKeyJWKS);
        BAIL_ON_ERROR(e);

        e = SSOJwkToPublicKeyPEM(pJwk, &p->pszHolderOfKeyPEM);
        BAIL_ON_ERROR(e);
    }

    e = SSOJwtHasClaim(p->pJwt, "lightwave_groups", &hasGroupsClaim);
    BAIL_ON_ERROR(e);
    if (hasGroupsClaim)
    {
        e = SSOJwtHasClaim(p->pJwt, "lightwave_groups", &hasGroupsClaim);
        BAIL_ON_ERROR(e);
    }

    e = SSOJwtGetStringClaim(p->pJwt, "lightwave_tenant", &p->pszTenant);
    BAIL_ON_ERROR(e);

    *pp = p;

error:

    if (e != SSOERROR_NONE)
    {
        OidcTokenDelete(p);
    }

    SSOStringFree(pszTokenClass);
    SSOStringFree(pszTokenType);
    SSOStringFree(pszHolderOfKeyJWKS);
    SSOJwkDelete(pJwk);

    return e;
}

SSOERROR
OidcTokenValidate(
    POIDC_TOKEN p,
    PCSTRING pszSigningCertificatePEM,
    PCSTRING pszIssuer, // not used for now
    PCSTRING pszResourceServerName, /* OPT */
    SSO_LONG clockToleranceInSeconds)
{
    SSOERROR e = SSOERROR_NONE;
    time_t now = time(NULL);
    bool validSignature = false;

    BAIL_ON_NULL_ARGUMENT(p);
    BAIL_ON_NULL_ARGUMENT(pszSigningCertificatePEM);
    // pszResourceServerName is nullable

    if (clockToleranceInSeconds < 0 || clockToleranceInSeconds > MAX_CLOCK_TOLERANCE_IN_SECONDS)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    // validate signature
    e = SSOJwtVerifySignature(p->pJwt, pszSigningCertificatePEM, &validSignature);
    BAIL_ON_ERROR(e);
    if (!validSignature)
    {
        e = SSOERROR_TOKEN_INVALID_SIGNATURE;
        BAIL_ON_ERROR(e);
    }

    // validate resource server name appears in audience
    if (pszResourceServerName != NULL)
    {
        bool resourceServerFound = false;
        size_t i = 0;
        for (i = 0; i < p->audienceSize; i++)
        {
            if (SSOStringEqual(p->ppszAudience[i], pszResourceServerName))
            {
                resourceServerFound = true;
                break;
            }
        }
        if (!resourceServerFound)
        {
            e = SSOERROR_TOKEN_INVALID_AUDIENCE;
            BAIL_ON_ERROR(e);
        }
    }

    // check for notBefore and notAfter
    if (now < p->issueTime - clockToleranceInSeconds || now > p->expirationTime + clockToleranceInSeconds)
    {
        e = SSOERROR_TOKEN_EXPIRED;
        BAIL_ON_ERROR(e);
    }

error:
    return e;
}

void
OidcTokenDelete(
    POIDC_TOKEN p)
{
    if (p != NULL)
    {
        SSOJwtDelete(p->pJwt);
        SSOStringFree(p->pszIssuer);
        SSOStringFree(p->pszSubject);
        SSOStringFree(p->pszHolderOfKeyPEM);
        SSOStringFree(p->pszTenant);
        SSOMemoryFreeArrayOfObjects((void**) p->ppszAudience, p->audienceSize, (GenericDestructorFunction) SSOStringFree);
        SSOMemoryFreeArrayOfObjects((void**) p->ppszGroups, p->groupsSize, (GenericDestructorFunction) SSOStringFree);
        SSOMemoryFree(p, sizeof(OIDC_TOKEN));
    }
}
