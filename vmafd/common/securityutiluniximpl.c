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
VmAfdInitializeSecurityContextImpl(
    PVM_AFD_CONNECTION pConnection,
    PVM_AFD_SECURITY_CONTEXT *ppSecurityContext
    )
{
    DWORD dwError = 0;
#ifndef __MACH__
    struct ucred credentials = {0};
    int credLength = sizeof (struct ucred);
    int sockopt = SO_PEERCRED;
#else
    struct xucred credentials = {0};
    int credLength = sizeof (struct xucred);
    int sockopt = LOCAL_PEERCRED;
#endif

    PVM_AFD_SECURITY_CONTEXT pSecurityContext = NULL;
    if ((getsockopt (
            pConnection->fd,
            SOL_SOCKET,
            sockopt,
            &credentials,
            &credLength)) < 0){
      dwError = LwErrnoToWin32Error (errno);
      BAIL_ON_VMAFD_ERROR (dwError);
    }
    dwError = VmAfdAllocateMemory(
                    sizeof (VM_AFD_SECURITY_CONTEXT),
                    (PVOID *)&pSecurityContext);
    BAIL_ON_VMAFD_ERROR (dwError);
#ifndef __MACH__
    pSecurityContext->uid = credentials.uid;
    pConnection->pid = credentials.pid;
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
      VmAfdFreeSecurityContext(pSecurityContext);
    }
    goto cleanup;
}


VOID
VmAfdFreeSecurityContextImpl(
    PVM_AFD_SECURITY_CONTEXT pSecurityContext
    )
{
    VMAFD_SAFE_FREE_MEMORY (pSecurityContext);
}

DWORD
VmAfdGetSecurityContextSizeImpl (
      PVM_AFD_SECURITY_CONTEXT pSecurityContext,
      PDWORD pdwSize
    )
{
    DWORD dwError = 0;
    DWORD dwSize = 0;

    dwSize = sizeof (VM_AFD_SECURITY_CONTEXT);

    *pdwSize = dwSize;

    return dwError;
}


