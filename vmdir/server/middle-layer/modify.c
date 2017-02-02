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

static
int
AddAttrToEntryStruct(
   VDIR_ENTRY *     e,
   VDIR_ATTRIBUTE * attr);

static
DWORD
AddAttrValsToEntryStruct(
   VDIR_ENTRY *     e,
   VDIR_ATTRIBUTE * eAttr,
   VDIR_ATTRIBUTE * modAttr,
   PSTR*            ppszErrMsg);

static
int
CheckIfAnAttrValAlreadyExists(
    PVDIR_SCHEMA_CTX  pSchemaCtx,
    VDIR_ATTRIBUTE *  eAttr,
    VDIR_ATTRIBUTE *  modAttr,
    PSTR*             ppszErrorMsg,
    BOOLEAN           bIsReplOp
    );

static
int
GenerateNewParent(
    PVDIR_ENTRY pEntry,
    PVDIR_ATTRIBUTE pDnAttr
    );

static
int
DelAttrValsFromEntryStruct(
   PVDIR_SCHEMA_CTX  pSchemaCtx,
   VDIR_ENTRY *      e,
   VDIR_ATTRIBUTE *  modAttr,
   PSTR*             ppszErrorMsg,
   BOOLEAN           bIsReplOp
   );

static
void
RemoveAttrVals(
   VDIR_ATTRIBUTE * eAttr,
   VDIR_ATTRIBUTE * modAttr);

static int
VmDirGenerateRenameAttrsMods(
    PVDIR_OPERATION pOperation
    );

static
int
_VmDirExternalModsSanityCheck(
    PVDIR_OPERATION     pOp,
    PVDIR_MODIFICATION  pMods
    );

static
int
_VmDirPatchBadMemberData(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    PVDIR_ENTRY         pEntry
    );

static
int
_VmDirAttrValueMetaDataToAdd(
    PVDIR_MODIFICATION  pMod,
    int                 currentVersion,
    PSTR                pTimeStamp,
    char *              usnChanged
    );

int
VmDirModifyEntryCoreLogic(
    VDIR_OPERATION *    pOperation, /* IN */
    ModifyReq *         modReq, /* IN */
    ENTRYID             entryId, /* IN */
    VDIR_ENTRY *        pEntry  /* OUT */
    )
{
    int       retVal = LDAP_SUCCESS;
    PSTR      pszLocalErrMsg = NULL;
    BOOLEAN   bDnModified = FALSE;
    BOOLEAN   bLeafNode = FALSE;
    PVDIR_ATTRIBUTE pAttrMemberOf = NULL;

    retVal = pOperation->pBEIF->pfnBEIdToEntry( pOperation->pBECtx,
                                                pOperation->pSchemaCtx,
                                                entryId,
                                                pEntry,
                                                VDIR_BACKEND_ENTRY_LOCK_WRITE );
    BAIL_ON_VMDIR_ERROR( retVal );

    if (modReq->dn.lberbv.bv_val == NULL) // If not already set by the caller
    {   // e.g. delete membership case via index lookup to get EID.
        retVal = VmDirBervalContentDup(&pEntry->dn, &modReq->dn);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg,
                                      "VmDirBervalContentDup failed - (%d)", retVal);
    }

    retVal = VmDirSrvAccessCheck( pOperation, &pOperation->conn->AccessInfo, pEntry, VMDIR_RIGHT_DS_WRITE_PROP);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg,
                                  "VmDirSrvAccessCheck failed - (%u)", retVal);

    // Apply modify operations to the current entry (in pack format)
    retVal = VmDirApplyModsToEntryStruct( pOperation->pSchemaCtx, modReq, pEntry, &bDnModified, &pszLocalErrMsg,
                                          pOperation->opType == VDIR_OPERATION_TYPE_REPL);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg,
                                  "ApplyModsToEntryStruct failed - (%d)(%s)", retVal, pszLocalErrMsg);

    if (bDnModified)
    {
        retVal = pOperation->pBEIF->pfnBEChkIsLeafEntry(
                                        pOperation->pBECtx,
                                        entryId,
                                        &bLeafNode);
        BAIL_ON_VMDIR_ERROR(retVal);

        if (bLeafNode == FALSE)
        {
            retVal = LDAP_NOT_ALLOWED_ON_NONLEAF;
            BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "Rename of a non-leaf node is not allowed." );
        }

        // Verify not a member of any groups
        retVal = VmDirFindMemberOfAttribute(pEntry, &pAttrMemberOf);
        if (pAttrMemberOf && pAttrMemberOf->numVals > 0)
        {
            retVal = LDAP_UNWILLING_TO_PERFORM;
            BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "Rename of a node with memberships is not allowed." );
        }
    }

    if (pOperation->opType != VDIR_OPERATION_TYPE_REPL)
    {
        // Schema check
        retVal = VmDirSchemaCheck(pEntry);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "Schema check failed - (%u)(%s)",
                                      retVal, VDIR_SAFE_STRING(VmDirSchemaCtxGetErrorMsg(pEntry->pSchemaCtx)));

        // check and read lock dn referenced entries
        retVal = pOperation->pBEIF->pfnBEChkDNReference( pOperation->pBECtx, pEntry );
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "BECheckDnRef, (%u)(%s)", retVal,
                                      VDIR_SAFE_STRING(pOperation->pBECtx->pszBEErrorMsg) );
    }

    // Execute plugin logic that require final entry image.  (Do this for both normal and repl routes)
    retVal = VmDirExecutePreModifyPlugins(pOperation, pEntry, retVal);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "PreModifyPlugins failed - (%u)", retVal);

    // Update DB
    retVal = pOperation->pBEIF->pfnBEEntryModify( pOperation->pBECtx, modReq->mods, pEntry );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "BEEntryModify, (%u)(%s)", retVal,
                                  VDIR_SAFE_STRING(pOperation->pBEErrorMsg) );

cleanup:

    VmDirFreeAttribute(pAttrMemberOf);
    VMDIR_SAFE_FREE_MEMORY( pszLocalErrMsg );

    return retVal;

error:

    VmDirLog( LDAP_DEBUG_ANY, "CoreLogicModifyEntry failed, DN = %s, (%u)(%s)",
                               VDIR_SAFE_STRING( modReq->dn.lberbv.bv_val ),
                               retVal, VDIR_SAFE_STRING(pszLocalErrMsg) );

    if ( pOperation->ldapResult.pszErrMsg == NULL )
    {
        pOperation->ldapResult.pszErrMsg = pszLocalErrMsg;
        pszLocalErrMsg = NULL;
    }

    goto cleanup;
}

/*
 * MLModify: Middle-layer modify functionality that sits between LDAP protocol head and the underlying DB.
 *
 */

int
VmDirMLModify(
    PVDIR_OPERATION pOperation
    )
{
    DWORD   dwError = 0;
    PSTR    pszLocalErrMsg = NULL;

    pOperation->pBECtx->pBE = VmDirBackendSelect(pOperation->request.modifyReq.dn.lberbv.bv_val);
    assert(pOperation->pBECtx->pBE);

    // AnonymousBind Or in case of a failed bind, do not grant modify access
    if (pOperation->conn->bIsAnonymousBind || VmDirIsFailedAccessInfo(&pOperation->conn->AccessInfo))
    {
        dwError = LDAP_INSUFFICIENT_ACCESS;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg, "Not bind/authenticate yet" );
    }

    // Mod request sanity check
    dwError = _VmDirExternalModsSanityCheck( pOperation, pOperation->request.modifyReq.mods );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInternalModifyEntry( pOperation);
    BAIL_ON_VMDIR_ERROR( dwError );

    if (pOperation->opType == VDIR_OPERATION_TYPE_EXTERNAL)
    {
        pOperation->pBEIF->pfnBESetMaxOriginatingUSN(pOperation->pBECtx,
                                                     pOperation->pBECtx->wTxnUSN);
    }

    VmDirPerformUrgentReplIfRequired(pOperation, pOperation->pBECtx->wTxnUSN);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);
    return pOperation->ldapResult.errCode;

