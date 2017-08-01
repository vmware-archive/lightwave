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

#ifndef _WIN32
typedef struct sockaddr_nl nl_addr;
#endif

static
PVOID
VmLinuxWaitOnEventWorker(
    PVOID pData
    );

DWORD
VmLinuxOpenConnection(
    DWORD dwEventType,
    PVMNETEVENT_FD pFD
    )
{
    DWORD dwError = 0;
    int netLinkFd = -1;

    if (!pFD)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMNETEVENT_ERROR(dwError);
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

    pFD->dwNetlinkFD = netLinkFd;

cleanup:

    return dwError;
error:

    if (pFD)
    {
        pFD->dwNetlinkFD = -1;
    }
    if (netLinkFd>=0)
    {
        close(netLinkFd);
    }
    goto cleanup;
}

DWORD
VmLinuxWaitOnEvent(
    VMNETEVENT_FD FD,
    PFN_VMNETEVENT_CALLBACK pCallback,
    pthread_t* pEventThread
    )
{
    DWORD dwError = 0;
    PVMNETEVENT_DATA pData = NULL;
    pthread_t eventWorkerThread;

    if (!pEventThread)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMNETEVENT_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(
                            sizeof(VMNETEVENT_DATA),
                            (PVOID*)&pData
                            );
    BAIL_ON_VMNETEVENT_ERROR(dwError);

    pData->eventFd = FD;
    pData->pfnCallBack = pCallback;

    dwError = pthread_create(
                        &eventWorkerThread,
                        NULL,
                        VmLinuxWaitOnEventWorker,
                        (PVOID)pData
                        );
    if (dwError)
    {
        dwError = LwErrnoToWin32Error(dwError);
        BAIL_ON_VMNETEVENT_ERROR(dwError);
    }

    *pEventThread = eventWorkerThread;

cleanup:

    return dwError;
error:
    goto cleanup;
}

VOID
VmLinuxCloseConnection(
    VMNETEVENT_FD FD
    )
{
	if (FD.dwNetlinkFD >=0)
        {
            close(FD.dwNetlinkFD);
        }
}

static
PVOID
VmLinuxWaitOnEventWorker(
    PVOID pData
    )
{
    DWORD dwError = 0;
    int iError = 0;
    DWORD len = 0;
    char buffer[VMDDNS_BUFFER_SIZE] = {0};
    nl_addr *bindAddr = NULL;
    struct nlmsghdr *nh = NULL;
    PVMNETEVENT_DATA pEventData = NULL;

    if (!pData)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pEventData = (PVMNETEVENT_DATA)pData;

    if (pEventData->eventFd.dwNetlinkFD < 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMNETEVENT_ERROR(dwError);
    }

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
    iError = bind(
                pEventData->eventFd.dwNetlinkFD,
                (struct sockaddr*)bindAddr,
                sizeof(nl_addr)
                );
    if (iError < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    nh = (struct nlmsghdr *)buffer;

    while(1)
    {
        len = recv(pEventData->eventFd.dwNetlinkFD, nh, 4096, 0);
        if(len < 0)
        {
            dwError = LwErrnoToWin32Error(errno);
            VmAfdLog(VMAFD_DEBUG_ANY, "Recieve failed. Error[%d]", dwError);
            continue;
        }

        else if (len == 0)
        {
            break;
        }

        for(; (NLMSG_OK (nh, len)) && (nh->nlmsg_type != NLMSG_DONE); nh = NLMSG_NEXT(nh, len))
        {
            if (nh->nlmsg_type != RTM_NEWADDR)
            {
                continue; /* some other kind of message */
            }

            if (pEventData->pfnCallBack)
            {
                dwError = pEventData->pfnCallBack();
                if (dwError)
                {
                    VmAfdLog(VMAFD_DEBUG_ANY, "Callback failed");
                }
            }
            memset(buffer, 0, VMDDNS_BUFFER_SIZE);
        }
    }

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pEventData);
    VMAFD_SAFE_FREE_MEMORY(bindAddr);
    VmAfdLog(VMAFD_DEBUG_ANY, "VmLinuxWaitOnEventWorker exiting. Error[%d]", dwError);
    return NULL;

error:

    goto cleanup;
}
