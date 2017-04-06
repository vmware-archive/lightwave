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
VmKdcRecvTcp(
    int sock,
    unsigned char *msg,
    int msgLen);

static DWORD
VmKdcSendTcpResponse(
    PVMKDC_CONTEXT pContext,
    PVMKDC_DATA krbMsg);

static
VOID
VmKdcSignalThreadDone(
    PVMKDC_CONTEXT pContext)
{
    pthread_mutex_lock(&pContext->pGlobals->mutex);
    pContext->pGlobals->workerThreadCount--;
    pthread_mutex_unlock(&pContext->pGlobals->mutex);
    pthread_cond_signal(&pContext->pGlobals->cond);
}


static
DWORD
VmKdcCreateThreadMaxLimit(
    PVMKDC_CONTEXT pContext,
    VmKdcStartRoutine* pStartRoutine)
{
    DWORD dwError = 0;


    pthread_mutex_lock(&pContext->pGlobals->mutex);
    while (pContext->pGlobals->workerThreadCount >= pContext->pGlobals->workerThreadMax)
    {
        pthread_cond_wait(&pContext->pGlobals->cond, &pContext->pGlobals->mutex);
    }

    dwError = pthread_create(
                  &pContext->pGlobals->thread,
                  &pContext->pGlobals->attrDetach,
                  pStartRoutine,
                  pContext);
    BAIL_ON_VMKDC_ERROR(dwError);
    pContext->pGlobals->workerThreadCount++;

error:
    pthread_mutex_unlock(&pContext->pGlobals->mutex);
    return dwError;
}


static DWORD
_VmKdcMakeIpAddress(
    int port,
    BOOLEAN bIsIpV6,
    struct sockaddr **ppAddr,
    int *pAddrLen,
    int *pAddrType)
{
    DWORD dwError = 0;
    struct sockaddr_in  *p4addr = NULL;
    struct sockaddr_in6 *p6addr = NULL;
    void                *pAddr = NULL;
    short               addrType = AF_INET;
    int                 addrLen = 0;

    addrLen = bIsIpV6 ? sizeof(struct sockaddr_in6) :
                        sizeof(struct sockaddr_in);
    dwError = VmKdcAllocateMemory(
                  addrLen,
                  (PVOID*)&pAddr);
    BAIL_ON_VMKDC_ERROR(dwError);

    if (bIsIpV6)
    {
        addrType = AF_INET6;
        p6addr = (struct sockaddr_in6 *) pAddr;
        p6addr->sin6_family = addrType;
        p6addr->sin6_port = htons((UINT16) port);
    }
    else
    {
        addrType = AF_INET;
        p4addr = (struct sockaddr_in *) pAddr;
        p4addr->sin_family = addrType;
        p4addr->sin_port = htons((UINT16) port);
    }

    *ppAddr = (struct sockaddr *) pAddr;
    *pAddrLen = addrLen;
    *pAddrType = addrType;

error:
    return dwError;
}


