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
DetectAndResolveAttrsConflicts(
    PVDIR_OPERATION     pOperation,
    PVDIR_ENTRY         pSupplierEntry,
    PVDIR_ATTRIBUTE     pAttrAttrSupplierMetaData,
    ENTRYID             entryId,
    PVDIR_ENTRY         pConsumerEntry
    );

static
int
SetAttributesNewMetaData(
    PVDIR_OPERATION     pOperation,
    PVDIR_ENTRY         pEntry,
    char *              localUsn,
    PVDIR_ATTRIBUTE *   ppAttrAttrMetaData,
    ENTRYID             entryId,
    PVDIR_ENTRY         pConsumerEntry
    );

static
int
SetupReplModifyRequest(
    VDIR_OPERATION *    modOp,
    PVDIR_ENTRY         pEntry,
    USN *               pNextLocalUsn,
    ENTRYID             entryId
    );

static
int
_VmDirPatchData(
    PVDIR_OPERATION     pOperation
    );

static
DWORD
_VmDirAssignEntryIdIfSpecialInternalEntry(
    PVDIR_ENTRY pEntry
    );

static
int
ReplFixUpEntryDn(
    PVDIR_ENTRY pEntry
    );

static
int
_VmDirDetatchValueMetaData(
    PVDIR_OPERATION     pOperation,
    PVDIR_ENTRY         pEntry,
    PVDIR_ATTRIBUTE *   ppAttrAttrValueMetaData
    );

static
int
_VmDirAttachValueMetaData(
    PVDIR_ATTRIBUTE pAttrAttrValueMetaData,
    PVDIR_ENTRY     pEntry,
    USN             localUsn
    );

static
int
_VmDeleteOldValueMetaData(
    PVDIR_OPERATION     pModOp,
    PVDIR_MODIFICATION  pMods,
    ENTRYID             entryId
    );

static
int
_VmSetupValueMetaData(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    PVDIR_OPERATION     pModOp,
    PVDIR_ATTRIBUTE     pAttrAttrValueMetaData,
    USN                 localUsn,
    ENTRYID             entryId
    );

static
int
_VmDirAttrValueMetaResolve(
    PVDIR_OPERATION pModOp,
    PVDIR_ATTRIBUTE pAttr,
    PVDIR_BERVALUE  suppAttrMetaValue,
    ENTRYID         entryId,
    PBOOLEAN        pInScope
    );

static
BOOLEAN
_VmDirIsBenignReplConflict(
    PVDIR_ATTRIBUTE pAttr,
    PVDIR_ENTRY     pSupplierEntry,
    PVDIR_ENTRY     pConsumerEntry
    );

/*
 * _VmDirAssignEntryIdIfSpecialInternalEntry()
 *
 * Internal entries from vmdir.h:
 *
 * #define DSE_ROOT_ENTRY_ID              1
 * #define SCHEMA_NAMING_CONTEXT_ID       2
 * #define SUB_SCEHMA_SUB_ENTRY_ID        3
 * #define CFG_ROOT_ENTRY_ID              4
 * #define CFG_INDEX_ENTRY_ID             5
 * #define CFG_ORGANIZATION_ENTRY_ID      6
 * #define DEL_ENTRY_CONTAINER_ENTRY_ID   7
 * #define DEFAULT_ADMINISTRATOR_ENTRY_ID 8
 *
 * Except System administrator and deleted objects container entries, rest are created at the initialization time of
 * all replicas => getting expected entry Ids.
 *
 */
static DWORD
_VmDirAssignEntryIdIfSpecialInternalEntry(
    PVDIR_ENTRY pEntry )
{
    DWORD       dwError = 0;
    PSTR        pszLocalErrMsg = NULL;

    dwError = VmDirNormalizeDN( &(pEntry->dn), pEntry->pSchemaCtx );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                  "_VmDirAssignEntryIdIfSpecialInternalEntry: DN normalization failed - (%u)(%s)",
                                  dwError, pEntry->dn.lberbv.bv_val );

    if (VmDirStringCompareA( BERVAL_NORM_VAL(pEntry->dn),
                             BERVAL_NORM_VAL(gVmdirServerGlobals.bvDefaultAdminDN), TRUE) == 0)
    {
        pEntry->eId = DEFAULT_ADMINISTRATOR_ENTRY_ID;
    }
    else if (VmDirStringCompareA( BERVAL_NORM_VAL(pEntry->dn),
                                  BERVAL_NORM_VAL(gVmdirServerGlobals.delObjsContainerDN), TRUE) == 0)
    {
        pEntry->eId = DEL_ENTRY_CONTAINER_ENTRY_ID;
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, pszLocalErrMsg );
    goto cleanup;
}


// Replicate Add Entry operation

int
ReplAddEntry(
    PVDIR_SCHEMA_CTX                pSchemaCtx,
    PVMDIR_REPLICATION_PAGE_ENTRY   pPageEntry,
    PVDIR_SCHEMA_CTX*               ppOutSchemaCtx,
    BOOLEAN                         bFirstReplicationCycle)
{
    int                 retVal = LDAP_SUCCESS;
    VDIR_OPERATION      op = {0};
    PVDIR_ATTRIBUTE     pAttr = NULL;
    PVDIR_ENTRY         pEntry = NULL;
    USN                 localUsn = 0;
    char                localUsnStr[VMDIR_MAX_USN_STR_LEN];
    size_t              localUsnStrlen = 0;
    int                 dbRetVal = 0;
    PVDIR_ATTRIBUTE     pAttrAttrMetaData = NULL;
    PVDIR_ATTRIBUTE     pAttrAttrValueMetaData = NULL;
    int                 i = 0;
    PVDIR_SCHEMA_CTX    pUpdateSchemaCtx = NULL;
    LDAPMessage *       ldapMsg = pPageEntry->entry;
    VDIR_ENTRY          consumerEntry = {0};

    retVal = VmDirInitStackOperation( &op,
                                      VDIR_OPERATION_TYPE_REPL,
                                      LDAP_REQ_ADD,
                                      pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(retVal);

    pEntry = op.request.addReq.pEntry;  // init pEntry after VmDirInitStackOperation

    op.ber = ldapMsg->lm_ber;

    retVal = VmDirParseEntry( &op );
    BAIL_ON_VMDIR_ERROR( retVal );

    op.pBEIF = VmDirBackendSelect(pEntry->dn.lberbv.bv_val);
    assert(op.pBEIF);

    // SJ-TBD: For every replicated Add do we really need to clone the schema context??
    pEntry->pSchemaCtx = VmDirSchemaCtxClone(op.pSchemaCtx);

    // Make sure Attribute has its ATDesc set
    retVal = VmDirSchemaCheckSetAttrDesc(pEntry->pSchemaCtx, pEntry);
    BAIL_ON_VMDIR_ERROR(retVal);

    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "ReplAddEntry: next entry being replicated/Added is: %s", pEntry->dn.lberbv.bv_val);

    // Set local attributes.

    if ((dbRetVal = op.pBEIF->pfnBEGetNextUSN( op.pBECtx, &localUsn )) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "ReplAddEntry: pfnBEGetNextUSN failed with error code: %d, error string: %s",
                  dbRetVal, VDIR_SAFE_STRING(op.pBEErrorMsg) );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    if ((retVal = VmDirStringNPrintFA( localUsnStr, sizeof(localUsnStr), sizeof(localUsnStr) - 1, "%ld", localUsn)) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "ReplAddEntry: VmDirStringNPrintFA failed with error code: %d", retVal);
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    localUsnStrlen = VmDirStringLenA( localUsnStr );

    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "ReplAddEntry: next generated localUSN: %s", localUsnStr);

    retVal = _VmDirDetatchValueMetaData(&op, pEntry, &pAttrAttrValueMetaData);
    BAIL_ON_VMDIR_ERROR( retVal );

    retVal = SetAttributesNewMetaData(&op, pEntry, localUsnStr, &pAttrAttrMetaData, 0, &consumerEntry);
    BAIL_ON_VMDIR_ERROR( retVal );

    // Creating deleted object scenario: Create attributes just with attribute meta data, and no values.
    for (i = 0; pAttrAttrMetaData->vals[i].lberbv.bv_val != NULL; i++)
    {
        if (pAttrAttrMetaData->vals[i].lberbv.bv_len != 0)
        {
            char * metaData = VmDirStringChrA( pAttrAttrMetaData->vals[i].lberbv.bv_val, ':');
            assert( metaData != NULL);

            *metaData++ = '\0'; // now pAttrAttrMetaData->vals[i].lberbv.bv_val is the attribute name

            retVal = VmDirAttributeAllocate( pAttrAttrMetaData->vals[i].lberbv.bv_val, 0, pEntry->pSchemaCtx, &pAttr );
            BAIL_ON_VMDIR_ERROR(retVal);
            VmDirStringCpyA( pAttr->metaData, VMDIR_MAX_ATTR_META_DATA_LEN, localUsnStr );
            pAttr->metaData[localUsnStrlen] = ':';
            // skip localUSN coming from the replication partner
            metaData = VmDirStringChrA( metaData, ':');
            VmDirStringCpyA( pAttr->metaData + localUsnStrlen + 1 /* for : */,
                             (VMDIR_MAX_ATTR_META_DATA_LEN - localUsnStrlen - 1), metaData + 1 /* skip : */);

            pAttr->next = pEntry->attrs;
            pEntry->attrs = pAttr;
        }
    }

    retVal = _VmDirAttachValueMetaData(pAttrAttrValueMetaData, pEntry, localUsn);
    BAIL_ON_VMDIR_ERROR( retVal );

    retVal = _VmDirPatchData( &op );
    BAIL_ON_VMDIR_ERROR( retVal );

    if (bFirstReplicationCycle)
    {
        retVal =  _VmDirAssignEntryIdIfSpecialInternalEntry( pEntry );
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    op.ulPartnerUSN = pPageEntry->ulPartnerUSN;

    if ((retVal = VmDirInternalAddEntry( &op )) != LDAP_SUCCESS)
    {
        // Reset retVal to LDAP level error space (for B/C)
        retVal = op.ldapResult.errCode;

        switch (retVal)
        {
            case LDAP_ALREADY_EXISTS:
                VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
                          "ReplAddEntry/VmDirInternalAddEntry: %d (Object already exists). "
                          "DN: %s, first attribute: %s, it's meta data: '%s' "
                          "NOT resolving this possible replication CONFLICT or initial objects creation scenario. "
                          "For this object, system may not converge. Partner USN %llu",
                          retVal, pEntry->dn.lberbv.bv_val, pEntry->attrs->type.lberbv.bv_val, pEntry->attrs->metaData,
                          pPageEntry->ulPartnerUSN);

                break;

            case LDAP_NO_SUCH_OBJECT:
                VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
                          "ReplAddEntryVmDirInternalAddEntry: %d (Parent object does not exist). "
                          "DN: %s, first attribute: %s, it's meta data: '%s' "
                          "NOT resolving this possible replication CONFLICT or out-of-parent-child-order replication scenario. "
                          "For this subtree, system may not converge. Partner USN %llu",
                          retVal, pEntry->dn.lberbv.bv_val, pEntry->attrs->type.lberbv.bv_val, pEntry->attrs->metaData,
                          pPageEntry->ulPartnerUSN);
                break;

            default:
                VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                          "ReplAddEntry/VmDirInternalAddEntry:  %d (%s). Partner USN %llu",
                          retVal, VDIR_SAFE_STRING( op.ldapResult.pszErrMsg ), pPageEntry->ulPartnerUSN);
                break;
        }
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    if (VmDirStringEndsWith(
            BERVAL_NORM_VAL(pEntry->dn), SCHEMA_NAMING_CONTEXT_DN, FALSE))
    {
        // schema entry updated, refresh replication schema ctx.
        assert( ppOutSchemaCtx );
        retVal = VmDirSchemaCtxAcquire(&pUpdateSchemaCtx);
        BAIL_ON_VMDIR_ERROR(retVal);
        *ppOutSchemaCtx = pUpdateSchemaCtx;

        VmDirSchemaCtxRelease(pSchemaCtx);
    }

