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

BOOLEAN
VmDnsCompareRecord(
    PVMDNS_RECORD       pRecord1,
    PVMDNS_RECORD       pRecord2
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;
    dwError = VmDnsFindRecordMethods(pRecord1, &idx);
    if(!dwError)
    {
        return gRecordMethods[idx].pfnCompare(pRecord1, pRecord2);
    }

    return dwError;
}

BOOLEAN
VmDnsMatchRecord(
    PVMDNS_RECORD   pRecord,
    PVMDNS_RECORD   pTemplate
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;
    dwError = VmDnsFindRecordMethods(pRecord, &idx);
    if(!dwError)
    {
        return gRecordMethods[idx].pfnMatch(pRecord, pTemplate);
    }

    return dwError;
}

VOID
VmDnsClearRecord(
    PVMDNS_RECORD       pRecord
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;
    if (pRecord)
    {
        dwError = VmDnsFindRecordMethods(pRecord, &idx);
        if(!dwError)
        {
            gRecordMethods[idx].pfnClear(pRecord);
        }
        else
        {
            VMDNS_SAFE_FREE_MEMORY(pRecord->pszName);
        }
    }
}

VOID
VmDnsRpcClearRecord(
    PVMDNS_RECORD   pRecord
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;
    if (pRecord)
    {
        dwError = VmDnsFindRecordMethods(pRecord, &idx);
        if(!dwError)
        {
            gRecordMethods[idx].pfnRpcClear(pRecord);
        }
        else
        {
            VmDnsRpcFreeMemory(pRecord->pszName);
        }
    }
}

BOOLEAN
VmDnsValidateRecord(
    PVMDNS_RECORD   pRecord
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;
    if (pRecord)
    {
        dwError = VmDnsFindRecordMethods(pRecord, &idx);
        if(!dwError)
        {
            return gRecordMethods[idx].pfnValidate(pRecord);
        }
    }

    VMDNS_LOG_DEBUG("%s validation of record failed.", __FUNCTION__);

    return FALSE;
}

