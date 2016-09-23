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
 * Module Name: Urgent Replication Coordinator thread
 *
 * Filename: urgentreplthread.c
 *
 * Abstract:
 * 1) Urgent replication coordinator thread will be signalled by writer thread
 *    to start urgent replication request.
 * 2) Will read replication Aggreements and send Urgent replication requests
 *    to all the replication partners.
 * 3) Waits for all the reponses and signals all the waiting writer threads.
 *
 */

#include "includes.h"

#define VMDIR_URGENT_REPL_RPC_RESPONSE_TIMEOUT (3 * SECONDS_IN_MINUTE)

#ifndef LOG_MUTEX
#define LOG_MUTEX(a)  { VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "[%s:%d]: LOG_MUTEX %s", __FUNCTION__, __LINE__,a);}
#endif

static
DWORD
VmDirReplUrgentReplCoordinatorThreadFun(
    PVOID pArg
    );

static
VOID
VmDirWaitForUrgentReplRequest(
    VOID
    );

static
VOID
_VmDirWaitForUrgentReplResponse(
    DWORD  dwRpcRequestsSent
    );

static
VOID
VmDirReplBroadcastUrgentReplDone(
    VOID
    );

DWORD
InitializeUrgentReplCoordinatorThread(
    VOID
    )
{
    DWORD               dwError = 0;
    PVDIR_THREAD_INFO   pThrInfo = NULL;

    dwError = VmDirSrvThrInit(
                &pThrInfo,
                gVmdirUrgentRepl.pUrgentReplThreadMutex,
                gVmdirUrgentRepl.pUrgentReplThreadCondition,
                TRUE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateThread(
                &pThrInfo->tid,
                FALSE,
                VmDirReplUrgentReplCoordinatorThreadFun,
                pThrInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSrvThrAdd(pThrInfo);

cleanup:
    return dwError;

error:
    VmDirSrvThrFree(pThrInfo);
    goto cleanup;
}

VOID
VmDirUrgentReplSignalUrgentReplCoordinatorThreadStart(
    VOID
    )
{
    DWORD      dwError = 0;
    BOOLEAN    bInUrgentReplThreadLock = FALSE;

    VmDirReplSetUrgentReplThreadCondition(TRUE);

    VMDIR_LOCK_MUTEX(bInUrgentReplThreadLock, gVmdirUrgentRepl.pUrgentReplThreadMutex);

    dwError = VmDirConditionSignal(gVmdirUrgentRepl.pUrgentReplThreadCondition);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL,
            "VmDirUrgentReplSignalUrgentReplCoordinatorThreadStart: signal");
cleanup:
    VMDIR_UNLOCK_MUTEX(bInUrgentReplThreadLock, gVmdirUrgentRepl.pUrgentReplThreadMutex);
    return;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
      "VmDirUrgentReplSignalUrgentReplCoordinatorThreadStart: error: %d", dwError);
    goto cleanup;
}

VOID
VmDirUrgentReplSignalUrgentReplCoordinatorThreadResponseRecv(
    VOID
    )
{
    DWORD      dwError = 0;
    BOOLEAN    bInUrgentReplResponseRecvLock = FALSE;

    VMDIR_LOCK_MUTEX(bInUrgentReplResponseRecvLock, gVmdirUrgentRepl.pUrgentReplResponseRecvMutex);
    LOG_MUTEX("lock");

    VmDirReplSetUrgentReplResponseRecvCondition_InLock(TRUE);

    dwError = VmDirConditionSignal(gVmdirUrgentRepl.pUrgentReplResponseRecvCondition);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_UNLOCK_MUTEX(bInUrgentReplResponseRecvLock, gVmdirUrgentRepl.pUrgentReplResponseRecvMutex);
    LOG_MUTEX("unlock");
    return;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
      "VmDirUrgentReplSignalUrgentReplCoordinatorThreadResponseRecv: error: %d", dwError);
    goto cleanup;
}

