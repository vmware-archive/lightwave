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
PVOID
VmDirIpcListen(
    PVOID pData
    );

static VOID
*unixInstanceThread (
    void *handle
    );


DWORD
VmDirIpcServerInit (
    VOID
    )
{
  DWORD dwError = 0;
  BOOLEAN bInLock = FALSE;
  int status = 0;
  PVM_DIR_CONNECTION pConnection = NULL;

  dwError = VmDirOpenServerConnection(&pConnection);
  BAIL_ON_VMDIR_ERROR(dwError);
  VMDIR_LOCK_MUTEX(bInLock, gVmdirGlobals.pMutexIPCConnection);
  gVmdirGlobals.pConnection = pConnection;
  pConnection = NULL;
  VMDIR_UNLOCK_MUTEX(bInLock, gVmdirGlobals.pMutexIPCConnection);
  status = pthread_create (
                              &gVmdirGlobals.pIPCServerThread,
                              NULL,
                              &VmDirIpcListen,
                              NULL
                            );
  dwError = LwErrnoToWin32Error(status);
  BAIL_ON_VMDIR_ERROR (dwError);
cleanup:
  VMDIR_UNLOCK_MUTEX(bInLock, gVmdirGlobals.pMutexIPCConnection);
  return dwError;
error:
  if (pConnection){
    VmDirFreeServerConnection (pConnection);
  }
  goto cleanup;
}

VOID
VmDirIpcServerShutDown (
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
  int status = 0;
  PVM_DIR_CONNECTION pConnection = NULL;
  PVM_DIR_CONNECTION pClientConnection = NULL;
  pthread_t unixInstanceThreadHandler;
  pthread_attr_t unixInstanceThreadHandlerAttr;
  VMDIR_LOCK_MUTEX(bInLock, gVmdirGlobals.pMutexIPCConnection);
  pConnection = gVmdirGlobals.pConnection;
  VMDIR_UNLOCK_MUTEX(bInLock, gVmdirGlobals.pMutexIPCConnection);
  if (pConnection == NULL){
    dwError = ERROR_INVALID_PARAMETER;
    BAIL_ON_VMDIR_ERROR (dwError);
  }
  while(!(dwError = VmDirAcceptConnection(pConnection, &pClientConnection)))
  {
      status = pthread_attr_init(&unixInstanceThreadHandlerAttr);
      dwError = LwErrnoToWin32Error (status);
      BAIL_ON_VMDIR_ERROR (dwError);
      status = pthread_attr_setdetachstate(
                                &unixInstanceThreadHandlerAttr,
                                PTHREAD_CREATE_DETACHED
                                );
      dwError = LwErrnoToWin32Error (status);
      BAIL_ON_VMDIR_ERROR (dwError);
      status = pthread_create(
      &unixInstanceThreadHandler,
      &unixInstanceThreadHandlerAttr,
      unixInstanceThread,
      pClientConnection
      );
      dwError = LwErrnoToWin32Error (status);
      BAIL_ON_VMDIR_ERROR (dwError);
      pClientConnection = NULL;
  }
  BAIL_ON_VMDIR_ERROR (dwError);
cleanup:
  VMDIR_UNLOCK_MUTEX(bInLock, gVmdirGlobals.pMutexIPCConnection);
  VmDirFreeServerConnection (pClientConnection);
  return NULL;
error:
  goto cleanup;
}


static
VOID *unixInstanceThread (void *handle)
{
  DWORD dwError = 0;
  DWORD dwRequestSize = 0;
  DWORD pdwResponseSize = 0;
  PBYTE pRequest = NULL;
  PBYTE ppResponse = NULL;
  PVM_DIR_SECURITY_CONTEXT pSecurityContext = NULL;
  PVM_DIR_CONNECTION pConnection = NULL;

  if (handle == NULL){
      dwError = ERROR_INVALID_PARAMETER;
      BAIL_ON_VMDIR_ERROR (dwError);
  }

  pConnection = (PVM_DIR_CONNECTION)handle;

  dwError = VmDirInitializeSecurityContext (pConnection, &pSecurityContext);
  BAIL_ON_VMDIR_ERROR (dwError);

  dwError = VmDirReadData(pConnection, &pRequest, &dwRequestSize);
  BAIL_ON_VMDIR_ERROR(dwError);

  dwError = VmDirLocalAPIHandler(
              pSecurityContext,
              pRequest,
              dwRequestSize,
              &ppResponse,
              &pdwResponseSize
              );
  BAIL_ON_VMDIR_ERROR (dwError);

  dwError = VmDirWriteData(pConnection, ppResponse, pdwResponseSize);
  BAIL_ON_VMDIR_ERROR(dwError);
cleanup:
  if (pConnection){
  VmDirFreeServerConnection (pConnection);
  }
  VMDIR_SAFE_FREE_MEMORY (pRequest);
  VMDIR_SAFE_FREE_MEMORY (ppResponse);
  if (pSecurityContext){
      VmDirFreeSecurityContext (pSecurityContext);
  }
  return NULL;
error:
  goto cleanup;
}
