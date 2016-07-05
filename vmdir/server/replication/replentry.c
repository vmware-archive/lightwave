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
    PVDIR_BERVALUE      pDn,
    PVDIR_ATTRIBUTE     pAttrAttrSupplierMetaData);

static
int
SetAttributesNewMetaData(
    PVDIR_OPERATION     pOperation,
    PVDIR_ENTRY         pEntry,
    char *              localUsn,
    PVDIR_ATTRIBUTE *   ppAttrAttrMetaData);

static
int
SetupReplModifyRequest(
    VDIR_OPERATION *    modOp,
    PVDIR_ENTRY         pEntry);

static
int
_VmDirPatchData(
    PVDIR_OPERATION     pOperation);

static DWORD
_VmDirAssignEntryIdIfSpecialInternalEntry(
    PVDIR_ENTRY pEntry
    );

int
ReplFixUpEntryDn(
    PVDIR_ENTRY pEntry
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
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    LDAPMessage *       ldapMsg,
    PVDIR_SCHEMA_CTX*   ppOutSchemaCtx,
    BOOLEAN             bFirstReplicationCycle)
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
    int                 i = 0;
    PVDIR_SCHEMA_CTX    pUpdateSchemaCtx = NULL;

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

    VmDirdSetLimitLocalUsnToBeSupplied(localUsn);

    if ((retVal = VmDirStringNPrintFA( localUsnStr, sizeof(localUsnStr), sizeof(localUsnStr) - 1, "%ld", localUsn)) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "ReplAddEntry: VmDirStringNPrintFA failed with error code: %d", retVal);
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    localUsnStrlen = VmDirStringLenA( localUsnStr );

    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "ReplAddEntry: next generated localUSN: %s", localUsnStr);

    retVal = SetAttributesNewMetaData( &op, pEntry, localUsnStr, &pAttrAttrMetaData );
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

    retVal = _VmDirPatchData( &op );
    BAIL_ON_VMDIR_ERROR( retVal );

    if (bFirstReplicationCycle)
    {
        retVal =  _VmDirAssignEntryIdIfSpecialInternalEntry( pEntry );
        BAIL_ON_VMDIR_ERROR( retVal );
    }

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
                          "For this object, system may not converge.",
                          retVal, pEntry->dn.lberbv.bv_val, pEntry->attrs->type.lberbv.bv_val, pEntry->attrs->metaData );

                break;

            case LDAP_NO_SUCH_OBJECT:
                VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
                          "ReplAddEntryVmDirInternalAddEntry: %d (Parent object does not exist). "
                          "DN: %s, first attribute: %s, it's meta data: '%s' "
                          "NOT resolving this possible replication CONFLICT or out-of-parent-child-order replication scenario. "
                          "For this subtree, system may not converge.",
                          retVal, pEntry->dn.lberbv.bv_val, pEntry->attrs->type.lberbv.bv_val, pEntry->attrs->metaData );
                break;

            default:
                VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                          "ReplAddEntry/VmDirInternalAddEntry:  %d (%s). ",
                          retVal, VDIR_SAFE_STRING( op.ldapResult.pszErrMsg ));
                break;
        }
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    if (pEntry->dn.bvnorm_val)
    {
        VDIR_BERVALUE dn = pEntry->dn;
        size_t offset = dn.bvnorm_len - (SCHEMA_NAMING_CONTEXT_DN_LEN);
        if (VmDirStringCompareA(
                dn.bvnorm_val + offset, SCHEMA_NAMING_CONTEXT_DN, FALSE) == 0)
        {   // schema entry updated, refresh replication schema ctx.
            assert( ppOutSchemaCtx );
            retVal = VmDirSchemaCtxAcquire(&pUpdateSchemaCtx);
            BAIL_ON_VMDIR_ERROR(retVal);
            *ppOutSchemaCtx = pUpdateSchemaCtx;

            VmDirSchemaCtxRelease(pSchemaCtx);
        }
    }

