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
DWORD
_VmDirRESTProxyResultGetHttpCode(
    PVDIR_PROXY_RESULT  pProxyResult,
    DWORD*              pdwHttpCode
    );

static
VOID
_VmDirSetProxyResult(
    PVDIR_REST_OPERATION    pRestOp,
    DWORD                   statusCode,
    DWORD                   dwInError,
    DWORD                   dwCurlError
    );

static
DWORD
_VmDirRESTFormHttpURL(
    PVDIR_REST_OPERATION    pRestOp,
    PSTR                    pszLeader,
    BOOLEAN                 bHttpRequest,
    PSTR*                   ppszURL
    );

static
DWORD
_VmDirRESTFormEncodedParam(
    PSTR    pszKey,
    PSTR    pszValue,
    PSTR    *ppEncodedParam
    );

static
DWORD
_VmDirRESTProxyReadRequest(
    PVDIR_REST_OPERATION    pRestOp,
    PVMREST_HANDLE          pRESTHandle,
    PREST_REQUEST           pRequest,
    DWORD                   dwParamCount
    );

static
size_t
_VmDirRESTWriteResponseCallback(
    PVOID   pMemPointer,
    size_t  responseSize,
    size_t  memorySize,
    PVOID   pContext
    );

static
DWORD
_VmDirRESTCurlToHttpCode(
        DWORD   dwCurlError
        );

