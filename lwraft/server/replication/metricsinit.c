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

PVM_METRICS_COUNTER pElectionTriggerCount;

DWORD
VmDirRaftMetricsInit(
    VOID
    )
{
    DWORD   dwError = 0;

    dwError = VmMetricsCounterNew(
            pmContext,
            "post_election_trigger_count",
            NULL,0,
            "Counter to monitor number of elections",
            &pElectionTriggerCount);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL, "VmDirReplicationMetricsInit failed (%d)", dwError);
    goto cleanup;
}