cleanup:
    // pAttrAttrMetaData is local, needs to be freed within the call
    VmDirFreeAttribute( pAttrAttrValueMetaData );
    VmDirFreeAttribute( pAttrAttrMetaData );
    VmDirFreeOperationContent(&op);
    VmDirFreeEntryContent(&consumerEntry);

    return retVal;

error:
    VmDirSchemaCtxRelease(pUpdateSchemaCtx);

    goto cleanup;
} // Replicate Add Entry operation

/* Replicate Delete Entry operation
 * Set modifications associated with a Delete operation, and pass-in the modifications, with correct attribute meta data
 * set, to InternalDeleteEntry function, which will apply the mods to the existing entry, and move the object to the
 * DeletedObjects container.
 */
int
ReplDeleteEntry(
    PVDIR_SCHEMA_CTX                pSchemaCtx,
    PVMDIR_REPLICATION_PAGE_ENTRY   pPageEntry
    )
{
    int                 retVal = LDAP_SUCCESS;
    VDIR_OPERATION      tmpAddOp = {0};
    VDIR_OPERATION      delOp = {0};
    ModifyReq *         mr = &(delOp.request.modifyReq);
    USN                 localUsn = {0};
    LDAPMessage *       ldapMsg = pPageEntry->entry;

    retVal = VmDirInitStackOperation( &delOp,
                                      VDIR_OPERATION_TYPE_REPL,
                                      LDAP_REQ_DELETE,
                                      pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirInitStackOperation( &tmpAddOp,
                                      VDIR_OPERATION_TYPE_REPL,
                                      LDAP_REQ_ADD,
                                      pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(retVal);

    tmpAddOp.ber = ldapMsg->lm_ber;

    retVal = VmDirParseEntry( &tmpAddOp );
    BAIL_ON_VMDIR_ERROR( retVal );

    retVal = ReplFixUpEntryDn(tmpAddOp.request.addReq.pEntry);
    BAIL_ON_VMDIR_ERROR( retVal );

    if (VmDirBervalContentDup( &tmpAddOp.reqDn, &mr->dn ) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "ReplDeleteEntry: BervalContentDup failed." );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    delOp.pBEIF = VmDirBackendSelect(mr->dn.lberbv.bv_val);
    assert(delOp.pBEIF);

    // SJ-TBD: What about if one or more attributes were meanwhile added to the entry? How do we purge them?
    retVal = SetupReplModifyRequest( &delOp, tmpAddOp.request.addReq.pEntry, &localUsn, 0);
    BAIL_ON_VMDIR_ERROR( retVal );

    delOp.ulPartnerUSN = pPageEntry->ulPartnerUSN;

    // SJ-TBD: What happens when DN of the entry has changed in the meanwhile? => conflict resolution.
    // Should objectGuid, instead of DN, be used to uniquely identify an object?
    if ((retVal = VmDirInternalDeleteEntry( &delOp )) != LDAP_SUCCESS)
    {
        // If VmDirInternall call failed, reset retVal to LDAP level error space (for B/C)
        retVal = delOp.ldapResult.errCode;

        switch (retVal)
        {
            case LDAP_NO_SUCH_OBJECT:
                VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
                          "ReplDeleteEntry/VmDirInternalDeleteEntry: %d (Object does not exist). "
                          "DN: %s, first attribute: %s, it's meta data: '%s'. "
                          "NOT resolving this possible replication CONFLICT. "
                          "For this object, system may not converge. Partner USN %llu",
                          retVal, mr->dn.lberbv.bv_val, mr->mods->attr.type.lberbv.bv_val, mr->mods->attr.metaData,
                          pPageEntry->ulPartnerUSN);
                retVal = LDAP_SUCCESS;
                break;

            case LDAP_NOT_ALLOWED_ON_NONLEAF:
                VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
                          "ReplDeleteEntry/VmDirInternalDeleteEntry: %d (Operation not allowed on non-leaf). "
                          "DN: %s, first attribute: %s, it's meta data: '%s'. "
                          "NOT resolving this possible replication CONFLICT. "
                          "For this object, system may not converge. Partner USN %llu",
                          retVal, mr->dn.lberbv.bv_val, mr->mods->attr.type.lberbv.bv_val, mr->mods->attr.metaData,
                          pPageEntry->ulPartnerUSN);
                break;

            case LDAP_NO_SUCH_ATTRIBUTE:
                VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
                          "ReplDeleteEntry/VmDirInternalDeleteEntry: %d (No such attribute). "
                          "DN: %s, first attribute: %s, it's meta data: '%s'. "
                          "NOT resolving this possible replication CONFLICT. "
                          "For this object, system may not converge. Partner USN %llu",
                          retVal, mr->dn.lberbv.bv_val, mr->mods->attr.type.lberbv.bv_val, mr->mods->attr.metaData,
                          pPageEntry->ulPartnerUSN);
                break;

            default:
                VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                          "ReplDeleteEntry/InternalDeleteEntry: %d (%s). Partner USN %llu",
                          retVal, VDIR_SAFE_STRING( delOp.ldapResult.pszErrMsg ),pPageEntry->ulPartnerUSN);
                break;
        }
        BAIL_ON_VMDIR_ERROR( retVal );
    }

cleanup:
    VmDirFreeModifyRequest( mr, FALSE );
    VmDirFreeOperationContent(&delOp);
    VmDirFreeOperationContent(&tmpAddOp);

    return retVal;

error:
    goto cleanup;
} // Replicate Delete entry operation

