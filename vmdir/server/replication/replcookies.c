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

static
int
_VmDirUpdateReplicationAgreement(
    PVDIR_SCHEMA_CTX                pSchemaCtx,
    VMDIR_REPLICATION_AGREEMENT *   replAgr,
    VDIR_BERVALUE *                 lastLocalUsnProcessed
    );

static
int
_VmDirUpdateServerObject(
    PVDIR_SCHEMA_CTX                pSchemaCtx,
    VDIR_BERVALUE *                 utdVector,
    VMDIR_REPLICATION_AGREEMENT *   replAgr
    );

int
VmDirReplCookieUpdate(
    PVDIR_SCHEMA_CTX                pSchemaCtx,
    struct berval *                 syncDoneCtrlVal,
    VMDIR_REPLICATION_AGREEMENT *   replAgr)
{
    int             retVal = LDAP_SUCCESS;
    VDIR_BERVALUE   bvLastLocalUsnProcessed = VDIR_BERVALUE_INIT;
    VDIR_BERVALUE   utdVector = VDIR_BERVALUE_INIT;

    // Update (both in memory and on disk) lastLocalUsnProcessed in the replication agreement, and the
    // utd vector in the server object.

    bvLastLocalUsnProcessed.lberbv.bv_val = syncDoneCtrlVal->bv_val;
    bvLastLocalUsnProcessed.lberbv.bv_len =
            VmDirStringChrA(bvLastLocalUsnProcessed.lberbv.bv_val, ',') -
            bvLastLocalUsnProcessed.lberbv.bv_val;

    // Note: We are effectively over-writing in ctrls[0]->ldctl_value.bv_val here, which should be ok.
    bvLastLocalUsnProcessed.lberbv.bv_val[bvLastLocalUsnProcessed.lberbv.bv_len] = '\0';

    utdVector.lberbv.bv_val = syncDoneCtrlVal->bv_val + bvLastLocalUsnProcessed.lberbv.bv_len + 1;
    utdVector.lberbv.bv_len = syncDoneCtrlVal->bv_len - bvLastLocalUsnProcessed.lberbv.bv_len - 1;

    // if lastLocalUsnProcessed is different
    if (VmDirStringCompareA(bvLastLocalUsnProcessed.lberbv.bv_val, replAgr->lastLocalUsnProcessed.lberbv.bv_val, TRUE) != 0)
    {
        VMDIR_LOG_DEBUG(
                LDAP_DEBUG_REPL,
                "VmDirReplUpdateCookies: Replication cycle done. Updating cookies");

        // Update disk copy of utdVector
        retVal = _VmDirUpdateServerObject(pSchemaCtx, &utdVector, replAgr);
        BAIL_ON_VMDIR_ERROR(retVal);

        // Update memory copy of utdVector
        retVal = VmDirUTDVectorCacheUpdate(utdVector.lberbv.bv_val);
        BAIL_ON_VMDIR_ERROR(retVal);

        // Update disk copy of lastLocalUsnProcessed
        retVal = _VmDirUpdateReplicationAgreement(pSchemaCtx, replAgr, &bvLastLocalUsnProcessed);
        BAIL_ON_VMDIR_ERROR(retVal);

        // Update memory copy of lastLocalUsnProcessed
        VmDirFreeBervalContent(&replAgr->lastLocalUsnProcessed);
        if (VmDirBervalContentDup(&bvLastLocalUsnProcessed, &replAgr->lastLocalUsnProcessed) != 0)
        {
            VMDIR_LOG_ERROR(
                    VMDIR_LOG_MASK_ALL,
                    "VmDirReplUpdateCookies: BervalContentDup failed.");

            retVal = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR(retVal);
        }
    }
    else
    {
        VMDIR_LOG_DEBUG(
                LDAP_DEBUG_REPL,
                "VmDirReplUpdateCookies: Replication cycle done.. Skipping updating Replication cookies.");
    }
    retVal = LDAP_SUCCESS;

cleanup:
    return retVal;

error:
    goto cleanup;
}