DWORD
VmDirTimedWaitForUrgentReplDone(
    UINT64 timeout,
    UINT64 startTime
    )
{
    BOOLEAN  bInUrgentReplDoneLock = FALSE;
    DWORD    dwError = 0;
    DWORD    dwNewTimeout = 0;
    UINT64   endTime = 0;

    VmDirReplSetUrgentReplDoneCondition(FALSE);
    dwNewTimeout = (DWORD)timeout;

    VMDIR_LOCK_MUTEX(bInUrgentReplDoneLock, gVmdirUrgentRepl.pUrgentReplDoneMutex);

    /*
     * In the case of suprious wakeup dwError will be '0'. Hence instead of
     * BAIL'ing out, VmDirTimedWaitForUrgentReplDone will calculate the new
     * Timeout and check VmDirReplGetUrgentReplDoneCondition. VmDirReplGetUrgentReplDoneCondition
     * will return FALSE, thread will again wait for the condition to happen
     * with a new timeout value.
     *
     * If Signalled but USN not updated. Then VmDirPerformUrgentReplication would
     * calculate the new time out and call VmDirTimedWaitForUrgentReplDone again.
     */
    while (VmDirReplGetUrgentReplDoneCondition() == FALSE)
    {
        dwError = VmDirConditionTimedWait(gVmdirUrgentRepl.pUrgentReplDoneCondition,
                                          gVmdirUrgentRepl.pUrgentReplDoneMutex,
                                          dwNewTimeout
                                          );
        BAIL_ON_VMDIR_ERROR(dwError);

        // calculate the new time out
        endTime = VmDirGetTimeInMilliSec();
        if ((startTime + timeout) > endTime)
        {
            dwNewTimeout = (DWORD)((startTime + timeout) - endTime);
        }
        else
        {
            // should we not check for success first??


            dwError = ETIMEDOUT;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bInUrgentReplDoneLock, gVmdirUrgentRepl.pUrgentReplDoneMutex);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
        " VmDirTimedWaitForUrgentReplDone: failed error: %d", dwError);
    goto cleanup;
}

static
DWORD
VmDirReplUrgentReplCoordinatorThreadFun(
    PVOID pArg
    )
{
    PSTR     pszPartnerHostName = NULL;
    DWORD    dwRpcRequestsSent = 0;
    DWORD    dwError = 0;
    time_t   startTime = 0;
    BOOLEAN  bInReplAgrsLock = FALSE;
    BOOLEAN  bUrgentReplPartialFailure = FALSE;
    VMDIR_REPLICATION_AGREEMENT    *pReplAgr = NULL;

    while (1)
    {
        /*
         * If strong consistency write requests were made when
         * urgent replication is active this boolean will be set
         * If urgent replication pending boolean is set, rather than blocking
         * thread will perform another urgent replication cycle
         */
        if (VmDirGetUrgentReplicationPending() == FALSE)
        {
            VmDirWaitForUrgentReplRequest();
        }

        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            goto cleanup;
        }

        VmDirSetUrgentReplicationPending(FALSE);
        VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL,
            "VmDirReplUrgentReplCoordinatorThreadFun: Initiating Urgent Replication Request to all Replication Partners");

        dwRpcRequestsSent = 0;
        bUrgentReplPartialFailure = FALSE;
	VmDirReplResetUrgentReplResponseCount();

        VMDIR_LOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);

        for (pReplAgr = gVmdirReplAgrs; pReplAgr != NULL; pReplAgr = pReplAgr->next)
        {
            if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
            {
                goto cleanup;
            }

            if (pReplAgr->isDeleted) // skip deleted RAs
            {
                VmDirReplUpdateUrgentReplCoordinatorTableForDelete(pReplAgr);
                continue;
            }

	    VMDIR_SAFE_FREE_MEMORY(pszPartnerHostName);

            dwError = VmDirReplURIToHostname(pReplAgr->ldapURI, &pszPartnerHostName);
            if (dwError != 0)
            {
                VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "VmDirReplUrgentReplCoordinatorThreadFun: URI:%s to host name failed status: %d",
                    pReplAgr->ldapURI,
                    dwError);
                bUrgentReplPartialFailure = TRUE;
                continue;
            }

            dwError = VmDirUrgentReplicationRequest(pszPartnerHostName);
            if (dwError != 0)
            {
                VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "VmDirReplUrgentReplCoordinatorThreadFun: VmDirUrgentReplicationRequest failed with status: %d",
                    dwError);
                bUrgentReplPartialFailure = TRUE;
                continue;
            }

            dwRpcRequestsSent++;
	    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL,
	        "VmDirReplUrgentReplCoordinatorThreadFun :requests:%d hostname: %s",
	        dwRpcRequestsSent, pReplAgr->ldapURI);
        }

        VMDIR_UNLOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);

        // Update Notified Time in all the entries in the Urgent Replication Table
	VmDirReplUpdateUrgentReplCoordinatorTableForRequest();
        startTime = time(NULL);

        while ((time(NULL) - startTime) < VMDIR_URGENT_REPL_RPC_RESPONSE_TIMEOUT &&
                VmDirReplGetUrgentReplResponseCount() < dwRpcRequestsSent)
        {
             _VmDirWaitForUrgentReplResponse(dwRpcRequestsSent);

             if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
             {
                 goto cleanup;
             }
             VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL,
                 "VmDirReplUrgentReplCoordinatorThreadFun:requests:%d Responses:%d",
                 dwRpcRequestsSent, VmDirReplGetUrgentReplResponseCount());
        }

        /*
         * if bUrgentReplPartialFailure is true
         * dwRpcRequestsSent is '0' and bUrgentReplPartialFailure is False - node with no repl partners
         *     - broadcast all writer threads without updating consensus
         * if all responses received,
         *     - broadcast all writer threads after updating consensus
         */
        if (bUrgentReplPartialFailure || dwRpcRequestsSent == 0 ||
            (VmDirReplGetUrgentReplResponseCount() == dwRpcRequestsSent &&
            VmDirUrgentReplUpdateConsensus())
           )
        {
            VmDirReplBroadcastUrgentReplDone();
        }
    }

