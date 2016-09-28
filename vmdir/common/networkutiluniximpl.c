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
VmDirOpenServerConnectionImpl(
	PVM_DIR_CONNECTION *ppConnection
	)
{
	DWORD dwError = 0;
	int socket_fd = -1, on = 1;
	struct sockaddr_un address = {0};
	PVM_DIR_CONNECTION pConnection = NULL;

	socket_fd = socket(PF_UNIX,SOCK_STREAM, 0);

	if (socket_fd < 0){
		dwError = LwErrnoToWin32Error(errno);
		BAIL_ON_VMDIR_ERROR(dwError);
	}

	if( setsockopt( socket_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) ) < 0){
		dwError = LwErrnoToWin32Error(errno);
		BAIL_ON_VMDIR_ERROR(dwError);
	}

	unlink (SOCKET_FILE_PATH);

	address.sun_family = AF_UNIX;
	snprintf (address.sun_path, sizeof(SOCKET_FILE_PATH), SOCKET_FILE_PATH);

	if (bind (socket_fd, (struct sockaddr *)&address, sizeof(struct sockaddr_un)) < 0){
		dwError = LwErrnoToWin32Error(errno);
		BAIL_ON_VMDIR_ERROR(dwError);
	}

	if (listen (socket_fd, 5) < 0){
		dwError = LwErrnoToWin32Error(errno);
		BAIL_ON_VMDIR_ERROR(dwError);
	}

	dwError = VmDirAllocateMemory(sizeof(VM_DIR_CONNECTION), (PVOID *)&pConnection);
	BAIL_ON_VMDIR_ERROR(dwError);
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
	VMDIR_SAFE_FREE_MEMORY (pConnection);
	goto cleanup;
}

VOID
VmDirCloseServerConnectionImpl(
	PVM_DIR_CONNECTION pConnection
	)
{
	if (pConnection->fd >= 0 ){
	  close (pConnection->fd);
          pConnection->fd = -1;
	}
}

VOID
VmDirShutdownServerConnectionImpl(
	PVM_DIR_CONNECTION pConnection
	)
{
        VmDirCloseServerConnectionImpl(pConnection);

}


DWORD
VmDirOpenClientConnectionImpl(
	PVM_DIR_CONNECTION *ppConnection
	)
{
	DWORD dwError = 0;
	int socket_fd = 0;
	struct sockaddr_un address;
	PVM_DIR_CONNECTION pConnection = NULL;

	socket_fd = socket (PF_UNIX, SOCK_STREAM, 0);
	if (socket_fd < 0){
		dwError = LwErrnoToWin32Error(errno);
		BAIL_ON_VMDIR_ERROR(dwError);
	}
	memset (&address, 0, sizeof(struct sockaddr_un));
	address.sun_family = AF_UNIX;
	snprintf (address.sun_path, sizeof(SOCKET_FILE_PATH), SOCKET_FILE_PATH);

	if (connect(socket_fd, (struct sockaddr *) &address, sizeof(struct sockaddr_un)) <0)
    {
        if (errno == ENOENT || errno == ECONNREFUSED)
        {
            dwError = VMDIR_ERROR_CANNOT_CONNECT_VMDIR;
        }
        else if (errno == EACCES)
        {
            dwError = ERROR_ACCESS_DENIED;
        }
        else
        {
            dwError = LwErrnoToWin32Error (errno);
        }
        BAIL_ON_VMDIR_ERROR(dwError);
    }

	dwError = VmDirAllocateMemory (sizeof(VM_DIR_CONNECTION),(PVOID *)&pConnection);
	BAIL_ON_VMDIR_ERROR(dwError);
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
	VMDIR_SAFE_FREE_MEMORY(pConnection);
	goto cleanup;
}

VOID
VmDirCloseClientConnectionImpl(
	PVM_DIR_CONNECTION pConnection
	)
{
	if (pConnection->fd > -1){
	    close(pConnection->fd);
	    pConnection->fd = -1;
	}
}

