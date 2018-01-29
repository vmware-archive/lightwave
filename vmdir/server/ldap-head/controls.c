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

static int
ParseSyncRequestControlVal(
    VDIR_OPERATION *            op,
    BerValue *                  controlValue,       // Input: control value encoded as ber
    SyncRequestControlValue *   syncReqCtrlVal,     // Output
    VDIR_LDAP_RESULT *          lr                  // Output
    );

static int
_ParsePagedResultControlVal(
    VDIR_OPERATION *                    op,
    BerValue *                          controlValue,       // Input: control value encoded as ber
    VDIR_PAGED_RESULT_CONTROL_VALUE*    pageResultCtrlVal,     // Output
    VDIR_LDAP_RESULT *                  lr                  // Output
    );

static
int
_ParseSyncStateControlVal(
    BerValue *  controlValue,   // Input: control value encoded as ber,
    int *       entryState,     // Output
    USN*        pPartnerUSN     // Output
    );

static
int
_ParseDigestControlVal(
    VDIR_OPERATION *                op,
    BerValue *                      controlValue,   // Input: control value encoded as ber
    VDIR_DIGEST_CONTROL_VALUE *     digestCtrlVal,  // Output
    VDIR_LDAP_RESULT *              lr              // Output
    );

static
int
_ParseRaftPingControlVal(
   VDIR_OPERATION *                 pOp,
   BerValue *                       pControlBer,    // Input: control value encoded as ber
   PVDIR_RAFT_PING_CONTROL_VALUE    pCtrlVal,       // Output
   VDIR_LDAP_RESULT *               pLdapResult     // Output
   );

static
int
_ParseRaftVoteControlVal(
    VDIR_OPERATION *                pOp,
    BerValue *                      pControlBer,    // Input: control value encoded as ber
    PVDIR_RAFT_VOTE_CONTROL_VALUE   pCtrlVal,       // Output
    VDIR_LDAP_RESULT *              pLdapResult     // Output
    );

static
int
_ParseStatePingControlVal(
    VDIR_OPERATION*                 op,
    BerValue*                       pControlValue,  // Input: control value encoded as ber
    VDIR_STATE_PING_CONTROL_VALUE*  pPingCtrlVal,   // Output
    VDIR_LDAP_RESULT*               lr              // Output
    );

/*
 * RFC 4511:
 * Section 4.1.1 Message Envelope:
 *
 *    LDAPMessage ::= SEQUENCE {
 *       messageID   MessageID,
 *       protocolOp   CHOICE {
 *          bindRequest BindRequest,
 *          bindResponse BindResponse,
 *          ... },
 *       controls [0] Controls OPTIONAL }
 *
 * Section 4.1.11 Controls:
 *
 *    Controls ::= SEQUENCE OF control Control
 *    Control ::= SEQUENCE {
 *       controlType LDAPOID,
 *       criticality BOOLEAN DEFAULT FALSE,
 *       controlValue OCTET STRING OPTIONAL }
 *
 */

int
ParseRequestControls(
   VDIR_OPERATION *    op,
   VDIR_LDAP_RESULT *  lr
   )
{
    int                     retVal = LDAP_SUCCESS;
    ber_tag_t               tag = LBER_ERROR;
    ber_len_t               len = 0;
    char *                  endOfCtrlsMarker = NULL;
    VDIR_LDAP_CONTROL **    control = &(op->reqControls);
    BerValue                lberBervType = {0};
    BerValue                lberBervCtlValue = {0};
    PSTR                    pszLocalErrorMsg = NULL;

    *control = NULL;

#ifndef _WIN32
    if (ber_pvt_ber_remaining(op->ber) != 0)
#else
    if (LBER_DEFAULT != ber_peek_tag(op->ber, &len))
#endif
    {
        if (ber_peek_tag(op->ber, &len) != LDAP_TAG_CONTROLS)
        {
            lr->errCode = LDAP_PROTOCOL_ERROR;
            retVal = LDAP_NOTICE_OF_DISCONNECT;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(
                    retVal,
                    pszLocalErrorMsg,
                    "ParseRequestControls: Request controls expected, but something else is there in the PDU.");
        }

        // Get controls. ber_first_element => skip the sequence header, set the cursor at the 1st control in the SEQ of SEQ
        for(tag = ber_first_element(op->ber, &len, &endOfCtrlsMarker); tag != LBER_ERROR;
            tag = ber_next_element(op->ber, &len, endOfCtrlsMarker))
        {
            // m => in-place
            if (ber_scanf(op->ber, "{m", &lberBervType) == LBER_ERROR)
            {
                lr->errCode = LDAP_PROTOCOL_ERROR;
                retVal = LDAP_NOTICE_OF_DISCONNECT;
                BAIL_ON_VMDIR_ERROR_WITH_MSG(
                        retVal,
                        pszLocalErrorMsg,
                        "ParseRequestControls Error in reading control type from the PDU.");
            }

            VMDIR_LOG_INFO(
                    LDAP_DEBUG_ARGS,
                    "    Request Control: %s",
                    lberBervType.bv_val);

            if (VmDirAllocateMemory(sizeof(VDIR_LDAP_CONTROL), (PVOID*)control) != 0)
            {
                retVal = lr->errCode = LDAP_OPERATIONS_ERROR;
                BAIL_ON_VMDIR_ERROR_WITH_MSG(
                        retVal,
                        pszLocalErrorMsg,
                        "ParseRequestControls: VmDirAllocateMemory failed");
            }

            // type points into in-place ber and does NOT own its content
            (*control)->type = lberBervType.bv_val;
            (*control)->criticality = FALSE;
            tag = ber_peek_tag(op->ber, &len);

            if (tag == LBER_BOOLEAN)
            {
                ber_int_t criticality;
                if (ber_scanf(op->ber, "b", &criticality) == LBER_ERROR)
                {
                    lr->errCode = LDAP_PROTOCOL_ERROR;
                    retVal = LDAP_NOTICE_OF_DISCONNECT;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG(
                            retVal,
                            pszLocalErrorMsg,
                            "ParseRequestControls: Error in reading control criticality from the PDU.");
                }
                if (criticality)
                {
                    (*control)->criticality = TRUE;
                }
                tag = ber_peek_tag(op->ber, &len);
            }

            if (tag == LBER_OCTETSTRING)
            {
                if (ber_scanf(op->ber, "m", &lberBervCtlValue) == LBER_ERROR)
                {
                    lr->errCode = LDAP_PROTOCOL_ERROR;
                    retVal = LDAP_NOTICE_OF_DISCONNECT;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG(
                            retVal,
                            pszLocalErrorMsg,
                            "ParseRequestControls: ber_scanf failed while parsing the control value.");
                }
            }

            // SJ-TBD: Make sure that the control appears only once in the request, and it is present only in a search
            // request
            if (VmDirStringCompareA((*control)->type, LDAP_CONTROL_SYNC, TRUE) == 0)
            {
                retVal = ParseSyncRequestControlVal(
                        op, &lberBervCtlValue, &((*control)->value.syncReqCtrlVal), lr);

                BAIL_ON_VMDIR_ERROR_WITH_MSG(
                        retVal,
                        pszLocalErrorMsg,
                        "ParseRequestControls: ParseSyncRequestControlVal failed.");

                op->syncReqCtrl = *control;
            }

            if (VmDirStringCompareA((*control)->type, VDIR_LDAP_CONTROL_SHOW_DELETED_OBJECTS, TRUE) == 0)
            {
                op->showDeletedObjectsCtrl = *control;
            }

            if (VmDirStringCompareA((*control)->type, VDIR_LDAP_CONTROL_SHOW_MASTER_KEY, TRUE) == 0)
            {
                op->showMasterKeyCtrl = *control;
            }

            if (VmDirStringCompareA((*control)->type, LDAP_CONTROL_PAGEDRESULTS, TRUE) == 0)
            {
                retVal = _ParsePagedResultControlVal(
                        op, &lberBervCtlValue, &((*control)->value.pagedResultCtrlVal), lr);

                BAIL_ON_VMDIR_ERROR_WITH_MSG(
                        retVal,
                        pszLocalErrorMsg,
                        "ParseRequestControls: _ParsePagedResultControlVal failed.");

                op->showPagedResultsCtrl = *control;
            }

            if (VmDirStringCompareA((*control)->type, LDAP_CONTROL_DIGEST_SEARCH, TRUE) == 0)
            {
                retVal = _ParseDigestControlVal(
                        op, &lberBervCtlValue, &((*control)->value.digestCtrlVal), lr);

                BAIL_ON_VMDIR_ERROR_WITH_MSG(
                        retVal,
                        pszLocalErrorMsg,
                        "ParseRequestControls: _ParseDigestControlVal failed.");

                op->digestCtrl = *control;
            }

            if (VmDirStringCompareA((*control)->type, LDAP_RAFT_PING_CONTROL, TRUE) == 0)
            {
                retVal = _ParseRaftPingControlVal(
                        op, &lberBervCtlValue, &((*control)->value.raftPingCtrlVal), lr);

                BAIL_ON_VMDIR_ERROR_WITH_MSG(
                        retVal,
                        pszLocalErrorMsg,
                        "ParseRequestControls: _ParseRaftPingControlVal failed.");

                op->raftPingCtrl = *control;
            }

            if (VmDirStringCompareA((*control)->type, LDAP_RAFT_VOTE_CONTROL, TRUE) == 0)
            {
                retVal = _ParseRaftVoteControlVal(
                        op, &lberBervCtlValue, &((*control)->value.raftVoteCtrlVal), lr);

                BAIL_ON_VMDIR_ERROR_WITH_MSG(
                        retVal,
                        pszLocalErrorMsg,
                        "ParseRequestControls: _ParseRaftVoteControlVal failed.");

                op->raftVoteCtrl = *control;
            }

            if (VmDirStringCompareA((*control)->type, LDAP_STATE_PING_CONTROL, TRUE) == 0)
            {
                retVal = _ParseStatePingControlVal(
                        op, &lberBervCtlValue, &((*control)->value.statePingCtrlVal), lr);

                BAIL_ON_VMDIR_ERROR_WITH_MSG(
                        retVal,
                        pszLocalErrorMsg,
                        "ParseRequestControls: _ParseStatePingControlVal failed.");

                op->statePingCtrl = *control;
            }

            if (ber_scanf( op->ber, "}") == LBER_ERROR) // end of control
            {
                lr->errCode = LDAP_PROTOCOL_ERROR;
                retVal = LDAP_NOTICE_OF_DISCONNECT;
                BAIL_ON_VMDIR_ERROR_WITH_MSG(
                        retVal,
                        pszLocalErrorMsg,
                        "ParseRequestControls: ber_scanf failed while parsing the end of control");

            }
            control = &((*control)->next);
        }

        retVal = LDAP_SUCCESS;
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return retVal;

error:
    DeleteControls(&(op->reqControls));
    if (pszLocalErrorMsg)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                pszLocalErrorMsg);

        VMDIR_APPEND_ERROR_MSG(lr->pszErrMsg, pszLocalErrorMsg);
    }
    goto cleanup;
}