cleanup:
    if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
    {
        VmDirReplBroadcastUrgentReplDone();
    }
    VMDIR_UNLOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);
    VmDirReplFreeUrgentReplCoordinatorTable();
    return dwError;
}

static
VOID
VmDirWaitForUrgentReplRequest(
    VOID
    )
{
    BOOLEAN    bInUrgentReplThreadLock = FALSE;
    DWORD      dwError = 0;

    VmDirReplSetUrgentReplThreadCondition(FALSE);

    VMDIR_LOCK_MUTEX(bInUrgentReplThreadLock, gVmdirUrgentRepl.pUrgentReplThreadMutex);

    // Either Strong consistency write request or main thread shutdown will exit condition wait call below
    while (VmDirReplGetUrgentReplThreadCondition() == FALSE && VmDirdState() != VMDIRD_STATE_SHUTDOWN)
    {
        dwError = VmDirConditionWait(gVmdirUrgentRepl.pUrgentReplThreadCondition,
                                     gVmdirUrgentRepl.pUrgentReplThreadMutex
                                     );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL,
            "VmDirWaitForUrgentReplRequest: signaled urgent replication request - starting execution" );

cleanup:
    VMDIR_UNLOCK_MUTEX(bInUrgentReplThreadLock, gVmdirUrgentRepl.pUrgentReplThreadMutex);
    return;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
        "VmDirWaitForUrgentReplRequest: wait on pUrgentReplThreadCondition failed with error: %d", dwError);
    goto cleanup;
}

