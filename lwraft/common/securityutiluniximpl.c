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
VmDirInitializeSecurityContextImpl(
    PVM_DIR_CONNECTION pConnection,
    PVM_DIR_SECURITY_CONTEXT *ppSecurityContext
    )
{
    DWORD dwError = 0;
    PVM_DIR_SECURITY_CONTEXT pSecurityContext = NULL;
#ifndef __MACH__
    struct ucred credentials = {0};
    int credLength = sizeof (struct ucred);
    int sockopt = SO_PEERCRED;
#else
    struct xucred credentials = {0};
    int credLength = sizeof (struct xucred);
    int sockopt = LOCAL_PEERCRED;
#endif
 
    if ((getsockopt (
            pConnection->fd,
            SOL_SOCKET,
            sockopt,
            &credentials,
            &credLength)) < 0){
      dwError = LwErrnoToWin32Error (errno);
      BAIL_ON_VMDIR_ERROR (dwError);
    }
    dwError = VmDirAllocateMemory(
                    sizeof (VM_DIR_SECURITY_CONTEXT),
                    (PVOID *)&pSecurityContext);
    BAIL_ON_VMDIR_ERROR (dwError);

#ifndef __MACH__
    pSecurityContext->uid = credentials.uid;
#else
    pSecurityContext->uid = credentials.cr_uid;
#endif

    *ppSecurityContext = pSecurityContext;
cleanup:
    return dwError;
error:
    if (ppSecurityContext != NULL){
      *ppSecurityContext = NULL;
    }
    if (pSecurityContext){
      VmDirFreeSecurityContext(pSecurityContext);
    }
    goto cleanup;
}


VOID
VmDirFreeSecurityContextImpl(
    PVM_DIR_SECURITY_CONTEXT pSecurityContext
    )
{
    VMDIR_SAFE_FREE_MEMORY (pSecurityContext);
}

DWORD
VmDirGetSecurityContextSizeImpl (
      PVM_DIR_SECURITY_CONTEXT pSecurityContext,
      PDWORD pdwSize
    )
{
    DWORD dwError = 0;
    DWORD dwSize = 0;

    dwSize = sizeof (VM_DIR_SECURITY_CONTEXT);

    *pdwSize = dwSize;

    return dwError;
}


DWORD
VmDirEncodeSecurityContextImpl (
      PVM_DIR_SECURITY_CONTEXT pSecurityContext,
      PBYTE pByteBuffer,
      DWORD dwAvailableBuffSize,
      PDWORD pdwBuffUsed
      )
{
    DWORD dwError = 0;
    DWORD dwSecurityContextSize = 0;
    PVM_DIR_SECURITY_CONTEXT pSecurityContextDest = NULL;

    dwError = VmDirGetSecurityContextSize (
                    pSecurityContext,
                    &dwSecurityContextSize
                    );
    BAIL_ON_VMDIR_ERROR (dwError);

    if (dwSecurityContextSize > dwAvailableBuffSize)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    pSecurityContextDest = (PVM_DIR_SECURITY_CONTEXT)pByteBuffer;
    pSecurityContextDest->uid = pSecurityContext->uid;

    *pdwBuffUsed = dwSecurityContextSize;

cleanup:
    return dwError;

error:
    if (pdwBuffUsed){
      *pdwBuffUsed = 0;
    }

    goto cleanup;

}

