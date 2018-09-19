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



/*
 * Module Name: Replicate entries
 *
 * Filename: replentry.c
 *
 * Abstract:
 *
 */

#include "includes.h"

static
int
SetupReplModifyRequest(
    PVDIR_OPERATION              pModOp,
    PVMDIR_REPLICATION_UPDATE    pUpdate
    );

static
int
_VmDirPatchData(
    PVDIR_OPERATION     pOperation
    );

static
int
ReplFixUpEntryDn(
    PVDIR_ENTRY pEntry
    );

static
VOID
_VmDirLogReplAddEntryContent(
    PVMDIR_REPLICATION_UPDATE     pUpdate
    );

static
VOID
_VmDirLogReplModifyEntryContent(
    PVMDIR_REPLICATION_UPDATE     pUpdate
    );

static
VOID
_VmDirLogReplDeleteEntryContent(
    PVMDIR_REPLICATION_UPDATE     pUpdate
    );

static
VOID
_VmDirLogReplEntryContent(
    PVDIR_ENTRY                   pEntry
    );

static
VOID
_VmDirLogReplModifyModContent(
    ModifyReq*  pModReq
    );

static
VOID
_VmDirReplModifyClearAllMods(
    ModifyReq*    pModifyReq
    );

// Replicate Add Entry operation
int
ReplAddEntry(
    PVMDIR_REPLICATION_UPDATE       pUpdate
    )
{
    int                          retVal = LDAP_SUCCESS;
    VDIR_OPERATION               op = {0};
    PVDIR_ATTRIBUTE              pAttr = NULL;
    PVDIR_ENTRY                  pEntry = NULL;
    USN                          localUsn = 0;
    PSTR                         pszAttrType = NULL;
    PVDIR_ATTRIBUTE              pAttrAttrMetaData = NULL;
    PLW_HASHMAP                  pMetaDataMap = NULL;
    LW_HASHMAP_ITER              iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR              pair = {NULL, NULL};
    PVMDIR_ATTRIBUTE_METADATA    pSupplierMetaData = NULL;
    DEQUE                        valueMetaDataQueue = {0};

    _VmDirLogReplAddEntryContent(pUpdate);

    retVal = VmDirInitStackOperation(&op, VDIR_OPERATION_TYPE_REPL, LDAP_REQ_ADD, NULL);
    BAIL_ON_VMDIR_ERROR(retVal);

    pEntry = op.request.addReq.pEntry = pUpdate->pEntry;
    pUpdate->pEntry = NULL;

    pEntry->pSchemaCtx = VmDirSchemaCtxClone(op.pSchemaCtx);

    // Make sure Attribute has its ATDesc set
    retVal = VmDirSchemaCheckSetAttrDesc(pEntry->pSchemaCtx, pEntry);
    BAIL_ON_VMDIR_ERROR(retVal);

    op.pBEIF = VmDirBackendSelect(pEntry->dn.lberbv.bv_val);
    assert(op.pBEIF);

    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "ReplAddEntry: next entry being replicated/Added is: %s", pEntry->dn.lberbv.bv_val);

    // Set local attributes.
    if((retVal = VmDirWriteQueuePush(
                    op.pBECtx,
                    gVmDirServerOpsGlobals.pWriteQueue,
                    op.pWriteQueueEle)) != 0)
    {
        VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s: failed with error code: %d, error string: %s",
            __FUNCTION__,
            retVal,
            VDIR_SAFE_STRING(op.pBEErrorMsg));
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    localUsn = op.pWriteQueueEle->usn;

    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "%s: next generated USN: %" PRId64, __FUNCTION__, localUsn);

    retVal = VmDirAttributeValueMetaDataListConvertToDequeue(
            pUpdate->pValueMetaDataList, &valueMetaDataQueue);
    BAIL_ON_VMDIR_ERROR(retVal);

    // need these before DetectAndResolveAttrsConflicts
    op.pszPartner = pUpdate->pszPartner;
    op.ulPartnerUSN = pUpdate->partnerUsn;

    retVal = VmDirAttributeMetaDataListConvertToHashMap(pUpdate->pMetaDataList, &pMetaDataMap);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirReplSetAttrNewMetaData(&op, pEntry, pMetaDataMap);
    BAIL_ON_VMDIR_ERROR(retVal);

    // Creating deleted object scenario: Create attributes just with attribute meta data, and no values.
    while (LwRtlHashMapIterate(pMetaDataMap, &iter, &pair))
    {
        pszAttrType = (PSTR) pair.pKey;
        pSupplierMetaData = (PVMDIR_ATTRIBUTE_METADATA) pair.pValue;

        pair.pKey = pair.pValue = NULL;
        retVal = LwRtlHashMapRemove(pMetaDataMap, pszAttrType, &pair);
        BAIL_ON_VMDIR_ERROR(retVal);

        retVal = VmDirAttributeAllocate(pszAttrType, 0, pEntry->pSchemaCtx, &pAttr);
        BAIL_ON_VMDIR_ERROR(retVal);

        // Set localUsn in the metaData
        retVal = VmDirMetaDataSetLocalUsn(pSupplierMetaData, localUsn);
        BAIL_ON_VMDIR_ERROR(retVal);

        pAttr->pMetaData = pSupplierMetaData;
        pair.pValue = NULL;

        pAttr->next = pEntry->attrs;
        pEntry->attrs = pAttr;

        VmDirFreeMetaDataMapPair(&pair, NULL);
    }

    retVal = VmDirValueMetaDataUpdateLocalUsn(pEntry, localUsn, &valueMetaDataQueue);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = _VmDirPatchData( &op );
    BAIL_ON_VMDIR_ERROR( retVal );

    if ((retVal = VmDirInternalAddEntry( &op )) != LDAP_SUCCESS)
    {
        // Reset retVal to LDAP level error space (for B/C)
        PSZ_METADATA_BUF    pszMetaData = {'\0'};

        retVal = op.ldapResult.errCode;

        //Ignore error - used only for logging
        VmDirMetaDataSerialize(pEntry->attrs->pMetaData, pszMetaData);

        switch (retVal)
        {
            case LDAP_ALREADY_EXISTS:
                VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
                          "ReplAddEntry/VmDirInternalAddEntry: %d (Object already exists). "
                          "DN: %s, first attribute: %s, it's meta data: '%s' "
                          "NOT resolving this possible replication CONFLICT or initial objects creation scenario. "
                          "For this object, system may not converge. Partner USN %" PRId64,
                          retVal, pEntry->dn.lberbv.bv_val, pEntry->attrs->type.lberbv.bv_val, pszMetaData,
                          pUpdate->partnerUsn);

                break;

            case LDAP_NO_SUCH_OBJECT:
                VMDIR_LOG_WARNING(LDAP_DEBUG_REPL,
                          "ReplAddEntryVmDirInternalAddEntry: %d (Parent object does not exist). "
                          "DN: %s, first attribute: %s, it's meta data: '%s' "
                          "NOT resolving this possible replication CONFLICT or out-of-parent-child-order replication scenario. "
                          "For this subtree, system may not converge. Partner USN %" PRId64,
                          retVal, pEntry->dn.lberbv.bv_val, pEntry->attrs->type.lberbv.bv_val, pszMetaData,
                          pUpdate->partnerUsn);
                break;

            default:
                VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                          "ReplAddEntry/VmDirInternalAddEntry:  %d (%s). Partner USN %" PRId64,
                          retVal, VDIR_SAFE_STRING( op.ldapResult.pszErrMsg ), pUpdate->partnerUsn);
                break;
        }
        BAIL_ON_VMDIR_ERROR( retVal );
    }

