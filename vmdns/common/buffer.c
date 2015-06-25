/*
 * Copyright (c) VMware Inc. 2011  All rights Reserved.
 *
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

    if (!pBuffer ||
        !szBufSize ||
        !ppVmDnsBuffer
       )
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

    *pBoolCursor = bBoolData;

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

    *pBooleanCursor = bBooleanData;

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

    *pCharCursor = cData;

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

    *pCharCursor = ucData;

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

    *pUint16Cursor = htons(uData);

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

    *pUint32Cursor = htonl(uData);

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

    *pUint64Cursor = uIntData;

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

    *pInt16Cursor = htons(iData);

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

    *pInt32Cursor = htonl(iData);

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

    *pInt64Cursor = iData;

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
        memcpy(
                ++pCursor,
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
    PBYTE pBlob,
    DWORD dwSize,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMDNS_ERROR(dwError);

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

    if (!pbData ||
        !pVmDnsBuffer
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCheckMemory(pVmDnsBuffer,sizeof(BOOL));
    BAIL_ON_VMDNS_ERROR(dwError);

    pCurrentPos = pVmDnsBuffer->pMessage + pVmDnsBuffer->szCursor;

    bData = *(PBOOL)pCurrentPos;

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

    if (!pbData ||
        !pVmDnsBuffer
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCheckMemory(pVmDnsBuffer,sizeof(BOOLEAN));
    BAIL_ON_VMDNS_ERROR(dwError);

    pCurrentPos = pVmDnsBuffer->pMessage + pVmDnsBuffer->szCursor;

    bData = *(PBOOLEAN)pCurrentPos;

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

    if (!pcData ||
        !pVmDnsBuffer
       )
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

    if (!pucData ||
        !pVmDnsBuffer
       )
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

    if (!puData ||
        !pVmDnsBuffer
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCheckMemory(pVmDnsBuffer,sizeof(UINT16));
    BAIL_ON_VMDNS_ERROR(dwError);

    pCurrentPos = pVmDnsBuffer->pMessage + pVmDnsBuffer->szCursor;

    uData = *(PUINT16)pCurrentPos;

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

    if (!puData ||
        !pVmDnsBuffer
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCheckMemory(pVmDnsBuffer,sizeof(UINT32));
    BAIL_ON_VMDNS_ERROR(dwError);

    pCurrentPos = pVmDnsBuffer->pMessage + pVmDnsBuffer->szCursor;

    uData = *(PUINT32)pCurrentPos;

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

    if (!puData ||
        !pVmDnsBuffer
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCheckMemory(pVmDnsBuffer,sizeof(UINT64));
    BAIL_ON_VMDNS_ERROR(dwError);

    pCurrentPos = pVmDnsBuffer->pMessage + pVmDnsBuffer->szCursor;

    uData = *(PUINT64)pCurrentPos;

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

    if (!piData ||
        !pVmDnsBuffer
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCheckMemory(pVmDnsBuffer,sizeof(INT16));
    BAIL_ON_VMDNS_ERROR(dwError);

    pCurrentPos = pVmDnsBuffer->pMessage + pVmDnsBuffer->szCursor;

    iData = *(PINT16)pCurrentPos;

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

    if (!piData ||
        !pVmDnsBuffer
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCheckMemory(pVmDnsBuffer,sizeof(INT32));
    BAIL_ON_VMDNS_ERROR(dwError);

    pCurrentPos = pVmDnsBuffer->pMessage + pVmDnsBuffer->szCursor;

    iData = *(PINT32)pCurrentPos;

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

    if (!piData ||
        !pVmDnsBuffer
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsCheckMemory(pVmDnsBuffer,sizeof(INT64));
    BAIL_ON_VMDNS_ERROR(dwError);

    pCurrentPos = pVmDnsBuffer->pMessage + pVmDnsBuffer->szCursor;

    iData = *(PINT64)pCurrentPos;

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
    PDWORD pdwStringLength
    )
{
    DWORD dwError = 0;
    DWORD dwStringLength = 0;
    PSTR pszString = NULL;
    PBYTE pCurrentPos = NULL;

    if (!pVmDnsBuffer ||
        !ppszString ||
        !pdwStringLength
       )
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
        dwError = VmDnsCheckMemory(pVmDnsBuffer, dwStringLength);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsAllocateMemory(
                                   dwStringLength+1,
                                   (PVOID *)&pszString
                                   );
        BAIL_ON_VMDNS_ERROR(dwError);

        pCurrentPos = pVmDnsBuffer->pMessage + pVmDnsBuffer->szCursor;

        memcpy(
                pszString,
                pCurrentPos,
                dwStringLength
                );

        pszString[dwStringLength] = '\0';

        pVmDnsBuffer->szCursor += dwStringLength;
    }

    *ppszString = pszString;
    *pdwStringLength = dwStringLength;
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
    VMDNS_SAFE_FREE_MEMORY(pszString);

    goto cleanup;
}

DWORD
VmDnsReadBlobFromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PBYTE *pBlob,
    PDWORD pdwSize
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMDNS_ERROR(dwError);

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
    size_t szNewSize   = 2*pVmDnsBuffer->szCurrentSize + szDataSize;
    size_t szRequired  = pVmDnsBuffer->szLength + szDataSize;

    if (szNewSize > pVmDnsBuffer->szMaxSize)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (pVmDnsBuffer->szCurrentSize < szRequired)
    {

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
        memcpy(pOutData, pInData, szData);
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


/*
 *static
 *DWORD
 *VmDnsWriteToBuffer16(
 *    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
 *    PVOID pData,
 *    size_t szDataSize
 *    )
 *{
 *    DWORD dwError = 0;
 *    size_t szRequired = pVmDnsBuffer->szLength + sizeof(INT16);
 *    PBYTE pCurrentPos = pVmDnsBuffer->pMessage + pVmDnsBuffer->szLength;
 *    INT16 i16Temp = 0;
 *    PINT16 pCursor = (PINT16) (pCurrentPos);
 *
 *    if (szRequired > pVmDnsBuffer->szCurrentSize ||
 *        szDataSize < sizeof(INT16))
 *    {
 *        dwError = ERROR_INSUFFICIENT_BUFFER;
 *        BAIL_ON_VMDNS_ERROR(dwError);
 *    }
 *
 *    i16Temp = htons(*(PINT16)pData);
 *
 *    *pCursor = i16Temp;
 *
 *    pVmDnsBuffer->szLength += sizeof(INT16);
 *
 *cleanup:
 *
 *    return dwError;
 *error:
 *
 *    goto cleanup;
 *}
 *
 *static
 *DWORD
 *VmDnsWriteToBuffer32(
 *    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
 *    PVOID pData,
 *    size_t szDataSize
 *    )
 *{
 *    DWORD dwError = 0;
 *    size_t szRequired = pVmDnsBuffer->szLength + sizeof(INT32);
 *    PBYTE pCurrentPos = pVmDnsBuffer->pMessage + pVmDnsBuffer->szLength;
 *    INT32 i32Temp = 0;
 *    PINT32 pCursor = (PINT32) (pCurrentPos);
 *
 *    if (
 *        szRequired > pVmDnsBuffer->szCurrentSize ||
 *        szDataSize < sizeof(INT32)
 *       )
 *    {
 *        dwError = ERROR_INSUFFICIENT_BUFFER;
 *        BAIL_ON_VMDNS_ERROR(dwError);
 *    }
 *
 *    i32Temp = htonl(*(PINT32)pData);
 *
 *    *pCursor = i32Temp;
 *
 *    pVmDnsBuffer->szLength += sizeof(INT32);
 *
 *cleanup:
 *
 *    return dwError;
 *error:
 *
 *    goto cleanup;
 *}
 *
 *static
 *DWORD
 *VmDnsReadFromBuffer16(
 *    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
 *    PVOID pData,
 *    size_t szDataSize
 *    )
 *{
 *    DWORD dwError = 0;
 *    INT16 i16Temp = 0;
 *    PBYTE pCurrentPos = NULL;
 *
 *    if (szDataSize < sizeof(INT16))
 *    {
 *      dwError = ERROR_INSUFFICIENT_BUFFER;
 *      BAIL_ON_VMDNS_ERROR(dwError);
 *    }
 *    if (!pVmDnsBuffer || !pData)
 *    {
 *        dwError = ERROR_INVALID_PARAMETER;
 *        BAIL_ON_VMDNS_ERROR(dwError);
 *    }
 *
 *    pCurrentPos=pVmDnsBuffer->pMessage+pVmDnsBuffer->szCursor;
 *
 *    i16Temp = ntohs(*(PINT16)pCurrentPos);
 *
 *    pVmDnsBuffer->szCursor += sizeof(INT16);
 *
 *    *(PINT16)pData = i16Temp;
 *
 *cleanup:
 *
 *    return dwError;
 *error:
 *
 *    goto cleanup;
 *}
 *
 *static
 *DWORD
 *VmDnsReadFromBuffer32(
 *    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
 *    PVOID pData,
 *    size_t szDataSize
 *    )
 *{
 *    DWORD dwError = 0;
 *    INT32 i32Temp = 0;
 *    PBYTE pCurrentPos = NULL;
 *
 *    if (szDataSize < sizeof(INT32))
 *    {
 *      dwError = ERROR_INSUFFICIENT_BUFFER;
 *      BAIL_ON_VMDNS_ERROR(dwError);
 *    }
 *    if (!pVmDnsBuffer || !pData)
 *    {
 *        dwError = ERROR_INVALID_PARAMETER;
 *        BAIL_ON_VMDNS_ERROR(dwError);
 *    }
 *
 *    pCurrentPos=pVmDnsBuffer->pMessage+pVmDnsBuffer->szCursor;
 *
 *    i32Temp = ntohl(*(PINT32)pCurrentPos);
 *
 *    pVmDnsBuffer->szCursor += sizeof(INT32);
 *
 *    *(PINT32)pData = i32Temp;
 *
 *cleanup:
 *
 *    return dwError;
 *error:
 *
 *    goto cleanup;
 *}
 *
 */