DWORD
VmKdcSrvOpenServicePort(
    PVMKDC_GLOBALS pGlobals,
    BOOLEAN bIpV6,
    VMKDC_SERVICE_PORT_TYPE portType)
{
    DWORD dwError = 0;
    INT64 sock = -1;
    int sts = 0;
    int on = 1;
    char *portTypeStr = "Udp";
    int socketType = SOCK_DGRAM;
    struct sockaddr *saddr = NULL;
    int saddr_len = 0;
    int saddr_type = AF_INET;
    int optname = 0;

    dwError = _VmKdcMakeIpAddress(
                   pGlobals->iListenPort,
                   bIpV6,
                   &saddr,
                   &saddr_len,
                   &saddr_type);
    BAIL_ON_VMKDC_ERROR(dwError);

    if (portType == VMKDC_SERVICE_PORT_TCP)
    {
        portTypeStr = "Tcp";
        socketType = SOCK_STREAM;
    }
    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL,
             "VmKdcSrvOpenServicePort%s called...",
             portTypeStr);

    sock = socket(saddr_type, socketType, 0);
    if (sock == -1)
    {
#ifdef _WIN32
        errno = WSAGetLastError();
#endif
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    if (bIpV6)
    {
        int on = 1;

#ifdef _WIN32
        setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (void *) &on, sizeof(on));
#else
        setsockopt(sock, SOL_IPV6, IPV6_V6ONLY, (void *) &on, sizeof(on));
#endif
    }

    if (portType == VMKDC_SERVICE_PORT_TCP)
    {
#ifdef _WIN32
        optname = SO_EXCLUSIVEADDRUSE;
#else
        optname = SO_REUSEADDR;
#endif
        if (setsockopt(sock,
                   SOL_SOCKET,
                   optname,
                   (const char *)(&on),
                   sizeof(on)) == -1)
        {
#ifdef _WIN32
            errno = WSAGetLastError();
#endif
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMKDC_ERROR(dwError);
        }
    }

    sts = bind(sock, saddr, saddr_len);
    if (sts == -1)
    {
        tcp_close(sock);
        sock = -1;
    }

    if (portType == VMKDC_SERVICE_PORT_TCP)
    {
        if (bIpV6)
        {
            pGlobals->iAcceptSock6 = sock;
            pGlobals->addrLen6 = saddr_len;
        }
        else
        {
            pGlobals->iAcceptSock = sock;
            pGlobals->addrLen = saddr_len;
        }
    }
    else
    {
        if (bIpV6)
        {
            pGlobals->iAcceptSock6Udp = sock;
            pGlobals->addrLen6 = saddr_len;
        }
        else
        {
            pGlobals->iAcceptSockUdp = sock;
            pGlobals->addrLen = saddr_len;
        }
    }

error:
    VMKDC_SAFE_FREE_MEMORY(saddr);
    if (dwError)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "dwError=%d errno=%d", dwError, errno);
        if (sock != -1)
        {
            tcp_close(sock);
        }
    }

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcSrvOpenServicePort%s done.", portTypeStr);

    return dwError;
}


DWORD
VmKdcSrvServicePortListen(
    PVMKDC_GLOBALS pGlobals)
{
    DWORD dwError = 0;
    int sts = 0;

    if (pGlobals->iAcceptSock >= 0)
    {
        sts = listen(pGlobals->iAcceptSock, 5);
        if (sts == -1)
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMKDC_ERROR(dwError);
        }
    }

    if (pGlobals->iAcceptSock6 >= 0)
    {
        sts = listen(pGlobals->iAcceptSock6, 5);
        if (sts == -1)
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMKDC_ERROR(dwError);
        }
    }
error:
    return dwError;
}


static DWORD
VmKdcSrvServiceAcceptConn(
    PVMKDC_GLOBALS pGlobals,
    int *acceptSocket,
    int *acceptSocketUdp)
{
    DWORD dwError = 0;
    fd_set rmask;
    INT64 sts = 0;
    INT64 maxFd = -1;

    FD_ZERO(&rmask);
    if (pGlobals->iAcceptSock >= 0)
    {
        FD_SET(pGlobals->iAcceptSock, &rmask);
        if (pGlobals->iAcceptSock > maxFd)
        {
            maxFd = pGlobals->iAcceptSock;
        }
    }
    if (pGlobals->iAcceptSockUdp >= 0)
    {
        FD_SET(pGlobals->iAcceptSockUdp, &rmask);
        if (pGlobals->iAcceptSockUdp > maxFd)
        {
            maxFd = pGlobals->iAcceptSockUdp;
        }
    }
    if (pGlobals->iAcceptSock6 >= 0)
    {
        FD_SET(pGlobals->iAcceptSock6, &rmask);
        if (pGlobals->iAcceptSock6 > maxFd)
        {
            maxFd = pGlobals->iAcceptSock6;
        }
    }
    if (pGlobals->iAcceptSock6Udp >= 0)
    {
        FD_SET(pGlobals->iAcceptSock6Udp, &rmask);
        if (pGlobals->iAcceptSock6Udp > maxFd)
        {
            maxFd = pGlobals->iAcceptSock6Udp;
        }
    }

    sts = select((int) (maxFd + 1), &rmask, NULL, NULL, NULL);
    if (sts <= 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }
    if (VmKdcdState() == VMKDC_SHUTDOWN)
    {
        goto error;
    }

    /* Inspect ether IPv4 or IPv6 TCP sockets for activity */
    if (FD_ISSET(pGlobals->iAcceptSock, &rmask))
    {
        sts = accept(pGlobals->iAcceptSock, NULL, NULL);
        if (sts != -1)
        {
            *acceptSocket = (int) sts;
        }
    }
    else if (FD_ISSET(pGlobals->iAcceptSock6, &rmask))
    {
        sts = accept(pGlobals->iAcceptSock6, NULL, NULL);
        if (sts != -1)
        {
            *acceptSocket = (int) sts;
        }
    }

    /* Inspect ether IPv4 or IPv6 UDP sockets for activity */
    if (FD_ISSET(pGlobals->iAcceptSockUdp, &rmask))
    {
        *acceptSocketUdp = (int) pGlobals->iAcceptSockUdp;
    }
    else if (FD_ISSET(pGlobals->iAcceptSock6Udp, &rmask))
    {
        *acceptSocketUdp = (int) pGlobals->iAcceptSock6Udp;
    }
error:
    return dwError;
}