// Replicate Modify Entry operation
int
ReplModifyEntry(
    PVDIR_SCHEMA_CTX                pSchemaCtx,
    PVMDIR_REPLICATION_PAGE_ENTRY   pPageEntry,
    PVDIR_SCHEMA_CTX*               ppOutSchemaCtx)
{
    int                 retVal = LDAP_SUCCESS;
    VDIR_OPERATION      modOp = {0};
    ModifyReq *         mr = &(modOp.request.modifyReq);
    int                 dbRetVal = 0;
    BOOLEAN             bHasTxn = FALSE;
    int                 deadLockRetries = 0;
    PVDIR_SCHEMA_CTX    pUpdateSchemaCtx = NULL;
    VDIR_ENTRY          e = {0};
    PVDIR_ATTRIBUTE     pAttrAttrValueMetaData = NULL;
    USN                 localUsn = {0};
    ENTRYID             entryId = 0;
    LDAPMessage *       ldapMsg = pPageEntry->entry;

    retVal = VmDirInitStackOperation( &modOp,
                                      VDIR_OPERATION_TYPE_REPL,
                                      LDAP_REQ_MODIFY,
                                      pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirParseBerToEntry(ldapMsg->lm_ber, &e, NULL, NULL);
    BAIL_ON_VMDIR_ERROR( retVal );

    retVal = ReplFixUpEntryDn(&e);
    BAIL_ON_VMDIR_ERROR( retVal );

    if (VmDirBervalContentDup( &e.dn, &mr->dn ) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "SetupReplModifyRequest: BervalContentDup failed." );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    // This is strict locking order:
    // Must acquire schema modification mutex before backend write txn begins
    retVal = VmDirSchemaModMutexAcquire(&modOp);
    BAIL_ON_VMDIR_ERROR( retVal );

    modOp.pBEIF = VmDirBackendSelect(mr->dn.lberbv.bv_val);
    assert(modOp.pBEIF);

    // ************************************************************************************
    // transaction retry loop begin.  make sure all function within are retry agnostic.
    // ************************************************************************************
txnretry:
    {
    if (bHasTxn)
    {
        modOp.pBEIF->pfnBETxnAbort( modOp.pBECtx );
        bHasTxn = FALSE;
    }

    deadLockRetries++;
    if (deadLockRetries > MAX_DEADLOCK_RETRIES)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "ReplModifyEntry: Ran out of deadlock retries." );
        retVal = LDAP_LOCK_DEADLOCK;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    // Transaction needed to process existing/local attribute meta data.
    if ((dbRetVal = modOp.pBEIF->pfnBETxnBegin( modOp.pBECtx, VDIR_BACKEND_TXN_WRITE)) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "ReplModifyEntry: pfnBETxnBegin failed with error code: %d, error string: %s",
                  dbRetVal, VDIR_SAFE_STRING(modOp.pBEErrorMsg) );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }
    bHasTxn = TRUE;

    if (mr->dn.bvnorm_val == NULL)
    {
        if ((retVal = VmDirNormalizeDN(&mr->dn, pSchemaCtx)) != 0)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "SetupReplModifyRequest: VmDirNormalizeDN failed on dn %s ",
                    VDIR_SAFE_STRING(mr->dn.lberbv.bv_val));
            retVal = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR( retVal );
        }
    }

    // Get EntryId
    retVal = modOp.pBEIF->pfnBEDNToEntryId(modOp.pBECtx, &mr->dn, &entryId);
    if (retVal != 0)
    {
        switch (retVal)
        {
            case ERROR_BACKEND_ENTRY_NOTFOUND:
                VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "ReplModifyEntry: entry %s doesn't exist error code %d",
                                VDIR_SAFE_STRING(mr->dn.bvnorm_val), retVal);
                break;
            case LDAP_LOCK_DEADLOCK:
                goto txnretry;
            default:
                VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "ReplModifyEntry: pfnBEDNToEntryId failed dn %s error code %d",
                                VDIR_SAFE_STRING(mr->dn.bvnorm_val), retVal);
                break;
        }
    }
    BAIL_ON_VMDIR_ERROR( retVal );

    retVal = _VmDirDetatchValueMetaData(&modOp, &e, &pAttrAttrValueMetaData);
    BAIL_ON_VMDIR_ERROR( retVal );

    if ((retVal = SetupReplModifyRequest(&modOp, &e, &localUsn, entryId)) != LDAP_SUCCESS)
    {
        switch (retVal)
        {
            case LDAP_NO_SUCH_OBJECT:
                VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
                          "ReplModifyEntry/SetupReplModifyRequest: %d (Object does not exist). "
                          "DN: %s, first attribute: %s, it's meta data: '%s'. "
                          "Possible replication CONFLICT. Object will get deleted from the system. "
                          "Partner USN %llu",
                          retVal, e.dn.lberbv.bv_val, e.attrs[0].type.lberbv.bv_val,
                          e.attrs[0].metaData, pPageEntry->ulPartnerUSN);
                break;

            case LDAP_LOCK_DEADLOCK:
                VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
                          "ReplModifyEntry/SetupReplModifyRequest: %d (%s). ",
                          retVal, VDIR_SAFE_STRING( modOp.ldapResult.pszErrMsg ));
                goto txnretry; // Possible retry.

            default:
                VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                          "ReplModifyEntry/SetupReplModifyRequest: %d (%s). Partner USN %llu",
                          retVal, VDIR_SAFE_STRING( modOp.ldapResult.pszErrMsg ), pPageEntry->ulPartnerUSN);
                break;
       }
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    // If some mods left after conflict resolution
    if (mr->mods != NULL)
    {
        retVal = _VmDeleteOldValueMetaData(&modOp, mr->mods, entryId);
        BAIL_ON_VMDIR_ERROR( retVal );

        modOp.ulPartnerUSN = pPageEntry->ulPartnerUSN;

        // SJ-TBD: What happens when DN of the entry has changed in the meanwhile? => conflict resolution.
        // Should objectGuid, instead of DN, be used to uniquely identify an object?
        if ((retVal = VmDirInternalModifyEntry( &modOp )) != LDAP_SUCCESS)
        {
            // If VmDirInternall call failed, reset retVal to LDAP level error space (for B/C)
            retVal = modOp.ldapResult.errCode;

            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "ReplModifyEntry: InternalModifyEntry failed. Error: %d, error string %s",
                      retVal, VDIR_SAFE_STRING( modOp.ldapResult.pszErrMsg ));

            switch (retVal)
            {
                case LDAP_LOCK_DEADLOCK:
                    goto txnretry; // Possible retry.

                default:
                    break;
            }
            BAIL_ON_VMDIR_ERROR( retVal );
        }
    }

    VMDIR_LOG_INFO(LDAP_DEBUG_REPL, "ReplModifyEntry: found AttrValueMetaData %s", pAttrAttrValueMetaData?"yes":"no");
    if (pAttrAttrValueMetaData)
    {
        retVal = _VmSetupValueMetaData(pSchemaCtx, &modOp, pAttrAttrValueMetaData, localUsn, entryId);
        BAIL_ON_VMDIR_ERROR( retVal );
        mr = &(modOp.request.modifyReq);
        if (mr->mods != NULL)
        {
            if ((retVal = VmDirInternalModifyEntry( &modOp )) != LDAP_SUCCESS)
            {
                retVal = modOp.ldapResult.errCode;
                switch (retVal)
                {
                    case LDAP_LOCK_DEADLOCK:
                        goto txnretry;
                    default:
                        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "ReplModifyEntry: InternalModifyEntry failed. Error: %d, error string %s",
                                        retVal, VDIR_SAFE_STRING( modOp.ldapResult.pszErrMsg ));
                        break;
                }
                BAIL_ON_VMDIR_ERROR( retVal );
            }
        }
    }

    if ((dbRetVal = modOp.pBEIF->pfnBETxnCommit( modOp.pBECtx)) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "ReplModifyEntry: pfnBETxnCommit failed with error code: %d, error string: %s",
                  dbRetVal, VDIR_SAFE_STRING(modOp.pBEErrorMsg) );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }
    bHasTxn = FALSE;
    // ************************************************************************************
    // transaction retry loop end.
    // ************************************************************************************
    }

    if (VmDirStringEndsWith(
            BERVAL_NORM_VAL(modOp.request.modifyReq.dn),
            SCHEMA_NAMING_CONTEXT_DN,
            FALSE))
    {
        // schema entry updated, refresh replication schema ctx.
        assert( ppOutSchemaCtx );
        retVal = VmDirSchemaCtxAcquire(&pUpdateSchemaCtx);
        BAIL_ON_VMDIR_ERROR(retVal);
        *ppOutSchemaCtx = pUpdateSchemaCtx;

        VmDirSchemaCtxRelease(pSchemaCtx);
    }

cleanup:
    // Release schema modification mutex
    (VOID)VmDirSchemaModMutexRelease(&modOp);
    VmDirFreeOperationContent(&modOp);
    VmDirFreeEntryContent(&e);
    VmDirFreeAttribute(pAttrAttrValueMetaData);
    return retVal;

