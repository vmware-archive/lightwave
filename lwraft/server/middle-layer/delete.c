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

static void
DeleteMods(
    ModifyReq * modReq);

int
VmDirMLDelete(
    PVDIR_OPERATION    pOperation
    )
{
    DWORD   dwError = 0;
    PSTR    pszLocalErrMsg = NULL;

    pOperation->pBECtx->pBE = VmDirBackendSelect(pOperation->reqDn.lberbv.bv_val);
    assert(pOperation->pBECtx->pBE);

    // AnonymousBind Or in case of a failed bind, do not grant delete access
    if (pOperation->conn->bIsAnonymousBind || VmDirIsFailedAccessInfo(&pOperation->conn->AccessInfo))
    {
        dwError = LDAP_INSUFFICIENT_ACCESS;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(
                dwError, pszLocalErrMsg,
                "Not bind/authenticate yet");
    }

    if (!VmDirValidTxnState(pOperation->pBECtx, pOperation->reqCode))
    {
       dwError = LDAP_UNWILLING_TO_PERFORM;
       BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg,
               "%s: invaid request for transaction state", __func__);
    }

    dwError = VmDirInternalDeleteEntry(pOperation);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);
    return pOperation->ldapResult.errCode;

error:
    VMDIR_SET_LDAP_RESULT_ERROR(&(pOperation->ldapResult), dwError, pszLocalErrMsg);
    goto cleanup;
}

/* VmDirInternalDeleteEntry: Interface that can be used "internally" by the server code. One of the main differences between
 * this function and MLDelete is that this function does not send back an LDAP result to the client.
 *
 * Return: VmDir level error code.  Also, pOperation->ldapResult content is set.
 */
