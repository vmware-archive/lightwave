/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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
 * Module Name: Replication thread
 *
 * Filename: thread.c
 *
 * Abstract:
 *
 */

#include "includes.h"

static
VMDIR_DC_CONNECTION_STATE
_VmDirReplicationConnect(
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr
    );

static
DWORD
_VmDirWaitForReplicationAgreement(
    PBOOLEAN pbExitReplicationThread
    );

static
VOID
_VmDirConsumePartner(
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr
    );

static
DWORD
vdirReplicationThrFun(
    PVOID   pArg
    );

static
BOOLEAN
_VmDirSkipReplicationCycle(
    VOID
    );

DWORD
VmDirGetReplCycleCounter(
    VOID
    )
{
    BOOLEAN bInLock = FALSE;
    DWORD   dwCount = 0;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirGlobals.replCycleDoneMutex);
    dwCount = gVmdirGlobals.dwReplCycleCounter;
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirGlobals.replCycleDoneMutex);

    return dwCount;
}

DWORD
InitializeReplicationThread(
    void)
{
    DWORD               dwError = 0;
    PVDIR_THREAD_INFO   pThrInfo = NULL;

    dwError = VmDirSrvThrInit(
            &pThrInfo,
            gVmdirGlobals.replAgrsMutex,
            gVmdirGlobals.replAgrsCondition,
            TRUE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateThread(
            &pThrInfo->tid,
            pThrInfo->bJoinThr,
            vdirReplicationThrFun,
            pThrInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSrvThrAdd(pThrInfo);

cleanup:

    return dwError;

error:

    VmDirSrvThrFree(pThrInfo);

    goto cleanup;
}

// vdirReplicationThrFun is the main replication function that:
//  - Executes replication cycles endlessly
//  - Each replication cycle consist of processing all the RAs for this vmdird instance.
//  - Sleeps for gVmdirServerGlobals.replInterval between replication cycles.
//

/*
     1.  Wait for a replication agreement
     2.  While server running
     2a.   Load schema context
     2b.   Load credentials
     2c.   For each replication agreement
     2ci.      Connect to system
     2cii.     Fetch Page
     2ciii.    Process Page
*/
static
DWORD
vdirReplicationThrFun(
    PVOID   pArg
    )
{
    int                             retVal = 0;
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr = NULL;
    BOOLEAN                         bInReplAgrsLock = FALSE;
    BOOLEAN                         bInReplCycleDoneLock = FALSE;
    BOOLEAN                         bExitThread = FALSE;
    int                             i = 0;

    /*
     * This is to address the backend's writer mutex contention problem so that
     * the replication thread wouldn't be starved by the local operational threads'
     * write operations.
     */
    VmDirRaiseThreadPriority(DEFAULT_THREAD_PRIORITY_DELTA);

    retVal = _VmDirWaitForReplicationAgreement(&bExitThread);
    BAIL_ON_VMDIR_ERROR(retVal);

    if (bExitThread)
    {
        goto cleanup;
    }

    while (1)
    {
        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            goto cleanup;
        }

        if (VmDirdState() == VMDIRD_STATE_STANDALONE)
        {
            VmDirSleep(1000);
            continue;
        }

        VMDIR_LOG_VERBOSE(
                VMDIR_LOG_MASK_ALL,
                "vdirReplicationThrFun: Executing replication cycle %u.",
                gVmdirGlobals.dwReplCycleCounter + 1);

        // purge RAs that have been marked as isDeleted = TRUE
        VmDirRemoveDeletedRAsFromCache();

        VMDIR_LOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);
        pReplAgr = gVmdirReplAgrs;
        VMDIR_UNLOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);

        for (; pReplAgr != NULL; pReplAgr = pReplAgr->next )
        {
            if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
            {
                goto cleanup;
            }

            if (pReplAgr->isDeleted || _VmDirSkipReplicationCycle()) // skip deleted RAs
            {
                continue;
            }

            if (_VmDirReplicationConnect(pReplAgr) != DC_CONNECTION_STATE_CONNECTED)
            {
                continue;
            }

            _VmDirConsumePartner(pReplAgr);
            /*
             * To avoid race condition after resetting the vector
             * if this node plays consumer role before supplying the new vector value
             * could result in longer replication cycle.
             */
            VmDirSleep(100);
        }

        VMDIR_LOG_DEBUG(
                VMDIR_LOG_MASK_ALL,
                "vdirReplicationThrFun: Done executing the replication cycle.");

        VMDIR_LOCK_MUTEX(bInReplCycleDoneLock, gVmdirGlobals.replCycleDoneMutex);
        gVmdirGlobals.dwReplCycleCounter++;

        if ( gVmdirGlobals.dwReplCycleCounter == 1 )
        {   // during promotion scenario, start listening on ldap ports after first cycle.
            VmDirConditionSignal(gVmdirGlobals.replCycleDoneCondition);
        }
        VMDIR_UNLOCK_MUTEX(bInReplCycleDoneLock, gVmdirGlobals.replCycleDoneMutex);

        if (VmDirdState() == VMDIRD_STATE_RESTORE)
        {
            VMDIR_LOG_INFO(
                    VMDIR_LOG_MASK_ALL,
                    "vdirReplicationThrFun: Done restoring the server by catching up with it's replication partner(s).");
            VmDirForceExit();
            break;
        }

        VMDIR_LOG_DEBUG(
                VMDIR_LOG_MASK_ALL,
                "vdirReplicationThrFun: Sleeping for the replication interval: %d seconds.",
                gVmdirServerGlobals.replInterval);

        for (i=0; i<gVmdirServerGlobals.replInterval; i++)
        {
            if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
            {
                goto cleanup;
            }

            // An RPC call requested a replication cycle to start immediately
            if (VmDirdGetReplNow() == TRUE)
            {
                VmDirdSetReplNow(FALSE);
                break;
            }
        }
    } // Endless replication loop

