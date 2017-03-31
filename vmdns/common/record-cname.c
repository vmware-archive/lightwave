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
VmDnsCompareCNameRecord(
    PVMDNS_RECORD   pRecord1,
    PVMDNS_RECORD   pRecord2
    )
{
    return VmDnsCompareRecordCommon(pRecord1, pRecord2) &&
        VmDnsStringCompareA(pRecord1->Data.CNAME.pNameHost,
                        pRecord2->Data.CNAME.pNameHost, FALSE) == 0;
}

VOID
VmDnsClearCNameRecord(
    PVMDNS_RECORD   pRecord
    )
{
    if (pRecord)
    {
        VMDNS_SAFE_FREE_STRINGA(pRecord->pszName);
        VMDNS_SAFE_FREE_MEMORY(pRecord->Data.CNAME.pNameHost);
    }
}

VOID
VmDnsRpcClearCNameRecord(
    PVMDNS_RECORD   pRecord
    )
{
    if (pRecord)
    {
        DWORD rpcStatus = rpc_s_ok;
        rpc_string_free((PBYTE*)&pRecord->pszName, &rpcStatus);
        rpc_string_free((PBYTE*)&pRecord->Data.CNAME.pNameHost, &rpcStatus);
    }
}

BOOLEAN
VmDnsValidateCNameRecord(
    PVMDNS_RECORD   pRecord
    )
{
    return VmDnsValidateRecordCommon(pRecord) &&
            VmDnsStringLenA(pRecord->Data.CNAME.pNameHost) <= VMDNS_NAME_LENGTH_MAX;
}

DWORD
VmDnsDuplicateCNameRecord(
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

    dwError = VmDnsCopyCNameRecord(pSrc, pRecord);
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

DWORD
VmDnsRpcDuplicateCNameRecord(
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

    dwError = VmDnsRpcCopyCNameRecord(pSrc, pRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppDest = pRecord;
    pRecord = NULL;

cleanup:
    return dwError;

error:
    VmDnsRpcClearCNameRecord(pRecord);
    VmDnsRpcFreeMemory(pRecord);
    if (ppDest)
    {
        *ppDest = NULL;
    }
    goto cleanup;
}

DWORD
VmDnsCopyCNameRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    )
{
    DWORD dwError = 0;
    PVMDNS_PTR_DATAA pData = NULL;

    pData = &pDest->Data.CNAME;

    VmDnsAllocateStringA(pSrc->pszName, &pDest->pszName);
    BAIL_ON_VMDNS_ERROR(dwError);

    pDest->iClass = pSrc->iClass;
    pDest->dwTtl = pSrc->dwTtl;
    pDest->dwType = pSrc->dwType;

    VmDnsAllocateStringA(pSrc->Data.CNAME.pNameHost, &pData->pNameHost);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    VmDnsClearRecord(pDest);

    goto cleanup;
}

DWORD
VmDnsRpcCopyCNameRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    )
{
    DWORD dwError = 0;
    PVMDNS_PTR_DATAA pData = NULL;

    pData = &pDest->Data.CNAME;

    VmDnsRpcAllocateStringA(pSrc->pszName, &pDest->pszName);
    BAIL_ON_VMDNS_ERROR(dwError);

    pDest->iClass = pSrc->iClass;
    pDest->dwTtl = pSrc->dwTtl;
    pDest->dwType = pSrc->dwType;

    VmDnsRpcAllocateStringA(pSrc->Data.CNAME.pNameHost, &pData->pNameHost);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    VmDnsRpcClearCNameRecord(pDest);

    goto cleanup;
}

DWORD
VmDnsCNameRecordToString(
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
                    "Class:         %hu\n"
                    "TTL:           %u\n"
                    "Target:        %s\n",
                    pszType,
                    pRecord->pszName,
                    pRecord->iClass,
                    pRecord->dwTtl,
                    pRecord->Data.CNAME.pNameHost
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppStr = pStr;

cleanup:
    return dwError;

error:
    VMDNS_SAFE_FREE_STRINGA(pStr);
    goto cleanup;
}

DWORD
VmDnsCNameRecordGetCN(
    PVMDNS_RECORD   pRecord,
    PSTR*           ppStr
    )
{
    return VmDnsAllocateStringA(pRecord->pszName, ppStr);
}

DWORD
VmDnsCNameRecordGetRDataLength(
    VMDNS_RECORD_DATA Data,
    PUINT16           puRDataLength,
    BOOL              bTokenizeDomainName
    )
{
    DWORD dwError = 0;
    UINT16 uRdataLength = 0;

    if (!puRDataLength)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (Data.CNAME.pNameHost != NULL)
    {
        dwError = VmDnsGetDomainNameLength(
                                      Data.CNAME.pNameHost,
                                      &uRdataLength,
                                      bTokenizeDomainName
                                      );
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    *puRDataLength = uRdataLength;

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
VmDnsSerializeDnsCNameRecord(
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

    dwError = VmDnsCNameRecordGetRDataLength(
                Data,
                &uSize,
                pVmDnsBuffer->bTokenizeDomainName);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT16ToBuffer(
                                 uSize,
                                 pVmDnsBuffer
                                 );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (Data.CNAME.pNameHost != NULL)
    {
        dwError = VmDnsWriteDomainNameToBuffer(
                                 Data.CNAME.pNameHost,
                                 pVmDnsBuffer,
                                 pVmDnsBuffer->bTokenizeDomainName
                                 );
        BAIL_ON_VMDNS_ERROR(dwError);
    }
cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsDeserializeDnsCNameRecord(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PVMDNS_RECORD_DATA pData
    )
{
    DWORD dwError = 0;
    UINT16 dwRDataLength = 0;
    UINT16 uReceivedRDataLength = 0;

    if (!pVmDnsBuffer ||
        !pData)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsReadUINT16FromBuffer(
                                pVmDnsBuffer,
                                &dwRDataLength
                                );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (dwRDataLength)
    {
        dwError = VmDnsReadDomainNameFromBuffer(
                                 pVmDnsBuffer,
                                 &pData->CNAME.pNameHost
                                 );
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsCNameRecordGetRDataLength(
                    *pData,
                    &uReceivedRDataLength,
                    pVmDnsBuffer->bTokenizeDomainName);
        BAIL_ON_VMDNS_ERROR(dwError);

    }
cleanup:

    return dwError;
error:

    goto cleanup;
}
