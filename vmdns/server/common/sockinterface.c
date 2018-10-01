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
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET           pSocket,
    VM_SOCK_EVENT_TYPE   sockEvent,
    PVM_SOCK_IO_BUFFER   pIoBuffer,
    DWORD                dwError
    );

static
DWORD
VmDnsOnTcpNewConnection(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET           pSocket,
    PVM_SOCK_IO_BUFFER   pIoBuffer
    );

static
DWORD
VmDnsOnDisconnect(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET           pSocket,
    PVM_SOCK_IO_BUFFER   pIoBuffer
    );

static
DWORD
VmDnsOnDataAvailable(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET           pSocket,
    PVM_SOCK_IO_BUFFER   pIoBuffer
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
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET           pSocket
    );

static
DWORD
VmDnsReceiveData(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET           pSocket,
    PVM_SOCK_IO_BUFFER   pIoBuffer
    );

static
DWORD
VmDnsTcpReceiveData(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET           pSocket,
    PVM_SOCK_IO_BUFFER   pIoBuffer
    );

static
DWORD
VmDnsUdpReceiveData(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET           pSocket,
    PVM_SOCK_IO_BUFFER   pIoBuffer
    );

static
DWORD
VmDnsOnTcpRequestDataRead(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET           pSocket,
    PVM_SOCK_IO_BUFFER   pIoBuffer
    );

static
DWORD
VmDnsOnTcpRequestSizeRead(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET           pSocket,
    PVM_SOCK_IO_BUFFER   pIoBuffer
    );

static
DWORD
VmDnsOnTcpResponseSizeWrite(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET           pSocket,
    PVM_SOCK_IO_BUFFER   pIoBuffer
    );

static
DWORD
VmDnsOnTcpResponseDataWrite(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET           pSocket,
    PVM_SOCK_IO_BUFFER   pIoBuffer
    );

static
DWORD
VmDnsOnUdpRequestDataRead(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET           pSocket,
    PVM_SOCK_IO_BUFFER   pIoBuffer
    );

static
DWORD
VmDnsOnUdpResponseDataWrite(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET           pSocket,
    PVM_SOCK_IO_BUFFER   pIoBuffer
    );

static
DWORD
VmDnsOnForwarderResponse(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    BOOL                 bUseUDP,
    PVM_SOCKET           pSocket,
    PVM_SOCK_IO_BUFFER   pIoBuffer
    );

static
DWORD
VmDnsOnUdpForwardResponse(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET           pSocket,
    PVM_SOCK_IO_BUFFER   pIoBuffer
    );

static
DWORD
VmDnsOnTcpForwardResponseSizeRead(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET           pSocket,
    PVM_SOCK_IO_BUFFER   pIoBuffer
    );

static
DWORD
VmDnsOnTcpForwardResponse(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET           pSocket,
    PVM_SOCK_IO_BUFFER   pIoBuffer
    );

static
DWORD
VmDnsOnTimerExpiration(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET           pSocket,
    PVM_SOCK_IO_BUFFER   pIoBuffer
    );

static
PVOID
VmDnsSockWorkerThreadProc(
    PVOID               pData
    );

static
DWORD
VmDnsAllocateQueryBuffer(
    PVM_SOCK_IO_BUFFER   pSrcIoBuffer,
    PVM_SOCK_IO_BUFFER*  ppIoBuffer
    );

static
DWORD
VmDnsOnForwarderRequest(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    BOOL                 bUseUDP,
    PVM_SOCKET           pSocket,
    PVM_SOCK_IO_BUFFER   pIoBuffer
    );

static
VOID
VmDnsMetricsRcodeUpdate(
    BOOL                    bQueryInZone,
    METRICS_VDNS_RCODE_OPS  operation,
    UCHAR                   rCode
    );

