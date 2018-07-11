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

static
DWORD
VmMetricsHistogramWithLabelTest()
{
    DWORD dwError = 0;
    PVM_METRICS_CONTEXT pContext = NULL;
    PVM_METRICS_HISTOGRAM pHistogram = NULL;
    PSTR pszData = NULL;
    PSTR pszExpected = NULL;
    DWORD dataLen = 0;

    VM_METRICS_LABEL pLabel[2] = {
                                    {"label1","value1"},
                                    {"label2","value2"}
                                 };

    uint64_t buckets[] = {1, 5, 10};

    dwError = VmMetricsInit(&pContext);
    if (dwError)
    {
        printf("FAIL: Error in MetricsInit for test: HistogramWithLabel, Error Code = %d\n", dwError);
        goto error;
    }

    dwError = VmMetricsHistogramNew(pContext,
                        "histogram_with_label_duration_milliseconds",
                        pLabel,
                        2,
                        "Test for histogram with label",
                        buckets,
                        3,
                        &pHistogram);
    if (dwError)
    {
        printf("FAIL: Error in HistogramNew for test: HistogramWithLabel, Error Code = %d\n", dwError);
        goto error;
    }

    VmMetricsHistogramUpdate(pHistogram, 3);

    VmMetricsHistogramUpdate(pHistogram, 7);

    VmMetricsHistogramUpdate(pHistogram, 2);

    dwError = VmMetricsGetPrometheusData(pContext, &pszData, &dataLen);
    if (dwError)
    {
        printf("FAIL: Error in GetPrometheusData for test: HistogramWithLabel, Error Code = %d\n", dwError);
        goto error;
    }

    pszExpected = "# HELP histogram_with_label_duration_milliseconds Test for histogram with label\n" \
                    "# TYPE histogram_with_label_duration_milliseconds histogram\n" \
                    "histogram_with_label_duration_milliseconds_bucket{le=\"1\",label1=\"value1\",label2=\"value2\"} 0\n" \
                    "histogram_with_label_duration_milliseconds_bucket{le=\"5\",label1=\"value1\",label2=\"value2\"} 2\n" \
                    "histogram_with_label_duration_milliseconds_bucket{le=\"10\",label1=\"value1\",label2=\"value2\"} 3\n" \
                    "histogram_with_label_duration_milliseconds_bucket{le=\"+Inf\",label1=\"value1\",label2=\"value2\"} 3\n" \
                    "histogram_with_label_duration_milliseconds_count{label1=\"value1\",label2=\"value2\"} 3\n" \
                    "histogram_with_label_duration_milliseconds_sum{label1=\"value1\",label2=\"value2\"} 12\n";

    if (strcmp(pszData, pszExpected) == 0)
    {
        printf("PASS: Expected Data received for test: HistogramWithLabel\n");
    }
    else
    {
        printf("FAIL: Unexpected Data received for test: HistogramWithLabel\n");
        goto error;
    }

cleanup:
    VM_COMMON_SAFE_FREE_MEMORY(pszData);
    VmMetricsDestroy(pContext);
    return dwError;

error:
    dwError = 1;
    goto cleanup;
}

