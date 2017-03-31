/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : storehash_util.c
 *
 * Abstract :
 *
 */
#include "includes.h"

/* This declaration is needed to complete the incomplete type.
 * It must be kept in sync with the type in common/structs.h
 */
typedef struct _VM_AFD_SECURITY_CONTEXT_
{
#if defined _WIN32
    PSID pSid;
#else
    uid_t uid;
#endif
} VM_AFD_SECURITY_CONTEXT;

static
DWORD
VmAfdComputeStoreMapHash (
                          DWORD dwStoreId,
                          PDWORD pdwHashedIndex
                         );
static
VOID
VmAfdReleaseStoreEntry(
                          DWORD dwHashedIndx
                       );

static
DWORD
VmAfdCloseStoreInstance (
                          DWORD dwHashedIndx,
                          PVM_AFD_SECURITY_CONTEXT pSecurityContext,
                          uintptr_t dwClientInstance
                        );
static
DWORD
VmAfdInitializeStoreEntry (
                            DWORD dwStoreId,
                            PVM_AFD_SECURITY_CONTEXT pSecurityContext,
                            DWORD dwHashedIndx
                          );

static
DWORD
VmAfdCreateNewClientInstance (
                              PVM_AFD_SECURITY_CONTEXT pSecurityContext,
                              DWORD dwHashedIndx,
                              uintptr_t* pdwClientInstance
                             );

static
VOID
VmAfdFreeContextListEntry (
                            PVECS_STORE_CONTEXT_LIST pContextListEntry
                          );

static
VOID
VmAfdFreeContextList (
                       PVECS_STORE_CONTEXT_LIST pContextList
                     );

static
VOID
VmAfdCleanSecurityDescriptor (
                      PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor
                      );

