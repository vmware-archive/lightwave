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

#define MAX_NUM_SECONDS_OF_SOCK_WRITE_RETRIES  10

static
int
IsAttrInReplScope(
    VDIR_OPERATION *    op,
    char *              attrType,
    char *              attrMetaData,
    USN                 priorSentUSNCreated,
    BOOLEAN *           inScope,
    PSTR*               ppszErrorMsg
    );

static
int
WriteAttributes(
   VDIR_OPERATION *   op,
   PVDIR_ENTRY        pEntry,
   uint32_t           iSearchReqSpecialChars,
   BerElement *       ber,
   PSTR*              ppszErrorMsg
   );

static
int
WriteMetaDataAttribute(
   VDIR_OPERATION *             pOp,
   VDIR_ATTRIBUTE *             pAttr,
   int                          numAttrMetaData,
   PATTRIBUTE_META_DATA_NODE    pAttrMetaData,
   BerElement *                 ber,
   BOOLEAN *                    nonTrivialAttrsInReplScope,
   PSTR*                        ppszErrorMsg
   );

static
int
WriteBerOnSocket(
   VDIR_CONNECTION * conn,
   BerElement *      ber );

static
ber_tag_t
GetResultTag(
   ber_tag_t tag );

static
VOID
SetSpecialReturnChar(
    SearchReq*      pSearchReq,
    uint32_t*       pSearchReqSpecialChars
    );

static
int
_WriteValueMetaDataAttribute(
   PVDIR_OPERATION              pOp,
   PDEQUE                       pAllValueMetaData,
   BerElement *                 pBer,
   PSTR*                        ppszErrorMsg
   );

static
int
PrepareValueMetaDataAttribute(
   PVDIR_OPERATION              pOp,
   PDEQUE                       pAllValueMetaData,
   PDEQUE                       pValueMetaDataToSend
   );

/*
    4.2.2. Bind Response
    The Bind response is defined as follows.
    BindResponse ::= [APPLICATION 1] SEQUENCE {
    COMPONENTS OF LDAPResult,
    serverSaslCreds [7] OCTET STRING OPTIONAL }
 */
VOID
VmDirSendSASLBindResponse(
    PVDIR_OPERATION     pOperation
    )
{
    DWORD               dwError = 0;
    BerElementBuffer    berbuf;
    BerElement *        ber = (BerElement *) &berbuf;
    PVDIR_LDAP_RESULT   pResult = &(pOperation->ldapResult);
    ber_tag_t           respType = GetResultTag(pOperation->reqCode);

    (void) memset( (char *)&berbuf, '\0', sizeof( BerElementBuffer ));

    ber_init2( ber, NULL, LBER_USE_DER );

    VMDIR_LOG_INFO( LDAP_DEBUG_ARGS, "VmDirSendLdapResponse: code (%d), Error (%d)(%s)",
                    respType, pResult->errCode, VDIR_SAFE_STRING(pResult->pszErrMsg));

    dwError = ber_printf( ber, "{it{ess" /*"}}"*/,
                        pOperation->msgId,                  // message sequence id
                        GetResultTag(pOperation->reqCode),  // ldap response type
                        pResult->errCode,                   // ldap return code e.g. LDAP_SASL_CONNTINUE
                        pResult->matchedDn.lberbv.bv_len > 0 ? pResult->matchedDn.lberbv.bv_val : "",
                        VDIR_SAFE_STRING(pResult->pszErrMsg));   // error text detail
    BAIL_ON_LBER_ERROR(dwError);

    // Send back TAG and SASL reply blob
    // NOTE, not exactly sure if we ever need to send Tag but WITHOUT blob reply if blob is empty.
    //       but this should NOT happen in GSSAPI scenario.
    if ( pResult->replyInfo.type == REP_SASL
         &&
         pResult->replyInfo.replyData.bvSaslReply.lberbv.bv_len > 0
       )
    {
        dwError = ber_printf( ber, "tO",
                              LDAP_TAG_SASL_RES_CREDS,
                              &pResult->replyInfo.replyData.bvSaslReply.lberbv );
        BAIL_ON_LBER_ERROR(dwError);
    }

    dwError = ber_printf( ber, "N}N}" );
    BAIL_ON_LBER_ERROR(dwError);

    dwError = WriteBerOnSocket( pOperation->conn, ber );
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    ber_free_buf( ber );

    return;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "lber error (%d)", dwError);
    goto cleanup;
}

void
VmDirSendLdapResult(
    PVDIR_OPERATION pOperation
    )
{
    BerElementBuffer berbuf;
    BerElement *     ber = (BerElement *) &berbuf;
    ber_int_t        msgId = 0;
    ber_tag_t        resCode = 0;
    size_t           iNumSearchEntrySent = 0;
    PCSTR            pszSocketInfo = NULL;

    (void)memset((char*)&berbuf, '\0', sizeof(BerElementBuffer));

    resCode = GetResultTag(pOperation->reqCode);
    msgId = (resCode != LBER_SEQUENCE) ? pOperation->msgId : 0;

    if (resCode == LDAP_RES_SEARCH_RESULT)
    {
        iNumSearchEntrySent = pOperation->request.searchReq.iNumEntrySent;
    }

    ber_init2(ber, NULL, LBER_USE_DER);

    if (pOperation->conn)
    {
        pszSocketInfo = pOperation->conn->szClientIP;
    }

    if (pOperation->ldapResult.errCode &&
        pOperation->ldapResult.errCode != LDAP_BUSY &&
        pOperation->ldapResult.errCode != LDAP_SASL_BIND_IN_PROGRESS)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "VmDirSendLdapResult: Request (%s), Error (%d), Message (%s), (%u) socket (%s)",
                VmDirLdapReqCodeToName(pOperation->reqCode),
                pOperation->ldapResult.errCode,
                VDIR_SAFE_STRING(pOperation->ldapResult.pszErrMsg),
                iNumSearchEntrySent,
                VDIR_SAFE_STRING(pszSocketInfo));
    }
    else if (pOperation->reqCode == LDAP_REQ_SEARCH)
    {
        VMDIR_LOG_INFO(
                LDAP_DEBUG_ARGS,
                "VmDirSendLdapResult: Request (%s), Error (%d), Message (%s), (%u) socket (%s)",
                VmDirLdapReqCodeToName(pOperation->reqCode),
                pOperation->ldapResult.errCode,
                VDIR_SAFE_STRING(pOperation->ldapResult.pszErrMsg),
                iNumSearchEntrySent,
                VDIR_SAFE_STRING(pszSocketInfo));
    }

    if (ber_printf(
            ber,
            "{it{essN}",
            msgId,
            resCode,
            pOperation->ldapResult.errCode,
            "",
            VDIR_SAFE_STRING(pOperation->ldapResult.pszErrMsg)) == -1)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "SendLdapResult: ber_printf (to print msgId ...) failed");
        goto done;
    }

    // If Search, Replication, and one or more entries were sent back => Send back Sync Done Control
    if (pOperation->reqCode == LDAP_REQ_SEARCH && pOperation->syncReqCtrl && pOperation->syncDoneCtrl)
    {
        if (WriteSyncDoneControl(pOperation, ber) != LDAP_SUCCESS)
        {
            goto done;
        }
    }

    if (pOperation->reqCode == LDAP_REQ_SEARCH && pOperation->showPagedResultsCtrl)
    {
        if (WritePagedSearchDoneControl(pOperation, ber) != LDAP_SUCCESS)
        {
            goto done;
        }
    }

    if (ber_printf(ber, "N}") == -1)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "SendLdapResult: ber_printf (to print msgId ...) failed");
        goto done;
    }

    if (WriteBerOnSocket(pOperation->conn, ber) != 0)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "SendLdapResult: WriteBerOnSocket failed");
        goto done;
    }

