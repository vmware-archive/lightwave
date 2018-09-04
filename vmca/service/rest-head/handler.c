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

/*
 * We provide this function as callback to c-rest-engine,
 * c-rest-engine will use this callback upon receiving a request
 */
DWORD
VMCARestApiRequestHandler(
    PVMREST_HANDLE  pRESTHandle,
    PREST_REQUEST   pRequest,
    PREST_RESPONSE* ppResponse,
    uint32_t        paramsCount
    )
{
    DWORD                   dwError     = 0;
    DWORD                   dwRspErr    = 0;
    PVMCA_REST_OPERATION    pRestOp     = NULL;

    if (!pRESTHandle || !pRequest || !ppResponse)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (VMCASrvGetState() == VMCAD_SHUTDOWN)
    {
        dwError = VMCA_ERROR_UNAVAILABLE;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCARestOperationCreate(&pRestOp);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARestOperationReadRequest(
                pRestOp, pRESTHandle, pRequest, paramsCount);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARestOperationParseRequestPayload(pRestOp);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARestOperationProcessRequest(pRestOp);
    BAIL_ON_VMCA_ERROR(dwError);

cleanup:
    // write response
    VMCASetRestResult(pRestOp, dwError, NULL);
    dwRspErr = VMCARestOperationWriteResponse(pRestOp, pRESTHandle, ppResponse);

    // free memory
    VMCAFreeRESTOperation(pRestOp);
    return dwError;

error:
    VMCALog(
            VMCA_LOG_LEVEL_ERROR,
            "%s failed, error (%d) for client %s",
            __FUNCTION__,
            dwError,
            pRestOp ? VMCA_SAFE_STRING(pRestOp->pszClientIP) : "");

    goto cleanup;
}
