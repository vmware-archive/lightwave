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

VOID
LwCARestMetricsUpdate(
    LWCA_METRICS_REQ_URLS       reqUrl,
    LWCA_METRICS_HTTP_METHODS   method,
    LWCA_METRICS_HTTP_CODES     code,
    uint64_t                    iStartTime,
    uint64_t                    iEndTime
    )
{
    PVM_METRICS_HISTOGRAM pHistogram = NULL;

    pHistogram = gpRestMetrics[reqUrl][method][code];
    if (pHistogram)
    {
        VmMetricsHistogramUpdate(pHistogram, LWCA_RESPONSE_TIME(iEndTime - iStartTime));
    }
}

VOID
LwCAApiMetricsUpdate(
    LWCA_METRICS_API_NAMES      api,
    LWCA_METRICS_RESPONSE_CODES code,
    uint64_t                    iStartTime,
    uint64_t                    iEndTime
    )
{
    PVM_METRICS_HISTOGRAM pHistogram = NULL;

    pHistogram = gpApiMetrics[api][code];
    if (pHistogram)
    {
        VmMetricsHistogramUpdate(pHistogram, LWCA_RESPONSE_TIME(iEndTime - iStartTime));
    }
}

VOID
LwCASecurityMetricsUpdate(
    LWCA_METRICS_SECURITY_APIS  api,
    LWCA_METRICS_RESPONSE_CODES code,
    uint64_t                    iStartTime,
    uint64_t                    iEndTime
    )
{
    PVM_METRICS_HISTOGRAM pHistogram = NULL;

    pHistogram = gpSecurityMetrics[api][code];
    if (pHistogram)
    {
        VmMetricsHistogramUpdate(pHistogram, LWCA_RESPONSE_TIME(iEndTime - iStartTime));
    }
}
