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

static
DWORD
VmDnsSockPosixCreateSignalSockets(
    PVM_SOCKET* ppReaderSocket,
    PVM_SOCKET* ppWriterSocket
    );

static
DWORD
VmDnsSockPosixEventQueueAdd_inlock(
    PVM_SOCK_EVENT_QUEUE pQueue,
    BOOL                 bOneShot,
    PVM_SOCKET           pSocket
    );

static
DWORD
VmDnsSockPosixEventQueueDelete_inlock(
    PVM_SOCK_EVENT_QUEUE pQueue,
    PVM_SOCKET           pSocket
    );

static
DWORD
VmDnsSockPosixAcceptConnection(
    PVM_SOCKET  pListener,
    PVM_SOCKET* ppSocket
    );

static
DWORD
VmDnsSockPosixSetDescriptorNonBlocking(
    int fd
    );

static
DWORD
VmDnsSockPosixSetReuseAddress(
    int fd
    );

static
VOID
VmDnsSockPosixFreeSocket(
    PVM_SOCKET pSocket
    );

DWORD
VmDnsSockPosixOpenClient(
    PCSTR                pszHost,
    USHORT               usPort,
    VM_SOCK_CREATE_FLAGS dwFlags,
    PVM_SOCKET*          ppSocket
    )
{
    DWORD dwError = 0;
    struct addrinfo  hints   = {0};
    struct addrinfo* pAddrInfo = NULL;
    struct addrinfo* pInfo = NULL;
    int fd = -1;
    PVM_SOCKET pSocket = NULL;
    CHAR szPort[32];

    if (!pszHost || !usPort || !ppSocket)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }

    sprintf(szPort, "%d", usPort);

    if (dwFlags & VM_SOCK_CREATE_FLAGS_IPV6)
    {
        hints.ai_family = AF_INET6;
    }
    else if (dwFlags & VM_SOCK_CREATE_FLAGS_IPV4)
    {
        hints.ai_family = AF_INET;
    }
    else
    {
        hints.ai_family = AF_UNSPEC;
    }

    if (dwFlags & VM_SOCK_CREATE_FLAGS_UDP)
    {
        hints.ai_socktype = SOCK_DGRAM;
    }
    else
    {
        hints.ai_socktype = SOCK_STREAM;
    }

    hints.ai_flags    = AI_CANONNAME | AI_NUMERICSERV;

    /* This will use DNS */
    if (getaddrinfo(pszHost, szPort, &hints, &pAddrInfo) != 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }

    for (pInfo = pAddrInfo; (fd < 0) && (pInfo != NULL); pInfo = pInfo->ai_next)
    {
        fd = socket(pInfo->ai_family, pInfo->ai_socktype, pInfo->ai_protocol);

        if (fd < 0)
        {
            continue;
        }

        if (connect(fd, pInfo->ai_addr, pInfo->ai_addrlen) < 0)
        {
            close(fd);
            fd = -1;

            continue;
        }

        break;
    }

    if (fd < 0)
    {
        dwError = ERROR_CONNECTION_UNAVAIL;
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }

    if (dwFlags & VM_SOCK_CREATE_FLAGS_NON_BLOCK)
    {
        dwError = VmDnsSockPosixSetDescriptorNonBlocking(fd);
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(sizeof(*pSocket), (PVOID*)&pSocket);
    BAIL_ON_POSIX_SOCK_ERROR(dwError);

    pSocket->refCount = 1;

    pSocket->type = VM_SOCK_TYPE_CLIENT;

    if (dwFlags & VM_SOCK_CREATE_FLAGS_UDP)
    {
        pSocket->protocol = VM_SOCK_PROTOCOL_UDP;
    }
    else
    {
        pSocket->protocol = VM_SOCK_PROTOCOL_TCP;
    }

    dwError = VmDnsAllocateMutex(&pSocket->pMutex);
    BAIL_ON_POSIX_SOCK_ERROR(dwError);

    memcpy(&pSocket->addr, pInfo->ai_addr, pInfo->ai_addrlen);
    pSocket->addrLen = pInfo->ai_addrlen;
    pSocket->fd = fd;

    *ppSocket = pSocket;

cleanup:

    if (pAddrInfo)
    {
        freeaddrinfo(pAddrInfo);
    }

    return dwError;

error:

    if (ppSocket)
    {
        *ppSocket = NULL;
    }
    if (pSocket)
    {
        VmDnsSockPosixFreeSocket(pSocket);
    }
    if (fd >= 0)
    {
        close(fd);
    }

    goto cleanup;
}