typedef DWORD
(*PVMDNS_SOCK_EVENT_HANDLER)(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
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
    [VM_SOCK_EVENT_TYPE_TCP_FWD_RESPONSE_SIZE_READ] =
            &VmDnsOnTcpForwardResponseSizeRead,
    [VM_SOCK_EVENT_TYPE_TCP_FWD_RESPONSE_DATA_READ] =
            &VmDnsOnTcpForwardResponse,
    [VM_SOCK_EVENT_TYPE_CONNECTION_CLOSED] =
            &VmDnsOnDisconnect,
    [VM_SOCK_EVENT_TYPE_TIMER_EXPIRATION] =
            &VmDnsOnTimerExpiration,
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
    [VM_SOCK_EVENT_TYPE_QUERY] =
            "QUERY",
    [VM_SOCK_EVENT_TYPE_TIMER_EXPIRATION] =
            "TIMER_EXPIRATION",
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

    dwError = VmDnsForwarderPacketListInitialize(
                    &pSockContext->pForwarderPacketList);
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

    dwError = VmDnsSockCreateTimerSocket(
                    VMW_DNS_TIMER_EXPIRATION_INITIAL_MS,
                    VMW_DNS_TIMER_EXPIRATION_INTERVAL_MS,
                    &pSockContext->pTimerSocket);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSockEventQueueAdd(
                    pSockContext->pEventQueue,
                    FALSE,
                    pSockContext->pTimerSocket);
    BAIL_ON_VMDNS_ERROR(dwError);

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
    PVMDNS_SOCK_CONTEXT pSockContext = gpSrvContext->pSockContext;

    if (pSockContext)
    {
        if (pSockContext->pTimerSocket)
        {
            VmDnsSockRelease(pSockContext->pTimerSocket);
        }
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
    VmDnsUdpReceiveData(
              pSockEventContext->pEventQueue,
              pSockContext->pListenerUDP,
              NULL);
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
            VMDNS_LOG_INFO(
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
                        pSockContext->pEventQueue,
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
                VmDnsSockClose(pSocket);
            }
            else
            {
#ifdef WIN32
                dwError = VmDnsUdpReceiveNewData(
                                      pSockContext->pEventQueue,
                                      pSocket);
#endif
            }
            pIoBuffer = NULL;
            dwError = 0;
        }

        VmDnsSockRelease(pSocket);

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
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET           pSocket,
    VM_SOCK_EVENT_TYPE   sockEvent,
    PVM_SOCK_IO_BUFFER   pIoBuffer,
    DWORD                dwError
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

            dwError = pEventHandler(pEventQueue, pSocket, pIoBuffer);
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

error:
    goto cleanup;
}

