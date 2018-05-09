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
                "post_dcerpc",
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
                pHistogram, VMDIR_RESPONSE_TIME(iStartTime, iEndTime));
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