error:
    if (bHasTxn)
    {
        modOp.pBEIF->pfnBETxnAbort( modOp.pBECtx );
    }
    VmDirSchemaCtxRelease(pUpdateSchemaCtx);

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
BOOLEAN
_VmDirIsBenignReplConflict(
    PVDIR_ATTRIBUTE pAttr,
    PVDIR_ENTRY     pSupplierEntry,
    PVDIR_ENTRY     pConsumerEntry
    )
{
    DWORD   dwError = 0;
    BOOLEAN bIsBenign = FALSE;
    CHAR    excludeAttrs[] = {ATTR_USN_CHANGED};  // supplier always send USNChanged, but it has a local context.
    int     i = 0;

    if (pConsumerEntry->eId == 0) // don't expect this, but pass through if no eid.
    {
        goto cleanup;
    }

    // query consumer entry if needed
    if (!pConsumerEntry->dn.lberbv_val)
    {
        PVDIR_BACKEND_INTERFACE pBE = NULL;
        pBE = VmDirBackendSelect(NULL);
        assert(pBE);

        dwError = pBE->pfnBESimpleIdToEntry(pConsumerEntry->eId, pConsumerEntry);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (i=0; i< VMDIR_ARRAY_SIZE(excludeAttrs); i++)
    {
        if (VmDirStringCompareA(pAttr->type.lberbv_val, &excludeAttrs[i], FALSE) == 0)
        {
            bIsBenign = TRUE;
            goto cleanup;
        }
    }

    bIsBenign = VmDirIsSameConsumerSupplierEntryAttr(pAttr, pSupplierEntry, pConsumerEntry);

cleanup:
    return bIsBenign;

error:
    goto cleanup;
}

/* Detect and resolve attribute level conflicts.
 *
 * Read consumer attributes' meta data corresponding to given supplier attributes' meta data, "compare" them, and "mark"
 * the losing supplier attribute meta data.
 *
 * To resolve conflicts between "simultaneous" modifications to same attribute on 2 replicas, following fields (in that
 * order/priority) in the attribute meta data are used:
 * 1) version #
 * 2) server ID
 *
 * Logic:
 *
 * - If supplier attribute version # is > consumer attribute version #, there is "no conflict", and supplier WINS,
 *      => supplier attribute mod should be applied to this consumer.
 * - If supplier attribute version # is < consumer attribute version #, there is a conflict, and supplier LOSES,
 *      => supplier attribute mod should NOT be applied to this consumer
 * - If supplier attribute version # is = consumer attribute version #, there is is conflict, and conflict is resolved
 *   by lexicographical comparison of supplier and consumer server IDs. I.e. the attribute change on the server with
 *   higher serverID WINs.
 *
 *   Parameters:
 *      (in) pDn:    DN of the entry being modified.
 *      (in) pAttrAttrSupplierMetaData:  attribute meta data attribute containing the meta data values present on supplier.
 *      (out) ppAttrConsumerMetaData: A list of Attribute structures containing consumer side attribute meta data for
 *                                    each attribute.
 *
 *   This function reads (from local DB) consumer attribute meta data, **CORRESPONDING** to the supplier attributes
 *   meta data (pAttrAttrSupplierMetaData), and resets the losing supplier meta data to an empty string in values of
 *   pAttrAttrSupplierMetaData. Before this function call, a value of pAttrAttrSupplierMetaData looks like:
 *      "<attr name>:<local USN>:<version no>:<originating server ID>:<originating time>:<originating USN>"
 *
 *   A losing supplier attribute meta data looks like: "<attr name>:"
 *   A winning supplier attribute meta data remains unchanged as:
 *      "<attr name>:<local USN>:<version no>:<originating server ID>:<originating time>:<originating USN>"
 */

static
int
DetectAndResolveAttrsConflicts(
    PVDIR_OPERATION     pOperation,
    PVDIR_ENTRY         pSupplierEntry,
    PVDIR_ATTRIBUTE     pAttrAttrSupplierMetaData,
    ENTRYID             entryId,
    PVDIR_ENTRY         pConsumerEntry
    )
{
    int             retVal = LDAP_SUCCESS;
    int             dbRetVal = 0;
    PVDIR_ATTRIBUTE pAttr = NULL;
    int             i = 0;

    assert( pOperation && pOperation->pSchemaCtx && pAttrAttrSupplierMetaData );

    for (i = 0; pAttrAttrSupplierMetaData->vals[i].lberbv.bv_val != NULL; i++)
    {
        // Format is: <attr name>:<local USN>:<version no>:<originating server ID>:<originating time>:<originating USN>
        char * metaData = VmDirStringChrA( pAttrAttrSupplierMetaData->vals[i].lberbv.bv_val, ':');
        assert( metaData != NULL);

        // metaData now points to <local USN>...
        metaData++;

        *(metaData - 1) = '\0';
        if (pAttr)
        {
            VmDirFreeAttribute( pAttr );
            pAttr = NULL;
        }

        if (VmDirStringCompareA(pAttrAttrSupplierMetaData->vals[i].lberbv.bv_val, ATTR_OBJECT_GUID, FALSE) == 0)
        {
            // Skipping metadata processing for ObjectGUID which should
            // never change.
            continue;
        }
        retVal = VmDirAttributeAllocate( pAttrAttrSupplierMetaData->vals[i].lberbv.bv_val, 0, pOperation->pSchemaCtx, &pAttr );
        *(metaData - 1) = ':';
        BAIL_ON_LDAP_ERROR( retVal, LDAP_OPERATIONS_ERROR, (pOperation->ldapResult.pszErrMsg),
                            "VmDirAttributeAllocate failed", VDIR_SAFE_STRING(pOperation->pBEErrorMsg));

        // Read consumer attribute meta data
        if ((dbRetVal = pOperation->pBEIF->pfnBEGetAttrMetaData( pOperation->pBECtx, pAttr, entryId )) != 0)
        {
            switch (dbRetVal)
            {
                case ERROR_BACKEND_ATTR_META_DATA_NOTFOUND: // OK, e.g. when a new attribute is being added
                    // => Supplier attribute meta data WINS against consumer attribute meta data
                    break;

                case ERROR_BACKEND_DEADLOCK:
                    BAIL_ON_LDAP_ERROR( retVal, LDAP_LOCK_DEADLOCK, (pOperation->ldapResult.pszErrMsg),
                                        "backend read entry failed - (%d)(%s)", retVal,
                                        VDIR_SAFE_STRING(pOperation->pBEErrorMsg));
                    break;

                default:
                    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "DetectAndResolveAttrsConflicts: pfnBEGetAttrMetaData failed "
                              "with error code: %d, error string: %s", dbRetVal,
                              VDIR_SAFE_STRING(pOperation->pBEErrorMsg) );

                    BAIL_ON_LDAP_ERROR( retVal, LDAP_OPERATIONS_ERROR, (pOperation->ldapResult.pszErrMsg),
                                        "pfnBEGetAttrMetaData failed - (%d)(%s)", dbRetVal,
                                        VDIR_SAFE_STRING(pOperation->pBEErrorMsg));
            }
        }
        else
        {
            int supplierVersionNum = 0;
            int consumerVersionNum = 0;

            // Format is: <local USN>:<version no>:<originating server ID>:<originating time>:<originating USN>
            supplierVersionNum = VmDirStringToIA(strchr(metaData, ':') + 1);
            consumerVersionNum = VmDirStringToIA(strchr(pAttr->metaData, ':') + 1);

            if (supplierVersionNum > consumerVersionNum)
            {
                VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL,
                    "DetectAndResolveAttrsConflicts: No conflict, supplier version wins. "
                    "DN: %s, attr: %s, supplier attr meta: %s, consumer attr meta: %s ",
                     pSupplierEntry->dn.lberbv.bv_val, pAttr->type.lberbv.bv_val, metaData, pAttr->metaData );
            }
            else if (supplierVersionNum < consumerVersionNum)
            {
                VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
                    "DetectAndResolveAttrsConflicts: Possible conflict, supplier version loses. "
                    "DN: %s, attr: %s, supplier attr meta: %s, consumer attr meta: %s",
                    pSupplierEntry->dn.lberbv.bv_val, pAttr->type.lberbv.bv_val, metaData, pAttr->metaData );

                if (VmDirStringCompareA( pAttr->type.lberbv_val, ATTR_USN_CHANGED, FALSE ) == 0)
                {
                    // Need to keep usnChanged to advance localUSN for this replication change.
                    VmDirFreeBervalContent(pAttrAttrSupplierMetaData->vals+i);
                    retVal = VmDirAllocateStringPrintf( &(pAttrAttrSupplierMetaData->vals[i].lberbv_val),
                                                        "%s:%s",
                                                        ATTR_USN_CHANGED, pAttr->metaData);
                    BAIL_ON_VMDIR_ERROR(retVal);
                    pAttrAttrSupplierMetaData->vals[i].bOwnBvVal = TRUE;
                }
                else
                {
                    // Supplier meta-data loses. pAttrAttrSupplierMetaData->vals[i].lberbv.bv_val conatins just the attribute name
                    // followed by a ':' now, and no associated meta data.
                    *metaData = '\0';
                }
                pAttrAttrSupplierMetaData->vals[i].lberbv.bv_len = strlen(pAttrAttrSupplierMetaData->vals[i].lberbv.bv_val);
            }
            else // supplierVersionNum = consumerVersionNum, compare serverIds, lexicographically larger one wins
            {
                // Format is: <local USN>:<version no>:<originating server ID>:<originating time>:<originating USN>
                char * supplierInvocationId = strchr(strchr(metaData, ':') + 1, ':') + 1;
                char * consumerInvocationId = strchr(strchr(pAttr->metaData, ':') + 1, ':') + 1;

                // compare supplier and consumer attr content, log warning msg if different.
                BOOLEAN bIsSameAttrValue = _VmDirIsBenignReplConflict(pAttr, pSupplierEntry, pConsumerEntry);

                if (strncmp( supplierInvocationId, consumerInvocationId, VMDIR_GUID_STR_LEN ) < 0)
                {
                    if (!bIsSameAttrValue)
                    {
                        VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
                            "DetectAndResolveAttrsConflicts: Possible conflict, supplier serverId loses. "
                            "DN: %s, attr: %s, supplier attr meta: %s, consumer attr meta: %s",
                            pSupplierEntry->dn.lberbv.bv_val, pAttr->type.lberbv.bv_val, metaData, pAttr->metaData );
                    }

                    if (VmDirStringCompareA( pAttr->type.lberbv_val, ATTR_USN_CHANGED, FALSE ) == 0)
                    {
                        // Need to keep usnChanged to advance localUSN for this replication change.
                        VmDirFreeBervalContent(pAttrAttrSupplierMetaData->vals+i);
                        retVal = VmDirAllocateStringPrintf( &(pAttrAttrSupplierMetaData->vals[i].lberbv_val),
                                                            "%s:%s",
                                                            ATTR_USN_CHANGED, pAttr->metaData);
                        BAIL_ON_VMDIR_ERROR(retVal);
                        pAttrAttrSupplierMetaData->vals[i].bOwnBvVal = TRUE;
                    }
                    else
                    {
                        // Supplier meta-data loses. pAttrAttrSupplierMetaData->vals[i].lberbv.bv_val conatins just the attribute
                        // name followed by a ':' now, and no associated meta data.
                        *metaData = '\0';
                    }
                    pAttrAttrSupplierMetaData->vals[i].lberbv.bv_len = strlen(pAttrAttrSupplierMetaData->vals[i].lberbv.bv_val);
               }
                else
                {
                    if (!bIsSameAttrValue)
                    {
                        VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
                            "DetectAndResolveAttrsConflicts: Possible conflict, supplier serverId wins."
                            "DN: %s, attr: %s, supplier attr meta: %s, consumer attr meta: %s",
                            pSupplierEntry->dn.lberbv.bv_val, pAttr->type.lberbv.bv_val, metaData, pAttr->metaData );
                    }
                }
            }
        }
    }

