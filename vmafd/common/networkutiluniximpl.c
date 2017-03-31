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
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

DWORD
VmAfdOpenServerConnectionImpl(
	PVM_AFD_CONNECTION *ppConnection
	)
{
	DWORD dwError = 0;
	int socket_fd = -1, on = 1;
	struct sockaddr_un address = {0};
	PVM_AFD_CONNECTION pConnection = NULL;

	socket_fd = socket(PF_UNIX,SOCK_STREAM, 0);

	if (socket_fd < 0){
		dwError = LwErrnoToWin32Error(errno);
		BAIL_ON_VMAFD_ERROR(dwError);
	}

	if( setsockopt( socket_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) ) < 0){
		dwError = LwErrnoToWin32Error(errno);
		BAIL_ON_VMAFD_ERROR(dwError);
	}

	unlink (SOCKET_FILE_PATH);

	address.sun_family = AF_UNIX;
	snprintf (address.sun_path, sizeof(SOCKET_FILE_PATH), SOCKET_FILE_PATH);

	if (bind (socket_fd, (struct sockaddr *)&address, sizeof(struct sockaddr_un)) < 0){
		dwError = LwErrnoToWin32Error(errno);
		BAIL_ON_VMAFD_ERROR(dwError);
	}

	if (listen (socket_fd, 5) < 0){
		dwError = LwErrnoToWin32Error(errno);
		BAIL_ON_VMAFD_ERROR(dwError);
	}

	dwError = VmAfdAllocateMemory(sizeof(VM_AFD_CONNECTION), (PVOID *)&pConnection);
	BAIL_ON_VMAFD_ERROR(dwError);
	pConnection->fd = socket_fd;
	*ppConnection = pConnection;

cleanup:
	return dwError;
error:
	if (ppConnection != NULL){
		*ppConnection = NULL;
	}
	if (socket_fd >=0 ){
		close (socket_fd);
	}
	VMAFD_SAFE_FREE_MEMORY (pConnection);
	goto cleanup;
}

VOID
VmAfdCloseServerConnectionImpl(
	PVM_AFD_CONNECTION pConnection
	)
{
	if (pConnection->fd >= 0 ){
	  close (pConnection->fd);
          pConnection->fd = -1;
	}
}

DWORD
VmAfdOpenClientConnectionImpl(
	PVM_AFD_CONNECTION *ppConnection
	)
{
	DWORD dwError = 0;
	int socket_fd = 0;
	struct sockaddr_un address;
	PVM_AFD_CONNECTION pConnection = NULL;

	socket_fd = socket (PF_UNIX, SOCK_STREAM, 0);
	if (socket_fd < 0){
		dwError = LwErrnoToWin32Error(errno);
		BAIL_ON_VMAFD_ERROR(dwError);
	}
	memset (&address, 0, sizeof(struct sockaddr_un));
	address.sun_family = AF_UNIX;
	snprintf (address.sun_path, sizeof(SOCKET_FILE_PATH), SOCKET_FILE_PATH);

	if (connect(socket_fd, (struct sockaddr *) &address, sizeof(struct sockaddr_un)) <0)
    {
        if (errno == ENOENT || errno == ECONNREFUSED)
        {
            dwError = ERROR_CANNOT_CONNECT_VMAFD;
        }
        else if (errno == EACCES)
        {
            dwError = ERROR_ACCESS_DENIED;
        }
        else
        {
            dwError = LwErrnoToWin32Error (errno);
        }
        BAIL_ON_VMAFD_ERROR(dwError);
    }

	dwError = VmAfdAllocateMemory (sizeof(VM_AFD_CONNECTION),(PVOID *)&pConnection);
	BAIL_ON_VMAFD_ERROR(dwError);
	pConnection->fd = socket_fd;
	*ppConnection = pConnection;

cleanup:
	return dwError;
error:
	if (ppConnection != NULL){
		*ppConnection = NULL;
	}
	if (socket_fd >= 0){
		close (socket_fd);
	}
	VMAFD_SAFE_FREE_MEMORY(pConnection);
	goto cleanup;
}

VOID
VmAfdCloseClientConnectionImpl(
	PVM_AFD_CONNECTION pConnection
	)
{
	if (pConnection->fd > -1){
	    close(pConnection->fd);
	    pConnection->fd = -1;
	}
}

