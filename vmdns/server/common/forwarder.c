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
* Module Name:  forward.c
*
* Abstract: VMware Domain Name Service.
*
* DNS forwarding routines
*/

#include "includes.h"

static
DWORD
VmDnsGetForwarders_inlock(
    PVMDNS_FORWARDER_CONTEXT    pForwarder,
    PSTR**                      pppszForwarders,
    PDWORD                      pdwCount
    );

static
int
VmDnsForwarderLookup(
    PVMDNS_FORWARDER_CONTEXT    pForwarder,
    PCSTR                       pszForwardHost
    );

static
DWORD
VmDnsForwarderAppend(
    PVMDNS_FORWARDER_CONTEXT    pForwarder,
    PCSTR                       pszForwarders
    );

static
DWORD
VmDnsForwarderRemoveAt(
    PVMDNS_FORWARDER_CONTEXT    pForwarder,
    int                         nIndex
    );

static
VOID
VmDnsFreeForwarderPacketContext(
    PVMDNS_FORWARDER_PACKET_CONTEXT pForwarderContext
    );

static
DWORD
VmDnsPeekResponseCode(
    DWORD       dwResponseSize,
    PBYTE       pResponseBytes,
    PDWORD      pdwResponseCode
    );

static
BOOLEAN
VmDnsValidateForwarder(
    PCSTR       pszForwarder
    );

static
VOID
VmDnsFreeForwarderEntry(
    PVMDNS_FORWARDER_ENTRY pForwarderEntry
    );

static
DWORD
VmDnsForwarderMetricsInit(
    PVMDNS_FORWARDER_ENTRY   pForwarderEntry
    );


static
DWORD
VmDnsForwarderMetricsDelete(
    PVMDNS_FORWARDER_ENTRY    pForwarderEntry
    );

