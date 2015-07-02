/*
 * Copyright (c) VMware Inc.  All rights Reserved.
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
VmDnsOnNewConnection(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    );

static
VOID
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
VmDnsOnTcpDataReceived(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    );

static
DWORD
VmDnsOnUdpDataReceived(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    );

static
DWORD
VmDnsOnTcpSizeReceived(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    );

static
DWORD
VmDnsOnTcpWriteSizeCompleted(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    );

static
DWORD
VmDnsOnTcpWriteDataCompleted(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    );

static
DWORD
VmDnsOnUdpWriteDataCompleted(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    );

static
DWORD
VmDnsDisconnectClient(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    );

static
PVOID
VmDnsSockWorkerThreadProc(
    PVOID               pData
    );

DWORD
VmDnsInitProtocolServer(
    VOID
    )
{
    DWORD dwError = 0;
    PVMDNS_SOCK_CONTEXT pSockContext = NULL;
    DWORD dwFlags = VM_SOCK_CREATE_FLAGS_REUSE_ADDR |
                    VM_SOCK_CREATE_FLAGS_IPV4 |
#ifdef AF_INET6
                    VM_SOCK_CREATE_FLAGS_IPV6 |
#endif
                    VM_SOCK_CREATE_FLAGS_NON_BLOCK;
    DWORD iThr = 0;

    dwError = VmDnsAllocateMemory(
                         sizeof(*pSockContext),
                         (PVOID*)&pSockContext
                         );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsAllocateMutex(&pSockContext->pMutex);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmwSockOpenServer(
                        VMW_DNS_PORT,
                        -1,
                        dwFlags | VM_SOCK_CREATE_FLAGS_UDP,
                        &pSockContext->pListenerUDP);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmwSockOpenServer(
                        VMW_DNS_PORT,
                        VMW_DNS_DEFAULT_THREAD_COUNT,
                        dwFlags | VM_SOCK_CREATE_FLAGS_TCP,
                        &pSockContext->pListenerTCP);
    BAIL_ON_VMDNS_ERROR(dwError);

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

    gpDNSDriverGlobals->pSockContext = pSockContext;

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
    if (gpDNSDriverGlobals->pSockContext)
    {
        VmDnsSockContextFree(gpDNSDriverGlobals->pSockContext);
        gpDNSDriverGlobals->pSockContext = NULL;
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
                VmDnsDisconnectClient(pSocket, pIoBuffer);
                pSocket = NULL;
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

    if (pIoBuffer)
    {
        VmwSockReleaseIoBuffer(pIoBuffer);
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
        switch (sockEvent)
        {
        case VM_SOCK_EVENT_TYPE_NEW_CONNECTION:

            dwError = VmDnsOnNewConnection(pSocket, pIoBuffer);
            BAIL_ON_VMDNS_ERROR(dwError);
            break;

        case VM_SOCK_EVENT_TYPE_DATA_AVAILABLE:

            dwError = VmDnsOnDataAvailable(pSocket, pIoBuffer);
            BAIL_ON_VMDNS_ERROR(dwError);
            break;

        case VM_SOCK_EVENT_TYPE_TCP_SIZE_READ_COMPLETED:

            dwError = VmDnsOnTcpSizeReceived(pSocket, pIoBuffer);
            BAIL_ON_VMDNS_ERROR(dwError);
            break;

        case VM_SOCK_EVENT_TYPE_TCP_DATA_READ_COMPLETED:

            dwError = VmDnsOnTcpDataReceived(pSocket, pIoBuffer);
            BAIL_ON_VMDNS_ERROR(dwError);
            break;

        case VM_SOCK_EVENT_TYPE_UDP_DATA_READ_COMPLETED:

            dwError = VmDnsOnUdpDataReceived(pSocket, pIoBuffer);
            BAIL_ON_VMDNS_ERROR(dwError);
            break;

        case VM_SOCK_EVENT_TYPE_TCP_SIZE_WRITE_COMPLETED:

            dwError = VmDnsOnTcpWriteSizeCompleted(pSocket, pIoBuffer);
            BAIL_ON_VMDNS_ERROR(dwError);
            break;

        case VM_SOCK_EVENT_TYPE_TCP_DATA_WRITE_COMPLETED:

            dwError = VmDnsOnTcpWriteDataCompleted(pSocket, pIoBuffer);
            BAIL_ON_VMDNS_ERROR(dwError);
            break;

        case VM_SOCK_EVENT_TYPE_UDP_DATA_WRITE_COMPLETED:

            dwError = VmDnsOnUdpWriteDataCompleted(pSocket, pIoBuffer);
            BAIL_ON_VMDNS_ERROR(dwError);
            break;

        case VM_SOCK_EVENT_TYPE_CONNECTION_CLOSED:

            VmDnsOnDisconnect(pSocket, pIoBuffer);
            break;

        default:
            dwError = ERROR_INVALID_MESSAGE;
            break;
        }

        if (pIoBuffer)
        {
            VmwSockReleaseIoBuffer(pIoBuffer);
        }
    }

cleanup:
    return dwError;

error :
    goto cleanup;
}

static
DWORD
VmDnsOnNewConnection(
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
VOID
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
        dwError = VmDnsOnTcpDataReceived(pSocket, pIoBuffer);
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
        dwError = VmDnsOnUdpDataReceived(pSocket, pIoBuffer);
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
                        VM_SOCK_EVENT_TYPE_TCP_SIZE_READ_COMPLETED,
                        sizeof(UINT16),
                        &pIoBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmwSockRead(
                        pSocket,
                        pIoBuffer);

    if (dwError == ERROR_SUCCESS)
    {
        dwError = VmDnsOnTcpSizeReceived(pSocket, pIoBuffer);
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
                        VM_SOCK_EVENT_TYPE_UDP_DATA_READ_COMPLETED,
                        VMDNS_UDP_PACKET_SIZE,
                        &pIoBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmwSockRead(
                        pSocket,
                        pIoBuffer);

    if (dwError == ERROR_SUCCESS)
    {
        dwError = VmDnsOnUdpDataReceived(pSocket, pIoBuffer);
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
VmDnsOnTcpSizeReceived(
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

    if (!pIoBuffer->dwBytesTransferred)
    {
        dwError = WSAECONNRESET;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    uSizeToRead = htons(*((UINT16*)(pIoBuffer->pData)));

    dwError = VmwSockAllocateIoBuffer(
                        VM_SOCK_EVENT_TYPE_TCP_DATA_READ_COMPLETED,
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
        dwError = VmDnsOnTcpDataReceived(pSocket, pIoNewBuffer);
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
VmDnsOnTcpDataReceived(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD dwError = 0;
    PVM_SOCK_IO_BUFFER  pIoNewBuffer = NULL;
    PVM_SOCK_IO_BUFFER  pIoSizeBuffer = NULL;
    PBYTE pResponse = NULL;
    DWORD dwDnsResponseSize = 0;

    if (!pIoBuffer)
    {
        dwError = ERROR_INVALID_MESSAGE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    if (!pIoBuffer->dwBytesTransferred)
    {
        dwError = WSAECONNRESET;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (pIoBuffer->dwExpectedSize > pIoBuffer->dwCurrentSize)
    {
        dwError = VmwSockRead(
                            pSocket,
                            pIoBuffer);
        if (dwError == ERROR_SUCCESS)
        {
            dwError = VmDnsOnTcpDataReceived(pSocket, pIoBuffer);
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
                                pIoBuffer->dwCurrentSize,
                                &pResponse,
                                &dwDnsResponseSize
                                );
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmwSockAllocateIoBuffer(
                                VM_SOCK_EVENT_TYPE_TCP_SIZE_WRITE_COMPLETED,
                                sizeof(INT16),
                                &pIoSizeBuffer);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmwSockAllocateIoBuffer(
                                VM_SOCK_EVENT_TYPE_TCP_DATA_WRITE_COMPLETED,
                                dwDnsResponseSize,
                                &pIoNewBuffer);
        BAIL_ON_VMDNS_ERROR(dwError);

        (*(UINT16*)pIoSizeBuffer->pData) = htons(dwDnsResponseSize);

#ifdef WIN32
        memcpy_s(
            pIoNewBuffer->pData,
            pIoNewBuffer->dwExpectedSize,
            pResponse,
            dwDnsResponseSize);
#else
        memcpy(
            pIoNewBuffer->pData,
            pResponse,
            dwDnsResponseSize);
#endif
        dwError = VmwSockWrite(
                            pSocket,
                            (struct sockaddr*)&pIoBuffer->clientAddr,
                            pIoSizeBuffer->addrLen,
                            pIoSizeBuffer);
        if (dwError == ERROR_SUCCESS)
        {
            dwError = VmDnsOnTcpWriteSizeCompleted(pSocket, pIoSizeBuffer);
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
                            pIoNewBuffer);
        if (dwError == ERROR_SUCCESS)
        {
            dwError = VmDnsOnTcpWriteDataCompleted(pSocket, pIoSizeBuffer);
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
VmDnsOnUdpDataReceived(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD dwError = 0;
    PVM_SOCK_IO_BUFFER  pIoNewBuffer = NULL;
    PBYTE pResponse = NULL;
    DWORD dwDnsResponseSize = 0;

    if (!pIoBuffer)
    {
        dwError = ERROR_INVALID_MESSAGE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    if (!pIoBuffer->dwBytesTransferred)
    {
        dwError = WSAECONNRESET;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsProcessRequest(
                            pIoBuffer->pData,
                            pIoBuffer->dwCurrentSize,
                            &pResponse,
                            &dwDnsResponseSize
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmwSockAllocateIoBuffer(
                            VM_SOCK_EVENT_TYPE_UDP_DATA_WRITE_COMPLETED,
                            dwDnsResponseSize,
                            &pIoNewBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

#ifdef WIN32
    memcpy_s(
        pIoNewBuffer->pData,
        pIoNewBuffer->dwExpectedSize,
        pResponse,
        dwDnsResponseSize);
#else
    memcpy(
        pIoNewBuffer->pData,
        pResponse,
        dwDnsResponseSize);
#endif

    dwError = VmwSockWrite(
                        pSocket,
                        (struct sockaddr*)&pIoBuffer->clientAddr,
                        pIoBuffer->addrLen,
                        pIoNewBuffer);
    if (dwError == ERROR_SUCCESS)
    {
        dwError = VmDnsOnUdpWriteDataCompleted(pSocket, pIoNewBuffer);
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
VmDnsOnTcpWriteSizeCompleted(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD dwError = 0;
    if (!pIoBuffer->dwCurrentSize)
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
VmDnsOnTcpWriteDataCompleted(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD dwError = 0;

    if (!pIoBuffer->dwBytesTransferred)
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
VmDnsOnUdpWriteDataCompleted(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD dwError = 0;

    if (!pIoBuffer->dwBytesTransferred)
    {
        dwError = WSAECONNRESET;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

#ifdef WIN32
    dwError = VmDnsUdpReceiveNewData(pSocket);
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
    }
    if (pSockContext->pListenerTCP)
    {
        VmwSockRelease(pSockContext->pListenerTCP);
    }
    if (pSockContext->pListenerUDP)
    {
        VmwSockClose(pSockContext->pListenerUDP);
    }
/*
 *    if (pSockInterface->pForwarders)
 *    {
 *        DWORD iForwarder = 0;
 *        PVMW_DNS_FORWARDER pForwarder = NULL;
 *
 *        while ((pForwarder = &pSockInterface->pForwarders[iForwarder++]) != NULL)
 *        {
 *            VmDnsSrvReleaseForwarder(pForwarder);
 *        }
 *
 *        VmwFreeMemory(pSockInterface->pForwarders);
 *    }
 */
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

    VmwSockClose(pSocket);
    VmwSockRelease(pSocket);

cleanup:

    if (pIoBuffer)
    {
        VmwSockReleaseIoBuffer(pIoBuffer);
    }

    return dwError;

error:

    goto cleanup;
}