cleanup:
    VMDIR_UNLOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);
    VMDIR_UNLOCK_MUTEX(bInReplCycleDoneLock, gVmdirGlobals.replCycleDoneMutex);
    return 0;

error:
    VmDirdStateSet(VMDIRD_STATE_FAILURE);
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "vdirReplicationThrFun: Replication has failed with unrecoverable error.");
    goto cleanup;
}

static
BOOLEAN
_VmDirSkipReplicationCycle(
    VOID
    )
{
    return (VmDirdState() == VMDIRD_STATE_READ_ONLY_DEMOTE ||
            VmDirdState() == VMDIRD_STATE_READ_ONLY);
}

static
VMDIR_DC_CONNECTION_STATE
_VmDirReplicationConnect(
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr
    )
{
    if (DC_CONNECTION_STATE_NOT_CONNECTED == pReplAgr->dcConn.connState ||
        DC_CONNECTION_STATE_FAILED        == pReplAgr->dcConn.connState)
    {
        VmDirInitDCConnThread(&pReplAgr->dcConn);
    }

    return pReplAgr->dcConn.connState;
}

DWORD
_VmDirWaitForReplicationAgreement(
    PBOOLEAN pbExitReplicationThread
    )
{
    DWORD dwError = 0;
    BOOLEAN bInReplAgrsLock = FALSE;
    int retVal = 0;
    PSTR pszPartnerHostName = NULL;

    assert(pbExitReplicationThread != NULL);

    VMDIR_LOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);
    while (gVmdirReplAgrs == NULL)
    {
        VMDIR_UNLOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);
        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            goto cleanup;
        }

        if (VmDirdState() == VMDIRD_STATE_RESTORE)
        {
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirWaitForReplicationAgreement: Done restoring the server, no partner to catch up with.");
            VmDirForceExit();
            *pbExitReplicationThread = TRUE;
            goto cleanup;
        }

        VMDIR_LOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);
        // wait till a replication agreement is created e.g. by vdcpromo
        if (VmDirConditionWait( gVmdirGlobals.replAgrsCondition, gVmdirGlobals.replAgrsMutex ) != 0)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirWaitForReplicationAgreement: VmDirConditionWait failed.");
            dwError = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR( dwError );

        }
        VMDIR_UNLOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);

        // When 1st RA is created for non-1st replica => try to perform special 1st replication cycle
        if (gVmdirReplAgrs)
        {
            if ( gFirstReplCycleMode == FIRST_REPL_CYCLE_MODE_COPY_DB )
            {
                if ((retVal=VmDirReplURIToHostname(gVmdirReplAgrs->ldapURI, &pszPartnerHostName)) !=0 ||
                    (retVal=VmDirFirstReplicationCycle( pszPartnerHostName, gVmdirReplAgrs)) != 0)
                {
                    VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL,
                          "vdirReplicationThrFun: VmDirReplURIToHostname or VmDirFirstReplicationCycle failed, error (%d).",
                          gVmdirReplAgrs->ldapURI, retVal);
                    VmDirForceExit();
                    *pbExitReplicationThread = TRUE;
                    dwError = LDAP_OPERATIONS_ERROR;
                    BAIL_ON_VMDIR_ERROR( dwError );
                }
                else
                {
                    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: VmDirFirstReplicationCycle() SUCCEEDED." );
                }
            }
            else if (gFirstReplCycleMode == FIRST_REPL_CYCLE_MODE_NONE)
            {
                // already promoted node and first partner added
            }
            else
            {
                assert(0);  // Lightwave server always use DB copy to bootstrap partner node.
            }
        }
        VMDIR_LOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);
    return dwError;

