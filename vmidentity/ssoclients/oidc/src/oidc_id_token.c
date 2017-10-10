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
OidcIDTokenBuild(
    POIDC_ID_TOKEN* pp,
    PCSTRING psz,
    PCSTRING pszSigningCertificatePEM,
    PCSTRING pszIssuer, // not used for now
    SSO_LONG clockToleranceInSeconds)
{
    SSOERROR e = SSOERROR_NONE;
    POIDC_ID_TOKEN p = NULL;

    BAIL_ON_NULL_ARGUMENT(pp);
    BAIL_ON_NULL_ARGUMENT(psz);
    BAIL_ON_NULL_ARGUMENT(pszSigningCertificatePEM);

    e = SSOMemoryAllocate(sizeof(OIDC_ID_TOKEN), (void**) &p);
    BAIL_ON_ERROR(e);

    e = OidcTokenBuild(&p->pToken, psz, pszSigningCertificatePEM, pszIssuer, NULL /* pszResourceServerName */, clockToleranceInSeconds, "id_token");
    BAIL_ON_ERROR(e);

    *pp = p;

error:

    if (e != SSOERROR_NONE)
    {
        OidcIDTokenDelete(p);
    }

    return e;
}

// on success, pp will be non-null, when done, OidcIDTokenDelete it
SSOERROR
OidcIDTokenParse(
    POIDC_ID_TOKEN* pp,
    PCSTRING psz)
{
    SSOERROR e = SSOERROR_NONE;
    POIDC_ID_TOKEN p = NULL;

    BAIL_ON_NULL_ARGUMENT(pp);
    BAIL_ON_NULL_ARGUMENT(psz);

    e = SSOMemoryAllocate(sizeof(OIDC_ID_TOKEN), (void**) &p);
    BAIL_ON_ERROR(e);

    e = OidcTokenParse(&p->pToken, psz, "id_token");
    BAIL_ON_ERROR(e);

    *pp = p;

error:

    if (e != SSOERROR_NONE)
    {
        OidcIDTokenDelete(p);
    }

    return e;
}

SSOERROR
OidcIDTokenValidate(
    POIDC_ID_TOKEN p,
    PCSTRING pszSigningCertificatePEM,
    PCSTRING pszIssuer, // not used for now
    SSO_LONG clockToleranceInSeconds)
{
    SSOERROR e = SSOERROR_NONE;

    BAIL_ON_NULL_ARGUMENT(p);

    e = OidcTokenValidate(p->pToken, pszSigningCertificatePEM, pszIssuer, NULL, clockToleranceInSeconds);
    BAIL_ON_ERROR(e);

error:
    return e;
}

void
OidcIDTokenDelete(
    POIDC_ID_TOKEN p)
{
    if (p != NULL)
    {
        OidcTokenDelete(p->pToken);
        SSOMemoryFree(p, sizeof(OIDC_ID_TOKEN));
    }
}

OIDC_TOKEN_TYPE
OidcIDTokenGetTokenType(
    PCOIDC_ID_TOKEN p)
{
    ASSERT_NOT_NULL(p);
    return p->pToken->tokenType;
}

PCSTRING
OidcIDTokenGetIssuer(
    PCOIDC_ID_TOKEN p)
{
    ASSERT_NOT_NULL(p);
    return p->pToken->pszIssuer;
}

PCSTRING
OidcIDTokenGetSubject(
    PCOIDC_ID_TOKEN p)
{
    ASSERT_NOT_NULL(p);
    return p->pToken->pszSubject;
}

void
OidcIDTokenGetAudience(
    PCOIDC_ID_TOKEN p,
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
OidcIDTokenGetAudienceSize(
    PCOIDC_ID_TOKEN p)
{
    ASSERT_NOT_NULL(p);
    return p->pToken->audienceSize;
}

PCSTRING
OidcIDTokenGetAudienceEntry(
    PCOIDC_ID_TOKEN p,
    int index)
{
    ASSERT_NOT_NULL(p);
    ASSERT_TRUE(0 <= index && index < p->pToken->audienceSize);
    return p->pToken->ppszAudience[index];
}

SSO_LONG
OidcIDTokenGetIssueTime(
    PCOIDC_ID_TOKEN p)
{
    ASSERT_NOT_NULL(p);
    return p->pToken->issueTime;
}

SSO_LONG
OidcIDTokenGetExpirationTime(
    PCOIDC_ID_TOKEN p)
{
    ASSERT_NOT_NULL(p);
    return p->pToken->expirationTime;
}

PCSTRING
OidcIDTokenGetHolderOfKeyPEM(
    PCOIDC_ID_TOKEN p)
{
    ASSERT_NOT_NULL(p);
    return p->pToken->pszHolderOfKeyPEM;
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

    *pppszGroups = p->pToken->ppszGroups;
    *pGroupsSize = p->pToken->groupsSize;
}

size_t
OidcIDTokenGetGroupsSize(
    PCOIDC_ID_TOKEN p)
{
    ASSERT_NOT_NULL(p);
    return p->pToken->groupsSize;
}

PCSTRING
OidcIDTokenGetGroupsEntry(
    PCOIDC_ID_TOKEN p,
    int index)
{
    ASSERT_NOT_NULL(p);
    ASSERT_TRUE(0 <= index && index < p->pToken->groupsSize);
    return p->pToken->ppszGroups[index];

}

PCSTRING
OidcIDTokenGetTenant(
    PCOIDC_ID_TOKEN p)
{
    ASSERT_NOT_NULL(p);
    return p->pToken->pszTenant;
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
    return SSOJwtGetStringClaim(p->pToken->pJwt, pszKey, ppszValue);
}
