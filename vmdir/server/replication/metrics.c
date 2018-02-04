/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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
VOID
_FreeCacheMapPair(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    );

DWORD
VmDirReplMetricsCacheInit(
    VOID
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PSTR    pszSearchBaseDN = NULL;
    VDIR_ENTRY_ARRAY    entryArray = {0};

    dwError = LwRtlCreateHashMap(
            &gVdirReplMetricsCache.pHashMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateRWLock(&gVdirReplMetricsCache.pLock);
    BAIL_ON_VMDIR_ERROR(dwError);

    // load server entries only if already promoted
    if (gVmdirServerGlobals.bPromoted)
    {
        dwError = VmDirAllocateStringPrintf(
                &pszSearchBaseDN,
                "cn=Sites,cn=Configuration,%s",
                gVmdirServerGlobals.systemDomainDN.lberbv_val);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirSimpleEqualFilterInternalSearch(
                pszSearchBaseDN,
                LDAP_SCOPE_SUBTREE,
                ATTR_OBJECT_CLASS,
                OC_DIR_SERVER,
                &entryArray);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (i = 0; i < entryArray.iSize; i++)
        {
            dwError = VmDirReplMetricsCacheAdd(&entryArray.pEntry[i]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    VmDirFreeEntryArrayContent(&entryArray);
    VMDIR_SAFE_FREE_MEMORY(pszSearchBaseDN);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed (%d)",
            __FUNCTION__,
            dwError);

    VmDirReplMetricsCacheShutdown();
    goto cleanup;
}

DWORD
VmDirReplMetricsCacheAdd(
    PVDIR_ENTRY pServerEntry
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    PSTR    pszHostname = NULL;
    PSTR    pszSite = NULL;
    PVDIR_ATTRIBUTE pAttr = NULL;
    PVMDIR_REPLICATION_METRICS  pReplMetrics = NULL;

    if (!pServerEntry)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    // get server hostname
    pAttr = VmDirFindAttrByName(pServerEntry, ATTR_CN);
    dwError = pAttr ? 0 : VMDIR_ERROR_INVALID_ENTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pAttr->vals[0].lberbv_val, &pszHostname);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (VmDirStringCompareA(
            gVmdirServerGlobals.bvServerObjName.lberbv_val,
            pszHostname,
            FALSE) == 0)
    {
        // don't need metrics for localhost
        goto cleanup;
    }

    // get server site
    dwError = VmDirServerDNToSiteName(pServerEntry->dn.lberbv_val, &pszSite);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_RWLOCK_WRITELOCK(bInLock, gVdirReplMetricsCache.pLock, 0);

    if (LwRtlHashMapFindKey(
            gVdirReplMetricsCache.pHashMap,
            (PVOID*)&pReplMetrics,
            pszHostname) == 0)
    {
        if (pReplMetrics->bActive)
        {
            // already exists and active
            VMDIR_LOG_WARNING(
                    VMDIR_LOG_MASK_ALL,
                    "%s attempt to add existing entry %s (%s)",
                    __FUNCTION__,
                    pszHostname,
                    pszSite);
        }
    }
    else
    {
        dwError = VmDirReplMetricsInit(pszHostname, pszSite, &pReplMetrics);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapInsert(
                gVdirReplMetricsCache.pHashMap,
                pReplMetrics->pszSrcHostname,
                pReplMetrics,
                NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pReplMetrics->bActive = TRUE;

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, gVdirReplMetricsCache.pLock);
    VMDIR_SAFE_FREE_MEMORY(pszHostname);
    VMDIR_SAFE_FREE_MEMORY(pszSite);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed (%d)",
            __FUNCTION__,
            dwError);

    VmDirFreeReplMetrics(pReplMetrics);
    goto cleanup;
}

DWORD
VmDirReplMetricsCacheFind(
    PCSTR                       pszHostname,
    PVMDIR_REPLICATION_METRICS* ppReplMetrics
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    PVMDIR_REPLICATION_METRICS  pReplMetrics = NULL;

    if (IsNullOrEmptyString(pszHostname) || !ppReplMetrics)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_RWLOCK_READLOCK(bInLock, gVdirReplMetricsCache.pLock, 0);

    if (LwRtlHashMapFindKey(
            gVdirReplMetricsCache.pHashMap,
            (PVOID*)&pReplMetrics,
            pszHostname) == 0 && pReplMetrics->bActive)
    {
        *ppReplMetrics = pReplMetrics;
    }
    else
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NOT_FOUND);
    }

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, gVdirReplMetricsCache.pLock);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
VmDirReplMetricsCacheRemove(
    PCSTR   pszHostname
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    PVMDIR_REPLICATION_METRICS  pReplMetrics = NULL;

    if (IsNullOrEmptyString(pszHostname))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VMDIR_RWLOCK_WRITELOCK(bInLock, gVdirReplMetricsCache.pLock, 0);

    if (LwRtlHashMapFindKey(
            gVdirReplMetricsCache.pHashMap,
            (PVOID*)&pReplMetrics,
            pszHostname) == 0 && pReplMetrics->bActive)
    {
        pReplMetrics->bActive = FALSE;
    }
    else
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NOT_FOUND);
    }

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInLock, gVdirReplMetricsCache.pLock);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

