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



#include "includes.h"


static PVMDNS_OPERATION_STATISTIC gVmdnsMetricsTable[] =
{
       [DNS_QUERY_COUNT] = &gVmdnsOPStatisticGlobals.dns_query_count,
       [FORWARDER_QUERY_COUNT] = &gVmdnsOPStatisticGlobals.forwarder_query_count
};

VOID
VmDnsOPStatisticUpdate(
    UINT16 opTag
    )
{
    PVMDNS_OPERATION_STATISTIC pStatistic = NULL;

    pStatistic = gVmdnsMetricsTable[opTag];
    InterlockedIncrement(&pStatistic->iCount);
}

LONG
VmDnsOPStatisticGetCount(
    UINT16 opTag
    )
{
    LONG    iCurrentCount = 0;
    PVMDNS_OPERATION_STATISTIC pStatistic = NULL;

    pStatistic = gVmdnsMetricsTable[opTag];
    if (pStatistic == NULL)
    {
        return 0;
    }

    iCurrentCount = pStatistic->iCount;

    return iCurrentCount;
}

