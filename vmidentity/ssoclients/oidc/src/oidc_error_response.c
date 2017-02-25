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

typedef struct
{
    const PCSTRING pszString;
    const SSOERROR code;
} OIDC_SERVER_ERROR_MAP_ENTRY;

static const OIDC_SERVER_ERROR_MAP_ENTRY errorStringToCode[] =
{
    { "invalid_request",            SSOERROR_OIDC_SERVER_INVALID_REQUEST },
    { "invalid_scope",              SSOERROR_OIDC_SERVER_INVALID_SCOPE },
    { "invalid_grant",              SSOERROR_OIDC_SERVER_INVALID_GRANT },
    { "invalid_client",             SSOERROR_OIDC_SERVER_INVALID_CLIENT },
    { "unauthorized_client",        SSOERROR_OIDC_SERVER_UNAUTHORIZED_CLIENT },
    { "unsupported_response_type",  SSOERROR_OIDC_SERVER_UNSUPPORTED_RESPONSE_TYPE },
    { "unsupported_grant_type",     SSOERROR_OIDC_SERVER_UNSUPPORTED_GRANT_TYPE },
    { "access_denied",              SSOERROR_OIDC_SERVER_ACCESS_DENIED },
    { "server_error",               SSOERROR_OIDC_SERVER_SERVER_ERROR },
};

SSOERROR
OidcErrorResponseParse(
    POIDC_ERROR_RESPONSE* pp,
    PCSTRING pszJsonResponse)
{
    SSOERROR e = SSOERROR_NONE;
    PSSO_JSON pJson = NULL;
    PSSO_JSON pJsonValue = NULL;
    POIDC_ERROR_RESPONSE p = NULL;

    ASSERT_NOT_NULL(pp);
    ASSERT_NOT_NULL(pszJsonResponse);

    e = SSOMemoryAllocate(sizeof(OIDC_ERROR_RESPONSE), (void**) &p);
    BAIL_ON_ERROR(e);

    e = SSOJsonParse(&pJson, pszJsonResponse);
    BAIL_ON_ERROR(e);

    e = SSOJsonObjectGet(pJson, "error", &pJsonValue);
    BAIL_ON_ERROR(e);
    e = SSOJsonStringValue(pJsonValue, &p->pszError);
    BAIL_ON_ERROR(e);
    SSOJsonDelete(pJsonValue);
    pJsonValue = NULL;

    e = SSOJsonObjectGet(pJson, "error_description", &pJsonValue);
    BAIL_ON_ERROR(e);
    e = SSOJsonStringValue(pJsonValue, &p->pszErrorDescription);
    BAIL_ON_ERROR(e);
    SSOJsonDelete(pJsonValue);
    pJsonValue = NULL;

    *pp = p;

error:
    SSOJsonDelete(pJson);
    SSOJsonDelete(pJsonValue);
    if (e != SSOERROR_NONE)
    {
        OidcErrorResponseDelete(p);
    }
    return e;
}

void
OidcErrorResponseDelete(
    POIDC_ERROR_RESPONSE p)
{
    if (p != NULL)
    {
        SSOStringFree(p->pszError);
        SSOStringFree(p->pszErrorDescription);
        SSOMemoryFree(p, sizeof(OIDC_ERROR_RESPONSE));
    }
}

PCSTRING
OidcErrorResponseGetErrorDescription(
    PCOIDC_ERROR_RESPONSE p)
{
    ASSERT_NOT_NULL(p);
    return p->pszErrorDescription;
}

SSOERROR
OidcErrorResponseGetSSOErrorCode(
    PCOIDC_ERROR_RESPONSE p)
{
    SSOERROR result = SSOERROR_OIDC_SERVER;
    size_t i = 0;
    size_t entryCount = sizeof(errorStringToCode) / sizeof(errorStringToCode[0]);

    ASSERT_NOT_NULL(p);

    for (i = 0; i < entryCount; i++)
    {
        if (SSOStringEqual(p->pszError, errorStringToCode[i].pszString))
        {
            result = errorStringToCode[i].code;
            break;
        }
    }

    return result;
}
