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

#define VMDNS_SRV_RDATA_SIZE_BASE 3*sizeof(WORD)

BOOLEAN
VmDnsCompareSrvRecord(
    PVMDNS_RECORD   pRecord1,
    PVMDNS_RECORD   pRecord2
    )
{
    return  VmDnsCompareRecordCommon(pRecord1, pRecord2) &&
            VmDnsStringCompareA(pRecord1->Data.SRV.pNameTarget,
                        pRecord2->Data.SRV.pNameTarget, FALSE) == 0 &&
            pRecord1->Data.SRV.wPort == pRecord2->Data.SRV.wPort &&
            pRecord1->Data.SRV.wPriority == pRecord2->Data.SRV.wPriority &&
            pRecord1->Data.SRV.wWeight == pRecord2->Data.SRV.wWeight;
}

BOOLEAN
VmDnsMatchSrvRecord(
    PVMDNS_RECORD   pRecord,
    PVMDNS_RECORD   pTemplate
    )
{
    return  VmDnsMatchRecordCommon(pRecord, pTemplate) &&
            VmDnsStringCompareA(pRecord->Data.SRV.pNameTarget,
                        pTemplate->Data.SRV.pNameTarget, FALSE) == 0;
}

VOID
VmDnsClearSrvRecord(
    PVMDNS_RECORD   pRecord
    )
{
    if (pRecord)
    {
        VMDNS_SAFE_FREE_STRINGA(pRecord->pszName);
        VMDNS_SAFE_FREE_STRINGA(pRecord->Data.SRV.pNameTarget);
    }
}

VOID
VmDnsRpcClearSrvRecord(
    PVMDNS_RECORD       pRecord
    )
{
    if (pRecord)
    {
        DWORD rpcStatus = rpc_s_ok;
        rpc_string_free((PBYTE*)&pRecord->pszName, &rpcStatus);
        rpc_string_free((PBYTE*)&pRecord->Data.SRV.pNameTarget, &rpcStatus);
    }
}

BOOLEAN
VmDnsValidateSrvRecord(
    PVMDNS_RECORD   pRecord
    )
{
    return VmDnsValidateRecordCommon(pRecord) &&
            VmDnsStringLenA(pRecord->Data.SRV.pNameTarget) <= VMDNS_NAME_LENGTH_MAX;
}

DWORD VmDnsDuplicateSrvRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    )
{
    DWORD            dwError = 0;
    PVMDNS_RECORD    pRecord = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pSrc, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppDest, dwError);

    dwError = VmDnsAllocateMemory(sizeof(VMDNS_RECORD), (void**)&pRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopySrvRecord(pSrc, pRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppDest = pRecord;
    pRecord = NULL;

cleanup:
    return dwError;

error:
    VMDNS_FREE_RECORD(pRecord);
    if (ppDest)
    {
        *ppDest = NULL;
    }
    goto cleanup;
}

DWORD VmDnsRpcDuplicateSrvRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    )
{
    DWORD            dwError = 0;
    PVMDNS_RECORD    pRecord = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pSrc, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppDest, dwError);

    dwError = VmDnsRpcAllocateMemory(sizeof(VMDNS_RECORD), (PVOID*)&pRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRpcCopySrvRecord(pSrc, pRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppDest = pRecord;
    pRecord = NULL;

cleanup:
    return dwError;

error:
    VmDnsRpcClearSrvRecord(pRecord);
    VmDnsRpcFreeMemory(pRecord);
    if (ppDest)
    {
        *ppDest = NULL;
    }
    goto cleanup;
}

DWORD VmDnsCopySrvRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    )
{
    DWORD dwError = 0;
    PVMDNS_SRV_DATAA pData = NULL;

    pData = &pDest->Data.SRV;

    VmDnsAllocateStringA(pSrc->pszName, &pDest->pszName);
    BAIL_ON_VMDNS_ERROR(dwError);

    pDest->iClass = pSrc->iClass;
    pDest->dwTtl = pSrc->dwTtl;
    pDest->dwType = pSrc->dwType;
    pData->wPort = pSrc->Data.SRV.wPort;
    pData->wPriority = pSrc->Data.SRV.wPriority;
    pData->wWeight = pSrc->Data.SRV.wWeight;

    VmDnsAllocateStringA(pSrc->Data.SRV.pNameTarget, &pData->pNameTarget);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    VmDnsClearRecord(pDest);

    goto cleanup;
}

DWORD VmDnsRpcCopySrvRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    )
{
    DWORD dwError = 0;
    PVMDNS_SRV_DATAA pData = NULL;

    pData = &pDest->Data.SRV;

    VmDnsRpcAllocateStringA(pSrc->pszName, &pDest->pszName);
    BAIL_ON_VMDNS_ERROR(dwError);

    pDest->iClass = pSrc->iClass;
    pDest->dwTtl = pSrc->dwTtl;
    pDest->dwType = pSrc->dwType;
    pData->wPort = pSrc->Data.SRV.wPort;
    pData->wPriority = pSrc->Data.SRV.wPriority;
    pData->wWeight = pSrc->Data.SRV.wWeight;

    VmDnsRpcAllocateStringA(pSrc->Data.SRV.pNameTarget, &pData->pNameTarget);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    VmDnsClearRecord(pDest);

    goto cleanup;
}

