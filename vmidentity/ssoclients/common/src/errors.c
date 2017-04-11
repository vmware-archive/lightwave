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
    const SSOERROR code;
    const PCSTRING pszString;
} SSOERROR_MAP_ENTRY;

static const SSOERROR_MAP_ENTRY errorCodeToString[] =
{
    // client error codes
    { SSOERROR_NONE,                                    "SSOERROR_NONE" },
    { SSOERROR_INVALID_ARGUMENT,                        "SSOERROR_INVALID_ARGUMENT" },
    { SSOERROR_OUT_OF_MEMORY,                           "SSOERROR_OUT_OF_MEMORY" },
    { SSOERROR_CURL_FAILURE,                            "SSOERROR_CURL_FAILURE" },
    { SSOERROR_CURL_INIT_FAILURE,                       "SSOERROR_CURL_INIT_FAILURE" },
    { SSOERROR_HTTP_SEND_FAILURE,                       "SSOERROR_HTTP_SEND_FAILURE" },
    { SSOERROR_JSON_FAILURE,                            "SSOERROR_JSON_FAILURE" },
    { SSOERROR_JSON_PARSE_FAILURE,                      "SSOERROR_JSON_PARSE_FAILURE" },
    { SSOERROR_OPENSSL_FAILURE,                         "SSOERROR_OPENSSL_FAILURE" },
    { SSOERROR_VMAFD_LOAD_FAILURE,                      "SSOERROR_VMAFD_LOAD_FAILURE" },
    { SSOERROR_VMAFD_CALL_FAILURE,                      "SSOERROR_VMAFD_CALL_FAILURE" },
    { SSOERROR_TOKEN_INVALID_SIGNATURE,                 "SSOERROR_TOKEN_INVALID_SIGNATURE" },
    { SSOERROR_TOKEN_INVALID_AUDIENCE,                  "SSOERROR_TOKEN_INVALID_AUDIENCE" },
    { SSOERROR_TOKEN_EXPIRED,                           "SSOERROR_TOKEN_EXPIRED" },

    // oidc server error codes
    { SSOERROR_OIDC_SERVER,                             "SSOERROR_OIDC_SERVER" },
    { SSOERROR_OIDC_SERVER_INVALID_REQUEST,             "SSOERROR_OIDC_SERVER_INVALID_REQUEST" },
    { SSOERROR_OIDC_SERVER_INVALID_SCOPE,               "SSOERROR_OIDC_SERVER_INVALID_SCOPE" },
    { SSOERROR_OIDC_SERVER_INVALID_GRANT,               "SSOERROR_OIDC_SERVER_INVALID_GRANT" },
    { SSOERROR_OIDC_SERVER_INVALID_CLIENT,              "SSOERROR_OIDC_SERVER_INVALID_CLIENT" },
    { SSOERROR_OIDC_SERVER_UNAUTHORIZED_CLIENT,         "SSOERROR_OIDC_SERVER_UNAUTHORIZED_CLIENT" },
    { SSOERROR_OIDC_SERVER_UNSUPPORTED_RESPONSE_TYPE,   "SSOERROR_OIDC_SERVER_UNSUPPORTED_RESPONSE_TYPE" },
    { SSOERROR_OIDC_SERVER_UNSUPPORTED_GRANT_TYPE,      "SSOERROR_OIDC_SERVER_UNSUPPORTED_GRANT_TYPE" },
    { SSOERROR_OIDC_SERVER_ACCESS_DENIED,               "SSOERROR_OIDC_SERVER_ACCESS_DENIED" },
    { SSOERROR_OIDC_SERVER_SERVER_ERROR,                "SSOERROR_OIDC_SERVER_SERVER_ERROR" },

    // REST server error codes
    { SSOERROR_REST_SERVER,                              "SSOERROR_REST_SERVER" },
    { SSOERROR_REST_SERVER_BAD_REQUEST,                  "SSOERROR_REST_SERVER_BAD_REQUEST" },
    { SSOERROR_REST_SERVER_FORBIDDEN,                    "SSOERROR_REST_SERVER_FORBIDDEN" },
    { SSOERROR_REST_SERVER_UNAUTHORIZED,                 "SSOERROR_REST_SERVER_UNAUTHORIZED" },
    { SSOERROR_REST_SERVER_NOT_FOUND,                    "SSOERROR_REST_SERVER_NOT_FOUND" },
    { SSOERROR_REST_SERVER_INTERNAL_SERVER_ERROR,        "SSOERROR_REST_SERVER_INTERNAL_SERVER_ERROR" },
    { SSOERROR_REST_SERVER_NOT_IMPLEMENTED,              "SSOERROR_REST_SERVER_NOT_IMPLEMENTED" },
    { SSOERROR_REST_SERVER_INSUFFICIENT_SCOPE,           "SSOERROR_REST_SERVER_INSUFFICIENT_SCOPE" },
    { SSOERROR_REST_SERVER_INVALID_REQUEST,              "SSOERROR_REST_SERVER_INVALID_REQUEST" },
    { SSOERROR_REST_SERVER_INVALID_TOKEN,                "SSOERROR_REST_SERVER_INVALID_TOKEN" },
};

PCSTRING
SSOErrorToString(
    SSOERROR code)
{
    PCSTRING result = "<invalid SSOERROR code>";

    size_t i = 0;
    size_t entryCount = sizeof(errorCodeToString) / sizeof(errorCodeToString[0]);
    for (i = 0; i < entryCount; i++)
    {
        if (errorCodeToString[i].code == code)
        {
            result = errorCodeToString[i].pszString;
            break;
        }
    }

    return result;
}
