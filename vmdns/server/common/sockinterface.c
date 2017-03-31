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
VOID
VmDnsSockContextFree(
    PVMDNS_SOCK_CONTEXT pSockInterface
    );

static
DWORD
VmDnsHandleSocketEvent(
    PVM_SOCKET          pSocket,
    VM_SOCK_EVENT_TYPE  sockEvent,
    PVM_SOCK_IO_BUFFER  pIoBuffer,
    DWORD               dwError
    );

static
DWORD
VmDnsOnTcpNewConnection(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    );

static
DWORD
VmDnsOnDisconnect(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    );

static
DWORD
VmDnsOnDataAvailable(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    );

static
PVOID
VmDnsSockWorkerThreadProc(
    PVOID               pData
    );

static
DWORD
VmDnsTcpReceiveNewData(
    PVM_SOCKET          pSocket
    );

static
DWORD
VmDnsUdpReceiveNewData(
    PVM_SOCKET          pSocket
    );

static
DWORD
VmDnsReceiveData(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    );

static
DWORD
VmDnsTcpReceiveData(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    );

static
DWORD
VmDnsUdpReceiveData(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    );

static
DWORD
VmDnsOnTcpRequestDataRead(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    );

static
DWORD
VmDnsOnTcpRequestSizeRead(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    );

static
DWORD
VmDnsOnTcpResponseSizeWrite(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    );

static
DWORD
VmDnsOnTcpResponseDataWrite(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    );

static
DWORD
VmDnsOnUdpRequestDataRead(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    );

static
DWORD
VmDnsOnUdpResponseDataWrite(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    );

static
DWORD
VmDnsDisconnectClient(
    PVM_SOCKET          pSocket
    );

static
PVOID
VmDnsSockWorkerThreadProc(
    PVOID               pData
    );

typedef DWORD
(*PVMDNS_SOCK_EVENT_HANDLER)(
    PVM_SOCKET pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    );

static PVMDNS_SOCK_EVENT_HANDLER eventHandlerMap[] =
{
    [VM_SOCK_EVENT_TYPE_UNKNOWN] =
            NULL,
    [VM_SOCK_EVENT_TYPE_DATA_AVAILABLE] =
            &VmDnsOnDataAvailable,
    [VM_SOCK_EVENT_TYPE_TCP_NEW_CONNECTION] =
            &VmDnsOnTcpNewConnection,
    [VM_SOCK_EVENT_TYPE_TCP_REQUEST_SIZE_READ] =
            &VmDnsOnTcpRequestSizeRead,
    [VM_SOCK_EVENT_TYPE_TCP_REQUEST_DATA_READ] =
            &VmDnsOnTcpRequestDataRead,
    [VM_SOCK_EVENT_TYPE_TCP_RESPONSE_SIZE_WRITE] =
            &VmDnsOnTcpResponseSizeWrite,
    [VM_SOCK_EVENT_TYPE_TCP_RESPONSE_DATA_WRITE] =
            &VmDnsOnTcpResponseDataWrite,
    [VM_SOCK_EVENT_TYPE_UDP_REQUEST_DATA_READ] =
            &VmDnsOnUdpRequestDataRead,
    [VM_SOCK_EVENT_TYPE_UDP_RESPONSE_DATA_WRITE] =
            &VmDnsOnUdpResponseDataWrite,
    [VM_SOCK_EVENT_TYPE_CONNECTION_CLOSED] =
            &VmDnsOnDisconnect,
    [VM_SOCK_EVENT_TYPE_MAX] = NULL
};

