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

static VMDIR_BKGD_TASK_CTX tasks[] =
{
        {
                VmDirBkgdUpdateLocalDomainControllerObj,
                60, // every minute
                "bkgdprevtime_replmetrics_updatelocaldomaincontrollerobj",
                {0}
        },
        {
                VmDirBkgdSrvStat,
                60 * 10, // every 10 minute
                "bkgdprevtime_update_srvstat",
                {0}
        },
        {
                VmDirBkgdCreateNewIntegChkReport,
                60 * 60 * 6, // every 6 hours
                "bkgdprevtime_replmetrics_createnewintegchkreport",
                {0}
        },
        {
                VmDirBkgdCompareIntegChkReports,
                60 * 60 * 24, // every day
                "bkgdprevtime_replmetrics_comparelasttwointegchkreports",
                {0}
        },
        {
                VmDirBkgdCountClosedConnections,
                60, // every minute
                "bkgdprevtime_replmetrics_countclosedconnections",
                {0}
        },
        {
                VmDirBkgdPingMaxOrigUsn,
                0,  // every time
                "bkgdprevtime_replmetrics_pingmaxorigusn",
                {0}
        }
};

DWORD
VmDirBkgdThreadInitialize(
    VOID
    )
{
    DWORD   dwError = 0;
    DWORD   dwNumTasks = 0, i = 0;
    PSTR    pszTmp = NULL;

    dwNumTasks = sizeof(tasks)/sizeof(tasks[0]);

    // load previous timestamps
    for (i = 0; i < dwNumTasks; i++)
    {
        VMDIR_SAFE_FREE_MEMORY(pszTmp);
        (VOID)VmDirBackendUniqKeyGetValue(tasks[i].pszPrevTimeKey, &pszTmp);

        dwError = VmDirStringCpyA(
                tasks[i].pszPrevTime,
                GENERALIZED_TIME_STR_SIZE,
                VDIR_SAFE_STRING(pszTmp));
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // start background thread
    gVmdirBkgdGlobals.bShutdown = FALSE;

    dwError = VmDirSrvThrInit(&gVmdirBkgdGlobals.pThrInfo, NULL, NULL, TRUE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateThread(
            &gVmdirBkgdGlobals.pThrInfo->tid,
            gVmdirBkgdGlobals.pThrInfo->bJoinThr,
            VmDirBkgdThreadFun,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszTmp);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

VOID
VmDirBkgdThreadShutdown(
    VOID
    )
{
    // stop background thread
    gVmdirBkgdGlobals.bShutdown = TRUE;

    if (gVmdirBkgdGlobals.pThrInfo)
    {
        VmDirSrvThrShutdown(gVmdirBkgdGlobals.pThrInfo);
        gVmdirBkgdGlobals.pThrInfo = NULL;
    }
}

DWORD
VmDirBkgdTaskUpdatePrevTime(
    PVMDIR_BKGD_TASK_CTX    pTaskCtx
    )
{
    DWORD   dwError = 0;

    if (!pTaskCtx)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    VmDirCurrentGeneralizedTime(
            pTaskCtx->pszPrevTime, sizeof(pTaskCtx->pszPrevTime));

    dwError = VmDirBackendUniqKeySetValue(
            pTaskCtx->pszPrevTimeKey, pTaskCtx->pszPrevTime, TRUE);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirBkgdThreadFun(
    PVOID   pArg
    )
{
    DWORD   dwNumTasks = 0;
    DWORD   i = 0;
    BOOLEAN bInLock = FALSE;
    char    pszExpireTime[GENERALIZED_TIME_STR_SIZE] = {0};
    PVDIR_THREAD_INFO       pThrInfo = NULL;

    pThrInfo = gVmdirBkgdGlobals.pThrInfo;

    // lower thread priority
    VmDirDropThreadPriority(DEFAULT_THREAD_PRIORITY_DELTA);

    // service warm-up time - 1 minute
    VMDIR_LOCK_MUTEX(bInLock, pThrInfo->mutex);
    (VOID)VmDirConditionTimedWait(pThrInfo->condition, pThrInfo->mutex, 1000 * 60);
    VMDIR_UNLOCK_MUTEX(bInLock, pThrInfo->mutex);

    dwNumTasks = sizeof(tasks)/sizeof(tasks[0]);

    while (!gVmdirBkgdGlobals.bShutdown)
    {
        // perform tasks only if promoted
        if (gVmdirServerGlobals.bPromoted)
        {
            for (i = 0; i < dwNumTasks; i++)
            {
                VmDirCurrentGeneralizedTimeWithOffset(
                        pszExpireTime, sizeof(pszExpireTime), tasks[i].dwPeriod);

                if (VmDirStringCompareA(tasks[i].pszPrevTime, pszExpireTime, FALSE) < 0)
                {
                    // don't bail - continue to next task
                    (VOID)tasks[i].pFunc(&tasks[i]);
                }
            }
        }

        // sleep for 10 seconds
        VMDIR_LOCK_MUTEX(bInLock, pThrInfo->mutex);
        (VOID)VmDirConditionTimedWait(pThrInfo->condition, pThrInfo->mutex, 1000 * 10);
        VMDIR_UNLOCK_MUTEX(bInLock, pThrInfo->mutex);
    }

    return 0;
}

DWORD
VmDirBkgdUpdateLocalDomainControllerObj(
    PVMDIR_BKGD_TASK_CTX    pTaskCtx
    )
{
    DWORD   dwError = 0;
    VDIR_BERVALUE   bvComment = VDIR_BERVALUE_INIT;

    if (!pTaskCtx)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    bvComment.lberbv.bv_val = (PSTR)__FUNCTION__;
    bvComment.lberbv.bv_len = VmDirStringLenA(__FUNCTION__);

    dwError = VmDirInternalEntryAttributeReplace(
            NULL,
            gVmdirServerGlobals.dcAccountDN.bvnorm_val,
            ATTR_COMMENT,
            &bvComment);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirBkgdTaskUpdatePrevTime(pTaskCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirBkgdCreateNewIntegChkReport(
    PVMDIR_BKGD_TASK_CTX    pTaskCtx
    )
{
    DWORD   dwError = 0;
    DWORD   dwJobInterval = 0;

    if (!pTaskCtx)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmRegCfgGetKeyDword(
        VMDIR_CONFIG_PARAMETER_PARAMS_KEY_PATH,
        VMDIR_REG_KEY_INTEGRITY_CHK_INTERVAL_IN_SEC,
        &dwJobInterval,
        0);
    if (dwError == LWREG_ERROR_NO_SUCH_KEY_OR_VALUE) // key not set, default to disable.
    {
        dwError = VmDirBkgdTaskUpdatePrevTime(pTaskCtx);
        BAIL_ON_VMDIR_ERROR(dwError);

        goto cleanup;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwJobInterval  = VMDIR_MAX(60 * 60 * 1, dwJobInterval);
    pTaskCtx->dwPeriod = dwJobInterval;

    dwError = VmDirIntegrityCheckStart(INTEGRITY_CHECK_JOB_START, pTaskCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    // NOT updating prev time here - will be updated by integrity check thread

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirBkgdCompareIntegChkReports(
    PVMDIR_BKGD_TASK_CTX    pTaskCtx
    )
{
    DWORD   dwError = 0;
    DWORD   dwJobInterval = 0;
    DWORD   i = 0;
    PCSTR   pszReportDir = VMDIR_INTEG_CHK_REPORTS_DIR;
    PCSTR   pszArchiveDir = VMDIR_INTEG_CHK_ARCHIVE_DIR;
    PSTR    pszReportPath = NULL;
    PSTR    pszArchivePath = NULL;
    PVMDIR_STRING_LIST  pReportFiles = NULL;
    PVMDIR_INTEGRITY_REPORT pOrgReport = NULL;
    PVMDIR_INTEGRITY_REPORT pNewReport = NULL;
    PVMDIR_INTEGRITY_REPORT pFnlReport = NULL;
    PVDIR_LINKED_LIST       pReportsByPartner = NULL;
    PVDIR_LINKED_LIST_NODE  pTail = NULL;
    PVDIR_LINKED_LIST_NODE  pNode = NULL;
    PVMDIR_REPLICATION_METRICS  pReplMetrics = NULL;

    if (!pTaskCtx)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmRegCfgGetKeyDword(
        VMDIR_CONFIG_PARAMETER_PARAMS_KEY_PATH,
        VMDIR_REG_KEY_INTEGRITY_RPT_INTERVAL_IN_SEC,
        &dwJobInterval,
        0);
    if (dwError == LWREG_ERROR_NO_SUCH_KEY_OR_VALUE) // key not set, default to disable.
    {
        dwError = VmDirBkgdTaskUpdatePrevTime(pTaskCtx);
        BAIL_ON_VMDIR_ERROR(dwError);

        goto cleanup;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwJobInterval  = VMDIR_MAX(60 * 60 * 12, dwJobInterval);
    pTaskCtx->dwPeriod = dwJobInterval;

    dwError = VmDirListFiles(pszReportDir, &pReportFiles);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLinkedListCreate(&pReportsByPartner);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < pReportFiles->dwCount; i++)
    {
        VMDIR_SAFE_FREE_MEMORY(pszReportPath);
        dwError = VmDirAllocateStringPrintf(
                &pszReportPath,
                "%s%s%s",
                pszReportDir,
                VMDIR_PATH_SEP,
                pReportFiles->pStringList[i]);
        BAIL_ON_VMDIR_ERROR(dwError);

        VmDirFreeIntegrityReport(pNewReport);
        pNewReport = NULL;
        dwError = VmDirIntegrityReportCreate(&pNewReport);
        BAIL_ON_VMDIR_ERROR(dwError);

        // if not able to load from file (e.g. not a report file) just ignore
        if (VmDirIntegrityReportLoadFile(pNewReport, pszReportPath) == 0)
        {
            // if previous and current reports are from the same partner, compute the intersect
            pOrgReport = pTail ? (PVMDIR_INTEGRITY_REPORT)pTail->pElement : NULL;
            if (pOrgReport && VmDirStringCompareA(pOrgReport->pszPartner, pNewReport->pszPartner, FALSE) == 0)
            {
                dwError = VmDirIntegrityReportRemoveNonOverlaps(pOrgReport, pNewReport);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            else // otherwise, update the linked list
            {
                dwError = VmDirLinkedListInsertTail(pReportsByPartner, pNewReport, &pTail);
                BAIL_ON_VMDIR_ERROR(dwError);
                pNewReport = NULL;
            }

            // move file to archive directory
            dwError = VmDirStringReplaceAll(pszReportPath, pszReportDir, pszArchiveDir, &pszArchivePath);
            BAIL_ON_VMDIR_ERROR(dwError);

            rename(pszReportPath, pszArchivePath);
            VMDIR_SAFE_FREE_MEMORY(pszArchivePath);
        }
    }

    // iterate final reports and collect counts
    pNode = pReportsByPartner->pHead;
    while (pNode)
    {
        pFnlReport = (PVMDIR_INTEGRITY_REPORT)pNode->pElement;
        if (VmDirReplMetricsCacheFind(pFnlReport->pszPartner, &pReplMetrics) == 0)
        {
            DWORD dwTotalCnt = pFnlReport->dwMismatchCnt + pFnlReport->dwMissingCnt;

            VmMetricsGaugeSet(pReplMetrics->pCountConflictPermanent, dwTotalCnt);

            // persist value in db so it can be retrieved after restart
            (VOID)VmDirReplMetricsPersistCountConflictPermanent(pReplMetrics, dwTotalCnt);
        }
        pNode = pNode->pNext;
    }

    dwError = VmDirBkgdTaskUpdatePrevTime(pTaskCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirStringListFree(pReportFiles);
    VMDIR_SAFE_FREE_MEMORY(pszReportPath);
    VmDirFreeIntegrityReport(pNewReport);
    VmDirFreeIntegrityReportList(pReportsByPartner);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirBkgdCountClosedConnections(
    PVMDIR_BKGD_TASK_CTX    pTaskCtx
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr = NULL;
    PVMDIR_REPLICATION_METRICS  pReplMetrics = NULL;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirGlobals.replAgrsMutex);

    for (pReplAgr = gVmdirReplAgrs; pReplAgr; pReplAgr = pReplAgr->next)
    {
        if (VmDirReplMetricsCacheFind(pReplAgr->pszHostname, &pReplMetrics) == 0)
        {
            if (pReplAgr->dcConn.connState == DC_CONNECTION_STATE_CONNECTED)
            {
                VmMetricsGaugeSet(pReplMetrics->pCountConnectionClosed, 0);
            }
            else
            {
                VmMetricsGaugeSet(pReplMetrics->pCountConnectionClosed, 1);
            }
        }
    }

    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirGlobals.replAgrsMutex);

    dwError = VmDirBkgdTaskUpdatePrevTime(pTaskCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirBkgdPingMaxOrigUsn(
    PVMDIR_BKGD_TASK_CTX    pTaskCtx
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    size_t  iSrvsCnt = 0;
    PSTR*   ppszSrvs = NULL;
    PSTR    pszLocalSrv = NULL;
    PSTR    pszDCAccountUPN = NULL;
    PSTR    pszDCAccountPasswd = NULL;
    PCSTR   pszFilter = "(objectclass=*)";
    LDAP*           pLd = NULL;
    LDAPControl     pingCtl = {0};
    LDAPControl*    pCtrls[2] = {&pingCtl, NULL};
    LDAPMessage*    pResult = NULL;

    if (!pTaskCtx)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    // get localhost server name
    pszLocalSrv = gVmdirServerGlobals.bvServerObjName.lberbv_val;

    // get dc account upn
    pszDCAccountUPN = gVmdirServerGlobals.dcAccountUPN.lberbv_val;

    // get dc account password
    dwError = VmDirReadDCAccountPassword(&pszDCAccountPasswd);
    BAIL_ON_VMDIR_ERROR(dwError);

    // create ping control
    dwError = VmDirCreateStatePingControlContent(&pingCtl);
    BAIL_ON_VMDIR_ERROR(dwError);

    // get list of all servers
    dwError = VmDirGetHostsInternal(&ppszSrvs, &iSrvsCnt);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < iSrvsCnt; i++)
    {
        if (VmDirStringCompareA(ppszSrvs[i], pszLocalSrv, FALSE) == 0)
        {
            // don't ping itself.
            continue;
        }

        // (TODO) either move to REST or make this long lasting connection
        VDIR_SAFE_UNBIND_EXT_S(pLd);
        if (VmDirSafeLDAPBindExt1(
                &pLd,
                ppszSrvs[i],
                pszDCAccountUPN,
                pszDCAccountPasswd,
                gVmdirGlobals.dwLdapConnectTimeoutSec))
        {
            VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL, "failed bind to %s, error (%d)", ppszSrvs[i], dwError);
            continue;
        }

        ldap_msgfree(pResult);
        if (ldap_search_ext_s(
                pLd,
                SERVER_STATE_PING_DN,
                LDAP_SCOPE_BASE,
                pszFilter,
                NULL,
                FALSE,
                pCtrls,
                NULL,
                NULL,
                0,
                &pResult))
        {
            VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL, "failed to send ping to %s, error (%d)", ppszSrvs[i], dwError);
            continue;
        }
    }

    // ping period is 0 - no need to keep track of prev time

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDCAccountPasswd);
    VmDirFreeCtrlContent(&pingCtl);
    VmDirFreeStrArray(ppszSrvs);
    VDIR_SAFE_UNBIND_EXT_S(pLd);
    ldap_msgfree(pResult);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}

DWORD
VmDirBkgdSrvStat(
    PVMDIR_BKGD_TASK_CTX    pTaskCtx
    )
{
    DWORD   dwError = 0;

    if (!pTaskCtx)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirUpdateSrvStat();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirBkgdTaskUpdatePrevTime(pTaskCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}