cleanup:
    if (pMetaDataMap)
    {
        LwRtlHashMapClear(
                pMetaDataMap,
                VmDirFreeMetaDataMapPair,
                NULL);
        LwRtlFreeHashMap(&pMetaDataMap);
    }
    // pAttrAttrMetaData is local, needs to be freed within the call
    VmDirFreeAttribute(pAttrAttrMetaData);
    VmDirFreeAttrValueMetaDataDequeueContent(&valueMetaDataQueue);
    VmDirFreeOperationContent(&op);

    return retVal;

error:
    goto cleanup;
} // Replicate Add Entry operation

/* Replicate Delete Entry operation
 * Set modifications associated with a Delete operation, and pass-in the modifications, with correct attribute meta data
 * set, to InternalDeleteEntry function, which will apply the mods to the existing entry, and move the object to the
 * DeletedObjects container.
 */
int
ReplDeleteEntry(
    PVMDIR_REPLICATION_UPDATE       pUpdate
    )
{
    int                 retVal = LDAP_SUCCESS;
    VDIR_OPERATION      delOp = {0};
    ModifyReq *         mr = &(delOp.request.modifyReq);

    _VmDirLogReplDeleteEntryContent(pUpdate);

    retVal = VmDirInitStackOperation(&delOp, VDIR_OPERATION_TYPE_REPL, LDAP_REQ_DELETE, NULL);
    BAIL_ON_VMDIR_ERROR(retVal);
   /*
    * Encode entry requires schema description
    * hence perform encode entry after VmDirSchemaCheckSetAttrDesc
    */
    retVal = VmDirSchemaCheckSetAttrDesc(delOp.pSchemaCtx, pUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = ReplFixUpEntryDn(pUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR(retVal);

    if (VmDirBervalContentDup( &pUpdate->pEntry->dn, &mr->dn ) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "ReplDeleteEntry: BervalContentDup failed." );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    delOp.pBEIF = VmDirBackendSelect(mr->dn.lberbv.bv_val);
    assert(delOp.pBEIF);

    // need these before DetectAndResolveAttrsConflicts
    delOp.pszPartner = pUpdate->pszPartner;
    delOp.ulPartnerUSN = pUpdate->partnerUsn;

    if((retVal = VmDirWriteQueuePush(
                    delOp.pBECtx,
                    gVmDirServerOpsGlobals.pWriteQueue,
                    delOp.pWriteQueueEle)) != 0)
    {
        VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s: failed with error code: %d, error string: %s",
            __FUNCTION__,
            retVal,
            VDIR_SAFE_STRING(delOp.pBEErrorMsg));
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    // SJ-TBD: What about if one or more attributes were meanwhile added to the entry? How do we purge them?
    retVal = SetupReplModifyRequest(&delOp, pUpdate);
    BAIL_ON_VMDIR_ERROR(retVal);

    // SJ-TBD: What happens when DN of the entry has changed in the meanwhile? => conflict resolution.
    // Should objectGuid, instead of DN, be used to uniquely identify an object?
    if ((retVal = VmDirInternalDeleteEntry( &delOp )) != LDAP_SUCCESS)
    {
        PSZ_METADATA_BUF    pszMetaData = {'\0'};

        retVal = delOp.ldapResult.errCode;

        //Ignore error - used only for logging
        VmDirMetaDataSerialize(mr->mods->attr.pMetaData, pszMetaData);
        // If VmDirInternall call failed, reset retVal to LDAP level error space (for B/C)
        switch (retVal)
        {
            case LDAP_NO_SUCH_OBJECT:
                VMDIR_LOG_WARNING(LDAP_DEBUG_REPL,
                          "ReplDeleteEntry/VmDirInternalDeleteEntry: %d (Object does not exist). "
                          "DN: %s, first attribute: %s, it's meta data: '%s'. "
                          "NOT resolving this possible replication CONFLICT. "
                          "For this object, system may not converge. Partner USN %" PRId64,
                          retVal, mr->dn.lberbv.bv_val, mr->mods->attr.type.lberbv.bv_val, pszMetaData,
                          pUpdate->partnerUsn);
                break;

            case LDAP_NOT_ALLOWED_ON_NONLEAF:
                VMDIR_LOG_WARNING(LDAP_DEBUG_REPL,
                          "ReplDeleteEntry/VmDirInternalDeleteEntry: %d (Operation not allowed on non-leaf). "
                          "DN: %s, first attribute: %s, it's meta data: '%s'. "
                          "NOT resolving this possible replication CONFLICT. "
                          "For this object, system may not converge. Partner USN %" PRId64,
                          retVal, mr->dn.lberbv.bv_val, mr->mods->attr.type.lberbv.bv_val, pszMetaData,
                          pUpdate->partnerUsn);
                break;

            case LDAP_NO_SUCH_ATTRIBUTE:
                VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
                          "ReplDeleteEntry/VmDirInternalDeleteEntry: %d (No such attribute). "
                          "DN: %s, first attribute: %s, it's meta data: '%s'. "
                          "NOT resolving this possible replication CONFLICT. "
                          "For this object, system may not converge. Partner USN %" PRId64,
                          retVal, mr->dn.lberbv.bv_val, mr->mods->attr.type.lberbv.bv_val, pszMetaData,
                          pUpdate->partnerUsn);
                break;

            default:
                VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                          "ReplDeleteEntry/InternalDeleteEntry: %d (%s). Partner USN %" PRId64,
                          retVal, VDIR_SAFE_STRING( delOp.ldapResult.pszErrMsg ),pUpdate->partnerUsn);
                break;
        }
        BAIL_ON_VMDIR_ERROR( retVal );
    }

cleanup:
    VmDirFreeModifyRequest( mr, FALSE );
    VmDirFreeOperationContent(&delOp);

    return retVal;

error:
    goto cleanup;
} // Replicate Delete entry operation