void
DeleteControls(
   VDIR_LDAP_CONTROL ** controls
   )
{
   VDIR_LDAP_CONTROL * curr = NULL;
   VDIR_LDAP_CONTROL * next = NULL;

   VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "DeleteControls: Begin." );

   if (controls != NULL)
   {
      for (curr = *controls; curr != NULL; curr = next)
      {
         next = curr->next;
         VMDIR_SAFE_FREE_MEMORY( curr );
      }
      *controls = NULL;
   }

   VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "DeleteControls: End." );
}

VOID
VmDirSetSyncDoneCtlbContinue(
    PVDIR_OPERATION pOp,
    DWORD           dwSentEntryCount
    )
{
    if (pOp &&
        pOp->syncDoneCtrl &&
        pOp->request.searchReq.sizeLimit > 0 &&
        pOp->request.searchReq.sizeLimit == dwSentEntryCount)
    {   // replication pull full page request sent and there could be more changes pending
        // ask consumer to come back.
        pOp->syncDoneCtrl->value.syncDoneCtrlVal.bContinue = TRUE;
    }
}

int
WriteSyncDoneControl(
    VDIR_OPERATION *     op,
    BerElement *    ber
    )
{
    int                     retVal = LDAP_OPERATIONS_ERROR;
    PLW_HASHTABLE_NODE      pNode = NULL;
    LW_HASHTABLE_ITER       iter = LW_HASHTABLE_ITER_INIT;
    UptoDateVectorEntry *   pUtdVectorEntry = NULL;
    VDIR_BERVALUE           bvCtrlVal = VDIR_BERVALUE_INIT;
    VDIR_BERVALUE           syncDoneCtrlType = {
                                {VmDirStringLenA( LDAP_CONTROL_SYNC_DONE ), LDAP_CONTROL_SYNC_DONE}, 0, 0, NULL };

    if ( op->syncDoneCtrl != NULL)
    {
        if (ber_printf( ber, "t{{O", LDAP_TAG_CONTROLS, &(syncDoneCtrlType.lberbv) ) == -1 )
        {
            VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "WriteSyncDoneControl: ber_printf (to print Sync Done Control ...) failed" );
            retVal = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR( retVal );
        }
        { // Construct string format of utdVector
            int     numEntries = LwRtlHashTableGetCount( op->syncDoneCtrl->value.syncDoneCtrlVal.htUtdVector );
            char *  writer = NULL;
            size_t  tmpLen = 0;
            size_t bufferSize = (numEntries + 1 /* for lastLocalUsn */) *
                                (VMDIR_GUID_STR_LEN + 1 + VMDIR_MAX_USN_STR_LEN + 1) +
                                (VMDIR_REPL_CONT_INDICATOR_LEN + 1);

            // Sync Done control value looks like: <lastLocalUsnChanged>,<serverId1>:<server 1 last originating USN>,
            // <serverId2>,<server 2 originating USN>,...,[continue:1,]

            if (VmDirAllocateMemory( bufferSize, (PVOID *)&bvCtrlVal.lberbv.bv_val) != 0)
            {
                retVal = LDAP_OPERATIONS_ERROR;
                BAIL_ON_VMDIR_ERROR( retVal );
            }
            writer = bvCtrlVal.lberbv.bv_val;
            bvCtrlVal.lberbv.bv_len = 0;

            VmDirStringPrintFA( writer, bufferSize, "%" PRId64 ",", op->syncDoneCtrl->value.syncDoneCtrlVal.intLastLocalUsnProcessed );
            tmpLen = VmDirStringLenA( writer );
            writer += tmpLen;
            bufferSize -= tmpLen;
            bvCtrlVal.lberbv.bv_len += tmpLen;

            while ((pNode = LwRtlHashTableIterate(op->syncDoneCtrl->value.syncDoneCtrlVal.htUtdVector, &iter)))
            {
                pUtdVectorEntry = LW_STRUCT_FROM_FIELD(pNode, UptoDateVectorEntry, Node);
                VmDirStringPrintFA( writer, bufferSize, "%s:%" PRId64 ",", pUtdVectorEntry->invocationId.lberbv.bv_val,
                                    pUtdVectorEntry->currMaxOrigUsnProcessed );
                tmpLen = VmDirStringLenA( writer );
                writer += tmpLen;
                bufferSize -= tmpLen;
                bvCtrlVal.lberbv.bv_len += tmpLen;
            }

            if (op->syncDoneCtrl->value.syncDoneCtrlVal.bContinue)
            {
                VmDirStringPrintFA( writer, bufferSize, VMDIR_REPL_CONT_INDICATOR );
                tmpLen = VmDirStringLenA( writer );
                writer += tmpLen;
                bufferSize -= tmpLen;
                bvCtrlVal.lberbv.bv_len += tmpLen;
            }
        }

        if (ber_printf( ber, "O}}", &bvCtrlVal.lberbv) == -1 )
        {
            VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "WriteSyncDoneControl: ber_printf (to print Sync Done Control ...) failed" );
            retVal = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR( retVal );
        }

        VMDIR_LOG_DEBUG( LDAP_DEBUG_REPL, "WriteSyncDoneControl: Sync Done Control Value: %s", bvCtrlVal.lberbv.bv_val );
    }

    retVal = LDAP_SUCCESS;

