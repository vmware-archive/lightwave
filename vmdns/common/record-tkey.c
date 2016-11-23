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

// TKEY
BOOLEAN
VmDnsCompareTkeyRecord(
    PVMDNS_RECORD   pRecord1,
    PVMDNS_RECORD   pRecord2
    )
{
    return FALSE;
}

VOID
VmDnsClearTkeyRecord(
    PVMDNS_RECORD       pRecord
    )
{
    if (pRecord)
    {
        VMDNS_SAFE_FREE_STRINGA(pRecord->pszName);
        VMDNS_SAFE_FREE_STRINGA(pRecord->Data.TKEY.pNameAlgorithm);
        VmDnsFreeBlob(pRecord->Data.TKEY.pKey);
        VmDnsFreeBlob(pRecord->Data.TKEY.pOtherData);
    }
}

VOID
VmDnsRpcClearTkeyRecord(
    PVMDNS_RECORD       pRecord
    )
{
    if (pRecord)
    {
        DWORD rpcStatus = rpc_s_ok;

        rpc_string_free(
                (PBYTE*)&pRecord->pszName,
                &rpcStatus
                );
        rpc_string_free(
                (PBYTE*)&pRecord->Data.TKEY.pNameAlgorithm,
                &rpcStatus
                );
        VmDnsRpcFreeBlob(pRecord->Data.TKEY.pKey);
        VmDnsRpcFreeBlob(pRecord->Data.TKEY.pOtherData);
    }
}

BOOLEAN
VmDnsValidateTkeyRecord(
    PVMDNS_RECORD   pRecord
    )
{
    return  VmDnsStringLenA(pRecord->pszName) <= VMDNS_NAME_LENGTH_MAX &&
            pRecord->dwType == VMDNS_RR_MTYPE_TKEY &&
            pRecord->iClass == VMDNS_CLASS_ANY &&
            pRecord->dwTtl == 0 &&
            VmDnsStringLenA(pRecord->Data.TKEY.pNameAlgorithm) <= VMDNS_NAME_LENGTH_MAX;
}