static
VOID
_VmDirWaitForUrgentReplResponse(
    DWORD  dwRpcRequestsSent
    )
{
    BOOLEAN  bInUrgentReplResponseRecvLock = FALSE;
    DWORD    dwError = 0;
    DWORD    dwMilliseconds = 0;
    DWORD    timeoutCount = 0;

    dwMilliseconds = 3000; // Timeout of 3 seconds

    VMDIR_LOCK_MUTEX(bInUrgentReplResponseRecvLock, gVmdirUrgentRepl.pUrgentReplResponseRecvMutex);
    LOG_MUTEX("lock");

    VmDirReplSetUrgentReplResponseRecvCondition_InLock(FALSE);

    // Either urgent replication response from repl Partner or main thread shutdown will exit condition wait call below
    while (VmDirReplGetUrgentReplResponseRecvCondition_InLock() == FALSE && VmDirdState() != VMDIRD_STATE_SHUTDOWN)
    {
        /*
         * Time out or condition met break out of the loop
         */
        if (timeoutCount >= 20 ||
            VmDirReplGetUrgentReplResponseCount_InLock() == dwRpcRequestsSent)
        {
            break;
        }
        LOG_MUTEX("ConditionTimedWait start");

        dwError = VmDirConditionTimedWait(gVmdirUrgentRepl.pUrgentReplResponseRecvCondition,
                                          gVmdirUrgentRepl.pUrgentReplResponseRecvMutex,
                                          dwMilliseconds
                                          );
        LOG_MUTEX("ConditionTimedWait finish");

        /*
         * Total time out is 60 seconds, but in the case of server shutdown
         * we might have to exit execution as soon as possible. In order to accomplish that
         * for every 3 seconds resume execution and check for director state.
         */
        if (dwError != 0)
        {
            if (dwError == ETIMEDOUT)
            {
	        timeoutCount++;
            }
            else
            {
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
    }
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_UNLOCK_MUTEX(bInUrgentReplResponseRecvLock, gVmdirUrgentRepl.pUrgentReplResponseRecvMutex);
    LOG_MUTEX("unlock");

    return;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
        "_VmDirWaitForUrgentReplResponse: pUrgentReplResponseRecvCondition wait failed with error: %d", dwError);
    goto cleanup;
}

static
VOID
VmDirReplBroadcastUrgentReplDone(
    VOID
    )
{
    DWORD     dwError = 0;
    BOOLEAN   bInUrgentReplDoneCondition = FALSE;

    VmDirReplSetUrgentReplDoneCondition(TRUE);

    VMDIR_LOCK_MUTEX(bInUrgentReplDoneCondition, gVmdirUrgentRepl.pUrgentReplDoneMutex);

    dwError = VmDirConditionBroadcast(gVmdirUrgentRepl.pUrgentReplDoneCondition);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL,
            " VmDirReplBroadcastUrgentReplDone: signal broadcast ");
cleanup:
    VMDIR_UNLOCK_MUTEX(bInUrgentReplDoneCondition, gVmdirUrgentRepl.pUrgentReplDoneMutex);
    return;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
      " VmDirReplBroadcastUrgentReplDone: failed with error: %d", dwError);
    goto cleanup;
}

BOOLEAN
VmDirUrgentReplCondTimedWait(
    VOID
    )
{
    BOOLEAN  bInUrgentReplStartLock = FALSE;
    DWORD    dwError = 0;
    DWORD    dwNewTimeout = 0;
    DWORD    dwTimeout = 0;
    UINT64   startTime = 0;
    UINT64   endTime = 0;
    BOOLEAN  btimeout = FALSE;

    dwNewTimeout = dwTimeout = MSECS_IN_SECOND;

    startTime = VmDirGetTimeInMilliSec();

    VMDIR_LOCK_MUTEX(bInUrgentReplStartLock, gVmdirUrgentRepl.pUrgentReplStartMutex);

    while (VmDirdGetReplNow() == FALSE)
    {
        dwError = VmDirConditionTimedWaitMilliSeconds(
                                gVmdirUrgentRepl.pUrgentReplStartCondition,
                                gVmdirUrgentRepl.pUrgentReplStartMutex,
                                dwNewTimeout);
        // BAIL In the case of error
        if (dwError != 0 && dwError != ETIMEDOUT)
        {
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        // calculate the new time out - for spurious wakeup
        endTime = VmDirGetTimeInMilliSec();
        if ((startTime + dwTimeout) > endTime)
        {
            dwNewTimeout = (DWORD)((startTime + dwTimeout) - endTime);
        }

        // Break only after 1000 milliseconds
        if ((endTime > startTime) && (endTime - startTime) >= MSECS_IN_SECOND)
        {
            btimeout = TRUE;
            break;
        }
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bInUrgentReplStartLock, gVmdirUrgentRepl.pUrgentReplStartMutex);
    return btimeout;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
        " VmDirUrgentReplTimedWaitUrgentReplStart: failed error: %d", dwError);
    goto cleanup;
}

VOID
VmDirUrgentReplSignal(
    VOID
    )
{
    DWORD      dwError = 0;
    BOOLEAN    bUrgentReplStartLock = FALSE;

    VMDIR_LOCK_MUTEX(bUrgentReplStartLock, gVmdirUrgentRepl.pUrgentReplStartMutex);

    dwError = VmDirConditionSignal(gVmdirUrgentRepl.pUrgentReplStartCondition);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_UNLOCK_MUTEX(bUrgentReplStartLock, gVmdirUrgentRepl.pUrgentReplStartMutex);
    return;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
      "VmDirUrgentReplSignalUrgentReplStart: error: %d", dwError);
    goto cleanup;
}
