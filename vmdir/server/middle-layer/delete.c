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
GenerateDeleteAttrsMods(
    PVDIR_OPERATION pOperation,
    VDIR_ENTRY *    pEntry
    );

static
BOOLEAN
_VmDirIsDeletedContainer(
    PCSTR   pszDN
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
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg, "Not bind/authenticate yet" );
    }

    dwError = VmDirInternalDeleteEntry( pOperation );
    BAIL_ON_VMDIR_ERROR( dwError );

    if (pOperation->opType == VDIR_OPERATION_TYPE_EXTERNAL)
    {
        pOperation->pBEIF->pfnBESetMaxOriginatingUSN(pOperation->pBECtx,
                                                     pOperation->pBECtx->wTxnUSN);
    }

    VmDirPerformUrgentReplIfRequired(pOperation, pOperation->pBECtx->wTxnUSN);

cleanup:
    VMDIR_SAFE_FREE_MEMORY( pszLocalErrMsg );
    return pOperation->ldapResult.errCode;

error:
    VMDIR_SET_LDAP_RESULT_ERROR( &(pOperation->ldapResult), dwError, pszLocalErrMsg);
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
    int             retVal = LDAP_SUCCESS;
    int             deadLockRetries = 0;
    VDIR_ENTRY      entry = {0};
    PVDIR_ENTRY     pEntry = NULL;
    BOOLEAN         leafNode = FALSE;
    DeleteReq *     delReq = &(pOperation->request.deleteReq);
    ModifyReq *     modReq = &(pOperation->request.modifyReq);
    BOOLEAN         bIsDomainObject = FALSE;
    BOOLEAN         bHasTxn = FALSE;
    BOOLEAN         bIsDeletedObj = FALSE;
    PSTR            pszLocalErrMsg = NULL;

    assert(pOperation && pOperation->pBECtx->pBE);

    if (VmDirdState() == VMDIRD_STATE_READ_ONLY)
    {
        retVal = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "Server in read-only mode" );
    }

    // Normalize DN
    retVal = VmDirNormalizeDN( &(delReq->dn), pOperation->pSchemaCtx );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "DN normalization failed - (%u)(%s)",
                                  retVal, VDIR_SAFE_STRING(VmDirSchemaCtxGetErrorMsg(pOperation->pSchemaCtx)) );

    // Execute pre modify apply Delete plugin logic
    retVal = VmDirExecutePreModApplyDeletePlugins(pOperation, NULL, retVal);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "PreModApplyDelete plugin failed - (%u)",  retVal );

    retVal = VmDirNormalizeMods( pOperation->pSchemaCtx, modReq->mods, &pszLocalErrMsg );
    BAIL_ON_VMDIR_ERROR( retVal );

    // make sure VDIR_BACKEND_CTX has usn change number by now
    if ( pOperation->pBECtx->wTxnUSN <= 0 )
    {
        retVal = VMDIR_ERROR_NO_USN;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "BECtx.wTxnUSN not set");
    }

    // BUGBUG, need to protect some system entries such as schema,domain....etc?

    // ************************************************************************************
    // transaction retry loop begin.  make sure all function within are retry agnostic.
    // ************************************************************************************
