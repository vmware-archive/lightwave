/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : ipcmarshalhelper.c
 *
 * Abstract :
 *
 */
#include "includes.h"

DWORD
VmAfdEncodeVecsStore (
    PVECS_SERV_STORE pStore,
    PBYTE *ppStoreBlob
    )
{
    DWORD dwError = 0;
    PBYTE pStoreBlob = NULL;
    PBYTE pStoreBlobCursor = NULL;
    DWORD dwSizeOfBlob = 0;

    if (!ppStoreBlob)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (pStore)
    {

        dwSizeOfBlob = sizeof(VECS_SERV_STORE) + sizeof (UINT32);
    }
    else
    {
        dwSizeOfBlob = sizeof (UINT32);
    }

    dwError = VmAfdAllocateMemory (
                        dwSizeOfBlob,
                        (PVOID *) &pStoreBlob
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    pStoreBlobCursor = pStoreBlob;

    *((PUINT32)pStoreBlobCursor) = (UINT32)dwSizeOfBlob;
    pStoreBlobCursor += sizeof (UINT32);

    if (pStore)
    {

        pStoreBlobCursor = memcpy (
                        (PVOID) pStoreBlobCursor,
                        (PVOID) pStore,
                        sizeof (VECS_SERV_STORE)
                        );
    }

    *ppStoreBlob = pStoreBlob;

cleanup:
    return dwError;
error:
    if (ppStoreBlob)
    {
        *ppStoreBlob = NULL;
    }
    VMAFD_SAFE_FREE_MEMORY (pStoreBlob);

    goto cleanup;
}

DWORD
VmAfdDecodeVecsStore (
    PBYTE pStoreBlob,
    DWORD dwSizeOfBlob,
    PVECS_SERV_STORE *ppStore
    )
{
    DWORD dwError = 0;
    PVECS_SERV_STORE pStore = NULL;
    DWORD dwSizeExpected = 0;
    PBYTE pStoreBlobCursor = pStoreBlob;

    if (!pStoreBlob ||
        !ppStore
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwSizeExpected = sizeof (VECS_SERV_STORE) + sizeof (UINT32);

    if (dwSizeExpected != dwSizeOfBlob)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateMemory (
                                sizeof (VECS_SERV_STORE),
                                (PVOID *) &pStore
                                );
    BAIL_ON_VMAFD_ERROR (dwError);

    pStoreBlobCursor += sizeof (UINT32);

    pStore->dwStoreId = ((PVECS_SERV_STORE) pStoreBlobCursor)->dwStoreId;
    pStore->refCount = ((PVECS_SERV_STORE) pStoreBlobCursor)->refCount;

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
VmAfdEncodeVecsStoreHandle (
                             PVECS_SRV_STORE_HANDLE pStore,
                             PBYTE *ppStoreBlob
                           )
{
    DWORD dwError = 0;
    PBYTE pStoreBlob = NULL;
    PBYTE pStoreBlobCursor = NULL;
    DWORD dwSizeOfBlob = sizeof (UINT32);

    if (!ppStoreBlob)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (pStore)
    {

        dwSizeOfBlob += sizeof(VECS_SRV_STORE_HANDLE);
    }

    dwError = VmAfdAllocateMemory (
                        dwSizeOfBlob,
                        (PVOID *) &pStoreBlob
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    pStoreBlobCursor = pStoreBlob;

    *((PUINT32)pStoreBlobCursor) = (UINT32)dwSizeOfBlob;
    pStoreBlobCursor += sizeof (UINT32);


    if (pStore)
    {

        pStoreBlobCursor = memcpy (
                        (PVOID) pStoreBlobCursor,
                        (PVOID) pStore,
                        sizeof (VECS_SRV_STORE_HANDLE)
                        );
    }

    *ppStoreBlob = pStoreBlob;

cleanup:
    return dwError;
error:
    if (ppStoreBlob)
    {
        *ppStoreBlob = NULL;
    }
    VMAFD_SAFE_FREE_MEMORY (pStoreBlob);

    goto cleanup;

}


DWORD
VmAfdDecodeVecsStoreHandle (
                            PBYTE pStoreBlob,
                            DWORD dwSizeOfBlob,
                            PVECS_SRV_STORE_HANDLE *ppStore
                           )
{
    DWORD dwError = 0;
    PVECS_SRV_STORE_HANDLE pStore = NULL;
    DWORD dwSizeExpected = 0;
    PBYTE pStoreBlobCursor = pStoreBlob;

    if (!pStoreBlob ||
        !ppStore
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwSizeExpected = sizeof (VECS_SRV_STORE_HANDLE) + sizeof (UINT32);

    if (dwSizeExpected != dwSizeOfBlob)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateMemory (
                                sizeof (VECS_SRV_STORE_HANDLE),
                                (PVOID *) &pStore
                                );
    BAIL_ON_VMAFD_ERROR (dwError);

    pStoreBlobCursor += sizeof (UINT32);

    pStore->dwStoreHandle = ((PVECS_SRV_STORE_HANDLE)pStoreBlobCursor)->dwStoreHandle;

    pStore->dwClientInstance = ((PVECS_SRV_STORE_HANDLE)pStoreBlobCursor)->dwClientInstance;

    pStore->dwStoreSessionID = ((PVECS_SRV_STORE_HANDLE)pStoreBlobCursor)->dwStoreSessionID;

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
VmAfdEncodeEnumContext(
                    PVECS_SRV_ENUM_CONTEXT pEnumContext,
                    PBYTE *ppEnumContextBlob
                    )
{
    DWORD dwError = 0;
    PBYTE pEnumContextBlob = 0;
    PBYTE pEnumContextBlobCursor = 0;
    DWORD dwSizeOfBlob = sizeof (UINT32);

    if (!ppEnumContextBlob)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (pEnumContext)
    {
        dwSizeOfBlob += sizeof (VECS_SRV_ENUM_CONTEXT);
        if (pEnumContext->pStore)
        {
            dwSizeOfBlob += sizeof (VECS_SERV_STORE);
        }
    }

    dwError = VmAfdAllocateMemory (
                                   dwSizeOfBlob,
                                   (PVOID *)&pEnumContextBlob
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    pEnumContextBlobCursor = pEnumContextBlob;


    *((PUINT32)pEnumContextBlobCursor) = (UINT32)dwSizeOfBlob;
    pEnumContextBlobCursor += sizeof (UINT32);

    if (pEnumContext)
    {
        pEnumContextBlobCursor = memcpy(
                                (PVOID) pEnumContextBlobCursor,
                                (PVOID) pEnumContext,
                                sizeof (VECS_SRV_ENUM_CONTEXT)
                                );


        ((PVECS_SRV_ENUM_CONTEXT) pEnumContextBlobCursor)->pStore =
                                                        (PVECS_SERV_STORE)
                                                        (
                                                        pEnumContextBlobCursor +
                                                        sizeof (VECS_SRV_ENUM_CONTEXT)
                                                        );
        pEnumContextBlobCursor += sizeof (VECS_SRV_ENUM_CONTEXT);

        if (pEnumContext->pStore)
        {

                pEnumContextBlobCursor = memcpy (
                                (PVOID) pEnumContextBlobCursor,
                                (PVOID) pEnumContext->pStore,
                                sizeof (VECS_SERV_STORE)
                                );
        }
    }

    *ppEnumContextBlob = pEnumContextBlob;

cleanup:
    return dwError;

error:

    if (ppEnumContextBlob)
    {
        *ppEnumContextBlob = NULL;
    }
    VMAFD_SAFE_FREE_MEMORY (pEnumContextBlob);

    goto cleanup;
}

DWORD
VmAfdDecodeEnumContext(
                      PBYTE pEnumContextBlob,
                      DWORD dwBlobSize,
                      PVECS_SRV_ENUM_CONTEXT *ppEnumContext
                      )
{
      DWORD dwError = 0;
      PVECS_SRV_ENUM_CONTEXT pEnumContext = NULL;
      PVECS_SERV_STORE pStore = NULL;
      PBYTE pEnumContextBlobCursor = pEnumContextBlob;
      DWORD dwExpectedBlobSize = 0;

      if (!pEnumContextBlob ||
          !dwBlobSize ||
          !ppEnumContext
         )
      {
          dwError = ERROR_INVALID_PARAMETER;
          BAIL_ON_VMAFD_ERROR (dwError);
      }

      dwExpectedBlobSize = sizeof (VECS_SRV_ENUM_CONTEXT) +
                           sizeof (VECS_SERV_STORE) +
                           sizeof (UINT32);

      if (dwBlobSize < dwExpectedBlobSize)
      {
          dwError = ERROR_INVALID_PARAMETER;
          BAIL_ON_VMAFD_ERROR (dwError);
      }

      dwError = VmAfdAllocateMemory (
                                     sizeof (VECS_SRV_ENUM_CONTEXT),
                                     (PVOID *) &pEnumContext
                                    );
      BAIL_ON_VMAFD_ERROR (dwError);

      dwError = VmAfdAllocateMemory (
                                     sizeof (VECS_SERV_STORE),
                                     (PVOID *)&pStore
                                    );
      BAIL_ON_VMAFD_ERROR (dwError);

      pEnumContextBlobCursor += sizeof (UINT32);

      pEnumContext->refCount =
                 ((PVECS_SRV_ENUM_CONTEXT)pEnumContextBlobCursor)->refCount;
      pEnumContext->dwIndex =
                 ((PVECS_SRV_ENUM_CONTEXT) pEnumContextBlobCursor)->dwIndex;
      pEnumContext->infoLevel =
                 ((PVECS_SRV_ENUM_CONTEXT) pEnumContextBlobCursor)->infoLevel;
      pEnumContext->dwLimit =
                 ((PVECS_SRV_ENUM_CONTEXT) pEnumContextBlobCursor)->dwLimit;

      pEnumContextBlobCursor += sizeof (VECS_SRV_ENUM_CONTEXT);

      pStore->refCount = ((PVECS_SERV_STORE) pEnumContextBlobCursor)->refCount;
      pStore->dwStoreId = ((PVECS_SERV_STORE) pEnumContextBlobCursor)->dwStoreId;

      pEnumContext->pStore = pStore;

      *ppEnumContext = pEnumContext;


cleanup:

      return dwError;

error:
      if (ppEnumContext)
      {
          *ppEnumContext = NULL;
      }

      VMAFD_SAFE_FREE_MEMORY (pEnumContext);

      VMAFD_SAFE_FREE_MEMORY (pStore);

      goto cleanup;
}


DWORD
VmAfdEncodeEnumContextHandle(
                    PVECS_SRV_ENUM_CONTEXT_HANDLE pEnumContext,
                    PBYTE *ppEnumContextBlob
                    )
{
    DWORD dwError = 0;
    PBYTE pEnumContextBlob = 0;
    PBYTE pEnumContextBlobCursor = 0;
    DWORD dwSizeOfBlob = sizeof (UINT32);

    if (!ppEnumContextBlob)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (pEnumContext)
    {
        dwSizeOfBlob += sizeof (VECS_SRV_ENUM_CONTEXT_HANDLE);
        if (pEnumContext->pStore)
        {
            dwSizeOfBlob += sizeof (VECS_SRV_STORE_HANDLE);
        }
    }

    dwError = VmAfdAllocateMemory (
                                   dwSizeOfBlob,
                                   (PVOID *)&pEnumContextBlob
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    pEnumContextBlobCursor = pEnumContextBlob;


    *((PUINT32)pEnumContextBlobCursor) = (UINT32)dwSizeOfBlob;
    pEnumContextBlobCursor += sizeof (UINT32);

    if (pEnumContext)
    {
        pEnumContextBlobCursor = memcpy(
                                (PVOID) pEnumContextBlobCursor,
                                (PVOID) pEnumContext,
                                sizeof (VECS_SRV_ENUM_CONTEXT_HANDLE)
                                );

        ((PVECS_SRV_ENUM_CONTEXT_HANDLE) pEnumContextBlobCursor)->pStore =
                                            (PVECS_SRV_STORE_HANDLE)
                                            (
                                             pEnumContextBlobCursor +
                                             sizeof (VECS_SRV_ENUM_CONTEXT_HANDLE)
                                            );
        pEnumContextBlobCursor += sizeof (VECS_SRV_ENUM_CONTEXT_HANDLE);

        if (pEnumContext->pStore)
        {

                pEnumContextBlobCursor = memcpy (
                                (PVOID) pEnumContextBlobCursor,
                                (PVOID) pEnumContext->pStore,
                                sizeof (VECS_SRV_STORE_HANDLE)
                                );
        }
    }

    *ppEnumContextBlob = pEnumContextBlob;

cleanup:
    return dwError;

error:

    if (ppEnumContextBlob)
    {
        *ppEnumContextBlob = NULL;
    }
    VMAFD_SAFE_FREE_MEMORY (pEnumContextBlob);

    goto cleanup;
}

DWORD
VmAfdDecodeEnumContextHandle(
                      PBYTE pEnumContextBlob,
                      DWORD dwBlobSize,
                      PVECS_SRV_ENUM_CONTEXT_HANDLE *ppEnumContext
                      )
{
      DWORD dwError = 0;
      PVECS_SRV_ENUM_CONTEXT_HANDLE pEnumContext = NULL;
      PVECS_SRV_STORE_HANDLE pStore = NULL;
      PBYTE pEnumContextBlobCursor = pEnumContextBlob;
      DWORD dwExpectedBlobSize = 0;

      if (!pEnumContextBlob ||
          !dwBlobSize ||
          !ppEnumContext
         )
      {
          dwError = ERROR_INVALID_PARAMETER;
          BAIL_ON_VMAFD_ERROR (dwError);
      }

      dwExpectedBlobSize = sizeof (VECS_SRV_ENUM_CONTEXT_HANDLE) +
                           sizeof (VECS_SRV_STORE_HANDLE) +
                           sizeof (UINT32);

      if (dwBlobSize < dwExpectedBlobSize)
      {
          dwError = ERROR_INVALID_PARAMETER;
          BAIL_ON_VMAFD_ERROR (dwError);
      }

      dwError = VmAfdAllocateMemory (
                                     sizeof (VECS_SRV_ENUM_CONTEXT_HANDLE),
                                     (PVOID *) &pEnumContext
                                    );
      BAIL_ON_VMAFD_ERROR (dwError);

      dwError = VmAfdAllocateMemory (
                                     sizeof (VECS_SRV_STORE_HANDLE),
                                     (PVOID *)&pStore
                                    );
      BAIL_ON_VMAFD_ERROR (dwError);

      pEnumContextBlobCursor += sizeof (UINT32);

      pEnumContext->dwIndex =
                 ((PVECS_SRV_ENUM_CONTEXT_HANDLE) pEnumContextBlobCursor)->dwIndex;
      pEnumContext->infoLevel =
                 ((PVECS_SRV_ENUM_CONTEXT_HANDLE) pEnumContextBlobCursor)->infoLevel;
      pEnumContext->dwLimit =
                 ((PVECS_SRV_ENUM_CONTEXT_HANDLE) pEnumContextBlobCursor)->dwLimit;

      pEnumContextBlobCursor += sizeof (VECS_SRV_ENUM_CONTEXT_HANDLE);

      pStore->dwStoreHandle =
        ((PVECS_SRV_STORE_HANDLE) pEnumContextBlobCursor)->dwStoreHandle;
      pStore->dwClientInstance =
          ((PVECS_SRV_STORE_HANDLE) pEnumContextBlobCursor)->dwClientInstance;
      pStore->dwStoreSessionID =
          ((PVECS_SRV_STORE_HANDLE) pEnumContextBlobCursor)->dwStoreSessionID;

      pEnumContext->pStore = pStore;

      *ppEnumContext = pEnumContext;


cleanup:

      return dwError;

error:
      if (ppEnumContext)
      {
          *ppEnumContext = NULL;
      }

      VMAFD_SAFE_FREE_MEMORY (pEnumContext);

      VMAFD_SAFE_FREE_MEMORY (pStore);

      goto cleanup;
}

