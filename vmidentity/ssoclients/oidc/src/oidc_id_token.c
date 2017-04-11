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
OidcIDTokenBuild(
    POIDC_ID_TOKEN* pp,
    PCSTRING psz,
    PCSTRING pszSigningCertificatePEM,
    PCSTRING pszIssuer, // not used for now
    SSO_LONG clockToleranceInSeconds)
{
    SSOERROR e = SSOERROR_NONE;
    POIDC_ID_TOKEN p = NULL;
    time_t now = time(NULL);
    bool validSignature = false;

    ASSERT_NOT_NULL(pp);
    ASSERT_NOT_NULL(psz);
    ASSERT_NOT_NULL(pszSigningCertificatePEM);

    if (clockToleranceInSeconds < 0 || clockToleranceInSeconds > MAX_CLOCK_TOLERANCE_IN_SECONDS)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = OidcIDTokenParse(&p, psz);
    BAIL_ON_ERROR(e);

    e = SSOJwtVerifySignature(p->pJwt, pszSigningCertificatePEM, &validSignature);
    BAIL_ON_ERROR(e);
    if (!validSignature)
    {
        e = SSOERROR_TOKEN_INVALID_SIGNATURE;
        BAIL_ON_ERROR(e);
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
        OidcIDTokenDelete(p);
    }

    return e;
}

SSOERROR
OidcIDTokenParse(
    POIDC_ID_TOKEN* pp,
    PCSTRING psz)
{
    SSOERROR e = SSOERROR_NONE;
    POIDC_ID_TOKEN p = NULL;
    PSTRING pszTokenClass = NULL;
    PSTRING pszHolderOfKeyJWKS = NULL;
    PSSO_JWK pJwk = NULL;
    bool hasHokJwksClaim = false;
    bool hasGroupsClaim = false;

    ASSERT_NOT_NULL(pp);
    ASSERT_NOT_NULL(psz);

    e = SSOMemoryAllocate(sizeof(OIDC_ID_TOKEN), (void**) &p);
    BAIL_ON_ERROR(e);

    e = SSOJwtParse(&p->pJwt, psz);
    BAIL_ON_ERROR(e);

    e = SSOJwtGetStringClaim(p->pJwt, "token_class", &pszTokenClass);
    BAIL_ON_ERROR(e);
    if (!SSOStringEqual(pszTokenClass, "id_token"))
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOJwtGetStringClaim(p->pJwt, "iss", &p->pszIssuer);
    BAIL_ON_ERROR(e);
    e = SSOJwtGetStringClaim(p->pJwt, "sub", &p->pszSubject);
    BAIL_ON_ERROR(e);
    e = SSOJwtGetStringClaim(p->pJwt, "aud", &p->pszAudience);
    BAIL_ON_ERROR(e);
    e = SSOJwtGetLongClaim(p->pJwt, "iat", &p->issueTime);
    BAIL_ON_ERROR(e);
    e = SSOJwtGetLongClaim(p->pJwt, "exp", &p->expirationTime);
    BAIL_ON_ERROR(e);

    e = SSOJwtHasClaim(p->pJwt, "groups", &hasGroupsClaim);
    BAIL_ON_ERROR(e);
    if (hasGroupsClaim)
    {
        e = SSOJwtGetStringArrayClaim(p->pJwt, "groups", &p->ppszGroups, &p->groupsSize);
        BAIL_ON_ERROR(e);
    }

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

    *pp = p;

error:

    if (e != SSOERROR_NONE)
    {
        OidcIDTokenDelete(p);
    }

    SSOStringFree(pszTokenClass);
    SSOStringFree(pszHolderOfKeyJWKS);
    SSOJwkDelete(pJwk);

    return e;
}

void
OidcIDTokenDelete(
    POIDC_ID_TOKEN p)
{
    if (p != NULL)
    {
        SSOJwtDelete(p->pJwt);
        SSOStringFree(p->pszIssuer);
        SSOStringFree(p->pszSubject);
        SSOStringFree(p->pszAudience);
        SSOStringFree(p->pszHolderOfKeyPEM);
        SSOMemoryFreeArrayOfObjects((void**) p->ppszGroups, p->groupsSize, (GenericDestructorFunction) SSOStringFree);
        SSOMemoryFree(p, sizeof(OIDC_ID_TOKEN));
    }
}

PCSTRING
OidcIDTokenGetIssuer(
    PCOIDC_ID_TOKEN p)
{
    ASSERT_NOT_NULL(p);
    return p->pszIssuer;
}

PCSTRING
OidcIDTokenGetSubject(
    PCOIDC_ID_TOKEN p)
{
    ASSERT_NOT_NULL(p);
    return p->pszSubject;
}

PCSTRING
OidcIDTokenGetAudience(
    PCOIDC_ID_TOKEN p)
{
    ASSERT_NOT_NULL(p);
    return p->pszAudience;
}

SSO_LONG
OidcIDTokenGetIssueTime(
    PCOIDC_ID_TOKEN p)
{
    ASSERT_NOT_NULL(p);
    return p->issueTime;
}

SSO_LONG
OidcIDTokenGetExpirationTime(
    PCOIDC_ID_TOKEN p)
{
    ASSERT_NOT_NULL(p);
    return p->expirationTime;
}

PCSTRING
OidcIDTokenGetHolderOfKeyPEM(
    PCOIDC_ID_TOKEN p)
{
    ASSERT_NOT_NULL(p);
    return p->pszHolderOfKeyPEM;
}

void
OidcIDTokenGetGroups(
    PCOIDC_ID_TOKEN p,
    const PSTRING** pppszGroups,
    size_t* pGroupsSize)
{
    ASSERT_NOT_NULL(p);
    ASSERT_NOT_NULL(pppszGroups);
    ASSERT_NOT_NULL(pGroupsSize);

    *pppszGroups = p->ppszGroups;
    *pGroupsSize = p->groupsSize;
}

SSOERROR
OidcIDTokenGetStringClaim(
    PCOIDC_ID_TOKEN p,
    PCSTRING pszKey,
    PSTRING* ppszValue)
{
    ASSERT_NOT_NULL(p);
    ASSERT_NOT_NULL(pszKey);
    ASSERT_NOT_NULL(ppszValue);
    return SSOJwtGetStringClaim(p->pJwt, pszKey, ppszValue);
}