VOID
VmDirReplMetricsCacheShutdown(
    VOID
    )
{
    BOOLEAN bInLock = FALSE;

    VMDIR_RWLOCK_WRITELOCK(bInLock, gVdirReplMetricsCache.pLock, 0);
    if (gVdirReplMetricsCache.pHashMap)
    {
        LwRtlHashMapClear(gVdirReplMetricsCache.pHashMap, _FreeCacheMapPair, NULL);
        LwRtlFreeHashMap(&gVdirReplMetricsCache.pHashMap);
    }
    VMDIR_RWLOCK_UNLOCK(bInLock, gVdirReplMetricsCache.pLock);
    VMDIR_SAFE_FREE_RWLOCK(gVdirReplMetricsCache.pLock);
}

DWORD
VmDirReplMetricsInit(
    PSTR                        pszHostname,
    PSTR                        pszSite,
    PVMDIR_REPLICATION_METRICS* ppReplMetrics
    )
{
    DWORD   dwError = 0;
    PVMDIR_REPLICATION_METRICS  pReplMetrics = NULL;

    // use identical bucket for all histograms
    uint64_t    buckets[4] = {1, 10, 100, 1000};

    // use this template to construct labels
    VM_METRICS_LABEL labels[4] =
    {
            {"src_hostname",    NULL},
            {"dst_hostname",    NULL},
            {"src_site",        NULL},
            {"dst_site",        NULL}
    };

    if (IsNullOrEmptyString(pszHostname) ||
        IsNullOrEmptyString(pszSite) ||
        !ppReplMetrics)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VMDIR_REPLICATION_METRICS),
            (PVOID*)&pReplMetrics);
    BAIL_ON_VMDIR_ERROR(dwError);

    // get src server hostname
    dwError = VmDirAllocateStringA(pszHostname, &pReplMetrics->pszSrcHostname);
    BAIL_ON_VMDIR_ERROR(dwError);

    //get src server site
    dwError = VmDirAllocateStringA(pszSite, &pReplMetrics->pszSrcSite);
    BAIL_ON_VMDIR_ERROR(dwError);

    // get dst server hostname (localhost)
    dwError = VmDirAllocateStringA(
            gVmdirServerGlobals.bvServerObjName.lberbv_val,
            &pReplMetrics->pszDstHostname);
    BAIL_ON_VMDIR_ERROR(dwError);

    // get dst server site (localhost)
    dwError = VmDirAllocateStringA(
            gVmdirServerGlobals.pszSiteName,
            &pReplMetrics->pszDstSite);
    BAIL_ON_VMDIR_ERROR(dwError);

    // fill in labels template
    labels[0].pszValue = pReplMetrics->pszSrcHostname;
    labels[1].pszValue = pReplMetrics->pszDstHostname;
    labels[2].pszValue = pReplMetrics->pszSrcSite;
    labels[3].pszValue = pReplMetrics->pszDstSite;

    // create time metrics
    dwError = VmMetricsGaugeNew(
            pmContext,
            "vmdir_repl_time_converge",
            labels, 4,
            "Time taken to replicate a change from the originating server to the destination server",
            &pReplMetrics->pTimeConverge);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmMetricsGaugeNew(
            pmContext,
            "vmdir_repl_time_onehop",
            labels, 4,
            "Time taken to replicate changes from direct partners to the destination server",
            &pReplMetrics->pTimeOnehop);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmMetricsHistogramNew(
            pmContext,
            "vmdir_repl_time_cycle_succeeded",
            labels, 2,
            "Time taken to complete replication cycle (successful) with each direct partner",
            buckets, 4,
            &pReplMetrics->pTimeCycleSucceeded);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmMetricsHistogramNew(
            pmContext,
            "vmdir_repl_time_cycle_failed",
            labels, 2,
            "Time taken to complete replication cycle (unsuccessful) with each direct partner",
            buckets, 4,
            &pReplMetrics->pTimeCycleFailed);
    BAIL_ON_VMDIR_ERROR(dwError);

    // create usn metrics
    dwError = VmMetricsHistogramNew(
            pmContext,
            "vmdir_repl_usn_behind",
            labels, 4,
            "USN difference to quantify replication lags between two nodes",
            buckets, 4,
            &pReplMetrics->pUsnBehind);
    BAIL_ON_VMDIR_ERROR(dwError);

    // create count metrics
    dwError = VmMetricsCounterNew(
            pmContext,
            "vmdir_repl_count_conflict_resolved",
            labels, 2,
            "Number of conflicts that are resolved during replication cycles",
            &pReplMetrics->pCountConflictResolved);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmMetricsCounterNew(
            pmContext,
            "vmdir_repl_count_conflict_permanent",
            labels, 2,
            "Number of conflicts that are permanent between two nodes",
            &pReplMetrics->pCountConflictPermanent);
    BAIL_ON_VMDIR_ERROR(dwError);

    /*
     * TODO need a counter per error code
    dwError = VmMetricsCounterNew(
            pmContext,
            "vmdir_repl_count_error",
            labels, 2,
            "Number of errors during replication cycles",
            &pReplMetrics->pCountError);
    BAIL_ON_VMDIR_ERROR(dwError);
     */

    *ppReplMetrics = pReplMetrics;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed (%d)",
            __FUNCTION__,
            dwError);

    VmDirFreeReplMetrics(pReplMetrics);
    goto cleanup;
}

