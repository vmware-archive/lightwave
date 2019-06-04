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

static int
_GenerateDeleteAttrsMods(
    PVDIR_OPERATION pOperation,
    VDIR_ENTRY *    pEntry
    );

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
                dwError,
                pszLocalErrMsg,
                "Not bind/authenticate yet");
    }

    dwError = VmDirInternalDeleteEntry(pOperation);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);
    return pOperation->ldapResult.errCode;

error:
    VMDIR_SET_LDAP_RESULT_ERROR(&pOperation->ldapResult, dwError, pszLocalErrMsg);
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
    BOOLEAN     bIsTombstoneObj = FALSE;
    PSTR        pszLocalErrMsg = NULL;
    PVDIR_OPERATION_ML_METRIC  pMLMetrics = NULL;

    assert(pOperation && pOperation->pBECtx->pBE);

    pMLMetrics = &pOperation->MLMetrics;
    VMDIR_COLLECT_TIME(pMLMetrics->iMLStartTime);

    if (VmDirdState() == VMDIRD_STATE_READ_ONLY)
    {
        retVal = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "Server in read-only mode" );
    }

    // make sure we have minimum DN length
    if (delReq->dn.lberbv_len < 3)
    {
        retVal = VMDIR_ERROR_INVALID_REQUEST;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "Invalid DN length - (%u)", delReq->dn.lberbv_len);
    }

    // Normalize DN
    retVal = VmDirNormalizeDN( &(delReq->dn), pOperation->pSchemaCtx );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "DN normalization failed - (%u)(%s)",
                                  retVal, VDIR_SAFE_STRING(VmDirSchemaCtxGetErrorMsg(pOperation->pSchemaCtx)) );

    bIsTombstoneObj = VmDirIsTombStoneObject(delReq->dn.lberbv_val);

    if (!bIsTombstoneObj)
    {
        VMDIR_COLLECT_TIME(pMLMetrics->iPrePluginsStartTime);

        // Execute pre modify apply Delete plugin logic
        retVal = VmDirExecutePreModApplyDeletePlugins(pOperation, NULL, retVal);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "PreModApplyDelete plugin failed - (%u)",  retVal );

        VMDIR_COLLECT_TIME(pMLMetrics->iPrePluginsEndTime);
    }

    retVal = VmDirNormalizeMods( pOperation->pSchemaCtx, modReq->mods, &pszLocalErrMsg );
    BAIL_ON_VMDIR_ERROR( retVal );

    // make sure VDIR_BACKEND_CTX has usn change number by now
    if ( pOperation->pBECtx->wTxnUSN <= 0 && !bIsTombstoneObj)
    {
        retVal = VMDIR_ERROR_NO_USN;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "BECtx.wTxnUSN not set");
    }

    if (!bIsTombstoneObj)
    {
        VMDIR_COLLECT_TIME(pMLMetrics->iWriteQueueWaitStartTime);

        retVal = VmDirWriteQueueWait(gVmDirServerOpsGlobals.pWriteQueue, pOperation->pWriteQueueEle);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(retVal, pszLocalErrMsg, "Failed in waiting for USN dispatch");

        VMDIR_COLLECT_TIME(pMLMetrics->iWriteQueueWaitEndTime);
    }

    // BUGBUG, need to protect some system entries such as schema,domain....etc?
    {
        if (pEntry)
        {
            VmDirFreeEntryContent(pEntry);
            memset(pEntry, 0, sizeof(VDIR_ENTRY));
            pEntry = NULL;
        }

        VMDIR_COLLECT_TIME(pMLMetrics->iBETxnBeginStartTime);

        retVal = pOperation->pBEIF->pfnBETxnBegin( pOperation->pBECtx, VDIR_BACKEND_TXN_WRITE);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "txn begin (%u)(%s)",
                                      retVal, VDIR_SAFE_STRING(pOperation->pBEErrorMsg));
        bHasTxn = TRUE;
        VMDIR_COLLECT_TIME(pMLMetrics->iBETxnBeginEndTime);

        // Read current entry from DB
        retVal = pOperation->pBEIF->pfnBEDNToEntry(
                                    pOperation->pBECtx,
                                    pOperation->pSchemaCtx,
                                    &(delReq->dn),
                                    &entry,
                                    VDIR_BACKEND_ENTRY_LOCK_WRITE);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(
            retVal,
            pszLocalErrMsg,
            "(%u)(%s)",
            retVal,
            VDIR_SAFE_STRING(pOperation->pBEErrorMsg));

        pEntry = &entry;

        // Parse Parent DN
        retVal = VmDirGetParentDN( &pEntry->dn, &pEntry->pdn );
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "Get ParentDn failed - (%u)",  retVal );

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

                BAIL_ON_VMDIR_ERROR_WITH_MSG(
                    retVal,
                    pszLocalErrMsg,
                    "parent (%s) lookup failed, (%s) retVal (%u)",
                    pEntry->pdn.lberbv_val,
                    VDIR_SAFE_STRING(pOperation->pBEErrorMsg),
                    retVal);
            }

            pEntry->pParentEntry = pParentEntry;        // pEntry takes over pParentEntry
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
                retVal,
                pszLocalErrMsg,
                "VmDirSrvAccessCheck failed - (%u)(%s)",
                retVal,
                VMDIR_ACCESS_DENIED_ERROR_MSG);

        if  (bIsTombstoneObj)
        {
            // Normalize index attribute, so mdb can cleanup index tables properly.
            retVal = VmDirEntryAttrValueNormalize(pEntry, TRUE);
            BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "Attr value normalization failed - (%u)", retVal );
        }
        else
        {
            // Make sure it is a leaf node
            retVal = pOperation->pBEIF->pfnBEChkIsLeafEntry(
                                    pOperation->pBECtx,
                                    pEntry->eId,
                                    &leafNode);
            BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "BEChkIsLeafEntry failed, (%u)(%s)",
                                        retVal, VDIR_SAFE_STRING(pOperation->pBEErrorMsg) );

            if (leafNode == FALSE)
            {
                retVal = VMDIR_ERROR_NOT_ALLOWED_ON_NONLEAF;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "Delete of a non-leaf node is not allowed." );
            }

            // Retrieve to determine whether it is domain object earlier
            // before attribute modifications
            // ('bIsDomainObject' is needed for a domain object deletion)
            retVal = VmDirIsDomainObjectWithEntry(pEntry, &bIsDomainObject);
            BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg,
                                        "VmDirIsDomainObjectWithEntry failed - (%u)", retVal );

            if (pOperation->opType != VDIR_OPERATION_TYPE_REPL)
            {
                // Generate mods to delete attributes that need not be present in a DELETED entry
                // Note: in case of executing the deadlock while loop multiple times, same attribute Delete mod be added
                // multiple times in the modReq, which is expected to work correctly.
                retVal = _GenerateDeleteAttrsMods( pOperation, pEntry );
                BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "_GenerateDeleteAttrsMods failed - (%u)", retVal);

                // Generate new meta-data for the attributes being updated
                retVal = VmDirGenerateModsNewMetaData(pOperation, modReq->mods, pEntry->eId);
                BAIL_ON_VMDIR_ERROR(retVal);
            }

            // Normalize attribute values in mods
            retVal = VmDirNormalizeMods( pOperation->pSchemaCtx, modReq->mods, &pszLocalErrMsg );
            BAIL_ON_VMDIR_ERROR( retVal );

            // Apply modify operations to the current entry in the DB.
            retVal = VmDirApplyModsToEntryStruct( pOperation->pSchemaCtx, modReq, pEntry, NULL, &pszLocalErrMsg,
                                                  pOperation->opType == VDIR_OPERATION_TYPE_REPL);
            BAIL_ON_VMDIR_ERROR( retVal );
        }

        // Update DBs

        // Update Entry
        retVal = pOperation->pBEIF->pfnBEEntryDelete(
                    pOperation->pBECtx,
                    bIsTombstoneObj ? NULL : modReq->mods,
                    pEntry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(
            retVal,
            pszLocalErrMsg,
            "BEEntryDelete (%u)(%s)",
            retVal,
            VDIR_SAFE_STRING(pOperation->pBEErrorMsg));

        // Use normalized DN value
        if (bIsDomainObject)
        {
            retVal = VmDirInternalRemoveOrgConfig(pOperation,
                                                  BERVAL_NORM_VAL(pEntry->dn));
            BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "Update domain list entry failed." );
        }

        retVal = pOperation->pBEIF->pfnBEDeleteAllAttrValueMetaData(pOperation->pBECtx, pOperation->pSchemaCtx, pEntry->eId);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(
            retVal,
            pszLocalErrMsg,
            "BEEntryDelete pfnBEDeleteAllAttrValueMetaData error: (%u)(%s)",
            retVal,
            VDIR_SAFE_STRING(pOperation->pBEErrorMsg));

        VMDIR_COLLECT_TIME(pMLMetrics->iBETxnCommitStartTime);

        retVal = pOperation->pBEIF->pfnBETxnCommit( pOperation->pBECtx);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "txn commit (%u)(%s)",
                                              retVal, VDIR_SAFE_STRING(pOperation->pBEErrorMsg));
        bHasTxn = FALSE;
        VMDIR_COLLECT_TIME(pMLMetrics->iBETxnCommitEndTime);

        if (pOperation->opType != VDIR_OPERATION_TYPE_REPL)
        {
            // update max orig usn
            pOperation->pBEIF->pfnBESetMaxOriginatingUSN(
                    pOperation->pBECtx, pOperation->pBECtx->wTxnUSN);
        }
    }

    gVmdirGlobals.dwLdapWrites++;

    VmDirAuditWriteOp(pOperation, VDIR_SAFE_STRING(pEntry->dn.lberbv_val), pEntry);

    // Post delete entry
    // TODO, make it into a separate file deletePlugin.c
    // clean lockout cache record if exists
    VdirLockoutCacheRemoveRec(pEntry->dn.bvnorm_val);