DWORD
VmDnsSockPosixOpenServer(
    USHORT               usPort,
    int                  iListenQueueSize,
    VM_SOCK_CREATE_FLAGS dwFlags,
    PVM_SOCKET*          ppSocket
    )
{
    DWORD dwError = 0;
    union
    {
#ifdef AF_INET6
        struct sockaddr_in6 servaddr_ipv6;
#endif
        struct sockaddr_in  servaddr_ipv4;
    } servaddr;
    struct
    {
        int domain;
        int type;
        int protocol;
    } socketParams;
    struct sockaddr* pSockAddr = NULL;
    socklen_t addrLen = 0;
    int fd = -1;
    PVM_SOCKET pSocket = NULL;

    if (dwFlags & VM_SOCK_CREATE_FLAGS_IPV6)
    {
#ifdef AF_INET6
        socketParams.domain = AF_INET6;
#else
        dwError = ERROR_NOT_SUPPORTED;
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
#endif
    }
    else
    {
        socketParams.domain = AF_INET;
    }

    if (dwFlags & VM_SOCK_CREATE_FLAGS_UDP)
    {
        socketParams.type = SOCK_DGRAM;
    }
    else
    {
        socketParams.type = SOCK_STREAM;
    }

    socketParams.protocol = 0;

    fd = socket(socketParams.domain, socketParams.type, socketParams.protocol);
    if (fd < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }

    if (dwFlags & VM_SOCK_CREATE_FLAGS_REUSE_ADDR)
    {
        dwError = VmDnsSockPosixSetReuseAddress(fd);
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    if (dwFlags & VM_SOCK_CREATE_FLAGS_IPV6)
    {
#ifdef AF_INET6
        servaddr.servaddr_ipv6.sin6_family = AF_INET6;
        servaddr.servaddr_ipv6.sin6_addr = in6addr_any;
        servaddr.servaddr_ipv6.sin6_port = htons(usPort);

        pSockAddr = (struct sockaddr*) &servaddr.servaddr_ipv6;
        addrLen = sizeof(servaddr.servaddr_ipv6);

#if defined(SOL_IPV6) && defined(IPV6_V6ONLY)
        int one = 1;
        setsockopt(fd, SOL_IPV6, IPV6_V6ONLY, (void *) &one, sizeof(one));
#endif

#else
        dwError = ERROR_NOT_SUPPORTED;
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
#endif
    }
    else
    {
        servaddr.servaddr_ipv4.sin_family = AF_INET;
        servaddr.servaddr_ipv4.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.servaddr_ipv4.sin_port = htons(usPort);

        pSockAddr = (struct sockaddr*) &servaddr.servaddr_ipv4;
        addrLen = sizeof(servaddr.servaddr_ipv4);
    }

    if (bind(fd, pSockAddr, addrLen) < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }

    if (dwFlags & VM_SOCK_CREATE_FLAGS_NON_BLOCK)
    {
        dwError = VmDnsSockPosixSetDescriptorNonBlocking(fd);
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }

    if (!(dwFlags & VM_SOCK_CREATE_FLAGS_UDP))
    {
        if (iListenQueueSize <= 0)
        {
            iListenQueueSize = VM_SOCK_POSIX_DEFAULT_LISTEN_QUEUE_SIZE;
        }

        if (listen(fd, iListenQueueSize) < 0)
        {
            dwError = LwErrnoToWin32Error(errno);
            BAIL_ON_POSIX_SOCK_ERROR(dwError);
        }
    }

    dwError = VmDnsAllocateMemory(sizeof(*pSocket), (PVOID*)&pSocket);
    BAIL_ON_POSIX_SOCK_ERROR(dwError);

    pSocket->refCount = 1;

    dwError = VmDnsAllocateMutex(&pSocket->pMutex);
    BAIL_ON_POSIX_SOCK_ERROR(dwError);

    pSocket->type = VM_SOCK_TYPE_LISTENER;

    if (dwFlags & VM_SOCK_CREATE_FLAGS_UDP)
    {
        pSocket->protocol = VM_SOCK_PROTOCOL_UDP;
    }
    else
    {
        pSocket->protocol = VM_SOCK_PROTOCOL_TCP;
    }

    pSocket->fd = fd;

    *ppSocket = pSocket;

cleanup:

    return dwError;

error:

    if (ppSocket)
    {
        *ppSocket = NULL;
    }

    if (pSocket)
    {
        VmDnsSockPosixFreeSocket(pSocket);
    }
    if (fd >= 0)
    {
        close(fd);
    }

    goto cleanup;
}

DWORD
VmDnsSockPosixCreateEventQueue(
    int                   iEventQueueSize,
    PVM_SOCK_EVENT_QUEUE* ppQueue
    )
{
    DWORD dwError = 0;
    PVM_SOCK_EVENT_QUEUE pQueue = NULL;

    if (!ppQueue)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }

    if (iEventQueueSize <= 0)
    {
        iEventQueueSize = VM_SOCK_POSIX_DEFAULT_QUEUE_SIZE;
    }

    dwError = VmDnsAllocateMemory(sizeof(*pQueue), (PVOID*)&pQueue);
    BAIL_ON_POSIX_SOCK_ERROR(dwError);

    dwError = VmDnsSockPosixCreateSignalSockets(
                    &pQueue->pSignalReader,
                    &pQueue->pSignalWriter);
    BAIL_ON_POSIX_SOCK_ERROR(dwError);

    dwError = VmDnsAllocateMutex(&pQueue->pMutex);
    BAIL_ON_POSIX_SOCK_ERROR(dwError);

    dwError = VmDnsAllocateMemory(
                    iEventQueueSize * sizeof(*pQueue->pEventArray),
                    (PVOID*)&pQueue->pEventArray);
    BAIL_ON_POSIX_SOCK_ERROR(dwError);

    pQueue->dwSize = iEventQueueSize;

    pQueue->epollFd = epoll_create(pQueue->dwSize);
    if (pQueue->epollFd < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }

    pQueue->state  = VM_SOCK_POSIX_EVENT_STATE_WAIT;
    pQueue->nReady = -1;
    pQueue->iReady = 0;

    dwError = VmDnsSockPosixEventQueueAdd_inlock(pQueue, FALSE, pQueue->pSignalReader);
    BAIL_ON_POSIX_SOCK_ERROR(dwError);

    *ppQueue = pQueue;

cleanup:

    return dwError;

error:

    if (ppQueue)
    {
        *ppQueue = NULL;
    }

    if (pQueue)
    {
        VmDnsSockPosixFreeEventQueue(pQueue);
    }

    goto cleanup;
}

