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
VmDnsCompareAddressRecord(
    PVMDNS_RECORD   pRecord1,
    PVMDNS_RECORD   pRecord2
    )
{
    return VmDnsCompareRecordCommon(pRecord1, pRecord2) &&
            pRecord1->Data.A.IpAddress == pRecord2->Data.A.IpAddress;
}

VOID
VmDnsClearAddressRecord(
    PVMDNS_RECORD       pRecord
    )
{
    VMDNS_SAFE_FREE_STRINGA(pRecord->pszName);
}

VOID
VmDnsRpcClearAddressRecord(
    PVMDNS_RECORD       pRecord
    )
{
    if (pRecord)
    {
        DWORD rpcStatus = rpc_s_ok;
        rpc_string_free((PBYTE*)&pRecord->pszName, &rpcStatus);
    }
}

BOOLEAN
VmDnsValidateAddressRecord(
    PVMDNS_RECORD       pRecord
    )
{
    return VmDnsValidateRecordCommon(pRecord);
}

DWORD VmDnsDuplicateAddressRecord(
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

    dwError = VmDnsCopyAddressRecord(pSrc, pRecord);
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
VmDnsRpcDuplicateAddressRecord(
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

    dwError = VmDnsRpcCopyAddressRecord(pSrc, pRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppDest = pRecord;
    pRecord = NULL;

cleanup:
    return dwError;

error:
    VmDnsRpcClearAddressRecord(pRecord);
    VmDnsRpcFreeMemory(pRecord);
    if (ppDest)
    {
        *ppDest = NULL;
    }
    goto cleanup;
}

DWORD
VmDnsCopyAddressRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    )
{
    DWORD dwError = 0;
    PVMDNS_A_DATA pData = NULL;

    pData = &pDest->Data.A;

    VmDnsAllocateStringA(pSrc->pszName, &pDest->pszName);
    BAIL_ON_VMDNS_ERROR(dwError);

    pDest->iClass = pSrc->iClass;
    pDest->dwTtl = pSrc->dwTtl;
    pDest->dwType = pSrc->dwType;

    pData->IpAddress = pSrc->Data.A.IpAddress;

cleanup:
    return dwError;

error:
    VmDnsClearRecord(pDest);

    goto cleanup;
}

DWORD
VmDnsRpcCopyAddressRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    )
{
    DWORD dwError = 0;
    PVMDNS_A_DATA pData = NULL;

    pData = &pDest->Data.A;

    VmDnsRpcAllocateStringA(pSrc->pszName, &pDest->pszName);
    BAIL_ON_VMDNS_ERROR(dwError);

    pDest->iClass = pSrc->iClass;
    pDest->dwTtl = pSrc->dwTtl;
    pDest->dwType = pSrc->dwType;

    pData->IpAddress = pSrc->Data.A.IpAddress;

cleanup:
    return dwError;

error:
    VmDnsRpcClearAddressRecord(pDest);

    goto cleanup;
}

DWORD
VmDnsAddressRecordToString(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PSTR  pStr = NULL;
    PCSTR pszType = NULL;
    CHAR  szAddr[INET_ADDRSTRLEN] = {0};
    VMDNS_IP4_ADDRESS ip4 = 0;

    dwError = VmDnsRecordTypeToString(pRecord->dwType, &pszType);
    BAIL_ON_VMDNS_ERROR(dwError);

    ip4 = htonl(pRecord->Data.A.IpAddress);

    if (!inet_ntop(
            AF_INET,
            &ip4,
            szAddr,
            sizeof(szAddr)))
    {
        dwError = ERROR_BAD_FORMAT;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateStringPrintfA(
                    &pStr,
                    "Type:          %s\n"
                    "Name:          %s\n"
                    "Class:         %hu\n"
                    "TTL:           %u\n"
                    "Address:       %s\n",
                    pszType,
                    pRecord->pszName,
                    pRecord->iClass,
                    pRecord->dwTtl,
                    szAddr
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
VmDnsAddressRecordGetCN(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    )
{
    return VmDnsAllocateStringA(pRecord->pszName, ppStr);
}

DWORD
VmDnsAddressRecordGetRDataLength(
    VMDNS_RECORD_DATA   Data,
    PUINT16             puRDataLength,
    BOOL                bTokenizeDomainName
    )
{
    DWORD dwError = 0;
    UINT16 uRDataLength = 0;

    if (!puRDataLength)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    uRDataLength = sizeof(VMDNS_IP4_ADDRESS);

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
VmDnsSerializeDnsAddressRecord(
    VMDNS_RECORD_DATA Data,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    )
{
    DWORD dwError = 0;
    UINT16 uRDataLength = 0;
    if (!pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAddressRecordGetRDataLength(
                Data,
                &uRDataLength,
                pVmDnsBuffer->bTokenizeDomainName);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT16ToBuffer(
                               uRDataLength,
                               pVmDnsBuffer
                               );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT32ToBuffer(
                                Data.A.IpAddress,
                                pVmDnsBuffer
                                );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsDeserializeDnsAddressRecord(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PVMDNS_RECORD_DATA pData
    )
{
    DWORD dwError = 0;
    UINT16 dwRDataLength = 0;

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

    if (dwRDataLength)
    {
        dwError = VmDnsReadUINT32FromBuffer(
                                      pVmDnsBuffer,
                                      &pData->A.IpAddress
                                      );
        BAIL_ON_VMDNS_ERROR(dwError);
    }



cleanup:

    return dwError;
error:

    goto cleanup;
}
