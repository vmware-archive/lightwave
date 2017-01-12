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
VmDirInitializeSecurityContext(
    PVM_DIR_CONNECTION pConnection,
    PVM_DIR_SECURITY_CONTEXT *ppSecurityContext
    )
{
    DWORD dwError  = 0 ;
    PVM_DIR_SECURITY_CONTEXT pSecurityContext = NULL;

    if (ppSecurityContext == NULL){
      dwError = ERROR_INVALID_PARAMETER ;
      BAIL_ON_VMDIR_ERROR (dwError);
    }

    if (pConnection == NULL){
      dwError = ERROR_INVALID_PARAMETER;
      BAIL_ON_VMDIR_ERROR (dwError);
    }

    dwError = VmDirInitializeSecurityContextImpl(
                    pConnection,
                    &pSecurityContext);
    BAIL_ON_VMDIR_ERROR (dwError);

    *ppSecurityContext = pSecurityContext;
cleanup:
    return dwError;
error:
    if (ppSecurityContext != NULL){
      *ppSecurityContext = NULL;
    }

    if (pSecurityContext)
    {
        VmDirFreeSecurityContext (pSecurityContext);
    }
    goto cleanup;
}


VOID
VmDirFreeSecurityContext(
    PVM_DIR_SECURITY_CONTEXT pSecurityContext
    )
{
    if (pSecurityContext){
     VmDirFreeSecurityContextImpl(pSecurityContext);
    }
}


DWORD
VmDirGetSecurityContextSize (
        PVM_DIR_SECURITY_CONTEXT pSecurityContext,
        PDWORD pdwSize
        )
{
    DWORD dwError = 0;
    DWORD dwSize = 0;
    if (!pSecurityContext || !pdwSize)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    dwError = VmDirGetSecurityContextSizeImpl(
                                pSecurityContext,
                                &dwSize
                                );
    BAIL_ON_VMDIR_ERROR (dwError);

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
VmDirEncodeSecurityContext (
        PVM_DIR_SECURITY_CONTEXT pSecurityContext,
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
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    dwError = VmDirEncodeSecurityContextImpl (
                            pSecurityContext,
                            pByteSecurityContext,
                            dwBufSize,
                            &dwBuffUsed
                            );
    BAIL_ON_VMDIR_ERROR (dwError);

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
VmDirDecodeSecurityContext (
        PBYTE pByteSecurityContext,
        DWORD dwBufSize,
        PVM_DIR_SECURITY_CONTEXT *ppSecurityContext
        )
{
    DWORD dwError = 0;
    PVM_DIR_SECURITY_CONTEXT pSecurityContext = NULL;

    if (!pByteSecurityContext ||
        !ppSecurityContext
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    dwError = VmDirDecodeSecurityContextImpl (
                          pByteSecurityContext,
                          dwBufSize,
                          &pSecurityContext
                          );
    BAIL_ON_VMDIR_ERROR (dwError);

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
        VmDirFreeSecurityContext (pSecurityContext);
    }

    goto cleanup;

}

BOOL
VmDirIsRootSecurityContext (
        PVM_DIR_SECURITY_CONTEXT pSecurityContext
        )
{
    BOOL bIsRoot = FALSE;
    DWORD dwError = 0;

    if (!pSecurityContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    bIsRoot = VmDirIsRootSecurityContextImpl (
                          pSecurityContext
                          );

error:
    if (dwError)
    {
        return FALSE;
    }
    return bIsRoot;
}

BOOL
VmDirEqualsSecurityContext (
      PVM_DIR_SECURITY_CONTEXT pSecurityContext1,
      PVM_DIR_SECURITY_CONTEXT pSecurityContext2
      )
{
    DWORD dwError = 0;
    BOOL bEqualsContext = FALSE;

    if (!pSecurityContext1 ||
        !pSecurityContext2
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    bEqualsContext = VmDirEqualsSecurityContextImpl(
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
VmDirAllocateContextFromName (
    PCWSTR pszAccountName,
    PVM_DIR_SECURITY_CONTEXT *ppSecurityContext
    )
{
    DWORD dwError = 0;
    PVM_DIR_SECURITY_CONTEXT pSecurityContext = NULL;

    if (IsNullOrEmptyString(pszAccountName) ||
        !ppSecurityContext
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    dwError = VmDirAllocateContextFromNameImpl (
                                pszAccountName,
                                &pSecurityContext
                                );
    BAIL_ON_VMDIR_ERROR (dwError);

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
        VmDirFreeSecurityContext (pSecurityContext);
    }

    goto cleanup;
}

DWORD
VmDirCopySecurityContext (
                          PVM_DIR_SECURITY_CONTEXT pSecurityContextSrc,
                          PVM_DIR_SECURITY_CONTEXT *ppSecurityContextDst
                          )
{
    DWORD dwError = 0;

    PVM_DIR_SECURITY_CONTEXT pSecurityContext = NULL;

    if (!pSecurityContextSrc ||
        !ppSecurityContextDst
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    dwError = VmDirCopySecurityContextImpl(
                                                pSecurityContextSrc,
                                                &pSecurityContext
                                               );
    BAIL_ON_VMDIR_ERROR (dwError);

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
        VmDirFreeSecurityContext (pSecurityContext);
    }

    goto cleanup;
}

DWORD
VmDirCreateRootSecurityContext (
    PVM_DIR_SECURITY_CONTEXT *ppSecurityContext
    )
{
    DWORD dwError = 0;

    PVM_DIR_SECURITY_CONTEXT pSecurityContext = NULL;

    if (!ppSecurityContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    dwError = VmDirCreateRootSecurityContextImpl(
                                            &pSecurityContext
                                            );

    BAIL_ON_VMDIR_ERROR (dwError);

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
        VmDirFreeSecurityContext(pSecurityContext);
    }

    goto cleanup;
}

DWORD
VmDirCreateWellKnownContext (
      VM_DIR_CONTEXT_TYPE contextType,
      PVM_DIR_SECURITY_CONTEXT *ppSecurityContext
      )
{
    DWORD dwError = 0;

    PVM_DIR_SECURITY_CONTEXT pSecurityContext = NULL;

    if (!ppSecurityContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    dwError = VmDirCreateWellKnownContextImpl (
                                              contextType,
                                              &pSecurityContext
                                              );
    BAIL_ON_VMDIR_ERROR (dwError);

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
        VmDirFreeSecurityContext (pSecurityContext);
    }

    goto cleanup;
}


BOOL
VmDirContextBelongsToGroup (
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PVM_DIR_SECURITY_CONTEXT pSecurityContextGroup
    )
{
    BOOL bResult = false;

    if (!pSecurityContext ||
        !pSecurityContextGroup
       )
    {
        goto error;
    }

    bResult = VmDirContextBelongsToGroupImpl(
                                          pSecurityContext,
                                          pSecurityContextGroup
                                          );

error:
    return bResult;
}

DWORD
VmDirAllocateNameFromContext (
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
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
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    dwError = VmDirAllocateNameFromContextImpl(
                                pSecurityContext,
                                &pszAccountName
                                );
    BAIL_ON_VMDIR_ERROR (dwError);

    *ppszAccountName = pszAccountName;

cleanup:
    return dwError;

error:
    if (ppszAccountName)
    {
        *ppszAccountName = NULL;
    }
    VMDIR_SAFE_FREE_MEMORY (pszAccountName);

    goto cleanup;
}