static void
VmKdcFreeRequest(
    PVMKDC_REQUEST *ppRequest)
{
    PVMKDC_REQUEST pRequest = *ppRequest;

    if (pRequest)
    {
        if (pRequest->requestSocket >= 0)
        {
            tcp_close(pRequest->requestSocket);
            pRequest->requestSocket = -1;
        }
        VMKDC_SAFE_FREE_MEMORY(pRequest->requestBuf);
        VMKDC_SAFE_FREE_MEMORY(pRequest->pvClientAddr);
        VMKDC_SAFE_FREE_KEY(pRequest->masterKey);
    }
    VMKDC_SAFE_FREE_MEMORY(pRequest);
    *ppRequest = NULL;
}


static DWORD
VmKdcAllocateRequest(
    int requestSocket,
    BOOLEAN bRequestIsUdp,
    PVMKDC_GLOBALS pGlobals,
    PVMKDC_REQUEST *ppRetRequest)
{
    PVMKDC_REQUEST pRequest = NULL;
    PUCHAR pRequestBuf = NULL;
    DWORD dwError = 0;
    PVOID pClientAddr = NULL;

    dwError = VmKdcAllocateMemory(
                  sizeof(VMKDC_REQUEST),
                  (PVOID*)&pRequest);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcAllocateMemory(
                  pGlobals->addrLen,
                  (PVOID*)&pClientAddr);
    BAIL_ON_VMKDC_ERROR(dwError);


    if (bRequestIsUdp)
    {
        dwError = VmKdcAllocateMemory(
                      VMKDC_UDP_READ_BUFSIZ,
                      (PVOID*) &pRequestBuf);
        BAIL_ON_VMKDC_ERROR(dwError);

        pRequest->requestBufLen = VMKDC_UDP_READ_BUFSIZ;
        pRequest->requestBuf = pRequestBuf;
        pRequest->bRequestIsUdp = bRequestIsUdp;
    }

    pthread_mutex_lock(&gVmkdcGlobals.mutex);
    /*
     * The global masterKey will be NULL when vmdir is unavailable.
     */
    if (gVmkdcGlobals.masterKey)
    {
        dwError = VmKdcCopyKey(gVmkdcGlobals.masterKey,
                               &pRequest->masterKey);
    }
    pthread_mutex_unlock(&gVmkdcGlobals.mutex);
    BAIL_ON_VMKDC_ERROR(dwError);

    pRequest->pvClientAddr = pClientAddr;
    pRequest->dwClientAddrLen = pGlobals->addrLen;
    pRequest->requestSocket = requestSocket;
    *ppRetRequest = pRequest;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_MEMORY(pRequest);
        VMKDC_SAFE_FREE_MEMORY(pClientAddr);
        VMKDC_SAFE_FREE_MEMORY(pRequestBuf);
    }
    return dwError;
}

static DWORD
VmKdcAllocateContext(
    int requestSocket,
    BOOLEAN bRequestIsUdp,
    PVMKDC_GLOBALS pGlobals,
    PVMKDC_CONTEXT *ppContext)
{
    DWORD dwError = 0;
    PVMKDC_CONTEXT pContext = NULL;
    PVMKDC_REQUEST pRequest = NULL;

    dwError = VmKdcAllocateRequest(
                  requestSocket,
                  bRequestIsUdp,
                  pGlobals,
                  &pRequest);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_CONTEXT), (PVOID*)&pContext);
    BAIL_ON_VMKDC_ERROR(dwError);

    pContext->pGlobals = pGlobals;
    pContext->pRequest = pRequest;

    *ppContext = pContext;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_MEMORY(pRequest);
        VMKDC_SAFE_FREE_MEMORY(pContext);
    }
    return dwError;
}


