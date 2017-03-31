/*
 * Copyright (C) 2016 VMware, Inc. All rights reserved.
 *
 * Module   : ddns.c
 *
 * Abstract :
 *
 */
#include "includes.h"

#ifndef _WIN32
typedef struct sockaddr_nl nl_addr;
#endif

static
DWORD
VmDdnsGetMachineInfo(
    PSTR *ppszDomain,
    PSTR *ppszHostname,
    PSTR *ppszMachineName,
    PVMDNS_SERVER_CONTEXT *ppServerContext
    );

#ifndef _WIN32
static
PVOID
VmDdnsUpdateWorker(
    PVOID pData
    );
#endif

static
DWORD
VmDdnsUpdate(
    PDDNS_CONTEXT pDdnsContext
    );

static
DWORD
VmDdnsDelete(
    PDDNS_CONTEXT pDdnsContext
    );

static
DWORD
VmDdnsRpcUpdate(
    PSTR pszZone,
    PSTR pszHostname,
    PSTR pszName,
    PVMDNS_SERVER_CONTEXT pServerContext
    );

static
DWORD
VmDdnsProtocolUpdate(
    PSTR pszDomain,
    PSTR pszHostname,
    PSTR pszMachineName,
    PVMDNS_SERVER_CONTEXT pServerContext,
    PDDNS_CONTEXT pDdnsContext,
    DWORD dwFlag
    );

static
DWORD
VmDdnsRpcDelete(
    PVMDNS_SERVER_CONTEXT pServerContext,
    PSTR pszZone,
    PSTR pszName
    );

VOID
VmDdnsShutdown(
    PDDNS_CONTEXT pDdnsContext
    );

