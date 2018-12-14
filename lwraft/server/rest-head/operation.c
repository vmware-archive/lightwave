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

DWORD
VmDirRESTOperationCreate(
    PVDIR_REST_OPERATION*   ppRestOp
    )
{
    DWORD   dwError = 0;
    PVDIR_REST_OPERATION    pRestOp = NULL;

    if (!ppRestOp)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_REST_OPERATION), (PVOID*)&pRestOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(
            &pRestOp->pParamMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateConnection(&pRestOp->pConn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTResultCreate(&pRestOp->pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    pRestOp->pResource = VmDirRESTGetResourceByPath(NULL);
    pRestOp->bisValidOrigin = FALSE;

    *ppRestOp = pRestOp;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    VmDirFreeRESTOperation(pRestOp);
    goto cleanup;
}

DWORD
VmDirRESTOperationReadRequest(
    PVDIR_REST_OPERATION    pRestOp,
    PVMREST_HANDLE          pRESTHandle,
    PREST_REQUEST           pRestReq,
    DWORD                   dwParamCount
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0, bytesRead = 0;
    PSTR    pszKey = NULL;
    PSTR    pszTemp = NULL;
    PSTR    pszVal = NULL;
    PSTR    pszBody = NULL;
    size_t  len = 0;

    if (!pRestOp || !pRESTHandle || !pRestReq)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    // read request methods
    dwError = VmRESTGetHttpMethod(pRestReq, &pRestOp->pszMethod);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Get the client IP
    dwError = VmRESTGetConnectionInfo(pRestReq, &pRestOp->pszClientIP, (INT32 *) &pRestOp->dwPort);
    BAIL_ON_VMDIR_ERROR(dwError);

    // read raw request URI as sent from client - for token POP validation
    dwError = VmRESTGetHttpURI(pRestReq, FALSE, &pRestOp->pszURI);
    BAIL_ON_VMDIR_ERROR(dwError);

    // read decoded request URI and truncate parameters
    dwError = VmRESTGetHttpURI(pRestReq, TRUE, &pRestOp->pszPath);
    BAIL_ON_VMDIR_ERROR(dwError);

    pszTemp = VmDirStringChrA(pRestOp->pszPath, '?');
    if (pszTemp)
    {
        *pszTemp = '\0';
    }

    // determine resource
    pRestOp->pResource = VmDirRESTGetResourceByPath(pRestOp->pszPath);
    if (pRestOp->pResource->rscType == VDIR_REST_RSC_UNKNOWN)
    {
        dwError = VMDIR_ERROR_INVALID_REQUEST;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // extract sub-path
    if (pRestOp->pResource->bIsEndpointPrefix)
    {
        dwError = VmDirAllocateStringA(
                pRestOp->pszPath + strlen(pRestOp->pResource->pszEndpoint) + 1,
                &pRestOp->pszSubPath);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // set resource-specifc http error mapping function
    pRestOp->pfnGetHttpError = pRestOp->pResource->pfnGetHttpError;

    // read request headers
    dwError = VmRESTGetHttpHeader(pRestReq, VMDIR_REST_HEADER_AUTHENTICATION, &pRestOp->pszAuth);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmRESTGetHttpHeader(pRestReq, VMDIR_REST_HEADER_IF_MATCH, &pRestOp->pszHeaderIfMatch);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmRESTGetHttpHeader(pRestReq, VMDIR_REST_HEADER_CONTENT_TYPE, &pRestOp->pszContentType);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmRESTGetHttpHeader(pRestReq, VMDIR_REST_HEADER_CONNECTION, &pRestOp->pszHeaderConnection);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmRESTGetHttpHeader(pRestReq, VMDIR_REST_HEADER_DATE, &pRestOp->pszDate);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmRESTGetHttpHeader(pRestReq, VMDIR_REST_HEADER_ORIGIN, &pRestOp->pszOrigin);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmRESTGetHttpHeader(pRestReq, VMDIR_REST_HEADER_REQUESTID, &pRestOp->pConn->pThrLogCtx->pszRequestId);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pRestOp->pszOrigin)
    {
        dwError = VmDirRESTIsValidOrigin(pRestOp->pszOrigin, &pRestOp->bisValidOrigin);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // read request params
    for (i = 1; i <= dwParamCount; i++)
    {
        dwError = VmRESTGetParamsByIndex(pRestReq, dwParamCount, i, &pszKey, &pszVal);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapInsert(pRestOp->pParamMap, pszKey, pszVal, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        pszKey = NULL;
        pszVal = NULL;
    }

    // read request input json
    do
    {
        if (bytesRead || !pszBody)
        {
            dwError = VmDirReallocateMemoryWithInit(
                    (PVOID)pszBody,
                    (PVOID*)&pszBody,
                    len + MAX_REST_PAYLOAD_LENGTH + 1,
                    len);     // +1 for NULL char
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        bytesRead = 0;
        dwError = VmRESTGetData(
                pRESTHandle, pRestReq, pszBody + len, &bytesRead);

        len += bytesRead;
    }
    while (dwError == REST_ENGINE_MORE_IO_REQUIRED);
    BAIL_ON_VMDIR_ERROR(dwError);

    // save for signature validation and proxy forwarding
    pRestOp->pszBody = pszBody;

    VMDIR_LOG_INFO(
            VMDIR_LOG_MASK_ALL,
            "Received REST request from: %s request type: %s request URI: %s",
            VDIR_SAFE_STRING(pRestOp->pszClientIP),
            VDIR_SAFE_STRING(pRestOp->pszMethod),
            VDIR_SAFE_STRING(pRestOp->pszPath));

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);
    VMDIR_SAFE_FREE_STRINGA(pszBody);
    goto cleanup;
}

DWORD
VmDirRESTOperationParseRequestPayload(
    PVDIR_REST_OPERATION    pRestOp
    )
{
    DWORD   dwError = 0;
    json_error_t    jError = {0};

    if (!pRestOp)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (!IsNullOrEmptyString(pRestOp->pszBody))
    {
        pRestOp->pjBody = json_loads(pRestOp->pszBody, 0, &jError);
        if (!pRestOp->pjBody)
        {
            VMDIR_LOG_ERROR(
                    VMDIR_LOG_MASK_ALL,
                    "%s failed to parse json payload: "
                    "(text=%s), "
                    "(source=%s), "
                    "(line=%d), "
                    "(column=%d), "
                    "(position=%d)",
                    __FUNCTION__,
                    jError.text,
                    jError.source,
                    jError.line,
                    jError.column,
                    jError.position);

            dwError = VMDIR_ERROR_INVALID_REQUEST;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

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
VmDirRESTOperationProcessRequest(
    PVDIR_REST_OPERATION    pRestOp
    )
{
    DWORD   dwError = 0;
    PVDIR_REST_PERF_METRIC  pPerfMetric = NULL;

    if (!pRestOp)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pPerfMetric = &pRestOp->perfMetric;

    dwError = VmDirRESTAuth(pRestOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (VmDirStringCompareA(pRestOp->pszMethod, HTTP_METHOD_OPTIONS, FALSE) != 0)
    {
        dwError = coapi_find_handler(
                gpVdirRestApiDef,
                pRestOp->pszPath,
                pRestOp->pszMethod,
                &pRestOp->pMethod);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_COLLECT_TIME(pPerfMetric->iHandlerStartTime);
        dwError = pRestOp->pMethod->pFnImpl((void*)pRestOp, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
        VMDIR_COLLECT_TIME(pPerfMetric->iHandlerEndTime);
    }

    VMDIR_LOG_INFO(
            VMDIR_LOG_MASK_ALL,
            "Locally processed REST request from: %s request type: %s request URI: %s",
            VDIR_SAFE_STRING(pRestOp->pszClientIP),
            VDIR_SAFE_STRING(pRestOp->pszMethod),
            VDIR_SAFE_STRING(pRestOp->pszPath));

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d) for client: %s",
            __FUNCTION__,
            dwError,
            VDIR_SAFE_STRING(pRestOp->pszClientIP));

    goto cleanup;
}

/*
 * Set HTTP headers as well as payload
 */
DWORD
VmDirRESTOperationWriteResponse(
    PVDIR_REST_OPERATION    pRestOp,
    PVMREST_HANDLE          pRESTHandle,
    PREST_RESPONSE*         ppResponse
    )
{
    DWORD   dwError = 0;
    DWORD   bytesWritten = 0;
    PVDIR_REST_RESULT   pResult = NULL;
    PVDIR_REST_RESOURCE pResource = NULL;
    PVDIR_HTTP_ERROR    pHttpError = NULL;
    PSTR    pszBodyLen = NULL;
    size_t  bodyLen = 0;
    size_t  sentLen = 0;

    if (!pRestOp || !pRESTHandle || !ppResponse)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pResult = pRestOp->pResult;
    pResource = pRestOp->pResource;

    dwError = VmRESTSetHttpStatusVersion(ppResponse, "HTTP/1.1");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = pRestOp->pfnGetHttpError(pResult, &pHttpError);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmRESTSetHttpStatusCode(ppResponse, pHttpError->pszHttpStatus);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmRESTSetHttpReasonPhrase(ppResponse, pHttpError->pszHttpReason);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmRESTSetHttpHeader(
            ppResponse, "Connection", pRestOp->pszHeaderConnection ? pRestOp->pszHeaderConnection : "close");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmRESTSetHttpHeader(ppResponse, "Content-Type", pResource->pszContentType);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTSetCORSHeaders(pRestOp, ppResponse);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTResultGenerateResponseBody(pResult, pResource);
    BAIL_ON_VMDIR_ERROR(dwError);

    bodyLen = pResult->dwBodyLen;

    dwError = VmDirAllocateStringPrintf(&pszBodyLen, "%u", bodyLen);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmRESTSetDataLength(
            ppResponse, bodyLen > MAX_REST_PAYLOAD_LENGTH ? NULL : pszBodyLen);
    BAIL_ON_VMDIR_ERROR(dwError);

    do
    {
        size_t chunkLen = bodyLen > MAX_REST_PAYLOAD_LENGTH ?
                MAX_REST_PAYLOAD_LENGTH : bodyLen;

        dwError = VmRESTSetData(
                pRESTHandle,
                ppResponse,
                VDIR_SAFE_STRING(pResult->pszBody) + sentLen,
                chunkLen,
                &bytesWritten);

        sentLen += bytesWritten;
        bodyLen -= bytesWritten;
    }
    while (dwError == REST_ENGINE_MORE_IO_REQUIRED);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!VMDIR_IS_HTTP_STATUS_OK(pHttpError->dwHttpStatus))
    {
        if (pHttpError->dwHttpStatus == HTTP_PRECONDITION_FAILED)
        {
            VMDIR_LOG_VERBOSE(
                    VMDIR_LOG_MASK_ALL,
                    "%s HTTP response status (%d), body (%.*s)",
                    __FUNCTION__,
                    pHttpError->dwHttpStatus,
                    VMDIR_MIN(sentLen, VMDIR_MAX_LOG_OUTPUT_LEN),
                    pResult->pszBody);
        }
        else
        {
            VMDIR_LOG_WARNING(
                    VMDIR_LOG_MASK_ALL,
                    "%s HTTP response status (%d), body (%.*s)",
                    __FUNCTION__,
                    pHttpError->dwHttpStatus,
                    VMDIR_MIN(sentLen, VMDIR_MAX_LOG_OUTPUT_LEN),
                    pResult->pszBody);
        }
    }

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszBodyLen);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

VOID
VmDirFreeRESTOperation(
    PVDIR_REST_OPERATION    pRestOp
    )
{
    if (pRestOp)
    {
        VMDIR_SAFE_FREE_MEMORY(pRestOp->pszAuth);
        VMDIR_SAFE_FREE_MEMORY(pRestOp->pszMethod);
        VMDIR_SAFE_FREE_MEMORY(pRestOp->pszURI);
        VMDIR_SAFE_FREE_MEMORY(pRestOp->pszPath);
        VMDIR_SAFE_FREE_MEMORY(pRestOp->pszSubPath);

        VMDIR_SAFE_FREE_MEMORY(pRestOp->pszHeaderIfMatch);
        VMDIR_SAFE_FREE_MEMORY(pRestOp->pszContentType);
        VMDIR_SAFE_FREE_MEMORY(pRestOp->pszClientIP);
        VMDIR_SAFE_FREE_MEMORY(pRestOp->pszHeaderConnection);
        VMDIR_SAFE_FREE_MEMORY(pRestOp->pszDate);
        VMDIR_SAFE_FREE_MEMORY(pRestOp->pszOrigin);

        VMDIR_SAFE_FREE_MEMORY(pRestOp->pszBody);
        if (pRestOp->pjBody)
        {
            json_decref(pRestOp->pjBody);
        }
        LwRtlHashMapClear(pRestOp->pParamMap, VmDirSimpleHashMapPairFree, NULL);
        LwRtlFreeHashMap(&pRestOp->pParamMap);
        VmDirDeleteConnection(&pRestOp->pConn);
        VmDirFreeRESTResult(pRestOp->pResult);
        VMDIR_SAFE_FREE_MEMORY(pRestOp);
    }
}