DWORD
VmAfdEncodeSecurityContextImpl (
      PVM_AFD_SECURITY_CONTEXT pSecurityContext,
      PBYTE pByteBuffer,
      DWORD dwAvailableBuffSize,
      PDWORD pdwBuffUsed
      )
{
    DWORD dwError = 0;
    DWORD dwSecurityContextSize = 0;
    PVM_AFD_SECURITY_CONTEXT pSecurityContextDest = NULL;

    dwError = VmAfdGetSecurityContextSize (
                    pSecurityContext,
                    &dwSecurityContextSize
                    );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (dwSecurityContextSize > dwAvailableBuffSize)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pSecurityContextDest = (PVM_AFD_SECURITY_CONTEXT)pByteBuffer;
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
VmAfdDecodeSecurityContextImpl (
      PBYTE pByteSecurityContext,
      DWORD dwBuffSize,
      PVM_AFD_SECURITY_CONTEXT *ppSecurityContext
      )
{
    DWORD dwError = 0;
    PVM_AFD_SECURITY_CONTEXT pSecurityContext = NULL;

    if (dwBuffSize < sizeof (VM_AFD_SECURITY_CONTEXT))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateMemory (
                      sizeof (VM_AFD_SECURITY_CONTEXT),
                      (PVOID *)&pSecurityContext
                      );
    BAIL_ON_VMAFD_ERROR (dwError);

    pSecurityContext->uid = ((PVM_AFD_SECURITY_CONTEXT)
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
        VmAfdFreeSecurityContext (pSecurityContext);
    }

    goto cleanup;

}


BOOL
VmAfdIsRootSecurityContextImpl (
        PVM_AFD_CONNECTION_CONTEXT pConnectionContext
        )
{
    BOOL bIsRoot = FALSE;

    if (pConnectionContext &&
        pConnectionContext->pSecurityContext &&
        pConnectionContext->pSecurityContext->uid == 0 )
    {
        bIsRoot = TRUE;
    }

    return bIsRoot;
}

BOOL
VmAfdEqualsSecurityContextImpl (
      PVM_AFD_SECURITY_CONTEXT pSecurityContext1,
      PVM_AFD_SECURITY_CONTEXT pSecurityContext2
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
VmAfdAllocateContextFromNameImpl (
        PCWSTR pszAccountName,
        PVM_AFD_SECURITY_CONTEXT *ppSecurityContext
        )
{
    DWORD dwError = 0;
    PSTR psazAccountName = NULL;
    PVM_AFD_SECURITY_CONTEXT pSecurityContext = NULL;


    PSTR pszBuffer = NULL;
    struct passwd pd = {0};
    struct passwd *pd_result = NULL;
    size_t szBufSize = 0;
    DWORD dwError1 = 0;


    dwError = VmAfdAllocateStringAFromW (
                            pszAccountName,
                            &psazAccountName
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

    szBufSize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (szBufSize == -1)
    {
        szBufSize = MAX_GWTPWR_BUF_LENGTH;
    }

    dwError = VmAfdAllocateMemory(szBufSize, (PVOID*)&pszBuffer);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError1 = getpwnam_r(
                    psazAccountName,
                    &pd,
                    pszBuffer,
                    szBufSize,
                    &pd_result
                    );

    if (dwError1)
    {
        dwError = LwErrnoToWin32Error(dwError1);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    if (!pd_result)
    {
        dwError = ERROR_NO_SUCH_USER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory (
                            sizeof (VM_AFD_SECURITY_CONTEXT),
                            (PVOID *)&pSecurityContext
                            );
    BAIL_ON_VMAFD_ERROR (dwError);


    pSecurityContext->uid = pd.pw_uid;

    *ppSecurityContext = pSecurityContext;

cleanup:
    VMAFD_SAFE_FREE_MEMORY (psazAccountName);
    VMAFD_SAFE_FREE_MEMORY (pszBuffer);

    return dwError;
error:
    if (ppSecurityContext)
    {
        *ppSecurityContext = NULL;
    }
    if (pSecurityContext)
    {
        VmAfdFreeSecurityContext(pSecurityContext);
    }

    goto cleanup;
}

DWORD
VmAfdCopySecurityContextImpl (
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

    dwError = VmAfdAllocateMemory (
                                    sizeof (VM_AFD_SECURITY_CONTEXT),
                                    (PVOID *) &pSecurityContext
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

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
        VmAfdFreeSecurityContext (pSecurityContext);
    }

    goto cleanup;
}

DWORD
VmAfdCreateAnonymousConnectionContextImpl(
    PVM_AFD_CONNECTION_CONTEXT *ppConnectionContext
    )
{
    DWORD dwError = 0;

    PVM_AFD_CONNECTION_CONTEXT pConnectionContext = NULL;
    PVM_AFD_SECURITY_CONTEXT pSecurityContext = NULL;

    if (ppConnectionContext == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateMemory (
                    sizeof(VM_AFD_CONNECTION_CONTEXT),
                    (PVOID *) &pConnectionContext
                    );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdAllocateMemory(
                    sizeof (VM_AFD_SECURITY_CONTEXT),
                    (PVOID *)&pSecurityContext);
    BAIL_ON_VMAFD_ERROR (dwError);

    pSecurityContext->uid = 0;

    pConnectionContext->pConnection = NULL;
    pConnectionContext->pSecurityContext = pSecurityContext;
    pConnectionContext->bAnonymousContext = TRUE;

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
        VmAfdFreeMemory(pConnectionContext);
    }

    goto cleanup;
}

DWORD
VmAfdCreateWellKnownContextImpl (
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

    dwError = VmAfdAllocateMemory (
                                    sizeof (VM_AFD_SECURITY_CONTEXT),
                                    (PVOID *)&pSecurityContext
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    switch (contextType)
    {
        case VM_AFD_CONTEXT_TYPE_ROOT:
          pSecurityContext->uid = 0;
          break;
        case VM_AFD_CONTEXT_TYPE_EVERYONE:
          pSecurityContext->uid = EVERYONE_UID;
          break;
        default:
          dwError = ERROR_INVALID_PARAMETER;
          break;
    }
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
VmAfdContextBelongsToGroupImpl (
        PVM_AFD_SECURITY_CONTEXT pSecurityContext,
        PVM_AFD_SECURITY_CONTEXT pSecurityContextGroup
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
VmAfdCheckAclContextImpl(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PSTR pszSddlAcl,
    BOOL *pbIsAllowed
    )
{
    DWORD dwError = 0;
    BOOL bIsAllowed = FALSE;
    GENERIC_MAPPING GenericMapping = {0};
    PACCESS_TOKEN pAccessToken = NULL;
    ACCESS_MASK accessDesired = KEY_READ;
    ACCESS_MASK accessGranted = 0;
    PSECURITY_DESCRIPTOR_RELATIVE pSDRel = NULL;
    PSECURITY_DESCRIPTOR_ABSOLUTE pSDAbs = NULL;
    DWORD dwSDRelSize = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(pConnectionContext, dwError);

    dwError = VmAfdCreateAccessTokenFromSecurityContext(
                  pConnectionContext->pSecurityContext,
                  &pAccessToken);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateSecurityDescriptorFromSddlCString(
                  &pSDRel,
                  &dwSDRelSize,
                  pszSddlAcl);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdSecurityAclSelfRelativeToAbsoluteSD(
                  &pSDAbs,
                  pSDRel);
    BAIL_ON_VMAFD_ERROR(dwError);

    bIsAllowed = VmAfdAclAccessCheck(
                  pSDAbs,
                  pAccessToken,
                  accessDesired,
                  0,
                  &GenericMapping,
                  &accessGranted,
                  &dwError);
    BAIL_ON_VMAFD_ERROR(dwError);

    *pbIsAllowed = bIsAllowed;

cleanup:
    VmAfdReleaseAccessToken(&pAccessToken);
    VmAfdFreeAbsoluteSecurityDescriptor(&pSDAbs);
    VMAFD_SAFE_FREE_MEMORY(pSDRel);
    return dwError;

error:
    goto cleanup;
}

DWORD
VmAfdGenRandomImpl (
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
        BAIL_ON_VMAFD_ERROR (dwError);
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
        BAIL_ON_VMAFD_ERROR (dwError);
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
VmAfdAllocateNameFromContextImpl (
    PVM_AFD_SECURITY_CONTEXT pSecurityContext,
    PWSTR *ppszAccountName
    )
{
    DWORD dwError = 0;

    PWSTR pszAccountName = NULL;

    WCHAR wszEveryone[] = GROUP_EVERYONE_W;

    PSTR pszBuffer = NULL;

    if (pSecurityContext->uid == EVERYONE_UID)
    {
        dwError = VmAfdAllocateStringW(
                                        wszEveryone,
                                        &pszAccountName
                                      );
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    else
    {
        struct passwd pd = {0};
        struct passwd *pd_result = NULL;
        size_t szBufSize = 0;
        DWORD dwError1 = 0;

        szBufSize = sysconf(_SC_GETPW_R_SIZE_MAX);
        if (szBufSize == -1)
        {
            szBufSize = MAX_GWTPWR_BUF_LENGTH;
        }

        dwError = VmAfdAllocateMemory(szBufSize, (PVOID*)&pszBuffer);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError1 = getpwuid_r(
                        pSecurityContext->uid,
                        &pd,
                        pszBuffer,
                        szBufSize,
                        &pd_result
                        );

        if (dwError1)
        {
            dwError = LwErrnoToWin32Error(dwError1);
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        if (pd_result)
        {
            dwError = VmAfdAllocateStringWFromA(
                                                 pd.pw_name,
                                                 &pszAccountName
                                               );
            BAIL_ON_VMAFD_ERROR (dwError);
        }
        else
        {
            dwError = ERROR_NO_SUCH_USER;
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }

    *ppszAccountName = pszAccountName;

cleanup:

    VMAFD_SAFE_FREE_MEMORY (pszBuffer);
    return dwError ;

error:
    if (ppszAccountName)
    {
        *ppszAccountName = NULL;
    }
    VMAFD_SAFE_FREE_MEMORY (pszAccountName);

    goto cleanup;
}
