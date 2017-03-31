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

    VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "ParseRequestControls: Begin." );

    *control = NULL;

#ifndef _WIN32
    if (ber_pvt_ber_remaining(op->ber) != 0)
#else
    if (LBER_DEFAULT != ber_peek_tag (op->ber, &len))
#endif
    {
        if (ber_peek_tag( op->ber, &len ) != LDAP_TAG_CONTROLS )
        {
            lr->errCode = LDAP_PROTOCOL_ERROR;
            retVal = LDAP_NOTICE_OF_DISCONNECT;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                           "ParseRequestControls: Request controls expected, but something else is there in the PDU.");
        }

        // Get controls. ber_first_element => skip the sequence header, set the cursor at the 1st control in the SEQ of SEQ
        for( tag = ber_first_element( op->ber, &len, &endOfCtrlsMarker ); tag != LBER_ERROR;
             tag = ber_next_element( op->ber, &len, endOfCtrlsMarker ) )
        {
            // m => in-place
            if (ber_scanf( op->ber, "{m", &lberBervType ) == LBER_ERROR)
            {
                lr->errCode = LDAP_PROTOCOL_ERROR;
                retVal = LDAP_NOTICE_OF_DISCONNECT;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
                                              "ParseRequestControls Error in reading control type from the PDU.");
            }
            if ( VmDirLogGetMask() & LDAP_DEBUG_ARGS)
            {
                VMDIR_LOG_INFO( LDAP_DEBUG_ARGS, "    Request Control: %s", lberBervType.bv_val );
            }
            if (VmDirAllocateMemory( sizeof( VDIR_LDAP_CONTROL), (PVOID *)control ) != 0)
            {
                retVal = lr->errCode = LDAP_OPERATIONS_ERROR;
                BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                                "ParseRequestControls: VmDirAllocateMemory failed");
            }
            // type points into in-place ber and does NOT own its content
            (*control)->type = lberBervType.bv_val;
            (*control)->criticality = FALSE;
            tag = ber_peek_tag( op->ber, &len );

            if (tag == LBER_BOOLEAN)
            {
                ber_int_t criticality;
                if (ber_scanf( op->ber, "b", &criticality) == LBER_ERROR)
                {
                    lr->errCode = LDAP_PROTOCOL_ERROR;
                    retVal = LDAP_NOTICE_OF_DISCONNECT;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
                                            "ParseRequestControls: Error in reading control criticality from the PDU.");
                }
                if (criticality)
                {
                    (*control)->criticality = TRUE;
                }
                tag = ber_peek_tag( op->ber, &len );
            }

            if (tag == LBER_OCTETSTRING)
            {
                if (ber_scanf( op->ber, "m", &lberBervCtlValue) == LBER_ERROR)
                {
                    lr->errCode = LDAP_PROTOCOL_ERROR;
                    retVal = LDAP_NOTICE_OF_DISCONNECT;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
                                             "ParseRequestControls: ber_scanf failed while parsing the control value.");
                }
            }

            // SJ-TBD: Make sure that the control appears only once in the request, and it is present only in a search
            // request
            if (VmDirStringCompareA( (*control)->type, LDAP_CONTROL_SYNC, TRUE ) == 0)
            {
                if (VmDirdGetRunMode() != VMDIR_RUNMODE_NORMAL)
                {
                    // Why block out-bound replication when catching up during restore mode?
                    //
                    // Reason: Partners have high-water-mark (lastLocalUsn) corresponding to this replica that is being
                    // restored. If out-bound replication is not blocked while restore/catching-up is going on, originating
                    // or replicated updates (if this replica is the only partner) made between the current-local-usn and
                    // high-water-marks, that partners remember, will not get replicated out (even if the invocationId
                    // has been fixed/changed).

                    retVal = lr->errCode = LDAP_UNWILLING_TO_PERFORM;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
                                 "ParseRequestControls: Server not in normal mode, not allowing outward replication.");
                }

                if ((retVal = ParseSyncRequestControlVal( op, &lberBervCtlValue, &((*control)->value.syncReqCtrlVal),
                                                          lr)) != LDAP_SUCCESS)
                {
                    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
                                                  "ParseRequestControls: ParseSyncRequestControlVal failed.");
                }
                op->syncReqCtrl = *control;
            }
            if (VmDirStringCompareA( (*control)->type, VDIR_LDAP_CONTROL_SHOW_DELETED_OBJECTS, TRUE ) == 0)
            {
                op->showDeletedObjectsCtrl = *control;
            }

            if (VmDirStringCompareA( (*control)->type, VDIR_LDAP_CONTROL_SHOW_MASTER_KEY, TRUE ) == 0)
            {
                op->showMasterKeyCtrl = *control;
            }

            if (VmDirStringCompareA((*control)->type, LDAP_CONTROL_CONSISTENT_WRITE, TRUE ) == 0)
            {
                op->strongConsistencyWriteCtrl = *control;
            }

            if (VmDirStringCompareA((*control)->type, VDIR_LDAP_CONTROL_MANAGEDDSAIT, TRUE ) == 0)
            {
                op->manageDsaITCtrl = *control;
            }

            if (VmDirStringCompareA( (*control)->type, LDAP_CONTROL_PAGEDRESULTS, TRUE ) == 0)
            {
                retVal = _ParsePagedResultControlVal( op,
                                                &lberBervCtlValue,
                                                &((*control)->value.pagedResultCtrlVal),
                                                lr);
                if (retVal != LDAP_SUCCESS)
                {
                    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
                                                  "ParseRequestControls: _ParsePagedResultControlVal failed.");
                }
                op->showPagedResultsCtrl = *control;
            }

            if ( ber_scanf( op->ber, "}") == LBER_ERROR ) // end of control
            {
                lr->errCode = LDAP_PROTOCOL_ERROR;
                retVal = LDAP_NOTICE_OF_DISCONNECT;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
                                              "ParseRequestControls: ber_scanf failed while parsing the end of control");

            }
            control = &((*control)->next);
        }

        retVal = LDAP_SUCCESS;
    }