DWORD
VmDirRESTForwardRequest(
    PVDIR_REST_OPERATION    pRestOp,
    uint32_t                dwParamCount,
    PREST_REQUEST           pRequest,
    PVMREST_HANDLE          pRESTHandle,
    BOOLEAN                 bHttpRequest
    )
{
    DWORD                   dwError = 0;
    DWORD                   dwCurlError = 0;
    DWORD                   statusCode = 0;
    CURL*                   pCurlHandle = NULL;
    PSTR                    pszURL = NULL;
    PSTR                    pszLeader = NULL;
    PSTR                    pszAuthHeader = NULL;
    PSTR                    pszIfMatchHeader = NULL;
    PSTR                    pszContentHeader = NULL;
    struct curl_slist*      pHeaders = NULL;
    uint64_t                uiStartTime = 0;

    uiStartTime = VmDirGetTimeInMilliSec();

    if (!pRestOp || !pRequest || !pRESTHandle)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = _VmDirRESTProxyReadRequest(
            pRestOp, pRESTHandle, pRequest, dwParamCount);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Get the leader for forwarding if leader not set no use of proceeding
    dwError = VmDirRaftGetLeader(&pszLeader);
    BAIL_ON_VMDIR_ERROR(dwError);
    if (!pszLeader)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NO_LEADER);
    }

    pCurlHandle = curl_easy_init();
    if (!pCurlHandle)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_CURL_FAILED_INIT);
    }

    if (bHttpRequest == TRUE)
    {
        dwCurlError = curl_easy_setopt(
                pCurlHandle,
                CURLOPT_PROTOCOLS,
                CURLPROTO_HTTP);
        BAIL_ON_CURL_ERROR(dwCurlError);
    }
    else
    {
        dwCurlError = curl_easy_setopt(
                pCurlHandle,
                CURLOPT_PROTOCOLS,
                CURLPROTO_HTTPS);
        BAIL_ON_CURL_ERROR(dwCurlError);

        //Skip Peer verification
        curl_easy_setopt(pCurlHandle, CURLOPT_SSL_VERIFYPEER, FALSE);
        BAIL_ON_CURL_ERROR(dwCurlError);

        //Skip Host Verification
        curl_easy_setopt(pCurlHandle, CURLOPT_SSL_VERIFYHOST, FALSE);
        BAIL_ON_CURL_ERROR(dwCurlError);
    }

    // If Authorization exists
    if (pRestOp->pszAuth && VmDirStringLenA(pRestOp->pszAuth) != 0)
    {
        dwError = VmDirAllocateStringPrintf(
                &pszAuthHeader,
                "Authorization: %s",
                pRestOp->pszAuth);
        BAIL_ON_VMDIR_ERROR(dwError);
        pHeaders = curl_slist_append(pHeaders, pszAuthHeader);
        if (!pHeaders)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_CURL_NULLSLIST);
        }
    }
    // If If-Match exists
    if (pRestOp->pszHeaderIfMatch && VmDirStringLenA(pRestOp->pszHeaderIfMatch) != 0)
    {
        dwError = VmDirAllocateStringPrintf(
                &pszIfMatchHeader,
                "If-Match: %s",
                pRestOp->pszHeaderIfMatch);
        BAIL_ON_VMDIR_ERROR(dwError);
        pHeaders = curl_slist_append(pHeaders, pszIfMatchHeader);
        if (!pHeaders)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_CURL_NULLSLIST);
        }
    }
    // Content-type header
    if (pRestOp->pszContentType && VmDirStringLenA(pRestOp->pszContentType))
    {
        dwError = VmDirAllocateStringPrintf(
                &pszContentHeader,
                "Content-Type: %s",
                pRestOp->pszContentType);
        BAIL_ON_VMDIR_ERROR(dwError);
        pHeaders = curl_slist_append(pHeaders, pszContentHeader);
        if (!pHeaders)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_CURL_NULLSLIST);
        }
    }
    dwCurlError = curl_easy_setopt(
            pCurlHandle,
            CURLOPT_HTTPHEADER,
            pHeaders);
    BAIL_ON_CURL_ERROR(dwCurlError);

    // Add the payload if exists
    if(pRestOp->pszInput && VmDirStringLenA(pRestOp->pszInput) != 0)
    {
        dwCurlError = curl_easy_setopt(
                pCurlHandle,
                CURLOPT_POSTFIELDS,
                pRestOp->pszInput);
        BAIL_ON_CURL_ERROR(dwCurlError);

        dwCurlError = curl_easy_setopt(
                pCurlHandle,
                CURLOPT_POSTFIELDSIZE,
                VmDirStringLenA(pRestOp->pszInput));
        BAIL_ON_CURL_ERROR(dwCurlError);
    }

    // set http URL
    dwError = _VmDirRESTFormHttpURL(
            pRestOp,
            pszLeader,
            bHttpRequest,
            &pszURL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwCurlError = curl_easy_setopt(pCurlHandle, CURLOPT_URL, pszURL);
    BAIL_ON_CURL_ERROR(dwCurlError);

    // set the appropiate method
    dwCurlError = curl_easy_setopt(pCurlHandle, CURLOPT_CUSTOMREQUEST, pRestOp->pszMethod);
    BAIL_ON_CURL_ERROR(dwCurlError);

    // set writeback function and data type
    dwCurlError = curl_easy_setopt(
            pCurlHandle,
            CURLOPT_WRITEFUNCTION,
            _VmDirRESTWriteResponseCallback);
    BAIL_ON_CURL_ERROR(dwCurlError);

    dwCurlError = curl_easy_setopt(
            pCurlHandle,
            CURLOPT_WRITEDATA,
            pRestOp->pProxyResult);
    BAIL_ON_CURL_ERROR(dwCurlError);

    // Set timeout for curl request
    dwCurlError = curl_easy_setopt(
            pCurlHandle,
            CURLOPT_TIMEOUT,
            gVmdirGlobals.dwProxyCurlTimeout);
    BAIL_ON_CURL_ERROR(dwCurlError);

    // send the request to leader
    dwCurlError = curl_easy_perform(pCurlHandle);
    BAIL_ON_CURL_ERROR(dwCurlError);

    VMDIR_LOG_INFO(
            VMDIR_LOG_MASK_ALL,
            "Proxy forwarding done to leader: %s time taken: %d milliseconds",
            pszLeader,
            VMDIR_RESPONSE_TIME(VmDirGetTimeInMilliSec()-uiStartTime));
    // set error
    dwCurlError = curl_easy_getinfo(pCurlHandle, CURLINFO_RESPONSE_CODE, &statusCode);
    BAIL_ON_CURL_ERROR(dwCurlError);

cleanup:
    _VmDirSetProxyResult(
            pRestOp, statusCode, dwError, dwCurlError);

    curl_slist_free_all(pHeaders);
    curl_easy_cleanup(pCurlHandle);

    VMDIR_SAFE_FREE_STRINGA(pszURL);
    VMDIR_SAFE_FREE_STRINGA(pszLeader);
    VMDIR_SAFE_FREE_STRINGA(pszAuthHeader);
    VMDIR_SAFE_FREE_STRINGA(pszIfMatchHeader);
    VMDIR_SAFE_FREE_STRINGA(pszContentHeader);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s Failed with error: %d curl error: %d, time taken %d for leader: %s",
            __FUNCTION__,
            dwError,
            dwCurlError,
            VMDIR_RESPONSE_TIME(VmDirGetTimeInMilliSec()-uiStartTime),
            pszLeader ? pszLeader : "");
    goto cleanup;

curlerror:
    dwError = VmDirCurlToDirError(dwCurlError);
    goto error;
}