DWORD
VmAfdAcceptConnectionImpl(
  PVM_AFD_CONNECTION pConnection,
  PVM_AFD_CONNECTION *ppConnection
  )
{
	DWORD dwError = 0;
	PVM_AFD_CONNECTION pTempConnection = NULL;
	int connection_fd = 0;
	struct sockaddr_un address = {0};
	socklen_t address_length = 0;

	connection_fd = accept(pConnection->fd,(struct sockaddr *)&address, &address_length);
	if (connection_fd < 0){
		dwError = LwErrnoToWin32Error(errno);
		BAIL_ON_VMAFD_ERROR(dwError);
	}

	dwError = VmAfdAllocateMemory(sizeof(VM_AFD_CONNECTION),(PVOID *)&pTempConnection);
	BAIL_ON_VMAFD_ERROR(dwError);

	pTempConnection->fd = connection_fd;
	*ppConnection = pTempConnection;
cleanup:
	return dwError;
error:
	if (pTempConnection){
		VMAFD_SAFE_FREE_MEMORY(pTempConnection);
	}
	if (ppConnection){
		*ppConnection = NULL;
	}
	if (connection_fd >= 0){
		close (connection_fd);
	}
	goto cleanup;
}

DWORD
VmAfdReadDataImpl(
	PVM_AFD_CONNECTION pConnection,
	PBYTE *ppResponse,
	PDWORD pdwResponseSize
	)
{
	DWORD dwError = 0;
	ssize_t dwBytesRead = 0;
	PBYTE pResponse = NULL;
    PBYTE   pResponseCursor  = NULL;
    DWORD   dwBytesSent      = 0;
    DWORD   dwTotalBytesRead = 0;

    do
    {
        dwBytesRead = read(pConnection->fd, (PVOID)&dwBytesSent, sizeof(DWORD));
    } while (dwBytesSent == -1 && errno == EINTR);

    if (dwBytesRead < sizeof(DWORD))
    {
        dwError = LwErrnoToWin32Error(dwError);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwBytesRead = 0;

    if (dwBytesSent == 0)
    {
#ifndef __MACH__
        dwError = ERROR_COMMUNICATION;
#else
        dwError = 70;
#endif
        goto error;
    }

    dwError = VmAfdAllocateMemory(dwBytesSent, (PVOID *)&pResponse);
    BAIL_ON_VMAFD_ERROR(dwError);

    pResponseCursor = pResponse;

    while (dwTotalBytesRead < dwBytesSent)
    {
        DWORD dwBytesToRead =
                    VMAFD_IPC_PACKET_SIZE < (dwBytesSent -
                                                dwTotalBytesRead)
                                        ? VMAFD_IPC_PACKET_SIZE
                                        : dwBytesSent - dwTotalBytesRead;
        do
        {
            dwBytesRead = read(pConnection->fd, pResponseCursor, dwBytesToRead);
        } while (dwBytesRead == -1 && errno == EINTR);

        if (dwBytesRead == -1)
        {
            dwError = LwErrnoToWin32Error(errno);
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        dwTotalBytesRead += dwBytesRead;
        pResponseCursor += dwBytesRead;
    }

    if (dwTotalBytesRead < dwBytesSent)
    {
        dwError = ERROR_IO_INCOMPLETE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    *pdwResponseSize = dwBytesRead;
    *ppResponse      = pResponse;

cleanup:
    return dwError;
error:
    if (ppResponse != NULL)
    {
        *ppResponse = NULL;
    }
    if (pdwResponseSize != NULL)
    {
        *pdwResponseSize = 0;
    }
    VMAFD_SAFE_FREE_MEMORY(pResponse);
    goto cleanup;
}

DWORD
VmAfdWriteDataImpl(
	PVM_AFD_CONNECTION pConnection,
	PBYTE pRequest,
	DWORD dwRequestSize
)
{
	DWORD dwError = 0;
	ssize_t dwBytesWritten = 0;
        DWORD dwTotalBytesWritten = 0;
        PBYTE pRequestCursor = pRequest;
        DWORD dwActualRequestSize = dwRequestSize;

        do
        {
                dwBytesWritten = write(pConnection->fd, (PVOID)&dwRequestSize, sizeof (DWORD));
        }while (dwBytesWritten == -1 && errno == EINTR);


        if (dwBytesWritten < sizeof (DWORD))
        {
                dwError = LwErrnoToWin32Error(errno);
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        dwBytesWritten = 0;

        while (dwRequestSize > 0)
        {
                DWORD dwBytesToWrite = VMAFD_IPC_PACKET_SIZE<dwRequestSize?VMAFD_IPC_PACKET_SIZE:dwRequestSize;

                do
                {
                    dwBytesWritten = write(pConnection->fd,pRequestCursor,dwBytesToWrite);
                }while (dwBytesWritten == -1 && errno == EINTR);

                if (dwBytesWritten == -1)
                {
                        dwError = LwErrnoToWin32Error(errno);
                        BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwTotalBytesWritten += dwBytesWritten;
                pRequestCursor += dwBytesWritten;
                dwRequestSize -= dwBytesWritten;
	}
	if (dwActualRequestSize != dwTotalBytesWritten){
		dwError = ERROR_IO_INCOMPLETE;
		BAIL_ON_VMAFD_ERROR(dwError);
	}
cleanup:
	return dwError;
error:
	goto cleanup;
}

VOID
VmAfdFreeConnectionImpl(
        PVM_AFD_CONNECTION pConnection
        )
{
        if (pConnection->fd >= 0){
                close(pConnection->fd);
        }
        VMAFD_SAFE_FREE_MEMORY (pConnection);
}

BOOLEAN
VmAfdCheckIfServerIsUp(
      PCWSTR pwszNetworkAddress,
      DWORD  dwPort
      )
{
    DWORD dwError = 0;
    DWORD dwNumFDs = 0;
    int sockfd = -1;
    fd_set fdset;
    PSTR pszNetworkAddress = NULL;
    PSTR pszPort = NULL;
    BOOLEAN bServerIsUp = FALSE;
    struct addrinfo* pHostInfo = NULL;
    struct addrinfo hints = {0};
    struct timeval tv = {0};

    if (IsNullOrEmptyString(pwszNetworkAddress) || !dwPort)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringAFromW(
                                pwszNetworkAddress,
                                &pszNetworkAddress
                                );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                                      &pszPort,
                                      "%d",
                                      dwPort
                                      );
    BAIL_ON_VMAFD_ERROR(dwError);


    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    dwError = getaddrinfo(
                      pszNetworkAddress,
                      pszPort,
                      &hints,
                      &pHostInfo);
    BAIL_ON_VMAFD_ERROR(dwError);

    sockfd = socket( pHostInfo->ai_family, SOCK_STREAM, pHostInfo->ai_protocol);

    if (sockfd == -1)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (connect( sockfd, pHostInfo->ai_addr, pHostInfo->ai_addrlen) == -1)
    {
        if (errno != EINPROGRESS)
        {
            dwError = LwErrnoToWin32Error(errno);
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        FD_ZERO(&fdset);
        FD_SET(sockfd, &fdset);
        tv.tv_sec = RPC_PING_TIMEOUT;

        dwNumFDs = select(sockfd+1, NULL, &fdset, NULL, &tv);

        if (dwNumFDs == -1)
        {
            dwError = LwErrnoToWin32Error(errno);
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        if (dwNumFDs > 0)
        {
            int iSocketError = 0;
            socklen_t slen = sizeof(iSocketError);
            if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &iSocketError, &slen) == -1)
            {
                dwError = LwErrnoToWin32Error(errno);
                BAIL_ON_VMAFD_ERROR(dwError);
            }
            if (iSocketError)
            {
                dwError = LwErrnoToWin32Error(iSocketError);
                BAIL_ON_VMAFD_ERROR(dwError);
            }

            bServerIsUp = TRUE;
        }
    }
    else
    {
        bServerIsUp = TRUE;
    }

cleanup:

    if (sockfd != -1)
    {
        close(sockfd);
    }
    if (pHostInfo)
    {
        freeaddrinfo(pHostInfo);
    }

    VMAFD_SAFE_FREE_MEMORY(pszNetworkAddress);
    VMAFD_SAFE_FREE_MEMORY(pszPort);

    return bServerIsUp;
error:

    bServerIsUp = FALSE;
    goto cleanup;
}