static PCSTR ppszEventTypeMap[] =
{
    [VM_SOCK_EVENT_TYPE_UNKNOWN] =
            "UNKNOWN",
    [VM_SOCK_EVENT_TYPE_DATA_AVAILABLE] =
            "DATA_AVAILABLE",
    [VM_SOCK_EVENT_TYPE_TCP_NEW_CONNECTION] =
            "TCP_NEW_CONNECTION",
    [VM_SOCK_EVENT_TYPE_TCP_REQUEST_SIZE_READ] =
            "TCP_REQUEST_SIZE_READ",
    [VM_SOCK_EVENT_TYPE_TCP_REQUEST_DATA_READ] =
            "TCP_REQUEST_DATA_READ",
    [VM_SOCK_EVENT_TYPE_TCP_RESPONSE_SIZE_WRITE] =
            "TCP_RESPONSE_SIZE_WRITE",
    [VM_SOCK_EVENT_TYPE_TCP_RESPONSE_DATA_WRITE] =
            "TCP_RESPONSE_DATA_WRITE",
    [VM_SOCK_EVENT_TYPE_TCP_FWD_REQUEST] =
            "TCP_FWD_REQUEST",
    [VM_SOCK_EVENT_TYPE_TCP_FWD_REQUEST_SIZE_WRITE] =
            "TCP_FWD_REQUEST_SIZE_WRITE",
    [VM_SOCK_EVENT_TYPE_TCP_FWD_REQUES_DATA_WRITE] =
            "TCP_FWD_REQUES_DATA_WRITE",
    [VM_SOCK_EVENT_TYPE_TCP_FWD_RESPONSE_SIZE_READ] =
            "TCP_FWD_RESPONSE_SIZE_READ",
    [VM_SOCK_EVENT_TYPE_TCP_FWD_RESPONSE_DATA_READ] =
            "TCP_FWD_RESPONSE_DATA_READ",
    [VM_SOCK_EVENT_TYPE_UDP_REQUEST_DATA_READ] =
            "UDP_REQUEST_DATA_READ",
    [VM_SOCK_EVENT_TYPE_UDP_RESPONSE_DATA_WRITE] =
            "UDP_RESPONSE_DATA_WRITE",
    [VM_SOCK_EVENT_TYPE_UDP_FWD_REQUEST] =
            "UDP_FWD_REQUEST",
    [VM_SOCK_EVENT_TYPE_UDP_FWD_REQUEST_DATA_WRITE] =
            "UDP_FWD_REQUEST_DATA_WRITE",
    [VM_SOCK_EVENT_TYPE_UDP_FWD_RESPONSE_DATA_READ] =
            "UDP_FWD_RESPONSE_DATA_READ",
    [VM_SOCK_EVENT_TYPE_CONNECTION_CLOSED] =
            "CONNECTION_CLOSED",
    [VM_SOCK_EVENT_TYPE_MAX] =
            "MAX",
};