// Replicate Modify Entry operation
int
ReplModifyEntry(
    PVMDIR_REPLICATION_UPDATE   pUpdate
    )
{
    int                 retVal = LDAP_SUCCESS;
    int                 dbRetVal = 0;
    VDIR_OPERATION      modOp = {0};
    ModifyReq *         mr = &(modOp.request.modifyReq);
    BOOLEAN             bHasTxn = FALSE;
    VDIR_BERVALUE       bvParentDn = VDIR_BERVALUE_INIT;
    ENTRYID             entryId = 0;
    DEQUE               valueMetaDataQueue = {0};

    _VmDirLogReplModifyEntryContent(pUpdate);

    retVal = VmDirInitStackOperation(&modOp,
                                     VDIR_OPERATION_TYPE_REPL,
                                     LDAP_REQ_MODIFY,
                                     NULL);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirGetParentDN(&pUpdate->pEntry->dn, &bvParentDn);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = ReplFixUpEntryDn(pUpdate->pEntry);
    BAIL_ON_VMDIR_ERROR( retVal );

    retVal = VmDirBervalContentDup(&pUpdate->pEntry->dn, &mr->dn);
    if (retVal)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL, "%s: BervalContentDup failed, Error: %d", __FUNCTION__, retVal);
        BAIL_WITH_VMDIR_ERROR(retVal, LDAP_OPERATIONS_ERROR);
    }

    // This is strict locking order:
    // Must acquire schema modification mutex before backend write txn begins
    retVal = VmDirSchemaModMutexAcquire(&modOp);
    BAIL_ON_VMDIR_ERROR(retVal);

    modOp.pBEIF = VmDirBackendSelect(mr->dn.lberbv.bv_val);
    assert(modOp.pBEIF);

    retVal = VmDirWriteQueuePush(
                    modOp.pBECtx,
                    gVmDirServerOpsGlobals.pWriteQueue,
                    modOp.pWriteQueueEle);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirWriteQueueWait(gVmDirServerOpsGlobals.pWriteQueue, modOp.pWriteQueueEle);
    BAIL_ON_VMDIR_ERROR(retVal);

    //Transaction needed to process existing/local attribute meta data.
    dbRetVal = modOp.pBEIF->pfnBETxnBegin(modOp.pBECtx, VDIR_BACKEND_TXN_WRITE);
    BAIL_ON_VMDIR_ERROR(dbRetVal);

    bHasTxn = TRUE;

    if (mr->dn.bvnorm_val == NULL)
    {
        retVal = VmDirNormalizeDN(&mr->dn, modOp.pSchemaCtx);
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    // Get EntryId
    retVal = modOp.pBEIF->pfnBEDNToEntryId(modOp.pBECtx, &mr->dn, &entryId);
    if (retVal != 0)
    {
        switch (retVal)
        {
            case ERROR_BACKEND_ENTRY_NOTFOUND:
                VMDIR_LOG_ERROR(
                        VMDIR_LOG_MASK_ALL,
                        "%s: entry %s doesn't exist error code %d",
                        __FUNCTION__,
                        VDIR_SAFE_STRING(mr->dn.bvnorm_val),
                        retVal);
                break;
            default:
                VMDIR_LOG_ERROR(
                        VMDIR_LOG_MASK_ALL,
                        "%s: pfnBEDNToEntryId failed dn %s error code %d",
                        __FUNCTION__,
                        VDIR_SAFE_STRING(mr->dn.bvnorm_val),
                        retVal);
                break;
        }
    }
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirAttributeValueMetaDataListConvertToDequeue(
            pUpdate->pValueMetaDataList, &valueMetaDataQueue);
    BAIL_ON_VMDIR_ERROR(retVal);

    // need these before DetectAndResolveAttrsConflicts
    modOp.pszPartner = pUpdate->pszPartner;
    modOp.ulPartnerUSN = pUpdate->partnerUsn;

    pUpdate->pEntry->eId = entryId;

    retVal = SetupReplModifyRequest(&modOp, pUpdate);
    if (retVal != LDAP_SUCCESS)
    {
        PSZ_METADATA_BUF    pszMetaData = {'\0'};

        //Ignore error - used only for logging
        VmDirMetaDataSerialize(pUpdate->pEntry->attrs[0].pMetaData, pszMetaData);

        switch (retVal)
        {
            case LDAP_NO_SUCH_OBJECT:
                VMDIR_LOG_WARNING(
                        LDAP_DEBUG_REPL,
                        "%s: %d (Object does not exist). "
                        "DN: %s, first attribute: %s, it's meta data: '%s'. "
                        "Possible replication CONFLICT. Object will get deleted from the system."
                        "Partner USN %" PRId64,
                        __FUNCTION__,
                        retVal,
                        pUpdate->pEntry->dn.lberbv.bv_val,
                        pUpdate->pEntry->attrs[0].type.lberbv.bv_val,
                        pszMetaData,
                        pUpdate->partnerUsn);
                break;

            default:
                VMDIR_LOG_ERROR(
                        VMDIR_LOG_MASK_ALL,
                        "%s: %d (%s). Partner USN %" PRId64,
                        __FUNCTION__,
                        retVal,
                        VDIR_SAFE_STRING(modOp.ldapResult.pszErrMsg),
                        pUpdate->partnerUsn);
                break;
        }
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    // If some mods left after conflict resolution
    if (mr->mods != NULL)
    {
        retVal = VmDirValueMetaDataDeleteOldForReplace(&modOp, mr->mods, entryId);
        BAIL_ON_VMDIR_ERROR(retVal);

        _VmDirLogReplModifyModContent(&modOp.request.modifyReq);

        // SJ-TBD: What happens when DN of the entry has changed in the meanwhile?
        // =>conflict resolution.
        // Should objectGuid, instead of DN, be used to uniquely identify an object?
        retVal = VmDirInternalModifyEntry(&modOp);
        if (retVal != LDAP_SUCCESS)
        {
            // If VmDirInternall call failed, reset retVal to LDAP level error space (for B/C)
            retVal = modOp.ldapResult.errCode;

            VMDIR_LOG_ERROR(
                    VMDIR_LOG_MASK_ALL,
                    "%s: InternalModifyEntry Error: %d, error string %s",
                    __FUNCTION__,
                    retVal,
                    VDIR_SAFE_STRING(modOp.ldapResult.pszErrMsg));

            BAIL_ON_VMDIR_ERROR(retVal);
        }
    }

    if (!dequeIsEmpty(&valueMetaDataQueue))
    {
        _VmDirReplModifyClearAllMods(&modOp.request.modifyReq);

        if (modOp.request.modifyReq.newdn.lberbv_val)
        {   // modrdn case, fix modifyReq.dn for multi-value attrs mod op.
            if (VmDirBervalContentDup(&mr->newdn, &mr->dn ) != 0)
            {
                BAIL_WITH_VMDIR_ERROR(retVal, LDAP_OPERATIONS_ERROR);
            }
        }

        retVal = VmDirReplSetAttrNewValueMetaData(
                &valueMetaDataQueue, modOp.pSchemaCtx, entryId, modOp.pWriteQueueEle->usn, &modOp);
        BAIL_ON_VMDIR_ERROR(retVal);

        mr = &(modOp.request.modifyReq);
        if (mr->mods != NULL)
        {
            _VmDirLogReplModifyModContent(&modOp.request.modifyReq);

            retVal = VmDirInternalModifyEntry(&modOp);
            if (retVal != LDAP_SUCCESS)
            {
                retVal = modOp.ldapResult.errCode;
                BAIL_ON_VMDIR_ERROR(retVal);
            }
        }
    }

    dbRetVal = modOp.pBEIF->pfnBETxnCommit(modOp.pBECtx);
    if (dbRetVal)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "%s: pfnBETxnCommit error: %d, error string: %s",
                __FUNCTION__,
                dbRetVal,
                VDIR_SAFE_STRING(modOp.pBEErrorMsg));
        BAIL_WITH_VMDIR_ERROR(retVal, LDAP_OPERATIONS_ERROR);
    }

