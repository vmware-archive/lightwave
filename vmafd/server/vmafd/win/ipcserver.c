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

#define BUFSIZE 512

static
PVOID
VmAfdIpcListen(
    PVOID pData
    );

static
PVOID
WindowsInstanceThread(
	PVOID handle
	);

DWORD
VmAfdIpcServerInit(
	)
{
	DWORD dwError = 0;
        dwError = pthread_create (
                        &gVmafdGlobals.pIPCServerThread,
                        NULL,
                        &VmAfdIpcListen,
                        NULL
                        );
        BAIL_ON_VMAFD_ERROR (dwError);
cleanup:
        VmAfdLog (VMAFD_DEBUG_DEBUG, "Exiting VmAfdIpcServerInit :%d", dwError);
	return dwError;
error:
        VmAfdLog (VMAFD_DEBUG_DEBUG, "Error in VmAfdIpcServerInit : %d", dwError);
	goto cleanup;

}

VOID
VmAfdIpcServerShutDown(
    VOID
    )
{
        DWORD dwError = 0;
        PVM_AFD_CONNECTION pConnection = NULL;
        pthread_mutex_lock (&gVmafdGlobals.mutexConnection);
        pConnection = gVmafdGlobals.pConnection;
        gVmafdGlobals.pConnection = NULL;
        pthread_mutex_unlock (&gVmafdGlobals.mutexConnection);
        if (pConnection){
                VmAfdFreeServerConnection (pConnection);
        }
}

static
PVOID
VmAfdIpcListen(
               PVOID pData
               )
{
    DWORD dwError = 0;
    PVM_AFD_CONNECTION pConnection = NULL;
    pthread_t windowsInstanceThreadHandler;
    pthread_attr_t windowsInstanceThreadHandlerAttr;

    dwError = pthread_attr_init (&windowsInstanceThreadHandlerAttr);
    BAIL_ON_VMAFD_ERROR (dwError);



    // The main loop creates an instance of the named pipe and
    // then waits for a client to connect to it. When the client
    // connects, a thread is created to handle communications
    // with that client, and this loop is free to wait for the
    // next client connect request. It is an infinite loop.

    for (;;)
    {

        dwError = VmAfdOpenServerConnection(&pConnection);
        BAIL_ON_VMAFD_ERROR (dwError);
        // Wait for the client to connect; if it succeeds,
        // the function returns a nonzero value. If the function
        // returns zero, GetLastError returns ERROR_PIPE_CONNECTED.
        pthread_mutex_lock (&gVmafdGlobals.mutexConnection);
        gVmafdGlobals.pConnection = pConnection;
        pthread_mutex_unlock (&gVmafdGlobals.mutexConnection);
        dwError = VmAfdAcceptConnection(pConnection, NULL);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = pthread_attr_setdetachstate (
            &windowsInstanceThreadHandlerAttr,
            PTHREAD_CREATE_DETACHED
            );
        BAIL_ON_VMAFD_ERROR (dwError);
        // Create a thread for this client
        dwError = pthread_create(
            &windowsInstanceThreadHandler,
            &windowsInstanceThreadHandlerAttr,
            WindowsInstanceThread,    // thread proc
            (PVOID) pConnection    // thread parameter
            );
        pConnection = NULL;
        BAIL_ON_VMAFD_ERROR (dwError);
    }
cleanup:
    pthread_attr_destroy (&windowsInstanceThreadHandlerAttr);
    VmAfdLog (VMAFD_DEBUG_DEBUG, "Exiting VmAfdIpcListen with code : %d", dwError);
    return NULL;
error:
    goto cleanup;

}

VOID
VmAfdCloseConnectionContext(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext
    )
{
    if (pConnectionContext){
        if (pConnectionContext->pStoreHandle)
        {
            VmAfdLog (VMAFD_DEBUG_ANY, "Process end the session with orphaned store handle");
            (DWORD)VecsSrvCloseCertStoreHandle (
                                    pConnectionContext->pStoreHandle,
                                    pConnectionContext
                                    );
        }

        VmAfdFreeServerConnection (pConnectionContext->pConnection);


        VmAfdFreeConnectionContext(pConnectionContext);
    }
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
    PVM_AFD_CONNECTION pConnection = NULL;
    PBYTE pchRequest = NULL;
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext = NULL;

    // Do some extra error checking since the app will keep running even if this
    // thread fails.

    if (handle == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    // Print verbose messages. In production code, this should be for debugging only.
    VmAfdLog (VMAFD_DEBUG_DEBUG, "InstanceThread created, ");
    // The thread's parameter is a handle to a pipe object instance.

    pConnection = (PVM_AFD_CONNECTION) handle;

    // Loop until done reading
    while (1)
    {
        VMAFD_SAFE_FREE_MEMORY (pchRequest);
        VMAFD_SAFE_FREE_MEMORY (ppResponse);
        // Read client requests from the pipe. This simplistic code only allows messages
        // up to BUFSIZE characters in length.
        dwError = VmAfdReadData(pConnection,&pchRequest,&dwRequestSize);
        if (dwError == ERROR_BROKEN_PIPE)
        {
            dwError = 0;
            break;
        }
        BAIL_ON_VMAFD_ERROR(dwError);

        if (!pConnectionContext)
        {
            dwError = VmAfdInitializeConnectionContext(
                                pConnection,
                                &pConnectionContext);
            BAIL_ON_VMAFD_ERROR (dwError);
        }

        // Process the incoming message.
        VmAfdLog (VMAFD_DEBUG_DEBUG, "InstanceThread Read success!");
        VmAfdLog (VMAFD_DEBUG_DEBUG, "Bytes Received (%d)", dwRequestSize);
        dwError = VmAfdLocalAPIHandler(
                                pConnectionContext,
                                pchRequest,
                                dwRequestSize,
                                &ppResponse,
                                &pdwResponseSize
                                );
        //dump (ppResponse, pdwResponseSize);

        // Write the reply to the pipe.
        dwError = VmAfdWriteData(pConnection,ppResponse,pdwResponseSize) ;
        BAIL_ON_VMAFD_ERROR(dwError);
        VmAfdLog (VMAFD_DEBUG_DEBUG, "InstanceThread Write success!");
    }
    // Flush the pipe to allow the client to read the pipe's contents
    // before disconnecting. Then disconnect the pipe, and close the
    // handle to this pipe instance.
cleanup:
    VMAFD_SAFE_FREE_MEMORY(pchRequest);
    VMAFD_SAFE_FREE_MEMORY(ppResponse);

    VmAfdCloseConnectionContext(pConnectionContext);

    VmAfdLog(VMAFD_DEBUG_DEBUG,"InstanceThread exitting.");
    return NULL;
error:
    VmAfdLog  (VMAFD_DEBUG_DEBUG,"Something went wrong. Exiting with error code:(%d)", dwError);
    goto cleanup;
}
