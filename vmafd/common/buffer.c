/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
VmAfdAdjustMemoryBuffer(
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
    size_t szDataSize
    );

static
DWORD
VmAfdCheckMemory(
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
    size_t szDataSize
    );

static
BOOL
VmAfdIsBigEndian(
    VOID
    );

static
DWORD
VmAfdNToH(
    PBYTE pInData,
    size_t szData,
    PBYTE pOutData
    );

static
DWORD
VmAfdHToN(
    PBYTE pInData,
    size_t szData,
    PBYTE pOutData
    );

DWORD
VmAfdAllocateBufferStream(
    size_t dwMaxSize,
    PVMAFD_MESSAGE_BUFFER *ppVmAfdBuffer
    )
{
    DWORD dwError = 0;
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer = NULL;

    if (!ppVmAfdBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (dwMaxSize > VMAFD_MAX_SIZE_BUFFER)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(
                    sizeof(VMAFD_MESSAGE_BUFFER),
                    (PVOID *)&pVmAfdBuffer
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    pVmAfdBuffer->szMaxSize = dwMaxSize? dwMaxSize : VMAFD_MAX_SIZE_BUFFER;

    pVmAfdBuffer->bCanWrite = TRUE;

    *ppVmAfdBuffer = pVmAfdBuffer;

cleanup:

    return dwError;
error:
    if (ppVmAfdBuffer)
    {
        *ppVmAfdBuffer = NULL;
    }
    VmAfdFreeBufferStream(pVmAfdBuffer);

    goto cleanup;
}

VOID
VmAfdFreeBufferStream(
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
    )
{
    if (pVmAfdBuffer)
    {
        if (pVmAfdBuffer->pMessage)
        {
            VMAFD_SAFE_FREE_MEMORY(pVmAfdBuffer->pMessage);
        }

        VMAFD_SAFE_FREE_MEMORY(pVmAfdBuffer);
    }
}

DWORD
VmAfdLockBufferForWrite(
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
    )
{
    DWORD dwError = 0;
    PBYTE pTempMessage = NULL;

    if (!pVmAfdBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdReallocateMemory(
                       pVmAfdBuffer->pMessage,
                       (PVOID *)&pTempMessage,
                       pVmAfdBuffer->szLength
                       );

    pVmAfdBuffer->bCanWrite = FALSE;
    pVmAfdBuffer->szCurrentSize = pVmAfdBuffer->szLength;
    pVmAfdBuffer->szCursor = 0;

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfdAllocateBufferStreamWithBuffer(
    PBYTE pBuffer,
    size_t szBufSize,
    size_t szMaxSize,
    BOOL bCanWrite,
    PVMAFD_MESSAGE_BUFFER *ppVmAfdBuffer
    )
{
    DWORD dwError = 0;
    PBYTE pTempBuffer = NULL;
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer = NULL;

    if (!pBuffer || !szBufSize || !ppVmAfdBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(
                       szBufSize,
                       (PVOID *)&pTempBuffer
                       );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdCopyMemory(
                        pTempBuffer,
                        szBufSize,
                        pBuffer,
                        szBufSize
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateBufferStream(
                            szMaxSize,
                            &pVmAfdBuffer
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

    pVmAfdBuffer->pMessage = pTempBuffer;
    pTempBuffer = NULL;
    pVmAfdBuffer->szLength = szBufSize;
    pVmAfdBuffer->szCurrentSize = szBufSize;

    pVmAfdBuffer->bCanWrite = bCanWrite;

    *ppVmAfdBuffer = pVmAfdBuffer;

cleanup:

    return dwError;
error:

    if (ppVmAfdBuffer)
    {
        *ppVmAfdBuffer = NULL;
    }
    VmAfdFreeBufferStream(pVmAfdBuffer);
    VMAFD_SAFE_FREE_MEMORY(pTempBuffer);

    goto cleanup;
}

DWORD
VmAfdCopyBufferFromBufferStream(
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
    PBYTE pBuffer,
    PDWORD pdwBufferSize
    )
{
    DWORD dwError = 0;

    if (!pVmAfdBuffer ||
        !pdwBufferSize
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pBuffer && pVmAfdBuffer->szLength)
    {
        dwError = VmAfdCopyMemory(
                          pBuffer,
                          *pdwBufferSize,
                          pVmAfdBuffer->pMessage,
                          pVmAfdBuffer->szLength
                          );
        BAIL_ON_VMAFD_ERROR (dwError);
    }

cleanup:

    if (pdwBufferSize && pVmAfdBuffer)
    {
        *pdwBufferSize = pVmAfdBuffer->szLength;
    }
    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfdWriteBoolToBuffer(
    BOOL bData,
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
    )
{
    DWORD dwError = 0;
    BOOL bBoolData = TRUE;
    PBOOL pBoolCursor = NULL;

    if (!pVmAfdBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    if (!pVmAfdBuffer->bCanWrite)
    {
        dwError = ERROR_CANTWRITE;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAdjustMemoryBuffer(
                           pVmAfdBuffer,
                           sizeof(BOOL)
                           );
    BAIL_ON_VMAFD_ERROR (dwError);

    pBoolCursor = (PBOOL)(pVmAfdBuffer->pMessage + pVmAfdBuffer->szLength);

    dwError = VmAfdHToN(
                    (PBYTE)&bData,
                    sizeof(BOOL),
                    (PBYTE)&bBoolData
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdCopyMemory(
                        pBoolCursor,
                        pVmAfdBuffer->szCurrentSize - pVmAfdBuffer->szLength,
                        &bBoolData,
                        sizeof(BOOL)
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    pVmAfdBuffer->szLength += sizeof(BOOL);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfdWriteBooleanToBuffer(
    BOOLEAN bData,
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
    )
{
    DWORD dwError = 0;
    BOOLEAN bBooleanData = FALSE;
    PBOOLEAN pBooleanCursor = NULL;

    if (!pVmAfdBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    if (!pVmAfdBuffer->bCanWrite)
    {
        dwError = ERROR_CANTWRITE;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAdjustMemoryBuffer(
                           pVmAfdBuffer,
                           sizeof(BOOLEAN)
                           );
    BAIL_ON_VMAFD_ERROR (dwError);

    pBooleanCursor = (PBOOLEAN)
                     (pVmAfdBuffer->pMessage + pVmAfdBuffer->szLength);

    dwError = VmAfdHToN(
                      (PBYTE)&bData,
                      sizeof(BOOLEAN),
                      (PBYTE)&bBooleanData
                      );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdCopyMemory(
                        pBooleanCursor,
                        pVmAfdBuffer->szCurrentSize - pVmAfdBuffer->szLength,
                        &bBooleanData,
                        sizeof(BOOLEAN)
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    pVmAfdBuffer->szLength += sizeof(BOOLEAN);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfdWriteCharToBuffer(
    CHAR cData,
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
    )
{
    DWORD dwError = 0;
    PCHAR pCharCursor = NULL;

    if (!pVmAfdBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    if (!pVmAfdBuffer->bCanWrite)
    {
        dwError = ERROR_CANTWRITE;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAdjustMemoryBuffer(
                           pVmAfdBuffer,
                           sizeof(CHAR)
                           );
    BAIL_ON_VMAFD_ERROR (dwError);

    pCharCursor = (PCHAR)(pVmAfdBuffer->pMessage + pVmAfdBuffer->szLength);

    dwError = VmAfdCopyMemory(
                        pCharCursor,
                        pVmAfdBuffer->szCurrentSize - pVmAfdBuffer->szLength,
                        &cData,
                        sizeof(CHAR)
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    pVmAfdBuffer->szLength += sizeof(CHAR);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfdWriteUCharToBuffer(
    UCHAR ucData,
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
    )
{
    DWORD dwError = 0;
    PUCHAR pCharCursor = NULL;

    if (!pVmAfdBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    if (!pVmAfdBuffer->bCanWrite)
    {
        dwError = ERROR_CANTWRITE;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAdjustMemoryBuffer(
                           pVmAfdBuffer,
                           sizeof(UCHAR)
                           );
    BAIL_ON_VMAFD_ERROR (dwError);

    pCharCursor = (PUCHAR)(pVmAfdBuffer->pMessage + pVmAfdBuffer->szLength);

    dwError = VmAfdCopyMemory(
                        pCharCursor,
                        pVmAfdBuffer->szCurrentSize - pVmAfdBuffer->szLength,
                        &ucData,
                        sizeof(UCHAR)
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    pVmAfdBuffer->szLength += sizeof(UCHAR);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfdWriteUINT16ToBuffer(
    UINT16 uData,
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
    )
{
    DWORD dwError = 0;
    UINT16 uInt16Data = 0;
    PUINT16 pUint16Cursor = NULL;

    if (!pVmAfdBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    if (!pVmAfdBuffer->bCanWrite)
    {
        dwError = ERROR_CANTWRITE;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAdjustMemoryBuffer(
                           pVmAfdBuffer,
                           sizeof(UINT16)
                           );
    BAIL_ON_VMAFD_ERROR (dwError);

    pUint16Cursor = (PUINT16)(pVmAfdBuffer->pMessage + pVmAfdBuffer->szLength);

    uInt16Data = htons(uData);

    dwError = VmAfdCopyMemory(
                        pUint16Cursor,
                        pVmAfdBuffer->szCurrentSize - pVmAfdBuffer->szLength,
                        &uInt16Data,
                        sizeof(UINT16)
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    pVmAfdBuffer->szLength += sizeof(UINT16);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfdWriteUINT32ToBuffer(
    UINT32 uData,
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
    )
{
    DWORD dwError = 0;
    UINT32 uInt32Data = 0;
    PUINT32 pUint32Cursor = NULL;

    if (!pVmAfdBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    if (!pVmAfdBuffer->bCanWrite)
    {
        dwError = ERROR_CANTWRITE;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAdjustMemoryBuffer(
                           pVmAfdBuffer,
                           sizeof(UINT32)
                           );
    BAIL_ON_VMAFD_ERROR (dwError);

    pUint32Cursor = (PUINT32)(pVmAfdBuffer->pMessage + pVmAfdBuffer->szLength);

    uInt32Data = htonl(uData);

    dwError = VmAfdCopyMemory(
                        pUint32Cursor,
                        pVmAfdBuffer->szCurrentSize - pVmAfdBuffer->szLength,
                        &uInt32Data,
                        sizeof(UINT32)
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    pVmAfdBuffer->szLength += sizeof(UINT32);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfdWriteUINT64ToBuffer(
    UINT64 uData,
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
    )
{
    DWORD dwError = 0;
    UINT64 uIntData = 0;
    PUINT64 pUint64Cursor = NULL;

    if (!pVmAfdBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    if (!pVmAfdBuffer->bCanWrite)
    {
        dwError = ERROR_CANTWRITE;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAdjustMemoryBuffer(
                           pVmAfdBuffer,
                           sizeof(UINT64)
                           );
    BAIL_ON_VMAFD_ERROR (dwError);

    pUint64Cursor = (PUINT64)(pVmAfdBuffer->pMessage + pVmAfdBuffer->szLength);

    dwError = VmAfdHToN(
                      (PBYTE)&uData,
                      sizeof(UINT64),
                      (PBYTE)&uIntData
                      );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdCopyMemory(
                         pUint64Cursor,
                         pVmAfdBuffer->szCurrentSize - pVmAfdBuffer->szLength,
                         &uIntData,
                         sizeof(UINT64)
                         );
    BAIL_ON_VMAFD_ERROR(dwError);

    pVmAfdBuffer->szLength += sizeof(UINT64);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfdWriteINT16ToBuffer(
    INT16 iData,
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
    )
{
    DWORD dwError = 0;
    INT16 iInt16Data = 0;
    PINT16 pInt16Cursor = NULL;

    if (!pVmAfdBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    if (!pVmAfdBuffer->bCanWrite)
    {
        dwError = ERROR_CANTWRITE;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAdjustMemoryBuffer(
                           pVmAfdBuffer,
                           sizeof(INT16)
                           );
    BAIL_ON_VMAFD_ERROR (dwError);

    pInt16Cursor = (PINT16)(pVmAfdBuffer->pMessage + pVmAfdBuffer->szLength);

    iInt16Data = htons(iData);

    dwError = VmAfdCopyMemory(
                      pInt16Cursor,
                      pVmAfdBuffer->szCurrentSize - pVmAfdBuffer->szLength,
                      &iInt16Data,
                      sizeof(INT16)
                      );
    BAIL_ON_VMAFD_ERROR(dwError);

    pVmAfdBuffer->szLength += sizeof(INT16);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfdWriteINT32ToBuffer(
    INT32 iData,
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
    )
{
    DWORD dwError = 0;
    INT32 iInt32Data = 0;
    PINT32 pInt32Cursor = NULL;

    if (!pVmAfdBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    if (!pVmAfdBuffer->bCanWrite)
    {
        dwError = ERROR_CANTWRITE;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAdjustMemoryBuffer(
                           pVmAfdBuffer,
                           sizeof(INT32)
                           );
    BAIL_ON_VMAFD_ERROR (dwError);

    pInt32Cursor = (PINT32)(pVmAfdBuffer->pMessage + pVmAfdBuffer->szLength);

    iInt32Data = htonl(iData);

    dwError = VmAfdCopyMemory(
                      pInt32Cursor,
                      pVmAfdBuffer->szCurrentSize - pVmAfdBuffer->szLength,
                      &iInt32Data,
                      sizeof(INT32)
                      );
    BAIL_ON_VMAFD_ERROR(dwError);

    pVmAfdBuffer->szLength += sizeof(INT32);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfdWriteINT64ToBuffer(
    INT64 iData,
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
    )
{
    DWORD dwError = 0;
    INT64 iInt64Data = 0;
    PINT64 pInt64Cursor = NULL;

    if (!pVmAfdBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    if (!pVmAfdBuffer->bCanWrite)
    {
        dwError = ERROR_CANTWRITE;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAdjustMemoryBuffer(
                           pVmAfdBuffer,
                           sizeof(INT64)
                           );
    BAIL_ON_VMAFD_ERROR (dwError);

    pInt64Cursor = (PINT64)(pVmAfdBuffer->pMessage + pVmAfdBuffer->szLength);

    dwError = VmAfdHToN(
                      (PBYTE)&iData,
                      sizeof(INT64),
                      (PBYTE)&iInt64Data
                      );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdCopyMemory(
                    pInt64Cursor,
                    pVmAfdBuffer->szCurrentSize - pVmAfdBuffer->szLength,
                    &iInt64Data,
                    sizeof(INT64)
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    pVmAfdBuffer->szLength += sizeof(INT64);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfdWriteStringToBuffer(
    PSTR pszString,
    UINT8 uStringLength,
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
    )
{
    DWORD dwError = 0;
    size_t szDataSize = uStringLength + 1;
    PSTR pCursor = NULL;

    if (!pVmAfdBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!pVmAfdBuffer->bCanWrite)
    {
        dwError = ERROR_CANTWRITE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAdjustMemoryBuffer(
                          pVmAfdBuffer,
                          szDataSize
                          );
    BAIL_ON_VMAFD_ERROR(dwError);

    pCursor = (PSTR)pVmAfdBuffer->pMessage + pVmAfdBuffer->szLength;

    if (pszString && uStringLength)
    {
        VmAfdCopyMemory(
                pCursor++,
                pVmAfdBuffer->szCurrentSize - pVmAfdBuffer->szLength,
                pszString,
                uStringLength
                );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pVmAfdBuffer->szLength += szDataSize;
cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfdWriteBlobToBuffer(
    PBYTE pBlob,
    DWORD dwSize,
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfdReadBoolFromBuffer(
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
    PBOOL pbData
    )
{
    DWORD dwError = 0;
    BOOL bData = FALSE;
    PBYTE pCurrentPos = NULL;

    if (!pbData || !pVmAfdBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdCheckMemory(pVmAfdBuffer,sizeof(BOOL));
    BAIL_ON_VMAFD_ERROR(dwError);

    pCurrentPos = pVmAfdBuffer->pMessage + pVmAfdBuffer->szCursor;

    dwError = VmAfdNToH(
                    pCurrentPos,
                    sizeof(BOOL),
                    (PBYTE)&bData
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    pVmAfdBuffer->szCursor += sizeof(BOOL);

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
VmAfdReadBooleanFromBuffer(
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
    PBOOLEAN pbData
    )
{
    DWORD dwError = 0;
    BOOLEAN bData = FALSE;
    PBYTE pCurrentPos = NULL;

    if (!pbData || !pVmAfdBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdCheckMemory(pVmAfdBuffer,sizeof(BOOLEAN));
    BAIL_ON_VMAFD_ERROR(dwError);

    pCurrentPos = pVmAfdBuffer->pMessage + pVmAfdBuffer->szCursor;

    dwError = VmAfdNToH(
                    pCurrentPos,
                    sizeof(BOOLEAN),
                    &bData
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    pVmAfdBuffer->szCursor += sizeof(BOOLEAN);

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
VmAfdReadCharFromBuffer(
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
    PCHAR pcData
    )
{
    DWORD dwError = 0;
    CHAR cData = FALSE;
    PBYTE pCurrentPos = NULL;

    if (!pcData || !pVmAfdBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdCheckMemory(pVmAfdBuffer,sizeof(CHAR));
    BAIL_ON_VMAFD_ERROR(dwError);

    pCurrentPos = pVmAfdBuffer->pMessage + pVmAfdBuffer->szCursor;

    cData = *(PCHAR)pCurrentPos;

    pVmAfdBuffer->szCursor += sizeof(CHAR);

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
VmAfdReadUCharFromBuffer(
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
    PUCHAR pucData
    )
{
    DWORD dwError = 0;
    UCHAR ucData = FALSE;
    PBYTE pCurrentPos = NULL;

    if (!pucData || !pVmAfdBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdCheckMemory(pVmAfdBuffer,sizeof(UCHAR));
    BAIL_ON_VMAFD_ERROR(dwError);

    pCurrentPos = pVmAfdBuffer->pMessage + pVmAfdBuffer->szCursor;

    ucData = *(PUCHAR)pCurrentPos;

    pVmAfdBuffer->szCursor += sizeof(UCHAR);

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
VmAfdReadUINT16FromBuffer(
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
    PUINT16 puData
    )
{
    DWORD dwError = 0;
    UINT16 uData = FALSE;
    PBYTE pCurrentPos = NULL;

    if (!puData || !pVmAfdBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdCheckMemory(pVmAfdBuffer,sizeof(UINT16));
    BAIL_ON_VMAFD_ERROR(dwError);

    pCurrentPos = pVmAfdBuffer->pMessage + pVmAfdBuffer->szCursor;

    dwError = VmAfdCopyMemory(
                    &uData,
                    sizeof(uData),
                    pCurrentPos,
                    sizeof(UINT16)
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    pVmAfdBuffer->szCursor += sizeof(UINT16);

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
VmAfdReadUINT32FromBuffer(
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
    PUINT32 puData
    )
{
    DWORD dwError = 0;
    UINT32 uData = FALSE;
    PBYTE pCurrentPos = NULL;

    if (!puData || !pVmAfdBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdCheckMemory(pVmAfdBuffer,sizeof(UINT32));
    BAIL_ON_VMAFD_ERROR(dwError);

    pCurrentPos = pVmAfdBuffer->pMessage + pVmAfdBuffer->szCursor;

    dwError = VmAfdCopyMemory(
                    &uData,
                    sizeof(uData),
                    pCurrentPos,
                    sizeof(UINT32)
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    pVmAfdBuffer->szCursor += sizeof(UINT32);

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
VmAfdReadUINT64FromBuffer(
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
    PUINT64 puData
    )
{
    DWORD dwError = 0;
    UINT64 uData = FALSE;
    PBYTE pCurrentPos = NULL;

    if (!puData || !pVmAfdBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdCheckMemory(pVmAfdBuffer,sizeof(UINT64));
    BAIL_ON_VMAFD_ERROR(dwError);

    pCurrentPos = pVmAfdBuffer->pMessage + pVmAfdBuffer->szCursor;

    dwError = VmAfdNToH(
                    pCurrentPos,
                    sizeof(UINT64),
                    (PBYTE)&uData
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    pVmAfdBuffer->szCursor += sizeof(UINT64);

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
VmAfdReadINT16FromBuffer(
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
    PINT16 piData
    )
{
    DWORD dwError = 0;
    INT16 iData = FALSE;
    PBYTE pCurrentPos = NULL;

    if (!piData || !pVmAfdBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdCheckMemory(pVmAfdBuffer,sizeof(INT16));
    BAIL_ON_VMAFD_ERROR(dwError);

    pCurrentPos = pVmAfdBuffer->pMessage + pVmAfdBuffer->szCursor;

    dwError = VmAfdCopyMemory(
                          &iData,
                          sizeof(iData),
                          pCurrentPos,
                          sizeof(INT16)
                          );
    BAIL_ON_VMAFD_ERROR(dwError);

    pVmAfdBuffer->szCursor += sizeof(INT16);

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
VmAfdReadINT32FromBuffer(
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
    PINT32 piData
    )
{
    DWORD dwError = 0;
    INT32 iData = FALSE;
    PBYTE pCurrentPos = NULL;

    if (!piData || !pVmAfdBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdCheckMemory(pVmAfdBuffer,sizeof(INT32));
    BAIL_ON_VMAFD_ERROR(dwError);

    pCurrentPos = pVmAfdBuffer->pMessage + pVmAfdBuffer->szCursor;

    dwError = VmAfdCopyMemory(
                          &iData,
                          sizeof(iData),
                          pCurrentPos,
                          sizeof(INT32)
                          );
    BAIL_ON_VMAFD_ERROR(dwError);

    pVmAfdBuffer->szCursor += sizeof(INT32);

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
VmAfdReadINT64FromBuffer(
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
    PINT64 piData
    )
{
    DWORD dwError = 0;
    INT64 iData = FALSE;
    PBYTE pCurrentPos = NULL;

    if (!piData || !pVmAfdBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdCheckMemory(pVmAfdBuffer,sizeof(INT64));
    BAIL_ON_VMAFD_ERROR(dwError);

    pCurrentPos = pVmAfdBuffer->pMessage + pVmAfdBuffer->szCursor;

    dwError = VmAfdNToH(
                    pCurrentPos,
                    sizeof(INT64),
                    (PBYTE)&iData
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    pVmAfdBuffer->szCursor += sizeof(INT64);

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
VmAfdReadStringFromBuffer(
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
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

    if (!pVmAfdBuffer || !ppszString || !pdwStringLength || !pbEndOfString)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdCheckMemory(pVmAfdBuffer, sizeof(UINT8));
    BAIL_ON_VMAFD_ERROR(dwError);

    pCurrentPos = pVmAfdBuffer->pMessage + pVmAfdBuffer->szCursor;

    dwStringLength  = *(PUINT8)pCurrentPos;

    pVmAfdBuffer->szCursor += sizeof(UINT8);

    if (dwStringLength)
    {
        if (dwStringLength > VMDDNS_NAME_LENGTH_MAX)
        {
            dwError = ERROR_LABEL_TOO_LONG;
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        if (VMDDNS_COMPRESSED_NAME(dwStringLength))
        {
            pCurrentPos = pVmAfdBuffer->pMessage + pVmAfdBuffer->szCursor;

            dwError = VmAfdReadOffsetStringFromBuffer(
                                pVmAfdBuffer,
                                (UINT8)*pCurrentPos,
                                &pszString,
                                &dwStringLength,
                                &bEndOfString
                                );
            BAIL_ON_VMAFD_ERROR(dwError);

            pVmAfdBuffer->szCursor += sizeof(UINT8);
        }
        else
        {
            dwError = VmAfdCheckMemory(pVmAfdBuffer, dwStringLength);
            BAIL_ON_VMAFD_ERROR(dwError);

            dwError = VmAfdAllocateMemory(
                            dwStringLength + 1,
                            (PVOID *)&pszString
                            );
            BAIL_ON_VMAFD_ERROR(dwError);

            pCurrentPos = pVmAfdBuffer->pMessage + pVmAfdBuffer->szCursor;

            dwError = VmAfdCopyMemory(
                        pszString,
                        dwStringLength + 1,
                        pCurrentPos,
                        dwStringLength
                        );
            BAIL_ON_VMAFD_ERROR(dwError);


            pVmAfdBuffer->szCursor += dwStringLength;
        }

        pszString[dwStringLength] = '\0';
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
    VMAFD_SAFE_FREE_MEMORY(pszString);

    goto cleanup;
}

DWORD
VmAfdReadOffsetStringFromBuffer(
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
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

    if (!pVmAfdBuffer || !ppszString || !pdwStringLength || !pbEndOfString)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(
                        VMDDNS_NAME_LENGTH_MAX + 1,
                        (PVOID *)&pszTempString
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    pCurrentPos = pVmAfdBuffer->pMessage + unOffset;

    pszTempStringCursor = pszTempString;

    dwLabelLength = *(PUINT8)pCurrentPos;

    while (dwLabelLength)
    {
        if (dwLabelLength > VMDDNS_LABEL_LENGTH_MAX)
        {
            dwError = ERROR_LABEL_TOO_LONG;
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        dwError = VmAfdCheckMemory(pVmAfdBuffer, dwLabelLength);
        BAIL_ON_VMAFD_ERROR(dwError);

        pCurrentPos += sizeof(UINT8);

        dwError = VmAfdCopyMemory(
                        pszTempStringCursor,
                        dwLabelLength + 1,
                        pCurrentPos,
                        dwLabelLength
                        );
        BAIL_ON_VMAFD_ERROR(dwError);

        if (dwLabelLength > (VMDDNS_NAME_LENGTH_MAX - dwStringLength))
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        pszTempStringCursor[dwLabelLength]='.';
        dwLabelLength++;

        pszTempStringCursor = &pszTempStringCursor[dwLabelLength];
        dwStringLength += dwLabelLength;

        if (dwStringLength > VMDDNS_NAME_LENGTH_MAX)
        {
            dwError = ERROR_LABEL_TOO_LONG;
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        pCurrentPos += dwLabelLength - 1;

        dwLabelLength = *(PUINT8)pCurrentPos;
    }

    bEndOfString = TRUE;

    dwError = VmAfdAllocateStringA(
                        pszTempString,
                        &pszString
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszString = pszString;
    *pdwStringLength = dwStringLength;
    *pbEndOfString = bEndOfString;
cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszTempString);

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
    VMAFD_SAFE_FREE_MEMORY(pszString);

    goto cleanup;
}

DWORD
VmAfdReadBlobFromBuffer(
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
    PBYTE *pBlob,
    PDWORD pdwSize
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmAfdIsTokenizedBuffer(
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
    BOOL bTokenized
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(pVmAfdBuffer, dwError);

    bTokenized = pVmAfdBuffer->bTokenizeDomainName;

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmAfdSetBufferTokenizedFlag(
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
    BOOL bTokenized
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(pVmAfdBuffer, dwError);

    pVmAfdBuffer->bTokenizeDomainName = bTokenized;

cleanup:
    return dwError;
error:
    goto cleanup;
}

static
DWORD
VmAfdAdjustMemoryBuffer(
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
    size_t szDataSize
    )
{
    DWORD dwError = 0;
    PBYTE pTempMessage = NULL;
    size_t szNewSize   = 0;
    size_t szRequired  = pVmAfdBuffer->szLength + szDataSize;

    if (pVmAfdBuffer->szCurrentSize < szRequired)
    {
        szNewSize   = 2*pVmAfdBuffer->szCurrentSize + szDataSize;
        if (szNewSize > pVmAfdBuffer->szMaxSize)
        {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        dwError = VmAfdReallocateMemoryWithInit(
                                  pVmAfdBuffer->pMessage,
                                  (PVOID *)&pTempMessage,
                                  szNewSize,
                                  pVmAfdBuffer->szCurrentSize
                                  );
        BAIL_ON_VMAFD_ERROR(dwError);

        pVmAfdBuffer->pMessage = pTempMessage;
        pVmAfdBuffer->szCurrentSize = szNewSize;
    }
cleanup:

    return dwError;
error:

    VMAFD_SAFE_FREE_MEMORY(pTempMessage);

    goto cleanup;
}

static
DWORD
VmAfdCheckMemory(
    PVMAFD_MESSAGE_BUFFER pVmAfdBuffer,
    size_t szDataSize
    )
{
    DWORD dwError = 0;
    size_t szRequired  = pVmAfdBuffer->szCursor + szDataSize;

    if (pVmAfdBuffer->szLength < szRequired)
    {
        dwError = ERROR_INSUFFICIENT_BUFFER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
cleanup:

    return dwError;
error:

    goto cleanup;
}

static
BOOL
VmAfdIsBigEndian(
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
VmAfdNToH(
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
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!VmAfdIsBigEndian())
    {
        for (dwInIndex = szData; dwOutIndex < szData && dwInIndex--; ++dwOutIndex)
        {
            pOutData[dwOutIndex] = pInData[dwInIndex];
        }
    }
    else
    {
        dwError = VmAfdCopyMemory(
                        pOutData,
                        szData,
                        pInData,
                        szData
                        );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    return dwError;
error:

    goto cleanup;
}

static
DWORD
VmAfdHToN(
    PBYTE pInData,
    size_t szData,
    PBYTE pOutData
    )
{
    return VmAfdNToH(
                pInData,
                szData,
                pOutData
                );
}