cleanup:
    VmDirWriteQueuePop(gVmDirServerOpsGlobals.pWriteQueue, modOp.pWriteQueueEle);

    // Release schema modification mutex
    VmDirFreeBervalContent(&bvParentDn);
    (VOID)VmDirSchemaModMutexRelease(&modOp);
    VmDirFreeOperationContent(&modOp);
    VmDirFreeAttrValueMetaDataDequeueContent(&valueMetaDataQueue);

    return retVal;

error:
    if (bHasTxn)
    {
        modOp.pBEIF->pfnBETxnAbort(modOp.pBECtx);
    }

    goto cleanup;
} // Replicate Modify Entry operation

/*
 * pEntry is off the wire from the supplier and the object it
 * represents may not have the same DN on this consumer.
 * The object GUID will be used to search the local system and
 * determine the local DN that is being used and adjust pEntry
 * to use the same local DN.
 */
static
int
ReplFixUpEntryDn(
    PVDIR_ENTRY pEntry
    )
{
    int                 retVal = LDAP_SUCCESS;
    VDIR_ENTRY_ARRAY    entryArray = {0};
    PVDIR_ATTRIBUTE     currAttr = NULL;
    PVDIR_ATTRIBUTE     prevAttr = NULL;
    PVDIR_ATTRIBUTE     pAttrObjectGUID = NULL;

    // Remove object GUID from list of attributes
    for (prevAttr = NULL, currAttr = pEntry->attrs; currAttr;
         prevAttr = currAttr, currAttr = currAttr->next)
    {
        if (VmDirStringCompareA( currAttr->type.lberbv.bv_val, ATTR_OBJECT_GUID, FALSE ) == 0)
        {
            if (prevAttr == NULL)
            {
                pEntry->attrs = currAttr->next;
            }
            else
            {
                prevAttr->next = currAttr->next;
            }
            pAttrObjectGUID = currAttr;
            break;
        }
    }

    if (!pAttrObjectGUID)
    {
        // Older replication partner does not send object GUID, no fixup needed
        goto cleanup;
    }

    retVal = VmDirSimpleEqualFilterInternalSearch("", LDAP_SCOPE_SUBTREE, ATTR_OBJECT_GUID, pAttrObjectGUID->vals[0].lberbv.bv_val, &entryArray);
    BAIL_ON_VMDIR_ERROR( retVal );

    if (entryArray.iSize != 1)
    {
        // object guid not found - entry missing or object GUID mismatch
        VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL,
                "%s got no result from object GUID lookup."
                "Entry (%s) missing or object GUID mismatch",
                __FUNCTION__, pEntry->dn.lberbv_val );
        goto cleanup;
    }

    if (VmDirStringCompareA(entryArray.pEntry[0].dn.lberbv_val, pEntry->dn.lberbv_val, FALSE) == 0)
    {
        // Remote and local object have same DN, no fixup needed
        goto cleanup;
    }

    retVal = VmDirBervalContentDup(&entryArray.pEntry[0].dn, &pEntry->dn);
    BAIL_ON_VMDIR_ERROR(retVal);