DWORD
VmDnsSockPosixEventQueueAdd(
    PVM_SOCK_EVENT_QUEUE pQueue,
    BOOL                 bOneShot,
    PVM_SOCKET           pSocket
    )
{
    DWORD   dwError = 0;

    if (!pQueue || !pSocket)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }

    dwError = VmDnsSockPosixEventQueueAdd_inlock(pQueue, bOneShot,pSocket);
    BAIL_ON_POSIX_SOCK_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsSockPosixEventQueueRemove(
    PVM_SOCK_EVENT_QUEUE pQueue,
    PVM_SOCKET           pSocket
    )
{
    DWORD   dwError = 0;

    if (!pQueue || !pSocket)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }

    dwError = VmDnsSockPosixEventQueueDelete_inlock(pQueue, pSocket);
    BAIL_ON_POSIX_SOCK_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsSockPosixWaitForEvent(
    PVM_SOCK_EVENT_QUEUE pQueue,
    int                  iTimeoutMS,
    PVM_SOCKET*          ppSocket,
    PVM_SOCK_EVENT_TYPE  pEventType,
    PVM_SOCK_IO_BUFFER*  ppIoBuffer
    )
{
    DWORD  dwError = 0;
    BOOLEAN bLocked = FALSE;
    VM_SOCK_EVENT_TYPE eventType = VM_SOCK_EVENT_TYPE_UNKNOWN;
    PVM_SOCKET pSocket = NULL;
    PVM_SOCK_IO_BUFFER pIoBuffer = NULL;
    PVM_SOCK_IO_CONTEXT pIoContext = NULL;

    if (!pQueue || !ppSocket || !pEventType)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }

    dwError = VmDnsLockMutex(pQueue->pMutex);
    BAIL_ON_POSIX_SOCK_ERROR(dwError);

    bLocked = TRUE;

    if (pQueue->bShutdown)
    {
        dwError = ERROR_SHUTDOWN_IN_PROGRESS;
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }


    if ((pQueue->state == VM_SOCK_POSIX_EVENT_STATE_PROCESS) &&
        (pQueue->iReady >= pQueue->nReady))
    {
        pQueue->state = VM_SOCK_POSIX_EVENT_STATE_WAIT;
    }

    if (pQueue->state == VM_SOCK_POSIX_EVENT_STATE_WAIT)
    {
        pQueue->iReady = 0;
        pQueue->nReady = -1;

        while (pQueue->nReady < 0)
        {
            pQueue->nReady = epoll_wait(
                                pQueue->epollFd,
                                pQueue->pEventArray,
                                pQueue->dwSize,
                                iTimeoutMS);

            if ((pQueue->nReady < 0) && (errno != EINTR))
            {
                dwError = LwErrnoToWin32Error(errno);
                BAIL_ON_POSIX_SOCK_ERROR(dwError);
            }

            if (pQueue->bShutdown)
            {
                dwError = ERROR_SHUTDOWN_IN_PROGRESS;
                BAIL_ON_POSIX_SOCK_ERROR(dwError);
            }
        }

        pQueue->state = VM_SOCK_POSIX_EVENT_STATE_PROCESS;
    }

    if (pQueue->state == VM_SOCK_POSIX_EVENT_STATE_PROCESS)
    {
        if (pQueue->iReady < pQueue->nReady)
        {
            struct epoll_event* pEvent = &pQueue->pEventArray[pQueue->iReady];
            PVM_SOCKET pEventSocket = (PVM_SOCKET)pEvent->data.ptr;

            if (!pEventSocket)
            {
                dwError = ERROR_INVALID_STATE;
                BAIL_ON_POSIX_SOCK_ERROR(dwError);
            }

            if (pEvent->events & (EPOLLERR | EPOLLHUP))
            {
                eventType = VM_SOCK_EVENT_TYPE_CONNECTION_CLOSED;

                pSocket = pEventSocket;

                dwError = VmDnsSockPosixEventQueueDelete_inlock(
                                        pQueue,
                                        pSocket);
                BAIL_ON_POSIX_SOCK_ERROR(dwError);
            }
            else if (pEventSocket->type == VM_SOCK_TYPE_LISTENER)
            {
                switch (pEventSocket->protocol)
                {
                    case VM_SOCK_PROTOCOL_TCP:

                        dwError = VmDnsSockPosixAcceptConnection(
                                        pEventSocket,
                                        &pSocket);
                        BAIL_ON_POSIX_SOCK_ERROR(dwError);

                        dwError = VmDnsSockPosixSetNonBlocking(pSocket);
                        BAIL_ON_POSIX_SOCK_ERROR(dwError);

                        dwError = VmDnsSockPosixEventQueueAdd_inlock(
                                        pQueue,
                                        FALSE,
                                        pSocket);
                        BAIL_ON_POSIX_SOCK_ERROR(dwError);

                        pSocket->bInEventQueue = TRUE;

                        eventType = VM_SOCK_EVENT_TYPE_TCP_NEW_CONNECTION;

                        break;

                    case VM_SOCK_PROTOCOL_UDP:
                        pSocket = VmDnsSockPosixAcquireSocket(pEventSocket);

                        eventType = VM_SOCK_EVENT_TYPE_DATA_AVAILABLE;

                        break;

                    default:

                        dwError = ERROR_INVALID_STATE;
                        BAIL_ON_POSIX_SOCK_ERROR(dwError);

                        break;
                }
            }
            else if (pEventSocket->type == VM_SOCK_TYPE_SIGNAL)
            {
                if (pQueue->bShutdown)
                {
                    dwError = ERROR_SHUTDOWN_IN_PROGRESS;
                    BAIL_ON_POSIX_SOCK_ERROR(dwError);
                }
                else
                {
                    pSocket = VmDnsSockPosixAcquireSocket(pEventSocket);
                }

            }
            else
            {
                pSocket = VmDnsSockPosixAcquireSocket(pEventSocket);
            }
        }

        pQueue->iReady++;
    }

    dwError = VmDnsSockPosixSetData(pSocket, NULL, (PVOID *)&pIoBuffer);
    BAIL_ON_POSIX_SOCK_ERROR(dwError);

    if (pIoBuffer && eventType == VM_SOCK_EVENT_TYPE_UNKNOWN)
    {

        pIoContext = CONTAINING_RECORD(pIoBuffer, VM_SOCK_IO_CONTEXT, IoBuffer);
        eventType = pIoContext->eventType;
    }

    *ppSocket = pSocket;
    *ppIoBuffer = pIoBuffer;
    *pEventType = eventType;

