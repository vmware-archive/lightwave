/*
 * Copyright (c) VMware Inc.  All rights Reserved.
 */

typedef struct _VM_SOCKET
{
    LONG             refCount;

    VM_SOCK_TYPE     type;
    VM_SOCK_PROTOCOL protocol;

    struct sockaddr  addr;
    socklen_t        addrLen;
    struct sockaddr* pAddr;   // Do not free

    PVMDNS_MUTEX       pMutex;

    int              fd;

    PVOID            pData;

} VM_SOCKET;

typedef struct _VM_SOCK_EVENT_QUEUE
{
    PVMDNS_MUTEX           pMutex;

    PVM_SOCKET           pSignalReader;
    PVM_SOCKET           pSignalWriter;

    VM_SOCK_POSIX_EVENT_STATE state;

    int                  epollFd;

    struct epoll_event * pEventArray;
    DWORD                dwSize;

    int                  nReady; // Number of ready descriptors
    int                  iReady; // Index when processing

} VM_SOCK_EVENT_QUEUE;

typedef struct _VM_SOCK_IO_CONTEXT
{
    VM_SOCK_EVENT_TYPE  eventType;
    VM_SOCK_IO_BUFFER   IoBuffer;
    CHAR                DataBuffer[1];
} VM_SOCK_IO_CONTEXT, *PVM_SOCK_IO_CONTEXT;