DWORD
VmDnsForwarderInit(
    PVMDNS_FORWARDER_CONTEXT*   ppForwarder
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_FORWARDER_CONTEXT pForwarderContext = NULL;

    BAIL_ON_VMDNS_INVALID_POINTER(ppForwarder, dwError);

    dwError = VmDnsAllocateMemory(
                        sizeof(VMDNS_FORWARDER_CONTEXT),
                        (PVOID*)&pForwarderContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateRWLock(&pForwarderContext->pLock);
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppForwarder = pForwarderContext;

cleanup:
    return dwError;

error:

    if (pForwarderContext)
    {
        VmDnsForwarderCleanup(pForwarderContext);
    }

    goto cleanup;
}

VOID
VmDnsForwarderCleanup(
    PVMDNS_FORWARDER_CONTEXT    pForwarder
    )
{
    DWORD i = 0;

    if (pForwarder)
    {
        for (i = 0; i < pForwarder->dwCount; ++i)
        {
            if (pForwarder->pForwarderEntries[i])
            {
                VmDnsFreeForwarderEntry(pForwarder->pForwarderEntries[i]);
            }
        }
        VMDNS_FREE_RWLOCK(pForwarder->pLock);
        VMDNS_SAFE_FREE_MEMORY(pForwarder);
    }
}

PVMDNS_FORWARDER_CONTEXT
VmDnsGetForwarderContext(
    )
{
    return gpSrvContext->pForwarderContext;
}

DWORD
VmDnsGetForwarders(
    PVMDNS_FORWARDER_CONTEXT    pForwarder,
    PSTR**                      pppszForwarders,
    PDWORD                      pdwCount
    )
{
    DWORD dwError = 0;
    BOOL bLocked = FALSE;

    VMDNS_LOCKREAD(pForwarder->pLock);
    bLocked = TRUE;

    dwError = VmDnsGetForwarders_inlock(
                         pForwarder,
                         pppszForwarders,
                         pdwCount
                         );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    if (bLocked)
    {
        VMDNS_UNLOCKREAD(pForwarder->pLock);
    }
    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsGetForwarderAtIndex(
    PVMDNS_FORWARDER_CONTEXT    pForwarder,
    DWORD                       dwIndex,
    PSTR*                       ppszForwarder
    )
{
    DWORD dwError = 0;
    PSTR  pszForwarder = NULL;
    BOOL  bLocked = FALSE;

    BAIL_ON_VMDNS_INVALID_POINTER(pForwarder, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppszForwarder, dwError);

    VMDNS_LOCKREAD(pForwarder->pLock);
    bLocked = TRUE;

    if (dwIndex >= pForwarder->dwCount)
    {
        dwError = ERROR_INVALID_INDEX;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateStringA(
                            pForwarder->pForwarderEntries[dwIndex]->pszForwarder,
                            &pszForwarder
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    *ppszForwarder = pszForwarder;
cleanup:

    if (bLocked)
    {
        VMDNS_UNLOCKREAD(pForwarder->pLock);
    }
    return dwError;
error:

    if (ppszForwarder)
    {
        *ppszForwarder = NULL;
    }
    VMDNS_SAFE_FREE_STRINGA(pszForwarder);
    goto cleanup;
}

DWORD
VmDnsSetForwarders(
    PVMDNS_FORWARDER_CONTEXT    pForwarder,
    DWORD                       dwCount,
    PSTR*                       ppszForwarders
    )
{
    DWORD dwError = 0;
    DWORD dwCurrentCount, i = 0;
    BOOL bLocked = FALSE;

    BAIL_ON_VMDNS_INVALID_POINTER(pForwarder, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(ppszForwarders, dwError);

    VMDNS_LOCKWRITE(pForwarder->pLock);
    bLocked = TRUE;

    dwCurrentCount = pForwarder->dwCount;
    for (i = 0; i < dwCurrentCount; ++i)
    {
        VmDnsFreeForwarderEntry(pForwarder->pForwarderEntries[i]);
    }

    pForwarder->dwCount = 0;

    for (i = 0; i < dwCount; ++i)
    {
        dwError = VmDnsForwarderAppend(
                            pForwarder,
                            ppszForwarders[i]);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:

    if (bLocked)
    {
        VMDNS_UNLOCKWRITE(pForwarder->pLock);
    }
    return dwError;

error:

    goto cleanup;
}


DWORD
VmDnsAddForwarder(
    PVMDNS_FORWARDER_CONTEXT    pForwarder,
    PCSTR                       pszForwarder
    )
{
    DWORD dwError = 0;
    BOOL bLocked = FALSE;
    PSTR* ppszForwarders = NULL;
    DWORD dwCount = 0;

    BAIL_ON_VMDNS_INVALID_POINTER(pForwarder, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pszForwarder, dwError);

    if (!VmDnsValidateForwarder(pszForwarder))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    VMDNS_LOCKWRITE(pForwarder->pLock);
    bLocked = TRUE;

    int index = VmDnsForwarderLookup(
                        pForwarder,
                        pszForwarder);
    if (index < 0)
    {
        dwError = VmDnsForwarderAppend(
                        pForwarder,
                        pszForwarder);
        BAIL_ON_VMDNS_ERROR(dwError);

        VMDNS_UNLOCKWRITE(pForwarder->pLock);
        bLocked = FALSE;

        dwError = VmDnsGetForwarders_inlock(
                        pForwarder,
                        &ppszForwarders,
                        &dwCount);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsStoreSaveForwarders(
                        pForwarder->dwCount,
                        ppszForwarders);
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        dwError = ERROR_ALREADY_EXISTS;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:

    if (ppszForwarders)
    {
        VmDnsFreeStringCountedArrayA(ppszForwarders, dwCount);
    }

    if (bLocked)
    {
        VMDNS_UNLOCKWRITE(pForwarder->pLock);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsDeleteForwarder(
    PVMDNS_FORWARDER_CONTEXT    pForwarder,
    PCSTR                       pszForwarder
    )
{
    DWORD dwError = 0;
    DWORD index = 0;
    BOOL bLocked = FALSE;
    PSTR* ppszForwarders = NULL;
    DWORD dwCount = 0;

    BAIL_ON_VMDNS_INVALID_POINTER(pForwarder, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pszForwarder, dwError);

    VMDNS_LOCKWRITE(pForwarder->pLock);
    bLocked = TRUE;

    index = VmDnsForwarderLookup(
                            pForwarder,
                            pszForwarder);
    if (index != -1)
    {
        dwError = VmDnsForwarderRemoveAt(
                            pForwarder,
                            index);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsGetForwarders_inlock(
                            pForwarder,
                            &ppszForwarders,
                            &dwCount);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsStoreSaveForwarders(
                            pForwarder->dwCount,
                            ppszForwarders);
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        dwError = ERROR_NOT_FOUND;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:

    if (ppszForwarders)
    {
        VmDnsFreeStringCountedArrayA(ppszForwarders, dwCount);
    }

    if (bLocked)
    {
        VMDNS_UNLOCKWRITE(pForwarder->pLock);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsGetForwarders_inlock(
    PVMDNS_FORWARDER_CONTEXT    pForwarder,
    PSTR**                      pppszForwarders,
    PDWORD                      pdwCount
    )
{
    DWORD dwError = 0;
    PSTR* pszForwarders = NULL;
    DWORD dwCount = 0, i = 0;

    BAIL_ON_VMDNS_INVALID_POINTER(pForwarder, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pppszForwarders, dwError);
    BAIL_ON_VMDNS_INVALID_POINTER(pdwCount, dwError);

    dwCount = pForwarder->dwCount;

    dwError = VmDnsAllocateMemory(
                        (dwCount + 1) * sizeof(PSTR),
                        (PVOID*)&pszForwarders);
    BAIL_ON_VMDNS_ERROR(dwError);

    for (; i < dwCount; ++i)
    {
        dwError = VmDnsAllocateStringA(
                            pForwarder->pForwarderEntries[i]->pszForwarder,
                            &pszForwarders[i]);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    *pdwCount = dwCount;
    *pppszForwarders = pszForwarders;

cleanup:

    return dwError;

error:
    VmDnsFreeStringArrayA(pszForwarders);

    goto cleanup;
}

static int
VmDnsForwarderLookup(
    PVMDNS_FORWARDER_CONTEXT    pForwarder,
    PCSTR                       pszForwardHost
    )
{
    int index = -1;
    DWORD i = 0;

    for (i = 0; i < pForwarder->dwCount; ++i)
    {
        int match = VmDnsStringCompareA(
                                pForwarder->pForwarderEntries[i]->pszForwarder,
                                pszForwardHost,
                                FALSE);
        if (match == 0)
        {
            index = i;
            break;
        }
    }

    return index;
}

DWORD
VmDnsForwarderAppend(
    PVMDNS_FORWARDER_CONTEXT    pForwarder,
    PCSTR                       pszForwarder
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PSTR szForwarderCopy = NULL;
    DWORD dwCount = pForwarder->dwCount;

    if (dwCount >= VMDNS_MAX_NUM_FORWARDS)
    {
        dwError = ERROR_OUT_OF_RANGE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateStringA(pszForwarder, &szForwarderCopy);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateMemory(
                        sizeof(VMDNS_FORWARDER_ENTRY),
                        (void**)&pForwarder->pForwarderEntries[dwCount]);
    BAIL_ON_VMDNS_ERROR(dwError);

    pForwarder->pForwarderEntries[dwCount]->pszForwarder = szForwarderCopy;
    szForwarderCopy = NULL;

    dwError = VmDnsForwarderMetricsInit(pForwarder->pForwarderEntries[dwCount]);
    BAIL_ON_VMDNS_ERROR(dwError);

    pForwarder->dwCount++;

cleanup:

    return dwError;
error:

    if (dwCount < VMDNS_MAX_NUM_FORWARDS && pForwarder->pForwarderEntries[dwCount])
    {
        VmDnsFreeForwarderEntry(pForwarder->pForwarderEntries[dwCount]);
    }
    VMDNS_SAFE_FREE_STRINGA(szForwarderCopy);

    goto cleanup;
}

DWORD
VmDnsForwarderRemoveAt(
    PVMDNS_FORWARDER_CONTEXT    pForwarder,
    int                         nIndex
    )
{
    DWORD dwError = 0;

    if ((DWORD)nIndex >= pForwarder->dwCount)
    {
        dwError = ERROR_OUT_OF_RANGE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    VmDnsFreeForwarderEntry(pForwarder->pForwarderEntries[nIndex]);
    pForwarder->pForwarderEntries[nIndex] = NULL;

#ifndef WIN32
    memmove(&pForwarder->pForwarderEntries[nIndex],
            &pForwarder->pForwarderEntries[nIndex+1],
            sizeof(PVMDNS_FORWARDER_ENTRY) * (pForwarder->dwCount - (nIndex + 1)));
#else
    memmove_s(&pForwarder->pForwarderEntries[nIndex],
            sizeof(PVMDNS_FORWARDER_ENTRY) * (pForwarder->dwCount - nIndex),
            &pForwarder->pForwarderEntries[nIndex + 1],
            sizeof(PVMDNS_FORWARDER_ENTRY) * (pForwarder->dwCount - (nIndex + 1)));
#endif
    pForwarder->pForwarderEntries[--pForwarder->dwCount] = NULL;

cleanup:

    return dwError;
error:

    goto cleanup;
}

DWORD
VmDnsAllocateForwarderPacketContext(
   PVMDNS_FORWARDER_PACKET_CONTEXT* ppForwarderContext
   )
{
    DWORD dwError = 0;

    PVMDNS_FORWARDER_PACKET_CONTEXT pForwarderContext = NULL;

    if (!ppForwarderContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(
                          sizeof(VMDNS_FORWARDER_PACKET_CONTEXT),
                          (PVOID*)&pForwarderContext
                          );
    BAIL_ON_VMDNS_ERROR(dwError);

    pForwarderContext->dwRefCount = 1;

    *ppForwarderContext = pForwarderContext;
cleanup:

    return dwError;
error:

    if (ppForwarderContext)
    {
        *ppForwarderContext = NULL;
    }
    if (pForwarderContext)
    {
        VmDnsFreeForwarderPacketContext(pForwarderContext);
    }
    goto cleanup;
}

PVMDNS_FORWARDER_PACKET_CONTEXT
VmDnsAcquireForwarderPacketContext(
    PVMDNS_FORWARDER_PACKET_CONTEXT pForwarderContext
    )
{
    if (pForwarderContext)
    {
        InterlockedIncrement(&pForwarderContext->dwRefCount);
    }

    return pForwarderContext;
}

VOID
VmDnsReleaseForwarderPacketContext(
    PVMDNS_FORWARDER_PACKET_CONTEXT pForwarderContext
    )
{
    if (pForwarderContext)
    {
        if (InterlockedDecrement(&pForwarderContext->dwRefCount) == 0)
        {
            VmDnsFreeForwarderPacketContext(pForwarderContext);
        }
    }
}

DWORD
VmDnsForwardRequest(
    PVMDNS_FORWARDER_PACKET_CONTEXT      pForwarderPacketContext,
    BOOL                                 bUseUDP,
    PVM_SOCK_IO_BUFFER                   pIoBuffer
    )
{
    DWORD dwError = 0;
    PSTR  pszForwarder = NULL;
    VM_SOCK_CREATE_FLAGS flags = (bUseUDP) ? VM_SOCK_CREATE_FLAGS_UDP : VM_SOCK_CREATE_FLAGS_TCP;
    PVM_SOCKET pSocket = NULL;

    DWORD dwQuerySize = 0;
    PBYTE pQueryBuffer = NULL;
    struct sockaddr_storage address;
    socklen_t addLenth = sizeof address;
    PVM_SOCK_IO_BUFFER pIoRequest = NULL;
    PVM_SOCK_IO_BUFFER pOldRequest = NULL;
    PVMDNS_FORWARDER_PACKET_CONTEXT pCurrentContext = NULL;

    if (!pIoBuffer || (bUseUDP && (pIoBuffer->dwTotalBytesTransferred > VMDNS_UDP_PACKET_SIZE))||
        !pIoBuffer->pData ||
        !pForwarderPacketContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwQuerySize = pIoBuffer->dwTotalBytesTransferred;
    pQueryBuffer = pIoBuffer->pData;

    pCurrentContext = VmDnsAcquireForwarderPacketContext(pForwarderPacketContext);

    dwError = VmDnsGetForwarderAtIndex(
                                gpSrvContext->pForwarderContext,
                                pCurrentContext->dwCurrentIndex++,
                                &pszForwarder
                                );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSockOpenClient(
                    pszForwarder,
                    VMW_DNS_PORT,
                    flags,
                    &pSocket);
    BAIL_ON_VMDNS_ERROR(dwError);
#if 1 /* TBD:Adam-This is a hacked version of what Aishu checked into dev */
{
typedef enum
{
    VM_SOCK_PROTOCOL_UNKNOWN = 0,
    VM_SOCK_PROTOCOL_TCP,
    VM_SOCK_PROTOCOL_UDP
} VM_SOCK_PROTOCOL;

typedef enum
{
    VM_SOCK_TYPE_UNKNOWN = 0,
    VM_SOCK_TYPE_CLIENT,
    VM_SOCK_TYPE_SERVER,
    VM_SOCK_TYPE_LISTENER,
    VM_SOCK_TYPE_SIGNAL
} VM_SOCK_TYPE;

typedef struct _VM_SOCKET
{
    LONG             refCount;

    VM_SOCK_TYPE     type;
    VM_SOCK_PROTOCOL protocol;

    struct sockaddr  addr;
    socklen_t        addrLen;
    struct sockaddr* pAddr;   // Do not free

    PVMDNS_MUTEX       pMutex;

    int              fd;

    PVOID            pData;

} VM_SOCKET;

    struct timeval tv = {0};
    VM_SOCKET *ps = (VM_SOCKET *) pSocket;
    tv.tv_sec = 5;

    setsockopt(ps->fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(ps->fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}
#endif

    dwError = VmDnsSockSetNonBlocking(pSocket);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSockGetAddress(
                    pSocket,
                    &address,
                    &addLenth);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSockAllocateIoBuffer(
                    bUseUDP?
                    VM_SOCK_EVENT_TYPE_UDP_FWD_RESPONSE_DATA_READ:
                    VM_SOCK_EVENT_TYPE_TCP_FWD_RESPONSE_DATA_READ,
                    (PVM_SOCK_EVENT_CONTEXT)pCurrentContext,
                    dwQuerySize,
                    &pIoRequest);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyMemory(
                    pIoRequest->pData,
                    pIoRequest->dwExpectedSize,
                    pQueryBuffer,
                    dwQuerySize);
    BAIL_ON_VMDNS_ERROR(dwError);

    memcpy(
        &pIoRequest->clientAddr,
        &pIoBuffer->clientAddr,
        pIoBuffer->addrLen);

    pIoRequest->addrLen = pIoBuffer->addrLen;
    pIoRequest->pClientSocket = VmDnsSockAcquire(pIoBuffer->pClientSocket);

    dwError = VmDnsSockSetData(
                            pSocket,
                            pIoRequest,
                            (PVOID*)&pOldRequest
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSockWrite(
                    pSocket,
                    NULL,
                    0,
                    pIoRequest);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSockEventQueueAdd(
                                gpSrvContext->pSockContext->pEventQueue,
                                TRUE,
                                pSocket
                                );
    BAIL_ON_VMDNS_ERROR(dwError);

//    VmDnsOPStatisticUpdate(FORWARDER_QUERY_COUNT);

cleanup:

    VMDNS_SAFE_FREE_MEMORY(pszForwarder);

    if (pOldRequest)
    {
        VMDNS_LOG_IO_RELEASE(pOldRequest);
        VmDnsSockReleaseIoBuffer(pOldRequest);
    }
    if (pSocket)
    {
        VmDnsSockRelease(pSocket);
    }

    return dwError;

error:

    if (pCurrentContext)
    {
        VmDnsReleaseForwarderPacketContext(pCurrentContext);
    }
    if (pIoRequest)
    {
        VMDNS_LOG_IO_RELEASE(pIoRequest);
        VmDnsSockReleaseIoBuffer(pIoRequest);
    }
    goto cleanup;
}

DWORD
VmDnsForwardResponse(
    BOOL        bUseUDP,
    PVM_SOCKET  pSocket,
    PBYTE*      ppResponse,
    PDWORD      pdwResponseSize,
    PDWORD      pdwRCode
    )
{
    DWORD dwError = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseCode = 0;
    UINT16 usExpectedSize = 0;
    PVM_SOCK_IO_BUFFER pIoSizeResponse = NULL;
    PVM_SOCK_IO_BUFFER pIoDataResponse = NULL;

    if (!pSocket ||
        !ppResponse ||
        !pdwResponseSize ||
        !pdwRCode
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (bUseUDP)
    {
        dwError = VmDnsSockAllocateIoBuffer(
                        VM_SOCK_EVENT_TYPE_UNKNOWN,
                        NULL,
                        VMDNS_UDP_PACKET_SIZE,
                        &pIoDataResponse);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsSockRead(
                        pSocket,
                        pIoDataResponse);
       BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        dwError = VmDnsSockAllocateIoBuffer(
                        VM_SOCK_EVENT_TYPE_UNKNOWN,
                        NULL,
                        sizeof(UINT16),
                        &pIoSizeResponse);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsSockRead(
                        pSocket,
                        pIoSizeResponse);
        BAIL_ON_VMDNS_ERROR(dwError);

        usExpectedSize = htons(*((UINT*)pIoSizeResponse->pData));

        dwError = VmDnsSockAllocateIoBuffer(
                        VM_SOCK_EVENT_TYPE_UNKNOWN,
                        NULL,
                        usExpectedSize,
                        &pIoDataResponse);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsSockRead(
                        pSocket,
                        pIoDataResponse);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(
                        pIoDataResponse->dwCurrentSize,
                        (PVOID*)&pResponse);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyMemory(
                      pResponse,
                      pIoDataResponse->dwCurrentSize,
                      pIoDataResponse->pData,
                      pIoDataResponse->dwCurrentSize);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsPeekResponseCode(
                      pIoDataResponse->dwCurrentSize,
                      pResponse,
                      &dwResponseCode
                      );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSockEventQueueRemove(
                                gpSrvContext->pSockContext->pEventQueue,
                                pSocket
                                );
    dwError = 0;

    *ppResponse = pResponse;
    *pdwResponseSize = pIoDataResponse->dwCurrentSize;
    *pdwRCode = dwResponseCode;
    //VmDnsOPStatisticUpdate(FORWARDER_QUERY_COUNT);

cleanup:

    if (pIoSizeResponse)
    {
      
        VMDNS_LOG_IO_RELEASE(pIoSizeResponse);
        VmDnsSockReleaseIoBuffer(pIoSizeResponse);
    }
    if (pIoDataResponse)
    {
        VMDNS_LOG_IO_RELEASE(pIoDataResponse);
        VmDnsSockReleaseIoBuffer(pIoDataResponse);
    }

    return dwError;

error:

    if (pdwResponseSize)
    {
        *pdwResponseSize = 0;
    }

    if (ppResponse)
    {
        *ppResponse = NULL;
    }

    if (pdwRCode)
    {
        *pdwRCode = 0;
    }

    if (pResponse)
    {
        VmDnsFreeMemory(pResponse);
    }

    goto cleanup;
}

static
VOID
VmDnsFreeForwarderEntry(
    PVMDNS_FORWARDER_ENTRY pForwarderEntry
    )
{
    if (pForwarderEntry)
    {
        (void) VmDnsForwarderMetricsDelete(pForwarderEntry);
        VMDNS_SAFE_FREE_STRINGA(pForwarderEntry->pszForwarder);
        VMDNS_SAFE_FREE_MEMORY(pForwarderEntry);
    }
}

static
VOID
VmDnsFreeForwarderPacketContext(
    PVMDNS_FORWARDER_PACKET_CONTEXT pForwarderContext
    )
{
    if (pForwarderContext)
    {
        VMDNS_SAFE_FREE_MEMORY(pForwarderContext);
    }
}

static
DWORD
VmDnsPeekResponseCode(
    DWORD dwResponseSize,
    PBYTE pResponseBytes,
    PDWORD pdwResponseCode
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVMDNS_HEADER pDnsHeader = NULL;
    PVMDNS_MESSAGE_BUFFER pDnsMessageBuffer = NULL;

    if (!pdwResponseCode)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    dwError = VmDnsGetDnsMessageBuffer(
                        pResponseBytes,
                        dwResponseSize,
                        &pDnsMessageBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsReadDnsHeaderFromBuffer(
                        pDnsMessageBuffer,
                        &pDnsHeader
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    *pdwResponseCode =  pDnsHeader->codes.RCODE;

cleanup:

    if (pDnsMessageBuffer)
    {
       VmDnsFreeBufferStream(pDnsMessageBuffer);
    }
    VMDNS_SAFE_FREE_MEMORY(pDnsHeader);

    return dwError;

error :

    goto cleanup;

}

static
BOOLEAN
VmDnsValidateForwarder(
    PCSTR       pszForwarder
    )
{
    if (!pszForwarder ||
        pszForwarder[0] == '\0' ||
        VmDnsStringChrA(pszForwarder, '.') == NULL
        )
    {
        return FALSE;
    }

    return TRUE;
}

static
DWORD
VmDnsForwarderMetricsInit(
    PVMDNS_FORWARDER_ENTRY pForwarderEntry
    )
{
    DWORD dwError = 0;
    UINT64 buckets[] = {1, 10, 100, 300, 1000};
    VM_METRICS_LABEL labelDurationOps[2][2] = {{{"operation","query"},{"forwarder",""}},
                                               {{"operation","update"},{"forwarder",""}}};

    labelDurationOps[0][1].pszValue = pForwarderEntry->pszForwarder;
    labelDurationOps[1][1].pszValue = pForwarderEntry->pszForwarder;

    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmMetricsHistogramNew(
                gVmDnsMetricsContext,
                "vmdns_forwarder_request_duration",
                labelDurationOps[0],
                2,
                "Forwarder Process Request Duration",
                buckets,
                5,
                &pForwarderEntry->ForwarderMetricsContext.pQueryDuration
                );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmMetricsHistogramNew(
                gVmDnsMetricsContext,
                "vmdns_forwarder_request_duration",
                labelDurationOps[1],
                2,
                "Forwarder Process Request Duration",
                buckets,
                5,
                &pForwarderEntry->ForwarderMetricsContext.pUpdateDuration
                );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    return dwError;

error:
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

static
DWORD
VmDnsForwarderMetricsDelete(
    PVMDNS_FORWARDER_ENTRY    pForwarderEntry
    )
{
    DWORD dwError = 0;
    BAIL_ON_VMDNS_ERROR(dwError);

/*    dwError = VmMetricsHistogramDelete(
                          gVmDnsMetricsContext,
                          pForwarderEntry->ForwarderMetricsContext.pQueryDuration);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmMetricsHistogramDelete(
                          gVmDnsMetricsContext,
                          pForwarderEntry->ForwarderMetricsContext.pUpdateDuration);
    BAIL_ON_VMDNS_ERROR(dwError);
*/

cleanup:
    return dwError;

error:
    VmDnsLog(VMDNS_LOG_LEVEL_ERROR, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}