cleanup:
    VmDirFreeAttribute( pAttr );
    return retVal;

ldaperror:
    goto cleanup;
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

/*
 * Find the attribute that holds attribute meta data.
 * Attributes for usnCreated/usnChanged are updated with current local USN
 * If we are doing a modify, attribute meta data is checked to see what wins.
 *    If supplier attribute won, update its meta data with current local USN.
 * If no attribute meta data exists, create it.
 */
static
int
SetAttributesNewMetaData(
    PVDIR_OPERATION     pOperation,
    PVDIR_ENTRY         pEntry,
    char *              localUsnStr,
    PVDIR_ATTRIBUTE *   ppAttrAttrMetaData,
    ENTRYID             entryId,
    PVDIR_ENTRY         pConsumerEntry
    )
{
    int                 retVal = LDAP_SUCCESS;
    VDIR_ATTRIBUTE *    currAttr = NULL;
    VDIR_ATTRIBUTE *    prevAttr = NULL;
    PVDIR_ATTRIBUTE     pAttrAttrMetaData = NULL;
    size_t              localUsnStrlen = VmDirStringLenA( localUsnStr );

    *ppAttrAttrMetaData = NULL;
    // Set attrMetaData for the attributes, part of the new attrMetaData info is
    // present in the incoming operational attribute "attrMetaData"
    // Remove "attrMetaData" from the Attribute list for the entry.
    // Also set new local values for uSNCreated uSNChanged

    for (prevAttr = NULL, currAttr = pEntry->attrs; currAttr;
         prevAttr = currAttr, currAttr = currAttr->next)
    {
        if (VmDirStringCompareA( currAttr->type.lberbv.bv_val, ATTR_ATTR_META_DATA, FALSE ) == 0)
        { // Remove "attrMetaData" attribute from the list
            if (prevAttr == NULL)
            {
                pEntry->attrs = currAttr->next;
            }
            else
            {
                prevAttr->next = currAttr->next;
            }
            *ppAttrAttrMetaData = pAttrAttrMetaData = currAttr;
            continue;
        }
        if (VmDirStringCompareA( currAttr->type.lberbv.bv_val, ATTR_USN_CREATED, FALSE ) == 0 ||
            VmDirStringCompareA( currAttr->type.lberbv.bv_val, ATTR_USN_CHANGED, FALSE ) == 0)
        {
            retVal = VmDirAllocateMemory( localUsnStrlen + 1, (PVOID*)&currAttr->vals[0].lberbv.bv_val );
            BAIL_ON_VMDIR_ERROR(retVal);

            retVal = VmDirCopyMemory( currAttr->vals[0].lberbv.bv_val, localUsnStrlen + 1, localUsnStr, localUsnStrlen );
            BAIL_ON_VMDIR_ERROR(retVal);

            currAttr->vals[0].lberbv.bv_len = localUsnStrlen;
            currAttr->vals[0].bOwnBvVal = TRUE;
            continue;
        }
    }
    if (pAttrAttrMetaData == NULL) // Hmmm ... attrMetaData not there?
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "SetAttributesNewMetaData: attrMetaData attribute not present in Entry: %s",
                  pEntry->dn.lberbv.bv_val );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    if (pOperation->reqCode == LDAP_REQ_MODIFY)
    {
        retVal = DetectAndResolveAttrsConflicts(
                        pOperation,
                        pEntry,
                        pAttrAttrMetaData,
                        entryId,
                        pConsumerEntry);
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    // SJ-TBD: Following logic to look for an attribute's meta-data in attrMetaData attribute values needs to be optimized.
    for (currAttr = pEntry->attrs; currAttr; currAttr = currAttr->next)
    {
        int i = 0;

        // Look for attribute meta data value for the currAttr, and update the localUSN
        for (i = 0; pAttrAttrMetaData->vals[i].lberbv.bv_val != NULL; i++)
        {
            // Format is: <attr name>:<local USN>:<version no>:<originating server ID>:<originating time>:<originating USN>
            char * metaData = VmDirStringChrA( pAttrAttrMetaData->vals[i].lberbv.bv_val, ':');
            if (metaData == NULL)
            {
                // Skipping metadata processing for ObjectGUID which should
                // never change.
                continue;
            }

            // metaData now points to <local USN>..., if meta data present, otherwise '\0'
            metaData++;

            if ((currAttr->type.lberbv.bv_len == (metaData - pAttrAttrMetaData->vals[i].lberbv.bv_val - 1)) &&
                VmDirStringNCompareA( currAttr->type.lberbv.bv_val, pAttrAttrMetaData->vals[i].lberbv.bv_val,
                                      currAttr->type.lberbv.bv_len, FALSE ) == 0)
            {
                // A loser meta-data => Set no meta-data
                if (*metaData == '\0')
                {
                    VmDirStringCpyA( currAttr->metaData, VMDIR_MAX_ATTR_META_DATA_LEN, "" );
                }
                else
                { // A winning supplier attribute meta data.
                    VmDirStringCpyA( currAttr->metaData, VMDIR_MAX_ATTR_META_DATA_LEN, localUsnStr );
                    currAttr->metaData[localUsnStrlen] = ':';
                    // skip localUSN coming from the replication partner
                    metaData = VmDirStringChrA( metaData, ':');
                    VmDirStringCpyA( currAttr->metaData + localUsnStrlen + 1 /* for : */,
                                     (VMDIR_MAX_ATTR_META_DATA_LEN - localUsnStrlen - 1), metaData + 1 /* skip : */);
                }
                // Mark attribute meta data value as "used"
                VmDirFreeBervalContent(pAttrAttrMetaData->vals+i);
                pAttrAttrMetaData->vals[i].lberbv.bv_val = "";
                pAttrAttrMetaData->vals[i].lberbv.bv_len = 0;
                break;
            }
        }
        // No matching attribute meta data found, a local/non-replicated attribute.
        // => create attrMetaData for the local/non-replicated attribute
        if (pAttrAttrMetaData->vals[i].lberbv.bv_val == NULL)
        {
            char  origTimeStamp[VMDIR_ORIG_TIME_STR_LEN];

            if (VmDirGenOriginatingTimeStr( origTimeStamp ) != 0)
            {
                VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "SetAttributesNewMetaData: VmDirGenOriginatingTimeStr failed." );
                retVal = LDAP_OPERATIONS_ERROR;
                BAIL_ON_VMDIR_ERROR( retVal );
            }
            if (pOperation->reqCode == LDAP_REQ_ADD)
            {
                // Format is: <local USN>:<version no>:<originating server ID>:<originating time>:<originating USN>
                if ((retVal = VmDirStringNPrintFA( currAttr->metaData, sizeof( currAttr->metaData ),
                                                   sizeof( currAttr->metaData ) - 1, "%s:%s:%s:%s:%s", localUsnStr, "1",
                                                   gVmdirServerGlobals.invocationId.lberbv.bv_val, origTimeStamp,
                                                   localUsnStr )) != 0)
                {
                    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                              "SetAttributesNewMetaData: VmDirStringNPrintFA failed with error code = %d.", retVal );
                    retVal = LDAP_OPERATIONS_ERROR;
                    BAIL_ON_VMDIR_ERROR( retVal );
                }
            }
            else if (pOperation->reqCode == LDAP_REQ_MODIFY || pOperation->reqCode == LDAP_REQ_DELETE)
            { // SJ-TBD: version number should really be incremented instead of setting to 1.
              // But, currently, for the local attributes that we are dealing with, exact version # is not important.

                // Format is: <local USN>:<version no>:<originating server ID>:<originating time>:<originating USN>
                if ((retVal = VmDirStringNPrintFA( currAttr->metaData, sizeof( currAttr->metaData ),
                                                   sizeof( currAttr->metaData ) - 1, "%s:%s:%s:%s:%s", localUsnStr, "1",
                                                   gVmdirServerGlobals.invocationId.lberbv.bv_val, origTimeStamp,
                                                   localUsnStr )) != 0)
                {
                    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                              "SetAttributesNewMetaData: VmDirStringNPrintFA failed with error code = %d.", retVal );
                    retVal = LDAP_OPERATIONS_ERROR;
                    BAIL_ON_VMDIR_ERROR( retVal );
                }
            }
            else
            {
                assert( FALSE );
            }
        }
    } // for loop to set attribute meta data for all the attributes.

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
    VDIR_OPERATION *    pOperation,
    PVDIR_ENTRY         pEntry,
    USN *               pNextLocalUsn,
    ENTRYID             entryId
    )
{
    int                 retVal = LDAP_SUCCESS;
    VDIR_MODIFICATION * mod = NULL;
    VDIR_ATTRIBUTE *    currAttr = NULL;
    unsigned int        i = 0;
    USN                 localUsn = 0;
    char                localUsnStr[VMDIR_MAX_USN_STR_LEN];
    size_t              localUsnStrlen = 0;
    int                 dbRetVal = 0;
    PVDIR_ATTRIBUTE     pAttrAttrMetaData = NULL;
    BOOLEAN             isDeleteObjReq = FALSE;
    VDIR_MODIFICATION * lastKnownDNMod = NULL;
    PVDIR_SCHEMA_CTX    pSchemaCtx = pOperation->pSchemaCtx;
    ModifyReq *         mr = &(pOperation->request.modifyReq);
    VDIR_ENTRY          consumerEntry = {0};

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

    if ((dbRetVal = pOperation->pBEIF->pfnBEGetNextUSN( pOperation->pBECtx, &localUsn )) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "SetupReplModifyRequest: BdbGetNextUSN failed with error code: %d, error string: %s",
                  dbRetVal, VDIR_SAFE_STRING(pOperation->pBEErrorMsg) );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    if ((retVal = VmDirStringNPrintFA( localUsnStr, sizeof(localUsnStr), sizeof(localUsnStr) - 1, "%ld",
                                       localUsn)) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "SetupReplModifyRequest: VmDirStringNPrintFA failed with error code: %d", retVal );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    localUsnStrlen = VmDirStringLenA( localUsnStr );

    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "SetupReplModifyRequest: next generated localUSN: %s", localUsnStr );

    consumerEntry.eId = entryId;
    retVal = SetAttributesNewMetaData(pOperation, pEntry, localUsnStr, &pAttrAttrMetaData, entryId, &consumerEntry);
    BAIL_ON_VMDIR_ERROR( retVal );

    for (currAttr = pEntry->attrs; currAttr; currAttr = currAttr->next)
    {
        // Skip attributes that have loser attribute meta data => no mods for them
        if (currAttr->metaData[0] == '\0')
        {
            continue;
        }

        if (VmDirStringCompareA(currAttr->type.lberbv.bv_val, ATTR_OBJECT_GUID, FALSE) == 0)
        {
            // Skipping metadata processing for ObjectGUID which should
            // never change.
            continue;
        }

        if (VmDirAllocateMemory( sizeof(VDIR_MODIFICATION), (PVOID *)&mod ) != 0)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "SetupReplModifyRequest: VmDirAllocateMemory error" );
            retVal = LDAP_OPERATIONS_ERROR;;
            BAIL_ON_VMDIR_ERROR( retVal );
        }
        mod->operation = MOD_OP_REPLACE;

        retVal = VmDirAttributeInitialize( currAttr->type.lberbv.bv_val, currAttr->numVals, pSchemaCtx, &mod->attr );
        BAIL_ON_VMDIR_ERROR( retVal );
        // Copy updated meta-data
        VmDirStringCpyA( mod->attr.metaData, VMDIR_MAX_ATTR_META_DATA_LEN, currAttr->metaData );
        for (i = 0; i < currAttr->numVals; i++)
        {
            if (VmDirBervalContentDup( &currAttr->vals[i], &mod->attr.vals[i] ) != 0)
            {
                VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "SetupReplModifyRequest: BervalContentDup failed." );
                retVal = LDAP_OPERATIONS_ERROR;
                BAIL_ON_VMDIR_ERROR( retVal );
            }
        }
        if (VmDirStringCompareA( mod->attr.type.lberbv.bv_val, ATTR_IS_DELETED, FALSE ) == 0 &&
            VmDirStringCompareA( mod->attr.vals[0].lberbv.bv_val, VMDIR_IS_DELETED_TRUE_STR, FALSE ) == 0)
        {
            isDeleteObjReq = TRUE;
        }
        if (VmDirStringCompareA( mod->attr.type.lberbv.bv_val, ATTR_LAST_KNOWN_DN, FALSE ) == 0)
        {
            lastKnownDNMod = mod;
        }

        mod->next = mr->mods;
        mr->mods = mod;
        mr->numMods++;
    }

    // Create Delete mods
    for (i = 0; pAttrAttrMetaData->vals[i].lberbv.bv_val != NULL; i++)
    {
        char *metaData = NULL;

        // Attribute meta data doesn't exist
        if (pAttrAttrMetaData->vals[i].lberbv.bv_len == 0)
        {
            continue;
        }

        metaData = VmDirStringChrA( pAttrAttrMetaData->vals[i].lberbv.bv_val, ':');
        if (metaData == NULL)
        {
            // Skipped metadata processing for ObjectGUID
            continue;
        }

        // Skip loser meta data (i.e. it's empty)
        if (*(metaData + 1 /* skip ':' */) == '\0')
        {
            continue;
        }

        // => over-write ':', pAttrAttrMetaData->vals[i].lberbv.bv_val points to be attribute name now
        *metaData++ = '\0';

        if (VmDirStringCompareA(pAttrAttrMetaData->vals[i].lberbv.bv_val, ATTR_OBJECT_GUID, FALSE) == 0)
        {
            // ObjectGUID is never modified
            continue;
        }

        if (VmDirAllocateMemory( sizeof(VDIR_MODIFICATION), (PVOID *)&mod ) != 0)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "SetupReplModifyRequest: VmDirAllocateMemory error" );
            retVal = LDAP_OPERATIONS_ERROR;;
            BAIL_ON_VMDIR_ERROR( retVal );
        }
        mod->operation = MOD_OP_DELETE;

        retVal = VmDirAttributeInitialize( pAttrAttrMetaData->vals[i].lberbv.bv_val, 0, pSchemaCtx, &mod->attr );
        BAIL_ON_VMDIR_ERROR( retVal );

        // Set localUsn in the metaData

        VmDirStringCpyA( mod->attr.metaData, VMDIR_MAX_ATTR_META_DATA_LEN, localUsnStr );
        mod->attr.metaData[localUsnStrlen] = ':';
        // skip localUSN coming from the replication partner
        metaData = VmDirStringChrA( metaData, ':') + 1;
        VmDirStringCpyA( mod->attr.metaData + localUsnStrlen + 1 /* for : */,
                         (VMDIR_MAX_ATTR_META_DATA_LEN - localUsnStrlen - 1), metaData );

        mod->next = mr->mods;
        mr->mods = mod;
        mr->numMods++;
    }

    if (isDeleteObjReq)
    {
        if (VmDirBervalContentDup( &lastKnownDNMod->attr.vals[0], &mr->dn ) != 0)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "ReplDeleteEntry: BervalContentDup failed." );
            retVal = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR( retVal );
        }
    }

    *pNextLocalUsn = localUsn;