done:
    ber_free_buf(ber);
}

int
VmDirSendSearchEntry(
   PVDIR_OPERATION     pOperation,
   PVDIR_ENTRY         pSrEntry
   )
{
    int                         retVal = LDAP_SUCCESS;
    BerElementBuffer            berbuf;
    BerElement *                ber = (BerElement *) &berbuf;
    BOOLEAN                     bFreeBer = FALSE;
    ber_len_t                   iBlobSize = 0;
    BerValue                    lberBervEntryBlob = {0};
    int                         nAttrs = 0;
    int                         nVals = 0;
    BOOLEAN                     attrMetaDataReqd = FALSE;
    SearchReq *                 sr = &(pOperation->request.searchReq);
    int                         i = 0;
    BOOLEAN                     nonTrivialAttrsInReplScope = FALSE;
    uint32_t                    iSearchReqSpecialChars = 0;
    PATTRIBUTE_META_DATA_NODE   pAttrMetaData = NULL;
    int                         numAttrMetaData = 0;
    DEQUE                       allValueMetaData = {0};
    DEQUE                       valueMetaDataToSend = {0};
    PVDIR_ATTRIBUTE             pAttr = NULL;
    USN                         usnChanged = 0;
    PSTR                        pszLocalErrorMsg = NULL;

    if ( !pOperation || !pSrEntry )
    {
        retVal = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    pSrEntry->bSearchEntrySent = FALSE;

    // see if client request has "*" and/or "+" ("-" for userpassword internal to vmdir)
    // WEI TODO: when we have attribute level ACL check, this information will be useful
    // return result will depend on ACL each on each attribute client is asking before
    // generating a final result to send back
    SetSpecialReturnChar(&pOperation->request.searchReq, &iSearchReqSpecialChars);

    if (pSrEntry->eId == DSE_ROOT_ENTRY_ID &&
        pOperation->request.searchReq.attrs == NULL)
    {
        //  For ADSI, if no specific attributes requested of DSE ROOT search,
        //  return ALL (include operational) attributes.
        iSearchReqSpecialChars |= LDAP_SEARCH_REQUEST_CHAR_OP;
    }

    // ACL check before processing/sending the current srEntry back
    retVal = VmDirSrvAccessCheck(
            pOperation,
            &pOperation->conn->AccessInfo,
            pSrEntry,
            pOperation->request.searchReq.accessRequired);
    BAIL_ON_VMDIR_ERROR(retVal);

    if (pOperation->opType == VDIR_OPERATION_TYPE_INTERNAL)
    {
        // This is an internal search operation.
        // Set bSearchEntrySent = TRUE to indicate that ACL
        // check passed and should be included in the result.
        pSrEntry->bSearchEntrySent = TRUE;
    }
    else if (sr->bStoreRsltInMem)
    {
        // This is an external search operation but wants to
        // store the result in memory instead of sending.
        // Set bSearchEntrySent = TRUE to indicate that ACL
        // check passed and should be included in the result.
        pSrEntry->bSearchEntrySent = TRUE;
    }
    else
    {
        // If not replication, and showDeletedObjectsCtrl not present, => don't send back Deleted objects (tombstones).
        if (pOperation->syncReqCtrl == NULL && pOperation->showDeletedObjectsCtrl == NULL)
        {
            pAttr = VmDirEntryFindAttribute(ATTR_IS_DELETED, pSrEntry);
            if (pAttr)
            {
                if (VmDirStringCompareA((PSTR)pAttr->vals[0].lberbv.bv_val, VMDIR_IS_DELETED_TRUE_STR, FALSE) == 0)
                {
                    goto cleanup; // Don't send this entry
                }
            }
        }
        // In case of replication request, skip certain updates
        if (pOperation->syncReqCtrl != NULL)
        {
            PVDIR_ATTRIBUTE                 pAttrUsnCreated = NULL;
            USN                             usnCreated = 0;
            VMDIR_REPLICATION_AGREEMENT *   replAgr = NULL;

            pAttr = VmDirEntryFindAttribute(ATTR_USN_CHANGED, pSrEntry);
            assert( pAttr != NULL );
            usnChanged = VmDirStringToLA( pAttr->vals[0].lberbv.bv_val, NULL, 10);

            // Check if usnChanged is beyond lowestPendingUncommittedUsn recorded at the beginning of replication search
            if (pOperation->lowestPendingUncommittedUsn)
            {
                if (usnChanged == pOperation->lowestPendingUncommittedUsn)
                {
                    // This usn is successfully committed, bump lowestPendingUncommittedUsn by 1
                    pOperation->lowestPendingUncommittedUsn++;

                    VMDIR_LOG_INFO( LDAP_DEBUG_REPL,
                            "SendSearchEntry: bumping lowestPendingUncommittedUsn to %" PRId64,
                            pOperation->lowestPendingUncommittedUsn );
                }
                else if (usnChanged > pOperation->lowestPendingUncommittedUsn)
                {
                    VMDIR_LOG_INFO( LDAP_DEBUG_REPL,
                            "SendSearchEntry: usnChanged = %" PRId64 ", lowestPendingUncommittedUsn = %" PRId64 ", "
                            "skipping entry: %s", usnChanged, pOperation->lowestPendingUncommittedUsn,
                            pSrEntry->dn.lberbv.bv_val );

                    // Shouldn't stop cycle until we don't have a skip, inform consumer to come back again
                    pOperation->syncDoneCtrl->value.syncDoneCtrlVal.bContinue = TRUE;
                    goto updateSyncDoneCtrl; // Don't send this entry
                }
            }

            // Don't send (skip) modifications to my server object, and my RAs
            pAttrUsnCreated = VmDirEntryFindAttribute(ATTR_USN_CREATED, pSrEntry);
            assert( pAttrUsnCreated != NULL );
            usnCreated = VmDirStringToLA( pAttrUsnCreated->vals[0].lberbv.bv_val, NULL, 10);
            // Only send back creation of certain objects, and not their modifications.
            // Check if consumer has already seen the creation. If yes, we are dealing with mods, which should be skipped
            // for my server object, and my RAs
            // Note: Skipping mods for RAs and Server Objects will cause inconsistencies with replicas. But these
            // two types only have local scope regarding functional effects. But if we are looking up / processing
            // information for these two types of objects on a replica, we need to watch out for potential
            // inconsistencies against the original source.
            if (pOperation->syncReqCtrl->value.syncReqCtrlVal.intLastLocalUsnProcessed > usnCreated)
            {
                if (strcmp(pSrEntry->dn.bvnorm_val, gVmdirServerGlobals.serverObjDN.bvnorm_val) == 0)
                {
                    VMDIR_LOG_INFO( LDAP_DEBUG_REPL, "SendSearchEntry: Not sending modifications to my server object, DN: %s",
                              gVmdirServerGlobals.serverObjDN.lberbv.bv_val );
                    goto cleanup; // Don't send this entry
                }
                for (replAgr = gVmdirReplAgrs; replAgr != NULL; replAgr = replAgr->next )
                {
                    if (strcmp(pSrEntry->dn.bvnorm_val, replAgr->dn.bvnorm_val) == 0)
                    {
                        VMDIR_LOG_INFO( LDAP_DEBUG_REPL, "SendSearchEntry: Not sending modifications to my RA object, DN: %s",
                                  replAgr->dn.bvnorm_val );
                        goto cleanup; // Don't send this entry
                    }
                }
            }

            // do not replicate DSE Root entry, because it is a "local" entry.
            if (pSrEntry->eId == DSE_ROOT_ENTRY_ID)
            {
                VMDIR_LOG_INFO( LDAP_DEBUG_REPL, "SendSearchEntry: Not sending modifications to DSE Root entry, DN: %s",
                          pSrEntry->dn.bvnorm_val );
                goto cleanup; // Don't send this entry
            }

        }

        // Approximate calculation for the required ber size, because apart from lengths and strings, ber also includes
        // tags.
        if ( VmDirComputeEncodedEntrySize(pSrEntry, &nAttrs, &nVals, &iBlobSize) != 0
             ||
             VmDirAllocateMemory(iBlobSize, (PVOID*)&lberBervEntryBlob.bv_val) != 0
           )
        {
            retVal = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg), "no memory");
        }
        lberBervEntryBlob.bv_len = iBlobSize;

        ber_init2( ber, &lberBervEntryBlob, LBER_USE_DER );  // ber takes over lberBervEntryBlob.lberbv.bv_val ownership
        bFreeBer = TRUE;

        if ( ber_printf( ber, "{it{O{", pOperation->msgId, LDAP_RES_SEARCH_ENTRY, &pSrEntry->dn ) == -1)
        {
            VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "SendSearchEntry: ber_printf (to print msgId ...) failed" );
            retVal = LDAP_OTHER;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                            "Encoding msgId, RES_SEARCH_ENTRY, DN failed");
        }
        // Determine if we need to send back the attribute metaData
        if ( pOperation->syncReqCtrl != NULL ) // Replication
        {
            attrMetaDataReqd = TRUE;
        }
        else // check if attrMetaData attribute has been requested explicitly.
        {
            if (sr->attrs != NULL)
            {
                for (i = 0; sr->attrs[i].lberbv.bv_val != NULL; i++)
                {
                    if (VmDirStringCompareA( sr->attrs[i].lberbv.bv_val, ATTR_ATTR_META_DATA, FALSE) == 0)
                    {
                        attrMetaDataReqd = TRUE;
                        break;
                    }
                }
            }
        }

        if (attrMetaDataReqd)
        {
            if ((pOperation->pBEIF->pfnBEGetAllAttrsMetaData( pOperation->pBECtx, pSrEntry->eId, &pAttrMetaData,
                                                              &numAttrMetaData )) != 0)
            {
                VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "SendSearchEntry: pfnBEGetAllAttrsMetaData failed for entryId: %ld",
                                   pSrEntry->eId);
                retVal = LDAP_OPERATIONS_ERROR;
                BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                                "pfnBEGetAllAttrsMetaData failed.");
            }

            if ((pOperation->pBEIF->pfnBEGetAllAttrValueMetaData( pOperation->pBECtx, pSrEntry->eId, &allValueMetaData)) != 0)
            {
                VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "SendSearchEntry: pfnBEGetAllAttrValueMetaData failed for entryId: %ld",
                                   pSrEntry->eId);
                retVal = LDAP_OPERATIONS_ERROR;
                BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                                "pfnBEGetAllAttrValueMetaData failed.");
            }

            // SJ-TBD: Following double for loop to be optimized
            // Copy attrMetaData to corresponding attributes
            for (i=0; i<numAttrMetaData; i++)
            {
                for ( pAttr = pSrEntry->attrs; pAttr != NULL; pAttr = pAttr->next)
                {
                    if (pAttr->pATDesc->usAttrID == pAttrMetaData[i].attrID)
                    {
                        VmDirStringCpyA( pAttr->metaData, VMDIR_MAX_ATTR_META_DATA_LEN, pAttrMetaData[i].metaData );
                        pAttrMetaData[i].metaData[0] = '\0';
                    }
                }
            }
        }

        retVal = WriteAttributes( pOperation, pSrEntry, iSearchReqSpecialChars , ber, &pszLocalErrorMsg );
        BAIL_ON_VMDIR_ERROR( retVal );

        if (attrMetaDataReqd)
        {
            retVal = WriteMetaDataAttribute( pOperation, pSrEntry->attrs, numAttrMetaData, pAttrMetaData, ber,
                                             &nonTrivialAttrsInReplScope, &pszLocalErrorMsg );
            BAIL_ON_VMDIR_ERROR( retVal );

            if (pOperation->syncReqCtrl == NULL)
            {
                if (!dequeIsEmpty(&allValueMetaData))
                {
                    retVal = _WriteValueMetaDataAttribute( pOperation, &allValueMetaData, ber, &pszLocalErrorMsg );
                    BAIL_ON_VMDIR_ERROR( retVal );
                }
            }
            else
            {
                retVal = PrepareValueMetaDataAttribute( pOperation, &allValueMetaData, &valueMetaDataToSend );
                BAIL_ON_VMDIR_ERROR( retVal );
                if (!dequeIsEmpty(&valueMetaDataToSend))
                {
                    retVal = _WriteValueMetaDataAttribute( pOperation, &valueMetaDataToSend, ber, &pszLocalErrorMsg );
                    BAIL_ON_VMDIR_ERROR( retVal );
                    nonTrivialAttrsInReplScope = TRUE;
                }
            }
        }

        if (ber_printf( ber, "N}N}" ) == -1)
        {
            VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                      "ber_printf (to terminate the entry and the complete search result entry message ...) failed" );
            retVal = LDAP_OTHER;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                            "Encoding terminating the entry failed.");
        }

        if ( pOperation->syncReqCtrl != NULL ) // Replication, => write Sync State Control
        {
            retVal = WriteSyncStateControl( pOperation, pSrEntry, ber, &pszLocalErrorMsg );
            BAIL_ON_VMDIR_ERROR( retVal );
        }

        if (ber_printf( ber, "N}" ) == -1)
        {
            VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                      "ber_printf (to terminate the entry and the complete search result entry message ...) failed" );;
            retVal = LDAP_OTHER;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                            "Encoding terminating the entry failed.");
        }

        if ((pOperation->syncReqCtrl == NULL) || (pOperation->syncReqCtrl != NULL && nonTrivialAttrsInReplScope ))
        {
            if (WriteBerOnSocket( pOperation->conn, ber ) != 0)
            {
                VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "SendSearchEntry: WriteBerOnSocket failed." );
                retVal = LDAP_UNAVAILABLE;
                BAIL_ON_VMDIR_ERROR( retVal );
            }

            pSrEntry->bSearchEntrySent = TRUE;
            sr->iNumEntrySent++;

            VMDIR_LOG_INFO( LDAP_DEBUG_REPL, "SendSearchEntry: Send entry: %s", pSrEntry->dn.lberbv.bv_val);
        }
        else
        {
            VMDIR_LOG_INFO( LDAP_DEBUG_REPL, "SendSearchEntry: NOT Sending entry: %s %p %d",
                            pSrEntry->dn.lberbv.bv_val, pOperation->syncReqCtrl, nonTrivialAttrsInReplScope);
        }

