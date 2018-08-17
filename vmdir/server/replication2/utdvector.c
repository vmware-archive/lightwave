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

static
DWORD
_VmDirUTDVectorPairToStr(
    LW_HASHMAP_PAIR    pair,
    BOOLEAN            bFirst,
    PSTR*              ppOutStr
    );

static
DWORD
_VmDirUTDVectorStrToPair(
    PSTR   pszKey,
    PSTR   pszValue,
    LW_HASHMAP_PAIR*   pPair
    );

DWORD
VmDirUTDVectorGlobalCacheInit(
    VOID
    )
{
    DWORD   dwError = 0;

    dwError = VmDirUTDVectorCacheInit(&gVmdirServerGlobals.pUtdVectorCache);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirUTDVectorGlobalCacheReplace(
    PCSTR   pszNewUTDVector
    )
{
    DWORD   dwError = 0;

    dwError = VmDirUTDVectorCacheReplace(gVmdirServerGlobals.pUtdVectorCache, pszNewUTDVector);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirUTDVectorGlobalCacheToString(
    PSTR*   ppszUTDVector
    )
{
    DWORD   dwError = 0;

    dwError = VmDirUTDVectorCacheToString(gVmdirServerGlobals.pUtdVectorCache, ppszUTDVector);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirUTDVectorGlobalCacheLookup(
    PCSTR   pszInvocationId,
    USN*    pUsn
    )
{
    DWORD   dwError = 0;

    dwError = VmDirUTDVectorCacheLookup(gVmdirServerGlobals.pUtdVectorCache, pszInvocationId, pUsn);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

VOID
VmDirFreeUTDVectorGlobalCache(
    VOID
    )
{
    VmDirFreeUTDVectorCache(gVmdirServerGlobals.pUtdVectorCache);

    return;

}

DWORD
VmDirUTDVectorCacheInit(
    PVMDIR_UTDVECTOR_CACHE*    ppUTDVector
    )
{
    DWORD                   dwError = 0;
    PVMDIR_UTDVECTOR_CACHE  pCache = NULL;

    if (!ppUTDVector)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VMDIR_UTDVECTOR_CACHE),
            (PVOID*)&pCache);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(
            &pCache->pUtdVectorMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
            "", &pCache->pszUtdVector);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateRWLock(
            &pCache->pUtdVectorLock);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppUTDVector = pCache;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirUTDVectorCacheReplace(
    PVMDIR_UTDVECTOR_CACHE pUTDVector,
    PCSTR                  pszNewUTDVector
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;

    if (!pUTDVector)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (IsNullOrEmptyString(pszNewUTDVector))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_RWLOCK_WRITELOCK(bInLock, pUTDVector->pUtdVectorLock, 0);

    // update hash map first
    LwRtlHashMapClear(pUTDVector->pUtdVectorMap, VmDirSimpleHashMapPairFreeKeyOnly, NULL);

    dwError = VmDirStrtoVector(pszNewUTDVector, _VmDirUTDVectorStrToPair, pUTDVector->pUtdVectorMap);
    BAIL_ON_VMDIR_ERROR(dwError);

    // update string
    VMDIR_SAFE_FREE_MEMORY(pUTDVector->pszUtdVector);
    dwError = VmDirAllocateStringA(pszNewUTDVector, &pUTDVector->pszUtdVector);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, pUTDVector->pUtdVectorLock);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirUTDVectorCacheToString(
    PVMDIR_UTDVECTOR_CACHE pUTDVector,
    PSTR*                  ppszUTDVector
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    PSTR    pszUTDVector = NULL;

    if (!pUTDVector || !ppszUTDVector)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_RWLOCK_READLOCK(bInLock, pUTDVector->pUtdVectorLock, 0);

    dwError = VmDirAllocateStringA(pUTDVector->pszUtdVector, &pszUTDVector);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszUTDVector = pszUTDVector;

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, pUTDVector->pUtdVectorLock);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirUTDVectorCacheLookup(
    PVMDIR_UTDVECTOR_CACHE  pUTDVector,
    PCSTR                   pszInvocationId,
    USN*                    pUsn
    )
{
    DWORD       dwError = 0;
    BOOLEAN     bInLock = FALSE;
    uintptr_t   usn = 0;
    PVMDIR_UTDVECTOR_CACHE  pCache = pUTDVector;

    if (IsNullOrEmptyString(pszInvocationId) || !pUsn || !pUTDVector)
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
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirUTDVectorCacheAdd(
    PVMDIR_UTDVECTOR_CACHE  pUTDVector,
    PCSTR                   pszInvocationId,
    PCSTR                   pszUsn
    )
{
    DWORD              dwError = 0;
    BOOLEAN            bInLock = FALSE;
    PSTR               pszDupKey = NULL;
    USN                Usn = 0;
    LW_HASHMAP_PAIR    pair = {0};

    if (IsNullOrEmptyString(pszInvocationId) || !pUTDVector)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateStringA(pszInvocationId, &pszDupKey);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToINT64(pszUsn, NULL, &Usn);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_RWLOCK_READLOCK(bInLock, pUTDVector->pUtdVectorLock, 0);

    dwError = LwRtlHashMapInsert(pUTDVector->pUtdVectorMap, pszDupKey, (PVOID)Usn, &pair);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSimpleHashMapPairFreeKeyOnly(&pair, NULL);
    pszDupKey = NULL;

    VMDIR_SAFE_FREE_STRINGA(pUTDVector->pszUtdVector);

    dwError = VmDirVectorToStr(pUTDVector->pUtdVectorMap, _VmDirUTDVectorPairToStr, &pUTDVector->pszUtdVector);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, pUTDVector->pUtdVectorLock);
    VMDIR_SAFE_FREE_MEMORY(pszDupKey);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

VOID
VmDirFreeUTDVectorCache(
    PVMDIR_UTDVECTOR_CACHE  pUTDVector
    )
{
    BOOLEAN bInLock = FALSE;

    if (!pUTDVector)
    {
        return;
    }

    VMDIR_RWLOCK_WRITELOCK(bInLock, pUTDVector->pUtdVectorLock, 0);

    if (pUTDVector->pUtdVectorMap)
    {
        LwRtlHashMapClear(pUTDVector->pUtdVectorMap, VmDirSimpleHashMapPairFreeKeyOnly, NULL);
        LwRtlFreeHashMap(&pUTDVector->pUtdVectorMap);
    }
    VMDIR_SAFE_FREE_MEMORY(pUTDVector->pszUtdVector);

    VMDIR_RWLOCK_UNLOCK(bInLock, pUTDVector->pUtdVectorLock);
    VMDIR_SAFE_FREE_RWLOCK(pUTDVector->pUtdVectorLock);

    VMDIR_SAFE_FREE_MEMORY(pUTDVector);
}

static
DWORD
_VmDirUTDVectorPairToStr(
    LW_HASHMAP_PAIR    pair,
    BOOLEAN            bFirst,
    PSTR*              ppOutStr
    )
{
    PSTR    pszTempStr = NULL;
    DWORD   dwError = 0;

    dwError = VmDirAllocateStringPrintf(&pszTempStr, "%s:%"PRId64",", pair.pKey, (USN)pair.pValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppOutStr = pszTempStr;
    pszTempStr = NULL;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszTempStr);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

static
DWORD
_VmDirUTDVectorStrToPair(
    PSTR                pszKey,
    PSTR                pszValue,
    LW_HASHMAP_PAIR*    pPair
    )
{
    PSTR    pszDupKey = NULL;
    DWORD   dwError = 0;
    USN     Usn = 0;

    dwError = VmDirAllocateStringA(pszKey, &pszDupKey);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToINT64(pszValue, NULL, &Usn);
    BAIL_ON_VMDIR_ERROR(dwError);

    pPair->pKey = (PVOID) pszDupKey;
    pPair->pValue = (PVOID) Usn;

    pszDupKey = NULL;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDupKey);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}
