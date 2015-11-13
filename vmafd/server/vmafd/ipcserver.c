/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : ipcserver.c
 *
 * Abstract :
 *
 */
#include "includes.h"
static
PVOID
VmAfdIpcListen(
    PVOID pData
    );

static VOID
*unixInstanceThread (
    void *handle
    );


DWORD
VmAfdIpcServerInit (
    VOID
    )
{
  DWORD dwError = 0;
  int status = 0;
  PVM_AFD_CONNECTION pConnection = NULL;

  dwError = VmAfdOpenServerConnection(&pConnection);
  BAIL_ON_VMAFD_ERROR(dwError);
  pthread_mutex_lock(&gVmafdGlobals.mutexConnection);
  gVmafdGlobals.pConnection = pConnection;
  pConnection = NULL;
  pthread_mutex_unlock(&gVmafdGlobals.mutexConnection);
  VmAfdLog(VMAFD_DEBUG_DEBUG, "Domain Socket Connection Initialized");
  status = pthread_create (
                              &gVmafdGlobals.pIPCServerThread,
                              NULL,
                              &VmAfdIpcListen,
                              NULL
                            );
  dwError = LwErrnoToWin32Error(status);
  BAIL_ON_VMAFD_ERROR (dwError);
  VmAfdLog (VMAFD_DEBUG_DEBUG,"Server Listening on Domain Socket");
cleanup:
  return dwError;
error:
  if (pConnection){
    VmAfdFreeServerConnection (pConnection);
  }
  goto cleanup;
}

VOID
VmAfdIpcServerShutDown (
    VOID
    )
{
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
  int status = 0;
  PVM_AFD_CONNECTION pConnection = NULL;
  PVM_AFD_CONNECTION pClientConnection = NULL;
  pthread_t unixInstanceThreadHandler;
  pthread_attr_t unixInstanceThreadHandlerAttr;
  pthread_mutex_lock (&gVmafdGlobals.mutexConnection);
  pConnection = gVmafdGlobals.pConnection;
  pthread_mutex_unlock(&gVmafdGlobals.mutexConnection);
  if (pConnection == NULL){
    dwError = ERROR_INVALID_PARAMETER;
    BAIL_ON_VMAFD_ERROR (dwError);
  }
  while(!(dwError = VmAfdAcceptConnection(pConnection, &pClientConnection)))
  {
      status = pthread_attr_init(&unixInstanceThreadHandlerAttr);
      dwError = LwErrnoToWin32Error (status);
      BAIL_ON_VMAFD_ERROR (dwError);
      status = pthread_attr_setdetachstate(
                                &unixInstanceThreadHandlerAttr,
                                PTHREAD_CREATE_DETACHED
                                );
      dwError = LwErrnoToWin32Error (status);
      BAIL_ON_VMAFD_ERROR (dwError);
      status = pthread_create(
      &unixInstanceThreadHandler,
      &unixInstanceThreadHandlerAttr,
      unixInstanceThread,
      pClientConnection
      );
      dwError = LwErrnoToWin32Error (status);
      BAIL_ON_VMAFD_ERROR (dwError);
      pClientConnection = NULL;
  }
  BAIL_ON_VMAFD_ERROR (dwError);
cleanup:
  VmAfdFreeServerConnection (pClientConnection);
  VmAfdLog (VMAFD_DEBUG_DEBUG, "Exiting VmAfdIpcListen with code (%d)", dwError);
  return NULL;
error:
  VmAfdLog (VMAFD_DEBUG_DEBUG, "Error in VmAfdIpcListen : (%d)", dwError);
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
  PVM_AFD_CONNECTION_CONTEXT pConnectionContext = NULL;
  PVM_AFD_CONNECTION pConnection = NULL;

  if (handle == NULL){
      dwError = ERROR_INVALID_PARAMETER;
      VmAfdLog(VMAFD_DEBUG_DEBUG, "Null handle passed to the connection thread");
      BAIL_ON_VMAFD_ERROR (dwError);
  }

  pConnection = (PVM_AFD_CONNECTION)handle;

  dwError = VmAfdInitializeConnectionContext(pConnection, &pConnectionContext);
  BAIL_ON_VMAFD_ERROR (dwError);
  VmAfdLog (VMAFD_DEBUG_DEBUG, "unixInstanceThread: Security Context is initialized" );

  dwError = VmAfdReadData(pConnection, &pRequest, &dwRequestSize);
  BAIL_ON_VMAFD_ERROR(dwError);

  dwError = VmAfdLocalAPIHandler(
              pConnectionContext,
              pRequest,
              dwRequestSize,
              &ppResponse,
              &pdwResponseSize
              );
  BAIL_ON_VMAFD_ERROR (dwError);

  dwError = VmAfdWriteData(pConnection, ppResponse, pdwResponseSize);
  BAIL_ON_VMAFD_ERROR(dwError);
cleanup:
  if (pConnection){
    VmAfdFreeServerConnection (pConnection);
  }
  VMAFD_SAFE_FREE_MEMORY (pRequest);
  VMAFD_SAFE_FREE_MEMORY (ppResponse);
  if (pConnectionContext){
    VmAfdFreeConnectionContext(pConnectionContext);
  }
  VmAfdLog (VMAFD_DEBUG_DEBUG, "Exiting connection thread");
  return NULL;
error:
  VmAfdLog(VMAFD_DEBUG_DEBUG, "Exiting with error code (%d)", dwError);
  goto cleanup;
}