DWORD
VmDirAcceptConnectionImpl(
  PVM_DIR_CONNECTION pConnection,
  PVM_DIR_CONNECTION *ppConnection
  )
{
	DWORD dwError = 0;
	PVM_DIR_CONNECTION pTempConnection = NULL;
	int connection_fd = 0;
	struct sockaddr_un address = {0};
	socklen_t address_length = 0;

	connection_fd = accept(pConnection->fd,(struct sockaddr *)&address, &address_length);
	if (connection_fd < 0){
		dwError = LwErrnoToWin32Error(errno);
		BAIL_ON_VMDIR_ERROR(dwError);
	}

	dwError = VmDirAllocateMemory(sizeof(VM_DIR_CONNECTION),(PVOID *)&pTempConnection);
	BAIL_ON_VMDIR_ERROR(dwError);

	pTempConnection->fd = connection_fd;
	*ppConnection = pTempConnection;
cleanup:
	return dwError;
error:
	if (pTempConnection){
		VMDIR_SAFE_FREE_MEMORY(pTempConnection);
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
VmDirReadDataImpl(
	PVM_DIR_CONNECTION pConnection,
	PBYTE *ppResponse,
	PDWORD pdwResponseSize
	)
{
	DWORD dwError = 0;
	ssize_t dwBytesRead = 0;
	PBYTE pResponse = NULL;
        PBYTE pResponseCursor = NULL;
        DWORD dwBytesSent = 0;
        DWORD dwTotalBytesRead = 0;

        do
        {
                dwBytesRead = read(pConnection->fd, (PVOID)&dwBytesSent, sizeof (DWORD));
        }while (dwBytesRead == -1 && errno == EINTR);

        if (dwBytesRead < sizeof (DWORD))
        {
                dwError = LwErrnoToWin32Error(errno);
                BAIL_ON_VMDIR_ERROR (dwError);
        }

        dwBytesRead = 0;

        dwError = VmDirAllocateMemory(
                                      dwBytesSent,
                                      (PVOID *) &pResponse
                                     );
        BAIL_ON_VMDIR_ERROR (dwError);

        pResponseCursor = pResponse;

        while (dwTotalBytesRead < dwBytesSent)
        {
                DWORD dwBytesToRead = VMDIR_IPC_PACKET_SIZE<(dwBytesSent-dwTotalBytesRead)?
                                      VMDIR_IPC_PACKET_SIZE:
                                      dwBytesSent-dwTotalBytesRead;
                do {
                        dwBytesRead = read(pConnection->fd,pResponseCursor,dwBytesToRead);
                }while (dwBytesRead == -1 && errno == EINTR);

                if (dwBytesRead == -1)
                {
                        dwError = LwErrnoToWin32Error(errno);
                        BAIL_ON_VMDIR_ERROR (dwError);
                }

                dwTotalBytesRead += dwBytesRead;
                pResponseCursor += dwBytesRead;

        }

	if (dwTotalBytesRead < dwBytesSent){
		dwError = ERROR_IO_INCOMPLETE;
		BAIL_ON_VMDIR_ERROR(dwError);
	}
	*pdwResponseSize = dwBytesRead;
	*ppResponse = pResponse;

cleanup:
	return dwError;
error:
	if (ppResponse != NULL){
	*ppResponse = NULL;
	}
	if (pdwResponseSize != NULL){
		*pdwResponseSize = 0;
	}
	VMDIR_SAFE_FREE_MEMORY(pResponse);
	goto cleanup;

}

DWORD
VmDirWriteDataImpl(
	PVM_DIR_CONNECTION pConnection,
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
                BAIL_ON_VMDIR_ERROR (dwError);
        }

        dwBytesWritten = 0;

        while (dwRequestSize > 0)
        {
                DWORD dwBytesToWrite = VMDIR_IPC_PACKET_SIZE<dwRequestSize?VMDIR_IPC_PACKET_SIZE:dwRequestSize;

                do
                {
                    dwBytesWritten = write(pConnection->fd,pRequestCursor,dwBytesToWrite);
                }while (dwBytesWritten == -1 && errno == EINTR);

                if (dwBytesWritten == -1)
                {
                        dwError = LwErrnoToWin32Error(errno);
                        BAIL_ON_VMDIR_ERROR(dwError);
                }

                dwTotalBytesWritten += dwBytesWritten;
                pRequestCursor += dwBytesWritten;
                dwRequestSize -= dwBytesWritten;
	}
	if (dwActualRequestSize != dwTotalBytesWritten){
		dwError = ERROR_IO_INCOMPLETE;
		BAIL_ON_VMDIR_ERROR(dwError);
	}
cleanup:
	return dwError;
error:
	goto cleanup;

}

VOID
VmDirFreeConnectionImpl(
        PVM_DIR_CONNECTION pConnection
        )
{
        if (pConnection->fd >= 0){
                close(pConnection->fd);
        }
        VMDIR_SAFE_FREE_MEMORY (pConnection);
}
