/*
 * Copyright (c) VMware Inc.  All rights Reserved.
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

    dwError = VmDnsRecordTypeToString(pRecord->dwType, &pszType);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringPrintfA(
                    &pStr,
                    "Type:          %s\n"
                    "Name:          %s\n"
                    "Class:         %hu\n"
                    "TTL:           %u\n"
                    "Address:       %hu.%hu.%hu.%hu\n",
                    pszType,
                    pRecord->pszName,
                    pRecord->iClass,
                    pRecord->dwTtl,
                    (unsigned short)((pRecord->Data.A.IpAddress >> 24) & 0xFF),
                    (unsigned short)((pRecord->Data.A.IpAddress >> 16) & 0xFF),
                    (unsigned short)((pRecord->Data.A.IpAddress >> 8) & 0xFF),
                    (unsigned short)((pRecord->Data.A.IpAddress) & 0xFF)
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
VmDnsSerializeDnsAddressRecord(
    VMDNS_RECORD_DATA Data,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    )
{
    DWORD dwError = 0;
    if (!pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsWriteUINT16ToBuffer(
                               sizeof(VMDNS_A_DATA),
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

    if (dwRDataLength < sizeof(VMDNS_A_DATA))
    {
        dwError = ERROR_INSUFFICIENT_BUFFER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsReadUINT32FromBuffer(
                              pVmDnsBuffer,
                              &pData->A.IpAddress
                              );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}
