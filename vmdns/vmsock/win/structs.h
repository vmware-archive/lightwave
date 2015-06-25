/*
 * Copyright (c) VMware Inc.  All rights Reserved.
 */

typedef struct _VM_SOCKET
{
    SOCKET                  hSocket;
    VM_SOCK_PROTOCOL        protocol;
    VM_SOCK_TYPE            type;
    ULONG                   refCount;
    PVM_SOCK_EVENT_QUEUE    pEventQueue;
    HANDLE                  hThreadListen;
    struct sockaddr_storage addr;
    int                     addrLen;
} VM_SOCKET;

typedef struct _VM_SOCK_EVENT_QUEUE
{
    HANDLE      hIOCP;
    HANDLE      hEventListen;
    BOOL        bShutdown;
}VM_SOCK_EVENT_QUEUE;

typedef struct _VM_SOCK_IO_CONTEXT
{
    OVERLAPPED          Overlapped;
    VM_SOCK_EVENT_TYPE  eventType;
    VM_SOCK_IO_BUFFER   IoBuffer;
    CHAR                DataBuffer[1];
} VM_SOCK_IO_CONTEXT, *PVM_SOCK_IO_CONTEXT;

