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
size_t
VmDirRESTWriteResponseCallback(
    PVOID   pMemPointer,
    size_t  responseSize,
    size_t  memorySize,
    PVOID   pContext
    );

static
DWORD
VmDirRESTFormEncodedParam(
    PSTR    pszKey,
    PSTR    pszValue,
    PSTR    *ppEncodedParam
    );

static
DWORD
VmDirRESTFormHttpURL(
    PVDIR_REST_OPERATION    pRestOp,
    PREST_REQUEST           pRequest,
    DWORD                   dwParamCount,
    PSTR                    pszLeader,
    PSTR*                   ppszURL
    );

static
DWORD
VmDirRESTWriteProxyResponse(
   VDIR_REST_CURL_RESPONSE  curlResponse,
   PREST_RESPONSE*          ppResponse,
   PVMREST_HANDLE           pRESTHandle,
   CURL*                    pCurlHandle
   );

DWORD
VmDirRESTForwardRequest(
    PVDIR_REST_OPERATION    pRestOp,
    uint32_t                dwParamCount,
    PREST_REQUEST           pRequest,
    PREST_RESPONSE*         ppResponse,
    PVMREST_HANDLE          pRESTHandle
    )
{
    DWORD                   dwError = 0;
    DWORD                   dwCurlError = 0;
    CURL*                   pCurlHandle = NULL;
    struct curl_slist*      pHeaders = NULL;
    PSTR                    pszAuthHeader = NULL;
    PSTR                    pszIfMatchHeader = 0;
    PSTR                    pszLeader = NULL;
    PSTR                    pszURL = NULL;
    VDIR_REST_CURL_RESPONSE curlResponse = {0};

    if (!pRestOp || !pRequest || !ppResponse || !pRESTHandle)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

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
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_CURLINIT_FAILED);
    }

    dwError = VmDirAllocateMemory(1, (PVOID*)&curlResponse.pResponse);
    BAIL_ON_VMDIR_ERROR(dwError);
    curlResponse.pResponse[curlResponse.dwResponseLen] = 0;

    dwCurlError = curl_easy_setopt(
                    pCurlHandle,
                    CURLOPT_PROTOCOLS,
                    CURLPROTO_HTTP);
    BAIL_ON_VMDIR_ERROR(dwCurlError);

    // Common header
    pHeaders = curl_slist_append(pHeaders, "Accept: application/json");
    if (!pHeaders)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_CURL_ERROR);
    }

    // If Authorization exists
    if (pRestOp->pszAuth && strlen(pRestOp->pszAuth) != 0)
    {
        dwError = VmDirAllocateStringPrintf(
                        &pszAuthHeader,
                        "Authorization: %s",
                        pRestOp->pszAuth);
        BAIL_ON_VMDIR_ERROR(dwError);
        pHeaders = curl_slist_append(pHeaders, pszAuthHeader);
        if (!pHeaders)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_CURL_ERROR);
        }
    }

    // If If-Match exists
    if (pRestOp->pszHeaderIfMatch && strlen(pRestOp->pszHeaderIfMatch) !=0)
    {
        dwError = VmDirAllocateStringPrintf(
                        &pszIfMatchHeader,
                        "If-Match: %s",
                        pRestOp->pszHeaderIfMatch);
        BAIL_ON_VMDIR_ERROR(dwError);
        pHeaders = curl_slist_append(pHeaders, pszIfMatchHeader);
        if (!pHeaders)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_CURL_ERROR);
        }
    }

    // Content-type header for all but GET requests
    if (VmDirStringCompareA(pRestOp->pszMethod, HTTP_METHOD_GET, FALSE) != 0)
    {
        pHeaders = curl_slist_append(pHeaders, "Content-Type: application/json");
        if (!pHeaders)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_CURL_ERROR);
        }
    }
    dwCurlError = curl_easy_setopt(
                    pCurlHandle,
                    CURLOPT_HTTPHEADER,
                    pHeaders);
    BAIL_ON_VMDIR_ERROR(dwCurlError);

    // Add the payload if exists
    if(pRestOp->pszInput && strlen(pRestOp->pszInput) != 0)
    {
        dwCurlError = curl_easy_setopt(
                        pCurlHandle,
                        CURLOPT_POSTFIELDS,
                        pRestOp->pszInput);
        BAIL_ON_VMDIR_ERROR(dwCurlError);

        dwCurlError = curl_easy_setopt(
                        pCurlHandle,
                        CURLOPT_POSTFIELDSIZE,
                        VmDirStringLenA(pRestOp->pszInput));
        BAIL_ON_VMDIR_ERROR(dwCurlError);
    }

    // set http URL
    dwError = VmDirRESTFormHttpURL(
                    pRestOp,
                    pRequest,
                    dwParamCount,
                    pszLeader,
                    &pszURL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwCurlError = curl_easy_setopt(pCurlHandle, CURLOPT_URL, pszURL);
    BAIL_ON_VMDIR_ERROR(dwCurlError);

    // set the appropiate method
    if (VmDirStringCompareA(pRestOp->pszMethod, HTTP_METHOD_GET, FALSE) == 0)
    {
        dwCurlError = curl_easy_setopt(pCurlHandle, CURLOPT_HTTPGET, 1L);
    }
    else if (VmDirStringCompareA(pRestOp->pszMethod, HTTP_METHOD_POST, FALSE) == 0)
    {
        dwCurlError = curl_easy_setopt(pCurlHandle, CURLOPT_POST, 1L);
    }
    else if (VmDirStringCompareA(pRestOp->pszMethod, HTTP_METHOD_PUT, FALSE) == 0)
    {
        dwCurlError = curl_easy_setopt(pCurlHandle, CURLOPT_PUT, 1L);
    }
    else if (VmDirStringCompareA(pRestOp->pszMethod, HTTP_METHOD_DEL, FALSE) == 0)
    {
        dwCurlError = curl_easy_setopt(pCurlHandle, CURLOPT_CUSTOMREQUEST, "DELETE");
    }
    else if (VmDirStringCompareA(pRestOp->pszMethod, HTTP_METHOD_PATCH, FALSE) == 0)
    {
        dwCurlError = curl_easy_setopt(pCurlHandle, CURLOPT_CUSTOMREQUEST, "PATCH");
    }
    BAIL_ON_VMDIR_ERROR(dwCurlError);

    // set writeback function and data type
    dwCurlError = curl_easy_setopt(
                    pCurlHandle,
                    CURLOPT_WRITEFUNCTION,
                    VmDirRESTWriteResponseCallback);
    BAIL_ON_VMDIR_ERROR(dwCurlError);

    dwCurlError = curl_easy_setopt(
                    pCurlHandle,
                    CURLOPT_WRITEDATA,
                    &curlResponse);
    BAIL_ON_VMDIR_ERROR(dwCurlError);

    // send the request to leader
    dwCurlError = curl_easy_perform(pCurlHandle);
    BAIL_ON_VMDIR_ERROR(dwCurlError);

    dwError = VmDirRESTWriteProxyResponse(
                    curlResponse,
                    ppResponse,
                    pRESTHandle,
                    pCurlHandle);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    curl_slist_free_all(pHeaders);
    curl_easy_cleanup(pCurlHandle);
    VMDIR_SAFE_FREE_STRINGA(pszURL);
    VMDIR_SAFE_FREE_STRINGA(pszLeader);
    VMDIR_SAFE_FREE_STRINGA(pszAuthHeader);
    VMDIR_SAFE_FREE_STRINGA(pszIfMatchHeader);
    VMDIR_SAFE_FREE_STRINGA(curlResponse.pResponse);
    return dwError;

error:
    if (dwCurlError)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "Curl Error: %d",
                dwCurlError);
        dwError = VMDIR_ERROR_CURL_ERROR;
    }
    goto cleanup;

}