int
VmDirInternalDeleteEntry(
    PVDIR_OPERATION    pOperation
    )
{
    int         retVal = LDAP_SUCCESS;
    VDIR_ENTRY  entry = {0};
    PVDIR_ENTRY pEntry = NULL;
    BOOLEAN     leafNode = FALSE;
    DeleteReq*  delReq = &(pOperation->request.deleteReq);
    ModifyReq*  modReq = &(pOperation->request.modifyReq);
    BOOLEAN     bIsDomainObject = FALSE;
    BOOLEAN     bHasTxn = FALSE;
    PSTR        pszLocalErrMsg = NULL;
    PVDIR_OPERATION_ML_METRIC   pMLMetrics = NULL;
    extern DWORD VmDirDeleteRaftPreCommit(PVDIR_SCHEMA_CTX, EntryId, char *, PVDIR_OPERATION);

    assert(pOperation && pOperation->pBECtx->pBE);

    pMLMetrics = &pOperation->MLMetrics;
    VMDIR_COLLECT_TIME(pMLMetrics->iMLStartTime);

    if (VmDirdState() == VMDIRD_STATE_READ_ONLY)
    {
        retVal = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(
                retVal, pszLocalErrMsg,
                "Server in read-only mode");
    }

    // make sure we have minimum DN length
    if (delReq->dn.lberbv_len < 3)
    {
        retVal = VMDIR_ERROR_INVALID_REQUEST;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(
                retVal, pszLocalErrMsg,
                "Invalid DN length - (%u)",
                delReq->dn.lberbv_len);
    }

    // Normalize DN
    retVal = VmDirNormalizeDN(&(delReq->dn), pOperation->pSchemaCtx);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(
            retVal, pszLocalErrMsg,
            "DN normalization failed - (%u)(%s)",
            retVal,
            VDIR_SAFE_STRING(VmDirSchemaCtxGetErrorMsg(pOperation->pSchemaCtx)));

    VMDIR_COLLECT_TIME(pMLMetrics->iBETxnBeginStartTime);

    retVal = pOperation->pBEIF->pfnBETxnBegin(pOperation->pBECtx, VDIR_BACKEND_TXN_WRITE, &bHasTxn);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg,
            "txn begin (%u)(%s)", retVal, VDIR_SAFE_STRING(pOperation->pBEErrorMsg));
    VMDIR_COLLECT_TIME(pMLMetrics->iBETxnBeginEndTime);

    if (bHasTxn)
    {
       retVal = VmDirValidateOp(pOperation, __func__);
       BAIL_ON_VMDIR_ERROR(retVal);
    }

    // Execute pre modify apply Delete plugin logic
    VMDIR_COLLECT_TIME(pMLMetrics->iPrePluginsStartTime);

    retVal = VmDirExecutePreModApplyDeletePlugins(pOperation, NULL, retVal);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(
            retVal, pszLocalErrMsg,
            "PreModApplyDelete plugin failed - (%u)",
            retVal);

    VMDIR_COLLECT_TIME(pMLMetrics->iPrePlugunsEndTim);

    retVal = VmDirNormalizeMods(pOperation->pSchemaCtx, modReq->mods, &pszLocalErrMsg);
    BAIL_ON_VMDIR_ERROR(retVal);

    // BUGBUG, need to protect some system entries such as schema,domain....etc?

    // Read current entry from DB
    retVal = pOperation->pBEIF->pfnBEDNToEntry(
            pOperation->pBECtx,
            pOperation->pSchemaCtx,
            &(delReq->dn),
            &entry,
            VDIR_BACKEND_ENTRY_LOCK_WRITE);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(
            retVal, pszLocalErrMsg,
            "(%u)(%s)",
            retVal,
            VDIR_SAFE_STRING(pOperation->pBEErrorMsg));

    pEntry = &entry;

    // Parse Parent DN
    retVal = VmDirGetParentDN(&pEntry->dn, &pEntry->pdn);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(
            retVal, pszLocalErrMsg,
            "Get ParentDn failed - (%u)",
            retVal);

    // get parent entry
    if (pEntry->pdn.lberbv.bv_val)
    {
        PVDIR_ENTRY     pParentEntry = NULL;

        retVal = VmDirAllocateMemory(sizeof(*pEntry), (PVOID)&pParentEntry);
        BAIL_ON_VMDIR_ERROR(retVal);

        retVal = pOperation->pBEIF->pfnBEDNToEntry(
                pOperation->pBECtx,
                pOperation->pSchemaCtx,
                &pEntry->pdn,
                pParentEntry,
                VDIR_BACKEND_ENTRY_LOCK_READ);
        if (retVal)
        {
            VmDirFreeEntryContent(pParentEntry);
            VMDIR_SAFE_FREE_MEMORY(pParentEntry);

            switch (retVal)
            {
            case VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND:
                BAIL_ON_VMDIR_ERROR_WITH_MSG(
                        retVal, pszLocalErrMsg,
                        "parent (%s) not found, (%s)",
                        pEntry->pdn.lberbv_val,
                        VDIR_SAFE_STRING(pOperation->pBEErrorMsg));

            default:
                BAIL_ON_VMDIR_ERROR_WITH_MSG(
                        retVal, pszLocalErrMsg,
                        "parent (%s) lookup failed, (%s)",
                        pEntry->pdn.lberbv_val,
                        VDIR_SAFE_STRING(pOperation->pBEErrorMsg));
            }
        }

        pEntry->pParentEntry = pParentEntry;    // pEntry takes over pParentEntry
        pParentEntry = NULL;
    }

    //
    // The delete will succeed if the caller either has the explicit right
    // to delete this object or if they have the right to delete children
    // of this object's parent.
    //
    retVal = VmDirSrvAccessCheck(
            pOperation,
            &pOperation->conn->AccessInfo,
            pEntry,
            VMDIR_RIGHT_DS_DELETE_OBJECT);
    if (retVal != ERROR_SUCCESS && pEntry->pParentEntry)
    {
        retVal = VmDirSrvAccessCheck(
                pOperation,
                &pOperation->conn->AccessInfo,
                pEntry->pParentEntry,
                VMDIR_RIGHT_DS_DELETE_CHILD);
    }
    BAIL_ON_VMDIR_ERROR_WITH_MSG(
            retVal, pszLocalErrMsg,
            "VmDirSrvAccessCheck failed - (%u)(%s)",
            retVal,
            VMDIR_ACCESS_DENIED_ERROR_MSG);

    // Make sure it is a leaf node
    retVal = pOperation->pBEIF->pfnBEChkIsLeafEntry(
            pOperation->pBECtx,
            pEntry->eId,
            &leafNode);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(
            retVal, pszLocalErrMsg,
            "BEChkIsLeafEntry failed, (%u)(%s)",
            retVal,
            VDIR_SAFE_STRING(pOperation->pBEErrorMsg));

    if (leafNode == FALSE)
    {
        retVal = VMDIR_ERROR_NOT_ALLOWED_ON_NONLEAF;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(
                retVal, pszLocalErrMsg,
                "Delete of a non-leaf node is not allowed.");
    }

    // Retrieve to determine whether it is domain object earlier
    // before attribute modifications
    // ('bIsDomainObject' is needed for a domain object deletion)
    retVal = VmDirIsDomainObjectWithEntry(pEntry, &bIsDomainObject);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(
            retVal, pszLocalErrMsg,
            "VmDirIsDomainObjectWithEntry failed - (%u)",
            retVal);

    retVal = GenerateDeleteAttrsMods(pOperation, pEntry);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(
            retVal, pszLocalErrMsg,
            "GenerateDeleteAttrsMods failed - (%u)",
            retVal);

    // Normalize attribute values in mods
    retVal = VmDirNormalizeMods(pOperation->pSchemaCtx, modReq->mods, &pszLocalErrMsg);
    BAIL_ON_VMDIR_ERROR(retVal);

    // Apply modify operations to the current entry in the DB.
    retVal = VmDirApplyModsToEntryStruct(pOperation->pSchemaCtx, modReq, pEntry, NULL, &pszLocalErrMsg);
    BAIL_ON_VMDIR_ERROR(retVal);

    // Update Entry
    retVal = pOperation->pBEIF->pfnBEEntryDelete(pOperation->pBECtx, modReq->mods, pEntry);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(
            retVal, pszLocalErrMsg,
            "BEEntryDelete (%u)(%s)",
            retVal,
            VDIR_SAFE_STRING(pOperation->pBEErrorMsg));

    retVal = DeleteRefAttributesValue(pOperation, &(pEntry->dn));
    BAIL_ON_VMDIR_ERROR_WITH_MSG(
            retVal, pszLocalErrMsg,
            "DeleteRefAttributesValue (%u)(%s)",
            retVal,
            VDIR_SAFE_STRING(pOperation->pBEErrorMsg));

    // Use normalized DN value
    if (bIsDomainObject)
    {
        retVal = VmDirInternalRemoveOrgConfig(pOperation, BERVAL_NORM_VAL(pEntry->dn));
        BAIL_ON_VMDIR_ERROR_WITH_MSG(
                retVal, pszLocalErrMsg,
                "Update domain list entry failed.");
    }

    if (pOperation->bNoRaftLog == FALSE)
    {
        retVal = VmDirDeleteRaftPreCommit(
            pOperation->pSchemaCtx, pEntry->eId, BERVAL_NORM_VAL(pEntry->dn), pOperation);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(
            retVal, pszLocalErrMsg,
            "VmDirDeleteRaftPreCommit error (%u)",
            retVal);
    }


    if (bHasTxn)
    {
        VMDIR_COLLECT_TIME(pMLMetrics->iBETxnCommitStartTime);

        retVal = pOperation->pBEIF->pfnBETxnCommit(pOperation->pBECtx);
        bHasTxn = FALSE;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(
             retVal, pszLocalErrMsg,
             "txn commit (%u)(%s)",
             retVal,
             VDIR_SAFE_STRING(pOperation->pBEErrorMsg));
        VMDIR_COLLECT_TIME(pMLMetrics->iBETxnCommitEndTime);
    }

    if (!pOperation->bSuppressLogInfo)
    {
        VMDIR_LOG_INFO(
                VMDIR_LOG_MASK_ALL,
                "Delete Entry (%s)",
                VDIR_SAFE_STRING(pEntry->dn.lberbv_val));
    }

    // Post delete entry
    // TODO, make it into a separate file deletePlugin.c
    // clean lockout cache record if exists
    VdirLockoutCacheRemoveRec(pEntry->dn.bvnorm_val);