cleanup:
    // pAttrAttrMetaData is local, needs to be freed within the call
    VmDirFreeAttribute( pAttrAttrMetaData );
    VmDirFreeEntryContent(&consumerEntry);

    return retVal;

error:
    goto cleanup;
}

/*
 * Determine whether the supplier's attr-value-meta-data wins by checking it against local
 * attr-meta-data and local attr-value-meta-data.
 * It first compares the <version><invocation-id> of that in local attr-meta-data which was
 * applied either in the previous transaction or the previous modification in the current transactions.
 * Then if the <version><invocation-id> matches, it looks up the local server to see if the same
 * attr-value-meta-data existi: if supplier's attr-value-meta-data has a newer timestamp then
 * it wins and inScope set to TRUE.
 */
static
int
_VmDirAttrValueMetaResolve(
    PVDIR_OPERATION pModOp,
    PVDIR_ATTRIBUTE pAttr,
    PVDIR_BERVALUE  suppAttrMetaValue,
    ENTRYID         entryId,
    PBOOLEAN        pInScope
    )
{
    int retVal = 0;
    char *ps = NULL, *pps = NULL, *ps_ts = NULL;
    char *pc = NULL, *ppc = NULL;
    int rc = 0;
    int psv_len = 0;
    VDIR_BERVALUE *pAVmeta = NULL;
    DEQUE valueMetaData = {0};

    *pInScope = TRUE;
    retVal = pModOp->pBEIF->pfnBEGetAttrMetaData(pModOp->pBECtx, pAttr, entryId);
    BAIL_ON_VMDIR_ERROR(retVal);

    ps = suppAttrMetaValue->lberbv.bv_val;
    VALUE_META_TO_NEXT_FIELD(ps, 2);
    pps = VmDirStringChrA(VmDirStringChrA(ps, ':')+1, ':');
    *pps = '\0';
    //ps points to supplier attr-value-meta "<version><originating-server-id>"

    pc = pAttr->metaData;
    VALUE_META_TO_NEXT_FIELD(pc, 1);
    ppc = VmDirStringChrA(VmDirStringChrA(pc, ':')+1, ':');
    *ppc = '\0';
    //pc points to consumer attr-meta "<version><originating-server-id>"

    rc = strcmp(ps, pc);
    *pps = ':';
    *ppc = ':';
    if (rc)
    {
        //consumer <version><originating-server-id> in metaValueData
        //   not match supplier's <version<<originating-server-id> in metaData
        //   this value-meta-data out of scope
        *pInScope = FALSE;
        goto cleanup;
    }

    retVal = pModOp->pBEIF->pfnBEGetAttrValueMetaData(pModOp->pBECtx, entryId, pAttr->pATDesc->usAttrID, &valueMetaData);
    BAIL_ON_VMDIR_ERROR(retVal);

    ps = suppAttrMetaValue->lberbv.bv_val;
    VALUE_META_TO_NEXT_FIELD(ps, 8);
    pps = VmDirStringChrA(ps, ':');
    //ps points to <value-size>:<value>
    *pps = '\0';
    psv_len = VmDirStringToIA(ps);
    *pps = ':';
    VALUE_META_TO_NEXT_FIELD(ps, 1);
    //ps now points to <value> of supplier's attr-value-meta-data
    ps_ts = suppAttrMetaValue->lberbv.bv_val;
    VALUE_META_TO_NEXT_FIELD(ps_ts, 5);
    //ps_ts points to <value-change-originating time>

    while(!dequeIsEmpty(&valueMetaData))
    {
        int pcv_len = 0;
        char *pc_ts = NULL;

        VmDirFreeBerval(pAVmeta);
        pAVmeta = NULL;

        dequePopLeft(&valueMetaData, (PVOID*)&pAVmeta);
        pc = pAVmeta->lberbv.bv_val;
        VALUE_META_TO_NEXT_FIELD(pc, 8);
        ppc = VmDirStringChrA(pc, ':');
        *ppc = '\0';
        pcv_len = VmDirStringToIA(pc);
        *ppc = ':';
        if (psv_len != pcv_len)
        {
            continue;
        }
        VALUE_META_TO_NEXT_FIELD(pc, 1);
        if (memcmp(ps, pc, pcv_len))
        {
            continue;
        }

        // Now found attr-value-meta-data with the same attribute value.
        // If this one' timestamp is later than the supplier's,
        // then the consumer won, this may occur like: add attr-a/value-a, then delete attr-a/value-a (in two repl cycles)
        pc_ts = pAVmeta->lberbv.bv_val;
        VALUE_META_TO_NEXT_FIELD(pc_ts, 5);
        rc = strncmp(pc_ts, ps_ts, VMDIR_ORIG_TIME_STR_LEN);
        if (rc > 0)
        {
          // If any of newer attr-value-meta-data with that entryid/attr-id/attr-value in the consumer,
          //   then the supplier attr-value-meta-data lose
          *pInScope = FALSE;
          VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "_VmDirAttrValueMetaResolve: supplier attr-value-meta lose: %s consumer: %s",
                VDIR_SAFE_STRING(suppAttrMetaValue->lberbv.bv_val), VDIR_SAFE_STRING(pAVmeta->lberbv.bv_val));
        }
    }
    if (*pInScope)
    {
        VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "_VmDirAttrValueMetaResolve: supplier attr-value-meta won: %s",
                VDIR_SAFE_STRING(suppAttrMetaValue->lberbv.bv_val));
    }

