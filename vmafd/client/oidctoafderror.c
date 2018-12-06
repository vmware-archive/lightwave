/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the �~@~\License�~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an �~@~\AS IS�~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "includes.h"

DWORD
VmAfdOidcToVmafdError(
    DWORD   dwOIDCError
    )
{
    DWORD   dwError = dwOIDCError;

    switch (dwOIDCError)
    {
        case SSOERROR_NONE:
            dwError = VMAFD_SSOERROR_NONE;
            break;

        case SSOERROR_INVALID_ARGUMENT:
            dwError = VMAFD_SSOERROR_INVALID_ARGUMENT;
            break;

        case SSOERROR_OUT_OF_MEMORY:
            dwError = VMAFD_SSOERROR_OUT_OF_MEMORY;
            break;

        case SSOERROR_CURL_FAILURE:
            dwError = VMAFD_SSOERROR_CURL_FAILURE;
            break;

        case SSOERROR_JSON_FAILURE:
            dwError = VMAFD_SSOERROR_JSON_FAILURE;
            break;

        case SSOERROR_JSON_PARSE_FAILURE:
            dwError = VMAFD_SSOERROR_JSON_PARSE_FAILURE;
            break;

        case SSOERROR_OPENSSL_FAILURE:
            dwError = VMAFD_SSOERROR_OPENSSL_FAILURE;
            break;

        case SSOERROR_VMAFD_LOAD_FAILURE:
            dwError = VMAFD_SSOERROR_VMAFD_LOAD_FAILURE;
            break;

        case SSOERROR_VMAFD_CALL_FAILURE:
            dwError = VMAFD_SSOERROR_VMAFD_CALL_FAILURE;
            break;

        case SSOERROR_TOKEN_INVALID_SIGNATURE:
            dwError = VMAFD_SSOERROR_TOKEN_INVALID_SIGNATURE;
            break;

        case SSOERROR_TOKEN_INVALID_AUDIENCE:
            dwError = VMAFD_SSOERROR_TOKEN_INVALID_AUDIENCE;
            break;

        case SSOERROR_TOKEN_EXPIRED:
            dwError = VMAFD_SSOERROR_TOKEN_EXPIRED;
            break;

        case SSOERROR_OIDC_SERVER:
            dwError = VMAFD_SSOERROR_OIDC_SERVER;
            break;

        case SSOERROR_OIDC_SERVER_INVALID_REQUEST:
            dwError = VMAFD_SSOERROR_OIDC_SERVER_INVALID_REQUEST;
            break;

        case SSOERROR_OIDC_SERVER_INVALID_SCOPE:
            dwError = VMAFD_SSOERROR_OIDC_SERVER_INVALID_SCOPE;
            break;

        case SSOERROR_OIDC_SERVER_INVALID_GRANT:
            dwError = VMAFD_SSOERROR_OIDC_SERVER_INVALID_GRANT;
            break;

        case SSOERROR_OIDC_SERVER_INVALID_CLIENT:
            dwError = VMAFD_SSOERROR_OIDC_SERVER_INVALID_CLIENT;
            break;

        case SSOERROR_OIDC_SERVER_UNSUPPORTED_RESPONSE_TYPE:
            dwError = VMAFD_SSOERROR_OIDC_SERVER_UNSUPPORTED_RESPONSE_TYPE;
            break;

        case SSOERROR_OIDC_SERVER_UNSUPPORTED_GRANT_TYPE:
            dwError = VMAFD_SSOERROR_OIDC_SERVER_UNSUPPORTED_GRANT_TYPE;
            break;

        case SSOERROR_OIDC_SERVER_ACCESS_DENIED:
            dwError = VMAFD_SSOERROR_OIDC_SERVER_ACCESS_DENIED;
            break;

        case SSOERROR_OIDC_SERVER_SERVER_ERROR:
            dwError = VMAFD_SSOERROR_OIDC_SERVER_SERVER_ERROR;
            break;

        default:
            dwError = VMAFD_SSOERROR_UNKNOWN;
            if (SSOErrorHasCurlError(dwOIDCError))
            {
                dwError = VMAFD_SSOERROR_CURL_START +
                          SSOErrorGetCurlCode(dwOIDCError);
            }
    }

    return dwError;
}