cleanup:
    VMDIR_SAFE_FREE_MEMORY( bvCtrlVal.lberbv.bv_val );

    return retVal;

error:
    goto cleanup;
}

int
WritePagedSearchDoneControl(
    VDIR_OPERATION *     op,
    BerElement *    ber
    )
{
    int                 retVal = LDAP_OPERATIONS_ERROR;
    BerElementBuffer    ctrlValBerbuf;
    BerElement *        ctrlValBer = (BerElement *) &ctrlValBerbuf;
    VDIR_BERVALUE       bvCtrlVal = VDIR_BERVALUE_INIT;

    if (!op || !op->showPagedResultsCtrl)
    {
        retVal = LDAP_PROTOCOL_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    if ( op->showPagedResultsCtrl)
    {
        VDIR_PAGED_RESULT_CONTROL_VALUE*    prCtrlVal = &op->showPagedResultsCtrl->value.pagedResultCtrlVal;
        VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE,
                "WritePagedSearchDoneControl: Paged Search Done Control Value: pageSize[%d] cookie[%s]",
                prCtrlVal->pageSize,
                prCtrlVal->cookie );

        (void) memset( (char *)&ctrlValBerbuf, '\0', sizeof( BerElementBuffer ));
        ber_init2( ctrlValBer, NULL, LBER_USE_DER );

        if (ber_printf( ctrlValBer, "{is}", 0, prCtrlVal->cookie) == -1 )
        {
            VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "SendLdapResult: ber_printf (to print Paged Search Done Control ...) failed" );
            retVal = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR( retVal );
        }

        bvCtrlVal.lberbv.bv_val = ctrlValBer->ber_buf;
        bvCtrlVal.lberbv.bv_len = ctrlValBer->ber_ptr - ctrlValBer->ber_buf;

        if (ber_printf(ber, "t{{sO}}", LDAP_TAG_CONTROLS, LDAP_CONTROL_PAGEDRESULTS, &bvCtrlVal.lberbv ) == -1)
        {
            VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "WritePagedSearchDoneControl: ber_printf (to print Search Done Control ...) failed" );
            retVal = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR( retVal );
        }
    }

    retVal = LDAP_SUCCESS;

cleanup:
    ber_free_buf( ctrlValBer );
    return retVal;

error:
    goto cleanup;
}

/* On the wire, write the Sync State Control associated with a search result entry. Sync State Control is part of the
 * LDAP Content Synchronization protocol.
 */

int
WriteSyncStateControl(
   VDIR_OPERATION *   op,
   VDIR_ENTRY *       pEntry,
   BerElement *       ber,
   PSTR*              ppszErrorMsg
   )
{
    int                 retVal = LDAP_SUCCESS;
    VDIR_BERVALUE       syncStateCtrlType = {
                            {VmDirStringLenA( LDAP_CONTROL_SYNC_STATE ), LDAP_CONTROL_SYNC_STATE}, 0, 0, NULL };
    int                 entryState = LDAP_SYNC_MODIFY;
    BerElementBuffer    ctrlValBerbuf;
    BerElement *        ctrlValBer = (BerElement *) &ctrlValBerbuf;
    VDIR_BERVALUE       bvCtrlVal = VDIR_BERVALUE_INIT;
    PVDIR_BERVALUE      pbvUSN = NULL;
    PSTR                pszLocalErrorMsg = NULL;
    BOOLEAN             bHasFinalSyncState = FALSE;
    BOOLEAN             bPresentInSyncStateOneMap = FALSE;
    CHAR                pszIDBuf[VMDIR_MAX_I64_ASCII_STR_LEN] = {0};
    PSTR                pszEID = NULL;
    PSTR                pszUSNCreated = NULL;
    VDIR_ATTRIBUTE *    pAttr = NULL;

    VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "WriteSyncStateControl: Begin" );

    /* Logic to determine the entry state is:
     *  Default entry state is LDAP_SYNC_MODIFY
     *
     *  If ATTR_USN_CREATED value is greater than the last local USN that the consumer has seen so far,
     *  entry state is LDAP_SYNC_ADD.
     *  Otherwise, if isDeleted=TRUE is in the replication scope, entry state is LDAP_SYNC_DELETE.
     *
     *  NOTE: The logic is based on the things (entry state + what has the consumer seen so far) seen by the supplier
     *  at this point in time. Things/state could be different at the time when consumer tries to apply this change.
     *  => may lead to various conflict resolution scenarios.
     */

    retVal = VmDirStringNPrintFA(pszIDBuf, VMDIR_MAX_I64_ASCII_STR_LEN, VMDIR_MAX_I64_ASCII_STR_LEN, "%llu", pEntry->eId);
    BAIL_ON_VMDIR_ERROR(retVal);

    bPresentInSyncStateOneMap = (LwRtlHashMapFindKey(op->conn->ReplConnState.phmSyncStateOneMap, NULL, pszIDBuf) == 0);

    for (pAttr = pEntry->attrs ; pAttr != NULL; pAttr = pAttr->next)
    {
        if (VmDirStringCompareA( pAttr->type.lberbv.bv_val, ATTR_USN_CHANGED, FALSE ) == 0)
        {
            pbvUSN = pAttr->vals;
        }

        // We are sending back this attribute's meta data. => Attribute is in
        // replication scope. SJ-TBD: we are overloading pAttr->metaData
        // field. Should we have another field (e.g. inReplScope) in Attribute
        if (pAttr->metaData[0] != '\0' && !bHasFinalSyncState)
        {
            if ( VmDirStringCompareA( pAttr->type.lberbv.bv_val, ATTR_IS_DELETED, FALSE ) == 0 &&
                 VmDirStringCompareA( pAttr->vals[0].lberbv.bv_val, VMDIR_IS_DELETED_TRUE_STR, FALSE ) == 0)
            {
                entryState = LDAP_SYNC_DELETE;
                /*
                 * If corresponding entry is present in the hashmap then consumer has seen this entry
                 * Send Sync State as Delete.
                 * If consumer has not seen this entry, send sync state add of tombstone entry directly
                 */
                if (bPresentInSyncStateOneMap)
                {
                    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: Avoiding add->delete race condition by sending sync state as delete", __FUNCTION__);
                }
                bHasFinalSyncState = bPresentInSyncStateOneMap;
                continue; // Look if it is LDAP_SYNC_ADD case
            }

            if (VmDirStringCompareA( pAttr->type.lberbv.bv_val, ATTR_USN_CREATED, FALSE ) == 0)
            {
                entryState = LDAP_SYNC_ADD;
                bHasFinalSyncState = TRUE;

                if (bPresentInSyncStateOneMap == FALSE)
                {
                    retVal = VmDirAllocateStringA(pszIDBuf, &pszEID);
                    BAIL_ON_VMDIR_ERROR(retVal);

                    retVal = VmDirAllocateStringA(pAttr->vals[0].lberbv_val, &pszUSNCreated);
                    BAIL_ON_VMDIR_ERROR(retVal);

                    retVal = LwRtlHashMapInsert(op->conn->ReplConnState.phmSyncStateOneMap, pszEID, pszUSNCreated, NULL);
                    BAIL_ON_VMDIR_ERROR(retVal);
                    pszEID = NULL; pszUSNCreated = NULL; // map takes over

                    VMDIR_LOG_VERBOSE(
                            LDAP_DEBUG_REPL,
                            "entry sync stat ADD %s at USNCreated %s",
                            pEntry->dn.lberbv_val,
                            pAttr->vals[0].lberbv_val);
                }
            }
        }
    }

    // we always send uSNChanged attribute
    assert(pbvUSN);

    if (ber_printf( ber, "t{{O", LDAP_TAG_CONTROLS, &(syncStateCtrlType.lberbv) ) == -1 )
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "WriteSyncStateControl: ber_printf (to print attribute name ...) failed" );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                        "Encoding sync state control failed.");
    }

    ////////// Build ber for the control value

    (void) memset( (char *)&ctrlValBerbuf, '\0', sizeof( BerElementBuffer ));
    ber_init2( ctrlValBer, NULL, LBER_USE_DER );

    if ( (VDIR_WRITE_OP_AUDIT_ENABLED &&
          (ber_printf( ctrlValBer, "{iO}", entryState, pbvUSN ) == -1 ))
        ||
         (ber_printf( ctrlValBer, "{i}", entryState) == -1)
       )
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "SendLdapResult: ber_printf (to print Sync Done Control ...) failed" );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                        "Encoding sync state control failed.");
    }

    ////////////// Building ber for the control value done

    bvCtrlVal.lberbv.bv_val = ctrlValBer->ber_buf;
    bvCtrlVal.lberbv.bv_len = ctrlValBer->ber_ptr - ctrlValBer->ber_buf;

    if (ber_printf( ber, "O", &bvCtrlVal.lberbv ) == -1 )
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "SendLdapResult: ber_printf (to print Sync Done Control ...) failed" );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                        "Encoding sync state control failed.");
    }

    if (ber_printf( ber, "}}" ) == -1 )
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "SendLdapResult: ber_printf (to print Sync Done Control ...) failed" );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(retVal, (pszLocalErrorMsg),
                                     "Encoding sync state control failed.");
    }