txnretry:
    if (bHasTxn)
    {
        pOperation->pBEIF->pfnBETxnAbort( pOperation->pBECtx );
        bHasTxn = FALSE;
    }

    deadLockRetries++;
    if (deadLockRetries > MAX_DEADLOCK_RETRIES)
    {
        retVal = VMDIR_ERROR_LOCK_DEADLOCK;
        BAIL_ON_VMDIR_ERROR( retVal );
    }
    else
    {
        if (pEntry)
        {
            VmDirFreeEntryContent(pEntry);
            memset(pEntry, 0, sizeof(VDIR_ENTRY));
            pEntry = NULL;
        }

        retVal = pOperation->pBEIF->pfnBETxnBegin( pOperation->pBECtx, VDIR_BACKEND_TXN_WRITE);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "txn begin (%u)(%s)",
                                      retVal, VDIR_SAFE_STRING(pOperation->pBEErrorMsg));
        bHasTxn = TRUE;

        // Read current entry from DB
        retVal = pOperation->pBEIF->pfnBEDNToEntry(
                                    pOperation->pBECtx,
                                    pOperation->pSchemaCtx,
                                    &(delReq->dn),
                                    &entry,
                                    VDIR_BACKEND_ENTRY_LOCK_WRITE);
        if (retVal != 0)
        {
            switch (retVal)
            {
                case VMDIR_ERROR_BACKEND_DEADLOCK:
                    goto txnretry; // Possible retry.

                default:
                    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "(%u)(%s)",
                                                  retVal, VDIR_SAFE_STRING(pOperation->pBEErrorMsg));
            }
        }

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

                switch (retVal)
                {
                    case VMDIR_ERROR_BACKEND_DEADLOCK:
                        goto txnretry; // Possible retry.

                    case VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND:
                        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "parent (%s) not found, (%s)",
                                                      pEntry->pdn.lberbv_val,
                                                      VDIR_SAFE_STRING(pOperation->pBEErrorMsg) );

                    default:
                        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "parent (%s) lookup failed, (%s)",
                                                      pEntry->pdn.lberbv_val,
                                                      VDIR_SAFE_STRING(pOperation->pBEErrorMsg) );
                }
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
                        pEntry,
                        VMDIR_RIGHT_DS_DELETE_CHILD);
        }
        BAIL_ON_VMDIR_ERROR_WITH_MSG(
            retVal,
            pszLocalErrMsg,
            "VmDirSrvAccessCheck failed - (%u)(%s)",
            retVal,
            VMDIR_ACCESS_DENIED_ERROR_MSG);

        // age off tombstone entry?
        if  (pEntry->pParentEntry &&
             _VmDirIsDeletedContainer(pEntry->pParentEntry->dn.lberbv_val))
        {
            bIsDeletedObj = TRUE;
            // Normalize index attribute, so mdb can cleanup index tables properly.
            retVal = VmDirEntryAttrValueNormalize(pEntry, TRUE);
            BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "Attr value normalization failed - (%u)", retVal );
        }

        if (!bIsDeletedObj)
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
                retVal = GenerateDeleteAttrsMods( pOperation, pEntry );
                BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "GenerateDeleteAttrsMods failed - (%u)", retVal);

                // Generate new meta-data for the attributes being updated
                if ((retVal = VmDirGenerateModsNewMetaData( pOperation, modReq->mods, pEntry->eId )) != 0)
                {

                    switch (retVal)
                    {
                        case VMDIR_ERROR_LOCK_DEADLOCK:
                            goto txnretry; // Possible retry.  BUGBUG, is modReq->mods in above call good for retry?

                        default:
                            BAIL_ON_VMDIR_ERROR( retVal );
                    }
                }
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
                    bIsDeletedObj ? NULL : modReq->mods,
                    pEntry);
        if (retVal != 0)
        {
            switch (retVal)
            {
                case VMDIR_ERROR_BACKEND_DEADLOCK:
                    goto txnretry; // Possible retry.

                default:
                    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "BEEntryDelete (%u)(%s)",
                                                  retVal, VDIR_SAFE_STRING(pOperation->pBEErrorMsg));
            }
        }

        // Use normalized DN value
        if (bIsDomainObject)
        {
            retVal = VmDirInternalRemoveOrgConfig(pOperation,
                                                  BERVAL_NORM_VAL(pEntry->dn));
            BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "Update domain list entry failed." );
        }

        retVal = pOperation->pBEIF->pfnBEDeleteAllAttrValueMetaData(pOperation->pBECtx, pOperation->pSchemaCtx, pEntry->eId);
        if (retVal != 0)
        {
            switch (retVal)
            {
                case VMDIR_ERROR_BACKEND_DEADLOCK:
                    goto txnretry; // Possible retry.
                default:
                    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "BEEntryDeleteL pfnBEDeleteAllAttrValueMetaData error: (%u)(%s)",
                                                  retVal, VDIR_SAFE_STRING(pOperation->pBEErrorMsg));
            }
        }

        retVal = pOperation->pBEIF->pfnBETxnCommit( pOperation->pBECtx);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "txn commit (%u)(%s)",
                                              retVal, VDIR_SAFE_STRING(pOperation->pBEErrorMsg));
        bHasTxn = FALSE;
    }
    // ************************************************************************************
    // transaction retry loop end.
    // ************************************************************************************

    gVmdirGlobals.dwLdapWrites++;

    VmDirAuditWriteOp(pOperation, VDIR_SAFE_STRING(pEntry->dn.lberbv_val));

    // Post delete entry
    // TODO, make it into a separate file deletePlugin.c
    // clean lockout cache record if exists
    VdirLockoutCacheRemoveRec(pEntry->dn.bvnorm_val);

