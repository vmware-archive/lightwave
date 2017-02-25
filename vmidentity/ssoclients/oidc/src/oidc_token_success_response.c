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
OidcTokenSuccessResponseParse(
    POIDC_TOKEN_SUCCESS_RESPONSE* pp,
    PCSTRING pszJsonResponse,
    PCSTRING pszSigningCertificatePEM,
    SSO_LONG clockToleranceInSeconds)
{
    SSOERROR e = SSOERROR_NONE;
    PSSO_JSON pJson = NULL;
    PSSO_JSON pJsonValue = NULL;
    PSTRING pszJsonString = NULL;
    bool isNullRefreshToken = false;
    POIDC_TOKEN_SUCCESS_RESPONSE p = NULL;

    ASSERT_NOT_NULL(pp);
    ASSERT_NOT_NULL(pszJsonResponse);
    ASSERT_NOT_NULL(pszSigningCertificatePEM);

    e = SSOMemoryAllocate(sizeof(OIDC_TOKEN_SUCCESS_RESPONSE), (void**) &p);
    BAIL_ON_ERROR(e);

    e = SSOJsonParse(&pJson, pszJsonResponse);
    BAIL_ON_ERROR(e);

    e = SSOJsonObjectGet(pJson, "id_token", &pJsonValue);
    BAIL_ON_ERROR(e);
    e = SSOJsonStringValue(pJsonValue, &pszJsonString);
    BAIL_ON_ERROR(e);
    e = OidcIDTokenBuild(&p->pIDToken, pszJsonString, pszSigningCertificatePEM, NULL /* pszIssuer */, clockToleranceInSeconds);
    BAIL_ON_ERROR(e);
    SSOJsonDelete(pJsonValue);
    pJsonValue = NULL;

    e = SSOJsonObjectGet(pJson, "access_token", &pJsonValue);
    BAIL_ON_ERROR(e);
    e = SSOJsonStringValue(pJsonValue, &p->pszAccessToken);
    BAIL_ON_ERROR(e);
    SSOJsonDelete(pJsonValue);
    pJsonValue = NULL;

    e = SSOJsonObjectGet(pJson, "refresh_token", &pJsonValue);
    BAIL_ON_ERROR(e);
    e = SSOJsonIsNull(pJsonValue, &isNullRefreshToken);
    BAIL_ON_ERROR(e);
    if (!isNullRefreshToken)
    {
        e = SSOJsonStringValue(pJsonValue, &p->pszRefreshToken);
        BAIL_ON_ERROR(e);
    }
    SSOJsonDelete(pJsonValue);
    pJsonValue = NULL;

    *pp = p;

error:
    SSOJsonDelete(pJson);
    SSOJsonDelete(pJsonValue);
    SSOStringFree(pszJsonString);

    if (e != SSOERROR_NONE)
    {
        OidcTokenSuccessResponseDelete(p);
    }
    return e;
}

void
OidcTokenSuccessResponseDelete(
    POIDC_TOKEN_SUCCESS_RESPONSE p)
{
    if (p != NULL)
    {
        OidcIDTokenDelete(p->pIDToken);
        SSOStringFree(p->pszAccessToken);
        SSOStringFree(p->pszRefreshToken);
        SSOMemoryFree(p, sizeof(OIDC_TOKEN_SUCCESS_RESPONSE));
    }
}

PCOIDC_ID_TOKEN
OidcTokenSuccessResponseGetIDToken(
    PCOIDC_TOKEN_SUCCESS_RESPONSE p)
{
    ASSERT_NOT_NULL(p);
    return p->pIDToken;
}

PCSTRING
OidcTokenSuccessResponseGetAccessToken(
    PCOIDC_TOKEN_SUCCESS_RESPONSE p)
{
    ASSERT_NOT_NULL(p);
    return p->pszAccessToken;
}

PCSTRING
OidcTokenSuccessResponseGetRefreshToken(
    PCOIDC_TOKEN_SUCCESS_RESPONSE p)
{
    ASSERT_NOT_NULL(p);
    return p->pszRefreshToken;
}
