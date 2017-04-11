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

static const REST_SERVER_ERROR_MAP_ENTRY errorStringToCode[] =
{
    { "bad_request",            SSOERROR_REST_SERVER_BAD_REQUEST },
    { "forbidden",              SSOERROR_REST_SERVER_FORBIDDEN },
    { "unauthorized",           SSOERROR_REST_SERVER_UNAUTHORIZED },
    { "not_found",              SSOERROR_REST_SERVER_NOT_FOUND },
    { "internal_server_error",  SSOERROR_REST_SERVER_INTERNAL_SERVER_ERROR },
    { "not_implemented",        SSOERROR_REST_SERVER_NOT_IMPLEMENTED },
    { "insufficient_scope",     SSOERROR_REST_SERVER_INSUFFICIENT_SCOPE },
    { "invalid_request",        SSOERROR_REST_SERVER_INVALID_REQUEST },
    { "invalid_token",          SSOERROR_REST_SERVER_INVALID_TOKEN }
};

void
RestServerErrorDelete(
    REST_SERVER_ERROR* pError)
{
    if (pError != NULL)
    {
        RestStringDataDelete(pError->error);
        RestStringDataDelete(pError->details);
        RestStringDataDelete(pError->cause);
        SSOMemoryFree(pError, sizeof(REST_SERVER_ERROR));
    }
}

SSOERROR
RestJsonToServerError(
    PCSSO_JSON pJson,
    REST_SERVER_ERROR** ppError)
{
    SSOERROR e = SSOERROR_NONE;
    REST_SERVER_ERROR* pError = NULL;

    if (pJson == NULL || ppError == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(REST_SERVER_ERROR), (void**) &pError);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "error", (void**) &(pError->error));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "details", (void**) &(pError->details));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(pJson, REST_JSON_OBJECT_TYPE_STRING, NULL, "cause", (void**) &(pError->cause));
    BAIL_ON_ERROR(e);

    *ppError = pError;

    error:

    if (e != SSOERROR_NONE)
    {
        RestServerErrorDelete(pError);
    }

    return e;
}

SSOERROR
RestServerErrorGetSSOErrorCode(
    PCSTRING error)
{
    SSOERROR e = SSOERROR_NONE;
    size_t i = 0;

    if (error == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    // default error code
    e = SSOERROR_REST_SERVER;

    for (i = 0; i < sizeof(errorStringToCode) / sizeof(errorStringToCode[0]); i++)
    {
        if (SSOStringEqual(error, errorStringToCode[i].error))
        {
            e = errorStringToCode[i].code;
            break;
        }
    }

    error:

    return e;
}
