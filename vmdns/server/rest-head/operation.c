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
VmDnsRESTOperationCreate(
    PVDNS_REST_OPERATION*   ppRestOp
    )
{
    DWORD   dwError = 0;
    PVDNS_REST_OPERATION    pRestOp = NULL;

    if (!ppRestOp)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(
                    sizeof(VDNS_REST_OPERATION),
                    (PVOID*)&pRestOp
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRESTResultCreate(&pRestOp->pResult);
    BAIL_ON_VMDNS_ERROR(dwError);

    pRestOp->pResource = VmDnsRESTGetResource(NULL);

    *ppRestOp = pRestOp;

cleanup:

    return dwError;

error:

    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed, error (%d)", __FUNCTION__, dwError);
    if (ppRestOp)
    {
       *ppRestOp = NULL;
    }
    VmDnsFreeRESTOperation(pRestOp);
    goto cleanup;
}

DWORD
VmDnsRESTOperationReadRequest(
    PVDNS_REST_OPERATION    pRestOp,
    PVMREST_HANDLE          pRESTHandle,
    PREST_REQUEST           pRestReq
    )
{
    DWORD   dwError = 0;
    DWORD   bytesRead = 0;
    json_error_t    jError = {0};
    PSTR    pszTmp = NULL;
    PSTR    pszInput = NULL;
    size_t  len = 0;

    if (!pRestOp || !pRESTHandle || !pRestReq)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    // read request methods
    dwError = VmRESTGetHttpMethod(pRestReq, &pRestOp->pszMethod);
    BAIL_ON_VMDNS_ERROR(dwError);

    // read request URI
    dwError = VmRESTGetHttpURI(pRestReq, &pRestOp->pszPath);
    BAIL_ON_VMDNS_ERROR(dwError);

    pszTmp = VmDnsStringChrA(pRestOp->pszPath, '?');
    if (pszTmp)
    {
        *pszTmp = '\0';
    }

    // determine resource
    pRestOp->pResource = VmDnsRESTGetResource(pRestOp->pszPath);
    if (pRestOp->pResource->rscType == VDNS_REST_RSC_UNKNOWN)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    // extract sub-path
    if (pRestOp->pResource->bIsEndpointPrefix)
    {
        dwError = VmDnsAllocateStringA(
                pRestOp->pszPath + strlen(pRestOp->pResource->pszEndpoint) + 1,
                &pRestOp->pszSubPath
                );
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    // read request authorization info
    dwError = VmRESTGetHttpHeader(pRestReq, VMDNS_REST_HEADER_AUTHENTICATION, &pRestOp->pszAuth);
    BAIL_ON_VMDNS_ERROR(dwError);

    // read request input json
    do
    {
        if (bytesRead || !pszInput)
        {
            dwError = VmDnsReallocateMemory(
                    (PVOID)pszInput,
                    (PVOID*)&pszInput,
                    len + MAX_REST_PAYLOAD_LENGTH + 1       // +1 for NULL char
                    );
            BAIL_ON_VMDNS_ERROR(dwError);
        }

        bytesRead = 0;
        dwError = VmRESTGetData(
                pRESTHandle,
                pRestReq,
                pszInput + len,
                &bytesRead
                );

        len += bytesRead;
    }
    while (dwError == REST_ENGINE_MORE_IO_REQUIRED);
    BAIL_ON_VMDNS_ERROR(dwError);
    pszInput[len] = 0;

    if (!IsNullOrEmptyString(pszInput))
    {
        pRestOp->pjInput = json_loads(pszInput, 0, &jError);
        if (!pRestOp->pjInput)
        {
            VmDnsLog(VMDNS_LOG_LEVEL_ERROR,
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

            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }

cleanup:
    VMDNS_SAFE_FREE_MEMORY(pszInput);
    return dwError;

error:
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

/*
 * Set HTTP headers as well as payload
 */
DWORD
VmDnsRESTOperationWriteResponse(
    PVDNS_REST_OPERATION    pRestOp,
    PVMREST_HANDLE          pRESTHandle,
    PREST_RESPONSE*         ppResponse
    )
{
    DWORD   dwError = 0;
    DWORD   bytesWritten = 0;
    PSTR    pszHttpStatus = NULL;
    PSTR    pszHttpReason = NULL;
    PSTR    pszBody = NULL;
    PSTR    pszBodyLen = NULL;
    size_t  bodyLen = 0;
    size_t  sentLen = 0;

    if (!pRestOp || !pRESTHandle || !ppResponse)
    {
        dwError  = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmRESTSetHttpStatusVersion(ppResponse, "HTTP/1.1");
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = pRestOp->pResource->pfnGetHttpError(
                       pRestOp->pResult,
                       &pszHttpStatus,
                       &pszHttpReason
                       );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmRESTSetHttpStatusCode(ppResponse, pszHttpStatus);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmRESTSetHttpReasonPhrase(ppResponse, pszHttpReason);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmRESTSetHttpHeader(ppResponse, "Connection", "close");
    BAIL_ON_VMDNS_ERROR(dwError);


    if (pRestOp->pResult->pszData)
    {
       dwError = VmRESTSetHttpHeader(ppResponse, "Content-Type", "text/plain");
       BAIL_ON_VMDNS_ERROR(dwError);

       bodyLen = pRestOp->pResult->dwDataLen;

       dwError = VmDnsAllocateMemory(
                    bodyLen + 1,
                    (PVOID*)&pszBody);
       BAIL_ON_VMDNS_ERROR(dwError);

       dwError = VmDnsCopyMemory(
                    (PVOID)pszBody,
                    bodyLen + 1,
                    (PVOID)pRestOp->pResult->pszData,
                    bodyLen
                    );
       BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
       dwError = VmRESTSetHttpHeader(ppResponse, "Content-Type", "application/json");
       BAIL_ON_VMDNS_ERROR(dwError);
       dwError = VmDnsRESTResultToResponseBody(
                                       pRestOp->pResult,
                                       pRestOp->pResource,
                                       &pszBody
                                       );
       BAIL_ON_VMDNS_ERROR(dwError);
       bodyLen = VmDnsStringLenA(VDNS_SAFE_STRING(pszBody));
    }

    dwError = VmDnsAllocateStringPrintfA(&pszBodyLen, "%ld", bodyLen);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmRESTSetDataLength(
                       ppResponse,
                       bodyLen > MAX_REST_PAYLOAD_LENGTH ? NULL : pszBodyLen
                       );
    BAIL_ON_VMDNS_ERROR(dwError);

    do
    {
        size_t chunkLen = bodyLen > MAX_REST_PAYLOAD_LENGTH ?
                MAX_REST_PAYLOAD_LENGTH : bodyLen;

        dwError = VmRESTSetData(
                pRESTHandle,
                ppResponse,
                VDNS_SAFE_STRING(pszBody) + sentLen,
                chunkLen,
                &bytesWritten
                );

        sentLen += bytesWritten;
        bodyLen -= bytesWritten;
    }
    while (dwError == REST_ENGINE_MORE_IO_REQUIRED);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    VMDNS_SAFE_FREE_STRINGA(pszBody);
    VMDNS_SAFE_FREE_STRINGA(pszBodyLen);
    return dwError;

error:
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

VOID
VmDnsFreeRESTOperation(
    PVDNS_REST_OPERATION    pRestOp
    )
{
    if (pRestOp)
    {
        VMDNS_SAFE_FREE_MEMORY(pRestOp->pszAuth);
        VMDNS_SAFE_FREE_MEMORY(pRestOp->pszMethod);
        VMDNS_SAFE_FREE_MEMORY(pRestOp->pszPath);
        VMDNS_SAFE_FREE_MEMORY(pRestOp->pszSubPath);
        VMDNS_SAFE_FREE_MEMORY(pRestOp->pszHeaderIfMatch);
        if (pRestOp->pjInput)
        {
            json_decref(pRestOp->pjInput);
        }
        VmDnsFreeRESTResult(pRestOp->pResult);
        VMDNS_SAFE_FREE_MEMORY(pRestOp);
    }
}