cleanup:

    if (retVal == 0)
    {
        int iPostCommitPluginRtn  = 0;

        VMDIR_COLLECT_TIME(pMLMetrics->iPostPluginsStartTime);

        // Execute post Delete commit plugin logic
        iPostCommitPluginRtn = VmDirExecutePostDeleteCommitPlugins(pOperation, pEntry, retVal);
        if (iPostCommitPluginRtn != LDAP_SUCCESS &&
            iPostCommitPluginRtn != pOperation->ldapResult.errCode) // pass through
        {
            VMDIR_LOG_ERROR(
                    LDAP_DEBUG_ANY,
                    "InternalDeleteEntry: VdirExecutePostDeleteCommitPlugins - code(%d)",
                    iPostCommitPluginRtn);
        }

        VMDIR_COLLECT_TIME(pMLMetrics->iPostPlugunsEndTime);
    }

    // collect metrics
    VMDIR_COLLECT_TIME(pMLMetrics->iMLEndTime);
    VmDirInternalMetricsUpdate(pOperation);
    VmDirInternalMetricsLogInefficientOp(pOperation);

    if (pOperation->opType != VDIR_OPERATION_TYPE_REPL)
    {
        // In case of replication, modReq is owned by the Replication thread/logic
        DeleteMods(modReq);
    }

    VmDirFreeEntryContent(&entry);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);
    return retVal;

