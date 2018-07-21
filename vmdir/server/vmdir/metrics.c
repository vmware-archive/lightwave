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
_VmDirGetBackupTimeTaken(
    PDWORD  pdwBackupTakenTime
    );

DWORD
VmDirRpcMetricsInit(
    VOID
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;

    // use identical bucket for all histograms
    uint64_t buckets[8] = {50, 100, 250, 500, 1000, 2500, 3000, 4000};

    // use this template to construct labels
    VM_METRICS_LABEL    labels[1] =
    {
            {"operation",   NULL}
    };

    for (i = 0; i < METRICS_RPC_OP_COUNT; i++)
    {
        labels[0].pszValue = VmDirMetricsRpcOperationString(i);

        dwError = VmMetricsHistogramNew(
                pmContext,
                "vmdir_dcerpc",
                labels, 1,
                "Histogram for DCERPC Request Durations for different operations",
                buckets, 8,
                &gpRpcMetrics[i]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
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
VmDirRpcMetricsUpdate(
    METRICS_RPC_OPS operation,
    uint64_t        iStartTime,
    uint64_t        iEndTime
    )
{
    PVM_METRICS_HISTOGRAM   pHistogram = NULL;

    pHistogram = gpRpcMetrics[operation];
    if (pHistogram)
    {
        VmMetricsHistogramUpdate(
                pHistogram, VMDIR_RESPONSE_TIME(iEndTime - iStartTime));
    }
}

VOID
VmDirRpcMetricsShutdown(
    VOID
    )
{
    DWORD   i = 0;

    for (i = 0; i < METRICS_RPC_OP_COUNT; i++)
    {
        gpRpcMetrics[i] = NULL;
    }
}


DWORD
VmDirSrvStatMetricsInit(
    VOID
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PSTR    pszGaugeName[METRICS_SRV_STAT_COUNT] = {
        "vmdir_srv_stat_dbsize",
        "vmdir_srv_stat_backuptimetaken",
        };
    PSTR    pszGaugeInfo[METRICS_SRV_STAT_COUNT] = {
        "Database size in megabytes",
        "Backup time taken in seconds",
        };

    for (i = 0; i < METRICS_SRV_STAT_COUNT; i++)
    {
        dwError = VmMetricsGaugeNew(
                pmContext,
                pszGaugeName[i],
                NULL, 0,
                pszGaugeInfo[i],
                &gpSrvStatMetrics[i]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
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
VmDirSrvStatMetricsUpdate(
    METRICS_SRV_STAT srvStat,
    uint64_t        iValue
    )
{
    PVM_METRICS_GAUGE   pGauge = NULL;

    pGauge = gpSrvStatMetrics[srvStat];
    if (pGauge)
    {
        VmMetricsGaugeSet(pGauge, iValue);
    }
}

VOID
VmDirSrvStatMetricsShutdown(
    VOID
    )
{
    DWORD   i = 0;

    for (i = 0; i < METRICS_SRV_STAT_COUNT; i++)
    {
        gpSrvStatMetrics[i] = NULL;
    }
}


DWORD
VmDirUpdateSrvStat(
    VOID
    )
{
    DWORD   dwError = 0;
    DWORD   dwDBSizeInMB = 0;
    DWORD   dwBackupTakenTime = 0;

    dwError = VmDirGetFileSizeInMB(VMDIR_DB_DIR "/" MDB_MAIN_DB, &dwDBSizeInMB);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSrvStatMetricsUpdate(METRICS_SRV_DBSIZE, dwDBSizeInMB);

    dwError = _VmDirGetBackupTimeTaken(&dwBackupTakenTime);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSrvStatMetricsUpdate(METRICS_SRV_BACKUP_TIMETAKEN, dwBackupTakenTime);

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmDirGetBackupTimeTaken(
    PDWORD  pdwBackupTakenTime
    )
{
    DWORD   dwError = 0;
    DWORD   dwBackupTimeTaken = 0;

    dwError = VmDirGetRegKeyValueDword(
        VMDIR_CONFIG_PARAMETER_V1_KEY_PATH,
        VMDIR_REG_KEY_BACKUP_TIME_TAKEN,
        &dwBackupTimeTaken,
        0);
    if (dwError == LWREG_ERROR_NO_SUCH_KEY_OR_VALUE)
    {
        dwError = 0;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    if (dwBackupTimeTaken)
    {   // backup job set value, report then reset it.
        dwError = VmDirSetRegKeyValueDword(
                    VMDIR_CONFIG_PARAMETER_V1_KEY_PATH,
                    VMDIR_REG_KEY_BACKUP_TIME_TAKEN,
                    0);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *pdwBackupTakenTime = dwBackupTimeTaken;

error:
    return dwError;
}

DWORD
VmDirLwGitHashMetricsInit(
    VOID
    )
{
    DWORD    dwError = 0;
    VM_METRICS_LABEL    labels[1] =
    {
            {"gitCommitHash", GIT_COMMIT_HASH}
    };

    dwError = VmMetricsGaugeNew(
            pmContext,
            "lw_git_commit_hash",
            labels, 1,
            "Lightwave git commit hash",
            &gpLwGitHashMetrics);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

VOID
VmDirLwGitHashMetricsShutdown(
    VOID
    )
{
    gpLwGitHashMetrics = NULL;
}
