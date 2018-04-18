/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

/*
 * Curl handle cache to pool curl handles
 * Key: http://$host_name OR https://$host_name
 * Value: stack of curl handles
 */

#include "includes.h"

DWORD
VmDirRESTCurlHandleCacheInit(
    PVDIR_REST_CURL_HANDLE_CACHE* ppRestproxyCache
    )
{
    DWORD   dwError = 0;
    PVDIR_REST_CURL_HANDLE_CACHE pRestCurlHandleCache = NULL;

    if (!ppRestproxyCache)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_REST_CURL_HANDLE_CACHE), (PVOID*)&pRestCurlHandleCache);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&pRestCurlHandleCache->pCacheMutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(
            &pRestCurlHandleCache->pCurlHandleMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppRestproxyCache = pRestCurlHandleCache;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);
    VmDirRESTCurlHandleCacheFree(pRestCurlHandleCache);
    goto cleanup;
}

DWORD
VmDirRESTCurlHandleCacheGet(
    PVDIR_REST_CURL_HANDLE_CACHE pRestCurlHandleCache,
    PSTR pszKey,
    CURL** ppCurlHandle)
{
    DWORD   dwError = 0;
    PDEQUE  pHandleQueue = NULL;
    CURL*   pCurlHandle = NULL;
    BOOLEAN bInLock = FALSE;

    if (!pszKey || !pRestCurlHandleCache || !ppCurlHandle)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_LOCK_MUTEX(bInLock, pRestCurlHandleCache->pCacheMutex);

    dwError = LwRtlHashMapFindKey(
            pRestCurlHandleCache->pCurlHandleMap, (PVOID*)&pHandleQueue, pszKey);
    if (dwError == LW_STATUS_NOT_FOUND)
    {
        dwError = 0;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pHandleQueue)
    {
        dwError = dequePop(pHandleQueue, (PVOID*)&pCurlHandle);
        if (dwError == ERROR_NO_MORE_ITEMS)
        {
            dwError = 0;
        }
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!pCurlHandle)
    {
        pCurlHandle = curl_easy_init();
        if (!pCurlHandle)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_CURL_FAILED_INIT);
        }
    }

    *ppCurlHandle = pCurlHandle;

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, pRestCurlHandleCache->pCacheMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);
    goto cleanup;
}

DWORD
VmDirRESTCurlHandleCachePut(
    PVDIR_REST_CURL_HANDLE_CACHE pRestCurlHandleCache,
    PSTR pszKey,
    CURL* pCurlHandle)
{
    DWORD   dwError = 0;
    PSTR    pszMapKey = 0;
    PDEQUE  pHandleQueue = NULL;
    BOOLEAN bInLock = FALSE;
    CURL*   pCurlHandleLocal = NULL;

    if (!pszKey || !pRestCurlHandleCache || !pCurlHandle)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_LOCK_MUTEX(bInLock, pRestCurlHandleCache->pCacheMutex);

    dwError = LwRtlHashMapFindKey(
            pRestCurlHandleCache->pCurlHandleMap, (PVOID*)&pHandleQueue, pszKey);
    if (dwError == LW_STATUS_NOT_FOUND)
    {
        dwError = 0;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!pHandleQueue)
    {
        dwError = dequeCreate(&pHandleQueue);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringPrintf(&pszMapKey, "%s", pszKey);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapInsert(
                pRestCurlHandleCache->pCurlHandleMap, pszMapKey, pHandleQueue, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (dequeGetSize(pHandleQueue) >= gVmdirServerGlobals.dwRESTWorker)
    {
        while (dequeGetSize(pHandleQueue) >= gVmdirServerGlobals.dwRESTWorker)
        {
            dwError = dequePopLeft(pHandleQueue, (PVOID*)&pCurlHandleLocal);
            BAIL_ON_VMDIR_ERROR(dwError);

            curl_easy_cleanup(pCurlHandleLocal);

            pCurlHandleLocal = NULL;
        }
    }

    curl_easy_reset(pCurlHandle);
    dwError = dequePush(pHandleQueue, (PVOID)pCurlHandle);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, pRestCurlHandleCache->pCacheMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);
    VMDIR_SAFE_FREE_MEMORY(pszMapKey);
    goto cleanup;
}

VOID
VmDirRESTCurlHandleCacheFree(
    PVDIR_REST_CURL_HANDLE_CACHE pRestCurlHandleCache
    )
{
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};
    PDEQUE          pHandleQueue = NULL;
    CURL*           pCurlHandle = NULL;

    if (pRestCurlHandleCache)
    {
        LwRtlHashMapResetIter(&iter);
        while(LwRtlHashMapIterate(pRestCurlHandleCache->pCurlHandleMap, &iter, &pair))
        {
            pHandleQueue = (PDEQUE)pair.pValue;
            dequePopLeft(pHandleQueue, (PVOID*)&pCurlHandle);

            while(pCurlHandle)
            {
                curl_easy_cleanup(pCurlHandle);
                pCurlHandle = NULL;
                dequePopLeft(pHandleQueue, (PVOID*)&pCurlHandle);
            }
        }

        LwRtlHashMapClear(pRestCurlHandleCache->pCurlHandleMap, VmDirSimpleHashMapPairFree, NULL);
        LwRtlFreeHashMap(&pRestCurlHandleCache->pCurlHandleMap);
        VmDirFreeMutex(pRestCurlHandleCache->pCacheMutex);
        VMDIR_SAFE_FREE_MEMORY(pRestCurlHandleCache);
    }
}
