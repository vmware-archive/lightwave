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

// SOA
BOOLEAN
VmDnsCompareSoaRecord(
    PVMDNS_RECORD   pRecord1,
    PVMDNS_RECORD   pRecord2
    )
{
    return VmDnsCompareRecordCommon(pRecord1, pRecord2) &&
        VmDnsStringCompareA(pRecord1->Data.SOA.pNameAdministrator,
                        pRecord2->Data.SOA.pNameAdministrator, FALSE) == 0 &&
        VmDnsStringCompareA(pRecord1->Data.SOA.pNamePrimaryServer,
                        pRecord2->Data.SOA.pNamePrimaryServer, FALSE) == 0 &&
        pRecord1->Data.SOA.dwDefaultTtl == pRecord2->Data.SOA.dwDefaultTtl &&
        pRecord1->Data.SOA.dwExpire == pRecord2->Data.SOA.dwExpire &&
        pRecord1->Data.SOA.dwRefresh == pRecord2->Data.SOA.dwRefresh &&
        pRecord1->Data.SOA.dwRetry == pRecord2->Data.SOA.dwRetry &&
        pRecord1->Data.SOA.dwSerialNo == pRecord2->Data.SOA.dwSerialNo;
}

BOOLEAN
VmDnsMatchSoaRecord(
    PVMDNS_RECORD   pRecord1,
    PVMDNS_RECORD   pRecord2
    )
{
    return  VmDnsMatchRecordCommon(pRecord1, pRecord2) &&
        VmDnsStringCompareA(pRecord1->Data.SOA.pNameAdministrator,
                        pRecord2->Data.SOA.pNameAdministrator, FALSE) == 0 &&
        VmDnsStringCompareA(pRecord1->Data.SOA.pNamePrimaryServer,
                        pRecord2->Data.SOA.pNamePrimaryServer, FALSE) == 0 &&
        pRecord1->Data.SOA.dwDefaultTtl == pRecord2->Data.SOA.dwDefaultTtl &&
        pRecord1->Data.SOA.dwExpire == pRecord2->Data.SOA.dwExpire &&
        pRecord1->Data.SOA.dwRefresh == pRecord2->Data.SOA.dwRefresh &&
        pRecord1->Data.SOA.dwRetry == pRecord2->Data.SOA.dwRetry &&
        pRecord1->Data.SOA.dwSerialNo == pRecord2->Data.SOA.dwSerialNo;
}

VOID
VmDnsClearSoaRecord(
    PVMDNS_RECORD       pRecord
    )
{
    if (pRecord)
    {
        VMDNS_SAFE_FREE_STRINGA(pRecord->pszName);
        VMDNS_SAFE_FREE_MEMORY(pRecord->Data.SOA.pNamePrimaryServer);
        VMDNS_SAFE_FREE_MEMORY(pRecord->Data.SOA.pNameAdministrator);
    }
}

VOID
VmDnsRpcClearSoaRecord(
    PVMDNS_RECORD       pRecord
    )
{
    if (pRecord)
    {
        DWORD rpcStatus = rpc_s_ok;
        rpc_string_free((PBYTE*)&pRecord->pszName, &rpcStatus);
        rpc_string_free((PBYTE*)&pRecord->Data.SOA.pNamePrimaryServer, &rpcStatus);
        rpc_string_free((PBYTE*)&pRecord->Data.SOA.pNameAdministrator, &rpcStatus);
    }
}

BOOLEAN
VmDnsValidateSoaRecord(
    PVMDNS_RECORD   pRecord
    )
{
    return VmDnsValidateRecordCommon(pRecord) &&
            VmDnsStringLenA(pRecord->Data.SOA.pNameAdministrator) <= VMDNS_NAME_LENGTH_MAX &&
            VmDnsStringLenA(pRecord->Data.SOA.pNamePrimaryServer) <= VMDNS_NAME_LENGTH_MAX;
}