error:
    goto cleanup;
}

static
VOID
_VmDirConsumePartner(
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr
    )
{
    int                            retVal = LDAP_SUCCESS;
    USN                            lastLocalUsnProcessed = 0;
    uint64_t                       uiStartTime = 0;
    uint64_t                       uiEndTime = 0;
    PVMDIR_REPLICATION_UPDATE_LIST pReplUpdateList = NULL;
    PVMDIR_REPLICATION_METRICS     pReplMetrics = NULL;

    uiStartTime = VmDirGetTimeInMilliSec();

    retVal = VmDirStringToINT64(
            pReplAgr->lastLocalUsnProcessed.lberbv.bv_val, NULL, &lastLocalUsnProcessed);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    retVal = VmDirReplUpdateListFetch(pReplAgr, &pReplUpdateList);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    VmDirReplUpdateListProcess(pReplUpdateList);

    retVal = VmDirReplCookieUpdate(
            NULL,
            pReplUpdateList->newHighWaterMark,
            pReplUpdateList->pNewUtdVector,
            pReplAgr);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    VMDIR_LOG_INFO(
            VMDIR_LOG_MASK_ALL,
            "Replication supplier %s USN range (%llu,%s) processed",
            pReplAgr->ldapURI,
            lastLocalUsnProcessed,
            pReplAgr->lastLocalUsnProcessed.lberbv_val);

collectmetrics:
    uiEndTime = VmDirGetTimeInMilliSec();
    if (VmDirReplMetricsCacheFind(pReplAgr->pszHostname, &pReplMetrics) == 0)
    {
        if (retVal == LDAP_SUCCESS)
        {
            VmMetricsHistogramUpdate(
                    pReplMetrics->pTimeCycleSucceeded,
                    VMDIR_RESPONSE_TIME(uiEndTime-uiStartTime));
        }
        // avoid collecting benign error counts
        else if (retVal != LDAP_UNAVAILABLE &&  // server in mid-shutdown
                 retVal != LDAP_SERVER_DOWN &&  // connection lost
                 retVal != LDAP_TIMEOUT)        // connection lost
        {
            VmMetricsHistogramUpdate(
                    pReplMetrics->pTimeCycleFailed,
                    VMDIR_RESPONSE_TIME(uiEndTime-uiStartTime));
        }
    }

    VmDirFreeReplUpdateList(pReplUpdateList);

    return;

ldaperror:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error code (%d)",
            __FUNCTION__,
            retVal);

    goto collectmetrics;
}