static
DWORD
VmDnsOnTcpNewConnection(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD dwError = 0;
    PVM_SOCK_IO_BUFFER pIoBufferNew = NULL;

    VmMetricsGaugeIncrement(gVmDnsGaugeMetrics[DNS_OUTSTANDING_REQUEST_COUNT]);

    if (!pSocket)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsSockAllocateIoBuffer(
                            VM_SOCK_EVENT_TYPE_TCP_REQUEST_SIZE_READ,
                            NULL,
                            sizeof(UINT16),
                            &pIoBufferNew
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSockSetData(pSocket, pIoBufferNew, NULL);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSockEventQueueAdd(
                            pEventQueue,
                            TRUE,
                            pSocket
                            );
    BAIL_ON_VMDNS_ERROR(dwError);

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
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    if (pSocket)
    {
        VmDnsSockClose(pSocket);
    }

    return ERROR_SUCCESS;
}

static
DWORD
VmDnsOnDataAvailable(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
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

    dwError = VmDnsReceiveData(pEventQueue, pSocket, pIoBuffer);
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
    PVM_SOCK_EVENT_QUEUE pEventQueue,
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
        dwError = VmDnsTcpReceiveData(pEventQueue, pSocket, pIoBuffer);
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else if (dwProtocol == SOCK_DGRAM)
    {
        dwError = VmDnsUdpReceiveData(pEventQueue, pSocket, pIoBuffer);
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
    PVM_SOCK_EVENT_QUEUE pEventQueue,
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
        dwError = VmDnsOnTcpRequestDataRead(pEventQueue, pSocket, pIoBuffer);
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
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET           pSocket,
    PVM_SOCK_IO_BUFFER   pIoBuffer
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
        dwError = VmDnsUdpReceiveNewData(pEventQueue,pSocket);
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        dwError = VmDnsOnUdpRequestDataRead(pEventQueue, pSocket, pIoBuffer);
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
        dwError = VmDnsOnTcpRequestSizeRead(NULL, pSocket, pIoBuffer);
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
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET           pSocket
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
        dwError = VmDnsOnUdpRequestDataRead(pEventQueue, pSocket, pIoBuffer);
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
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET           pSocket,
    PVM_SOCK_IO_BUFFER   pIoBuffer
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

    dwError = VmDnsSockRead(
                        pSocket,
                        pIoBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

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

    dwError = VmDnsSockSetData(pSocket, pIoNewBuffer, &pOldData);
    BAIL_ON_VMDNS_ERROR(dwError);
    pIoNewBuffer = NULL;

    if (pEventQueue)
    {
        dwError = VmDnsSockEventQueueRearm(pEventQueue, TRUE, pSocket);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

#ifdef WIN32
    dwError = VmDnsSockRead(
                        pSocket,
                        pIoNewBuffer);

    if (dwError == ERROR_SUCCESS)
    {
        dwError = VmDnsOnTcpRequestDataRead(pEventQueue, pSocket, pIoNewBuffer);
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
#endif

cleanup:

    if (pIoNewBuffer)
    {
        VMDNS_LOG_IO_RELEASE(pIoBuffer);
        VmDnsSockReleaseIoBuffer(pIoNewBuffer);
    }
    if (pOldData)
    {
        VMDNS_LOG_IO_RELEASE(pOldData);
        VmDnsSockReleaseIoBuffer(pOldData);
    }

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDnsOnTcpRequestDataRead(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD dwError = 0;
    DWORD dwFrwdError = 0;
    PVM_SOCK_IO_BUFFER  pIoNewBuffer = NULL;
    PVM_SOCK_IO_BUFFER  pIoSizeBuffer = NULL;
    PBYTE pResponse = NULL;
    DWORD dwDnsResponseSize = 0;
    UCHAR rCode = 0;
    BOOL bQueryInZone = FALSE;
    BOOL bUpdateInZone = FALSE;

    if (!pIoBuffer)
    {
        dwError = ERROR_INVALID_MESSAGE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsSockRead(
                         pSocket,
                         pIoBuffer
                         );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (!pIoBuffer->dwTotalBytesTransferred)
    {
        dwError = WSAECONNRESET;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (pIoBuffer->dwExpectedSize > pIoBuffer->dwTotalBytesTransferred)
    {
        dwError = VmDnsSockSetData(pSocket, pIoBuffer, NULL);
        BAIL_ON_VMDNS_ERROR(dwError);
        pIoBuffer = NULL;

        if (pEventQueue)
        {
            dwError = VmDnsSockEventQueueRearm(pEventQueue, TRUE, pSocket);
            BAIL_ON_VMDNS_ERROR(dwError);
        }

        dwError = ERROR_IO_PENDING;
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        dwError = VmDnsSockEventQueueRemove(pEventQueue, pSocket);
        BAIL_ON_VMDNS_ERROR(dwError);

        dwError = VmDnsProcessRequest(
                            pIoBuffer->pData,
                            pIoBuffer->dwTotalBytesTransferred,
                            &pResponse,
                            &dwDnsResponseSize,
                            &rCode,
                            &bQueryInZone,
                            &bUpdateInZone
                            );

        if (rCode && !bUpdateInZone && !bQueryInZone)
        {
            dwFrwdError = VmDnsOnForwarderRequest(
                                    pEventQueue,
                                    FALSE,
                                    pSocket,
                                    pIoBuffer
                                    );
            if (dwFrwdError == ERROR_SUCCESS)
            {
                // Forwarder state machine will fulfill the response, cleanup
                goto cleanup;
            }
        }

        if (pResponse && dwDnsResponseSize > 0)
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
                                        pEventQueue,
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
                                        pEventQueue,
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
    if (bQueryInZone || dwFrwdError)
    {
        VmDnsMetricsRcodeUpdate(bQueryInZone, METRICS_VDNS_RCODE_OP_TCP_REQ_READ, rCode);
    }

    VMDNS_SAFE_FREE_MEMORY(pResponse);
    return dwError;

error:

    goto cleanup;
}

DWORD
VmDnsOnUdpRequestDataRead(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD dwError = 0;
    DWORD dwFrwdError = 0;
    PVM_SOCK_IO_BUFFER  pIoNewBuffer = NULL;
    PBYTE pResponse = NULL;
    DWORD dwDnsResponseSize = 0;
    UCHAR rCode = 0;
    BOOL bQueryInZone = FALSE;
    BOOL bUpdateInZone = FALSE;

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

    (VOID)VmDnsLogDnsMessage(VMDNS_LOG_LEVEL_DEBUG, "DNS UDP REQ: ", pIoBuffer->pData, pIoBuffer->dwTotalBytesTransferred);

    dwError = VmDnsProcessRequest(
                        pIoBuffer->pData,
                        pIoBuffer->dwTotalBytesTransferred,
                        &pResponse,
                        &dwDnsResponseSize,
                        &rCode,
                        &bQueryInZone,
                        &bUpdateInZone
                        );
    if (rCode && !bUpdateInZone && !bQueryInZone)
    {
        dwFrwdError = VmDnsOnForwarderRequest(
                            pEventQueue,
                            TRUE,
                            pSocket,
                            pIoBuffer
                            );
        if (dwFrwdError == ERROR_SUCCESS)
        {
            // Forwarder state machine will fulfill the response, cleanup
            goto cleanup;
        }
    }


    if (pResponse && dwDnsResponseSize > 0)
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

        (VOID)VmDnsLogDnsMessage(VMDNS_LOG_LEVEL_DEBUG, "DNS UDP RESP: ", pResponse, dwDnsResponseSize);

        dwError = VmDnsSockWrite(
                        pSocket,
                        (struct sockaddr*)&pIoBuffer->clientAddr,
                        pIoBuffer->addrLen,
                        pIoNewBuffer
                    );

        VMDNS_LOG_DEBUG("DNS RESP: Status: %d", dwError);

        if (dwError == ERROR_SUCCESS)
        {
            dwError = VmDnsOnUdpResponseDataWrite(
                                    pEventQueue,
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

cleanup:

    if (pIoNewBuffer)
    {
        VMDNS_LOG_IO_RELEASE(pIoNewBuffer);
        VmDnsSockReleaseIoBuffer(pIoNewBuffer);
    }
    if (bQueryInZone || dwFrwdError)
    {
        VmDnsMetricsRcodeUpdate(bQueryInZone, METRICS_VDNS_RCODE_OP_UDP_REQ_READ, rCode);
    }

    VMDNS_SAFE_FREE_MEMORY(pResponse);
    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDnsOnTcpResponseSizeWrite(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
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
    PVM_SOCK_EVENT_QUEUE pEventQueue,
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
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD dwError = 0;

    VMDNS_LOG_DEBUG(
            "Complete Event - Buffer Size: %d, Buffer Ptr: %p, Thread Ptr: %p",
            pIoBuffer ? pIoBuffer->dwTotalBytesTransferred : 0,
            pIoBuffer,
            pthread_self());

    if (!pIoBuffer->dwTotalBytesTransferred)
    {
        dwError = WSAECONNRESET;
        BAIL_ON_VMDNS_ERROR(dwError);
    }
#ifdef WIN32
    dwError = VmDnsUdpReceiveNewData(pEventQueue, pSocket);
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
VmDnsAllocateQueryBuffer(
    PVM_SOCK_IO_BUFFER  pSrcIoBuffer,
    PVM_SOCK_IO_BUFFER* ppIoBuffer
    )
{
    DWORD dwError = 0;
    PVM_SOCK_IO_BUFFER pIoBuffer = NULL;

    if (!pSrcIoBuffer || !ppIoBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsSockAllocateIoBuffer(
                                  VM_SOCK_EVENT_TYPE_QUERY,
                                  NULL,
                                  pSrcIoBuffer->dwTotalBytesTransferred,
                                  &pIoBuffer
                                  );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsCopyMemory(
                        pIoBuffer->pData,
                        pIoBuffer->dwExpectedSize,
                        pSrcIoBuffer->pData,
                        pSrcIoBuffer->dwCurrentSize
                        );
    BAIL_ON_VMDNS_ERROR(dwError);

    pIoBuffer->dwCurrentSize = 0;
    pIoBuffer->dwTotalBytesTransferred = 0;

    memcpy(
        &pIoBuffer->clientAddr,
        &pSrcIoBuffer->clientAddr,
        pSrcIoBuffer->addrLen);

    pIoBuffer->addrLen = pSrcIoBuffer->addrLen;

    *ppIoBuffer = pIoBuffer;

cleanup:

    return dwError;
error:

    if (ppIoBuffer)
    {
        *ppIoBuffer = NULL;
    }
    if (pIoBuffer)
    {
        VmDnsSockReleaseIoBuffer(pIoBuffer);
    }
    goto cleanup;
}

static
DWORD
VmDnsOnForwarderRequest(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    BOOL                 bUseUDP,
    PVM_SOCKET           pSocket,
    PVM_SOCK_IO_BUFFER   pIoBuffer
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

    pForwarderContext->pClientSocket = VmDnsSockAcquire(pSocket);

    dwError = VmDnsAllocateQueryBuffer(
                                  pIoBuffer,
                                  &pForwarderContext->pQueryBuffer
                                  );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsForwardRequest(
                          pForwarderContext,
                          pEventQueue,
                          bUseUDP
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
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    BOOL                 bUseUDP,
    PVM_SOCKET           pSocket,
    PVM_SOCK_IO_BUFFER   pIoBuffer
    )
{
    DWORD dwError = 0;
    DWORD dwForwardRequestError = 0;
    PVMDNS_FORWARDER_PACKET_CONTEXT pForwarderContext = NULL;
    PVM_SOCK_EVENT_CONTEXT pSockEventContext = NULL;
    PVM_SOCKET pClientSocket = NULL;
    PVM_SOCK_IO_BUFFER pQueryIoBuffer = NULL;
    PVM_SOCK_IO_BUFFER pIoNewBuffer = NULL;
    PVM_SOCK_IO_BUFFER pIoSizeBuffer = NULL;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    DWORD dwResponseCode = 0;
    BOOL bUpdateRCodeMetric = FALSE;
    PVMDNS_SOCK_CONTEXT pSockContext = gpSrvContext->pSockContext;
    PVMDNS_FORWARDER_PACKET_ENTRY pForwarderPacketEntry = NULL;

    if (!pSocket || !pIoBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsSockSetEventContext(pIoBuffer,NULL,&pSockEventContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    pForwarderContext = (PVMDNS_FORWARDER_PACKET_CONTEXT)pSockEventContext;

    if (!pForwarderContext)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pClientSocket = VmDnsSockAcquire(pForwarderContext->pClientSocket);
    pQueryIoBuffer = pForwarderContext->pQueryBuffer;

    if (!pClientSocket)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    pForwarderPacketEntry = pForwarderContext->pForwarderPacketEntry;
    if (pForwarderPacketEntry)
    {
        dwError = VmDnsForwarderPacketListRemoveEntry(
                                  pSockContext->pForwarderPacketList,
                                  pForwarderPacketEntry);
        BAIL_ON_VMDNS_ERROR(dwError);

        VmDnsForwarderPacketEntryRelease(pForwarderPacketEntry);
        VmDnsReleaseForwarderPacketContext(pForwarderContext);
    }

    dwError = VmDnsCompleteForwardResponse(
                              bUseUDP,
                              pEventQueue,
                              pSocket,
                              pIoBuffer,
                              &pResponse,
                              &dwResponseSize,
                              &dwResponseCode
                              );

    (VOID)VmDnsLogDnsMessage(VMDNS_LOG_LEVEL_DEBUG, "DNS FWD RESP ", pResponse, dwResponseSize);

    if (dwResponseCode || dwError)
    {
        dwForwardRequestError = VmDnsForwardRequest(
                                      pForwarderContext,
                                      pEventQueue,
                                      bUseUDP
                                      );
    }
    if (!dwResponseCode || dwForwardRequestError)
    {
        bUpdateRCodeMetric = TRUE;
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

            (VOID)VmDnsLogDnsMessage(VMDNS_LOG_LEVEL_DEBUG, "DNS FWD UDP RESP: ", pIoNewBuffer->pData, pIoNewBuffer->dwTotalBytesTransferred);

            dwError = VmDnsSockWrite(
                            pClientSocket,
                            (struct sockaddr*)&pQueryIoBuffer->clientAddr,
                            pQueryIoBuffer->addrLen,
                            pIoNewBuffer
                            );

            VMDNS_LOG_DEBUG("DNS FWD UDP RESP: Status: %d", dwError);

            if (dwError == ERROR_SUCCESS)
            {
                dwError = VmDnsOnUdpResponseDataWrite(
                                        NULL,
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
                                        NULL,
                                        pClientSocket,
                                        pIoSizeBuffer
                                        );
                BAIL_ON_VMDNS_ERROR(dwError);
            }
            else if (dwError == ERROR_IO_PENDING)
            {
                pIoSizeBuffer = NULL;
            }

            (VOID)VmDnsLogDnsMessage(VMDNS_LOG_LEVEL_DEBUG, "DNS TCP RESP: ", pIoNewBuffer->pData, pIoNewBuffer->dwTotalBytesTransferred);

            dwError = VmDnsSockWrite(
                            pClientSocket,
                            NULL,
                            0,
                            pIoNewBuffer
                            );

            VMDNS_LOG_DEBUG("DNS TCP RESP: Status: %d", dwError);

            if (dwError == ERROR_SUCCESS)
            {
                dwError = VmDnsOnTcpResponseDataWrite(
                                        NULL,
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

    VMDNS_SAFE_FREE_MEMORY(pResponse);

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
    if (bUpdateRCodeMetric)
    {
        VmDnsMetricsRcodeUpdate(false, METRICS_VDNS_RCODE_OP_FORWARDER_RESP, dwResponseCode);
    }

    return dwError;
error:

    goto cleanup;
}

static
DWORD
VmDnsOnUdpForwardResponse(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD dwError = 0;

    if (!pIoBuffer || !pSocket)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsSockRead(
                          pSocket,
                          pIoBuffer
                          );
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsOnForwarderResponse(pEventQueue,TRUE, pSocket, pIoBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}


static
DWORD
VmDnsOnTcpForwardResponseSizeRead(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET           pSocket,
    PVM_SOCK_IO_BUFFER   pIoBuffer
    )
{
    DWORD dwError = 0;
    UINT16 uSizeToRead = 0;
    PVM_SOCK_IO_BUFFER  pIoNewBuffer = NULL;
    PVOID pOldData = NULL;
    PVMDNS_FORWARDER_PACKET_CONTEXT pForwarderContext = NULL;
    PVM_SOCK_EVENT_CONTEXT pSockEventContext = NULL;

    if (!pIoBuffer || !pSocket)
    {
        dwError = ERROR_INVALID_MESSAGE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsSockSetEventContext(pIoBuffer,NULL,&pSockEventContext);
    BAIL_ON_VMDNS_ERROR(dwError);

    pForwarderContext = (PVMDNS_FORWARDER_PACKET_CONTEXT)pSockEventContext;

    pSockEventContext = NULL;

    if (!pForwarderContext)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsSockRead(
                        pSocket,
                        pIoBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

    if (!pIoBuffer->dwTotalBytesTransferred)
    {
        dwError = WSAECONNRESET;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    uSizeToRead = htons(*((UINT16*)(pIoBuffer->pData)));

    dwError = VmDnsSockAllocateIoBuffer(
                        VM_SOCK_EVENT_TYPE_TCP_FWD_RESPONSE_DATA_READ,
                        (PVM_SOCK_EVENT_CONTEXT)pForwarderContext,
                        uSizeToRead,
                        &pIoNewBuffer);
    BAIL_ON_VMDNS_ERROR(dwError);

    dwError = VmDnsSockSetData(pSocket, pIoNewBuffer, &pOldData);
    BAIL_ON_VMDNS_ERROR(dwError);

    pIoNewBuffer = NULL;

    if (pEventQueue)
    {
        dwError = VmDnsSockEventQueueRearm(pEventQueue, TRUE, pSocket);
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:

    if (pIoNewBuffer)
    {
        VMDNS_LOG_IO_RELEASE(pIoBuffer);
        VmDnsSockReleaseIoBuffer(pIoNewBuffer);
    }
    if (pOldData)
    {
        VMDNS_LOG_IO_RELEASE(pOldData);
        VmDnsSockReleaseIoBuffer(pOldData);
    }

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDnsOnTcpForwardResponse(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD dwError = 0;

    if (!pIoBuffer || !pSocket)
    {
        dwError = ERROR_INVALID_MESSAGE;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    dwError = VmDnsSockRead(
                         pSocket,
                         pIoBuffer
                         );
    BAIL_ON_VMDNS_ERROR(dwError);

    if (!pIoBuffer->dwTotalBytesTransferred)
    {
        dwError = WSAECONNRESET;
        BAIL_ON_VMDNS_ERROR(dwError);
    }

    if (pIoBuffer->dwExpectedSize > pIoBuffer->dwTotalBytesTransferred)
    {
        dwError = VmDnsSockSetData(pSocket, pIoBuffer, NULL);
        BAIL_ON_VMDNS_ERROR(dwError);
        pIoBuffer = NULL;

        if (pEventQueue)
        {
            dwError = VmDnsSockEventQueueRearm(pEventQueue, TRUE, pSocket);
            BAIL_ON_VMDNS_ERROR(dwError);
        }

        dwError = ERROR_IO_PENDING;
        BAIL_ON_VMDNS_ERROR(dwError);
    }
    else
    {
        dwError = VmDnsOnForwarderResponse(
                                    pEventQueue,
                                    FALSE,
                                    pSocket,
                                    pIoBuffer
                                    );
        BAIL_ON_VMDNS_ERROR(dwError);
    }

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
VOID
VmDnsMetricsRcodeUpdate(
    BOOL                    bQueryInZone,
    METRICS_VDNS_RCODE_OPS  operation,
    UCHAR                   rCode
    )
{
    METRICS_VDNS_RCODE_DOMAINS metricsRcodeDomain = 0;
    METRICS_VDNS_RCODE_ERRORS metricsRcodeError = 0;

    metricsRcodeDomain = VmDnsMetricsMapRcodeDomainToEnum(bQueryInZone);
    metricsRcodeError = VmDnsMetricsMapRcodeErrorToEnum(rCode);

    VmMetricsCounterIncrement(gVmDnsRcodeErrorMetrics[metricsRcodeDomain][operation][metricsRcodeError]);
}

static
DWORD
VmDnsOnTimerExpiration(
    PVM_SOCK_EVENT_QUEUE pEventQueue,
    PVM_SOCKET           pSocket,
    PVM_SOCK_IO_BUFFER   pIoBuffer
    )
{
    DWORD dwError = 0;
    BOOLEAN bLocked = FALSE;
    PVMDNS_SOCK_CONTEXT pSockContext = gpSrvContext->pSockContext;
    PLIST_ENTRY pListEntry = NULL;
    PVMDNS_FORWARDER_PACKET_CONTEXT pForwarderPacketContext = NULL;
    PVMDNS_FORWARDER_PACKET_LIST pForwarderPacketList = NULL;
    PVMDNS_FORWARDER_PACKET_ENTRY pForwarderPacketEntry = NULL;
    UINT64 uiCurrentTime = 0;
    UINT64 uiExpirationTime = 0;
    PVM_SOCK_IO_BUFFER pSockIoBuffer = NULL;
    BOOLEAN bPacketListLocked = FALSE;

    uiCurrentTime = VmDnsGetTimeInMilliSec();

    dwError = VmDnsLockMutex(pSockContext->pMutex);
    BAIL_ON_VMDNS_ERROR(dwError);
    bLocked = TRUE;

    pForwarderPacketList = pSockContext->pForwarderPacketList;

    dwError = VmDnsLockMutex(pForwarderPacketList->pLock);
    BAIL_ON_VMDNS_ERROR(dwError);
    bPacketListLocked = TRUE;

    for (pListEntry = pForwarderPacketList->ForwarderPacketListHead.Flink;
        (pListEntry != &pForwarderPacketList->ForwarderPacketListHead);
        pListEntry = pListEntry->Flink)
    {
        pForwarderPacketEntry = CONTAINING_RECORD(pListEntry,
                                                  VMDNS_FORWARDER_PACKET_ENTRY,
                                                  ForwarderPacketList);

        if (pForwarderPacketEntry)
        {
            uiExpirationTime = pForwarderPacketEntry->uiExpirationTime;
            if (uiCurrentTime < uiExpirationTime)
            {
                break;
            }

            pListEntry = pListEntry->Flink;

            RemoveEntryList(&pForwarderPacketEntry->ForwarderPacketList);
            --pForwarderPacketList->dwCurrentCount;

            dwError = VmDnsSockEventQueueRemove(pEventQueue, pForwarderPacketEntry->pSocket);
            BAIL_ON_VMDNS_ERROR(dwError);

            dwError = VmDnsSockGetData(pForwarderPacketEntry->pSocket,
                                       (PVOID*)&pSockIoBuffer);
            BAIL_ON_VMDNS_ERROR(dwError);

            if (pSockIoBuffer)
            {
                VmDnsSockReleaseIoBuffer(pSockIoBuffer);
            }

            VmDnsForwarderPacketEntryRelease(pForwarderPacketEntry);
            VmDnsForwarderPacketEntryRelease(pForwarderPacketEntry);

            pForwarderPacketContext = pForwarderPacketEntry->pForwarderPacketContext;

            VmDnsReleaseForwarderPacketContext(pForwarderPacketContext);
            VmDnsReleaseForwarderPacketContext(pForwarderPacketContext);
        }
    }

cleanup:
    if (bPacketListLocked)
    {
        VmDnsUnlockMutex(pForwarderPacketList->pLock);
    }
    if (bLocked)
    {
        VmDnsUnlockMutex(pSockContext->pMutex);
    }

    return dwError;

error:

    goto cleanup;
}
