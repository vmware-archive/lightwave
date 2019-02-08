/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

VOID
VmDirConsumePartner(
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr
    )
{
    int                            retVal = LDAP_SUCCESS;
    USN                            lastLocalUsnProcessed = 0;
    uint64_t                       uiStartTime = 0;
    uint64_t                       uiEndTime = 0;
    PVMDIR_REPLICATION_UPDATE_LIST pReplUpdateList = NULL;
    PVMDIR_REPLICATION_METRICS     pReplMetrics = NULL;

    uiStartTime = VmDirGetTimeInMilliSec();

    retVal = VmDirStringToINT64(
            pReplAgr->lastLocalUsnProcessed.lberbv.bv_val, NULL, &lastLocalUsnProcessed);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    retVal = VmDirReplUpdateListFetch(pReplAgr, &pReplUpdateList);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    retVal = VmDirReplUpdateListExpand(pReplUpdateList);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    VmDirReplUpdateListApply(pReplUpdateList);

    retVal = VmDirReplCookieUpdate(
            NULL,
            pReplUpdateList->newHighWaterMark,
            pReplUpdateList->pNewUtdVector,
            pReplAgr);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    VMDIR_LOG_INFO(
            VMDIR_LOG_MASK_ALL,
            "Replication supplier %s USN range (%llu,%s) processed",
            pReplAgr->ldapURI,
            lastLocalUsnProcessed,
            pReplAgr->lastLocalUsnProcessed.lberbv_val);

collectmetrics:
    uiEndTime = VmDirGetTimeInMilliSec();
    if (VmDirReplMetricsCacheFind(pReplAgr->pszHostname, &pReplMetrics) == 0)
    {
        if (retVal == LDAP_SUCCESS)
        {
            VmMetricsHistogramUpdate(
                    pReplMetrics->pTimeCycleSucceeded,
                    VMDIR_RESPONSE_TIME(uiStartTime, uiEndTime));
        }
        // avoid collecting benign error counts
        else if (retVal != LDAP_UNAVAILABLE &&  // server in mid-shutdown
                 retVal != LDAP_SERVER_DOWN &&  // connection lost
                 retVal != LDAP_TIMEOUT)        // connection lost
        {
            VmMetricsHistogramUpdate(
                    pReplMetrics->pTimeCycleFailed,
                    VMDIR_RESPONSE_TIME(uiStartTime, uiEndTime));
        }
    }

    VmDirFreeReplUpdateList(pReplUpdateList);

    return;

ldaperror:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error code (%d)",
            __FUNCTION__,
            retVal);

    goto collectmetrics;
}
