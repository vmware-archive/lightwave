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

DWORD
VmDnsSockOpenClient(
    PCSTR                pszHost,
    USHORT               usPort,
    VM_SOCK_CREATE_FLAGS dwFlags,
    PVM_SOCKET*          ppSocket
    )
{
    DWORD dwError = 0;

    if (!pszHost || !usPort || !ppSocket )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMSOCK_ERROR(dwError);
    }

    dwError = gpVmDnsSockPackage->pfnOpenClientSocket(
                                    pszHost,
                                    usPort,
                                    dwFlags,
                                    ppSocket);

error:

    return dwError;
}

DWORD
VmDnsSockOpenServer(
    USHORT               usPort,
    int                  iListenQueueSize,
    VM_SOCK_CREATE_FLAGS dwFlags,
    PVM_SOCKET*          ppSocket
    )
{
    DWORD dwError = 0;

    if (!usPort || !ppSocket)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMSOCK_ERROR(dwError);
    }

    dwError = gpVmDnsSockPackage->pfnOpenServerSocket(
                                    usPort,
                                    iListenQueueSize,
                                    dwFlags,
                                    ppSocket);
error:

    return dwError;
}

DWORD
VmDnsSockCreateEventQueue(
    int                   iEventQueueSize,  /*         OPTIONAL */
    PVM_SOCK_EVENT_QUEUE* ppQueue           /*     OUT          */
    )
{
    DWORD dwError = 0;

    if (!iEventQueueSize || !ppQueue)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMSOCK_ERROR(dwError);
    }

    dwError = gpVmDnsSockPackage->pfnCreateEventQueue(iEventQueueSize, ppQueue);

error:

    return dwError;
}

DWORD
VmDnsSockEventQueueAdd(
    PVM_SOCK_EVENT_QUEUE pQueue,
    BOOL                 bOneShot,
    PVM_SOCKET           pSocket
    )
{
    DWORD dwError = 0;

    if (!pQueue || !pSocket)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMSOCK_ERROR(dwError);
    }

    dwError = gpVmDnsSockPackage->pfnAddEventQueue(pQueue, bOneShot,pSocket);

error:

    return dwError;
}

DWORD
VmDnsSockEventQueueRearm(
    PVM_SOCK_EVENT_QUEUE pQueue,
    BOOL                 bOneShot,
    PVM_SOCKET           pSocket
    )
{
    DWORD dwError = 0;

    if (!pQueue || !pSocket)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMSOCK_ERROR(dwError);
    }

    dwError = gpVmDnsSockPackage->pfnRearmEventQueue(pQueue, bOneShot, pSocket);

error:

    return dwError;
}

DWORD
VmDnsSockEventQueueRemove(
    PVM_SOCK_EVENT_QUEUE pQueue,
    PVM_SOCKET           pSocket
    )
{
    DWORD dwError = 0;

#ifndef WIN32
    if (!pQueue || !pSocket)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMSOCK_ERROR(dwError);
    }

    dwError = gpVmDnsSockPackage->pfnRemoveEventQueue(pQueue, pSocket);
#endif

error:
    return dwError;
}

DWORD
VmDnsSockWaitForEvent(
    PVM_SOCK_EVENT_QUEUE pQueue,
    int                  iTimeoutMS,
    PVM_SOCKET*          ppSocket,
    PVM_SOCK_EVENT_TYPE  pEventType,
    PVM_SOCK_IO_BUFFER*  ppIoEvent
    )
{
    DWORD dwError = 0;

    if (!pQueue || !ppSocket || !pEventType)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMSOCK_ERROR(dwError);
    }

    dwError = gpVmDnsSockPackage->pfnWaitForEvent(
                                    pQueue,
                                    iTimeoutMS,
                                    ppSocket,
                                    pEventType,
                                    ppIoEvent
                                    );

error:

    return dwError;
}

VOID
VmDnsSockShutdownEventQueue(
    PVM_SOCK_EVENT_QUEUE pQueue
    )
{
    if (pQueue)
    {
        gpVmDnsSockPackage->pfnShutdownEventQueue(pQueue);
    }
}

