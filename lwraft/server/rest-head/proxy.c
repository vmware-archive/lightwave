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
VOID
_VmDirRESTProxySetResult(
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

static
BOOL
_VmDirRESTProxyIsRetriableError(
    DWORD   dwCurlError,
    DWORD   statusCode
    );

DWORD
VmDirRESTProxyForwardRequest(
    PVDIR_REST_OPERATION    pRestOp,
    BOOLEAN                 bHttpRequest
    )
{
    DWORD                   dwError = 0;
    DWORD                   dwCurlError = 0;
    DWORD                   statusCode = 0;
    DWORD                   dwNumRetry = 0;
    CURL*                   pCurlHandle = NULL;
    PSTR                    pszURL = NULL;
    PSTR                    pszLeader = NULL;
    PSTR                    pszAuthHeader = NULL;
    PSTR                    pszIfMatchHeader = NULL;
    PSTR                    pszContentHeader = NULL;
    PSTR                    pszRequestIdHeader = NULL;
    struct curl_slist*      pHeaders = NULL;
    uint64_t                uiStartTime = 0;

    uiStartTime = VmDirGetTimeInMilliSec();

    if (!pRestOp)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    // set proxy-specifc http error mapping function
    pRestOp->pfnGetHttpError = VmDirRESTProxyGetHttpError;

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
    if (!IsNullOrEmptyString(pRestOp->pszAuth))
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
    if (!IsNullOrEmptyString(pRestOp->pszHeaderIfMatch))
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
    if (!IsNullOrEmptyString(pRestOp->pszContentType))
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
    // requestid header
    if (pRestOp->pConn->pThrLogCtx &&
        !IsNullOrEmptyString(pRestOp->pConn->pThrLogCtx->pszRequestId))
    {
        dwError = VmDirAllocateStringPrintf(
                &pszRequestIdHeader,
                "%s: %s",
                VMDIR_REST_HEADER_REQUESTID, pRestOp->pConn->pThrLogCtx->pszRequestId);
        BAIL_ON_VMDIR_ERROR(dwError);
        pHeaders = curl_slist_append(pHeaders, pszRequestIdHeader);
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
            pRestOp->pResult->pProxyResult);
    BAIL_ON_CURL_ERROR(dwCurlError);

    // Set timeout for curl request
    dwCurlError = curl_easy_setopt(
            pCurlHandle,
            CURLOPT_TIMEOUT,
            gVmdirGlobals.dwProxyCurlTimeout);
    BAIL_ON_CURL_ERROR(dwCurlError);

    while (dwNumRetry++ < VMDIR_REST_MAX_RETRY)
    {
        dwError = 0;
        statusCode = 0;
        dwCurlError = 0;

        VMDIR_SAFE_FREE_MEMORY(pszLeader);
        dwError = VmDirRaftGetLeader(&pszLeader);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (pszLeader)
        {
            VMDIR_SAFE_FREE_MEMORY(pszURL);
            // set http URL
            dwError = _VmDirRESTFormHttpURL(
                    pRestOp,
                    pszLeader,
                    bHttpRequest,
                    &pszURL);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwCurlError = curl_easy_setopt(pCurlHandle, CURLOPT_URL, pszURL);
            BAIL_ON_CURL_ERROR(dwCurlError);

            // send the request to leader
            dwCurlError = curl_easy_perform(pCurlHandle);

            // if curl error is not set then get the status
            if (!dwCurlError)
            {
                dwCurlError = curl_easy_getinfo(pCurlHandle, CURLINFO_RESPONSE_CODE, &statusCode);
                BAIL_ON_CURL_ERROR(dwCurlError);
            }

            if (!_VmDirRESTProxyIsRetriableError(dwCurlError, statusCode))
            {
                break;
            }

            VMDIR_LOG_ERROR(
                    VMDIR_LOG_MASK_ALL,
                    "Proxy request failed. Will retry from %s to leader: %s error code: %d http status code: %d",
                    VDIR_SAFE_STRING(pRestOp->pszClientIP), pszLeader, dwCurlError, statusCode);
        }
        else
        {
            dwError = VMDIR_ERROR_NO_LEADER;
        }

        VmDirSleep(VMDIR_REST_RETRY_INTERVAL_MS);
    }
    BAIL_ON_CURL_ERROR(dwCurlError);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO(
            VMDIR_LOG_MASK_ALL,
            "Proxy forwarding done for client: %s to leader: %s time taken: %d ms",
            VDIR_SAFE_STRING(pRestOp->pszClientIP),
            VDIR_SAFE_STRING(pszLeader),
            VMDIR_RESPONSE_TIME(VmDirGetTimeInMilliSec()-uiStartTime));

cleanup:
    _VmDirRESTProxySetResult(
            pRestOp, statusCode, dwError, dwCurlError);

    curl_slist_free_all(pHeaders);
    curl_easy_cleanup(pCurlHandle);

    VMDIR_SAFE_FREE_STRINGA(pszURL);
    VMDIR_SAFE_FREE_STRINGA(pszLeader);
    VMDIR_SAFE_FREE_STRINGA(pszAuthHeader);
    VMDIR_SAFE_FREE_STRINGA(pszIfMatchHeader);
    VMDIR_SAFE_FREE_STRINGA(pszContentHeader);
    VMDIR_SAFE_FREE_STRINGA(pszRequestIdHeader);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s Failed with error: %d curl error: %d, time taken %d ms for client: %s to leader: %s",
            __FUNCTION__,
            dwError,
            dwCurlError,
            VMDIR_RESPONSE_TIME(VmDirGetTimeInMilliSec()-uiStartTime),
            VDIR_SAFE_STRING(pRestOp->pszClientIP),
            VDIR_SAFE_STRING(pszLeader));
    goto cleanup;

curlerror:
    dwError = VmDirCurlToDirError(dwCurlError);
    goto error;
}

