/*
 * Copyright © 2012-2017 VMware, Inc.  All Rights Reserved.
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
VmDirOpenServerConnectionImpl(
	PVM_DIR_CONNECTION *ppConnection
	)
{
	DWORD dwError = 0;
        PVM_DIR_CONNECTION pConnection = NULL;
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	LPTSTR lpszPipename = TEXT(NAME_OF_PIPE);
        SECURITY_ATTRIBUTES saEveryOneAccess = {0};
        SECURITY_DESCRIPTOR sdEveryoneSID = {0};

        if (!InitializeSecurityDescriptor (
                                           &sdEveryoneSID,
                                           SECURITY_DESCRIPTOR_REVISION
                                          )
           )
        {
                dwError = GetLastError();
                BAIL_ON_VMDIR_ERROR (dwError);
        }

        if (!SetSecurityDescriptorDacl(
                                        &sdEveryoneSID,
                                        TRUE, //IsDaclPresent
                                        NULL, //A NULL DACL grants access to everyone
                                        FALSE //Don't Use a default DACL
                                      )
           )
        {
                dwError = GetLastError();
                BAIL_ON_VMDIR_ERROR (dwError);
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
		BAIL_ON_VMDIR_ERROR(dwError);
	}
	dwError = VmDirAllocateMemory (sizeof(VM_DIR_CONNECTION), (PVOID *)&pConnection);
	BAIL_ON_VMDIR_ERROR(dwError);
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
        VMDIR_SAFE_FREE_MEMORY (pConnection);
	goto cleanup;
}

VOID
VmDirCloseServerConnectionImpl(
	PVM_DIR_CONNECTION pConnection
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

VOID
VmDirShutdownServerConnectionImpl(
	PVM_DIR_CONNECTION pConnection
	)
{
	if (pConnection->hConnection != INVALID_HANDLE_VALUE){
                PVM_DIR_CONNECTION pClientConnection = NULL;
                DWORD dwEndData = ERROR_GRACEFUL_DISCONNECT;

                VmDirOpenClientConnection(&pClientConnection);
                if (pClientConnection)
                {
                        VmDirWriteData (pClientConnection, (PBYTE)&dwEndData, sizeof (DWORD));
                }
                VmDirCloseServerConnection(pConnection);
                VmDirCloseClientConnection (pClientConnection);
	}
	pConnection = NULL;
}


DWORD
VmDirOpenClientConnectionImpl(
	PVM_DIR_CONNECTION *ppConnection
	)
{
	DWORD dwError = 0;
	HANDLE hPipe = INVALID_HANDLE_VALUE;
        DWORD dwRetryCounter = 0;
        PVM_DIR_CONNECTION pConnection = NULL;
	LPTSTR lpszPipename = TEXT(NAME_OF_PIPE);

        while (1)
        {
                hPipe = CreateFile(
                                   lpszPipename,
                                   GENERIC_READ|GENERIC_WRITE, // only need read access
                                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                                   NULL,
                                   OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,
                                   NULL
                                  );
                if (hPipe != INVALID_HANDLE_VALUE){
                        break;
                }

		dwError = GetLastError();
                if (dwError == ERROR_FILE_NOT_FOUND)
                {
                        if (PIPE_CLIENT_RETRY_COUNT == dwRetryCounter++)
                        {
                                BAIL_ON_VMDIR_ERROR (dwError);
                        }

                        Sleep(PIPE_TIMEOUT_INTERVAL);
                        continue;
                }

                if (dwError != ERROR_PIPE_BUSY)
                {
                        BAIL_ON_VMDIR_ERROR (dwError);
                }

                if (! WaitNamedPipe (
                                     lpszPipename,
                                     PIPE_TIMEOUT_INTERVAL //client timeout
                                    )
                       )
                {
                     dwError = GetLastError();
                     if (PIPE_CLIENT_RETRY_COUNT == dwRetryCounter++)
                     {
                          BAIL_ON_VMDIR_ERROR (dwError);
                     }
                }
	}
	dwError = VmDirAllocateMemory (sizeof(VM_DIR_CONNECTION),(PVOID *)&pConnection);
	BAIL_ON_VMDIR_ERROR (dwError);
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
        VMDIR_SAFE_FREE_MEMORY (pConnection);
	goto cleanup;
}

VOID
VmDirCloseClientConnectionImpl(
	PVM_DIR_CONNECTION pConnection
	)
{
	if (pConnection ->hConnection != INVALID_HANDLE_VALUE){
		CloseHandle(pConnection->hConnection);
		pConnection->hConnection = INVALID_HANDLE_VALUE;
	}
	pConnection = NULL;
}

DWORD
VmDirAcceptConnectionImpl(
	PVM_DIR_CONNECTION pConnection,
	PVM_DIR_CONNECTION *ppConnection
	)
{
	DWORD dwError = 0;
	BOOL bConnected = FALSE;
	bConnected = ConnectNamedPipe(pConnection->hConnection,NULL);
	if (!bConnected)
        {
		dwError = GetLastError();
		BAIL_ON_VMDIR_ERROR (dwError);
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
VmDirReadDataImpl(
	PVM_DIR_CONNECTION pConnection,
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
                           NULL
                          );

        if (!bResult || dwBytesRead == 0)
        {
                dwError = GetLastError();
                BAIL_ON_VMDIR_ERROR (dwError);
        }

        if (!dwBytesSent)
        {
                dwError = ERROR_NO_DATA;
                BAIL_ON_VMDIR_ERROR (dwError);
        }

	dwError = VmDirAllocateMemory(
                                      dwBytesSent,
                                      (PVOID *)&pResponse);
	BAIL_ON_VMDIR_ERROR(dwError);

        pResponseCursor = pResponse;

        while (dwTotalBytesRead < dwBytesSent)
        {
                DWORD dwBytesToRead = dwBytesSent - dwTotalBytesRead;
                dwBytesToRead = dwBytesToRead > VMDIR_IPC_PACKET_SIZE? VMDIR_IPC_PACKET_SIZE: dwBytesToRead;

                bResult = ReadFile(
                           pConnection->hConnection,
                           pResponseCursor,
                           dwBytesToRead,
                           &dwBytesRead,
                           NULL
                          );
                if (!bResult || dwBytesRead == 0)
                {
                        dwError = GetLastError();
                        if (dwError != ERROR_MORE_DATA)
                        {
                          BAIL_ON_VMDIR_ERROR(dwError);
                        }
                        dwError = 0;
                }

                dwTotalBytesRead += dwBytesRead;
                pResponseCursor += dwBytesRead;
        }

        if (dwTotalBytesRead < dwBytesSent)
        {
                dwError = ERROR_IO_INCOMPLETE;
                BAIL_ON_VMDIR_ERROR (dwError);
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
        VMDIR_SAFE_FREE_MEMORY (pResponse);
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
	BOOL bResult;
	DWORD dwTotalBytesWritten = 0;
        DWORD dwBytesWritten = 0;
        DWORD dwActualRequestSize = dwRequestSize;
        PBYTE pRequestCursor = pRequest;

        bResult = WriteFile(pConnection->hConnection, (PBYTE)&dwRequestSize, sizeof (DWORD), &dwBytesWritten, NULL);

        if (!bResult)
        {
                dwError = GetLastError();
                BAIL_ON_VMDIR_ERROR (dwError);
        }

        dwBytesWritten = 0;

        while(dwRequestSize > 0)
        {
                DWORD dwBytesToWrite = VMDIR_IPC_PACKET_SIZE<dwRequestSize?VMDIR_IPC_PACKET_SIZE:dwRequestSize;

                bResult = WriteFile(pConnection->hConnection,pRequest,dwBytesToWrite,&dwBytesWritten,NULL);
                if (!bResult)
                {
                        dwError = GetLastError();
                        BAIL_ON_VMDIR_ERROR(dwError);
                }

                dwTotalBytesWritten += dwBytesWritten;
                pRequestCursor += dwBytesWritten;
                dwRequestSize -= dwBytesWritten;
        }

	if (dwActualRequestSize != dwBytesWritten){
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
	if (pConnection->hConnection != INVALID_HANDLE_VALUE){
	    CloseHandle(pConnection->hConnection);
		pConnection->hConnection = INVALID_HANDLE_VALUE;
	}
	VMDIR_SAFE_FREE_MEMORY (pConnection);
}
