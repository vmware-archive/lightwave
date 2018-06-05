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

DWORD
VmDirLdapMetricsInit(
    VOID
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0, j = 0, k = 0;

    // use identical bucket for all histograms
    uint64_t buckets[8] = {50, 100, 250, 500, 1000, 2500, 3000, 4000};

    // use this template to construct labels
    VM_METRICS_LABEL    labels[3] =
    {
            {"operation",   NULL},
            {"op_type",     NULL},
            {"code",        NULL}
    };

    for (i = 0; i < METRICS_LDAP_OP_COUNT; i++)
    {
        for (j = 0; j < METRICS_LDAP_OP_TYPE_COUNT; j++)
        {
            for (k = 0; k < METRICS_LDAP_ERROR_COUNT; k++)
            {
                labels[0].pszValue = VmDirMetricsLdapOperationString(i);
                labels[1].pszValue = VmDirMetricsLdapOpTypeString(j);
                labels[2].pszValue = VmDirMetricsLdapErrorString(k);

                dwError = VmMetricsHistogramNew(
                        pmContext,
                        "vmdir_ldap",
                        labels, 3,
                        "Histogram for LDAP request durations in total",
                        buckets, 8,
                        &gpLdapMetrics[i][j][k][METRICS_LAYER_PROTOCOL]);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = VmMetricsHistogramNew(
                        pmContext,
                        "vmdir_ldap_middlelayer",
                        labels, 3,
                        "Histogram for LDAP request durations in the middle layer",
                        buckets, 8,
                        &gpLdapMetrics[i][j][k][METRICS_LAYER_MIDDLELAYER]);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = VmMetricsHistogramNew(
                        pmContext,
                        "vmdir_ldap_middlelayer_backend",
                        labels, 3,
                        "Histogram for LDAP request durations in the backend",
                        buckets, 8,
                        &gpLdapMetrics[i][j][k][METRICS_LAYER_BACKEND]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
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
VmDirLdapMetricsUpdate(
    METRICS_LDAP_OPS        operation,
    METRICS_LDAP_OP_TYPES   opType,
    METRICS_LDAP_ERRORS     error,
    METRICS_LAYERS          layer,
    uint64_t                iStartTime,
    uint64_t                iEndTime
    )
{
    PVM_METRICS_HISTOGRAM   pHistogram = NULL;

    pHistogram = gpLdapMetrics[operation][opType][error][layer];
    if (pHistogram)
    {
        VmMetricsHistogramUpdate(
                pHistogram, VMDIR_RESPONSE_TIME(iEndTime - iStartTime));
    }
}

VOID
VmDirLdapMetricsShutdown(
    VOID
    )
{
    DWORD   i = 0, j = 0, k = 0, l = 0;

    for (i = 0; i < METRICS_LDAP_OP_COUNT; i++)
    {
        for (j = 0; j < METRICS_LDAP_OP_TYPE_COUNT; j++)
        {
            for (k = 0; k < METRICS_LDAP_ERROR_COUNT; k++)
            {
                for (l = 0; l < METRICS_LAYER_COUNT; l++)
                {
                    gpLdapMetrics[i][j][k][l] = NULL;
                }
            }
        }
    }
}
