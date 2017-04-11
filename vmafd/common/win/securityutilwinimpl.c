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

DWORD
VmAfdInitializeSecurityContextImpl(
    PVM_AFD_CONNECTION pConnection,
    PVM_AFD_SECURITY_CONTEXT *ppSecurityContext
    )
{
    DWORD dwError = 0;
    BOOL bResult = FALSE;
    BOOL bIsImpersonating = FALSE;
    PVM_AFD_SECURITY_CONTEXT pSecurityContext = NULL;
    PTOKEN_USER ptUser = NULL;
    HANDLE hToken = NULL;
    DWORD dwTokenSize = 0;
    DWORD dwSidLength = 0;
    DWORD dwPid = 0;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering VmAfdInitializeSecurityContext");

    bResult = ImpersonateNamedPipeClient (pConnection->hConnection);
    if (bResult == FALSE){
      dwError = GetLastError ();
      BAIL_ON_VMAFD_ERROR (dwError);
    }

    bIsImpersonating = TRUE;

    bResult = OpenThreadToken(
                    GetCurrentThread(),
                    TOKEN_ALL_ACCESS,
                    TRUE,
                    &hToken);
    if (bResult == FALSE){
      dwError = GetLastError ();
      BAIL_ON_VMAFD_ERROR (dwError);
    }
    dwError = VmAfdAllocateMemory(BUFSIZ, (PVOID *)&ptUser);
    BAIL_ON_VMAFD_ERROR (dwError);

    bResult = GetTokenInformation (
                    hToken,
                    TokenUser,
                    ptUser,
                    BUFSIZ,
                    &dwTokenSize);
    if (bResult == FALSE){
      dwError = GetLastError();
      BAIL_ON_VMAFD_ERROR (dwError);
    }
    dwError = VmAfdAllocateMemory (
                    sizeof(VM_AFD_SECURITY_CONTEXT),
                    (PVOID *) &pSecurityContext
                    );
    BAIL_ON_VMAFD_ERROR (dwError);
    bResult = IsValidSid(ptUser->User.Sid);
    if (bResult == FALSE){
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    dwSidLength = GetLengthSid(ptUser->User.Sid);
    dwError = VmAfdAllocateMemory(
                    dwSidLength,
                    (PVOID *)&(pSecurityContext->pSid)
                    );
    BAIL_ON_VMAFD_ERROR (dwError);
    bResult = CopySid(
                    dwSidLength,
                    pSecurityContext->pSid,
                    ptUser->User.Sid
                    );
    if (bResult == FALSE){
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (GetNamedPipeClientProcessId(pConnection->hConnection, &dwPid))
    {
        pConnection->pid = dwPid;
    }

    *ppSecurityContext = pSecurityContext;
cleanup:

    if (bIsImpersonating &&
        !RevertToSelf())
    {
        dwError = GetLastError();
    }

    if (hToken != INVALID_HANDLE_VALUE){
      CloseHandle(hToken);
    }

    VMAFD_SAFE_FREE_MEMORY (ptUser);
    VmAfdLog (VMAFD_DEBUG_DEBUG, "Exiting VmAfdInitializeSecurityContext");
    return dwError;
error:
    if (ppSecurityContext != NULL)
    {
      *ppSecurityContext = NULL;
    }
    if (pSecurityContext)
    {
        VmAfdFreeSecurityContext (pSecurityContext);
    }

    VmAfdLog (
        VMAFD_DEBUG_ERROR,
        "Something went wrong. Exiting with code: (%d)",
        dwError
        );

    goto cleanup;

}

VOID
VmAfdFreeSecurityContextImpl(
    PVM_AFD_SECURITY_CONTEXT pSecurityContext
    )
{
    if (pSecurityContext->pSid != NULL){
      VMAFD_SAFE_FREE_MEMORY(pSecurityContext->pSid);
    }
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

    if (IsValidSid (pSecurityContext->pSid))
    {
        dwSize = GetLengthSid(pSecurityContext->pSid);
    }

    else
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

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
VmAfdEncodeSecurityContextImpl (
            PVM_AFD_SECURITY_CONTEXT pSecurityContext,
            PBYTE pByteSecurityContext,
            DWORD dwBuffSize,
            PDWORD pdwBuffUsed
            )
{
    DWORD dwError = 0;
    DWORD dwSidSize = 0;
    PSID pSidDest = NULL;

    if (! IsValidSid (pSecurityContext->pSid))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwSidSize = GetLengthSid (pSecurityContext->pSid);

    if (dwSidSize > dwBuffSize)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pSidDest = (PSID) pByteSecurityContext;

    if (! CopySid (dwBuffSize, pSidDest, pSecurityContext->pSid))
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (pdwBuffUsed)
    {
        *pdwBuffUsed = dwSidSize;
    }

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
VmAfdDecodeSecurityContextImpl (
        PBYTE pByteSecurityContext,
        DWORD dwBuffSize,
        PVM_AFD_SECURITY_CONTEXT *ppSecurityContext
        )
{
    DWORD dwError = 0;
    DWORD dwSidSize = 0;

    PVM_AFD_SECURITY_CONTEXT pSecurityContext = NULL;

    PSID pSid = (PSID) pByteSecurityContext;
    PSID pSidDest = NULL;

    if (! IsValidSid (pSid))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }


    dwSidSize = GetLengthSid (pSid);

    dwError = VmAfdAllocateMemory(
                            sizeof (VM_AFD_SECURITY_CONTEXT),
                            (PVOID *) &pSecurityContext
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdAllocateMemory (
                            dwSidSize,
                            (PVOID *) &pSidDest
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (!CopySid(dwSidSize, pSidDest, pSid))
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pSecurityContext->pSid = pSidDest;
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
        VmAfdFreeSecurityContext(pSecurityContext);
    }
    if (pSidDest)
    {
        VMAFD_SAFE_FREE_MEMORY (pSidDest);
    }
    goto cleanup;
}

BOOL
IsAdminToken(HANDLE hToken)
{
    BOOL bIsRoot = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    DWORD dwSidSize = SECURITY_MAX_SID_SIZE;
    PSID pAdminSID = NULL;
    PSID pSystemSID = NULL;
    HANDLE hLinkedToken = NULL;
    OSVERSIONINFO osver = {sizeof(OSVERSIONINFO)};

    dwError = VmAfdAllocateMemory(dwSidSize, &pAdminSID);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!CreateWellKnownSid(
                WinBuiltinAdministratorsSid,
                NULL,
                pAdminSID,
                &dwSidSize))
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwSidSize = SECURITY_MAX_SID_SIZE;
    dwError = VmAfdAllocateMemory(dwSidSize, &pSystemSID);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!CreateWellKnownSid(
                WinLocalSystemSid,
                NULL,
                pSystemSID,
                &dwSidSize))
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!CheckTokenMembership(hToken, pAdminSID, &bIsRoot))
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!bIsRoot && !CheckTokenMembership(hToken, pSystemSID, &bIsRoot))
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    // If UAC is enabled then we have to check the linked token
    if (!bIsRoot)
    {
        if (!GetVersionEx(&osver))
        {
            dwError = GetLastError();
            BAIL_ON_VMAFD_ERROR (dwError);
        }

        if (osver.dwMajorVersion < 6)
        {
            dwError = ERROR_NOT_SUPPORTED;
            BAIL_ON_VMAFD_ERROR (dwError);
        }

        if (!GetTokenInformation(
                    hToken,
                    TokenLinkedToken,
                    (VOID*) &hLinkedToken,
                    sizeof(HANDLE),
                    &dwSidSize) )
        {
            dwError = GetLastError();
            BAIL_ON_VMAFD_ERROR_NO_LOG (dwError);
        }

        if (!CheckTokenMembership(hLinkedToken, &pAdminSID, &bIsRoot))
        {
            dwError = GetLastError();
            BAIL_ON_VMAFD_ERROR_NO_LOG (dwError);
        }
    }

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pAdminSID);
    VMAFD_SAFE_FREE_MEMORY(pSystemSID);

    if (hLinkedToken) CloseHandle(hLinkedToken);

    return bIsRoot;
