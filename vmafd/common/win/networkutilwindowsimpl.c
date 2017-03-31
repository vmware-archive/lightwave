/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
VmAfdOpenServerConnectionImpl(
                              PVM_AFD_CONNECTION *ppConnection
                              )
{
    DWORD dwError = 0;
    PVM_AFD_CONNECTION pConnection = NULL;
    HANDLE hPipe = INVALID_HANDLE_VALUE;
    LPTSTR lpszPipename = TEXT(NAME_OF_PIPE);
    SECURITY_ATTRIBUTES saEveryOneAccess = {0};
    SECURITY_DESCRIPTOR sdEveryoneSID = {0};

    if (!InitializeSecurityDescriptor (
                    &sdEveryoneSID,
                    SECURITY_DESCRIPTOR_REVISION
                    ))
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!SetSecurityDescriptorDacl(
                    &sdEveryoneSID,
                    TRUE, //IsDaclPresent
                    NULL, //A NULL DACL grants access to everyone
                    FALSE //Don't Use a default DACL
                    ))
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    saEveryOneAccess.nLength = sizeof (SECURITY_ATTRIBUTES);
    saEveryOneAccess.lpSecurityDescriptor = &sdEveryoneSID;
    saEveryOneAccess.bInheritHandle = FALSE;

    hPipe = CreateNamedPipe(
                    lpszPipename,             // pipe name
                    PIPE_ACCESS_DUPLEX,       // read/write access
                    PIPE_TYPE_MESSAGE |       // message type pipe
                    PIPE_READMODE_MESSAGE |   // message-read mode
                    PIPE_WAIT |
                    PIPE_REJECT_REMOTE_CLIENTS, // blocking mode
                    PIPE_UNLIMITED_INSTANCES, // max. instances
                    BUFSIZ,                  // output buffer size
                    BUFSIZ,                  // input buffer size
                    0,                        // client time-out
                    &saEveryOneAccess
                    );
    if (hPipe == INVALID_HANDLE_VALUE){
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    dwError = VmAfdAllocateMemory (sizeof(VM_AFD_CONNECTION), (PVOID *)&pConnection);
    BAIL_ON_VMAFD_ERROR(dwError);
    pConnection->hConnection = hPipe;
    *ppConnection = pConnection;
cleanup:
    return dwError;
error:
    if (ppConnection != NULL){
        *ppConnection = NULL;
    }
    if (hPipe != INVALID_HANDLE_VALUE){
        FlushFileBuffers(hPipe);
        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
        hPipe = INVALID_HANDLE_VALUE;
    }
    VMAFD_SAFE_FREE_MEMORY (pConnection);
    goto cleanup;
}

VOID
VmAfdCloseServerConnectionImpl(
                               PVM_AFD_CONNECTION pConnection
                               )
{
    if (pConnection->hConnection != INVALID_HANDLE_VALUE){
        FlushFileBuffers(pConnection->hConnection);
        DisconnectNamedPipe(pConnection->hConnection);
        CloseHandle(pConnection->hConnection);
        pConnection->hConnection = INVALID_HANDLE_VALUE;
    }
    pConnection = NULL;
}