error:
    VMDIR_SET_LDAP_RESULT_ERROR( &(pOperation->ldapResult), dwError, pszLocalErrMsg);
    goto cleanup;
}

/* VmDirInternalModifyEntry: Interface that can be used "internally" by the server code, e.g. to modify schema, indices,
 * config etc. entries in the BDB store. One of the main differences between this function and MLModify is that
 * this function does not send back an LDAP result to the client.
 *
 * Return: VmDir level error code.  Also, pOperation->ldapResult content is set.
 */
int
VmDirInternalModifyEntry(
    PVDIR_OPERATION pOperation
    )
{
    int          retVal = LDAP_SUCCESS;
    int          deadLockRetries = 0;
    VDIR_ENTRY   entry = {0};
    PVDIR_ENTRY  pEntry = NULL;
    ModifyReq*   modReq = NULL;
    ENTRYID      entryId = 0;
    BOOLEAN      bHasTxn = FALSE;
    PSTR         pszLocalErrMsg = NULL;

    assert(pOperation && pOperation->pBEIF);

    if (VmDirdState() == VMDIRD_STATE_READ_ONLY)
    {
        retVal = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "Server in read-only mode");
    }

    modReq = &(pOperation->request.modifyReq);

    // Normalize DN
    retVal = VmDirNormalizeDN( &(modReq->dn), pOperation->pSchemaCtx);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "DN normalization failed - (%u)(%s)",
                                  retVal, VDIR_SAFE_STRING(VmDirSchemaCtxGetErrorMsg(pOperation->pSchemaCtx)) );

    // Acquire schema modification mutex
    retVal = VmDirSchemaModMutexAcquire(pOperation);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "Failed to lock schema mod mutex", retVal );


    if (pOperation->opType != VDIR_OPERATION_TYPE_REPL)
    {
        // Generate mods based on MODN request
        retVal = VmDirGenerateRenameAttrsMods( pOperation );
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "GenerateDeleteAttrsMods failed - (%u)", retVal);
    }

    // Execute pre modify plugin logic
    retVal = VmDirExecutePreModApplyModifyPlugins(pOperation, NULL, retVal);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "PreModApplyModify plugin failed - (%u)",  retVal );

    // Normalize attribute values in mods
    retVal = VmDirNormalizeMods( pOperation->pSchemaCtx, modReq->mods, &pszLocalErrMsg );
    BAIL_ON_VMDIR_ERROR( retVal );

    // make sure VDIR_BACKEND_CTX has usn change number by now
    if ( pOperation->pBECtx->wTxnUSN <= 0 )
    {
        retVal = VMDIR_ERROR_NO_USN;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "BECtx.wTxnUSN not set");
    }

    // ************************************************************************************
    // transaction retry loop begin.  make sure all function within are retry agnostic.
    // ************************************************************************************
txnretry:
    if (bHasTxn)
    {
        pOperation->pBEIF->pfnBETxnAbort( pOperation->pBECtx);
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
        retVal = pOperation->pBEIF->pfnBEDNToEntryId( pOperation->pBECtx, &(modReq->dn), &entryId);
        if (retVal != 0)
        {
            switch (retVal)
            {
                case VMDIR_ERROR_BACKEND_DEADLOCK:
                    goto txnretry; // Possible retry.

                default:
                    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "BEEntryModify (%u)(%s)",
                                                  retVal, VDIR_SAFE_STRING(pOperation->pBEErrorMsg));
            }
        }

        if (pOperation->opType != VDIR_OPERATION_TYPE_REPL)
        {
            // Generate attributes' new meta-data
            if ((retVal = VmDirGenerateModsNewMetaData( pOperation, modReq->mods, entryId )) != 0)
            {
                switch (retVal)
                {
                    case VMDIR_ERROR_LOCK_DEADLOCK:
                        goto txnretry; // Possible retry.

                    default:
                        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg,
                                                      "GenerateModsNewMetaData (%u)", retVal );
                }
            }
        }

        pEntry = &entry;

        if ((retVal = VmDirModifyEntryCoreLogic( pOperation, &pOperation->request.modifyReq, entryId, pEntry )) != 0)
        {
            switch (retVal)
            {
                case VMDIR_ERROR_LOCK_DEADLOCK:
                    goto txnretry; // Possible retry.

                default:
                    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg,
                                                  "CoreLogicModifyEntry failed. (%u)", retVal );
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

cleanup:

    {
        int iPostCommitPluginRtn = 0;

        // Execute post modify plugin logic
        iPostCommitPluginRtn = VmDirExecutePostModifyCommitPlugins(pOperation, &entry, retVal);
        if ( iPostCommitPluginRtn != LDAP_SUCCESS
             &&
             iPostCommitPluginRtn != pOperation->ldapResult.errCode    // pass through
           )
        {
            VmDirLog( LDAP_DEBUG_ANY, "InternalModifyEntry: VdirExecutePostModifyCommitPlugins - code(%d)",
                      iPostCommitPluginRtn);
        }
    }

    // Release schema modification mutex
    (VOID)VmDirSchemaModMutexRelease(pOperation);

    VmDirFreeEntryContent ( &entry );
    VMDIR_SAFE_FREE_MEMORY( pszLocalErrMsg );

    return retVal;

error:

    if (bHasTxn)
    {
        pOperation->pBEIF->pfnBETxnAbort( pOperation->pBECtx );
    }

    VMDIR_SET_LDAP_RESULT_ERROR( &(pOperation->ldapResult), retVal, pszLocalErrMsg);

    goto cleanup;
}

/*
 * Convenient function to replace ONE single value attribute via InternalModifyEntry
 * *****************************************************************************
 * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
 * You should NOT call this function while in a backend txn/ctx.
 * *****************************************************************************
 * This may not be easy to determine as we could call this in different places, which
 * may be nested in external and internal OPERATION.
 * A better approach is to pass in pOperation and use the same beCtx if exists.
 * However, this could also cause logic error, e.g. you could lost track if entry/data
 * has already been changed by beCtx and reread them.
 * *****************************************************************************
 */
