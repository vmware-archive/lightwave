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

static
BOOL
_VmDirRESTProcessLocally(
    PVDIR_REST_OPERATION    pRestOp
    );

/*
 * We provide this function as callback to c-rest-engine,
 * c-rest-engine will use this callback upon receiving a request
 */
DWORD
VmDirHTTPRequestHandler(
    PVMREST_HANDLE  pRESTHandle,
    PREST_REQUEST   pRequest,
    PREST_RESPONSE* ppResponse,
    uint32_t        paramsCount
    )
{
    DWORD   dwError = 0;

    dwError = VmDirRESTRequestHandlerInternal(
            pRESTHandle, pRequest, ppResponse, paramsCount, TRUE);//TRUE - if HTTP request
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
VmDirHTTPSRequestHandler(
    PVMREST_HANDLE  pRESTHandle,
    PREST_REQUEST   pRequest,
    PREST_RESPONSE* ppResponse,
    uint32_t        paramsCount
    )
{
    DWORD   dwError = 0;

    dwError = VmDirRESTRequestHandlerInternal(
            pRESTHandle, pRequest, ppResponse, paramsCount, FALSE);//FALSE - if HTTPS request
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
VmDirRESTRequestHandlerInternal(
    PVMREST_HANDLE  pRESTHandle,
    PREST_REQUEST   pRequest,
    PREST_RESPONSE* ppResponse,
    uint32_t        paramsCount,
    BOOLEAN         bHttpRequest
    )
{
    DWORD   dwError = 0;
    DWORD   dwRspErr = 0;   // don't bail on this
    PVDIR_REST_OPERATION    pRestOp = NULL;
    uint64_t    iStartTime = 0;
    uint64_t    iEndTime = 0;

    if (!pRESTHandle || !pRequest || !ppResponse)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_UNAVAILABLE);
    }

    iStartTime = VmDirGetTimeInMilliSec();

    dwError = VmDirRESTOperationCreate(&pRestOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTOperationReadRequest(
            pRestOp, pRESTHandle, pRequest, paramsCount);
    BAIL_ON_VMDIR_ERROR(dwError);

    // if node is leader or the request needs to be processed locally
    if (_VmDirRESTProcessLocally(pRestOp))
    {
        dwError = VmDirRESTOperationParseRequestPayload(pRestOp);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirRESTOperationProcessRequest(pRestOp);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VmDirRESTProxyForwardRequest(pRestOp, bHttpRequest);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    // write response
    VMDIR_SET_REST_RESULT(pRestOp, NULL, dwError, NULL);
    dwRspErr = VmDirRESTOperationWriteResponse(pRestOp, pRESTHandle, ppResponse);

    // collect metrics
    iEndTime = VmDirGetTimeInMilliSec();
    VmDirRestMetricsUpdateFromHandler(pRestOp, iStartTime, iEndTime);

    // free memory
    VmDirFreeRESTOperation(pRestOp);

    // if failed to write response, must return error to c-rest-engine
    // we choose to return dwError over dwRspErr
    return dwRspErr ? (dwError ? dwError : dwRspErr) : 0;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d) for client: %s",
            __FUNCTION__,
            dwError,
            pRestOp ? VDIR_SAFE_STRING(pRestOp->pszClientIP) : "");

    goto cleanup;
}

static
BOOL
_VmDirRESTProcessLocally(
    PVDIR_REST_OPERATION    pRestOp
    )
{
    BOOL    bReturn = FALSE;
    VDIR_RAFT_ROLE  role = VDIR_RAFT_ROLE_CANDIDATE;

    VmDirRaftGetRole(&role);

    if (!pRestOp || !pRestOp->pszPath)
    {
        // This is an invalid case, we will return true so that
        // the error is handled locally in the upcoming calls
        bReturn = TRUE;
    }
    else if (role == VDIR_RAFT_ROLE_LEADER)
    {
        bReturn = TRUE;
    }
    else if (pRestOp->pResource->rscType == VDIR_REST_RSC_METRICS)
    {
        // currently only metrics API will be an exception
        // Any more exceptions should be added here.
        bReturn = TRUE;
    }
    else
    {
        bReturn = FALSE;
    }

    return bReturn;
}
