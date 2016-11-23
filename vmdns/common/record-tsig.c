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

// TSIG
BOOLEAN
VmDnsCompareTsigRecord(
    PVMDNS_RECORD   pRecord1,
    PVMDNS_RECORD   pRecord2
    )
{
    return FALSE;
}

VOID
VmDnsClearTsigRecord(
    PVMDNS_RECORD       pRecord
    )
{
    if (pRecord)
    {
        VMDNS_SAFE_FREE_STRINGA(pRecord->pszName);
        VMDNS_SAFE_FREE_MEMORY(pRecord->Data.TSIG.pNameAlgorithm);
        VmDnsFreeBlob(pRecord->Data.TSIG.pSignature);
        VmDnsFreeBlob(pRecord->Data.TSIG.pOtherData);
    }
}

VOID
VmDnsRpcClearTsigRecord(
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
                (PBYTE*)&pRecord->Data.TSIG.pNameAlgorithm,
                &rpcStatus
                );
        VmDnsRpcFreeBlob(pRecord->Data.TSIG.pSignature);
        VmDnsRpcFreeBlob(pRecord->Data.TSIG.pOtherData);
    }
}

BOOLEAN
VmDnsValidateTsigRecord(
    PVMDNS_RECORD   pRecord
    )
{
    return  VmDnsStringLenA(pRecord->pszName) <= VMDNS_NAME_LENGTH_MAX &&
            pRecord->dwType == VMDNS_RR_MTYPE_TSIG &&
            pRecord->iClass == VMDNS_CLASS_ANY &&
            pRecord->dwTtl == 0 &&
            VmDnsStringLenA(pRecord->Data.TSIG.pNameAlgorithm) <= VMDNS_NAME_LENGTH_MAX;
}