cleanup:
    ber_free_buf( ctrlValBer );

    if (ppszErrorMsg)
    {
        *ppszErrorMsg = pszLocalErrorMsg;
    }
    else
    {
        VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    }

    VMDIR_SAFE_FREE_MEMORY(pszEID);
    VMDIR_SAFE_FREE_MEMORY(pszUSNCreated);

    VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "WriteSyncStateControl: End" );

    return( retVal );

error:

    goto cleanup;
}

static
int
_ParseSyncStateControlVal(
    BerValue *  controlValue,   // Input: control value encoded as ber,
    int *       entryState,     // Output
    USN*        pPartnerUSN     // Output
    )
{
    int                 retVal = LDAP_SUCCESS;
    BerElementBuffer    berbuf;
    BerElement *        ber = (BerElement *)&berbuf;
    BerValue            bvPartnerUSN = {0};

    ber_init2( ber, controlValue, LBER_USE_DER );

    if  (VDIR_WRITE_OP_AUDIT_ENABLED)
    {
        if (ber_scanf( ber, "{im}", entryState, &bvPartnerUSN ) == -1 )
        {
            BAIL_WITH_VMDIR_ERROR(retVal, LDAP_OPERATIONS_ERROR);
        }

        if (pPartnerUSN && bvPartnerUSN.bv_len > 0)
        {
            *pPartnerUSN = atol(bvPartnerUSN.bv_val);
        }
    }
    else
    {
        if (ber_scanf( ber, "{i}", entryState ) == -1 )
        {
            BAIL_WITH_VMDIR_ERROR(retVal, LDAP_OPERATIONS_ERROR);
        }
    }


cleanup:

    return retVal;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: ber_scanf to read entryState failed", __FUNCTION__ );
    goto cleanup;
}

int
ParseAndFreeSyncStateControl(
    LDAPControl ***pCtrls,
    int*        piEntryState,
    USN*        pulPartnerUSN
    )
{
    int retVal = LDAP_SUCCESS;
    int entryState = -1;
    USN ulPartnerUSN = 0;

    if (pCtrls == NULL)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "ParseAndFreeSyncStateControl: pCtrls is NULL" );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
    }

    if ((*pCtrls)[0] == NULL)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "ParseAndFreeSyncStateControl: (*pCtrls)[0] is NULL" );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
    }

    if (VmDirStringCompareA((*pCtrls)[0]->ldctl_oid, LDAP_CONTROL_SYNC_STATE, TRUE) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "ParseAndFreeSyncStateControl: (*pCtrls)[0]->ldctrl_oid is not expected: %s", VDIR_SAFE_STRING((*pCtrls)[0]->ldctl_oid));
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
    }

    retVal = _ParseSyncStateControlVal(&(*pCtrls)[0]->ldctl_value, &entryState, &ulPartnerUSN);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    ldap_controls_free(*pCtrls);
    *pCtrls = NULL;

    *piEntryState = entryState;
    *pulPartnerUSN = ulPartnerUSN;

cleanup:
    return retVal;

ldaperror:
    goto cleanup;
}