cleanup:
    // pAttrAttrMetaData is local, needs to be freed within the call
    VmDirFreeAttribute( pAttrAttrMetaData );

    VmDirFreeOperationContent(&op);

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
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    LDAPMessage *       ldapMsg)
{
    int                 retVal = LDAP_SUCCESS;
    VDIR_OPERATION      tmpAddOp = {0};
    VDIR_OPERATION      delOp = {0};
    ModifyReq *         mr = &(delOp.request.modifyReq);

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
    retVal = SetupReplModifyRequest( &delOp, tmpAddOp.request.addReq.pEntry);
    BAIL_ON_VMDIR_ERROR( retVal );

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
                          "For this object, system may not converge.",
                          retVal, mr->dn.lberbv.bv_val, mr->mods->attr.type.lberbv.bv_val, mr->mods->attr.metaData );
                retVal = LDAP_SUCCESS;
                break;

            case LDAP_NOT_ALLOWED_ON_NONLEAF:
                VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
                          "ReplDeleteEntry/VmDirInternalDeleteEntry: %d (Operation not allowed on non-leaf). "
                          "DN: %s, first attribute: %s, it's meta data: '%s'. "
                          "NOT resolving this possible replication CONFLICT. "
                          "For this object, system may not converge.",
                          retVal, mr->dn.lberbv.bv_val, mr->mods->attr.type.lberbv.bv_val, mr->mods->attr.metaData );
                break;

            case LDAP_NO_SUCH_ATTRIBUTE:
                VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
                          "ReplDeleteEntry/VmDirInternalDeleteEntry: %d (No such attribute). "
                          "DN: %s, first attribute: %s, it's meta data: '%s'. "
                          "NOT resolving this possible replication CONFLICT. "
                          "For this object, system may not converge. ",
                          retVal, mr->dn.lberbv.bv_val, mr->mods->attr.type.lberbv.bv_val, mr->mods->attr.metaData );
                break;

            default:
                VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                          "ReplDeleteEntry/InternalDeleteEntry: %d (%s). ",
                  retVal, VDIR_SAFE_STRING( delOp.ldapResult.pszErrMsg ));
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
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    LDAPMessage *       ldapMsg,
    PVDIR_SCHEMA_CTX*   ppOutSchemaCtx)
{
    int                 retVal = LDAP_SUCCESS;
    VDIR_OPERATION      modOp = {0};
    ModifyReq *         mr = &(modOp.request.modifyReq);
    int                 dbRetVal = 0;
    BOOLEAN             bHasTxn = FALSE;
    int                 deadLockRetries = 0;
    PVDIR_SCHEMA_CTX    pUpdateSchemaCtx = NULL;
    VDIR_ENTRY          e = {0};

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

    if ((retVal = SetupReplModifyRequest( &modOp, &e)) != LDAP_SUCCESS)
    {
        switch (retVal)
        {
            case LDAP_NO_SUCH_OBJECT:
                VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
                          "ReplModifyEntry/SetupReplModifyRequest: %d (Object does not exist). "
                          "DN: %s, first attribute: %s, it's meta data: '%s'. "
                          "Possible replication CONFLICT. Object will get deleted from the system.",
                          retVal, e.dn.lberbv.bv_val, e.attrs[0].type.lberbv.bv_val,
                          e.attrs[0].metaData );
                break;

            case LDAP_LOCK_DEADLOCK:
                VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
                          "ReplModifyEntry/SetupReplModifyRequest: %d (%s). ",
                          retVal, VDIR_SAFE_STRING( modOp.ldapResult.pszErrMsg ));
                goto txnretry; // Possible retry.

            default:
                VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                          "ReplModifyEntry/SetupReplModifyRequest: %d (%s). ",
                          retVal, VDIR_SAFE_STRING( modOp.ldapResult.pszErrMsg ));
                break;
       }
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    // If some mods left after conflict resolution
    if (mr->mods != NULL)
    {
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

    if (modOp.request.modifyReq.dn.bvnorm_val)
    {
        VDIR_BERVALUE dn = modOp.request.modifyReq.dn;
        size_t offset = dn.bvnorm_len - (SCHEMA_NAMING_CONTEXT_DN_LEN);
        if (VmDirStringCompareA(
                dn.bvnorm_val + offset, SCHEMA_NAMING_CONTEXT_DN, FALSE) == 0)
        {   // schema entry updated, refresh replication schema ctx.
            assert( ppOutSchemaCtx );
            retVal = VmDirSchemaCtxAcquire(&pUpdateSchemaCtx);
            BAIL_ON_VMDIR_ERROR(retVal);
            *ppOutSchemaCtx = pUpdateSchemaCtx;

            VmDirSchemaCtxRelease(pSchemaCtx);
        }
    }

cleanup:
    VmDirFreeOperationContent(&modOp);
    VmDirFreeEntryContent(&e);
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
        retVal = VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION;
        BAIL_ON_VMDIR_ERROR(retVal);
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
    PVDIR_BERVALUE      pDn,
    PVDIR_ATTRIBUTE     pAttrAttrSupplierMetaData)
{
    int             retVal = LDAP_SUCCESS;
    int             dbRetVal = 0;
    PVDIR_ATTRIBUTE pAttr = NULL;
    int             i = 0;
    ENTRYID         entryId = 0;

    assert( pOperation && pOperation->pSchemaCtx && pAttrAttrSupplierMetaData );

    // Normalize DN
    if (pDn->bvnorm_val == NULL)
    {
        if ((retVal = VmDirNormalizeDN( pDn, pOperation->pSchemaCtx)) != 0)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "DetectAndResolveAttrsConflicts: VmDirNormalizeDN failed with "
                      "error code: %d, error string: %s", retVal,
                      VDIR_SAFE_STRING(VmDirSchemaCtxGetErrorMsg(pOperation->pSchemaCtx)));

            BAIL_ON_LDAP_ERROR( retVal, LDAP_OPERATIONS_ERROR, (pOperation->ldapResult.pszErrMsg),
                                "DN normalization failed - (%d)(%s)", retVal,
                                VDIR_SAFE_STRING(VmDirSchemaCtxGetErrorMsg(pOperation->pSchemaCtx)));
        }
    }

    // Get EntryId
    retVal = pOperation->pBEIF->pfnBEDNToEntryId(  pOperation->pBECtx, pDn, &entryId );
    if (retVal != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "DetectAndResolveAttrsConflicts: BdbDNToEntryId failed with error code: %d, "
                  "error string: %s", retVal, VDIR_SAFE_STRING(pOperation->pBEErrorMsg) );

        switch (retVal)
        {
            case ERROR_BACKEND_ENTRY_NOTFOUND:
                BAIL_ON_LDAP_ERROR( retVal, LDAP_NO_SUCH_OBJECT, (pOperation->ldapResult.pszErrMsg),
                                    "DN doesn't exist.");
                break;

            case ERROR_BACKEND_DEADLOCK:
                BAIL_ON_LDAP_ERROR( retVal, LDAP_LOCK_DEADLOCK, (pOperation->ldapResult.pszErrMsg),
                                    "backend read entry failed - (%d)(%s)", retVal,
                                    VDIR_SAFE_STRING(pOperation->pBEErrorMsg));
                break;

            default:
                BAIL_ON_LDAP_ERROR( retVal, LDAP_OPERATIONS_ERROR, (pOperation->ldapResult.pszErrMsg),
                                    "backend read entry failed - (%d)(%s)", retVal,
                                    VDIR_SAFE_STRING(pOperation->pBEErrorMsg));
                break;
        }
    }

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
                     pDn->lberbv.bv_val, pAttr->type.lberbv.bv_val, metaData, pAttr->metaData );
            }
            else if (supplierVersionNum < consumerVersionNum)
            {
                VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
                    "DetectAndResolveAttrsConflicts: Possible conflict, supplier version loses. "
                    "DN: %s, attr: %s, supplier attr meta: %s, consumer attr meta: %s",
                    pDn->lberbv.bv_val, pAttr->type.lberbv.bv_val, metaData, pAttr->metaData );

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

                if (strncmp( supplierInvocationId, consumerInvocationId, VMDIR_GUID_STR_LEN ) < 0)
                {
                    VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
                        "DetectAndResolveAttrsConflicts: Possible conflict, supplier serverId loses. "
                        "DN: %s, attr: %s, supplier attr meta: %s, consumer attr meta: %s",
                        pDn->lberbv.bv_val, pAttr->type.lberbv.bv_val, metaData, pAttr->metaData );

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
                    VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
                        "DetectAndResolveAttrsConflicts: Possible conflict, supplier serverId wins."
                        "DN: %s, attr: %s, supplier attr meta: %s, consumer attr meta: %s",
                        pDn->lberbv.bv_val, pAttr->type.lberbv.bv_val, metaData, pAttr->metaData );
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
    PVDIR_ATTRIBUTE *   ppAttrAttrMetaData)
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

    if (pOperation->reqCode == LDAP_REQ_MODIFY )
    {
        retVal = DetectAndResolveAttrsConflicts( pOperation, &pEntry->dn, pAttrAttrMetaData );
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
    PVDIR_ENTRY         pEntry)
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

    VmDirdSetLimitLocalUsnToBeSupplied(localUsn);

    if ((retVal = VmDirStringNPrintFA( localUsnStr, sizeof(localUsnStr), sizeof(localUsnStr) - 1, "%ld",
                                       localUsn)) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "SetupReplModifyRequest: VmDirStringNPrintFA failed with error code: %d", retVal );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    localUsnStrlen = VmDirStringLenA( localUsnStr );

    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "SetupReplModifyRequest: next generated localUSN: %s", localUsnStr );

    retVal = SetAttributesNewMetaData( pOperation, pEntry, localUsnStr, &pAttrAttrMetaData);
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

cleanup:
    // pAttrAttrMetaData is local, needs to be freed within the call
    VmDirFreeAttribute( pAttrAttrMetaData );

    return retVal;

error:
    goto cleanup;
}