error:
    if (bHasTxn)
    {
        pOperation->pBEIF->pfnBETxnAbort(pOperation->pBECtx);
    }

    VMDIR_SET_LDAP_RESULT_ERROR(&pOperation->ldapResult, retVal, pszLocalErrMsg);
    goto cleanup;
}

void
DeleteMods(
    ModifyReq * modReq)
{
    VDIR_MODIFICATION * currMod = NULL;
    VDIR_MODIFICATION * tmpMod = NULL;

    for (currMod = modReq->mods; currMod != NULL; )
    {
        tmpMod = currMod->next;
        VmDirModificationFree(currMod);
        currMod = tmpMod;
    }
    modReq->numMods = 0;
}

/* DeleteRefAttributesValue: For the given DN (dn), find out in which groups it appears as member attribute value.
 * Delete this member attribute value from these groups.
 *
 * Returns LDAP error codes including LDAP_LOCK_DEADLOCK
 */

int
DeleteRefAttributesValue(
    VDIR_OPERATION * pOperation,
    VDIR_BERVALUE * dn
    )
{
    int               retVal = LDAP_SUCCESS;
    VDIR_FILTER *     f = NULL;
    VDIR_CANDIDATES * cl = NULL;
    VDIR_ENTRY        groupEntry = {0};
    VDIR_ENTRY *      pGroupEntry = NULL;
    int          i = 0;
    VDIR_MODIFICATION mod = {0};
    ModifyReq    mr;
    VDIR_BERVALUE     delVals[2];
    PSTR              pszLocalErrorMsg = NULL;

    assert(pOperation != NULL && pOperation->pBEIF != NULL && dn != NULL);

    retVal = VmDirNormalizeDN(dn, pOperation->pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(retVal);

    // Set filter
    retVal = VmDirAllocateMemory(sizeof(VDIR_FILTER), (PVOID *)&f);
    BAIL_ON_VMDIR_ERROR(retVal);

    f->choice = LDAP_FILTER_EQUALITY;
    f->filtComp.ava.type.lberbv.bv_val = ATTR_MEMBER;
    f->filtComp.ava.type.lberbv.bv_len = ATTR_MEMBER_LEN;
    f->filtComp.ava.value = *dn;
    if ((f->filtComp.ava.pATDesc = VmDirSchemaAttrNameToDesc(pOperation->pSchemaCtx, ATTR_MEMBER)) == NULL)
    {
        retVal = VMDIR_ERROR_NO_SUCH_ATTRIBUTE;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(retVal, (pszLocalErrorMsg),
                                        "undefined attribute (%s)",
                                        VDIR_SAFE_STRING(ATTR_MEMBER));
    }
    // Set ModifyReq structure
    memset(&mr, 0, sizeof(ModifyReq));
    mod.operation = MOD_OP_DELETE;
    mod.attr.type.lberbv.bv_val = ATTR_MEMBER;
    mod.attr.type.lberbv.bv_len = ATTR_MEMBER_LEN;
    mod.attr.pATDesc = f->filtComp.ava.pATDesc;

    mod.attr.next = NULL;
    delVals[0] = *dn;
    memset(&(delVals[1]), 0, sizeof(VDIR_BERVALUE));
    mod.attr.vals = delVals;
    mod.attr.numVals = 1;
    mod.next = NULL;
    mr.mods = &mod;
    mr.numMods = 1;

    retVal = pOperation->pBEIF->pfnBEGetCandidates(pOperation->pBECtx, f, 0);
    if (retVal != 0)
    {
        if (retVal == VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND)
        {
            retVal = LDAP_SUCCESS;  // no member refer to this DN. return ok/0
        }
        else
        {
            retVal = VMDIR_ERROR_GENERIC;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(retVal, (pszLocalErrorMsg),
                        "DeleteRefAttributesValue: Building group list (BdbGetCandidates()) failed.");
        }
    }
    else
    {
        cl = f->candidates;

        for (i = 0; i < cl->size; i++)
        {
            pGroupEntry = &groupEntry;
            if ((retVal = VmDirModifyEntryCoreLogic(pOperation, &mr, cl->eIds[i], TRUE, pGroupEntry)) != 0)
            {
                switch (retVal)
                {
                    case VMDIR_ERROR_BACKEND_PARENT_NOTFOUND:
                    case VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND:
                    case VMDIR_ERROR_ENTRY_NOT_FOUND:
                        continue;

                    default: // Including LDAP_LOCK_DEADLOCK, which is handled by the caller
                        BAIL_ON_VMDIR_ERROR(retVal);
                }
            }

            VmDirFreeBervalContent(&(mr.dn)); // VmDirModifyEntryCoreLogic fill in DN if not exists
            VmDirFreeEntryContent(pGroupEntry);
            pGroupEntry = NULL; // Reset to NULL so that DeleteEntry is no-op.
        }
    }

cleanup:
    memset(&(f->filtComp.ava.value), 0, sizeof(VDIR_BERVALUE)); // Since ava.value is NOT owned by filter.
    DeleteFilter(f);
    VmDirFreeEntryContent(pGroupEntry);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);

    return retVal;

error:

    VMDIR_APPEND_ERROR_MSG(pOperation->ldapResult.pszErrMsg, pszLocalErrorMsg);
    goto cleanup;
}

