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
VmDirInternalMetricsUpdate(
    METRICS_LDAP_OPS        operation,
    VDIR_OPERATION_PROTOCOL protocol,
    VDIR_OPERATION_TYPE     opType,
    int                     errCode,
    uint64_t                iMLStartTime,
    uint64_t                iMLEndTime,
    uint64_t                iBEStartTime,
    uint64_t                iBEEndTime
    )
{
    if (operation != METRICS_LDAP_OP_IGNORE)
    {
        if (protocol == VDIR_OPERATION_PROTOCOL_LDAP)
        {
            VmDirLdapMetricsUpdate(
                    operation,
                    VmDirMetricsMapLdapOpTypeToEnum(opType),
                    VmDirMetricsMapLdapErrorToEnum(errCode),
                    METRICS_LAYER_MIDDLELAYER,
                    iMLStartTime,
                    iMLEndTime);

            VmDirLdapMetricsUpdate(
                    operation,
                    VmDirMetricsMapLdapOpTypeToEnum(opType),
                    VmDirMetricsMapLdapErrorToEnum(errCode),
                    METRICS_LAYER_BACKEND,
                    iBEStartTime,
                    iBEEndTime);
        }
        else if (protocol == VDIR_OPERATION_PROTOCOL_REST)
        {
            VmDirRestMetricsUpdate(
                    operation,
                    VmDirMetricsMapLdapErrorToEnum(errCode),
                    METRICS_LAYER_MIDDLELAYER,
                    iMLStartTime,
                    iMLEndTime);

            VmDirRestMetricsUpdate(
                    operation,
                    VmDirMetricsMapLdapErrorToEnum(errCode),
                    METRICS_LAYER_BACKEND,
                    iBEStartTime,
                    iBEEndTime);
        }
    }
}
