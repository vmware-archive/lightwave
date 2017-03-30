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


static DWORD
VmSockWinAcceptConnection(
    PVM_SOCKET              pListenSocket,
    SOCKET                  clientSocket,
    struct sockaddr*        pClientAddress,
    int                     addrLen
    );

static DWORD
VmSockWinCopyTargetAddress(
    struct addrinfo*        pInfo,
    PVM_SOCKET              pSocket
    );

static VOID
VmSockWinFreeSocket(
    PVM_SOCKET              pSocket
    );

static DWORD WINAPI
VmSockWinListenerThreadProc(
    LPVOID                  pThreadParam
    );

/**
 * @brief Opens a client socket
 *
 * @param[in]  pszHost  Target host name or IP Address.
 *                      An empty string will imply the local host.
 * @param[in]  usPort   16 bit port number
 * @param[in]  dwFlags  32 bit flags specifying socket creation preferences
 * @param[out] ppSocket Pointer to created socket context
 *
 * @return 0 on success
 */
DWORD
VmSockWinOpenClient(
    PCSTR                   pszHost,
    USHORT                  usPort,
    VM_SOCK_CREATE_FLAGS    dwFlags,
    PVM_SOCKET*             ppSocket
    )
{
    DWORD                   dwError = 0;
    DWORD                   dwSockFlags = 0;
    int                     nAddressFamily = AF_INET;
    int                     nConnectionType = SOCK_STREAM;
    struct addrinfo         hints = { 0 };
    struct addrinfo*        pAddrInfo = NULL;
    struct addrinfo*        pInfo = NULL;
    struct addrinfo*        pClientAddress = NULL;
    CHAR                    szPort[32] = { 0 };
    SOCKET                  socket = INVALID_SOCKET;
    PVM_SOCKET              pSocket = NULL;

    if (!pszHost || !usPort || !ppSocket)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

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
        nConnectionType = hints.ai_socktype = SOCK_DGRAM;
    }
    else
    {
        nConnectionType = hints.ai_socktype = SOCK_STREAM;
    }

    hints.ai_flags = AI_CANONNAME | AI_NUMERICSERV;

    sprintf_s(szPort, sizeof(szPort), "%d", usPort);

    if (getaddrinfo(pszHost, szPort, &hints, &pAddrInfo) != 0)
    {
        dwError = WSAGetLastError();
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    for (pInfo = pAddrInfo;
        (socket == INVALID_SOCKET && pInfo != NULL);
        pInfo = pInfo->ai_next)
    {
        socket = WSASocketW(
                        pInfo->ai_family,
                        pInfo->ai_socktype,
                        pInfo->ai_protocol,
                        NULL,
                        0,
                        dwSockFlags);

        if (socket == INVALID_SOCKET)
        {
            continue;
        }

        if (nConnectionType == SOCK_STREAM)
        {
            if (connect(socket, pInfo->ai_addr, pInfo->ai_addrlen) < 0)
            {
                dwError = WSAGetLastError();
                continue;
            }
        }

        pClientAddress = pInfo;
    }

    if (socket == INVALID_SOCKET)
    {
        dwError = ERROR_CONNECTION_UNAVAIL;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(sizeof(*pSocket), (PVOID*)&pSocket);
    BAIL_ON_VMDNS_ERROR(dwError);

    pSocket->refCount = 1;
    pSocket->type = VM_SOCK_TYPE_CLIENT;

    if (nConnectionType == SOCK_STREAM)
    {
        pSocket->protocol = VM_SOCK_PROTOCOL_TCP;
    }
    else
    {
        pSocket->protocol = VM_SOCK_PROTOCOL_UDP;
    }

    dwError = VmSockWinCopyTargetAddress(pClientAddress, pSocket);
    BAIL_ON_VMDNS_ERROR(dwError);

    pSocket->hSocket = socket;
    socket = INVALID_SOCKET;

    *ppSocket = pSocket;
    pSocket = NULL;

cleanup:

    if (pAddrInfo)
    {
        freeaddrinfo(pAddrInfo);
    }

    return dwError;

error :
    if (socket != INVALID_SOCKET)
    {
        closesocket(socket);
    }

    VMDNS_SAFE_FREE_MEMORY(pSocket);

    goto cleanup;
}

/**
 * @brief Opens a server socket
 *
 * @param[in] usPort 16 bit local port number that the server listens on
 * @param[in,optional] iListenQueueSize
 *       size of connection acceptance queue.
 *       This value can be (-1) to use the default value.
 *
 * @param[in]  dwFlags 32 bit flags defining socket creation preferences
 * @param[out] ppSocket Pointer to created socket
 *
 * @return 0 on success
 */
DWORD
VmSockWinOpenServer(
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
    SOCKET socket = INVALID_SOCKET;
    PVM_SOCKET pSocket = NULL;
    DWORD dwSockFlags = 0;

    if (dwFlags & VM_SOCK_CREATE_FLAGS_IPV6)
    {
#ifdef AF_INET6
        socketParams.domain = AF_INET6;
#else
        dwError = ERROR_NOT_SUPPORTED;
        BAIL_ON_VMDNS_ERROR(dwError);
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

    if (dwFlags & VM_SOCK_CREATE_FLAGS_NON_BLOCK)
    {
        dwSockFlags = WSA_FLAG_OVERLAPPED;
    }

    socket = WSASocketW(
                    socketParams.domain,
                    socketParams.type,
                    socketParams.protocol,
                    NULL,
                    0,
                    dwSockFlags);
    if (socket == INVALID_SOCKET)
    {
        dwError = WSAGetLastError();
        BAIL_ON_VMDNS_ERROR(dwError);
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
#else
        dwError = ERROR_NOT_SUPPORTED;
        BAIL_ON_VMDNS_ERROR(dwError);
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

    if (bind(socket, pSockAddr, addrLen) < 0)
    {
        dwError = WSAGetLastError();
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (!(dwFlags & VM_SOCK_CREATE_FLAGS_UDP))
    {
        if (iListenQueueSize <= 0)
        {
            iListenQueueSize = VM_SOCK_WINDOWS_DEFAULT_LISTEN_QUEUE_SIZE;
        }

        if (listen(socket, iListenQueueSize) < 0)
        {
            dwError = WSAGetLastError();
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }

    dwError = VmDnsAllocateMemory(sizeof(*pSocket), (PVOID*)&pSocket);
    BAIL_ON_VMDNS_ERROR(dwError);

    pSocket->refCount = 1;
    pSocket->type = VM_SOCK_TYPE_LISTENER;

    if (dwFlags & VM_SOCK_CREATE_FLAGS_UDP)
    {
        pSocket->protocol = VM_SOCK_PROTOCOL_UDP;
    }
    else
    {
        pSocket->protocol = VM_SOCK_PROTOCOL_TCP;
    }

    pSocket->hSocket = socket;
    socket = INVALID_SOCKET;

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
        VmSockWinFreeSocket(pSocket);
    }
    if (socket != INVALID_SOCKET)
    {
        closesocket(socket);
    }

    goto cleanup;
}

/**
 * @brief Creates a Event queue to be used for detecting events on sockets
 *
 * @param[in,optional] iEventQueueSize
 *       specifies the event queue size.
 *       This value can be (-1) to use the default value
 * @param[out] ppQueue Pointer to accept created event queue
 *
 * @return 0 on success
 */
DWORD
VmSockWinCreateEventQueue(
    int                     iEventQueueSize,
    PVM_SOCK_EVENT_QUEUE*   ppQueue
    )
{
    DWORD dwError = 0;
    int sockError = 0;
    PVM_SOCK_EVENT_QUEUE pQueue = NULL;

    if (!ppQueue)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (iEventQueueSize <= 0)
    {
        iEventQueueSize = VM_SOCK_WINDOWS_DEFAULT_QUEUE_SIZE;
    }

    dwError = VmDnsAllocateMemory(sizeof(*pQueue), (PVOID*)&pQueue);
    BAIL_ON_VMDNS_ERROR(dwError);

    pQueue->hIOCP = CreateIoCompletionPort(
                                INVALID_HANDLE_VALUE,
                                NULL,
                                0,
                                0);
    if (!pQueue->hIOCP)
    {
        dwError = GetLastError();
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pQueue->hEventListen = WSACreateEvent();
    if (pQueue->hEventListen == WSA_INVALID_EVENT)
    {
        dwError = WSAGetLastError();
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    *ppQueue = pQueue;

cleanup:

    return dwError;

error:

    if (ppQueue)
    {
        *ppQueue = NULL;
    }

    VmSockWinCloseEventQueue(pQueue);
    goto cleanup;
}

DWORD
VmSockWinEventQueueAdd(
    PVM_SOCK_EVENT_QUEUE pQueue,
    PVM_SOCKET           pSocket
    )
{
    DWORD   dwError = 0;
    BOOLEAN bLocked = TRUE;
    int sockError = 0;
    HANDLE hTemp = NULL;

    if (!pQueue || !pSocket || pSocket->hSocket == INVALID_SOCKET)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    hTemp = CreateIoCompletionPort(
                        (HANDLE)pSocket->hSocket,
                        pQueue->hIOCP,
                        0,
                        0);
    if (hTemp != pQueue->hIOCP)
    {
        dwError = GetLastError();
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    sockError = WSAEventSelect(
                        pSocket->hSocket,
                        pQueue->hEventListen,
                        FD_ACCEPT);
    BAIL_ON_VMDNS_ERROR(dwError);

    pSocket->pEventQueue = pQueue;

cleanup:

    return dwError;

error:

    goto cleanup;
}

/**
 * @brief Starts socket listener
 *
 * @param[in] pSocket Pointer to Socket
 * @param[in,optional] iListenQueueSize
 *
 * @return 0 on success
 */
DWORD
VmSockWinStartListening(
    PVM_SOCKET           pSocket,
    int                  iListenQueueSize
    )
{
    DWORD dwError = 0;
    DWORD dwThreadId = 0;
    int sockError = 0;
    HANDLE hThreadListen = NULL;

    if (!pSocket ||
        !pSocket->pEventQueue ||
        pSocket->hThreadListen)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    hThreadListen = CreateThread(
                            NULL,
                            0,
                            VmSockWinListenerThreadProc,
                            pSocket,
                            0,
                            &dwThreadId);
    if (!hThreadListen)
    {
        dwError = GetLastError();
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pSocket->hThreadListen = hThreadListen;
    hThreadListen = NULL;

cleanup:

    return dwError;

error:

    if (hThreadListen)
    {
        CloseHandle(hThreadListen);
    }

    goto cleanup;
}

/**
 * @brief Waits for an event on the event queue
 *
 * @param[in] pQueue   Pointer to event queue
 * @param[in,optional] iTimeoutMS
 *       Timeout in milliseconds.
 *       Waits forever if (-1) is passed in.
 * @param[out]    ppSocket   Pointer to socket that has an event
 * @param[in,out] pEventType Event type detected on socket
 *
 * @return 0 on success
 */
DWORD
VmSockWinWaitForEvent(
    PVM_SOCK_EVENT_QUEUE pQueue,
    int                  iTimeoutMS,
    PVM_SOCKET*          ppSocket,
    PVM_SOCK_EVENT_TYPE  pEventType,
    PVM_SOCK_IO_BUFFER*  ppIoBuffer
    )
{
    DWORD dwError = 0;
    BOOL bRetVal = TRUE;
    DWORD dwIoSize = 0;
    PVM_SOCKET pSocket = NULL;
    PVM_SOCK_IO_CONTEXT pIoContext = NULL;

    if (!pQueue ||
        !pQueue->hIOCP ||
        !ppIoBuffer ||
        !pEventType)
    {
        dwError = ERROR_INTERNAL_ERROR;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    bRetVal = GetQueuedCompletionStatus(
                        pQueue->hIOCP,
                        &dwIoSize,
                        (PULONG_PTR)&pSocket,
                        (LPOVERLAPPED*)&pIoContext,
                        iTimeoutMS);
    if (!bRetVal)
    {
        dwError = GetLastError();
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    VMDNS_LOG_DEBUG("IO Completed Socket:%d, Address:%p Event: %d, Size: %d",
                pSocket ? pSocket->hSocket : 0,
                &pIoContext->IoBuffer,
                pIoContext->eventType,
                dwIoSize);

    if (dwIoSize)
    {
        pIoContext->IoBuffer.dwTotalBytesTransferred += dwIoSize;
    }
    else
    {
        pIoContext->IoBuffer.dwTotalBytesTransferred = 0;
    }

    *ppIoBuffer = &pIoContext->IoBuffer;
    *ppSocket = pSocket;
    *pEventType = pIoContext->eventType;

cleanup:

    return dwError;

error:

    if (ppIoBuffer)
    {
        *ppIoBuffer = NULL;
    }

    if (ppSocket)
    {
        *ppSocket = NULL;
    }

    goto cleanup;
}

/**
 * @brief Closes and frees event queue
 *
 * @param[in] pQueue Pointer to event queue
 *
 * @return 0 on success
 */

VOID
VmSockWinCloseEventQueue(
    PVM_SOCK_EVENT_QUEUE pQueue
    )
{
    if (pQueue)
    {
        if (pQueue->hEventListen != WSA_INVALID_EVENT)
        {
            WSACloseEvent(pQueue->hEventListen);
        }

        if (pQueue->hIOCP)
        {
            CloseHandle(pQueue->hIOCP);
        }

        VMDNS_SAFE_FREE_MEMORY(pQueue);
    }
}

/**
 * @brief sets socket to be non-blocking
 *
 * @param[in] pSocket Pointer to socket
 *
 * @return 0 on success
 */

DWORD
VmSockWinSetNonBlocking(
    PVM_SOCKET           pSocket
    )
{
    return 0;
}


/**
 * @brief sets socket to be non-blocking
 *
 * @param[in] pSocket Pointer to socket
 * @param[in] TimeOut Timeout in seconds
 *
 * @return 0 on success
 */

DWORD
VmSockWinSetTimeOut(
    PVM_SOCKET           pSocket,
    DWORD                dwTimeOut
    )
{
    return ERROR_NOT_SUPPORTED;
}


/**
 * @brief Retrieves the protocol the socket has been configured with
 *
 * @param[in]     pSocket     Pointer to socket
 * @param[in,out] pdwProtocol Protocol the socket has been configured with
 *                            This will be one of { SOCK_STREAM, SOCK_DGRAM... }
 */
DWORD
VmSockWinGetProtocol(
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
        BAIL_ON_VMDNS_ERROR(dwError);
    }

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
        BAIL_ON_VMDNS_ERROR(dwError);

        break;
    }

    *pdwProtocol = dwProtocol;

cleanup:

    return dwError;

error:

    if (pdwProtocol)
    {
        *pdwProtocol = 0;
    }

    goto cleanup;
}

/**
 * @brief Sets data associated with the socket
 *
 * @param[in] pSocket Pointer to socket
 * @param[in] pData   Pointer to data associated with the socket
 * @param[in,out,optional] ppOldData Pointer to receive old data
 *
 * @return 0 on success
 */
DWORD
VmSockWinSetData(
    PVM_SOCKET           pSocket,
    PVOID                pData,
    PVOID*               ppOldData
    )
{
    return 0;
}

/**
 * @brief Gets data currently associated with the socket.
 *
 * @param[in]     pSocket Pointer to socket
 * @param[in,out] ppData  Pointer to receive data
 *
 * @return 0 on success
 */
DWORD
VmSockWinGetData(
    PVM_SOCKET          pSocket,
    PVOID*              ppData
    )
{
    return 0;
}

/**
 * @brief Reads data from the socket
 *
 * @param[in]     pSocket      Pointer to socket
 * @param[in]     pBuffer      Buffer to read the data into
 * @param[in]     dwBufSize    Maximum size of the passed in buffer
 * @param[in,out] pdwBytesRead Number of bytes read in to the buffer
 * @param[in,out,optional] pClientAddress Client address to fill in optionally
 * @param[in,out,optional] pAddrLength    Length of the client address
 *
 * @return 0 on success
 */
DWORD
VmSockWinRead(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD   dwError = 0;
    int sockError = 0;
    PVM_SOCK_IO_CONTEXT pIoContext = NULL;
    DWORD dwBytesRead = 0, dwBytesToRead = 0;
    DWORD dwFlags = 0;
    WSABUF wsaBuff = { 0 };
    LPOVERLAPPED pOverlapped = NULL;

    if (!pSocket || !pIoBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pIoContext = CONTAINING_RECORD(pIoBuffer, VM_SOCK_IO_CONTEXT, IoBuffer);
    if (pIoBuffer->dwExpectedSize <= pIoBuffer->dwCurrentSize)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    wsaBuff.buf = pIoBuffer->pData + pIoBuffer->dwCurrentSize;
    wsaBuff.len = pIoBuffer->dwExpectedSize - pIoBuffer->dwCurrentSize;
    pOverlapped = (pSocket->pEventQueue) ? &pIoContext->Overlapped : NULL;

    if (pSocket->protocol == VM_SOCK_PROTOCOL_UDP)
    {
        VMDNS_LOG_DEBUG(
                "WSA WSARecvFrom - Socket: %d Address: %p, Event: %d, Size: %d",
                pSocket->hSocket,
                (DWORD)pIoBuffer,
                pIoContext->eventType,
                pIoBuffer->dwExpectedSize);

        pIoContext->IoBuffer.addrLen = sizeof pIoContext->IoBuffer.clientAddr;

        sockError = WSARecvFrom(
                            pSocket->hSocket,
                            &wsaBuff,
                            1,
                            &dwBytesRead,
                            &dwFlags,
                            (struct sockaddr*)&pIoContext->IoBuffer.clientAddr,
                            &pIoContext->IoBuffer.addrLen,
                            pOverlapped,
                            NULL);

        VMDNS_LOG_DEBUG(
                "WSA WSARecvFrom - Status: %d Byes Transfered : %d ",
                sockError,
                dwBytesRead);

        if (sockError == SOCKET_ERROR)
        {
            dwError = WSAGetLastError();
            BAIL_ON_VMDNS_ERROR(dwError);
        }
        else if (pSocket->pEventQueue)
        {
            dwError = ERROR_IO_PENDING;
        }
    }
    else if (pSocket->protocol == VM_SOCK_PROTOCOL_TCP)
    {

        VMDNS_LOG_DEBUG(
                "WSA WSARecv - Socket: %d Address: %p, Event: %d, Size: %d",
                pSocket->hSocket,
                (DWORD)pIoBuffer,
                pIoContext->eventType,
                pIoBuffer->dwExpectedSize);

        sockError = WSARecv(
                        pSocket->hSocket,
                        &wsaBuff,
                        1,
                        &dwBytesRead,
                        &dwFlags,
                        pOverlapped,
                        NULL);

        VMDNS_LOG_DEBUG(
                "WSA WSARecv - Status: %d Byes Transfered : %d ",
                sockError,
                dwBytesRead);

        if (sockError == SOCKET_ERROR)
        {
            dwError = WSAGetLastError();
            BAIL_ON_VMDNS_ERROR(dwError);
        }
        else if (pSocket->pEventQueue)
        {
            dwError = ERROR_IO_PENDING;
        }
    }

    pIoContext->IoBuffer.dwCurrentSize += dwBytesRead;

cleanup:

    return dwError;

error:

    goto cleanup;
}
/**
 * @brief Writes data to the socket
 *
 * @param[in]     pSocket      Pointer to socket
 * @param[in]     pBuffer      Buffer from which bytes have to be written
 * @param[in]     dwBufLen     Number of bytes to write from the buffer
 * @param[in,out] pdwBytesWrtten Number of bytes written to the socket
 * In case of UDP sockets, it is mandatory to provide the client address and
 * length.
 *
 * @return 0 on success
 */
DWORD
VmSockWinWrite(
    PVM_SOCKET          pSocket,
    struct sockaddr*    pClientAddress,
    socklen_t           addrLength,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD   dwError = 0;
    int sockError = 0;
    PVM_SOCK_IO_CONTEXT pIoContext = NULL;
    DWORD dwBytesWritten = 0;
    DWORD dwFlags = 0;
    WSABUF wsaBuff = { 0 };
    LPOVERLAPPED pOverlapped = NULL;

    if (!pSocket || !pIoBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pIoContext = CONTAINING_RECORD(pIoBuffer, VM_SOCK_IO_CONTEXT, IoBuffer);
    if (pIoBuffer->dwExpectedSize <= pIoBuffer->dwCurrentSize)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    wsaBuff.buf = pIoBuffer->pData + pIoBuffer->dwCurrentSize;
    wsaBuff.len = pIoBuffer->dwExpectedSize - pIoBuffer->dwCurrentSize;
    pOverlapped = (pSocket->pEventQueue) ? &pIoContext->Overlapped : NULL;

    if (pSocket->protocol == VM_SOCK_PROTOCOL_UDP)
    {
        if (!pClientAddress || addrLength <= 0)
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDNS_ERROR(dwError);
        }

        memcpy_s(
            &pIoBuffer->clientAddr,
            sizeof pIoBuffer->clientAddr,
            pClientAddress, addrLength);

        VMDNS_LOG_DEBUG("WSA SendTo - Address: %p, Event: %d, Size: %d",
            (DWORD)pIoBuffer, pIoContext->eventType, pIoBuffer->dwExpectedSize);

        sockError = WSASendTo(
                            pSocket->hSocket,
                            &wsaBuff,
                            1,
                            &dwBytesWritten,
                            dwFlags,
                            (struct sockaddr*)&pIoBuffer->clientAddr,
                            addrLength,
                            pOverlapped,
                            NULL);

        VMDNS_LOG_DEBUG(
                "WSA WSASendTo - Status: %d Byes Transfered : %d ",
                sockError,
                dwBytesWritten);
    }
    else if (pSocket->protocol == VM_SOCK_PROTOCOL_TCP)
    {
        VMDNS_LOG_DEBUG("WSA WSASend - Address: %p, Event: %d, Size: %d",
            (DWORD)pIoBuffer, pIoContext->eventType, pIoBuffer->dwExpectedSize);

        sockError = WSASend(
                        pSocket->hSocket,
                        &wsaBuff,
                        1,
                        &dwBytesWritten,
                        dwFlags,
                        pOverlapped,
                        NULL);

        VMDNS_LOG_DEBUG(
                "WSA WSASendTo - Status: %d Byes Transfered : %d ",
                sockError,
                dwBytesWritten);
    }

    if (sockError == SOCKET_ERROR)
    {
        dwError = WSAGetLastError();
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else if (pSocket->pEventQueue)
    {
        dwError = ERROR_IO_PENDING;
    }

    pIoContext->IoBuffer.dwCurrentSize += dwBytesWritten;

    pIoBuffer = NULL;

cleanup:

    return dwError;

error:

    goto cleanup;
}
/**
 * @brief  Acquires a reference on the socket
 *
 * @return Pointer to acquired socket
 */

PVM_SOCKET
VmSockWinAcquire(
    PVM_SOCKET           pSocket
    )
{
    if (pSocket)
    {
        InterlockedIncrement(&pSocket->refCount);
    }

    return pSocket;
}

/**
 * @brief Releases current reference to socket
 *
 */
VOID
VmSockWinRelease(
    PVM_SOCKET           pSocket
    )
{
    if (pSocket)
    {
        if (InterlockedDecrement(&pSocket->refCount) == 0)
        {
            VmSockWinFreeSocket(pSocket);
        }
    }
}

/**
 * @brief Closes the socket
 *        This call does not release the reference to the socket or free it.
 */
DWORD
VmSockWinClose(
    PVM_SOCKET           pSocket
    )
{
    VMDNS_LOG_DEBUG("Close Connectiom - Socket: %d", (DWORD)pSocket->hSocket);

    if (pSocket->hSocket != INVALID_SOCKET)
    {
        closesocket(pSocket->hSocket);
        pSocket->hSocket = INVALID_SOCKET;
    }

    return 0;
}

/**
 * @brief Checks if the string forms a valid IPV4 or IPV6 Address
 *
 * @return TRUE(1) if the string is a valid IP Address, 0 otherwise.
 */
BOOLEAN
VmSockWinIsValidIPAddress(
    PCSTR                pszAddress
    )
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}

static DWORD
VmSockWinCopyTargetAddress(
    struct addrinfo*    pInfo,
    PVM_SOCKET          pSocket
    )
{
    memcpy_s(&pSocket->addr, pInfo->ai_addrlen, pInfo->ai_addr, pInfo->ai_addrlen);
    pSocket->addrLen = pInfo->ai_addrlen;
    return 0;
}

static VOID
VmSockWinFreeSocket(
    PVM_SOCKET  pSocket
    )
{
    if (pSocket->hSocket != INVALID_SOCKET)
    {
        CancelIo((HANDLE)pSocket->hSocket);
        closesocket(pSocket->hSocket);
        pSocket->hSocket = INVALID_SOCKET;
    }

    VMDNS_SAFE_FREE_MEMORY(pSocket);
}

static VOID
VmSockWinDisconnectSocket(
    SOCKET clientSocket
    )
{
    LINGER sockopt = { 0 };

    sockopt.l_onoff = 1;
    sockopt.l_linger = 0;

    setsockopt(clientSocket, SOL_SOCKET, SO_LINGER, (char*)&sockopt, sizeof(sockopt));

    CancelIo((HANDLE)clientSocket);
    closesocket(clientSocket);
}

DWORD WINAPI
VmSockWinListenerThreadProc(
    LPVOID pThreadParam
    )
{
    DWORD dwError = 0;
    PVM_SOCKET pListenSocket = (PVM_SOCKET)pThreadParam;
    PVM_SOCK_EVENT_QUEUE pQueue = NULL;
    WSANETWORKEVENTS events = { 0 };
    int socketError = 0;
    SOCKET clientSocket = INVALID_SOCKET;
    int nAddrLen = -1;

    if (!pListenSocket || !pListenSocket->pEventQueue)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pQueue = pListenSocket->pEventQueue;

    while (!pQueue->bShutdown)
    {
        dwError = WSAWaitForMultipleEvents(
                                    1,
                                    &pQueue->hEventListen,
                                    FALSE,
                                    100,
                                    FALSE);
        if (dwError == WSA_WAIT_TIMEOUT)
        {
            dwError = 0;
        }

        BAIL_ON_VMDNS_ERROR(dwError);

        if (pQueue->bShutdown)
        {
            goto cleanup;
        }

        socketError = WSAEnumNetworkEvents(
                                    pListenSocket->hSocket,
                                    pQueue->hEventListen,
                                    &events);
        if (socketError == SOCKET_ERROR)
        {
            BAIL_ON_VMDNS_ERROR(WSAGetLastError());
        }

        if (events.lNetworkEvents & FD_ACCEPT)
        {
            if (events.iErrorCode[FD_ACCEPT_BIT] == 0 &&
                !pQueue->bShutdown)
            {
                struct sockaddr_storage clientAddress = { 0 };
                int addLen = sizeof clientAddress;
                clientSocket = accept(
                                    pListenSocket->hSocket,
                                    (struct sockaddr*)&clientAddress,
                                    &addLen);

                if (clientSocket == SOCKET_ERROR)
                {
                    BAIL_ON_VMDNS_ERROR(WSAGetLastError());
                }
                else
                {
                    dwError = VmSockWinAcceptConnection(
                                    pListenSocket,
                                    clientSocket,
                                    (struct sockaddr*)&clientAddress,
                                    addLen);
                    BAIL_ON_VMDNS_ERROR(WSAGetLastError());
                }
            }
            else
            {
                BAIL_ON_VMDNS_ERROR(WSAGetLastError());
            }
        }
    }
cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
VmSockWinAcceptConnection(
    PVM_SOCKET              pListenSocket,
    SOCKET                  clientSocket,
    struct sockaddr*        pClientAddr,
    int                     addrlen
    )
{
    DWORD dwError = 0;
    HANDLE hTemp = NULL;
    const char chOpt = 1;
    PVM_SOCKET pClientSocket = NULL;
    PVM_SOCK_IO_BUFFER pIoBuffer = NULL;
    PVM_SOCK_IO_CONTEXT pIoContext = NULL;

    if (!pListenSocket ||
        !pListenSocket->hSocket ||
        !pListenSocket->pEventQueue ||
        !pListenSocket->pEventQueue->hIOCP
        || clientSocket == INVALID_SOCKET)
    {
        dwError = ERROR_INVALID_SERVER_STATE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(
                    sizeof(*pClientSocket),
                    (PVOID*)&pClientSocket);
    BAIL_ON_VMDNS_ERROR(dwError);

    pClientSocket->hSocket = clientSocket;
    pClientSocket->pEventQueue = pListenSocket->pEventQueue;
    pClientSocket->protocol = pListenSocket->protocol;
    pClientSocket->type = VM_SOCK_TYPE_SERVER;
    memcpy_s(&pClientSocket->addr, sizeof pClientSocket->addr, pClientAddr, addrlen);
    pClientSocket->addrLen = addrlen;
    pClientSocket->refCount = 1;

    clientSocket = INVALID_SOCKET;

    if (setsockopt(
                pClientSocket->hSocket,
                IPPROTO_TCP,
                TCP_NODELAY,
                &chOpt,
                sizeof(char)))
    {
        dwError = WSAGetLastError();
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    hTemp = CreateIoCompletionPort(
                    (HANDLE)pClientSocket->hSocket,
                    pListenSocket->pEventQueue->hIOCP,
                    (ULONG_PTR)pClientSocket,
                    0
                    );
    if (hTemp != pListenSocket->pEventQueue->hIOCP)
    {
        dwError = GetLastError();
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmSockWinAllocateIoBuffer(
                    VM_SOCK_EVENT_TYPE_TCP_NEW_CONNECTION,
                    0,
                    &pIoBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

    pIoContext = CONTAINING_RECORD(pIoBuffer, VM_SOCK_IO_CONTEXT, IoBuffer);

    VMDNS_LOG_DEBUG("New Connectiom - Socket: %d Address: %p, Event: %d, Size: %d",
                    pClientSocket->hSocket,
                    (DWORD)pIoBuffer,
                    pIoContext->eventType,
                    pIoBuffer->dwCurrentSize);

    if (!PostQueuedCompletionStatus(
                    pListenSocket->pEventQueue->hIOCP,
                    0,
                    (ULONG_PTR)pClientSocket,
                    (LPOVERLAPPED)pIoContext))
    {
        dwError = GetLastError();
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pClientSocket = NULL;
    pIoBuffer = NULL;

cleanup:

    return dwError;

error:

    if (pClientSocket)
    {
        VmSockWinFreeSocket(pClientSocket);
    }

    if (pIoBuffer)
    {
        VmSockWinFreeIoBuffer(pIoBuffer);
    }

    goto cleanup;
}

DWORD
VmSockWinAllocateIoBuffer(
    VM_SOCK_EVENT_TYPE      eventType,
    DWORD                   dwSize,
    PVM_SOCK_IO_BUFFER*     ppIoBuffer
    )
{
    DWORD dwError = 0;
    PVM_SOCK_IO_CONTEXT pIoContext = NULL;

    if (!ppIoBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateMemory(sizeof(VM_SOCK_IO_CONTEXT) + dwSize, (PVOID*)&pIoContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    pIoContext->eventType = eventType;
    pIoContext->IoBuffer.dwExpectedSize = dwSize;
    pIoContext->IoBuffer.pData = pIoContext->DataBuffer;

    VMDNS_LOG_INFO("Buffer Allocated - Address: %p, Event: %d, Size: %d", 
        (DWORD)&pIoContext->IoBuffer, 
        pIoContext->eventType, 
        pIoContext->IoBuffer.dwExpectedSize);

    *ppIoBuffer = &(pIoContext->IoBuffer);

cleanup:

    return dwError;

error:

    if (ppIoBuffer)
    {
        *ppIoBuffer = NULL;
    }

    if (pIoContext)
    {
        VmSockWinFreeIoBuffer(&pIoContext->IoBuffer);
    }

    goto cleanup;
}

VOID
VmSockWinFreeIoBuffer(
    PVM_SOCK_IO_BUFFER     pIoBuffer
    )
{
    PVM_SOCK_IO_CONTEXT pIoContext = CONTAINING_RECORD(pIoBuffer, VM_SOCK_IO_CONTEXT, IoBuffer);
    VMDNS_LOG_INFO("Freeing Io Buffer - Address: %p, Event: %d, Size: %d", (DWORD)pIoBuffer, pIoContext->eventType, pIoBuffer->dwCurrentSize);
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
VmSockWinGetAddress(
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

    memcpy_s(pAddress, *pAddresLen, &pSocket->addr, pSocket->addrLen);
    *pAddresLen = pSocket->addrLen;

cleanup:

    return dwError;

error :

    goto cleanup;

}
