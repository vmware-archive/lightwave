/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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

/*
 * Map curl error to vmdir
 */

DWORD
VmDirCurlToDirError(
    DWORD dwCurlError
    )
{
    DWORD dwError = 0;
    switch (dwCurlError)
    {
        case CURLE_UNSUPPORTED_PROTOCOL:
            dwError = VMDIR_ERROR_CURL_UNSUPPORTED_PROTOCOL;
            break;

        case CURLE_FAILED_INIT:
            dwError = VMDIR_ERROR_CURL_FAILED_INIT;
            break;

        case CURLE_URL_MALFORMAT:
            dwError = VMDIR_ERROR_CURL_URLMALFORMAT;
            break;

        case CURLE_NOT_BUILT_IN:
            dwError = VMDIR_ERROR_CURL_NOT_BUILT_IN;
            break;

        case CURLE_COULDNT_RESOLVE_PROXY:
            dwError = VMDIR_ERROR_CURL_COULDNT_RESOLVE_PROXY;
            break;

        case CURLE_COULDNT_RESOLVE_HOST:
            dwError = VMDIR_ERROR_CURL_COULDNT_RESOLVE_HOST;
            break;

        case CURLE_COULDNT_CONNECT:
            dwError = VMDIR_ERROR_CURL_COULDNT_CONNECT;
            break;

        case CURLE_HTTP2://16
            dwError = VMDIR_ERROR_CURL_HTTP2;
            break;

        case CURLE_HTTP_RETURNED_ERROR://22
            dwError = VMDIR_ERROR_CURL_HTTP_RETURNED_ERROR;
            break;

        case CURLE_WRITE_ERROR://23
            dwError = VMDIR_ERROR_CURL_WRITE_ERROR;
            break;

        case CURLE_OUT_OF_MEMORY://27
            dwError = VMDIR_ERROR_CURL_OUT_OF_MEMORY;
            break;

        case CURLE_OPERATION_TIMEDOUT:
            dwError = VMDIR_ERROR_CURL_OPERATION_TIMEDOUT;
            break;

        case CURLE_HTTP_POST_ERROR:
            dwError = VMDIR_ERROR_CURL_HTTP_POST_ERROR;
            break;

        case CURLE_BAD_FUNCTION_ARGUMENT:
            dwError = VMDIR_ERROR_CURL_BAD_FUNCTION_ARGUMENT;
            break;

        case CURLE_INTERFACE_FAILED:
            dwError = VMDIR_ERROR_CURL_INTERFACE_FAILED;
            break;

        case CURLE_SEND_ERROR:
            dwError = VMDIR_ERROR_CURL_SEND_ERROR;
            break;

        case CURLE_RECV_ERROR:
            dwError = VMDIR_ERROR_CURL_RECV_ERROR;
        break;

        case CURLE_NO_CONNECTION_AVAILABLE:
            dwError = VMDIR_ERROR_CURL_NO_CONN_AVAILABLE;
        break;

        default:
            dwError = VMDIR_ERROR_CURL_GENERIC_ERROR;
    }
    return dwError;
}