DWORD
VmDnsSrvRecordToString(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PSTR  pStr = NULL;
    PCSTR pszType = NULL;

    dwError = VmDnsRecordTypeToString(pRecord->dwType, &pszType);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringPrintfA(
                    &pStr,
                    "Type:          %s\n"
                    "Name:          %s\n"
                    "Target:        %s\n"
                    "Port:          %hu\n"
                    "Priority:      %hu\n"
                    "Weight:        %hu\n",
                    pszType,
                    pRecord->pszName,
                    pRecord->Data.SRV.pNameTarget,
                    pRecord->Data.SRV.wPort,
                    pRecord->Data.SRV.wPriority,
                    pRecord->Data.SRV.wWeight
                    );
    *ppStr = pStr;
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDNS_SAFE_FREE_STRINGA(pStr);
    goto cleanup;
}

DWORD
VmDnsSrvRecordGetCN(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PSTR  pStr = NULL;

    dwError = VmDnsAllocateStringA(pRecord->pszName, &pStr);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppStr = pStr;

cleanup:
    return dwError;

error:
    VMDNS_SAFE_FREE_STRINGA(pStr);
    goto cleanup;
}

DWORD
VmDnsSrvGetRDataLength(
    VMDNS_RECORD_DATA   Data,
    PUINT16             puRDataLength,
    BOOL                bTokenizeDomainName
    )
{
    DWORD dwError = 0;
    UINT16 uRDataLength = 0;
    UINT16 uNameTargetLength = 0;

    if (!puRDataLength)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsGetDomainNameLength(
                              Data.SRV.pNameTarget,
                              &uNameTargetLength,
                              bTokenizeDomainName
                              );
    BAIL_ON_VMDNS_ERROR(dwError);

    uRDataLength += uNameTargetLength;

    uRDataLength += sizeof(UINT16); //wPriority

    uRDataLength += sizeof(UINT16); //wWeight

    uRDataLength += sizeof(UINT16); //wPort

    *puRDataLength = uRDataLength;

cleanup:

    return dwError;
error:

    if (puRDataLength)
    {
        *puRDataLength = 0;
    }
    goto cleanup;
}


DWORD
VmDnsSerializeDnsSrvRecord(
    VMDNS_RECORD_DATA Data,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    )
{
    DWORD dwError = 0;
    UINT16 uSize = 0;

    if (!pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsSrvGetRDataLength(
                                Data,
                                &uSize,
                                pVmDnsBuffer->bTokenizeDomainName
                                );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT16ToBuffer(
                                uSize,
                                pVmDnsBuffer
                                );
    BAIL_ON_VMDNS_ERROR(dwError);


    dwError = VmDnsWriteUINT16ToBuffer(
                                Data.SRV.wPriority,
                                pVmDnsBuffer
                                );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT16ToBuffer(
                                Data.SRV.wWeight,
                                pVmDnsBuffer
                                );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT16ToBuffer(
                                Data.SRV.wPort,
                                pVmDnsBuffer
                                );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteDomainNameToBuffer(
                                Data.SRV.pNameTarget,
                                pVmDnsBuffer,
                                pVmDnsBuffer->bTokenizeDomainName
                                );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsDeserializeDnsSrvRecord(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PVMDNS_RECORD_DATA pData
    )
{
    DWORD dwError = 0;
    UINT16 dwRDataLength = 0;
    UINT16 uReceivedRDataLength = 0;

    if (!pVmDnsBuffer ||
        !pData
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsReadUINT16FromBuffer(
                                  pVmDnsBuffer,
                                  &dwRDataLength
                                  );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (!dwRDataLength)
    {
        dwError = ERROR_EMPTY;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsReadUINT16FromBuffer(
                                 pVmDnsBuffer,
                                 &pData->SRV.wPriority
                                 );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadUINT16FromBuffer(
                                 pVmDnsBuffer,
                                 &pData->SRV.wWeight
                                 );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadUINT16FromBuffer(
                                 pVmDnsBuffer,
                                 &pData->SRV.wPort
                                 );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadDomainNameFromBuffer(
                                 pVmDnsBuffer,
                                 &pData->SRV.pNameTarget
                                 );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSrvGetRDataLength(
                *pData,
                &uReceivedRDataLength,
                pVmDnsBuffer->bTokenizeDomainName
                );
    BAIL_ON_VMDNS_ERROR(dwError);


cleanup:

    return dwError;
error:

    goto cleanup;
}
