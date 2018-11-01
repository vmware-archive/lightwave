/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

DWORD
LwCAOIDCToLwCAError(
    DWORD       dwOIDCError
    )
{
    DWORD       dwError = 0;

    switch (dwOIDCError)
    {
        case SSOERROR_NONE:
            dwError = LWCA_SUCCESS;
            break;

        case SSOERROR_INVALID_ARGUMENT:
        case SSOERROR_JSON_FAILURE:
        case SSOERROR_JSON_PARSE_FAILURE:
            dwError = LWCA_ERROR_INVALID_PARAMETER;
            break;

        case SSOERROR_OUT_OF_MEMORY:
            dwError = LWCA_OUT_OF_MEMORY_ERROR;
            break;

        case SSOERROR_CURL_FAILURE:
            dwError = LWCA_ERROR_CURL_GENERIC_ERROR;
            break;

        case SSOERROR_CURL_INIT_FAILURE:
            dwError = LWCA_ERROR_CURL_FAILED_INIT;
            break;

        case SSOERROR_HTTP_SEND_FAILURE:
            dwError = LWCA_ERROR_CURL_SEND_ERROR;
            break;

        case SSOERROR_TOKEN_INVALID_SIGNATURE:
        case SSOERROR_TOKEN_INVALID_AUDIENCE:
        case SSOERROR_OIDC_SERVER_INVALID_SCOPE:
        case SSOERROR_OIDC_SERVER_ACCESS_DENIED:
        case SSOERROR_OIDC_SERVER_UNSUPPORTED_GRANT_TYPE:
        case SSOERROR_TOKEN_EXPIRED:
            dwError = LWCA_ERROR_OIDC_BAD_AUTH_DATA;
            break;

        case SSOERROR_VMAFD_LOAD_FAILURE:
        case SSOERROR_VMAFD_CALL_FAILURE:
            dwError = LWCA_ERROR_VMAFD_UNAVAILABLE;
            break;

        default:
            dwError = LWCA_ERROR_OIDC_UNAVAILABLE;
    }

    return dwError;
}
