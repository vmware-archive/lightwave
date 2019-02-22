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

static uint64_t buckets[8] = {50, 100, 250, 500, 1000, 2500, 3000, 4000};

static
DWORD
_LwCARestMetricsInit(
    VOID
    );

static
DWORD
_LwCAApiMetricsInit(
    VOID
    );

static
DWORD
_LwCASecurityMetricsInit(
    VOID
    );

static
DWORD
_LwCADbMetricsInit(
    VOID
    );

static
VOID
_LwCARestMetricsShutdown(
    VOID
    );

static
VOID
_LwCAApiMetricsShutdown(
    VOID
    );

static
VOID
_LwCASecurityMetricsShutdown(
    VOID
    );

static
VOID
_LwCADbMetricsShutdown(
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

    dwError = _LwCAApiMetricsInit();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCASecurityMetricsInit();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = _LwCADbMetricsInit();
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
    _LwCAApiMetricsShutdown();
    _LwCASecurityMetricsShutdown();
    _LwCADbMetricsShutdown();
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
DWORD
_LwCAApiMetricsInit(
    VOID
    )
{
    DWORD dwError = 0;
    DWORD i = 0, j = 0;

    VM_METRICS_LABEL labels[2] =
        {
            {"api", NULL},
            {"response", NULL}
        };

    for (i = 0; i < LWCA_METRICS_API_COUNT; i++)
    {
        for (j = 0; j < LWCA_METRICS_RESPONSE_COUNT; j++)
        {
            labels[0].pszValue = LwCAMetricsApiNameString(i);
            labels[1].pszValue = LwCAMetricsResponseString(j);

            dwError = VmMetricsHistogramNew(
                        gpmContext,
                        "mutentca_api_duration",
                        labels, 2,
                        "Histogram for Backend API request duration",
                        buckets, 8,
                        &gpApiMetrics[i][j]);
            BAIL_ON_LWCA_ERROR(dwError);
        }
    }

cleanup:
    return dwError;

error:
    LWCA_LOG_ERROR("Failed to initialize Backend API Metrics. Error %d", dwError);

    goto cleanup;
}

static
DWORD
_LwCASecurityMetricsInit(
    VOID
    )
{
    DWORD dwError = 0;
    DWORD i = 0, j = 0;

    VM_METRICS_LABEL labels[2] =
        {
            {"api", NULL},
            {"response", NULL}
        };

    for (i = 0; i < LWCA_METRICS_SECURITY_COUNT; i++)
    {
        for (j = 0; j < LWCA_METRICS_RESPONSE_COUNT; j++)
        {
            labels[0].pszValue = LwCAMetricsSecurityApiNameString(i);
            labels[1].pszValue = LwCAMetricsResponseString(j);

            dwError = VmMetricsHistogramNew(
                        gpmContext,
                        "mutentca_security_api_duration",
                        labels, 2,
                        "Histogram for Security API request duration",
                        buckets, 8,
                        &gpSecurityMetrics[i][j]);
            BAIL_ON_LWCA_ERROR(dwError);
        }
    }

cleanup:
    return dwError;

error:
    LWCA_LOG_ERROR("Failed to initialize Security API Metrics. Error %d", dwError);

    goto cleanup;
}


static
DWORD
_LwCADbMetricsInit(
    VOID
    )
{
    DWORD dwError = 0;
    DWORD i = 0, j = 0;

    VM_METRICS_LABEL labels[2] =
        {
            {"api", NULL},
            {"response", NULL}
        };

    for (i = 0; i < LWCA_METRICS_DB_COUNT; i++)
    {
        for (j = 0; j < LWCA_METRICS_RESPONSE_COUNT; j++)
        {
            labels[0].pszValue = LwCAMetricsDbApiNameString(i);
            labels[1].pszValue = LwCAMetricsResponseString(j);

            dwError = VmMetricsHistogramNew(
                        gpmContext,
                        "mutentca_db_api_duration",
                        labels, 2,
                        "Histogram for DB Plugin API request duration",
                        buckets, 8,
                        &gpDbMetrics[i][j]);
            BAIL_ON_LWCA_ERROR(dwError);
        }
    }

cleanup:
    return dwError;

error:
    LWCA_LOG_ERROR("Failed to initialize DB Plugin API Metrics. Error %d", dwError);

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

static
VOID
_LwCAApiMetricsShutdown(
    VOID
    )
{
    DWORD i = 0, j = 0;

    for (i = 0; i < LWCA_METRICS_API_COUNT; i++)
    {
        for (j = 0; j < LWCA_METRICS_RESPONSE_COUNT; j++)
        {
            // Cleanup is done in VmMetricsDestroy
            gpApiMetrics[i][j] = NULL;
        }
    }
}

static
VOID
_LwCADbMetricsShutdown(
    VOID
    )
{
    DWORD i = 0, j = 0;

    for (i = 0; i < LWCA_METRICS_DB_COUNT; i++)
    {
        for (j = 0; j < LWCA_METRICS_RESPONSE_COUNT; j++)
        {
            // Cleanup is done in VmMetricsDestroy
            gpDbMetrics[i][j] = NULL;
        }
    }
}
static
VOID
_LwCASecurityMetricsShutdown(
    VOID
    )
{
    DWORD i = 0, j = 0;

    for (i = 0; i < LWCA_METRICS_SECURITY_COUNT; i++)
    {
        for (j = 0; j < LWCA_METRICS_RESPONSE_COUNT; j++)
        {
            // Cleanup is done in VmMetricsDestroy
            gpSecurityMetrics[i][j] = NULL;
        }
    }
}