DWORD
VmDirInternalEntryAttributeReplace(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    PCSTR               pszNormDN,
    PCSTR               pszAttrName,
    PVDIR_BERVALUE      pBervAttrValue
    )
{
    DWORD               dwError = 0;
    VDIR_OPERATION      ldapOp = {0};
    PVDIR_MODIFICATION  pMod = NULL;

    if ( !pszNormDN || !pszAttrName || !pBervAttrValue)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirInitStackOperation( &ldapOp,
                                       VDIR_OPERATION_TYPE_INTERNAL,
                                       LDAP_REQ_MODIFY,
                                       pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

    ldapOp.pBEIF = VmDirBackendSelect(NULL);
    assert(ldapOp.pBEIF);

    ldapOp.reqDn.lberbv.bv_val = (PSTR)pszNormDN;
    ldapOp.reqDn.lberbv.bv_len = VmDirStringLenA(pszNormDN);

    dwError = VmDirAllocateMemory(
                    sizeof(*pMod)*1,
                    (PVOID)&pMod);
    BAIL_ON_VMDIR_ERROR(dwError);

    pMod->next = NULL;
    pMod->operation = MOD_OP_REPLACE;
    dwError = VmDirModAddSingleValueAttribute(
                    pMod,
                    ldapOp.pSchemaCtx,
                    pszAttrName,
                    pBervAttrValue->lberbv.bv_val,
                    pBervAttrValue->lberbv.bv_len);
    BAIL_ON_VMDIR_ERROR(dwError);

    ldapOp.request.modifyReq.dn.lberbv.bv_val = (PSTR)pszNormDN;
    ldapOp.request.modifyReq.dn.lberbv.bv_len = VmDirStringLenA(pszNormDN);
    ldapOp.request.modifyReq.mods = pMod;
    pMod = NULL;
    ldapOp.request.modifyReq.numMods = 1;

    dwError = VmDirInternalModifyEntry(&ldapOp);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    VmDirFreeOperationContent(&ldapOp);

    if (pMod)
    {
        VmDirModificationFree(pMod);
    }

    return dwError;

error:
    goto cleanup;
}

////////////////////////////////////////////////////////////////////////////////
// BUGBUG BUGBUG BUGBUG BUGBUG BUGBUG BUGBUG BUGBUG BUGBUG BUGBUG BUGBUG
// We have run into issue where member of a group does not exists in the system.
// We do not know how this happens. Most likely due to replication.
// We need to clean it up. Otherwise, no new member can be added to this group.
////////////////////////////////////////////////////////////////////////////////
static
int
_VmDirPatchBadMemberData(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    PVDIR_ENTRY         pEntry
    )
{
    int                 retVal = LDAP_SUCCESS;
    unsigned            iCnt = 0;
    unsigned            iMatch = 0;
    PVDIR_ATTRIBUTE     pAttrMembers = NULL;
    PVDIR_ATTRIBUTE     pLocalAttr = NULL;
    PVDIR_ENTRY         pLocalEntry = NULL;
    PSTR*               ppList = NULL;

    pAttrMembers = VmDirEntryFindAttribute(ATTR_MEMBER, pEntry );
    if ( pAttrMembers != NULL )
    {
        retVal = VmDirAllocateMemory( sizeof(PSTR) * pAttrMembers->numVals, (PVOID)&ppList);
        BAIL_ON_VMDIR_ERROR( retVal);

        for ( iCnt=0; iCnt<pAttrMembers->numVals; iCnt++ )
        {
            retVal = VmDirNormalizeDN( &(pAttrMembers->vals[iCnt]), pEntry->pSchemaCtx);
            BAIL_ON_VMDIR_ERROR( retVal );

            VmDirFreeEntry( pLocalEntry );
            pLocalEntry = NULL;

            retVal = VmDirSimpleDNToEntry( pAttrMembers->vals[iCnt].bvnorm_val, &pLocalEntry );
            if (retVal == 0 )
            {
                ppList[iMatch++] = pAttrMembers->vals[iCnt].lberbv_val;
            }
            else if ( retVal == VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND )
            {
                VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL, "Bad member [%s] in group [%s]",
                                   pAttrMembers->vals[iCnt].lberbv_val,
                                   pEntry->dn.lberbv_val);
            }
            else
            {
                BAIL_ON_VMDIR_ERROR( retVal );
            }
        }
    }

    if ( ppList && iMatch < pAttrMembers->numVals )
    {
        retVal = VmDirAttributeAllocate( ATTR_MEMBER,
                                         iMatch,
                                         pEntry->pSchemaCtx,
                                         &pLocalAttr);
        BAIL_ON_VMDIR_ERROR( retVal );

        for (iCnt = 0; iCnt < iMatch; iCnt++)
        {
            retVal = VmDirAllocateStringA( ppList[iCnt],
                                           &(pLocalAttr->vals[iCnt].lberbv_val));
            BAIL_ON_VMDIR_ERROR(retVal);

            pLocalAttr->vals[iCnt].lberbv_len = VmDirStringLenA(pLocalAttr->vals[iCnt].lberbv_val);
            pLocalAttr->vals[iCnt].bOwnBvVal = TRUE;
        }

        // replace pAttrMembers with pLocalAttr
        retVal = VmDirEntryReplaceAttribute( pEntry, pLocalAttr );
        BAIL_ON_VMDIR_ERROR( retVal );
        pLocalAttr = NULL;
    }

cleanup:
    VmDirFreeEntry( pLocalEntry );
    VmDirFreeAttribute( pLocalAttr );
    VMDIR_SAFE_FREE_MEMORY( ppList );

    return retVal;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s:%d failed, error (%d)", __FUNCTION__, __LINE__,retVal );

    goto cleanup;
}

/*
 * ApplyModsToEntryStruct: Applies the list of given modification operations (ADD/DELETE/REPLACE) to the Entry read
 * from DB.
 *
 * TODO - ideally, duplicate attribute value check should happen after we apply mod into entry,
 *      hence have the final images.  we should further investigate how to merge logic in
 *      1. NormalizeMods->AttributeDupValueCheck and
 *      2. CheckIfAnAttrValAlreadyExists
 *      to better enforce this constraint.
 */