int
GenerateDeleteAttrsMods(
    PVDIR_OPERATION pOperation,
    VDIR_ENTRY *    pEntry
    )
{
    int                 retVal = 0;
    VDIR_MODIFICATION * delMod = NULL;
    VDIR_ATTRIBUTE *    attr = NULL;
    ModifyReq *         modReq = &(pOperation->request.modifyReq);

    for (attr = pEntry->attrs; attr != NULL; attr = attr->next)
    {
        if (VmDirStringCompareA(attr->type.lberbv.bv_val, ATTR_DN, FALSE) == 0)
        {
            continue;
        }

        retVal = VmDirAllocateMemory(
                sizeof(VDIR_MODIFICATION), (PVOID*)&delMod);
        BAIL_ON_VMDIR_ERROR(retVal);

        delMod->operation = MOD_OP_DELETE;

        delMod->attr.next = NULL;
        delMod->attr.type = attr->type;
        delMod->attr.pATDesc = attr->pATDesc;
        delMod->attr.vals = NULL;
        delMod->attr.numVals = 0;

        delMod->next = modReq->mods;
        modReq->mods = delMod;
        modReq->numMods++;
    }

    retVal = VmDirAppendAMod(
            pOperation,
            MOD_OP_DELETE,
            ATTR_DN,
            ATTR_DN_LEN,
            pOperation->request.deleteReq.dn.lberbv.bv_val,
            pOperation->request.deleteReq.dn.lberbv.bv_len);
    BAIL_ON_VMDIR_ERROR(retVal);

cleanup:
    return retVal;

error:
    goto cleanup;
}
