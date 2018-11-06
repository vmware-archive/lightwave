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

static
DWORD
_LwCARestReadURIParams(
    PLWCA_REST_OPERATION   pRestOp
    );

DWORD
LwCARestOperationCreate(
    PLWCA_REST_OPERATION*   ppRestOp
    )
{
    DWORD                   dwError = 0;
    PLWCA_REST_OPERATION    pRestOp = NULL;

    if (!ppRestOp)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_REST_OPERATION), (PVOID*)&pRestOp);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwRtlCreateHashMap(
                &pRestOp->pParamMap,
                LwRtlHashDigestPstrCaseless,
                LwRtlHashEqualPstrCaseless,
                NULL);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCARestResultCreate(&pRestOp->pResult);
    BAIL_ON_LWCA_ERROR(dwError);

    pRestOp->pResource = LwCARestGetResource(NULL);

    *ppRestOp = pRestOp;

cleanup:
    return dwError;

error:
    LWCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    LwCAFreeRESTOperation(pRestOp);
    goto cleanup;
}

DWORD
LwCARestOperationReadRequest(
    PLWCA_REST_OPERATION    pRestOp,
    PVMREST_HANDLE          pRESTHandle,
    PREST_REQUEST           pRestReq,
    DWORD                   dwParamCount
    )
{
    DWORD           dwError     = 0;
    DWORD           dwIdx       = 0;
    DWORD           bytesRead   = 0;
    PSTR            pszTmp      = NULL;
    PSTR            pszKey      = NULL;
    PSTR            pszVal      = NULL;
    PSTR            pszBody     = NULL;
    size_t          len         = 0;

    if (!pRestOp || !pRESTHandle || !pRestReq)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    // read request methods
    dwError = VmRESTGetHttpMethod(pRestReq, &pRestOp->pszMethod);
    BAIL_ON_CREST_ERROR_WITH_MSG(dwError, "Failed to get REST http request method");

    // Get the client IP
    dwError = VmRESTGetConnectionInfo(pRestReq, &pRestOp->pszClientIP, &pRestOp->dwPort);
    BAIL_ON_CREST_ERROR_WITH_MSG(dwError, "Failed to get REST connection info");

    // read raw request URI as sent from client - for token POP validation
    dwError = VmRESTGetHttpURI(pRestReq, FALSE, &pRestOp->pszURI);
    BAIL_ON_CREST_ERROR_WITH_MSG(dwError, "Failed to get REST http uri");

    // read decoded request URI and truncate parameters
    dwError = VmRESTGetHttpURI(pRestReq, TRUE, &pRestOp->pszPath);
    BAIL_ON_CREST_ERROR_WITH_MSG(dwError, "Failed to get REST http uri");

    pszTmp = LwCAStringChrA(pRestOp->pszPath, '?');
    if (pszTmp)
    {
        *pszTmp = '\0';
    }

    // determine resource
    pRestOp->pResource = LwCARestGetResource(pRestOp->pszPath);
    if (pRestOp->pResource->rscType == LWCA_REST_RSC_UNKNOWN)
    {
        dwError = LWCA_ERROR_INVALID_REQUEST;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    // extract sub-path
    if (pRestOp->pResource->bIsEndpointPrefix)
    {
        dwError = LwCAAllocateStringA(
                pRestOp->pszPath + strlen(pRestOp->pResource->pszEndpoint) + 1,
                &pRestOp->pszSubPath);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    // set resource-specific http error mapping function
    pRestOp->pfnGetHttpError = pRestOp->pResource->pfnGetHttpError;

    // read request headers
    dwError = VmRESTGetHttpHeader(pRestReq, LWCA_REST_HEADER_AUTHORIZATION, &pRestOp->pszAuth);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = VmRESTGetHttpHeader(pRestReq, LWCA_REST_HEADER_IF_MATCH, &pRestOp->pszHeaderIfMatch);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = VmRESTGetHttpHeader(pRestReq, LWCA_REST_HEADER_CONTENT_TYPE, &pRestOp->pszContentType);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = VmRESTGetHttpHeader(pRestReq, LWCA_REST_HEADER_CONNECTION, &pRestOp->pszHeaderConnection);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = VmRESTGetHttpHeader(pRestReq, LWCA_REST_HEADER_DATE, &pRestOp->pszDate);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = VmRESTGetHttpHeader(pRestReq, LWCA_REST_HEADER_ORIGIN, &pRestOp->pszOrigin);
    BAIL_ON_LWCA_ERROR(dwError);

    // read request params
    for (dwIdx = 1; dwIdx <= dwParamCount; ++dwIdx)
    {
        dwError = VmRESTGetParamsByIndex(pRestReq, dwParamCount, dwIdx, &pszKey, &pszVal);
        BAIL_ON_CREST_ERROR_WITH_MSG(dwError, "Failed to get params from REST request");

        dwError = LwRtlHashMapInsert(pRestOp->pParamMap, pszKey, pszVal, NULL);
        BAIL_ON_LWCA_ERROR(dwError);

        pszKey = NULL;
        pszVal = NULL;
    }

    // read uri params
    dwError = _LwCARestReadURIParams(pRestOp);
    BAIL_ON_LWCA_ERROR(dwError);

    // read request input json
    do
    {
        if (bytesRead || !pszBody)
        {
            dwError = LwCAReallocateMemoryWithInit(
                        (PVOID)pszBody,
                        (PVOID*)&pszBody,
                        len + LWCA_MAX_REST_PAYLOAD_LENGTH + 1,
                        len);     // +1 for NULL char
            BAIL_ON_LWCA_ERROR(dwError);
        }

        bytesRead = 0;
        dwError = VmRESTGetData(pRESTHandle, pRestReq, pszBody + len, &bytesRead);

        len += bytesRead;
    }
    while (dwError == REST_ENGINE_MORE_IO_REQUIRED);
    BAIL_ON_CREST_ERROR_WITH_MSG(dwError, "Failed to get REST request data");

    // save for signature validation
    pRestOp->pszBody = pszBody;

    LWCA_LOG_INFO(
            "Received REST request from: (%s), request type: (%s), request URI: (%s)",
            LWCA_SAFE_STRING(pRestOp->pszClientIP),
            LWCA_SAFE_STRING(pRestOp->pszMethod),
            LWCA_SAFE_STRING(pRestOp->pszPath));

cleanup:
    return dwError;

error:
    LWCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    LWCA_SAFE_FREE_MEMORY(pszBody);
    goto cleanup;
}

DWORD
LwCARestOperationParseRequestPayload(
    PLWCA_REST_OPERATION    pRestOp
    )
{
    DWORD           dwError = 0;
    json_error_t    jError  = {0};

    if (!pRestOp)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pRestOp->pszBody))
    {
        pRestOp->pjBody = json_loads(pRestOp->pszBody, 0, &jError);
        if (!pRestOp->pjBody)
        {
            LWCA_LOG_ERROR(
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

            dwError = LWCA_ERROR_INVALID_REQUEST;
            BAIL_ON_LWCA_ERROR(dwError);
        }
    }

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
LwCARestOperationProcessRequest(
    PLWCA_REST_OPERATION    pRestOp
    )
{
    DWORD                   dwError = 0;
    BOOLEAN                 bAuthenticated = FALSE;

    if (!pRestOp)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCARestAuth(pRestOp, &pRestOp->pReqCtx, &bAuthenticated);
    BAIL_ON_LWCA_ERROR(dwError);
    if (!bAuthenticated)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_REST_UNAUTHENTICATED);
    }

    if (LwCAStringCompareA(pRestOp->pszMethod, LWCA_HTTP_METHOD_OPTIONS, FALSE) != 0)
    {
        dwError = coapi_find_handler(
                    gpLwCARestApiDef,
                    pRestOp->pszPath,
                    pRestOp->pszMethod,
                    &pRestOp->pMethod);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = pRestOp->pMethod->pFnImpl((PVOID)pRestOp, NULL);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    LWCA_LOG_INFO(
            "Locally processed REST request from: %s request type: %s request URI: %s",
            LWCA_SAFE_STRING(pRestOp->pszClientIP),
            LWCA_SAFE_STRING(pRestOp->pszMethod),
            LWCA_SAFE_STRING(pRestOp->pszPath));


cleanup:

    return dwError;

error:

    LWCA_LOG_ERROR(
            "%s failed, error (%d) for client: %s",
            __FUNCTION__,
            dwError,
            LWCA_SAFE_STRING(pRestOp->pszClientIP));

    goto cleanup;
}

/*
 * Set HTTP headers as well as payload
 */
DWORD
LwCARestOperationWriteResponse(
    PLWCA_REST_OPERATION    pRestOp,
    PVMREST_HANDLE          pRESTHandle,
    PREST_RESPONSE*         ppResponse
    )
{
    DWORD               dwError         = 0;
    DWORD               bytesWritten    = 0;
    PLWCA_REST_RESULT   pResult         = NULL;
    PLWCA_REST_RESOURCE pResource       = NULL;
    PLWCA_HTTP_ERROR    pHttpError      = NULL;
    PSTR                pszBodyLen      = NULL;
    size_t              bodyLen         = 0;
    size_t              sentLen         = 0;

    if (!pRestOp || !pRESTHandle || !ppResponse)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pResult = pRestOp->pResult;
    pResource = pRestOp->pResource;

    dwError = VmRESTSetHttpStatusVersion(ppResponse, "HTTP/1.1");
    BAIL_ON_CREST_ERROR_WITH_MSG(dwError, "Failed to set REST http response status version");

    dwError = pRestOp->pfnGetHttpError(pResult, &pHttpError);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = VmRESTSetHttpStatusCode(ppResponse, pHttpError->pszHttpStatus);
    BAIL_ON_CREST_ERROR_WITH_MSG(dwError, "Failed to set REST http response status code");

    dwError = VmRESTSetHttpReasonPhrase(ppResponse, pHttpError->pszHttpReason);
    BAIL_ON_CREST_ERROR_WITH_MSG(dwError, "Failed to set REST http response reason");

    dwError = VmRESTSetHttpHeader(ppResponse, "Connection", pRestOp->pszHeaderConnection ? pRestOp->pszHeaderConnection : "close");
    BAIL_ON_CREST_ERROR_WITH_MSG(dwError, "Failed to set REST http response header 'Connection'");

    dwError = VmRESTSetHttpHeader(ppResponse, "Content-Type", pResource->pszContentType);
    BAIL_ON_CREST_ERROR_WITH_MSG(dwError, "Failed to set REST http response header 'Content-Type'");

    dwError = LwCARestResultGenerateResponseBody(pResult, pResource, pHttpError);
    BAIL_ON_LWCA_ERROR(dwError);

    bodyLen = pResult->dwBodyLen;

    dwError = LwCAAllocateStringPrintfA(&pszBodyLen, "%ld", bodyLen);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = VmRESTSetDataLength(ppResponse, bodyLen > LWCA_MAX_REST_PAYLOAD_LENGTH ? NULL : pszBodyLen);
    BAIL_ON_CREST_ERROR_WITH_MSG(dwError, "Failed to set REST http response data length");

    do
    {
        size_t chunkLen = bodyLen > LWCA_MAX_REST_PAYLOAD_LENGTH ? LWCA_MAX_REST_PAYLOAD_LENGTH : bodyLen;

        dwError = VmRESTSetData(
                    pRESTHandle,
                    ppResponse,
                    LWCA_SAFE_STRING(pResult->pszBody) + sentLen,
                    chunkLen,
                    &bytesWritten);

        sentLen += bytesWritten;
        bodyLen -= bytesWritten;
    }
    while (dwError == REST_ENGINE_MORE_IO_REQUIRED);
    BAIL_ON_CREST_ERROR_WITH_MSG(dwError, "Failed to set REST http response data");

    LWCA_LOG_INFO(
            "Responded to REST request from: (%s), request type: (%s), request URI: (%s), response status: (%d)",
            LWCA_SAFE_STRING(pRestOp->pszClientIP),
            LWCA_SAFE_STRING(pRestOp->pszMethod),
            LWCA_SAFE_STRING(pRestOp->pszPath),
            pHttpError->httpStatus);

    if (pHttpError->httpStatus != HTTP_OK)
    {
        LWCA_LOG_WARNING(
                "%s HTTP response status (%d), body (%.*s)",
                __FUNCTION__,
                pHttpError->httpStatus,
                LWCA_MIN(sentLen, LWCA_MAX_LOG_OUTPUT_LEN),
                pResult->pszBody);
    }

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszBodyLen);

    return dwError;

error:
    LWCA_LOG_ERROR(
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

VOID
LwCAFreeRESTOperation(
    PLWCA_REST_OPERATION    pRestOp
    )
{
    if (pRestOp)
    {
        LWCA_SAFE_FREE_MEMORY(pRestOp->pszAuth);
        LWCA_SAFE_FREE_MEMORY(pRestOp->pszMethod);
        LWCA_SAFE_FREE_MEMORY(pRestOp->pszURI);
        LWCA_SAFE_FREE_MEMORY(pRestOp->pszPath);
        LWCA_SAFE_FREE_MEMORY(pRestOp->pszSubPath);

        LWCA_SAFE_FREE_MEMORY(pRestOp->pszHeaderIfMatch);
        LWCA_SAFE_FREE_MEMORY(pRestOp->pszContentType);
        LWCA_SAFE_FREE_MEMORY(pRestOp->pszClientIP);
        LWCA_SAFE_FREE_MEMORY(pRestOp->pszHeaderConnection);
        LWCA_SAFE_FREE_MEMORY(pRestOp->pszDate);
        LWCA_SAFE_FREE_MEMORY(pRestOp->pszOrigin);

        LWCA_SAFE_FREE_MEMORY(pRestOp->pszBody);
        LwCAJsonCleanupObject(pRestOp->pjBody);
        LwRtlHashMapClear(pRestOp->pParamMap, VmSimpleHashMapPairFree, NULL);
        LwRtlFreeHashMap(&pRestOp->pParamMap);
        LwCAFreeRESTResult(pRestOp->pResult);

        LwCARequestContextFree(pRestOp->pReqCtx);

        LWCA_SAFE_FREE_MEMORY(pRestOp);
    }
}

static
DWORD
_LwCARestReadURIParams(
    PLWCA_REST_OPERATION   pRestOp
    )
{
    DWORD   dwError                     = 0;
    PSTR    pszKey                      = NULL;
    PSTR    pszCAId                     = NULL;
    PCSTR   pcszURI                     = NULL;
    PCSTR   pcszTempURI                 = NULL;

    // This method reads ca-id uri param

    pcszURI = strstr(pRestOp->pszURI, LWCA_REST_INTERMEDIATE_URI_PREFIX);

    if (IsNullOrEmptyString(pcszURI))
    {
        // No URI params to extract
        goto error;
    }

    pcszURI = pcszURI + LwCAStringLenA(LWCA_REST_INTERMEDIATE_URI_PREFIX);
    pcszTempURI = LwCAStringChrA(pcszURI, '/');

    if (IsNullOrEmptyString(pcszTempURI))
    {
        dwError = LwCAAllocateStringA(pcszURI, &pszCAId);
        BAIL_ON_LWCA_ERROR(dwError);
    }
    else
    {
        dwError = LwCAAllocateStringWithLengthA(pcszURI, (pcszTempURI - pcszURI), &pszCAId);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateStringA(LWCA_REST_PARAM_CA_ID, &pszKey);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwRtlHashMapInsert(pRestOp->pParamMap, pszKey, pszCAId, NULL);
    BAIL_ON_LWCA_ERROR(dwError);

    // the below values will be released when pParamMap is released
    pszKey = NULL;
    pszCAId = NULL;

error:
    LWCA_SAFE_FREE_STRINGA(pszKey);
    LWCA_SAFE_FREE_STRINGA(pszCAId);
    return dwError;
}
