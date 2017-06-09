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

/*
 * Module Name:  buffer.c
 *
 * Abstract: Common buffer handling functions.
 *
 */

#include "includes.h"

static
DWORD
VmDnsAdjustMemoryBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    size_t szDataSize
    );

static
DWORD
VmDnsCheckMemory(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    size_t szDataSize
    );

static
BOOL
VmDnsIsBigEndian(
    VOID
    );

static
DWORD
VmDnsNToH(
    PBYTE pInData,
    size_t szData,
    PBYTE pOutData
    );

static
DWORD
VmDnsHToN(
    PBYTE pInData,
    size_t szData,
    PBYTE pOutData
    );

DWORD
VmDnsAllocateBufferStream(
    size_t dwMaxSize,
    PVMDNS_MESSAGE_BUFFER *ppVmDnsBuffer
    )
{
    DWORD dwError = 0;
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer = NULL;

    if (!ppVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (dwMaxSize > VMDNS_MAX_SIZE_BUFFER)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(
                    sizeof(VMDNS_MESSAGE_BUFFER),
                    (PVOID *)&pVmDnsBuffer
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    pVmDnsBuffer->szMaxSize = dwMaxSize? dwMaxSize : VMDNS_MAX_SIZE_BUFFER;

    pVmDnsBuffer->bCanWrite = TRUE;

    *ppVmDnsBuffer = pVmDnsBuffer;

cleanup:

    return dwError;
error:
    if (ppVmDnsBuffer)
    {
        *ppVmDnsBuffer = NULL;
    }
    VmDnsFreeBufferStream(pVmDnsBuffer);

    goto cleanup;
}

VOID
VmDnsFreeBufferStream(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    )
{
    if (pVmDnsBuffer)
    {
        if (pVmDnsBuffer->pMessage)
        {
            VMDNS_SAFE_FREE_MEMORY(pVmDnsBuffer->pMessage);
        }

        VMDNS_SAFE_FREE_MEMORY(pVmDnsBuffer);
    }
}

DWORD
VmDnsLockBufferForWrite(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    )
{
    DWORD dwError = 0;
    PBYTE pTempMessage = NULL;

    if (!pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsReallocateMemory(
                       pVmDnsBuffer->pMessage,
                       (PVOID *)&pTempMessage,
                       pVmDnsBuffer->szLength
                       );

    pVmDnsBuffer->bCanWrite = FALSE;
    pVmDnsBuffer->szCurrentSize = pVmDnsBuffer->szLength;
    pVmDnsBuffer->szCursor = 0;

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsAllocateBufferStreamWithBuffer(
    PBYTE pBuffer,
    size_t szBufSize,
    size_t szMaxSize,
    BOOL bCanWrite,
    PVMDNS_MESSAGE_BUFFER *ppVmDnsBuffer
    )
{
    DWORD dwError = 0;
    PBYTE pTempBuffer = NULL;
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer = NULL;

    if (!pBuffer || !szBufSize || !ppVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(
                       szBufSize,
                       (PVOID *)&pTempBuffer
                       );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyMemory(
                        pTempBuffer,
                        szBufSize,
                        pBuffer,
                        szBufSize
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateBufferStream(
                            szMaxSize,
                            &pVmDnsBuffer
                            );
    BAIL_ON_VMDNS_ERROR (dwError);

    pVmDnsBuffer->pMessage = pTempBuffer;
    pTempBuffer = NULL;
    pVmDnsBuffer->szLength = szBufSize;
    pVmDnsBuffer->szCurrentSize = szBufSize;

    pVmDnsBuffer->bCanWrite = bCanWrite;

    *ppVmDnsBuffer = pVmDnsBuffer;

cleanup:

    return dwError;
error:

    if (ppVmDnsBuffer)
    {
        *ppVmDnsBuffer = NULL;
    }
    VmDnsFreeBufferStream(pVmDnsBuffer);
    VMDNS_SAFE_FREE_MEMORY(pTempBuffer);

    goto cleanup;
}

DWORD
VmDnsCopyBufferFromBufferStream(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PBYTE pBuffer,
    PDWORD pdwBufferSize
    )
{
    DWORD dwError = 0;

    if (!pVmDnsBuffer ||
        !pdwBufferSize
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (pBuffer && pVmDnsBuffer->szLength)
    {
        dwError = VmDnsCopyMemory(
                          pBuffer,
                          *pdwBufferSize,
                          pVmDnsBuffer->pMessage,
                          pVmDnsBuffer->szLength
                          );
        BAIL_ON_VMDNS_ERROR (dwError);
    }

cleanup:

    if (pdwBufferSize && pVmDnsBuffer)
    {
        *pdwBufferSize = pVmDnsBuffer->szLength;
    }
    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsWriteBoolToBuffer(
    BOOL bData,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    )
{
    DWORD dwError = 0;
    BOOL bBoolData = TRUE;
    PBOOL pBoolCursor = NULL;

    if (!pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR (dwError);
    }
    if (!pVmDnsBuffer->bCanWrite)
    {
        dwError = ERROR_CANTWRITE;
        BAIL_ON_VMDNS_ERROR (dwError);
    }

    dwError = VmDnsAdjustMemoryBuffer(
                           pVmDnsBuffer,
                           sizeof(BOOL)
                           );
    BAIL_ON_VMDNS_ERROR (dwError);

    pBoolCursor = (PBOOL)(pVmDnsBuffer->pMessage + pVmDnsBuffer->szLength);

    dwError = VmDnsHToN(
                    (PBYTE)&bData,
                    sizeof(BOOL),
                    (PBYTE)&bBoolData
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyMemory(
                        pBoolCursor,
                        pVmDnsBuffer->szCurrentSize - pVmDnsBuffer->szLength,
                        &bBoolData,
                        sizeof(BOOL)
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    pVmDnsBuffer->szLength += sizeof(BOOL);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsWriteBooleanToBuffer(
    BOOLEAN bData,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    )
{
    DWORD dwError = 0;
    BOOLEAN bBooleanData = FALSE;
    PBOOLEAN pBooleanCursor = NULL;

    if (!pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR (dwError);
    }
    if (!pVmDnsBuffer->bCanWrite)
    {
        dwError = ERROR_CANTWRITE;
        BAIL_ON_VMDNS_ERROR (dwError);
    }

    dwError = VmDnsAdjustMemoryBuffer(
                           pVmDnsBuffer,
                           sizeof(BOOLEAN)
                           );
    BAIL_ON_VMDNS_ERROR (dwError);

    pBooleanCursor = (PBOOLEAN)
                     (pVmDnsBuffer->pMessage + pVmDnsBuffer->szLength);

    dwError = VmDnsHToN(
                      (PBYTE)&bData,
                      sizeof(BOOLEAN),
                      (PBYTE)&bBooleanData
                      );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyMemory(
                        pBooleanCursor,
                        pVmDnsBuffer->szCurrentSize - pVmDnsBuffer->szLength,
                        &bBooleanData,
                        sizeof(BOOLEAN)
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    pVmDnsBuffer->szLength += sizeof(BOOLEAN);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsWriteCharToBuffer(
    CHAR cData,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    )
{
    DWORD dwError = 0;
    PCHAR pCharCursor = NULL;

    if (!pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR (dwError);
    }
    if (!pVmDnsBuffer->bCanWrite)
    {
        dwError = ERROR_CANTWRITE;
        BAIL_ON_VMDNS_ERROR (dwError);
    }

    dwError = VmDnsAdjustMemoryBuffer(
                           pVmDnsBuffer,
                           sizeof(CHAR)
                           );
    BAIL_ON_VMDNS_ERROR (dwError);

    pCharCursor = (PCHAR)(pVmDnsBuffer->pMessage + pVmDnsBuffer->szLength);

    dwError = VmDnsCopyMemory(
                        pCharCursor,
                        pVmDnsBuffer->szCurrentSize - pVmDnsBuffer->szLength,
                        &cData,
                        sizeof(CHAR)
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    pVmDnsBuffer->szLength += sizeof(CHAR);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsWriteUCharToBuffer(
    UCHAR ucData,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    )
{
    DWORD dwError = 0;
    PUCHAR pCharCursor = NULL;

    if (!pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR (dwError);
    }
    if (!pVmDnsBuffer->bCanWrite)
    {
        dwError = ERROR_CANTWRITE;
        BAIL_ON_VMDNS_ERROR (dwError);
    }

    dwError = VmDnsAdjustMemoryBuffer(
                           pVmDnsBuffer,
                           sizeof(UCHAR)
                           );
    BAIL_ON_VMDNS_ERROR (dwError);

    pCharCursor = (PUCHAR)(pVmDnsBuffer->pMessage + pVmDnsBuffer->szLength);

    dwError = VmDnsCopyMemory(
                        pCharCursor,
                        pVmDnsBuffer->szCurrentSize - pVmDnsBuffer->szLength,
                        &ucData,
                        sizeof(UCHAR)
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    pVmDnsBuffer->szLength += sizeof(UCHAR);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsWriteUINT16ToBuffer(
    UINT16 uData,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    )
{
    DWORD dwError = 0;
    UINT16 uInt16Data = 0;
    PUINT16 pUint16Cursor = NULL;

    if (!pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR (dwError);
    }
    if (!pVmDnsBuffer->bCanWrite)
    {
        dwError = ERROR_CANTWRITE;
        BAIL_ON_VMDNS_ERROR (dwError);
    }

    dwError = VmDnsAdjustMemoryBuffer(
                           pVmDnsBuffer,
                           sizeof(UINT16)
                           );
    BAIL_ON_VMDNS_ERROR (dwError);

    pUint16Cursor = (PUINT16)(pVmDnsBuffer->pMessage + pVmDnsBuffer->szLength);

    uInt16Data = htons(uData);

    dwError = VmDnsCopyMemory(
                        pUint16Cursor,
                        pVmDnsBuffer->szCurrentSize - pVmDnsBuffer->szLength,
                        &uInt16Data,
                        sizeof(UINT16)
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    pVmDnsBuffer->szLength += sizeof(UINT16);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsWriteUINT32ToBuffer(
    UINT32 uData,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    )
{
    DWORD dwError = 0;
    UINT32 uInt32Data = 0;
    PUINT32 pUint32Cursor = NULL;

    if (!pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR (dwError);
    }
    if (!pVmDnsBuffer->bCanWrite)
    {
        dwError = ERROR_CANTWRITE;
        BAIL_ON_VMDNS_ERROR (dwError);
    }

    dwError = VmDnsAdjustMemoryBuffer(
                           pVmDnsBuffer,
                           sizeof(UINT32)
                           );
    BAIL_ON_VMDNS_ERROR (dwError);

    pUint32Cursor = (PUINT32)(pVmDnsBuffer->pMessage + pVmDnsBuffer->szLength);

    uInt32Data = htonl(uData);

    dwError = VmDnsCopyMemory(
                        pUint32Cursor,
                        pVmDnsBuffer->szCurrentSize - pVmDnsBuffer->szLength,
                        &uInt32Data,
                        sizeof(UINT32)
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    pVmDnsBuffer->szLength += sizeof(UINT32);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsWriteUINT64ToBuffer(
    UINT64 uData,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    )
{
    DWORD dwError = 0;
    UINT64 uIntData = 0;
    PUINT64 pUint64Cursor = NULL;

    if (!pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR (dwError);
    }
    if (!pVmDnsBuffer->bCanWrite)
    {
        dwError = ERROR_CANTWRITE;
        BAIL_ON_VMDNS_ERROR (dwError);
    }

    dwError = VmDnsAdjustMemoryBuffer(
                           pVmDnsBuffer,
                           sizeof(UINT64)
                           );
    BAIL_ON_VMDNS_ERROR (dwError);

    pUint64Cursor = (PUINT64)(pVmDnsBuffer->pMessage + pVmDnsBuffer->szLength);

    dwError = VmDnsHToN(
                      (PBYTE)&uData,
                      sizeof(UINT64),
                      (PBYTE)&uIntData
                      );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyMemory(
                         pUint64Cursor,
                         pVmDnsBuffer->szCurrentSize - pVmDnsBuffer->szLength,
                         &uIntData,
                         sizeof(UINT64)
                         );
    BAIL_ON_VMDNS_ERROR(dwError);

    pVmDnsBuffer->szLength += sizeof(UINT64);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsWriteINT16ToBuffer(
    INT16 iData,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    )
{
    DWORD dwError = 0;
    INT16 iInt16Data = 0;
    PINT16 pInt16Cursor = NULL;

    if (!pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR (dwError);
    }
    if (!pVmDnsBuffer->bCanWrite)
    {
        dwError = ERROR_CANTWRITE;
        BAIL_ON_VMDNS_ERROR (dwError);
    }

    dwError = VmDnsAdjustMemoryBuffer(
                           pVmDnsBuffer,
                           sizeof(INT16)
                           );
    BAIL_ON_VMDNS_ERROR (dwError);

    pInt16Cursor = (PINT16)(pVmDnsBuffer->pMessage + pVmDnsBuffer->szLength);

    iInt16Data = htons(iData);

    dwError = VmDnsCopyMemory(
                      pInt16Cursor,
                      pVmDnsBuffer->szCurrentSize - pVmDnsBuffer->szLength,
                      &iInt16Data,
                      sizeof(INT16)
                      );
    BAIL_ON_VMDNS_ERROR(dwError);

    pVmDnsBuffer->szLength += sizeof(INT16);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsWriteINT32ToBuffer(
    INT32 iData,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    )
{
    DWORD dwError = 0;
    INT32 iInt32Data = 0;
    PINT32 pInt32Cursor = NULL;

    if (!pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR (dwError);
    }
    if (!pVmDnsBuffer->bCanWrite)
    {
        dwError = ERROR_CANTWRITE;
        BAIL_ON_VMDNS_ERROR (dwError);
    }

    dwError = VmDnsAdjustMemoryBuffer(
                           pVmDnsBuffer,
                           sizeof(INT32)
                           );
    BAIL_ON_VMDNS_ERROR (dwError);

    pInt32Cursor = (PINT32)(pVmDnsBuffer->pMessage + pVmDnsBuffer->szLength);

    iInt32Data = htonl(iData);

    dwError = VmDnsCopyMemory(
                      pInt32Cursor,
                      pVmDnsBuffer->szCurrentSize - pVmDnsBuffer->szLength,
                      &iInt32Data,
                      sizeof(INT32)
                      );
    BAIL_ON_VMDNS_ERROR(dwError);

    pVmDnsBuffer->szLength += sizeof(INT32);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsWriteINT64ToBuffer(
    INT64 iData,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    )
{
    DWORD dwError = 0;
    INT64 iInt64Data = 0;
    PINT64 pInt64Cursor = NULL;

    if (!pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR (dwError);
    }
    if (!pVmDnsBuffer->bCanWrite)
    {
        dwError = ERROR_CANTWRITE;
        BAIL_ON_VMDNS_ERROR (dwError);
    }

    dwError = VmDnsAdjustMemoryBuffer(
                           pVmDnsBuffer,
                           sizeof(INT64)
                           );
    BAIL_ON_VMDNS_ERROR (dwError);

    pInt64Cursor = (PINT64)(pVmDnsBuffer->pMessage + pVmDnsBuffer->szLength);

    dwError = VmDnsHToN(
                      (PBYTE)&iData,
                      sizeof(INT64),
                      (PBYTE)&iInt64Data
                      );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyMemory(
                    pInt64Cursor,
                    pVmDnsBuffer->szCurrentSize - pVmDnsBuffer->szLength,
                    &iInt64Data,
                    sizeof(INT64)
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    pVmDnsBuffer->szLength += sizeof(INT64);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsWriteStringToBuffer(
    PSTR pszString,
    UINT8 uStringLength,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    )
{
    DWORD dwError = 0;
    size_t szDataSize = uStringLength + 1;
    PSTR pCursor = NULL;

    if (!pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (!pVmDnsBuffer->bCanWrite)
    {
        dwError = ERROR_CANTWRITE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAdjustMemoryBuffer(
                          pVmDnsBuffer,
                          szDataSize
                          );
    BAIL_ON_VMDNS_ERROR(dwError);

    pCursor = (PSTR)pVmDnsBuffer->pMessage + pVmDnsBuffer->szLength;

    pCursor[0] = uStringLength;

    if (pszString && uStringLength)
    {
        VmDnsCopyMemory(
                ++pCursor,
                pVmDnsBuffer->szCurrentSize - pVmDnsBuffer->szLength,
                pszString,
                uStringLength
                );
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pVmDnsBuffer->szLength += szDataSize;
cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsWriteBlobToBuffer(
    PVMDNS_BLOB pBlob,
    BOOL bWriteSize,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    )
{
    DWORD dwError = 0;
    PBYTE pCursor = NULL;

    if (!pVmDnsBuffer || !pBlob)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    if (!pVmDnsBuffer->bCanWrite)
    {
        dwError = ERROR_CANTWRITE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (bWriteSize)
    {
        dwError = VmDnsWriteUINT16ToBuffer(
                            pBlob->unSize,
                            pVmDnsBuffer
                            );
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (pBlob->unSize && !pBlob->pData)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        dwError = VmDnsAdjustMemoryBuffer(
                            pVmDnsBuffer,
                            pBlob->unSize
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        pCursor = pVmDnsBuffer->pMessage + pVmDnsBuffer->szLength;

        dwError = VmDnsCopyMemory(
                        pCursor,
                        pVmDnsBuffer->szCurrentSize - pVmDnsBuffer->szLength,
                        pBlob->pData,
                        pBlob->unSize
                        );
        BAIL_ON_VMDNS_ERROR(dwError);

        pVmDnsBuffer->szLength += pBlob->unSize;
    }


cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsReadBoolFromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PBOOL pbData
    )
{
    DWORD dwError = 0;
    BOOL bData = FALSE;
    PBYTE pCurrentPos = NULL;

    if (!pbData || !pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCheckMemory(pVmDnsBuffer,sizeof(BOOL));
    BAIL_ON_VMDNS_ERROR(dwError);

    pCurrentPos = pVmDnsBuffer->pMessage + pVmDnsBuffer->szCursor;

    dwError = VmDnsNToH(
                    pCurrentPos,
                    sizeof(BOOL),
                    (PBYTE)&bData
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    pVmDnsBuffer->szCursor += sizeof(BOOL);

    *pbData = bData;

cleanup:

    return dwError;
error:

    if (pbData)
    {
        *pbData = FALSE;
    }
    goto cleanup;
}

DWORD
VmDnsReadBooleanFromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PBOOLEAN pbData
    )
{
    DWORD dwError = 0;
    BOOLEAN bData = FALSE;
    PBYTE pCurrentPos = NULL;

    if (!pbData || !pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCheckMemory(pVmDnsBuffer,sizeof(BOOLEAN));
    BAIL_ON_VMDNS_ERROR(dwError);

    pCurrentPos = pVmDnsBuffer->pMessage + pVmDnsBuffer->szCursor;

    dwError = VmDnsNToH(
                    pCurrentPos,
                    sizeof(BOOLEAN),
                    &bData
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    pVmDnsBuffer->szCursor += sizeof(BOOLEAN);

    *pbData = bData;

cleanup:

    return dwError;
error:

    if (pbData)
    {
        *pbData = FALSE;
    }
    goto cleanup;
}

DWORD
VmDnsReadCharFromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PCHAR pcData
    )
{
    DWORD dwError = 0;
    CHAR cData = FALSE;
    PBYTE pCurrentPos = NULL;

    if (!pcData || !pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCheckMemory(pVmDnsBuffer,sizeof(CHAR));
    BAIL_ON_VMDNS_ERROR(dwError);

    pCurrentPos = pVmDnsBuffer->pMessage + pVmDnsBuffer->szCursor;

    cData = *(PCHAR)pCurrentPos;

    pVmDnsBuffer->szCursor += sizeof(CHAR);

    *pcData = cData;

cleanup:

    return dwError;
error:

    if (pcData)
    {
        *pcData = 0;
    }
    goto cleanup;
}


DWORD
VmDnsReadUCharFromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PUCHAR pucData
    )
{
    DWORD dwError = 0;
    UCHAR ucData = FALSE;
    PBYTE pCurrentPos = NULL;

    if (!pucData || !pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCheckMemory(pVmDnsBuffer,sizeof(UCHAR));
    BAIL_ON_VMDNS_ERROR(dwError);

    pCurrentPos = pVmDnsBuffer->pMessage + pVmDnsBuffer->szCursor;

    ucData = *(PUCHAR)pCurrentPos;

    pVmDnsBuffer->szCursor += sizeof(UCHAR);

    *pucData = ucData;

cleanup:

    return dwError;
error:

    if (pucData)
    {
        *pucData = 0;
    }
    goto cleanup;
}

DWORD
VmDnsReadUINT16FromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PUINT16 puData
    )
{
    DWORD dwError = 0;
    UINT16 uData = FALSE;
    PBYTE pCurrentPos = NULL;

    if (!puData || !pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCheckMemory(pVmDnsBuffer,sizeof(UINT16));
    BAIL_ON_VMDNS_ERROR(dwError);

    pCurrentPos = pVmDnsBuffer->pMessage + pVmDnsBuffer->szCursor;

    dwError = VmDnsCopyMemory(
                    &uData,
                    sizeof(uData),
                    pCurrentPos,
                    sizeof(UINT16)
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    pVmDnsBuffer->szCursor += sizeof(UINT16);

    *puData = ntohs(uData);

cleanup:

    return dwError;
error:

    if (puData)
    {
        *puData = 0;
    }
    goto cleanup;
}

DWORD
VmDnsReadUINT32FromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PUINT32 puData
    )
{
    DWORD dwError = 0;
    UINT32 uData = FALSE;
    PBYTE pCurrentPos = NULL;

    if (!puData || !pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCheckMemory(pVmDnsBuffer,sizeof(UINT32));
    BAIL_ON_VMDNS_ERROR(dwError);

    pCurrentPos = pVmDnsBuffer->pMessage + pVmDnsBuffer->szCursor;

    dwError = VmDnsCopyMemory(
                    &uData,
                    sizeof(uData),
                    pCurrentPos,
                    sizeof(UINT32)
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    pVmDnsBuffer->szCursor += sizeof(UINT32);

    *puData = ntohl(uData);

cleanup:

    return dwError;
error:

    if (puData)
    {
        *puData = 0;
    }
    goto cleanup;
}

DWORD
VmDnsReadUINT64FromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PUINT64 puData
    )
{
    DWORD dwError = 0;
    UINT64 uData = FALSE;
    PBYTE pCurrentPos = NULL;

    if (!puData || !pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCheckMemory(pVmDnsBuffer,sizeof(UINT64));
    BAIL_ON_VMDNS_ERROR(dwError);

    pCurrentPos = pVmDnsBuffer->pMessage + pVmDnsBuffer->szCursor;

    dwError = VmDnsNToH(
                    pCurrentPos,
                    sizeof(UINT64),
                    (PBYTE)&uData
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    pVmDnsBuffer->szCursor += sizeof(UINT64);

    *puData = uData;

cleanup:

    return dwError;
error:

    if (puData)
    {
        *puData = 0;
    }
    goto cleanup;
}

DWORD
VmDnsReadINT16FromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PINT16 piData
    )
{
    DWORD dwError = 0;
    INT16 iData = FALSE;
    PBYTE pCurrentPos = NULL;

    if (!piData || !pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCheckMemory(pVmDnsBuffer,sizeof(INT16));
    BAIL_ON_VMDNS_ERROR(dwError);

    pCurrentPos = pVmDnsBuffer->pMessage + pVmDnsBuffer->szCursor;

    dwError = VmDnsCopyMemory(
                          &iData,
                          sizeof(iData),
                          pCurrentPos,
                          sizeof(INT16)
                          );
    BAIL_ON_VMDNS_ERROR(dwError);

    pVmDnsBuffer->szCursor += sizeof(INT16);

    *piData = ntohs(iData);

cleanup:

    return dwError;
error:

    if (piData)
    {
        *piData = 0;
    }
    goto cleanup;
}

DWORD
VmDnsReadINT32FromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PINT32 piData
    )
{
    DWORD dwError = 0;
    INT32 iData = FALSE;
    PBYTE pCurrentPos = NULL;

    if (!piData || !pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCheckMemory(pVmDnsBuffer,sizeof(INT32));
    BAIL_ON_VMDNS_ERROR(dwError);

    pCurrentPos = pVmDnsBuffer->pMessage + pVmDnsBuffer->szCursor;

    dwError = VmDnsCopyMemory(
                          &iData,
                          sizeof(iData),
                          pCurrentPos,
                          sizeof(INT32)
                          );
    BAIL_ON_VMDNS_ERROR(dwError);

    pVmDnsBuffer->szCursor += sizeof(INT32);

    *piData = ntohl(iData);

cleanup:

    return dwError;
error:

    if (piData)
    {
        *piData = 0;
    }
    goto cleanup;
}

DWORD
VmDnsReadINT64FromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PINT64 piData
    )
{
    DWORD dwError = 0;
    INT64 iData = FALSE;
    PBYTE pCurrentPos = NULL;

    if (!piData || !pVmDnsBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCheckMemory(pVmDnsBuffer,sizeof(INT64));
    BAIL_ON_VMDNS_ERROR(dwError);

    pCurrentPos = pVmDnsBuffer->pMessage + pVmDnsBuffer->szCursor;

    dwError = VmDnsNToH(
                    pCurrentPos,
                    sizeof(INT64),
                    (PBYTE)&iData
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    pVmDnsBuffer->szCursor += sizeof(INT64);

    *piData = iData;

cleanup:

    return dwError;
error:

    if (piData)
    {
        *piData = 0;
    }
    goto cleanup;
}

DWORD
VmDnsReadStringFromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PSTR *ppszString,
    PDWORD pdwStringLength,
    PBOOL pbEndOfString
    )
{
    DWORD dwError = 0;
    DWORD dwStringLength = 0;
    PSTR pszString = NULL;
    PBYTE pCurrentPos = NULL;
    BOOL bEndOfString = FALSE;

    if (!pVmDnsBuffer || !ppszString || !pdwStringLength || !pbEndOfString)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCheckMemory(pVmDnsBuffer, sizeof(UINT8));
    BAIL_ON_VMDNS_ERROR(dwError);

    pCurrentPos = pVmDnsBuffer->pMessage + pVmDnsBuffer->szCursor;

    dwStringLength  = *(PUINT8)pCurrentPos;

    pVmDnsBuffer->szCursor += sizeof(UINT8);

    if (dwStringLength)
    {
        if (dwStringLength > VMDNS_NAME_LENGTH_MAX)
        {
            dwError = ERROR_LABEL_TOO_LONG;
            BAIL_ON_VMDNS_ERROR(dwError);
        }

        if (VMDNS_COMPRESSED_NAME(dwStringLength))
        {
            pCurrentPos = pVmDnsBuffer->pMessage + pVmDnsBuffer->szCursor;

            dwError = VmDnsReadOffsetStringFromBuffer(
                                pVmDnsBuffer,
                                (UINT8)*pCurrentPos,
                                &pszString,
                                &dwStringLength,
                                &bEndOfString
                                );
            BAIL_ON_VMDNS_ERROR(dwError);

            pVmDnsBuffer->szCursor += sizeof(UINT8);
        }
        else
        {
            dwError = VmDnsCheckMemory(pVmDnsBuffer, dwStringLength);
            BAIL_ON_VMDNS_ERROR(dwError);

            dwError = VmDnsAllocateMemory(
                            dwStringLength + 1,
                            (PVOID *)&pszString
                            );
            BAIL_ON_VMDNS_ERROR(dwError);

            pCurrentPos = pVmDnsBuffer->pMessage + pVmDnsBuffer->szCursor;

            dwError = VmDnsCopyMemory(
                        pszString,
                        dwStringLength + 1,
                        pCurrentPos,
                        dwStringLength
                        );
            BAIL_ON_VMDNS_ERROR(dwError);

            pVmDnsBuffer->szCursor += dwStringLength;
            pszString[dwStringLength] = '\0';
        }
    }
    else
    {
        bEndOfString = TRUE;
    }

    *ppszString = pszString;
    *pdwStringLength = dwStringLength;
    *pbEndOfString = bEndOfString;
cleanup:

    return dwError;
error:

    if (ppszString)
    {
        *ppszString = NULL;
    }
    if (pdwStringLength)
    {
        *pdwStringLength = 0;
    }
    if (pbEndOfString)
    {
        *pbEndOfString = TRUE;
    }
    VMDNS_SAFE_FREE_MEMORY(pszString);

    goto cleanup;
}

DWORD
VmDnsReadOffsetStringFromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    UINT8 unOffset,
    PSTR *ppszString,
    PDWORD pdwStringLength,
    PBOOL pbEndOfString
    )
{
    DWORD dwError = 0;
    DWORD dwStringLength = 0;
    DWORD dwLabelLength = 0;
    PBYTE pCurrentPos = NULL;
    PSTR pszString = NULL;
    PSTR pszTempString = NULL;
    PSTR pszTempStringCursor = NULL;
    BOOL bEndOfString = FALSE;

    if (!pVmDnsBuffer || !ppszString || !pdwStringLength || !pbEndOfString)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(
                        VMDNS_NAME_LENGTH_MAX + 2,
                        (PVOID *)&pszTempString
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    pCurrentPos = pVmDnsBuffer->pMessage + unOffset;

    pszTempStringCursor = pszTempString;

    // Get the first part of the label
    dwLabelLength = *(PUINT8)pCurrentPos;
    pCurrentPos += sizeof(UINT8);

    while (dwLabelLength)
    {
        if (dwLabelLength > VMDNS_LABEL_LENGTH_MAX)
        {
            dwError = ERROR_LABEL_TOO_LONG;
            BAIL_ON_VMDNS_ERROR(dwError);
        }
        if (dwStringLength > VMDNS_NAME_LENGTH_MAX)
        {
            dwError = ERROR_LABEL_TOO_LONG;
            BAIL_ON_VMDNS_ERROR(dwError);
        }
        if (dwLabelLength > (VMDNS_NAME_LENGTH_MAX - dwStringLength))
        {
            dwError = ERROR_LABEL_TOO_LONG;
            BAIL_ON_VMDNS_ERROR(dwError);
        }

        dwError = VmDnsCheckMemory(pVmDnsBuffer, dwLabelLength);
        BAIL_ON_VMDNS_ERROR(dwError);

        if ((pCurrentPos + dwLabelLength) >
            (pVmDnsBuffer->pMessage + pVmDnsBuffer->szLength ))
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDNS_ERROR(dwError);
        }

        dwError = VmDnsCopyMemory(
                        pszTempStringCursor,
                        VMDNS_NAME_LENGTH_MAX - dwStringLength,
                        pCurrentPos,
                        dwLabelLength
                        );

        BAIL_ON_VMDNS_ERROR(dwError);

        pCurrentPos += dwLabelLength;
        pszTempStringCursor += dwLabelLength;

        // Append . at the end
        *pszTempStringCursor = '.';
        ++dwLabelLength;
        ++pszTempStringCursor;

        dwStringLength += dwLabelLength;

        // Go to the next part of the label
        dwLabelLength = *(PUINT8)pCurrentPos;
        pCurrentPos += sizeof(UINT8);
    }

    bEndOfString = TRUE;

    dwError = VmDnsAllocateStringA(
                        pszTempString,
                        &pszString
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppszString = pszString;
    *pdwStringLength = dwStringLength;
    *pbEndOfString = bEndOfString;
cleanup:

    VMDNS_SAFE_FREE_MEMORY(pszTempString);

    return dwError;
error:

    if (ppszString)
    {
        *ppszString = NULL;
    }
    if (pdwStringLength)
    {
        *pdwStringLength = 0;
    }
    if (pbEndOfString)
    {
        *pbEndOfString = TRUE;
    }
    VMDNS_SAFE_FREE_MEMORY(pszString);

    goto cleanup;
}

DWORD
VmDnsReadBlobFromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PVMDNS_BLOB *ppBlob
    )
{
    DWORD dwError = 0;
    UINT16 unBlobSize = 0;
    PBYTE pCurrentPos = NULL;
    PVMDNS_BLOB pBlob = NULL;

    if (!pVmDnsBuffer || !ppBlob)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsReadUINT16FromBuffer(
                        pVmDnsBuffer,
                        &unBlobSize
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCheckMemory(
                    pVmDnsBuffer,
                    unBlobSize
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateBlob(
                        unBlobSize,
                        &pBlob
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    pCurrentPos = pVmDnsBuffer->pMessage + pVmDnsBuffer->szCursor;

    if (unBlobSize)
    {
        dwError = VmDnsCopyMemory(
                        pBlob->pData,
                        unBlobSize,
                        pCurrentPos,
                        unBlobSize
                        );
        BAIL_ON_VMDNS_ERROR(dwError);

        pVmDnsBuffer->szCursor += unBlobSize;
    }

    *ppBlob = pBlob;


cleanup:

    return dwError;
error:

    VmDnsFreeBlob(pBlob);
    if (ppBlob)
    {
        *ppBlob = NULL;
    }

    goto cleanup;
}

DWORD
VmDnsIsTokenizedBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    BOOL bTokenized
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMDNS_INVALID_POINTER(pVmDnsBuffer, dwError);

    bTokenized = pVmDnsBuffer->bTokenizeDomainName;

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDnsSetBufferTokenizedFlag(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    BOOL bTokenized
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMDNS_INVALID_POINTER(pVmDnsBuffer, dwError);

    pVmDnsBuffer->bTokenizeDomainName = bTokenized;

cleanup:
    return dwError;
error:
    goto cleanup;
}

static
DWORD
VmDnsAdjustMemoryBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    size_t szDataSize
    )
{
    DWORD dwError = 0;
    PBYTE pTempMessage = NULL;
    size_t szNewSize   = 0;
    size_t szRequired  = pVmDnsBuffer->szLength + szDataSize;

    if (pVmDnsBuffer->szCurrentSize < szRequired)
    {
        szNewSize   = 2*pVmDnsBuffer->szCurrentSize + szDataSize;
        if (szNewSize > pVmDnsBuffer->szMaxSize)
        {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
            BAIL_ON_VMDNS_ERROR(dwError);
        }

        dwError = VmDnsReallocateMemoryWithInit(
                                  pVmDnsBuffer->pMessage,
                                  (PVOID *)&pTempMessage,
                                  szNewSize,
                                  pVmDnsBuffer->szCurrentSize
                                  );
        BAIL_ON_VMDNS_ERROR(dwError);

        pVmDnsBuffer->pMessage = pTempMessage;
        pVmDnsBuffer->szCurrentSize = szNewSize;
    }
cleanup:

    return dwError;
error:

    VMDNS_SAFE_FREE_MEMORY(pTempMessage);

    goto cleanup;
}


static
DWORD
VmDnsCheckMemory(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    size_t szDataSize
    )
{
    DWORD dwError = 0;
    size_t szRequired  = pVmDnsBuffer->szCursor + szDataSize;

    if (pVmDnsBuffer->szLength < szRequired)
    {
        dwError = ERROR_INSUFFICIENT_BUFFER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }
cleanup:

    return dwError;
error:

    goto cleanup;
}

static
BOOL
VmDnsIsBigEndian(
    VOID
    )
{
    union {
        UINT32 uiData32;
        CHAR cData[4];
    } DnsEndianDetect = {0x01020304};

    return DnsEndianDetect.cData[0] == 1;
}

static
DWORD
VmDnsNToH(
    PBYTE pInData,
    size_t szData,
    PBYTE pOutData
    )
{
    DWORD dwError = 0;
    DWORD dwInIndex = 0;
    DWORD dwOutIndex = 0;

    if (!pInData || !szData || !pOutData)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (!VmDnsIsBigEndian())
    {
        for (dwInIndex = szData; dwOutIndex < szData && dwInIndex--; ++dwOutIndex)
        {
            pOutData[dwOutIndex] = pInData[dwInIndex];
        }
    }
    else
    {
        dwError = VmDnsCopyMemory(
                        pOutData,
                        szData,
                        pInData,
                        szData
                        );
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:

    return dwError;
error:

    goto cleanup;
}

static
DWORD
VmDnsHToN(
    PBYTE pInData,
    size_t szData,
    PBYTE pOutData
    )
{
    return VmDnsNToH(
                pInData,
                szData,
                pOutData
                );
}
