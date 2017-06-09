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

#ifndef _OIDC_H_
#define _OIDC_H_

// OIDC_CLIENT

SSOERROR
OidcClientGlobalInit();

void
OidcClientGlobalCleanup();

SSOERROR
OidcClientBuild(
    POIDC_CLIENT* pp,
    PCSTRING pszServer, // OPT: null means use HA to get affinitized host
    int portNumber,
    PCSTRING pszTenant,
    SSO_LONG clockToleranceInSeconds);

void
OidcClientDelete(
    POIDC_CLIENT p);

SSOERROR
OidcClientAcquireTokensByPassword(
    PCOIDC_CLIENT p,
    PCSTRING pszUsername,
    PCSTRING pszPassword,
    PCSTRING pszScope,
    POIDC_TOKEN_SUCCESS_RESPONSE* ppOutTokenSuccessResponse, /* OUT */
    POIDC_ERROR_RESPONSE* ppOutTokenErrorResponse /* OUT */);

SSOERROR
OidcClientAcquireTokensByRefreshToken(
    PCOIDC_CLIENT p,
    PCSTRING pszRefreshToken,
    POIDC_TOKEN_SUCCESS_RESPONSE* ppOutTokenSuccessResponse, /* OUT */
    POIDC_ERROR_RESPONSE* ppOutTokenErrorResponse /* OUT */);

SSOERROR
OidcClientAcquireTokensBySolutionUserCredentials(
    PCOIDC_CLIENT p,
    PCSTRING pszCertificateSubjectDN,
    PCSTRING pszPrivateKeyPEM,
    PCSTRING pszScope,
    POIDC_TOKEN_SUCCESS_RESPONSE* ppOutTokenSuccessResponse, /* OUT */
    POIDC_ERROR_RESPONSE* ppOutTokenErrorResponse /* OUT */);

PCSTRING
OidcClientGetSigningCertificatePEM(
    PCOIDC_CLIENT p);

// OIDC_SERVER_METADATA

SSOERROR
OidcServerMetadataAcquire(
    POIDC_SERVER_METADATA* pp,
    PCSTRING pszServer,
    int portNumber,
    PCSTRING pszTenant);

void
OidcServerMetadataDelete(
    POIDC_SERVER_METADATA p);

PCSTRING
OidcServerMetadataGetTokenEndpointUrl(
    PCOIDC_SERVER_METADATA p);

PCSTRING
OidcServerMetadataGetSigningCertificatePEM(
    PCOIDC_SERVER_METADATA p);

// OIDC_ID_TOKEN

SSOERROR
OidcIDTokenBuild(
    POIDC_ID_TOKEN* pp,
    PCSTRING psz,
    PCSTRING pszSigningCertificatePEM,
    PCSTRING pszIssuer, // not used for now
    SSO_LONG clockToleranceInSeconds);

void
OidcIDTokenDelete(
    POIDC_ID_TOKEN p);

OIDC_TOKEN_TYPE
OidcIDTokenGetTokenType(
    PCOIDC_ID_TOKEN p);

PCSTRING
OidcIDTokenGetIssuer(
    PCOIDC_ID_TOKEN p);

PCSTRING
OidcIDTokenGetSubject(
    PCOIDC_ID_TOKEN p);

void
OidcIDTokenGetAudience(
    PCOIDC_ID_TOKEN p,
    const PSTRING** pppszAudience,
    size_t* pAudienceSize);

SSO_LONG
OidcIDTokenGetIssueTime(
    PCOIDC_ID_TOKEN p);

SSO_LONG
OidcIDTokenGetExpirationTime(
    PCOIDC_ID_TOKEN p);

PCSTRING
OidcIDTokenGetHolderOfKeyPEM(
    PCOIDC_ID_TOKEN p);

void
OidcIDTokenGetGroups(
    PCOIDC_ID_TOKEN p,
    const PSTRING** pppszGroups,
    size_t* pGroupsSize);

PCSTRING
OidcIDTokenGetTenant(
    PCOIDC_ID_TOKEN p);

SSOERROR
OidcIDTokenGetStringClaim(
    PCOIDC_ID_TOKEN p,
    PCSTRING pszKey,
    PSTRING* ppszValue);

// OIDC_ACCESS_TOKEN

SSOERROR
OidcAccessTokenBuild(
    POIDC_ACCESS_TOKEN* pp,
    PCSTRING psz,
    PCSTRING pszSigningCertificatePEM,
    PCSTRING pszIssuer, // not used for now
    PCSTRING pszResourceServerName,
    SSO_LONG clockToleranceInSeconds);

void
OidcAccessTokenDelete(
    POIDC_ACCESS_TOKEN p);

OIDC_TOKEN_TYPE
OidcAccessTokenGetTokenType(
    PCOIDC_ACCESS_TOKEN p);

PCSTRING
OidcAccessTokenGetIssuer(
    PCOIDC_ACCESS_TOKEN p);

PCSTRING
OidcAccessTokenGetSubject(
    PCOIDC_ACCESS_TOKEN p);

void
OidcAccessTokenGetAudience(
    PCOIDC_ACCESS_TOKEN p,
    const PSTRING** pppzAudience,
    size_t* pAudienceSize);

SSO_LONG
OidcAccessTokenGetIssueTime(
    PCOIDC_ACCESS_TOKEN p);

SSO_LONG
OidcAccessTokenGetExpirationTime(
    PCOIDC_ACCESS_TOKEN p);

PCSTRING
OidcAccessTokenGetHolderOfKeyPEM(
    PCOIDC_ACCESS_TOKEN p);

void
OidcAccessTokenGetGroups(
    PCOIDC_ACCESS_TOKEN p,
    const PSTRING** pppszGroups,
    size_t* pGroupsSize);

PCSTRING
OidcAccessTokenGetTenant(
    PCOIDC_ACCESS_TOKEN p);

SSOERROR
OidcAccessTokenGetStringClaim(
    PCOIDC_ACCESS_TOKEN p,
    PCSTRING pszKey,
    PSTRING* ppszValue);

// OIDC_TOKEN_SUCCESS_RESPONSE

void
OidcTokenSuccessResponseDelete(
    POIDC_TOKEN_SUCCESS_RESPONSE p);

PCOIDC_ID_TOKEN
OidcTokenSuccessResponseGetIDToken(
    PCOIDC_TOKEN_SUCCESS_RESPONSE p);

PCSTRING
OidcTokenSuccessResponseGetAccessToken(
    PCOIDC_TOKEN_SUCCESS_RESPONSE p);

PCSTRING
OidcTokenSuccessResponseGetRefreshToken(
    PCOIDC_TOKEN_SUCCESS_RESPONSE p);

// OIDC_ERROR_RESPONSE

void
OidcErrorResponseDelete(
    POIDC_ERROR_RESPONSE p);

PCSTRING
OidcErrorResponseGetErrorDescription(
    PCOIDC_ERROR_RESPONSE p);

#endif
