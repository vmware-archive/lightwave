/*
 * Copyright (C) 2017 VMware, Inc. All rights reserved.
 *
 * Module   : ddns.c
 *
 * Abstract :
 *
 */
#include "includes.h"


static
DWORD
VmDdnsGetMachineInfo(
    PSTR *ppszDomain,
    PSTR *ppszHostname,
    PSTR *ppszMachineName,
    PVMDNS_SERVER_CONTEXT *ppServerContext
    );

static
DWORD
VmDdnsRpcDelete(
    PVMDNS_SERVER_CONTEXT pServerContext,
    PSTR pszZone,
    PSTR pszName
    );

static
DWORD
VmDdnsRpcUpdate(
    VMDNS_IP4_ADDRESS SourceIp4,
    VMDNS_IP6_ADDRESS SourceIp6,
    PSTR pszZone,
    PSTR pszHostname,
    PSTR pszName,
    PVMDNS_SERVER_CONTEXT pServerContext
    );

static
DWORD
VmAfdDDNSUpdateDNS(
    VMDNS_IP4_ADDRESS SourceIp4,
    VMDNS_IP6_ADDRESS SourceIp6
    );

DWORD
VmAfdUpdateIP(
    );

DWORD
VmAfdDDNSInit(
    PVMNETEVENT_HANDLE* ppHandle
    )
{
    DWORD dwError = 0;
    PVMNETEVENT_HANDLE pHandle = NULL;

    if (!ppHandle)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdUpdateIP();
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmNetEventRegister(
                          VMNET_EVENT_TYPE_IPCHANGE,
                          VmAfdUpdateIP,
                          &pHandle
                          );
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppHandle = pHandle;

cleanup:

    return dwError;
error:

    if (ppHandle)
    {
        *ppHandle = NULL;
    }
    if (pHandle)
    {
        VmNetEventUnregister(pHandle);
    }
    goto cleanup;
}

DWORD
VmAfdDetectSourceIP(
      VMDNS_IP4_ADDRESS* pSourceIp4,
      VMDNS_IP6_ADDRESS* pSourceIp6
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
    VMDNS_IP6_ADDRESS SourceIp6 = {0};
    VMDNS_IP4_ADDRESS SourceIp4 = 0;
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
        dwError = VmAfdCopyMemory(
                        &SourceIp4,
                        VMDNS_IP4_ADDRESS_SIZE,
                        recvBuff,
                        recvLen
                        );
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdCopyMemory(
                        &SourceIp6,
                        VMDNS_IP6_ADDRESS_SIZE,
                        recvBuff,
                        recvLen
                        );
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    *pSourceIp6 = SourceIp6;
    *pSourceIp4 = SourceIp4;

cleanup:
    VMAFD_SAFE_FREE_MEMORY(serverAddr);
    return dwError;

error:
    if(pSourceIp4)
    {
        *pSourceIp4 = 0;
    }
    goto cleanup;
}


DWORD
VmAfdUpdateIP(
    )
{
    DWORD dwError = 0;
    VMDNS_IP6_ADDRESS IP6 = {0};
    VMDNS_IP4_ADDRESS IP4 = 0;

    dwError = VmAfdDetectSourceIP(
                              &IP4,
                              &IP6
                              );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdDDNSUpdateDNS(
                          IP4,
                          IP6
                          );
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:
    goto cleanup;
}



VOID
VmAfdDDNSShutDown(
    PVMNETEVENT_HANDLE pHandle
    )
{
    if (pHandle)
    {
        VmNetEventUnregister(pHandle);
    }
}

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
VmAfdDDNSUpdateDNS(
    VMDNS_IP4_ADDRESS SourceIp4,
    VMDNS_IP6_ADDRESS SourceIp6
    )
{
    DWORD dwError = 0;
    PSTR pszDomain = NULL;
    PSTR pszHostname = NULL;
    PSTR pszMachineName = NULL;
    PVMDNS_SERVER_CONTEXT pServerContext = NULL;

    dwError = VmDdnsGetMachineInfo(
                      &pszDomain,
                      &pszHostname,
                      &pszMachineName,
                      &pServerContext
                      );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmDdnsRpcUpdate(
                      SourceIp4,
                      SourceIp6,
                      pszDomain,
                      pszHostname,
                      pszMachineName,
                      pServerContext
                      );
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

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
VmDdnsRpcUpdate(
    VMDNS_IP4_ADDRESS SourceIp4,
    VMDNS_IP6_ADDRESS SourceIp6,
    PSTR pszZone,
    PSTR pszHostname,
    PSTR pszName,
    PVMDNS_SERVER_CONTEXT pServerContext
    )
{
    DWORD dwError = 0;
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

    record.iClass = VMDNS_CLASS_IN;
    record.pszName = pszName;
    record.dwType = VMDNS_RR_TYPE_A;
    record.dwTtl = 3600;

    if(SourceIp4)
    {
        record.Data.A.IpAddress = SourceIp4;
    }
    else
    {
        record.Data.AAAA.Ip6Address = SourceIp6;
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


