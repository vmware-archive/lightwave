/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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

static
DWORD
_VmDirDeadlockDetectionVectorPairToStr(
    LW_HASHMAP_PAIR    pair,
    BOOLEAN            bStart,
    PSTR*              ppOutStr
    );

static
DWORD
_VmDirDeadlockDetectionVectorStrToPair(
    PSTR   pszKey,
    PSTR   pszValue,
    LW_HASHMAP_PAIR  *pPair
    );

static
DWORD
_VmDirDDVectorUpdateInLock(
    DWORD   dwValue
    );

static
DWORD
_VmDirDDVectorUpdateDefaultsInLock(
    DWORD dwValue
    );

DWORD
VmDirDDVectorInit(
    VOID
    )
{
    DWORD   dwError = 0;

    dwError = VmDirAllocateMemory(
            sizeof(VMDIR_REPL_DEADLOCKDETECTION_VECTOR),
            (PVOID*)&gVmdirServerGlobals.pReplDeadlockDetectionVector);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(
            &gVmdirServerGlobals.pReplDeadlockDetectionVector->pEmptyPageSentMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(
            &gVmdirServerGlobals.pReplDeadlockDetectionVector->pMutex);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

VOID
VmDirDDVectorShutdown(
    VOID
    )
{
    BOOLEAN bInLock = FALSE;

    if (gVmdirServerGlobals.pReplDeadlockDetectionVector)
    {
        VMDIR_LOCK_MUTEX(bInLock, gVmdirServerGlobals.pReplDeadlockDetectionVector->pMutex);

        if (gVmdirServerGlobals.pReplDeadlockDetectionVector->pEmptyPageSentMap)
        {
            LwRtlHashMapClear(
                    gVmdirServerGlobals.pReplDeadlockDetectionVector->pEmptyPageSentMap,
                    VmDirSimpleHashMapPairFree,
                    NULL);
            LwRtlFreeHashMap(&gVmdirServerGlobals.pReplDeadlockDetectionVector->pEmptyPageSentMap);
        }

        VMDIR_SAFE_FREE_MEMORY(gVmdirServerGlobals.pReplDeadlockDetectionVector->pszInvocationId);

        VMDIR_UNLOCK_MUTEX(bInLock, gVmdirServerGlobals.pReplDeadlockDetectionVector->pMutex);
    }

    VMDIR_SAFE_FREE_MUTEX(gVmdirServerGlobals.pReplDeadlockDetectionVector->pMutex);
    VMDIR_SAFE_FREE_MEMORY(gVmdirServerGlobals.pReplDeadlockDetectionVector);
}

DWORD
VmDirDDVectorToString(
    PCSTR   pszInvocationId,
    PSTR*   ppszDeadlockDetectionVectorStr
    )
{
    PSTR      pszDeadlockDetectionVectorStr = NULL;
    DWORD     dwError = 0;
    PDWORD    pdwValue = NULL;
    DWORD     dwCacheEmptyPageCnt = 0;
    BOOLEAN   bInLock = FALSE;
    BOOLEAN   bRevertDefaultValue = FALSE;

    if (ppszDeadlockDetectionVectorStr == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_LOCK_MUTEX(bInLock, gVmdirServerGlobals.pReplDeadlockDetectionVector->pMutex);

    /*
     * Best effort if invocation id is NULL ignore.
     * Ping-pong state is handled by providing supplier's empty page
     * count as 'gVmdirGlobals.dwEmptyPageCnt' so that partner considers only his state
     */
    if (gVmdirServerGlobals.pReplDeadlockDetectionVector->pszInvocationId &&
        pszInvocationId &&
        VmDirStringCompareA(
                pszInvocationId,
                gVmdirServerGlobals.pReplDeadlockDetectionVector->pszInvocationId,
                FALSE) == 0)
    {
        // will reach here only if key is present
        LwRtlHashMapFindKey(
                gVmdirServerGlobals.pReplDeadlockDetectionVector->pEmptyPageSentMap,
                (PVOID*)&pdwValue,
                gVmdirServerGlobals.bvServerObjName.lberbv_val);

        dwCacheEmptyPageCnt = *pdwValue;

        dwError = _VmDirDDVectorUpdateDefaultsInLock(gVmdirGlobals.dwEmptyPageCnt);
        BAIL_ON_VMDIR_ERROR(dwError);

        bRevertDefaultValue = TRUE;
    }

    dwError = VmDirVectorToStr(
            gVmdirServerGlobals.pReplDeadlockDetectionVector->pEmptyPageSentMap,
            _VmDirDeadlockDetectionVectorPairToStr,
            &pszDeadlockDetectionVectorStr);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszDeadlockDetectionVectorStr = pszDeadlockDetectionVectorStr;
    pszDeadlockDetectionVectorStr = NULL;

    if (bRevertDefaultValue)
    {
        dwError = _VmDirDDVectorUpdateDefaultsInLock(dwCacheEmptyPageCnt);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirServerGlobals.pReplDeadlockDetectionVector->pMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

//String: vector:<server-name:counter,server-name:counter...>

DWORD
VmDirDDVectorParseString(
    PSTR       pszDeadlockDetectionVectorStr,
    PBOOLEAN   pbCompleteReplCycle
    )
{
    PSTR      pszLocalVectorStr = NULL;
    PDWORD    pdwValue = NULL;
    DWORD     dwError = 0;
    DWORD     dwMinConsecutiveEmptyPage = INT_MAX;
    BOOLEAN   bInLock = FALSE;
    LW_HASHMAP_ITER  iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR  pair = {NULL, NULL};

    VMDIR_LOCK_MUTEX(bInLock, gVmdirServerGlobals.pReplDeadlockDetectionVector->pMutex);

    if (pszDeadlockDetectionVectorStr && pbCompleteReplCycle)
    {
        dwError = VmDirAllocateStringA(pszDeadlockDetectionVectorStr+VmDirStringLenA("vector:"), &pszLocalVectorStr);
        BAIL_ON_VMDIR_ERROR(dwError);

        // cache the counter value
        if (LwRtlHashMapFindKey(
                    gVmdirServerGlobals.pReplDeadlockDetectionVector->pEmptyPageSentMap,
                    (PVOID*)&pdwValue,
                    gVmdirServerGlobals.bvServerObjName.lberbv_val) == 0)
        {
            dwMinConsecutiveEmptyPage = *pdwValue;
            dwMinConsecutiveEmptyPage++;
            VMDIR_LOG_INFO(LDAP_DEBUG_REPL, "consecutive empty pages received: %d", dwMinConsecutiveEmptyPage);
        }

        // clear and reconstruct the vector
        LwRtlHashMapClear(
                gVmdirServerGlobals.pReplDeadlockDetectionVector->pEmptyPageSentMap,
                VmDirSimpleHashMapPairFree,
                NULL);

        dwError = VmDirStrtoVector(
                pszLocalVectorStr,
                _VmDirDeadlockDetectionVectorStrToPair,
                gVmdirServerGlobals.pReplDeadlockDetectionVector->pEmptyPageSentMap);
        BAIL_ON_VMDIR_ERROR(dwError);

        // update the cached counter value
        dwError = _VmDirDDVectorUpdateDefaultsInLock(dwMinConsecutiveEmptyPage);
        BAIL_ON_VMDIR_ERROR(dwError);

        // check for replication cycle completion
        while (LwRtlHashMapIterate(gVmdirServerGlobals.pReplDeadlockDetectionVector->pEmptyPageSentMap, &iter, &pair))
        {
            dwMinConsecutiveEmptyPage = VMDIR_MIN(dwMinConsecutiveEmptyPage, *(PDWORD)pair.pValue);
        }

        *pbCompleteReplCycle = (dwMinConsecutiveEmptyPage >= gVmdirGlobals.dwEmptyPageCnt);
        if (*pbCompleteReplCycle)
        {
            VMDIR_LOG_INFO(
                    VMDIR_LOG_MASK_ALL,
                    "%s: Break Empty Pages Received, vec: %s",
                    __FUNCTION__,
                    pszDeadlockDetectionVectorStr);
        }
    }
    else // if vector not present - non empty page - clear the vector contents
    {
        _VmDirDDVectorUpdateInLock(0);
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirServerGlobals.pReplDeadlockDetectionVector->pMutex);
    VMDIR_SAFE_FREE_MEMORY(pszLocalVectorStr);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

BOOLEAN
VmDirConsumerRoleActive(
    VOID
    )
{
    BOOLEAN   bInLock = FALSE;
    BOOLEAN   bActiveReplCycle = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirServerGlobals.pReplDeadlockDetectionVector->pMutex);

    // if consumer role is active, then DD vector will definitely have bvServerObjName key
    if (LwRtlHashMapFindKey(
            gVmdirServerGlobals.pReplDeadlockDetectionVector->pEmptyPageSentMap,
            NULL,
            gVmdirServerGlobals.bvServerObjName.lberbv_val) == 0)
    {
        bActiveReplCycle = TRUE;
    }

    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirServerGlobals.pReplDeadlockDetectionVector->pMutex);

    return bActiveReplCycle;
}

VOID
VmDirDDVectorClear(
    VOID
    )
{
    BOOLEAN   bInLock = FALSE;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirServerGlobals.pReplDeadlockDetectionVector->pMutex);

    LwRtlHashMapClear(
            gVmdirServerGlobals.pReplDeadlockDetectionVector->pEmptyPageSentMap,
            VmDirSimpleHashMapPairFree,
            NULL);

    VMDIR_SAFE_FREE_MEMORY(gVmdirServerGlobals.pReplDeadlockDetectionVector->pszInvocationId);

    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirServerGlobals.pReplDeadlockDetectionVector->pMutex);
}

DWORD
VmDirDDVectorUpdate(
    PCSTR   pszInvocationId,
    DWORD   dwValue
    )
{
    DWORD     dwError = 0;
    BOOLEAN   bInLock = FALSE;

    dwError = VmDirAllocateStringA(
            pszInvocationId,
            &gVmdirServerGlobals.pReplDeadlockDetectionVector->pszInvocationId);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOCK_MUTEX(bInLock, gVmdirServerGlobals.pReplDeadlockDetectionVector->pMutex);

    dwError = _VmDirDDVectorUpdateInLock(dwValue);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirServerGlobals.pReplDeadlockDetectionVector->pMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

static
DWORD
_VmDirDDVectorUpdateInLock(
    DWORD   dwValue
    )
{
    DWORD     dwError = 0;

    LwRtlHashMapClear(
            gVmdirServerGlobals.pReplDeadlockDetectionVector->pEmptyPageSentMap,
            VmDirSimpleHashMapPairFree,
            NULL);

    dwError = _VmDirDDVectorUpdateDefaultsInLock(dwValue);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

static
DWORD
_VmDirDDVectorUpdateDefaultsInLock(
    DWORD dwValue
    )
{
    DWORD              dwError = 0;
    PSTR               pszKey = NULL;
    PDWORD             pdwValue = NULL;
    LW_HASHMAP_PAIR    pair = {NULL, NULL};
    LW_HASHMAP_PAIR    prevPair = {NULL, NULL};

    dwError = VmDirAllocateStringA(
            gVmdirServerGlobals.bvServerObjName.lberbv_val,
            &pszKey);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
            sizeof(DWORD),
            (PVOID*)&pdwValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pdwValue = dwValue;

    pair.pKey = pszKey; pair.pValue = pdwValue;
    pszKey = NULL; pdwValue = NULL;

    dwError = LwRtlHashMapInsert(
            gVmdirServerGlobals.pReplDeadlockDetectionVector->pEmptyPageSentMap,
            pair.pKey,
            pair.pValue,
            &prevPair);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSimpleHashMapPairFree(&prevPair, NULL);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszKey);
    VMDIR_SAFE_FREE_MEMORY(pdwValue);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

static
DWORD
_VmDirDeadlockDetectionVectorPairToStr(
    LW_HASHMAP_PAIR    pair,
    BOOLEAN            bStart,
    PSTR*              ppOutStr
    )
{
    PSTR    pszTempStr = NULL;
    DWORD   dwError = 0;

    VMDIR_LOG_INFO(LDAP_DEBUG_REPL, "%s: key: %s value: %d", (PSTR)pair.pKey, *(PDWORD)pair.pValue);

    if (bStart)
    {
        dwError = VmDirAllocateStringPrintf(&pszTempStr, "vector:%s:%d", pair.pKey, *(PDWORD)pair.pValue);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VmDirAllocateStringPrintf(&pszTempStr, ",%s:%d", pair.pKey, *(PDWORD)pair.pValue);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

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
_VmDirDeadlockDetectionVectorStrToPair(
    PSTR   pszKey,
    PSTR   pszValue,
    LW_HASHMAP_PAIR   *pPair
    )
{
    PSTR    pszDupKey = NULL;
    DWORD   dwError = 0;
    PDWORD  pdwValue = NULL;

    dwError = VmDirAllocateStringA(pszKey, &pszDupKey);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
            sizeof(DWORD),
            (PVOID*)&pdwValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pdwValue = VmDirStringToIA(pszValue);

    pPair->pKey = (PVOID) pszDupKey;
    pPair->pValue = (PVOID) pdwValue;

    pszDupKey = NULL; pdwValue = NULL;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDupKey);
    VMDIR_SAFE_FREE_MEMORY(pdwValue);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

VOID
VmDirPopulateInvocationIdInReplAgr(
    VOID
    )
{
    PSTR      pszCfgBaseDN = NULL;
    DWORD     dwError = 0;
    size_t    iCnt = 0;
    BOOLEAN   bInLock = FALSE;
    VDIR_ENTRY_ARRAY   entryArray = {0};
    PVDIR_ATTRIBUTE    pAttrCN = NULL;
    PVDIR_ATTRIBUTE    pAttrInvocID = NULL;
    PVMDIR_REPLICATION_AGREEMENT   pReplAgr = NULL;

    dwError = VmDirAllocateStringPrintf(
            &pszCfgBaseDN,
            "cn=%s,%s",
            VMDIR_CONFIGURATION_CONTAINER_NAME,
            gVmdirServerGlobals.systemDomainDN.lberbv_val);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSimpleEqualFilterInternalSearch(
            pszCfgBaseDN,
            LDAP_SCOPE_SUBTREE,
            ATTR_OBJECT_CLASS,
            OC_DIR_SERVER,
            &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOCK_MUTEX(bInLock, gVmdirGlobals.replAgrsMutex);

    //complexity should be a problem since size of 'n' number of partners is always small here.
    for (iCnt = 0; iCnt < entryArray.iSize; iCnt++)
    {
        pAttrCN = VmDirFindAttrByName(&entryArray.pEntry[iCnt], ATTR_CN);
        pAttrInvocID = VmDirFindAttrByName(&entryArray.pEntry[iCnt], ATTR_INVOCATION_ID);

        if (pAttrCN && pAttrInvocID)
        {
            for (pReplAgr = gVmdirReplAgrs; pReplAgr; pReplAgr = pReplAgr->next)
            {
                if (VmDirStringCompareA(pReplAgr->pszHostname, pAttrCN->vals[0].lberbv_val, FALSE) == 0 &&
                    pReplAgr->pszInvocationID == NULL)
                {
                    VmDirAllocateStringA(pAttrInvocID->vals[0].lberbv_val, &pReplAgr->pszInvocationID);
                }
            }
        }
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirGlobals.replAgrsMutex);
    VMDIR_SAFE_FREE_MEMORY(pszCfgBaseDN);
    VmDirFreeEntryArrayContent(&entryArray);
    return;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}
