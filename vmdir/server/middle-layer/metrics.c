/*
 * Copyright © 2019 VMware, Inc.  All Rights Reserved.
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
    PVDIR_OPERATION pOp
    )
{
    if (pOp)
    {
        int   errCode =
            VmDirMetricsMapLdapErrorToEnum(pOp->ldapResult.errCode);
        METRICS_LDAP_OPS            operation =
            VmDirMetricsMapLdapOperationToEnum(pOp->reqCode);
        VDIR_OPERATION_TYPE         opType =
            VmDirMetricsMapLdapOpTypeToEnum(pOp->opType);
        VDIR_OPERATION_PROTOCOL     protocol = pOp->protocol;
        PVDIR_OPERATION_ML_METRIC   pMLMetrics = &pOp->MLMetrics;

        if (operation != METRICS_LDAP_OP_IGNORE)
        {
            if (protocol == VDIR_OPERATION_PROTOCOL_LDAP)
            {
                VmDirLdapMetricsUpdate(
                        operation,
                        opType,
                        errCode,
                        METRICS_LAYER_MIDDLELAYER,
                        pMLMetrics->iMLStartTime,
                        pMLMetrics->iMLEndTime);

                VmDirLdapMetricsUpdate(
                        operation,
                        opType,
                        errCode,
                        METRICS_LAYER_PRE_PLUGINS,
                        pMLMetrics->iPrePluginsStartTime,
                        pMLMetrics->iPrePluginsEndTime);

                VmDirLdapMetricsUpdate(
                        operation,
                        opType,
                        errCode,
                        METRICS_LAYER_WRITE_QUEUE,
                        pMLMetrics->iWriteQueueWaitStartTime,
                        pMLMetrics->iWriteQueueWaitEndTime);

                VmDirLdapMetricsUpdate(
                        operation,
                        opType,
                        errCode,
                        METRICS_LAYER_BACKEND,
                        pMLMetrics->iBETxnBeginStartTime,
                        pMLMetrics->iBETxnCommitEndTime);

                VmDirLdapMetricsUpdate(
                        operation,
                        opType,
                        errCode,
                        METRICS_LAYER_BACKEND_TXN_BEGIN,
                        pMLMetrics->iBETxnBeginStartTime,
                        pMLMetrics->iBETxnBeginEndTime);

                VmDirLdapMetricsUpdate(
                        operation,
                        opType,
                        errCode,
                        METRICS_LAYER_BACKEND_TXN_COMMIT,
                        pMLMetrics->iBETxnCommitStartTime,
                        pMLMetrics->iBETxnCommitEndTime);

                VmDirLdapMetricsUpdate(
                        operation,
                        opType,
                        errCode,
                        METRICS_LAYER_POST_PLUGINS,
                        pMLMetrics->iPostPluginsStartTime,
                        pMLMetrics->iPostPluginsEndTime);
            }
            else if (protocol == VDIR_OPERATION_PROTOCOL_REST)
            {
                VmDirRestMetricsUpdate(
                        operation,
                        errCode,
                        METRICS_LAYER_MIDDLELAYER,
                        pMLMetrics->iMLStartTime,
                        pMLMetrics->iMLEndTime);

                VmDirRestMetricsUpdate(
                        operation,
                        errCode,
                        METRICS_LAYER_PRE_PLUGINS,
                        pMLMetrics->iPrePluginsStartTime,
                        pMLMetrics->iPrePluginsEndTime);

                VmDirRestMetricsUpdate(
                        operation,
                        errCode,
                        METRICS_LAYER_WRITE_QUEUE,
                        pMLMetrics->iWriteQueueWaitStartTime,
                        pMLMetrics->iWriteQueueWaitEndTime);

                VmDirRestMetricsUpdate(
                        operation,
                        errCode,
                        METRICS_LAYER_BACKEND,
                        pMLMetrics->iBETxnBeginStartTime,
                        pMLMetrics->iBETxnCommitEndTime);

                VmDirRestMetricsUpdate(
                        operation,
                        errCode,
                        METRICS_LAYER_BACKEND_TXN_BEGIN,
                        pMLMetrics->iBETxnBeginStartTime,
                        pMLMetrics->iBETxnBeginEndTime);

                VmDirRestMetricsUpdate(
                        operation,
                        errCode,
                        METRICS_LAYER_BACKEND_TXN_COMMIT,
                        pMLMetrics->iBETxnCommitStartTime,
                        pMLMetrics->iBETxnCommitEndTime);

                VmDirRestMetricsUpdate(
                        operation,
                        errCode,
                        METRICS_LAYER_POST_PLUGINS,
                        pMLMetrics->iPostPluginsStartTime,
                        pMLMetrics->iPostPluginsEndTime);
            }
        }
    }
}

VOID
VmDirInternalMetricsLogInefficientOp(
    PVDIR_OPERATION pOperation
    )
{
    uint64_t                  iRespTime = 0;
    PVDIR_OPERATION_ML_METRIC pMLMetrics = NULL;
    METRICS_LDAP_OPS          op = VmDirMetricsMapLdapOperationToEnum(pOperation->reqCode);

    if (pOperation)
    {
        pMLMetrics = &pOperation->MLMetrics;
        iRespTime = VMDIR_RESPONSE_TIME(pMLMetrics->iMLStartTime, pMLMetrics->iMLEndTime);

        if ((op == METRICS_LDAP_OP_SEARCH && iRespTime > gVmdirServerGlobals.dwEfficientReadOpTimeMS)
                || iRespTime > gVmdirServerGlobals.dwEfficientWriteOpTimeMS)
        {
            VMDIR_LOG_WARNING(
                    VMDIR_LOG_MASK_ALL,
                    "[Middle Layer] Inefficient operation of type %s"
                    " total time(%d) Preplugin(%d) WriteQueue(%d)BETxnBegin(%d) BETxnCommit(%d) PostPlugin(%d)",
                    VmDirMetricsLdapOperationString(op),
                    VMDIR_RESPONSE_TIME(pMLMetrics->iMLStartTime, pMLMetrics->iMLEndTime),
                    VMDIR_RESPONSE_TIME(
                        pMLMetrics->iPrePluginsStartTime, pMLMetrics->iPrePluginsEndTime),
                    VMDIR_RESPONSE_TIME(
                        pMLMetrics->iWriteQueueWaitStartTime, pMLMetrics->iWriteQueueWaitEndTime),
                    VMDIR_RESPONSE_TIME(
                        pMLMetrics->iBETxnBeginStartTime, pMLMetrics->iBETxnBeginEndTime),
                    VMDIR_RESPONSE_TIME(
                        pMLMetrics->iBETxnCommitStartTime, pMLMetrics->iBETxnCommitEndTime),
                    VMDIR_RESPONSE_TIME(
                        pMLMetrics->iPostPluginsStartTime, pMLMetrics->iPostPluginsEndTime));
        }
    }
}