cleanup:
    VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "ParseRequestControls: End." );
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);

    return retVal;

error:
    DeleteControls(&(op->reqControls));

    if (pszLocalErrorMsg)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, pszLocalErrorMsg);
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

    VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "WriteSyncDoneControl: Begin." );

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
                                (VMDIR_GUID_STR_LEN + 1 + VMDIR_MAX_USN_STR_LEN + 1);

            // Sync Done control value looks like: <lastLocalUsnChanged>,<serverId1>:<server 1 last originating USN>,
            // <serverId2>,<server 2 originating USN>,...

            if (VmDirAllocateMemory( bufferSize, (PVOID *)&bvCtrlVal.lberbv.bv_val) != 0)
            {
                retVal = LDAP_OPERATIONS_ERROR;
                BAIL_ON_VMDIR_ERROR( retVal );
            }
            writer = bvCtrlVal.lberbv.bv_val;
            bvCtrlVal.lberbv.bv_len = 0;

            VmDirStringPrintFA( writer, bufferSize, "%ld,", op->syncDoneCtrl->value.syncDoneCtrlVal.intLastLocalUsnProcessed );
            tmpLen = VmDirStringLenA( writer );
            writer += tmpLen;
            bufferSize -= tmpLen;
            bvCtrlVal.lberbv.bv_len += tmpLen;

            while ((pNode = LwRtlHashTableIterate(op->syncDoneCtrl->value.syncDoneCtrlVal.htUtdVector, &iter)))
            {
                pUtdVectorEntry = LW_STRUCT_FROM_FIELD(pNode, UptoDateVectorEntry, Node);
                VmDirStringPrintFA( writer, bufferSize, "%s:%ld,", pUtdVectorEntry->invocationId.lberbv.bv_val,
                                    pUtdVectorEntry->currMaxOrigUsnProcessed );
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
    VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "WriteSyncDoneControl: Begin." );
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
   VDIR_ATTRIBUTE *   pAttr,
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
    PSTR                pszLocalErrorMsg = NULL;

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

    for ( ; pAttr != NULL; pAttr = pAttr->next)
    {
        if (pAttr->metaData[0] != '\0') // We are sending back this attribute's meta data. => Attribute is in
                                        // replication scope. SJ-TBD: we are overloading pAttr->metaData
                                        // field. Should we have another field (e.g. inReplScope) in Attribute
        {
            if ( VmDirStringCompareA( pAttr->type.lberbv.bv_val, ATTR_IS_DELETED, FALSE ) == 0 &&
                 VmDirStringCompareA( pAttr->vals[0].lberbv.bv_val, VMDIR_IS_DELETED_TRUE_STR, FALSE ) == 0)
            {
                entryState = LDAP_SYNC_DELETE;
                continue; // Look if it is LDAP_SYNC_ADD case
            }

            if (VmDirStringCompareA( pAttr->type.lberbv.bv_val, ATTR_USN_CREATED, FALSE ) == 0)
            {
                entryState = LDAP_SYNC_ADD;
                break;
            }
        }
    }

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

    if (ber_printf( ctrlValBer, "{i}", entryState ) == -1 )
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

    VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "WriteSyncStateControl: End" );

    return( retVal );