DWORD VmDnsDuplicateSoaRecord(
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

    dwError = VmDnsCopySoaRecord(pSrc, pRecord);
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

DWORD VmDnsRpcDuplicateSoaRecord(
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

    dwError = VmDnsRpcCopySoaRecord(pSrc, pRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppDest = pRecord;
    pRecord = NULL;

cleanup:
    return dwError;

error:
    VmDnsRpcClearSoaRecord(pRecord);
    VmDnsRpcFreeMemory(pRecord);
    if (ppDest)
    {
        *ppDest = NULL;
    }

    goto cleanup;
}

DWORD VmDnsCopySoaRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    )
{
    DWORD dwError = 0;
    PVMDNS_SOA_DATAA pData = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pSrc, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pDest, dwError);

    pData = &pDest->Data.SOA;

    VmDnsAllocateStringA(pSrc->pszName, &pDest->pszName);
    BAIL_ON_VMDNS_ERROR(dwError);

    pDest->iClass = pSrc->iClass;
    pDest->dwTtl = pSrc->dwTtl;
    pDest->dwType = pSrc->dwType;
    pData->dwDefaultTtl = pSrc->Data.SOA.dwDefaultTtl;
    pData->dwExpire = pSrc->Data.SOA.dwExpire;
    pData->dwRefresh = pSrc->Data.SOA.dwRefresh;
    pData->dwRetry = pSrc->Data.SOA.dwRetry;
    pData->dwSerialNo = pSrc->Data.SOA.dwSerialNo;

    VmDnsAllocateStringA(pSrc->Data.SOA.pNameAdministrator,
                         &pData->pNameAdministrator);
    BAIL_ON_VMDNS_ERROR(dwError);

    VmDnsAllocateStringA(pSrc->Data.SOA.pNamePrimaryServer,
                         &pData->pNamePrimaryServer);
    BAIL_ON_VMDNS_ERROR(dwError);
cleanup:
    return dwError;

error:
    VmDnsClearRecord(pDest);

    goto cleanup;
}

DWORD VmDnsRpcCopySoaRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    )
{
    DWORD dwError = 0;
    PVMDNS_SOA_DATAA pData = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pSrc, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pDest, dwError);

    pData = &pDest->Data.SOA;

    VmDnsRpcAllocateStringA(pSrc->pszName, &pDest->pszName);
    BAIL_ON_VMDNS_ERROR(dwError);

    pDest->iClass = pSrc->iClass;
    pDest->dwTtl = pSrc->dwTtl;
    pDest->dwType = pSrc->dwType;
    pData->dwDefaultTtl = pSrc->Data.SOA.dwDefaultTtl;
    pData->dwExpire = pSrc->Data.SOA.dwExpire;
    pData->dwRefresh = pSrc->Data.SOA.dwRefresh;
    pData->dwRetry = pSrc->Data.SOA.dwRetry;
    pData->dwSerialNo = pSrc->Data.SOA.dwSerialNo;

    VmDnsRpcAllocateStringA(pSrc->Data.SOA.pNameAdministrator,
                         &pData->pNameAdministrator);
    BAIL_ON_VMDNS_ERROR(dwError);

    VmDnsRpcAllocateStringA(pSrc->Data.SOA.pNamePrimaryServer,
                         &pData->pNamePrimaryServer);
    BAIL_ON_VMDNS_ERROR(dwError);
cleanup:
    return dwError;

error:
    VmDnsClearRecord(pDest);

    goto cleanup;
}

