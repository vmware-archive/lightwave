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


DWORD
VmAfdInitializeConnectionContext(
    PVM_AFD_CONNECTION pConnection,
    PVM_AFD_CONNECTION_CONTEXT *ppConnectionContext
    )
{
    DWORD dwError = 0;

    PVM_AFD_CONNECTION_CONTEXT pConnectionContext = NULL;
    PVM_AFD_SECURITY_CONTEXT pSecurityContext = NULL;

    if (pConnection == NULL ||
        ppConnectionContext == NULL){
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateMemory (
                    sizeof(VM_AFD_CONNECTION_CONTEXT),
                    (PVOID *) &pConnectionContext
                    );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdInitializeSecurityContextImpl(
                    pConnection,
                    &pSecurityContext
                    );
    BAIL_ON_VMAFD_ERROR (dwError);

    pConnectionContext->pConnection = pConnection;
    pConnectionContext->pSecurityContext = pSecurityContext;

    *ppConnectionContext = pConnectionContext;

cleanup:

    return dwError;
error:
    if (ppConnectionContext != NULL){
        *ppConnectionContext = NULL;
    }

    if (pSecurityContext){
        VmAfdFreeSecurityContext(pSecurityContext);
    }

    if (pConnectionContext){
        VmAfdFreeMemory(pSecurityContext);
    }

    goto cleanup;
}

VOID
VmAfdFreeConnectionContext(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext
    )
{
    if (pConnectionContext != NULL)
    {
        VmAfdFreeSecurityContext(pConnectionContext->pSecurityContext);
    }

    VMAFD_SAFE_FREE_MEMORY (pConnectionContext);
}

DWORD
VmAfdInitializeSecurityContext(
    PVM_AFD_CONNECTION pConnection,
    PVM_AFD_SECURITY_CONTEXT *ppSecurityContext
    )
{
    DWORD dwError  = 0 ;
    PVM_AFD_SECURITY_CONTEXT pSecurityContext = NULL;

    if (ppSecurityContext == NULL){
      dwError = ERROR_INVALID_PARAMETER ;
      BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = gIPCVtable.pfnInitializeSecurityContext(
                    pConnection,
                    &pSecurityContext);
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppSecurityContext = pSecurityContext;
cleanup:
    return dwError;
error:
    if (ppSecurityContext != NULL){
      *ppSecurityContext = NULL;
    }

    if (pSecurityContext)
    {
        VmAfdFreeSecurityContext (pSecurityContext);
    }
    goto cleanup;
}


VOID
VmAfdFreeSecurityContext(
    PVM_AFD_SECURITY_CONTEXT pSecurityContext
    )
{
    if (pSecurityContext){
     gIPCVtable.pfnFreeSecurityContext(pSecurityContext);
    }
}


DWORD
VmAfdGetSecurityContextSize (
        PVM_AFD_SECURITY_CONTEXT pSecurityContext,
        PDWORD pdwSize
        )
{
    DWORD dwError = 0;
    DWORD dwSize = 0;
    if (!pSecurityContext || !pdwSize)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = gIPCVtable.pfnGetSecurityContextSize(
                                pSecurityContext,
                                &dwSize
                                );
    BAIL_ON_VMAFD_ERROR (dwError);

    *pdwSize = dwSize;

cleanup:
    return dwError;
error:
    if (pdwSize)
    {
        *pdwSize = 0;
    }
    goto cleanup;
}

DWORD
VmAfdEncodeSecurityContext (
        PVM_AFD_SECURITY_CONTEXT pSecurityContext,
        PBYTE pByteSecurityContext,
        DWORD dwBufSize,
        PDWORD pdwBuffUsed
        )
{
    DWORD dwError = 0;
    DWORD dwBuffUsed = 0;

    if (!pSecurityContext ||
        !pByteSecurityContext ||
        !pdwBuffUsed
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = gIPCVtable.pfnEncodeSecurityContext (
                            pSecurityContext,
                            pByteSecurityContext,
                            dwBufSize,
                            &dwBuffUsed
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

    *pdwBuffUsed = dwBuffUsed;

cleanup:
    return dwError;
error:
    if (pdwBuffUsed)
    {
        *pdwBuffUsed = 0;
    }

    goto cleanup;
}

DWORD
VmAfdDecodeSecurityContext (
        PBYTE pByteSecurityContext,
        DWORD dwBufSize,
        PVM_AFD_SECURITY_CONTEXT *ppSecurityContext
        )
{
    DWORD dwError = 0;
    PVM_AFD_SECURITY_CONTEXT pSecurityContext = NULL;

    if (!pByteSecurityContext ||
        !ppSecurityContext
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = gIPCVtable.pfnDecodeSecurityContext (
                          pByteSecurityContext,
                          dwBufSize,
                          &pSecurityContext
                          );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppSecurityContext = pSecurityContext;
cleanup:
    return dwError;
error:
    if (ppSecurityContext)
    {
        *ppSecurityContext = NULL;
    }
    if (pSecurityContext)
    {
        VmAfdFreeSecurityContext (pSecurityContext);
    }

    goto cleanup;

}

BOOL
VmAfdIsRootSecurityContext (
        PVM_AFD_CONNECTION_CONTEXT pConnectionContext
        )
{
    BOOL bIsRoot = FALSE;
    DWORD dwError = 0;

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    bIsRoot = gIPCVtable.pfnIsRootSecurityContext (
                          pConnectionContext
                          );

error:
    if (dwError)
    {
        return FALSE;
    }
    return bIsRoot;
}

BOOL
VmAfdEqualsSecurityContext (
      PVM_AFD_SECURITY_CONTEXT pSecurityContext1,
      PVM_AFD_SECURITY_CONTEXT pSecurityContext2
      )
{
    DWORD dwError = 0;
    BOOL bEqualsContext = FALSE;

    if (!pSecurityContext1 ||
        !pSecurityContext2
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    bEqualsContext = gIPCVtable.pfnEqualsContext (
                              pSecurityContext1,
                              pSecurityContext2
                              );
error:
    if (dwError)
    {
        return FALSE;
    }
    return bEqualsContext;
}


DWORD
VmAfdAllocateContextFromName (
    PCWSTR pszAccountName,
    PVM_AFD_SECURITY_CONTEXT *ppSecurityContext
    )
{
    DWORD dwError = 0;
    PVM_AFD_SECURITY_CONTEXT pSecurityContext = NULL;

    if (IsNullOrEmptyString(pszAccountName) ||
        !ppSecurityContext
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = gIPCVtable.pfnAllocateContextFromName (
                                pszAccountName,
                                &pSecurityContext
                                );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppSecurityContext = pSecurityContext;

cleanup:
    return dwError;
error:
    if (ppSecurityContext)
    {
        *ppSecurityContext = NULL;
    }
    if (pSecurityContext)
    {
        VmAfdFreeSecurityContext (pSecurityContext);
    }

    goto cleanup;
}

DWORD
VmAfdCopySecurityContext (
                          PVM_AFD_SECURITY_CONTEXT pSecurityContextSrc,
                          PVM_AFD_SECURITY_CONTEXT *ppSecurityContextDst
                          )
{
    DWORD dwError = 0;

    PVM_AFD_SECURITY_CONTEXT pSecurityContext = NULL;

    if (!pSecurityContextSrc ||
        !ppSecurityContextDst
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = gIPCVtable.pfnCopySecurityContext(
                                                pSecurityContextSrc,
                                                &pSecurityContext
                                               );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppSecurityContextDst = pSecurityContext;

cleanup:
    return dwError;

error:
    if (ppSecurityContextDst)
    {
        *ppSecurityContextDst = NULL;
    }
    if (pSecurityContext)
    {
        VmAfdFreeSecurityContext (pSecurityContext);
    }

    goto cleanup;
}

DWORD
VmAfdCreateAnonymousConnectionContext (
    PVM_AFD_CONNECTION_CONTEXT *ppConnectionContext
    )
{
    DWORD dwError = 0;

    PVM_AFD_CONNECTION_CONTEXT pConnectionContext = NULL;

    if (!ppConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = gIPCVtable.pfnCreateAnonymousConnectionContext(
                                            &pConnectionContext
                                            );

    BAIL_ON_VMAFD_ERROR (dwError);

    *ppConnectionContext = pConnectionContext;

cleanup:
    return dwError;

error:
    if (ppConnectionContext)
    {
        *ppConnectionContext = NULL;
    }

    if (pConnectionContext)
    {
        VmAfdFreeConnectionContext(pConnectionContext);
    }

    goto cleanup;
}

DWORD
VmAfdCreateWellKnownContext (
      VM_AFD_CONTEXT_TYPE contextType,
      PVM_AFD_SECURITY_CONTEXT *ppSecurityContext
      )
{
    DWORD dwError = 0;

    PVM_AFD_SECURITY_CONTEXT pSecurityContext = NULL;

    if (!ppSecurityContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = gIPCVtable.pfnCreateWellKnownContext (
                                              contextType,
                                              &pSecurityContext
                                              );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppSecurityContext = pSecurityContext;

cleanup:
    return dwError;

error:
    if (ppSecurityContext)
    {
        *ppSecurityContext = NULL;
    }
    if (pSecurityContext)
    {
        VmAfdFreeSecurityContext (pSecurityContext);
    }

    goto cleanup;
}


BOOL
VmAfdContextBelongsToGroup (
    PVM_AFD_SECURITY_CONTEXT pSecurityContext,
    PVM_AFD_SECURITY_CONTEXT pSecurityContextGroup
    )
{
    BOOL bResult = false;

    if (!pSecurityContext ||
        !pSecurityContextGroup
       )
    {
        goto error;
    }

    bResult = gIPCVtable.pfnContextBelongsToGroup(
                                          pSecurityContext,
                                          pSecurityContextGroup
                                          );

error:
    return bResult;
}

DWORD
VmAfdCheckAclContext(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PSTR pszSddlAcl,
    BOOL *pbIsAllowed
    )
{
    DWORD dwError = 0;
    BOOL bIsAllowed = FALSE;

    BAIL_ON_VMAFD_INVALID_POINTER(pConnectionContext, dwError);

    dwError = gIPCVtable.pfnCheckAclContext(
                  pConnectionContext,
                  pszSddlAcl,
                  &bIsAllowed
              );
    BAIL_ON_VMAFD_ERROR(dwError);

    *pbIsAllowed = bIsAllowed;

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmAfdGenRandom(
                PDWORD pdwRandomNumber
              )
{
    DWORD dwError = 0;
    DWORD dwRandomNumber = 0;

    if (!pdwRandomNumber)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdGenRandomImpl(
                                 &dwRandomNumber
                                );
    BAIL_ON_VMAFD_ERROR (dwError);

    *pdwRandomNumber = dwRandomNumber;

cleanup:
    return dwError;

error:
    if (pdwRandomNumber)
    {
        *pdwRandomNumber = 0;
    }

    goto cleanup;
}

DWORD
VmAfdAllocateNameFromContext (
    PVM_AFD_SECURITY_CONTEXT pSecurityContext,
    PWSTR *ppszAccountName
    )
{
    DWORD dwError = 0;
    PWSTR pszAccountName = NULL;

    if (!pSecurityContext ||
        !ppszAccountName
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateNameFromContextImpl(
                                pSecurityContext,
                                &pszAccountName
                                );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppszAccountName = pszAccountName;

cleanup:
    return dwError;

error:
    if (ppszAccountName)
    {
        *ppszAccountName = NULL;
    }
    VMAFD_SAFE_FREE_MEMORY (pszAccountName);

    goto cleanup;
}