cleanup:

    if (bLocked)
    {
        VmDnsUnlockMutex(pQueue->pMutex);
    }



    return dwError;

error:

    if (ppSocket)
    {
        *ppSocket = NULL;
    }
    if (ppIoBuffer)
    {
        *ppIoBuffer = NULL;
    }
    if (pEventType)
    {
        *pEventType = VM_SOCK_EVENT_TYPE_UNKNOWN;
    }
    if (pSocket)
    {
        VmDnsSockPosixReleaseSocket(pSocket);
    }

    goto cleanup;
}

VOID
VmDnsSockPosixShutdownEventQueue(
    PVM_SOCK_EVENT_QUEUE pQueue
    )
{

    LONG result = 0;
    if (pQueue)
    {
        result = InterlockedExchange((LONG*)(&pQueue->bShutdown), TRUE);
        if (pQueue->pSignalWriter)
        {
            char szBuf[] = {0};
            ssize_t nWritten = 0;
            nWritten = write(pQueue->pSignalWriter->fd, szBuf, sizeof(szBuf));
        }
    }

}

VOID
VmDnsSockPosixFreeEventQueue(
    PVM_SOCK_EVENT_QUEUE pQueue
    )
{
    if (pQueue->pSignalReader)
    {
        VmDnsSockPosixReleaseSocket(pQueue->pSignalReader);
    }
    if (pQueue->pSignalWriter)
    {
        VmDnsSockPosixReleaseSocket(pQueue->pSignalWriter);
    }
    if (pQueue->pMutex)
    {
        VmDnsFreeMutex(pQueue->pMutex);
    }
    if (pQueue->epollFd >= 0)
    {
        close(pQueue->epollFd);
    }
    if (pQueue->pEventArray)
    {
        VmDnsFreeMemory(pQueue->pEventArray);
        pQueue->pEventArray = NULL;
    }
    VmDnsFreeMemory(pQueue);
}

