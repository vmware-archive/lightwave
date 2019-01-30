/*
 * Copyright © 2019 VMware, Inc.  All Rights Reserved.
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
static
PVOID
_VmDirIpcListen(
    PVOID pData
    );

static
PVOID
_unixInstanceThread(
    PVOID   pHandle
    );

DWORD
VmDirIpcServerInit(
    VOID
    )
{
    DWORD               dwError = 0;
    BOOLEAN             bInLock = FALSE;
    int                 status = 0;
    PVM_DIR_CONNECTION  pConnection = NULL;

    dwError = VmDirOpenServerConnection(&pConnection, POST_MGR_SOCKET_FILE_PATH);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOCK_MUTEX(bInLock, gPostMgrGlobals.pMutexIPCConnection);

    gPostMgrGlobals.pConnection = pConnection;
    pConnection = NULL;

    VMDIR_UNLOCK_MUTEX(bInLock, gPostMgrGlobals.pMutexIPCConnection);

    status = pthread_create(
                           &gPostMgrGlobals.pIPCServerThread,
                           NULL,
                           &_VmDirIpcListen,
                           NULL
                           );
    dwError = LwErrnoToWin32Error(status);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gPostMgrGlobals.pMutexIPCConnection);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed with error (%u)", __FUNCTION__, dwError);
    if (pConnection)
    {
        VmDirFreeServerConnection(pConnection);
    }
    goto cleanup;
}

VOID
VmDirIpcServerShutDown(
    VOID
    )
{
    PVM_DIR_CONNECTION  pConnection = NULL;
    BOOLEAN             bInLock = FALSE;

    if (gPostMgrGlobals.pMutexIPCConnection)
    {
        VMDIR_LOCK_MUTEX(bInLock, gPostMgrGlobals.pMutexIPCConnection);

        pConnection = gPostMgrGlobals.pConnection;
        gPostMgrGlobals.pConnection = NULL;

        VMDIR_UNLOCK_MUTEX(bInLock, gPostMgrGlobals.pMutexIPCConnection);

        if (pConnection)
        {
            VmDirShutdownServerConnection(pConnection);
        }
    }
}

static
PVOID
_VmDirIpcListen(
    PVOID pData
    )
{
    DWORD               dwError = 0;
    BOOLEAN             bInLock = FALSE;
    int                 status = 0;
    PVM_DIR_CONNECTION  pConnection = NULL;
    PVM_DIR_CONNECTION  pClientConnection = NULL;
    pthread_t           unixInstanceThreadHandler;
    pthread_attr_t      unixInstanceThreadHandlerAttr;

    VMDIR_LOCK_MUTEX(bInLock, gPostMgrGlobals.pMutexIPCConnection);
    pConnection = gPostMgrGlobals.pConnection;
    VMDIR_UNLOCK_MUTEX(bInLock, gPostMgrGlobals.pMutexIPCConnection);

    if (pConnection == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    while (!(dwError = VmDirAcceptConnection(pConnection, &pClientConnection)))
    {
        status = pthread_attr_init(&unixInstanceThreadHandlerAttr);
        dwError = LwErrnoToWin32Error(status);
        BAIL_ON_VMDIR_ERROR(dwError);

        status = pthread_attr_setdetachstate(
                                &unixInstanceThreadHandlerAttr,
                                PTHREAD_CREATE_DETACHED
                                );
        dwError = LwErrnoToWin32Error(status);
        BAIL_ON_VMDIR_ERROR(dwError);

        status = pthread_create(
                &unixInstanceThreadHandler,
                &unixInstanceThreadHandlerAttr,
                _unixInstanceThread,
                pClientConnection
                );
        dwError = LwErrnoToWin32Error(status);
        BAIL_ON_VMDIR_ERROR(dwError);

        pClientConnection = NULL;
    }

    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gPostMgrGlobals.pMutexIPCConnection);
    VmDirFreeServerConnection(pClientConnection);
    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Exiting _VmDirIpcListen");
    return NULL;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed with error (%u)", __FUNCTION__, dwError);
    goto cleanup;
}

static
PVOID
_unixInstanceThread(
    PVOID   pHandle
    )
{
    DWORD                     dwError = 0;
    DWORD                     dwRequestSize = 0;
    DWORD                     pdwResponseSize = 0;
    PBYTE                     pRequest = NULL;
    PBYTE                     ppResponse = NULL;
    PVM_DIR_SECURITY_CONTEXT  pSecurityContext = NULL;
    PVM_DIR_CONNECTION        pConnection = NULL;

    if (pHandle == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pConnection = (PVM_DIR_CONNECTION) pHandle;

    dwError = VmDirInitializeSecurityContext(pConnection, &pSecurityContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirReadData(pConnection, &pRequest, &dwRequestSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLocalAPIHandler(
                  pSecurityContext,
                  pRequest,
                  dwRequestSize,
                  &ppResponse,
                  &pdwResponseSize
                  );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirWriteData(pConnection, ppResponse, pdwResponseSize);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pConnection)
    {
        VmDirFreeServerConnection(pConnection);
    }

    VMDIR_SAFE_FREE_MEMORY(pRequest);
    VMDIR_SAFE_FREE_MEMORY(ppResponse);

    if (pSecurityContext)
    {
        VmDirFreeSecurityContext(pSecurityContext);
    }
    return NULL;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed with error (%u)", __FUNCTION__, dwError);
    goto cleanup;
}