VOID
VmDnsSockFreeEventQueue(
    PVM_SOCK_EVENT_QUEUE pQueue
    )
{
#ifndef WIN32
    if (pQueue)
    {
        gpVmDnsSockPackage->pfnFreeEventQueue(pQueue);
    }
#endif
}

DWORD
VmDnsSockSetNonBlocking(
    PVM_SOCKET pSocket
    )
{
    DWORD dwError = 0;

    if (!pSocket)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMSOCK_ERROR(dwError);
    }

    dwError = gpVmDnsSockPackage->pfnSetNonBlocking(pSocket);

error:

    return dwError;
}

DWORD
VmDnsSockSetTimeOut(
    PVM_SOCKET pSocket,
    DWORD      dwTimeOut
    )
{
    DWORD dwError = 0;

    if (!pSocket)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMSOCK_ERROR(dwError);
    }

    if (dwTimeOut)
    {
        dwError = gpVmDnsSockPackage->pfnSetTimeOut(pSocket, dwTimeOut);
    }

error:

    return dwError;
}


DWORD
VmDnsSockGetProtocol(
    PVM_SOCKET           pSocket,
    PDWORD               pdwProtocol
    )
{
    DWORD dwError = 0;

    if (!pSocket)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMSOCK_ERROR(dwError);
    }

    dwError = gpVmDnsSockPackage->pfnGetProtocol(pSocket, pdwProtocol);

error:

    return dwError;
}

DWORD
VmDnsSockSetData(
    PVM_SOCKET           pSocket,
    PVOID                pData,
    PVOID*               ppOldData
    )
{
    DWORD dwError = 0;

    if (!pSocket)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMSOCK_ERROR(dwError);
    }

    dwError = gpVmDnsSockPackage->pfnSetData(pSocket, pData, ppOldData);

error:

    return dwError;
}

DWORD
VmDnsSockGetData(
    PVM_SOCKET          pSocket,
    PVOID*              ppData
    )
{
    DWORD dwError = 0;

    if (!pSocket || !ppData)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMSOCK_ERROR(dwError);
    }

    dwError = gpVmDnsSockPackage->pfnGetData(pSocket, ppData);

error:

    return dwError;
}

DWORD
VmDnsSockRead(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD dwError = 0;

    if (!pSocket || !pIoBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMSOCK_ERROR(dwError);
    }

    dwError = gpVmDnsSockPackage->pfnRead(
                            pSocket,
                            pIoBuffer);

error:

    return dwError;
}

DWORD
VmDnsSockWrite(
    PVM_SOCKET              pSocket,
    const struct sockaddr*  pClientAddress,
    socklen_t               addrLength,
    PVM_SOCK_IO_BUFFER      pIoBuffer
)
{
    DWORD dwError = 0;

    if (!pSocket || !pIoBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMSOCK_ERROR(dwError);
    }

    dwError = gpVmDnsSockPackage->pfnWrite(
                            pSocket,
                            pClientAddress,
                            addrLength,
                            pIoBuffer);
    BAIL_ON_VMSOCK_ERROR(dwError);

error:

    return dwError;
}

PVM_SOCKET
VmDnsSockAcquire(
    PVM_SOCKET           pSocket
    )
{
    return pSocket ? gpVmDnsSockPackage->pfnAcquireSocket(pSocket) : NULL;
}

VOID
VmDnsSockRelease(
    PVM_SOCKET           pSocket
    )
{
    if (pSocket)
    {
        gpVmDnsSockPackage->pfnReleaseSocket(pSocket);
    }
}

DWORD
VmDnsSockClose(
    PVM_SOCKET pSocket
    )
{
    DWORD dwError = 0;

    if (!pSocket)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMSOCK_ERROR(dwError);
    }

    dwError = gpVmDnsSockPackage->pfnCloseSocket(pSocket);
    BAIL_ON_VMSOCK_ERROR(dwError);

