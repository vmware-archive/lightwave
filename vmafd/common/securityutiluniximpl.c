#include "includes.h"

DWORD
VmAfdInitializeSecurityContextImpl(
    PVM_AFD_CONNECTION pConnection,
    PVM_AFD_SECURITY_CONTEXT *ppSecurityContext
    )
{
    DWORD dwError = 0;
    struct ucred credentials = {0};
    int credLength = sizeof (struct ucred);
    PVM_AFD_SECURITY_CONTEXT pSecurityContext = NULL;
    if ((getsockopt (
            pConnection->fd,
            SOL_SOCKET,
            SO_PEERCRED,
            &credentials,
            &credLength)) < 0){
      dwError = LwErrnoToWin32Error (errno);
      BAIL_ON_VMAFD_ERROR (dwError);
    }
    dwError = VmAfdAllocateMemory(
                    sizeof (VM_AFD_SECURITY_CONTEXT),
                    (PVOID *)&pSecurityContext);
    BAIL_ON_VMAFD_ERROR (dwError);
    pSecurityContext->uid = credentials.uid;
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
    struct passwd *pd = NULL;
    PSTR psazAccountName = NULL;
    PVM_AFD_SECURITY_CONTEXT pSecurityContext = NULL;

    dwError = VmAfdAllocateStringAFromW (
                            pszAccountName,
                            &psazAccountName
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

    pd = getpwnam (psazAccountName);
    if (pd == NULL)
    {
        dwError = ERROR_NONE_MAPPED;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateMemory (
                            sizeof (VM_AFD_SECURITY_CONTEXT),
                            (PVOID *)&pSecurityContext
                            );
    BAIL_ON_VMAFD_ERROR (dwError);


    pSecurityContext->uid = pd->pw_uid;

    *ppSecurityContext = pSecurityContext;

cleanup:
    VMAFD_SAFE_FREE_MEMORY (psazAccountName);

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
        struct passwd *pd = NULL;
        pd = getpwuid(pSecurityContext->uid);

        if (pd)
        {
            dwError = VmAfdAllocateStringWFromA(
                                                 pd->pw_name,
                                                 &pszAccountName
                                               );
            BAIL_ON_VMAFD_ERROR (dwError);
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
    VMAFD_SAFE_FREE_MEMORY (pszAccountName);

    goto cleanup;
}
