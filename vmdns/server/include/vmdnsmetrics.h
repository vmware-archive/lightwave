/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include <vmmetrics.h>

typedef enum
{
    /*DNS Metrics - Error Count*/
    DNS_ERROR_NXDOMAIN_ERR_COUNT,
    DNS_ERROR_NOT_IMPLEMENTED_COUNT,
    DNS_ERROR_UNKNOWN_COUNT,
    DNS_NO_ERROR,

    /*DNS Metrics - Cache Lookup,Miss,Purge Counts*/
    CACHE_ZONE_LOOKUP,
    CACHE_ZONE_MISS,
    CACHE_CACHE_LOOKUP,
    CACHE_CACHE_MISS,
    CACHE_MODIFY_PURGE_COUNT,
    CACHE_LRU_PURGE_COUNT,
    CACHE_NOTIFY_PURGE_COUNT,

    VDNS_COUNTER_COUNT,

} VDNS_COUNTER_METRICS;

typedef enum
{
    /*DNS Protocol - Query,Update Duration*/
    DNS_QUERY_DURATION,
    DNS_UPDATE_DURATION,

    /*STORE - Query,Update Duration*/
    STORE_QUERY_DURATION,
    STORE_UPDATE_DURATION,

    /*RPC Protocol - Query,Update Duration*/
    RPC_QUERY_DURATION,
    RPC_UPDATE_DURATION,

    VDNS_HISTOGRAM_COUNT

} VDNS_HISTOGRAM_METRICS;

typedef enum
{
    DNS_OUTSTANDING_REQUEST_COUNT,
    DNS_ACTIVE_SERVICE_THREADS,

    CACHE_OBJECT_COUNT,

    VDNS_GAUGE_COUNT,

} VDNS_GAUGE_METRICS;

#define VDNS_RESPONSE_TIME(val) ((val) ? (val) : 1)

extern PVM_METRICS_COUNTER gVmDnsCounterMetrics[VDNS_COUNTER_COUNT];
extern PVM_METRICS_HISTOGRAM gVmDnsHistogramMetrics[VDNS_HISTOGRAM_COUNT];
extern PVM_METRICS_GAUGE gVmDnsGaugeMetrics[VDNS_GAUGE_COUNT];
extern PVM_METRICS_CONTEXT gVmDnsMetricsContext;

