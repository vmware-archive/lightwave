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
VmDnsOnForwarderResponse(
    BOOL                bUseUDP,
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    );

static
DWORD
VmDnsOnUdpForwardResponse(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    );

static
DWORD
VmDnsOnTcpForwardResponse(
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

static
DWORD
VmDnsOnForwarderRequest(
    BOOL                bUseUDP,
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
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
    [VM_SOCK_EVENT_TYPE_UDP_FWD_RESPONSE_DATA_READ] =
            &VmDnsOnUdpForwardResponse,
    [VM_SOCK_EVENT_TYPE_TCP_FWD_RESPONSE_DATA_READ] =
            &VmDnsOnTcpForwardResponse,
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
    dwError = VmDnsSockOpenServer(
                        VMW_DNS_PORT,
                        -1,
                        dwFlags | VM_SOCK_CREATE_FLAGS_UDP |
                                  VM_SOCK_CREATE_FLAGS_IPV4,
                        &pSockContext->pListenerUDP);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSockOpenServer(
                        VMW_DNS_PORT,
                        VMW_DNS_DEFAULT_THREAD_COUNT,
                        dwFlags | VM_SOCK_CREATE_FLAGS_TCP |
                                  VM_SOCK_CREATE_FLAGS_IPV4,
                        &pSockContext->pListenerTCP);
    BAIL_ON_VMDNS_ERROR(dwError);

#ifdef AF_INET6
    /* Handle IPv6 case */
    dwError = VmDnsSockOpenServer(
                        VMW_DNS_PORT,
                        -1,
                        dwFlags | VM_SOCK_CREATE_FLAGS_UDP |
                                  VM_SOCK_CREATE_FLAGS_IPV6,
                        &pSockContext->pListenerUDP6);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSockOpenServer(
                        VMW_DNS_PORT,
                        VMW_DNS_DEFAULT_THREAD_COUNT,
                        dwFlags | VM_SOCK_CREATE_FLAGS_TCP |
                                  VM_SOCK_CREATE_FLAGS_IPV6,
                        &pSockContext->pListenerTCP6);
    BAIL_ON_VMDNS_ERROR(dwError);
#endif

    dwError = VmDnsSockCreateEventQueue(-1, &pSockContext->pEventQueue);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSockEventQueueAdd(
                        pSockContext->pEventQueue,
                        FALSE,
                        pSockContext->pListenerTCP);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSockEventQueueAdd(
                        pSockContext->pEventQueue,
                        FALSE,
                        pSockContext->pListenerUDP);
    BAIL_ON_VMDNS_ERROR(dwError);

#ifdef AF_INET6
    dwError = VmDnsSockEventQueueAdd(
                        pSockContext->pEventQueue,
                        FALSE,
                        pSockContext->pListenerTCP6);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSockEventQueueAdd(
                        pSockContext->pEventQueue,
                        FALSE,
                        pSockContext->pListenerUDP6);
    BAIL_ON_VMDNS_ERROR(dwError);
#endif

#ifdef WIN32
    dwError = VmDnsSockStartListening(
                        pSockContext->pListenerTCP,
                        VMW_DNS_DEFAULT_THREAD_COUNT);
    BAIL_ON_VMDNS_ERROR(dwError);
#endif

    dwError = VmDnsAllocateMemory(
                        sizeof(PVMDNS_THREAD) * VMW_DNS_DEFAULT_THREAD_COUNT,
                        (PVOID*)&pSockContext->pWorkerThreads);
    BAIL_ON_VMDNS_ERROR(dwError);

    pSockContext->dwNumThreads = 0;

    for (; iThr < VMW_DNS_DEFAULT_THREAD_COUNT; iThr++)
    {
        dwError = VmDnsAllocateMemory(
                            sizeof(VMDNS_THREAD),
                            (PVOID)&pSockContext->pWorkerThreads[iThr]
                            );
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsCreateThread(
                            pSockContext->pWorkerThreads[iThr],
                            FALSE,
                            (PVMDNS_START_ROUTINE)&VmDnsSockWorkerThreadProc,
                            pSockContext
                            );
        BAIL_ON_VMDNS_ERROR(dwError);
        pSockContext->dwNumThreads++;
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

        dwError = VmDnsSockWaitForEvent(
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

        VmMetricsGaugeIncrement(gVmDnsGaugeMetrics[DNS_ACTIVE_SERVICE_THREADS]);

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

        VmMetricsGaugeDecrement(gVmDnsGaugeMetrics[DNS_ACTIVE_SERVICE_THREADS]);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

error:

    if (pSocket)
    {
        VmDnsSockRelease(pSocket);
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
                    "New Event - %s, Buffer Size: %d, Buffer Ptr: %p, Thread Ptr: %p",
                    ppszEventTypeMap[sockEvent],
                    pIoBuffer ? pIoBuffer->dwTotalBytesTransferred : 0,
                    pIoBuffer,
                    pthread_self());

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
	    VMDNS_LOG_IO_RELEASE(pIoBuffer);
        VmDnsSockReleaseIoBuffer(pIoBuffer);
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
    VmMetricsGaugeIncrement(gVmDnsGaugeMetrics[DNS_OUTSTANDING_REQUEST_COUNT]);

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
    VmMetricsGaugeDecrement(gVmDnsGaugeMetrics[DNS_OUTSTANDING_REQUEST_COUNT]);
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
        VmDnsSockClose(pSocket);
    }

    if (pIoBuffer)
    {
	    VMDNS_LOG_IO_RELEASE(pIoBuffer);
        VmDnsSockReleaseIoBuffer(pIoBuffer);
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
	    VMDNS_LOG_IO_RELEASE(pIoBuffer);
        VmDnsSockReleaseIoBuffer(pIoBuffer);
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

    dwError = VmDnsSockGetProtocol(pSocket, &dwProtocol);
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
	    VMDNS_LOG_IO_RELEASE(pIoBuffer);
        VmDnsSockReleaseIoBuffer(pIoBuffer);
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
	    VMDNS_LOG_IO_RELEASE(pIoBuffer);
        VmDnsSockReleaseIoBuffer(pIoBuffer);
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
	    VMDNS_LOG_IO_RELEASE(pIoBuffer);
        VmDnsSockReleaseIoBuffer(pIoBuffer);
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

    dwError = VmDnsSockAllocateIoBuffer(
                        VM_SOCK_EVENT_TYPE_TCP_REQUEST_SIZE_READ,
                        NULL,
                        sizeof(UINT16),
                        &pIoBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSockRead(
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
	    VMDNS_LOG_IO_RELEASE(pIoBuffer);
        VmDnsSockReleaseIoBuffer(pIoBuffer);
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
    VmMetricsGaugeIncrement(gVmDnsGaugeMetrics[DNS_OUTSTANDING_REQUEST_COUNT]);

    dwError = VmDnsSockAllocateIoBuffer(
                        VM_SOCK_EVENT_TYPE_UDP_REQUEST_DATA_READ,
                        NULL,
                        VMDNS_UDP_PACKET_SIZE,
                        &pIoBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);


    dwError = VmDnsSockRead(
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
	    VMDNS_LOG_IO_RELEASE(pIoBuffer);
        VmDnsSockReleaseIoBuffer(pIoBuffer);
    }

    VmMetricsGaugeDecrement(gVmDnsGaugeMetrics[DNS_OUTSTANDING_REQUEST_COUNT]);
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

    dwError = VmDnsSockAllocateIoBuffer(
                        VM_SOCK_EVENT_TYPE_TCP_REQUEST_DATA_READ,
                        NULL,
                        uSizeToRead,
                        &pIoNewBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSockSetData(pSocket, pIoBuffer, &pOldData);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSockRead(
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
	    VMDNS_LOG_IO_RELEASE(pIoBuffer);
        VmDnsSockReleaseIoBuffer(pIoNewBuffer);
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
        dwError = VmDnsSockRead(
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

        if (!rCode)
        {
            dwError = VmDnsSockAllocateIoBuffer(
                                VM_SOCK_EVENT_TYPE_TCP_RESPONSE_SIZE_WRITE,
                                NULL,
                                sizeof(INT16),
                                &pIoSizeBuffer
                                );
            BAIL_ON_VMDNS_ERROR(dwError);

            dwError = VmDnsSockAllocateIoBuffer(
                                VM_SOCK_EVENT_TYPE_TCP_RESPONSE_DATA_WRITE,
                                NULL,
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

            dwError = VmDnsSockWrite(
                            pSocket,
                            (struct sockaddr*)&pIoBuffer->clientAddr,
                            pIoBuffer->addrLen,
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

            dwError = VmDnsSockWrite(
                            pSocket,
                            NULL,
                            0,
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
        else
        {
            dwError = VmDnsOnForwarderRequest(FALSE, pSocket, pIoBuffer);
            BAIL_ON_VMDNS_ERROR(dwError);
        }
    }
cleanup:

    if (pIoSizeBuffer)
    {
	    VMDNS_LOG_IO_RELEASE(pIoSizeBuffer);
        VmDnsSockReleaseIoBuffer(pIoSizeBuffer);
    }
    if (pIoNewBuffer)
    {
	    VMDNS_LOG_IO_RELEASE(pIoNewBuffer);
        VmDnsSockReleaseIoBuffer(pIoNewBuffer);
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

    if (!rCode)
    {

        dwError = VmDnsSockAllocateIoBuffer(
                            VM_SOCK_EVENT_TYPE_UDP_RESPONSE_DATA_WRITE,
                            NULL,
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


        dwError = VmDnsSockWrite(
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
    }
    else
    {
       dwError = VmDnsOnForwarderRequest(
                                    TRUE,
                                    pSocket,
                                    pIoBuffer
                                    );
       BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:

    if (pIoNewBuffer)
    {
	    VMDNS_LOG_IO_RELEASE(pIoBuffer);
        VmDnsSockReleaseIoBuffer(pIoNewBuffer);
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
        if (pSockContext->pListenerTCP)
        {
            VmDnsSockEventQueueRemove(pSockContext->pEventQueue,
                    pSockContext->pListenerTCP);
            VmDnsSockRelease(pSockContext->pListenerTCP);
        }
        if (pSockContext->pListenerUDP)
        {
            VmDnsSockEventQueueRemove(pSockContext->pEventQueue,
                    pSockContext->pListenerUDP);
            VmDnsSockRelease(pSockContext->pListenerUDP);
        }
#ifdef AF_INET6
        if (pSockContext->pListenerTCP6)
        {
            VmDnsSockEventQueueRemove(pSockContext->pEventQueue,
                        pSockContext->pListenerTCP6);
            VmDnsSockRelease(pSockContext->pListenerTCP6);
        }
        if (pSockContext->pListenerUDP6)
        {
            VmDnsSockEventQueueRemove(pSockContext->pEventQueue,
                        pSockContext->pListenerUDP6);
            VmDnsSockRelease(pSockContext->pListenerUDP6);
        }
#endif
        VmDnsSockShutdownEventQueue(pSockContext->pEventQueue);
    }

    if (pSockContext->pWorkerThreads)
    {
        DWORD iThr = 0;

        for (; iThr < pSockContext->dwNumThreads; iThr++)
        {
            PVMDNS_THREAD pThread = pSockContext->pWorkerThreads[iThr];

            if (pThread)
            {
                VmDnsThreadJoin(pThread, NULL);
                VmDnsFreeThread(pThread);
            }
        }

        if (pSockContext->pEventQueue)
        {
            VmDnsSockFreeEventQueue(pSockContext->pEventQueue);
            pSockContext->pEventQueue = NULL;
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

    VmDnsSockClose(pSocket);
    VmDnsSockRelease(pSocket);

cleanup:

    return dwError;

error:
    
    goto cleanup;
}

static
DWORD
VmDnsOnForwarderRequest(
    BOOL                bUseUDP,
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD dwError = 0;
    PVMDNS_FORWARDER_PACKET_CONTEXT pForwarderContext = NULL;

    if (!pSocket || !pIoBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsAllocateForwarderPacketContext(
                                &pForwarderContext
                                );
    BAIL_ON_VMDNS_ERROR(dwError);

    pIoBuffer->pClientSocket = VmDnsSockAcquire(pSocket);

    dwError = VmDnsForwardRequest(
                          pForwarderContext,
                          TRUE,
                          pIoBuffer
                          );
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    if (pForwarderContext)
    {
        VmDnsReleaseForwarderPacketContext(pForwarderContext);
    }
    return dwError;
error:

    goto cleanup;
}

static
DWORD
VmDnsOnForwarderResponse(
    BOOL                bUseUDP,
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD dwError = 0;
    DWORD dwForwardRequestError = 0;
    PVMDNS_FORWARDER_PACKET_CONTEXT pForwarderContext = NULL;
    PVM_SOCK_EVENT_CONTEXT pSockEventContext = NULL;
    PVM_SOCKET pClientSocket = NULL;
    PVM_SOCK_IO_BUFFER pIoNewBuffer = NULL;
    PVM_SOCK_IO_BUFFER pIoSizeBuffer = NULL;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    DWORD dwResponseCode = 0;

    if (!pSocket || !pIoBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }


    dwError = VmDnsSockSetEventContext(pIoBuffer,NULL,&pSockEventContext);
    BAIL_ON_VMDNS_ERROR(dwError);
    pClientSocket = VmDnsSockAcquire(pIoBuffer->pClientSocket);

    if (!pClientSocket)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pForwarderContext = (PVMDNS_FORWARDER_PACKET_CONTEXT)pSockEventContext;

    if (!pForwarderContext)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsForwardResponse(
                              TRUE,
                              pSocket,
                              &pResponse,
                              &dwResponseSize,
                              &dwResponseCode
                              );

    if (dwResponseCode || dwError)
    {
        dwForwardRequestError = VmDnsForwardRequest(
                                      pForwarderContext,
                                      bUseUDP,
                                      pIoBuffer
                                      );
    }
    if (!dwResponseCode || dwForwardRequestError)
    {
        if (bUseUDP)
        {
            dwError = VmDnsSockAllocateIoBuffer(
                                VM_SOCK_EVENT_TYPE_UDP_RESPONSE_DATA_WRITE,
                                NULL,
                                dwResponseSize,
                                &pIoNewBuffer
                                );
            BAIL_ON_VMDNS_ERROR(dwError);

            dwError = VmDnsCopyMemory(
                            pIoNewBuffer->pData,
                            pIoNewBuffer->dwExpectedSize,
                            pResponse,
                            dwResponseSize
                            );
            BAIL_ON_VMDNS_ERROR(dwError);

            dwError = VmDnsSockWrite(
                            pClientSocket,
                            (struct sockaddr*)&pIoBuffer->clientAddr,
                            pIoBuffer->addrLen,
                            pIoNewBuffer
                            );
            if (dwError == ERROR_SUCCESS)
            {
                dwError = VmDnsOnUdpResponseDataWrite(
                                        pClientSocket,
                                        pIoNewBuffer
                                        );
                BAIL_ON_VMDNS_ERROR(dwError);
            }
        }
        else
        {
            dwError = VmDnsSockAllocateIoBuffer(
                                VM_SOCK_EVENT_TYPE_TCP_RESPONSE_SIZE_WRITE,
                                NULL,
                                sizeof(INT16),
                                &pIoSizeBuffer
                                );
            BAIL_ON_VMDNS_ERROR(dwError);

            dwError = VmDnsSockAllocateIoBuffer(
                                VM_SOCK_EVENT_TYPE_TCP_RESPONSE_DATA_WRITE,
                                NULL,
                                dwResponseSize,
                                &pIoNewBuffer
                                );
            BAIL_ON_VMDNS_ERROR(dwError);

            (*(UINT16*)pIoSizeBuffer->pData) = htons(dwResponseSize);

            dwError = VmDnsCopyMemory(
                            pIoNewBuffer->pData,
                            pIoNewBuffer->dwExpectedSize,
                            pResponse,
                            dwResponseSize
                            );
            BAIL_ON_VMDNS_ERROR(dwError);

            dwError = VmDnsSockWrite(
                            pClientSocket,
                            NULL,
                            0,
                            pIoSizeBuffer
                            );
            if (dwError == ERROR_SUCCESS)
            {
                dwError = VmDnsOnTcpResponseSizeWrite(
                                        pClientSocket,
                                        pIoSizeBuffer
                                        );
                BAIL_ON_VMDNS_ERROR(dwError);
            }
            else if (dwError == ERROR_IO_PENDING)
            {
                pIoSizeBuffer = NULL;
            }

            dwError = VmDnsSockWrite(
                            pClientSocket,
                            NULL,
                            0,
                            pIoNewBuffer
                            );
            if (dwError == ERROR_SUCCESS)
            {
                dwError = VmDnsOnTcpResponseDataWrite(
                                        pClientSocket,
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
    }

cleanup:

    if (pForwarderContext)
    {
        VmDnsReleaseForwarderPacketContext(pForwarderContext);
    }
    if (pClientSocket)
    {
        VmDnsSockRelease(pClientSocket);
    }
    if (pIoNewBuffer)
    {
	VMDNS_LOG_IO_RELEASE(pIoNewBuffer);
        VmDnsSockReleaseIoBuffer(pIoNewBuffer);
    }
    if (pIoSizeBuffer)
    {
	VMDNS_LOG_IO_RELEASE(pIoSizeBuffer);
        VmDnsSockReleaseIoBuffer(pIoSizeBuffer);
    }

    return dwError;
error:

    goto cleanup;
}

static
DWORD
VmDnsOnUdpForwardResponse(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD dwError = 0;
    dwError = VmDnsOnForwarderResponse(TRUE, pSocket, pIoBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}

static
DWORD
VmDnsOnTcpForwardResponse(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{

    //TODO: Change to use TCP
    DWORD dwError = 0;
    dwError = VmDnsOnForwarderResponse(FALSE, pSocket, pIoBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;
error:

    goto cleanup;
}