updateSyncDoneCtrl:
        if (pOperation->syncReqCtrl != NULL)
        {
            if (pOperation->syncDoneCtrl->value.syncDoneCtrlVal.bContinue)
            {
                VMDIR_LOG_INFO(
                        LDAP_DEBUG_REPL,
                        "%s: update lastLocalUsnProcessed from %" PRId64 " to lowestPendingUncommittedUsn %" PRId64 " to avoid retry",
                        __FUNCTION__,
                        pOperation->syncDoneCtrl->value.syncDoneCtrlVal.intLastLocalUsnProcessed,
                        pOperation->lowestPendingUncommittedUsn-1);
                /*
                 * Sending high watermark to consumer results in repl cycle retry.
                 * Avoid retry by sending lowestpendingUncommittedUsn-1.
                 */
                pOperation->syncDoneCtrl->value.syncDoneCtrlVal.intLastLocalUsnProcessed = pOperation->lowestPendingUncommittedUsn-1;
            }
            else if (usnChanged  > pOperation->syncDoneCtrl->value.syncDoneCtrlVal.intLastLocalUsnProcessed)
            {
                // record max local usnChanged in syncControlDone
                pOperation->syncDoneCtrl->value.syncDoneCtrlVal.intLastLocalUsnProcessed = usnChanged;
            }
        }

        retVal = LDAP_SUCCESS;
    }