DWORD
VmDdnsInitThread(
    PDDNS_CONTEXT *ppDdnsContext
    )
{
    DWORD dwError = 0;
    DWORD netLinkFd = 0;
    DWORD enableDns = 0;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE;
    PDDNS_CONTEXT pDdnsContext = NULL;

    if(!ppDdnsContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfSrvGetDomainState(&domainState);

    if(domainState != VMAFD_DOMAIN_STATE_CLIENT || dwError)
    {
        VmAfdLog(VMAFD_DEBUG_ANY, "DDNS Client not started. Domain State invalid");
        return 0;
    }

    dwError = VmAfdAllocateMemory(
                        sizeof(DDNS_CONTEXT),
                        (PVOID *)&pDdnsContext
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRegGetInteger(
                      VMAFD_REG_KEY_ENABLE_DNS,
                      &enableDns
                      );
    if(dwError)
    {
        enableDns = 0;
        dwError = 0;
    }
    pDdnsContext->bIsEnabledDnsUpdates = enableDns;

    dwError = pthread_mutex_init(&pDdnsContext->ddnsMutex, NULL);
    if(dwError)
    {
  #ifndef _WIN32
        dwError = LwErrnoToWin32Error(dwError);
  #endif
        BAIL_ON_VMAFD_ERROR(dwError);
    }

  #ifndef _WIN32
    dwError = pipe(pDdnsContext->pipeFd);
    if(dwError < 0)
    {

        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    netLinkFd = socket(
                AF_NETLINK,
                SOCK_RAW,
                NETLINK_ROUTE);
    if(netLinkFd < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
  #endif
    pDdnsContext->netLinkFd = netLinkFd;
    pDdnsContext->idSeed = VMDDNS_ID_SEED;

    //first update
    dwError = VmDdnsUpdate(pDdnsContext);
    if(dwError)
    {
        VmAfdLog(VMAFD_DEBUG_DEBUG, "First update failed!");
    }

  #ifndef _WIN32
    dwError = pthread_create(
                  &pDdnsContext->thread,
                  NULL,
                  &VmDdnsUpdateWorker,
                  (PVOID)pDdnsContext
                  );
    if(dwError)
    {
        dwError = LwErrnoToWin32Error(dwError);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
  #endif

    VmAfdLog(VMAFD_DEBUG_ANY, "Started DDNS client Thread successfully");
    *ppDdnsContext = pDdnsContext;

cleanup:

    return dwError;

error:
    if(pDdnsContext)
    {
        VmDdnsShutdown(pDdnsContext);
    }
    if(ppDdnsContext)
    {
        *ppDdnsContext = NULL;
    }
    goto cleanup;
}

VOID
VmDdnsShutdown(
    PDDNS_CONTEXT pDdnsContext
    )
{
    DWORD dwError = 0;
    char notifyDdns = 0;

    VmAfdLog(VMAFD_DEBUG_ANY, "Shutting down DDNS Client service");

    if(!pDdnsContext)
    {
        VmAfdLog(VMAFD_DEBUG_ERROR, "Context invalid! Shutdown failed!");
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

  #ifndef _WIN32
    dwError = write(pDdnsContext->pipeFd[1], (PVOID *)&notifyDdns, sizeof(notifyDdns));
    if(dwError < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        VmAfdLog(
            VMAFD_DEBUG_ERROR,
            "Write socket failed: %d",
            dwError
            );
    }
  #endif

    dwError = pthread_join(pDdnsContext->thread, NULL);
    if(dwError)
    {
  #ifndef _WIN32
        dwError = LwErrnoToWin32Error(dwError);
  #endif
        VmAfdLog(
            VMAFD_DEBUG_ANY,
            "DDNS join failed. Error [%d]",
            dwError
            );
    }

  #ifndef _WIN32
    dwError = close(pDdnsContext->pipeFd[0]);
    if(dwError < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        VmAfdLog(
            VMAFD_DEBUG_ANY,
            "Pipe close failed. Error [%d]",
            dwError
            );
    }

    dwError = close(pDdnsContext->pipeFd[1]);
    if(dwError < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        VmAfdLog(
            VMAFD_DEBUG_ANY,
            "Pipe[1] close failed. Error [%d]",
            dwError
            );
    }

    dwError = close(pDdnsContext->netLinkFd);
    if(dwError < 0)
    {
        dwError = LwErrnoToWin32Error(errno);

        VmAfdLog(
            VMAFD_DEBUG_ANY,
            "Netlink close failed. Error [%d]",
            dwError
            );
    }
  #endif

    dwError = pthread_mutex_destroy(&pDdnsContext->ddnsMutex);
    if(dwError)
    {
      #ifndef _WIN32
        dwError = LwErrnoToWin32Error(dwError);
      #endif
        VmAfdLog(
            VMAFD_DEBUG_ANY,
            "Pthread mutex failed. Error [%d]",
            dwError
            );
    }
cleanup:

    VMAFD_SAFE_FREE_MEMORY(pDdnsContext);
    return;

error:

    goto cleanup;
}

VOID
VmDdnsExit(
    PDDNS_CONTEXT pDdnsContext
)
{
    DWORD dwError = 0;

    if(pDdnsContext)
    {
        dwError = VmDdnsDelete(pDdnsContext);
        if(dwError)
        {
            VmAfdLog(VMAFD_DEBUG_ANY, "Ddns delete failed");
        }
        VmDdnsShutdown(pDdnsContext);
    }
}

DWORD
VmDdnsGetSourceIp(
      VMDNS_IP4_ADDRESS** ppSourceIp4,
      VMDNS_IP6_ADDRESS** ppSourceIp6
      )
{
    DWORD dwError = 0;
    DWORD srcIpClientSocket = 0;
    DWORD recvLen = 0;
    DWORD socketLen = 0;
    struct addrinfo hints = {0};
    struct addrinfo *serverAddr = NULL;
    char message = '1';
    char recvBuff[VMDNS_IP6_ADDRESS_SIZE] = {0};
    VMDNS_IP6_ADDRESS* pSourceIp6 = NULL;
    VMDNS_IP4_ADDRESS* pSourceIp4 = NULL;
    struct timeval recvTimeOut = {0};
    PSTR pDCName = NULL;
    PWSTR pwDCName = NULL;

    dwError = VmAfSrvGetDCName(&pwDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwDCName, &pDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    dwError = getaddrinfo(
                    pDCName,
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

    srcIpClientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    recvTimeOut.tv_sec = VMDDNS_RECV_TIMEOUT;
    recvTimeOut.tv_usec = 0;

    dwError = setsockopt(
                    srcIpClientSocket,
                    SOL_SOCKET,
                    SO_RCVTIMEO,
                    (PSTR)&recvTimeOut,
                    sizeof(struct timeval)
                    );
    if(dwError < 0)
    {
  #ifndef _WIN32
        dwError = LwErrnoToWin32Error(errno);
  #endif
        VmAfdLog(VMAFD_DEBUG_ANY, "Setsockopt failed!");
    }

  #ifndef _WIN32
    dwError = sendto(
                  srcIpClientSocket,
                  &message,
                  sizeof(message),
                  0,
                  serverAddr->ai_addr,
                  serverAddr->ai_addrlen
                  );
    if(dwError < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    recvLen = recvfrom(
                  srcIpClientSocket,
                  (PCVOID *)recvBuff,
                  VMDNS_IP6_ADDRESS_SIZE,
                  0,
                  (struct sockaddr *)&serverAddr->ai_addr,
                  &socketLen
                  );
    if(recvLen < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
  #endif

    if(recvLen == VMDNS_IP4_ADDRESS_SIZE)
    {
        dwError = VmAfdAllocateMemory(
                          VMDNS_IP4_ADDRESS_SIZE,
                          (PVOID *)&pSourceIp4
                          );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdCopyMemory(
                        pSourceIp4,
                        VMDNS_IP4_ADDRESS_SIZE,
                        recvBuff,
                        recvLen
                        );
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdAllocateMemory(
                          VMDNS_IP6_ADDRESS_SIZE,
                          (PVOID *)&pSourceIp6
                          );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdCopyMemory(
                        pSourceIp6,
                        VMDNS_IP6_ADDRESS_SIZE,
                        recvBuff,
                        recvLen
                        );
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    *ppSourceIp6 = pSourceIp6;
    *ppSourceIp4 = pSourceIp4;

cleanup:
    VMAFD_SAFE_FREE_MEMORY(serverAddr);
    return dwError;

error:
    VMAFD_SAFE_FREE_MEMORY(pSourceIp4);
    VMAFD_SAFE_FREE_MEMORY(pSourceIp6);
    if(ppSourceIp4)
    {
        *ppSourceIp4 = NULL;
    }
    if(ppSourceIp6)
    {
        *ppSourceIp6 = NULL;
    }
    goto cleanup;
}

// Only for LINUX
#ifndef _WIN32
static
PVOID
VmDdnsUpdateWorker(
    PVOID pdata
    )
{
    DWORD len = 0;
    DWORD maxFd = 0;
    DWORD dwError = 0;
    char buffer[VMDDNS_BUFFER_SIZE] = {0};
    PDDNS_CONTEXT pDdnsContext = (PDDNS_CONTEXT)pdata;
    nl_addr *bindAddr = NULL;
    struct nlmsghdr *nh = NULL;
    fd_set readFs;

    dwError = VmAfdAllocateMemory(
                      sizeof(nl_addr),
                      (PVOID *)&bindAddr
                      );
    BAIL_ON_VMAFD_ERROR(dwError);

    bindAddr->nl_family = AF_NETLINK;
    bindAddr->nl_pad = 0;
    bindAddr->nl_pid = getpid();
    bindAddr->nl_groups = RTMGRP_IPV6_IFADDR | RTMGRP_IPV4_IFADDR;

    //bind to the IFADDR API
    dwError = bind(
                pDdnsContext->netLinkFd,
                (struct sockaddr*)bindAddr,
                sizeof(nl_addr)
                );
    if (dwError < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    nh = (struct nlmsghdr *)buffer;
    maxFd = (pDdnsContext->netLinkFd > pDdnsContext->pipeFd[0]) ?
            pDdnsContext->netLinkFd
            : pDdnsContext->pipeFd[0];
    while(1)
    {
        FD_ZERO(&readFs);
        FD_SET(pDdnsContext->netLinkFd, &readFs);
        FD_SET(pDdnsContext->pipeFd[0], &readFs);
        dwError = select(maxFd + 1, &readFs, NULL, NULL, NULL);
        if (dwError < 0)
        {
             dwError = LwErrnoToWin32Error(errno);
             VmAfdLog(VMAFD_DEBUG_ANY, "Select failed. Error[%d]", dwError);
             continue;
        }

        if(FD_ISSET(pDdnsContext->netLinkFd, &readFs))
        {
            len = recv(pDdnsContext->netLinkFd, nh, 4096, 0);
            if(len < 0)
            {
                dwError = LwErrnoToWin32Error(errno);
                VmAfdLog(VMAFD_DEBUG_ANY, "Reciev failed. Error[%d]", dwError);
                continue;
            }

            for(; (NLMSG_OK (nh, len)) && (nh->nlmsg_type != NLMSG_DONE); nh = NLMSG_NEXT(nh, len))
            {
                if (nh->nlmsg_type != RTM_NEWADDR)
                {
                    continue; /* some other kind of message */
                }
                // Update detected
                dwError = VmDdnsUpdate(pDdnsContext);
                if(dwError)
                {
                    VmAfdLog(VMAFD_DEBUG_ANY, "DDNS Update failed");
                }
            }
            memset(buffer, 0, VMDDNS_BUFFER_SIZE);
        }
        else if(FD_ISSET(pDdnsContext->pipeFd[0], &readFs))
        {
            VmAfdLog(
                VMAFD_DEBUG_ANY,
                "Recieved Terminate. DDNS Client exiting.");
            break;
        }
        else
        {
            VmAfdLog(
                VMAFD_DEBUG_ANY,
                "Invalid file descriptor set.");
        }
    }

cleanup:

    VMAFD_SAFE_FREE_MEMORY(bindAddr);
    VmAfdLog(VMAFD_DEBUG_ANY, "DDNS Update Worker exiting. Error[%d]", dwError);
    return NULL;

error:

    goto cleanup;
}
#endif

static
DWORD
VmDdnsGetMachineInfo(
    PSTR *ppszDomain,
    PSTR *ppszHostname,
    PSTR *ppszMachineName,
    PVMDNS_SERVER_CONTEXT *ppServerContext
    )
{
    DWORD dwError = 0;
    PVMAFD_REG_ARG pArgs = NULL;
    PSTR pszDCName = NULL;
    PSTR pszDomain = NULL;
    PSTR pszHostname = NULL;
    PSTR pszMachineName = NULL;
    PWSTR pwszDCName = NULL;
    PVMDNS_SERVER_CONTEXT pServerContext = NULL;

    if(!ppszDomain || !ppszHostname || !ppszMachineName || !ppServerContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetMachineInfo(&pArgs);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetAffinitizedDC(&pwszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszDCName, &pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdGetHostName(&pszHostname);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdGetCanonicalHostName(pszHostname, &pszMachineName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmDnsOpenServerWithTimeOutA(
                        pszDCName,
                        pArgs->pszAccount,
                        pArgs->pszDomain,
                        pArgs->pszPassword,
                        0,
                        NULL,
                        RPC_PING_TIMEOUT,
                        &pServerContext
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringA(pArgs->pszDomain, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszDomain = pszDomain;
    *ppszHostname = pszHostname;
    *ppszMachineName = pszMachineName;
    *ppServerContext = pServerContext;

  cleanup:

    if (pArgs)
    {
        VmAfdFreeRegArgs(pArgs);
    }
    return dwError;

  error:

    VMAFD_SAFE_FREE_STRINGA(pszHostname);
    VMAFD_SAFE_FREE_STRINGA(pszMachineName);
    VMAFD_SAFE_FREE_STRINGA(pszDCName);
    VMAFD_SAFE_FREE_STRINGA(pszDomain);
    VMAFD_SAFE_FREE_STRINGW(pwszDCName);
    if(pServerContext)
    {
        VmDnsCloseServer(pServerContext);
    }
    if(ppszHostname)
    {
        *ppszHostname = NULL;
    }
    if(ppszDomain)
    {
        *ppszDomain = NULL;
    }
    if(ppszMachineName)
    {
        *ppszMachineName = NULL;
    }
    goto cleanup;
}

static
DWORD
VmDdnsUpdate(
    PDDNS_CONTEXT pDdnsContext
    )
{
    DWORD dwError = 0;
    BOOL mutexIsLocked = FALSE;
    PSTR pszDomain = NULL;
    PSTR pszHostname = NULL;
    PSTR pszMachineName = NULL;
    PVMDNS_SERVER_CONTEXT pServerContext = NULL;

    if(!pDdnsContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmDdnsGetMachineInfo(
                      &pszDomain,
                      &pszHostname,
                      &pszMachineName,
                      &pServerContext
                      );
    BAIL_ON_VMAFD_ERROR(dwError);

    VMAFD_LOCK_MUTEX(mutexIsLocked, &pDdnsContext->ddnsMutex);

    if(pDdnsContext->bIsEnabledDnsUpdates)
    {
        dwError = VmDdnsProtocolUpdate(
                            pszDomain,
                            pszHostname,
                            pszMachineName,
                            pServerContext,
                            pDdnsContext,
                            VMDDNS_UPDATE_PACKET
                            );
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmDdnsRpcUpdate(
                          pszDomain,
                          pszHostname,
                          pszMachineName,
                          pServerContext
                          );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    VMAFD_UNLOCK_MUTEX(mutexIsLocked, &pDdnsContext->ddnsMutex);
    if(pServerContext)
    {
        VmDnsCloseServer(pServerContext);
    }
    VMAFD_SAFE_FREE_STRINGA(pszDomain);
    VMAFD_SAFE_FREE_STRINGA(pszHostname);
    VMAFD_SAFE_FREE_STRINGA(pszMachineName);
    return dwError;

error:
    VmAfdLog(VMAFD_DEBUG_ANY, "Dns Update failed!");
    goto cleanup;

}

static
DWORD
VmDdnsDelete(
    PDDNS_CONTEXT pDdnsContext
    )
{
    DWORD dwError = 0;
    BOOL mutexIsLocked = FALSE;
    PSTR pszDomain = NULL;
    PSTR pszHostname = NULL;
    PSTR pszMachineName = NULL;
    PVMDNS_SERVER_CONTEXT pServerContext = NULL;

    if(!pDdnsContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmDdnsGetMachineInfo(
                      &pszDomain,
                      &pszHostname,
                      &pszMachineName,
                      &pServerContext
                      );
    BAIL_ON_VMAFD_ERROR(dwError);

    VMAFD_LOCK_MUTEX(mutexIsLocked, &pDdnsContext->ddnsMutex);

    if(pDdnsContext->bIsEnabledDnsUpdates)
    {
        dwError = VmDdnsProtocolUpdate(
                            pszDomain,
                            pszHostname,
                            pszMachineName,
                            pServerContext,
                            pDdnsContext,
                            VMDDNS_DELETE_PACKET
                            );
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmDdnsRpcDelete(
                        pServerContext,
                        pszDomain,
                        pszMachineName
                        );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    VMAFD_UNLOCK_MUTEX(mutexIsLocked, &pDdnsContext->ddnsMutex);
    VMAFD_SAFE_FREE_STRINGA(pszDomain);
    VMAFD_SAFE_FREE_STRINGA(pszHostname);
    VMAFD_SAFE_FREE_STRINGA(pszMachineName);
    if(pServerContext)
    {
        VmDnsCloseServer(pServerContext);
    }
    return dwError;

error:

    goto cleanup;

}

static
DWORD
VmDdnsProtocolUpdate(
    PSTR pszDomain,
    PSTR pszHostname,
    PSTR pszMachineName,
    PVMDNS_SERVER_CONTEXT pServerContext,
    PDDNS_CONTEXT pDdnsContext,
    DWORD dwFlag
    )
{
    DWORD dwError = 0;
    DWORD udpSocket = 0;
    DWORD packetSize = 0;
    DWORD socketLen = 0;
    PSTR pDnsPacket = NULL;
    PSTR pDCName = NULL;
    PWSTR pwDCName = NULL;
    PDDNS_UPDATE_HEADER pHeader = NULL;
    PSTR pDnsPort = VMDNS_SERVER_PORT;
    struct timeval recvTimeOut = {0};
    struct addrinfo *serverAddr = NULL;
    struct addrinfo hints = {0};

    if(!pszDomain || !pszMachineName || !pszHostname || !pServerContext || !pDdnsContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfSrvGetDCName(&pwDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwDCName, &pDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    dwError = getaddrinfo(
                    pDCName,
                    pDnsPort,
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

    udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    recvTimeOut.tv_sec = VMDDNS_RECV_TIMEOUT;
    recvTimeOut.tv_usec = 0;

    dwError = setsockopt(
                    udpSocket,
                    SOL_SOCKET,
                    SO_RCVTIMEO,
                    (PSTR)&recvTimeOut,
                    sizeof(struct timeval)
                    );
    if(dwError < 0)
    {
      #ifndef _WIN32
        dwError = LwErrnoToWin32Error(errno);
      #endif
        VmAfdLog(VMAFD_DEBUG_ANY, "Setsockopt failed!");
    }

    dwError = VmDdnsUpdateMakePacket(
                        pszDomain,
                        pszHostname,
                        pszMachineName,
                        &pDnsPacket,
                        &packetSize,
                        pDdnsContext->idSeed++,
                        dwFlag
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

  #ifndef _WIN32
    dwError = sendto(
                  udpSocket,
                  (PCVOID *)pDnsPacket,
                  packetSize,
                  0,
                  serverAddr->ai_addr,
                  serverAddr->ai_addrlen
                  );
    if(dwError < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    VMAFD_SAFE_FREE_STRINGA(pDnsPacket);

    dwError = recvfrom(
                  udpSocket,
                  (PCVOID *)pDnsPacket,
                  packetSize,
                  0,
                  (struct sockaddr *)&serverAddr->ai_addr,
                  &socketLen
                  );
    if(dwError < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
  #endif

    pHeader = (PDDNS_UPDATE_HEADER)pDnsPacket;
    dwError = pHeader->headerCodes & 0xFF00;
    if(dwError)
    {
        VmAfdLog(VMAFD_DEBUG_ERROR,
            "DNS Update Failed: %d",
            dwError);
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    VmAfdLog(VMAFD_DEBUG_ANY, "Response recieved: %d", dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(serverAddr);
    VMAFD_SAFE_FREE_STRINGA(pDCName);
    VMAFD_SAFE_FREE_STRINGA(pDnsPacket);
    VMAFD_SAFE_FREE_STRINGW(pwDCName);
    close(udpSocket);
    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDdnsRpcUpdate(
    PSTR pszZone,
    PSTR pszHostname,
    PSTR pszName,
    PVMDNS_SERVER_CONTEXT pServerContext
    )
{
    DWORD dwError = 0;
    VMDNS_IP4_ADDRESS* pV4Address = NULL;
    VMDNS_IP6_ADDRESS* pV6Address = NULL;
    VMDNS_RECORD record = {0};

    if(!pszZone || pszHostname || pszName || pServerContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    //Delete all records
    dwError = VmDdnsRpcDelete(
                  pServerContext,
                  pszZone,
                  pszName);
    BAIL_ON_VMAFD_ERROR(dwError);

    //Add updated records

    dwError = VmDdnsGetSourceIp(
                    &pV4Address,
                    &pV6Address
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

    record.iClass = VMDNS_CLASS_IN;
    record.pszName = pszName;
    record.dwType = VMDNS_RR_TYPE_A;
    record.dwTtl = 3600;

    if(pV4Address)
    {
        record.Data.A.IpAddress = (DWORD)*pV4Address;
    }
    else
    {
        record.Data.AAAA.Ip6Address = pV6Address[0];
    }

    dwError = VmDnsAddRecordA(
                      pServerContext,
                      pszZone,
                      &record
                      );
    if (dwError)
    {
          VmAfdLog(VMAFD_DEBUG_ANY,
                 "%s: failed to add DNS AAAA record for %s (%u)",
                 __FUNCTION__, record.pszName, dwError);
    }

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pV4Address);
    VMAFD_SAFE_FREE_MEMORY(pV6Address);
    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDdnsRpcDelete(
    PVMDNS_SERVER_CONTEXT pServerContext,
    PSTR pszZone,
    PSTR pszName
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    PVMDNS_RECORD_ARRAY pIpV4RecordArray = NULL;
    PVMDNS_RECORD_ARRAY pIpV6RecordArray = NULL;

    if(!pServerContext || !pszZone || !pszName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    //Query all A records
    dwError = VmDnsQueryRecordsA(
                        pServerContext,
                        pszZone,
                        pszName,
                        VMDNS_RR_TYPE_A,
                        0,
                        &pIpV4RecordArray
                        );
    if(dwError)
    {
        if(dwError == ERROR_NOT_FOUND)
        {
            //do nothing
            dwError = 0;
        }
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        for(dwIndex = 0; dwIndex < pIpV4RecordArray->dwCount; dwIndex++)
        {
            dwError = VmDnsDeleteRecordA(
                                pServerContext,
                                pszZone,
                                &pIpV4RecordArray->Records[dwIndex]
                                );
            if(dwError == ERROR_NOT_FOUND)
            {
              //do nothing
                dwError = 0;
            }
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }

    //Query all AAAA Records
    dwError = VmDnsQueryRecordsA(
                        pServerContext,
                        pszZone,
                        pszName,
                        VMDNS_RR_TYPE_AAAA,
                        0,
                        &pIpV6RecordArray
                        );
    if(dwError)
    {
        if(dwError == ERROR_NOT_FOUND)
        {
            //do nothing
            dwError = 0;
        }
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        for(dwIndex = 0; dwIndex < pIpV6RecordArray->dwCount; dwIndex++)
        {
            dwError = VmDnsDeleteRecordA(
                                pServerContext,
                                pszZone,
                                &pIpV6RecordArray->Records[dwIndex]
                                );
            if(dwError == ERROR_NOT_FOUND)
            {
              //do nothing
                dwError = 0;
            }
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }

cleanup:

    if (pIpV4RecordArray)
    {
        VmDnsFreeRecordArray(pIpV4RecordArray);
    }
    if (pIpV6RecordArray)
    {
        VmDnsFreeRecordArray(pIpV6RecordArray);
    }
    return dwError;

error:

    goto cleanup;
}