DWORD
VmDnsDuplicateRecord(
    PVMDNS_RECORD   pRecord,
    PVMDNS_RECORD   *ppDest
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;

    BAIL_ON_VMDNS_INVALID_POINTER(pRecord, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppDest, dwError);

    dwError = VmDnsFindRecordMethods(pRecord, &idx);
    if(!dwError)
    {
        dwError = gRecordMethods[idx].pfnDuplicate(pRecord, ppDest);
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsRpcDuplicateRecord(
    PVMDNS_RECORD   pRecord,
    PVMDNS_RECORD   *ppDest
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;

    BAIL_ON_VMDNS_INVALID_POINTER(pRecord, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppDest, dwError);

    dwError = VmDnsFindRecordMethods(pRecord, &idx);
    if(!dwError)
    {
        dwError = gRecordMethods[idx].pfnRpcDuplicate(pRecord, ppDest);
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsCopyRecord(
    PVMDNS_RECORD   pRecord,
    PVMDNS_RECORD   pDest
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;

    BAIL_ON_VMDNS_INVALID_POINTER(pRecord, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pDest, dwError);

    dwError = VmDnsFindRecordMethods(pRecord, &idx);
    if(!dwError)
    {
        dwError = gRecordMethods[idx].pfnCopy(pRecord, pDest);
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsRpcCopyRecord(
    PVMDNS_RECORD   pRecord,
    PVMDNS_RECORD   pDest
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;

    BAIL_ON_VMDNS_INVALID_POINTER(pRecord, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pDest, dwError);

    dwError = VmDnsFindRecordMethods(pRecord, &idx);
    if(!dwError)
    {
        dwError = gRecordMethods[idx].pfnRpcCopy(pRecord, pDest);
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsRecordToString(
    PVMDNS_RECORD   pRecord,
    PSTR*           ppStr
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;

    BAIL_ON_VMDNS_INVALID_POINTER(pRecord, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppStr, dwError);

    dwError = VmDnsFindRecordMethods(pRecord, &idx);
    if(!dwError)
    {
        dwError = gRecordMethods[idx].pfnToString(pRecord, ppStr);
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsRecordGetCN(
    PVMDNS_RECORD   pRecord,
    PSTR*           ppStr
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_SUCCESS;

    BAIL_ON_VMDNS_INVALID_POINTER(pRecord, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppStr, dwError);

    dwError = VmDnsFindRecordMethods(pRecord, &idx);
    if(!dwError)
    {
        dwError = gRecordMethods[idx].pfnGetCN(pRecord, ppStr);
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

VOID
VmDnsClearRecordArray(
    PVMDNS_RECORD_ARRAY pRecordArray
    )
{
    int i = 0;
    if (pRecordArray)
    {
        for (; i < pRecordArray->dwCount; ++i)
        {
            VmDnsClearRecord(&(pRecordArray->Records[i]));
        }
    }
}

DWORD
VmDnsCreateSoaRecord(
    PVMDNS_ZONE_INFO    pZoneInfo,
    PVMDNS_RECORD*      ppRecord
    )
{
    DWORD           dwError = 0;
    PVMDNS_RECORD   pRecord = NULL;
    PSTR            pszName = NULL;
    PSTR            pszPrimaryDnsName = NULL;
    PSTR            pszRName = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pZoneInfo, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppRecord, dwError);

    dwError = VmDnsAllocateMemory(sizeof(VMDNS_RECORD), (void**)&pRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

    pRecord->dwType = VMDNS_RR_TYPE_SOA;

    dwError = VmDnsAllocateStringA(pZoneInfo->pszName, &pszName);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringA(pZoneInfo->pszPrimaryDnsSrvName,
                                    &pszPrimaryDnsName);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringA(pZoneInfo->pszRName, &pszRName);
    BAIL_ON_VMDNS_ERROR(dwError);

    pRecord->pszName = pszName;
    pRecord->iClass = VMDNS_CLASS_IN;
    pRecord->Data.SOA.pNameAdministrator = pszRName;
    pRecord->Data.SOA.pNamePrimaryServer = pszPrimaryDnsName;
    pRecord->Data.SOA.dwDefaultTtl = pZoneInfo->minimum;
    pRecord->Data.SOA.dwExpire = pZoneInfo->expire;
    pRecord->Data.SOA.dwRefresh = pZoneInfo->refreshInterval;
    pRecord->Data.SOA.dwRetry = pZoneInfo->retryInterval;
    pRecord->Data.SOA.dwSerialNo = pZoneInfo->serial;
    pszName = NULL;
    pszRName = NULL;

    *ppRecord = pRecord;

cleanup:

    return dwError;

error:
    VmDnsFreeMemory(pszName);
    VmDnsFreeMemory(pszRName);
    VMDNS_FREE_RECORD(pRecord);
    goto cleanup;
}

DWORD
VmDnsWriteRecordToBuffer(
    PVMDNS_RECORD pDnsRecord,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;

    if (!pDnsRecord || !pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsWriteDomainNameToBuffer(
                            pDnsRecord->pszName,
                            pVmDnsBuffer,
                            pVmDnsBuffer->bTokenizeDomainName
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT16ToBuffer(
                        pDnsRecord->dwType,
                        pVmDnsBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT16ToBuffer(
                        pDnsRecord->iClass,
                        pVmDnsBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT32ToBuffer(
                        pDnsRecord->dwTtl,
                        pVmDnsBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    for (; dwIndex < gRecordMethodMapSize; ++dwIndex)
    {
        if (pDnsRecord->dwType == gRecordMethods[dwIndex].type)
        {
            dwError = gRecordMethods[dwIndex].pfnSerialize(
                                                    pDnsRecord->Data,
                                                    pVmDnsBuffer
                                                    );
            BAIL_ON_VMDNS_ERROR(dwError);
            break;
        }
    }
cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsGetRDataLength(
    PVMDNS_RECORD  pDnsRecord,
    PUINT16        puRdataLength,
    BOOL           bTokenizeDomainName
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    UINT16 uRdataLength = 0;

    if (!pDnsRecord || !puRdataLength)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    for (; dwIndex < gRecordMethodMapSize; ++dwIndex)
    {
        if (pDnsRecord->dwType == gRecordMethods[dwIndex].type)
        {
            dwError = gRecordMethods[dwIndex].pfnGetRDataLength(
                                                        pDnsRecord->Data,
                                                        &uRdataLength,
                                                        bTokenizeDomainName
                                                    );
            BAIL_ON_VMDNS_ERROR(dwError);
            break;
        }
    }

    *puRdataLength = uRdataLength;

cleanup:

    return dwError;
error:

    if (puRdataLength)
    {
        *puRdataLength = 0;
    }
    goto cleanup;
}

DWORD
VmDnsSerializeDnsRecord(
    PVMDNS_RECORD pDnsRecord,
    PBYTE* ppBytes,
    DWORD* pdwSize,
    BOOL bTokenizeDomainName
    )
{
    DWORD dwError = 0;
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer = NULL;
    PBYTE pBytes = NULL;
    DWORD dwSize =  0;

    if (!pDnsRecord || !ppBytes || !pdwSize)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR (dwError);
    }

    dwError = VmDnsAllocateBufferStream(
                            0,
                            &pVmDnsBuffer
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    pVmDnsBuffer->bTokenizeDomainName = bTokenizeDomainName;

    dwError = VmDnsWriteRecordToBuffer(
                        pDnsRecord,
                        pVmDnsBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyBufferFromBufferStream(
                            pVmDnsBuffer,
                            NULL,
                            &dwSize
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateMemory(
                        dwSize,
                        (PVOID *)&pBytes
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyBufferFromBufferStream(
                            pVmDnsBuffer,
                            pBytes,
                            &dwSize
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppBytes = pBytes;
    *pdwSize = dwSize;

cleanup:
    if (pVmDnsBuffer)
    {
        VmDnsFreeBufferStream(pVmDnsBuffer);
    }

    return dwError;
error:
    if (ppBytes)
    {
        *ppBytes = NULL;
    }
    if (pdwSize)
    {
        *pdwSize = 0;
    }
    VMDNS_SAFE_FREE_MEMORY(pBytes);

    goto cleanup;
}

DWORD
VmDnsReadRecordFromBuffer(
        PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
        PVMDNS_RECORD *ppDnsRecord
        )
{
    DWORD dwError = 0;
    PVMDNS_RECORD pDnsRecord = NULL;
    DWORD dwIndex =  0;
    PBYTE pRawTsigPtr = NULL;

    if (!pVmDnsBuffer || !ppDnsRecord)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(
                        sizeof(VMDNS_RECORD),
                        (PVOID *)&pDnsRecord
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    pRawTsigPtr = pVmDnsBuffer->pMessage + pVmDnsBuffer->szCursor;

    dwError = VmDnsReadDomainNameFromBuffer(
                            pVmDnsBuffer,
                            &pDnsRecord->pszName
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadUINT16FromBuffer(
                            pVmDnsBuffer,
                            &pDnsRecord->dwType
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (pDnsRecord->dwType == VMDNS_RR_MTYPE_TSIG)
    {
        pDnsRecord->Data.TSIG.pRawTsigPtr = pRawTsigPtr;
    }
    else
    {
        pRawTsigPtr = NULL;
    }

    dwError = VmDnsReadUINT16FromBuffer(
                            pVmDnsBuffer,
                            &pDnsRecord->iClass
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadUINT32FromBuffer(
                            pVmDnsBuffer,
                            &pDnsRecord->dwTtl
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    for (; dwIndex < gRecordMethodMapSize; ++dwIndex)
    {
        if (pDnsRecord->dwType == gRecordMethods[dwIndex].type)
        {
            dwError = gRecordMethods[dwIndex].pfnDeSerialize(
                                                    pVmDnsBuffer,
                                                    &pDnsRecord->Data
                                                    );
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }

    *ppDnsRecord = pDnsRecord;

cleanup:

    return dwError;
error:

    if (ppDnsRecord)
    {
        *ppDnsRecord = NULL;
    }
    VMDNS_FREE_RECORD(pDnsRecord);
    goto cleanup;
}


DWORD
VmDnsDeserializeDnsRecord(
    PBYTE pBytes,
    DWORD dwSize,
    PVMDNS_RECORD *ppDnsRecord,
    BOOL bTokenizeDomainName
    )
{
    DWORD dwError = 0;
    PVMDNS_RECORD pDnsRecord = NULL;
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer = NULL;

    if (!pBytes || !dwSize || !ppDnsRecord)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateBufferStreamWithBuffer(
                            pBytes,
                            dwSize,
                            0,
                            FALSE,
                            &pVmDnsBuffer
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    pVmDnsBuffer->bTokenizeDomainName = bTokenizeDomainName;

    dwError = VmDnsReadRecordFromBuffer(
                        pVmDnsBuffer,
                        &pDnsRecord
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppDnsRecord = pDnsRecord;

cleanup:

    if (pVmDnsBuffer)
    {
        VmDnsFreeBufferStream(pVmDnsBuffer);
    }
    return dwError;
error:

    if (ppDnsRecord)
    {
        *ppDnsRecord = NULL;
    }
    VMDNS_FREE_RECORD(pDnsRecord);
    goto cleanup;
}

DWORD
VmDnsRecordTypeToString(
    VMDNS_RR_TYPE       type,
    PCSTR*              ppszName
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    DWORD idx = 0;
    for (; idx < gRecordTypeMapSize; ++idx)
    {
        if (type == gRecordTypeMap[idx].type)
        {
            dwError = ERROR_SUCCESS;
            *ppszName = gRecordTypeMap[idx].pszName;
            break;
        }
    }

    return dwError;
}

DWORD
VmDnsServiceTypeToString(
    VMDNS_SERVICE_TYPE  service,
    PCSTR*              ppszName
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    DWORD idx = 0;
    for (; idx < gServiceNameMapSize; ++idx)
    {
        if (service == gServiceNameMap[idx].type)
        {
            dwError = ERROR_SUCCESS;
            *ppszName = gServiceNameMap[idx].pszName;
            break;
        }
    }

    return dwError;
}

DWORD
VmDnsProtocolToString(
    VMDNS_SERVICE_PROTOCOL  protocol,
    PCSTR*                  ppszName
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    DWORD idx = 0;
    for (; idx < gProtocolNameMapSize; ++idx)
    {
        if (protocol == gProtocolNameMap[idx].protocol)
        {
            dwError = ERROR_SUCCESS;
            *ppszName = gProtocolNameMap[idx].pszName;
            break;
        }
    }

    return dwError;
}

BOOLEAN
VmDnsFindRecordMethods(
    PVMDNS_RECORD   pRecord,
    DWORD           *pIdx
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    DWORD idx = 0;
    for (; idx < gRecordMethodMapSize; ++idx)
    {
        if (pRecord->dwType == gRecordMethods[idx].type)
        {
            dwError = ERROR_SUCCESS;
            *pIdx = idx;
            break;
        }
    }

    return dwError;
}

BOOLEAN
VmDnsMatchRecordCommon(
    PVMDNS_RECORD   pRecord1,
    PVMDNS_RECORD   pRecord2
    )
{
    BOOLEAN result = TRUE;
    if (!pRecord1 && !pRecord2)
    {
        result = TRUE;
    }
    else if (!pRecord1 || !pRecord2)
    {
        result = FALSE;
    }
    else if (pRecord1->dwType != VMDNS_RR_QTYPE_ANY &&
             pRecord1->dwType != pRecord2->dwType)
    {
        result = FALSE;
    }
    else if (pRecord1->iClass != VMDNS_CLASS_NONE &&
             pRecord1->iClass != pRecord2->iClass)
    {
        result = FALSE;
    }
    else if (VmDnsStringCompareA(pRecord1->pszName, pRecord2->pszName, TRUE))
    {
        result = FALSE;
    }

    return result;
}

BOOLEAN
VmDnsCompareRecordCommon(
    PVMDNS_RECORD   pRecord1,
    PVMDNS_RECORD   pRecord2
    )
{
    BOOLEAN result = TRUE;

    if (!VmDnsMatchRecordCommon(pRecord1, pRecord2))
    {
        result = FALSE;
    }

    return result;
}

BOOLEAN
VmDnsValidateRecordCommon(
    PVMDNS_RECORD   pRecord
    )
{
    if (pRecord &&
        VmDnsStringLenA(pRecord->pszName) <= VMDNS_NAME_LENGTH_MAX &&
        pRecord->iClass == VMDNS_CLASS_IN)
    {
        return TRUE;
    }

    return FALSE;
}

DWORD
VmDnsGetDomainNameLength(
    PSTR pszDomainName,
    PUINT16 puSize,
    BOOL bTokenizeDomainName
    )
{
    DWORD dwError = 0;
    PSTR pszTempString = NULL;
    PSTR  pToken = NULL;
    PSTR  pNextToken = NULL;
    UINT16 uTotalStringLength = 0;

    if (!puSize)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (bTokenizeDomainName)
    {
        dwError = VmDnsAllocateStringA(
                                pszDomainName,
                                &pszTempString
                                );
        BAIL_ON_VMDNS_ERROR(dwError);

        pToken = VmDnsStringTokA(
                          pszTempString,
                          ".",
                          &pNextToken
                          );

        while(++uTotalStringLength && pToken)
        {
            UINT16 uStringLength = 0;

            uStringLength = VmDnsStringLenA(pToken);

            uTotalStringLength += uStringLength;

            pToken = VmDnsStringTokA(
                                 NULL,
                                 ".",
                                 &pNextToken
                                 );
        }
    }
    else
    {
        uTotalStringLength = VmDnsStringLenA(pszDomainName);
    }

    *puSize = uTotalStringLength;

cleanup:

    VMDNS_SAFE_FREE_STRINGA(pszTempString);
    return dwError;
error:

    if (puSize)
    {
        *puSize = 0;
    }
    goto cleanup;
}

DWORD
VmDnsWriteDomainNameLabelsToBuffer(
    PSTR pszDomainName,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    )
{
    DWORD dwError = 0;
    PSTR pszTempString = NULL;
    PSTR  pToken = NULL;
    PSTR  pNextToken = NULL;
    DWORD dwTotalStringLength = 0;

    if (!pszDomainName || !pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateStringA(
                        pszDomainName,
                        &pszTempString
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    pToken = VmDnsStringTokA(
                    pszTempString,
                    ".",
                    &pNextToken
                    );

    while(pToken)
    {
        DWORD dwStringLength = 0;

        dwStringLength = VmDnsStringLenA(pToken);
        if (dwStringLength > VMDNS_LABEL_LENGTH_MAX)
        {
            dwError = ERROR_LABEL_TOO_LONG;
            BAIL_ON_VMDNS_ERROR(dwError);
        }

        dwError = VmDnsWriteStringToBuffer(
                            pToken,
                            dwStringLength,
                            pVmDnsBuffer
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        dwTotalStringLength += dwStringLength+1;

        if (dwTotalStringLength > VMDNS_NAME_LENGTH_MAX)
        {
            dwError = ERROR_LABEL_TOO_LONG;
            BAIL_ON_VMDNS_ERROR(dwError);
        }

        pToken = VmDnsStringTokA(
                        NULL,
                        ".",
                        &pNextToken
                        );
    }

    if (++dwTotalStringLength > VMDNS_NAME_LENGTH_MAX)
    {
        dwError = ERROR_LABEL_TOO_LONG;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsWriteCharToBuffer(
                        0,
                        pVmDnsBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    VMDNS_SAFE_FREE_STRINGA(pszTempString);
    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsWriteDomainNameStringToBuffer(
    PSTR pszDomainName,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    )
{
    DWORD dwError = 0;
    DWORD dwStringLength = 0;

    if (!pszDomainName || !pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwStringLength = VmDnsStringLenA(pszDomainName);

    if (dwStringLength > VMDNS_NAME_LENGTH_MAX)
    {
        dwError = ERROR_LABEL_TOO_LONG;
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    if (dwStringLength)
    {
        dwError = VmDnsWriteStringToBuffer(
                            pszDomainName,
                            dwStringLength,
                            pVmDnsBuffer
                            );
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsWriteCharToBuffer(
                        0,
                        pVmDnsBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}


DWORD
VmDnsWriteDomainNameToBuffer(
    PSTR pszDomainName,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    BOOL bTokenizeDomainName
    )
{
    if (bTokenizeDomainName)
    {
        return VmDnsWriteDomainNameLabelsToBuffer(
            pszDomainName,
            pVmDnsBuffer);
    }
    else
    {
        return VmDnsWriteDomainNameStringToBuffer(
            pszDomainName,
            pVmDnsBuffer);
    }
}

DWORD
VmDnsReadDomainNameFromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PSTR *ppszDomainName
    )
{
    DWORD dwError = 0;
    DWORD dwTotalStringLength = 0;
    BOOL bEndOfString = FALSE;
    PSTR pszTempString = NULL;
    PSTR pszTempStringCursor = NULL;
    PSTR pszLabels = NULL;
    PSTR pszDomainName = NULL;

    if (!pVmDnsBuffer || !ppszDomainName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(
                        VMDNS_NAME_LENGTH_MAX + 2,
                        (PVOID *)&pszTempString
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    pszTempStringCursor = pszTempString;

    do
    {
        DWORD dwLabelLength = 0;
        dwError = VmDnsReadStringFromBuffer(
                            pVmDnsBuffer,
                            &pszLabels,
                            &dwLabelLength,
                            &bEndOfString
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        if (dwLabelLength)
        {
            if (dwLabelLength > (VMDNS_NAME_LENGTH_MAX - dwTotalStringLength))
            {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMDNS_ERROR(dwError);
            }
            dwError = VmDnsCopyMemory(
                          pszTempStringCursor,
                          VMDNS_NAME_LENGTH_MAX - dwTotalStringLength,
                          pszLabels,
                          dwLabelLength
                          );
            BAIL_ON_VMDNS_ERROR(dwError);

            if (!bEndOfString)
            {
                if (pszTempStringCursor[dwLabelLength - 1] != '.')
                {
                    pszTempStringCursor[dwLabelLength]='.';
                    dwLabelLength++;
                }
           }
        }

        pszTempStringCursor += dwLabelLength;
        dwTotalStringLength += dwLabelLength;
        VMDNS_SAFE_FREE_STRINGA(pszLabels);

        if (dwTotalStringLength > VMDNS_NAME_LENGTH_MAX)
        {
            dwError = ERROR_LABEL_TOO_LONG;
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }while(!bEndOfString);


    if (dwTotalStringLength > 0
         && !VmDnsCheckIfIPV4AddressA(pszTempString)
         && !VmDnsCheckIfIPV6AddressA(pszTempString))
    {
        if (pszTempString[dwTotalStringLength - 1] != '.')
        {
            pszTempString[dwTotalStringLength]='.';
            dwTotalStringLength++;
        }
    }


    dwError = VmDnsAllocateStringA(
                        pszTempString,
                        &pszDomainName
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppszDomainName = pszDomainName;

cleanup:

    VMDNS_SAFE_FREE_STRINGA(pszTempString);
    VMDNS_SAFE_FREE_STRINGA(pszLabels);

    return dwError;

error:

    if (ppszDomainName)
    {
        *ppszDomainName = NULL;
    }

    goto cleanup;
}

DWORD
VmDnsParseRecordType(
    PSTR            pszRecordType,
    VMDNS_RR_TYPE*  pType
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_INVALID_PARAMETER;

    BAIL_ON_VMDNS_INVALID_POINTER(pszRecordType, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pType, dwError);

    for (; idx < gRecordTypeMapSize; ++idx)
    {
        if (!VmDnsStringCompareA(pszRecordType,
                    gRecordTypeMap[idx].pszName,
                    FALSE))
        {
            *pType = gRecordTypeMap[idx].type;
            dwError = ERROR_SUCCESS;
            break;
        }
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsParseServiceType(
    PSTR                pszServiceType,
    VMDNS_SERVICE_TYPE* pType,
    PSTR*               ppszName
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_INVALID_PARAMETER;

    BAIL_ON_VMDNS_INVALID_POINTER(pszServiceType, dwError);

    for (; idx < gServiceNameMapSize; ++idx)
    {
        if (!VmDnsStringCompareA(pszServiceType,
                    gServiceNameMap[idx].pszUserFriendlyName,
                    FALSE))
        {
            dwError = ERROR_SUCCESS;
            if (pType)
            {
                *pType = gServiceNameMap[idx].type;
            }
            if (ppszName)
            {
                dwError = VmDnsAllocateStringA(gServiceNameMap[idx].pszName, ppszName);
            }
            break;
        }
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsParseServiceProtocol(
    PSTR                    pszServiceType,
    VMDNS_SERVICE_PROTOCOL* pProtocol,
    PSTR*                   ppszName
    )
{
    DWORD idx = 0;
    DWORD dwError = ERROR_INVALID_PARAMETER;

    BAIL_ON_VMDNS_INVALID_POINTER(pszServiceType, dwError);

    for (; idx < gProtocolNameMapSize; ++idx)
    {
        if (!VmDnsStringCompareA(pszServiceType,
                                gProtocolNameMap[idx].pszUserFriendlyName,
                                FALSE))
        {
            dwError = ERROR_SUCCESS;
            if (pProtocol)
            {
                *pProtocol = gProtocolNameMap[idx].protocol;
            }
            if (ppszName)
            {
                dwError = VmDnsAllocateStringA(gProtocolNameMap[idx].pszName, ppszName);
            }
            break;
        }
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

BOOL
VmDnsIsSupportedRecordType(
    VMDNS_RR_TYPE   dwRecordType
    )
{
    return
        dwRecordType == VMDNS_RR_TYPE_A     ||
        dwRecordType == VMDNS_RR_TYPE_AAAA  ||
        dwRecordType == VMDNS_RR_TYPE_CNAME ||
        dwRecordType == VMDNS_RR_TYPE_NS    ||
        dwRecordType == VMDNS_RR_TYPE_SOA   ||
        dwRecordType == VMDNS_RR_TYPE_SRV   ||
        dwRecordType == VMDNS_RR_QTYPE_ANY;
}

BOOL
VmDnsIsUpdatePermitted(
    VMDNS_RR_TYPE   dwRecordType
    )
{
    return
        dwRecordType == VMDNS_RR_TYPE_SOA   ||
        dwRecordType == VMDNS_RR_TYPE_CNAME;
}

BOOL
VmDnsIsRecordRType(
    VMDNS_RR_TYPE   dwRecordType
    )
{
    return
        dwRecordType == VMDNS_RR_TYPE_NONE  ||
        dwRecordType == VMDNS_RR_TYPE_A     ||
        dwRecordType == VMDNS_RR_TYPE_NS    ||
        dwRecordType == VMDNS_RR_TYPE_MD    ||
        dwRecordType == VMDNS_RR_TYPE_MF    ||
        dwRecordType == VMDNS_RR_TYPE_CNAME ||
        dwRecordType == VMDNS_RR_TYPE_SOA   ||
        dwRecordType == VMDNS_RR_TYPE_MB    ||
        dwRecordType == VMDNS_RR_TYPE_MG    ||
        dwRecordType == VMDNS_RR_TYPE_MR    ||
        dwRecordType == VMDNS_RR_TYPE_NULL  ||
        dwRecordType == VMDNS_RR_TYPE_WKS   ||
        dwRecordType == VMDNS_RR_TYPE_PTR   ||
        dwRecordType == VMDNS_RR_TYPE_HINFO ||
        dwRecordType == VMDNS_RR_TYPE_MINFO ||
        dwRecordType == VMDNS_RR_TYPE_MX    ||
        dwRecordType == VMDNS_RR_TYPE_TXT   ||
        dwRecordType == VMDNS_RR_TYPE_RP    ||
        dwRecordType == VMDNS_RR_TYPE_AFSDB ||
        dwRecordType == VMDNS_RR_TYPE_SIG   ||
        dwRecordType == VMDNS_RR_TYPE_AAAA  ||
        dwRecordType == VMDNS_RR_TYPE_LOC   ||
        dwRecordType == VMDNS_RR_TYPE_SRV   ||
        dwRecordType == VMDNS_RR_TYPE_CERT  ||
        dwRecordType == VMDNS_RR_TYPE_DS    ||
        dwRecordType == VMDNS_RR_TYPE_SSHFP ||
        dwRecordType == VMDNS_RR_TYPE_IPSEC ||
        dwRecordType == VMDNS_RR_TYPE_RRSIG ||
        dwRecordType == VMDNS_RR_TYPE_DNSKEY;
}

BOOL
VmDnsIsRecordQType(
    VMDNS_RR_TYPE   dwRecordType
    )
{
    return
        dwRecordType == VMDNS_RR_QTYPE_AXFR     ||
        dwRecordType == VMDNS_RR_QTYPE_MAILB    ||
        dwRecordType == VMDNS_RR_QTYPE_MAILA    ||
        dwRecordType == VMDNS_RR_QTYPE_ANY;
}

BOOL
VmDnsIsRecordMType(
    VMDNS_RR_TYPE   dwRecordType
    )
{
    return
        dwRecordType == VMDNS_RR_MTYPE_OPT  ||
        dwRecordType == VMDNS_RR_MTYPE_TKEY ||
        dwRecordType == VMDNS_RR_MTYPE_TSIG;
}
