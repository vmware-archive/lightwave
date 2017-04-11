/*
 * Copyright (C) 2016 VMware, Inc. All rights reserved.
 *
 * Module   : sourceip.c
 *
 * Abstract :
 *
 */

 #include "includes.h"

static
PVOID
VmAfdSourceIpWorker(
    PVOID pData
    );

VOID
VmAfdShutdownSrcIpThread(
    PSOURCE_IP_CONTEXT pSourceIpContext
    );

DWORD
VmAfdInitSourceIpThread(
    PSOURCE_IP_CONTEXT* ppSourceIpContext
    )
{
    DWORD dwError = 0;
    DWORD sourceIpSocket = 0;
    PSOURCE_IP_CONTEXT pSourceIpContext = NULL;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE;

    if(!ppSourceIpContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfSrvGetDomainState(&domainState);
    if(dwError || domainState != VMAFD_DOMAIN_STATE_CONTROLLER)
    {
       VmAfdLog(VMAFD_DEBUG_ANY, "Source IP thread nt started");
       return 0;
    }

    dwError = VmAfdAllocateMemory(
                      sizeof(SOURCE_IP_CONTEXT),
                      (PVOID *)&pSourceIpContext
                      );
    BAIL_ON_VMAFD_ERROR(dwError);

    sourceIpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sourceIpSocket < 0)
    {
  #ifndef _WIN32
        dwError = LwErrnoToWin32Error(errno);
  #endif
        BAIL_ON_VMAFD_ERROR(dwError);
    }

  #ifndef _WIN32
    dwError = pthread_create(
                    &pSourceIpContext->srcIpThread,
                    NULL,
                    &VmAfdSourceIpWorker,
                    (PVOID)pSourceIpContext
                    );
    if(dwError)
    {
        dwError = LwErrnoToWin32Error(dwError);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
  #endif

    *ppSourceIpContext = pSourceIpContext;

cleanup:

    return dwError;

error:

    if(pSourceIpContext)
    {
        VmAfdShutdownSrcIpThread(pSourceIpContext);
    }
    if(ppSourceIpContext)
    {
        *ppSourceIpContext = NULL;
    }
    goto cleanup;
}

VOID
VmAfdShutdownSrcIpThread(
    PSOURCE_IP_CONTEXT pSourceIpContext
    )
 {
    DWORD dwError = 0;
    DWORD socketShutdown = 0;
    struct addrinfo hints = {0};
    struct addrinfo *serverAddr = NULL;
    char message = '0';

    if(!pSourceIpContext)
    {
        VmAfdLog(VMAFD_DEBUG_ANY, "Shutdown Failed");
        return;
    }

  #ifndef _WIN32
    dwError = close(pSourceIpContext->sourceIpSocket);
    if(dwError < 0)
    {
       dwError = LwErrnoToWin32Error(errno);
       BAIL_ON_VMAFD_ERROR(dwError);
    }
  #endif

    socketShutdown = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(socketShutdown < 0)
    {
  #ifndef _WIN32
       dwError = LwErrnoToWin32Error(errno);
  #endif
       BAIL_ON_VMAFD_ERROR(dwError);
    }
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    dwError = getaddrinfo(
                   NULL,
                   VMDNS_SOURCEIP_UDP_PORT,
                   &hints,
                   &serverAddr
                   );
    if(dwError < 0)
    {
  #ifndef _WIN32
       dwError = LwErrnoToWin32Error(errno);
  #endif
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = sendto(
                 socketShutdown,
                 (PVOID)&message,
                 sizeof(message),
                 0,
                 serverAddr->ai_addr,
                 serverAddr->ai_addrlen
                 );
    if(dwError < 0)
    {
  #ifndef _WIN32
         dwError = LwErrnoToWin32Error(errno);
  #endif
         BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = pthread_join(pSourceIpContext->srcIpThread, NULL);
    if(dwError)
    {
  #ifndef _WIN32
       dwError = LwErrnoToWin32Error(dwError);
  #endif
       BAIL_ON_VMAFD_ERROR(dwError);
   }

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pSourceIpContext);
    VMAFD_SAFE_FREE_MEMORY(serverAddr);
    return;

error:

   VmAfdLog(VMAFD_DEBUG_ERROR, "Source IP shutdown failed!(%d)", dwError);
   goto cleanup;

}

static
PVOID
VmAfdSourceIpWorker(
    PVOID pData
)
{
    DWORD dwError = 0;
    DWORD clientAddrLen = 0;
    DWORD messageLen = 0;
    DWORD messageType = 0;
    PVOID pVoid  = NULL;
    struct addrinfo hints = {0};
    struct addrinfo *serverAddr = NULL;
    struct sockaddr clientAddr = {0};
    char message = '0';
    char buffer[VMDNS_IP6_ADDRESS_SIZE] = {0};
    PSOURCE_IP_CONTEXT pSourceIpContext = (PSOURCE_IP_CONTEXT)pData;


    if(!pData)
    {
        VmAfdLog(VMAFD_DEBUG_ERROR, "Source Ip context invalid. Thread failure!");
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    dwError = getaddrinfo(NULL, VMDNS_SOURCEIP_UDP_PORT, &hints, &serverAddr);
    if(dwError)
    {
  #ifndef _WIN32
        dwError = LwErrnoToWin32Error(dwError);
  #endif
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = bind(
                pSourceIpContext->sourceIpSocket,
                serverAddr->ai_addr,
                serverAddr->ai_addrlen
                );
    if(dwError < 0)
    {
  #ifndef _WIN32
        dwError = LwErrnoToWin32Error(errno);
  #endif
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    while(1)
    {
        messageLen = recvfrom(
                      pSourceIpContext->sourceIpSocket,
                      (PVOID)&message,
                      sizeof(message),
                      0,
                      (struct sockaddr *)&clientAddr,
                      &clientAddrLen
                    );
        if(messageLen < 0)
        {
            VmAfdLog(VMAFD_DEBUG_ANY, "Recieve failed");
            continue;
        }
        messageType = (DWORD)message;

        if(!messageType)
        {
            VmAfdLog(VMAFD_DEBUG_ANY, "Terminating Source IP thread");
            break;
        }

        if(clientAddr.sa_family == AF_INET)
        {
            messageLen = sizeof(VMDNS_IP4_ADDRESS);
            pVoid = (PVOID)inet_ntop(AF_INET, &clientAddr.sa_data, buffer, messageLen);
            if(!pVoid)
            {
          #ifndef _WIN32
                dwError = LwErrnoToWin32Error(errno);
          #endif
                BAIL_ON_VMAFD_ERROR(dwError);
            }
        }
        else
        {
            messageLen = sizeof(VMDNS_IP6_ADDRESS);
            pVoid = (PVOID)inet_ntop(AF_INET6, &clientAddr.sa_data, buffer, messageLen);
            if(!pVoid)
            {
          #ifndef _WIN32
                dwError = LwErrnoToWin32Error(errno);
          #endif
                BAIL_ON_VMAFD_ERROR(dwError);
            }
        }
        dwError = sendto(
                    pSourceIpContext->sourceIpSocket,
                    buffer,
                    messageLen,
                    0,
                    (struct sockaddr *)&clientAddr,
                    sizeof(clientAddr)
                    );
        if(dwError < 0)
        {
            VmAfdLog(VMAFD_DEBUG_ANY, "Sendto failed");
        }
    }

cleanup:
    return NULL;

error:
    goto cleanup;
}
