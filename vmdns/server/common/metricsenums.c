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

PSTR
VmDnsMetricsRcodeErrorString(
    METRICS_VDNS_RCODE_ERRORS error
    )
{
    static PSTR pszRcodeErrors[METRICS_VDNS_RCODE_ERROR_COUNT] =
    {
            "NOERROR",
            "FORMAT_ERROR",
            "SERVER_FAILURE",
            "NAME_ERROR",
            "NOT_IMPLEMENTED",
            "REFUSED",
            "YXDOMAIN",
            "YXRRSET",
            "NXRRSET",
            "NOTAUTH",
            "NOTZONE",
            "BADSIG",
            "BADKEY",
            "BADTIME",
            "BADMODE",
            "BADNAME",
            "BADALG",
            "UNKNOWN",
    };

    return pszRcodeErrors[error];
}

PSTR
VmDnsMetricsRcodeOperationString(
    METRICS_VDNS_RCODE_OPS operation
    )
{
    static PSTR pszRcodeOperations[METRICS_VDNS_RCODE_OP_COUNT] =
    {
            "TcpRequestDataRead",
            "UdpRequestDataRead",
            "ForwarderResponse"
    };

    return pszRcodeOperations[operation];
}

PSTR
VmDnsMetricsRcodeDomainString(
    METRICS_VDNS_RCODE_DOMAINS domain
    )
{
    static PSTR pszRcodeDomains[METRICS_VDNS_RCODE_DOMAIN_COUNT] =
    {
            "lightwave",
            "other"
    };

    return pszRcodeDomains[domain];
}

METRICS_VDNS_RCODE_ERRORS
VmDnsMetricsMapRcodeErrorToEnum(
    UCHAR rCode
    )
{
    METRICS_VDNS_RCODE_ERRORS metricsRcodeError = METRICS_VDNS_RCODE_UNKNOWN;

    switch (rCode)
    {
        case VM_DNS_RCODE_NOERROR:
            metricsRcodeError = METRICS_VDNS_RCODE_NOERROR;
            break;

        case VM_DNS_RCODE_FORMAT_ERROR:
            metricsRcodeError = METRICS_VDNS_RCODE_FORMAT_ERROR;
            break;

        case VM_DNS_RCODE_SERVER_FAILURE:
            metricsRcodeError = METRICS_VDNS_RCODE_SERVER_FAILURE;
            break;

        case VM_DNS_RCODE_NAME_ERROR:
            metricsRcodeError = METRICS_VDNS_RCODE_NAME_ERROR;
            break;

        case VM_DNS_RCODE_NOT_IMPLEMENTED:
            metricsRcodeError = METRICS_VDNS_RCODE_NOT_IMPLEMENTED;
            break;

        case VM_DNS_RCODE_REFUSED:
            metricsRcodeError = METRICS_VDNS_RCODE_REFUSED;
            break;

        case VM_DNS_RCODE_YXDOMAIN:
            metricsRcodeError = METRICS_VDNS_RCODE_YXDOMAIN;
            break;

        case VM_DNS_RCODE_YXRRSET:
            metricsRcodeError = METRICS_VDNS_RCODE_YXRRSET;
            break;

        case VM_DNS_RCODE_NXRRSET:
            metricsRcodeError = METRICS_VDNS_RCODE_NXRRSET;
            break;

        case VM_DNS_RCODE_NOTAUTH:
            metricsRcodeError = METRICS_VDNS_RCODE_NOTAUTH;
            break;

        case VM_DNS_RCODE_NOTZONE:
            metricsRcodeError = METRICS_VDNS_RCODE_NOTZONE;
            break;

        case VM_DNS_RCODE_BADSIG:
            metricsRcodeError = METRICS_VDNS_RCODE_BADSIG;
            break;

        case VM_DNS_RCODE_BADKEY:
            metricsRcodeError = METRICS_VDNS_RCODE_BADKEY;
            break;

        case VM_DNS_RCODE_BADTIME:
            metricsRcodeError = METRICS_VDNS_RCODE_BADTIME;
            break;

        case VM_DNS_RCODE_BADMODE:
            metricsRcodeError = METRICS_VDNS_RCODE_BADMODE;
            break;

        case VM_DNS_RCODE_BADNAME:
            metricsRcodeError = METRICS_VDNS_RCODE_BADNAME;
            break;

        case VM_DNS_RCODE_BADALG:
            metricsRcodeError = METRICS_VDNS_RCODE_BADALG;
            break;

        default:
            metricsRcodeError = METRICS_VDNS_RCODE_UNKNOWN;
            break;
    }

    return metricsRcodeError;
}

METRICS_VDNS_RCODE_DOMAINS
VmDnsMetricsMapRcodeDomainToEnum(
    BOOL bQueryInZone
    )
{
    METRICS_VDNS_RCODE_DOMAINS metricsRcodeDomain = METRICS_VDNS_RCODE_DOMAIN_OTHER;

    switch (bQueryInZone)
    {
        case true:
            metricsRcodeDomain = METRICS_VDNS_RCODE_DOMAIN_LIGHTWAVE;
            break;

        case false:
            metricsRcodeDomain = METRICS_VDNS_RCODE_DOMAIN_OTHER;
            break;

        default:
            metricsRcodeDomain = METRICS_VDNS_RCODE_DOMAIN_OTHER;
            break;
    }

    return metricsRcodeDomain;
}
