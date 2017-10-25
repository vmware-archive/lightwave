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

PVM_METRICS_CONTEXT pmContext = NULL;

PVM_METRICS_HISTOGRAM pRpcRequestDuration[METRICS_RPC_OP_COUNT];

static
DWORD
_VmDirRpcMetricsInit(
    VOID);

DWORD
VmDirMetricsInitialize(
    VOID
    )
{
    DWORD   dwError = 0;

    dwError = VmMetricsInit(&pmContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapMetricsInit();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirRpcMetricsInit();
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirMetricsInitialize failed (%d)", dwError);

    goto cleanup;
}

static
DWORD
_VmDirRpcMetricsInit(
    VOID)
{
    DWORD dwError = 0;
    DWORD i = 0;

    uint64_t buckets[5] = {1, 10, 100, 500, 1000};

    VM_METRICS_LABEL labelOps[METRICS_RPC_OP_COUNT][1] = {
        {{"operation", "GeneratePassword"}},
        {{"operation", "CreateUser"}},
        {{"operation", "CreateUserEx"}},
        {{"operation", "SetLogLevel"}},
        {{"operation", "SetLogMask"}},
        {{"operation", "SetState"}},
        {{"operation", "SuperLogQueryServerData"}},
        {{"operation", "SuperLogEnable"}},
        {{"operation", "SuperLogDisable"}},
        {{"operation", "IsSuperLogEnabled"}},
        {{"operation", "SuperLogFlush"}},
        {{"operation", "SuperLogSetSize"}},
        {{"operation", "SuperLogGetSize"}},
        {{"operation", "SuperLogGetEntriesLdapOperation"}},
        {{"operation", "OpenDatabaseFile"}},
        {{"operation", "ReadDatabaseFile"}},
        {{"operation", "CloseDatabaseFile"}},
        {{"operation", "SetBackendState"}},
        {{"operation", "GetState"}},
        {{"operation", "GetLogLevel"}},
        {{"operation", "GetLogMask"}},
        {{"operation", "SetMode"}},
        {{"operation", "GetMode"}},
        {{"operation", "RaftRequestVote"}},
        {{"operation", "RaftAppendEntries"}}
    };

    for (i=0; i < METRICS_RPC_OP_COUNT; i++)
    {
        dwError = VmMetricsHistogramNew(pmContext,
                                "post_dcerpc_request_duration",
                                labelOps[i], 1,
                                "Histogram for DCERPC Request Durations for different operations",
                                buckets, 5,
                                &pRpcRequestDuration[i]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s (%d)", __FUNCTION__, dwError);
    goto cleanup;
}