DWORD
VmAfdOpenClientConnectionImpl(
	PVM_AFD_CONNECTION *ppConnection
	)
{
	DWORD dwError = 0;
	HANDLE hPipe = INVALID_HANDLE_VALUE;
        DWORD dwRetryCounter = 0;
        PVM_AFD_CONNECTION pConnection = NULL;
	LPTSTR lpszPipename = TEXT(NAME_OF_PIPE);

        while (1)
        {

                hPipe = CreateFile(
                                   lpszPipename,
                                   GENERIC_READ|GENERIC_WRITE,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                                   NULL,
                                   OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,
                                   NULL
                                  );
                if (hPipe != INVALID_HANDLE_VALUE)
                {
                        dwError = 0;
                        break;
                }

                dwError = GetLastError();
                if (dwError == ERROR_FILE_NOT_FOUND)
                {
                        dwError = ERROR_CANNOT_CONNECT_VMAFD;
                        if (PIPE_CLIENT_RETRY_COUNT == dwRetryCounter++)
                        {
                                BAIL_ON_VMAFD_ERROR (dwError);
                        }

                        Sleep(PIPE_TIMEOUT_INTERVAL);
                        continue;
                }
                else if (dwError != ERROR_PIPE_BUSY)
                {
                        BAIL_ON_VMAFD_ERROR (dwError);
                }

                if (! WaitNamedPipe (
                                     lpszPipename,
                                     PIPE_TIMEOUT_INTERVAL //client timeout
                                    )
                       )
                {
                     dwError = GetLastError();
                     if (dwError == ERROR_FILE_NOT_FOUND)
                     {
                         dwError = ERROR_CANNOT_CONNECT_VMAFD;
                     }

                     if (PIPE_CLIENT_RETRY_COUNT == dwRetryCounter++)
                     {
                          BAIL_ON_VMAFD_ERROR (dwError);
                     }
                }
	}
	dwError = VmAfdAllocateMemory (sizeof(VM_AFD_CONNECTION),(PVOID *)&pConnection);
	BAIL_ON_VMAFD_ERROR (dwError);
	pConnection->hConnection = hPipe;
        *ppConnection = pConnection;
cleanup:
	return dwError;
error:
	if (ppConnection != NULL){
		*ppConnection = NULL;
	}
        if (hPipe != INVALID_HANDLE_VALUE){
                CloseHandle(hPipe);
                hPipe = INVALID_HANDLE_VALUE;
        }
        VMAFD_SAFE_FREE_MEMORY (pConnection);
	goto cleanup;
}

VOID
VmAfdCloseClientConnectionImpl(
	PVM_AFD_CONNECTION pConnection
	)
{
	if (pConnection ->hConnection != INVALID_HANDLE_VALUE){
		CloseHandle(pConnection->hConnection);
		pConnection->hConnection = INVALID_HANDLE_VALUE;
	}
	pConnection = NULL;
}