cleanup:
    if (bFreeBer)
    {
        ber_free_buf( ber );
    }
    VMDIR_SAFE_FREE_MEMORY( pAttrMetaData );
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    VmDirFreeAttrValueMetaDataContent(&allValueMetaData);
    VmDirFreeAttrValueMetaDataContent(&valueMetaDataToSend);
    return( retVal );

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "SendSearchEntry failed DN=(%s), (%u)(%s)",
                     (pSrEntry && pSrEntry->dn.lberbv.bv_val) ? pSrEntry->dn.lberbv.bv_val : "",
                     retVal, VDIR_SAFE_STRING( pszLocalErrorMsg));

    if ( !pOperation->ldapResult.pszErrMsg && pszLocalErrorMsg )
    {
        pOperation->ldapResult.pszErrMsg = pszLocalErrorMsg;
        pszLocalErrorMsg = NULL;
    }

    goto cleanup;
}

/*
 * Test whether the origUsn is in scope so that attribute, attr-meta-data or
 * attr-value-meta-data be sent back to the replicaiton consumer based on whether
 * the origUsn for that invocationId has been processed already by the consumer
 */
static
int
_VmDirIsUsnInScope(
    VDIR_OPERATION *    op,
    PCSTR               pAttrName,
    char *              origInvocationId,
    USN                 origUsn,
    USN                 priorSentUSNCreated,
    BOOLEAN *           isUsnInScope
    )
{
    int                     retVal = LDAP_SUCCESS;
    PLW_HASHTABLE_NODE      pNode = NULL;
    PSTR                    pszLocalErrorMsg = NULL;
    UptoDateVectorEntry *   utdVectorEntry = NULL;
    int                     rc = 0;

    *isUsnInScope = FALSE;
    rc = LwRtlHashTableFindKey( op->syncDoneCtrl->value.syncDoneCtrlVal.htUtdVector, &pNode, origInvocationId );
    rc = LwNtStatusToWin32Error(rc);
    if (rc != 0 && rc != ERROR_NOT_FOUND)
    {
        VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "_VmDirIsUsnInScope: LwRtlHashTableFindKey failed for origInvocationId: %s",
                  origInvocationId );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg), "LwRtlHashTableFindKey failed.");
    }

    if (pNode == NULL)
    {
        UptoDateVectorEntry * utdVectorEntry = NULL;
        VDIR_BERVALUE         bvServerId = VDIR_BERVALUE_INIT;

        if (VmDirAllocateMemory( sizeof( UptoDateVectorEntry ), (PVOID *)&utdVectorEntry) != 0)
        {
            retVal = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                            "_VmDirIsUsnInScope: BervalContentDup failed.");
        }
        bvServerId.lberbv.bv_val = origInvocationId;
        bvServerId.lberbv.bv_len = VmDirStringLenA( origInvocationId );
        if (VmDirBervalContentDup( &bvServerId, &utdVectorEntry->invocationId ) != 0)
        {
            retVal = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                            "_VmDirIsUsnInScope: BervalContentDup failed.");
        }
        utdVectorEntry->currMaxOrigUsnProcessed = origUsn;
        LwRtlHashTableResizeAndInsert( op->syncDoneCtrl->value.syncDoneCtrlVal.htUtdVector,
                                       &utdVectorEntry->Node, &pNode);
        assert( pNode == NULL );    // assert the key of added node is unique
        *isUsnInScope = TRUE;
        goto cleanup;
    }

    utdVectorEntry = (UptoDateVectorEntry *)LW_STRUCT_FROM_FIELD(pNode, UptoDateVectorEntry, Node);
    if (origUsn > utdVectorEntry->reqLastOrigUsnProcessed )
    {
        // attribute or the valueMetaData item in scope if origUsn valueMetaData is > the current highest
        if (origUsn > utdVectorEntry->currMaxOrigUsnProcessed )
        {
            utdVectorEntry->currMaxOrigUsnProcessed = origUsn;
        }

        // Note, this handles ADD->MODIFY case but not multiple MODIFYs scenario.
        // However, it is fine as consumer should be able to handle redundant feed from supplier.
        // The key point here is to NOT send ATTR_USN_CREATED, so we can derive correct sync_state in WriteSyncStateControl.
        if (origUsn > priorSentUSNCreated)
        {
            *isUsnInScope = TRUE;

            if (priorSentUSNCreated > 0)
            {
                VMDIR_LOG_VERBOSE(LDAP_DEBUG_REPL, "%s new usn %llu after prior usncreated %llu attr %s",
                                        __FUNCTION__, origUsn, priorSentUSNCreated, VDIR_SAFE_STRING(pAttrName));
            }
        }
        else
        {
            VMDIR_LOG_VERBOSE(LDAP_DEBUG_REPL, "%s skip prior usncreated %llu attr %s",
                                    __FUNCTION__, priorSentUSNCreated, VDIR_SAFE_STRING(pAttrName));
        }

        goto cleanup;
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return( retVal );

error:
    goto cleanup;
}

