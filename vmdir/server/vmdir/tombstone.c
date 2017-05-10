/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
 * Module Name: Tombstone Reaping Thread
 *
 * Registry key to control the amount of time between reapings:
 * services\vmdir\parameters\TombstoneExpirationPeriod
 */

#include "includes.h"


static
DWORD
_VmDirTombstoneReapingThreadFun(
    PVOID pArg
    );

DWORD
VmDirInitTombstoneReapingThread(
    VOID
    )
{
    DWORD dwError = 0;
    PVDIR_THREAD_INFO pThrInfo = NULL;

    //
    // If the user sets the registry value to zero we take that to mean that
    // they don't want deleted objects to be reaped automatically (but will
    // presumably do so on their own accordance/volition).
    //
    if (gVmdirServerGlobals.dwTombstoneExpirationPeriod == 0)
    {
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Tombstone reaping disabled by user");
        goto cleanup;
    }

    dwError = VmDirSrvThrInit(&pThrInfo, NULL, NULL, TRUE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateThread(
                &pThrInfo->tid,
                pThrInfo->bJoinThr,
                _VmDirTombstoneReapingThreadFun,
                pThrInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSrvThrAdd(pThrInfo);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Tombstone reaping thread started");

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed, error %d", __FUNCTION__, dwError);

    VmDirSrvThrFree(pThrInfo);

    goto cleanup;
}

BOOL
_VmDirIsEntryExpired(
    PVDIR_ENTRY pEntry,
    DWORD dwExpirationPeriod
    )
{
    PVDIR_ATTRIBUTE pAttr = NULL;
    BOOL bExpired = FALSE;
    char pszTimeBuf[GENERALIZED_TIME_STR_LEN + 1] = {0};
    int retVal = 0;

    pAttr = VmDirFindAttrByName(pEntry, ATTR_MODIFYTIMESTAMP);
    assert(pAttr != NULL && pAttr->numVals == 1);

    /*
     * Since we store our timestamps in this string format ("20161007003515.0Z")
     * the easiest way to compare them is to generate another string version
     * that's offset by our expiration period. Then the strings can just be
     * compared lexically.
     */
    VmDirCurrentGeneralizedTimeWithOffset(
        pszTimeBuf,
        sizeof(pszTimeBuf),
        dwExpirationPeriod);
    retVal = VmDirStringCompareA(pAttr->vals[0].lberbv.bv_val, pszTimeBuf, TRUE);
    if (retVal < 0)
    {
        bExpired = TRUE;
    }
    else
    {
        bExpired = FALSE;
    }

    return bExpired;
}

static
DWORD
_VmDirReapExpiredEntries(
    DWORD dwExpirationPeriodInSec
    )
{
    DWORD dwError = 0;
    PVDIR_ENTRY pEntry = NULL;
    PVDIR_BACKEND_INTERFACE pBE = NULL;
    PVDIR_BACKEND_PARENT_ID_INDEX_ITERATOR pIterator = NULL;
    ENTRYID eId = 0;
    VDIR_ENTRY entry = {0};
    DWORD dwExpiredCount = 0;
    DWORD dwUnexpiredCount = 0;
    int expiredInBatch = 0;

    pBE = VmDirBackendSelect(NULL);
    dwError = VmDirSimpleDNToEntry(gVmdirServerGlobals.delObjsContainerDN.lberbv.bv_val, &pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

newParentIdIndexIterator:

    dwError = pBE->pfnBEParentIdIndexIteratorInit(pEntry->eId, &pIterator);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (pIterator->bHasNext)
    {
        dwError = pBE->pfnBEParentIdIndexIterate(pIterator, &eId);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = pBE->pfnBESimpleIdToEntry(eId, &entry);
        if (dwError != 0)
        {
            //
            // We have seen instances in the wild where this call fails due
            // to a bad entry, so let's keep going if this fails.
            //
            continue;
        }

        if (_VmDirIsEntryExpired(&entry, gVmdirServerGlobals.dwTombstoneExpirationPeriod))
        {
            dwError = VmDirDeleteEntry(&entry);
            if (dwError != ERROR_SUCCESS)
            {
                //
                // Log, but otherwise ignore, any error encountered while removing
                // tombstones.
                //
                VMDIR_LOG_ERROR(
                    VMDIR_LOG_MASK_ALL,
                    "Deleting entry %s failed, error (%d)",
                    VDIR_SAFE_STRING(entry.dn.lberbv.bv_val),
                    dwError);
            }

            expiredInBatch++;

            if ((++dwExpiredCount % TOMBSTONE_REAPING_THROTTLE_COUNT) == 0)
            {
                if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
                {
                    break;
                }
                VmDirSleep(TOMBSTONE_REAPING_THROTTLE_SLEEP);
            }
        }
        else
        {
            dwUnexpiredCount++;
        }

        VmDirFreeEntryContent(&entry);

        if (expiredInBatch >= VDIR_REAP_EXPIRED_ENTRIES_BATCH)
        {
            // Must shorten the read-only transaction so that
            // MDB can free the allocated blocks from the expired entries.
            pBE->pfnBEParentIdIndexIteratorFree(pIterator);
            pIterator = NULL;
            expiredInBatch = 0;
            goto newParentIdIndexIterator;
        }
    }

cleanup:
    VMDIR_LOG_INFO(
        VMDIR_LOG_MASK_ALL,
        "Tombstone reaping thread removed %d expired entries, %d non-expired entries remain",
        dwExpiredCount,
        dwUnexpiredCount);

    pBE->pfnBEParentIdIndexIteratorFree(pIterator);
    VmDirFreeEntry(pEntry);
    return dwError;

error:
    if (dwError != ERROR_INVALID_STATE)
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                "%s failed, error (%d)", __FUNCTION__, dwError );
    }

    goto cleanup;
}

static
DWORD
_VmDirTombstoneReapingThreadFun(
    PVOID pArg
    )
{
    DWORD dwError = 0;
    BOOLEAN bInLock = FALSE;
    PVDIR_THREAD_INFO pThreadInfo = (PVDIR_THREAD_INFO)pArg;

    VmDirDropThreadPriority(DEFAULT_THREAD_PRIORITY_DELTA);

    while (TRUE)
    {
        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            goto cleanup;
        }

        VMDIR_LOCK_MUTEX(bInLock, pThreadInfo->mutexUsed);

        // main thread will signal during shutdown
        VmDirConditionTimedWait(
                pThreadInfo->conditionUsed,
                pThreadInfo->mutexUsed,
                gVmdirServerGlobals.dwTombstoneThreadFrequency * 1000);
        // ignore error

        VMDIR_UNLOCK_MUTEX(bInLock, pThreadInfo->mutexUsed);

        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            goto cleanup;
        }

        //
        // If lotus hasn't been promoted yet then this will be NULL.
        //
        if (gVmdirServerGlobals.delObjsContainerDN.lberbv.bv_val == NULL)
        {
            continue;
        }

        //
        // _VmDirReapExpiredEntries will log if it fails, so we ignore the
        // error here.
        //
        (VOID)_VmDirReapExpiredEntries(gVmdirServerGlobals.dwTombstoneExpirationPeriod);
    }

cleanup:
    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "Tombstone reaping thread exiting");

    return dwError;
}