error:

    return dwError;
}

BOOLEAN
VmDnsSockIsValidIPAddress(
    PCSTR pszAddress
    )
{
    BOOLEAN bIsValid = TRUE;

    if (!IsNullOrEmptyString(pszAddress))
    {
        struct sockaddr_storage ipv4Addr;

        bIsValid = (inet_pton(AF_INET, pszAddress, &ipv4Addr) == 1);

#ifdef AF_INET6
        if (!bIsValid)
        {
            struct sockaddr_storage ipv6Addr;

            bIsValid = (inet_pton(AF_INET6, pszAddress, &ipv6Addr) == 1);
        }
#endif
    }

    return bIsValid;
}


DWORD
VmDnsSockGetAddress(
    PVM_SOCKET                  pSocket,
    struct sockaddr_storage*    pAddress,
    socklen_t*                  addresLen
    )
{
    DWORD dwError = 0;

    if (!pSocket)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMSOCK_ERROR(dwError);
    }

    dwError = gpVmDnsSockPackage->pfnGetAddress(pSocket, pAddress, addresLen);
    BAIL_ON_VMSOCK_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmDnsSockStartListening(
    PVM_SOCKET           pSocket,
    int                  iListenQueueSize
    )
{
    DWORD dwError = 0;

    if (!pSocket)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMSOCK_ERROR(dwError);
    }

    dwError = gpVmDnsSockPackage->pfnStartListening(pSocket, iListenQueueSize);
    BAIL_ON_VMSOCK_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmDnsSockAllocateIoBuffer(
    VM_SOCK_EVENT_TYPE          eventType,
    PVM_SOCK_EVENT_CONTEXT      pEventContext,
    DWORD                       dwSize,
    PVM_SOCK_IO_BUFFER*         ppIoBuffer
    )
{
    DWORD dwError = 0;

    if (!ppIoBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMSOCK_ERROR(dwError);
    }

    dwError = gpVmDnsSockPackage->pfnAllocateIoBuffer(
                                                  eventType,
                                                  pEventContext,
                                                  dwSize,
                                                  ppIoBuffer
                                                  );
    BAIL_ON_VMSOCK_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmDnsSockSetEventContext(
    PVM_SOCK_IO_BUFFER      pIoBuffer,
    PVM_SOCK_EVENT_CONTEXT  pEventContext,
    PVM_SOCK_EVENT_CONTEXT* ppOldEventContext
    )
{
    DWORD dwError = 0;

    if (!pIoBuffer || !ppOldEventContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMSOCK_ERROR(dwError);
    }

    dwError = gpVmDnsSockPackage->pfnSetEventContext(pIoBuffer, pEventContext, ppOldEventContext);
    BAIL_ON_VMSOCK_ERROR(dwError);

cleanup:

    return dwError;
error:

    if (ppOldEventContext)
    {
        *ppOldEventContext = NULL;
    }
    goto cleanup;
}

DWORD
VmDnsSockGetEventContext(
    PVM_SOCK_IO_BUFFER      pIoBuffer,
    PVM_SOCK_EVENT_CONTEXT* ppEventContext
    )
{
    DWORD dwError = 0;

    if (!pIoBuffer || !ppEventContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMSOCK_ERROR(dwError);
    }

    dwError = gpVmDnsSockPackage->pfnGetEventContext(pIoBuffer, ppEventContext);
    BAIL_ON_VMSOCK_ERROR(dwError);

cleanup:

    return dwError;
error:

    if (ppEventContext)
    {
        *ppEventContext = NULL;
    }
    goto cleanup;
}



DWORD
VmDnsSockReleaseIoBuffer(
    PVM_SOCK_IO_BUFFER  pIoBuffer
    )
{
    DWORD dwError = 0;

    if (!pIoBuffer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMSOCK_ERROR(dwError);
    }

    gpVmDnsSockPackage->pfnReleaseIoBuffer(pIoBuffer);

error:

    return dwError;

}
