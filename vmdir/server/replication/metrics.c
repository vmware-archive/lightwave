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

PVM_METRICS_HISTOGRAM pReplCycleDuration;

DWORD
VmDirReplMetricsInit(
    VOID
    )
{
    DWORD dwError = 0;
    uint64_t buckets[4] = {1, 10, 100, 1000};

    dwError = VmMetricsHistogramNew(pmContext,
                    "vmdir_repl_cycle_duration",
                    NULL, 0,
                    "Histogram for Replication Cycle Duration",
                    buckets, 4,
                    &pReplCycleDuration);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirReplMetricsInit failed (%d)", dwError);

    goto cleanup;
}

DWORD
VmDirReplNewPartnerMetricsInit(
    PVMDIR_REPLICATION_AGREEMENT pReplAgr
    )
{
    DWORD dwError = 0;
    uint64_t buckets[4] = {1, 10, 100, 1000};

    VM_METRICS_LABEL partnerLabel[1] = {{"partner",pReplAgr->ldapURI}};

    dwError = VmMetricsCounterNew(pmContext,
                            "vmdir_repl_unfinished",
                            partnerLabel, 1,
                            "Number of unfinished replication attempts per partner",
                            &pReplAgr->ReplMetrics.pReplUnfinished);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmMetricsGaugeNew(pmContext,
                            "vmdir_repl_high_water_mark",
                            partnerLabel, 1,
                            "The high water mark USN per partner",
                            &pReplAgr->ReplMetrics.pReplUsn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmMetricsCounterNew(pmContext,
                            "vmdir_repl_changes",
                            partnerLabel, 1,
                            "Number of changes applied per partner",
                            &pReplAgr->ReplMetrics.pReplChanges);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmMetricsHistogramNew(pmContext,
                            "vmdir_repl_sync_duration",
                            partnerLabel, 1,
                            "Replication sync duration per partner",
                            buckets, 4,
                            &pReplAgr->ReplMetrics.pReplSyncDuration);
    BAIL_ON_VMDIR_ERROR(dwError);



cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirReplNewPartnerMetricsInit failed (%d)", dwError);

    goto cleanup;
}

DWORD
VmDirReplPartnerMetricsDelete(
    PVMDIR_REPLICATION_AGREEMENT pReplAgr
    )
{
    DWORD dwError = 0;

    dwError = VmMetricsCounterDelete(pmContext, pReplAgr->ReplMetrics.pReplUnfinished);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmMetricsGaugeDelete(pmContext, pReplAgr->ReplMetrics.pReplUsn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmMetricsCounterDelete(pmContext, pReplAgr->ReplMetrics.pReplChanges);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmMetricsHistogramDelete(pmContext, pReplAgr->ReplMetrics.pReplSyncDuration);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirReplPartnerMetricsDelete failed (%d)", dwError);

    goto cleanup;
}