DWORD
VmDnsSockPosixSetNonBlocking(
    PVM_SOCKET pSocket
    )
{
    DWORD dwError = 0;
    BOOLEAN bLocked = FALSE;

    dwError = VmDnsLockMutex(pSocket->pMutex);
    BAIL_ON_POSIX_SOCK_ERROR(dwError);

    bLocked = TRUE;

    dwError = VmDnsSockPosixSetDescriptorNonBlocking(pSocket->fd);
    BAIL_ON_POSIX_SOCK_ERROR(dwError);

cleanup:

    if (bLocked)
    {
        VmDnsUnlockMutex(pSocket->pMutex);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsSockPosixSetTimeOut(
    PVM_SOCKET pSocket,
    DWORD      dwTimeOut
    )
{
    DWORD dwError = 0;
    BOOLEAN bLocked = FALSE;

    if (!pSocket)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }

    if (dwTimeOut)
    {
        struct timeval tv = {0};
        dwError = VmDnsLockMutex(pSocket->pMutex);
        BAIL_ON_POSIX_SOCK_ERROR(dwError);

        bLocked = TRUE;

        tv.tv_sec = dwTimeOut;
        if (setsockopt(pSocket->fd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0)
        {
            dwError = LwErrnoToWin32Error(errno);
            BAIL_ON_POSIX_SOCK_ERROR(dwError);
        }

        if (setsockopt(pSocket->fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0)
        {
            dwError = LwErrnoToWin32Error(errno);
            BAIL_ON_POSIX_SOCK_ERROR(dwError);
        }
    }

cleanup:

    if (bLocked)
    {
        VmDnsUnlockMutex(pSocket->pMutex);
    }
    return dwError;

error:

    goto cleanup;
}


DWORD
VmDnsSockPosixGetProtocol(
    PVM_SOCKET           pSocket,
    PDWORD               pdwProtocol
    )
{
    DWORD dwError = 0;
    BOOLEAN bLocked = FALSE;
    DWORD dwProtocol = 0;

    if (!pSocket || !pdwProtocol)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }

    dwError = VmDnsLockMutex(pSocket->pMutex);
    BAIL_ON_POSIX_SOCK_ERROR(dwError);

    bLocked = TRUE;

    switch (pSocket->protocol)
    {
        case VM_SOCK_PROTOCOL_UDP:

            dwProtocol = SOCK_DGRAM;

            break;

        case VM_SOCK_PROTOCOL_TCP:

            dwProtocol = SOCK_STREAM;

            break;

        default:

            dwError = ERROR_INTERNAL_ERROR;
            BAIL_ON_POSIX_SOCK_ERROR(dwError);

            break;
    }

    *pdwProtocol = dwProtocol;

cleanup:

    if (bLocked)
    {
        VmDnsUnlockMutex(pSocket->pMutex);
    }

    return dwError;

error:

    if (pdwProtocol)
    {
        *pdwProtocol = 0;
    }

    goto cleanup;
}

DWORD
VmDnsSockPosixSetData(
    PVM_SOCKET           pSocket,
    PVOID                pData,
    PVOID*               ppOldData
    )
{
    DWORD dwError = 0;
    BOOLEAN bLocked = FALSE;
    PVOID   pOldData = NULL;

    if (!pSocket)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }

    dwError = VmDnsLockMutex(pSocket->pMutex);
    BAIL_ON_POSIX_SOCK_ERROR(dwError);

    bLocked = TRUE;

    pOldData = pSocket->pData;

    pSocket->pData = pData;

    if (ppOldData)
    {
        *ppOldData = pOldData;
    }

cleanup:

    if (bLocked)
    {
        VmDnsUnlockMutex(pSocket->pMutex);
    }

    return dwError;

error:

    if (ppOldData)
    {
        *ppOldData = NULL;
    }

    goto cleanup;
}

DWORD
VmDnsSockPosixGetData(
    PVM_SOCKET          pSocket,
    PVOID*              ppData
    )
{
    DWORD dwError = 0;
    BOOLEAN bLocked = FALSE;

    if (!pSocket || !ppData)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }

    dwError = VmDnsLockMutex(pSocket->pMutex);
    BAIL_ON_POSIX_SOCK_ERROR(dwError);

    bLocked = TRUE;

    *ppData = pSocket->pData;

cleanup:

    if (bLocked)
    {
        VmDnsUnlockMutex(pSocket->pMutex);
    }

    return dwError;

error:

    if (ppData)
    {
        *ppData = NULL;
    }

    goto cleanup;
}

