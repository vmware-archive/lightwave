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

#ifndef VM_METRICS_H_
#define VM_METRICS_H_

/* Metrics context identifier */
typedef struct _VM_METRICS_CONTEXT VM_METRICS_CONTEXT, *PVM_METRICS_CONTEXT;

/* Counter metrics identifier */
typedef struct _VM_METRICS_COUNTER VM_METRICS_COUNTER, *PVM_METRICS_COUNTER;

/* Gauge metrics identifier */
typedef struct _VM_METRICS_GAUGE VM_METRICS_GAUGE, *PVM_METRICS_GAUGE;

/* Histogram metrics identifier */
typedef struct _VM_METRICS_HISTOGRAM VM_METRICS_HISTOGRAM, *PVM_METRICS_HISTOGRAM;

/*
 * Structure for Label
 */
typedef struct _VM_METRICS_LABEL
{
    PSTR pszKey;
    PSTR pszValue;
} VM_METRICS_LABEL, *PVM_METRICS_LABEL;

/*
 * Initialize the metrics context
 */
DWORD
VmMetricsInit(
    PVM_METRICS_CONTEXT* ppContext
    );

/*
 * Add a new counter metric
 */
DWORD
VmMetricsCounterNew(
    PVM_METRICS_CONTEXT pContext,
    PCSTR pszName,
    const PVM_METRICS_LABEL pLabel,
    const DWORD iLabelCount,
    PCSTR pszDescription,
    PVM_METRICS_COUNTER* ppCounter
    );

/*
 * Add a new gauge metric
 */
DWORD
VmMetricsGaugeNew(
    PVM_METRICS_CONTEXT pContext,
    PCSTR pszName,
    const PVM_METRICS_LABEL pLabel,
    const DWORD iLabelCount,
    PCSTR pszDescription,
    PVM_METRICS_GAUGE* ppGauge
    );

/*
 * Add a new histogram metric
 */
DWORD
VmMetricsHistogramNew(
    PVM_METRICS_CONTEXT pContext,
    PCSTR pszName,
    const PVM_METRICS_LABEL pLabel,
    const DWORD iLabelCount,
    PCSTR pszDescription,
    const uint64_t iBuckets[],
    const DWORD iBucketSize,
    PVM_METRICS_HISTOGRAM* ppHistogram
    );

/*
 * Increment the counter by 1
 */
VOID
VmMetricsCounterIncrement(
    PVM_METRICS_COUNTER pCounter
    );

/*
 * Add the value to the counter
 */
VOID
VmMetricsCounterAdd(
    PVM_METRICS_COUNTER pCounter,
    uint64_t value
    );

/*
 * Set the value of the gague
 */
VOID
VmMetricsGaugeSet(
    PVM_METRICS_GAUGE pGauge,
    int64_t value
    );

/*
 * Increment the gauge by 1
 */
VOID
VmMetricsGaugeIncrement(
    PVM_METRICS_GAUGE pGauge
    );

/*
 * Decrement the gauge by 1
 */
VOID
VmMetricsGaugeDecrement(
    PVM_METRICS_GAUGE pGauge
    );

/*
 * Add the value to the gauge
 */
VOID
VmMetricsGaugeAdd(
    PVM_METRICS_GAUGE pGauge,
    uint64_t value
    );

/*
 * Subtract the value from the gauge
 */
VOID
VmMetricsGaugeSubtract(
    PVM_METRICS_GAUGE pGauge,
    uint64_t value
    );

/*
 * Set the value of the gauge to the current Unix time in secondss
 */
VOID
VmMetricsGaugeSetToCurrentTime(
    PVM_METRICS_GAUGE pGauge
    );

/*
 * Add the given value to the histogram
 */
VOID
VmMetricsHistogramUpdate(
    PVM_METRICS_HISTOGRAM pHistogram,
    uint64_t value
    );

/*
 * Return the list of all metrics data in Prometheus format (string/text)
 */
DWORD
VmMetricsGetPrometheusData(
    PVM_METRICS_CONTEXT pContext,
    PSTR* ppszData,
    DWORD* pDataLen
    );

/*
 * Free the data returned by VmMetricsGetPrometheusData()
 */
VOID
VmMetricsFreePrometheusData(
    PSTR pszData
    );

/*
 * Delete a counter metric
 */
DWORD
VmMetricsCounterDelete(
    PVM_METRICS_CONTEXT pContext,
    PVM_METRICS_COUNTER pCounter
    );

/*
 * Delete a gauge metric
 */
DWORD
VmMetricsGaugeDelete(
    PVM_METRICS_CONTEXT pContext,
    PVM_METRICS_GAUGE pGauge
    );

/*
 * Delete a histogram metric
 */
DWORD
VmMetricsHistogramDelete(
    PVM_METRICS_CONTEXT pContext,
    PVM_METRICS_HISTOGRAM pHistogram
    );

/*
 * Destroy the metrics context structure
 */
VOID
VmMetricsDestroy(
    PVM_METRICS_CONTEXT pContext
    );

#endif /* VM_METRICS_H_ */
