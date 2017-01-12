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
VmDnsCompareIp6AddressRecord(
    PVMDNS_RECORD   pRecord1,
    PVMDNS_RECORD   pRecord2
    )
{
    return VmDnsCompareRecordCommon(pRecord1, pRecord2) &&
            !memcmp(pRecord1->Data.AAAA.Ip6Address.IP6Byte,
                pRecord2->Data.AAAA.Ip6Address.IP6Byte, 16*sizeof(BYTE));
}

VOID
VmDnsClearIp6AddressRecord(
    PVMDNS_RECORD       pRecord
    )
{
    VMDNS_SAFE_FREE_STRINGA(pRecord->pszName);
}

VOID
VmDnsRpcClearIp6AddressRecord(
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
VmDnsValidateIp6AddressRecord(
    PVMDNS_RECORD       pRecord
    )
{
    return VmDnsValidateRecordCommon(pRecord);
}

DWORD
VmDnsDuplicateIp6AddressRecord(
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

    dwError = VmDnsCopyIp6AddressRecord(pSrc, pRecord);
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
VmDnsRpcDuplicateIp6AddressRecord(
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

    dwError = VmDnsRpcCopyIp6AddressRecord(pSrc, pRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppDest = pRecord;
    pRecord = NULL;

cleanup:
    return dwError;

error:
    VmDnsRpcClearIp6AddressRecord(pRecord);
    VmDnsRpcFreeMemory(pRecord);
    if (ppDest)
    {
        *ppDest = NULL;
    }
    goto cleanup;
}

DWORD
VmDnsCopyIp6AddressRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    )
{
    DWORD dwError = 0;
    PVMDNS_AAAA_DATA pData = NULL;

    pData = &pDest->Data.AAAA;

    VmDnsAllocateStringA(pSrc->pszName, &pDest->pszName);
    BAIL_ON_VMDNS_ERROR(dwError);

    pDest->iClass = pSrc->iClass;
    pDest->dwTtl = pSrc->dwTtl;
    pDest->dwType = pSrc->dwType;

    dwError = VmDnsCopyMemory(
                  pData->Ip6Address.IP6Byte,
                  sizeof(pData->Ip6Address.IP6Byte),
                  pSrc->Data.AAAA.Ip6Address.IP6Byte,
                  16*sizeof(BYTE)
                  );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    VmDnsClearRecord(pDest);

    goto cleanup;
}

DWORD
VmDnsRpcCopyIp6AddressRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    )
{
    DWORD dwError = 0;
    PVMDNS_AAAA_DATA pData = NULL;

    pData = &pDest->Data.AAAA;

    VmDnsRpcAllocateStringA(pSrc->pszName, &pDest->pszName);
    BAIL_ON_VMDNS_ERROR(dwError);

    pDest->iClass = pSrc->iClass;
    pDest->dwTtl = pSrc->dwTtl;
    pDest->dwType = pSrc->dwType;

    dwError = VmDnsCopyMemory(
                      pData->Ip6Address.IP6Byte,
                      sizeof(pData->Ip6Address.IP6Byte),
                      pSrc->Data.AAAA.Ip6Address.IP6Byte,
                      16*sizeof(BYTE)
                      );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    VmDnsRpcClearIp6AddressRecord(pDest);

    goto cleanup;
}

DWORD
VmDnsIp6AddressRecordToString(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PSTR  pStr = NULL;
    PCSTR pszType = NULL;
    CHAR  szAddr[INET6_ADDRSTRLEN];

    dwError = VmDnsRecordTypeToString(pRecord->dwType, &pszType);
    BAIL_ON_VMDNS_ERROR(dwError);

    if (!inet_ntop(
            AF_INET6,
            pRecord->Data.AAAA.Ip6Address.IP6Byte,
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
    *ppStr = NULL;
    goto cleanup;
}

DWORD
VmDnsIp6AddressRecordGetCN(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    )
{
    return VmDnsAllocateStringA(pRecord->pszName, ppStr);
}

DWORD
VmDnsIp6AddressRecordGetRDataLength(
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

    uRDataLength = sizeof(VMDNS_IP6_ADDRESS);

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
VmDnsSerializeDnsIp6AddressRecord(
    VMDNS_RECORD_DATA Data,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    )
{
    DWORD dwError = 0;
    UINT16 idx = 0;
    UINT16 ip6AddrLength = 16;
    if (!pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsIp6AddressRecordGetRDataLength(
                Data,
                &ip6AddrLength,
                pVmDnsBuffer->bTokenizeDomainName);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT16ToBuffer(
                               ip6AddrLength,
                               pVmDnsBuffer
                               );
    BAIL_ON_VMDNS_ERROR(dwError);

    for (; idx < ip6AddrLength; ++idx)
    {
        dwError = VmDnsWriteUCharToBuffer(
                                   Data.AAAA.Ip6Address.IP6Byte[idx],
                                   pVmDnsBuffer
                                   );
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:

    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsDeserializeDnsIp6AddressRecord(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PVMDNS_RECORD_DATA pData
    )
{
    DWORD dwError = 0;
    DWORD idx = 0;
    const DWORD ip6AddrLength = 16;
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
        for (; idx < ip6AddrLength; ++idx)
        {
            dwError = VmDnsReadUCharFromBuffer(
                                      pVmDnsBuffer,
                                      &pData->AAAA.Ip6Address.IP6Byte[idx]
                                      );
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }


cleanup:

    return dwError;
error:

    goto cleanup;
}