error:

    goto cleanup;
}

int
ParseSyncStateControlVal(
    BerValue *  controlValue,   // Input: control value encoded as ber,
    int *       entryState)     // Output
{
    int                 retVal = LDAP_SUCCESS;
    BerElementBuffer    berbuf;
    BerElement *        ber = (BerElement *)&berbuf;

    VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "ParseSyncStateControlVal: Begin." );

    ber_init2( ber, controlValue, LBER_USE_DER );

    if (ber_scanf( ber, "{i}", entryState ) == -1 )
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "ParseSyncStateControlVal: ber_scanf to read entryState failed" );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

cleanup:
    VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "ParseSyncStateControlVal: Begin." );
    return retVal;

error:
    goto cleanup;
}

int
ParseAndFreeSyncStateControl(
    LDAPControl ***pCtrls,
    int *piEntryState
    )
{
    int retVal = LDAP_SUCCESS;
    int entryState = -1;

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

    retVal = ParseSyncStateControlVal(&(*pCtrls)[0]->ldctl_value, &entryState);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    ldap_controls_free(*pCtrls);
    *pCtrls = NULL;

    *piEntryState = entryState;

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

    VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "ParseSyncRequestControlVal: Begin." );

    ber_init2( ber, controlValue, LBER_USE_DER );

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

    if (ber_scanf( ber, "{i", &(syncReqCtrlVal->mode) ) == LBER_ERROR)
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "ParseSyncRequestControlVal: ber_scanf failed while parsing the sync request "
                  "control mode" );
        lr->errCode = LDAP_PROTOCOL_ERROR;
        retVal = LDAP_NOTICE_OF_DISCONNECT;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                        "Error in reading sync request control mode from PDU.");
    }

    syncReqCtrlVal->bvLastLocalUsnProcessed.lberbv.bv_val = "";
    syncReqCtrlVal->intLastLocalUsnProcessed = 0;

    if (VmDirAllocateMemory( sizeof( VDIR_LDAP_CONTROL ), (PVOID *)&op->syncDoneCtrl) != 0)
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "ParseSyncRequestControlVal: VmDirAllocateMemory failed " );
        lr->errCode = retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                        "ParseSyncRequestControlVal: VmDirAllocateMemory failed.");
    }
    op->syncDoneCtrl->type = LDAP_CONTROL_SYNC_DONE;
    if (LwRtlCreateHashTable( &op->syncDoneCtrl->value.syncDoneCtrlVal.htUtdVector, UtdVectorEntryGetKey,
                              LwRtlHashDigestPstr, LwRtlHashEqualPstr, NULL, VMDIR_UTD_VECTOR_HASH_TABLE_SIZE ) != 0)
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "UpdateSyncDoneUtdVectorEntry: LwRtlCreateHashTable failed" );
        lr->errCode = retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                        "UpdateSyncDoneUtdVectorEntry: LwRtlCreateHashTable failed");

    }

    tag = ber_peek_tag( ber, &len );

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

        if (ber_scanf( ber, "{mmm}",
                        &syncReqCtrlVal->reqInvocationId.lberbv,
                        &syncReqCtrlVal->bvLastLocalUsnProcessed.lberbv,
                        &syncReqCtrlVal->bvUtdVector.lberbv ) == LBER_ERROR )
        {
            VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "ParseSyncRequestControlVal: ber_scanf failed while parsing "
                      "lastLocalUsnProcessed in the sync request control value" );
            lr->errCode = LDAP_PROTOCOL_ERROR;
            retVal = LDAP_NOTICE_OF_DISCONNECT;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                            "Error in reading lastLocalUsnProcessed in the sync request control value");
        }

        VMDIR_LOG_DEBUG( LDAP_DEBUG_REPL, "ParseSyncRequestControlVal: ServerId: %s, lastLocalUsnProcessed: %s, utdVector: %s",
                  syncReqCtrlVal->reqInvocationId.lberbv.bv_val, syncReqCtrlVal->bvLastLocalUsnProcessed.lberbv.bv_val,
                  syncReqCtrlVal->bvUtdVector.lberbv.bv_val );

        syncReqCtrlVal->intLastLocalUsnProcessed = op->syncDoneCtrl->value.syncDoneCtrlVal.intLastLocalUsnProcessed =
                                   VmDirStringToLA( syncReqCtrlVal->bvLastLocalUsnProcessed.lberbv.bv_val, NULL, 10 );
        {
            char *                  nextServerIdStr = NULL;
            char *                  nextOrigUsnStr = NULL;

            nextServerIdStr = syncReqCtrlVal->bvUtdVector.lberbv.bv_val;

            while( nextServerIdStr != NULL && nextServerIdStr[0] != '\0')
            {
                PLW_HASHTABLE_NODE pNode = NULL;

                if (VmDirAllocateMemory( sizeof(UptoDateVectorEntry), (PVOID *)&utdVectorEntry ) != 0)
                {
                    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "ParseSyncRequestControlVal: VmDirAllocateMemory failed " );
                    lr->errCode = retVal = LDAP_OPERATIONS_ERROR;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                                    "ParseSyncRequestControlVal: VmDirAllocateMemory failed");
                }

                nextOrigUsnStr = VmDirStringChrA( nextServerIdStr, ':');
                *nextOrigUsnStr = '\0';
                nextOrigUsnStr++;
                utdVectorEntry->invocationId.lberbv.bv_val = nextServerIdStr;
                utdVectorEntry->invocationId.lberbv.bv_len = VmDirStringLenA( nextServerIdStr );

                nextServerIdStr = VmDirStringChrA( nextOrigUsnStr, ',');
                *nextServerIdStr = '\0';
                nextServerIdStr++;

                utdVectorEntry->currMaxOrigUsnProcessed = utdVectorEntry->reqLastOrigUsnProcessed =
                                                              atol( nextOrigUsnStr );

                LwRtlHashTableResizeAndInsert( op->syncDoneCtrl->value.syncDoneCtrlVal.htUtdVector,
                                               &utdVectorEntry->Node, &pNode);
                assert( pNode == NULL );    // assert the key of added node is unique.
            }
        }

        tag = ber_peek_tag( ber, &len );
    }
    if (tag == LBER_BOOLEAN)
    {
        ber_int_t reloadHint;
        if (ber_scanf( ber, "b", &reloadHint) == LBER_ERROR)
        {
            VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "ParseSyncRequestControlVal: Error in reading reloadHint from the PDU" );
            lr->errCode = LDAP_PROTOCOL_ERROR;
            retVal = LDAP_NOTICE_OF_DISCONNECT;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                            "Error in reading reloadHint from the PDU.");
        }
        if (reloadHint)
        {
            syncReqCtrlVal->reloadHint = TRUE;
        }
    }
    if ( ber_scanf( ber, "}") == LBER_ERROR ) // End of control value
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "ParseSyncRequestControlVal: ber_scanf failed while parsing the end of "
                  "sync request control value." );
        lr->errCode = LDAP_PROTOCOL_ERROR;
        retVal = LDAP_NOTICE_OF_DISCONNECT;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                        "Decoding error while parsing the end of sync request control value.");
    }

    backendCtx.pBE = VmDirBackendSelect("");
    maxPartnerVisibleUSN = backendCtx.pBE->pfnBEGetLeastOutstandingUSN( &backendCtx, FALSE ) - 1;

    if (syncReqCtrlVal->intLastLocalUsnProcessed > maxPartnerVisibleUSN)
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "ParseSyncRequestControlVal: ServerId %s has processed my USN (%u), my max USN is (%u).",
                         syncReqCtrlVal->reqInvocationId.lberbv.bv_val, syncReqCtrlVal->intLastLocalUsnProcessed, maxPartnerVisibleUSN );
        lr->errCode = LDAP_UNWILLING_TO_PERFORM;
        retVal = LDAP_NOTICE_OF_DISCONNECT;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg), "Partner is ahead of my changes.");
    }