cleanup:
    VmDirFreeAttribute( pAttrObjectGUID );
    VmDirFreeEntryArrayContent(&entryArray);
    return retVal;

error:

    goto cleanup;
}

static
int
_VmDirPatchData(
    PVDIR_OPERATION     pOperation
    )
{
    int                 retVal = LDAP_SUCCESS;
    VDIR_ATTRIBUTE *    currAttr = NULL;
    VDIR_ATTRIBUTE *    prevAttr = NULL;
    PVDIR_ENTRY         pEntry = NULL;
    size_t              containerOCLen = VmDirStringLenA( OC_CONTAINER );
    int                 i = 0;

    pEntry = pOperation->request.addReq.pEntry;

    for (prevAttr = NULL, currAttr = pEntry->attrs; currAttr;
         prevAttr = currAttr, currAttr = currAttr->next)
    {
        // map attribute vmwSecurityDescriptor => nTSecurityDescriptor
        if (VmDirStringCompareA( currAttr->type.lberbv.bv_val, ATTR_VMW_OBJECT_SECURITY_DESCRIPTOR, FALSE ) == 0)
        {
            currAttr->type.lberbv.bv_val = ATTR_OBJECT_SECURITY_DESCRIPTOR;
            currAttr->type.lberbv.bv_len = ATTR_OBJECT_SECURITY_DESCRIPTOR_LEN;
            continue;
        }
        // map object class value vmwContainer => container
        if (VmDirStringCompareA( currAttr->type.lberbv.bv_val, ATTR_OBJECT_CLASS, FALSE ) == 0)
        {
            for (i = 0; currAttr->vals[i].lberbv.bv_val != NULL; i++)
            {
                if (VmDirStringCompareA( currAttr->vals[i].lberbv.bv_val, OC_VMW_CONTAINER, FALSE ) == 0)
                {
                    retVal = VmDirAllocateMemory( containerOCLen + 1, (PVOID*)&currAttr->vals[i].lberbv.bv_val );
                    BAIL_ON_VMDIR_ERROR(retVal);

                    retVal = VmDirCopyMemory( currAttr->vals[i].lberbv.bv_val, containerOCLen + 1, OC_CONTAINER,
                                              containerOCLen );
                    BAIL_ON_VMDIR_ERROR(retVal);

                    currAttr->vals[i].lberbv.bv_len = containerOCLen;
                    currAttr->vals[i].bOwnBvVal = TRUE;

                    break;
                }
            }
            continue;
        }
        // remove vmwOrganizationGuid attribute
        if (VmDirStringCompareA( currAttr->type.lberbv.bv_val, ATTR_VMW_ORGANIZATION_GUID, FALSE ) == 0)
        { // Remove "vmwOrganizationGuid" attribute from the list.
            if (prevAttr == NULL)
            {
                pEntry->attrs = currAttr->next;
            }
            else
            {
                prevAttr->next = currAttr->next;
            }
            continue;
        }
    }

cleanup:
    return retVal;

error:
    goto cleanup;
}



