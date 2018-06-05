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
    /*DNS Metrics - Error Count using rcode*/
    METRICS_VDNS_RCODE_NOERROR,
    METRICS_VDNS_RCODE_FORMAT_ERROR,
    METRICS_VDNS_RCODE_SERVER_FAILURE,
    METRICS_VDNS_RCODE_NAME_ERROR,
    METRICS_VDNS_RCODE_NOT_IMPLEMENTED,
    METRICS_VDNS_RCODE_REFUSED,
    METRICS_VDNS_RCODE_YXDOMAIN,
    METRICS_VDNS_RCODE_YXRRSET,
    METRICS_VDNS_RCODE_NXRRSET,
    METRICS_VDNS_RCODE_NOTAUTH,
    METRICS_VDNS_RCODE_NOTZONE,
    METRICS_VDNS_RCODE_BADSIG,
    METRICS_VDNS_RCODE_BADKEY,
    METRICS_VDNS_RCODE_BADTIME,
    METRICS_VDNS_RCODE_BADMODE,
    METRICS_VDNS_RCODE_BADNAME,
    METRICS_VDNS_RCODE_BADALG,
    METRICS_VDNS_RCODE_UNKNOWN,

    METRICS_VDNS_RCODE_ERROR_COUNT,

} METRICS_VDNS_RCODE_ERRORS;

typedef enum
{
    /*DNS Metrics - Operation Type for rcode error count*/
    METRICS_VDNS_RCODE_OP_TCP_REQ_READ,
    METRICS_VDNS_RCODE_OP_UDP_REQ_READ,
    METRICS_VDNS_RCODE_OP_FORWARDER_RESP,

    METRICS_VDNS_RCODE_OP_COUNT,

} METRICS_VDNS_RCODE_OPS;

typedef enum
{
    /*DNS Metrics - Domain for rcode error count*/
    METRICS_VDNS_RCODE_DOMAIN_LIGHTWAVE,
    METRICS_VDNS_RCODE_DOMAIN_OTHER,

    METRICS_VDNS_RCODE_DOMAIN_COUNT,

} METRICS_VDNS_RCODE_DOMAINS;

typedef enum
{
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

extern PVM_METRICS_COUNTER gVmDnsRcodeErrorMetrics[METRICS_VDNS_RCODE_DOMAIN_COUNT]
                                                   [METRICS_VDNS_RCODE_OP_COUNT]
                                                    [METRICS_VDNS_RCODE_ERROR_COUNT];
extern PVM_METRICS_COUNTER gVmDnsCounterMetrics[VDNS_COUNTER_COUNT];
extern PVM_METRICS_HISTOGRAM gVmDnsHistogramMetrics[VDNS_HISTOGRAM_COUNT];
extern PVM_METRICS_GAUGE gVmDnsGaugeMetrics[VDNS_GAUGE_COUNT];
extern PVM_METRICS_CONTEXT gVmDnsMetricsContext;

// metricsenums.c
PSTR
VmDnsMetricsRcodeDomainString(
    METRICS_VDNS_RCODE_DOMAINS domain
    );

PSTR
VmDnsMetricsRcodeOperationString(
    METRICS_VDNS_RCODE_OPS operation
    );

PSTR
VmDnsMetricsRcodeErrorString(
    METRICS_VDNS_RCODE_ERRORS error
    );

METRICS_VDNS_RCODE_ERRORS
VmDnsMetricsMapRcodeErrorToEnum(
    UCHAR rCode
    );

METRICS_VDNS_RCODE_DOMAINS
VmDnsMetricsMapRcodeDomainToEnum(
    BOOL bQueryInZone
    );