DWORD
VmDnsSoaRecordToString(
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
                    "Name:          %s\n"
                    "Type:          %s\n"
                    "Class:         %hu\n"
                    "Default TTL:   %u\n"
                    "Expire:        %u\n"
                    "Retry:         %u\n"
                    "Serial:        %u\n",
                    pRecord->pszName,
                    pszType,
                    pRecord->iClass,
                    pRecord->Data.SOA.dwDefaultTtl,
                    pRecord->Data.SOA.dwExpire,
                    pRecord->Data.SOA.dwRefresh,
                    pRecord->Data.SOA.dwSerialNo
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
VmDnsSoaRecordGetCN(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    )
{
    return VmDnsAllocateStringA(pRecord->pszName, ppStr);
}

DWORD
VmDnsSoaGetRDataLength(
    VMDNS_RECORD_DATA   Data,
    PUINT16             puRDataLength,
    BOOL                bTokenizeDomainName
    )
{
    DWORD dwError = 0;
    UINT16 uRDataLength = 0;
    UINT16 uPrimaryNameServerLength = 0;
    UINT16 uSecondaryNameServerLength = 9;

    if (!puRDataLength)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsGetDomainNameLength(
                              Data.SOA.pNamePrimaryServer,
                              &uPrimaryNameServerLength,
                              bTokenizeDomainName
                              );
    BAIL_ON_VMDNS_ERROR(dwError);

    uRDataLength += uPrimaryNameServerLength;

    dwError = VmDnsGetDomainNameLength(
                              Data.SOA.pNameAdministrator,
                              &uSecondaryNameServerLength,
                              bTokenizeDomainName
                              );
    BAIL_ON_VMDNS_ERROR(dwError);

    uRDataLength += uSecondaryNameServerLength;

    uRDataLength += sizeof(UINT32); //dwSerialNo

    uRDataLength += sizeof(UINT32); //dwRefresh

    uRDataLength += sizeof(UINT32); //dwRetry

    uRDataLength += sizeof(UINT32); //dwExpire

    uRDataLength += sizeof(UINT32); //dwDefaultTtl

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
VmDnsSerializeDnsSoaRecord(
    VMDNS_RECORD_DATA Data,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    )
{
    DWORD dwError = 0;
    UINT16 uTotalDataLength = 0;

    if (!pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsSoaGetRDataLength(
                Data,
                &uTotalDataLength,
                pVmDnsBuffer->bTokenizeDomainName);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT16ToBuffer(
                              uTotalDataLength,
                              pVmDnsBuffer
                              );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteDomainNameToBuffer(
                              Data.SOA.pNamePrimaryServer,
                              pVmDnsBuffer,
                              pVmDnsBuffer->bTokenizeDomainName
                              );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteDomainNameToBuffer(
                              Data.SOA.pNameAdministrator,
                              pVmDnsBuffer,
                              pVmDnsBuffer->bTokenizeDomainName
                              );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT32ToBuffer(
                              Data.SOA.dwSerialNo,
                              pVmDnsBuffer
                              );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT32ToBuffer(
                              Data.SOA.dwRefresh,
                              pVmDnsBuffer
                              );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT32ToBuffer(
                              Data.SOA.dwRetry,
                              pVmDnsBuffer
                              );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT32ToBuffer(
                              Data.SOA.dwExpire,
                              pVmDnsBuffer
                              );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT32ToBuffer(
                              Data.SOA.dwDefaultTtl,
                              pVmDnsBuffer
                              );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsDeserializeDnsSoaRecord(
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

    dwError = VmDnsReadDomainNameFromBuffer(
                                  pVmDnsBuffer,
                                  &pData->SOA.pNamePrimaryServer
                                  );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadDomainNameFromBuffer(
                                  pVmDnsBuffer,
                                  &pData->SOA.pNameAdministrator
                                  );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadUINT32FromBuffer(
                                  pVmDnsBuffer,
                                  &pData->SOA.dwSerialNo
                                  );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadUINT32FromBuffer(
                                  pVmDnsBuffer,
                                  &pData->SOA.dwRefresh
                                  );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadUINT32FromBuffer(
                                  pVmDnsBuffer,
                                  &pData->SOA.dwRetry
                                  );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadUINT32FromBuffer(
                                  pVmDnsBuffer,
                                  &pData->SOA.dwExpire
                                  );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadUINT32FromBuffer(
                                  pVmDnsBuffer,
                                  &pData->SOA.dwDefaultTtl
                                  );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSoaGetRDataLength(
                *pData,
                &uReceivedRDataLength,
                pVmDnsBuffer->bTokenizeDomainName);
    BAIL_ON_VMDNS_ERROR(dwError);


cleanup:

    return dwError;
error:

    goto cleanup;
}