/* Create modify request corresponding to the given entry. Main steps are:
 *  - Create replace mods for the "local" attributes.
 *  - Create replace mods for the attributes present in the entry.
 *  - Create delete mods for the attributes that only have attribute meta data, but no attribute.
 *
 *  Also detect that if an object is being deleted, in which case set the correct targetDn.
 */

static
int
SetupReplModifyRequest(
    PVDIR_OPERATION              pOperation,
    PVMDIR_REPLICATION_UPDATE    pUpdate
    )
{
    int                 retVal = LDAP_SUCCESS;
    VDIR_MODIFICATION * mod = NULL;
    VDIR_ATTRIBUTE *    currAttr = NULL;
    unsigned int        i = 0;
    PVDIR_ATTRIBUTE     pAttrAttrMetaData = NULL;
    BOOLEAN             isDeleteObjReq = FALSE;
    VDIR_MODIFICATION * lastKnownDNMod = NULL;
    PVDIR_SCHEMA_CTX    pSchemaCtx = pOperation->pSchemaCtx;
    ModifyReq *         mr = &(pOperation->request.modifyReq);
    VDIR_ENTRY          consumerEntry = {0};
    PLW_HASHMAP         pMetaDataMap = NULL;
    PSTR                pszAttrType = NULL;
    LW_HASHMAP_ITER     iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR     pair = {NULL, NULL};
    PVMDIR_ATTRIBUTE_METADATA    pSupplierMetaData = NULL;
    PVDIR_ENTRY         pEntry = NULL;

    pEntry = pUpdate->pEntry;

    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "SetupReplModifyRequest: next entry being replicated/Modified is: %s",
              pEntry->dn.lberbv.bv_val );

    // SJ-TBD: For every replicated Add do we really need to clone the schema context??
    if (pEntry->pSchemaCtx == NULL)
    {
        pEntry->pSchemaCtx = VmDirSchemaCtxClone(pOperation->pSchemaCtx);
    }

    // Make sure Attribute has its ATDesc set
    retVal = VmDirSchemaCheckSetAttrDesc(pEntry->pSchemaCtx, pEntry);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirAttributeMetaDataListConvertToHashMap(pUpdate->pMetaDataList, &pMetaDataMap);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirReplSetAttrNewMetaData(pOperation, pEntry, pMetaDataMap);
    BAIL_ON_VMDIR_ERROR(retVal);

    for (currAttr = pEntry->attrs; currAttr; currAttr = currAttr->next)
    {
        // Skip attributes that have loser attribute meta data => no mods for them
        if (IS_VMDIR_REPL_ATTR_CONFLICT(currAttr->pMetaData))
        {
            continue;
        }

        if (VmDirStringCompareA(currAttr->type.lberbv.bv_val, ATTR_OBJECT_GUID, FALSE) == 0)
        {
            // Skipping metadata processing for ObjectGUID which should never change.
            continue;
        }

        if (VmDirAllocateMemory(sizeof(VDIR_MODIFICATION), (PVOID *)&mod) != 0)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: VmDirAllocateMemory error", __FUNCTION__);
            BAIL_WITH_VMDIR_ERROR(retVal, LDAP_OPERATIONS_ERROR);
        }
        mod->operation = MOD_OP_REPLACE;

        retVal = VmDirAttributeInitialize(
                currAttr->type.lberbv.bv_val, currAttr->numVals, pSchemaCtx, &mod->attr);
        BAIL_ON_VMDIR_ERROR(retVal);

        mod->attr.pMetaData = currAttr->pMetaData;
        currAttr->pMetaData = NULL;

        for (i = 0; i < currAttr->numVals; i++)
        {
            if (VmDirBervalContentDup(&currAttr->vals[i], &mod->attr.vals[i]) != 0)
            {
                VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: BervalContentDup failed.", __FUNCTION__);
                BAIL_WITH_VMDIR_ERROR(retVal, LDAP_OPERATIONS_ERROR);
            }
        }

        if (VmDirStringCompareA(mod->attr.type.lberbv_val, ATTR_IS_DELETED, FALSE) == 0 &&
            VmDirStringCompareA(mod->attr.vals[0].lberbv_val, VMDIR_IS_DELETED_TRUE_STR, FALSE) == 0)
        {
            isDeleteObjReq = TRUE;
        }

        if (VmDirStringCompareA(mod->attr.type.lberbv.bv_val, ATTR_LAST_KNOWN_DN, FALSE) == 0)
        {
            lastKnownDNMod = mod;
        }

        if (VmDirStringCompareA(mod->attr.type.lberbv.bv_val, ATTR_DN, FALSE) == 0)
        {   // modrdn case, piggyback newdn for subsequent multi-value mod op.
            if (VmDirBervalContentDup(&currAttr->vals[0], &mr->newdn) != 0)
            {
                BAIL_WITH_VMDIR_ERROR(retVal, LDAP_OPERATIONS_ERROR);
            }
        }

        mod->next = mr->mods;
        mr->mods = mod;
        mr->numMods++;
    }

    // Create Delete Mods
    while (LwRtlHashMapIterate(pMetaDataMap, &iter, &pair))
    {
        pszAttrType = (PSTR) pair.pKey;
        pSupplierMetaData = (PVMDIR_ATTRIBUTE_METADATA) pair.pValue;

        pair.pKey = pair.pValue = NULL;
        retVal = LwRtlHashMapRemove(pMetaDataMap, pszAttrType, &pair);
        BAIL_ON_VMDIR_ERROR(retVal);

        // Skip metadata processing for ObjectGUID which should never change.
        if (VmDirStringCompareA(pszAttrType, ATTR_OBJECT_GUID, FALSE) != 0)
        {
            if (VmDirAllocateMemory(sizeof(VDIR_MODIFICATION), (PVOID *)&mod) != 0)
            {
                VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: VmDirAllocateMemory error", __FUNCTION__);
                BAIL_WITH_VMDIR_ERROR(retVal, LDAP_OPERATIONS_ERROR);
            }
            mod->operation = MOD_OP_DELETE;

            retVal = VmDirAttributeInitialize(pszAttrType, 0, pSchemaCtx, &mod->attr);
            BAIL_ON_VMDIR_ERROR(retVal);

            // Set localUsn in the metaData
            retVal = VmDirMetaDataSetLocalUsn(pSupplierMetaData, pOperation->pWriteQueueEle->usn);
            BAIL_ON_VMDIR_ERROR(retVal);

            mod->attr.pMetaData = pSupplierMetaData;
            pair.pValue = NULL;

            mod->next = mr->mods;
            mr->mods = mod;
            mr->numMods++;
        }

        VmDirFreeMetaDataMapPair(&pair, NULL);
    }

    if (isDeleteObjReq)
    {
        if (VmDirBervalContentDup( &lastKnownDNMod->attr.vals[0], &mr->dn ) != 0)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "ReplDeleteEntry: BervalContentDup failed." );
            BAIL_WITH_VMDIR_ERROR(retVal, LDAP_OPERATIONS_ERROR);
        }
    }