DWORD
VmDirDecodeSecurityContextImpl (
      PBYTE pByteSecurityContext,
      DWORD dwBuffSize,
      PVM_DIR_SECURITY_CONTEXT *ppSecurityContext
      )
{
    DWORD dwError = 0;
    PVM_DIR_SECURITY_CONTEXT pSecurityContext = NULL;

    if (dwBuffSize < sizeof (VM_DIR_SECURITY_CONTEXT))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    dwError = VmDirAllocateMemory (
                      sizeof (VM_DIR_SECURITY_CONTEXT),
                      (PVOID *)&pSecurityContext
                      );
    BAIL_ON_VMDIR_ERROR (dwError);

    pSecurityContext->uid = ((PVM_DIR_SECURITY_CONTEXT)
                            pByteSecurityContext)->uid;

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
VmDirIsRootSecurityContextImpl (
        PVM_DIR_SECURITY_CONTEXT pSecurityContext
        )
{
    BOOL bIsRoot = FALSE;

    if (pSecurityContext->uid == 0 )
    {
        bIsRoot = TRUE;
    }

    return bIsRoot;
}

BOOL
VmDirEqualsSecurityContextImpl (
      PVM_DIR_SECURITY_CONTEXT pSecurityContext1,
      PVM_DIR_SECURITY_CONTEXT pSecurityContext2
      )
{
    BOOL bEqualsContext = FALSE;

    if (pSecurityContext2->uid == pSecurityContext1->uid)
    {
        bEqualsContext = TRUE;
    }

    return bEqualsContext;
}


DWORD
VmDirAllocateContextFromNameImpl (
        PCWSTR pszAccountName,
        PVM_DIR_SECURITY_CONTEXT *ppSecurityContext
        )
{
    DWORD dwError = 0;
    struct passwd *pd = NULL;
    PSTR psazAccountName = NULL;
    PVM_DIR_SECURITY_CONTEXT pSecurityContext = NULL;

    dwError = VmDirAllocateStringAFromW (
                            pszAccountName,
                            &psazAccountName
                            );
    BAIL_ON_VMDIR_ERROR (dwError);

    pd = getpwnam (psazAccountName);
    if (pd == NULL)
    {
        dwError = LwErrnoToWin32Error (errno);
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    dwError = VmDirAllocateMemory (
                            sizeof (VM_DIR_SECURITY_CONTEXT),
                            (PVOID *)&pSecurityContext
                            );
    BAIL_ON_VMDIR_ERROR (dwError);


    pSecurityContext->uid = pd->pw_uid;

    *ppSecurityContext = pSecurityContext;

cleanup:
    VMDIR_SAFE_FREE_MEMORY (psazAccountName);

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
VmDirCopySecurityContextImpl (
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

    dwError = VmDirAllocateMemory (
                                    sizeof (VM_DIR_SECURITY_CONTEXT),
                                    (PVOID *) &pSecurityContext
                                  );
    BAIL_ON_VMDIR_ERROR (dwError);

    pSecurityContext->uid = pSecurityContextSrc->uid;

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
VmDirCreateRootSecurityContextImpl (
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

    dwError = VmDirAllocateMemory (
                      sizeof (VM_DIR_SECURITY_CONTEXT),
                      (PVOID *)&pSecurityContext
                      );
    BAIL_ON_VMDIR_ERROR (dwError);

    pSecurityContext->uid = 0;

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
VmDirCreateWellKnownContextImpl (
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

    dwError = VmDirAllocateMemory (
                                    sizeof (VM_DIR_SECURITY_CONTEXT),
                                    (PVOID *)&pSecurityContext
                                  );
    BAIL_ON_VMDIR_ERROR (dwError);

    switch (contextType)
    {
        case VM_DIR_CONTEXT_TYPE_ROOT:
          pSecurityContext->uid = 0;
          break;
        case VM_DIR_CONTEXT_TYPE_EVERYONE:
          pSecurityContext->uid = EVERYONE_UID;
          break;
        default:
          dwError = ERROR_INVALID_PARAMETER;
          break;
    }
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
VmDirContextBelongsToGroupImpl (
        PVM_DIR_SECURITY_CONTEXT pSecurityContext,
        PVM_DIR_SECURITY_CONTEXT pSecurityContextGroup
        )
{
    BOOL bResult = false;

    if (pSecurityContextGroup->uid == EVERYONE_UID)
    {
      bResult = true;
    }

    /*TODO: Processing for checking if context belongs to group
     */

//error:
    return bResult;
}

DWORD
VmDirGenRandomImpl (
        PDWORD pdwRandomNumber
        )
{
    DWORD dwError = 0;
    FILE *fpUrandom = NULL;

    DWORD dwRandomNumber = 0;
    DWORD dwBytesRead = 0;

    fpUrandom = fopen("/dev/urandom", "r");

    if (!fpUrandom)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    dwBytesRead = fread(
                        (VOID *)&dwRandomNumber,
                        sizeof (DWORD),
                        1,
                        fpUrandom
                       );
    if (dwBytesRead < sizeof (DWORD) &&
        !feof (fpUrandom)
       )
    {
        dwError = LwErrnoToWin32Error (ferror(fpUrandom));
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    *pdwRandomNumber = dwRandomNumber;

cleanup:

    if (fpUrandom)
    {
        fclose(fpUrandom);
    }
    return dwError;

error:
    *pdwRandomNumber = 0;

    goto cleanup;
}

DWORD
VmDirAllocateNameFromContextImpl (
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PWSTR *ppszAccountName
    )
{
    DWORD dwError = 0;

    PWSTR pszAccountName = NULL;

    WCHAR wszEveryone[] = GROUP_EVERYONE_W;

    if (pSecurityContext->uid == EVERYONE_UID)
    {
        dwError = VmDirAllocateStringW(
                                        wszEveryone,
                                        &pszAccountName
                                      );
        BAIL_ON_VMDIR_ERROR (dwError);
    }
    else
    {
        struct passwd *pd = NULL;
        pd = getpwuid(pSecurityContext->uid);

        if (pd)
        {
            dwError = VmDirAllocateStringWFromA(
                                                 pd->pw_name,
                                                 &pszAccountName
                                               );
            BAIL_ON_VMDIR_ERROR (dwError);
        }
    }

    *ppszAccountName = pszAccountName;

cleanup:
    return dwError ;

error:
    if (ppszAccountName)
    {
        *ppszAccountName = NULL;
    }
    VMDIR_SAFE_FREE_MEMORY (pszAccountName);

    goto cleanup;
}
