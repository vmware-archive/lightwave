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

static LWCA_REST_RESOURCE resources[LWCA_REST_RSC_COUNT] =
{
    {
        LWCA_REST_RSC_API,
        "/v1/mutentca",
        TRUE,
        LwCARestUnknownSetResult,
        LwCARestUnknownGetHttpError,
        "application/json",
    },
    {
        LWCA_REST_RSC_UNKNOWN,
        NULL,
        FALSE,
        LwCARestUnknownSetResult,
        LwCARestUnknownGetHttpError,
        NULL
    }
};

PLWCA_REST_RESOURCE
LwCARestGetResource(
    PCSTR pcszPath
    )
{
    DWORD   dwIdx       = 0;
    BOOLEAN bValidPath  = FALSE;

    bValidPath = !IsNullOrEmptyString(pcszPath);

    for (; resources[dwIdx].pszEndpoint; ++dwIdx)
    {
        if (bValidPath)
        {
            if (resources[dwIdx].bIsEndpointPrefix)
            {
                if (LwCAStringStartsWith(pcszPath, resources[dwIdx].pszEndpoint, FALSE))
                {
                    break;
                }
            }
            else
            {
                if (!LwCAStringCompareA(pcszPath, resources[dwIdx].pszEndpoint, FALSE))
                {
                    break;
                }
            }
        }
    }

    return &resources[dwIdx];
}

DWORD
LwCARestUnknownSetResult(
    PLWCA_REST_RESULT   pRestRslt,
    PCSTR               pcszRequestId,
    DWORD               dwErr,
    PCSTR               pcszErrDetail
    )
{
    DWORD dwError = 0;

    if (!pRestRslt)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateStringA(LWCA_SAFE_STRING(pcszRequestId), &pRestRslt->pszRequestId);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCARestResultSetError(pRestRslt, (int)dwErr, pcszErrDetail);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    return dwError;

error:
    LWCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
LwCARestUnknownGetHttpError(
    PLWCA_REST_RESULT   pRestRslt,
    PCSTR               pcszMethod,
    PLWCA_HTTP_ERROR*   ppHttpError
    )
{
    DWORD               dwError = 0;
    int                 httpStatus = 0;
    PLWCA_HTTP_ERROR    pHttpError = NULL;

    if (!pRestRslt || !ppHttpError)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    switch ((DWORD)pRestRslt->errCode)
    {
    case LWCA_SUCCESS:
        if (LwCAStringCompareA(pcszMethod, "DELETE", false) == 0)
        {
            httpStatus = HTTP_NO_CONTENT;
        }
        else
        {
            httpStatus = HTTP_OK;
        }
        break;

    case LWCA_ERROR_INVALID_REQUEST:
    case LWCA_SN_POLICY_VIOLATION:
    case LWCA_SAN_POLICY_VIOLATION:
    case LWCA_KEY_USAGE_POLICY_VIOLATION:
    case LWCA_CERT_DURATION_POLICY_VIOLATION:
    case LWCA_INVALID_CSR_FIELD:
    case LWCA_INVALID_CA_FOR_CERT_REVOKE:
        httpStatus = HTTP_BAD_REQUEST;
        break;

    case LWCA_ERROR_OIDC_BAD_AUTH_DATA:
    case LWCA_ERROR_OIDC_INVALID_POP:
    case LWCA_ERROR_OIDC_UNKNOWN_TOKEN:
    case LWCA_ERROR_REST_UNAUTHENTICATED:
        httpStatus = HTTP_UNAUTHORIZED;
        break;

    case LWCA_CA_ALREADY_EXISTS:
    case LWCA_CRL_CERT_ALREADY_REVOKED:
    case LWCA_CA_ALREADY_REVOKED:
        httpStatus = HTTP_CONFLICT;
        break;

    case LWCA_CA_MISSING:
        httpStatus = HTTP_NOT_FOUND;
        break;

    case LWCA_ERROR_AUTHZ_UNAUTHORIZED:
    case LWCA_CA_REVOKED:
        httpStatus = HTTP_FORBIDDEN;
        break;

    default:
        httpStatus = HTTP_INTERNAL_SERVER_ERROR;
        break;
    }

    pHttpError = LwCARestGetHttpError(httpStatus);

    *ppHttpError = pHttpError;

cleanup:
    return dwError;

error:
    LWCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}
