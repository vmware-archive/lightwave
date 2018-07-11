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
VmMetricsGaugeWithLabelTest()
{
    DWORD dwError = 0;
    PVM_METRICS_CONTEXT pContext = NULL;
    PVM_METRICS_GAUGE pGauge = NULL;
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
        printf("FAIL: Error in MetricsInit for test: GaugeWithLabel, Error Code = %d\n", dwError);
        goto error;
    }

    dwError = VmMetricsGaugeNew(pContext,
                        "gauge_with_label_value",
                        pLabel,
                        2,
                        "Test for gauge with label",
                        &pGauge);
    if (dwError)
    {
        printf("FAIL: Error in GaugeNew for test: GaugeWithLabel, Error Code = %d\n", dwError);
        goto error;
    }

    VmMetricsGaugeSubtract(pGauge, 9);

    VmMetricsGaugeIncrement(pGauge);

    VmMetricsGaugeSet(pGauge, -4);

    VmMetricsGaugeAdd(pGauge, 40);

    VmMetricsGaugeDecrement(pGauge);

    dwError = VmMetricsGetPrometheusData(pContext, &pszData, &dataLen);
    if (dwError)
    {
        printf("FAIL: Error in GetPrometheusData for test: GaugeWithLabel, Error Code = %d\n", dwError);
        goto error;
    }

    pszExpected = "# HELP gauge_with_label_value Test for gauge with label\n" \
                    "# TYPE gauge_with_label_value gauge\n" \
                    "gauge_with_label_value{label1=\"value1\",label2=\"value2\"} 35\n";

    if (strcmp(pszData, pszExpected) == 0)
    {
        printf("PASS: Expected Data received for test: GaugeWithLabel\n");
    }
    else
    {
        printf("FAIL: Unexpected Data received for test: GaugeWithLabel\n");
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
VmMetricsGaugeWithoutLabelTest()
{
    DWORD dwError = 0;
    PVM_METRICS_CONTEXT pContext = NULL;
    PVM_METRICS_GAUGE pGauge = NULL;
    PSTR pszData = NULL;
    PSTR pszExpected = NULL;
    DWORD dataLen = 0;

    dwError = VmMetricsInit(&pContext);
    if (dwError)
    {
        printf("FAIL: Error in MetricsInit for test: GaugeWithoutLabel, Error Code = %d\n", dwError);
        goto error;
    }

    dwError = VmMetricsGaugeNew(pContext,
                        "gauge_without_label_value",
                        NULL,
                        0,
                        "Test for gauge without label",
                        &pGauge);
    if (dwError)
    {
        printf("FAIL: Error in GaugeNew for test: GaugeWithoutLabel, Error Code = %d\n", dwError);
        goto error;
    }

    VmMetricsGaugeSetToCurrentTime(pGauge);

    VmMetricsGaugeSet(pGauge, 10);

    VmMetricsGaugeIncrement(pGauge);

    VmMetricsGaugeSubtract(pGauge, 9);

    VmMetricsGaugeAdd(pGauge, 34);

    VmMetricsGaugeSubtract(pGauge, 9);

    dwError = VmMetricsGetPrometheusData(pContext, &pszData, &dataLen);
    if (dwError)
    {
        printf("FAIL: Error in GetPrometheusData for test: GaugeWithoutLabel, Error Code = %d\n", dwError);
        goto error;
    }

    pszExpected = "# HELP gauge_without_label_value Test for gauge without label\n" \
                    "# TYPE gauge_without_label_value gauge\n" \
                    "gauge_without_label_value 27\n";

    if (strcmp(pszData, pszExpected) == 0)
    {
        printf("PASS: Expected Data received for test: GaugeWithoutLabel\n");
    }
    else
    {
        printf("FAIL: Unexpected Data received for test: GaugeWithoutLabel\n");
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
VmMetricsGaugeWithInvalidLabelTest()
{
    DWORD dwError = 0;
    PVM_METRICS_CONTEXT pContext = NULL;
    PVM_METRICS_GAUGE pGauge = NULL;
    VM_METRICS_LABEL pLabel[2] = {
                                    {"label1",NULL},
                                    {"label2","value2"}
                                 };

    dwError = VmMetricsInit(&pContext);
    if (dwError)
    {
        printf("FAIL: Error in MetricsInit for test: GaugeWithInvalidLabel, Error Code = %d\n", dwError);
        goto error;
    }

    dwError = VmMetricsGaugeNew(pContext,
                        "gauge_with_invalid_label_value",
                        pLabel,
                        2,
                        "Test for gauge with invalid label",
                        &pGauge);
    switch (dwError)
    {
        case VM_COMMON_ERROR_INVALID_PARAMETER:
            printf("PASS: Expected Result for test: GaugeWithInvalidLabel\n");
            dwError = 0;
            break;

        case 0:
            printf("FAIL: Unexpected Result in GaugeNew for test: GaugeWithInvalidLabel, Error Code = %d\n", dwError);
            goto error;
            break;

        default:
            printf("FAIL: Unexpected Error in GaugeNew for test: GaugeWithInvalidLabel, Error Code = %d\n", dwError);
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
VmMetricsGaugeTest()
{
    DWORD dwError = 0;

    dwError = VmMetricsGaugeWithLabelTest();
    dwError += VmMetricsGaugeWithoutLabelTest();
    dwError += VmMetricsGaugeWithInvalidLabelTest();

    return dwError;
}
