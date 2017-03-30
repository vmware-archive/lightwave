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


typedef enum
{
    VM_SOCK_PROTOCOL_UNKNOWN = 0,
    VM_SOCK_PROTOCOL_TCP,
    VM_SOCK_PROTOCOL_UDP
} VM_SOCK_PROTOCOL;

typedef enum
{
    VM_SOCK_TYPE_UNKNOWN = 0,
    VM_SOCK_TYPE_CLIENT,
    VM_SOCK_TYPE_SERVER,
    VM_SOCK_TYPE_LISTENER,
    VM_SOCK_TYPE_SIGNAL
} VM_SOCK_TYPE;

typedef DWORD (*PFN_OPEN_CLIENT_SOCKET)(
                    PCSTR                pszHost,
                    USHORT               usPort,
                    VM_SOCK_CREATE_FLAGS dwFlags,
                    PVM_SOCKET*          ppSocket
                    );

typedef DWORD (*PFN_OPEN_SERVER_SOCKET)(
                    USHORT               usPort,
                    int                  iListenQueueSize,
                    VM_SOCK_CREATE_FLAGS dwFlags,
                    PVM_SOCKET*          ppSocket
                    );

typedef DWORD (*PFN_START_LISTENING)(
                    PVM_SOCKET           pSocket,
                    int                  iListenQueueSize
                    );

typedef DWORD (*PFN_CREATE_EVENT_QUEUE)(
                    int                   iEventQueueSize,
                    PVM_SOCK_EVENT_QUEUE* ppQueue
                    );

typedef DWORD (*PFN_ADD_EVENT_QUEUE)(
                    PVM_SOCK_EVENT_QUEUE pQueue,
                    PVM_SOCKET           pSocket
                    );

typedef DWORD (*PFN_WAIT_FOR_EVENT)(
                    PVM_SOCK_EVENT_QUEUE pQueue,
                    int                  iTimeoutMS,
                    PVM_SOCKET*          ppSocket,
                    PVM_SOCK_EVENT_TYPE  pEventType,
                    PVM_SOCK_IO_BUFFER*  ppIoBuffer
                    );

typedef VOID (*PFN_CLOSE_EVENT_QUEUE)(
                    PVM_SOCK_EVENT_QUEUE pQueue
                    );

typedef DWORD (*PFN_SET_NON_BLOCKING)(
                    PVM_SOCKET pSocket
                    );

typedef DWORD (*PFN_SET_TIMEOUT)(
                    PVM_SOCKET pSocket,
                    DWORD      dwTimeOut
                    );

typedef DWORD (*PFN_GET_PROTOCOL)(
                    PVM_SOCKET           pSocket,
                    PDWORD               pdwProtocol
                    );

typedef DWORD (*PFN_SET_DATA)(
                    PVM_SOCKET           pSocket,
                    PVOID                pData,
                    PVOID*               ppOldData
                    );

typedef DWORD (*PFN_GET_DATA)(
                    PVM_SOCKET          pSocket,
                    PVOID*              ppData
                    );

typedef DWORD (*PFN_READ)(
                    PVM_SOCKET          pSocket,
                    PVM_SOCK_IO_BUFFER  pIoBuffer
                    );

typedef DWORD (*PFN_WRITE)(
                    PVM_SOCKET          pSocket,
                    const struct sockaddr*    pClientAddress,
                    socklen_t           addrLength,
                    PVM_SOCK_IO_BUFFER  pIoBuffer
                    );

typedef PVM_SOCKET (*PFN_ACQUIRE_SOCKET)(
                    PVM_SOCKET           pSocket
                    );

typedef VOID (*PFN_RELEASE_SOCKET)(
                    PVM_SOCKET           pSocket
                    );

typedef DWORD (*PFN_CLOSE_SOCKET)(
                    PVM_SOCKET pSocket
                    );

typedef DWORD (*PFN_GET_ADDRESS)(
                    PVM_SOCKET                  pSocket,
                    struct sockaddr_storage*    pAddress,
                    socklen_t*                  pAddresLen
                    );

typedef DWORD (*PFN_ALLOCATE_IO_BUFFER)(
                    VM_SOCK_EVENT_TYPE      eventType,
                    DWORD                   dwSize,
                    PVM_SOCK_IO_BUFFER*     ppIoBuffer
                    );

typedef VOID(*PFN_RELEASE_IO_BUFFER)(
                    PVM_SOCK_IO_BUFFER      pIoBuffer
                    );

typedef struct _VM_SOCK_PACKAGE
{
    PFN_OPEN_CLIENT_SOCKET pfnOpenClientSocket;
    PFN_OPEN_SERVER_SOCKET pfnOpenServerSocket;
    PFN_START_LISTENING    pfnStartListening;
    PFN_CREATE_EVENT_QUEUE pfnCreateEventQueue;
    PFN_ADD_EVENT_QUEUE    pfnAddEventQueue;
    PFN_WAIT_FOR_EVENT     pfnWaitForEvent;
    PFN_CLOSE_EVENT_QUEUE  pfnCloseEventQueue;
    PFN_SET_NON_BLOCKING   pfnSetNonBlocking;
    PFN_SET_TIMEOUT        pfnSetTimeOut;
    PFN_GET_PROTOCOL       pfnGetProtocol;
    PFN_SET_DATA           pfnSetData;
    PFN_GET_DATA           pfnGetData;
    PFN_READ               pfnRead;
    PFN_WRITE              pfnWrite;
    PFN_ACQUIRE_SOCKET     pfnAcquireSocket;
    PFN_RELEASE_SOCKET     pfnReleaseSocket;
    PFN_CLOSE_SOCKET       pfnCloseSocket;
    PFN_GET_ADDRESS        pfnGetAddress;
    PFN_ALLOCATE_IO_BUFFER pfnAllocateIoBuffer;
    PFN_RELEASE_IO_BUFFER  pfnReleaseIoBuffer;
} VM_SOCK_PACKAGE, *PVM_SOCK_PACKAGE;