DWORD
VmDirRESTWriteProxyResponse(
    PVDIR_REST_OPERATION     pRestOp,
    PREST_RESPONSE*          ppResponse,
    PVMREST_HANDLE           pRESTHandle
    )
{
    DWORD               dwError = 0;
    DWORD               bytesWritten = 0;
    DWORD               dwHttpErrorCode = 0;
    PSTR                pszBodyLen = NULL;
    PVDIR_HTTP_ERROR    pHttpError = NULL;
    size_t              sentLen = 0;

    if (!pRestOp || !ppResponse || !pRESTHandle )
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmRESTSetHttpStatusVersion(ppResponse, "HTTP/1.1");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirRESTProxyResultGetHttpCode(pRestOp->pProxyResult, &dwHttpErrorCode);
    BAIL_ON_VMDIR_ERROR(dwError);

    pHttpError = VmDirRESTGetHttpError(dwHttpErrorCode);

    dwError = VmRESTSetHttpStatusCode(ppResponse, pHttpError->pszHttpStatus);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmRESTSetHttpReasonPhrase(ppResponse, pHttpError->pszHttpReason);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmRESTSetHttpHeader(ppResponse, "Connection", "close");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmRESTSetHttpHeader(ppResponse, "Content-Type", "application/json");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pszBodyLen, "%ld", pRestOp->pProxyResult->dwResponseLen);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmRESTSetDataLength(
            ppResponse,
            pRestOp->pProxyResult->dwResponseLen > MAX_REST_PAYLOAD_LENGTH ? NULL : pszBodyLen);
    BAIL_ON_VMDIR_ERROR(dwError);

    do
    {
        size_t chunkLen = pRestOp->pProxyResult->dwResponseLen > MAX_REST_PAYLOAD_LENGTH ?
            MAX_REST_PAYLOAD_LENGTH : pRestOp->pProxyResult->dwResponseLen;

        dwError = VmRESTSetData(
                pRESTHandle,
                ppResponse,
                VDIR_SAFE_STRING(pRestOp->pProxyResult->pResponse) + sentLen,
                chunkLen,
                &bytesWritten);
        sentLen += bytesWritten;
        pRestOp->pProxyResult->dwResponseLen -= bytesWritten;
    }
    while (dwError == REST_ENGINE_MORE_IO_REQUIRED);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszBodyLen);
    return dwError;

error:
    goto cleanup;

}

DWORD
VmDirRESTCreateProxyResult(
    PVDIR_PROXY_RESULT* ppProxyResult
    )
{
    DWORD               dwError = 0;
    PVDIR_PROXY_RESULT  pProxyResult = NULL;

    if (!ppProxyResult)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_PROXY_RESULT), (PVOID*)&pProxyResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    pProxyResult->dwResponseLen = 0;
    *ppProxyResult = pProxyResult;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed , error (%d)",
            __FUNCTION__,
            dwError);

    VmDirFreeProxyResult(pProxyResult);
    goto cleanup;
}

VOID
VmDirFreeProxyResult(
    PVDIR_PROXY_RESULT pProxyResult
    )
{
    if (pProxyResult)
    {
        VMDIR_SAFE_FREE_MEMORY(pProxyResult->pResponse);
        VMDIR_SAFE_FREE_MEMORY(pProxyResult);
    }
}