int
VmDirApplyModsToEntryStruct(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    ModifyReq *         modReq,
    PVDIR_ENTRY         pEntry,
    PBOOLEAN            pbDnModified,
    PSTR*               ppszErrorMsg,
    BOOLEAN             bIsReplOp
    )
{
    int                 retVal = LDAP_SUCCESS;
    VDIR_MODIFICATION * currMod = NULL;
    VDIR_MODIFICATION * prevMod = NULL;
    PSTR                pszLocalErrorMsg = NULL;

    if ( pSchemaCtx == NULL || modReq == NULL || pEntry == NULL || ppszErrorMsg == NULL)
    {
        retVal = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    if (pEntry->allocType == ENTRY_STORAGE_FORMAT_PACK)
    {
        retVal = VmDirEntryUnpack( pEntry );
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                        "EntryUnpack failed failed (%s)",
                                        VDIR_SAFE_STRING(pEntry->dn.lberbv.bv_val));
    }

    for (currMod = modReq->mods; currMod != NULL; )
    {
        switch (currMod->operation)
        {
            case MOD_OP_ADD:
            {
                PVDIR_ATTRIBUTE attr = VmDirFindAttrByName( pEntry, currMod->attr.type.lberbv.bv_val);

                if (attr == NULL) // New attribute case
                {
                    retVal = AddAttrToEntryStruct( pEntry, &(currMod->attr) );
                    BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                                    "AddAttrToEntryStruct failed (%s)",
                                                    VDIR_SAFE_STRING(currMod->attr.type.lberbv.bv_val));
                }
                else // Add values to an existing attribute case.
                {
                    retVal = CheckIfAnAttrValAlreadyExists(pSchemaCtx, attr, &(currMod->attr), &pszLocalErrorMsg, bIsReplOp);
                    BAIL_ON_VMDIR_ERROR( retVal );

                    retVal = AddAttrValsToEntryStruct( pEntry, attr, &(currMod->attr), &pszLocalErrorMsg );
                    BAIL_ON_VMDIR_ERROR(retVal);
                }
                prevMod = currMod;
                currMod = currMod->next;
                break;
            }
            case MOD_OP_DELETE:
            {
                PVDIR_ATTRIBUTE attr = VmDirFindAttrByName( pEntry, currMod->attr.type.lberbv.bv_val);
                if (attr == NULL) // Attribute to be deleted does not exist in the entry
                {
                    if ( currMod->attr.numVals == 0 ) // If whole attribute is to be deleted, ignore this mod.
                    {
                        currMod->ignore = TRUE;
                    }
                    else // If some specific attribute values are to be deleted from the attribute => error case
                    {
                        retVal = VMDIR_ERROR_NO_SUCH_ATTRIBUTE;
                        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                                        "Attribute (%s) being deleted does not exists.",
                                                        VDIR_SAFE_STRING(currMod->attr.type.lberbv.bv_val));
                    }
                }
                else
                {
                    retVal = DelAttrValsFromEntryStruct(pSchemaCtx, pEntry, &(currMod->attr), &pszLocalErrorMsg, bIsReplOp);
                    if ((retVal == VMDIR_ERROR_NO_SUCH_ATTRIBUTE) &&
                        (VmDirStringCompareA(currMod->attr.type.lberbv.bv_val, ATTR_USER_PASSWORD, FALSE) == 0))
                    {   // for user password change, the old password supplied != existing record
                        retVal = VMDIR_ERROR_USER_INVALID_CREDENTIAL;
                    }
                    BAIL_ON_VMDIR_ERROR( retVal );
                }
                prevMod = currMod;
                currMod = currMod->next;
                break;
            }
            case MOD_OP_REPLACE:
            {
                // RFC 4511: Section 4.6:
                //  replace: replace all existing values of the modification attribute with the new values listed,
                //  creating the attribute if it did not already exist. A replace with no value will delete the entire
                // attribute if it exists, and it is ignored if the attribute does not exist.

                if (currMod->attr.numVals == 0)
                {
                    currMod->operation = MOD_OP_DELETE;
                    continue; // re-process the current altered modification
                }
                else
                {
                    VDIR_MODIFICATION * newDelMod = NULL;
                    PVDIR_ATTRIBUTE attr = VmDirFindAttrByName( pEntry, currMod->attr.type.lberbv.bv_val);

                    if (attr == NULL) // New attribute case
                    {
                        currMod->operation = MOD_OP_ADD;
                        continue; // re-process the current altered modification
                    }
                    else // Real replace attribute values case
                    {
                        // If DN is being modified, may need to re-parent
                        if (VmDirStringCompareA(attr->type.lberbv.bv_val, ATTR_DN, FALSE) == 0)
                        {
                            if (pbDnModified)
                            {
                                *pbDnModified = TRUE;
                            }

                            retVal = GenerateNewParent(pEntry, &(currMod->attr));
                            BAIL_ON_VMDIR_ERROR(retVal);
                        }

                        // Setup Delete attribute and Add attribute mods. Change currMod->operation to ADD, and
                        // insert a DELETE mod for this attribute before this mod.

                        currMod->operation = MOD_OP_ADD;

                        retVal = VmDirAllocateMemory( sizeof(VDIR_MODIFICATION), (PVOID *)&newDelMod );
                        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg), "no memory");

                        newDelMod->operation = MOD_OP_DELETE;
                        newDelMod->attr.pATDesc = currMod->attr.pATDesc;
                        newDelMod->attr.type.lberbv.bv_val = currMod->attr.type.lberbv.bv_val;
                        newDelMod->attr.type.lberbv.bv_len = currMod->attr.type.lberbv.bv_len;
                        newDelMod->next = currMod;

                        currMod = newDelMod;

                        if (prevMod == NULL)
                        {
                            modReq->mods = currMod;
                        }
                        else
                        {
                            prevMod->next = currMod;
                        }
                        continue;
                    }
                }
                break;
            }
            default:
                assert( FALSE );
        }
    }

    retVal = _VmDirPatchBadMemberData( pSchemaCtx, pEntry );
    BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                    "Patch Bad member data failed (%s)",
                                    VDIR_SAFE_STRING(pEntry->dn.lberbv.bv_val));

cleanup:

    if (ppszErrorMsg)
    {
        *ppszErrorMsg = pszLocalErrorMsg;
    }
    else
    {
        VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    }

    return retVal;

error:

    goto cleanup;
}

/*
 * AddAttrToEntryStruct: Adds an attribute to an Entry struct. After the following change, an Entry is kind of
 * mix of SEARCH_REPLY and ADD_REQUEST format. I.e. e->attr points to an array of Attributes, and the last attribute
 * in this array now points to a linked list of new attributes.
 */

static
int
AddAttrToEntryStruct(
   VDIR_ENTRY *     e,
   VDIR_ATTRIBUTE * attr
   )
{
    VDIR_ATTRIBUTE * currAttr = NULL;
    VDIR_ATTRIBUTE * attrCopy = NULL;
    int         retVal = 0;

    assert( e->allocType == ENTRY_STORAGE_FORMAT_NORMAL );
    // make a copy of the attribute.
    retVal = VmDirAttributeDup( attr, &attrCopy );
    BAIL_ON_VMDIR_ERROR( retVal );

    // Go to the last attribute.
    for (currAttr = e->attrs; currAttr->next != NULL; currAttr = currAttr->next)
    {
        ;
    }
    currAttr->next = attrCopy;
    attrCopy->next = NULL;

cleanup:

    return retVal;

error:
    goto cleanup;
}

/* GenerateModsNewMetaData:
 *
 * Returns:
 *      LDAP_SUCCESS: On Success
 *      LDAP_OPERATIONS_ERROR: In case of an error
 *
 */

int
VmDirGenerateModsNewMetaData(
    PVDIR_OPERATION          pOperation,
    PVDIR_MODIFICATION       pmods,
    USN                      entryId
    )
{
    int                  retVal = LDAP_SUCCESS;
    int                  dbRetVal = 0;
    PVDIR_MODIFICATION   pMod = NULL;
    PVDIR_MODIFICATION   pUsnChangedMod = NULL;
    char                 origTimeStamp[VMDIR_ORIG_TIME_STR_LEN];
    int                  currentVersion = 0;
    PSTR                 pszLocalErrMsg = NULL;

    // Look for Replace USN_MODIFIED mod
    for (pMod = pmods; pMod; pMod = pMod->next)
    {
        if (VmDirStringCompareA(ATTR_USN_CHANGED, pMod->attr.type.lberbv.bv_val, FALSE) == 0)
        {
            pUsnChangedMod = pMod;
            break;
        }
    }
    assert(pUsnChangedMod);

    retVal = VmDirGenOriginatingTimeStr( origTimeStamp );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg,
                                  "GenerateModsNewMetaData: VmDirGenOriginatingTimeStr failed.");

    if (gVmdirServerGlobals.invocationId.lberbv.bv_val == NULL)
    {
        retVal = VMDIR_ERROR_GENERIC;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg,
                "GenerateModsNewMetaData: gVmdirServerGlobals.invocationId.lberbv.bv_val not set.");
    }

    for (pMod = pmods; pMod; pMod = pMod->next)
    {
        if ((dbRetVal = pOperation->pBEIF->pfnBEGetAttrMetaData( pOperation->pBECtx, &(pMod->attr), entryId )) != 0)
        {
            switch (dbRetVal)
            {
                case VMDIR_ERROR_BACKEND_DEADLOCK:
                     retVal = dbRetVal;
                     BAIL_ON_VMDIR_ERROR( retVal );

                case VMDIR_ERROR_BACKEND_ATTR_META_DATA_NOTFOUND:
                    currentVersion = 0;
                    break;

                default:
                    retVal = dbRetVal;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg,
                                        "pfnBEGetAttrMetaData failed - (%d)(%s)", retVal,
                                        VDIR_SAFE_STRING(pOperation->pBEErrorMsg));
            }
        }
        else
        {
            currentVersion = VmDirStringToIA(strchr(pMod->attr.metaData, ':') + 1);
        }

        if(currentVersion > 0 && !pMod->attr.pATDesc->bSingleValue)
        {
            if (VDIR_CONCURRENT_ATTR_VALUE_UPDATE_ENABLED && //When concurrent attribute update feature enabled.
                (pMod->operation == MOD_OP_ADD || (pMod->operation == MOD_OP_DELETE && pMod->attr.numVals > 0)))
            {
                //Create attr-value-meta-data instread of attr-meta-data PR 1531924
                retVal = _VmDirAttrValueMetaDataToAdd(pMod, currentVersion, origTimeStamp, pUsnChangedMod->attr.vals[0].lberbv.bv_val);
                BAIL_ON_VMDIR_ERROR( retVal );
                continue;
            }
            else
            {
                // MOD_OP_REPALCE or MOD_OP_DELETE with empty value on multi-value attr
                // Create the list of attr-value-meta-data to be deleted.
                // If VDIR_CONCURRENT_ATTR_VALUE_UPDATE_ENABLED is FALSE,
                // then pfnBEGetAttrValueMetaData() does nothing.
                retVal = pOperation->pBEIF->pfnBEGetAttrValueMetaData(pOperation->pBECtx, entryId,
                                                                      pMod->attr.pATDesc->usAttrID,
                                                                      &pMod->attr.valueMetaDataToDelete);
                BAIL_ON_VMDIR_ERROR( retVal );
            }
        }

        // Get here if modify is NOT to add/delete a value on a multivalue value attribute
        // or there is NO any value yet for the attribute.

        // Force version gap if specified by pMod composer.
        // User case: force sync schema metadata version in 6.5 schema patch.
        currentVersion += pMod->usForceVersionGap;

        // SJ-TBD: Since, currently, Replace mod is replaced by Delete and Add mods, the logic to set new attribute
        // meta data in each of these 2 mods is bit strange, but works, because both Delete and Add mods read
        // current attribute meta data from the DB, and not Add mod seeing attribute meta data from the previous
        // Delete and therefore increasing the version # one extra time.

        // Format is: <local USN>:<version no>:<originating server ID>:<originating time>:<originating USN>
        VmDirStringNPrintFA( pMod->attr.metaData, sizeof( pMod->attr.metaData ), sizeof( pMod->attr.metaData ) - 1,
                             "%s:%d:%s:%s:%s", pUsnChangedMod->attr.vals[0].lberbv.bv_val, currentVersion + 1,
                             gVmdirServerGlobals.invocationId.lberbv.bv_val,
                             origTimeStamp, pUsnChangedMod->attr.vals[0].lberbv.bv_val );

    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);

    return retVal;

