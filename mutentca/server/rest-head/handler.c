/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the [0m~@~\License[0m~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an [0m~@~\AS IS[0m~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "includes.h"

static
DWORD
_LwCARestApiRequestHandler(
    PREST_API_DEF       pRestApiDef,
    PVMREST_HANDLE      pRESTHandle,
    PREST_REQUEST       pRequest,
    PREST_RESPONSE*     ppResponse,
    uint32_t            paramsCount
    );

/*
 * We provide this function as callback to c-rest-engine,
 * c-rest-engine will use this callback upon receiving a request
 */
DWORD
LwCARestApiRequestHandler(
    PVMREST_HANDLE      pRESTHandle,
    PREST_REQUEST       pRequest,
    PREST_RESPONSE*     ppResponse,
    uint32_t            paramsCount
    )
{
    return _LwCARestApiRequestHandler(
                gpLwCARestApiDef,
                pRESTHandle,
                pRequest,
                ppResponse,
                paramsCount);
}

/*
 * We provide this function as callback to c-rest-engine,
 * c-rest-engine will use this callback upon receiving a request
 */
DWORD
LwCARestMetricsApiRequestHandler(
    PVMREST_HANDLE      pRESTHandle,
    PREST_REQUEST       pRequest,
    PREST_RESPONSE*     ppResponse,
    uint32_t            paramsCount
    )
{
    return _LwCARestApiRequestHandler(
                gpLwCARestMetricsApiDef,
                pRESTHandle,
                pRequest,
                ppResponse,
                paramsCount);
}

static
DWORD
_LwCARestApiRequestHandler(
    PREST_API_DEF       pRestApiDef,
    PVMREST_HANDLE      pRESTHandle,
    PREST_REQUEST       pRequest,
    PREST_RESPONSE*     ppResponse,
    uint32_t            paramsCount
    )
{
    DWORD                   dwError     = 0;
    DWORD                   dwRspErr    = 0;
    PLWCA_REST_OPERATION    pRestOp     = NULL;

    if (!pRestApiDef || !pRESTHandle || !pRequest || !ppResponse)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (LwCASrvGetState() == LWCAD_SHUTDOWN)
    {
        dwError = LWCA_ERROR_UNAVAILABLE;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCARestOperationCreate(&pRestOp);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCARestOperationReadRequest(pRestOp, pRESTHandle, pRequest, paramsCount);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCARestOperationParseRequestPayload(pRestOp);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCARestOperationProcessRequest(pRestOp, pRestApiDef);
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    // write response
    LwCASetRestResult(pRestOp, dwError);
    dwRspErr = LwCARestOperationWriteResponse(pRestOp, pRESTHandle, ppResponse);

    // free memory
    LwCAFreeRESTOperation(pRestOp);

    // if failed to write response, must return error to c-rest-engine
    // we choose to return dwError over dwRspErr
    return dwRspErr ? (dwError ? dwError : dwRspErr) : 0;

error:
    LWCA_LOG_ERROR(
            "%s failed, error (%d) for client: (%s), requestID: (%s)",
            __FUNCTION__,
            dwError,
            pRestOp ? LWCA_SAFE_STRING(pRestOp->pszClientIP) : "",
            pRestOp ? LWCA_SAFE_STRING(pRestOp->pszRequestId) : "");

    goto cleanup;
}