cleanup:
    if (pMetaDataMap)
    {
        LwRtlHashMapClear(
                pMetaDataMap,
                VmDirFreeMetaDataMapPair,
                NULL);
        LwRtlFreeHashMap(&pMetaDataMap);
    }
    // pAttrAttrMetaData is local, needs to be freed within the call
    VmDirFreeAttribute(pAttrAttrMetaData);
    VmDirFreeEntryContent(&consumerEntry);

    return retVal;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "failed, error (%d)", retVal);
    goto cleanup;
}

/*TODO: Move this function to entry.c*/
static
VOID
_VmDirLogReplEntryContent(
    PVDIR_ENTRY                   pEntry
    )
{
    PVDIR_ATTRIBUTE pAttr = NULL;
    int             iCnt = 0;

    for (pAttr = pEntry->attrs; pAttr; pAttr = pAttr->next)
    {
        for (iCnt=0; iCnt < pAttr->numVals; iCnt++)
        {
            PCSTR pszLogValue = (0 == VmDirStringCompareA( pAttr->type.lberbv.bv_val, ATTR_USER_PASSWORD, FALSE)) ?
                                  "XXX" : pAttr->vals[iCnt].lberbv_val;

            if (iCnt < MAX_NUM_CONTENT_LOG)
            {
                VMDIR_LOG_INFO( LDAP_DEBUG_REPL_ATTR, "%s %s %d (%.*s)",
                    __FUNCTION__,
                    pAttr->type.lberbv.bv_val,
                    iCnt+1,
                    VMDIR_MIN(pAttr->vals[iCnt].lberbv_len, VMDIR_MAX_LOG_OUTPUT_LEN),
                    VDIR_SAFE_STRING(pszLogValue));
            }
            else if (iCnt == MAX_NUM_CONTENT_LOG)
            {
                VMDIR_LOG_INFO( LDAP_DEBUG_REPL_ATTR, "%s Total value count %d)", __FUNCTION__, pAttr->numVals);
            }
            else
            {
                break;
            }
        }
    }
}

