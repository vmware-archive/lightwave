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

static VMCA_REST_RESOURCE resources[VMCA_REST_RSC_COUNT] =
{
    {
        VMCA_REST_RSC_UNKNOWN,
        NULL,
        FALSE,
        VMCARestUnknownSetResult,
        VMCARestUnknownGetHttpError,
        NULL,
        NULL
    }
};

PVMCA_REST_RESOURCE
VMCARestGetResource(
    PSTR pszPath
    )
{
    DWORD   i          = 0;
    BOOLEAN bValidPath = FALSE;

    bValidPath = !IsNullOrEmptyString(pszPath);

    for (i = 0; resources[i].pszEndpoint; i++)
    {
        if (bValidPath)
        {
            if (resources[i].bIsEndpointPrefix)
            {
                if (VMCAStringStartsWith(
                        pszPath, resources[i].pszEndpoint, FALSE))
                {
                    break;
                }
            }
            else
            {
                if (VMCAStringCompareA(
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
VMCARestUnknownSetResult(
    PVMCA_REST_RESULT   pRestRslt,
    DWORD               dwErr,
    PSTR                pszErrMsg
    )
{
    DWORD   dwError = 0;

    if (!pRestRslt)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCARestResultSetError(pRestRslt, (int)dwErr, pszErrMsg);
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
VMCARestUnknownGetHttpError(
    PVMCA_REST_RESULT   pRestRslt,
    PVMCA_HTTP_ERROR*   ppHttpError
    )
{
    DWORD               dwError = 0;
    int                 httpStatus = 0;
    PVMCA_HTTP_ERROR    pHttpError = NULL;

    if (!pRestRslt || !ppHttpError)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    switch ((DWORD)pRestRslt->errCode)
    {
    case VMCA_SUCCESS:
        httpStatus = HTTP_OK;
        break;

    case VMCA_ERROR_INVALID_REQUEST:
        httpStatus = HTTP_BAD_REQUEST;
        break;

    default:
        httpStatus = HTTP_INTERNAL_SERVER_ERROR;
        break;
    }

    pHttpError = VMCARestGetHttpError(httpStatus);

    *ppHttpError = pHttpError;

cleanup:
    return dwError;

error:
    VMCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}