cleanup:
    if (!bIsTombstoneObj)
    {
        VmDirWriteQueuePop(gVmDirServerOpsGlobals.pWriteQueue, pOperation->pWriteQueueEle);
    }

    {
        int iPostCommitPluginRtn  = 0;

        if (!bIsTombstoneObj)
        {
            VMDIR_COLLECT_TIME(pMLMetrics->iPostPluginsStartTime);

            // Execute post Delete commit plugin logic
            iPostCommitPluginRtn = VmDirExecutePostDeleteCommitPlugins(pOperation, pEntry, retVal);
            if (iPostCommitPluginRtn != LDAP_SUCCESS &&
                iPostCommitPluginRtn != pOperation->ldapResult.errCode)
            {
                VMDIR_LOG_INFO(
                        LDAP_DEBUG_ANY,
                        "%s: VdirExecutePostDeleteCommitPlugins - code(%d)",
                        __FUNCTION__,
                        iPostCommitPluginRtn);
            }

            VMDIR_COLLECT_TIME(pMLMetrics->iPostPluginsEndTime);
        }
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

static
DWORD
constructDeletedObjDN(
    VDIR_BERVALUE *    dn,
    const char *       objectGuidStr,
    VDIR_BERVALUE *    deletedObjDN
    )
{
    DWORD           dwError = 0;
    VDIR_BERVALUE   parentDN = VDIR_BERVALUE_INIT;
    size_t          deletedObjDNLen = 0;
    size_t          delObjsConatinerDNLength = gVmdirServerGlobals.delObjsContainerDN.lberbv.bv_len;
    char *          delObjsConatinerDN = gVmdirServerGlobals.delObjsContainerDN.lberbv.bv_val;

    deletedObjDN->lberbv.bv_val = NULL;
    deletedObjDN->lberbv.bv_len = 0;

    if (!delObjsConatinerDN)
    {
        dwError = VMDIR_ERROR_ENTRY_NOT_FOUND;
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    dwError = VmDirGetParentDN( dn, &parentDN );
    BAIL_ON_VMDIR_ERROR( dwError );

    // Format of the DN of a deleted object is:
    //     <original RDN>#objectGUID:<object GUID string>,<DN of the Deleted objects container>

    deletedObjDNLen = (parentDN.lberbv.bv_len ? dn->lberbv.bv_len - parentDN.lberbv.bv_len - 1 /* Count out RDN separator */ : dn->lberbv.bv_len)
                      + 1 /* for # */ + ATTR_OBJECT_GUID_LEN + 1 /* for : */ + VmDirStringLenA( objectGuidStr )
                      + 1 /* for , */ + delObjsConatinerDNLength;

    dwError = VmDirAllocateMemory( deletedObjDNLen + 1, (PVOID *)&deletedObjDN->lberbv.bv_val );
    BAIL_ON_VMDIR_ERROR( dwError );

    deletedObjDN->lberbv.bv_len = parentDN.lberbv.bv_len ? dn->lberbv.bv_len - parentDN.lberbv.bv_len - 1 /* Count out RDN separator */
                                            : dn->lberbv.bv_len;

    // TODO: how do we know the actual buffer size ?
    dwError = VmDirCopyMemory( deletedObjDN->lberbv.bv_val, deletedObjDN->lberbv.bv_len, dn->lberbv.bv_val, deletedObjDN->lberbv.bv_len );
    BAIL_ON_VMDIR_ERROR( dwError );

    deletedObjDN->lberbv.bv_val[deletedObjDN->lberbv.bv_len] = '#';
    deletedObjDN->lberbv.bv_len++;

    // TODO: how do we know the actual buffer size ?
    dwError = VmDirCopyMemory( deletedObjDN->lberbv.bv_val + deletedObjDN->lberbv.bv_len, ATTR_OBJECT_GUID_LEN, ATTR_OBJECT_GUID,
                               ATTR_OBJECT_GUID_LEN );
    BAIL_ON_VMDIR_ERROR( dwError );

    deletedObjDN->lberbv.bv_len += ATTR_OBJECT_GUID_LEN;
    deletedObjDN->lberbv.bv_val[deletedObjDN->lberbv.bv_len] = ':';
    deletedObjDN->lberbv.bv_len++;

    // TODO: how do we know the actual buffer size ?
    dwError = VmDirCopyMemory( deletedObjDN->lberbv.bv_val + deletedObjDN->lberbv.bv_len, VmDirStringLenA( objectGuidStr ),
                               (PVOID)objectGuidStr, VmDirStringLenA( objectGuidStr ) );
    BAIL_ON_VMDIR_ERROR( dwError );

    deletedObjDN->lberbv.bv_len += VmDirStringLenA( objectGuidStr );

    // TODO: how do we know the actual buffer size ?
    VmDirStringPrintFA( deletedObjDN->lberbv.bv_val + deletedObjDN->lberbv.bv_len, delObjsConatinerDNLength + 2, ",%s",
                        delObjsConatinerDN );

    deletedObjDN->lberbv.bv_len += delObjsConatinerDNLength + 1 /* for , */;

cleanup:
    VmDirFreeBervalContent( &parentDN );

    return dwError;

error:
    if (deletedObjDN->lberbv.bv_val != NULL)
    {
        VmDirFreeMemory( deletedObjDN->lberbv.bv_val );
        deletedObjDN->lberbv.bv_val = NULL;
    }
    deletedObjDN->lberbv.bv_len = 0;
    goto cleanup;
}

static int
_GenerateDeleteAttrsMods(
    PVDIR_OPERATION pOperation,
    PVDIR_ENTRY     pEntry
    )
{
    int                 retVal = 0;
    PVDIR_MODIFICATION  pDelMod = NULL;
    PVDIR_ATTRIBUTE     pAttr = NULL;
    PVDIR_ATTRIBUTE     pObjectGuidAttr = NULL;
    VDIR_BERVALUE       bvDeletedObjDN = VDIR_BERVALUE_INIT;
    ModifyReq*          pModReq = &(pOperation->request.modifyReq);

    for (pAttr = pEntry->attrs; pAttr != NULL; pAttr = pAttr->next)
    {
        // Retain the following kind of attributes
        if (pAttr->pATDesc->usage != VDIR_LDAP_USER_APPLICATIONS_ATTRIBUTE ||
            VmDirStringCompareA(pAttr->type.lberbv.bv_val, ATTR_OBJECT_CLASS, FALSE) == 0 ||
            VmDirStringCompareA(pAttr->type.lberbv.bv_val, ATTR_OBJECT_SECURITY_DESCRIPTOR, FALSE ) == 0)
        {
            continue;
        }

        retVal = VmDirAllocateMemory(
                sizeof(VDIR_MODIFICATION), (PVOID*)&pDelMod);
        BAIL_ON_VMDIR_ERROR(retVal);

        pDelMod->operation = MOD_OP_DELETE;

        pDelMod->attr.next = NULL;
        pDelMod->attr.type = pAttr->type;
        pDelMod->attr.pATDesc = pAttr->pATDesc;
        pDelMod->attr.vals = NULL;
        pDelMod->attr.numVals = 0;

        pDelMod->next = pModReq->mods;
        pModReq->mods = pDelMod;
        pModReq->numMods++;
    }

    // Add mod to set new DN.
    pObjectGuidAttr = VmDirEntryFindAttribute(ATTR_OBJECT_GUID, pEntry);
    assert(pObjectGuidAttr);

    retVal = constructDeletedObjDN(
            &pOperation->request.deleteReq.dn,
            pObjectGuidAttr->vals[0].lberbv.bv_val,
            &bvDeletedObjDN);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirAppendAMod(
            pOperation,
            MOD_OP_REPLACE,
            ATTR_DN,
            ATTR_DN_LEN,
            bvDeletedObjDN.lberbv.bv_val,
            bvDeletedObjDN.lberbv.bv_len);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirAppendAMod(
            pOperation,
            MOD_OP_REPLACE,
            ATTR_OBJECT_CLASS,
            ATTR_OBJECT_CLASS_LEN,
            OC_DELETED_OBJECT,
            OC_DELETED_OBJECT_LEN);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirAppendAMod(
            pOperation,
            MOD_OP_REPLACE,
            ATTR_OBJECT_SECURITY_DESCRIPTOR,
            ATTR_OBJECT_SECURITY_DESCRIPTOR_LEN,
            (PSTR) gVmdirdSDGlobals.pSDdcAdminRPWPDE,
            gVmdirdSDGlobals.ulSDdcAdminRPWPDELen);
    BAIL_ON_VMDIR_ERROR(retVal);

cleanup:
    VmDirFreeMemory(bvDeletedObjDN.lberbv.bv_val);
    return retVal;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: error %d", __FUNCTION__, retVal);
    goto cleanup;
}
