/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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
VmDnsFreeBufferContext(
    PVMDNS_SOCK_BUF_CONTEXT pMessage
    );

static
DWORD
VmDnsCopyZoneInformation(
    PVMDNS_ZONE_INFO pZoneInfoIn,
    PVMDNS_ZONE_INFO *ppZoneInfoOut
    );

static
DWORD
VmDnsProcessHeader(
    PVMDNS_HEADER pRequestHeader,
    PVMDNS_HEADER pResponseHeader
    );

static
DWORD
VmDnsGetRecordNameFromQuery(
    PCSTR pszZoneName,
    PCSTR pszQueryName,
    PSTR* ppszRecordName
    );

static
DWORD
VmDnsHandleProcessRequestError(
    VMDNS_HEADER ResponseHeader,
    PBYTE *ppDnsResponse,
    PDWORD pdwDnsResponseSize
    );


DWORD
VmDnsCreateBufferContext(
    PVMDNS_SOCK_BUF_CONTEXT *ppMessage
    )
{
    DWORD dwError = 0;
    PVMDNS_SOCK_BUF_CONTEXT pMessage = NULL;

    dwError = VmDnsAllocateMemory(
                    sizeof(VMDNS_SOCK_BUF_CONTEXT),
                    (PVOID*)&pMessage
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    pMessage->refCount = 1;

    *ppMessage = pMessage;

cleanup:

    return dwError;

error:

    *ppMessage = NULL;

    if (pMessage)
    {
      VmDnsFreeBufferContext(pMessage);
    }
    goto cleanup;
}

PVMDNS_SOCK_BUF_CONTEXT
VmDnsAcquireBufferContext(
    PVMDNS_SOCK_BUF_CONTEXT pMessage
    )
{
    InterlockedIncrement(&pMessage->refCount);

    return pMessage;
}

VOID
VmDnsReleaseBufferContext(
    PVMDNS_SOCK_BUF_CONTEXT pMessage
    )
{
    if (InterlockedDecrement(&pMessage->refCount) == 0)
    {
        VmDnsFreeBufferContext(pMessage);
    }
}

DWORD
VmDnsGetAuthorityZone(
    PCSTR pszQueryName,
    PVMDNS_ZONE_INFO *ppZoneInfo
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    DWORD dwMaxMatchedIndex = ~0;
    PVMDNS_ZONE_INFO_ARRAY pZoneArray = NULL;
    PVMDNS_ZONE_INFO pZoneInfo = NULL;
    size_t szQueryNameLength = 0;
    size_t szMaxNameMatched = 0;

    if (IsNullOrEmptyString(pszQueryName) || !ppZoneInfo)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsZoneList(&pZoneArray);
    if (dwError == ERROR_NOT_FOUND)
    {
        dwError = DNS_ERROR_ZONE_DOES_NOT_EXIST;
        goto cleanup;
    }
    BAIL_ON_VMDNS_ERROR(dwError);

    szQueryNameLength = strlen(pszQueryName);

    for (; dwIndex < pZoneArray->dwCount; ++dwIndex)
    {
        VMDNS_ZONE_INFO pZoneInfoCursor =
                                pZoneArray->ZoneInfos[dwIndex];
        size_t szZoneNameLength = strlen(pZoneInfoCursor.pszName);

        size_t szStartCursor = szQueryNameLength - szZoneNameLength;

        if (szStartCursor >= 0 &&
            !VmDnsStringNCompareA(
                            &pszQueryName[szStartCursor],
                            pZoneInfoCursor.pszName,
                            szZoneNameLength,
                            FALSE)
           )
        {
            if (szZoneNameLength > szMaxNameMatched)
            {
                szMaxNameMatched = szZoneNameLength;
                dwMaxMatchedIndex = dwIndex;
            }
        }
    }

    if (szMaxNameMatched)
    {
        dwError = VmDnsCopyZoneInformation(
                          &pZoneArray->ZoneInfos[dwMaxMatchedIndex],
                          &pZoneInfo
                          );
    }
    else
    {
        dwError = DNS_ERROR_ZONE_DOES_NOT_EXIST;
    }
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppZoneInfo = pZoneInfo;

cleanup:

    VMDNS_FREE_ZONE_INFO_ARRAY(pZoneArray);
    return dwError;
error:

    if (ppZoneInfo)
    {
        *ppZoneInfo = NULL;
    }
    VMDNS_FREE_ZONE_INFO(pZoneInfo);
    goto cleanup;
}


DWORD
VmDnsProcessRequestFromCache(
    PBYTE pDnsRequest,
    DWORD dwDnsRequestSize,
    PBYTE *ppDnsResponse,
    PDWORD pdwDnsResponseSize,
    PBOOL pbFound
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    DWORD dwQDCount = 0;
    PVMDNS_MESSAGE_BUFFER pDnsMessageBuffer = NULL;
    PBYTE pDnsResponse = NULL;
    DWORD dwDnsResponseSize = 0;
    VMDNS_HEADER ResponseHeader = {0};
    PVMDNS_ZONE_INFO pZoneInfo = NULL;
    PVMDNS_RECORD_ARRAY *ppRecordsArray = NULL;
    PSTR pszRecordName = NULL;
    PVMDNS_MESSAGE pDnsMessage = NULL;

    if (
        !pDnsRequest ||
        !ppDnsResponse ||
        !pdwDnsResponseSize ||
        !dwDnsRequestSize
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsGetDnsMessage(
                          pDnsRequest,
                          dwDnsRequestSize,
                          &pDnsMessage
                          );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsProcessHeader(
                          pDnsMessage->pHeader,
                          &ResponseHeader
                          );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateBufferStream(0, &pDnsMessageBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSetBufferTokenizedFlag(pDnsMessageBuffer, TRUE);
    BAIL_ON_VMDNS_ERROR(dwError);

    if (!ResponseHeader.codes.RCODE)
    {
        dwQDCount = pDnsMessage->pHeader->usQDCount;
        ResponseHeader.usQDCount = dwQDCount;

        dwError = VmDnsAllocateMemory(
                            sizeof(PVMDNS_RECORD_ARRAY)*dwQDCount,
                            (PVOID) &ppRecordsArray
                            );
        BAIL_ON_VMDNS_ERROR(dwError);
        for (; dwIndex < dwQDCount; ++dwIndex)
        {
            PVMDNS_QUESTION pQuestion = pDnsMessage->pQuestions[dwIndex];

            VMDNS_SAFE_FREE_MEMORY(pszRecordName);

            dwError = VmDnsGetAuthorityZone(
                                pQuestion->pszQName,
                                &pZoneInfo);
            if (dwError == DNS_ERROR_ZONE_DOES_NOT_EXIST)
            {
                ResponseHeader.codes.RCODE = VM_DNS_RCODE_NAME_ERROR;
                continue;
            }
            BAIL_ON_VMDNS_ERROR(dwError);

            ResponseHeader.codes.AA = 1;

            dwError = VmDnsGetRecordNameFromQuery(
                                pZoneInfo->pszName,
                                pQuestion->pszQName,
                                &pszRecordName
                                );
            BAIL_ON_VMDNS_ERROR(dwError);

            dwError = VmDnsZoneQuery(
                                pZoneInfo->pszName,
                                pszRecordName,
                                pQuestion[dwIndex].uQType,
                                &ppRecordsArray[dwIndex]
                                );
            if (dwError)
            {
                if (dwError == ERROR_NOT_SUPPORTED)
                {
                    ResponseHeader.codes.RCODE = VM_DNS_RCODE_NOT_IMPLEMENTED;
                }

                dwError = 0;//TODO: How does DNS handle multiple questions
                //scenario where you have answer to one question but not to
                //others
            }

            if (ppRecordsArray[dwIndex] && ppRecordsArray[dwIndex]->dwCount)
            {
                VmDnsZoneRestoreRecordFQDN(
                                pZoneInfo->pszName,
                                ppRecordsArray[dwIndex]);
            }
        }

        for(dwIndex = 0; dwIndex < dwQDCount; ++dwIndex)
        {
            PVMDNS_RECORD_ARRAY pRecordArray = ppRecordsArray[dwIndex];
            if (pRecordArray)
            {
                ResponseHeader.usANCount += pRecordArray->dwCount;
            }
        }
    }

    dwError = VmDnsWriteHeaderToBuffer(
                            &ResponseHeader,
                            pDnsMessageBuffer
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    for (dwIndex = 0; dwIndex < dwQDCount; ++dwIndex)
    {
        dwError = VmDnsWriteQuestionToBuffer(
                                pDnsMessage->pQuestions[dwIndex],
                                pDnsMessageBuffer);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    for (dwIndex = 0; dwIndex<dwQDCount; ++dwIndex)
    {
        DWORD dwRecordIndex = 0;
        PVMDNS_RECORD_ARRAY pRecordArray = ppRecordsArray[dwIndex];
        if (pRecordArray)
        {
            for (;dwRecordIndex < pRecordArray->dwCount; ++dwRecordIndex)
            {
                dwError = VmDnsWriteRecordToBuffer(
                                        &pRecordArray->Records[dwRecordIndex],
                                        pDnsMessageBuffer
                                        );
                BAIL_ON_VMDNS_ERROR(dwError);
            }
        }
    }

    dwError = VmDnsCopyBufferFromBufferStream(
                            pDnsMessageBuffer,
                            NULL,
                            &dwDnsResponseSize
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateMemory(
                            dwDnsResponseSize,
                            (PVOID)&pDnsResponse
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyBufferFromBufferStream(
                            pDnsMessageBuffer,
                            pDnsResponse,
                            &dwDnsResponseSize
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    *pbFound = ResponseHeader.codes.RCODE == VM_DNS_RCODE_NOERROR;

cleanup:

    if (pDnsMessageBuffer)
    {
        VmDnsFreeBufferStream(pDnsMessageBuffer);
    }
    if (pDnsMessage)
    {
        VmDnsFreeDnsMessage(pDnsMessage);
    }
    *ppDnsResponse = pDnsResponse;
    *pdwDnsResponseSize = dwDnsResponseSize;
    return dwError;
error:

    dwError = VmDnsHandleProcessRequestError(
                            ResponseHeader,
                            &pDnsResponse,
                            &dwDnsResponseSize);
    goto cleanup;
}


DWORD
VmDnsProcessRequest(
    PBYTE       pDnsRequest,
    DWORD       dwDnsRequestSize,
    PBYTE*      ppDnsResponse,
    PDWORD      pdwDnsResponseSize
    )
{
    DWORD dwError = 0;
    BOOL bFound = FALSE;
    PBYTE pDnsResponse = NULL;
    DWORD dwDnsResponseSize = 0;
    PBYTE pForwarderResponse = NULL;
    DWORD dwForwarderResponseSize = 0;

    dwError = VmDnsProcessRequestFromCache(
                        pDnsRequest,
                        dwDnsRequestSize,
                        &pDnsResponse,
                        &dwDnsResponseSize,
                        &bFound);
    BAIL_ON_VMDNS_ERROR(dwError);

    if (!bFound)
    {
        dwError = VmDnsForwarderResolveRequest(
                        VmDnsGetForwarderContext(),
                        TRUE,
                        FALSE,
                        dwDnsRequestSize,
                        pDnsRequest,
                        &dwForwarderResponseSize,
                        &pForwarderResponse);
    }

    if (dwForwarderResponseSize > 0 &&
        pForwarderResponse)
    {
        *ppDnsResponse = pForwarderResponse;
        *pdwDnsResponseSize = dwForwarderResponseSize;
        pForwarderResponse = NULL;
    }
    else
    {
        *ppDnsResponse = pDnsResponse;
        *pdwDnsResponseSize = dwDnsResponseSize;
        pDnsResponse = NULL;
    }

cleanup:

    VMDNS_SAFE_FREE_MEMORY(pDnsResponse);
    VMDNS_SAFE_FREE_MEMORY(pForwarderResponse);

    return dwError;

error:

    if (pdwDnsResponseSize)
    {
        *pdwDnsResponseSize = 0;
    }

    if (ppDnsResponse)
    {
        *ppDnsResponse = NULL;
    }

    goto cleanup;

}

VOID
VmDnsFreeQuestion(
    PVMDNS_QUESTION pQuestion
    )
{
    if (pQuestion)
    {
        VMDNS_SAFE_FREE_MEMORY(pQuestion->pszQName);
        VMDNS_SAFE_FREE_MEMORY(pQuestion);
    }
}

VOID
VmDnsFreeQuestions(
    PVMDNS_QUESTION *pQuestions,
    DWORD dwCount
    )
{
    DWORD dwIndex = 0;
    if (pQuestions)
    {
        for (; dwIndex < dwCount; ++dwIndex)
        {
            if (pQuestions[dwIndex])
            {
                VmDnsFreeQuestion(pQuestions[dwIndex]);
            }
        }
        VMDNS_SAFE_FREE_MEMORY(pQuestions);
    }
}

VOID
VmDnsFreeRecordsArray(
    PVMDNS_RECORD *pRecords,
    DWORD dwCount
    )
{
    DWORD dwIndex = 0;
    if (pRecords)
    {
        for (; dwIndex < dwCount; ++dwIndex)
        {
            if (pRecords[dwIndex])
            {
                VmDnsClearRecord(pRecords[dwIndex]);
                VMDNS_SAFE_FREE_MEMORY(pRecords[dwIndex]);
            }
        }
        VMDNS_SAFE_FREE_MEMORY(pRecords);
    }
}

VOID
VmDnsFreeDnsMessage(
    PVMDNS_MESSAGE pVmDnsMessage
    )
{
    if (pVmDnsMessage)
    {
        if (pVmDnsMessage->pQuestions)
        {
            VmDnsFreeQuestions(
                  pVmDnsMessage->pQuestions,
                  pVmDnsMessage->pHeader->usQDCount
                  );
        }
        if (pVmDnsMessage->pRecords)
        {
            VmDnsFreeRecordsArray(
                  pVmDnsMessage->pRecords,
                  pVmDnsMessage->pHeader->usANCount
                  );
        }
        VMDNS_SAFE_FREE_MEMORY(pVmDnsMessage->pHeader);
        VMDNS_SAFE_FREE_MEMORY(pVmDnsMessage);
    }
}

static
DWORD
VmDnsCopyZoneInformation(
    PVMDNS_ZONE_INFO pZoneInfoIn,
    PVMDNS_ZONE_INFO *ppZoneInfoOut
    )
{
    DWORD dwError = 0;
    PVMDNS_ZONE_INFO pZoneInfoOut = NULL;

    dwError = VmDnsAllocateMemory(
                          sizeof(VMDNS_ZONE_INFO),
                          (PVOID)&pZoneInfoOut
                          );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (!IsNullOrEmptyString(pZoneInfoIn->pszName))
    {
        dwError = VmDnsAllocateStringA(
                          pZoneInfoIn->pszName,
                          &pZoneInfoOut->pszName);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pZoneInfoIn->pszPrimaryDnsSrvName))
    {
        dwError = VmDnsAllocateStringA(
                            pZoneInfoIn->pszPrimaryDnsSrvName,
                            &pZoneInfoOut->pszPrimaryDnsSrvName);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pZoneInfoIn->pszRName))
    {
        dwError = VmDnsAllocateStringA(
                          pZoneInfoIn->pszRName,
                          &pZoneInfoOut->pszRName);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pZoneInfoOut->serial = pZoneInfoIn->serial;
    pZoneInfoOut->refreshInterval = pZoneInfoIn->refreshInterval;
    pZoneInfoOut->retryInterval = pZoneInfoIn->retryInterval;
    pZoneInfoOut->expire = pZoneInfoIn->expire;
    pZoneInfoOut->minimum = pZoneInfoIn->minimum;
    pZoneInfoOut->dwFlags = pZoneInfoIn->dwFlags;

    *ppZoneInfoOut = pZoneInfoOut;

cleanup:

    return dwError;
error:

    *ppZoneInfoOut = NULL;
    VMDNS_FREE_ZONE_INFO(pZoneInfoOut);
    goto cleanup;
}

static
VOID
VmDnsFreeBufferContext(
    PVMDNS_SOCK_BUF_CONTEXT pMessage
    )
{
    if (pMessage->pData)
    {
        VmDnsFreeMemory(pMessage->pData);
    }
    VmDnsFreeMemory(pMessage);
}

DWORD
VmDnsGetDnsMessage(
    PBYTE pDnsRequest,
    DWORD dwDnsRequestSize,
    PVMDNS_MESSAGE *ppDnsMessage
    )
{
    DWORD dwError = 0;
    PVMDNS_MESSAGE pDnsMessage = NULL;
    PVMDNS_MESSAGE_BUFFER pDnsMessageBuffer = NULL;

    if (!pDnsRequest || !dwDnsRequestSize || !ppDnsMessage)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateBufferStreamWithBuffer(
                                          pDnsRequest,
                                          dwDnsRequestSize,
                                          0,
                                          FALSE,
                                          &pDnsMessageBuffer
                                          );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsParseMessage(
                              pDnsMessageBuffer,
                              &pDnsMessage
                              );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppDnsMessage = pDnsMessage;

cleanup:

    if (pDnsMessageBuffer)
    {
        VmDnsFreeBufferStream(pDnsMessageBuffer);
    }
    return dwError;
error:

    if (ppDnsMessage)
    {
        *ppDnsMessage = NULL;
    }
    if (pDnsMessage)
    {
        VmDnsFreeDnsMessage(pDnsMessage);
    }
    goto cleanup;
}

static
DWORD
VmDnsProcessHeader(
    PVMDNS_HEADER pRequestHeader,
    PVMDNS_HEADER pResponseHeader
    )
{
    DWORD dwError = 0;

    if (!pRequestHeader || pRequestHeader->codes.QR == VM_DNS_QUERY_OP_RESPONSE)
    {
        pResponseHeader->codes.RCODE = VM_DNS_RCODE_FORMAT_ERROR;
        goto cleanup;
    }

    pResponseHeader->usId = pRequestHeader->usId;
    pResponseHeader->codes.QR = VM_DNS_QUERY_OP_RESPONSE;

    if (pRequestHeader->codes.TC)
    {
        pResponseHeader->codes.RCODE = VM_DNS_RCODE_NOT_IMPLEMENTED;
        goto cleanup;
    }
    if (pRequestHeader->codes.RD)
    {
        pResponseHeader->codes.RCODE = VM_DNS_RCODE_REFUSED;
        goto cleanup;
    }
    switch (pRequestHeader->codes.opcode)
    {
        case 1:
        case 2:
            pResponseHeader->codes.RCODE = 4;
            break;
        default:
            break;
    }

cleanup:

    return dwError;
}

static
DWORD
VmDnsGetRecordNameFromQuery(
    PCSTR pszZoneName,
    PCSTR pszQueryName,
    PSTR* ppszRecordName
    )
{
    DWORD dwError = 0;
    size_t szZoneNameLength = 0;
    size_t szQueryNameLength = 0;
    PSTR pszTempQueryName = NULL;
    PSTR pszRecordName = NULL;

    if (!pszZoneName || !pszQueryName || !ppszRecordName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    szZoneNameLength = strlen(pszZoneName);
    szQueryNameLength = strlen(pszQueryName);

    if (szZoneNameLength > szQueryNameLength)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (
        szZoneNameLength == szQueryNameLength &&
        !VmDnsStringNCompareA(
                    pszZoneName,
                    pszQueryName,
                    szZoneNameLength,
                    FALSE
                    )
        )
    {
        dwError = VmDnsAllocateStringA(
                                    pszZoneName,
                                    &pszRecordName
                                    );
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        dwError = VmDnsAllocateMemory(
                                  szQueryNameLength - szZoneNameLength + 1,
                                  (PVOID*)&pszRecordName
                                  );
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsStringNCpyA(
                                 pszRecordName,
                                 szQueryNameLength - szZoneNameLength,
                                 pszQueryName,
                                 szQueryNameLength - szZoneNameLength - 1
                                 );
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    *ppszRecordName = pszRecordName;

cleanup:

    VMDNS_SAFE_FREE_MEMORY(pszTempQueryName);
    return dwError;
error:

    if (ppszRecordName)
    {
        *ppszRecordName = NULL;
    }
    VMDNS_SAFE_FREE_MEMORY(pszRecordName);
    goto cleanup;
}

static
DWORD
VmDnsHandleProcessRequestError(
    VMDNS_HEADER ResponseHeader,
    PBYTE *ppDnsResponse,
    PDWORD pdwDnsResponseSize
    )
{

    PVMDNS_MESSAGE_BUFFER pDnsMessageBuffer = NULL;
    DWORD dwError = 0;

    DWORD dwDnsResponseSize = 0;
    PBYTE pDnsResponse = NULL;

    if (ppDnsResponse && *ppDnsResponse)
    {
        VMDNS_SAFE_FREE_MEMORY(*ppDnsResponse);
    }

    dwError = VmDnsAllocateBufferStream(0, &pDnsMessageBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

    if (!ResponseHeader.codes.RCODE)
    {
        ResponseHeader.codes.RCODE = VM_DNS_RCODE_SERVER_FAILURE;
    }

    dwError = VmDnsWriteHeaderToBuffer(&ResponseHeader, pDnsMessageBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyBufferFromBufferStream(
                                       pDnsMessageBuffer,
                                       NULL,
                                       &dwDnsResponseSize
                                       );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateMemory(dwDnsResponseSize, (PVOID)&pDnsResponse);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyBufferFromBufferStream(
                                       pDnsMessageBuffer,
                                       pDnsResponse,
                                       &dwDnsResponseSize
                                       );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppDnsResponse = pDnsResponse;
    *pdwDnsResponseSize = dwDnsResponseSize;

cleanup:

    if (pDnsMessageBuffer)
    {
        VmDnsFreeBufferStream(pDnsMessageBuffer);
    }

    return dwError;
error:

    if (ppDnsResponse)
    {
        *ppDnsResponse = NULL;
    }
    if (pdwDnsResponseSize)
    {
        *pdwDnsResponseSize = 0;
    }
    VMDNS_SAFE_FREE_MEMORY(pDnsResponse);
    goto cleanup;
}
