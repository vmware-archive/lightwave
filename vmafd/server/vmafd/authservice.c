/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : authservice.c
 *
 * Abstract :
 *
 */
#include "includes.h"


DWORD
VecsSrvCreateCertStoreWithAuth (
    PCWSTR pszStoreName,
    PCWSTR pszPassword,
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PVECS_SRV_STORE_HANDLE *ppStore
    )
{
    DWORD dwError = 0;
    DWORD dwDeleteError = 0;
    PVECS_SRV_STORE_HANDLE pStore = NULL;
    PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    PVECS_SERV_STORE pStoreInstance = NULL;
    BOOL bIsHoldingLock = FALSE;

    pthread_mutex_lock (&gVmafdGlobals.mutexCreateStore);

    bIsHoldingLock = TRUE;

    dwError = VecsSrvCreateCertStore (
                                pszStoreName,
                                pszPassword,
                                &pStoreInstance
                                );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdInitializeSecurityDescriptor(
                            pConnectionContext->pSecurityContext,
                            1,
                            &pSecurityDescriptor
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VecsDbSetSecurityDescriptor (
                                pStoreInstance->dwStoreId,
                                pSecurityDescriptor
                                );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdGetStoreHandle (
                                    (PWSTR)pszStoreName,
                                    (PWSTR)pszPassword,
                                    pConnectionContext->pSecurityContext,
                                    &pStore
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    pthread_mutex_unlock (&gVmafdGlobals.mutexCreateStore);

    bIsHoldingLock = FALSE;


    *ppStore = pStore;

cleanup:

    if (pSecurityDescriptor)
    {
        VmAfdFreeSecurityDescriptor (pSecurityDescriptor);
    }

    if (bIsHoldingLock)
    {
        pthread_mutex_unlock(&gVmafdGlobals.mutexCreateStore);
    }
    VMAFD_SAFE_FREE_MEMORY (pStoreInstance);


    return dwError;

error:
    if (dwError != ERROR_ALREADY_EXISTS)
    {
        dwDeleteError = VecsSrvDeleteCertStore(pszStoreName);
    }

    if (ppStore)
    {
        *ppStore = NULL;
    }

    if (pStore)
    {
        VmAfdReleaseStoreHandle (pStore);
    }

    goto cleanup;
}

DWORD
VecsSrvDeleteCertStoreWithAuth (
    PCWSTR pszStoreName,
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext
    )
{
    DWORD dwError = 0;
    PVECS_SRV_STORE_HANDLE pStore = NULL;

    dwError = VmAfdGetStoreHandle(
                                  (PWSTR)pszStoreName,
                                  (PWSTR)NULL,
                                  pConnectionContext->pSecurityContext,
                                  &pStore
                                 );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdCheckOwnerShipWithHandle(
                                            pStore,
                                            pConnectionContext
                                           );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (!VmAfdCanDeleteStore (pStore))
    {
        dwError = ERROR_BUSY;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VecsSrvDeleteCertStore (
                              pszStoreName
                              );
    BAIL_ON_VMAFD_ERROR (dwError);

    VmAfdDeleteStoreEntry (pStore);
    pStore = NULL;

cleanup:

    if (pStore)
    {
      VmAfdCloseStoreHandle (
                             pStore,
                             pConnectionContext->pSecurityContext
                            );
    }

    return dwError;
error:
    goto cleanup;
}

DWORD
VecsSrvOpenCertStoreWithAuth(
                             PCWSTR pszStoreName,
                             PCWSTR pszPassword,
                             PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
                             PVECS_SRV_STORE_HANDLE *ppStore
                            )
{
    DWORD dwError = 0;
    PVECS_SRV_STORE_HANDLE pStore = NULL;

    if (IsNullOrEmptyString (pszStoreName) ||
        !pConnectionContext ||
        !pConnectionContext->pSecurityContext ||
        !ppStore
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdGetStoreHandle (
                                    (PWSTR)pszStoreName,
                                    (PWSTR)pszPassword,
                                    pConnectionContext->pSecurityContext,
                                    &pStore
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppStore = pStore;

cleanup:

    return dwError;

error:
    if (ppStore)
    {
        *ppStore = NULL;
    }
    VMAFD_SAFE_FREE_MEMORY (pStore);
    goto cleanup;
}

DWORD
VecsSrvSetPermission (
    PVECS_SRV_STORE_HANDLE pStore,
    PCWSTR pszUserName,
    UINT32 accessMask,
    VMAFD_ACE_TYPE aceType,
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext
    )
{
    DWORD dwError = 0;

    PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    PVECS_SERV_STORE pStoreInstance = NULL;
    BOOL bIsHoldingLock = FALSE;
    PWSTR pwszAccountName = NULL;
    DWORD dwLogError = 0;

    dwError = VmAfdCheckOwnerShipWithHandle (
                            pStore,
                            pConnectionContext
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

    pthread_mutex_lock (&gVmafdGlobals.mutexStoreState);

    bIsHoldingLock = TRUE;

    dwError = VmAfdGetSecurityDescriptorFromHandle (
                                pStore,
                                &pSecurityDescriptor
                                );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdGetStoreFromHandle (
                                        pStore,
                                        pConnectionContext->pSecurityContext,
                                        &pStoreInstance
                                      );
    BAIL_ON_VMAFD_ERROR (dwError);


    dwError = VmAfdModifyPermissions (
                                pStoreInstance,
                                pszUserName,
                                accessMask,
                                aceType,
                                pSecurityDescriptor,
                                VMW_IPC_MODIFY_PERMISSIONS_SET
                                );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdSetSecurityDescriptorForHandle (
                                pStore,
                                pSecurityDescriptor
                                );
    BAIL_ON_VMAFD_ERROR (dwError);

    pthread_mutex_unlock (&gVmafdGlobals.mutexStoreState);

    bIsHoldingLock = FALSE;

    dwLogError = VmAfdAllocateNameFromContext (
                                               pConnectionContext->pSecurityContext,
                                               &pwszAccountName
                                              );
    if (!IsNullOrEmptyString(pwszAccountName))
    {
        PSTR pszAccountName = NULL;
        PSTR paszUserName = NULL;
        dwLogError = VmAfdAllocateStringAFromW(
                                               pwszAccountName,
                                               &pszAccountName
                                              );
        dwLogError = VmAfdAllocateStringAFromW (
                                                pszUserName,
                                                &paszUserName
                                               );
        if (pszAccountName)
        {
           VmAfdLog (VMAFD_DEBUG_ANY,
                     "User %s changed permission of Store with ID: %d \n "
                     "Permission %s %s was granted to user %s",
                     pszAccountName,
                     pStoreInstance->dwStoreId,
                     accessMask & READ_STORE ? "read" : "",
                     accessMask & WRITE_STORE ? "write": "",
                     !IsNullOrEmptyString(paszUserName)? paszUserName: ""
                    );
        }
        VMAFD_SAFE_FREE_MEMORY (pszAccountName);
        VMAFD_SAFE_FREE_MEMORY (paszUserName);
    }


cleanup:
    if (pSecurityDescriptor)
    {
        VmAfdFreeSecurityDescriptor (pSecurityDescriptor);
    }

    if (bIsHoldingLock)
    {
        pthread_mutex_unlock(&gVmafdGlobals.mutexStoreState);
    }

    VMAFD_SAFE_FREE_MEMORY (pStoreInstance);

    VMAFD_SAFE_FREE_MEMORY (pwszAccountName);

    return dwError;

error:
    goto cleanup;
}

DWORD
VecsSrvRevokePermission (
    PVECS_SRV_STORE_HANDLE pStore,
    PCWSTR pszUserName,
    UINT32 accessMask,
    VMAFD_ACE_TYPE aceType,
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext
    )
{
    DWORD dwError = 0;

    PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    PVECS_SERV_STORE pStoreInstance = NULL;
    BOOL bIsHoldingLock = FALSE;
    PWSTR pwszAccountName = NULL;
    DWORD dwLogError = 0;

    dwError = VmAfdCheckOwnerShipWithHandle (
                            pStore,
                            pConnectionContext
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

    pthread_mutex_lock (&gVmafdGlobals.mutexStoreState);

    bIsHoldingLock = TRUE;

    dwError = VmAfdGetSecurityDescriptorFromHandle (
                                pStore,
                                &pSecurityDescriptor
                                );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdGetStoreFromHandle (
                                        pStore,
                                        pConnectionContext->pSecurityContext,
                                        &pStoreInstance
                                      );
    BAIL_ON_VMAFD_ERROR (dwError);


    dwError = VmAfdModifyPermissions (
                                pStoreInstance,
                                pszUserName,
                                accessMask,
                                aceType,
                                pSecurityDescriptor,
                                VMW_IPC_MODIFY_PERMISSIONS_REVOKE
                                );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdSetSecurityDescriptorForHandle (
                                pStore,
                                pSecurityDescriptor
                                );
    BAIL_ON_VMAFD_ERROR (dwError);

    pthread_mutex_unlock (&gVmafdGlobals.mutexStoreState);

    bIsHoldingLock = FALSE;

    dwLogError = VmAfdAllocateNameFromContext (
                                               pConnectionContext->pSecurityContext,
                                               &pwszAccountName
                                              );
    if (!IsNullOrEmptyString(pwszAccountName))
    {
        PSTR pszAccountName = NULL;
        PSTR paszUserName = NULL;
        dwLogError = VmAfdAllocateStringAFromW(
                                               pwszAccountName,
                                               &pszAccountName
                                              );
        dwLogError = VmAfdAllocateStringAFromW (
                                                pszUserName,
                                                &paszUserName
                                               );
        if (pszAccountName)
        {
           VmAfdLog (VMAFD_DEBUG_ANY,
                     "User %s changed permission of Store with ID: %d \n "
                     "Permission %s %s was revoked from user %s",
                     pszAccountName,
                     pStoreInstance->dwStoreId,
                     accessMask & READ_STORE ? "read" : "",
                     accessMask & WRITE_STORE ? "write": "",
                     !IsNullOrEmptyString(paszUserName)? paszUserName: ""
                    );
        }
        VMAFD_SAFE_FREE_MEMORY (pszAccountName);
        VMAFD_SAFE_FREE_MEMORY (paszUserName);
    }


cleanup:
    if (pSecurityDescriptor)
    {
        VmAfdFreeSecurityDescriptor (pSecurityDescriptor);
    }

    if (bIsHoldingLock)
    {
        pthread_mutex_unlock(&gVmafdGlobals.mutexStoreState);
    }

    VMAFD_SAFE_FREE_MEMORY (pStoreInstance);
    VMAFD_SAFE_FREE_MEMORY (pwszAccountName);

    return dwError;

error:
    goto cleanup;
}


DWORD
VecsSrvChangeOwner (
    PVECS_SRV_STORE_HANDLE pStore,
    PCWSTR pszUserName,
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext
    )
{
    DWORD dwError = 0;
    PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    PVECS_SERV_STORE pStoreInstance = NULL;
    BOOL bIsHoldingLock = FALSE;

    dwError = VmAfdCheckOwnerShipWithHandle (
                              pStore,
                              pConnectionContext
                              );
    BAIL_ON_VMAFD_ERROR (dwError);

    pthread_mutex_lock (&gVmafdGlobals.mutexStoreState);

    bIsHoldingLock = TRUE;

    dwError = VmAfdGetSecurityDescriptorFromHandle (
                              pStore,
                              &pSecurityDescriptor
                              );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdGetStoreFromHandle (
                                        pStore,
                                        pConnectionContext->pSecurityContext,
                                        &pStoreInstance
                                      );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdModifyOwner (
                                pStoreInstance,
                                pszUserName,
                                pSecurityDescriptor
                                );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdSetSecurityDescriptorForHandle (
                                pStore,
                                pSecurityDescriptor
                                );
    BAIL_ON_VMAFD_ERROR (dwError);

    pthread_mutex_unlock (&gVmafdGlobals.mutexStoreState);

    bIsHoldingLock = FALSE;

cleanup:
    if (pSecurityDescriptor)
    {
        VmAfdFreeSecurityDescriptor (pSecurityDescriptor);
    }

    if (bIsHoldingLock)
    {
        pthread_mutex_unlock(&gVmafdGlobals.mutexStoreState);
    }

    VMAFD_SAFE_FREE_MEMORY (pStoreInstance);

    return dwError;

error:
    goto cleanup;
}

DWORD
VecsSrvEnumFilteredStores (
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PWSTR **ppwszStoreNames,
    PDWORD pdwCount
    )
{
    DWORD dwError = 0;
    DWORD dwCount = 0;
    PBYTE pContextBlob = NULL;
    DWORD dwContextSize = 0;
    DWORD dwContextSizeRead = 0;

    PWSTR *pwszStoreName = NULL;

    if (!pConnectionContext ||
        !pConnectionContext->pSecurityContext ||
        !ppwszStoreNames ||
        !pdwCount
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (VmAfdIsRootSecurityContext(pConnectionContext))
    {
        dwError = VecsSrvEnumCertStore(
                                       &pwszStoreName,
                                       &dwCount
                                      );
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    else
    {

        dwError = VmAfdGetSecurityContextSize (
                                            pConnectionContext->pSecurityContext,
                                            &dwContextSize
                                          );
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VmAfdAllocateMemory (
                                    dwContextSize,
                                    (PVOID *) &pContextBlob
                                  );
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VmAfdEncodeSecurityContext (
                                          pConnectionContext->pSecurityContext,
                                          pContextBlob,
                                          dwContextSize,
                                          &dwContextSizeRead
                                        );
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VecsDbEnumFilteredStores (
                                        pContextBlob,
                                        dwContextSizeRead,
                                        &pwszStoreName,
                                        &dwCount
                                       );
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    *ppwszStoreNames = pwszStoreName;
    *pdwCount = dwCount;

cleanup:
    VMAFD_SAFE_FREE_MEMORY (pContextBlob);

    return dwError;

error:
    if (ppwszStoreNames)
    {
        *ppwszStoreNames = NULL;
    }

    if (pwszStoreName)
    {
        VmAfdFreeStringArrayW (pwszStoreName, dwCount);
    }

    if (pdwCount)
    {
        *pdwCount = 0;
    }

    goto cleanup;
}


DWORD
VecsSrvCloseCertStoreHandle (
                              PVECS_SRV_STORE_HANDLE pStore,
                              PVM_AFD_CONNECTION_CONTEXT pConnectionContext
                            )
{
    DWORD dwError = 0;

    if (!pStore ||
        !pConnectionContext ||
        !pConnectionContext->pSecurityContext
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR  (dwError);
    }

    if (!VmAfdIsValidStoreHandle (pStore,pConnectionContext->pSecurityContext))
    {
        dwError = ERROR_INVALID_HANDLE;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    VmAfdCloseStoreHandle(
                           pStore,
                           pConnectionContext->pSecurityContext
                         );

    if (pStore != pConnectionContext->pStoreHandle)
    {
        VMAFD_SAFE_FREE_MEMORY(pConnectionContext->pStoreHandle);
        pConnectionContext->pStoreHandle = NULL;
    }
    BAIL_ON_VMAFD_ERROR (dwError);
cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VecsSrvAllocateCertEnumContextHandle(
    PVECS_SRV_STORE_HANDLE         pStore,
    DWORD                          dwMaxCount,
    ENTRY_INFO_LEVEL               infoLevel,
    PVECS_SRV_ENUM_CONTEXT_HANDLE* ppContext
    )
{
    DWORD dwError = 0;
    PVECS_SRV_ENUM_CONTEXT_HANDLE pContext = NULL;

    dwError = VmAfdAllocateMemory(
                    sizeof(VECS_SRV_ENUM_CONTEXT_HANDLE),
                    (PVOID*)&pContext);
    BAIL_ON_VMAFD_ERROR(dwError);

    pContext->pStore = VmAfdAcquireStoreHandle(pStore);

    if (!dwMaxCount || (dwMaxCount > 256))
    {
        pContext->dwLimit = 256;
    }
    else
    {
        pContext->dwLimit = dwMaxCount;
    }
    pContext->infoLevel = infoLevel;

    *ppContext = pContext;

cleanup:

    return dwError;

error:

    if (ppContext)
    {
        *ppContext = NULL;
    }

    goto cleanup;
}

DWORD
VecsSrvEnumCertsHandle(
    PVECS_SRV_ENUM_CONTEXT_HANDLE pContext,
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PVMAFD_CERT_ARRAY *ppCertContainer
    )
{
    DWORD dwError = 0;
    PVMAFD_CERT_ARRAY pCertContainer = NULL;
    PVECS_SERV_STORE pStore = NULL;

    dwError = VmAfdGetStoreFromHandle (
                                        pContext->pStore,
                                        pConnectionContext->pSecurityContext,
                                        &pStore
                                      );
    BAIL_ON_VMAFD_ERROR (dwError);

    switch (pContext->infoLevel)
    {
        case ENTRY_INFO_LEVEL_1:
          dwError = VecsDbEnumInfoLevel1(
                        pStore->dwStoreId,
                        pContext->dwIndex,
                        pContext->dwLimit,
                        &pCertContainer
                        );
          BAIL_ON_VMAFD_ERROR (dwError);
          break;

        case ENTRY_INFO_LEVEL_2:
          dwError = VecsDbEnumInfoLevel2(
                      pStore->dwStoreId,
                      pContext->dwIndex,
                      pContext->dwLimit,
                      &pCertContainer
                      );
          BAIL_ON_VMAFD_ERROR(dwError);
          break;

       default:
          dwError = ERROR_INVALID_PARAMETER;
          BAIL_ON_VMAFD_ERROR (dwError);
    }

    *ppCertContainer = pCertContainer;

cleanup:

    VMAFD_SAFE_FREE_MEMORY (pStore);

    return dwError;
error:
    if (ppCertContainer)
    {
      *ppCertContainer = NULL;
    }
    if (pCertContainer)
    {
        VmAfdFreeCertArray(pCertContainer);
    }

    goto cleanup;
}

DWORD
VecsSrvEndEnumContextHandle (
           PVECS_SRV_ENUM_CONTEXT_HANDLE pEnumContext,
           PVM_AFD_CONNECTION_CONTEXT pConnectionContext
          )
{
    DWORD dwError = 0;

    if (!pEnumContext ||
        !pConnectionContext
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!VmAfdIsValidStoreHandle(
                      pEnumContext->pStore,
                      pConnectionContext->pSecurityContext
                      )
       )
    {
        dwError = ERROR_INVALID_HANDLE;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    VmAfdReleaseStoreHandle(
                            pEnumContext->pStore
                           );

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VecsSrvValidateAceType (
         DWORD dwAceType,
         VMAFD_ACE_TYPE *pAceType
         )
{
    DWORD dwError = 0;
    VMAFD_ACE_TYPE aceType = VMAFD_ACE_TYPE_UNKNOWN;

    if (!pAceType)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    switch (dwAceType)
    {
        case 1:
          aceType = VMAFD_ACE_TYPE_ALLOWED;
          break;
        case 2:
          aceType = VMAFD_ACE_TYPE_DENIED;
          break;
        default:
          aceType = VMAFD_ACE_TYPE_UNKNOWN;
          dwError = ERROR_INVALID_PARAMETER;
          break;
    }

cleanup:
    return dwError;

error:
    if (pAceType)
    {
        *pAceType = aceType;
    }

    goto cleanup;
}

DWORD
VecsSrvGetPermissions (
    PVECS_SRV_STORE_HANDLE pStore,
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PWSTR *ppszOwnerName,
    PDWORD pdwUserCount,
    PVECS_STORE_PERMISSION_W *ppPermissions
    )
{
    DWORD dwError = 0;
    PWSTR pszOwnerName = NULL;
    DWORD dwUserCount = 0;
    PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    PVECS_STORE_PERMISSION_W pPermissions = NULL;


    if (!pStore ||
        !ppszOwnerName ||
        !pdwUserCount ||
        !ppPermissions
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdCheckOwnerShipWithHandle(
                                    pStore,
                                    pConnectionContext
                                    );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdGetSecurityDescriptorFromHandle (
                                           pStore,
                                           &pSecurityDescriptor
                                           );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdAllocateNameFromContext (
                                  pSecurityDescriptor->pOwnerSecurityContext,
                                  &pszOwnerName
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (
        pSecurityDescriptor->pAcl &&
        pSecurityDescriptor->pAcl->dwAceCount
       )
    {
        DWORD dwIndex = 0;
        PVMAFD_ACE_LIST pAceListCursor = NULL;
        dwUserCount = pSecurityDescriptor->pAcl->dwAceCount;

        dwError = VmAfdAllocateMemory(
                              dwUserCount * sizeof (VECS_STORE_PERMISSION_W),
                              (PVOID *)&pPermissions
                              );
        BAIL_ON_VMAFD_ERROR (dwError);

        pAceListCursor = pSecurityDescriptor->pAcl->pAceList;

        for (; pAceListCursor && dwIndex < dwUserCount; dwIndex++)
        {

          PVECS_STORE_PERMISSION_W pCursor = &pPermissions[dwIndex];

          dwError = VmAfdAllocateNameFromContext(
                                  pAceListCursor->Ace.pSecurityContext,
                                  &pCursor->pszUserName
                                  );
          BAIL_ON_VMAFD_ERROR (dwError);

          pCursor->dwAccessMask = pAceListCursor->Ace.accessMask;

          pAceListCursor = pAceListCursor->pNext;
        }
    }

    *ppszOwnerName = pszOwnerName;
    *pdwUserCount = dwUserCount;
    *ppPermissions = pPermissions;

cleanup:
    if (pSecurityDescriptor)
    {
        VmAfdFreeSecurityDescriptor(pSecurityDescriptor);
    }

    return dwError;

error:
    if (ppszOwnerName)
    {
        *ppszOwnerName = NULL;
    }
    if (pdwUserCount)
    {
        *pdwUserCount = 0;
    }
    if (ppPermissions)
    {
        *ppPermissions = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY (pszOwnerName);
    if (pPermissions)
    {
        VmAfdFreeStorePermissionArray(
                              pPermissions,
                              dwUserCount
                              );
    }

    goto cleanup;
}