cleanup:
    VmDirFreeBerval(pAVmeta);
    VmDirFreeAttrValueMetaDataContent(&valueMetaData);
    return retVal;

error:
    goto cleanup;
}

/*
 * First determine if each attribute value meta data in pAttrAttrValueMetaData
 * win those in local database (if they exist locally); then create mod for
 * adding/deleting the attribute value for those with winning attribute value meta data
 * from supplier.
 */
static
int
_VmSetupValueMetaData(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    PVDIR_OPERATION     pModOp,
    PVDIR_ATTRIBUTE     pAttrAttrValueMetaData,
    USN                 localUsn,
    ENTRYID             entryId
    )
{
    PVDIR_ATTRIBUTE pAttr = NULL;
    int i = 0;
    int retVal = 0;
    VDIR_BERVALUE * pAVmeta = NULL;
    char av_meta_pre[VMDIR_MAX_ATTR_META_DATA_LEN];
    int new_av_len = 0;
    BOOLEAN inScope = FALSE;
    VDIR_MODIFICATION * mod = NULL, *modp = NULL, *pre_modp = NULL;
    ModifyReq * mr = NULL;
    VDIR_MODIFICATION * currMod = NULL;
    VDIR_MODIFICATION * tmpMod = NULL;
    char *p = NULL, *pp = NULL;

    mr = &(pModOp->request.modifyReq);
    //clear all mods that should have been applied.
    for ( currMod = mr->mods; currMod != NULL; )
    {
        tmpMod = currMod->next;
        VmDirModificationFree(currMod);
        currMod = tmpMod;
    }
    mr->mods = NULL;
    mr->numMods = 0;

    //format of a value meta data item:
    //       <attr-name>:<local-usn>:<version-no>:<originating-server-id>:<value-change-originating-server-id>
    //       :<value-change-originating time>:<value-change-originating-usn>:
    //Remaining portion of attr-value-meta-data:   <opcode>:<value-size>:<value>
    for ( i=0; i<(int)pAttrAttrValueMetaData->numVals; i++ )
    {
       if (!VmDirValidValueMetaEntry(&pAttrAttrValueMetaData->vals[i]))
       {
          retVal = ERROR_INVALID_PARAMETER;
          VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmSetupValueMetaData: invalid attr-value-meta: %s",
                         VDIR_SAFE_STRING(pAttrAttrValueMetaData->vals[i].lberbv.bv_val));
          BAIL_ON_VMDIR_ERROR(retVal);
       }

       p = pAttrAttrValueMetaData->vals[i].lberbv.bv_val;
       pp = VmDirStringChrA(p, ':');
       *pp = '\0';
       // p points to attr-name
       VmDirFreeAttribute(pAttr);
       pAttr = NULL;

       retVal = VmDirAttributeAllocate(p, 1, pSchemaCtx, &pAttr);
       BAIL_ON_VMDIR_ERROR(retVal);

       *pp = ':';
       retVal = _VmDirAttrValueMetaResolve(pModOp, pAttr, &pAttrAttrValueMetaData->vals[i], entryId, &inScope);
       BAIL_ON_VMDIR_ERROR(retVal);

       if (!inScope)
       {
          continue;
       }

       VALUE_META_TO_NEXT_FIELD(p, 2);
       // p now points to <version>...
       // Need to replace supp's <local-usn> with new locally generated local-usn.
       retVal = VmDirStringNPrintFA(av_meta_pre, sizeof(av_meta_pre), sizeof(av_meta_pre) - 1,
                    "%s:%ld:", pAttr->type.lberbv.bv_val, localUsn);
       BAIL_ON_VMDIR_ERROR(retVal);

       //av_meta_pre contains "<attr-name>:<new-local-usn>:"
       //re-calculate the length of attr-value-meta-data.
       new_av_len =  (int)strlen(av_meta_pre) +
                     (int)pAttrAttrValueMetaData->vals[i].lberbv.bv_len -
                     (int)(p - pAttrAttrValueMetaData->vals[i].lberbv.bv_val);
       retVal = VmDirAllocateMemory(sizeof(VDIR_BERVALUE), (PVOID)&pAVmeta);
       BAIL_ON_VMDIR_ERROR(retVal);

       retVal = VmDirAllocateMemory(new_av_len, (PVOID)&pAVmeta->lberbv.bv_val);
       BAIL_ON_VMDIR_ERROR(retVal);

       pAVmeta->bOwnBvVal = TRUE;
       pAVmeta->lberbv.bv_len = new_av_len;
       retVal = VmDirCopyMemory(pAVmeta->lberbv.bv_val, new_av_len, av_meta_pre, strlen(av_meta_pre));
       BAIL_ON_VMDIR_ERROR(retVal);

       retVal = VmDirCopyMemory(pAVmeta->lberbv.bv_val+strlen(av_meta_pre),
                                 new_av_len - strlen(av_meta_pre), p, new_av_len - strlen(av_meta_pre));
       BAIL_ON_VMDIR_ERROR(retVal);

       //Write the attr-value-meta-data to backend index database.
       retVal = dequePush(&pAttr->valueMetaDataToAdd, (PVOID)pAVmeta);
       BAIL_ON_VMDIR_ERROR(retVal);
       pAVmeta = NULL;

       retVal = pModOp->pBEIF->pfnBEUpdateAttrValueMetaData( pModOp->pBECtx, entryId, pAttr->pATDesc->usAttrID,
                                                             BE_INDEX_OP_TYPE_UPDATE, &pAttr->valueMetaDataToAdd );
       BAIL_ON_VMDIR_ERROR(retVal);

       //Now create mod for attribute value add/delete.
       retVal = VmDirAllocateMemory( sizeof(VDIR_MODIFICATION), (PVOID *)&mod);
       BAIL_ON_VMDIR_ERROR(retVal);

       VALUE_META_TO_NEXT_FIELD(p, 5);
       // p points to <opcode>...
       pp = VmDirStringChrA(p, ':');
       *pp = '\0';
       mod->operation = VmDirStringToIA(p);
       *pp = ':';
       retVal = VmDirAttributeInitialize(pAttr->type.lberbv.bv_val, 1, pSchemaCtx, &mod->attr );
       BAIL_ON_VMDIR_ERROR( retVal );

       VALUE_META_TO_NEXT_FIELD(p, 1);
       // p points to <value-size><value>
       pp = VmDirStringChrA(p, ':');
       *pp = '\0';
       mod->attr.vals[0].lberbv.bv_len = VmDirStringToIA(p);
       *pp = ':';
       VALUE_META_TO_NEXT_FIELD(p, 1);
       //p points to <value>
       retVal = VmDirAllocateMemory(mod->attr.vals[0].lberbv.bv_len + 1, (PVOID *)&mod->attr.vals[0].lberbv.bv_val);
       BAIL_ON_VMDIR_ERROR( retVal );

       mod->attr.vals[0].bOwnBvVal = TRUE;
       retVal = VmDirCopyMemory(mod->attr.vals[0].lberbv.bv_val, mod->attr.vals[0].lberbv.bv_len,
                                 p, mod->attr.vals[0].lberbv.bv_len);
       BAIL_ON_VMDIR_ERROR( retVal );

       for( modp=mr->mods; modp; pre_modp=modp,modp=modp->next )
       {
           if (modp->attr.pATDesc->usAttrID == mod->attr.pATDesc->usAttrID &&
               modp->operation == mod->operation)
           {
               break;
           }
       }

       if (modp == NULL)
       {
           if (pre_modp == NULL)
           {
               mr->mods = mod;
           }
           else
           {
               pre_modp->next = mod;
           }
           mr->numMods++;
           mod = NULL;
       }
       else
       {
           // add/delete attr value on the same attribute exists, merge the new mod into it.
           retVal = VmDirReallocateMemoryWithInit( modp->attr.vals, (PVOID*)(&(modp->attr.vals)),
                         (modp->attr.numVals + 2)*sizeof(VDIR_BERVALUE), (modp->attr.numVals + 1)*sizeof(VDIR_BERVALUE));
           BAIL_ON_VMDIR_ERROR(retVal);
           retVal = VmDirBervalContentDup(&mod->attr.vals[0], &modp->attr.vals[modp->attr.numVals]);
           BAIL_ON_VMDIR_ERROR(retVal);
           modp->attr.numVals++;
           memset(&(modp->attr.vals[modp->attr.numVals]), 0, sizeof(VDIR_BERVALUE) );
           VmDirModificationFree(mod);
           mod = NULL;
       }
    }

