/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ?~@~\License?~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ?~@~\AS IS?~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "includes.h"

DWORD
VmDirUTDVectorCacheInit(
    VOID
    )
{
    DWORD   dwError = 0;

    dwError = VmDirAllocateMemory(
            sizeof(VMDIR_UTDVECTOR_CACHE),
            (PVOID*)&gVmdirServerGlobals.pUtdVectorCache);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(
            &gVmdirServerGlobals.pUtdVectorCache->pUtdVectorMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
            "", &gVmdirServerGlobals.pUtdVectorCache->pszUtdVector);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateRWLock(
            &gVmdirServerGlobals.pUtdVectorCache->pUtdVectorLock);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "failed, error (%d)",
            dwError);

    goto cleanup;
}

DWORD
VmDirUTDVectorCacheUpdate(
    PCSTR   pszNewUTDVector
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    PSTR    pszCopy = NULL;     // need free
    PSTR    pszSave = NULL;
    PSTR    pszPair = NULL;
    PSTR    pszTmp = NULL;      // need free
    PSTR    pszInvoId = NULL;   // need free
    PSTR    pszUSN = NULL;
    USN     usn = 0;
    PVMDIR_UTDVECTOR_CACHE  pCache = gVmdirServerGlobals.pUtdVectorCache;

    if (IsNullOrEmptyString(pszNewUTDVector))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateStringA(pszNewUTDVector, &pszCopy);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_RWLOCK_WRITELOCK(bInLock, pCache->pUtdVectorLock, 0);

    // update hash map first
    LwRtlHashMapClear(pCache->pUtdVectorMap, VmDirSimpleHashMapPairFreeKeyOnly, NULL);

    for (pszPair = VmDirStringTokA(pszCopy, ",", &pszSave);
         !IsNullOrEmptyString(pszPair);
         pszPair = VmDirStringTokA(NULL, ",", &pszSave))
    {
        pszTmp = VmDirStringTokA(pszPair, ":", &pszUSN);

        dwError = VmDirAllocateStringA(pszTmp, &pszInvoId);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirStringToUSN(pszUSN, &usn);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapInsert(pCache->pUtdVectorMap, pszInvoId, (PVOID)(uintptr_t)usn, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
        pszInvoId = NULL;
    }

    // update string
    VMDIR_SAFE_FREE_MEMORY(pCache->pszUtdVector);
    dwError = VmDirAllocateStringA(pszNewUTDVector, &pCache->pszUtdVector);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, pCache->pUtdVectorLock);
    VMDIR_SAFE_FREE_MEMORY(pszCopy);
    VMDIR_SAFE_FREE_MEMORY(pszInvoId);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "failed, error (%d)",
            dwError);

    goto cleanup;
}

DWORD
VmDirUTDVectorCacheToString(
    PSTR*   ppszUTDVector
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    PVMDIR_UTDVECTOR_CACHE  pCache = gVmdirServerGlobals.pUtdVectorCache;

    if (!ppszUTDVector)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_RWLOCK_READLOCK(bInLock, pCache->pUtdVectorLock, 0);

    dwError = VmDirAllocateStringA(pCache->pszUtdVector, ppszUTDVector);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, pCache->pUtdVectorLock);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "failed, error (%d)",
            dwError);

    goto cleanup;
}

DWORD
VmDirUTDVectorCacheLookup(
    PCSTR   pszInvocationId,
    USN*    pUsn
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    uintptr_t   usn = 0;
    PVMDIR_UTDVECTOR_CACHE  pCache = gVmdirServerGlobals.pUtdVectorCache;

    if (IsNullOrEmptyString(pszInvocationId) || !pUsn)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_RWLOCK_READLOCK(bInLock, pCache->pUtdVectorLock, 0);

    dwError = LwRtlHashMapFindKey(pCache->pUtdVectorMap, (PVOID*)&usn, pszInvocationId);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pUsn = (USN)usn;

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, pCache->pUtdVectorLock);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "failed, error (%d)",
            dwError);

    goto cleanup;
}

VOID
VmDirUTDVectorCacheShutdown(
    VOID
    )
{
    BOOLEAN bInLock = FALSE;
    PVMDIR_UTDVECTOR_CACHE  pCache = gVmdirServerGlobals.pUtdVectorCache;

    VMDIR_RWLOCK_WRITELOCK(bInLock, pCache->pUtdVectorLock, 0);

    if (pCache->pUtdVectorMap)
    {
        LwRtlHashMapClear(pCache->pUtdVectorMap, VmDirSimpleHashMapPairFreeKeyOnly, NULL);
        LwRtlFreeHashMap(&pCache->pUtdVectorMap);
    }
    VMDIR_SAFE_FREE_MEMORY(pCache->pszUtdVector);

    VMDIR_RWLOCK_UNLOCK(bInLock, pCache->pUtdVectorLock);
    VMDIR_SAFE_FREE_RWLOCK(pCache->pUtdVectorLock);

    VMDIR_SAFE_FREE_MEMORY(gVmdirServerGlobals.pUtdVectorCache);
}