static
DWORD
_VmDirRESTProxyResultGetHttpCode(
    PVDIR_PROXY_RESULT  pProxyResult,
    DWORD*              pdwHttpCode
    )
{
    DWORD dwError = 0;

    if (!pProxyResult)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (pProxyResult->statusCode)
    {
        *pdwHttpCode = pProxyResult->statusCode;
    }
    else if(pProxyResult->dwCurlError)
    {
        *pdwHttpCode = _VmDirRESTCurlToHttpCode(pProxyResult->dwCurlError);
    }
    else
    {
        *pdwHttpCode = 500;
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
VOID
_VmDirSetProxyResult(
    PVDIR_REST_OPERATION    pRestOp,
    DWORD                   statusCode,
    DWORD                   dwInError,
    DWORD                   dwCurlError
    )
{
    DWORD   dwError = 0;

    if (!pRestOp)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pRestOp->pProxyResult->dwError = dwInError;
    pRestOp->pProxyResult->dwCurlError = dwCurlError;
    pRestOp->pProxyResult->statusCode = statusCode;

cleanup:
    return;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "VmDirSetProxyResult failed : %d",
            dwError);
    goto cleanup;
}

static
DWORD
_VmDirRESTFormHttpURL(
    PVDIR_REST_OPERATION    pRestOp,
    PSTR                    pszLeader,
    BOOLEAN                 bHttpRequest,
    PSTR*                   ppszURL
    )
{
    DWORD               dwError = 0;
    DWORD               currQueryLen = 0;
    DWORD               portNumber = 0;
    PSTR                pszURL = NULL;
    PSTR                pszQuery = NULL;
    PSTR                pszEncodedParam = NULL;
    PSTR                pszRequest = NULL;
    LW_HASHMAP_ITER     iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR     pair = {NULL, NULL};

    if (!pRestOp || !ppszURL || !pszLeader)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    // append parameters to query
    while(LwRtlHashMapIterate(pRestOp->pParamMap, &iter, &pair))
    {
        dwError = _VmDirRESTFormEncodedParam((PSTR)pair.pKey, (PSTR)pair.pValue, &pszEncodedParam);
        BAIL_ON_VMDIR_ERROR(dwError);

        // readjust size of query
        dwError = VmDirReallocateMemoryWithInit(
                (PVOID)pszQuery,
                (PVOID*)&pszQuery,
                currQueryLen + VmDirStringLenA(pszEncodedParam) + 2, // +2 for & and \0
                currQueryLen);
        BAIL_ON_VMDIR_ERROR(dwError);

        // Copy parameter
        dwError = VmDirStringCpyA(
                &pszQuery[currQueryLen],
                VmDirStringLenA(pszEncodedParam) + 1,
                pszEncodedParam);
        BAIL_ON_VMDIR_ERROR(dwError);

        currQueryLen = VmDirStringLenA(pszQuery);
        pszQuery[currQueryLen++] = '&';
        VMDIR_SAFE_FREE_MEMORY(pszEncodedParam);
    }

    if(pszQuery)
    {
        // Remove the trailing &
        pszQuery[currQueryLen-1] = '\0';
    }

    if (bHttpRequest == TRUE)
    {
        portNumber = DEFAULT_HTTP_PORT_NUM;
        pszRequest = "http";
    }
    else
    {
        portNumber = DEFAULT_HTTPS_PORT_NUM;
        pszRequest = "https";
    }

    dwError = VmDirAllocateStringPrintf(
            &pszURL,
            "%s://%s:%d%s%s%s",
            pszRequest,
            pszLeader,
            portNumber,
            pRestOp->pszPath,
            pszQuery ? "?" : "",
            VDIR_SAFE_STRING(pszQuery));
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszURL = pszURL;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszQuery);
    return dwError;

error:
    VMDIR_SAFE_FREE_STRINGA(pszEncodedParam);
    VMDIR_SAFE_FREE_STRINGA(pszURL);
    goto cleanup;

}

// Would be avoided if c-rest-engine provides a way to
// get the encoded uri - 1972913
static
DWORD
_VmDirRESTFormEncodedParam(
    PSTR    pszKey,
    PSTR    pszValue,
    PSTR    *ppEncodedParam
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    DWORD   j = 0;
    PSTR    pEncodedParam = NULL;
    PSTR    pEncodedValue = NULL;
    char    currchar;

    if (!pszKey || !pszValue || !ppEncodedParam)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    // memory considering all characters are encoded
    // For example "=" is encoded as "%3D"
    // So each encoded character expands into 3 characters

    dwError = VmDirAllocateMemory(
            VmDirStringLenA(pszValue)*3 + 1, // +1 for \0
            (PVOID*)&pEncodedValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    while(pszValue[i] != '\0')
    {
        currchar = pszValue[i++];
        if ((currchar >= 'a' && currchar <= 'z') ||
                (currchar >= 'A' && currchar <= 'Z') ||
                (currchar >= '0' && currchar <= '9') ||
                (currchar >= 39 && currchar <= 42) ||
                (currchar >= 45 && currchar <= 46) ||
                (currchar == 33) ||
                (currchar == 95) ||
                (currchar == 126))
        {
            pEncodedValue[j++] = currchar;
        }
        else
        {
            pEncodedValue[j++] = '%';
            sprintf(pEncodedValue + j, "%02X", currchar);
            j += 2;
        }
    }
    dwError = VmDirAllocateStringPrintf(
            &pEncodedParam,
            "%s=%s",
            pszKey,
            pEncodedValue);
    BAIL_ON_VMDIR_ERROR(dwError);
    *ppEncodedParam = pEncodedParam;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pEncodedValue);
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pEncodedParam);
    goto cleanup;

}