cleanup:
    // Even in the error case, syncDoneCtrl should be freed during operation delete.
    VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "ParseSyncRequestControlVal: End." );
    VmDirBackendCtxContentFree( &backendCtx );
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

/* Generates the LdapControl to be communicated to the client
   in the case of Strong Consistency Write task */
int
WriteConsistencyWriteDoneControl(
    VDIR_OPERATION *       pOp,
    BerElement *           pBer
    )
{
    int                 retVal = LDAP_OPERATIONS_ERROR;
    BerElementBuffer    ctrlValBerbuf;
    BerElement *        pCtrlValBer = (BerElement *) &ctrlValBerbuf;
    VDIR_BERVALUE       bvCtrlVal = VDIR_BERVALUE_INIT;
    DWORD               dwStatus = 0;
    PSTR                pControlsString = NULL;

    if (pOp == NULL || pBer == NULL)
    {
       VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "WriteConsistencyWriteDoneControl: VDIR_OPERATION or BerElement is NULL failed");
       BAIL_ON_VMDIR_ERROR(retVal);
    }

    (void) memset((char *)&ctrlValBerbuf, '\0', sizeof(BerElementBuffer));
    ber_init2(pCtrlValBer, NULL, LBER_USE_DER);

    dwStatus = pOp->strongConsistencyWriteCtrl->value.scwDoneCtrlVal.status;
    pControlsString = pOp->strongConsistencyWriteCtrl->type;

    if (ber_printf(pCtrlValBer, "{i}", dwStatus) == -1)
    {
       VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "WriteConsistencyWriteDoneControl: ber_printf (to print status...) failed");
       BAIL_ON_VMDIR_ERROR(retVal);
    }

    bvCtrlVal.lberbv.bv_val = pCtrlValBer->ber_buf;
    bvCtrlVal.lberbv.bv_len = pCtrlValBer->ber_ptr - pCtrlValBer->ber_buf;

    if (ber_printf(pBer, "t{{sO}}", LDAP_TAG_CONTROLS, pControlsString, &bvCtrlVal.lberbv) == -1)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "WriteConsistencyWriteDoneControl: ber_printf (to print ldapControl...) failed");
	BAIL_ON_VMDIR_ERROR(retVal);
    }

    retVal = LDAP_SUCCESS;

cleanup:
    ber_free_buf(pCtrlValBer);
    return retVal;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "WriteConsistencyWriteDoneControl: failed");
    goto cleanup;
}