static VOID
VmKdcFreeContext(
    PVMKDC_CONTEXT *ppContext)
{
    PVMKDC_CONTEXT pContext = NULL;

    if (ppContext)
    {
        pContext = *ppContext;
    }
    if (pContext)
    {
        VmKdcFreeRequest(&pContext->pRequest);
        VMKDC_SAFE_FREE_MEMORY(pContext);
    }
    *ppContext = NULL;
}


static DWORD
VmKdcReadTcpRequestSize(
    PVMKDC_CONTEXT pContext)
{
    DWORD dwError = 0;
    INT32 krbMsgLen = 0;
    PUCHAR requestBuf = NULL;

    dwError = VmKdcRecvTcp(
                  pContext->pRequest->requestSocket,
                  (void *) &krbMsgLen,
                  sizeof(krbMsgLen));
    BAIL_ON_VMKDC_ERROR(dwError);

    krbMsgLen = ntohl(krbMsgLen);
    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL,
             "VmKdcReadTcpRequestSize: KrbMsgLen=%d", krbMsgLen);
    if (krbMsgLen > MAX_KRB_REQ_LEN || krbMsgLen < 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    dwError = VmKdcAllocateMemory(
                  krbMsgLen,
                  (PVOID*) &requestBuf);
    BAIL_ON_VMKDC_ERROR(dwError);

error:
    if (dwError)
    {
        if (pContext->pRequest)
         {
            VmKdcFreeRequest(&pContext->pRequest);
         }
    }
    else
    {
        pContext->pRequest->requestAllocLen = krbMsgLen;
        pContext->pRequest->requestBuf = requestBuf;
    }
    return dwError;
}

DWORD
VmKdcSendTcp(
    int sock,
    unsigned char *msg,
    int msgLen)
{
    DWORD dwError = 0;
    int sts = 0;
    int len = 0;
    int slen = 0;

    slen = msgLen;
    do
    {
        sts = send(sock, &msg[len], slen, 0);
        if (sts > 0)
        {
            slen -= sts;
            len += sts;
        }
        else
        {
#ifdef _WIN32
            errno = WSAGetLastError();
#endif
            dwError =  errno;
            BAIL_ON_VMKDC_ERROR(dwError);
        }
    } while (sts > 0 && slen < msgLen);

error:
    return dwError;
}

static DWORD
VmKdcRecvTcp(
    int sock,
    unsigned char *msg,
    int msgLen)
{
    DWORD dwError = 0;
    int sts = 0;
    int len = 0;
    int rlen = 0;

    len = msgLen;

    // pGlobals is useful for timeout information when
    // this is implemented to use select() with a timeout.
    do
    {
        /* TBD: Need select() around recv() to prevent DoS timeout attack */
        sts = recv(sock, &msg[rlen], len, 0);
        if (sts > 0)
        {
            rlen += sts;
            len -= sts;
        }
        else
        {
#ifdef _WIN32
            errno = WSAGetLastError();
#endif
            dwError =  errno;
            if (sts == 0)
            {
                VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcRecvTcp: peer disconnection detected.");
                dwError = ERROR_PROTOCOL;
            }
            BAIL_ON_VMKDC_ERROR(dwError);
        }
    } while (sts > 0 && len > 0);
error:

    return dwError;
}


static DWORD
VmKdcReadTcpRequest(
    PVMKDC_CONTEXT pContext)
{
    DWORD dwError = 0;

    dwError = VmKdcRecvTcp(
                  pContext->pRequest->requestSocket,
                  pContext->pRequest->requestBuf,
                  pContext->pRequest->requestAllocLen);
    BAIL_ON_VMKDC_ERROR(dwError);
    pContext->pRequest->requestBufLen = pContext->pRequest->requestAllocLen;

error:
    return dwError;
}

