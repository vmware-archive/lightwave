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

// (TODO) Deprecated
SSOERROR
OidcAccessTokenBuild(
    POIDC_ACCESS_TOKEN* pp,
    PCSTRING psz,
    PCSTRING pszSigningCertificatePEM,
    PCSTRING pszIssuer, // not used for now
    PCSTRING pszResourceServerName, /* OPT */
    SSO_LONG clockToleranceInSeconds)
{
    SSOERROR e = SSOERROR_NONE;
    POIDC_ACCESS_TOKEN p = NULL;

    BAIL_ON_NULL_ARGUMENT(pp);
    BAIL_ON_NULL_ARGUMENT(psz);
    BAIL_ON_NULL_ARGUMENT(pszSigningCertificatePEM);
    // pszResourceServerName is nullable

    e = SSOMemoryAllocate(sizeof(OIDC_ACCESS_TOKEN), (void**) &p);
    BAIL_ON_ERROR(e);

    e = OidcTokenBuild(&p->pToken, psz, pszSigningCertificatePEM, pszIssuer, pszResourceServerName, clockToleranceInSeconds, "access_token");
    BAIL_ON_ERROR(e);

    *pp = p;

error:

    if (e != SSOERROR_NONE)
    {
        OidcAccessTokenDelete(p);
    }

    return e;
}

// on success, pp will be non-null, when done, OidcAccessTokenDelete it
SSOERROR
OidcAccessTokenParse(
    POIDC_ACCESS_TOKEN* pp,
    PCSTRING psz)
{
    SSOERROR e = SSOERROR_NONE;
    POIDC_ACCESS_TOKEN p = NULL;

    BAIL_ON_NULL_ARGUMENT(pp);
    BAIL_ON_NULL_ARGUMENT(psz);

    e = SSOMemoryAllocate(sizeof(OIDC_ACCESS_TOKEN), (void**) &p);
    BAIL_ON_ERROR(e);

    e = OidcTokenParse(&p->pToken, psz, "access_token");
    BAIL_ON_ERROR(e);

    *pp = p;

error:

    if (e != SSOERROR_NONE)
    {
        OidcAccessTokenDelete(p);
    }

    return e;
}

SSOERROR
OidcAccessTokenValidate(
    POIDC_ACCESS_TOKEN p,
    PCSTRING pszSigningCertificatePEM,
    PCSTRING pszIssuer, // not used for now
    PCSTRING pszResourceServerName,
    SSO_LONG clockToleranceInSeconds)
{
    SSOERROR e = SSOERROR_NONE;

    BAIL_ON_NULL_ARGUMENT(p);

    e = OidcTokenValidate(p->pToken, pszSigningCertificatePEM, pszIssuer, pszResourceServerName, clockToleranceInSeconds);
    BAIL_ON_ERROR(e);

error:
    return e;
}

void
OidcAccessTokenDelete(
    POIDC_ACCESS_TOKEN p)
{
    if (p != NULL)
    {
        OidcTokenDelete(p->pToken);
        SSOMemoryFree(p, sizeof(OIDC_ACCESS_TOKEN));
    }
}

OIDC_TOKEN_TYPE
OidcAccessTokenGetTokenType(
    PCOIDC_ACCESS_TOKEN p)
{
    ASSERT_NOT_NULL(p);
    return p->pToken->tokenType;
}

PCSTRING
OidcAccessTokenGetIssuer(
    PCOIDC_ACCESS_TOKEN p)
{
    ASSERT_NOT_NULL(p);
    return p->pToken->pszIssuer;
}

PCSTRING
OidcAccessTokenGetSubject(
    PCOIDC_ACCESS_TOKEN p)
{
    ASSERT_NOT_NULL(p);
    return p->pToken->pszSubject;
}

void
OidcAccessTokenGetAudience(
    PCOIDC_ACCESS_TOKEN p,
    const PSTRING** pppszAudience,
    size_t* pAudienceSize)
{
    ASSERT_NOT_NULL(p);
    ASSERT_NOT_NULL(pppszAudience);
    ASSERT_NOT_NULL(pAudienceSize);

    *pppszAudience = p->pToken->ppszAudience;
    *pAudienceSize = p->pToken->audienceSize;
}

size_t
OidcAccessTokenGetAudienceSize(
    PCOIDC_ACCESS_TOKEN p)
{
    ASSERT_NOT_NULL(p);
    return p->pToken->audienceSize;
}

PCSTRING
OidcAccessTokenGetAudienceEntry(
    PCOIDC_ACCESS_TOKEN p,
    int index)
{
    ASSERT_NOT_NULL(p);
    ASSERT_TRUE(0 <= index && index < p->pToken->audienceSize);
    return p->pToken->ppszAudience[index];
}

SSO_LONG
OidcAccessTokenGetIssueTime(
    PCOIDC_ACCESS_TOKEN p)
{
    ASSERT_NOT_NULL(p);
    return p->pToken->issueTime;
}

SSO_LONG
OidcAccessTokenGetExpirationTime(
    PCOIDC_ACCESS_TOKEN p)
{
    ASSERT_NOT_NULL(p);
    return p->pToken->expirationTime;
}

PCSTRING
OidcAccessTokenGetHolderOfKeyPEM(
    PCOIDC_ACCESS_TOKEN p)
{
    ASSERT_NOT_NULL(p);
    return p->pToken->pszHolderOfKeyPEM;
}

void
OidcAccessTokenGetGroups(
    PCOIDC_ACCESS_TOKEN p,
    const PSTRING** pppszGroups,
    size_t* pGroupsSize)
{
    ASSERT_NOT_NULL(p);
    ASSERT_NOT_NULL(pppszGroups);
    ASSERT_NOT_NULL(pGroupsSize);

    *pppszGroups = p->pToken->ppszGroups;
    *pGroupsSize = p->pToken->groupsSize;
}

size_t
OidcAccessTokenGetGroupsSize(
    PCOIDC_ACCESS_TOKEN p)
{
    ASSERT_NOT_NULL(p);
    return p->pToken->groupsSize;
}

PCSTRING
OidcAccessTokenGetGroupsEntry(
    PCOIDC_ACCESS_TOKEN p,
    int index)
{
    ASSERT_NOT_NULL(p);
    ASSERT_TRUE(0 <= index && index < p->pToken->groupsSize);
    return p->pToken->ppszGroups[index];

}

PCSTRING
OidcAccessTokenGetTenant(
    PCOIDC_ACCESS_TOKEN p)
{
    ASSERT_NOT_NULL(p);
    return p->pToken->pszTenant;
}

SSOERROR
OidcAccessTokenGetStringClaim(
    PCOIDC_ACCESS_TOKEN p,
    PCSTRING pszKey,
    PSTRING* ppszValue)
{
    ASSERT_NOT_NULL(p);
    ASSERT_NOT_NULL(pszKey);
    ASSERT_NOT_NULL(ppszValue);
    return SSOJwtGetStringClaim(p->pToken->pJwt, pszKey, ppszValue);
}