DWORD
VmAfdAcceptConnectionImpl(
	PVM_AFD_CONNECTION pConnection,
	PVM_AFD_CONNECTION *ppConnection
	)
{
	DWORD dwError = 0;
	BOOL bConnected = FALSE;
	bConnected = ConnectNamedPipe(pConnection->hConnection,NULL);
	if (!bConnected)
        {
		dwError = GetLastError();
		BAIL_ON_VMAFD_ERROR (dwError);
	}
cleanup:
	return dwError;
error:
        if (dwError == ERROR_PIPE_CONNECTED)
        {
                dwError = 0;
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
    BOOL bResult;
    PBYTE pResponse = NULL;
    DWORD dwBytesRead = 0;
    DWORD dwBytesSent = 0;
    DWORD dwTotalBytesRead = 0;
    PBYTE pResponseCursor = NULL;

    bResult = ReadFile(
                    pConnection->hConnection,
                    (PBYTE)&dwBytesSent,
                    sizeof (DWORD),
                    &dwBytesRead,
                    NULL);

    if (!bResult || dwBytesRead == 0)
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR_NO_LOG (dwError);
    }

    if (!dwBytesSent)
    {
        dwError = ERROR_NO_DATA;
        BAIL_ON_VMAFD_ERROR_NO_LOG (dwError);
    }

    dwError = VmAfdAllocateMemory(
                        dwBytesSent,
                        (PVOID *)&pResponse);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

    pResponseCursor = pResponse;

    while (dwTotalBytesRead < dwBytesSent)
    {
        DWORD dwBytesToRead = dwBytesSent - dwTotalBytesRead;
        dwBytesToRead = dwBytesToRead > VMAFD_IPC_PACKET_SIZE? VMAFD_IPC_PACKET_SIZE: dwBytesToRead;

        bResult = ReadFile(
                        pConnection->hConnection,
                        pResponseCursor,
                        dwBytesToRead,
                        &dwBytesRead,
                        NULL);
        if (!bResult || dwBytesRead == 0)
        {
            dwError = GetLastError();
            if (dwError != ERROR_MORE_DATA)
            {
                BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);
            }
            dwError = 0;
        }

        dwTotalBytesRead += dwBytesRead;
        pResponseCursor += dwBytesRead;
    }

    if (dwTotalBytesRead < dwBytesSent)
    {
            dwError = ERROR_IO_INCOMPLETE;
            BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);
    }
    *pdwResponseSize = dwBytesRead;
    *ppResponse = pResponse;

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
    VMAFD_SAFE_FREE_MEMORY (pResponse);
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
	BOOL bResult;
	DWORD dwTotalBytesWritten = 0;
        DWORD dwBytesWritten = 0;
        DWORD dwActualRequestSize = dwRequestSize;
        PBYTE pRequestCursor = pRequest;

        bResult = WriteFile(pConnection->hConnection, (PBYTE)&dwRequestSize, sizeof (DWORD), &dwBytesWritten, NULL);

        if (!bResult)
        {
                dwError = GetLastError();
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        dwBytesWritten = 0;

        while(dwRequestSize > 0)
        {
                DWORD dwBytesToWrite = VMAFD_IPC_PACKET_SIZE<dwRequestSize?VMAFD_IPC_PACKET_SIZE:dwRequestSize;

                bResult = WriteFile(pConnection->hConnection,pRequest,dwBytesToWrite,&dwBytesWritten,NULL);
                if (!bResult)
                {
                        dwError = GetLastError();
                        BAIL_ON_VMAFD_ERROR(dwError);
                }

                dwTotalBytesWritten += dwBytesWritten;
                pRequestCursor += dwBytesWritten;
                dwRequestSize -= dwBytesWritten;
        }

	if (dwActualRequestSize != dwBytesWritten){
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
	if (pConnection->hConnection != INVALID_HANDLE_VALUE){
	    CloseHandle(pConnection->hConnection);
		pConnection->hConnection = INVALID_HANDLE_VALUE;
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
    BOOLEAN bServerIsUp = FALSE;
    SOCKET sockfd = INVALID_SOCKET;
    WSADATA wsaData;
    DWORD dwNonBlockMode = 1;
    fd_set fdset;
    PSTR pszNetworkAddress = NULL;
    PSTR pszPort = NULL;
    struct addrinfo* pHostInfo = NULL;
    struct addrinfo hints = {0};
    struct timeval tv;

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

    dwError = WSAStartup(MAKEWORD(2, 2), &wsaData);
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

    if (sockfd == INVALID_SOCKET)
    {
        dwError = WSAGetLastError();
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (ioctlsocket(sockfd, FIONBIO, &dwNonBlockMode) == SOCKET_ERROR )
    {
        dwError = WSAGetLastError();
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (connect(sockfd, pHostInfo->ai_addr, pHostInfo->ai_addrlen) == SOCKET_ERROR)
    {
        dwError = WSAGetLastError();
        if (dwError != WSAEINPROGRESS && dwError != WSAEWOULDBLOCK)
        {
          BAIL_ON_VMAFD_ERROR(dwError);
        }

        FD_ZERO(&fdset);
        FD_SET(sockfd, &fdset);
        tv.tv_sec = RPC_PING_TIMEOUT;

        dwNumFDs = select(0, NULL, &fdset, NULL, &tv);

        if (dwNumFDs > 0)
        {
            int iSocketError = 0;
            socklen_t slen = sizeof(iSocketError);

            if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &iSocketError, &slen) == SOCKET_ERROR)
            {
                dwError = WSAGetLastError();
                BAIL_ON_VMAFD_ERROR(dwError);
            }

            if (iSocketError)
            {
                dwError = iSocketError;
                BAIL_ON_VMAFD_ERROR(dwError);
            }

            bServerIsUp = TRUE;
        }
        else if (dwNumFDs < 0)
        {
            dwError = WSAGetLastError();
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }
    else
    {
        bServerIsUp = TRUE;
    }

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszNetworkAddress);
    VMAFD_SAFE_FREE_MEMORY(pszPort);

    if (sockfd != INVALID_SOCKET)
    {
        closesocket(sockfd);
    }
    WSACleanup();

    return bServerIsUp;
error:

    bServerIsUp = FALSE;
    goto cleanup;
}
