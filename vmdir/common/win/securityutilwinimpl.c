/*
 * Copyright © 2012-2017 VMware, Inc.  All Rights Reserved.
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

BOOL
VmDirIsAdminToken(
    HANDLE hToken
    )
{
    BOOL bIsRoot = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    DWORD dwSidSize = SECURITY_MAX_SID_SIZE;
    PSID pAdminSID = NULL;
    PSID pSystemSID = NULL;
    HANDLE hLinkedToken = INVALID_HANDLE_VALUE;
    OSVERSIONINFO osver = {sizeof(OSVERSIONINFO)};

    dwError = VmDirAllocateMemory(dwSidSize, &pAdminSID);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!CreateWellKnownSid(
                WinBuiltinAdministratorsSid,
                NULL,
                pAdminSID,
                &dwSidSize))
    {
        dwError = GetLastError();
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwSidSize = SECURITY_MAX_SID_SIZE;
    dwError = VmDirAllocateMemory(dwSidSize, &pSystemSID);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!CreateWellKnownSid(
                WinLocalSystemSid,
                NULL,
                pSystemSID,
                &dwSidSize))
    {
        dwError = GetLastError();
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    if (!CheckTokenMembership(hToken, pAdminSID, &bIsRoot))
    {
        dwError = GetLastError();
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    if (!bIsRoot && !CheckTokenMembership(hToken, pSystemSID, &bIsRoot))
    {
        dwError = GetLastError();
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    // If UAC is enabled then we have to check the linked token
    if (!bIsRoot)
    {
        if (!GetVersionEx(&osver))
        {
            dwError = GetLastError();
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (osver.dwMajorVersion < 6)
        {
            dwError = ERROR_NOT_SUPPORTED;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (!GetTokenInformation(
                    hToken,
                    TokenLinkedToken,
                    (VOID*)&hLinkedToken,
                    sizeof(HANDLE),
                    &dwSidSize))
        {
            dwError = GetLastError();
            BAIL_ON_VMDIR_ERROR (dwError);
        }

        if (!CheckTokenMembership(hLinkedToken, &pAdminSID, &bIsRoot))
        {
            dwError = GetLastError();
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pAdminSID);
    VMDIR_SAFE_FREE_MEMORY(pSystemSID);

    if (hLinkedToken != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hLinkedToken);
    }

    return bIsRoot;
error:
    // Clean up.
    goto cleanup;
}

DWORD
VmDirGetSidFromToken(
    HANDLE hToken,
    PSID *ppSid
    )
{
    BOOL bResult = FALSE;
    DWORD dwError = 0;
    PSID pSid = NULL;
    DWORD dwTokenSize = 0;
    PTOKEN_USER ptUser = NULL;
    DWORD dwSidLength = 0;

    bResult = GetTokenInformation(
                    hToken,
                    TokenUser,
                    NULL,
                    0,
                    &dwTokenSize);
    if (!bResult && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
    {
        dwError = GetLastError();
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(dwTokenSize, (PVOID *)&ptUser);
    BAIL_ON_VMDIR_ERROR(dwError);

    bResult = GetTokenInformation(
                    hToken,
                    TokenUser,
                    ptUser,
                    dwTokenSize,
                    &dwTokenSize);
    if (!bResult)
    {
        dwError = GetLastError();
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    bResult = IsValidSid(ptUser->User.Sid);
    if (!bResult)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_PARAMETER);
    }

    dwSidLength = GetLengthSid(ptUser->User.Sid);
    dwError = VmDirAllocateMemory(dwSidLength, (PVOID *)&pSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    bResult = CopySid(dwSidLength, pSid, ptUser->User.Sid);
    if (!bResult)
    {
        dwError = GetLastError();
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppSid = pSid;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(ptUser);
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pSid);

    goto cleanup;
}

DWORD
VmDirInitializeSecurityContextImpl(
    PVM_DIR_CONNECTION pConnection,
    PVM_DIR_SECURITY_CONTEXT *ppSecurityContext
    )
{
    DWORD dwError = 0;
    BOOL bResult = FALSE;
    BOOL bIsImpersonating = FALSE;
    PVM_DIR_SECURITY_CONTEXT pSecurityContext = NULL;
    HANDLE hToken = NULL;

    bResult = ImpersonateNamedPipeClient(pConnection->hConnection);
    if (!bResult)
    {
        dwError = GetLastError ();
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    bIsImpersonating = TRUE;

    bResult = OpenThreadToken(
                    GetCurrentThread(),
                    TOKEN_ALL_ACCESS,
                    TRUE,
                    &hToken);
    if (!bResult)
    {
        dwError = GetLastError();
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
                sizeof(VM_DIR_SECURITY_CONTEXT),
                (PVOID *)&pSecurityContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetSidFromToken(hToken, &pSecurityContext->pSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    pSecurityContext->bRoot = VmDirIsAdminToken(hToken);
    *ppSecurityContext = pSecurityContext;

cleanup:
    if (bIsImpersonating && !RevertToSelf())
    {
        dwError = GetLastError();
    }

    if (hToken != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hToken);
    }

    return dwError;

error:
    *ppSecurityContext = NULL;

    VmDirFreeSecurityContext(pSecurityContext);

    goto cleanup;
}

VOID
VmDirFreeSecurityContextImpl(
    PVM_DIR_SECURITY_CONTEXT pSecurityContext
    )
{
    if (pSecurityContext != NULL)
    {
        VMDIR_SAFE_FREE_MEMORY(pSecurityContext->pSid);
        VMDIR_SAFE_FREE_MEMORY(pSecurityContext);
    }
}

DWORD
VmDirGetSecurityContextSizeImpl (
      PVM_DIR_SECURITY_CONTEXT pSecurityContext,
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
        BAIL_ON_VMDIR_ERROR (dwError);
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
VmDirEncodeSecurityContextImpl (
            PVM_DIR_SECURITY_CONTEXT pSecurityContext,
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
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    dwSidSize = GetLengthSid (pSecurityContext->pSid);

    if (dwSidSize > dwBuffSize)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    pSidDest = (PSID) pByteSecurityContext;

    if (! CopySid (dwBuffSize, pSidDest, pSecurityContext->pSid))
    {
        dwError = GetLastError();
        BAIL_ON_VMDIR_ERROR (dwError);
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
VmDirDecodeSecurityContextImpl (
        PBYTE pByteSecurityContext,
        DWORD dwBuffSize,
        PVM_DIR_SECURITY_CONTEXT *ppSecurityContext
        )
{
    DWORD dwError = 0;
    DWORD dwSidSize = 0;

    PVM_DIR_SECURITY_CONTEXT pSecurityContext = NULL;

    PSID pSid = (PSID) pByteSecurityContext;
    PSID pSidDest = NULL;

    if (! IsValidSid (pSid))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR (dwError);
    }


    dwSidSize = GetLengthSid (pSid);

    dwError = VmDirAllocateMemory(
                            sizeof (VM_DIR_SECURITY_CONTEXT),
                            (PVOID *) &pSecurityContext
                            );
    BAIL_ON_VMDIR_ERROR (dwError);

    dwError = VmDirAllocateMemory (
                            dwSidSize,
                            (PVOID *) &pSidDest
                            );
    BAIL_ON_VMDIR_ERROR (dwError);

    if (!CopySid(dwSidSize, pSidDest, pSid))
    {
        dwError = GetLastError();
        BAIL_ON_VMDIR_ERROR (dwError);
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
        VmDirFreeSecurityContext(pSecurityContext);
    }
    if (pSidDest)
    {
        VMDIR_SAFE_FREE_MEMORY (pSidDest);
    }
    goto cleanup;
}

BOOL
VmDirIsRootSecurityContextImpl (
      PVM_DIR_SECURITY_CONTEXT pSecurityContext
      )
{
    return pSecurityContext->bRoot;
}

BOOL
VmDirEqualsSecurityContextImpl (
     PVM_DIR_SECURITY_CONTEXT pSecurityContext1,
     PVM_DIR_SECURITY_CONTEXT pSecurityContext2
    )
{
    BOOL bEqualsContext = FALSE;
    DWORD dwError = 0;

    if (!EqualSid (pSecurityContext1->pSid, pSecurityContext2->pSid))
    {
        dwError = GetLastError();
        bEqualsContext = FALSE;
        BAIL_ON_VMDIR_ERROR (dwError);
    }
    else
    {
        bEqualsContext = TRUE;
    }

error:
    return bEqualsContext;
}


DWORD
VmDirAllocateContextFromNameImpl (
      PCWSTR pszAccountName,
      PVM_DIR_SECURITY_CONTEXT *ppSecurityContext
      )
{
    DWORD dwError = 0;
    PVM_DIR_SECURITY_CONTEXT pSecurityContext = NULL;
    SID_NAME_USE sidNameUse = 0;
    PSTR paszAccountName = NULL;
    DWORD dwSidSize = 0;
    DWORD dwDomainSize = 0;
    PSID pSid = NULL;
    LPTSTR pszReferenceDomainName = NULL;

    if (pszAccountName)
    {
        dwError = VmDirAllocateStringAFromW(
                                      pszAccountName,
                                      &paszAccountName
                                      );
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    if (! LookupAccountName (
                          NULL, //Name of the system. NULL for local system
                          paszAccountName,
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
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    dwError = VmDirAllocateMemory (
                                dwSidSize,
                                (PVOID *) &pSid
                                );
    BAIL_ON_VMDIR_ERROR (dwError);

    dwError = VmDirAllocateMemory (
                              dwDomainSize * sizeof (TCHAR),
                              (PVOID *) &pszReferenceDomainName
                              );
    BAIL_ON_VMDIR_ERROR (dwError);

    if ( ! LookupAccountName (
                              NULL,
                              paszAccountName,
                              pSid,
                              &dwSidSize,
                              pszReferenceDomainName,
                              &dwDomainSize,
                              &sidNameUse
                              )
       )
    {
        dwError = GetLastError();
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    dwError = VmDirAllocateMemory (
                              sizeof (VM_DIR_SECURITY_CONTEXT),
                              (PVOID *) &pSecurityContext
                              );
    BAIL_ON_VMDIR_ERROR (dwError);

    pSecurityContext->pSid = pSid;

    pSid = NULL;

    *ppSecurityContext = pSecurityContext;

cleanup:
    VMDIR_SAFE_FREE_MEMORY (paszAccountName);
    VMDIR_SAFE_FREE_MEMORY (pszReferenceDomainName);

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
    if (pSid)
    {
        VMDIR_SAFE_FREE_MEMORY (pSid);
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
    BOOL bResult = FALSE;
    DWORD dwSidLength = 0;

    if (!pSecurityContextSrc ||
        !ppSecurityContextDst
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    bResult = IsValidSid(pSecurityContextSrc->pSid);
    if (bResult == FALSE){
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    dwError = VmDirAllocateMemory (
                                   sizeof (VM_DIR_SECURITY_CONTEXT),
                                   (PVOID *) &pSecurityContext
                                  );
    BAIL_ON_VMDIR_ERROR (dwError);


    dwSidLength = GetLengthSid(pSecurityContextSrc->pSid);
    dwError = VmDirAllocateMemory(
                    dwSidLength,
                    (PVOID *)&(pSecurityContext->pSid)
                    );
    BAIL_ON_VMDIR_ERROR (dwError);

    bResult = CopySid(
                    dwSidLength,
                    pSecurityContext->pSid,
                    pSecurityContextSrc->pSid
                    );
    if (bResult == FALSE){
        dwError = GetLastError();
        BAIL_ON_VMDIR_ERROR (dwError);
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

    DWORD dwRootSidLength = SECURITY_MAX_SID_SIZE;

    SID sTmpSid[SECURITY_MAX_SID_SIZE] = {0};
    PVM_DIR_SECURITY_CONTEXT pSecurityContext = NULL;

    PSID pRootSid = NULL;


    if (!ppSecurityContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR (dwError);
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
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    dwError = VmDirAllocateMemory (
                                    dwRootSidLength,
                                    (PVOID *)&pRootSid
                                  );
    BAIL_ON_VMDIR_ERROR (dwError);

    if (!CreateWellKnownSid (
                              WinLocalSystemSid,
                              NULL,
                              pRootSid,
                              &dwRootSidLength
                            )
       )
    {
        dwError = GetLastError();
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    dwError = VmDirAllocateMemory (
                                    sizeof (VM_DIR_SECURITY_CONTEXT),
                                    (PVOID *) &pSecurityContext
                                  );
    BAIL_ON_VMDIR_ERROR (dwError);

    pSecurityContext->pSid = pRootSid;

    pRootSid = NULL;

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

    VMDIR_SAFE_FREE_MEMORY (pRootSid);

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

    PSID pWellKnownSid = NULL;

    if (!ppSecurityContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    switch (contextType)
    {
        DWORD dwMaxSidSize = SECURITY_MAX_SID_SIZE;
        SID sTmpSid[SECURITY_MAX_SID_SIZE] = {0};

        case VM_DIR_CONTEXT_TYPE_ROOT:

          if (!CreateWellKnownSid(
                            WinAccountAdministratorSid,
                            NULL,
                            sTmpSid,
                            &dwMaxSidSize
                          )
              )
          {
              dwError = GetLastError();
              BAIL_ON_VMDIR_ERROR (dwError);
          }

          dwError = VmDirAllocateMemory (
                                    dwMaxSidSize,
                                    (PVOID *)&pWellKnownSid
                                  );
          BAIL_ON_VMDIR_ERROR (dwError);

          if (!CreateWellKnownSid (
                              WinAccountAdministratorSid,
                              NULL,
                              pWellKnownSid,
                              &dwMaxSidSize
                            )
             )
          {
              dwError = GetLastError();
              BAIL_ON_VMDIR_ERROR (dwError);
          }

          break;

        case VM_DIR_CONTEXT_TYPE_EVERYONE:
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
              BAIL_ON_VMDIR_ERROR (dwError);
          }

          dwError = VmDirAllocateMemory (
                                    dwMaxSidSize,
                                    (PVOID *)&pWellKnownSid
                                  );
          BAIL_ON_VMDIR_ERROR (dwError);

          if (!CreateWellKnownSid (
                              WinWorldSid,
                              NULL,
                              pWellKnownSid,
                              &dwMaxSidSize
                            )
             )
          {
              dwError = GetLastError();
              BAIL_ON_VMDIR_ERROR (dwError);
          }

          break;
    }

    dwError = VmDirAllocateMemory (
                                    sizeof (VM_DIR_SECURITY_CONTEXT),
                                    (PVOID *) &pSecurityContext
                                  );
    BAIL_ON_VMDIR_ERROR (dwError);

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
        VmDirFreeSecurityContext (pSecurityContext);
    }

    VMDIR_SAFE_FREE_MEMORY (pWellKnownSid);

    goto cleanup;
}

BOOL
VmDirContextBelongsToGroupImpl (
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
    PVM_DIR_SECURITY_CONTEXT pSecurityContextGroup
    )
{
    BOOL bResult = false;

    if (IsWellKnownSid (
          pSecurityContextGroup,
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
VmDirAllocateNameFromContextImpl(
    PVM_DIR_SECURITY_CONTEXT pSecurityContext,
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
    BAIL_ON_VMDIR_ERROR (dwError);

    dwError = VmDirAllocateMemory (
                                   sizeof(WCHAR) * dwAccountNameSize,
                                   (PVOID *) &pszAccountName
                                  );
    BAIL_ON_VMDIR_ERROR (dwError);

    dwError = VmDirAllocateMemory (
                                   sizeof (WCHAR) * dwDomainNameSize,
                                   (PVOID *) &pszDomainName
                                  );
    BAIL_ON_VMDIR_ERROR (dwError);

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

        BAIL_ON_VMDIR_ERROR (dwError);
    }

    *ppszAccountName = pszAccountName;

cleanup:
    VMDIR_SAFE_FREE_MEMORY (pszDomainName);

    return dwError;

error:
    if (ppszAccountName)
    {
        *ppszAccountName = NULL;
    }

    VMDIR_SAFE_FREE_MEMORY (pszAccountName);

    goto cleanup;
}