static DWORD
VmKdcSendTcpResponse(
    PVMKDC_CONTEXT pContext,
    PVMKDC_DATA krbMsg)
{
    DWORD dwError = 0;
    PUCHAR krbMsgPtr = NULL;
    DWORD krbMsgLen = 0;
    UINT32 netLen = 0;
    UCHAR netLenBuf[sizeof(netLen)] = {0};

    krbMsgLen = VMKDC_GET_LEN_DATA(krbMsg);
    krbMsgPtr = VMKDC_GET_PTR_DATA(krbMsg);

    netLen = htonl(krbMsgLen);
    memcpy(netLenBuf, &netLen, sizeof(netLen));

    dwError = VmKdcSendTcp(
                  pContext->pRequest->requestSocket,
                  netLenBuf,
                  sizeof(netLenBuf));
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcSendTcp(pContext->pRequest->requestSocket, krbMsgPtr, krbMsgLen);
    BAIL_ON_VMKDC_ERROR(dwError);
    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL,
             "VmKdcListenKdcService: sent %d bytes", krbMsgLen);

error:
    return dwError;
}

DWORD
VmKdcProcessUdpRequest(PVMKDC_CONTEXT pContext)
{
    DWORD dwError = 0;
    int len = 0;
    PVMKDC_DATA krbMsg = NULL;

    dwError = VmKdcProcessKdcReq(pContext, &krbMsg);
    BAIL_ON_VMKDC_ERROR(dwError);

    len = sendto(pContext->pRequest->requestSocket,
                 VMKDC_GET_PTR_DATA(krbMsg),
                 VMKDC_GET_LEN_DATA(krbMsg),
                 0,
                 (struct sockaddr *) pContext->pRequest->pvClientAddr,
                 pContext->pRequest->dwClientAddrLen);
    if (len <= 0)
    {
        dwError = ERROR_PROTOCOL;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

error:
    VmKdcSignalThreadDone(pContext);
    VmKdcFreeData(krbMsg);
    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcProcessUdpRequest: finished");

    //Each UDP accept() call will not create a new socket (file descriptor),
    // so there is no socket leak here.
    pContext->pRequest->requestSocket = -1;
    VmKdcFreeContext(&pContext);

    /* Running as a detached thread; no one really cares about return status */
    return dwError;
}


DWORD
VmKdcProcessTcpRequest(PVMKDC_CONTEXT pContext)
{
    DWORD dwError = 0;
    PVMKDC_DATA krbMsg = NULL;

    dwError = VmKdcReadTcpRequestSize(pContext);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcReadTcpRequest(pContext);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcProcessKdcReq(pContext, &krbMsg);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcSendTcpResponse(
                  pContext,
                  krbMsg);
    BAIL_ON_VMKDC_ERROR(dwError);

error:
    VmKdcSignalThreadDone(pContext);
    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcProcessTcpRequest: done");
    VmKdcFreeContext(&pContext);
    VmKdcFreeData(krbMsg);

    /* Running as a detached thread; no one really cares about return status */
    return dwError;
}

PVOID
VmKdcProcessThread(PVOID pVoidContext)
{
    DWORD dwError = 0;
    PVMKDC_CONTEXT pContext = (PVMKDC_CONTEXT) pVoidContext;

    if (pContext->pRequest->bRequestIsUdp)
    {
        VmKdcProcessUdpRequest(pContext);
    }
    else
    {
        VmKdcProcessTcpRequest(pContext);
    }

    /* Running as a detached thread; no one really cares about return status */
    return dwError ? /* error pointer */ NULL : NULL;
}

static DWORD
VmKdcReadUdpRequest(PVMKDC_CONTEXT pContext)
{
    DWORD dwError = 0;
    int len = 0;

    len = recvfrom(pContext->pRequest->requestSocket,
                   pContext->pRequest->requestBuf,
                   VMKDC_UDP_READ_BUFSIZ,
                   0,
                   (struct sockaddr *) pContext->pRequest->pvClientAddr,
                   (socklen_t *) &pContext->pRequest->dwClientAddrLen);
    if (len <= 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }
    pContext->pRequest->requestBufLen = len;

error:
    return dwError;
}


static
PVOID
VmKdcListenKdcService(
    PVOID pInfo
    )
{
    PVMKDC_CONTEXT pContext = NULL;
    PVMKDC_GLOBALS pGlobals = (PVMKDC_GLOBALS) pInfo;
    DWORD dwError = 0;
    int requestSocket = -1;
    int requestSocketUdp = -1;

    do
    {
        requestSocket = -1;
        requestSocketUdp = -1;
        dwError = VmKdcSrvServiceAcceptConn(
                      pGlobals,
                      &requestSocket,
                      &requestSocketUdp);
        if (VmKdcdState() == VMKDC_SHUTDOWN)
        {
            break;
        }
        if (requestSocket >= 0 || requestSocketUdp >= 0)
        {
            VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "vmkdc: accepted connection!");
        }
        else
        {
            continue;
        }
        if (requestSocketUdp != -1)
        {
            /* Process UDP request here */
            dwError = VmKdcAllocateContext(
                          requestSocketUdp,
                          TRUE,
                          pGlobals,
                          &pContext);
            BAIL_ON_VMKDC_ERROR(dwError);

            dwError = VmKdcReadUdpRequest(pContext);
            if (dwError)
            {
                pContext->pRequest->requestSocket = -1;
                VmKdcFreeContext(&pContext);
            } else
            {
                /* TBD: Use thread pool here */
                dwError = VmKdcCreateThreadMaxLimit(
                          pContext,
                          VmKdcProcessThread);
                BAIL_ON_VMKDC_ERROR(dwError);
            }
        }

        if (requestSocket != -1)
        {
            /* Process TCP request here */
            dwError = VmKdcAllocateContext(
                          requestSocket,
                          FALSE,
                          pGlobals,
                          &pContext);
            BAIL_ON_VMKDC_ERROR(dwError);

            /* TBD: Use thread pool here */
            dwError = VmKdcCreateThreadMaxLimit(
                          pContext,
                          VmKdcProcessThread);
            BAIL_ON_VMKDC_ERROR(dwError);
        }
    } while ((requestSocket >= 0 || requestSocketUdp >= 0) && VmKdcdState() != VMKDC_SHUTDOWN);
error:
    return NULL;
}