error:

    VmDirLog(LDAP_DEBUG_ANY, "VmDirGenerateModsNewMetaData failed: (%u)(%s)",
                             retVal, VDIR_SAFE_STRING(pszLocalErrMsg));

    goto cleanup;
}


/*
 * NormalizeMods:
 * 1. Normalize attribute values present in the modifications list.
 * 2. Make sure no duplicate value
 */

int
VmDirNormalizeMods(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    PVDIR_MODIFICATION  pMods,
    PSTR*               ppszErrorMsg
    )
{
    int            retVal = LDAP_SUCCESS;
    PVDIR_MODIFICATION pMod = NULL;
    unsigned int            i = 0;
    PSTR           pszDupAttributeName = NULL;
    PSTR           pszLocalErrorMsg = NULL;

    for (pMod = pMods; pMod != NULL; pMod = pMod->next)
    {
        if ( pMod->attr.pATDesc == NULL
             &&
             (pMod->attr.pATDesc = VmDirSchemaAttrNameToDesc( pSchemaCtx, pMod->attr.type.lberbv.bv_val)) == NULL
           )
        {
            retVal = VMDIR_ERROR_UNDEFINED_TYPE;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                            "Undefined attribute (%s)",
                                            VDIR_SAFE_STRING(pMod->attr.type.lberbv.bv_val));
        }
        for (i=0; i < pMod->attr.numVals; i++)
        {
            retVal = VmDirSchemaBervalNormalize( pSchemaCtx, pMod->attr.pATDesc, &pMod->attr.vals[i]);
            BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                        "attribute value normalization failed (%s)(%.*s)",
                        VDIR_SAFE_STRING(pMod->attr.pATDesc->pszName),
                        VMDIR_MIN(pMod->attr.vals[i].lberbv.bv_len, VMDIR_MAX_LOG_OUTPUT_LEN),
                        VDIR_SAFE_STRING(pMod->attr.vals[i].lberbv.bv_val));
        }

        // Make sure we have no duplicate value in mod->attr
        retVal = VmDirAttributeDupValueCheck(&pMod->attr, &pszDupAttributeName);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                        "attribute (%s) has duplicate value",
                                        VDIR_SAFE_STRING(pszDupAttributeName));
    }

cleanup:

    if (ppszErrorMsg)
    {
        *ppszErrorMsg = pszLocalErrorMsg;
    }
    else
    {
        VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    }

    return retVal;

error:
    goto cleanup;
}

/*
 * Mod request sanity check
 * 1. modify attribute is defined
 * 2. attribute can be modified ( we only enforce this for external operation )
 */
static
int
_VmDirExternalModsSanityCheck(
    PVDIR_OPERATION     pOp,
    PVDIR_MODIFICATION  pMods
    )
{
    int                 retVal          = LDAP_SUCCESS;
    PSTR                pszLocalErrMsg  = NULL;
    PVDIR_MODIFICATION  pLocalMod       = NULL;

    for (pLocalMod = pMods; pLocalMod != NULL; pLocalMod = pLocalMod->next)
    {
        if ( pLocalMod->attr.pATDesc == NULL
             &&
             (pLocalMod->attr.pATDesc = VmDirSchemaAttrNameToDesc( pOp->pSchemaCtx,
                                                                   pLocalMod->attr.type.lberbv.bv_val)) == NULL
           )
        {
            retVal = VMDIR_ERROR_UNDEFINED_TYPE;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrMsg),
                                            "Undefined attribute (%s)",
                                            VDIR_SAFE_STRING(pLocalMod->attr.type.lberbv.bv_val));
        }

        // Make sure attribute can be modified
        if ( pLocalMod->attr.pATDesc->bNoUserModifiable == TRUE
             &&
             pOp->conn->AccessInfo.bindEID != DEFAULT_ADMINISTRATOR_ENTRY_ID // exempt default administrator
           )
        {
            retVal = VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrMsg),
                                            "attribute (%s) can not be modified",
                                            VDIR_SAFE_STRING(pLocalMod->attr.pATDesc->pszName));
        }

        if ( pLocalMod->operation == MOD_OP_ADD ||
             pLocalMod->operation == MOD_OP_REPLACE )
        {
            // ADD or REPLACE principal name, validate its syntax.
            if ( VmDirStringCompareA(pLocalMod->attr.type.lberbv_val, ATTR_KRB_UPN, FALSE) == 0 ||
                 VmDirStringCompareA(pLocalMod->attr.type.lberbv_val, ATTR_KRB_SPN, FALSE) == 0 )
            {
                retVal = VmDirValidatePrincipalName( &(pLocalMod->attr), &pszLocalErrMsg);
                BAIL_ON_VMDIR_ERROR(retVal);
            }
            else if (VmDirStringCompareA(pLocalMod->attr.type.lberbv_val, ATTR_OBJECT_SECURITY_DESCRIPTOR, FALSE) == 0)
            {
                BOOLEAN bReturn = FALSE;

                bReturn = VmDirValidRelativeSecurityDescriptor(
                            (PSECURITY_DESCRIPTOR_RELATIVE)pLocalMod->attr.vals[0].lberbv_val,
                            (ULONG)pLocalMod->attr.vals[0].lberbv_len,
                            OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION);
                if (!bReturn)
                {
                    retVal = VMDIR_ERROR_BAD_ATTRIBUTE_DATA;
                    BAIL_ON_VMDIR_ERROR(retVal);
                }
            }
        }
        else if (pLocalMod->operation == MOD_OP_DELETE)
        {
            if (VmDirStringCompareA(pLocalMod->attr.type.lberbv_val, ATTR_OBJECT_SECURITY_DESCRIPTOR, FALSE) == 0)
            {
                retVal = VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION;
                BAIL_ON_VMDIR_ERROR(retVal);
            }
        }
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);

    return retVal;

error:

    VMDIR_SET_LDAP_RESULT_ERROR( &(pOp->ldapResult), retVal, pszLocalErrMsg);

    goto cleanup;
}