static int
ParseSyncRequestControlVal(
    VDIR_OPERATION *            op,
    BerValue *                  controlValue,       // Input: control value encoded as ber
    SyncRequestControlValue *   syncReqCtrlVal,     // Output
    VDIR_LDAP_RESULT *          lr                  // Output
    )
{
    int                     retVal = LDAP_SUCCESS;
    ber_tag_t               tag = LBER_ERROR;
    ber_len_t               len = 0;
    UptoDateVectorEntry *   utdVectorEntry = NULL;
    BerElementBuffer        berbuf;
    BerElement *            ber = (BerElement *)&berbuf;
    PSTR                    pszLocalErrorMsg = NULL;
    VDIR_BACKEND_CTX        backendCtx = {0};
    USN                     maxPartnerVisibleUSN = 0;

    VMDIR_LOG_DEBUG(LDAP_DEBUG_TRACE, "ParseSyncRequestControlVal: Begin");

    ber_init2(ber, controlValue, LBER_USE_DER);

    /* http://www.rfc-editor.org/rfc/rfc4533.txt
     *
     *  syncCookie ::= OCTET STRING
     *
     *  syncRequestValue ::= SEQUENCE {
     *          mode ENUMERATED {
     *              -- 0 unused
     *              refreshOnly       (1),
     *              -- 2 reserved
     *              refreshAndPersist (3)
     *          },
     *          cookie     syncCookie OPTIONAL,
     *          reloadHint BOOLEAN DEFAULT FALSE
     *  }
    */

    if (ber_scanf(ber, "{i", &(syncReqCtrlVal->mode)) == LBER_ERROR)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "ParseSyncRequestControlVal: ber_scanf failed while parsing the sync request "
                "control mode");

        lr->errCode = LDAP_PROTOCOL_ERROR;
        retVal = LDAP_NOTICE_OF_DISCONNECT;

        BAIL_ON_VMDIR_ERROR_WITH_MSG(
                retVal,
                pszLocalErrorMsg,
                "Error in reading sync request control mode from PDU.");
    }

    syncReqCtrlVal->bvLastLocalUsnProcessed.lberbv.bv_val = "";
    syncReqCtrlVal->intLastLocalUsnProcessed = 0;

    if (VmDirAllocateMemory(sizeof(VDIR_LDAP_CONTROL), (PVOID*)&op->syncDoneCtrl) != 0)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "ParseSyncRequestControlVal: VmDirAllocateMemory failed");

        lr->errCode = retVal = LDAP_OPERATIONS_ERROR;

        BAIL_ON_VMDIR_ERROR_WITH_MSG(
                retVal,
                pszLocalErrorMsg,
                "ParseSyncRequestControlVal: VmDirAllocateMemory failed.");
    }
    op->syncDoneCtrl->type = LDAP_CONTROL_SYNC_DONE;
    if (LwRtlCreateHashTable(
            &op->syncDoneCtrl->value.syncDoneCtrlVal.htUtdVector,
            UtdVectorEntryGetKey,
            LwRtlHashDigestPstr,
            LwRtlHashEqualPstr,
            NULL,
            VMDIR_UTD_VECTOR_HASH_TABLE_SIZE) != 0)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "UpdateSyncDoneUtdVectorEntry: LwRtlCreateHashTable failed");

        lr->errCode = retVal = LDAP_OPERATIONS_ERROR;

        BAIL_ON_VMDIR_ERROR_WITH_MSG(
                retVal,
                pszLocalErrorMsg,
                "UpdateSyncDoneUtdVectorEntry: LwRtlCreateHashTable failed.");
    }

    tag = ber_peek_tag(ber, &len);

    if (tag == LBER_SEQUENCE)
    { // syncCookie

        /* syncCookie ::= SEQUENCE {
         *                      reqServerId             LDAPString,
         *                      lastLocalUsnProcessed   INTEGER (0 .. maxInt),
         *                      utdVector               UptoDateVectorEntryList }
         *
         *   UptoDateVectorEntryList ::= SEQUENCE OF uptoDateVectorEntry UptoDateVectorEntry
         *
         *   UptoDateVectorEntry ::= SEQUENCE {
         *                            serverId              LDAPString,
         *                            lastOrigUsnProcessed  INTEGER (0 .. maxInt) }
         */
        // {lastLocalUsnProcessed{{<serverid1><lastOrigUsnProcessed1>}{<serverid2><lastOrigUsnProcessed2>}...}}

        if (ber_scanf(
                ber,
                "{mmm}",
                &syncReqCtrlVal->reqInvocationId.lberbv,
                &syncReqCtrlVal->bvLastLocalUsnProcessed.lberbv,
                &syncReqCtrlVal->bvUtdVector.lberbv) == LBER_ERROR)
        {
            VMDIR_LOG_ERROR(
                    VMDIR_LOG_MASK_ALL,
                    "ParseSyncRequestControlVal: ber_scanf failed while parsing "
                    "lastLocalUsnProcessed in the sync request control value");

            lr->errCode = LDAP_PROTOCOL_ERROR;
            retVal = LDAP_NOTICE_OF_DISCONNECT;

            BAIL_ON_VMDIR_ERROR_WITH_MSG(
                    retVal,
                    pszLocalErrorMsg,
                    "Error in reading lastLocalUsnProcessed in the sync request control value.");
        }

        VMDIR_LOG_DEBUG(
                LDAP_DEBUG_REPL,
                "ParseSyncRequestControlVal: ServerId: %s, lastLocalUsnProcessed: %s, utdVector: %s",
                syncReqCtrlVal->reqInvocationId.lberbv.bv_val,
                syncReqCtrlVal->bvLastLocalUsnProcessed.lberbv.bv_val,
                syncReqCtrlVal->bvUtdVector.lberbv.bv_val);

        syncReqCtrlVal->intLastLocalUsnProcessed =
                op->syncDoneCtrl->value.syncDoneCtrlVal.intLastLocalUsnProcessed =
                        VmDirStringToLA(syncReqCtrlVal->bvLastLocalUsnProcessed.lberbv.bv_val, NULL, 10);

        {
            char* nextServerIdStr = NULL;
            char* nextOrigUsnStr = NULL;

            nextServerIdStr = syncReqCtrlVal->bvUtdVector.lberbv.bv_val;

            while (nextServerIdStr != NULL && nextServerIdStr[0] != '\0')
            {
                PLW_HASHTABLE_NODE pNode = NULL;

                // Ignore continue indicator in sync request control
                if (VmDirStringNCompareA(
                        nextServerIdStr,
                        VMDIR_REPL_CONT_INDICATOR,
                        VMDIR_REPL_CONT_INDICATOR_LEN,
                        FALSE) == 0)
                {
                    nextServerIdStr = VmDirStringChrA(nextServerIdStr, ',') + 1;
                    continue;
                }

                if (VmDirAllocateMemory(sizeof(UptoDateVectorEntry), (PVOID*)&utdVectorEntry) != 0)
                {
                    VMDIR_LOG_ERROR(
                            VMDIR_LOG_MASK_ALL,
                            "ParseSyncRequestControlVal: VmDirAllocateMemory failed");

                    lr->errCode = retVal = LDAP_OPERATIONS_ERROR;

                    BAIL_ON_VMDIR_ERROR_WITH_MSG(
                            retVal,
                            pszLocalErrorMsg,
                            "ParseSyncRequestControlVal: VmDirAllocateMemory failed.");
                }

                nextOrigUsnStr = VmDirStringChrA(nextServerIdStr, ':');
                *nextOrigUsnStr = '\0';
                nextOrigUsnStr++;
                utdVectorEntry->invocationId.lberbv.bv_val = nextServerIdStr;
                utdVectorEntry->invocationId.lberbv.bv_len = VmDirStringLenA(nextServerIdStr);

                nextServerIdStr = VmDirStringChrA(nextOrigUsnStr, ',');
                *nextServerIdStr = '\0';
                nextServerIdStr++;

                utdVectorEntry->currMaxOrigUsnProcessed =
                        utdVectorEntry->reqLastOrigUsnProcessed =
                                atol(nextOrigUsnStr);

                LwRtlHashTableResizeAndInsert(
                        op->syncDoneCtrl->value.syncDoneCtrlVal.htUtdVector,
                        &utdVectorEntry->Node,
                        &pNode);
                assert(pNode == NULL);    // assert the key of added node is unique.
            }
        }

        tag = ber_peek_tag(ber, &len);
    }
    if (tag == LBER_BOOLEAN)
    {
        ber_int_t firstPage;
        if (ber_scanf(ber, "b", &firstPage) == LBER_ERROR)
        {
            VMDIR_LOG_ERROR(
                    VMDIR_LOG_MASK_ALL,
                    "ParseSyncRequestControlVal: Error in reading reloadHint from the PDU");

            lr->errCode = LDAP_PROTOCOL_ERROR;
            retVal = LDAP_NOTICE_OF_DISCONNECT;

            BAIL_ON_VMDIR_ERROR_WITH_MSG(
                    retVal,
                    pszLocalErrorMsg,
                    "Error in reading reloadHint from the PDU.");
        }
        if (firstPage)
        {
            syncReqCtrlVal->bFirstPage = TRUE;
        }
    }
    if (ber_scanf(ber, "}") == LBER_ERROR) // End of control value
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "ParseSyncRequestControlVal: ber_scanf failed while parsing the end of "
                "sync request control value");

        lr->errCode = LDAP_PROTOCOL_ERROR;
        retVal = LDAP_NOTICE_OF_DISCONNECT;

        BAIL_ON_VMDIR_ERROR_WITH_MSG(
                retVal,
                pszLocalErrorMsg,
                "Decoding error while parsing the end of sync request control value.");
    }

    backendCtx.pBE = VmDirBackendSelect("");
    maxPartnerVisibleUSN = backendCtx.pBE->pfnBEGetLeastOutstandingUSN(&backendCtx, FALSE) - 1;

    if (syncReqCtrlVal->intLastLocalUsnProcessed > maxPartnerVisibleUSN)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "ParseSyncRequestControlVal: ServerId %s has processed my USN (%u), my max USN is (%u)",
                syncReqCtrlVal->reqInvocationId.lberbv.bv_val,
                syncReqCtrlVal->intLastLocalUsnProcessed, maxPartnerVisibleUSN);

        lr->errCode = LDAP_UNWILLING_TO_PERFORM;
        retVal = LDAP_NOTICE_OF_DISCONNECT;

        BAIL_ON_VMDIR_ERROR_WITH_MSG(
                retVal,
                pszLocalErrorMsg,
                "Partner is ahead of my changes.");
    }

    if (op->conn->ReplConnState.phmSyncStateOneMap == NULL)
    {
        // replication request, create connection level map for SyncStateControl adjustment (ADD -> MODIFY)
        if (LwRtlCreateHashMap(
                &op->conn->ReplConnState.phmSyncStateOneMap,
                LwRtlHashDigestPstrCaseless,
                LwRtlHashEqualPstrCaseless,
                NULL) != 0)
        {
            lr->errCode = retVal = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR(retVal);
        }
    }
    else if (syncReqCtrlVal->bFirstPage)
    {
        /*
         * Consumer set syncReqCtrlVal->bFirstPage in the first page request of a
         * 1. fresh cycle
         * 2. retry per cycle (out of order)
         * 3. retry from scratch (i.e. failed to fill hole in case 2 and high watermark == 0).
         *
         * In this case, we should cleanup phmSyncStateOneMap to derive proper SYNC_STATE.
         */
        LwRtlHashMapClear(op->conn->ReplConnState.phmSyncStateOneMap, VmDirSimpleHashMapPairFree, NULL);
        VMDIR_LOG_INFO(
                LDAP_DEBUG_REPL,
                "ParseSyncRequestControlVal: phmSyncStateOneMap Cleared because of replication cycle retry");
    }

