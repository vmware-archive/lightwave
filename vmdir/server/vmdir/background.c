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
                VmDirBkgdCreateNewIntegChkReport,
                60 * 60 * 3, // every 3 hours
                "bkgdprevtime_replmetrics_createnewintegchkreport",
                {0}
        },
        {
                VmDirBkgdCompareLastTwoIntegChkReports,
                60 * 60, // every hour
                "bkgdprevtime_replmetrics_comparelasttwointegchkreports",
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
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

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
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

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
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

DWORD
VmDirBkgdCreateNewIntegChkReport(
    PVMDIR_BKGD_TASK_CTX    pTaskCtx
    )
{
    DWORD   dwError = 0;

    if (!pTaskCtx)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    // TODO (phase 4)
//    dwError = VmDirIntegrityCheckStart(INTEGRITY_CHECK_JOB_START);
//    BAIL_ON_VMDIR_ERROR(dwError);

    // NOT updating prev time here - will be updated by integrity check thread

cleanup:
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
VmDirBkgdCompareLastTwoIntegChkReports(
    PVMDIR_BKGD_TASK_CTX    pTaskCtx
    )
{
    DWORD   dwError = 0;

    if (!pTaskCtx)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    /* TODO (phase 4)
    // for each partner
    for (;;)
    {
        // read all reports for partner
        // find entries which appears in all reports

        // collect metrics

        // move all reports to subdir except for the latest one (for next time)
    }
    */

    dwError = VmDirBkgdTaskUpdatePrevTime(pTaskCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
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
        dwError = VmDirSafeLDAPBindExt1(
                &pLd,
                ppszSrvs[i],
                pszDCAccountUPN,
                pszDCAccountPasswd,
                gVmdirGlobals.dwLdapConnectTimeoutSec);

        if (dwError)
        {
            VMDIR_LOG_WARNING(
                    VMDIR_LOG_MASK_ALL,
                    "%s failed bind to %s, error (%d)",
                    __FUNCTION__,
                    ppszSrvs[i],
                    dwError);
            continue;
        }

        ldap_msgfree(pResult);
        dwError = ldap_search_ext_s(
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
                &pResult);

        if (dwError)
        {
            VMDIR_LOG_WARNING(
                    VMDIR_LOG_MASK_ALL,
                    "%s failed to send ping to %s, error (%d)",
                    __FUNCTION__,
                    ppszSrvs[i],
                    dwError);
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
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}
