/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
VmDnsProcessQuery(
    PVMDNS_MESSAGE pDnsMessage,
    PBYTE *ppDnsResponse,
    PDWORD pdwDnsResponseSize,
    PCHAR pcRCode
    );

static
DWORD
VmDnsProcessUpdate(
    PVMDNS_UPDATE_MESSAGE pDnsUpdateMessage,
    PBYTE *ppDnsResponse,
    PDWORD pdwDnsResponseSize,
    PCHAR pcrCode
    );

static
DWORD
VmDnsProcessUpdateZone(
    PVMDNS_UPDATE_ZONE pDnsUpdateZone,
    PVMDNS_ZONE_OBJECT *ppZoneObject,
    PCHAR pcRCode
    );

static
DWORD
VmDnsProcessUpdatePrerequisites(
    DWORD dwPRCount,
    PVMDNS_RECORD *pPrerequisite,
    PVMDNS_UPDATE_ZONE pDnsUpdateZone,
    PVMDNS_ZONE_OBJECT pZoneObject,
    PCHAR pcRCode
    );

static
DWORD
VmDnsProcessUpdatePrescan(
    DWORD dwUPCount,
    PVMDNS_RECORD *pUpdate,
    PVMDNS_UPDATE_ZONE pDnsUpdateZone,
    PVMDNS_ZONE_OBJECT pZoneObject,
    PCHAR pcRCode
    );

static
DWORD
VmDnsProcessUpdateCommit(
    DWORD dwUPCount,
    PVMDNS_RECORD *pUpdate,
    PVMDNS_UPDATE_ZONE pDnsUpdateZone,
    PVMDNS_ZONE_OBJECT pZoneObject,
    PCHAR pcRCode
    );

static
DWORD
VmDnsBuildResponseHeader(
    PVMDNS_HEADER pRequestHeader,
    PVMDNS_HEADER pResponseHeader
    );

static
DWORD
VmDnsGetQueryResponse(
    PVMDNS_MESSAGE      pResponseMessage,
    PBYTE               *ppDnsResponse,
    PDWORD              pdwDnsResponseSize
    );

static
DWORD
VmDnsGetUpdateResponse(
    PVMDNS_UPDATE_MESSAGE   pResponseMessage,
    PBYTE                   *ppDnsResponse,
    PDWORD                  pdwDnsResponseSize
    );