DWORD
VmKdcInitConnAcceptThread(
    PVMKDC_GLOBALS pGlobals)
{
    DWORD  dwError = 0;
    int    sts = 0;

    sts = pthread_create(
            &pGlobals->thread,
            NULL,
            VmKdcListenKdcService,
            pGlobals);
    if (sts)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }
    BAIL_ON_VMKDC_ERROR(dwError);
error:

    return dwError;
}


void
VmKdcSrvCloseSocketAcceptFd(
    VOID
    )
{
    pthread_mutex_lock(&gVmkdcGlobals.mutex);

    if (gVmkdcGlobals.iAcceptSock >= 0)
    {
        tcp_close(gVmkdcGlobals.iAcceptSock);
        gVmkdcGlobals.iAcceptSock = -1;
    }

    if (gVmkdcGlobals.iAcceptSockUdp >= 0)
    {
        tcp_close(gVmkdcGlobals.iAcceptSockUdp);
        gVmkdcGlobals.iAcceptSockUdp = -1;
    }

    if (gVmkdcGlobals.iAcceptSock6 >= 0)
    {
        tcp_close(gVmkdcGlobals.iAcceptSock6);
        gVmkdcGlobals.iAcceptSock6 = -1;
    }

    if (gVmkdcGlobals.iAcceptSock6Udp >= 0)
    {
        tcp_close(gVmkdcGlobals.iAcceptSock6Udp);
        gVmkdcGlobals.iAcceptSock6Udp = -1;
    }

    pthread_mutex_unlock(&gVmkdcGlobals.mutex);
}


void
VmKdcSrvIncrementThreadCount(
    PVMKDC_GLOBALS pGlobals)
{
    pthread_mutex_lock(&gVmkdcGlobals.mutex);
    pGlobals->workerThreadCount++;
    pthread_mutex_unlock(&gVmkdcGlobals.mutex);
}

void
VmKdcSrvDecrementThreadCount(
    PVMKDC_GLOBALS pGlobals)
{
    pthread_mutex_lock(&gVmkdcGlobals.mutex);
    pGlobals->workerThreadCount--;
    pthread_mutex_unlock(&gVmkdcGlobals.mutex);
}


void
VmKdcSrvGetThreadCount(
    PVMKDC_GLOBALS pGlobals,
    PDWORD pWorkerThreadCount)
{
    pthread_mutex_lock(&gVmkdcGlobals.mutex);
    *pWorkerThreadCount = pGlobals->workerThreadCount;
    pthread_mutex_unlock(&gVmkdcGlobals.mutex);
}