static
DWORD
VmDirRESTFormHttpURL(
    PVDIR_REST_OPERATION    pRestOp,
    PREST_REQUEST           pRequest,
    DWORD                   dwParamCount,
    PSTR                    pszLeader,
    PSTR*                   ppszURL
    )
{
    DWORD   i = 0;
    DWORD   dwError = 0;
    DWORD   currQueryLen = 0;
    PSTR    pszKey = NULL;
    PSTR    pszValue = NULL;
    PSTR    pszQuery = NULL;
    PSTR    pszEncodedParam = NULL;

    if (!pRestOp || !pRequest || !ppszURL || !pszLeader)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    // append parameters to query
    for(i = 1; i <= dwParamCount; i++)
    {
        dwError = VmRESTGetParamsByIndex(pRequest, dwParamCount, i, &pszKey, &pszValue);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirRESTFormEncodedParam(pszKey, pszValue, &pszEncodedParam);
        BAIL_ON_VMDIR_ERROR(dwError);

        // append the param to the query
        if (!pszQuery)
        {
            dwError = VmDirAllocateStringPrintf(
                            &pszQuery,
                            "?%s",
                            pszEncodedParam);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            // readjust size of query
            dwError = VmDirReallocateMemory(
                            (PVOID)pszQuery,
                            (PVOID*)&pszQuery,
                            currQueryLen + VmDirStringLenA(pszEncodedParam) + 2); // +2 for & and \0
            BAIL_ON_VMDIR_ERROR(dwError);

            pszQuery[currQueryLen++] = '&';

            // Copy parameter
            dwError = VmDirStringCpyA(
                            &pszQuery[currQueryLen],
                            VmDirStringLenA(pszEncodedParam) + 1,
                            pszEncodedParam);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        currQueryLen = VmDirStringLenA(pszQuery);
        VMDIR_SAFE_FREE_MEMORY(pszEncodedParam);
        VMDIR_SAFE_FREE_MEMORY(pszValue);
        VMDIR_SAFE_FREE_MEMORY(pszKey);
    }

    // If parameters passed append to the URL
    if (pszQuery)
    {
        dwError = VmDirAllocateStringPrintf(
                    ppszURL,
                    "http://%s:%d%s%s",
                    pszLeader,
                    DEFAULT_REST_PORT_NUM,
                    pRestOp->pszPath,
                    pszQuery);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VmDirAllocateStringPrintf(
                    ppszURL,
                    "http://%s:%d%s",
                    pszLeader,
                    DEFAULT_REST_PORT_NUM,
                    pRestOp->pszPath);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszQuery);
    return dwError;

error:
    goto cleanup;

}

static
size_t
VmDirRESTWriteResponseCallback(
    PVOID   pMemPointer,
    size_t  responseSize,
    size_t  memorySize,
    PVOID   pContext
    )
{
    DWORD                       dwError = 0;
    size_t                      bytesRead = responseSize * memorySize;
    PVDIR_REST_CURL_RESPONSE    pCurlResponse = NULL;

    if (!pMemPointer || !pContext)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pCurlResponse = (PVDIR_REST_CURL_RESPONSE)pContext;

    dwError = VmDirReallocateMemory(
                    (PVOID)pCurlResponse->pResponse,
                    (PVOID*)&pCurlResponse->pResponse,
                    pCurlResponse->dwResponseLen + bytesRead + 1);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCopyMemory(
                    pCurlResponse->pResponse + pCurlResponse->dwResponseLen,
                    bytesRead,
                    pMemPointer,
                    bytesRead);
    BAIL_ON_VMDIR_ERROR(dwError);

    pCurlResponse->dwResponseLen += bytesRead;
    pCurlResponse->pResponse[pCurlResponse->dwResponseLen] = 0;

cleanup:
    return memorySize;

error:
    VMDIR_LOG_ERROR(
        VMDIR_LOG_MASK_ALL,
        "Proxy writeback failed!");
    memorySize = 0;
    goto cleanup;
}

static
DWORD
VmDirRESTWriteProxyResponse(
   VDIR_REST_CURL_RESPONSE  curlResponse,
   PREST_RESPONSE*          ppResponse,
   PVMREST_HANDLE           pRESTHandle,
   CURL*                    pCurlHandle
   )
{
    DWORD               dwError = 0;
    DWORD               dwCurlError = 0;
    DWORD               bytesWritten = 0;
    int64_t             statusCode = 0;
    PSTR                pszBodyLen = NULL;
    size_t              sentLen = 0;
    PVDIR_HTTP_ERROR    pHttpError = NULL;

    if (!ppResponse || !pRESTHandle || !pCurlHandle)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmRESTSetHttpStatusVersion(ppResponse, "HTTP/1.1");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwCurlError = curl_easy_getinfo(pCurlHandle, CURLINFO_RESPONSE_CODE, &statusCode);
    BAIL_ON_VMDIR_ERROR(dwCurlError);
    pHttpError = VmDirRESTGetHttpError(statusCode);

    dwError = VmRESTSetHttpStatusCode(ppResponse, pHttpError->pszHttpStatus);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmRESTSetHttpReasonPhrase(ppResponse, pHttpError->pszHttpReason);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmRESTSetHttpHeader(ppResponse, "Connection", "close");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmRESTSetHttpHeader(ppResponse, "Content-Type", "application/json");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pszBodyLen, "%ld", curlResponse.dwResponseLen);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmRESTSetDataLength(
                ppResponse,
                curlResponse.dwResponseLen > MAX_REST_PAYLOAD_LENGTH ? NULL : pszBodyLen);
    BAIL_ON_VMDIR_ERROR(dwError);

    do
    {
        size_t chunkLen = curlResponse.dwResponseLen > MAX_REST_PAYLOAD_LENGTH ?
                MAX_REST_PAYLOAD_LENGTH : curlResponse.dwResponseLen;

        dwError = VmRESTSetData(
                    pRESTHandle,
                    ppResponse,
                    VDIR_SAFE_STRING(curlResponse.pResponse) + sentLen,
                    chunkLen,
                    &bytesWritten);
        BAIL_ON_VMDIR_ERROR(dwError);

        sentLen += bytesWritten;
        curlResponse.dwResponseLen -= bytesWritten;
    }
    while (dwError == REST_ENGINE_MORE_IO_REQUIRED);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszBodyLen);
    return dwError;

error:
    if (dwCurlError)
    {
        VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "Curl Error: %d",
            dwCurlError);
        dwError = VMDIR_ERROR_CURL_ERROR;
    }
    goto cleanup;
}


static
DWORD
VmDirRESTFormEncodedParam(
    PSTR    pszKey,
    PSTR    pszValue,
    PSTR    *ppEncodedParam
    )
{
    DWORD   dwError = 0;
    DWORD   i=0;
    DWORD   j=0;
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
    goto cleanup;

}