DWORD
VmDnsSockPosixRead(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD   dwError = 0;
    BOOLEAN bLocked = FALSE;
    int     flags   = 0;
    ssize_t nRead   = 0;
    DWORD dwBufSize = 0;
    DWORD dwSockAddrLen = 0;

    if (!pSocket || !pIoBuffer || !pIoBuffer->pData)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }

    if (pIoBuffer->dwExpectedSize < pIoBuffer->dwCurrentSize)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }

    dwBufSize = pIoBuffer->dwExpectedSize - pIoBuffer->dwCurrentSize;
    dwSockAddrLen = sizeof pIoBuffer->clientAddr;

    dwError = VmDnsLockMutex(pSocket->pMutex);
    BAIL_ON_POSIX_SOCK_ERROR(dwError);

    bLocked = TRUE;

    nRead = recvfrom(
                pSocket->fd,
                pIoBuffer->pData + pIoBuffer->dwCurrentSize,
                dwBufSize,
                flags,
                (struct sockaddr*)&pIoBuffer->clientAddr,
                &dwSockAddrLen);
    if (nRead < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }

    pIoBuffer->addrLen = dwSockAddrLen;

    pIoBuffer->dwCurrentSize += nRead;
    pIoBuffer->dwTotalBytesTransferred += nRead;

cleanup:

    if (bLocked)
    {
        VmDnsUnlockMutex(pSocket->pMutex);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsSockPosixWrite(
    PVM_SOCKET          pSocket,
    const struct sockaddr*    pClientAddress,
    socklen_t           addrLength,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD   dwError  = 0;
    BOOLEAN bLocked  = FALSE;
    int     flags    = 0;
    ssize_t nWritten = 0;
    DWORD dwBytesToWrite = 0;
    const struct sockaddr* pClientAddressLocal = NULL;
    socklen_t addrLengthLocal = 0;

    if (!pSocket || !pIoBuffer || !pIoBuffer->pData)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }

    dwBytesToWrite = pIoBuffer->dwExpectedSize - pIoBuffer->dwCurrentSize;

    if (pClientAddress && addrLength)
    {
        pClientAddressLocal = pClientAddress;
        addrLengthLocal = addrLength;
    }
    else
    {
        pClientAddressLocal = &pSocket->addr;
        addrLengthLocal = pSocket->addrLen;
    }

    dwError = VmDnsLockMutex(pSocket->pMutex);
    BAIL_ON_POSIX_SOCK_ERROR(dwError);

    bLocked = TRUE;

    nWritten = sendto(
                    pSocket->fd,
                    pIoBuffer->pData,
                    dwBytesToWrite,
                    flags,
                    pClientAddressLocal,
                    addrLengthLocal);
    if (nWritten < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }

    pIoBuffer->dwCurrentSize += nWritten;
    pIoBuffer->dwTotalBytesTransferred += nWritten;

cleanup:

    if (bLocked)
    {
        VmDnsUnlockMutex(pSocket->pMutex);
    }

    return dwError;

error:

    goto cleanup;
}

PVM_SOCKET
VmDnsSockPosixAcquireSocket(
    PVM_SOCKET           pSocket
    )
{
    if (pSocket)
    {
        InterlockedIncrement(&pSocket->refCount);
    }

    return pSocket;
}

VOID
VmDnsSockPosixReleaseSocket(
    PVM_SOCKET           pSocket
    )
{
    if (pSocket)
    {
        if (InterlockedDecrement(&pSocket->refCount) == 0)
        {
            VmDnsSockPosixFreeSocket(pSocket);
        }
    }
}

DWORD
VmDnsSockPosixCloseSocket(
    PVM_SOCKET pSocket
    )
{
    DWORD dwError = 0;
    BOOLEAN bLocked = FALSE;

    dwError = VmDnsLockMutex(pSocket->pMutex);
    BAIL_ON_POSIX_SOCK_ERROR(dwError);

    bLocked = TRUE;

    if (pSocket->fd >= 0)
    {
        close(pSocket->fd);
        pSocket->fd = -1;
    }

cleanup:

    if (bLocked)
    {
        VmDnsUnlockMutex(pSocket->pMutex);
    }

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDnsSockPosixCreateSignalSockets(
    PVM_SOCKET* ppReaderSocket,
    PVM_SOCKET* ppWriterSocket
    )
{
    DWORD dwError = 0;
    PVM_SOCKET pReaderSocket = NULL;
    PVM_SOCKET pWriterSocket = NULL;
    PVM_SOCKET* sockets[] = { &pReaderSocket, &pWriterSocket };
    int   fdPair[] = { -1, -1 };
    DWORD iSock = 0;

    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, fdPair) < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }

    for (; iSock < sizeof(sockets)/sizeof(sockets[0]); iSock++)
    {
        PVM_SOCKET pSocket = NULL;

        dwError = VmDnsAllocateMemory(sizeof(VM_SOCKET), (PVOID*)sockets[iSock]);
        BAIL_ON_POSIX_SOCK_ERROR(dwError);

        pSocket = *sockets[iSock];

        pSocket->refCount = 1;

        dwError = VmDnsAllocateMutex(&pSocket->pMutex);
        BAIL_ON_POSIX_SOCK_ERROR(dwError);

        pSocket->protocol = VM_SOCK_PROTOCOL_TCP;
        pSocket->type = VM_SOCK_TYPE_SIGNAL;
        pSocket->fd = fdPair[iSock];

        fdPair[iSock] = -1;
    }

    *ppReaderSocket = pReaderSocket;
    *ppWriterSocket = pWriterSocket;