/*
 * AddAttrValsToEntryStruct: Adds new attribute values for an attribute to an Entry struct. Inserts the new attribute
 * values in the middle (near where the existing BerValues are there for the attribute) of e->bvs after reallocating
 * the extra required space.
 */

static
DWORD
AddAttrValsToEntryStruct(
   VDIR_ENTRY *     e,
   VDIR_ATTRIBUTE * eAttr,    // Entry attribute to be updated with new attribute values
   VDIR_ATTRIBUTE * modAttr,  // Modify attribute containing new attribute values
   PSTR*            ppszErrMsg
   )
{
    unsigned int     i = 0;
    unsigned int     j = 0;
    DWORD   dwError = ERROR_SUCCESS;
    PSTR    pszErrMsg = NULL;

    assert( e->allocType == ENTRY_STORAGE_FORMAT_NORMAL );

    if ( (size_t)eAttr->numVals + (size_t)modAttr->numVals > UINT16_MAX)
    {
        dwError = VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszErrMsg),
                                      "Too many %s attribute values, max %u allowed.",
                                      VDIR_SAFE_STRING(eAttr->type.lberbv_val), UINT16_MAX);
    }

    dwError = VmDirReallocateMemoryWithInit( eAttr->vals, (PVOID*)(&(eAttr->vals)),
                                             (eAttr->numVals + modAttr->numVals + 1) * sizeof( VDIR_BERVALUE ),
                                             (eAttr->numVals + 1) * sizeof( VDIR_BERVALUE ) );
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0, j = eAttr->numVals; i < modAttr->numVals; i++, j++)
    {
        dwError = VmDirBervalContentDup( &modAttr->vals[i], &eAttr->vals[j] );
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    memset( &(eAttr->vals[j]), 0, sizeof(VDIR_BERVALUE) ); // set last BerValue.lberbv.bv_val to NULL;
    eAttr->numVals += modAttr->numVals;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszErrMsg);
    return dwError;

error:
    if (ppszErrMsg)
    {
        *ppszErrMsg = pszErrMsg;
        pszErrMsg = NULL;
    }

    goto cleanup;
}

/*
 * CheckIfAnAttrValAlreadyExists: Checks if the attribute values present in the modAttr already exist in the entry
 * attribute (eAttr).
 */

static
int
CheckIfAnAttrValAlreadyExists(
    PVDIR_SCHEMA_CTX  pSchemaCtx,
    VDIR_ATTRIBUTE *  eAttr,
    VDIR_ATTRIBUTE *  modAttr,
    PSTR*             ppszErrorMsg,
    BOOLEAN           bIsReplOp
    )
{
    int retVal = LDAP_SUCCESS;
    int i = 0;
    int j = 0;
    int numVals = modAttr->numVals;
    PSTR    pszLocalErrorMsg = NULL;

    for (i=0; i < (int)eAttr->numVals; i++)
    {
        retVal = VmDirSchemaBervalNormalize( pSchemaCtx, modAttr->pATDesc, // Assumption: modAttr type is same as eAttr type
                                              &eAttr->vals[i]) ;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                "normalize (%s)(%.*s)",
                                VDIR_SAFE_STRING(modAttr->pATDesc->pszName),
                                VMDIR_MIN(eAttr->vals[i].lberbv.bv_len, VMDIR_MAX_LOG_OUTPUT_LEN),
                                VDIR_SAFE_STRING(eAttr->vals[i].lberbv.bv_val));

        for (j = 0; j < (int)modAttr->numVals; j++)
        {
            // modAttr values are already normalized.
            assert( modAttr->vals[j].bvnorm_val );

            if (eAttr->vals[i].bvnorm_len == modAttr->vals[j].bvnorm_len &&
                memcmp( eAttr->vals[i].bvnorm_val, modAttr->vals[j].bvnorm_val, modAttr->vals[j].bvnorm_len) == 0)
            {
                break;
            }
        }
        if (j != modAttr->numVals) // found a match in middle
        {
            if (bIsReplOp)
            {
                /*
                 * This is considered no error during replicaiton processing that implements concurrently
                 * adding/deleting values on multi-valued attribute (PR 1531924)
                 * E.g. at the supplier, delete a value in one LDAP modify, then add the same value
                 * in the 2nd LDAP modify, only the 2nd valueMetaData is kept, and used to created the mod,
                 * but both LDAP modifies happended to be contained in the same replModify.
                 */
                modAttr->vals[j].bvnorm_val[0] = '\0';
                modAttr->vals[j].bvnorm_len = 1;
                numVals--;
            }
            else
            {
                retVal = VMDIR_ERROR_TYPE_OR_VALUE_EXISTS;
                BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                    "Attribute (%s) value (%.*s) exists",
                                    VDIR_SAFE_STRING(modAttr->pATDesc->pszName),
                                    VMDIR_MIN(eAttr->vals[i].lberbv.bv_len, VMDIR_MAX_LOG_OUTPUT_LEN),
                                    VDIR_SAFE_STRING(eAttr->vals[i].lberbv.bv_val));
            }
        }
    }

    if (numVals < (int)modAttr->numVals)
    {
         VDIR_BERVALUE * vals = modAttr->vals;
         int modAttrNumVals = modAttr->numVals;

         retVal = VmDirAllocateMemory(sizeof(VDIR_BERVALUE) * (numVals+1), (PVOID*)&modAttr->vals);
         BAIL_ON_VMDIR_ERROR(retVal);

         for (i = 0, j=0; i < modAttrNumVals; i++)
         {
             if (vals[i].bvnorm_len == 1 && vals[i].bvnorm_val[0] == '\0')
             {
                 VmDirFreeBervalContent(&vals[i]);
             } else
             {
                 modAttr->vals[j++] = vals[i];
             }
         }
         modAttr->vals[j].lberbv.bv_val = NULL;
         modAttr->vals[j].lberbv.bv_len = 0;
         modAttr->numVals = j;
         VMDIR_SAFE_FREE_MEMORY(vals);
    }

cleanup:

    if (ppszErrorMsg)
    {
        *ppszErrorMsg = pszLocalErrorMsg;
    }
    else
    {
        VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    }

    return retVal;

error:
    goto cleanup;
}

/*
 * DelAttrValsFromEntryStruct: Deletes a complete attribute or specific attribute values from the entry.
 * Assumption: This function assumes/asserts that the modAttr does exist in the entry.
 *
 */
