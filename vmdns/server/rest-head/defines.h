/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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

// C REST ENGINE CONFIG VALUES
#define REST_API_SPEC               VMDNS_CONFIG_DIR VMDNS_PATH_SEPARATOR_STR "vmdns-rest.json"
#define VMDNS_REST_DEBUGLOGFILE     "/var/log/lightwave/vmdns-rest.log"
//#define VMDNS_REST_DEBUGLOGFILE     VMDNS_LOG_DIR    VMDNS_PATH_SEPARATOR_STR "vmdns-rest.log"    TODO use this when lightwave-first is complete
#define VMDNS_REST_CLIENTCNT        "64"
#define VMDNS_REST_WORKERTHCNT      "64"

//Rest Listen Port Registry Key
#define VMDNS_REG_KEY_REST_LISTEN_PORT        "RestListenPort"
#define VMDNS_REG_CONFIG_KEY_PATH             "Services\\vmdns\\Parameters"
#define VDNS_SAFE_STRING(str) ((str) ? (str) : "")

#define MAX_REST_PAYLOAD_LENGTH     4096

// HTTP headers
#define VMDNS_REST_HEADER_AUTHENTICATION    "Authorization"
#define VMDNS_REST_HEADER_IF_MATCH          "If-Match"

// HTTP STATUS CODES
// 1xx Informational
#define HTTP_CONTINUE                           100
#define HTTP_SWITCHING_PROTOCOLS                101
#define HTTP_PROCESSING                         102
// 2xx Success
#define HTTP_OK                                 200
#define HTTP_CREATED                            201
#define HTTP_ACCEPTED                           202
#define HTTP_NON_AUTHORITATIVE_INFORMATION      203
#define HTTP_NO_CONTENT                         204
#define HTTP_RESET_CONTENT                      205
#define HTTP_PARTIAL_CONTENT                    206
#define HTTP_MULTI_STATUS                       207
#define HTTP_ALREADY_REPORTED                   208
#define HTTP_IM_USED                            226
// 3xx Redirection
#define HTTP_MULTIPLE_CHOICES                   300
#define HTTP_MOVED_PERMANENTLY                  301
#define HTTP_FOUND                              302
#define HTTP_SEE_OTHER                          303
#define HTTP_NOT_MODIFIED                       304
#define HTTP_USE_PROXY                          305
#define HTTP_TEMPORARY_REDIRECT                 307
#define HTTP_PERMANENT_REDIRECT                 308
// 4xx Client Error
#define HTTP_BAD_REQUEST                        400
#define HTTP_UNAUTHORIZED                       401
#define HTTP_PAYMENT_REQUIRED                   402
#define HTTP_FORBIDDEN                          403
#define HTTP_NOT_FOUND                          404
#define HTTP_METHOD_NOT_ALLOWED                 405
#define HTTP_NOT_ACCEPTABLE                     406
#define HTTP_PROXY_AUTHENTICATION_REQUIRED      407
#define HTTP_REQUEST_TIMEOUT                    408
#define HTTP_CONFLICT                           409
#define HTTP_GONE                               410
#define HTTP_LENGTH_REQUIRED                    411
#define HTTP_PRECONDITION_FAILED                412
#define HTTP_PAYLOAD_TOO_LARGE                  413
#define HTTP_REQUEST_URI_TOO_LONG               414
#define HTTP_UNSUPPORTED_MEDIA_TYPE             415
#define HTTP_REQUESTED_RANGE_NOT_SATISFIABLE    416
#define HTTP_EXPECTATION_FAILED                 417
#define HTTP_I_M_A_TEAPOT                       418
#define HTTP_MISDIRECTED_REQUEST                421
#define HTTP_UNPROCESSABLE_ENTITY               422
#define HTTP_LOCKED                             423
#define HTTP_FAILED_DEPENDENCY                  424
#define HTTP_UPGRADE_REQUIRED                   426
#define HTTP_PRECONDITION_REQUIRED              428
#define HTTP_TOO_MANY_REQUESTS                  429
#define HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE    431
#define HTTP_CONNECTION_CLOSED_WITHOUT_RESPONSE 444
#define HTTP_UNAVAILABLE_FOR_LEGAL_REASONS      451
#define HTTP_CLIENT_CLOSED_REQUEST              499
// 5xx Server Error
#define HTTP_INTERNAL_SERVER_ERROR              500
#define HTTP_NOT_IMPLEMENTED                    501
#define HTTP_BAD_GATEWAY                        502
#define HTTP_SERVICE_UNAVAILABLE                503
#define HTTP_GATEWAY_TIMEOUT                    504
#define HTTP_HTTP_VERSION_NOT_SUPPORTED         505
#define HTTP_VARIANT_ALSO_NEGOTIATES            506
#define HTTP_INSUFFICIENT_STORAGE               507
#define HTTP_LOOP_DETECTED                      508
#define HTTP_NOT_EXTENDED                       510
#define HTTP_NETWORK_AUTHENTICATION_REQUIRED    511
#define HTTP_NETWORK_CONNECT_TIMEOUT_ERROR      599


#define VMDNS_SET_REST_RESULT(pRestOp, dwError, pszErrMsg)       \
    do                                                                  \
    {                                                                   \
        PVDNS_REST_RESOURCE pResource = NULL;                           \
        PVDNS_REST_RESULT pRestRslt = NULL;                             \
        if (pRestOp)                                                    \
        {                                                               \
            pResource = ((PVDNS_REST_OPERATION)pRestOp)->pResource;     \
            pRestRslt = ((PVDNS_REST_OPERATION)pRestOp)->pResult;       \
            (pResource)->pfnSetResult(                                   \
                    pRestRslt, dwError, pszErrMsg);          \
        }                                                               \
    } while (0)
