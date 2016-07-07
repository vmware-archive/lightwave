/*
 * Copyright (C) 2015 VMware, Inc. All rights reserved.
 *
 * Module   : ddns.c
 *
 * Abstract :
 *
 */


#include "includes.h"
#define BUFFER_SIZE 8*1024

#ifndef _WIN32
typedef struct sockaddr_nl nl_addr;
#endif

static
DWORD
VmDdnsGetMachineInfo(
    PVMAFD_REG_ARG *ppArgs,
    PSTR *ppszDCName,
    PSTR *ppszHostname,
    PSTR *ppszMachineName
);

static
DWORD
VmDdnsDeleteRecords(
    PVMDNS_SERVER_CONTEXT pServerContext,
    PSTR pszZone,
    PSTR pszName
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

DWORD
VmDdnsInitThread(
    PDDNS_CONTEXT *ppDdnsContext
)
{
    DWORD dwError = 0;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE;
    PDDNS_CONTEXT pDdnsContext = NULL;

    dwError = VmAfSrvGetDomainState(&domainState);
    BAIL_ON_VMAFD_ERROR(dwError);

    if(domainState == VMAFD_DOMAIN_STATE_CLIENT)
    {
          dwError = VmAfdAllocateMemory(sizeof(DDNS_CONTEXT),
                                       (PVOID *)&pDdnsContext);
          BAIL_ON_VMAFD_ERROR(dwError);

          dwError = pthread_mutex_init(&pDdnsContext->ddnsMutex, NULL);
          if (dwError)
          {
          #ifndef _WIN32
               dwError = LwErrnoToWin32Error(dwError);
          #endif
               BAIL_ON_VMAFD_ERROR(dwError);
          }
		
          #ifndef _WIN32
          pDdnsContext->eventFd = eventfd(0, 0);
          #endif

          #ifndef _WIN32
          pDdnsContext->netLinkFd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
          #endif

          //first update
          dwError = VmDdnsUpdate(pDdnsContext);
          BAIL_ON_VMAFD_ERROR(dwError);
          //start the service thread
          VmAfdLog(VMAFD_DEBUG_ANY, "Starting DDNS Client Thread");

          #ifndef _WIN32
          dwError = pthread_create(
                        &pDdnsContext->thread,
                        NULL,
                        &VmDdnsUpdateWorker,
                        (PVOID)pDdnsContext
                    );
          if (dwError)
          {
              dwError = LwErrnoToWin32Error(dwError);
              BAIL_ON_VMAFD_ERROR(dwError);
          }
          #endif
          if(!pDdnsContext)
          {
              dwError = ERROR_INVALID_PARAMETER;
              BAIL_ON_VMAFD_ERROR(dwError);
          }
          *ppDdnsContext = pDdnsContext;

          VmAfdLog(VMAFD_DEBUG_ANY, "Started DDNS client Thread successfully");
    }

    else
          VmAfdLog(VMAFD_DEBUG_ANY, "DDNS Client not started. Domain State invalid");

  cleanup:

    if(pDdnsContext)
    {
        pDdnsContext = NULL;
    }
    return dwError;

  error:

    if(ppDdnsContext)
    {
        ppDdnsContext = NULL;
    }
    goto cleanup;
}

VOID
VmDdnsShutdown(
    PDDNS_CONTEXT pDdnsContext
)
{
    PVMAFD_REG_ARG pArgs = NULL;
    PSTR pszDCName = NULL;
    PSTR pszHostname = NULL;
    PSTR pszMachineName = NULL;
    PSTR pszZone = NULL;
    PSTR pszName = NULL;
    PVMDNS_SERVER_CONTEXT pServerContext = NULL;

    #ifndef _WIN32
    char buf[10];
    sprintf(buf, "Terminate!!");

    VmAfdLog(VMAFD_DEBUG_ANY, "Shutting down DDNS Client service");

    write(pDdnsContext->eventFd, (void *)&buf, sizeof(buf));
    pthread_join(pDdnsContext->thread, NULL);

    close(pDdnsContext->eventFd);
    close(pDdnsContext->netLinkFd);
    #endif

    VmDdnsGetMachineInfo(
                  &pArgs,
                  &pszDCName,
                  &pszHostname,
                  &pszMachineName
    );
    VmDnsOpenServerWithTimeOutA(
                    pszDCName,
                    pArgs->pszAccount,
                    pArgs->pszDomain,
                    pArgs->pszPassword,
                    0,
                    NULL,
                    RPC_PING_TIMEOUT,
                    &pServerContext
                  );

    pszZone = (PSTR)pArgs->pszDomain;
    pszName = (PSTR)pszMachineName;

    VmDdnsDeleteRecords(
                    pServerContext,
                    pszZone,
                    pszName
                  );

    pthread_mutex_destroy(&pDdnsContext->ddnsMutex);
    VMAFD_SAFE_FREE_STRINGA(pszHostname);
    VMAFD_SAFE_FREE_STRINGA(pszMachineName);
    VMAFD_SAFE_FREE_STRINGA(pszDCName);

    if (pServerContext)
    {
        VmDnsCloseServer(pServerContext);
    }

    if (pArgs)
    {
        VmAfdFreeRegArgs(pArgs);
    }

}

static
DWORD
VmDdnsGetMachineInfo(
    PVMAFD_REG_ARG *ppArgs,
    PSTR *ppszDCName,
    PSTR *ppszHostname,
    PSTR *ppszMachineName
)
{
    DWORD dwError = 0;
    PVMAFD_REG_ARG pArgs = NULL;
    PSTR pszDCName = NULL;
    PSTR pszHostname = NULL;
    PSTR pszMachineName = NULL;
    PWSTR pwszDCName = NULL;

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

    *ppArgs = pArgs;
    *ppszDCName = pszDCName;
    *ppszHostname = pszHostname;
    *ppszMachineName = pszMachineName;

    if ( !ppArgs || !ppszDCName || !ppszHostname || !ppszMachineName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

  cleanup:

    return dwError;

  error:

    VMAFD_SAFE_FREE_STRINGA(pszHostname);
    VMAFD_SAFE_FREE_STRINGA(pszMachineName);
    VMAFD_SAFE_FREE_STRINGA(pszDCName);
    if (pArgs)
    {
        VmAfdFreeRegArgs(pArgs);
    }

    goto cleanup;
}

static
DWORD
VmDdnsDeleteRecords(
    PVMDNS_SERVER_CONTEXT pServerContext,
    PSTR pszZone,
    PSTR pszName
)
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    PVMDNS_RECORD_ARRAY pIpV4RecordArray = NULL;
    PVMDNS_RECORD_ARRAY pIpV6RecordArray = NULL;

    //Query all A records
    dwError = VmDnsQueryRecordsA(
                        pServerContext,
                        pszZone,
                        pszName,
                        VMDNS_RR_TYPE_A,
                        0,
                        &pIpV4RecordArray);
    if(dwError == 0){
        for(; dwIndex < pIpV4RecordArray->dwCount; dwIndex++)
        {
            dwError = VmDnsDeleteRecordA(
                                pServerContext,
                                pszZone,
                                &pIpV4RecordArray->Records[dwIndex]
                                );
            BAIL_ON_VMAFD_ERROR(dwError);

        }
    }
    else if(dwError == ERROR_NOT_FOUND)
    {
      //do nothing
        dwError = 0;
    }
    else
    {
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    //Query all AAAA Records
    dwError = VmDnsQueryRecordsA(
                        pServerContext,
                        pszZone,
                        pszName,
                        VMDNS_RR_TYPE_AAAA,
                        0,
                        &pIpV6RecordArray);

    //Delete all A records
    if(dwError == 0)
    {
        for(dwIndex = 0; dwIndex < pIpV6RecordArray->dwCount; dwIndex++)
        {
            dwError = VmDnsDeleteRecordA(
                                pServerContext,
                                pszZone,
                                &pIpV6RecordArray->Records[dwIndex]
                                );
            BAIL_ON_VMAFD_ERROR(dwError);

        }

        VmAfdLog(VMAFD_DEBUG_ANY, "AAAA Records Deleted successfully");
    }
    else
    if(dwError == ERROR_NOT_FOUND)
    {
        dwError = 0;
    }
    else
    {
        BAIL_ON_VMAFD_ERROR(dwError);
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

// Only for LINUX
#ifndef _WIN32
static
PVOID
VmDdnsUpdateWorker(
    PVOID pdata
)
{
    DWORD len, maxFd, dwError;
    char buffer[BUFFER_SIZE];
    PDDNS_CONTEXT pDdnsContext = (PDDNS_CONTEXT)pdata;
    nl_addr *bindAddr;
    bindAddr = (nl_addr *)malloc(sizeof(nl_addr));
    struct nlmsghdr *nh;
    fd_set readFs;

    bindAddr->nl_family = AF_NETLINK;
    bindAddr->nl_pad = 0;
    bindAddr->nl_pid = getpid();
    bindAddr->nl_groups = RTMGRP_IPV6_IFADDR | RTMGRP_IPV4_IFADDR;

    //bind to the IFADDR API
    dwError = bind(pDdnsContext->netLinkFd, (struct sockaddr*)bindAddr, sizeof(nl_addr));
    if (dwError)
    {
    #ifndef _WIN32
        dwError = LwErrnoToWin32Error(dwError);
    #endif
    }

    nh = (struct nlmsghdr *)buffer;

    maxFd = (pDdnsContext->netLinkFd > pDdnsContext->eventFd) ? pDdnsContext->netLinkFd : pDdnsContext->eventFd;
    while (1)
    {
        FD_ZERO(&readFs);
        FD_SET(pDdnsContext->netLinkFd, &readFs);
        FD_SET(pDdnsContext->eventFd, &readFs);
        dwError = select(maxFd + 1, &readFs, NULL, NULL, NULL);
        if (dwError)
        {
        #ifndef _WIN32
             dwError = LwErrnoToWin32Error(dwError);
        #endif
        }

        if(FD_ISSET(pDdnsContext->netLinkFd, &readFs)){
            len = recv (pDdnsContext->netLinkFd,nh,4096,0);
            for (;(NLMSG_OK (nh, len)) && (nh->nlmsg_type != NLMSG_DONE); nh = NLMSG_NEXT(nh, len))
            {
                if (nh->nlmsg_type != RTM_NEWADDR)
                {
                    continue; /* some other kind of message */
                }
                // Update detected
                dwError = VmDdnsUpdate(pDdnsContext);
                if(dwError){
                    VmAfdLog(VMAFD_DEBUG_ANY, "DDNS Update Failed. Error : %d", dwError);
                }
            }
        }
        else if(FD_ISSET(pDdnsContext->eventFd, &readFs))
        {
                VmAfdLog(VMAFD_DEBUG_ANY, "Recieved Terminate. DDNS Client exiting.");
                break;
        }
    }
    pDdnsContext = NULL;
    free(bindAddr);
    bindAddr = NULL;
    return NULL;
}
#endif

static
DWORD
VmDdnsUpdate(
    PDDNS_CONTEXT pDdnsContext
)
{
    DWORD dwError = 0;
    PVMAFD_REG_ARG pArgs = NULL;
    PSTR pszDCName = NULL;
    PSTR pszHostname = NULL;
    PSTR pszMachineName = NULL;
    PVMDNS_SERVER_CONTEXT pServerContext = NULL;
    VMDNS_IP4_ADDRESS* pV4Addresses = NULL;
    DWORD dwNumV4Address = 0;
    VMDNS_IP6_ADDRESS* pV6Addresses = NULL;
    DWORD dwNumV6Address = 0;
    VMDNS_RECORD record = {0};
    PSTR pszZone = NULL;
    PSTR pszName = NULL;
    size_t i = 0;

    dwError = VmDdnsGetMachineInfo(
                  &pArgs,
                  &pszDCName,
                  &pszHostname,
                  &pszMachineName
    );
    BAIL_ON_VMAFD_ERROR(dwError);

    pszName = (PSTR)pszMachineName;
    pszZone = (PSTR)pArgs->pszDomain;

    dwError = pthread_mutex_lock(&pDdnsContext->ddnsMutex);
    if (dwError)
    {
    #ifndef _WIN32
         dwError = LwErrnoToWin32Error(dwError);
    #endif
         BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmDnsOpenServerWithTimeOutA(
                    pszDCName,
                    pArgs->pszAccount,
                    pArgs->pszDomain,
                    pArgs->pszPassword,
                    0,
                    NULL,
                    RPC_PING_TIMEOUT,
                    &pServerContext);
                    BAIL_ON_VMAFD_ERROR(dwError);

    BAIL_ON_VMAFD_ERROR(dwError);

    //Delete all records

    dwError = VmDdnsDeleteRecords(
                  pServerContext,
                  pszZone,
                  pszName
    );
    BAIL_ON_VMAFD_ERROR(dwError);

    //Add updated records

    dwError = VmAfSrvGetIPAddressesWrap(
                    &pV4Addresses,
                    &dwNumV4Address,
                    &pV6Addresses,
                    &dwNumV6Address);
    if (dwError)
    {
        VmAfdLog(VMAFD_DEBUG_ANY,
                         "%s: failed to get interface addresses (%u)",
                 __FUNCTION__, dwError);
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    record.iClass = VMDNS_CLASS_IN;
    record.pszName = pszName;
    record.dwType = VMDNS_RR_TYPE_A;
    record.dwTtl = 3600;

    //Add the A records
    for (i = 0; i < dwNumV4Address; i++)
    {
        record.Data.A.IpAddress = pV4Addresses[i];

        dwError = VmDnsAddRecordA(
                        pServerContext,
                        pszZone,
                        &record);
        if (dwError)
        {
            VmAfdLog(VMAFD_DEBUG_ANY,
                     "%s: failed to add DNS A record for %s (%u)",
                     __FUNCTION__, record.pszName, dwError);
        }
    }

    //Add the AAAA records
    record.iClass = VMDNS_CLASS_IN;
    record.pszName = pszName;
    record.dwType = VMDNS_RR_TYPE_AAAA;
    record.dwTtl = 3600;
    for (i = 0; i < dwNumV6Address; i++)
    {
        record.Data.AAAA.Ip6Address = pV6Addresses[i];

        dwError = VmDnsAddRecordA(
                        pServerContext,
                        pszZone,
                        &record);
        if (dwError)
        {
            VmAfdLog(VMAFD_DEBUG_ANY,
                     "%s: failed to add DNS AAAA record for %s (%u)",
                     __FUNCTION__, record.pszName, dwError);
        }
    }

    dwError = pthread_mutex_unlock(&pDdnsContext->ddnsMutex);
    if (dwError)
    {
    #ifndef _WIN32
         dwError = LwErrnoToWin32Error(dwError);
    #endif
         BAIL_ON_VMAFD_ERROR(dwError);
    }

  cleanup:

    VMAFD_SAFE_FREE_MEMORY(pV4Addresses);
    VMAFD_SAFE_FREE_MEMORY(pV6Addresses);
    VMAFD_SAFE_FREE_STRINGA(pszDCName);
    VMAFD_SAFE_FREE_STRINGA(pszHostname);
    VMAFD_SAFE_FREE_STRINGA(pszMachineName);

    if (pServerContext)
    {
        VmDnsCloseServer(pServerContext);
    }

    if (pArgs)
    {
        VmAfdFreeRegArgs(pArgs);
    }
    return dwError;

  error:

    goto cleanup;
}