DWORD
VmAfdGetStoreHandle (
                      PWSTR pszStoreName,
                      PWSTR pszPassword,
                      PVM_AFD_SECURITY_CONTEXT pSecurityContext,
                      PVECS_SRV_STORE_HANDLE *ppStore
                    )
{
    DWORD dwError = 0;
    PVECS_SRV_STORE_HANDLE pStore = NULL;
    BOOL bIsHoldingLock = FALSE;
    DWORD dwStoreId = 0;
    DWORD dwHashedIndx = -1;
    uintptr_t dwClientInstance = 0;
    PSTR pszStoreNameA = NULL;

    if (IsNullOrEmptyString (pszStoreName) ||
        !pSecurityContext ||
        !ppStore
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VecsDbGetCertStore(
                                  pszStoreName,
                                  pszPassword,
                                  &dwStoreId
                                );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdComputeStoreMapHash(
                                        dwStoreId,
                                        &dwHashedIndx
                                      );
    BAIL_ON_VMAFD_ERROR (dwError);

    VMAFD_LOCK_MUTEX_EXCLUSIVE (&gVmafdGlobals.rwlockStoreMap, bIsHoldingLock);

    if (gVecsGlobalStoreMap[dwHashedIndx].status ==
                                 STORE_MAP_ENTRY_STATUS_EMPTY)
    {
        dwError = VmAfdInitializeStoreEntry (
                                              dwStoreId,
                                              pSecurityContext,
                                              dwHashedIndx
                                            );
        BAIL_ON_VMAFD_ERROR (dwError);
        dwClientInstance = (uintptr_t)gVecsGlobalStoreMap[dwHashedIndx].pStoreContextList;
    }
    else
    {
        dwError = VmAfdCreateNewClientInstance (
                                                 pSecurityContext,
                                                 dwHashedIndx,
                                                 &dwClientInstance
                                               );
        BAIL_ON_VMAFD_ERROR (dwError);

        gVecsGlobalStoreMap[dwHashedIndx].pStore =
           VecsSrvAcquireCertStore (gVecsGlobalStoreMap[dwHashedIndx].pStore);
    }

    gVecsGlobalStoreMap[dwHashedIndx].status =
                                       STORE_MAP_ENTRY_STATUS_OPEN;

    VMAFD_LOCK_MUTEX_UNLOCK (&gVmafdGlobals.rwlockStoreMap, bIsHoldingLock);

    dwError = VmAfdAllocateMemory (
                                   sizeof (VECS_SRV_STORE_HANDLE),
                                   (PVOID *) &pStore
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    pStore->dwStoreHandle = dwHashedIndx;
    pStore->dwClientInstance = dwClientInstance;

    pStore->dwStoreSessionID =
      gVecsGlobalStoreMap[dwHashedIndx].dwStoreSessionID;

    *ppStore = pStore;

cleanup:

    VMAFD_LOCK_MUTEX_UNLOCK (&gVmafdGlobals.rwlockStoreMap, bIsHoldingLock);

    return dwError;

error:
    if (dwError == ERROR_NO_MORE_USER_HANDLES &&
        pszStoreName && pSecurityContext)
    {
        DWORD dwError2 = 0;

        dwError2 = VmAfdAllocateStringAFromW(
                         pszStoreName,
                         &pszStoreNameA);
        if (dwError2 == 0)
        {
#ifndef _WIN32
            VmAfdLog(VMAFD_DEBUG_ANY,
                     "No VECS user handles available.  Store %s, client uid %d",
                     pszStoreNameA, pSecurityContext->uid);
#else
            VmAfdLog(VMAFD_DEBUG_ANY,
                     "No VECS user handles available.  Store %s",
                     pszStoreNameA);
#endif
            VMAFD_SAFE_FREE_STRINGA(pszStoreNameA);
        }
    }
    if (ppStore)
    {
        *ppStore = NULL;
    }
    if (dwHashedIndx != -1 &&
        dwClientInstance &&
        pSecurityContext
       )
    {
        VMAFD_LOCK_MUTEX_EXCLUSIVE (&gVmafdGlobals.rwlockStoreMap, bIsHoldingLock);
        VmAfdCloseStoreInstance (
                                  dwHashedIndx,
                                  pSecurityContext,
                                  dwClientInstance
                                );
        VMAFD_LOCK_MUTEX_UNLOCK (&gVmafdGlobals.rwlockStoreMap, bIsHoldingLock);
    }
    VMAFD_SAFE_FREE_MEMORY (pStore);

    goto cleanup;
}

VOID
VmAfdCloseStoreHandle (
                        PVECS_SRV_STORE_HANDLE pStoreHandle,
                        PVM_AFD_SECURITY_CONTEXT pSecurityContext
                      )
{
    if (pStoreHandle)
    {
        BOOLEAN bIsHoldingLock = FALSE;
        VMAFD_LOCK_MUTEX_EXCLUSIVE (&gVmafdGlobals.rwlockStoreMap, bIsHoldingLock);
        VmAfdCloseStoreInstance (
                                 pStoreHandle->dwStoreHandle,
                                 pSecurityContext,
                                 pStoreHandle->dwClientInstance
                                );
        VMAFD_LOCK_MUTEX_UNLOCK (&gVmafdGlobals.rwlockStoreMap, bIsHoldingLock);
        VMAFD_SAFE_FREE_MEMORY (pStoreHandle);
    }
}

PVECS_SRV_STORE_HANDLE
VmAfdAcquireStoreHandle (
                          PVECS_SRV_STORE_HANDLE pStoreHandle
                        )
{
    if (
        pStoreHandle &&
        gVecsGlobalStoreMap[pStoreHandle->dwStoreHandle].pStore
       )
    {
        PVECS_SERV_STORE pStore = NULL;
        BOOLEAN bIsHoldingLock = FALSE;

        VMAFD_LOCK_MUTEX_EXCLUSIVE (&gVmafdGlobals.rwlockStoreMap, bIsHoldingLock);

        pStore =
          gVecsGlobalStoreMap[pStoreHandle->dwStoreHandle].pStore;

        pStore = VecsSrvAcquireCertStore (pStore);
        VMAFD_LOCK_MUTEX_UNLOCK (&gVmafdGlobals.rwlockStoreMap, bIsHoldingLock);
    }

    return pStoreHandle;
}

VOID
VmAfdReleaseStoreHandle (
                          PVECS_SRV_STORE_HANDLE pStoreHandle
                        )
{
    if (
        pStoreHandle
        )
    {
       BOOLEAN bIsHoldingLock = FALSE;

       VMAFD_LOCK_MUTEX_EXCLUSIVE (&gVmafdGlobals.rwlockStoreMap, bIsHoldingLock);

        VmAfdReleaseStoreEntry (pStoreHandle->dwStoreHandle);

        VMAFD_LOCK_MUTEX_UNLOCK (&gVmafdGlobals.rwlockStoreMap, bIsHoldingLock);
    }
}

BOOL
VmAfdIsValidStoreHandle (
                PVECS_SRV_STORE_HANDLE pStore,
                PVM_AFD_SECURITY_CONTEXT pSecurityContext
              )
{
    VECS_SRV_STORE_MAP storeMapEntry = {0};
    PVECS_STORE_CONTEXT_LIST pContextListCursor = NULL;
    BOOL bResult = 0;
    BOOL bIsHoldingLock = FALSE;

    if (!pStore ||
        !pSecurityContext
       )
    {
        goto error;
    }

    VMAFD_LOCK_MUTEX_SHARED (&gVmafdGlobals.rwlockStoreMap, bIsHoldingLock);

    storeMapEntry = gVecsGlobalStoreMap[pStore->dwStoreHandle];

    if (storeMapEntry.status != STORE_MAP_ENTRY_STATUS_OPEN)
    {
       goto error;
    }

    if (!storeMapEntry.pStore)
    {
        goto error;
    }

    if (pStore->dwStoreSessionID != storeMapEntry.dwStoreSessionID)
    {
        goto error;
    }

    pContextListCursor = storeMapEntry.pStoreContextList;

    while (pContextListCursor)
    {
        if ((pContextListCursor == (PVECS_STORE_CONTEXT_LIST)pStore->dwClientInstance) &&
            VmAfdEqualsSecurityContext(
                                          pContextListCursor->pSecurityContext,
                                          pSecurityContext
                                       )
           )
        {
            bResult = 1;
            break;
        }

        pContextListCursor = pContextListCursor->pNext;
    }

error:

    VMAFD_LOCK_MUTEX_UNLOCK (&gVmafdGlobals.rwlockStoreMap, bIsHoldingLock);

    return bResult;
}

DWORD
VmAfdGetStoreStatus(
                    PVECS_SRV_STORE_HANDLE pStore,
                    STORE_MAP_ENTRY_STATUS *pStoreStatus
                   )
{
    DWORD dwError = 0;
    STORE_MAP_ENTRY_STATUS storeStatus = STORE_MAP_ENTRY_STATUS_EMPTY;
    BOOLEAN bIsHoldingLock = FALSE;

    if (
        !pStore ||
        !pStoreStatus
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }


    VMAFD_LOCK_MUTEX_SHARED (&gVmafdGlobals.rwlockStoreMap, bIsHoldingLock);


    /*TODO: Do we really need to validate handle before
     * returning the store status?. Also where is this call
     * used at all
     */

    /*if (!VmAfdIsValidStoreHandle(
                                  pStore,
                                  pSecurityContext
                                )
        )
    {
        dwError = ERROR_INVALID_HANDLE;
        BAIL_ON_VMAFD_ERROR (dwError);
    }*/


    storeStatus = gVecsGlobalStoreMap[pStore->dwStoreHandle].status;

    *pStoreStatus = storeStatus;

cleanup:

    VMAFD_LOCK_MUTEX_UNLOCK (&gVmafdGlobals.rwlockStoreMap, bIsHoldingLock);

    return dwError;
error:
    if (pStoreStatus)
    {
        *pStoreStatus = STORE_MAP_ENTRY_STATUS_EMPTY;
    }

    goto cleanup;
}

DWORD
VmAfdGetStoreFromHandle (
                          PVECS_SRV_STORE_HANDLE pStoreHandle,
                          PVM_AFD_SECURITY_CONTEXT pSecurityContext,
                          PVECS_SERV_STORE *ppStore
                        )
{
    DWORD dwError = 0;
    PVECS_SERV_STORE pStore = NULL;
    VECS_SRV_STORE_MAP storeMapEntry = {0};

    BOOL bIsHoldingLock = FALSE;

    if (
        !pStoreHandle ||
        !ppStore
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }


    if (!VmAfdIsValidStoreHandle (
                                  pStoreHandle,
                                  pSecurityContext
                                 )
       )
    {
        dwError = ERROR_INVALID_HANDLE;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    VMAFD_LOCK_MUTEX_SHARED (&gVmafdGlobals.rwlockStoreMap, bIsHoldingLock);

    storeMapEntry = gVecsGlobalStoreMap[pStoreHandle->dwStoreHandle];

    if (
        storeMapEntry.status != STORE_MAP_ENTRY_STATUS_OPEN ||
        !storeMapEntry.pStore
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateMemory (
                                    sizeof (VECS_SERV_STORE),
                                    (PVOID *) &pStore
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);


    pStore->dwStoreId = storeMapEntry.pStore->dwStoreId;
    pStore->refCount = storeMapEntry.pStore->refCount;

    *ppStore = pStore;
cleanup:

    VMAFD_LOCK_MUTEX_UNLOCK (&gVmafdGlobals.rwlockStoreMap, bIsHoldingLock);
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
VmAfdGetSecurityDescriptorFromHandle (
                             PVECS_SRV_STORE_HANDLE pStore,
                             PVMAFD_SECURITY_DESCRIPTOR *ppSecurityDescriptor
                           )
{
    DWORD dwError = 0;
    PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    BOOL bIsHoldingLock = FALSE;

    VECS_SRV_STORE_MAP storeMapEntry = {0};

    if (!pStore ||
        !ppSecurityDescriptor
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }


    VMAFD_LOCK_MUTEX_SHARED (&gVmafdGlobals.rwlockStoreMap, bIsHoldingLock);

    storeMapEntry = gVecsGlobalStoreMap[pStore->dwStoreHandle];

    if (storeMapEntry.status != STORE_MAP_ENTRY_STATUS_OPEN)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }


    dwError = VmAfdCopySecurityDescriptor (
                                           storeMapEntry.pSecurityDescriptor,
                                           &pSecurityDescriptor
                                          );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppSecurityDescriptor = pSecurityDescriptor;

cleanup:

    VMAFD_LOCK_MUTEX_UNLOCK (&gVmafdGlobals.rwlockStoreMap, bIsHoldingLock);

    return dwError;

error:
    if (ppSecurityDescriptor)
    {
        *ppSecurityDescriptor = NULL;
    }

    if (pSecurityDescriptor)
    {
        VmAfdFreeSecurityDescriptor (pSecurityDescriptor);
    }

    goto cleanup;
}

DWORD
VmAfdSetSecurityDescriptorForHandle (
                             PVECS_SRV_STORE_HANDLE pStore,
                             PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor
                             )
{
    DWORD dwError = 0;
    PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptorTmp = NULL;
    PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptorToClean = NULL;
    BOOL bIsHoldingLock = FALSE;

    if (!pStore ||
        !pSecurityDescriptor
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdCopySecurityDescriptor (
                                            pSecurityDescriptor,
                                            &pSecurityDescriptorTmp
                                          );
    BAIL_ON_VMAFD_ERROR (dwError);


    VMAFD_LOCK_MUTEX_EXCLUSIVE (&gVmafdGlobals.rwlockStoreMap, bIsHoldingLock);

    dwError = VecsDbSetSecurityDescriptor (
              gVecsGlobalStoreMap[pStore->dwStoreHandle].pStore->dwStoreId,
              pSecurityDescriptorTmp
              );
    BAIL_ON_VMAFD_ERROR (dwError);

    pSecurityDescriptorToClean =
      gVecsGlobalStoreMap[pStore->dwStoreHandle].pSecurityDescriptor;

    VmAfdCleanSecurityDescriptor (pSecurityDescriptorTmp);

    gVecsGlobalStoreMap[pStore->dwStoreHandle].pSecurityDescriptor =
                            pSecurityDescriptorTmp;

cleanup:
    if (pSecurityDescriptorToClean)
    {
        VmAfdFreeSecurityDescriptor(pSecurityDescriptorToClean);
    }

    VMAFD_LOCK_MUTEX_UNLOCK (&gVmafdGlobals.rwlockStoreMap, bIsHoldingLock);

    return dwError;

error:
    if (pSecurityDescriptorTmp)
    {
        VmAfdFreeSecurityDescriptor (pSecurityDescriptorTmp);
    }

    goto cleanup;
}

BOOL
VmAfdCanDeleteStore (
                      PVECS_SRV_STORE_HANDLE pStoreHandle
                    )
{
    BOOL bResult = 0;

    if (pStoreHandle)
    {

        BOOLEAN bIsHoldingLock = FALSE;

        VMAFD_LOCK_MUTEX_SHARED (&gVmafdGlobals.rwlockStoreMap, bIsHoldingLock);

        if (gVecsGlobalStoreMap[pStoreHandle->dwStoreHandle].pStore &&
            gVecsGlobalStoreMap[pStoreHandle->dwStoreHandle].pStore->refCount ==1
           )
        {
            bResult = 1;
        }

        VMAFD_LOCK_MUTEX_UNLOCK (&gVmafdGlobals.rwlockStoreMap, bIsHoldingLock);
    }

    return bResult;
}

VOID
VmAfdDeleteStoreEntry (
                        PVECS_SRV_STORE_HANDLE pStoreHandle
                      )
{
    DWORD dwStoreMapIndex = 0;
    if (pStoreHandle)
    {
        BOOLEAN bIsHoldingLock = FALSE;
        dwStoreMapIndex = pStoreHandle->dwStoreHandle;

        VMAFD_LOCK_MUTEX_EXCLUSIVE (&gVmafdGlobals.rwlockStoreMap, bIsHoldingLock);

        VMAFD_SAFE_FREE_MEMORY (
                            gVecsGlobalStoreMap[dwStoreMapIndex].pStore
                            );

        VmAfdFreeSecurityDescriptor (
                            gVecsGlobalStoreMap[dwStoreMapIndex].pSecurityDescriptor
                            );

        gVecsGlobalStoreMap[dwStoreMapIndex].pSecurityDescriptor = NULL;

        VmAfdFreeContextList (
                            gVecsGlobalStoreMap[dwStoreMapIndex].pStoreContextList
                            );
        gVecsGlobalStoreMap[dwStoreMapIndex].pStoreContextList = NULL;

        gVecsGlobalStoreMap[dwStoreMapIndex].status = STORE_MAP_ENTRY_STATUS_EMPTY;

        VMAFD_LOCK_MUTEX_UNLOCK (&gVmafdGlobals.rwlockStoreMap, bIsHoldingLock);

    }

    VMAFD_SAFE_FREE_MEMORY (pStoreHandle);
}



VOID
VmAfdTearDownStoreHashMap (
                            VOID
                          )
{
    DWORD dwIndex = 0;
    BOOLEAN bIsHoldingLock = FALSE;

    VMAFD_LOCK_MUTEX_EXCLUSIVE (&gVmafdGlobals.rwlockStoreMap, bIsHoldingLock);

    for (; dwIndex < VECS_STOREHASH_MAP_SIZE; dwIndex++)
    {
        VECS_SRV_STORE_MAP storeMapEntry = gVecsGlobalStoreMap[dwIndex];

        VMAFD_SAFE_FREE_MEMORY (storeMapEntry.pStore);

        if (storeMapEntry.pSecurityDescriptor)
        {
            VmAfdFreeSecurityDescriptor (storeMapEntry.pSecurityDescriptor);
            storeMapEntry.pSecurityDescriptor = NULL;
        }

        if (storeMapEntry.pStoreContextList)
        {
            VmAfdFreeContextList(storeMapEntry.pStoreContextList);
            storeMapEntry.pStoreContextList = NULL;
        }


        gVecsGlobalStoreMap[dwIndex].status = STORE_MAP_ENTRY_STATUS_EMPTY;
    }

    VMAFD_LOCK_MUTEX_UNLOCK (&gVmafdGlobals.rwlockStoreMap, bIsHoldingLock);

}

static
VOID
VmAfdReleaseStoreEntry(
                          DWORD dwHashedIndx
                       )
{

    if (
          gVecsGlobalStoreMap[dwHashedIndx].pStore
       )
    {
            PVECS_SERV_STORE pStore =
                gVecsGlobalStoreMap[dwHashedIndx].pStore;

            if (InterlockedDecrement(&pStore->refCount) == 0)
            {
                gVecsGlobalStoreMap[dwHashedIndx].status =
                          STORE_MAP_ENTRY_STATUS_CLOSED;
                if (gVecsGlobalStoreMap[dwHashedIndx].pStoreContextList)
                {
                    VmAfdFreeContextList (
                                    gVecsGlobalStoreMap[dwHashedIndx].pStoreContextList
                                    );
                    gVecsGlobalStoreMap[dwHashedIndx].pStoreContextList = NULL;
                }
            }
     }

}

//TODO: Change to VOID?
static
DWORD
VmAfdCloseStoreInstance (
                          DWORD dwHashedIndx,
                          PVM_AFD_SECURITY_CONTEXT pSecurityContext,
                          uintptr_t dwClientInstance
                        )
{
    DWORD dwError = 0;
    PVECS_STORE_CONTEXT_LIST pContextCursor = NULL;
    PVECS_STORE_CONTEXT_LIST pContextListEntryToClean = NULL;

    if (dwHashedIndx == -1 ||
        !pSecurityContext ||
        !dwClientInstance
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (gVecsGlobalStoreMap[dwHashedIndx].pStoreContextList)
    {
        pContextCursor = gVecsGlobalStoreMap[dwHashedIndx].pStoreContextList;
    }

    while (pContextCursor)
    {
        if ((pContextCursor == (PVECS_STORE_CONTEXT_LIST)dwClientInstance) &&
            VmAfdEqualsSecurityContext(
                                    pContextCursor->pSecurityContext,
                                    pSecurityContext
                                    )
           )
        {
            pContextListEntryToClean = pContextCursor;

            if (pContextCursor->pPrev)
            {
                pContextCursor->pPrev->pNext = pContextCursor->pNext;
            }
            else
            {
                gVecsGlobalStoreMap[dwHashedIndx].pStoreContextList =
                                pContextCursor->pNext;
            }

            if (pContextCursor->pNext)
            {
                pContextCursor->pNext->pPrev = pContextCursor->pPrev;
            }
            break;
        }

        pContextCursor  = pContextCursor->pNext;
    }

    VmAfdReleaseStoreEntry (
                            dwHashedIndx
                           );

cleanup:
    VmAfdFreeContextListEntry (
                                pContextListEntryToClean
                              );
    return dwError;

error:
    goto cleanup;
}


static
VOID
VmAfdFreeContextListEntry (
                            PVECS_STORE_CONTEXT_LIST pContextListEntry
                          )
{
    if (pContextListEntry)
    {
        if (pContextListEntry->pSecurityContext)
        {
            VmAfdFreeSecurityContext(
                pContextListEntry->pSecurityContext
                );
        }
        VMAFD_SAFE_FREE_MEMORY (pContextListEntry);
    }
}

static
VOID
VmAfdFreeContextList (
                       PVECS_STORE_CONTEXT_LIST pContextList
                     )
{
    PVECS_STORE_CONTEXT_LIST pContextListCursor = NULL;
    PVECS_STORE_CONTEXT_LIST pContextListCursorNext = NULL;

    pContextListCursor = pContextList;

    while (pContextListCursor)
    {
        pContextListCursorNext = pContextListCursor->pNext;

        VmAfdFreeContextListEntry (pContextListCursor);

        pContextListCursor = pContextListCursorNext;
    }
}

static
DWORD
VmAfdInitializeStoreEntry (
                            DWORD dwStoreId,
                            PVM_AFD_SECURITY_CONTEXT pSecurityContext,
                            DWORD dwHashedIndx
                          )
{
    DWORD dwError = 0;
    PVECS_SERV_STORE pStore = NULL;
    PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    PVECS_STORE_CONTEXT_LIST pStoreContextList = NULL;
    VECS_SRV_STORE_MAP storeMapEntry = gVecsGlobalStoreMap[dwHashedIndx];


    if (!dwStoreId ||
        !pSecurityContext
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateMemory (
                                    sizeof (VECS_SERV_STORE),
                                    (PVOID *)&pStore
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    pStore->refCount = 1;
    pStore->dwStoreId = dwStoreId;


    dwError = VecsDbGetSecurityDescriptor (
                                            dwStoreId,
                                            &pSecurityDescriptor
                                          );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdAllocateMemory (
                                    sizeof (VECS_STORE_CONTEXT_LIST),
                                    (PVOID *)&pStoreContextList
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdCopySecurityContext(
                                        pSecurityContext,
                                        &pStoreContextList->pSecurityContext
                                      );
    BAIL_ON_VMAFD_ERROR (dwError);

    storeMapEntry.pStore = pStore;
    storeMapEntry.pSecurityDescriptor = pSecurityDescriptor;
    storeMapEntry.pStoreContextList = pStoreContextList;

    dwError = VmAfdGenRandom (
                          &(storeMapEntry.dwStoreSessionID)
                          );
    BAIL_ON_VMAFD_ERROR (dwError);

    gVecsGlobalStoreMap[dwHashedIndx] = storeMapEntry;

cleanup:

    return dwError;

error:
    if (pStore)
    {
        VecsSrvReleaseCertStore (pStore);
    }
    if (pSecurityDescriptor)
    {
        VmAfdFreeSecurityDescriptor (pSecurityDescriptor);
    }
    if (pStoreContextList)
    {
        VmAfdFreeContextList (pStoreContextList);
    }


    goto cleanup;
}

static
DWORD
VmAfdCreateNewClientInstance (
                              PVM_AFD_SECURITY_CONTEXT pSecurityContext,
                              DWORD dwHashedIndx,
                              uintptr_t* pdwClientInstance
                             )
{
    DWORD dwError = 0;
    PVECS_STORE_CONTEXT_LIST pContextListEntryNew = NULL;
    PVM_AFD_SECURITY_CONTEXT pSecurityContextTmp = NULL;
    uintptr_t dwClientInstance = 0;
    VECS_SRV_STORE_MAP storeMapEntry = gVecsGlobalStoreMap[dwHashedIndx];

    if (!pSecurityContext ||
        !pdwClientInstance
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdCopySecurityContext (
                                        pSecurityContext,
                                        &pSecurityContextTmp
                                       );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdAllocateMemory (
                                   sizeof (VECS_STORE_CONTEXT_LIST),
                                   (PVOID *)&pContextListEntryNew
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    pContextListEntryNew->pSecurityContext = pSecurityContextTmp;
    pContextListEntryNew->pNext = storeMapEntry.pStoreContextList;

    if (storeMapEntry.pStoreContextList)
    {
      storeMapEntry.pStoreContextList->pPrev =
                                         pContextListEntryNew;
    }

    gVecsGlobalStoreMap[dwHashedIndx].pStoreContextList = pContextListEntryNew;

    dwClientInstance = (uintptr_t)pContextListEntryNew;

    *pdwClientInstance = dwClientInstance;

cleanup:

    return dwError;

error:
    if (pdwClientInstance)
    {
        *pdwClientInstance = 0;
    }
    if (pContextListEntryNew)
    {
        VmAfdFreeContextListEntry(
                                  pContextListEntryNew
                                 );
    }
    if (pSecurityContextTmp)
    {
        VmAfdFreeSecurityContext(
                                  pSecurityContextTmp
                                );
    }

    goto cleanup;
}

static
DWORD
VmAfdComputeStoreMapHash (
                          DWORD dwStoreId,
                          PDWORD pdwHashedIndex
                        )
{
    DWORD dwError = 0;
    DWORD dwHashedIndx = 0;
    DWORD dwHashedIndxInitial = 0;

    if (!dwStoreId)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwHashedIndx = (dwStoreId*131) % VECS_STOREHASH_MAP_SIZE;

    dwHashedIndxInitial = dwHashedIndx;

   while ((gVecsGlobalStoreMap[dwHashedIndx].status !=
                STORE_MAP_ENTRY_STATUS_EMPTY)
          )
   {
      if (gVecsGlobalStoreMap[dwHashedIndx].pStore &&
          (gVecsGlobalStoreMap[dwHashedIndx].pStore->dwStoreId == dwStoreId)
         )
      {
          break;
      }
      //TODO: This should be bettered
      //This won't scale well
      dwHashedIndx++;

      if (dwHashedIndx == VECS_STOREHASH_MAP_SIZE)
      {
          dwHashedIndx = 0;
      }

      if (dwHashedIndx == dwHashedIndxInitial)
      {
          break;
      }
   }

   if ((dwHashedIndx == dwHashedIndxInitial) &&
       gVecsGlobalStoreMap[dwHashedIndx].status != STORE_MAP_ENTRY_STATUS_EMPTY &&
       gVecsGlobalStoreMap[dwHashedIndx].pStore->dwStoreId != dwStoreId
      )
   {
      while (gVecsGlobalStoreMap[dwHashedIndx].status !=
             STORE_MAP_ENTRY_STATUS_CLOSED
            )
      {
          dwHashedIndx ++;
          if (dwHashedIndx == VECS_STOREHASH_MAP_SIZE)
          {
              dwHashedIndx = 0;
          }
          if (dwHashedIndx == dwHashedIndxInitial)
          {
              break;
          }
      }
   }

   if ((dwHashedIndx == dwHashedIndxInitial) &&
       gVecsGlobalStoreMap[dwHashedIndx].status == STORE_MAP_ENTRY_STATUS_OPEN &&
       gVecsGlobalStoreMap[dwHashedIndx].pStore->dwStoreId != dwStoreId
      )
   {
      dwError = ERROR_NO_MORE_USER_HANDLES;
      BAIL_ON_VMAFD_ERROR (dwError);
   }

   *pdwHashedIndex = dwHashedIndx;

cleanup:
   return dwError;

error:
   if (pdwHashedIndex)
   {
      *pdwHashedIndex = 0;
   }
   goto cleanup;
}

static
VOID
VmAfdCleanSecurityDescriptor (
                        PVMAFD_SECURITY_DESCRIPTOR pSecurityDescriptor
                        )
{
    PVMAFD_ACE_LIST pAceListCursor = NULL;
    PVMAFD_ACE_LIST pPrev = NULL;
    PVMAFD_ACE_LIST pAceToClean = NULL;

    if (pSecurityDescriptor &&
        pSecurityDescriptor->pAcl
       )
    {
        pAceListCursor = pSecurityDescriptor->pAcl->pAceList;

        while (pAceListCursor)
        {
            if (!pAceListCursor->Ace.accessMask)
            {
                if (!pPrev)
                {
                    pSecurityDescriptor->pAcl->pAceList = pAceListCursor->pNext;
                }
                else
                {
                    pPrev->pNext = pAceListCursor->pNext;
                }
                    pAceToClean = pAceListCursor;
                    pAceListCursor = pAceListCursor->pNext;
                    pAceToClean->pNext = NULL;
                    VmAfdFreeAceList (pAceToClean);
                    pSecurityDescriptor->pAcl->dwAceCount--;
            }
            else
            {
              pPrev = pAceListCursor;
              pAceListCursor = pAceListCursor->pNext;
            }
        }
    }
}


