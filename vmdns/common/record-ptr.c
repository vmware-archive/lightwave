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
VmDnsComparePtrRecord(
    PVMDNS_RECORD   pRecord1,
    PVMDNS_RECORD   pRecord2
    )
{
    return VmDnsCompareRecordCommon(pRecord1, pRecord2) &&
        VmDnsStringCompareA(pRecord1->Data.PTR.pNameHost,
                        pRecord2->Data.PTR.pNameHost, FALSE) == 0;
}

VOID
VmDnsClearPtrRecord(
    PVMDNS_RECORD   pRecord
    )
{
    if (pRecord)
    {
        VMDNS_SAFE_FREE_STRINGA(pRecord->pszName);
        VMDNS_SAFE_FREE_MEMORY(pRecord->Data.PTR.pNameHost);
    }
}

VOID
VmDnsRpcClearPtrRecord(
    PVMDNS_RECORD   pRecord
    )
{
    if (pRecord)
    {
        DWORD rpcStatus = rpc_s_ok;
        rpc_string_free((PBYTE*)&pRecord->pszName, &rpcStatus);
        rpc_string_free((PBYTE*)&pRecord->Data.PTR.pNameHost, &rpcStatus);
    }
}

BOOLEAN
VmDnsValidatePtrRecord(
    PVMDNS_RECORD   pRecord
    )
{
    return VmDnsValidateRecordCommon(pRecord) &&
            VmDnsStringLenA(pRecord->Data.PTR.pNameHost) <= VMDNS_NAME_LENGTH_MAX;
}

DWORD
VmDnsDuplicatePtrRecord(
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

    dwError = VmDnsCopyPtrRecord(pSrc, pRecord);
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
VmDnsRpcDuplicatePtrRecord(
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

    dwError = VmDnsRpcCopyPtrRecord(pSrc, pRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppDest = pRecord;
    pRecord = NULL;

cleanup:
    return dwError;

error:
    VmDnsRpcClearPtrRecord(pRecord);
    VmDnsRpcFreeMemory(pRecord);
    if (ppDest)
    {
        *ppDest = NULL;
    }
    goto cleanup;
}

DWORD
VmDnsCopyPtrRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    )
{
    DWORD dwError = 0;
    PVMDNS_PTR_DATAA pData = NULL;

    pData = &pDest->Data.PTR;

    VmDnsAllocateStringA(pSrc->pszName, &pDest->pszName);
    BAIL_ON_VMDNS_ERROR(dwError);

    pDest->iClass = pSrc->iClass;
    pDest->dwTtl = pSrc->dwTtl;
    pDest->dwType = pSrc->dwType;

    VmDnsAllocateStringA(pSrc->Data.PTR.pNameHost, &pData->pNameHost);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    if (pDest)
    {
        VmDnsClearRecord(pDest);
    }

    goto cleanup;
}

DWORD
VmDnsRpcCopyPtrRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    )
{
    DWORD dwError = 0;
    PVMDNS_PTR_DATAA pData = NULL;

    pData = &pDest->Data.PTR;

    VmDnsRpcAllocateStringA(pSrc->pszName, &pDest->pszName);
    BAIL_ON_VMDNS_ERROR(dwError);

    pDest->iClass = pSrc->iClass;
    pDest->dwTtl = pSrc->dwTtl;
    pDest->dwType = pSrc->dwType;

    VmDnsRpcAllocateStringA(pSrc->Data.PTR.pNameHost, &pData->pNameHost);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    VmDnsRpcClearPtrRecord(pDest);

    goto cleanup;
}

DWORD
VmDnsPtrRecordToString(
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
                    "Server:        %s\n",
                    pszType,
                    pRecord->pszName,
                    pRecord->iClass,
                    pRecord->dwTtl,
                    pRecord->Data.PTR.pNameHost
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
VmDnsPtrRecordGetCN(
    PVMDNS_RECORD   pRecord,
    PSTR*           ppStr
    )
{
    return VmDnsAllocateStringA(pRecord->pszName, ppStr);
}

DWORD
VmDnsPtrRecordGetRDataLength(
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

    dwError = VmDnsGetDomainNameLength(
                                  Data.PTR.pNameHost,
                                  &uRdataLength,
                                  bTokenizeDomainName
                                  );
    BAIL_ON_VMDNS_ERROR(dwError);

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
VmDnsSerializeDnsPtrRecord(
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

    dwError = VmDnsPtrRecordGetRDataLength(
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

    dwError = VmDnsWriteDomainNameToBuffer(
                                 Data.PTR.pNameHost,
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
VmDnsDeserializeDnsPtrRecord(
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

    if (!dwRDataLength)
    {
        dwError = ERROR_EMPTY;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsReadDomainNameFromBuffer(
                             pVmDnsBuffer,
                             &pData->NS.pNameHost
                             );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsPtrRecordGetRDataLength(
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
