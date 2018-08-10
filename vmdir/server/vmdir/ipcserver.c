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

static
DWORD
VmDirIpcListen(
    PVOID   pData
    );

static
PVOID
unixInstanceThread(
    PVOID   handle
    );


DWORD
VmDirIpcServerInit(
    VOID
    )
{
    DWORD   dwError = 0;
    gVmdirGlobals.bIPCShutdown = FALSE;

    dwError = VmDirOpenServerConnection(&gVmdirGlobals.pIPCConn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvThrInit(&gVmdirGlobals.pIPCSrvThrInfo, NULL, NULL, TRUE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateThread(
            &gVmdirGlobals.pIPCSrvThrInfo->tid,
            gVmdirGlobals.pIPCSrvThrInfo->bJoinThr,
            VmDirIpcListen,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    VmDirIpcServerShutDown();
    goto cleanup;
}

VOID
VmDirIpcServerShutDown(
    VOID
    )
{
    gVmdirGlobals.bIPCShutdown = TRUE;

    VmDirShutdownServerConnection(gVmdirGlobals.pIPCConn);
    gVmdirGlobals.pIPCConn = NULL;

    if (gVmdirGlobals.pIPCSrvThrInfo)
    {
        VmDirSrvThrShutdown(gVmdirGlobals.pIPCSrvThrInfo);
        gVmdirGlobals.pIPCSrvThrInfo = NULL;
    }

}

static
DWORD
VmDirIpcListen(
    PVOID   pData
    )
{
    DWORD                  dwError = 0;
    int                    status = 0;
    PVM_DIR_CONNECTION     pConnection = NULL;
    PVM_DIR_CONNECTION     pClientConnection = NULL;
    pthread_t              unixInstanceThreadHandler;
    pthread_attr_t         unixInstanceThreadHandlerAttr;

    pConnection = gVmdirGlobals.pIPCConn;

    while (TRUE)
    {
        dwError = VmDirAcceptConnection(pConnection, &pClientConnection);
        if (gVmdirGlobals.bIPCShutdown)
        {
            // shutdown initiated - ignore dwError and exit
            goto cleanup;
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        status = pthread_attr_init(&unixInstanceThreadHandlerAttr);

        dwError = LwErrnoToWin32Error(status);
        BAIL_ON_VMDIR_ERROR(dwError);

        status = pthread_attr_setdetachstate(
                &unixInstanceThreadHandlerAttr, PTHREAD_CREATE_DETACHED);

        dwError = LwErrnoToWin32Error(status);
        BAIL_ON_VMDIR_ERROR(dwError);

        status = pthread_create(
                &unixInstanceThreadHandler,
                &unixInstanceThreadHandlerAttr,
                unixInstanceThread,
                pClientConnection);

        dwError = LwErrnoToWin32Error(status);
        BAIL_ON_VMDIR_ERROR(dwError);

        pClientConnection = NULL;
    }

cleanup:
    VmDirFreeServerConnection (pClientConnection);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

static
PVOID
unixInstanceThread(
    PVOID   handle
    )
{
    DWORD   dwError = 0;
    DWORD   dwRequestSize = 0;
    DWORD   pdwResponseSize = 0;
    PBYTE   pRequest = NULL;
    PBYTE   ppResponse = NULL;
    PVM_DIR_SECURITY_CONTEXT    pSecurityContext = NULL;
    PVM_DIR_CONNECTION          pConnection = NULL;

    if (handle == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pConnection = (PVM_DIR_CONNECTION)handle;

    dwError = VmDirInitializeSecurityContext(pConnection, &pSecurityContext);
    BAIL_ON_VMDIR_ERROR (dwError);

    dwError = VmDirReadData(pConnection, &pRequest, &dwRequestSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLocalAPIHandler(
            pSecurityContext,
            pRequest,
            dwRequestSize,
            &ppResponse,
            &pdwResponseSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirWriteData(pConnection, ppResponse, pdwResponseSize);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pRequest);
    VMDIR_SAFE_FREE_MEMORY(ppResponse);
    VmDirFreeServerConnection(pConnection);
    VmDirFreeSecurityContext(pSecurityContext);
    return NULL;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}