DWORD
VmDirRESTProxyResultCreate(
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

    *ppProxyResult = pProxyResult;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed , error (%d)",
            __FUNCTION__,
            dwError);

    VmDirFreeRESTProxyResult(pProxyResult);
    goto cleanup;
}

VOID
VmDirFreeRESTProxyResult(
    PVDIR_PROXY_RESULT pProxyResult
    )
{
    if (pProxyResult)
    {
        VMDIR_SAFE_FREE_MEMORY(pProxyResult->pResponse);
        VMDIR_SAFE_FREE_MEMORY(pProxyResult);
    }
}

DWORD
VmDirRESTProxyGetHttpError(
    PVDIR_REST_RESULT   pRestRslt,
    PVDIR_HTTP_ERROR*   ppHttpError
    )
{
    DWORD   dwError = 0;
    DWORD   dwHttpStatus = 0;
    PVDIR_PROXY_RESULT  pProxyResult = NULL;

    if (!pRestRslt || !ppHttpError)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pProxyResult = pRestRslt->pProxyResult;

    if (pProxyResult->statusCode)
    {
        dwHttpStatus = pProxyResult->statusCode;
    }
    else if(pProxyResult->dwCurlError)
    {
        dwHttpStatus = _VmDirRESTCurlToHttpCode(pProxyResult->dwCurlError);
    }
    else if(pProxyResult->dwError == VMDIR_ERROR_NO_LEADER)
    {
        dwHttpStatus = HTTP_SERVICE_UNAVAILABLE;
    }
    else
    {
        dwHttpStatus = HTTP_INTERNAL_SERVER_ERROR;
    }

    *ppHttpError = VmDirRESTGetHttpError(dwHttpStatus);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed , error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

/*
 * Special set result logic for proxy
 *  - take over pRestOp->pProxyResult content
 *  - set pRestOp->pResult->bErrSet = TRUE
 *  - don't set pRestOp->pResult->errCode
 *  - don't set pRestOp->pResult->pszErrMsg
 */
static
VOID
_VmDirRESTProxySetResult(
    PVDIR_REST_OPERATION    pRestOp,
    DWORD                   statusCode,
    DWORD                   dwInError,
    DWORD                   dwCurlError
    )
{
    DWORD   dwError = 0;
    PVDIR_REST_RESULT   pResult = NULL;
    PVDIR_PROXY_RESULT  pProxyResult = NULL;

    if (!pRestOp)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pResult = pRestOp->pResult;
    pProxyResult = pResult->pProxyResult;

    pProxyResult->dwError = dwInError;
    pProxyResult->dwCurlError = dwCurlError;
    pProxyResult->statusCode = statusCode;

    // take over proxy result content
    pResult->pszBody = pProxyResult->pResponse;
    pProxyResult->pResponse = NULL;

    pResult->dwBodyLen = pProxyResult->dwResponseLen;
    pProxyResult->dwResponseLen = 0;

    // this is the final result state, set bErrSet to avoid result override
    pResult->bErrSet = TRUE;

cleanup:
    return;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
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
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

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
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    VMDIR_SAFE_FREE_MEMORY(pEncodedParam);
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
        case CURLE_OK:
            httpStatus = HTTP_OK;
            break;

        case CURLE_COULDNT_RESOLVE_PROXY:
        case CURLE_COULDNT_RESOLVE_HOST:
        case CURLE_COULDNT_CONNECT:
        case CURLE_SSL_CONNECT_ERROR:
        case CURLE_SEND_ERROR:
        case CURLE_RECV_ERROR:
        case CURLE_NO_CONNECTION_AVAILABLE:
            httpStatus = HTTP_NETWORK_CONNECT_TIMEOUT_ERROR;
            break;

        case CURLE_URL_MALFORMAT:
            httpStatus = HTTP_BAD_REQUEST;
            break;

        case CURLE_OPERATION_TIMEDOUT:
        case CURLE_GOT_NOTHING:
            httpStatus = HTTP_REQUEST_TIMEOUT;
            break;

        default:
            httpStatus = HTTP_INTERNAL_SERVER_ERROR;
    }

    return httpStatus;
}

static
BOOL
_VmDirRESTProxyIsRetriableError(
    DWORD   dwCurlError,
    DWORD   statusCode
    )
{
    DWORD   httpStatus = 0;
    BOOL    bReturn = FALSE;

    if (statusCode)
    {
        httpStatus = statusCode;
    }
    else
    {
        httpStatus = _VmDirRESTCurlToHttpCode(dwCurlError);
    }

    if (httpStatus == HTTP_REQUEST_TIMEOUT || // 408
        httpStatus == HTTP_NETWORK_CONNECT_TIMEOUT_ERROR || // 599
        httpStatus == HTTP_SERVICE_UNAVAILABLE) // 503
    {
        bReturn = TRUE;
    }

    return bReturn;
}