static
int
_VmDirUpdateReplicationAgreement(
    PVDIR_SCHEMA_CTX                pSchemaCtx,
    VMDIR_REPLICATION_AGREEMENT *   replAgr,
    VDIR_BERVALUE *                 lastLocalUsnProcessed)
{
    // Load my Replication Agreements
    int                  retVal = LDAP_SUCCESS;
    VDIR_OPERATION       op = {0};
    VDIR_MODIFICATION *  mod = NULL;
    VDIR_BERVALUE        attrLLUsnProcessed = {
                            {ATTR_LAST_LOCAL_USN_PROCESSED_LEN, ATTR_LAST_LOCAL_USN_PROCESSED}, 0, 0, NULL };
    VDIR_BERVALUE *      vals;

    retVal = VmDirInitStackOperation(&op,
                                     VDIR_OPERATION_TYPE_INTERNAL,
                                     LDAP_REQ_MODIFY,
                                     pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(retVal);

    if (VmDirBervalContentDup(&replAgr->dn, &op.reqDn) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: BervalContentDup failed.", __FUNCTION__);
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    op.pBEIF = VmDirBackendSelect(op.reqDn.lberbv.bv_val);
    assert(op.pBEIF);

    if (VmDirAllocateMemory(sizeof(VDIR_MODIFICATION), (PVOID *)&mod) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: VmDirAllocateMemory failed.", __FUNCTION__);
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR(retVal);
    }
    if (VmDirAllocateMemory(2 * sizeof(VDIR_BERVALUE), (PVOID *)&vals) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: VmDirAllocateMemory failed.", __FUNCTION__);
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    if (VmDirBervalContentDup(&op.reqDn, &op.request.modifyReq.dn) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: BervalContentDup failed.", __FUNCTION__);
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR(retVal);
    }
    op.request.modifyReq.mods = mod;
    op.request.modifyReq.numMods = 1;

    mod->operation = MOD_OP_REPLACE;
    mod->attr.type = attrLLUsnProcessed;
    mod->attr.vals = vals;
    vals[0] = *lastLocalUsnProcessed;
    vals[1].lberbv.bv_val = NULL;
    vals[1].lberbv.bv_len = 0;
    mod->attr.numVals = 1;

    if ((retVal = VmDirInternalModifyEntry(&op)) != 0)
    {
        // If VmDirInternall call failed, reset retVal to LDAP level error space (for B/C)
        retVal = op.ldapResult.errCode;

        VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s: InternalModifyEntry failed. Error code: %d, Error string: %s",
            __FUNCTION__,
            retVal,
            VDIR_SAFE_STRING(op.ldapResult.pszErrMsg));
        BAIL_ON_VMDIR_ERROR(retVal);
    }

cleanup:
    VmDirFreeOperationContent(&op);
    return retVal;

error:
    goto cleanup;
}

static
int
_VmDirUpdateServerObject(
    PVDIR_SCHEMA_CTX                pSchemaCtx,
    VDIR_BERVALUE *                 utdVector,
    VMDIR_REPLICATION_AGREEMENT *   replAgr)
{
    int                      retVal = LDAP_SUCCESS;
    VDIR_OPERATION           op = {0};
    VDIR_MODIFICATION *      mod = NULL;
    VDIR_BERVALUE            attrUtdVector = {{ATTR_UP_TO_DATE_VECTOR_LEN, ATTR_UP_TO_DATE_VECTOR}, 0, 0, NULL};
    VDIR_BERVALUE *          vals = NULL;
    VDIR_BERVALUE            utdVectorCopy = VDIR_BERVALUE_INIT;

    retVal = VmDirInitStackOperation(&op,
                                      VDIR_OPERATION_TYPE_INTERNAL,
                                      LDAP_REQ_MODIFY,
                                      pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(retVal);

    if (VmDirAllocateMemory(sizeof(VDIR_MODIFICATION), (PVOID *)&mod) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: VmDirAllocateMemory failed.", __FUNCTION__);
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }
    if (VmDirAllocateMemory(2 * sizeof(VDIR_BERVALUE), (PVOID *)&vals) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: VmDirAllocateMemory failed.", __FUNCTION__);
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    if (VmDirBervalContentDup(&gVmdirServerGlobals.serverObjDN, &op.reqDn) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: BervalContentDup failed.", __FUNCTION__);
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    op.pBEIF = VmDirBackendSelect(op.reqDn.lberbv.bv_val);
    assert(op.pBEIF);

    if (VmDirBervalContentDup(&op.reqDn, &op.request.modifyReq.dn) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: BervalContentDup failed.", __FUNCTION__);
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    op.request.modifyReq.mods = mod;
    op.request.modifyReq.numMods = 1;

    mod->operation = MOD_OP_REPLACE;
    mod->attr.type = attrUtdVector;
    mod->attr.vals = vals;

    if (VmDirBervalContentDup(utdVector, &utdVectorCopy) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: BervalContentDup failed.", __FUNCTION__);
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    vals[0] = utdVectorCopy;
    vals[1].lberbv.bv_val = NULL;
    vals[1].lberbv.bv_len = 0;
    mod->attr.numVals = 1;

    if ((retVal = VmDirInternalModifyEntry(&op)) != 0)
    {
        VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s: InternalModifyEntry failed. retVal: %d ldapError: %d string: %s",
            __FUNCTION__,
            retVal,
            op.ldapResult.errCode,
            VDIR_SAFE_STRING(op.ldapResult.pszErrMsg));
        BAIL_ON_VMDIR_ERROR(retVal);
    }

cleanup:
    VmDirFreeOperationContent(&op);
    return retVal;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: failed (%u)", __FUNCTION__, retVal);
    goto cleanup;
}
