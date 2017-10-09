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
VmMetricsCounterWithLabelTest()
{
    DWORD dwError = 0;
    PVM_METRICS_CONTEXT pContext = NULL;
    PVM_METRICS_COUNTER pCounter = NULL;
    PSTR pszData = NULL;
    PSTR pszExpected = NULL;
    DWORD dataLen = 0;

    VM_METRICS_LABEL pLabel[2] = {
                                    {"label1","value1"},
                                    {"label2","value2"}
                                 };

    dwError = VmMetricsInit(&pContext);
    if (dwError)
    {
        printf("FAIL: Error in MetricsInit for test: CounterWithLabel, Error Code = %d\n", dwError);
        goto error;
    }

    dwError = VmMetricsCounterNew(pContext,
                        "counter_with_label_count",
                        pLabel,
                        2,
                        "Test for counter with label",
                        &pCounter);
    if (dwError)
    {
        printf("FAIL: Error in CounterNew for test: CounterWithLabel, Error Code = %d\n", dwError);
        goto error;
    }

    VmMetricsCounterIncrement(pCounter);

    VmMetricsCounterAdd(pCounter, 34);

    dwError = VmMetricsGetPrometheusData(pContext, &pszData, &dataLen);
    if (dwError)
    {
        printf("FAIL: Error in GetPrometheusData for test: CounterWithLabel, Error Code = %d\n", dwError);
        goto error;
    }

    pszExpected = "# HELP counter_with_label_count Test for counter with label\n" \
                    "# TYPE counter_with_label_count counter\n" \
                    "counter_with_label_count{label1=\"value1\",label2=\"value2\"} 35\n";

    if (strcmp(pszData, pszExpected) == 0)
    {
        printf("PASS: Expected Data received for test: CounterWithLabel\n");
    }
    else
    {
        printf("FAIL: Unexpected Data received for test: CounterWithLabel\n");
        goto error;
    }

cleanup:
    VM_METRICS_SAFE_FREE_MEMORY(pszData);
    VmMetricsDestroy(pContext);
    return dwError;

error:
    dwError = 1;
    goto cleanup;
}

static
DWORD
VmMetricsCounterWithoutLabelTest()
{
    DWORD dwError = 0;
    PVM_METRICS_CONTEXT pContext = NULL;
    PVM_METRICS_COUNTER pCounter = NULL;
    PSTR pszData = NULL;
    PSTR pszExpected = NULL;
    DWORD dataLen = 0;

    dwError = VmMetricsInit(&pContext);
    if (dwError)
    {
        printf("FAIL: Error in MetricsInit for test: CounterWithoutLabel, Error Code = %d\n", dwError);
        goto error;
    }

    dwError = VmMetricsCounterNew(pContext,
                        "counter_without_label_count",
                        NULL,
                        0,
                        "Test for counter without label",
                        &pCounter);
    if (dwError)
    {
        printf("FAIL: Error in CounterNew for test: CounterWithoutLabel, Error Code = %d\n", dwError);
        goto error;
    }

    VmMetricsCounterAdd(pCounter, 23);

    VmMetricsCounterIncrement(pCounter);

    VmMetricsGetPrometheusData(pContext, &pszData, &dataLen);
    if (dwError)
    {
        printf("FAIL: Error in GetPrometheusData for test: CounterWithoutLabel, Error Code = %d\n", dwError);
        goto error;
    }

    pszExpected = "# HELP counter_without_label_count Test for counter without label\n" \
                    "# TYPE counter_without_label_count counter\n" \
                    "counter_without_label_count 24\n";

    if (strcmp(pszData, pszExpected) == 0)
    {
        printf("PASS: Expected Data received for test: CounterWithoutLabel\n");
    }
    else
    {
        printf("FAIL: Unexpected Data received for test: CounterWithoutLabel\n");
        goto error;
    }

cleanup:
    VM_METRICS_SAFE_FREE_MEMORY(pszData);
    VmMetricsDestroy(pContext);

    return dwError;

error:
    dwError = 1;
    goto cleanup;
}

static
DWORD
VmMetricsCounterWithInvalidLabelTest()
{
    DWORD dwError = 0;
    PVM_METRICS_CONTEXT pContext = NULL;
    PVM_METRICS_COUNTER pCounter = NULL;
    VM_METRICS_LABEL pLabel[2] = {
                                    {"label1","value1"},
                                    {"label2",NULL}
                                 };

    dwError = VmMetricsInit(&pContext);
    if (dwError)
    {
        printf("FAIL: Error in MetricsInit for test: CounterWithInvalidLabel, Error Code = %d\n", dwError);
        goto error;
    }

    dwError = VmMetricsCounterNew(pContext,
                        "counter_with_invalid_label_count",
                        pLabel,
                        2,
                        "Test for counter with invalid label",
                        &pCounter);
    switch (dwError)
    {
        case VM_METRICS_ERROR_INVALID_PARAMETER:
            printf("PASS: Expected Result for test: CounterWithInvalidLabel\n");
            dwError = 0;
            break;

        case 0:
            printf("FAIL: Unexpected Result in CounterNew for test: CounterWithInvalidLabel, Error Code = %d\n", dwError);
            goto error;
            break;

        default:
            printf("FAIL: Unexpected Error in CounterNew for test: CounterWithInvalidLabel, Error Code = %d\n", dwError);
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
VmMetricsCounterTest()
{
    DWORD dwError = 0;

    dwError = VmMetricsCounterWithLabelTest();
    dwError += VmMetricsCounterWithoutLabelTest();
    dwError += VmMetricsCounterWithInvalidLabelTest();

    return dwError;
}