static
VOID
_VmDirLogReplAddEntryContent(
    PVMDIR_REPLICATION_UPDATE pUpdate
    )
{
    VMDIR_LOG_INFO( LDAP_DEBUG_REPL_ATTR,
              "%s, DN:%s, SYNC_STATE:ADD, partner USN:%" PRId64,
              __FUNCTION__,
              pUpdate->pEntry->dn.lberbv.bv_val,
              pUpdate->partnerUsn);

    if (VmDirLogLevelAndMaskTest(VMDIR_LOG_VERBOSE, LDAP_DEBUG_REPL_ATTR))
    {
        _VmDirLogReplEntryContent(pUpdate->pEntry);
    }
}

static
VOID
_VmDirLogReplDeleteEntryContent(
    PVMDIR_REPLICATION_UPDATE pUpdate
    )
{
    VMDIR_LOG_INFO( LDAP_DEBUG_REPL_ATTR,
              "%s, DN:%s SYNC_STATE:Delete, partner USN:%" PRId64,
              __FUNCTION__,
              pUpdate->pEntry->dn.lberbv.bv_val,
              pUpdate->partnerUsn);
}

static
VOID
_VmDirLogReplModifyEntryContent(
    PVMDIR_REPLICATION_UPDATE pUpdate
    )
{
    VMDIR_LOG_INFO( LDAP_DEBUG_REPL_ATTR,
              "%s, DN:%s, SYNC_STATE:Modify, partner USN:%" PRId64,
              __FUNCTION__,
              pUpdate->pEntry->dn.lberbv.bv_val,
              pUpdate->partnerUsn);

    _VmDirLogReplEntryContent(pUpdate->pEntry);
}

static
VOID
_VmDirLogReplModifyModContent(
    ModifyReq*  pModReq
    )
{
    PVDIR_MODIFICATION  pMod = pModReq->mods;
    int                 iCnt = 0;

    for (; pMod; pMod = pMod->next)
    {
        for (iCnt=0; iCnt < pMod->attr.numVals; iCnt++)
        {
            PCSTR pszLogValue = (VmDirIsSensitiveAttr(pMod->attr.type.lberbv.bv_val)) ?
                                  "XXX" : pMod->attr.vals[iCnt].lberbv_val;

            if (iCnt < MAX_NUM_CONTENT_LOG)
            {
                VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL,
                    "%s MOD %d, %s, %s: (%.*s)",
                    __FUNCTION__,
                    iCnt+1,
                    VmDirLdapModOpTypeToName(pMod->operation),
                    VDIR_SAFE_STRING(pMod->attr.type.lberbv.bv_val),
                    VMDIR_MIN(pMod->attr.vals[iCnt].lberbv_len, VMDIR_MAX_LOG_OUTPUT_LEN),
                    VDIR_SAFE_STRING(pszLogValue));
            }
            else if (iCnt == MAX_NUM_CONTENT_LOG)
            {
                VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL,
                    "%s Total value count %d)", __FUNCTION__, pMod->attr.numVals);
            }
            else
            {
                break;
            }
        }
    }
}

static
VOID
_VmDirReplModifyClearAllMods(
    ModifyReq*    pModifyReq
    )
{
    PVDIR_MODIFICATION    pCurrMod = NULL;
    PVDIR_MODIFICATION    pTempMod = NULL;

    if (pModifyReq)
    {
        pCurrMod = pModifyReq->mods;

        while(pCurrMod != NULL)
        {
            pTempMod = pCurrMod->next;
            VmDirModificationFree(pCurrMod);
            pCurrMod = pTempMod;
        }

        pModifyReq->mods = NULL;
        pModifyReq->numMods = 0;
    }
}
