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

#ifndef _STRUCTS_H_
#define _STRUCTS_H_

typedef struct OIDC_CLIENT
{
    PSTRING pszServer;
    PSTRING pszTokenEndpointUrl;
    PSTRING pszSigningCertificatePEM;
    SSO_LONG clockToleranceInSeconds;
    PSSO_CDC pClientDCCache;
} OIDC_CLIENT;

typedef struct OIDC_SERVER_METADATA
{
    PSTRING pszTokenEndpointUrl;
    PSTRING pszSigningCertificatePEM;
} OIDC_SERVER_METADATA;

typedef struct OIDC_TOKEN
{
    PSSO_JWT pJwt;
    OIDC_TOKEN_TYPE tokenType;
    PSTRING pszIssuer;
    PSTRING pszSubject;
    PSTRING* ppszAudience;
    size_t audienceSize;
    SSO_LONG issueTime;
    SSO_LONG expirationTime;
    PSTRING pszHolderOfKeyPEM;
    PSTRING* ppszGroups;
    size_t groupsSize;
    PSTRING pszTenant;
} OIDC_TOKEN, *POIDC_TOKEN;

typedef struct OIDC_ID_TOKEN
{
    POIDC_TOKEN pToken;
} OIDC_ID_TOKEN;

typedef struct OIDC_ACCESS_TOKEN
{
    POIDC_TOKEN pToken;
} OIDC_ACCESS_TOKEN;

typedef struct OIDC_TOKEN_SUCCESS_RESPONSE
{
    OIDC_ID_TOKEN* pIDToken;
    PSTRING pszAccessToken;
    PSTRING pszRefreshToken;
} OIDC_TOKEN_SUCCESS_RESPONSE;

typedef struct OIDC_ERROR_RESPONSE
{
    PSTRING pszError;
    PSTRING pszErrorDescription;
} OIDC_ERROR_RESPONSE;

#endif
