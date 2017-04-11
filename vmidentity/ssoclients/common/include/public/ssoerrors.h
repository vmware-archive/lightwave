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

#ifndef _SSOERRORS_H_
#define _SSOERRORS_H_

// client error codes
#define SSOERROR_NONE                   0
#define SSOERROR_INVALID_ARGUMENT       1
#define SSOERROR_OUT_OF_MEMORY          2
#define SSOERROR_CURL_FAILURE           3
#define SSOERROR_CURL_INIT_FAILURE      4
#define SSOERROR_HTTP_SEND_FAILURE      5
#define SSOERROR_JSON_FAILURE           6
#define SSOERROR_JSON_PARSE_FAILURE     7
#define SSOERROR_OPENSSL_FAILURE        8
#define SSOERROR_VMAFD_LOAD_FAILURE     9
#define SSOERROR_VMAFD_CALL_FAILURE     10
#define SSOERROR_TOKEN_INVALID_SIGNATURE    11
#define SSOERROR_TOKEN_INVALID_AUDIENCE     12
#define SSOERROR_TOKEN_EXPIRED              13

// OIDC server error codes
#define SSOERROR_OIDC_SERVER                            100
#define SSOERROR_OIDC_SERVER_INVALID_REQUEST            101
#define SSOERROR_OIDC_SERVER_INVALID_SCOPE              102
#define SSOERROR_OIDC_SERVER_INVALID_GRANT              103
#define SSOERROR_OIDC_SERVER_INVALID_CLIENT             104
#define SSOERROR_OIDC_SERVER_UNAUTHORIZED_CLIENT        105
#define SSOERROR_OIDC_SERVER_UNSUPPORTED_RESPONSE_TYPE  106
#define SSOERROR_OIDC_SERVER_UNSUPPORTED_GRANT_TYPE     107
#define SSOERROR_OIDC_SERVER_ACCESS_DENIED              108
#define SSOERROR_OIDC_SERVER_SERVER_ERROR               109

// REST server error codes
#define SSOERROR_REST_SERVER                            200
#define SSOERROR_REST_SERVER_BAD_REQUEST                201
#define SSOERROR_REST_SERVER_FORBIDDEN                  202
#define SSOERROR_REST_SERVER_UNAUTHORIZED               203
#define SSOERROR_REST_SERVER_NOT_FOUND                  204
#define SSOERROR_REST_SERVER_INTERNAL_SERVER_ERROR      205
#define SSOERROR_REST_SERVER_NOT_IMPLEMENTED            206
#define SSOERROR_REST_SERVER_INSUFFICIENT_SCOPE         207
#define SSOERROR_REST_SERVER_INVALID_REQUEST            208
#define SSOERROR_REST_SERVER_INVALID_TOKEN              209

PCSTRING
SSOErrorToString(
    SSOERROR code);

#endif