error:
      // Clean up.
    goto cleanup;
}


BOOL
VmAfdIsRootSecurityContextImpl (
      PVM_AFD_CONNECTION_CONTEXT pConnectionContext
      )
{
    BOOL bIsRoot = FALSE, bIsImpersonating = FALSE;
    DWORD dwError = ERROR_SUCCESS ;
    HANDLE hToken = NULL;

    if (pConnectionContext == NULL ||
        pConnectionContext->pSecurityContext == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!IsValidSid (pConnectionContext->pSecurityContext->pSid))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    //
    // anonymous contexts are created by the system. during initialization or
    // from a an rpc context so skip the access checks
    //
    if (pConnectionContext->bAnonymousContext)
    {
        dwError = ERROR_SUCCESS;
        goto cleanup;
    }

    if ((pConnectionContext->pConnection == NULL) ||
        (pConnectionContext->pConnection->hConnection == NULL))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!ImpersonateNamedPipeClient(pConnectionContext->pConnection->hConnection))
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    bIsImpersonating = TRUE;

    if (!OpenThreadToken(GetCurrentThread(), TOKEN_ALL_ACCESS, TRUE, &hToken ))
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!IsAdminToken(hToken))
    {
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);
    }

    bIsRoot = TRUE;
cleanup:

    if (bIsImpersonating)
    {
        RevertToSelf();
    }

    if (hToken) CloseHandle(hToken);

    return bIsRoot;