DWORD
VmDnsDuplicateTkeyRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    )
{
    DWORD            dwError = 0;
    PVMDNS_RECORD    pRecord = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pSrc, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppDest, dwError);

    dwError = VmDnsAllocateMemory(
                        sizeof(VMDNS_RECORD),
                        (void**)&pRecord
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyTkeyRecord(pSrc, pRecord);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppDest = pRecord;

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
VmDnsRpcDuplicateTkeyRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   *ppDest
    )
{
    DWORD            dwError = 0;
    PVMDNS_RECORD    pRecord = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pSrc, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppDest, dwError);

    dwError = VmDnsRpcAllocateMemory(
                        sizeof(VMDNS_RECORD),
                        (PVOID*)&pRecord
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRpcCopyTkeyRecord(
                        pSrc,
                        pRecord
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppDest = pRecord;

cleanup:
    return dwError;

error:
    VmDnsRpcClearTkeyRecord(pRecord);
    VmDnsRpcFreeMemory(pRecord);
    if (ppDest)
    {
        *ppDest = NULL;
    }

    goto cleanup;
}

DWORD
VmDnsCopyTkeyRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    )
{
    DWORD dwError = 0;
    PVMDNS_TKEY_DATAA pData = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pSrc, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pDest, dwError);

    pData = &pDest->Data.TKEY;

    dwError = VmDnsAllocateStringA(
                        pSrc->pszName,
                        &pDest->pszName
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    pDest->dwType = pSrc->dwType;
    pDest->iClass = pSrc->iClass;
    pDest->dwTtl = pSrc->dwTtl;

    dwError = VmDnsAllocateStringA(
                        pSrc->Data.TKEY.pNameAlgorithm,
                        &pData->pNameAlgorithm
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyBlob(
                    pSrc->Data.TKEY.pKey,
                    &pData->pKey
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyBlob(
                    pSrc->Data.TKEY.pOtherData,
                    &pData->pOtherData
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    pData->dwCreateTime = pSrc->Data.TKEY.dwCreateTime;
    pData->dwExpireTime = pSrc->Data.TKEY.dwExpireTime;
    pData->wMode = pSrc->Data.TKEY.wMode;
    pData->wError = pSrc->Data.TKEY.wError;


cleanup:
    return dwError;

error:
    VmDnsClearRecord(pDest);

    goto cleanup;
}

DWORD
VmDnsRpcCopyTkeyRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    )
{
    DWORD dwError = 0;
    PVMDNS_TKEY_DATAA pData = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pSrc, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pDest, dwError);

    pData = &pDest->Data.TKEY;

    dwError = VmDnsRpcAllocateStringA(
                        pSrc->pszName,
                        &pDest->pszName
                        );
    BAIL_ON_VMDNS_ERROR(dwError);


    pDest->dwType = pSrc->dwType;
    pDest->iClass = pSrc->iClass;
    pDest->dwTtl = pSrc->dwTtl;

    dwError = VmDnsRpcAllocateStringA(
                        pSrc->Data.TKEY.pNameAlgorithm,
                        &pData->pNameAlgorithm
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRpcCopyBlob(
                    pSrc->Data.TKEY.pKey,
                    &pData->pKey
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRpcCopyBlob(
                    pSrc->Data.TKEY.pOtherData,
                    &pData->pOtherData
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    pData->dwCreateTime = pSrc->Data.TKEY.dwCreateTime;
    pData->dwExpireTime = pSrc->Data.TKEY.dwExpireTime;
    pData->wMode = pSrc->Data.TKEY.wMode;
    pData->wError = pSrc->Data.TKEY.wError;


cleanup:
    return dwError;

error:
    VmDnsRpcClearRecord(pDest);

    goto cleanup;
}

DWORD
VmDnsTkeyRecordToString(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PSTR  pStr = NULL;
    PCSTR pszType = NULL;

    dwError = VmDnsRecordTypeToString(
                        pRecord->dwType,
                        &pszType
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateStringPrintfA(
                    &pStr,
                    "Name:              %s\n"
                    "Type:              %s\n"
                    "Algorithm:         %s\n"
                    "Inception:         %u\n"
                    "Expiration:        %u\n"
                    "Mode:              %u\n"
                    "Error:             %u\n",
                    pRecord->pszName,
                    pszType,
                    pRecord->Data.TKEY.pNameAlgorithm,
                    pRecord->Data.TKEY.dwCreateTime,
                    pRecord->Data.TKEY.dwExpireTime,
                    pRecord->Data.TKEY.wMode,
                    pRecord->Data.TKEY.wError
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
VmDnsTkeyRecordGetCN(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    )
{
    return VmDnsAllocateStringA(pRecord->pszName, ppStr);
}

DWORD
VmDnsTkeyRecordGetRDataLength(
    VMDNS_RECORD_DATA   Data,
    PUINT16             puRDataLength,
    BOOL                bTokenizeDomainName
    )
{
    DWORD dwError = 0;
    UINT16 uRDataLength = 0;
    UINT16 uNameAlgorithm = 0;

    if (!puRDataLength)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsGetDomainNameLength(
                            Data.TKEY.pNameAlgorithm,
                            &uNameAlgorithm,
                            bTokenizeDomainName
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    uRDataLength += uNameAlgorithm;

    uRDataLength += Data.TKEY.pKey->unSize; //pKey

    uRDataLength += Data.TKEY.pOtherData->unSize; //pOtherData

    uRDataLength += sizeof(UINT32); //dwCreateTime

    uRDataLength += sizeof(UINT32); //dwExpireTime

    uRDataLength += sizeof(UINT16); //wMode

    uRDataLength += sizeof(UINT16); //wError

    uRDataLength += sizeof(UINT16); //wKeyLength

    uRDataLength += sizeof(UINT16); //wOtherLength

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
VmDnsSerializeDnsTkeyRecord(
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

    dwError = VmDnsTkeyRecordGetRDataLength(
                        Data,
                        &uTotalDataLength,
                        pVmDnsBuffer->bTokenizeDomainName
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT16ToBuffer(
                        uTotalDataLength,
                        pVmDnsBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteDomainNameToBuffer(
                        Data.TKEY.pNameAlgorithm,
                        pVmDnsBuffer,
                        pVmDnsBuffer->bTokenizeDomainName
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT32ToBuffer(
                        Data.TKEY.dwCreateTime,
                        pVmDnsBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT32ToBuffer(
                        Data.TKEY.dwExpireTime,
                        pVmDnsBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT16ToBuffer(
                        Data.TKEY.wMode,
                        pVmDnsBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT16ToBuffer(
                        Data.TKEY.wError,
                        pVmDnsBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteBlobToBuffer(
                        Data.TKEY.pKey,
                        TRUE,
                        pVmDnsBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteBlobToBuffer(
                        Data.TKEY.pOtherData,
                        TRUE,
                        pVmDnsBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);


cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsDeserializeDnsTkeyRecord(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PVMDNS_RECORD_DATA pData
    )
{
    DWORD dwError = 0;
    UINT16 dwRDataLength = 0;
    //UINT16 uReceivedRDataLength = 0;

    if (!pVmDnsBuffer || !pData)
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
                            &pData->TKEY.pNameAlgorithm
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadUINT32FromBuffer(
                            pVmDnsBuffer,
                            &pData->TKEY.dwCreateTime
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadUINT32FromBuffer(
                            pVmDnsBuffer,
                            &pData->TKEY.dwExpireTime
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadUINT16FromBuffer(
                            pVmDnsBuffer,
                            &pData->TKEY.wMode
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadUINT16FromBuffer(
                            pVmDnsBuffer,
                            &pData->TKEY.wError
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadBlobFromBuffer(
                            pVmDnsBuffer,
                            &pData->TKEY.pKey
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadBlobFromBuffer(
                            pVmDnsBuffer,
                            &pData->TKEY.pOtherData
                            );
    BAIL_ON_VMDNS_ERROR(dwError);


cleanup:

    return dwError;
error:

    goto cleanup;
}