cleanup:
    VmDirFreeAttribute(pAttr);
    return retVal;

error:
    VmDirFreeBerval(pAVmeta);
    VmDirModificationFree(mod);
    goto cleanup;
}

/*
 * Detach attribute value meta data from the entry's attributes,
 * and set ppAttrAttrValueMetaData to the attribute value meta attribute
 * so that it will be handled seperated.
 */
static
int
_VmDirDetatchValueMetaData(
    PVDIR_OPERATION     pOperation,
    PVDIR_ENTRY         pEntry,
    PVDIR_ATTRIBUTE *   ppAttrAttrValueMetaData
    )
{
    int                 retVal = LDAP_SUCCESS;
    VDIR_ATTRIBUTE *    currAttr = NULL;
    VDIR_ATTRIBUTE *    prevAttr = NULL;
    PVDIR_ATTRIBUTE     pAttrAttrValueMetaData = NULL;

    *ppAttrAttrValueMetaData = NULL;
    for ( prevAttr = NULL, currAttr = pEntry->attrs;
          currAttr;
          prevAttr = currAttr, currAttr = currAttr->next )
    {
        if (VmDirStringCompareA( currAttr->type.lberbv.bv_val, ATTR_ATTR_VALUE_META_DATA, FALSE ) == 0)
        { // Remove "attrValueMetaData" attribute from the list
            if (prevAttr == NULL)
            {
                pEntry->attrs = currAttr->next;
            }
            else
            {
                prevAttr->next = currAttr->next;
            }
            *ppAttrAttrValueMetaData = pAttrAttrValueMetaData = currAttr;
            goto cleanup;
        }
    }

cleanup:
    return retVal;
}

/* If any mod is a MOD_OP_REPLACE on a multi-value attribute,
 * delete that attribute's attr-value-meta-data
 */
static
int
_VmDeleteOldValueMetaData(
    PVDIR_OPERATION     pModOp,
    PVDIR_MODIFICATION  pMods,
    ENTRYID             entryId
    )
{
    int retVal = LDAP_SUCCESS;
    VDIR_MODIFICATION * modp = NULL;
    DEQUE valueMetaDataToDelete = {0};

    for( modp=pMods; modp; modp=modp->next )
    {
         if (modp->operation != MOD_OP_REPLACE || modp->attr.pATDesc->bSingleValue)
         {
             continue;
         }
         retVal = pModOp->pBEIF->pfnBEGetAttrValueMetaData(pModOp->pBECtx, entryId, modp->attr.pATDesc->usAttrID, &valueMetaDataToDelete);
         BAIL_ON_VMDIR_ERROR(retVal);
         if (dequeIsEmpty(&valueMetaDataToDelete))
         {
             continue;
         }
         retVal = pModOp->pBEIF->pfnBEUpdateAttrValueMetaData( pModOp->pBECtx, entryId, modp->attr.pATDesc->usAttrID,
                                                             BE_INDEX_OP_TYPE_DELETE, &valueMetaDataToDelete );
         BAIL_ON_VMDIR_ERROR(retVal);
    }

cleanup:
    VmDirFreeAttrValueMetaDataContent(&valueMetaDataToDelete);
    return retVal;

error:
    goto cleanup;
}

/*
 * Attach and alter attribute value meta data to that attribute in pEntry
 * so that they can be inserted into the backend index when the
 * entry is added to backend.
 */
static
int
_VmDirAttachValueMetaData(
    PVDIR_ATTRIBUTE pAttrAttrValueMetaData,
    PVDIR_ENTRY     pEntry,
    USN             localUsn
    )
{
    int i = 0;
    int retVal = 0;
    char *p = NULL;
    VDIR_BERVALUE *pAVmeta = NULL;

    if (pAttrAttrValueMetaData == NULL)
    {
        goto cleanup;
    }

    for (i = 0; pAttrAttrValueMetaData->vals[i].lberbv.bv_val != NULL; i++)
    {
        if (!VmDirValidValueMetaEntry(&pAttrAttrValueMetaData->vals[i]))
        {
            retVal = ERROR_INVALID_PARAMETER;
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirAttachValueMetaData: invalid attr-value-meta: %s",
                     VDIR_SAFE_STRING(pAttrAttrValueMetaData->vals[i].lberbv.bv_val));
            BAIL_ON_VMDIR_ERROR(retVal);
        }
        if (pAttrAttrValueMetaData->vals[i].lberbv.bv_len != 0)
        {
            PVDIR_ATTRIBUTE attr = NULL;
            p = VmDirStringChrA( pAttrAttrValueMetaData->vals[i].lberbv.bv_val, ':');
            *p = '\0';
            attr = VmDirEntryFindAttribute(pAttrAttrValueMetaData->vals[i].lberbv.bv_val, pEntry);
            *p = ':';
            if (attr)
            {
               int new_av_len = 0;
               char av_meta_pre[VMDIR_MAX_ATTR_META_DATA_LEN] = {0};

               VALUE_META_TO_NEXT_FIELD(p, 2);
               // p now points to <version>...
               retVal = VmDirStringNPrintFA(av_meta_pre, sizeof(av_meta_pre), sizeof(av_meta_pre) -1,
                            "%s:%ld:", attr->type.lberbv.bv_val, localUsn);
               BAIL_ON_VMDIR_ERROR(retVal);

               //av_meta_pre contains "<attr-name>:<new-local-usn>:"
               //re-calculate the length of attr-value-meta-data.
               new_av_len =  (int)strlen(av_meta_pre) +
                             (int)pAttrAttrValueMetaData->vals[i].lberbv.bv_len -
                             (int)(p - pAttrAttrValueMetaData->vals[i].lberbv.bv_val);
               retVal = VmDirAllocateMemory(sizeof(VDIR_BERVALUE), (PVOID)&pAVmeta);
               BAIL_ON_VMDIR_ERROR(retVal);

               retVal = VmDirAllocateMemory(new_av_len, (PVOID)&pAVmeta->lberbv.bv_val);
               BAIL_ON_VMDIR_ERROR(retVal);

               pAVmeta->bOwnBvVal = TRUE;
               pAVmeta->lberbv.bv_len = new_av_len;
               retVal = VmDirCopyMemory(pAVmeta->lberbv.bv_val, new_av_len, av_meta_pre, strlen(av_meta_pre));
               BAIL_ON_VMDIR_ERROR(retVal);

               retVal = VmDirCopyMemory(pAVmeta->lberbv.bv_val+strlen(av_meta_pre), new_av_len - strlen(av_meta_pre),
                                         p, new_av_len - strlen(av_meta_pre));
               BAIL_ON_VMDIR_ERROR(retVal);

               retVal = dequePush(&attr->valueMetaDataToAdd, pAVmeta);
               BAIL_ON_VMDIR_ERROR(retVal);
               pAVmeta = NULL;
            }
        }
    }

cleanup:
    return retVal;

error:
    VmDirFreeBerval(pAVmeta);
    goto cleanup;
}