error:
      // Clean up.
    goto cleanup;
}

BOOL
VmAfdEqualsSecurityContextImpl (
     PVM_AFD_SECURITY_CONTEXT pSecurityContext1,
     PVM_AFD_SECURITY_CONTEXT pSecurityContext2
    )
{
    BOOL bEqualsContext = FALSE;
    DWORD dwError = 0;

    if (!EqualSid (pSecurityContext1->pSid, pSecurityContext2->pSid))
    {
        dwError = GetLastError();
        bEqualsContext = FALSE;
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    else
    {
        bEqualsContext = TRUE;
    }

error:
    return bEqualsContext;
}


DWORD
VmAfdAllocateContextFromNameImpl (
      PCWSTR pszAccountName,
      PVM_AFD_SECURITY_CONTEXT *ppSecurityContext
      )
{
    DWORD dwError = 0;
    PVM_AFD_SECURITY_CONTEXT pSecurityContext = NULL;
    SID_NAME_USE sidNameUse = 0;
    PSTR paszAccountName = NULL;
    DWORD dwSidSize = 0;
    DWORD dwDomainSize = 0;
    PSID pSid = NULL;
    LPTSTR pszReferenceDomainName = NULL;

    if (pszAccountName)
    {
        dwError = VmAfdAllocateStringAFromW(
                                      pszAccountName,
                                      &paszAccountName
                                      );
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (! LookupAccountName (
                          NULL, //Name of the system. NULL for local system
                          pszAccountName,
                          NULL, //First time call to get size. So pass NULL for sid
                          &dwSidSize,
                          NULL,
                          &dwDomainSize,
                          &sidNameUse
                          )
       )
    {
        dwError = GetLastError();

        if (dwError == ERROR_INSUFFICIENT_BUFFER &&
            dwSidSize > 0
           )
        {
            dwError = 0;
        }
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateMemory (
                                dwSidSize,
                                (PVOID *) &pSid
                                );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdAllocateMemory (
                              dwDomainSize * sizeof (TCHAR),
                              (PVOID *) &pszReferenceDomainName
                              );
    BAIL_ON_VMAFD_ERROR (dwError);

    if ( ! LookupAccountName (
                              NULL,
                              pszAccountName,
                              pSid,
                              &dwSidSize,
                              pszReferenceDomainName,
                              &dwDomainSize,
                              &sidNameUse
                              )
       )
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateMemory (
                              sizeof (VM_AFD_SECURITY_CONTEXT),
                              (PVOID *) &pSecurityContext
                              );
    BAIL_ON_VMAFD_ERROR (dwError);

    pSecurityContext->pSid = pSid;

    pSid = NULL;

    *ppSecurityContext = pSecurityContext;

cleanup:
    VMAFD_SAFE_FREE_MEMORY (paszAccountName);
    VMAFD_SAFE_FREE_MEMORY (pszReferenceDomainName);

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
    if (pSid)
    {
        VMAFD_SAFE_FREE_MEMORY (pSid);
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
    BOOL bResult = FALSE;
    DWORD dwSidLength = 0;

    if (!pSecurityContextSrc ||
        !ppSecurityContextDst
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    bResult = IsValidSid(pSecurityContextSrc->pSid);
    if (bResult == FALSE){
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateMemory (
                                   sizeof (VM_AFD_SECURITY_CONTEXT),
                                   (PVOID *) &pSecurityContext
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);


    dwSidLength = GetLengthSid(pSecurityContextSrc->pSid);
    dwError = VmAfdAllocateMemory(
                    dwSidLength,
                    (PVOID *)&(pSecurityContext->pSid)
                    );
    BAIL_ON_VMAFD_ERROR (dwError);

    bResult = CopySid(
                    dwSidLength,
                    pSecurityContext->pSid,
                    pSecurityContextSrc->pSid
                    );
    if (bResult == FALSE){
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR (dwError);
    }

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

    DWORD dwRootSidLength = SECURITY_MAX_SID_SIZE;

    SID sTmpSid[SECURITY_MAX_SID_SIZE] = {0};
    PVM_AFD_SECURITY_CONTEXT pSecurityContext = NULL;
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext = NULL;

    PSID pRootSid = NULL;

    if (!ppConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!IsAdminToken(NULL))
    {
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!CreateWellKnownSid(
                            WinLocalSystemSid,
                            NULL,
                            sTmpSid,
                            &dwRootSidLength
                          )
       )
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateMemory (
                                    dwRootSidLength,
                                    (PVOID *)&pRootSid
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (!CreateWellKnownSid (
                              WinLocalSystemSid,
                              NULL,
                              pRootSid,
                              &dwRootSidLength
                            )
       )
    {
        dwError = GetLastError();
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateMemory (
                                    sizeof (VM_AFD_SECURITY_CONTEXT),
                                    (PVOID *) &pSecurityContext
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdAllocateMemory (
                                    sizeof (VM_AFD_CONNECTION_CONTEXT),
                                    (PVOID *) &pConnectionContext
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    pSecurityContext->pSid = pRootSid;

    pRootSid = NULL;

    pConnectionContext->bAnonymousContext = TRUE;
    pConnectionContext->pConnection = NULL;
    pConnectionContext->pSecurityContext = pSecurityContext;

    *ppConnectionContext = pConnectionContext;

    pSecurityContext = NULL;
    pConnectionContext = NULL;

cleanup:

    return dwError;

error:
    if (ppConnectionContext)
    {
        *ppConnectionContext = NULL;
    }
    if (pSecurityContext)
    {
        VmAfdFreeSecurityContext (pSecurityContext);
    }
    if (pConnectionContext)
    {
        VmAfdFreeConnectionContext (pConnectionContext);
    }

    VMAFD_SAFE_FREE_MEMORY (pRootSid);

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

    PSID pWellKnownSid = NULL;

    if (!ppSecurityContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    switch (contextType)
    {
        DWORD dwMaxSidSize = SECURITY_MAX_SID_SIZE;
        SID sTmpSid[SECURITY_MAX_SID_SIZE] = {0};

        case VM_AFD_CONTEXT_TYPE_ROOT:

          if (!CreateWellKnownSid(
                            WinAccountAdministratorSid,
                            NULL,
                            sTmpSid,
                            &dwMaxSidSize
                          )
              )
          {
              dwError = GetLastError();
              BAIL_ON_VMAFD_ERROR (dwError);
          }

          dwError = VmAfdAllocateMemory (
                                    dwMaxSidSize,
                                    (PVOID *)&pWellKnownSid
                                  );
          BAIL_ON_VMAFD_ERROR (dwError);

          if (!CreateWellKnownSid (
                              WinAccountAdministratorSid,
                              NULL,
                              pWellKnownSid,
                              &dwMaxSidSize
                            )
             )
          {
              dwError = GetLastError();
              BAIL_ON_VMAFD_ERROR (dwError);
          }

          break;

        case VM_AFD_CONTEXT_TYPE_EVERYONE:
          if (!CreateWellKnownSid(
                            WinWorldSid,
                            NULL,
                            sTmpSid,
                            &dwMaxSidSize
                          )
              )
          {
              dwError = GetLastError();
              if (dwError == ERROR_INSUFFICIENT_BUFFER &&
                  dwMaxSidSize > 0
                 )
              {
                  dwError = 0;
              }
              BAIL_ON_VMAFD_ERROR (dwError);
          }

          dwError = VmAfdAllocateMemory (
                                    dwMaxSidSize,
                                    (PVOID *)&pWellKnownSid
                                  );
          BAIL_ON_VMAFD_ERROR (dwError);

          if (!CreateWellKnownSid (
                              WinWorldSid,
                              NULL,
                              pWellKnownSid,
                              &dwMaxSidSize
                            )
             )
          {
              dwError = GetLastError();
              BAIL_ON_VMAFD_ERROR (dwError);
          }

          break;
    }

    dwError = VmAfdAllocateMemory (
                                    sizeof (VM_AFD_SECURITY_CONTEXT),
                                    (PVOID *) &pSecurityContext
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    pSecurityContext->pSid = pWellKnownSid;

    pWellKnownSid = NULL;

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

    VMAFD_SAFE_FREE_MEMORY (pWellKnownSid);

    goto cleanup;
}

BOOL
VmAfdContextBelongsToGroupImpl (
    PVM_AFD_SECURITY_CONTEXT pSecurityContext,
    PVM_AFD_SECURITY_CONTEXT pSecurityContextGroup
    )
{
    BOOL bResult = false;

    if (IsWellKnownSid (
          pSecurityContextGroup->pSid,
          WinWorldSid
          )
       )
    {
        bResult = true;
    }

    //TODO: Other checking to see if the context belongs to group

//error:
    return bResult;
}

DWORD
VmAfdCheckAclContextImpl (
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PSTR pszSddlAcl,
    BOOL *pbIsAllowed
    )
{
    DWORD dwError = 0;
    BOOL bIsAllowed = false;

    // TODO: check the ACL instead of checking for root
    bIsAllowed = VmAfdIsRootSecurityContextImpl(pConnectionContext);

    *pbIsAllowed = bIsAllowed;

    return dwError;
}

DWORD
VmAfdGenRandomImpl (
        PDWORD pdwRandomNumber
        )
{
    DWORD dwError = 0;

    DWORD dwRandomNumber = 0;

    HCRYPTPROV hCryptProv = NULL;

    if (!CryptAcquireContext (
              &hCryptProv,
              NULL,
              NULL,
              PROV_RSA_FULL,
              0
              )
       )
    {
        dwError = GetLastError();

        if (dwError == NTE_BAD_KEYSET)
        {
            dwError = 0;
            if (!CryptAcquireContext(
                  &hCryptProv,
                  NULL,
                  NULL,
                  PROV_RSA_FULL,
                  CRYPT_NEWKEYSET
                  )
               )
            {
                dwError = GetLastError();
            }
        }

        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!CryptGenRandom (
              hCryptProv,
              sizeof (DWORD),
              (PBYTE) &dwRandomNumber
              )
       )
    {
        dwError = GetLastError();

        BAIL_ON_VMAFD_ERROR (dwError);
    }

    *pdwRandomNumber = dwRandomNumber;

cleanup:
    return dwError;

error:
    *pdwRandomNumber = 0;

    goto cleanup;
}

DWORD
VmAfdAllocateNameFromContextImpl(
    PVM_AFD_SECURITY_CONTEXT pSecurityContext,
    PWSTR *ppszAccountName
    )
{
    DWORD dwError = 0;
    PWSTR pszAccountName = NULL;
    PWSTR pszDomainName = NULL;

    DWORD dwAccountNameSize = 0;
    DWORD dwDomainNameSize = 0;
    SID_NAME_USE sdUse = 0;

    if (!LookupAccountSidW (
                           NULL,
                           pSecurityContext->pSid,
                           NULL,
                           &dwAccountNameSize,
                           NULL,
                           &dwDomainNameSize,
                           &sdUse
                          )
       )
    {
        dwError = GetLastError();

        if (dwError == ERROR_INSUFFICIENT_BUFFER)
        {
            dwError = 0;
        }
    }
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdAllocateMemory (
                                   sizeof(WCHAR) * dwAccountNameSize,
                                   (PVOID *) &pszAccountName
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdAllocateMemory (
                                   sizeof (WCHAR) * dwDomainNameSize,
                                   (PVOID *) &pszDomainName
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (!LookupAccountSidW (
                            NULL,
                            pSecurityContext->pSid,
                            pszAccountName,
                            &dwAccountNameSize,
                            pszDomainName,
                            &dwDomainNameSize,
                            &sdUse
                          )
       )
    {
        dwError = GetLastError();

        BAIL_ON_VMAFD_ERROR (dwError);
    }

    *ppszAccountName = pszAccountName;

cleanup:
    VMAFD_SAFE_FREE_MEMORY (pszDomainName);

    return dwError;

error:
    if (ppszAccountName)
    {
        *ppszAccountName = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY (pszAccountName);

    goto cleanup;
}