static
int
DelAttrValsFromEntryStruct(
   PVDIR_SCHEMA_CTX  pSchemaCtx,
   VDIR_ENTRY *      e,
   VDIR_ATTRIBUTE *  modAttr,
   PSTR*             ppszErrorMsg,
   BOOLEAN           bIsReplOp
   )
{
    int                 retVal = LDAP_SUCCESS;
    VDIR_ATTRIBUTE *    prevAttr = NULL;
    VDIR_ATTRIBUTE *    currAttr = NULL;
    VDIR_ATTRIBUTE *    eAttr = NULL;
    unsigned int        i = 0;
    unsigned int        j = 0;
    PSTR                pszLocalErrorMsg = NULL;

    assert( e->allocType == ENTRY_STORAGE_FORMAT_NORMAL );

    // Locate which attribute (values) we are trying to delete
    for (currAttr = e->attrs; currAttr != NULL; prevAttr = currAttr, currAttr = currAttr->next)
    {
        if (VmDirStringCompareA( modAttr->type.lberbv.bv_val, currAttr->type.lberbv.bv_val, FALSE) == 0)
        {
            break;
        }
    }

    assert( currAttr != NULL );

    eAttr = currAttr;

    // Normalize eAttr values
    for (i = 0; i < eAttr->numVals; i++)
    {
        retVal = VmDirSchemaBervalNormalize( pSchemaCtx, eAttr->pATDesc, &eAttr->vals[i] );
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                            "normalize (%s)(%.*s)",
                            VDIR_SAFE_STRING(eAttr->pATDesc->pszName),
                            VMDIR_MIN(eAttr->vals[i].lberbv.bv_len, VMDIR_MAX_LOG_OUTPUT_LEN),
                            VDIR_SAFE_STRING(eAttr->vals[i].lberbv.bv_val));
    }

    // Complete attribute is to be deleted.
    if (modAttr->numVals == 0)
    {
        // Make a copy of BerValues into modAttr so that these values can be used to delete from the index,
        // if it is an indexed attribute.

        VMDIR_SAFE_FREE_MEMORY( modAttr->vals );
        retVal = VmDirAllocateMemory( (eAttr->numVals + 1) * sizeof( VDIR_BERVALUE ), (PVOID *)&modAttr->vals);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg), "no memory");

        for (i = 0; i < eAttr->numVals; i++)
        {
            VmDirBervalContentDup( &eAttr->vals[i], &modAttr->vals[i] );
        }
        modAttr->numVals = eAttr->numVals;

        // Adjust the "next" pointer of the attribute before the attribute being deleted. => Altering the attribute
        // chain.
        if (prevAttr == NULL) // if it is the first attribute
        {
            e->attrs = eAttr->next;
        }
        else
        {
            prevAttr->next = eAttr->next;
        }

        VmDirFreeAttribute( eAttr );
    }
    else // Specific attribute values need to be deleted.
    {
        unsigned int matched_numVals = 0;
        // Check if all attribute values that are being deleted exist in the Attribute
        for (i=0; i < modAttr->numVals; i++)
        {
            // modAttr values are already normalized.
            assert( modAttr->vals[i].bvnorm_val );

            for (j = 0; j < eAttr->numVals; j++)
            {
                // eAttr values are already normalized.
                assert( eAttr->vals[j].bvnorm_val );

                if (modAttr->vals[i].bvnorm_len == eAttr->vals[j].bvnorm_len &&
                    memcmp( modAttr->vals[i].bvnorm_val, eAttr->vals[j].bvnorm_val, modAttr->vals[i].bvnorm_len) == 0)
                {
                    // found a match
                    if (i > matched_numVals)
                    {
                        // need to shift value position
                        modAttr->vals[matched_numVals] = modAttr->vals[i];
                        memset(&modAttr->vals[i], 0, sizeof(modAttr->vals[i]));
                    }
                    matched_numVals++;
                    break;
                }
            }
            if (j == eAttr->numVals) // did not find a match
            {
                if (bIsReplOp)
                {
                    // This is considered no error during replicaiton processing that implements concurrently
                    // adding/deleting values on multi-valued attribute (PR 1531924)
                    VmDirFreeBervalContent(&modAttr->vals[i]);
                    continue;
                }

                retVal = VMDIR_ERROR_NO_SUCH_ATTRIBUTE;
                BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                        "Attribute (%s) value (%.*s) being deleted does not exist.",
                                        VDIR_SAFE_STRING(modAttr->type.lberbv.bv_val),
                                        VMDIR_MIN(modAttr->vals[i].lberbv.bv_len, VMDIR_MAX_LOG_OUTPUT_LEN),
                                        VDIR_SAFE_STRING(modAttr->vals[i].lberbv.bv_val));
            }
        }

        // update modAttr numVals
        modAttr->numVals = matched_numVals;

        // All values are being deleted. => Delete the whole attribute
        if (matched_numVals == eAttr->numVals)
        {
            // Adjust the "next" pointer of the attribute before the attribute being deleted. => Altering the attribute
            // chain.
            if (prevAttr == NULL) // if it is the first attribute
            {
                e->attrs = eAttr->next;
            }
            else
            {
                prevAttr->next = eAttr->next;
            }
            VmDirFreeAttribute( eAttr );
        }
        else
        {
            RemoveAttrVals(eAttr, modAttr);
        }
    }

cleanup:

    if (ppszErrorMsg)
    {
        *ppszErrorMsg = pszLocalErrorMsg;
    }
    else
    {
        VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    }

    return retVal;

error:
    goto cleanup;
}

/*
 * RemoveAttrVals: Removes certain attribute values from an attribute, by moving them closer. Leave other attributes
 * and e->bvs intact.
 */

static
void
RemoveAttrVals(
   VDIR_ATTRIBUTE * eAttr,    // Entry attribute to be updated after removing certain attribute values
   VDIR_ATTRIBUTE * modAttr   // Modify attribute containing attribute values to be deleted
   )
{
    unsigned int        i = 0;
    unsigned int        j = 0;
    int                 k = 0;

    for (i=0; i < eAttr->numVals; i++)
    {
        // eAttr values are already normalized.
        assert( eAttr->vals[i].bvnorm_val );

        for (j = 0; j < modAttr->numVals; j++)
        {
            // modAttr values are already normalized.
            assert( modAttr->vals[j].bvnorm_val );

            if (eAttr->vals[i].bvnorm_len == modAttr->vals[j].bvnorm_len &&
                memcmp( eAttr->vals[i].bvnorm_val, modAttr->vals[j].bvnorm_val, modAttr->vals[j].bvnorm_len) == 0)
            {
                break;
            }
        }
        if (j == modAttr->numVals) // current value (i th) is NOT being deleted
        {
            eAttr->vals[k++] = eAttr->vals[i];
        }
        else // current value (i th) is being deleted
        {
            VmDirFreeBervalContent( &eAttr->vals[i] );
        }
    }
    eAttr->numVals = k;
    memset( &(eAttr->vals[k]), 0, sizeof(VDIR_BERVALUE) ); // set last BerValue.lberbv.bv_val to NULL;

    return;
}

/*
 * TODO: Should not allow renaming computers, domain controllers, replication
 * agreements, server objects
 */