cleanup:

    return dwError;

error:

    *ppReaderSocket = NULL;
    *ppWriterSocket = NULL;

    if (pReaderSocket)
    {
        VmDnsSockPosixFreeSocket(pReaderSocket);
    }
    if (pWriterSocket)
    {
        VmDnsSockPosixFreeSocket(pWriterSocket);
    }
    for (iSock = 0; iSock < sizeof(fdPair)/sizeof(fdPair[0]); iSock++)
    {
        if (fdPair[iSock] >= 0)
        {
            close(fdPair[iSock]);
        }
    }

    goto cleanup;
}

static
DWORD
VmDnsSockPosixEventQueueAdd_inlock(
    PVM_SOCK_EVENT_QUEUE pQueue,
    BOOL                 bOneShot,
    PVM_SOCKET           pSocket
    )
{
    DWORD dwError = 0;
    struct epoll_event event = {0};

    event.data.ptr = pSocket;
    event.events = EPOLLIN;
    if (bOneShot)
    {
       event.events = event.events | EPOLLONESHOT;
    }


    VmDnsSockPosixAcquireSocket(pSocket);
    if (pSocket->bInEventQueue == FALSE &&
        epoll_ctl(pQueue->epollFd, EPOLL_CTL_ADD, pSocket->fd, &event) < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }
    pSocket->bInEventQueue = TRUE;


error:

    return dwError;
}

static
DWORD
VmDnsSockPosixEventQueueDelete_inlock(
    PVM_SOCK_EVENT_QUEUE pQueue,
    PVM_SOCKET           pSocket
    )
{
    DWORD dwError = 0;
    struct epoll_event event = {0};


    if (pSocket->bInEventQueue == TRUE &&
        epoll_ctl(pQueue->epollFd, EPOLL_CTL_DEL, pSocket->fd, &event) < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }
    pSocket->bInEventQueue = FALSE;

    VmDnsSockPosixReleaseSocket(pSocket);

error:
    return dwError;
}

static
DWORD
VmDnsSockPosixAcceptConnection(
    PVM_SOCKET  pListener,
    PVM_SOCKET* ppSocket
    )
{
    DWORD dwError = 0;
    PVM_SOCKET pSocket = NULL;
    int fd = -1;

    dwError = VmDnsAllocateMemory(sizeof(*pSocket), (PVOID*)&pSocket);
    BAIL_ON_POSIX_SOCK_ERROR(dwError);

    pSocket->refCount = 1;

    dwError = VmDnsAllocateMutex(&pSocket->pMutex);
    BAIL_ON_POSIX_SOCK_ERROR(dwError);

    pSocket->protocol = pListener->protocol;
    pSocket->type = VM_SOCK_TYPE_SERVER;

    fd = accept(pListener->fd, &pSocket->addr, &pSocket->addrLen);
    if (fd < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }


    pSocket->fd = fd;

    pSocket->pAddr = &pSocket->addr;

    *ppSocket = pSocket;

cleanup:

    return dwError;

error:

    *ppSocket = NULL;

    if (pSocket)
    {
        VmDnsSockPosixFreeSocket(pSocket);
    }
    if (fd >= 0)
    {
        close(fd);
    }

    goto cleanup;
}

static
DWORD
VmDnsSockPosixSetDescriptorNonBlocking(
    int fd
    )
{
    DWORD dwError = 0;
    int flags = 0;

    if ((flags = fcntl(fd, F_GETFL, 0)) < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }

    flags |= O_NONBLOCK;

    if ((flags = fcntl(fd, F_SETFL, flags)) < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }

error:

    return dwError;
}

static
DWORD
VmDnsSockPosixSetReuseAddress(
    int fd
    )
{
    DWORD dwError = 0;
    int on = 1;

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }

error:

    return dwError;
}

static
VOID
VmDnsSockPosixFreeSocket(
    PVM_SOCKET pSocket
    )
{
    if (pSocket->fd >= 0)
    {
        close(pSocket->fd);
    }
    if (pSocket->pMutex)
    {
        VmDnsFreeMutex(pSocket->pMutex);
    }
    VmDnsFreeMemory(pSocket);
}