static
DWORD
VmMetricsHistogramWithoutLabelTest()
{
    DWORD dwError = 0;
    PVM_METRICS_CONTEXT pContext = NULL;
    PVM_METRICS_HISTOGRAM pHistogram = NULL;
    PSTR pszData = NULL;
    PSTR pszExpected = NULL;
    DWORD dataLen = 0;

    uint64_t buckets[] = {1, 10, 100};

    dwError = VmMetricsInit(&pContext);
    if (dwError)
    {
        printf("FAIL: Error in MetricsInit for test: HistogramWithoutLabel, Error Code = %d\n", dwError);
        goto error;
    }

    dwError = VmMetricsHistogramNew(pContext,
                        "histogram_without_label_duration_milliseconds",
                        NULL,
                        0,
                        "Test for histogram without label",
                        buckets,
                        3,
                        &pHistogram);
    if (dwError)
    {
        printf("FAIL: Error in HistogramNew for test: HistogramWithoutLabel, Error Code = %d\n", dwError);
        goto error;
    }

    VmMetricsHistogramUpdate(pHistogram, 1);

    VmMetricsHistogramUpdate(pHistogram, 123);

    VmMetricsHistogramUpdate(pHistogram, 20);

    dwError = VmMetricsGetPrometheusData(pContext, &pszData, &dataLen);
    if (dwError)
    {
        printf("FAIL: Error in GetPrometheusData for test: HistogramWithoutLabel, Error Code = %d\n", dwError);
        goto error;
    }

    pszExpected = "# HELP histogram_without_label_duration_milliseconds Test for histogram without label\n" \
                    "# TYPE histogram_without_label_duration_milliseconds histogram\n" \
                    "histogram_without_label_duration_milliseconds_bucket{le=\"1\"} 1\n" \
                    "histogram_without_label_duration_milliseconds_bucket{le=\"10\"} 1\n" \
                    "histogram_without_label_duration_milliseconds_bucket{le=\"100\"} 2\n" \
                    "histogram_without_label_duration_milliseconds_bucket{le=\"+Inf\"} 3\n" \
                    "histogram_without_label_duration_milliseconds_count 3\n" \
                    "histogram_without_label_duration_milliseconds_sum 144\n";

    if (strcmp(pszData, pszExpected) == 0)
    {
        printf("PASS: Expected Data received for test: HistogramWithoutLabel\n");
    }
    else
    {
        printf("FAIL: Unexpected Data received for test: HistogramWithoutLabel\n");
        goto error;
    }

cleanup:
    VM_COMMON_SAFE_FREE_MEMORY(pszData);
    VmMetricsDestroy(pContext);

    return dwError;

error:
    dwError = 1;
    goto cleanup;
}

static
DWORD
VmMetricsHistogramWithInvalidLabelTest()
{
    DWORD dwError = 0;
    PVM_METRICS_CONTEXT pContext = NULL;
    PVM_METRICS_HISTOGRAM pHistogram = NULL;
    VM_METRICS_LABEL pLabel[2] = {
                                    {"label1","value1"},
                                    {NULL,"value2"}
                                 };

    uint64_t buckets[] = {1, 10, 100};

    dwError = VmMetricsInit(&pContext);
    if (dwError)
    {
        printf("FAIL: Error in MetricsInit for test: HistogramWithInvalidLabel, Error Code = %d\n", dwError);
        goto error;
    }

    dwError = VmMetricsHistogramNew(pContext,
                        "histogram_with_invalid_label_duration_milliseconds",
                        pLabel,
                        2,
                        "Test for histogram with invalid label",
                        buckets,
                        3,
                        &pHistogram);
    switch (dwError)
    {
        case VM_COMMON_ERROR_INVALID_PARAMETER:
            printf("PASS: Expected Result for test: HistogramWithInvalidLabel\n");
            dwError = 0;
            break;

        case 0:
            printf("FAIL: Unexpected Result in HistogramNew for test: HistogramWithInvalidLabel, Error Code = %d\n", dwError);
            goto error;
            break;

        default:
            printf("FAIL: Unexpected Error in HistogramNew for test: HistogramWithInvalidLabel, Error Code = %d\n", dwError);
            goto error;
            break;
    }

cleanup:
    VmMetricsDestroy(pContext);
    return dwError;

error:
    dwError = 1;
    goto cleanup;
}

DWORD
VmMetricsHistogramTest()
{
    DWORD dwError = 0;

    dwError = VmMetricsHistogramWithLabelTest();
    dwError += VmMetricsHistogramWithoutLabelTest();
    dwError += VmMetricsHistogramWithInvalidLabelTest();

    return dwError;
}
