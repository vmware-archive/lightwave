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


/**
 * @brief Opens a client socket
 *
 * @param[in]  pszHost  Target hostname or IP Address.
 *                      An empty string will imply the localhost.
 * @param[in]  usPort   16 bit port number
 * @param[in]  dwFlags  32 bit flags specifying socket creation preferences
 * @param[out] ppSocket Pointer to created socket context
 *
 * @return 0 on success
 */
DWORD
VmDnsSockPosixOpenClient(
    PCSTR                pszHost,
    USHORT               usPort,
    VM_SOCK_CREATE_FLAGS dwFlags,
    PVM_SOCKET*          ppSocket
    );

/**
 * @brief Opens a server socket
 *
 * @param[in] usPort 16 bit local port number that the server listens on
 * @param[in,optional] iListenQueueSize
 *       size of connection acceptance queue.
 *       This value can be (-1) to use the default value.
 *
 * @param[in]  dwFlags 32 bit flags defining socket creation preferences
 * @param[out] ppSocket Pointer to created socket
 *
 * @return 0 on success
 */
DWORD
VmDnsSockPosixOpenServer(
    USHORT               usPort,
    int                  iListenQueueSize,
    VM_SOCK_CREATE_FLAGS dwFlags,
    PVM_SOCKET*          ppSocket
    );

/**
 * @brief Creates a Event queue to be used for detecting events on sockets
 *
 * @param[in,optional] iEventQueueSize
 *       specifies the event queue size.
 *       This value can be (-1) to use the default value
 * @param[out] ppQueue Pointer to accept created event queue
 *
 * @return 0 on success
 */
DWORD
VmDnsSockPosixCreateEventQueue(
    int                   iEventQueueSize,
    PVM_SOCK_EVENT_QUEUE* ppQueue
    );

/**
 * @brief Adds a socket to the event queue
 *
 * @param[in] pQueue  Pointer to Event queue
 * @param[in] pSocket Pointer to Socket
 *
 * @return 0 on success
 */
DWORD
VmDnsSockPosixEventQueueAdd(
    PVM_SOCK_EVENT_QUEUE pQueue,
    BOOL                 bOneShot,
    PVM_SOCKET           pSocket
    );

/**
 * @brief Rearms a socket to the event queue
 *
 * @param[in] pQueue  Pointer to Event queue
 * @param[in] pSocket Pointer to Socket
 *
 * @return 0 on success
 */
DWORD
VmDnsSockPosixEventQueueRearm(
    PVM_SOCK_EVENT_QUEUE pQueue,
    BOOL                 bOneShot,
    PVM_SOCKET           pSocket
    );

/**
 * @brief Removes a socket on the event queue
 *
 * @param[in] pQueue  Pointer to Event queue
 * @param[in] pSocket Pointer to Socket
 *
 * @return 0 on success
 */
DWORD
VmDnsSockPosixEventQueueRemove(
    PVM_SOCK_EVENT_QUEUE pQueue,
    PVM_SOCKET           pSocket
    );

/**
 * @brief Waits for an event on the event queue
 *
 * @param[in] pQueue   Pointer to event queue
 * @param[in,optional] iTimeoutMS
 *       Timeout in milliseconds.
 *       Waits forever if (-1) is passed in.
 * @param[out]    ppSocket   Pointer to socket that has an event
 * @param[in,out] pEventType Event type detected on socket
 *
 * @return 0 on success
 */
DWORD
VmDnsSockPosixWaitForEvent(
    PVM_SOCK_EVENT_QUEUE pQueue,
    int                  iTimeoutMS,
    PVM_SOCKET*          ppSocket,
    PVM_SOCK_EVENT_TYPE  pEventType,
    PVM_SOCK_IO_BUFFER*  ppIoBuffer
    );

/**
 * @brief Shuts down the event queue
 *
 * @param[in] pQueue Pointer to event queue
 *
 *
 */

VOID
VmDnsSockPosixShutdownEventQueue(
    PVM_SOCK_EVENT_QUEUE pQueue
    );

/**
 * @brief frees event queue
 *
 * @param[in] pQueue Pointer to event queue
 *
 *
 */
VOID
VmDnsSockPosixFreeEventQueue(
    PVM_SOCK_EVENT_QUEUE pQueue
    );

/**
 * @brief sets socket to be non-blocking
 *
 * @param[in] pSocket Pointer to socket
 *
 * @return 0 on success
 */

DWORD
VmDnsSockPosixSetNonBlocking(
    PVM_SOCKET           pSocket
    );


