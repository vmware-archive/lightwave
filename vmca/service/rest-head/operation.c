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

DWORD
VMCARestOperationCreate(
    PVMCA_REST_OPERATION*   ppRestOp
    )
{
    DWORD                   dwError = 0;
    PVMCA_REST_OPERATION    pRestOp = NULL;

    if (!ppRestOp)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocateMemory(
            sizeof(VMCA_REST_OPERATION), (PVOID*)&pRestOp);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = LwRtlCreateHashMap(
            &pRestOp->pParamMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARestResultCreate(&pRestOp->pResult);
    BAIL_ON_VMCA_ERROR(dwError);

    pRestOp->pResource = VMCARestGetResource(NULL);

    *ppRestOp = pRestOp;

cleanup:
    return dwError;

error:
    VMCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    VMCAFreeRESTOperation(pRestOp);
    goto cleanup;
}

DWORD
VMCARestOperationReadRequest(
    PVMCA_REST_OPERATION    pRestOp,
    PVMREST_HANDLE          pRESTHandle,
    PREST_REQUEST           pRestReq,
    DWORD                   dwParamCount
    )
{
    DWORD           dwError     = 0;
    DWORD           i           = 0;
    DWORD           bytesRead   = 0;
    PSTR            pszTmp      = NULL;
    PSTR            pszKey      = NULL;
    PSTR            pszVal      = NULL;
    PSTR            pszBody     = NULL;
    size_t          len         = 0;

    if (!pRestOp || !pRESTHandle || !pRestReq)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    // read request methods
    dwError = VmRESTGetHttpMethod(pRestReq, &pRestOp->pszMethod);
    BAIL_ON_VMCA_ERROR(dwError);

    // Get the client IP
    dwError = VmRESTGetConnectionInfo(pRestReq, &pRestOp->pszClientIP, &pRestOp->dwPort);
    BAIL_ON_VMCA_ERROR(dwError);

    // read raw request URI as sent from client - for token POP validation
    dwError = VmRESTGetHttpURI(pRestReq, FALSE, &pRestOp->pszURI);
    BAIL_ON_VMCA_ERROR(dwError);

    // read decoded request URI and truncate parameters
    dwError = VmRESTGetHttpURI(pRestReq, TRUE, &pRestOp->pszPath);
    BAIL_ON_VMCA_ERROR(dwError);

    pszTmp = VMCAStringChrA(pRestOp->pszPath, '?');
    if (pszTmp)
    {
        *pszTmp = '\0';
    }

    // determine resource
    pRestOp->pResource = VMCARestGetResource(pRestOp->pszPath);
    if (pRestOp->pResource->rscType == VMCA_REST_RSC_UNKNOWN)
    {
        dwError = VMCA_ERROR_INVALID_REQUEST;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    // extract sub-path
    if (pRestOp->pResource->bIsEndpointPrefix)
    {
        dwError = VMCAAllocateStringA(
                pRestOp->pszPath + strlen(pRestOp->pResource->pszEndpoint) + 1,
                &pRestOp->pszSubPath);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    // set resource-specific http error mapping function
    pRestOp->pfnGetHttpError = pRestOp->pResource->pfnGetHttpError;

    // read request headers
    dwError = VmRESTGetHttpHeader(pRestReq, VMCA_REST_HEADER_AUTHORIZATION, &pRestOp->pszAuth);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmRESTGetHttpHeader(pRestReq, VMCA_REST_HEADER_IF_MATCH, &pRestOp->pszHeaderIfMatch);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmRESTGetHttpHeader(pRestReq, VMCA_REST_HEADER_CONTENT_TYPE, &pRestOp->pszContentType);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmRESTGetHttpHeader(pRestReq, VMCA_REST_HEADER_CONNECTION, &pRestOp->pszHeaderConnection);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmRESTGetHttpHeader(pRestReq, VMCA_REST_HEADER_DATE, &pRestOp->pszDate);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmRESTGetHttpHeader(pRestReq, VMCA_REST_HEADER_ORIGIN, &pRestOp->pszOrigin);
    BAIL_ON_VMCA_ERROR(dwError);

    // read request params
    for (i = 1; i <= dwParamCount; i++)
    {
        dwError = VmRESTGetParamsByIndex(pRestReq, dwParamCount, i, &pszKey, &pszVal);
        BAIL_ON_VMCA_ERROR(dwError);

        dwError = LwRtlHashMapInsert(pRestOp->pParamMap, pszKey, pszVal, NULL);
        BAIL_ON_VMCA_ERROR(dwError);

        pszKey = NULL;
        pszVal = NULL;
    }

    // read request input json
    do
    {
        if (bytesRead || !pszBody)
        {
            dwError = VMCAReallocateMemoryWithInit(
                    (PVOID)pszBody,
                    (PVOID*)&pszBody,
                    len + MAX_REST_PAYLOAD_LENGTH + 1,
                    len);     // +1 for NULL char
            BAIL_ON_VMCA_ERROR(dwError);
        }

        bytesRead = 0;
        dwError = VmRESTGetData(
                pRESTHandle, pRestReq, pszBody + len, &bytesRead);

        len += bytesRead;
    }
    while (dwError == REST_ENGINE_MORE_IO_REQUIRED);
    BAIL_ON_VMCA_ERROR(dwError);

    // save for signature validation
    pRestOp->pszBody = pszBody;

    VMCA_LOG_INFO(
            "Received REST request from: %s request type: %s request URI: %s",
            VMCA_SAFE_STRING(pRestOp->pszClientIP),
            VMCA_SAFE_STRING(pRestOp->pszMethod),
            VMCA_SAFE_STRING(pRestOp->pszPath));

cleanup:
    return dwError;

error:
    VMCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    VMCA_SAFE_FREE_MEMORY(pszBody);
    goto cleanup;
}

DWORD
VMCARestOperationParseRequestPayload(
    PVMCA_REST_OPERATION    pRestOp
    )
{
    DWORD           dwError = 0;
    json_error_t    jError  = {0};

    if (!pRestOp)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pRestOp->pszBody))
    {
        pRestOp->pjBody = json_loads(pRestOp->pszBody, 0, &jError);
        if (!pRestOp->pjBody)
        {
            VMCA_LOG_ERROR(
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

            dwError = VMCA_ERROR_INVALID_REQUEST;
            BAIL_ON_VMCA_ERROR(dwError);
        }
    }

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
VMCARestOperationProcessRequest(
    PVMCA_REST_OPERATION    pRestOp
    )
{
    DWORD   dwError = 0;

    if (!pRestOp)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    // TODO: Will add RestAuth code in a subsequent commit
    //dwError = VMCARestAuth(pRestOp);
    //BAIL_ON_VMCA_ERROR(dwError);

    if (VMCAStringCompareA(pRestOp->pszMethod, HTTP_METHOD_OPTIONS, FALSE) != 0)
    {
        dwError = coapi_find_handler(
                gpVMCARestApiDef,
                pRestOp->pszPath,
                pRestOp->pszMethod,
                &pRestOp->pMethod);
        BAIL_ON_VMCA_ERROR(dwError);

        dwError = pRestOp->pMethod->pFnImpl((void*)pRestOp, NULL);
        BAIL_ON_VMCA_ERROR(dwError);
    }

    VMCA_LOG_INFO(
            "Locally processed REST request from: %s request type: %s request URI: %s",
            VMCA_SAFE_STRING(pRestOp->pszClientIP),
            VMCA_SAFE_STRING(pRestOp->pszMethod),
            VMCA_SAFE_STRING(pRestOp->pszPath));

cleanup:
    return dwError;

error:
    VMCA_LOG_ERROR(
            "%s failed, error (%d) for client: %s",
            __FUNCTION__,
            dwError,
            VMCA_SAFE_STRING(pRestOp->pszClientIP));

    goto cleanup;
}

/*
 * Set HTTP headers as well as payload
 */
DWORD
VMCARestOperationWriteResponse(
    PVMCA_REST_OPERATION    pRestOp,
    PVMREST_HANDLE          pRESTHandle,
    PREST_RESPONSE*         ppResponse
    )
{
    DWORD               dwError         = 0;
    DWORD               bytesWritten    = 0;
    PVMCA_REST_RESULT   pResult         = NULL;
    PVMCA_REST_RESOURCE pResource       = NULL;
    PVMCA_HTTP_ERROR    pHttpError      = NULL;
    PSTR                pszBodyLen      = NULL;
    size_t              bodyLen         = 0;
    size_t              sentLen         = 0;

    if (!pRestOp || !pRESTHandle || !ppResponse)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    pResult = pRestOp->pResult;
    pResource = pRestOp->pResource;

    dwError = VmRESTSetHttpStatusVersion(ppResponse, "HTTP/1.1");
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = pRestOp->pfnGetHttpError(pResult, &pHttpError);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmRESTSetHttpStatusCode(ppResponse, pHttpError->pszHttpStatus);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmRESTSetHttpReasonPhrase(ppResponse, pHttpError->pszHttpReason);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmRESTSetHttpHeader(
            ppResponse, "Connection", pRestOp->pszHeaderConnection ? pRestOp->pszHeaderConnection : "close");
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmRESTSetHttpHeader(ppResponse, "Content-Type", pResource->pszContentType);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCARestResultGenerateResponseBody(pResult, pResource);
    BAIL_ON_VMCA_ERROR(dwError);

    bodyLen = pResult->dwBodyLen;

    dwError = VMCAAllocateStringPrintfA(&pszBodyLen, "%ld", bodyLen);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VmRESTSetDataLength(
            ppResponse, bodyLen > MAX_REST_PAYLOAD_LENGTH ? NULL : pszBodyLen);
    BAIL_ON_VMCA_ERROR(dwError);

    do
    {
        size_t chunkLen = bodyLen > MAX_REST_PAYLOAD_LENGTH ?
                MAX_REST_PAYLOAD_LENGTH : bodyLen;

        dwError = VmRESTSetData(
                pRESTHandle,
                ppResponse,
                VMCA_SAFE_STRING(pResult->pszBody) + sentLen,
                chunkLen,
                &bytesWritten);

        sentLen += bytesWritten;
        bodyLen -= bytesWritten;
    }
    while (dwError == REST_ENGINE_MORE_IO_REQUIRED);
    BAIL_ON_VMCA_ERROR(dwError);

    if (pHttpError->httpStatus != HTTP_OK)
    {
        if (pHttpError->httpStatus == HTTP_PRECONDITION_FAILED)
        {
            VMCA_LOG_VERBOSE(
                    "%s HTTP response status (%d), body (%.*s)",
                    __FUNCTION__,
                    pHttpError->httpStatus,
                    VMCA_MIN(sentLen, VMCA_MAX_LOG_OUTPUT_LEN),
                    pResult->pszBody);
        }
        else
        {
            VMCA_LOG_WARNING(
                    "%s HTTP response status (%d), body (%.*s)",
                    __FUNCTION__,
                    pHttpError->httpStatus,
                    VMCA_MIN(sentLen, VMCA_MAX_LOG_OUTPUT_LEN),
                    pResult->pszBody);
        }
    }

cleanup:
    VMCA_SAFE_FREE_STRINGA(pszBodyLen);

    return dwError;

error:
    VMCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

VOID
VMCAFreeRESTOperation(
    PVMCA_REST_OPERATION    pRestOp
    )
{
    if (pRestOp)
    {
        VMCA_SAFE_FREE_MEMORY(pRestOp->pszAuth);
        VMCA_SAFE_FREE_MEMORY(pRestOp->pszMethod);
        VMCA_SAFE_FREE_MEMORY(pRestOp->pszURI);
        VMCA_SAFE_FREE_MEMORY(pRestOp->pszPath);
        VMCA_SAFE_FREE_MEMORY(pRestOp->pszSubPath);

        VMCA_SAFE_FREE_MEMORY(pRestOp->pszHeaderIfMatch);
        VMCA_SAFE_FREE_MEMORY(pRestOp->pszContentType);
        VMCA_SAFE_FREE_MEMORY(pRestOp->pszClientIP);
        VMCA_SAFE_FREE_MEMORY(pRestOp->pszHeaderConnection);
        VMCA_SAFE_FREE_MEMORY(pRestOp->pszDate);
        VMCA_SAFE_FREE_MEMORY(pRestOp->pszOrigin);

        VMCA_SAFE_FREE_MEMORY(pRestOp->pszBody);
        if (pRestOp->pjBody)
        {
            json_decref(pRestOp->pjBody);
        }
        LwRtlHashMapClear(pRestOp->pParamMap, VmSimpleHashMapPairFree, NULL);
        LwRtlFreeHashMap(&pRestOp->pParamMap);
        VMCAFreeRESTResult(pRestOp->pResult);
        VMCA_SAFE_FREE_MEMORY(pRestOp);
    }
}
