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

#include "includes.h"

static VDIR_HTTP_ERROR httpErrors[] =
{
    {HTTP_CONTINUE,                             "100",  "Continue"},
    {HTTP_SWITCHING_PROTOCOLS,                  "101",  "Switching Protocols"},
    {HTTP_PROCESSING,                           "102",  "Processing"},
    {HTTP_OK,                                   "200",  "OK"},
    {HTTP_CREATED,                              "201",  "Created"},
    {HTTP_ACCEPTED,                             "202",  "Accepted"},
    {HTTP_NON_AUTHORITATIVE_INFORMATION,        "203",  "Non-authoritative Information"},
    {HTTP_NO_CONTENT,                           "204",  "No Content"},
    {HTTP_RESET_CONTENT,                        "205",  "Reset Content"},
    {HTTP_PARTIAL_CONTENT,                      "206",  "Partial Content"},
    {HTTP_MULTI_STATUS,                         "207",  "Multi-Status"},
    {HTTP_ALREADY_REPORTED,                     "208",  "Already Reported"},
    {HTTP_IM_USED,                              "226",  "IM Used"},
    {HTTP_MULTIPLE_CHOICES,                     "300",  "Multiple Choices"},
    {HTTP_MOVED_PERMANENTLY,                    "301",  "Moved Permanently"},
    {HTTP_FOUND,                                "302",  "Found"},
    {HTTP_SEE_OTHER,                            "303",  "See Other"},
    {HTTP_NOT_MODIFIED,                         "304",  "Not Modified"},
    {HTTP_USE_PROXY,                            "305",  "Use Proxy"},
    {HTTP_TEMPORARY_REDIRECT,                   "307",  "Temporary Redirect"},
    {HTTP_PERMANENT_REDIRECT,                   "308",  "Permanent Redirect"},
    {HTTP_BAD_REQUEST,                          "400",  "Bad Request"},
    {HTTP_UNAUTHORIZED,                         "401",  "Unauthorized"},
    {HTTP_PAYMENT_REQUIRED,                     "402",  "Payment Required"},
    {HTTP_FORBIDDEN,                            "403",  "Forbidden"},
    {HTTP_NOT_FOUND,                            "404",  "Not Found"},
    {HTTP_METHOD_NOT_ALLOWED,                   "405",  "Method Not Allowed"},
    {HTTP_NOT_ACCEPTABLE,                       "406",  "Not Acceptable"},
    {HTTP_PROXY_AUTHENTICATION_REQUIRED,        "407",  "Proxy Authentication Required"},
    {HTTP_REQUEST_TIMEOUT,                      "408",  "Request Timeout"},
    {HTTP_CONFLICT,                             "409",  "Conflict"},
    {HTTP_GONE,                                 "410",  "Gone"},
    {HTTP_LENGTH_REQUIRED,                      "411",  "Length Required"},
    {HTTP_PRECONDITION_FAILED,                  "412",  "Precondition Failed"},
    {HTTP_PAYLOAD_TOO_LARGE,                    "413",  "Payload Too Large"},
    {HTTP_REQUEST_URI_TOO_LONG,                 "414",  "Request-URI Too Long"},
    {HTTP_UNSUPPORTED_MEDIA_TYPE,               "415",  "Unsupported Media Type"},
    {HTTP_REQUESTED_RANGE_NOT_SATISFIABLE,      "416",  "Requested Range Not Satisfiable"},
    {HTTP_EXPECTATION_FAILED,                   "417",  "Expectation Failed"},
    {HTTP_I_M_A_TEAPOT,                         "418",  "I'm a teapot"},
    {HTTP_MISDIRECTED_REQUEST,                  "421",  "Misdirected Request"},
    {HTTP_UNPROCESSABLE_ENTITY,                 "422",  "Unprocessable Entity"},
    {HTTP_LOCKED,                               "423",  "Locked"},
    {HTTP_FAILED_DEPENDENCY,                    "424",  "Failed Dependency"},
    {HTTP_UPGRADE_REQUIRED,                     "426",  "Upgrade Required"},
    {HTTP_PRECONDITION_REQUIRED,                "428",  "Precondition Required"},
    {HTTP_TOO_MANY_REQUESTS,                    "429",  "Too Many Requests"},
    {HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE,      "431",  "Request Header Fields Too Large"},
    {HTTP_CONNECTION_CLOSED_WITHOUT_RESPONSE,   "444",  "Connection Closed Without Response"},
    {HTTP_UNAVAILABLE_FOR_LEGAL_REASONS,        "451",  "Unavailable For Legal Reasons"},
    {HTTP_CLIENT_CLOSED_REQUEST,                "499",  "Client Closed Request"},
    {HTTP_INTERNAL_SERVER_ERROR,                "500",  "Internal Server Error"},
    {HTTP_NOT_IMPLEMENTED,                      "501",  "Not Implemented"},
    {HTTP_BAD_GATEWAY,                          "502",  "Bad Gateway"},
    {HTTP_SERVICE_UNAVAILABLE,                  "503",  "Service Unavailable"},
    {HTTP_GATEWAY_TIMEOUT,                      "504",  "Gateway Timeout"},
    {HTTP_HTTP_VERSION_NOT_SUPPORTED,           "505",  "HTTP Version Not Supported"},
    {HTTP_VARIANT_ALSO_NEGOTIATES,              "506",  "Variant Also Negotiates"},
    {HTTP_INSUFFICIENT_STORAGE,                 "507",  "Insufficient Storage"},
    {HTTP_LOOP_DETECTED,                        "508",  "Loop Detected"},
    {HTTP_NOT_EXTENDED,                         "510",  "Not Extended"},
    {HTTP_NETWORK_AUTHENTICATION_REQUIRED,      "511",  "Network Authentication Required"},
    {HTTP_NETWORK_CONNECT_TIMEOUT_ERROR,        "599",  "Network Connect Timeout Error"},
    {0,                                         "0",    "Unknown Error"}
};

PVDIR_HTTP_ERROR
VmDirRESTGetHttpError(
    int httpStatus
    )
{
    DWORD i = 0;

    for (i = 0; httpErrors[i].httpStatus; i++)
    {
        if (httpErrors[i].httpStatus == httpStatus)
        {
            break;
        }
    }

    return &httpErrors[i];
}
