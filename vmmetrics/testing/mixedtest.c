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
#include "vmmetrics.h"

DWORD
VmMetricsMixedTest()
{
    DWORD dwError = 0;

    PVM_METRICS_CONTEXT pContext = NULL;
    PVM_METRICS_HISTOGRAM pHistogram1 = NULL;
    PVM_METRICS_COUNTER pCounter = NULL;
    PVM_METRICS_GAUGE pGauge = NULL;
    PVM_METRICS_HISTOGRAM pHistogram2 = NULL;

    PSTR pszData = NULL;
    PSTR pszExpected = NULL;
    DWORD dataLen = 0;

    VM_METRICS_LABEL pLabel[2] = {
                                    {"label1","value1"},
                                    {"label2","value2"}
                                 };

    uint64_t buckets1[] = {1, 5, 10};
    uint64_t buckets2[] = {1, 10, 100};

    dwError = VmMetricsInit(&pContext);
    if (dwError)
    {
        printf("FAIL: Error in MetricsInit for test: MixedTest, Error Code = %d\n", dwError);
        goto error;
    }

    dwError = VmMetricsHistogramNew(pContext,
                        "histogram1_duration_milliseconds",
                        pLabel,
                        2,
                        "Mixed Test histogram 1",
                        buckets1,
                        3,
                        &pHistogram1);
    if (dwError)
    {
        printf("FAIL: Error in HistogramNew for test: MixedTest, Error Code = %d\n", dwError);
        goto error;
    }

    dwError = VmMetricsCounterNew(pContext,
                        "counter1_count",
                        NULL,
                        0,
                        "Mixed Test counter",
                        &pCounter);
    if (dwError)
    {
        printf("FAIL: Error in CounterNew for test: MixedTest, Error Code = %d\n", dwError);
        goto error;
    }

    dwError = VmMetricsGaugeNew(pContext,
                        "gauge1_value",
                        NULL,
                        0,
                        "Mixed Test gauge",
                        &pGauge);
    if (dwError)
    {
        printf("FAIL: Error in GaugeNew for test: MixedTest, Error Code = %d\n", dwError);
        goto error;
    }

    VmMetricsHistogramUpdate(pHistogram1, 3);

    VmMetricsGaugeSet(pGauge, -4);

    VmMetricsGaugeIncrement(pGauge);

    dwError = VmMetricsHistogramNew(pContext,
                        "histogram2_duration_milliseconds",
                        NULL,
                        0,
                        "Mixed Test histogram 2",
                        buckets2,
                        3,
                        &pHistogram2);
    if (dwError)
    {
        printf("FAIL: Error in HistogramNew for test: MixedTest, Error Code = %d\n", dwError);
        goto error;
    }

    VmMetricsHistogramUpdate(pHistogram2, 34);

    VmMetricsCounterIncrement(pCounter);

    VmMetricsGaugeAdd(pGauge, 5);

    VmMetricsHistogramUpdate(pHistogram2, 2);

    VmMetricsHistogramUpdate(pHistogram1, 20);

    dwError = VmMetricsGetPrometheusData(pContext, &pszData, &dataLen);
    if (dwError)
    {
        printf("FAIL: Error in GetPrometheusData for test: MixedTest, Error Code = %d\n", dwError);
        goto error;
    }

    pszExpected = "# HELP histogram2_duration_milliseconds Mixed Test histogram 2\n" \
                    "# TYPE histogram2_duration_milliseconds histogram\n" \
                    "histogram2_duration_milliseconds_bucket{le=\"1\"} 0\n" \
                    "histogram2_duration_milliseconds_bucket{le=\"10\"} 1\n" \
                    "histogram2_duration_milliseconds_bucket{le=\"100\"} 2\n" \
                    "histogram2_duration_milliseconds_bucket{le=\"+Inf\"} 2\n" \
                    "histogram2_duration_milliseconds_count 2\n" \
                    "histogram2_duration_milliseconds_sum 36\n" \
                    "# HELP gauge1_value Mixed Test gauge\n" \
                    "# TYPE gauge1_value gauge\n" \
                    "gauge1_value 2\n" \
                    "# HELP counter1_count Mixed Test counter\n" \
                    "# TYPE counter1_count counter\n" \
                    "counter1_count 1\n" \
                    "# HELP histogram1_duration_milliseconds Mixed Test histogram 1\n" \
                    "# TYPE histogram1_duration_milliseconds histogram\n" \
                    "histogram1_duration_milliseconds_bucket{le=\"1\",label1=\"value1\",label2=\"value2\"} 0\n" \
                    "histogram1_duration_milliseconds_bucket{le=\"5\",label1=\"value1\",label2=\"value2\"} 1\n" \
                    "histogram1_duration_milliseconds_bucket{le=\"10\",label1=\"value1\",label2=\"value2\"} 1\n" \
                    "histogram1_duration_milliseconds_bucket{le=\"+Inf\",label1=\"value1\",label2=\"value2\"} 2\n" \
                    "histogram1_duration_milliseconds_count{label1=\"value1\",label2=\"value2\"} 2\n" \
                    "histogram1_duration_milliseconds_sum{label1=\"value1\",label2=\"value2\"} 23\n";

    if (strcmp(pszData, pszExpected) == 0)
    {
        printf("PASS: Expected Data received for test: MixedTest\n");
    }
    else
    {
        printf("FAIL: Unexpected Data received for test: MixedTest\n");
        goto error;
    }

cleanup:
    VmMetricsFreePrometheusData(pszData);
    VmMetricsDestroy(pContext);

    return dwError;

error:
    dwError = 1;
    goto cleanup;
}
