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

/*
 * IMPORTANT:
 * You do not need to call this function from consuming applications as the GCC constructor attribute will
 * guarantee its execution.  This attribute will also enforce that the function is called when the consuming
 * application is running with only a single thread.  Read further for details regarding why this is necessary.
 * If you must call this function in a consuming application, ensure it is done at process startup while there
 * is only a single thread running.
 * This is a wrapper for curl_global_init, from its documentation:
 * This function is not thread safe.
 * You must not call it when any other thread in the program (i.e. a thread sharing the same memory) is running.
 * This doesn't just mean no other thread that is using libcurl.
 * Because curl_global_init calls functions of other libraries that are similarly thread unsafe,
 * it could conflict with any other thread that uses these other libraries.
 */
SSOERROR
__attribute__((constructor))
OidcClientGlobalInit();

/*
 * IMPORTANT:
 * You do not need to call this function from consuming applications as the GCC destructor attribute will
 * guarantee its execution.  The attribute will also enfoce that the function is called when the consuming
 * application is running with only a single thread.
 * If you must call this function in a consuming application, ensure it is done at process exit when there
 * is only a single thread running.
 * This function is not thread safe. Call it right before process exit as the curl_global_cleanup
 * documentation states.
 */
void
__attribute__((destructor))
OidcClientGlobalCleanup();

// make sure you call OidcClientGlobalInit once per process before calling this
// on success, pp will be non-null, when done, OidcClientDelete it
// psztlsCAPath: NULL means skip tls validation, otherwise LIGHTWAVE_TLS_CA_PATH will work on lightwave client and server
SSOERROR
OidcClientBuild(
    POIDC_CLIENT* pp,
    PCSTRING pszServer, // OPT: null means use HA to get affinitized host
    int portNumber,
    PCSTRING pszTenant,
    PCSTRING pszClientID /* OPT */,
    PCSTRING pszTlsCAPath /* OPT, see comment above */);

void
OidcClientDelete(
    POIDC_CLIENT p);

// on success, ppOutTokenSuccessResponse will be non-null
// on error, ppOutTokenErrorResponse might be non-null (it will carry error info returned by the server if any)
// delete both when done, whether invocation is successful or not, using OidcTokenSuccessResponseDelete and OidcErrorResponseDelete
SSOERROR
OidcClientAcquireTokensByPassword(
    PCOIDC_CLIENT p,
    PCSTRING pszUsername,
    PCSTRING pszPassword,
    PCSTRING pszScope,
    POIDC_TOKEN_SUCCESS_RESPONSE* ppOutTokenSuccessResponse, /* OUT */
    POIDC_ERROR_RESPONSE* ppOutTokenErrorResponse /* OUT */);

// on success, ppOutTokenSuccessResponse will be non-null
// on error, ppOutTokenErrorResponse might be non-null (it will carry error info returned by the server if any)
// delete both when done, whether invocation is successful or not, using OidcTokenSuccessResponseDelete and OidcErrorResponseDelete
SSOERROR
OidcClientAcquireTokensByRefreshToken(
    PCOIDC_CLIENT p,
    PCSTRING pszRefreshToken,
    POIDC_TOKEN_SUCCESS_RESPONSE* ppOutTokenSuccessResponse, /* OUT */
    POIDC_ERROR_RESPONSE* ppOutTokenErrorResponse /* OUT */);

// on success, ppOutTokenSuccessResponse will be non-null
// on error, ppOutTokenErrorResponse might be non-null (it will carry error info returned by the server if any)
// delete both when done, whether invocation is successful or not, using OidcTokenSuccessResponseDelete and OidcErrorResponseDelete
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

// make sure you call OidcClientGlobalInit once per process before calling this
// on success, pp will be non-null, when done, OidcServerMetadataDelete it
// psztlsCAPath: NULL means skip tls validation, otherwise LIGHTWAVE_TLS_CA_PATH will work on lightwave client and server
SSOERROR
OidcServerMetadataAcquire(
    POIDC_SERVER_METADATA* pp,
    PCSTRING pszServer,
    int portNumber,
    PCSTRING pszTenant,
    PCSTRING pszTlsCAPath /* OPT, see comment above */);

// make sure you call OidcClientGlobalInit once per process before calling this
// on success, pp will be non-null, when done, OidcServerMetadataDelete it
// psztlsCAPath: NULL means skip tls validation, otherwise LIGHTWAVE_TLS_CA_PATH will work on lightwave client and server
SSOERROR
OidcServerMetadataAcquireFromIssuer(
    POIDC_SERVER_METADATA* pp,
    PCSTRING pszIssuer,
    PCSTRING pszTlsCAPath /* OPT, see comment above */);

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

// (TODO) Deprecated
SSOERROR
OidcIDTokenBuild(
    POIDC_ID_TOKEN* pp,
    PCSTRING psz,
    PCSTRING pszSigningCertificatePEM,
    PCSTRING pszIssuer, // not used for now
    SSO_LONG clockToleranceInSeconds);

// on success, pp will be non-null, when done, OidcIDTokenDelete it
SSOERROR
OidcIDTokenParse(
    POIDC_ID_TOKEN* pp,
    PCSTRING psz);

SSOERROR
OidcIDTokenValidate(
    POIDC_ID_TOKEN p,
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

size_t
OidcIDTokenGetAudienceSize(
    PCOIDC_ID_TOKEN p);

PCSTRING
OidcIDTokenGetAudienceEntry(
    PCOIDC_ID_TOKEN p,
    int index);

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

size_t
OidcIDTokenGetGroupsSize(
    PCOIDC_ID_TOKEN p);

PCSTRING
OidcIDTokenGetGroupsEntry(
    PCOIDC_ID_TOKEN p,
    int index);

PCSTRING
OidcIDTokenGetTenant(
    PCOIDC_ID_TOKEN p);

SSOERROR
OidcIDTokenGetStringClaim(
    PCOIDC_ID_TOKEN p,
    PCSTRING pszKey,
    PSTRING* ppszValue);

// OIDC_ACCESS_TOKEN

// (TODO) Deprecated
SSOERROR
OidcAccessTokenBuild(
    POIDC_ACCESS_TOKEN* pp,
    PCSTRING psz,
    PCSTRING pszSigningCertificatePEM,
    PCSTRING pszIssuer, // not used for now
    PCSTRING pszResourceServerName,
    SSO_LONG clockToleranceInSeconds);

// on success, pp will be non-null, when done, OidcAccessTokenDelete it
SSOERROR
OidcAccessTokenParse(
    POIDC_ACCESS_TOKEN* pp,
    PCSTRING psz);

SSOERROR
OidcAccessTokenValidate(
    POIDC_ACCESS_TOKEN p,
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

size_t
OidcAccessTokenGetAudienceSize(
    PCOIDC_ACCESS_TOKEN p);

PCSTRING
OidcAccessTokenGetAudienceEntry(
    PCOIDC_ACCESS_TOKEN p,
    int index);

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

size_t
OidcAccessTokenGetGroupsSize(
    PCOIDC_ACCESS_TOKEN p);

PCSTRING
OidcAccessTokenGetGroupsEntry(
    PCOIDC_ACCESS_TOKEN p,
    int index);

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

PCSTRING
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
OidcErrorResponseGetError(
    PCOIDC_ERROR_RESPONSE p);

PCSTRING
OidcErrorResponseGetErrorDescription(
    PCOIDC_ERROR_RESPONSE p);

#endif