static
DWORD
_VmDirRESTProxyReadRequest(
    PVDIR_REST_OPERATION    pRestOp,
    PVMREST_HANDLE          pRESTHandle,
    PREST_REQUEST           pRequest,
    DWORD                   dwParamCount
    )
{
    DWORD   dwError = 0;
    DWORD   bytesRead = 0;
    DWORD   len = 0;
    DWORD   i = 0;
    PSTR    pszInput = NULL;
    PSTR    pszTmp = NULL;
    PSTR    pszKey = NULL;
    PSTR    pszVal = NULL;

    if (!pRestOp || !pRESTHandle || !pRequest)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    // request method
    dwError = VmRESTGetHttpMethod(pRequest, &pRestOp->pszMethod);
    BAIL_ON_VMDIR_ERROR(dwError);

    // request URI
    dwError = VmRESTGetHttpURI(pRequest, &pRestOp->pszPath);
    BAIL_ON_VMDIR_ERROR(dwError);

    pszTmp = VmDirStringChrA(pRestOp->pszPath, '?');
    if (pszTmp)
    {
        *pszTmp = '\0';
    }

    // auth header
    dwError = VmRESTGetHttpHeader(pRequest, VMDIR_REST_HEADER_AUTHENTICATION, &pRestOp->pszAuth);
    BAIL_ON_VMDIR_ERROR(dwError);

    // if-match
    dwError = VmRESTGetHttpHeader(pRequest, VMDIR_REST_HEADER_IF_MATCH, &pRestOp->pszHeaderIfMatch);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Content-type
    dwError = VmRESTGetHttpHeader(pRequest, VMDIR_REST_HEADER_CONTENT_TYPE, &pRestOp->pszContentType);
    BAIL_ON_VMDIR_ERROR(dwError);

    // read request input json
    do
    {
        if (bytesRead || !pszInput)
        {
            dwError = VmDirReallocateMemoryWithInit(
                    (PVOID)pszInput,
                    (PVOID*)&pszInput,
                    len + MAX_REST_PAYLOAD_LENGTH + 1,  // +1 for NULL char
                    len);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        bytesRead = 0;
        dwError = VmRESTGetData(
                pRESTHandle, pRequest, pszInput + len, &bytesRead);

        len += bytesRead;
    }
    while (dwError == REST_ENGINE_MORE_IO_REQUIRED);
    BAIL_ON_VMDIR_ERROR(dwError);

    pRestOp->pszInput = pszInput;

    // Read request params
    for (i = 1; i <= dwParamCount; i++)
    {
        dwError = VmRESTGetParamsByIndex(pRequest, dwParamCount, i, &pszKey, &pszVal);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapInsert(pRestOp->pParamMap, pszKey, pszVal, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        pszKey = NULL;
        pszVal = NULL;
    }

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_STRINGA(pszInput);
    goto cleanup;
}

static
size_t
_VmDirRESTWriteResponseCallback(
    PVOID   pMemPointer,
    size_t  responseSize,
    size_t  memorySize,
    PVOID   pContext
    )
{
    DWORD                       dwError = 0;
    size_t                      bytesRead = responseSize * memorySize;
    PVDIR_PROXY_RESULT          pProxyResult = NULL;

    if (!pMemPointer || !pContext)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pProxyResult = (PVDIR_PROXY_RESULT)pContext;

    dwError = VmDirReallocateMemoryWithInit(
            (PVOID)pProxyResult->pResponse,
            (PVOID*)&pProxyResult->pResponse,
            pProxyResult->dwResponseLen + bytesRead + 1,
            pProxyResult->dwResponseLen);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCopyMemory(
            (PVOID)&pProxyResult->pResponse[pProxyResult->dwResponseLen],
            bytesRead,
            pMemPointer,
            bytesRead);
    BAIL_ON_VMDIR_ERROR(dwError);

    pProxyResult->dwResponseLen += bytesRead;

cleanup:
    return bytesRead;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "Proxy writeback failed with error: %d!",
            dwError);
    bytesRead = 0;
    goto cleanup;

}

static
DWORD
_VmDirRESTCurlToHttpCode(
    DWORD   dwCurlError
    )
{
    DWORD httpStatus = 0;

    switch(dwCurlError)
    {
        case CURLE_COULDNT_RESOLVE_PROXY:
        case CURLE_COULDNT_RESOLVE_HOST:
        case CURLE_COULDNT_CONNECT:
            httpStatus = HTTP_NETWORK_CONNECT_TIMEOUT_ERROR;
            break;

        case CURLE_URL_MALFORMAT:
            httpStatus = HTTP_BAD_REQUEST;
            break;

        case CURLE_OPERATION_TIMEDOUT:
            httpStatus = HTTP_REQUEST_TIMEOUT;
            break;

        default:
            httpStatus = HTTP_INTERNAL_SERVER_ERROR;
    }

    return httpStatus;
}