DWORD
VmDnsProcessRequest(
    PBYTE pDnsRequest,
    DWORD dwDnsRequestSize,
    PBYTE *ppDnsResponse,
    PDWORD pdwDnsResponseSize,
    PUCHAR pRCode
    )
{
    DWORD dwError = 0;
    PBYTE pDnsResponse = NULL;
    DWORD dwDnsResponseSize = 0;
    PBYTE pForwarderResponse = NULL;
    DWORD dwForwarderResponseSize = 0;
    PVMDNS_MESSAGE_BUFFER pDnsMessageBuffer = NULL;
    PVMDNS_HEADER pDnsHeader = NULL;
    PVMDNS_MESSAGE pDnsMessage = NULL;
    PVMDNS_UPDATE_MESSAGE pDnsUpdateMessage = NULL;
    UCHAR rCode = 0;
    UCHAR rCodeFwder = 0;

    if (!pDnsRequest || !ppDnsResponse ||
        !pdwDnsResponseSize || !dwDnsRequestSize || !pRCode)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsGetDnsMessageBuffer(
                        pDnsRequest,
                        dwDnsRequestSize,
                        &pDnsMessageBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadDnsHeaderFromBuffer(
                        pDnsMessageBuffer,
                        &pDnsHeader
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (pDnsHeader && pDnsHeader->codes.opcode == VM_DNS_OPCODE_QUERY)
    {
        dwError = VmDnsParseQueryMessage(
                        pDnsMessageBuffer,
                        pDnsHeader,
                        &pDnsMessage
                        );
        BAIL_ON_VMDNS_ERROR(dwError);

        pDnsHeader = NULL;

        dwError = VmDnsProcessQuery(
                        pDnsMessage,
                        &pDnsResponse,
                        &dwDnsResponseSize,
                        &rCode
                        );
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else if (pDnsHeader && pDnsHeader->codes.opcode == VM_DNS_OPCODE_UPDATE)
    {
        dwError = VmDnsParseUpdateMessage(
                        pDnsMessageBuffer,
                        pDnsHeader,
                        &pDnsUpdateMessage
                        );
        BAIL_ON_VMDNS_ERROR(dwError);

        pDnsHeader = NULL;

        dwError = VmDnsProcessUpdate(
                        pDnsUpdateMessage,
                        &pDnsResponse,
                        &dwDnsResponseSize,
                        &rCode
                        );
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (rCode != 0)
    {
        dwError = VmDnsForwarderResolveRequest(
                        gpSrvContext->pForwarderContext,
                        TRUE,
                        FALSE,
                        dwDnsRequestSize,
                        pDnsRequest,
                        &dwForwarderResponseSize,
                        &pForwarderResponse,
                        &rCodeFwder
                        );

        if (dwError == 0 &&
            (dwForwarderResponseSize > 0 || pForwarderResponse))
        {
            VMDNS_SAFE_FREE_MEMORY(pDnsResponse);
            pDnsResponse = pForwarderResponse;
            dwDnsResponseSize = dwForwarderResponseSize;
            rCode = rCodeFwder;

            pForwarderResponse = NULL;
        }

        dwError = 0;
    }

cleanup:

    if (pDnsMessageBuffer)
    {
        VmDnsFreeBufferStream(pDnsMessageBuffer);
    }
    if (pDnsMessage)
    {
        VmDnsFreeDnsMessage(pDnsMessage);
    }
    if (pDnsUpdateMessage)
    {
        VmDnsFreeDnsUpdateMessage(pDnsUpdateMessage);
    }

    VMDNS_SAFE_FREE_MEMORY(pForwarderResponse);
    VMDNS_SAFE_FREE_MEMORY(pDnsHeader);

    *ppDnsResponse = pDnsResponse;
    *pdwDnsResponseSize = dwDnsResponseSize;
    *pRCode = rCode;

    return dwError;

error:

    VMDNS_SAFE_FREE_MEMORY(pDnsResponse);
    goto cleanup;
}


static
DWORD
VmDnsProcessQuery(
    PVMDNS_MESSAGE  pDnsMessage,
    PBYTE           *ppDnsResponse,
    PDWORD          pdwDnsResponseSize,
    PCHAR           pcRCode
    )
{
    DWORD dwError = 0;
    VMDNS_HEADER ResponseHeader = { 0 };
    VMDNS_MESSAGE ResponseMessage = { 0 };
    PVMDNS_ZONE_OBJECT pZoneObject = NULL;
    PVMDNS_RECORD_LIST pAnswerList = NULL;
    PBYTE pDnsResponse = NULL;
    DWORD dwDnsResponseSize = 0;

    BAIL_ON_VMDNS_INVALID_POINTER(pDnsMessage, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppDnsResponse, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pdwDnsResponseSize, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pcRCode, dwError);

    if (VMDNS_READY != VmDnsSrvGetState())
    {
        dwError = ERROR_NOT_READY;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsBuildResponseHeader(
                        pDnsMessage->pHeader,
                        &ResponseHeader
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    ResponseMessage.pHeader = &ResponseHeader;

    ResponseHeader.usQDCount = pDnsMessage->pHeader->usQDCount;

    dwError = VmDnsCopyQuestions(
                    pDnsMessage->pQuestions,
                    pDnsMessage->pHeader->usQDCount,
                    &ResponseMessage.pQuestions
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (pDnsMessage->pHeader->usQDCount != 1)
    {
        ResponseHeader.codes.RCODE = VM_DNS_RCODE_FORMAT_ERROR;
        goto response;
    }

    PVMDNS_QUESTION pQuestion = pDnsMessage->pQuestions[0];

    if (!VmDnsIsSupportedRecordType(pQuestion->uQType) &&
        !VmDnsSecIsRRTypeSec(pQuestion->uQType))
    {
        ResponseHeader.codes.RCODE = VM_DNS_RCODE_NOT_IMPLEMENTED;
        goto response;
    }

    if (pQuestion->uQType == VMDNS_RR_MTYPE_TKEY)
    {
        dwError = VmDnsSecProcessTkeyQuery(
                            pDnsMessage,
                            &ResponseMessage
                            );
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        dwError = VmDnsCacheFindZoneByQName(
                            gpSrvContext->pCacheContext,
                            pQuestion->pszQName,
                            &pZoneObject
                            );
        BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

        if (dwError == ERROR_NOT_FOUND)
        {
            ResponseHeader.codes.RCODE = VM_DNS_RCODE_NAME_ERROR;
            dwError = ERROR_SUCCESS;
            goto response;
        }

        dwError = VmDnsSrvQueryRecords(
                            pZoneObject,
                            pQuestion->pszQName,
                            pQuestion->uQType,
                            0,
                            &pAnswerList
                            );
        BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

        if (dwError == ERROR_NOT_FOUND ||
            !pAnswerList ||
            !VmDnsRecordListGetSize(pAnswerList))
        {
            ResponseHeader.codes.RCODE = VM_DNS_RCODE_NAME_ERROR;
            dwError = ERROR_SUCCESS;
            goto response;
        }

        ResponseHeader.usANCount = pAnswerList->dwCurrentSize;

        dwError = VmDnsCopyRecordArray(
                            pAnswerList,
                            &ResponseMessage.pAnswers
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        ResponseHeader.codes.RCODE = VM_DNS_RCODE_NOERROR;
    }


response:

    dwError = VmDnsGetQueryResponse(
                        &ResponseMessage,
                        &pDnsResponse,
                        &dwDnsResponseSize
                        );

    *pcRCode = ResponseHeader.codes.RCODE;

    *ppDnsResponse = pDnsResponse;
    *pdwDnsResponseSize = dwDnsResponseSize;

    VmDnsZoneObjectRelease(pZoneObject);
    VmDnsRecordListRelease(pAnswerList);

    VmDnsCleanupDnsMessage(&ResponseMessage);

    return dwError;

error:

    ResponseHeader.codes.RCODE = VM_DNS_RCODE_SERVER_FAILURE;
    goto response;
}

static
DWORD
VmDnsProcessUpdate(
    PVMDNS_UPDATE_MESSAGE   pDnsUpdateMessage,
    PBYTE                   *ppDnsResponse,
    PDWORD                  pdwDnsResponseSize,
    PCHAR                   pcRCode
    )
{
    DWORD dwError = 0;
    DWORD dwZOCount = 0;
    DWORD dwPRCount = 0;
    DWORD dwUPCount = 0;
    DWORD dwADCount = 0;
    BOOL bIsVerified = FALSE;
    VMDNS_HEADER ResponseHeader = { 0 };
    VMDNS_UPDATE_MESSAGE ResponseMessage = { 0 };
    PVMDNS_GSS_CONTEXT_HANDLE pGssSecCtx = NULL;
    PVMDNS_ZONE_OBJECT pZoneObject = NULL;
    PBYTE pDnsResponse = NULL;
    DWORD dwDnsResponseSize = 0;

    BAIL_ON_VMDNS_INVALID_POINTER(pDnsUpdateMessage, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppDnsResponse, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pdwDnsResponseSize, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pcRCode, dwError);

    if (VMDNS_READY != VmDnsSrvGetState())
    {
        dwError = ERROR_NOT_READY;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsBuildResponseHeader(
                        pDnsUpdateMessage->pHeader,
                        &ResponseHeader
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    ResponseMessage.pHeader = &ResponseHeader;

    dwZOCount = pDnsUpdateMessage->pHeader->usZOCount;
    dwPRCount = pDnsUpdateMessage->pHeader->usPRCount;
    dwUPCount = pDnsUpdateMessage->pHeader->usUPCount;
    dwADCount = pDnsUpdateMessage->pHeader->usADCount;

    ResponseHeader.usZOCount = dwZOCount;
    ResponseHeader.usPRCount = dwPRCount;
    ResponseHeader.usUPCount = dwUPCount;

    dwError = VmDnsCopyZone(
                    pDnsUpdateMessage->pZone,
                    &ResponseMessage.pZone
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyRecords(
                    pDnsUpdateMessage->pPrerequisite,
                    dwPRCount,
                    &ResponseMessage.pPrerequisite
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyRecords(
                    pDnsUpdateMessage->pUpdate,
                    dwUPCount,
                    &ResponseMessage.pUpdate
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    // Process zone section
    if (dwZOCount != 1 || !pDnsUpdateMessage->pZone)
    {
        ResponseHeader.codes.RCODE = VM_DNS_RCODE_FORMAT_ERROR;
        goto response;
    }
    else
    {
        dwError = VmDnsProcessUpdateZone(
                            pDnsUpdateMessage->pZone,
                            &pZoneObject,
                            &ResponseHeader.codes.RCODE
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        if (ResponseHeader.codes.RCODE != VM_DNS_RCODE_NOERROR)
        {
            goto response;
        }
    }

    // Process prerequisite section
    if (dwPRCount > 0 && pDnsUpdateMessage->pPrerequisite)
    {
        dwError = VmDnsProcessUpdatePrerequisites(
                                dwPRCount,
                                pDnsUpdateMessage->pPrerequisite,
                                pDnsUpdateMessage->pZone,
                                pZoneObject,
                                &ResponseHeader.codes.RCODE
                                );
        BAIL_ON_VMDNS_ERROR(dwError);

        if (ResponseHeader.codes.RCODE != VM_DNS_RCODE_NOERROR)
        {
            goto response;
        }
    }

    dwError = VmDnsSecCheckUpdatePermissions(
                        pDnsUpdateMessage,
                        &pGssSecCtx,
                        &ResponseMessage,
                        &bIsVerified
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (!bIsVerified)
    {
        goto response;
    }

    // Process update section
    if (dwUPCount > 0 && pDnsUpdateMessage->pUpdate)
    {
        dwError = VmDnsProcessUpdatePrescan(
                            dwUPCount,
                            pDnsUpdateMessage->pUpdate,
                            pDnsUpdateMessage->pZone,
                            pZoneObject,
                            &ResponseHeader.codes.RCODE
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        if (ResponseHeader.codes.RCODE != VM_DNS_RCODE_NOERROR)
        {
            goto response;
        }
        else
        {
            dwError = VmDnsProcessUpdateCommit(
                                dwUPCount,
                                pDnsUpdateMessage->pUpdate,
                                pDnsUpdateMessage->pZone,
                                pZoneObject,
                                &ResponseHeader.codes.RCODE
                                );
            BAIL_ON_VMDNS_ERROR(dwError);

            if (ResponseHeader.codes.RCODE != VM_DNS_RCODE_NOERROR)
            {
                goto response;
            }
        }
    }

    dwError = VmDnsSecSignUpdateMessage(
                            pGssSecCtx,
                            &ResponseMessage
                            );
    BAIL_ON_VMDNS_ERROR(dwError);


response:

    dwError = VmDnsGetUpdateResponse(
                        &ResponseMessage,
                        &pDnsResponse,
                        &dwDnsResponseSize
                        );

    *pcRCode = ResponseHeader.codes.RCODE;

    *ppDnsResponse = pDnsResponse;
    *pdwDnsResponseSize = dwDnsResponseSize;

    VmDnsZoneObjectRelease(pZoneObject);

    VmDnsCleanupDnsUpdateMessage(&ResponseMessage);

    return dwError;

error:

    ResponseHeader.codes.RCODE = VM_DNS_RCODE_SERVER_FAILURE;
    goto response;
}

static
DWORD
VmDnsProcessUpdateZone(
    PVMDNS_UPDATE_ZONE pDnsUpdateZone,
    PVMDNS_ZONE_OBJECT *ppZoneObject,
    PCHAR pcRCode
    )
{
    DWORD dwError = 0;
    PVMDNS_ZONE_OBJECT pZoneObject = NULL;
    UCHAR rCode = VM_DNS_RCODE_NOERROR;

    BAIL_ON_VMDNS_INVALID_POINTER(pDnsUpdateZone, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppZoneObject, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pcRCode, dwError);

    if (pDnsUpdateZone->uType != VMDNS_RR_TYPE_SOA)
    {
        rCode = VM_DNS_RCODE_FORMAT_ERROR;
        goto response;
    }

    dwError = VmDnsSrvFindZone(
                    pDnsUpdateZone->pszName,
                    &pZoneObject
                    );
    BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

    if (dwError == ERROR_NOT_FOUND)
    {
        rCode = VM_DNS_RCODE_NOTAUTH;
        dwError = ERROR_SUCCESS;
    }


response:

    if (pcRCode && ppZoneObject)
    {
        *pcRCode = rCode;
        *ppZoneObject = pZoneObject;
    }

    return dwError;

error:

    VmDnsZoneObjectRelease(pZoneObject);

    rCode = VM_DNS_RCODE_SERVER_FAILURE;
    goto response;
}

static
DWORD
VmDnsProcessUpdatePrerequisites(
    DWORD dwPRCount,
    PVMDNS_RECORD *pPrerequisiteArray,
    PVMDNS_UPDATE_ZONE pDnsUpdateZone,
    PVMDNS_ZONE_OBJECT pZoneObject,
    PCHAR pcRCode
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    DWORD dwSize = 0;
    UINT16 uRDataLength = 0;
    PVMDNS_RECORD pRecord = NULL;
    PVMDNS_RECORD_OBJECT pRecordObj = NULL;
    PVMDNS_RECORD_LIST pRecordList = NULL;
    PVMDNS_RECORD_LIST pTempRecordList = NULL;
    UCHAR rCode = VM_DNS_RCODE_NOERROR;

    BAIL_ON_VMDNS_INVALID_POINTER(pPrerequisiteArray, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pDnsUpdateZone, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pZoneObject, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pcRCode, dwError);

    dwError = VmDnsRecordListCreate(&pTempRecordList);
    BAIL_ON_VMDNS_ERROR(dwError);

    for (; dwIndex < dwPRCount; ++dwIndex)
    {
        VmDnsRecordObjectRelease(pRecordObj);
        VmDnsRecordListRelease(pRecordList);
        pRecordObj = NULL;
        pRecordList = NULL;

        pRecord = pPrerequisiteArray[dwIndex];

        if (!VmDnsIsSupportedRecordType(pRecord->dwType))
        {
            continue;
        }

        if (pRecord->dwTtl != 0)
        {
            rCode = VM_DNS_RCODE_FORMAT_ERROR;
            goto response;
        }

        if (pRecord->iClass == VMDNS_CLASS_ANY)
        {
            dwError = VmDnsGetRDataLength(
                                pRecord,
                                &uRDataLength,
                                0
                                );
            BAIL_ON_VMDNS_ERROR(dwError);

            if (uRDataLength != 0)
            {
                rCode = VM_DNS_RCODE_FORMAT_ERROR;
                goto response;
            }

            if (pRecord->dwType == VMDNS_RR_QTYPE_ANY)
            {
                dwError = VmDnsSrvQueryRecords(
                                    pZoneObject,
                                    pRecord->pszName,
                                    VMDNS_RR_QTYPE_ANY,
                                    0,
                                    &pRecordList
                                    );
                BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

                if (dwError == ERROR_NOT_FOUND ||
                    !pRecordList ||
                    !VmDnsRecordListGetSize(pRecordList))
                {
                    rCode = VM_DNS_RCODE_NXRRSET;
                    dwError = ERROR_SUCCESS;
                    goto response;
                }
            }
            else
            {
                dwError = VmDnsSrvQueryRecords(
                                    pZoneObject,
                                    pRecord->pszName,
                                    pRecord->dwType,
                                    0,
                                    &pRecordList
                                    );
                BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

                if (dwError == ERROR_NOT_FOUND ||
                    !pRecordList ||
                    !VmDnsRecordListGetSize(pRecordList))
                {
                    rCode = VM_DNS_RCODE_NXRRSET;
                    dwError = ERROR_SUCCESS;
                    goto response;
                }
            }
        }
        else if (pRecord->iClass == VMDNS_CLASS_NONE)
        {
            dwError = VmDnsGetRDataLength(
                                pRecord,
                                &uRDataLength,
                                0
                                );
            BAIL_ON_VMDNS_ERROR(dwError);

            if (uRDataLength != 0)
            {
                rCode = VM_DNS_RCODE_FORMAT_ERROR;
                goto response;
            }

            if (pRecord->dwType == VMDNS_RR_QTYPE_ANY)
            {
                dwError = VmDnsSrvQueryRecords(
                                    pZoneObject,
                                    pRecord->pszName,
                                    VMDNS_RR_QTYPE_ANY,
                                    0,
                                    &pRecordList
                                    );
                BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

                if (dwError != ERROR_NOT_FOUND &&
                    pRecordList &&
                    VmDnsRecordListGetSize(pRecordList))
                {
                    rCode = VM_DNS_RCODE_YXDOMAIN;
                    goto response;
                }
                dwError = (dwError == ERROR_NOT_FOUND) ? ERROR_SUCCESS : dwError;
            }
            else
            {
                dwError = VmDnsSrvQueryRecords(
                                    pZoneObject,
                                    pRecord->pszName,
                                    pRecord->dwType,
                                    0,
                                    &pRecordList
                                    );
                BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

                if (dwError != ERROR_NOT_FOUND &&
                    pRecordList &&
                    VmDnsRecordListGetSize(pRecordList))
                {
                    rCode = VM_DNS_RCODE_YXRRSET;
                    goto response;
                }
                dwError = (dwError == ERROR_NOT_FOUND) ? ERROR_SUCCESS : dwError;
            }
        }
        else if (pRecord->iClass == pDnsUpdateZone->uClass)
        {
            dwError = VmDnsRecordObjectCreate(
                                pRecord,
                                &pRecordObj
                                );
            BAIL_ON_VMDNS_ERROR(dwError);

            dwError = VmDnsRecordListAdd(
                                pTempRecordList,
                                pRecordObj
                                );
            BAIL_ON_VMDNS_ERROR(dwError);

            VmDnsRecordObjectRelease(pRecordObj);
            pRecordObj = NULL;
        }
        else
        {
            rCode = VM_DNS_RCODE_FORMAT_ERROR;
            goto response;
        }
    }

    dwSize = VmDnsRecordListGetSize(pTempRecordList);

    for (dwIndex = 0; dwIndex < dwSize; ++dwIndex)
    {
        VmDnsRecordObjectRelease(pRecordObj);
        VmDnsRecordListRelease(pRecordList);
        pRecordObj = NULL;
        pRecordList = NULL;

        pRecordObj = VmDnsRecordListGetRecord(pTempRecordList, dwIndex);

        dwError = VmDnsSrvQueryRecords(
                            pZoneObject,
                            pRecordObj->pRecord->pszName,
                            pRecordObj->pRecord->dwType,
                            0,
                            &pRecordList
                            );
        BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

        if (!pRecordList ||
            VmDnsRecordListGetSize(pRecordList))
        {
            rCode = VM_DNS_RCODE_NXRRSET;
            dwError = ERROR_SUCCESS;
            goto response;
        }
    }


response:

    VmDnsRecordObjectRelease(pRecordObj);
    VmDnsRecordListRelease(pRecordList);
    VmDnsRecordListRelease(pTempRecordList);

    if (pcRCode)
    {
        *pcRCode = rCode;
    }

    return dwError;

error:

    rCode = VM_DNS_RCODE_SERVER_FAILURE;
    goto response;
}

static
DWORD
VmDnsProcessUpdatePrescan(
    DWORD dwUPCount,
    PVMDNS_RECORD *pUpdate,
    PVMDNS_UPDATE_ZONE pDnsUpdateZone,
    PVMDNS_ZONE_OBJECT pZoneObject,
    PCHAR pcRCode
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    UINT16 uRDataLength = 0;
    PVMDNS_RECORD pRecord = NULL;
    PVMDNS_RECORD_LIST pRecordList = NULL;
    UCHAR rCode = VM_DNS_RCODE_NOERROR;

    BAIL_ON_VMDNS_INVALID_POINTER(pUpdate, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pDnsUpdateZone, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pZoneObject, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pcRCode, dwError);

    for (; dwIndex < dwUPCount; ++dwIndex)
    {
        VmDnsRecordListRelease(pRecordList);
        pRecordList = NULL;

        pRecord = pUpdate[dwIndex];

        if (!VmDnsIsSupportedRecordType(pRecord->dwType))
        {
            continue;
        }

        if (pRecord->iClass == pDnsUpdateZone->uClass)
        {
            if (VmDnsIsRecordQType(pRecord->dwType) &&
                VmDnsIsRecordMType(pRecord->dwType))
            {
                rCode = VM_DNS_RCODE_FORMAT_ERROR;
                goto response;
            }
        }
        else if (pRecord->iClass == VMDNS_CLASS_ANY)
        {
            dwError = VmDnsGetRDataLength(
                                pRecord,
                                &uRDataLength,
                                0
                                );
            BAIL_ON_VMDNS_ERROR(dwError);

            if ((pRecord->dwTtl != 0 && uRDataLength != 0) ||
                (VmDnsIsRecordQType(pRecord->dwType) &&
                 VmDnsIsRecordMType(pRecord->dwType)))
            {
                rCode = VM_DNS_RCODE_FORMAT_ERROR;
                goto response;
            }
        }
        else if (pRecord->iClass == VMDNS_CLASS_NONE)
        {
            if (pRecord->dwTtl != 0 ||
                (VmDnsIsRecordQType(pRecord->dwType) &&
                 VmDnsIsRecordMType(pRecord->dwType)))
            {
                rCode = VM_DNS_RCODE_FORMAT_ERROR;
                goto response;
            }
        }
        else
        {
            rCode = VM_DNS_RCODE_FORMAT_ERROR;
            goto response;
        }
    }


response:

    VmDnsRecordListRelease(pRecordList);

    if (pcRCode)
    {
        *pcRCode = rCode;
    }

    return dwError;

error:

    rCode = VM_DNS_RCODE_SERVER_FAILURE;
    goto response;
}

static
DWORD
VmDnsProcessUpdateCommit(
    DWORD dwUPCount,
    PVMDNS_RECORD *pUpdate,
    PVMDNS_UPDATE_ZONE pDnsUpdateZone,
    PVMDNS_ZONE_OBJECT pZoneObject,
    PCHAR pcRCode
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    DWORD dwIndex2 = 0;
    BOOL bLoopBreak = FALSE;
    DWORD dwSize = 0;
    PVMDNS_RECORD pRecord = NULL;
    PVMDNS_RECORD_OBJECT pRecordObj = NULL;
    PVMDNS_RECORD_LIST pRecordList = NULL;
    PVMDNS_RECORD_LIST pTempRecordList = NULL;
    UCHAR rCode = VM_DNS_RCODE_NOERROR;

    BAIL_ON_VMDNS_INVALID_POINTER(pUpdate, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pDnsUpdateZone, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pZoneObject, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pcRCode, dwError);

    for (; dwIndex < dwUPCount; ++dwIndex)
    {
        VmDnsRecordObjectRelease(pRecordObj);
        VmDnsRecordListRelease(pRecordList);
        pRecordObj = NULL;
        pRecordList = NULL;

        pRecord = pUpdate[dwIndex];

        if (!VmDnsIsSupportedRecordType(pRecord->dwType))
        {
            continue;
        }

        if (pRecord->iClass == pDnsUpdateZone->uClass)
        {
            if (pRecord->dwType == VMDNS_RR_TYPE_CNAME)
            {
                dwError = VmDnsSrvQueryRecords(
                                    pZoneObject,
                                    pRecord->pszName,
                                    VMDNS_RR_QTYPE_ANY,
                                    0,
                                    &pTempRecordList
                                    );
                BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

                if (dwError != ERROR_NOT_FOUND &&
                    pTempRecordList &&
                    VmDnsRecordListGetSize(pTempRecordList))
                {
                    dwError = VmDnsSrvGetInverseRRTypeRecordList(
                                            pTempRecordList,
                                            VMDNS_RR_TYPE_CNAME,
                                            &pRecordList
                                            );
                    BAIL_ON_VMDNS_ERROR(dwError);

                    if (pRecordList &&
                        VmDnsRecordListGetSize(pRecordList))
                    {
                        continue;
                    }
                }
                dwError = (dwError == ERROR_NOT_FOUND) ? ERROR_SUCCESS : dwError;
            }
            else
            {
                dwError = VmDnsSrvQueryRecords(
                                    pZoneObject,
                                    pRecord->pszName,
                                    VMDNS_RR_TYPE_CNAME,
                                    0,
                                    &pRecordList
                                    );
                BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

                if (dwError != ERROR_NOT_FOUND &&
                    pRecordList &&
                    VmDnsRecordListGetSize(pRecordList))
                {
                    continue;
                }
                dwError = (dwError == ERROR_NOT_FOUND) ? ERROR_SUCCESS : dwError;

                VmDnsRecordListRelease(pTempRecordList);
                VmDnsRecordListRelease(pRecordList);
                pTempRecordList = NULL;
                pRecordList = NULL;
            }

            if (pRecord->dwType == VMDNS_RR_TYPE_SOA)
            {
                dwError = VmDnsSrvQueryRecords(
                                    pZoneObject,
                                    pRecord->pszName,
                                    VMDNS_RR_TYPE_SOA,
                                    0,
                                    &pRecordList
                                    );
                BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

                if (dwError == ERROR_NOT_FOUND ||
                    !pRecordList ||
                    !VmDnsRecordListGetSize(pRecordList))
                {
                    dwError = ERROR_SUCCESS;
                    continue;
                }
                else if (pRecordList &&
                         VmDnsRecordListGetSize(pRecordList) == 1)
                {
                    pRecordObj = VmDnsRecordListGetRecord(pRecordList, 0);

                    if (pRecordObj->pRecord->Data.SOA.dwSerialNo >
                        pRecord->Data.SOA.dwSerialNo)
                    {
                        continue;
                    }
                }
                else
                {
                    rCode = VM_DNS_RCODE_SERVER_FAILURE;
                    goto response;
                }

                VmDnsRecordObjectRelease(pRecordObj);
                VmDnsRecordListRelease(pRecordList);
                pRecordObj = NULL;
                pRecordList = NULL;
            }

            dwError = VmDnsSrvQueryRecords(
                                pZoneObject,
                                pRecord->pszName,
                                pRecord->dwType,
                                0,
                                &pRecordList
                                );
            BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

            if (dwError != ERROR_NOT_FOUND &&
                pRecordList &&
                VmDnsRecordListGetSize(pRecordList))
            {
                dwSize = VmDnsRecordListGetSize(pRecordList);

                for (dwIndex2 = 0; dwIndex2 < dwSize; ++dwIndex2)
                {
                    VmDnsRecordObjectRelease(pRecordObj);
                    pRecordObj = NULL;

                    pRecordObj = VmDnsRecordListGetRecord(pRecordList, dwIndex);

                    if (pRecord->dwType == VMDNS_RR_TYPE_CNAME ||
                        pRecord->dwType == VMDNS_RR_TYPE_SOA ||
                        /* the following cases are commented out because we do not
                         * have support for wks records yet.
                         */
                        /*(
                        pRecord->dwType == VMDNS_RR_TYPE_WKS &&
                        pRecord->Data.WKS.chProtocol == pRecordObj->pRecord->Data.WKS.chProtocol &&
                        pRecord->Data.WKS.IpAddress == pRecordObj->pRecord->Data.WKS.IpAddress
                        ) ||*/
                        VmDnsCompareRecord(pRecord, pRecordObj->pRecord))
                    {
                        dwError = VmDnsSrvUpdateRecord(
                                            pZoneObject,
                                            pRecordObj->pRecord,
                                            pRecord
                                            );
                        BAIL_ON_VMDNS_ERROR(dwError);

                        bLoopBreak = TRUE;
                        break;
                    }
                }

                if (bLoopBreak)
                {
                    bLoopBreak = FALSE;
                    continue;
                }
            }
            dwError = (dwError == ERROR_NOT_FOUND) ? ERROR_SUCCESS : dwError;

            dwError = VmDnsSrvAddRecord(
                                pZoneObject,
                                pRecord
                                );
            BAIL_ON_VMDNS_ERROR(dwError);
        }
        else if (pRecord->iClass == VMDNS_CLASS_ANY)
        {
            if (pRecord->dwType == VMDNS_RR_QTYPE_ANY)
            {
                if (!VmDnsStringCompareA(pRecord->pszName,
                                            pDnsUpdateZone->pszName,
                                            FALSE))
                {
                    dwError = VmDnsSrvQueryRecords(
                                        pZoneObject,
                                        pRecord->pszName,
                                        VMDNS_RR_QTYPE_ANY,
                                        0,
                                        &pTempRecordList
                                        );
                    BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

                    if (dwError != ERROR_NOT_FOUND &&
                        pTempRecordList &&
                        VmDnsRecordListGetSize(pTempRecordList))
                    {
                        /*
                         * FIXME:
                         * The following check for ~(SOA|NS) will not work.
                         * Currently, the code will delete ~SOA first which results
                         * in losing all NS records too.  The second check for ~NS
                         * will always be hit and it will delete SOA records too.
                         * To fix, modify the VmDnsSrvGetInverseRRTypeRecordList()
                         * to return a record list that does ~(RR_TYPE | RR_TYPE)
                         * better.
                         */
                        dwError = VmDnsSrvGetInverseRRTypeRecordList(
                                                pTempRecordList,
                                                VMDNS_RR_TYPE_SOA,
                                                &pRecordList
                                                );
                        BAIL_ON_VMDNS_ERROR(dwError);

                        if (pRecordList &&
                            VmDnsRecordListGetSize(pRecordList))
                        {
                            dwError = VmDnsSrvDeleteRecords(
                                                pZoneObject,
                                                pTempRecordList
                                                );
                            BAIL_ON_VMDNS_ERROR(dwError);
                        }

                        VmDnsRecordListRelease(pRecordList);
                        pRecordList = NULL;

                        dwError = VmDnsSrvGetInverseRRTypeRecordList(
                                                pTempRecordList,
                                                VMDNS_RR_TYPE_NS,
                                                &pRecordList
                                                );
                        BAIL_ON_VMDNS_ERROR(dwError);

                        if (pRecordList &&
                            VmDnsRecordListGetSize(pRecordList))
                        {
                            dwError = VmDnsSrvDeleteRecords(
                                                pZoneObject,
                                                pTempRecordList
                                                );
                            BAIL_ON_VMDNS_ERROR(dwError);
                        }
                    }
                    dwError = (dwError == ERROR_NOT_FOUND) ? ERROR_SUCCESS : dwError;
                }
                else
                {
                    dwError = VmDnsSrvQueryRecords(
                                        pZoneObject,
                                        pRecord->pszName,
                                        VMDNS_RR_QTYPE_ANY,
                                        0,
                                        &pRecordList
                                        );
                    BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

                    if (dwError != ERROR_NOT_FOUND &&
                        pRecordList &&
                        VmDnsRecordListGetSize(pRecordList))
                    {
                        dwError = VmDnsSrvDeleteRecords(
                                            pZoneObject,
                                            pRecordList
                                            );
                        BAIL_ON_VMDNS_ERROR(dwError);
                    }
                    dwError = (dwError == ERROR_NOT_FOUND) ? ERROR_SUCCESS : dwError;
                }
            }
            else if (!VmDnsStringCompareA(pRecord->pszName,
                                            pDnsUpdateZone->pszName,
                                            FALSE) &&
                     (pRecord->dwType == VMDNS_RR_TYPE_SOA ||
                      pRecord->dwType == VMDNS_RR_TYPE_NS))
            {
                continue;
            }
            else
            {
                dwError = VmDnsSrvQueryRecords(
                                    pZoneObject,
                                    pRecord->pszName,
                                    pRecord->dwType,
                                    0,
                                    &pRecordList
                                    );
                BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

                if (dwError != ERROR_NOT_FOUND &&
                    pRecordList &&
                    VmDnsRecordListGetSize(pRecordList))
                {
                    dwError = VmDnsSrvDeleteRecords(
                                        pZoneObject,
                                        pRecordList
                                        );
                    BAIL_ON_VMDNS_ERROR(dwError);
                }
                dwError = (dwError == ERROR_NOT_FOUND) ? ERROR_SUCCESS : dwError;
            }
        }
        else if (pRecord->iClass == VMDNS_CLASS_NONE)
        {
            if (pRecord->dwType == VMDNS_RR_TYPE_SOA)
            {
                continue;
            }

            dwError = VmDnsSrvQueryRecords(
                                pZoneObject,
                                pRecord->pszName,
                                pRecord->dwType,
                                0,
                                &pRecordList
                                );
            BAIL_ON_VMDNS_ERROR_IF(dwError && dwError != ERROR_NOT_FOUND);

            if (dwError != ERROR_NOT_FOUND &&
                pRecordList &&
                VmDnsRecordListGetSize(pRecordList))
            {
                dwSize = VmDnsRecordListGetSize(pRecordList);

                if (pRecord->dwType == VMDNS_RR_TYPE_NS && dwSize == 1)
                {
                    continue;
                }

                for (dwIndex2 = 0; dwIndex2 < dwSize; ++dwIndex2)
                {
                    if (VmDnsCompareRecord(pRecord,
                          VmDnsRecordListGetRecord(pRecordList, dwIndex2)->pRecord))
                    {
                        dwError = VmDnsSrvDeleteRecord(
                                            pZoneObject,
                                            pRecord
                                            );
                        BAIL_ON_VMDNS_ERROR(dwError);
                    }
                }
            }
            dwError = (dwError == ERROR_NOT_FOUND) ? ERROR_SUCCESS : dwError;
        }
    }

    rCode = VM_DNS_RCODE_NOERROR;

response:

    VmDnsRecordObjectRelease(pRecordObj);
    VmDnsRecordListRelease(pRecordList);

    if (pcRCode)
    {
        *pcRCode = rCode;
    }
    return dwError;

error:

    rCode = VM_DNS_RCODE_SERVER_FAILURE;
    goto response;
}

static
DWORD
VmDnsBuildResponseHeader(
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
    pResponseHeader->codes.opcode = pRequestHeader->codes.opcode;

    if (pRequestHeader->codes.TC || pRequestHeader->codes.RD)
    {
        pResponseHeader->codes.RCODE = VM_DNS_RCODE_NOT_IMPLEMENTED;
        goto cleanup;
    }

    switch (pRequestHeader->codes.opcode)
    {
    case VM_DNS_OPCODE_QUERY:
        pResponseHeader->codes.AA = 1;
        pResponseHeader->codes.RCODE = VM_DNS_RCODE_NOERROR;
        break;
    case VM_DNS_OPCODE_UPDATE:
        pResponseHeader->codes.RCODE = VM_DNS_RCODE_NOERROR;
        break;
    case VM_DNS_OPCODE_IQUERY:
    case VM_DNS_OPCODE_STATUS:
    case VM_DNS_OPCODE_NOTIFY:
        pResponseHeader->codes.RCODE = VM_DNS_RCODE_NOT_IMPLEMENTED;
        break;
    default:
        pResponseHeader->codes.RCODE = VM_DNS_RCODE_FORMAT_ERROR;
        break;
    }


cleanup:

    return dwError;
}

static
DWORD
VmDnsGetQueryResponse(
    PVMDNS_MESSAGE  pResponseMessage,
    PBYTE           *ppDnsResponse,
    PDWORD          pdwDnsResponseSize
    )
{
    DWORD dwError = 0;
    DWORD dwDnsResponseSize = 0;
    PVMDNS_MESSAGE_BUFFER pDnsMessageBuffer = NULL;
    PBYTE pDnsResponse = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pResponseMessage, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppDnsResponse, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pdwDnsResponseSize, dwError);

    dwError = VmDnsAllocateBufferStream(
                        0,
                        &pDnsMessageBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSetBufferTokenizedFlag(
                        pDnsMessageBuffer,
                        TRUE
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteDnsHeaderToBuffer(
                            pResponseMessage->pHeader,
                            pDnsMessageBuffer
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteQueryMessageToBuffer(
                            pResponseMessage,
                            pDnsMessageBuffer
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

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

    *ppDnsResponse = pDnsResponse;
    *pdwDnsResponseSize = dwDnsResponseSize;


cleanup:
    VmDnsFreeBufferStream(pDnsMessageBuffer);
    return dwError;

error:
    if (ppDnsResponse)
    {
        *ppDnsResponse = NULL;
    }

    VMDNS_SAFE_FREE_MEMORY(pDnsResponse);

    goto cleanup;
}

static
DWORD
VmDnsGetUpdateResponse(
    PVMDNS_UPDATE_MESSAGE   pResponseMessage,
    PBYTE                   *ppDnsResponse,
    PDWORD                  pdwDnsResponseSize
    )
{
    DWORD dwError = 0;
    DWORD dwDnsResponseSize = 0;
    PVMDNS_MESSAGE_BUFFER pDnsMessageBuffer = NULL;
    PBYTE pDnsResponse = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pResponseMessage, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppDnsResponse, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pdwDnsResponseSize, dwError);

    dwError = VmDnsAllocateBufferStream(
                        0,
                        &pDnsMessageBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSetBufferTokenizedFlag(
                            pDnsMessageBuffer,
                            TRUE
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteDnsHeaderToBuffer(
                        pResponseMessage->pHeader,
                        pDnsMessageBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUpdateMessageToBuffer(
                            pResponseMessage,
                            pDnsMessageBuffer
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

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

    *ppDnsResponse = pDnsResponse;
    *pdwDnsResponseSize = dwDnsResponseSize;


cleanup:
    VmDnsFreeBufferStream(pDnsMessageBuffer);
    return dwError;

error:
    if (ppDnsResponse)
    {
        *ppDnsResponse = NULL;
    }

    VMDNS_SAFE_FREE_MEMORY(pDnsResponse);

    goto cleanup;
}
