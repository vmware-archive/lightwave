/*
 * Copyright (c) VMware Inc. 2011  All rights Reserved.
 *
 * Module Name:  vmdnsbufer.h
 *
 * Abstract: VMware Domain Name Service.
 *
 */

#ifndef _VMDNS_BUFFER_H__
#define _VMDNS_BUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif

#define VMDNS_MAX_SIZE_BUFFER 2048

typedef struct __VMDNS_MESSAGE_BUFFER *PVMDNS_MESSAGE_BUFFER;

DWORD
VmDnsAllocateBufferStream(
    size_t dwMaxSize,
    PVMDNS_MESSAGE_BUFFER *ppVmDnsBuffer
    );

VOID
VmDnsFreeBufferStream(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

DWORD
VmDnsLockBufferForWrite(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

DWORD
VmDnsAllocateBufferStreamWithBuffer(
    PBYTE pBuffer,
    size_t szBufSize,
    size_t szMaxSize,
    BOOL bCanWrite,
    PVMDNS_MESSAGE_BUFFER *ppVmDnsBuffer
    );

DWORD
VmDnsAllocateBufferStreamFromBufferStream(
    PVMDNS_MESSAGE_BUFFER pVmDnsBufferSource,
    BOOL bCanWrite,
    PVMDNS_MESSAGE_BUFFER *ppVmDnsBufferDest
    );

DWORD
VmDnsCopyBufferFromBufferStream(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PBYTE pBuffer,
    PDWORD pdwBufferSize
    );

DWORD
VmDnsWriteBoolToBuffer(
    BOOL bData,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

DWORD
VmDnsWriteBooleanToBuffer(
    BOOLEAN bData,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

DWORD
VmDnsWriteCharToBuffer(
    CHAR cData,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

DWORD
VmDnsWriteUCharToBuffer(
    UCHAR ucData,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

DWORD
VmDnsWriteUINT16ToBuffer(
    UINT16 uData,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

DWORD
VmDnsWriteUINT32ToBuffer(
    UINT32 uData,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

DWORD
VmDnsWriteUINT64ToBuffer(
    UINT64 uData,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

DWORD
VmDnsWriteINT16ToBuffer(
    INT16 iData,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

DWORD
VmDnsWriteINT32ToBuffer(
    INT32 iData,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

DWORD
VmDnsWriteINT64ToBuffer(
    INT64 iData,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

DWORD
VmDnsWriteStringToBuffer(
    PSTR pszString,
    UINT8 uStringLength,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

DWORD
VmDnsWriteBlobToBuffer(
    PBYTE pBlob,
    DWORD dwSize,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

DWORD
VmDnsReadBoolFromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PBOOL pbData
    );

DWORD
VmDnsReadBooleanFromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PBOOLEAN pbData
    );

DWORD
VmDnsReadCharFromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PCHAR pcData
    );

DWORD
VmDnsReadUCharFromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PUCHAR pucData
    );

DWORD
VmDnsReadUINT16FromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PUINT16 puData
    );

DWORD
VmDnsReadUINT32FromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PUINT32 puData
    );

DWORD
VmDnsReadUINT64FromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PUINT64 puData
    );

DWORD
VmDnsReadINT16FromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PINT16 piData
    );

DWORD
VmDnsReadINT32FromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PINT32 piData
    );

DWORD
VmDnsReadINT64FromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PINT64 piData
    );

DWORD
VmDnsReadStringFromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PSTR *ppszString,
    PDWORD pdwStringLength
    );

DWORD
VmDnsReadBlobFromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PBYTE *pBlob,
    PDWORD pdwSize
    );

DWORD
VmDnsWriteDomainNameToBuffer(
    PSTR pszDomainName,
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer
    );

DWORD
VmDnsReadDomainNameFromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PSTR *ppszDomainName
    );

DWORD
VmDnsReadRecordFromBuffer(
    PVMDNS_MESSAGE_BUFFER pVmDnsBuffer,
    PVMDNS_RECORD *ppDnsRecord
    );

#ifdef __cplusplus
}
#endif

#endif /* _VMDNS_BUFFER_H__ */
