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

VOID
VmDirAuditWriteOp(
    PVDIR_OPERATION pOp,
    PCSTR           pszDN
    )
{
    PVDIR_CONNECTION pConn = NULL;
    PCSTR            pszBy = NULL;

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

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "%s Entry (%s)(from %s)(by %s)(via %s)(USN %llu,%llu)",
                    VmDirLdapReqCodeToName(pOp->reqCode),
                    VDIR_SAFE_STRING(pszDN),
                    pConn ? pConn->szClientIP : "",
                    VDIR_SAFE_STRING(pszBy),
                    VmDirOperationTypeToName(pOp->opType),
                    pOp->pBECtx->wTxnUSN,
                    pOp->ulPartnerUSN);

error:
    return;
}