static
int
IsAttrInReplScope(
    VDIR_OPERATION *    op,
    char *              attrType,
    char *              attrMetaData,
    USN                 priorSentUSNCreated,
    BOOLEAN *           inScope,
    PSTR*               ppszErrorMsg
    )
{
    int                     retVal = LDAP_SUCCESS;
    char                    origInvocationId[VMDIR_GUID_STR_LEN];
    //origUsn is set to <originating USN>
    USN                     origUsn = VmDirStringToLA( VmDirStringRChrA( attrMetaData, ':' ) + 1, NULL, 10 );
    PSTR                    pszLocalErrorMsg = NULL;

    *inScope = FALSE;

    // attrMetaData format is: <local USN>:<version no>:<originating server ID>:<originating time>:<originating USN>
    VmDirStringNCpyA( origInvocationId, VMDIR_GUID_STR_LEN,
                      VmDirStringChrA( VmDirStringChrA( attrMetaData, ':' ) + 1, ':') + 1, VMDIR_GUID_STR_LEN - 1);
    origInvocationId[VMDIR_GUID_STR_LEN - 1] = '\0';

    // Skip the attribute:
    //    - if the originating server for the current state is same as the requesting server or if it is one of those
    //      attributes that have "local" scope only. E.g. sending ATTR_LAST_LOCAL_USN_PROCESSED and
    //      ATTR_UP_TO_DATE_VECTOR, causes continuous back-forth replication of Replication Agreements and Server
    //      entries between various servers.

    assert( op->syncReqCtrl != NULL );

    if ((attrType != NULL && (VmDirStringCompareA( attrType, ATTR_LAST_LOCAL_USN_PROCESSED, FALSE) == 0 ||
                              VmDirStringCompareA( attrType, ATTR_UP_TO_DATE_VECTOR, FALSE) == 0 ||
                              VmDirStringCompareA( attrType, VDIR_ATTRIBUTE_SEQUENCE_RID, FALSE) == 0)))

    {
        // Reset metaData value so that we don't send local only attribute back.
        attrMetaData[0] = '\0';
        *inScope = FALSE;
        goto cleanup;
    }
    else if ( attrType != NULL && (VmDirStringCompareA( attrType, ATTR_USN_CHANGED, FALSE) == 0))
    {
        ; // always send uSNChanged. (PR 1573117)
    }
    else if ( attrType != NULL && gVmdirServerGlobals.dwDomainFunctionalLevel >= VDIR_DFL_MODDN &&
              (VmDirStringCompareA( attrType, ATTR_OBJECT_GUID, FALSE) == 0))
    {
        ; // always send objectGUID to uniquely identify entries, regardless
          // which node the entry was created. (PR 1730608)
    }
    else if (VmDirStringCompareA( origInvocationId,
                  op->syncReqCtrl->value.syncReqCtrlVal.reqInvocationId.lberbv.bv_val,TRUE ) == 0)
    {
        // Change is originated from the requesting server.
        // Reset metaData value so that we don't send metaData as well as this attribute back.
        attrMetaData[0] = '\0';
        *inScope = FALSE;
        goto cleanup;
    }
    else
    {
        BOOLEAN usnInScope = FALSE;
        retVal = _VmDirIsUsnInScope(op, attrType, origInvocationId, origUsn, priorSentUSNCreated, &usnInScope);
        BAIL_ON_VMDIR_ERROR(retVal);
        if (!usnInScope)
        {
            VMDIR_LOG_VERBOSE( LDAP_DEBUG_REPL_ATTR,
                      "IsAttrInReplScope: Attribute: %s, metaData: %s, replication scope = FALSE",
                      attrType, attrMetaData );

            // Reset metaData value so that we don't send metaData for this attribute back.
            attrMetaData[0] = '\0';
            *inScope = FALSE;
            goto cleanup;
        }
    }

    VMDIR_LOG_INFO( LDAP_DEBUG_REPL_ATTR, "IsAttrInReplScope: Attribute: %s, metaData: %s, replication scope = TRUE",
              attrType, attrMetaData );
    *inScope = TRUE;

cleanup:

    if (ppszErrorMsg)
    {
        *ppszErrorMsg = pszLocalErrorMsg;
    }
    else
    {
        VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    }

    return( retVal );

error:

    goto cleanup;
}