DWORD
VmDnsInitProtocolServer(
    VOID
    )
{
    DWORD dwError = 0;
    PVMDNS_SOCK_CONTEXT pSockContext = NULL;
    DWORD dwFlags = VM_SOCK_CREATE_FLAGS_REUSE_ADDR |
                    VM_SOCK_CREATE_FLAGS_NON_BLOCK;
    DWORD iThr = 0;

    dwError = VmDnsAllocateMemory(
                         sizeof(*pSockContext),
                         (PVOID*)&pSockContext
                         );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateMutex(&pSockContext->pMutex);
    BAIL_ON_VMDNS_ERROR(dwError);

    /* Handle IPv4 case */
    dwError = VmwSockOpenServer(
                        VMW_DNS_PORT,
                        -1,
                        dwFlags | VM_SOCK_CREATE_FLAGS_UDP |
                                  VM_SOCK_CREATE_FLAGS_IPV4,
                        &pSockContext->pListenerUDP);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmwSockOpenServer(
                        VMW_DNS_PORT,
                        VMW_DNS_DEFAULT_THREAD_COUNT,
                        dwFlags | VM_SOCK_CREATE_FLAGS_TCP |
                                  VM_SOCK_CREATE_FLAGS_IPV4,
                        &pSockContext->pListenerTCP);
    BAIL_ON_VMDNS_ERROR(dwError);

#ifdef AF_INET6
    /* Handle IPv6 case */
    dwError = VmwSockOpenServer(
                        VMW_DNS_PORT,
                        -1,
                        dwFlags | VM_SOCK_CREATE_FLAGS_UDP |
                                  VM_SOCK_CREATE_FLAGS_IPV6,
                        &pSockContext->pListenerUDP6);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmwSockOpenServer(
                        VMW_DNS_PORT,
                        VMW_DNS_DEFAULT_THREAD_COUNT,
                        dwFlags | VM_SOCK_CREATE_FLAGS_TCP |
                                  VM_SOCK_CREATE_FLAGS_IPV6,
                        &pSockContext->pListenerTCP6);
    BAIL_ON_VMDNS_ERROR(dwError);
#endif

    dwError = VmwSockCreateEventQueue(-1, &pSockContext->pEventQueue);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmwSockEventQueueAdd(
                        pSockContext->pEventQueue,
                        pSockContext->pListenerTCP);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmwSockEventQueueAdd(
                        pSockContext->pEventQueue,
                        pSockContext->pListenerUDP);
    BAIL_ON_VMDNS_ERROR(dwError);

#ifdef AF_INET6
    dwError = VmwSockEventQueueAdd(
                        pSockContext->pEventQueue,
                        pSockContext->pListenerTCP6);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmwSockEventQueueAdd(
                        pSockContext->pEventQueue,
                        pSockContext->pListenerUDP6);
    BAIL_ON_VMDNS_ERROR(dwError);
#endif

#ifdef WIN32
    dwError = VmwSockStartListening(
                        pSockContext->pListenerTCP,
                        VMW_DNS_DEFAULT_THREAD_COUNT);
    BAIL_ON_VMDNS_ERROR(dwError);
#endif

    dwError = VmDnsAllocateMemory(
                        sizeof(PVMDNS_THREAD) * VMW_DNS_DEFAULT_THREAD_COUNT,
                        (PVOID*)&pSockContext->pWorkerThreads);
    BAIL_ON_VMDNS_ERROR(dwError);

    pSockContext->dwNumThreads = VMW_DNS_DEFAULT_THREAD_COUNT;

    for (; iThr < pSockContext->dwNumThreads; iThr++)
    {
        dwError = VmDnsAllocateMemory(
                            sizeof(VMDNS_THREAD),
                            (PVOID)&pSockContext->pWorkerThreads[iThr]
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsCreateThread(
                            pSockContext->pWorkerThreads[iThr],
                            TRUE,
                            (PVMDNS_START_ROUTINE)&VmDnsSockWorkerThreadProc,
                            pSockContext
                            );
            BAIL_ON_VMDNS_ERROR(dwError);
    }

    gpSrvContext->pSockContext = pSockContext;

cleanup:

    return dwError;

error:

    if (pSockContext)
    {
        VmDnsSockContextFree(pSockContext);
    }

    goto cleanup;
}

VOID
VmDnsShutdownProtocolServer(
    VOID
    )
{
    if (gpSrvContext->pSockContext)
    {
        VmDnsSockContextFree(gpSrvContext->pSockContext);
        gpSrvContext->pSockContext = NULL;
    }
}

static
PVOID
VmDnsSockWorkerThreadProc(
    PVOID pData
    )
{
    DWORD dwError = 0;
    PVMDNS_SOCK_CONTEXT pSockContext = (PVMDNS_SOCK_CONTEXT)pData;
    PVM_SOCKET pSocket = NULL;
    PVM_SOCK_IO_BUFFER pIoBuffer = NULL;

#ifdef WIN32
    VmDnsUdpReceiveData(pSockContext->pListenerUDP, NULL);
#endif

    for(;;)
    {
        VM_SOCK_EVENT_TYPE eventType = VM_SOCK_EVENT_TYPE_UNKNOWN;

        dwError = VmwSockWaitForEvent(
                        pSockContext->pEventQueue,
                        -1,
                        &pSocket,
                        &eventType,
                        &pIoBuffer);

        if (dwError == ERROR_SHUTDOWN_IN_PROGRESS)
        {
            VMDNS_LOG_DEBUG(
                "%s shutdown in progress, exit sock worker loop.",
                __FUNCTION__);
            break;
        }

        if (!pSocket)
        {
            // Data received is via UDP, no client sockets
            pSocket = pSockContext->pListenerUDP;
        }

        dwError = VmDnsHandleSocketEvent(
                        pSocket,
                        eventType,
                        pIoBuffer,
                        dwError);

        if (dwError == ERROR_SUCCESS ||
            dwError == ERROR_IO_PENDING)
        {
            dwError = ERROR_SUCCESS;
        }
        else
        {
            if (pSocket != pSockContext->pListenerUDP)
            {
                VmDnsDisconnectClient(pSocket);
                pSocket = NULL;
            }
            else
            {
#ifdef WIN32
                dwError = VmDnsUdpReceiveNewData(pSocket);
#endif
            }
            pIoBuffer = NULL;
            dwError = 0;
        }
        BAIL_ON_VMDNS_ERROR(dwError);
    }

error:

    if (pSocket)
    {
        VmwSockRelease(pSocket);
    }

    return NULL;
}

static
DWORD
VmDnsHandleSocketEvent(
    PVM_SOCKET          pSocket,
    VM_SOCK_EVENT_TYPE  sockEvent,
    PVM_SOCK_IO_BUFFER  pIoBuffer,
    DWORD               dwError
    )
{
    if (dwError == ERROR_SUCCESS)
    {
        PVMDNS_SOCK_EVENT_HANDLER pEventHandler = eventHandlerMap[sockEvent];
        if (pEventHandler)
        {
            VMDNS_LOG_DEBUG(
                    "New Event - %s, Buffer Size: %d, Buffer Ptr: %p",
                    ppszEventTypeMap[sockEvent],
                    pIoBuffer ? pIoBuffer->dwTotalBytesTransferred : 0,
                    pIoBuffer);

            dwError = pEventHandler(pSocket, pIoBuffer);
            BAIL_ON_VMDNS_ERROR(dwError);
        }
        else
        {
            dwError = ERROR_INVALID_STATE;
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }

cleanup:

    if (dwError != ERROR_IO_PENDING && pIoBuffer)
    {
        VmwSockReleaseIoBuffer(pIoBuffer);
    }

    return dwError;

error :
    goto cleanup;
}

static
DWORD
VmDnsOnTcpNewConnection(
    PVM_SOCKET pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD dwError = 0;

    if (!pSocket)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

#ifdef WIN32
    dwError = VmDnsTcpReceiveNewData(pSocket);
    BAIL_ON_VMDNS_ERROR(dwError);
#endif
cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDnsOnDisconnect(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{

    if (pSocket)
    {
        VmwSockClose(pSocket);
    }

    if (pIoBuffer)
    {
        VmwSockReleaseIoBuffer(pIoBuffer);
    }

    return ERROR_SUCCESS;
}

static
DWORD
VmDnsOnDataAvailable(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD dwError = 0;

    if (!pSocket)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsReceiveData(pSocket, pIoBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:
    if (pIoBuffer)
    {
        VmwSockReleaseIoBuffer(pIoBuffer);
    }

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDnsReceiveData(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD dwError = 0;
    DWORD dwProtocol = 0;

    if (!pSocket)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmwSockGetProtocol(pSocket, &dwProtocol);
    BAIL_ON_VMDNS_ERROR(dwError);

    if (dwProtocol == SOCK_STREAM)
    {
        dwError = VmDnsTcpReceiveData(pSocket, pIoBuffer);
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else if (dwProtocol == SOCK_DGRAM)
    {
        dwError = VmDnsUdpReceiveData(pSocket, pIoBuffer);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:
    if (pIoBuffer)
    {
        VmwSockReleaseIoBuffer(pIoBuffer);
    }

    return dwError;

error:

    goto cleanup;

}

static
DWORD
VmDnsTcpReceiveData(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD dwError = 0;

    if (!pSocket)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (!pIoBuffer)
    {
        dwError = VmDnsTcpReceiveNewData(pSocket);
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        dwError = VmDnsOnTcpRequestDataRead(pSocket, pIoBuffer);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:
    if (pIoBuffer)
    {
        VmwSockReleaseIoBuffer(pIoBuffer);
    }

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDnsUdpReceiveData(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD dwError = 0;
    if (!pSocket)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (!pIoBuffer)
    {
        dwError = VmDnsUdpReceiveNewData(pSocket);
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        dwError = VmDnsOnUdpRequestDataRead(pSocket, pIoBuffer);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:
    if (pIoBuffer)
    {
        VmwSockReleaseIoBuffer(pIoBuffer);
    }

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDnsTcpReceiveNewData(
    PVM_SOCKET          pSocket
    )
{
    DWORD dwError = 0;
    PVM_SOCK_IO_BUFFER  pIoBuffer = NULL;

    dwError = VmwSockAllocateIoBuffer(
                        VM_SOCK_EVENT_TYPE_TCP_REQUEST_SIZE_READ,
                        sizeof(UINT16),
                        &pIoBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmwSockRead(
                        pSocket,
                        pIoBuffer);

    if (dwError == ERROR_SUCCESS)
    {
        dwError = VmDnsOnTcpRequestSizeRead(pSocket, pIoBuffer);
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else if (dwError == ERROR_IO_PENDING)
    {
        // fail for linux?
        pIoBuffer = NULL;
    }
    else
    {
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:

    if (pIoBuffer)
    {
        VmwSockReleaseIoBuffer(pIoBuffer);
    }

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDnsUdpReceiveNewData(
    PVM_SOCKET          pSocket
    )
{
    DWORD dwError = 0;
    PVM_SOCK_IO_BUFFER  pIoBuffer = NULL;

    dwError = VmwSockAllocateIoBuffer(
                        VM_SOCK_EVENT_TYPE_UDP_REQUEST_DATA_READ,
                        VMDNS_UDP_PACKET_SIZE,
                        &pIoBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmwSockRead(
                        pSocket,
                        pIoBuffer);

    if (dwError == ERROR_SUCCESS)
    {
        dwError = VmDnsOnUdpRequestDataRead(pSocket, pIoBuffer);
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else if (dwError == ERROR_IO_PENDING)
    {
        // fail for linux?
        pIoBuffer = NULL;
    }
    else
    {
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:

    if (pIoBuffer)
    {
        VmwSockReleaseIoBuffer(pIoBuffer);
    }

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDnsOnTcpRequestSizeRead(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD dwError = 0;
    UINT16 uSizeToRead = 0;
    PVM_SOCK_IO_BUFFER  pIoNewBuffer = NULL;
    PVOID pOldData = NULL;

    if (!pIoBuffer)
    {
        dwError = ERROR_INVALID_MESSAGE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (!pIoBuffer->dwTotalBytesTransferred)
    {
        dwError = WSAECONNRESET;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    uSizeToRead = htons(*((UINT16*)(pIoBuffer->pData)));

    dwError = VmwSockAllocateIoBuffer(
                        VM_SOCK_EVENT_TYPE_TCP_REQUEST_DATA_READ,
                        uSizeToRead,
                        &pIoNewBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmwSockSetData(pSocket, pIoBuffer, &pOldData);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmwSockRead(
                        pSocket,
                        pIoNewBuffer);

    if (dwError == ERROR_SUCCESS)
    {
        dwError = VmDnsOnTcpRequestDataRead(pSocket, pIoNewBuffer);
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    if (dwError == ERROR_IO_PENDING)
    {
        pIoNewBuffer = NULL;
    }
    else
    {
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:

    if (pIoNewBuffer)
    {
        VmwSockReleaseIoBuffer(pIoNewBuffer);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsOnTcpRequestDataRead(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD dwError = 0;
    PVM_SOCK_IO_BUFFER  pIoNewBuffer = NULL;
    PVM_SOCK_IO_BUFFER  pIoSizeBuffer = NULL;
    PBYTE pResponse = NULL;
    DWORD dwDnsResponseSize = 0;
    UCHAR rCode = 0;

    if (!pIoBuffer)
    {
        dwError = ERROR_INVALID_MESSAGE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    if (!pIoBuffer->dwTotalBytesTransferred)
    {
        dwError = WSAECONNRESET;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (pIoBuffer->dwExpectedSize > pIoBuffer->dwTotalBytesTransferred)
    {
        dwError = VmwSockRead(
                        pSocket,
                        pIoBuffer
                        );
        if (dwError == ERROR_SUCCESS)
        {
            dwError = VmDnsOnTcpRequestDataRead(
                                pSocket,
                                pIoBuffer
                                );
            BAIL_ON_VMDNS_ERROR(dwError);
        }
        else if(dwError == ERROR_IO_PENDING)
        {
            pIoBuffer = NULL;
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }
    else
    {
        dwError = VmDnsProcessRequest(
                            pIoBuffer->pData,
                            pIoBuffer->dwTotalBytesTransferred,
                            &pResponse,
                            &dwDnsResponseSize,
                            &rCode
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmwSockAllocateIoBuffer(
                            VM_SOCK_EVENT_TYPE_TCP_RESPONSE_SIZE_WRITE,
                            sizeof(INT16),
                            &pIoSizeBuffer
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmwSockAllocateIoBuffer(
                            VM_SOCK_EVENT_TYPE_TCP_RESPONSE_DATA_WRITE,
                            dwDnsResponseSize,
                            &pIoNewBuffer
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        (*(UINT16*)pIoSizeBuffer->pData) = htons(dwDnsResponseSize);

        dwError = VmDnsCopyMemory(
                        pIoNewBuffer->pData,
                        pIoNewBuffer->dwExpectedSize,
                        pResponse,
                        dwDnsResponseSize
                        );
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmwSockWrite(
                        pSocket,
                        (struct sockaddr*)&pIoBuffer->clientAddr,
                        pIoSizeBuffer->addrLen,
                        pIoSizeBuffer
                        );
        if (dwError == ERROR_SUCCESS)
        {
            dwError = VmDnsOnTcpResponseSizeWrite(
                                    pSocket,
                                    pIoSizeBuffer
                                    );
            BAIL_ON_VMDNS_ERROR(dwError);
        }
        else if (dwError == ERROR_IO_PENDING)
        {
            pIoSizeBuffer = NULL;
        }

        dwError = VmwSockWrite(
                        pSocket,
                        (struct sockaddr*)&pIoBuffer->clientAddr,
                        pIoNewBuffer->addrLen,
                        pIoNewBuffer
                        );
        if (dwError == ERROR_SUCCESS)
        {
            dwError = VmDnsOnTcpResponseDataWrite(
                                    pSocket,
                                    pIoSizeBuffer
                                    );
            BAIL_ON_VMDNS_ERROR(dwError);
        }
        else if (dwError == ERROR_IO_PENDING)
        {
            pIoNewBuffer = NULL;
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }
cleanup:

    if (pIoSizeBuffer)
    {
        VmwSockReleaseIoBuffer(pIoSizeBuffer);
    }
    if (pIoNewBuffer)
    {
        VmwSockReleaseIoBuffer(pIoNewBuffer);
    }

    VMDNS_SAFE_FREE_MEMORY(pResponse);
    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsOnUdpRequestDataRead(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD dwError = 0;
    PVM_SOCK_IO_BUFFER  pIoNewBuffer = NULL;
    PBYTE pResponse = NULL;
    DWORD dwDnsResponseSize = 0;
    UCHAR rCode = 0;

    if (!pIoBuffer)
    {
        dwError = ERROR_INVALID_MESSAGE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    if (!pIoBuffer->dwTotalBytesTransferred)
    {
        dwError = WSAECONNRESET;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsProcessRequest(
                        pIoBuffer->pData,
                        pIoBuffer->dwTotalBytesTransferred,
                        &pResponse,
                        &dwDnsResponseSize,
                        &rCode
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmwSockAllocateIoBuffer(
                        VM_SOCK_EVENT_TYPE_UDP_RESPONSE_DATA_WRITE,
                        dwDnsResponseSize,
                        &pIoNewBuffer
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyMemory(
                    pIoNewBuffer->pData,
                    pIoNewBuffer->dwExpectedSize,
                    pResponse,
                    dwDnsResponseSize
                    );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmwSockWrite(
                    pSocket,
                    (struct sockaddr*)&pIoBuffer->clientAddr,
                    pIoBuffer->addrLen,
                    pIoNewBuffer
                    );
    if (dwError == ERROR_SUCCESS)
    {
        dwError = VmDnsOnUdpResponseDataWrite(
                                pSocket,
                                pIoNewBuffer
                                );
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else if (dwError == ERROR_IO_PENDING)
    {
        pIoNewBuffer = NULL;
        BAIL_ON_VMDNS_ERROR(dwError);
    }
cleanup:

    if (pIoNewBuffer)
    {
        VmwSockReleaseIoBuffer(pIoNewBuffer);
    }

    VMDNS_SAFE_FREE_MEMORY(pResponse);
    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDnsOnTcpResponseSizeWrite(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD dwError = 0;
    if (!pIoBuffer->dwTotalBytesTransferred)
    {
        dwError = WSAECONNRESET;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
VmDnsOnTcpResponseDataWrite(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD dwError = 0;

    if (!pIoBuffer->dwTotalBytesTransferred)
    {
        dwError = WSAECONNRESET;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
VmDnsOnUdpResponseDataWrite(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD dwError = 0;

    if (!pIoBuffer->dwTotalBytesTransferred)
    {
        dwError = WSAECONNRESET;
        BAIL_ON_VMDNS_ERROR(dwError);
    }
#ifdef WIN32
    dwError = VmDnsUdpReceiveNewData(pSocket);
    BAIL_ON_VMDNS_ERROR(dwError);
#endif
cleanup:
    return dwError;

error:
    goto cleanup;
}

static
VOID
VmDnsSockContextFree(
    PVMDNS_SOCK_CONTEXT pSockContext
    )
{
    if (pSockContext->pEventQueue)
    {
        VmwSockCloseEventQueue(pSockContext->pEventQueue);
        pSockContext->pEventQueue = NULL;
    }
    if (pSockContext->pListenerTCP)
    {
        VmwSockRelease(pSockContext->pListenerTCP);
    }
    if (pSockContext->pListenerUDP)
    {
        VmwSockClose(pSockContext->pListenerUDP);
    }
    if (pSockContext->pWorkerThreads)
    {
        DWORD iThr = 0;

        for (; iThr < pSockContext->dwNumThreads; iThr++)
        {
            PVMDNS_THREAD pThread = pSockContext->pWorkerThreads[iThr];

            if (pThread)
            {
                VmDnsFreeThread(pThread);
            }
        }

        VmDnsFreeMemory(pSockContext->pWorkerThreads);
    }
    if (pSockContext->pMutex)
    {
        VmDnsFreeMutex(pSockContext->pMutex);
    }
    VmDnsFreeMemory(pSockContext);
}

static
DWORD
VmDnsDisconnectClient(
    PVM_SOCKET          pSocket
    )
{
    DWORD dwError = 0;

    if (!pSocket)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    VmwSockClose(pSocket);
    VmwSockRelease(pSocket);

cleanup:

    return dwError;

error:

    goto cleanup;
}
