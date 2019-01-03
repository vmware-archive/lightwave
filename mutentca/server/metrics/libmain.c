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

PVM_METRICS_CONTEXT gpmContext = NULL;

static
DWORD
_LwCARestMetricsInit(
    VOID
    );

static
VOID
_LwCARestMetricsShutdown(
    VOID
    );

DWORD
LwCAMetricsInitialize(
    VOID
    )
{
    DWORD dwError = 0;

    dwError = VmMetricsInit(&gpmContext);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCARestMetricsInit();
    BAIL_ON_LWCA_ERROR(dwError);

cleanup:
    return dwError;

error:
    LWCA_LOG_ERROR("Failed to initialize metrics. Error %d", dwError);

    goto cleanup;
}

VOID
LwCAMetricsShutdown(
    VOID
    )
{
    _LwCARestMetricsShutdown();
    VmMetricsDestroy(gpmContext);
}

static
DWORD
_LwCARestMetricsInit(
    VOID
    )
{
    DWORD dwError = 0;
    DWORD i = 0, j = 0, k = 0;

    uint64_t buckets[8] = {50, 100, 250, 500, 1000, 2500, 3000, 4000};

    VM_METRICS_LABEL labels[3] =
        {
            {"reqUrl", NULL},
            {"method", NULL},
            {"httpStatus", NULL}
        };

    for (i = 0; i < LWCA_METRICS_REQ_URL_COUNT; i++)
    {
        for (j = 0; j < LWCA_METRICS_HTTP_METHOD_COUNT; j++)
        {
            for (k = 0; k < LWCA_METRICS_HTTP_CODE_COUNT; k++)
            {
                labels[0].pszValue = LwCAMetricsReqUrlString(i);
                labels[1].pszValue = LwCAMetricsHttpMethodString(j);
                labels[2].pszValue = LwCAMetricsHttpStatusCodeString(k);

                dwError = VmMetricsHistogramNew(
                            gpmContext,
                            "mutentca_rest_api_duration",
                            labels, 3,
                            "Histogram for REST API request duration",
                            buckets, 8,
                            &gpRestMetrics[i][j][k]);
                BAIL_ON_LWCA_ERROR(dwError);
            }
        }
    }

cleanup:
    return dwError;

error:
    LWCA_LOG_ERROR("Failed to initialize REST API Metrics. Error %d", dwError);

    goto cleanup;
}

static
VOID
_LwCARestMetricsShutdown(
    VOID
    )
{
    DWORD i = 0, j = 0, k = 0;

    for (i = 0; i < LWCA_METRICS_REQ_URL_COUNT; i++)
    {
        for (j = 0; j < LWCA_METRICS_HTTP_METHOD_COUNT; j++)
        {
            for (k = 0; k < LWCA_METRICS_HTTP_CODE_COUNT; k++)
            {
                // Cleanup is done in VmMetricsDestroy
                gpRestMetrics[i][j][k] = NULL;
            }
        }
    }
}