static int
VmDirGenerateRenameAttrsMods(
    PVDIR_OPERATION pOperation
    )
{
    int                 retVal = 0;
    ModifyReq *         modReq = &(pOperation->request.modifyReq);
    VDIR_BERVALUE       parentdn = VDIR_BERVALUE_INIT;
    VDIR_BERVALUE       NewDn = VDIR_BERVALUE_INIT;
    VDIR_BERVALUE       OldRdn = VDIR_BERVALUE_INIT;
    VDIR_BERVALUE       NewRdn = VDIR_BERVALUE_INIT;
    PSTR pszOldRdnAttrName = NULL;
    PSTR pszOldRdnAttrVal = NULL;
    PSTR pszNewRdnAttrName = NULL;
    PSTR pszNewRdnAttrVal = NULL;

    if (modReq->newrdn.lberbv.bv_len == 0)
    {
        goto cleanup; // Nothing to do
    }

    if (strchr(modReq->newrdn.lberbv.bv_val, RDN_SEPARATOR_CHAR)) // FIXME : Need to handle escape
    {
        retVal = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        //BAIL_ON_VMDIR_ERROR_WITH_MSG(retVal, pszLocalErrMsg, "New RDN has more than one component");
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    if (modReq->newSuperior.lberbv.bv_len)
    {
        retVal = VmDirNormalizeDN( &(modReq->newSuperior), pOperation->pSchemaCtx);
        //BAIL_ON_VMDIR_ERROR_WITH_MSG(retVal, pszLocalErrMsg, "DN normalization failed - (%u)(%s)",
        //                              retVal, VDIR_SAFE_STRING(VmDirSchemaCtxGetErrorMsg(pOperation->pSchemaCtx)) );
        BAIL_ON_VMDIR_ERROR(retVal)

        retVal = VmDirCatDN(&modReq->newrdn, &modReq->newSuperior, &NewDn);
        BAIL_ON_VMDIR_ERROR(retVal);
    }
    else
    {
        retVal = VmDirGetParentDN(&modReq->dn, &parentdn);
        BAIL_ON_VMDIR_ERROR(retVal);

        retVal = VmDirCatDN(&modReq->newrdn, &parentdn, &NewDn);
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    retVal = VmDirNormalizeDN( &NewDn, pOperation->pSchemaCtx);
    //BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrMsg, "DN normalization failed - (%u)(%s)",
    //                              retVal, VDIR_SAFE_STRING(VmDirSchemaCtxGetErrorMsg(pOperation->pSchemaCtx)) );
    BAIL_ON_VMDIR_ERROR( retVal );

    retVal = VmDirGetRdn(&NewDn, &NewRdn);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirRdnToNameValue(&NewRdn, &pszNewRdnAttrName, &pszNewRdnAttrVal);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirGetRdn(&modReq->dn, &OldRdn);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirRdnToNameValue(&OldRdn, &pszOldRdnAttrName, &pszOldRdnAttrVal);
    BAIL_ON_VMDIR_ERROR(retVal);

    // Change DN
    retVal = VmDirAppendAMod(pOperation, MOD_OP_REPLACE, ATTR_DN, ATTR_DN_LEN, NewDn.bvnorm_val, NewDn.bvnorm_len);
    BAIL_ON_VMDIR_ERROR( retVal );


    if (strcmp(pszNewRdnAttrName, pszOldRdnAttrName) == 0)
    {
        if (strcmp(pszNewRdnAttrVal, pszOldRdnAttrVal) != 0)
        {
            // Change was like CN=User1,... to CN=User2,... then may want to
            // modify CN if bDeleteOldRdn
            if (modReq->bDeleteOldRdn)
            {
                retVal = VmDirAppendAMod(pOperation, MOD_OP_DELETE, pszOldRdnAttrName, (int) VmDirStringLenA(pszOldRdnAttrName), pszOldRdnAttrVal, VmDirStringLenA(pszOldRdnAttrVal));
                BAIL_ON_VMDIR_ERROR( retVal );
            }

            retVal = VmDirAppendAMod(pOperation, MOD_OP_ADD, pszNewRdnAttrName, (int)VmDirStringLenA(pszNewRdnAttrName), pszNewRdnAttrVal, VmDirStringLenA(pszNewRdnAttrVal));
            BAIL_ON_VMDIR_ERROR( retVal );
        }
    }
    else
    {
        // If change was like CN=User1,... to OU=MyOU,... then
        // need to add attribute OU=MyOu and potentially delete attribute CN=User1
        retVal = VmDirAppendAMod(pOperation, MOD_OP_ADD, pszNewRdnAttrName, (int)VmDirStringLenA(pszNewRdnAttrName), pszNewRdnAttrVal, VmDirStringLenA(pszNewRdnAttrVal));
        BAIL_ON_VMDIR_ERROR( retVal );

        if (modReq->bDeleteOldRdn)
        {
            retVal = VmDirAppendAMod(pOperation, MOD_OP_DELETE, pszOldRdnAttrName,(int) VmDirStringLenA(pszOldRdnAttrName), NULL, 0);
            BAIL_ON_VMDIR_ERROR( retVal );
        }
    }

cleanup:

    VmDirFreeBervalContent(&parentdn);
    VmDirFreeBervalContent(&NewDn);
    VmDirFreeBervalContent(&OldRdn);
    VmDirFreeBervalContent(&NewRdn);
    VMDIR_SAFE_FREE_STRINGA(pszOldRdnAttrName);
    VMDIR_SAFE_FREE_STRINGA(pszOldRdnAttrVal);
    VMDIR_SAFE_FREE_STRINGA(pszNewRdnAttrName);
    VMDIR_SAFE_FREE_STRINGA(pszNewRdnAttrVal);
    return retVal;

error:
    goto cleanup;
}

static
int
GenerateNewParent(
    PVDIR_ENTRY pEntry,
    PVDIR_ATTRIBUTE pDnAttr
    )
{
    int retVal = 0;
    VDIR_BERVALUE  NewParent = VDIR_BERVALUE_INIT;

    if (!pEntry->pdn.bvnorm_val)
    {
        VmDirFreeBervalContent(&pEntry->pdn);

        retVal = VmDirGetParentDN(&pEntry->dn, &pEntry->pdn);
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    retVal = VmDirGetParentDN(&pDnAttr->vals[0], &NewParent);
    BAIL_ON_VMDIR_ERROR(retVal);

    if (pEntry->pdn.bvnorm_val == NULL ||
        NewParent.bvnorm_val == NULL ||
        VmDirStringCompareA(pEntry->pdn.bvnorm_val, NewParent.bvnorm_val, FALSE) != 0)
    {
        retVal = VmDirBervalContentDup(&NewParent, &pEntry->newpdn);
        BAIL_ON_VMDIR_ERROR(retVal);
    }

cleanup:
    VmDirFreeBervalContent(&NewParent);
    return retVal;
error:
    goto cleanup;
}

static
int
_VmDirAttrValueMetaDataToAdd(
    PVDIR_MODIFICATION   pMod,
    int                  currentVersion,
    PSTR                 pTimeStamp,
    char *               usnChanged
    )
{
    int retVal = 0;
    VDIR_BERVALUE *pAVmeta = NULL;
    char *p1 = NULL;
    char *p2 = NULL;
    int i=0, value_len = 0, av_meta_len = 0;

    for (i=0; i<(int)pMod->attr.numVals; i++)
    {
        char av_meta_pre[VMDIR_MAX_ATTR_VALUE_META_DATA_LEN] = {0};

        value_len = (int)pMod->attr.vals[i].lberbv_len;
        p1 = strchr(pMod->attr.metaData, ':') + 1;
        p2 = strchr((strchr(p1, ':') + 1), ':');
        *p2 = '\0';

        retVal = VmDirStringNPrintFA(av_meta_pre, sizeof(av_meta_pre), sizeof(av_meta_pre) - 1, "%s:%s:%s:%s:%s:%s:%d:%d:",
                     pMod->attr.type.lberbv.bv_val, usnChanged, p1, gVmdirServerGlobals.invocationId.lberbv.bv_val,
                     pTimeStamp, usnChanged, pMod->operation, value_len);
        BAIL_ON_VMDIR_ERROR(retVal);

        *p2 = ':';
        retVal = VmDirAllocateMemory(sizeof(VDIR_BERVALUE), (PVOID)&pAVmeta);
        BAIL_ON_VMDIR_ERROR(retVal);

        av_meta_len = (int)strlen(av_meta_pre) + value_len;
        retVal = VmDirAllocateMemory(av_meta_len, (PVOID)&pAVmeta->lberbv.bv_val);
        BAIL_ON_VMDIR_ERROR(retVal);

        pAVmeta->bOwnBvVal = TRUE;
        pAVmeta->lberbv.bv_len = av_meta_len;
        retVal = VmDirCopyMemory(pAVmeta->lberbv.bv_val, av_meta_len, av_meta_pre, (int)strlen(av_meta_pre));
        BAIL_ON_VMDIR_ERROR(retVal);

        retVal = VmDirCopyMemory((char *)pAVmeta->lberbv.bv_val+(int)strlen(av_meta_pre), value_len,
                                  (char *)pMod->attr.vals[i].lberbv.bv_val, value_len);
        BAIL_ON_VMDIR_ERROR(retVal);

        retVal = dequePush(&pMod->attr.valueMetaDataToAdd, pAVmeta);
        BAIL_ON_VMDIR_ERROR(retVal);
        pAVmeta = NULL;
    }

cleanup:
    return retVal;

error:
    VmDirFreeBerval(pAVmeta);
    goto cleanup;
}