DWORD
VmDnsDuplicateTsigRecord(
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

    dwError = VmDnsCopyTsigRecord(pSrc, pRecord);
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
VmDnsRpcDuplicateTsigRecord(
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

    dwError = VmDnsRpcCopyTsigRecord(
                        pSrc,
                        pRecord
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppDest = pRecord;

cleanup:
    return dwError;

error:
    VmDnsRpcClearTsigRecord(pRecord);
    VmDnsRpcFreeMemory(pRecord);
    if (ppDest)
    {
        *ppDest = NULL;
    }

    goto cleanup;
}

DWORD
VmDnsCopyTsigRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    )
{
    DWORD dwError = 0;
    PVMDNS_TSIG_DATAA pData = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pSrc, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pDest, dwError);

    pData = &pDest->Data.TSIG;

    dwError = VmDnsAllocateStringA(
                        pSrc->pszName,
                        &pDest->pszName
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    pDest->dwType = pSrc->dwType;
    pDest->iClass = pSrc->iClass;
    pDest->dwTtl = pSrc->dwTtl;

    dwError = VmDnsAllocateStringA(
                        pSrc->Data.TSIG.pNameAlgorithm,
                        &pData->pNameAlgorithm
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyBlob(
                    pSrc->Data.TSIG.pSignature,
                    &pData->pSignature
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyBlob(
                    pSrc->Data.TSIG.pOtherData,
                    &pData->pOtherData
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    pData->unCreateTime = pSrc->Data.TSIG.unCreateTime;
    pData->wFudgeTime = pSrc->Data.TSIG.wFudgeTime;
    pData->wOriginalXid = pSrc->Data.TSIG.wOriginalXid;
    pData->wError = pSrc->Data.TSIG.wError;


cleanup:
    return dwError;

error:
    VmDnsClearRecord(pDest);

    goto cleanup;
}

DWORD
VmDnsRpcCopyTsigRecord(
    PVMDNS_RECORD   pSrc,
    PVMDNS_RECORD   pDest
    )
{
    DWORD dwError = 0;
    PVMDNS_TSIG_DATAA pData = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(pSrc, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pDest, dwError);

    pData = &pDest->Data.TSIG;

    dwError = VmDnsRpcAllocateStringA(
                        pSrc->pszName,
                        &pDest->pszName
                        );
    BAIL_ON_VMDNS_ERROR(dwError);


    pDest->dwType = pSrc->dwType;
    pDest->iClass = pSrc->iClass;
    pDest->dwTtl = pSrc->dwTtl;

    dwError = VmDnsRpcAllocateStringA(
                        pSrc->Data.TSIG.pNameAlgorithm,
                        &pData->pNameAlgorithm
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRpcCopyBlob(
                    pSrc->Data.TSIG.pSignature,
                    &pData->pSignature
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsRpcCopyBlob(
                    pSrc->Data.TSIG.pOtherData,
                    &pData->pOtherData
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    pData->unCreateTime = pSrc->Data.TSIG.unCreateTime;
    pData->wFudgeTime = pSrc->Data.TSIG.wFudgeTime;
    pData->wOriginalXid = pSrc->Data.TSIG.wOriginalXid;
    pData->wError = pSrc->Data.TSIG.wError;


cleanup:
    return dwError;

error:
    VmDnsRpcClearRecord(pDest);

    goto cleanup;
}

DWORD
VmDnsTsigRecordToString(
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
                    "Signed:            %s\n"
                    "Fudge:             %u\n"
                    "Sig Length:        %u\n"
                    // TODO: Add way of printing blob
                    "Sig                \n"
                    "Error:             %u\n",
                    pRecord->pszName,
                    pszType,
                    pRecord->Data.TSIG.pNameAlgorithm,
                    pRecord->Data.TSIG.unCreateTime,
                    pRecord->Data.TSIG.wFudgeTime,
                    pRecord->Data.TSIG.pSignature->unSize,
                    pRecord->Data.TSIG.wError
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
VmDnsTsigRecordGetCN(
    PVMDNS_RECORD       pRecord,
    PSTR*               ppStr
    )
{
    return VmDnsAllocateStringA(pRecord->pszName, ppStr);
}

DWORD
VmDnsTsigRecordGetRDataLength(
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
                            Data.TSIG.pNameAlgorithm,
                            &uNameAlgorithm,
                            bTokenizeDomainName
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    uRDataLength += uNameAlgorithm;

    uRDataLength += Data.TSIG.pSignature->unSize; //pSignature

    uRDataLength += Data.TSIG.pOtherData->unSize; //pOtherData

    uRDataLength += sizeof(UINT32); //unCreateTime

    uRDataLength += sizeof(UINT32); //wFudgeTime

    uRDataLength += sizeof(UINT16); //wSignatureLength

    uRDataLength += sizeof(UINT16); //wOriginalXid

    uRDataLength += sizeof(UINT16); //wError

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
VmDnsSerializeDnsTsigRecord(
    VMDNS_RECORD_DATA Data,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    )
{
    DWORD dwError = 0;
    UINT64 unTsigCombinedTimes = 0;
    UINT16 uTotalDataLength = 0;

    if (!pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsTsigRecordGetRDataLength(
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
                        Data.TSIG.pNameAlgorithm,
                        pVmDnsBuffer,
                        pVmDnsBuffer->bTokenizeDomainName
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    unTsigCombinedTimes = VMDNS_TSIG_COMBINE_TIMES(
                                    Data.TSIG.unCreateTime,
                                    Data.TSIG.wFudgeTime
                                    );

    dwError = VmDnsWriteUINT64ToBuffer(
                        unTsigCombinedTimes,
                        pVmDnsBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteBlobToBuffer(
                        Data.TSIG.pSignature,
                        TRUE,
                        pVmDnsBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT16ToBuffer(
                        Data.TSIG.wOriginalXid,
                        pVmDnsBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteUINT16ToBuffer(
                        Data.TSIG.wError,
                        pVmDnsBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsWriteBlobToBuffer(
                        Data.TSIG.pOtherData,
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
VmDnsDeserializeDnsTsigRecord(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PVMDNS_RECORD_DATA pData
    )
{
    DWORD dwError = 0;
    UINT64 unTsigCombinedTimes = 0;
    UINT16 dwRDataLength = 0;
    UINT16 uReceivedRDataLength = 0;

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
                            &pData->TSIG.pNameAlgorithm
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadUINT64FromBuffer(
                            pVmDnsBuffer,
                            &unTsigCombinedTimes
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    pData->TSIG.unCreateTime = VMDNS_TSIG_GET_CREATE_TIME(unTsigCombinedTimes);
    pData->TSIG.wFudgeTime = VMDNS_TSIG_GET_FUDGE_TIME(unTsigCombinedTimes);

    dwError = VmDnsReadBlobFromBuffer(
                            pVmDnsBuffer,
                            &pData->TSIG.pSignature
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadUINT16FromBuffer(
                            pVmDnsBuffer,
                            &pData->TSIG.wOriginalXid
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadUINT16FromBuffer(
                            pVmDnsBuffer,
                            &pData->TSIG.wError
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadBlobFromBuffer(
                            pVmDnsBuffer,
                            &pData->TSIG.pOtherData
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsTsigRecordGetRDataLength(
                        *pData,
                        &uReceivedRDataLength,
                        pVmDnsBuffer->bTokenizeDomainName);
    BAIL_ON_VMDNS_ERROR(dwError);


cleanup:

    return dwError;
error:

    goto cleanup;
}
