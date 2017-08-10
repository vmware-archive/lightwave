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

static VDNS_REST_RESOURCE resources[VDNS_REST_RSC_COUNT] =
{
    {
        VDNS_REST_RSC_METRICS,
        "/v1/dns/metrics",
        FALSE,
        VmDnsRESTUnknownSetResult,
        VmDnsRESTUnknownGetHttpError,
        "error-code",
        "error-message"
    },
    {
       VDNS_REST_RSC_UNKNOWN,
       NULL,
       FALSE,
       VmDnsRESTUnknownSetResult,
       VmDnsRESTUnknownGetHttpError,
       NULL,
       NULL
    }
};

PVDNS_REST_RESOURCE
VmDnsRESTGetResource(
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
                if (VmDnsStringStartsWith(
                        pszPath, resources[i].pszEndpoint, FALSE))
                {
                    break;
                }
            }
            else
            {
                if (VmDnsStringCompareA(
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
VmDnsRESTUnknownSetResult(
    PVDNS_REST_RESULT   pRestRslt,
    DWORD               dwErr,
    PSTR                pszErrMsg
    )
{
    DWORD   dwError = 0;

    if (!pRestRslt)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsRESTResultSetError(pRestRslt, (int)dwErr, pszErrMsg);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDnsRESTUnknownGetHttpError(
    PVDNS_REST_RESULT   pRestRslt,
    PSTR*               ppszHttpStatus,
    PSTR*               ppszHttpReason
    )
{
    DWORD   dwError = 0;
    int     httpStatus = 0;
    PVDNS_HTTP_ERROR    pHttpError = NULL;

    if (!ppszHttpStatus || !ppszHttpReason)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    switch ((DWORD)pRestRslt->errCode)
    {
    case 0:
        httpStatus = HTTP_OK;
        break;

    case 1:
        httpStatus = HTTP_BAD_REQUEST;
        break;

    default:
        httpStatus = HTTP_INTERNAL_SERVER_ERROR;
        break;
    }

    pHttpError = VmDnsRESTGetHttpError(httpStatus);

    *ppszHttpStatus = pHttpError->pszHttpStatus;
    *ppszHttpReason = pHttpError->pszHttpReason;

cleanup:
    return dwError;

error:
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}