static
int
WriteAttributes(
   VDIR_OPERATION *   op,
   PVDIR_ENTRY        pEntry,
   uint32_t           iSearchReqSpecialChars,
   BerElement *       ber,
   PSTR*              ppszErrorMsg
   )
{
    int             retVal = LDAP_SUCCESS;
    unsigned int    i = 0;
    SearchReq *     sr = &(op->request.searchReq);
    BOOLEAN         mapFSPDN = FALSE;
    PVDIR_ATTRIBUTE pAttr = NULL;
    PVDIR_ATTRIBUTE pRetAttrs[3] = {pEntry->attrs, pEntry->pComputedAttrs, NULL};
    DWORD           dwCnt = 0;
    PSTR            pszLocalErrorMsg = NULL;
    CHAR            pszIDBuf[VMDIR_MAX_I64_ASCII_STR_LEN] = {0};
    PSTR            pszPriorSentUSNCreated = NULL;
    USN             priorSentUSNCreated = 0;

    if (op->syncReqCtrl != NULL)
    {
        assert( VmDirStringNPrintFA(pszIDBuf, VMDIR_MAX_I64_ASCII_STR_LEN, VMDIR_MAX_I64_ASCII_STR_LEN, "%llu", pEntry->eId) == 0 );

        if (LwRtlHashMapFindKey(op->conn->ReplConnState.phmSyncStateOneMap, (PVOID*)&pszPriorSentUSNCreated, pszIDBuf) == 0)
        {   // we have already sent this entry back with sync_state ADD in the same replication cycle
            priorSentUSNCreated = VmDirStringToLA( pszPriorSentUSNCreated, NULL, 10 );

            VMDIR_LOG_VERBOSE(LDAP_DEBUG_REPL, "%s sent %s with USNCreatd %llu before", __FUNCTION__, pEntry->dn.lberbv_val, priorSentUSNCreated);
        }
    }

    // loop through both normal and computed attributes
    for ( dwCnt = 0, pAttr = pRetAttrs[dwCnt];
          pAttr != NULL;
          ++dwCnt, pAttr = pRetAttrs[dwCnt])
    {
        for ( ; pAttr != NULL;  pAttr = pAttr->next )
        {
            BOOLEAN bSendAttribute = FALSE;

            if (op->syncReqCtrl != NULL) // Replication,
            {
                // Filter attributes based on the input utdVector, and attribute's meta-data
                retVal = IsAttrInReplScope( op, pAttr->type.lberbv.bv_val, pAttr->metaData, priorSentUSNCreated, &bSendAttribute, &pszLocalErrorMsg );
                BAIL_ON_VMDIR_ERROR( retVal );
            }
            else
            {
                // WEI: the correct way to handle this is to have attribute level 'ACL' check
                // For instance, the 'userPassword' attribute ACL defines:
                // Deny 'READ' access to everyone including administrator/admins/SELF
                // Possibly only allow 'backup Operators' 'READ' access
                // Allow 'administrator/admins' WRITE (set password)
                // Allow 'SELF' WRITE (reset password)

                // For now, do not send 'userPassword' attribute for everyone when implicitly/explicitly requested

                // normal ldap client search request
                // check if we need to return this attribute

                if (VmDirStringCompareA(pAttr->type.lberbv.bv_val, ATTR_USER_PASSWORD, FALSE) == 0)
                {
                    if ((iSearchReqSpecialChars & LDAP_SEARCH_REQUEST_CHAR_PASSWD) != 0)
                    {   // vmdir specific - return password only if special char '-' is requested
                        bSendAttribute = TRUE;
                    }
                }
                else if (VmDirStringCompareA(pAttr->type.lberbv.bv_val, ATTR_KRB_MASTER_KEY, FALSE) == 0)
                {
                    //only return master key if all of following conditions are satisfied
                    if (op->showMasterKeyCtrl && // server control must be present
                            op->conn->bIsLdaps && // must be ldaps
                            op->conn->AccessInfo.pszNormBindedDn && // can't be anonymous
                            VmDirStringCompareA(op->reqDn.bvnorm_val, gVmdirServerGlobals.systemDomainDN.bvnorm_val, FALSE) == 0 && // must query system domain object
                            op->request.searchReq.scope == LDAP_SCOPE_BASE && // scope must be base
                            op->request.searchReq.attrs != NULL && // attribute must be krbMKey only
                            VmDirStringCompareA(op->request.searchReq.attrs[0].lberbv.bv_val, ATTR_KRB_MASTER_KEY, FALSE) == 0 &&
                            op->request.searchReq.attrs[1].lberbv.bv_val == NULL)
                    {
                        retVal = VmDirSrvAccessCheckIsAdminRole( //must be administrator
                                                        op,
                                                        op->conn->AccessInfo.pszNormBindedDn,
                                                        &op->conn->AccessInfo,
                                                        &bSendAttribute);
                        BAIL_ON_VMDIR_ERROR( retVal );
                    }
                }
                else if ( (VmDirStringCompareA(pAttr->type.lberbv.bv_val, ATTR_VMW_STS_PASSWORD, FALSE) == 0)
                           ||
                           (VmDirStringCompareA(pAttr->type.lberbv.bv_val, ATTR_VMW_STS_TENANT_KEY, FALSE) == 0)
                        )
                {
                    //only admin users can read ATTR_VMW_STS_PASSWORD, ATTR_VMW_STS_TENANT_KEY attribute
                    {
                        retVal = VmDirSrvAccessCheckIsAdminRole( //must be administrator
                                                        op,
                                                        op->conn->AccessInfo.pszNormBindedDn,
                                                        &op->conn->AccessInfo,
                                                        &bSendAttribute);
                        BAIL_ON_VMDIR_ERROR( retVal );
                    }
                }
                else if (((iSearchReqSpecialChars & LDAP_SEARCH_REQUEST_CHAR_USER) != 0)                   &&
                         pAttr->pATDesc->usage == VDIR_LDAP_USER_APPLICATIONS_ATTRIBUTE)
                {
                    bSendAttribute = TRUE;   // return all user attributes - "*"
                }
                else if (((iSearchReqSpecialChars & LDAP_SEARCH_REQUEST_CHAR_OP) != 0)                     &&
                         (pAttr->pATDesc->usage == VDIR_LDAP_DIRECTORY_OPERATION_ATTRIBUTE ||
                          pAttr->pATDesc->usage == VDIR_LDAP_DSA_OPERATION_ATTRIBUTE       ||
                          pAttr->pATDesc->usage == VDIR_LDAP_DISTRIBUTED_OPERATION_ATTRIBUTE))
                {
                    bSendAttribute = TRUE;   // return all operational attributes - "+"
                }
                else
                {
                    for (i = 0; sr->attrs && sr->attrs[i].lberbv.bv_val != NULL; i++)
                    {
                        if (VmDirStringCompareA( sr->attrs[i].lberbv.bv_val, pAttr->type.lberbv.bv_val, FALSE) == 0)
                        {
                            //
                            // Access checks for a search request requires the
                            // caller to have VMDIR_RIGHT_DS_READ_PROP access
                            // to the entry. This will allow them to "see" the
                            // entry and any attributes EXCEPT for the entry's
                            // security descriptor. The SD is goverened by a
                            // separate permission, VMDIR_ENTRY_READ_ACL. So,
                            // if the caller requested that attribute we have
                            // to make sure they have the permission required.
                            //
                            if (VmDirStringCompareA(pAttr->type.lberbv.bv_val, ATTR_ACL_STRING, FALSE) == 0 ||
                                VmDirStringCompareA(pAttr->type.lberbv.bv_val, ATTR_OBJECT_SECURITY_DESCRIPTOR, FALSE) == 0)
                            {
                                bSendAttribute = (VmDirSrvAccessCheck(op, &op->conn->AccessInfo, pEntry, VMDIR_ENTRY_READ_ACL) == 0);
                            }
                            else
                            {
                                bSendAttribute = TRUE;
                            }

                            break;
                        }
                    }
                }
            }

            if (bSendAttribute)
            {
                if (ber_printf( ber, "{O[", &(pAttr->type) ) == -1 )
                {
                    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "WriteAttributes: ber_printf (to print attribute name ...) failed" );
                    retVal = LDAP_OTHER;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                                    "Encoding attribute type failed.");
                }

                // Non-replication/normal search request, and the attribute is FSP enabled (SJ-TBD, for now ATTR_MEMBER)
                if (op->syncReqCtrl == NULL && VmDirStringCompareA(pAttr->type.lberbv.bv_val, ATTR_MEMBER, FALSE) == 0)
                {
                    mapFSPDN = TRUE;
                }
                else
                {
                    mapFSPDN = FALSE;
                }

                if ( VmDirStringCompareA( pAttr->type.lberbv_val, ATTR_OBJECT_CLASS, FALSE ) == 0 )
                {
                    if ( op->syncReqCtrl == NULL ) // Not replication
                    {
                        BerValue bvOCTop = {OC_TOP_LEN, OC_TOP};
                        // explicitly send TOP as we don't store it in Entry/DB
                        if (ber_printf( ber, "O", &bvOCTop ) == -1 )
                        {
                            VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "WriteAttributes: ber_printf (to print an attribute value ...) failed." );
                            retVal = LDAP_OTHER;
                            BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                                            "Encoding an attribute value failed.");
                        }
                    }

                    // ADSI wants structure objetclass at the end to display correctly.
                    for ( i = pAttr->numVals ; i > 0; i-- )
                    {
                        if (ber_printf( ber, "O", &(pAttr->vals[i-1]) ) == -1 )
                        {
                            VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "WriteAttributes: ber_printf (to print an attribute value ...) failed." );
                            retVal = LDAP_OTHER;
                            BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                                            "Encoding an attribute value failed.");
                        }
                    }
                }
                else
                {
                    for ( i = 0; i< pAttr->numVals; i++ )
                    {
                        // SJ-TBD: A better/bit-more-expensive way is to Normalize the value, do GetParentDN, check if the 1st
                        // RDN is FSP container RDN ...
                        if (mapFSPDN && VmDirStringStrA(pAttr->vals[i].lberbv.bv_val, FSP_CONTAINER_RDN_ATTR_VALUE) != NULL)
                        {
                            char * tmpPos = VmDirStringChrA(pAttr->vals[i].lberbv.bv_val, ',');
                            assert( tmpPos != NULL);
                            *tmpPos = '\0';
                            pAttr->vals[i].lberbv.bv_len = tmpPos - pAttr->vals[i].lberbv.bv_val;
                        }
                        if (ber_printf( ber, "O", &(pAttr->vals[i]) ) == -1 )
                        {
                            VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "WriteAttributes: ber_printf (to print an attribute value ...) failed." );
                            retVal = LDAP_OTHER;
                            BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                                            "Encoding an attribute value failed.");
                        }
                    }
                }

                if ( ber_printf( ber, "]N}" ) == -1 )
                {
                    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "WriteAttributes: ber_printf (to terminate an attribute's values ...) failed" );
                    retVal = LDAP_OTHER;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                                    "Encoding terminating an attribute type failed.");
                }
            }
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

    return( retVal );

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "WriteAttributes failed (%u)(%s)",
                     retVal, VDIR_SAFE_STRING(pszLocalErrorMsg) );

    goto cleanup;
}