cleanup:
    // Even in the error case, syncDoneCtrl should be freed during operation delete.
    VMDIR_LOG_DEBUG(LDAP_DEBUG_TRACE, "ParseSyncRequestControlVal: End");
    VmDirBackendCtxContentFree(&backendCtx);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return retVal;

error:
    VMDIR_APPEND_ERROR_MSG(lr->pszErrMsg, pszLocalErrorMsg);
    goto cleanup;
}

static int
_ParsePagedResultControlVal(
    VDIR_OPERATION *                    op,
    BerValue *                          controlValue,       // Input: control value encoded as ber
    VDIR_PAGED_RESULT_CONTROL_VALUE *   pageResultCtrlVal,     // Output
    VDIR_LDAP_RESULT *                  lr                  // Output
    )
{
    int                     retVal = LDAP_SUCCESS;
    BerElementBuffer        berbuf;
    BerElement *            ber = (BerElement *)&berbuf;
    PSTR                    pszLocalErrorMsg = NULL;
    PSTR                    pszCookie = NULL;

    VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "_ParsePagedResultControlVal: Start." );

    if (!op)
    {
        retVal = LDAP_PROTOCOL_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    ber_init2( ber, controlValue, LBER_USE_DER );

    /* http://www.ietf.org/rfc/rfc2696.txt
     *
     * The searchControlValue is an OCTET STRING wrapping the BER-encoded version of the following SEQUENCE:
     *
     * realSearchControlValue ::= SEQUENCE {
     *        size            INTEGER (0..maxInt),
     *                                -- requested page size from client
     *                                -- result set size estimate from server
     *        cookie          OCTET STRING
    }
    */

    if (ber_scanf(ber, "{ia}",
                    &pageResultCtrlVal->pageSize,
                    &pszCookie) == LBER_ERROR)
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "_ParsePagedResultControlVal: ber_scanf failed while parsing "
                  "pageSize and cookie in the page result control value" );
        lr->errCode = LDAP_PROTOCOL_ERROR;
        retVal = LDAP_NOTICE_OF_DISCONNECT;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                        "Error in reading pageSize and cookie in the page result control value");
    }

    if (pageResultCtrlVal->pageSize == 0)
    {
        retVal = LDAP_CANCELLED;
        VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "Search query was cancelled.");
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    VmDirStringNCpyA(
        pageResultCtrlVal->cookie,
        VMDIR_ARRAY_SIZE(pageResultCtrlVal->cookie),
        pszCookie,
        VMDIR_ARRAY_SIZE(pageResultCtrlVal->cookie) - 1);

    VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "pageSize:%d", pageResultCtrlVal->pageSize );
    VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "cookie:%s", pageResultCtrlVal->cookie );

cleanup:
    if (pszCookie)
    {
        ber_memfree(pszCookie);
    }
    // Even in the error case, syncDoneCtrl should be freed during operation delete.
    VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "_ParsePagedResultControlVal: End." );
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return retVal;

error:
    VMDIR_APPEND_ERROR_MSG(lr->pszErrMsg, pszLocalErrorMsg);
    goto cleanup;
}