cleanup:

    {
        int iPostCommitPluginRtn  = 0;

        // Execute post Delete commit plugin logic
        iPostCommitPluginRtn = VmDirExecutePostDeleteCommitPlugins(pOperation, pEntry, retVal);
        if ( iPostCommitPluginRtn != LDAP_SUCCESS
                &&
                iPostCommitPluginRtn != pOperation->ldapResult.errCode    // pass through
        )
        {
            VmDirLog( LDAP_DEBUG_ANY, "InternalDeleteEntry: VdirExecutePostDeleteCommitPlugins - code(%d)",
                    iPostCommitPluginRtn);
        }
    }

    if (pOperation->opType != VDIR_OPERATION_TYPE_REPL)
    {
        // In case of replication, modReq is owned by the Replication thread/logic
        DeleteMods ( modReq );
    }

    VmDirFreeEntryContent ( &entry );

    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);

    return retVal;

error:
    if (bHasTxn)
    {
        pOperation->pBEIF->pfnBETxnAbort( pOperation->pBECtx );
    }

    VMDIR_SET_LDAP_RESULT_ERROR( &(pOperation->ldapResult), retVal, pszLocalErrMsg);

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
GenerateDeleteAttrsMods(
    PVDIR_OPERATION pOperation,
    VDIR_ENTRY *    pEntry
    )
{
    int                 retVal = 0;
    VDIR_MODIFICATION * delMod = NULL;
    VDIR_ATTRIBUTE *    attr = NULL;
    PVDIR_ATTRIBUTE     objectGuidAttr = NULL;
    VDIR_BERVALUE       deletedObjDN = VDIR_BERVALUE_INIT;
    ModifyReq *         modReq = &(pOperation->request.modifyReq);

    for ( attr = pEntry->attrs; attr != NULL; attr = attr->next )
    {
        // Retain the following kind of attributes
        if (attr->pATDesc->usage != VDIR_LDAP_USER_APPLICATIONS_ATTRIBUTE ||
            VmDirStringCompareA( attr->type.lberbv.bv_val, ATTR_OBJECT_CLASS, FALSE ) == 0)
        {
            continue;
        }
        retVal = VmDirAllocateMemory( sizeof( VDIR_MODIFICATION ), (PVOID *)&(delMod) );
        BAIL_ON_VMDIR_ERROR( retVal );

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

    // Add mod to set new DN.
    objectGuidAttr = VmDirEntryFindAttribute(ATTR_OBJECT_GUID, pEntry);
    assert( objectGuidAttr );

    retVal = constructDeletedObjDN( &pOperation->request.deleteReq.dn, objectGuidAttr->vals[0].lberbv.bv_val, &deletedObjDN );
    BAIL_ON_VMDIR_ERROR( retVal );

    retVal = VmDirAppendAMod( pOperation, MOD_OP_REPLACE, ATTR_DN, ATTR_DN_LEN,
                               deletedObjDN.lberbv.bv_val, deletedObjDN.lberbv.bv_len );
    BAIL_ON_VMDIR_ERROR( retVal );

cleanup:
    VmDirFreeMemory( deletedObjDN.lberbv.bv_val );

    return retVal;

error:
    goto cleanup;
}
static
BOOLEAN
_VmDirIsDeletedContainer(
    PCSTR   pszDN
    )
{
    BOOLEAN bRtn = FALSE;

    if (pszDN &&
        VmDirStringCompareA(pszDN, gVmdirServerGlobals.delObjsContainerDN.lberbv_val, FALSE) == 0)
    {
        bRtn = TRUE;
    }

    return bRtn;
}