static
int
WriteMetaDataAttribute(
   VDIR_OPERATION *             pOp,
   VDIR_ATTRIBUTE *             pAttr,
   int                          numAttrMetaData,
   PATTRIBUTE_META_DATA_NODE    pAttrMetaData,
   BerElement *                 ber,
   BOOLEAN *                    nonTrivialAttrsInReplScope,
   PSTR*                        ppszErrorMsg
   )
{
    int             retVal = LDAP_SUCCESS;
    VDIR_BERVALUE   attrMetaData = { {ATTR_ATTR_META_DATA_LEN, ATTR_ATTR_META_DATA}, 0, 0, NULL };
    int             i = 0;
    VDIR_BERVALUE   berVal = VDIR_BERVALUE_INIT;
    char            attrMetaDataVal[256 + VMDIR_MAX_ATTR_META_DATA_LEN];
    PSTR            pszLocalErrorMsg = NULL;

    *nonTrivialAttrsInReplScope = FALSE;

    if (ber_printf( ber, "{O[", &(attrMetaData) ) == -1 )
    {
        VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "WriteMetaDataAttribute: ber_printf (to print attribute name ...) failed" );
        retVal = LDAP_OTHER;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                        "Encoding attribute type failed.");
    }

    for ( ; pAttr != NULL; pAttr = pAttr->next)
    {
        // By this time, we have already filtered out attributes that should be send back to replication consumer
        // in prior WriteAttributes -> IsAttrInReplScope call.  They contain proper pAttr->metaData value.
        if (pAttr->metaData[0] != '\0')
        {
            VmDirStringPrintFA( attrMetaDataVal, sizeof(attrMetaDataVal), "%.256s:%s", pAttr->type.lberbv.bv_val, pAttr->metaData);
            berVal.lberbv.bv_val = attrMetaDataVal;
            berVal.lberbv.bv_len = VmDirStringLenA( attrMetaDataVal );
            if (VmDirStringCompareA( pAttr->type.lberbv.bv_val, ATTR_MODIFYTIMESTAMP, FALSE ) != 0 &&
                VmDirStringCompareA( pAttr->type.lberbv.bv_val, ATTR_MODIFIERS_NAME, FALSE ) != 0  &&
                VmDirStringCompareA( pAttr->type.lberbv.bv_val, ATTR_USN_CHANGED, FALSE ) != 0     &&
                VmDirStringCompareA( pAttr->type.lberbv.bv_val, ATTR_OBJECT_GUID, FALSE ) != 0)
            {
                // To prevent endless replication ping pong, supplier should send result only if there are changes
                // to attribute other than ATTR_USN_CHANGED, ATTR_MODIFYTIMESTAMP,ATTR_MODIFIERS_NAME and ATTR_OBJECT_GUID.
                *nonTrivialAttrsInReplScope = TRUE;
            }
            if (ber_printf( ber, "O", &berVal ) == -1 )
            {
                VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "WriteMetaDataAttribute: ber_printf (to print an attribute value ...) failed." );
                retVal = LDAP_OTHER;
                BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                                "Encoding an attribute value failed.");
            }
        }
    }
    // include attrMetaData for the deleted attributes
    for (i = 0; i<numAttrMetaData; i++)
    {
        if (pAttrMetaData[i].metaData[0] != '\0')
        {
            BOOLEAN                 bSendAttrMetaData = TRUE;
            PVDIR_SCHEMA_AT_DESC    pATDesc = NULL;

            if (pOp->syncReqCtrl != NULL) // Replication
            {
                retVal = IsAttrInReplScope( pOp, NULL, pAttrMetaData[i].metaData, 0, &bSendAttrMetaData, &pszLocalErrorMsg );
                BAIL_ON_VMDIR_ERROR( retVal );
            }
            else
            {
                bSendAttrMetaData = TRUE;
            }
            if (bSendAttrMetaData)
            {
                if ((pATDesc = VmDirSchemaAttrIdToDesc(pOp->pSchemaCtx, pAttrMetaData[i].attrID)) == NULL)
                {
                    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL,
                              "WriteMetaDataAttribute: VmDirSchemaAttrIdToDesc failed for attribute id: %d.",
                              pAttrMetaData[i].attrID  );
                    retVal = LDAP_OTHER;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                                    "WriteMetaDataAttribute: VmDirSchemaAttrIdToDesc failed.");
                }
                VmDirStringPrintFA( attrMetaDataVal, sizeof(attrMetaDataVal), "%.256s:%s", pATDesc->pszName, pAttrMetaData[i].metaData);
                berVal.lberbv.bv_val = attrMetaDataVal;
                berVal.lberbv.bv_len = VmDirStringLenA( attrMetaDataVal );
                *nonTrivialAttrsInReplScope = TRUE;
                if (ber_printf( ber, "O", &berVal ) == -1 )
                {
                    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "WriteMetaDataAttribute: ber_printf (to print an attribute value ...) failed." );
                    retVal = LDAP_OTHER;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                                    "Encoding an attribute value failed.");
                }
            }
        }
    }

    if ( ber_printf( ber, "]N}" ) == -1 )
    {
        VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "WriteMetaDataAttribute: ber_printf (to terminate an attribute's values ...) failed" );
        retVal = LDAP_OTHER;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                        "Encoding terminating an attribute type failed.");
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

    return( retVal );

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "WriteMetaDataAttribute failed (%u)(%s)",
                     retVal, VDIR_SAFE_STRING(pszLocalErrorMsg) );
    goto cleanup;
}

/*
 * Write Attribute Value Meta Data.
 */
static
int
_WriteValueMetaDataAttribute(
   PVDIR_OPERATION  pOp,
   PDEQUE           pValueMetaData,
   BerElement *     pBer,
   PSTR*            ppszErrorMsg
   )
{
    int retVal = LDAP_SUCCESS;
    VDIR_BERVALUE   attrValueMetaData = { {ATTR_ATTR_VALUE_META_DATA_LEN, ATTR_ATTR_VALUE_META_DATA}, 0, 0, NULL };
    VDIR_BERVALUE *pAVmeta = NULL;
    PSTR pszLocalErrorMsg = NULL;

    if (ber_printf( pBer, "{O[", &(attrValueMetaData) ) == -1 )
    {
        VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "_WriteValueMetaDataAttribute: ber_printf (to print attribute name ...) failed" );
        retVal = LDAP_OTHER;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                        "Encoding attribute type failed.");
    }

    while (!dequeIsEmpty(pValueMetaData))
    {
        VmDirFreeBerval(pAVmeta);
        pAVmeta = NULL;

        dequePopLeft(pValueMetaData, (PVOID*)&pAVmeta);
        if (ber_printf( pBer, "O", pAVmeta ) == -1 )
        {
            VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "_WriteValueMetaDataAttribute: ber_printf (to print an attribute value ...) failed." );
            retVal = LDAP_OTHER;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(retVal, (pszLocalErrorMsg), "Encoding an attribute value failed.");
        }
    }

    if ( ber_printf( pBer, "]N}" ) == -1 )
    {
        VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "_WriteValueMetaDataAttribute: ber_printf (to terminate an attribute's values ...) failed" );
        retVal = LDAP_OTHER;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                        "Encoding terminating an attribute type failed.");
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
    VmDirFreeBerval(pAVmeta);
    return retVal;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "_WriteValueMetaDataAttribute failed (%u)(%s)",
                     retVal, VDIR_SAFE_STRING(pszLocalErrorMsg) );
    goto cleanup;
}