static
int
_ParseRaftPingControlVal(
   VDIR_OPERATION *                 pOp,
   BerValue *                       pControlBer,    // Input: control value encoded as ber
   PVDIR_RAFT_PING_CONTROL_VALUE    pCtrlVal,       // Output
   VDIR_LDAP_RESULT *               pLdapResult     // Output
   )
{
    int                 retVal = LDAP_SUCCESS;
    BerElementBuffer    berbuf;
    BerElement *        ber = (BerElement *)&berbuf;
    PSTR                pszLocalErrorMsg = NULL;
    BerValue            localLeader = {0};
    ber_int_t           localTerm = 0;

    if (!pOp)
    {
        retVal = LDAP_PROTOCOL_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    ber_init2( ber, pControlBer, LBER_USE_DER );

    /*
     * https://confluence.eng.vmware.com/display/LIG/XXXXXX TBD
     *
     * The PingControl is a null terminated STRING wrapping the BER-encoded version of the following SEQUENCE:
     *
     * ControlValue ::= SEQUENCE {
     *        term                    Integer
     *        Leader                  OCTET STRING
     *        ClusterStateNodeSeenMyOrgUSN    OCTET STRING
     *  }
     */

    if (ber_scanf(ber, "{im}", &localTerm, &localLeader) == LBER_ERROR)
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s: ber_scanf failed while parsing filter value", __FUNCTION__);
        pLdapResult->errCode = LDAP_PROTOCOL_ERROR;
        retVal = LDAP_NOTICE_OF_DISCONNECT;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                        "Error in reading cluster state control filter");
    }

    retVal = VmDirAllocateStringA(localLeader.bv_val, &(pCtrlVal->pszFQDN));
    BAIL_ON_VMDIR_ERROR(retVal);

    pCtrlVal->term = localTerm;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);

    return retVal;

error:
    VMDIR_APPEND_ERROR_MSG(pLdapResult->pszErrMsg, pszLocalErrorMsg);
    goto cleanup;
}

static
int
_ParseRaftVoteControlVal(
    VDIR_OPERATION *                pOp,
    BerValue *                      pControlBer,    // Input: control value encoded as ber
    PVDIR_RAFT_VOTE_CONTROL_VALUE   pCtrlVal,       // Output
    VDIR_LDAP_RESULT *              pLdapResult     // Output
    )
{
    int                 retVal = LDAP_SUCCESS;
    BerElementBuffer    berbuf;
    BerElement *        ber = (BerElement *)&berbuf;
    PSTR                pszLocalErrorMsg = NULL;
    BerValue            localCandidateId = {0};
    ber_int_t           localTerm = 0;

    if (!pOp)
    {
        retVal = LDAP_PROTOCOL_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    ber_init2( ber, pControlBer, LBER_USE_DER );

    /*
     * https://confluence.eng.vmware.com/display/LIG/XXXXXX TBD
     *
     * The VoteControl is a null terminated STRING wrapping the BER-encoded version of the following SEQUENCE:
     *
     * ControlValue ::= SEQUENCE {
     *        term                    Integer
     *        CandidateId             OCTET STRING
     *  }
     */

    if (ber_scanf(ber, "{im}", &localTerm, &localCandidateId) == LBER_ERROR)
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s: ber_scanf failed while parsing filter value", __FUNCTION__);
        pLdapResult->errCode = LDAP_PROTOCOL_ERROR;
        retVal = LDAP_NOTICE_OF_DISCONNECT;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                        "Error in reading cluster state control filter");
    }

    retVal = VmDirAllocateStringA(localCandidateId.bv_val, &(pCtrlVal->pszCandidateId));
    BAIL_ON_VMDIR_ERROR(retVal);

    pCtrlVal->term = localTerm;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);

    return retVal;

error:
    VMDIR_APPEND_ERROR_MSG(pLdapResult->pszErrMsg, pszLocalErrorMsg);
    goto cleanup;
}

static
int
_ParseDigestControlVal(
    VDIR_OPERATION *                op,
    BerValue *                      controlValue,   // Input: control value encoded as ber
    VDIR_DIGEST_CONTROL_VALUE *     digestCtrlVal,  // Output
    VDIR_LDAP_RESULT *              lr              // Output
    )
{
    int                 retVal = LDAP_SUCCESS;
    BerElementBuffer    berbuf;
    BerElement *        ber = (BerElement *)&berbuf;
    PSTR                pszLocalErrorMsg = NULL;
    BerValue            localBV = {0};

    if (!op)
    {
        retVal = LDAP_PROTOCOL_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    ber_init2( ber, controlValue, LBER_USE_DER );

    /*
     *
     * The DigestControlValue is an OCTET STRING wrapping the BER-encoded version of the following SEQUENCE:
     *
     * realSearchControlValue ::= SEQUENCE {
     *        digest            OCTET STRING
     *  }
     */

    if ((ber_scanf(ber, "{m}", &localBV) == LBER_ERROR)
        ||
        localBV.bv_len != SHA_DIGEST_LENGTH
       )
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s: ber_scanf failed while parsing digest control value", __FUNCTION__);
        lr->errCode = LDAP_PROTOCOL_ERROR;
        retVal = LDAP_NOTICE_OF_DISCONNECT;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                        "Error in reading digest control digest value");
    }

    memcpy(digestCtrlVal->sha1Digest, localBV.bv_val, SHA_DIGEST_LENGTH);

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return retVal;

error:
    VMDIR_APPEND_ERROR_MSG(lr->pszErrMsg, pszLocalErrorMsg);
    goto cleanup;
}

static
int
_ParseStatePingControlVal(
    VDIR_OPERATION*                 op,
    BerValue*                       pControlValue,  // Input: control value encoded as ber
    VDIR_STATE_PING_CONTROL_VALUE*  pPingCtrlVal,   // Output
    VDIR_LDAP_RESULT*               lr              // Output
    )
{
    int     retVal = LDAP_SUCCESS;
    BerElementBuffer    berbuf;
    BerElement*         ber = (BerElement*)&berbuf;
    BerValue            berv = {0};
    unsigned char*      reader = NULL;

    // variables for decoding payload elements
    int     n = 0;
    int     i = 0;
    int     len = 0;
    PSTR    pszKey = NULL;
    PSTR    pszVal = NULL;

    // expected key strings
    PCSTR   pszHostnameKey = "hostname";
    PCSTR   pszInvocationIdKey = "invocationid";
    PCSTR   pszMaxOrigUsnKey = "maxorigusn";

    if (!op || !pControlValue || !pPingCtrlVal || !lr)
    {
        retVal = LDAP_PROTOCOL_ERROR;
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    ber_init2(ber, pControlValue, LBER_USE_DER);

    /*
     * The statePingControlValue is an OCTET STRING wrapping the BER-encoded version of the following SEQUENCE:
     *
     * statePingControlValue ::= SEQUENCE {
     *        encoded_payload       OCTET STRING
     *  }
     */

    if (ber_scanf(ber, "{m}", &berv) == LBER_ERROR)
    {
        lr->errCode = LDAP_PROTOCOL_ERROR;
        retVal = LDAP_NOTICE_OF_DISCONNECT;
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    // decode payload
    reader = berv.bv_val;

    // first read the number of elements in the payload
    n = VmDirDecodeShort(&reader);

    // decode each element
    for (i = 0; i < n; i++)
    {
        len = VmDirDecodeShort(&reader);
        pszKey = reader;
        reader += (len + 1); // skipping \0

        len = VmDirDecodeShort(&reader);
        pszVal = reader;
        reader += (len + 1); // skipping \0

        if (VmDirStringCompareA(pszKey, pszHostnameKey, FALSE) == 0)
        {
            retVal = VmDirAllocateStringA(pszVal, &pPingCtrlVal->pszFQDN);
            BAIL_ON_VMDIR_ERROR(retVal);
        }
        else if (VmDirStringCompareA(pszKey, pszInvocationIdKey, FALSE) == 0)
        {
            retVal = VmDirAllocateStringA(pszVal, &pPingCtrlVal->pszInvocationId);
            BAIL_ON_VMDIR_ERROR(retVal);
        }
        else if (VmDirStringCompareA(pszKey, pszMaxOrigUsnKey, FALSE) == 0)
        {
            retVal = VmDirStringToUSN(pszVal, &pPingCtrlVal->maxOrigUsn);
            BAIL_ON_VMDIR_ERROR(retVal);
        }
    }

cleanup:
    return retVal;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "failed, error (%d)",
            retVal);

    goto cleanup;
}

int
VmDirCreateDigestControlContent(
    PCSTR           pszDigest,
    DWORD           dwDigestLen,
    LDAPControl*    pDigestCtrl
    )
{
    int             retVal = LDAP_SUCCESS;
    BerElement*     pBer = NULL;
    BerValue        localBV = {0};

    if (!pszDigest || !pDigestCtrl)
    {
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
    }

    if ((pBer = ber_alloc()) == NULL)
    {
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
    }

    localBV.bv_val = (char*)pszDigest;
    localBV.bv_len = dwDigestLen;

    if (ber_printf(pBer, "{O}", &localBV) == -1)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "%s: ber_printf failed.",
                __FUNCTION__);

        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
    }

    memset(pDigestCtrl, 0, sizeof(LDAPControl));
    pDigestCtrl->ldctl_oid = LDAP_CONTROL_DIGEST_SEARCH;
    pDigestCtrl->ldctl_iscritical = '1';

    if (ber_flatten2(pBer, &pDigestCtrl->ldctl_value, 1))
    {
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
    }