DWORD
VmDnsSockPosixStartListening(
    PVM_SOCKET           pSocket,
    int                  iListenQueueSize
    )
{
    return ERROR_NOT_SUPPORTED;
}

DWORD
VmDnsSockPosixAllocateIoBuffer(
    VM_SOCK_EVENT_TYPE          eventType,
    PVM_SOCK_EVENT_CONTEXT      pEventContext,
    DWORD                       dwSize,
    PVM_SOCK_IO_BUFFER*         ppIoBuffer
    )
{
    DWORD dwError = 0;
    PVM_SOCK_IO_CONTEXT pIoContext = NULL;

    if (!ppIoBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(
                    sizeof(VM_SOCK_IO_CONTEXT) + dwSize,
                    (PVOID*)&pIoContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    pIoContext->eventType = eventType;
    pIoContext->pEventContext = pEventContext;
    pIoContext->IoBuffer.dwExpectedSize = dwSize;
    pIoContext->IoBuffer.pData = pIoContext->DataBuffer;

    *ppIoBuffer = &pIoContext->IoBuffer;

cleanup:

    return dwError;

error:

    if (pIoContext)
    {
        VmDnsSockPosixFreeIoBuffer(&pIoContext->IoBuffer);
    }

    goto cleanup;
}

DWORD
VmDnsSockPosixSetEventContext(
    PVM_SOCK_IO_BUFFER      pIoBuffer,
    PVM_SOCK_EVENT_CONTEXT  pEventContext,
    PVM_SOCK_EVENT_CONTEXT* ppOldEventContext
    )
{
    DWORD dwError = 0;
    PVM_SOCK_IO_CONTEXT pIoContext = NULL;
    PVM_SOCK_EVENT_CONTEXT pOldEventContext = NULL;

    if (!pIoBuffer || !ppOldEventContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }

    pIoContext = CONTAINING_RECORD(pIoBuffer, VM_SOCK_IO_CONTEXT, IoBuffer);

    pOldEventContext = (PVM_SOCK_EVENT_CONTEXT)
                        InterlockedExchange(&(pIoContext->pEventContext), pEventContext);

    *ppOldEventContext = pOldEventContext;

cleanup:

    return dwError;
error:

    if (ppOldEventContext)
    {
        *ppOldEventContext = NULL;
    }
    goto cleanup;
}

DWORD
VmDnsSockPosixGetEventContext(
    PVM_SOCK_IO_BUFFER        pIoBuffer,
    PVM_SOCK_EVENT_CONTEXT*   ppEventContext
    )
{
    DWORD dwError = 0;
    PVM_SOCK_EVENT_CONTEXT pEventContext = NULL;
    PVM_SOCK_IO_CONTEXT pIoContext = NULL;

    if (!pIoBuffer || !ppEventContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_POSIX_SOCK_ERROR(dwError);
    }


    pIoContext = CONTAINING_RECORD(pIoBuffer, VM_SOCK_IO_CONTEXT, IoBuffer);

    pEventContext = pIoContext->pEventContext;

    *ppEventContext = pEventContext;

cleanup:

    return dwError;
error:

    if (ppEventContext)
    {
        *ppEventContext = NULL;
    }
    goto cleanup;
}


VOID
VmDnsSockPosixFreeIoBuffer(
    PVM_SOCK_IO_BUFFER     pIoBuffer
    )
{
//    VMDNS_LOG_DEBUG("pIoBuffer:%p released from thread %p", pIoBuffer, pthread_self());
    if (pIoBuffer && pIoBuffer->pClientSocket)
    {
        VmDnsSockPosixReleaseSocket(pIoBuffer->pClientSocket);
    }
    PVM_SOCK_IO_CONTEXT pIoContext = CONTAINING_RECORD(pIoBuffer, VM_SOCK_IO_CONTEXT, IoBuffer);
    VMDNS_SAFE_FREE_MEMORY(pIoContext);
}

/**
 * @brief  VmwGetClientAddreess
 *
 * @param[in] pSocket
 * @param[in] pAddress
 * @param[in] addresLen
 *
 * @return DWORD - 0 on success
 */
DWORD
VmDnsSockPosixGetAddress(
    PVM_SOCKET                  pSocket,
    struct sockaddr_storage*    pAddress,
    socklen_t*                  pAddresLen
    )
{
    DWORD dwError = ERROR_SUCCESS;

    if (!pSocket ||
        !pAddresLen ||
        !pAddress)
    {
        dwError = ERROR_SUCCESS;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    memcpy(pAddress, &pSocket->addr, pSocket->addrLen);

cleanup:

    return dwError;

error :

    goto cleanup;

}
