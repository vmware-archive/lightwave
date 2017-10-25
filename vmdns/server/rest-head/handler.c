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

/*
 * We provide this function as callback to c-rest-engine,
 * c-rest-engine will use this callback upon receiving a request
 */
DWORD
VmDnsRESTRequestHandler(
    PVMREST_HANDLE  pRESTHandle,
    PREST_REQUEST   pRequest,
    PREST_RESPONSE* ppResponse,
    uint32_t        paramsCount
    )
{
    DWORD   dwError = 0;
    DWORD   dwRestOpErr = 0;    // don't bail on this
    PVDNS_REST_OPERATION    pRestOp = NULL;

    if (!pRESTHandle || !pRequest || !ppResponse)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (VmDnsdState() == VMDNS_SHUTDOWN)
    {
        goto cleanup;
    }

    dwRestOpErr = VmDnsRESTOperationCreate(&pRestOp);
    if (dwRestOpErr)
    {
        dwError = VmDnsRESTWriteSimpleErrorResponse(
                pRESTHandle, ppResponse, 500);  // 500 = Internal Server Error
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        dwRestOpErr = VmDnsRESTProcessRequest(
                pRestOp, pRESTHandle, pRequest);

        dwError = VmDnsRESTOperationWriteResponse(
                pRestOp, pRESTHandle, ppResponse);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:
    VmDnsFreeRESTOperation(pRestOp);
    return dwError;

error:
    VmDnsLog(
            VMDNS_LOG_LEVEL_ERROR,
            "%s failed, error (%d), rest operation error (%d)",
            __FUNCTION__,
            dwError,
            dwRestOpErr);

    goto cleanup;
}

DWORD
VmDnsRESTProcessRequest(
    PVDNS_REST_OPERATION    pRestOp,
    PVMREST_HANDLE          pRESTHandle,
    PREST_REQUEST           pRequest
    )
{
    DWORD   dwError = 0;
    PREST_API_METHOD    pMethod = NULL;

    if (!pRestOp || !pRESTHandle || !pRequest)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsRESTOperationReadRequest(pRestOp, pRESTHandle, pRequest);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = coapi_find_handler(
            gpVdnsRestApiDef,
            pRestOp->pszPath,
            pRestOp->pszMethod,
            &pMethod);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = pMethod->pFnImpl((void*)pRestOp, NULL);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VMDNS_SET_REST_RESULT(pRestOp, dwError, NULL);
    return dwError;

error:
    VmDnsLog(
            VMDNS_LOG_LEVEL_ERROR,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
VmDnsRESTWriteSimpleErrorResponse(
    PVMREST_HANDLE  pRESTHandle,
    PREST_RESPONSE* ppResponse,
    int             httpStatus
    )
{
    DWORD   dwError = 0;
    PVDNS_HTTP_ERROR    pHttpError = NULL;

    if (!pRESTHandle || !ppResponse)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmRESTSetHttpStatusVersion(ppResponse, "HTTP/1.1");
    BAIL_ON_VMDNS_ERROR(dwError);

    pHttpError = VmDnsRESTGetHttpError(httpStatus);

    dwError = VmRESTSetHttpStatusCode(ppResponse, pHttpError->pszHttpStatus);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmRESTSetHttpReasonPhrase(ppResponse, pHttpError->pszHttpReason);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmRESTSetHttpHeader(ppResponse, "Connection", "close");
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    VmDnsLog(
            VMDNS_LOG_LEVEL_ERROR,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}