VOID
VmDirFreeReplMetrics(
    PVMDIR_REPLICATION_METRICS  pReplMetrics
    )
{
    if (pReplMetrics)
    {
        VMDIR_SAFE_FREE_MEMORY(pReplMetrics->pszSrcHostname);
        VMDIR_SAFE_FREE_MEMORY(pReplMetrics->pszSrcSite);
        VMDIR_SAFE_FREE_MEMORY(pReplMetrics->pszDstHostname);
        VMDIR_SAFE_FREE_MEMORY(pReplMetrics->pszDstSite);
        (VOID)VmMetricsGaugeDelete(pmContext, pReplMetrics->pTimeConverge);
        (VOID)VmMetricsGaugeDelete(pmContext, pReplMetrics->pTimeOnehop);
        (VOID)VmMetricsHistogramDelete(pmContext, pReplMetrics->pTimeCycleSucceeded);
        (VOID)VmMetricsHistogramDelete(pmContext, pReplMetrics->pTimeCycleFailed);
        (VOID)VmMetricsHistogramDelete(pmContext, pReplMetrics->pUsnBehind);
        (VOID)VmMetricsCounterDelete(pmContext, pReplMetrics->pCountConflictResolved);
        (VOID)VmMetricsCounterDelete(pmContext, pReplMetrics->pCountConflictPermanent);
    }
}

static
VOID
_FreeCacheMapPair(
    PLW_HASHMAP_PAIR    pPair,
    PVOID               pUnused
    )
{
    VmDirFreeReplMetrics((PVMDIR_REPLICATION_METRICS)pPair->pValue);
}
