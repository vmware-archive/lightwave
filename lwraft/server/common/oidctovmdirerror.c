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



/*
 * Module Name: Directory common
 *
 * Filename: vmdirtoldaperror.c
 *
 * Abstract: Map OIDC error to VMDIR error
 *
 */

#include "includes.h"

/*
 * Map oidc to vmdir error
 */
DWORD
VmDirOidcToVmdirError(
    DWORD   dwOIDCError
    )
{
    DWORD   dwError = dwOIDCError;

    switch (dwOIDCError)
    {
        case SSOERROR_NONE:
            dwError = VMDIR_SUCCESS;
            break;

        case SSOERROR_INVALID_ARGUMENT:
        case SSOERROR_JSON_FAILURE:
        case SSOERROR_JSON_PARSE_FAILURE:
            dwError = VMDIR_ERROR_INVALID_PARAMETER;
            break;

        case SSOERROR_OUT_OF_MEMORY:
            dwError = VMDIR_ERROR_NO_MEMORY;
            break;

        case SSOERROR_CURL_FAILURE:
            dwError = VMDIR_ERROR_CURL_GENERIC_ERROR;
            break;

        case SSOERROR_CURL_INIT_FAILURE:
            dwError = VMDIR_ERROR_CURL_FAILED_INIT;
            break;

        case SSOERROR_HTTP_SEND_FAILURE:
            dwError = VMDIR_ERROR_CURL_SEND_ERROR;
            break;

        case SSOERROR_TOKEN_INVALID_SIGNATURE:
        case SSOERROR_TOKEN_INVALID_AUDIENCE:
        case SSOERROR_TOKEN_EXPIRED:
            dwError = VMDIR_ERROR_AUTH_BAD_DATA;
            break;

        case SSOERROR_VMAFD_LOAD_FAILURE:
        case SSOERROR_VMAFD_CALL_FAILURE:
            dwError = VMDIR_ERROR_AFD_UNAVAILABLE;
            break;

        default:
            dwError = VMDIR_ERROR_OIDC_UNAVAILABLE;
    }

    return dwError;
}
