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

static VDIR_REST_RESOURCE resources[VDIR_REST_RSC_COUNT] =
{
    {
        VDIR_REST_RSC_LDAP,
        VMDIR_V1_LDAP_RESOURCE,
        FALSE,
        VmDirRESTLdapSetResult,
        VmDirRESTLdapGetHttpError,
        "application/json",
        "error_code",
        "error_message"
    },
    {
        VDIR_REST_RSC_OBJECT,
        VMDIR_V1_OBJ_RESOURCE,
        TRUE,
        VmDirRESTLdapSetResult,
        VmDirRESTLdapGetHttpError,
        "application/json",
        "error_code",
        "error_message"
    },
    {
        VDIR_REST_RSC_ETCD,
        "/v1/post/etcd",
        TRUE,
        VmDirRESTLdapSetResult,
        VmDirRESTLdapGetHttpError,
        "application/json",
        "code",
        "error"
    },
    {
        VDIR_REST_RSC_METRICS,
        "/v1/post/metrics",
        FALSE,
        VmDirRESTUnknownSetResult,
        VmDirRESTUnknownGetHttpError,
        "text/plain",
        "error_code",
        "error_message"
    },
    {
        VDIR_REST_RSC_IDP,
        VMDIR_V1_RESOURCE_IDP,
        FALSE,
        VmDirRESTUnknownSetResult,
        VmDirRESTUnknownGetHttpError,
        "application/json",
        "error_code",
        "error_message"
    },
    {
        VDIR_REST_RSC_UNKNOWN,
        NULL,
        FALSE,
        VmDirRESTUnknownSetResult,
        VmDirRESTUnknownGetHttpError,
        NULL,
        NULL,
        NULL
    }
};

PVDIR_REST_RESOURCE
VmDirRESTGetResourceByPath(
    PSTR    pszPath
    )
{
    DWORD   i = 0;
    BOOLEAN bValidPath = FALSE;

    bValidPath = !IsNullOrEmptyString(pszPath);

    for (i = 0; resources[i].pszEndpoint; i++)
    {
        if (bValidPath)
        {
            if (resources[i].bIsEndpointPrefix)
            {
                if (VmDirStringStartsWith(
                        pszPath, resources[i].pszEndpoint, FALSE))
                {
                    break;
                }
            }
            else
            {
                if (VmDirStringCompareA(
                        pszPath, resources[i].pszEndpoint, FALSE) == 0)
                {
                    break;
                }
            }
        }
    }

    return &resources[i];
}

DWORD
VmDirRESTUnknownSetResult(
    PVDIR_REST_RESULT   pRestRslt,
    PVDIR_LDAP_RESULT   pLdapRslt,  // ignored
    DWORD               dwErr,
    PSTR                pszErrMsg
    )
{
    DWORD   dwError = 0;

    if (!pRestRslt)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirRESTResultSetError(pRestRslt, (int)dwErr, pszErrMsg);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
VmDirRESTUnknownGetHttpError(
    PVDIR_REST_RESULT   pRestRslt,
    PVDIR_HTTP_ERROR*   ppHttpError
    )
{
    DWORD   dwError = 0;
    int     httpStatus = 0;

    if (!pRestRslt || !ppHttpError)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    switch ((DWORD)pRestRslt->errCode)
    {
    case VMDIR_SUCCESS:
        httpStatus = HTTP_OK;
        break;

    case VMDIR_ERROR_INVALID_REQUEST:
        httpStatus = HTTP_BAD_REQUEST;
        break;

    case VMDIR_ERROR_UNAVAILABLE:
        httpStatus = HTTP_SERVICE_UNAVAILABLE;
        break;

    default:
        httpStatus = HTTP_INTERNAL_SERVER_ERROR;
        break;
    }

    *ppHttpError = VmDirRESTGetHttpError(httpStatus);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}
