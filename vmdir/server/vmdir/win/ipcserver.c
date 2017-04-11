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

#define BUFSIZE 512

static
PVOID
VmDirIpcListen(
    PVOID pData
    );

static
PVOID
WindowsInstanceThread(
    PVOID handle
    );

DWORD
VmDirIpcServerInit(
    )
{
    DWORD dwError = 0;
        dwError = pthread_create (
                        &gVmdirGlobals.pIPCServerThread,
                        NULL,
                        &VmDirIpcListen,
                        NULL
                        );
        BAIL_ON_VMDIR_ERROR (dwError);
cleanup:
    return dwError;
error:
    goto cleanup;

}

VOID
VmDirIpcServerShutDown(
    VOID
    )
{
        PVM_DIR_CONNECTION pConnection = NULL;
        BOOLEAN bInLock = FALSE;

    if (gVmdirGlobals.pMutexIPCConnection)
    {
        VMDIR_LOCK_MUTEX(bInLock, gVmdirGlobals.pMutexIPCConnection);
        pConnection = gVmdirGlobals.pConnection;
        gVmdirGlobals.pConnection = NULL;
        VMDIR_UNLOCK_MUTEX(bInLock, gVmdirGlobals.pMutexIPCConnection);
        if (pConnection)
        {
            VmDirShutdownServerConnection(pConnection);
        }
    }
}

static
PVOID
VmDirIpcListen(
    PVOID pData
    )
{
    DWORD dwError = 0;
    BOOLEAN bInLock = FALSE;
    PVM_DIR_CONNECTION pConnection = NULL;
    pthread_t windowsInstanceThreadHandler;
    pthread_attr_t windowsInstanceThreadHandlerAttr;


    // The main loop creates an instance of the named pipe and
    // then waits for a client to connect to it. When the client
    // connects, a thread is created to handle communications
    // with that client, and this loop is free to wait for the
    // next client connect request. It is an infinite loop.

    for (;;)
    {

        dwError = VmDirOpenServerConnection(&pConnection);
        BAIL_ON_VMDIR_ERROR (dwError);
        // Wait for the client to connect; if it succeeds,
        // the function returns a nonzero value. If the function
        // returns zero, GetLastError returns ERROR_PIPE_CONNECTED.
        VMDIR_LOCK_MUTEX(bInLock, gVmdirGlobals.pMutexIPCConnection);
        gVmdirGlobals.pConnection = pConnection;
        VMDIR_UNLOCK_MUTEX(bInLock, gVmdirGlobals.pMutexIPCConnection);
        dwError = VmDirAcceptConnection(pConnection, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = pthread_attr_init (&windowsInstanceThreadHandlerAttr);
        BAIL_ON_VMDIR_ERROR (dwError);
        dwError = pthread_attr_setdetachstate (
                                &windowsInstanceThreadHandlerAttr,
                                PTHREAD_CREATE_DETACHED
                                );
        BAIL_ON_VMDIR_ERROR (dwError);
        // Create a thread for this client
        dwError = pthread_create(
        &windowsInstanceThreadHandler,
                        &windowsInstanceThreadHandlerAttr,
        WindowsInstanceThread,    // thread proc
        (PVOID) pConnection    // thread parameter
                        );
        pConnection = NULL;
        BAIL_ON_VMDIR_ERROR (dwError);
    }
cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirGlobals.pMutexIPCConnection);
    return NULL;
error:
    goto cleanup;

}


static
PVOID
WindowsInstanceThread(
    PVOID handle
	)
// This routine is a thread processing function to read from and reply to a client
// via the open pipe connection passed from the main loop. Note this allows
// the main loop to continue executing, potentially creating more threads of
// of this procedure to run concurrently, depending on the number of incoming
// client connections.
{

	DWORD dwError = 0;
	DWORD dwRequestSize = 0;
	PBYTE ppResponse = NULL;
	DWORD pdwResponseSize = 0;
	PVM_DIR_CONNECTION pConnection = NULL;
	PBYTE pchRequest = NULL;
        PVM_DIR_SECURITY_CONTEXT pSecurityContext = NULL;

	// Do some extra error checking since the app will keep running even if this
	// thread fails.

	if (handle == NULL)
	{
      dwError = ERROR_INVALID_PARAMETER;
		BAIL_ON_VMDIR_ERROR (dwError);
	}

	dwError = VmDirAllocateMemory(BUFSIZ, &pchRequest);
	BAIL_ON_VMDIR_ERROR(dwError);

	// Print verbose messages. In production code, this should be for debugging only.
	// The thread's parameter is a handle to a pipe object instance.

	pConnection = (PVM_DIR_CONNECTION) handle;

	// Loop until done reading
	while (1)
	{
		// Read client requests from the pipe. This simplistic code only allows messages
		// up to BUFSIZE characters in length.
		dwError = VmDirReadData(pConnection,&pchRequest,&dwRequestSize);
		if (dwError == ERROR_BROKEN_PIPE){
			dwError = 0;
			break;
		}
		BAIL_ON_VMDIR_ERROR(dwError);

                dwError = VmDirInitializeSecurityContext(pConnection, &pSecurityContext);
                BAIL_ON_VMDIR_ERROR (dwError);
		// Process the incoming message.
		dwError = VmDirLocalAPIHandler(
                                                pSecurityContext,
						pchRequest,
						dwRequestSize,
						&ppResponse,
						&pdwResponseSize
						);
		//dump (ppResponse, pdwResponseSize);

		// Write the reply to the pipe.
		dwError = VmDirWriteData(pConnection,ppResponse,pdwResponseSize) ;
		BAIL_ON_VMDIR_ERROR(dwError);
	}
	// Flush the pipe to allow the client to read the pipe's contents
	// before disconnecting. Then disconnect the pipe, and close the
	// handle to this pipe instance.
cleanup:
	VMDIR_SAFE_FREE_MEMORY(pchRequest);
	VMDIR_SAFE_FREE_MEMORY(ppResponse);
        VmDirFreeServerConnection (pConnection);
        if (pSecurityContext){
          VmDirFreeSecurityContext(pSecurityContext);
        }
	return NULL;
error:
	goto cleanup;
}