cleanup:

    if (pBer)
    {
        ber_free(pBer, 1);
    }
    return retVal;

ldaperror:
    VmDirFreeCtrlContent(pDigestCtrl);
    goto cleanup;
}

int
VmDirCreateStatePingControlContent(
    LDAPControl*    pPingCtrl
    )
{
    int     retVal = LDAP_SUCCESS;
    SIZE_T  sizeBuffer = VMDIR_MAX_CONTROL_PAYLOAD_SIZE;
    SIZE_T  sizeRemain = sizeBuffer;
    unsigned char   buffer[VMDIR_MAX_CONTROL_PAYLOAD_SIZE] = {0};   // encoded payload
    unsigned char*  writer = buffer;
    VDIR_BERVALUE   berVal = VDIR_BERVALUE_INIT;
    BerElement*     pBer = NULL;

    // variables for hostname
    PCSTR   pszHostnameKey = "hostname";
    SIZE_T  lenHostnameKey = VmDirStringLenA(pszHostnameKey);

    // variables for invocationId
    PCSTR   pszInvocationIdKey = "invocationid";
    SIZE_T  lenInvocationIdKey = VmDirStringLenA(pszInvocationIdKey);

    // variables for maxOrigUSN
    VDIR_BACKEND_CTX    beCtx = {0};
    PCSTR   pszMaxOrigUsnKey = "maxorigusn";
    SIZE_T  lenMaxOrigUsnKey = VmDirStringLenA(pszMaxOrigUsnKey);
    USN     maxOrigUsn = 0;
    PSTR    pszMaxOrigUsnVal = NULL;
    SIZE_T  lenMaxOrigUsnVal = 0;

    if (!pPingCtrl)
    {
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
    }

    if ((pBer = ber_alloc()) == NULL)
    {
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
    }

    // write number of elements inside the payload (currently 3)
    VmDirEncodeShort(&writer, 3);
    sizeRemain -= 2;

    // 1. hostname
    VmDirEncodeShort(&writer, lenHostnameKey);
    sizeRemain -= 2;

    retVal = VmDirCopyMemory(
            writer,
            sizeRemain,
            pszHostnameKey,
            lenHostnameKey + 1);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    writer += lenHostnameKey;
    *writer++ = '\0';
    sizeRemain -= (lenHostnameKey + 1);

    VmDirEncodeShort(&writer, gVmdirServerGlobals.bvServerObjName.lberbv.bv_len);
    sizeRemain -= 2;

    retVal = VmDirCopyMemory(
            writer,
            sizeRemain,
            gVmdirServerGlobals.bvServerObjName.lberbv.bv_val,
            gVmdirServerGlobals.bvServerObjName.lberbv.bv_len + 1);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    writer += gVmdirServerGlobals.bvServerObjName.lberbv.bv_len;
    *writer++ = '\0';
    sizeRemain -= (gVmdirServerGlobals.bvServerObjName.lberbv.bv_len + 1);

    // 2. invocationId
    VmDirEncodeShort(&writer, lenInvocationIdKey);
    sizeRemain -= 2;

    retVal = VmDirCopyMemory(
            writer,
            sizeRemain,
            pszInvocationIdKey,
            lenInvocationIdKey + 1);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    writer += lenInvocationIdKey;
    *writer++ = '\0';
    sizeRemain -= (lenInvocationIdKey + 1);

    VmDirEncodeShort(&writer, gVmdirServerGlobals.invocationId.lberbv.bv_len);
    sizeRemain -= 2;

    retVal = VmDirCopyMemory(
            writer,
            sizeRemain,
            gVmdirServerGlobals.invocationId.lberbv.bv_val,
            gVmdirServerGlobals.invocationId.lberbv.bv_len + 1);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    writer += gVmdirServerGlobals.invocationId.lberbv.bv_len;
    *writer++ = '\0';
    sizeRemain -= (gVmdirServerGlobals.invocationId.lberbv.bv_len + 1);

    // 3. maxOrigUSN
    beCtx.pBE = VmDirBackendSelect(NULL);
    maxOrigUsn = beCtx.pBE->pfnBEGetMaxOriginatingUSN(&beCtx);

    retVal = VmDirAllocateStringPrintf(&pszMaxOrigUsnVal, "%" PRId64, maxOrigUsn);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    lenMaxOrigUsnVal = VmDirStringLenA(pszMaxOrigUsnVal);

    VmDirEncodeShort(&writer, lenMaxOrigUsnKey);
    sizeRemain -= 2;

    retVal = VmDirCopyMemory(
            writer,
            sizeRemain,
            pszMaxOrigUsnKey,
            lenMaxOrigUsnKey + 1);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    writer += lenMaxOrigUsnKey;
    *writer++ = '\0';
    sizeRemain -= (lenMaxOrigUsnKey + 1);

    VmDirEncodeShort(&writer, lenMaxOrigUsnVal);
    sizeRemain -= 2;

    retVal = VmDirCopyMemory(
            writer,
            sizeRemain,
            pszMaxOrigUsnVal,
            lenMaxOrigUsnVal + 1);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    writer += lenMaxOrigUsnVal;
    *writer++ = '\0';
    sizeRemain -= (lenMaxOrigUsnVal + 1);

    // write the encoded payload to ber
    berVal.lberbv.bv_val = buffer;
    berVal.lberbv.bv_len = sizeBuffer - sizeRemain;

    if (ber_printf(pBer, "{O}", &berVal) == -1)
    {
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
    }

    memset(pPingCtrl, 0, sizeof(LDAPControl));
    pPingCtrl->ldctl_oid = LDAP_STATE_PING_CONTROL;
    pPingCtrl->ldctl_iscritical = '1';

    if (ber_flatten2(pBer, &pPingCtrl->ldctl_value, 1))
    {
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
    }

cleanup:
    if (pBer)
    {
        ber_free(pBer, 1);
    }
    VmDirBackendCtxContentFree(&beCtx);
    VMDIR_SAFE_FREE_MEMORY(pszMaxOrigUsnVal);
    return retVal;

ldaperror:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "failed, error (%d)",
            retVal);

    VmDirFreeCtrlContent(pPingCtrl);
    goto cleanup;
}