/**
 * @brief sets socket to be non-blocking
 *
 * @param[in] pSocket Pointer to socket
 *
 * @return 0 on success
 */

DWORD
VmDnsSockPosixSetTimeOut(
    PVM_SOCKET           pSocket,
    DWORD                dwTimeOut
    );


/**
 * @brief Retrieves the protocol the socket has been configured with
 *
 * @param[in]     pSocket     Pointer to socket
 * @param[in,out] pdwProtocol Protocol the socket has been configured with
 *                            This will be one of { SOCK_STREAM, SOCK_DGRAM... }
 */
DWORD
VmDnsSockPosixGetProtocol(
    PVM_SOCKET           pSocket,
    PDWORD               pdwProtocol
    );

/**
 * @brief Sets data associated with the socket
 *
 * @param[in] pSocket Pointer to socket
 * @param[in] pData   Pointer to data associated with the socket
 * @param[in,out,optional] ppOldData Pointer to receive old data
 *
 * @return 0 on success
 */
DWORD
VmDnsSockPosixSetData(
    PVM_SOCKET           pSocket,
    PVOID                pData,
    PVOID*               ppOldData
    );

/**
 * @brief Gets data currently associated with the socket.
 *
 * @param[in]     pSocket Pointer to socket
 * @param[in,out] ppData  Pointer to receive data
 *
 * @return Pointer to current data associated with the socket
 */
DWORD
VmDnsSockPosixGetData(
    PVM_SOCKET          pSocket,
    PVOID*              ppData
    );

/**
 * @brief Reads data from the socket
 *
 * @param[in]     pSocket      Pointer to socket
 * @param[in]     pBuffer      Buffer to read the data into
 * @param[in]     dwBufSize    Maximum size of the passed in buffer
 * @param[in,out] pdwBytesRead Number of bytes read in to the buffer
 * @param[in,out,optional] pClientAddress Client address to fill in optionally
 * @param[in,out,optional] pAddrLength    Length of the client address
 *
 * @return 0 on success
 */
DWORD
VmDnsSockPosixRead(
    PVM_SOCKET          pSocket,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    );

/**
 * @brief Writes data to the socket
 *
 * @param[in]     pSocket      Pointer to socket
 * @param[in]     pBuffer      Buffer from which bytes have to be written
 * @param[in]     dwBufLen     Number of bytes to write from the buffer
 * @param[in,out] pdwBytesRead Number of bytes written to the socket
 * @param[in,optional] pClientAddress Client address to send to
 * @param[in,optional] addrLength     Length of the client address
 *
 * In case of UDP sockets, it is mandatory to provide the client address and
 * length.
 *
 * @return 0 on success
 */
DWORD
VmDnsSockPosixWrite(
    PVM_SOCKET          pSocket,
    const struct sockaddr*    pClientAddress,
    socklen_t           addrLength,
    PVM_SOCK_IO_BUFFER  pIoBuffer
    );

/**
 * @brief  Acquires a reference on the socket
 *
 * @return Pointer to acquired socket
 */

PVM_SOCKET
VmDnsSockPosixAcquireSocket(
    PVM_SOCKET           pSocket
    );

/**
 * @brief Releases current reference to socket
 *
 */
VOID
VmDnsSockPosixReleaseSocket(
    PVM_SOCKET           pSocket
    );

/**
 * @brief Closes the socket
 *        This call does not release the reference to the socket or free it.
 */
DWORD
VmDnsSockPosixCloseSocket(
    PVM_SOCKET           pSocket
    );

DWORD
VmDnsSockPosixGetAddress(
    PVM_SOCKET                  pSocket,
    struct sockaddr_storage*    pAddress,
    socklen_t*                  pAddresLen
    );

DWORD
VmDnsSockPosixAllocateIoBuffer(
    VM_SOCK_EVENT_TYPE          eventType,
    PVM_SOCK_EVENT_CONTEXT      pEventContext,
    DWORD                       dwSize,
    PVM_SOCK_IO_BUFFER*         ppIoBuffer
    );

DWORD
VmDnsSockPosixSetEventContext(
    PVM_SOCK_IO_BUFFER      pIoBuffer,
    PVM_SOCK_EVENT_CONTEXT  pEventContext,
    PVM_SOCK_EVENT_CONTEXT* ppOldEventContext
    );

DWORD
VmDnsSockPosixGetEventContext(
    PVM_SOCK_IO_BUFFER        pIoBuffer,
    PVM_SOCK_EVENT_CONTEXT*   ppEventContext
    );

VOID
VmDnsSockPosixFreeIoBuffer(
    PVM_SOCK_IO_BUFFER     pIoBuffer
    );