static
int
PrepareValueMetaDataAttribute(
   PVDIR_OPERATION  pOp,
   PDEQUE           pAllValueMetaData,
   PDEQUE           pValueMetaDataToSend
   )
{
    int retVal = LDAP_SUCCESS;
    VDIR_BERVALUE *pAVmeta = NULL;
    char origInvocationId[VMDIR_GUID_STR_LEN] = {0};
    USN  origUsn = 0;
    char *p1 = NULL, *p2 = NULL;

    if (dequeIsEmpty(pAllValueMetaData))
    {
        goto cleanup;
    }

    while (!dequeIsEmpty(pAllValueMetaData))
    {
        if (pAVmeta)
        {
            VmDirFreeBerval(pAVmeta);
            pAVmeta = NULL;
        }
        dequePopLeft(pAllValueMetaData, (PVOID*)&pAVmeta);
        if (pOp->syncReqCtrl)
        {
            BOOLEAN usnInScope = FALSE;

            // Format is: <attr-name>:<local-usn>:<version-no>:<originating-server-id>:<value-change-originating-server-id>
            //            :<value-change-originating time>:<value-change-originating-usn>:<opcode>:<value-size>:<value>
            p1 = VmDirStringChrA(VmDirStringChrA(VmDirStringChrA(VmDirStringChrA(pAVmeta->lberbv.bv_val, ':' ) + 1, ':') + 1, ':') + 1, ':') + 1;
            //p1 points to <value-change-originating-server-id>

            retVal = VmDirStringNCpyA(origInvocationId, VMDIR_GUID_STR_LEN, p1, VMDIR_GUID_STR_LEN - 1);
            BAIL_ON_VMDIR_ERROR(retVal);

            origInvocationId[VMDIR_GUID_STR_LEN - 1] = '\0';

            p2 = VmDirStringChrA(VmDirStringChrA(p1, ':' ) + 1, ':') + 1;
            p1 = VmDirStringChrA(p2, ':');
            *p1 = '\0';
            // p2 now points to <value-change-originating-usn>
            origUsn = VmDirStringToLA(p2, NULL, 10 );
            *p1 = ':';

            if (VmDirStringCompareA(origInvocationId, pOp->syncReqCtrl->value.syncReqCtrlVal.reqInvocationId.lberbv.bv_val,TRUE ) == 0)
            {
                continue;
                //Change is originated from the requesting server. Don't send it.
            }
            retVal = _VmDirIsUsnInScope(pOp, NULL, origInvocationId, origUsn, 0, &usnInScope);
            BAIL_ON_VMDIR_ERROR(retVal);
            if (!usnInScope)
            {
                continue;
            }
            VMDIR_LOG_INFO( LDAP_DEBUG_REPL, "PrepareValueMetaDataAttribute: valueMata %s, usnInScope true",
                  VDIR_SAFE_STRING(pAVmeta->lberbv.bv_val));
        }
        retVal = dequePush(pValueMetaDataToSend, pAVmeta);
        BAIL_ON_VMDIR_ERROR( retVal );
        pAVmeta = NULL;
    }

cleanup:
    VmDirFreeBerval(pAVmeta);
    return retVal;

error:
    goto cleanup;
}

static
int
WriteBerOnSocket(
   VDIR_CONNECTION * conn,
   BerElement * ber )
{
   long         retVal = 0;
   uint64_t     iWriteStartTimeInMSec = 0;

   iWriteStartTimeInMSec = VmDirGetTimeInMilliSec();
   while( VmDirGetTimeInMilliSec() - iWriteStartTimeInMSec <
          (MAX_NUM_SECONDS_OF_SOCK_WRITE_RETRIES * MSECS_IN_SECOND) )
   {
      if ( (retVal = ber_flush2( conn->sb, ber, LBER_FLUSH_FREE_NEVER )) == 0 )
      {
         break;
      }

#ifdef _WIN32
      // in ber_flush2 (liblber) call, sock_errset() call WSASetLastError()
      errno = WSAGetLastError();
#endif

      if ( errno == EWOULDBLOCK || errno == EAGAIN )
      {
         continue;
      }
   }

   if (retVal != 0)
   {
      VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "WriteBerOnSocket:  ber_flush2() call failed with errno = %d.", errno );
   }

   return retVal;
}

static
ber_tag_t
GetResultTag(
   ber_tag_t tag )
{
   VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "GetResultTag: Begin" );

   switch( tag )
   {
      case LDAP_REQ_ADD:
         tag = LDAP_RES_ADD;
         break;
      case LDAP_REQ_BIND:
         tag = LDAP_RES_BIND;
         break;
      case LDAP_REQ_COMPARE:
         tag = LDAP_RES_COMPARE;
         break;
      case LDAP_REQ_EXTENDED:
         tag = LDAP_RES_EXTENDED;
         break;
      case LDAP_REQ_MODIFY:
         tag = LDAP_RES_MODIFY;
         break;
      case LDAP_REQ_MODRDN:
         tag = LDAP_RES_MODRDN;
         break;
      case LDAP_REQ_DELETE:
         tag = LDAP_RES_DELETE;
         break;
      case LDAP_REQ_SEARCH:
         tag = LDAP_RES_SEARCH_RESULT;
         break;
      case LDAP_REQ_ABANDON:
      case LDAP_REQ_UNBIND:
      default:
         tag = LBER_SEQUENCE;
         break;
   }

   VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "GetResultTag: End" );
   return tag;
}

static
VOID
SetSpecialReturnChar(
    SearchReq*      pSearchReq,
    uint32_t*       pSearchReqSpecialChars
    )
{
    int     iCnt = 0;

    assert(pSearchReq && pSearchReqSpecialChars);

    *pSearchReqSpecialChars = 0;

    if (pSearchReq->attrs == NULL)
    {   // if no attribute requested, default to return user attributes
        *pSearchReqSpecialChars |= LDAP_SEARCH_REQUEST_CHAR_USER;
    }

    // see if "*" and/or "+" as part of the request attribute list.
    for (iCnt = 0; pSearchReq->attrs && pSearchReq->attrs[iCnt].lberbv.bv_val != NULL; iCnt++)
    {
        if (VmDirStringCompareA( pSearchReq->attrs[iCnt].lberbv.bv_val, "*", FALSE) == 0)
        {
            *pSearchReqSpecialChars |= LDAP_SEARCH_REQUEST_CHAR_USER;
        }
        else if (VmDirStringCompareA( pSearchReq->attrs[iCnt].lberbv.bv_val, "+", FALSE) == 0)
        {
            *pSearchReqSpecialChars |= LDAP_SEARCH_REQUEST_CHAR_OP;
        }
        else if (VmDirStringCompareA( pSearchReq->attrs[iCnt].lberbv.bv_val, "-", FALSE) == 0)
        {
            *pSearchReqSpecialChars |= LDAP_SEARCH_REQUEST_CHAR_PASSWD;
        }
    }
}
