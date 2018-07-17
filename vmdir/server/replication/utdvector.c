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
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirUTDVectorCacheUpdate(
    PCSTR   pszNewUTDVector
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    PVMDIR_UTDVECTOR_CACHE  pCache = gVmdirServerGlobals.pUtdVectorCache;

    if (IsNullOrEmptyString(pszNewUTDVector))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_RWLOCK_WRITELOCK(bInLock, pCache->pUtdVectorLock, 0);

    // update hash map first
    LwRtlHashMapClear(pCache->pUtdVectorMap, VmDirSimpleHashMapPairFreeKeyOnly, NULL);

    dwError = VmDirStrtoVector(pszNewUTDVector, _VmDirUTDVectorStrToPair, pCache->pUtdVectorMap);
    BAIL_ON_VMDIR_ERROR(dwError);

    // update string
    VMDIR_SAFE_FREE_MEMORY(pCache->pszUtdVector);
    dwError = VmDirAllocateStringA(pszNewUTDVector, &pCache->pszUtdVector);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, pCache->pUtdVectorLock);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
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
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
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
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirUTDVectorLookup(
    PLW_HASHMAP pUtdVectorMap,
    PCSTR   pszInvocationId,
    USN*    pUsn
    )
{
    DWORD   dwError = 0;
    uintptr_t   usn = 0;

    if (!pUtdVectorMap || IsNullOrEmptyString(pszInvocationId) || !pUsn)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = LwRtlHashMapFindKey(pUtdVectorMap, (PVOID*)&usn, pszInvocationId);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pUsn = (USN)usn;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
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

DWORD
VmDirStringToUTDVector(
    PCSTR          pszUTDVector,
    PLW_HASHMAP*   ppMap
    )
{
    DWORD         dwError = 0;
    PLW_HASHMAP   pMap = NULL;

    if (!pszUTDVector || !ppMap)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = LwRtlCreateHashMap(
            &pMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (IsNullOrEmptyString(pszUTDVector) == FALSE)
    {
        dwError = VmDirStrtoVector(pszUTDVector, _VmDirUTDVectorStrToPair, pMap);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_LOG_INFO(LDAP_DEBUG_REPL, "%s: Starting UTDVector: %s", __FUNCTION__, pszUTDVector);

    *ppMap = pMap;

cleanup:
    return dwError;

error:
    LwRtlHashMapClear(pMap, VmDirSimpleHashMapPairFreeKeyOnly, NULL);
    LwRtlFreeHashMap(&pMap);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirUTDVectorToString(
    PLW_HASHMAP   pMap,
    PSTR*         ppszUTDVector
    )
{
    DWORD   dwError = 0;
    PSTR    pszUTDvector = NULL;

    if (!pMap || !ppszUTDVector)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirVectorToStr(pMap, _VmDirUTDVectorPairToStr, &pszUTDvector);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszUTDVector = pszUTDvector;
    pszUTDvector = NULL;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszUTDvector);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}


DWORD
VmDirSyncDoneCtrlFromLocalCache(
    USN              lastSupplierUsnProcessed,
    PLW_HASHMAP      pUtdVectorMap,
    struct berval*   pPageSyncDoneCtrl
    )
{
    PSTR   pszUtdVectorStr = NULL;
    PSTR   pszHighWatermark = NULL;
    PSTR   pszSyncDoneCtrlVal = NULL;
    DWORD  dwError = 0;

    if (!pUtdVectorMap || !pPageSyncDoneCtrl)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateStringPrintf(&pszHighWatermark, "%" PRId64 ",", lastSupplierUsnProcessed);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirUTDVectorToString(pUtdVectorMap, &pszUtdVectorStr);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pszSyncDoneCtrlVal, "%s%s", pszHighWatermark, pszUtdVectorStr);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: update cookie: %s", __FUNCTION__, pszSyncDoneCtrlVal);

    pPageSyncDoneCtrl->bv_val = pszSyncDoneCtrlVal;
    pPageSyncDoneCtrl->bv_len = VmDirStringLenA(pszSyncDoneCtrlVal);
    pszSyncDoneCtrlVal = NULL;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszHighWatermark);
    VMDIR_SAFE_FREE_MEMORY(pszUtdVectorStr);
    VMDIR_SAFE_FREE_MEMORY(pszSyncDoneCtrlVal);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirUpdateUtdVectorLocalCache(
    PLW_HASHMAP      pUtdVectorMap,
    struct berval*   pPageSyncDoneCtrl
    )
{
    DWORD   dwError = 0;
    DWORD   dwCount = 0;
    USN     currUsn = 0;
    USN     cachedUsn = 0;
    PSTR    pszDupKey = NULL;
    PSTR    pszKey = NULL;
    PSTR    pszVal = NULL;
    PVMDIR_STRING_LIST   pStrList = NULL;

    if (!pUtdVectorMap || !pPageSyncDoneCtrl)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirStringToTokenList(pPageSyncDoneCtrl->bv_val, ",", &pStrList);
    BAIL_ON_VMDIR_ERROR(dwError);

    //skip high watermark
    for (dwCount = 1; dwCount < pStrList->dwCount; dwCount++)
    {
        if (VmDirStringStrA(pStrList->pStringList[dwCount], VMDIR_REPL_DD_VEC_INDICATOR) != NULL ||
            VmDirStringStrA(pStrList->pStringList[dwCount], VMDIR_REPL_CONT_INDICATOR_STR) != NULL)
        {
            break;
        }

        if (pStrList->pStringList[dwCount][0] != '\0')
        {
            pszKey = VmDirStringTokA((PSTR)pStrList->pStringList[dwCount], ":", &pszVal);

            dwError = VmDirStringToUSN(pszVal, &currUsn);
            BAIL_ON_VMDIR_ERROR(dwError);

            VMDIR_SAFE_FREE_MEMORY(pszDupKey);

            dwError = VmDirAllocateStringA(pszKey, &pszDupKey);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = LwRtlHashMapFindKey(pUtdVectorMap, (PVOID*)&cachedUsn, pszKey);
            if ((dwError == 0 && (currUsn > cachedUsn)) ||
                 dwError == LW_STATUS_NOT_FOUND)
            {
                dwError = LwRtlHashMapInsert(
                        pUtdVectorMap,
                        pszDupKey,
                        (PVOID)currUsn,
                        NULL);
                BAIL_ON_VMDIR_ERROR(dwError);

                pszDupKey = NULL;
                dwError = 0;
            }
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDupKey);
    VmDirStringListFree(pStrList);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
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
    PSTR   pszKey,
    PSTR   pszValue,
    LW_HASHMAP_PAIR*  pPair
    )
{
    PSTR    pszDupKey = NULL;
    DWORD   dwError = 0;
    USN     Usn = 0;

    dwError = VmDirAllocateStringA(pszKey, &pszDupKey);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToUSN(pszValue, &Usn);
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
