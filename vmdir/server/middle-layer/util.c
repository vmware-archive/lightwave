/*
 * Copyright © 2016 VMware, Inc.  All Rights Reserved.
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

static
LONG
_VmDirLogEntryConvergenceTime(
    PVDIR_ENTRY   pEntry
    );

VOID
VmDirAuditWriteOp(
    PVDIR_OPERATION pOp,
    PCSTR           pszDN,
    PVDIR_ENTRY     pEntry
    )
{
    PVDIR_CONNECTION pConn = NULL;
    PCSTR            pszBy = NULL;
    LONG             convTime = 0;

    if (!pOp || !pOp->pBECtx || !pszDN)
    {
        goto error;
    }

    pConn = pOp->conn;
    if (pOp->opType == VDIR_OPERATION_TYPE_EXTERNAL)
    {
        assert(pConn);  // must have pConn for external operation
        if (pConn->pSaslInfo)
        {
            pszBy = pConn->pSaslInfo->pszBindUserName;
        }
        else
        {
            pszBy = pConn->AccessInfo.pszBindedDn;
        }
    }

    if (pOp->opType == VDIR_OPERATION_TYPE_REPL)
    {
        // best effort to calculate the convergence time
        convTime = _VmDirLogEntryConvergenceTime(pEntry);
    }

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "%s Entry (%s)(from %s)(by %s)(via %s)(USN %llu,%llu) (conv Time: %llu)",
                    VmDirLdapReqCodeToName(pOp->reqCode),
                    VDIR_SAFE_STRING(pszDN),
                    pConn ? pConn->szClientIP : "",
                    VDIR_SAFE_STRING(pszBy),
                    VmDirOperationTypeToName(pOp->opType),
                    pOp->pBECtx->wTxnUSN,
                    pOp->ulPartnerUSN,
                    convTime);

error:
    return;
}

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

static
LONG
_VmDirLogEntryConvergenceTime(
    PVDIR_ENTRY   pEntry
    )
{
    DWORD   dwError = 0;
    PSTR    pszModifyTime = NULL;
    LONG    modified = 0;
    LONG    now = 0;
    LONG    duration = 0;
    PVDIR_ATTRIBUTE pAttr = NULL;

    if (pEntry)
    {
        pAttr = VmDirFindAttrByName(pEntry, ATTR_MODIFYTIMESTAMP);

        if (pAttr)
        {
            pszModifyTime = pAttr->vals[0].lberbv_val;

            dwError = VmDirConvertTimestampToEpoch(pszModifyTime, &modified);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirConvertTimestampToEpoch(NULL, &now);
            BAIL_ON_VMDIR_ERROR(dwError);

            duration = modified < now ? now - modified : 0;
        }
    }

cleanup:
    return duration;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", dwError);
    goto cleanup;
}
