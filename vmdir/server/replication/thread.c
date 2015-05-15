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
 * Module Name: Replication thread
 *
 * Filename: thread.c
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

static int
ReplAddEntry(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    LDAPMessage *       entry,
    BOOLEAN             replStateGood,
    BOOLEAN             bFirstReplicationCycle);

static int
ReplDeleteEntry(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    LDAPMessage *       entry,
    BOOLEAN             replStateGood);

static int
ReplModifyEntry(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    LDAPMessage *       entry,
    BOOLEAN             replStateGood,
    PVDIR_SCHEMA_CTX*   ppOutSchemaCtx);

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
    PVDIR_ENTRY         pEntry,
    BOOLEAN             replStateGood);

static
int
UpdateReplicationAgreement(
    PVDIR_SCHEMA_CTX                pSchemaCtx,
    VMDIR_REPLICATION_AGREEMENT *   replAgr,
    VDIR_BERVALUE *                 lastLocalUsnProcessed);

static
int
UpdateServerObject(
    PVDIR_SCHEMA_CTX                pSchemaCtx,
    VDIR_BERVALUE *                 utdVector,
    VMDIR_REPLICATION_AGREEMENT *   replAgr);

static
DWORD
_vdirReplicationLoadPasswords(
    PSTR *ppszPassword,
    PSTR *ppszOldPassword,
    PBOOLEAN pbPasswordChanged
    );

static
DWORD
_vdirReplicationConnect(
    PSTR pszUPN,
    PSTR pszDN,
    PSTR pszPassword,
    PSTR pszOldPassword,
    PVMDIR_REPLICATION_AGREEMENT pReplAgr,
    LDAP **pLd
    );

static
DWORD
_vdirReplConnect(
    PSTR pszPartnerHostName,
    PSTR pszUPN,
    PSTR pszDN,
    PSTR pszPassword,
    PSTR pszLdapURI,
    LDAP **pLd
    );

static
DWORD
vdirReplicationThrFun(
    PVOID   pArg
    );

static
DWORD
VmDirSrvCreateReplAgrObj(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    PCSTR            pszReplURI,
    PCSTR            pszLastLocalUsnProcessed,
    PSTR             pszReplAgrsContainerDN
    );

static
DWORD
VmDirSetGlobalServerId();

static
int
_VmDirPatchData(
    PVDIR_OPERATION     pOperation);

static DWORD
_VmDirAssignEntryIdIfSpecialInternalEntry(
    PVDIR_ENTRY pEntry
    );

#if 0   // 2013 port
static DWORD
_VmDirStorePartnerCertificate(
    PCSTR pszPartnerHostName);
#endif

DWORD
InitializeReplicationThread(
    void)
{
    DWORD               dwError = 0;
    PVDIR_THREAD_INFO   pThrInfo = NULL;

    dwError = VmDirAllocateMemory( sizeof(*pThrInfo), (PVOID)&pThrInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSrvThrInit(
                pThrInfo,
                gVmdirGlobals.replAgrsMutex,       // alternative mutex
                gVmdirGlobals.replAgrsCondition,   // alternative cond
                TRUE);

    dwError = VmDirCreateThread( &pThrInfo->tid, FALSE, vdirReplicationThrFun, pThrInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSrvThrAdd(pThrInfo);

cleanup:

    return dwError;

error:

    if (pThrInfo)
    {
        VmDirSrvThrFree(pThrInfo);
    }

    goto cleanup;
}

// vdirReplicationThrFun is the main replication function that:
//  - Executes replication cycles endlessly
//  - Each replication cycle consist of processing all the RAs for this vmdird instance.
//  - Sleeps for gVmdirServerGlobals.replInterval between replication cycles.
//
// While processing an RA (replicating from a replication partner):
//  - Updates are read from the partner in "pages" (page size being gVmdirServerGlobals.replPageSize)
//  - There is a re-try (do-while) loop outside the above "for" loop (that is reading updates in pages).
//
// So, in summary, there are 4 loops in this function:
//  - An endless "while" loop executing replication cycles.
//  - A "for" loop processing this instances RAs.
//  - A do-while re-try loop
//  - A "read updates in pages" "for" loop.

// replStateGood local variable discussion:
//
// Following discussion is in the context of while processing a single Replication Agreement:
//
// A TRUE value indicates that we have successfully applied all the replication updates so far in this replication cycle
// i.e. we are in replication Good state. As soon as we see a replication failure in the current replication cycle,
// _ReplStateGood is set to FALSE, at which point we stop setting gVmdirGlobals.limitLocalUsnToBeReplicated,
// meaning we are done setting the replication local USN upper limit.
//
// Replication searches do NOT process any entries that have been updated after this limit (fix for bug# 863244), if
// non-zero.
//
// After seeing a replication failure, we finish processing remaining updates from the current replication partner,
// and then restart replication (processing same RA) from the beginning (because after a failure, replication cookies
// are not updated).

static
DWORD
vdirReplicationThrFun(
    PVOID   pArg
    )
{
//SUNG, this huge list of variables is scary.  Need to modulize.......
    int                             retVal = 0;
    LDAP *                          ld = NULL;
    LDAPMessage *                   searchRes = NULL;
    LDAPMessage *                   entry = NULL;
    LDAPControl                     syncReqCtrl = {0}; // real LDAPControl, not LBERLIB_LDAPCONTROL
    LDAPControl *                   srvCtrls[2]; // real LDAPControl, not LBERLIB_LDAPCONTROL
    VMDIR_REPLICATION_AGREEMENT *   replAgr = NULL;
    BOOLEAN                         bInReplAgrsLock = FALSE;
    BOOLEAN                         bInReplCycleDoneLock = FALSE;
    PVDIR_SCHEMA_CTX                pSchemaCtx = NULL;
    BOOLEAN                         bReplStateGood = TRUE;
    int                             i = 0;
    USN                             lastLocalUsnProcessed = 0;
    BOOLEAN                         bMoreUpdatesExpected = FALSE;
    BOOLEAN                         bReTrialNeeded = FALSE;
    char                            filterStr[ATTR_USN_CHANGED_LEN + 2 + VMDIR_MAX_USN_STR_LEN + 1];
    LDAPControl **                  searchResCtrls = NULL; // real LDAPControl, not LBERLIB_LDAPCONTROL
    int                             errCode = 0;
    BOOLEAN                         bFirstReplicationCycle = FALSE;
    PSTR                            pszDcAccountPwd = NULL;
    PSTR                            pszDcAccountOldPwd = NULL;
    BOOLEAN                         bPasswordChanged = FALSE;
    PSTR                            pszDcAccountUPN = NULL; // Don't free
    PSTR                            pszDcAccountDN = NULL; // Don't free

    while (gVmdirReplAgrs == NULL)
    {
        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            goto cleanup;
        }

        if (VmDirdGetRestoreMode())
        {
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: Done restoring the server, no partner to catch up with.");
            VmDirForceExit();
            goto cleanup;
        }

        VMDIR_LOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);
        // wait till a replication agreement is created e.g. by vdcpromo
        if (VmDirConditionWait( gVmdirGlobals.replAgrsCondition, gVmdirGlobals.replAgrsMutex ) != 0)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: VmDirConditionWait failed.");
            retVal = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR( retVal );

        }
        VMDIR_UNLOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);

        // When 1st RA is created for non-1st replica => try to perform special 1st replication cycle
        if (gVmdirReplAgrs && gVmdirServerGlobals.serverId != 1)
        {
            /*

    // For 2013, use ldap replication mode only.
    // BUGBUG BUGBUB RPC mode is NOT yet stable and will crash in steps below:
    // 1. start up ssosetup in node 1
    // 2. start up ssosetup in node 2 to join node 1 (HA mode)
    // 3. start up ssosetup in node 3 to join node 1 (HA mode)
    // repeat step 3 a couple times will eventually crash node 1 vmdir.
    // A more aggressive test setup is step 1 + 2 + run multiple nodes step 3 in parallel.
    // i.e. have, say 4 nodes, execute step 3 at the same time.

            if ((retVal = VmDirFirstReplicationCycle( pszPartnerHostName, gVmdirReplAgrs )) == 0)
            {
                VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: VmDirFirstReplicationCycle() SUCCEEDED." );

                VMDIR_LOCK_MUTEX(bInReplCycleDoneLock, gVmdirGlobals.replCycleDoneMutex);
                VmDirConditionSignal(gVmdirGlobals.replCycleDoneCondition);
                VMDIR_UNLOCK_MUTEX(bInReplCycleDoneLock, gVmdirGlobals.replCycleDoneMutex);
            }
            else
            {
                // Just log the error. and try to perform normal replication logic.
                VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: VmDirFirstReplicationCycle() FAILED. retVal = %d, "
                          "Going to try normal replication logic.", retVal );
*/
             VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: LDAP replication mode");

            bFirstReplicationCycle = TRUE;
        }
    }

    // SJ-TBD: Does the schema context need to be acquired more often? How do replication updates handle schema updates?
    if (VmDirSchemaCtxAcquire(&pSchemaCtx) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: VmDirSchemaCtxAcquire failed.");
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }


    pszDcAccountUPN = gVmdirServerGlobals.dcAccountUPN.lberbv_val;
    pszDcAccountDN = gVmdirServerGlobals.dcAccountDN.lberbv.bv_val;
    while (1)
    {
        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            goto cleanup;
        }
        VMDIR_LOG_INFO(LDAP_DEBUG_REPL, "vdirReplicationThrFun: Executing next replication cycle.");

        // purge RAs that have been marked as isDeleted = TRUE
        VmDirRemoveDeletedRAsFromCache();

        VMDIR_LOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);
        replAgr = gVmdirReplAgrs;
        VMDIR_UNLOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);

        bPasswordChanged = FALSE;
        retVal = _vdirReplicationLoadPasswords(
                    &pszDcAccountPwd,
                    &pszDcAccountOldPwd,
                    &bPasswordChanged);
        if (retVal)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: Error loading passwords from registry: %u\n", retVal);
        }

        for (; replAgr != NULL; replAgr = replAgr->next )
        {
            int         numEntriesReceived = 0;
            int         numEntriesSuccessfullyProcessed = 0;

            if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
            {
                goto cleanup;
            }

            if (replAgr->isDeleted) // skip deleted RAs
            {
                continue;
            }

            if (bPasswordChanged)
            {
                replAgr->newPasswordFailTime = 0;
                replAgr->oldPasswordFailTime = 0;
            }

            retVal = _vdirReplicationConnect(
                        pszDcAccountUPN,
                        pszDcAccountDN,
                        pszDcAccountPwd,
                        pszDcAccountOldPwd,
                        replAgr,
                        &ld);
            if (ld == NULL)
            {
                continue;
            }

            gVmdirGlobals.limitLocalUsnToBeReplicated = 0;

            do // do-while ( !replStateGood ); error re-try loop
            {
                bReplStateGood = TRUE;
                bReTrialNeeded = FALSE;
                bMoreUpdatesExpected = TRUE;

                lastLocalUsnProcessed = VmDirStringToLA( replAgr->lastLocalUsnProcessed.lberbv.bv_val, NULL, 10 );

                // Read and process pages of changes, as long as we are receiving number of entries = asked page size
                for (numEntriesReceived = gVmdirServerGlobals.replPageSize; bMoreUpdatesExpected == TRUE;)
                {
                    // Shutdown check, before doing the next search operation, possibly time consuming
                    if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
                    {
                        break;
                    }

                    // TODO, need to better handle LDAPControl memory ownership
                    // We owns syncReqCtrl.ldctl_value.bv_val in CreateSyncRequestControl call below.
                    VMDIR_SAFE_FREE_MEMORY(syncReqCtrl.ldctl_value.bv_val);
                    memset(&syncReqCtrl, 0, sizeof(syncReqCtrl));

                    if ( (retVal = CreateSyncRequestControl( replAgr, &syncReqCtrl )) != LDAP_SUCCESS)
                    {
                        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: CreateSyncRequestControl (to construct Sync "
                                  "Request control value ...) failed. Error: %d", retVal);
                        break;
                    }

                    srvCtrls[0] = &syncReqCtrl;
                    srvCtrls[1] = NULL;

                    VmDirStringNPrintFA( filterStr, sizeof(filterStr), sizeof(filterStr) -1, "%s>=%ld",
                                         ATTR_USN_CHANGED, lastLocalUsnProcessed + 1);

                    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "vdirReplicationThrFun: filter: %s", filterStr);

                    if ((retVal = ldap_search_ext_s( ld, "", LDAP_SCOPE_SUBTREE, filterStr, NULL, FALSE,
                                                     srvCtrls, NULL, NULL, gVmdirServerGlobals.replPageSize,
                                                     &searchRes )) != LDAP_SUCCESS)
                    {
                        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: ldap_search_ext_s failed. Error: %d, "
                                  "error string %s", retVal, ldap_err2string( retVal ));
                        break;
                    }

                    numEntriesReceived = ldap_count_entries( ld, searchRes );
                    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "vdirReplicationThrFun: numEntriesReceived: %d", numEntriesReceived);

                    if (numEntriesReceived > 0)
                    {
                        LDAPControl **      ctrls = NULL; // real LDAPControl, not LBERLIB_LDAPCONTROL
                        int                 entryState;

                        for (numEntriesSuccessfullyProcessed = 0, entry = ldap_first_entry( ld, searchRes );
                             entry != NULL;
                             entry = ldap_next_entry( ld, entry ) )
                        {
                            retVal = ldap_get_entry_controls( ld, entry, &ctrls );

                            if (retVal != LDAP_SUCCESS)
                            {
                                VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: ldap_get_entry_controls failed. "
                                          "Error: %d, error string %s", retVal, ldap_err2string( retVal ));
                                break;
                            }

                            assert( ctrls[0] != NULL && VmDirStringCompareA( ctrls[0]->ldctl_oid,
                                                                             LDAP_CONTROL_SYNC_STATE, TRUE ) == 0 );


                            if (ParseSyncStateControlVal( &(ctrls[0]->ldctl_value), &entryState ) != LDAP_SUCCESS)
                            {
                                VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: ParseSyncStateControlVal failed to "
                                          "read entryState");
                                ldap_controls_free( ctrls );
                                break;
                            }

                            ldap_controls_free( ctrls );

                            switch (entryState)
                            {
                                case LDAP_SYNC_ADD:
                                    if ((retVal = ReplAddEntry( pSchemaCtx, entry, bReplStateGood,
                                                                bFirstReplicationCycle )) != LDAP_SUCCESS)
                                    {
                                        VMDIR_LOG_DEBUG(VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: ReplAddEntry failed. "
                                                  "Error: %d", retVal );

                                        if (retVal == LDAP_NO_SUCH_OBJECT)
                                        {
                                            // Entering the replication "recovery" mode. This state could be caused by:
                                            // - out-of-sequence parent-child updates arrival OR
                                            // - the conflict (parent deleted on one replica, child added on another)
                                            //
                                            // In both the cases, we do the re-trials, when in the 2nd scenario it is
                                            // not really needed, because re-trials are not going to help.
                                            //
                                            // - Stop updating gVmdirServerGlobals.limitLocalUsnToBeReplicated, which
                                            //   is the limit of local usnChanged that other replicas will see from this
                                            //   replica.
                                            // - Replicate the remaining pages, and start again from where the last
                                            //   (success) cookie was saved.

                                            VMDIR_LOG_DEBUG(VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: During Add, encountered "
                                                      "Parent Object does not exist error , entering replication "
                                                      "recovery Mode (re-trial)" );

                                            bReplStateGood = FALSE;
                                        }
                                    }
                                    break;
                                case LDAP_SYNC_MODIFY:
                                    if ((retVal = ReplModifyEntry( pSchemaCtx, entry, bReplStateGood,
                                                                   &pSchemaCtx )) != LDAP_SUCCESS)
                                    {
                                        VMDIR_LOG_DEBUG(VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: ReplModifyEntry failed. "
                                                  "Error: %d", retVal );
                                    }
                                    break;
                                case LDAP_SYNC_DELETE:
                                    if ((retVal = ReplDeleteEntry( pSchemaCtx, entry, bReplStateGood )) != LDAP_SUCCESS)
                                    {
                                        VMDIR_LOG_DEBUG(VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: ReplDeleteEntry failed. "
                                                  "Error: %d", retVal );
                                    }
                                    break;
                                default:
                                    retVal = LDAP_OPERATIONS_ERROR;
                                    break;
                            }
                            if (retVal == LDAP_SUCCESS)
                            {
                                numEntriesSuccessfullyProcessed++;
                            }
                        } // Loop to process entries within the page

                        VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "vdirReplicationThrFun: numEntriesProcessed: %d",
                                  numEntriesSuccessfullyProcessed );
                    } // not an empty page.

                    // if we received full page, there are possibly more updates to be replicated
                    // Note: Even if we hit the page boundary scenario, i.e. don't get any updates while reading the
                    // next page, the logic should still work (because in this case supplier returns back exactly same
                    // replication cookie as it gets from the consumer), except we are doing one extra page search request.
                    bMoreUpdatesExpected = (numEntriesReceived == gVmdirServerGlobals.replPageSize);

                    bReTrialNeeded = (bReplStateGood == FALSE);

                    retVal = ldap_parse_result( ld, searchRes, &errCode, NULL, NULL, NULL, &searchResCtrls, 0 );
                    if (retVal != 0)
                    {
                        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: ldap_parse_result: %u errCode: %u", retVal, errCode);
                        BAIL_ON_VMDIR_ERROR( retVal );
                    }

                    if (searchResCtrls[0] == NULL)
                    {
                        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: ldap_parse_result returned empty ctrl");
                        retVal = LDAP_OPERATIONS_ERROR;
                        BAIL_ON_VMDIR_ERROR( retVal );
                    }
                    else if (VmDirStringCompareA (searchResCtrls[0]->ldctl_oid, LDAP_CONTROL_SYNC_DONE, TRUE) != 0)
                    {
                        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: ctrl oid is wrong");
                        retVal = LDAP_OPERATIONS_ERROR;
                        BAIL_ON_VMDIR_ERROR( retVal );
                    }


                    // Replication cycle done
                    if ( bMoreUpdatesExpected == FALSE && bReTrialNeeded == FALSE )
                    {
                        if ((retVal = VmDirReplUpdateCookies( pSchemaCtx, &(searchResCtrls[0]->ldctl_value),
                                                              replAgr )) != LDAP_SUCCESS)
                        {
                            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: UpdateCookies failed. Error: %d", retVal);
                        }
                    }
                    else
                    { // Get last local USN processed from the cookie (searchResCtrls[0]->ldctl_value.bv_val)
                        VDIR_BERVALUE   bvLastLocalUsnProcessed = VDIR_BERVALUE_INIT;

                        bvLastLocalUsnProcessed.lberbv.bv_val = searchResCtrls[0]->ldctl_value.bv_val;
                        bvLastLocalUsnProcessed.lberbv.bv_len = VmDirStringChrA(bvLastLocalUsnProcessed.lberbv.bv_val, ',')
                                                                - bvLastLocalUsnProcessed.lberbv.bv_val;
                        // Note: We are effectively over-writing in searchResCtrls[0]->ldctl_value.bv_val here,
                        // which should be ok.
                        bvLastLocalUsnProcessed.lberbv.bv_val[bvLastLocalUsnProcessed.lberbv.bv_len] = '\0';

                        lastLocalUsnProcessed = VmDirStringToLA( bvLastLocalUsnProcessed.lberbv.bv_val, NULL, 10 );
                    }

                    ldap_controls_free( searchResCtrls );
                    ldap_msgfree( searchRes );

                    if (retVal != LDAP_SUCCESS)
                    {
                        break;
                    }

                } // For loop to process paged results

            } while ( bReTrialNeeded );

            // Now, let other replicas see everything on this replica.
            gVmdirGlobals.limitLocalUsnToBeReplicated = 0;

            VDIR_SAFE_UNBIND_EXT_S( ld );

        } // For loop to process replication agreements

        VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "vdirReplicationThrFun: Done executing the replication cycle.");

        bFirstReplicationCycle = FALSE;

        VMDIR_LOCK_MUTEX(bInReplCycleDoneLock, gVmdirGlobals.replCycleDoneMutex);
        VmDirConditionSignal(gVmdirGlobals.replCycleDoneCondition);
        VMDIR_UNLOCK_MUTEX(bInReplCycleDoneLock, gVmdirGlobals.replCycleDoneMutex);

        if (VmDirdGetRestoreMode())
        {
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
                "vdirReplicationThrFun: Done restoring the server by catching up with it's replication partner(s)." );
            VmDirForceExit();
            break;
        }

        VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "vdirReplicationThrFun: Sleeping for the replication interval: %d seconds.",
                  gVmdirServerGlobals.replInterval );

        for (i=0; i<gVmdirServerGlobals.replInterval; i++)
        {
            if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
            {
                goto cleanup;
            }
            if (VmDirdGetReplNow() == TRUE)
            { // Break from finishing sleeping for replication interval

                VmDirdSetReplNow(FALSE);
                break;
            }
            VmDirSleep( 1000 ); // sleep for 1000 msecs (1 sec).
        }
    } // Endless replication loop

cleanup:
    VMDIR_SECURE_FREE_STRINGA(pszDcAccountPwd);
    VMDIR_SECURE_FREE_STRINGA(pszDcAccountOldPwd);

    VMDIR_UNLOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);
    VMDIR_UNLOCK_MUTEX(bInReplCycleDoneLock, gVmdirGlobals.replCycleDoneMutex);

    VMDIR_SAFE_FREE_MEMORY(syncReqCtrl.ldctl_value.bv_val);
    VmDirSchemaCtxRelease(pSchemaCtx);

    // TODO: should we return dwError?
    return 0;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun exiting with error (%u)", retVal);

    goto cleanup;
}

DWORD
VmDirCacheKrb5Creds(
    PCSTR pszDcAccountUPN,
    PCSTR pszDcAccountPwd)
{
    krb5_error_code             dwError = 0;
    krb5_context                pKrb5Ctx = NULL;
    krb5_creds                  myCreds = {0};
    krb5_creds                  credsToMatch = {0};
    krb5_principal              pKrb5TgtPrincipal = NULL;
    krb5_principal              pKrb5DCAccountPrincipal = NULL;
    krb5_ccache                 pDefCredCache = NULL;
    krb5_get_init_creds_opt *   pOptions = NULL; // Not really used right now.
    PSTR                        pszLocalErrorMsg = NULL;
    PCSTR                       pKrb5ErrMsg = NULL;
    PSTR                        pszTgtUPN = NULL;
    time_t                      currentTimeInSecs = 0;
    krb5_keytab                 keyTabHandle = NULL;

    if (pszDcAccountUPN == NULL ||
        pszDcAccountPwd == NULL)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = krb5_init_context(&pKrb5Ctx);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                            "VmDirCacheKrb5Creds: %s failed. krb5ErrCode = %d, krb5ErrMsg = %s", "krb5_init_context()",
                            dwError, (pKrb5ErrMsg = krb5_get_error_message(pKrb5Ctx, dwError)) );

    dwError = krb5_get_init_creds_opt_alloc(pKrb5Ctx, &pOptions);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                "VmDirCacheKrb5Creds: %s failed. krb5ErrCode = %d, krb5ErrMsg = %s", "krb5_get_init_creds_opt_alloc()",
                dwError, (pKrb5ErrMsg = krb5_get_error_message(pKrb5Ctx, dwError)) );

    dwError = krb5_parse_name(pKrb5Ctx, pszDcAccountUPN, &pKrb5DCAccountPrincipal);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                             "VmDirCacheKrb5Creds: %s failed. krb5ErrCode = %d, krb5ErrMsg = %s", "krb5_parse_name()",
                              dwError, (pKrb5ErrMsg = krb5_get_error_message(pKrb5Ctx, dwError)) );

    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "VmDirKerberosBasedBind: DCAccountPrinicpal=(%s)",
                                       VDIR_SAFE_STRING(pszDcAccountUPN));

    dwError = krb5_cc_default(pKrb5Ctx, &pDefCredCache);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                                  "VmDirCacheKrb5Creds: %s failed. krb5ErrCode = %d, krb5ErrMsg = %s", "krb5_cc_default()",
                                  dwError, (pKrb5ErrMsg = krb5_get_error_message(pKrb5Ctx, dwError)) );

    VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "VmDirKerberosBasedBind: credCache = %s",
                                       VDIR_SAFE_STRING(krb5_cc_get_name(pKrb5Ctx, pDefCredCache)));

    // non-1st replica, before 1st replication cycle is over scenario
    // => credCache has not yet been initialized with the creds for the DC machine account => initialize it
    if (gVmdirKrbGlobals.pszRealm == NULL || gVmdirKrbGlobals.bTryInit)
    {
        gVmdirKrbGlobals.bTryInit = FALSE;

        dwError = krb5_get_init_creds_password(
                        pKrb5Ctx,                           // [in] context - Library context
                        &myCreds,                           // [out] creds - New credentials
                        pKrb5DCAccountPrincipal,            // [in] client - Client principal
                        (PSTR)pszDcAccountPwd,              // [in] password - Password (or NULL)
                        NULL,                               // [in] prompter - Prompter function
                        0,                                  // [in] data - Prompter callback data
                        0,                                  // [in] start_time - Time when ticket becomes valid (0 for now)
                        NULL,                               // [in] in_tkt_service - Service name of initial credentials (or NULL)
                        pOptions);                          // [in] k5_gic_options - Initial credential options
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                  "VmDirCacheKrb5Creds: %s failed. krb5ErrCode = %d, krb5ErrMsg = %s", "krb5_get_init_creds_password()",
                  dwError, (pKrb5ErrMsg = krb5_get_error_message(pKrb5Ctx, dwError)) );

        dwError = krb5_cc_initialize(pKrb5Ctx, pDefCredCache,
                                     myCreds.client /* This is canonical, otherwise use pKrb5DCAccountPrincipal */);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                          "VmDirCacheKrb5Creds: %s failed. krb5ErrCode = %d, krb5ErrMsg = %s", "krb5_cc_initialize()",
                          dwError, (pKrb5ErrMsg = krb5_get_error_message(pKrb5Ctx, dwError)) );

        dwError = krb5_cc_store_cred(pKrb5Ctx, pDefCredCache, &myCreds);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                          "VmDirCacheKrb5Creds: %s failed. krb5ErrCode = %d, krb5ErrMsg = %s", "krb5_cc_store_cred()",
                          dwError, (pKrb5ErrMsg = krb5_get_error_message(pKrb5Ctx, dwError)) );

        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "First init creds cache, UPN(%s), size (%d) passed",
                                            VDIR_SAFE_STRING( pszDcAccountUPN ),
                                            pszDcAccountPwd ? VmDirStringLenA(pszDcAccountPwd) : 0);
    }
    else
    {
        // Try to retrieve creds for the DC machine account
        memset(&credsToMatch, 0, sizeof(credsToMatch));
        credsToMatch.client = pKrb5DCAccountPrincipal;

        dwError = VmDirAllocateStringAVsnprintf(&pszTgtUPN, "krbtgt/%s@%s",
                                                gVmdirKrbGlobals.pszRealm, gVmdirKrbGlobals.pszRealm);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "creadsToMatch = (%s)", pszTgtUPN);

        dwError = krb5_parse_name(pKrb5Ctx, pszTgtUPN, &pKrb5TgtPrincipal);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                              "VmDirCacheKrb5Creds: %s failed. krb5ErrCode = %d, krb5ErrMsg = %s", "krb5_parse_name()",
                              dwError, (pKrb5ErrMsg = krb5_get_error_message(pKrb5Ctx, dwError)) );

        credsToMatch.server = pKrb5TgtPrincipal;
        currentTimeInSecs = time (NULL);

        dwError = krb5_cc_retrieve_cred(pKrb5Ctx, pDefCredCache, KRB5_TC_MATCH_SRV_NAMEONLY, &credsToMatch, &myCreds);
        if (dwError == KRB5_FCC_NOFILE
            ||
            dwError == KRB5_CC_NOTFOUND
            ||
            (dwError == 0 && ( (currentTimeInSecs - myCreds.times.starttime) > (myCreds.times.endtime - myCreds.times.starttime)/2 ) ) )
        {
            BOOLEAN     bLogKeytabInit = dwError;

            dwError = krb5_kt_default(pKrb5Ctx, &keyTabHandle);
            BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                      "VmDirCacheKrb5Creds: %s failed. krb5ErrCode = %d, krb5ErrMsg = %s", "krb5_kt_default()",
                      dwError, (pKrb5ErrMsg = krb5_get_error_message(pKrb5Ctx, dwError)) );

            dwError = krb5_get_init_creds_keytab(
                            pKrb5Ctx,                   // [in] context - Library context
                            &myCreds,                   // [out] creds - New credentials
                            pKrb5DCAccountPrincipal,    // [in] client - Client principal
                            keyTabHandle,               // [in] Key table handle
                            0,                          // [in] start_time - Time when ticket becomes valid (0 for now)
                            NULL,                       // [in] in_tkt_service - Service name of initial credentials (or NULL)
                            pOptions);                  // [in] k5_gic_options - Initial credential options
            if (dwError == KRB5KDC_ERR_PREAUTH_FAILED)
            {
                gVmdirKrbGlobals.bTryInit = TRUE;
            }
            BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                  "VmDirCacheKrb5Creds: %s failed. krb5ErrCode = %d, krb5ErrMsg = %s", "krb5_get_init_creds_keytab()",
                  dwError, (pKrb5ErrMsg = krb5_get_error_message(pKrb5Ctx, dwError)) );

            dwError = krb5_cc_initialize(pKrb5Ctx, pDefCredCache,
                                         myCreds.client /* This is canonical, otherwise use pKrb5DCAccountPrincipal */);
            BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                      "VmDirCacheKrb5Creds: %s failed. krb5ErrCode = %d, krb5ErrMsg = %s", "krb5_cc_initialize()",
                      dwError, (pKrb5ErrMsg = krb5_get_error_message(pKrb5Ctx, dwError)) );

            dwError = krb5_cc_store_cred(pKrb5Ctx, pDefCredCache, &myCreds);
            BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                      "VmDirCacheKrb5Creds: %s failed. krb5ErrCode = %d, krb5ErrMsg = %s", "krb5_cc_store_cred()",
                      dwError, (pKrb5ErrMsg = krb5_get_error_message(pKrb5Ctx, dwError)) );

            if ( bLogKeytabInit )
            {
                VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Init creds cache via keytab, UPN(%s) passed.",
                                VDIR_SAFE_STRING( pszDcAccountUPN ));
            }
        }
        else
        {
            BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                  "VmDirCacheKrb5Creds: %s failed. krb5ErrCode = %d, krb5ErrMsg = %s", "krb5_cc_retrieve_cred()",
                  dwError, (pKrb5ErrMsg = krb5_get_error_message(pKrb5Ctx, dwError)) );
        }
    }

cleanup:
    if (pKrb5Ctx)
    {
        if (pKrb5TgtPrincipal)
        {
            krb5_free_principal(pKrb5Ctx, pKrb5TgtPrincipal);
        }
        if (pKrb5DCAccountPrincipal)
        {
            krb5_free_principal(pKrb5Ctx, pKrb5DCAccountPrincipal);
        }
        if (pDefCredCache)
        {
            krb5_cc_close(pKrb5Ctx, pDefCredCache);
        }
        if (pOptions)
        {
            krb5_get_init_creds_opt_free(pKrb5Ctx, pOptions);
        }
        if (pKrb5ErrMsg)
        {
            krb5_free_error_message(pKrb5Ctx, pKrb5ErrMsg);
        }
        if (keyTabHandle)
        {
            krb5_kt_close(pKrb5Ctx, keyTabHandle); // SJ-TBD: Do I really need to do that?
        }
        krb5_free_cred_contents( pKrb5Ctx, &myCreds );
        krb5_free_context(pKrb5Ctx);
    }
    VMDIR_SAFE_FREE_MEMORY(pszTgtUPN);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);

    return dwError;

error:
    // SRP should work and KRB needs DNS.  So, makes this verbose for 2015 to reduce log noise.
    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "%s", VDIR_SAFE_STRING(pszLocalErrorMsg));
    goto cleanup;
}

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

static int
ReplAddEntry(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    LDAPMessage *       ldapMsg,
    BOOLEAN             replStateGood,
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

    if (replStateGood)
    {
        gVmdirGlobals.limitLocalUsnToBeReplicated = localUsn;
    }

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
        retVal = _VmDirAssignEntryIdIfSpecialInternalEntry( pEntry );
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    if ((retVal = VmDirInternalAddEntry( &op )) != LDAP_SUCCESS)
    {
        // If VmDirInternall call failed, reset retVal to LDAP level error space (for B/C)
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

cleanup:
    // pAttrAttrMetaData is local, needs to be freed within the call
    VmDirFreeAttribute( pAttrAttrMetaData );

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
static int
ReplDeleteEntry(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    LDAPMessage *       ldapMsg,
    BOOLEAN             replStateGood)
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

    if (VmDirBervalContentDup( &tmpAddOp.reqDn, &mr->dn ) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "ReplDeleteEntry: BervalContentDup failed." );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    delOp.pBEIF = VmDirBackendSelect(mr->dn.lberbv.bv_val);
    assert(delOp.pBEIF);

    // SJ-TBD: What about if one or more attributes were meanwhile added to the entry? How do we purge them?
    retVal = SetupReplModifyRequest( &delOp, tmpAddOp.request.addReq.pEntry, replStateGood );
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
static int
ReplModifyEntry(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    LDAPMessage *       ldapMsg,
    BOOLEAN             replStateGood,
    PVDIR_SCHEMA_CTX*   ppOutSchemaCtx)
{
    int                 retVal = LDAP_SUCCESS;
    VDIR_OPERATION      tmpAddOp = {0};
    VDIR_OPERATION      modOp = {0};
    ModifyReq *         mr = &(modOp.request.modifyReq);
    int                 dbRetVal = 0;
    BOOLEAN             bHasTxn = FALSE;
    int                 deadLockRetries = 0;
    PVDIR_SCHEMA_CTX    pUpdateSchemaCtx = NULL;

    retVal = VmDirInitStackOperation( &modOp,
                                      VDIR_OPERATION_TYPE_REPL,
                                      LDAP_REQ_MODIFY,
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

    if (VmDirBervalContentDup( &tmpAddOp.reqDn, &mr->dn ) != 0)
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

    if ((retVal = SetupReplModifyRequest( &modOp, tmpAddOp.request.addReq.pEntry, replStateGood )) != LDAP_SUCCESS)
    {
        switch (retVal)
        {
            case LDAP_NO_SUCH_OBJECT:
                VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
                          "ReplModifyEntry/SetupReplModifyRequest: %d (Object does not exist). "
                          "DN: %s, first attribute: %s, it's meta data: '%s'. "
                          "Possible replication CONFLICT. Object will get deleted from the system.",
                          retVal, tmpAddOp.reqDn.lberbv.bv_val, tmpAddOp.request.addReq.pEntry->attrs[0].type.lberbv.bv_val,
                          tmpAddOp.request.addReq.pEntry->attrs[0].metaData );
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

    if ((modOp.request.modifyReq.dn.bvnorm_val != NULL)     &&
        VmDirStringCompareA(modOp.request.modifyReq.dn.bvnorm_val, SUB_SCHEMA_SUB_ENTRY_DN, FALSE) == 0)
    {   // schema entry updated, refresh replication schema ctx.
        assert( ppOutSchemaCtx );
        retVal = VmDirSchemaCtxAcquire(&pUpdateSchemaCtx);
        BAIL_ON_VMDIR_ERROR(retVal);
        *ppOutSchemaCtx = pUpdateSchemaCtx;

        VmDirSchemaCtxRelease(pSchemaCtx);
    }

cleanup:
    VmDirFreeOperationContent(&modOp);
    VmDirFreeOperationContent(&tmpAddOp);

    return retVal;

error:
    if (bHasTxn)
    {
        modOp.pBEIF->pfnBETxnAbort( modOp.pBECtx );
    }
    VmDirSchemaCtxRelease(pUpdateSchemaCtx);

    goto cleanup;
} // Replicate Modify Entry operation

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
            // Format is: <local USN>:<version no>:<originating server ID>:<originating time>:<originating USN>
            int supplierVersionNum = VmDirStringToIA(strchr(metaData, ':') + 1);
            int consumerVersionNum = VmDirStringToIA(strchr(pAttr->metaData, ':') + 1);

            if (supplierVersionNum > consumerVersionNum)
            {
                VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "DetectAndResolveAttrsConflicts: No replication conflict: "
                          "During Modify, supplier attribute has version # > consumer attribute version #, DN: %s, "
                          "attribute: %s, supplier attribute meta data: %s, consumer attribute meta data: %s",
                           pDn->lberbv.bv_val, pAttr->type.lberbv.bv_val, metaData, pAttr->metaData );
            }
            else if (supplierVersionNum < consumerVersionNum)
            {
                VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL, "DetectAndResolveAttrsConflicts: Possible replication conflict: "
                          "During Modify, supplier attribute has version # < consumer attribute version #, "
                          " => SUPPLIER attribute meta data LOSES to CONSUMER attribute meta data. DN: %s, "
                          "attribute: %s, supplier attribute meta data: %s, consumer attribute meta data: %s",
                           pDn->lberbv.bv_val, pAttr->type.lberbv.bv_val, metaData, pAttr->metaData );

                // Supplier meta-data loses. pAttrAttrSupplierMetaData->vals[i].lberbv.bv_val conatins just the attribute name
                // followed by a ':' now, and no associated meta data.
                *metaData = '\0';
                pAttrAttrSupplierMetaData->vals[i].lberbv.bv_len = strlen(pAttrAttrSupplierMetaData->vals[i].lberbv.bv_val);
            }
            else // supplierVersionNum = consumerVersionNum, compare serverIds, lexicographically larger one wins
            {
                // Format is: <local USN>:<version no>:<originating server ID>:<originating time>:<originating USN>
                char * supplierInvocationId = strchr(strchr(metaData, ':') + 1, ':') + 1;
                char * consumerInvocationId = strchr(strchr(pAttr->metaData, ':') + 1, ':') + 1;

                if (strncmp( supplierInvocationId, consumerInvocationId, VMDIR_GUID_STR_LEN ) < 0)
                {
                    VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL, "DetectAndResolveAttrsConflicts: Possible replication conflict: "
                              "During Modify, supplier attribute has version # = consumer attribute version #, "
                              "Supplier serverId is < (lexicographically) consumer serverId "
                              "=> SUPPLIER attribute meta data LOSES to CONSUMER attribute meta data. DN: %s, "
                              "attribute: %s, supplier attribute meta data: %s, consumer attribute meta data: %s",
                               pDn->lberbv.bv_val, pAttr->type.lberbv.bv_val, metaData, pAttr->metaData );

                    // Supplier meta-data loses. pAttrAttrSupplierMetaData->vals[i].lberbv.bv_val conatins just the attribute
                    // name followed by a ':' now, and no associated meta data.
                    *metaData = '\0';
                    pAttrAttrSupplierMetaData->vals[i].lberbv.bv_len = strlen(pAttrAttrSupplierMetaData->vals[i].lberbv.bv_val);
                }
                else
                {
                    VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL, "DetectAndResolveAttrsConflicts: Possible replication conflict: "
                              "During Modify, supplier attribute has version # = consumer attribute version #, "
                              "Supplier serverId is >= (lexicographically) consumer serverId "
                              "=> SUPPLIER attribute meta data WINS against CONSUMER attribute meta data. DN: %s, "
                              "attribute: %s, supplier attribute meta data: %s, consumer attribute meta data: %s",
                               pDn->lberbv.bv_val, pAttr->type.lberbv.bv_val, metaData, pAttr->metaData );
                }
            }
        }
        VmDirFreeAttribute( pAttr );
        pAttr = NULL;
    }

cleanup:
    VmDirFreeAttribute( pAttr );
    return retVal;

ldaperror:
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
            assert( metaData != NULL);

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
    BOOLEAN             replStateGood)
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

    if (replStateGood)
    {
        gVmdirGlobals.limitLocalUsnToBeReplicated = localUsn;
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

    retVal = SetAttributesNewMetaData( pOperation, pEntry, localUsnStr, &pAttrAttrMetaData);
    BAIL_ON_VMDIR_ERROR( retVal );


    for (currAttr = pEntry->attrs; currAttr; currAttr = currAttr->next)
    {
        // Not a loser attribute meta data. Skip attributes that have loser attribute meta data => no mods for them
        if (currAttr->metaData[0] != '\0')
        {
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
    }

    // Create Delete mods
    for (i = 0; pAttrAttrMetaData->vals[i].lberbv.bv_val != NULL; i++)
    {
        // Not used meta data
        if (pAttrAttrMetaData->vals[i].lberbv.bv_len != 0)
        {
            char * metaData = VmDirStringChrA( pAttrAttrMetaData->vals[i].lberbv.bv_val, ':');
            assert( metaData != NULL);

            // not a loser meta data
            if (*(metaData + 1 /* skip ':' */) != '\0')
            {
                // => over-write ':', pAttrAttrMetaData->vals[i].lberbv.bv_val points to be attribute name now
                *metaData++ = '\0';

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
        }
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

static
DWORD
VmDirSrvCreateReplAgrObj(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    PCSTR            pszReplURI,
    PCSTR            pszLastLocalUsnProcessed,
    PSTR             pszReplAgrDN
    )
{
    DWORD dwError = 0;
    PSTR  ppszReplAgrObjAttrs[] =
    {
            ATTR_OBJECT_CLASS,                  OC_REPLICATION_AGREEMENT,
            ATTR_OBJECT_CLASS,                  OC_TOP,
            ATTR_LABELED_URI,                   (PSTR) pszReplURI,
            ATTR_LAST_LOCAL_USN_PROCESSED,      (PSTR) pszLastLocalUsnProcessed,
            NULL
    };

    dwError = VmDirSimpleEntryCreate(pSchemaCtx, ppszReplAgrObjAttrs, pszReplAgrDN, 0);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}

int
VmDirReplUpdateCookies(
    PVDIR_SCHEMA_CTX                pSchemaCtx,
    struct berval *                 syncDoneCtrlVal,
    VMDIR_REPLICATION_AGREEMENT *   replAgr)
{
    int             retVal = LDAP_SUCCESS;
    VDIR_BERVALUE   bvLastLocalUsnProcessed = VDIR_BERVALUE_INIT;
    VDIR_BERVALUE   utdVector = VDIR_BERVALUE_INIT;

    // Update (both in memory and on disk) lastLocalUsnProcessed in the replication agreement, and the
    // utd vector in the server object.

    bvLastLocalUsnProcessed.lberbv.bv_val = syncDoneCtrlVal->bv_val;
    bvLastLocalUsnProcessed.lberbv.bv_len =
                        VmDirStringChrA(bvLastLocalUsnProcessed.lberbv.bv_val, ',') - bvLastLocalUsnProcessed.lberbv.bv_val;
    // Note: We are effectively over-writing in ctrls[0]->ldctl_value.bv_val here, which should be ok.
    bvLastLocalUsnProcessed.lberbv.bv_val[bvLastLocalUsnProcessed.lberbv.bv_len] = '\0';

    utdVector.lberbv.bv_val = syncDoneCtrlVal->bv_val + bvLastLocalUsnProcessed.lberbv.bv_len + 1;
    utdVector.lberbv.bv_len = syncDoneCtrlVal->bv_len - bvLastLocalUsnProcessed.lberbv.bv_len - 1;

    // if lastLocalUsnProcessed is different
    if (strcmp(bvLastLocalUsnProcessed.lberbv.bv_val, replAgr->lastLocalUsnProcessed.lberbv.bv_val) != 0)
    {
        VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "VmDirReplUpdateCookies: Replication cycle done. Updating cookies" );;

        // Update disk copy of utdVector
        retVal = UpdateServerObject( pSchemaCtx, &utdVector, replAgr );
        BAIL_ON_VMDIR_ERROR( retVal );

        // Update memory copy of utdVector
        VmDirFreeBervalContent( &gVmdirServerGlobals.utdVector );
        if (VmDirBervalContentDup( &utdVector, &gVmdirServerGlobals.utdVector ) != 0)
        {
            VMDIR_LOG_ERROR(LDAP_DEBUG_ANY, "VmDirReplUpdateCookies: BervalContentDup failed." );
            retVal = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR( retVal );
        }

        // Update disk copy of lastLocalUsnProcessed
        retVal = UpdateReplicationAgreement( pSchemaCtx, replAgr, &bvLastLocalUsnProcessed );
        BAIL_ON_VMDIR_ERROR( retVal );

        // Update memory copy of lastLocalUsnProcessed
        VmDirFreeBervalContent( &replAgr->lastLocalUsnProcessed );
        if (VmDirBervalContentDup( &bvLastLocalUsnProcessed, &replAgr->lastLocalUsnProcessed ) != 0)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirReplUpdateCookies: BervalContentDup failed." );
            retVal = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR( retVal );
        }
    }
    else
    {
        VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL, "VmDirReplUpdateCookies: Replication cycle done.. Skipping updating Replication cookies.");
    }
    retVal = LDAP_SUCCESS;

cleanup:
    return retVal;

error:
    goto cleanup;
}

static
int
UpdateReplicationAgreement(
    PVDIR_SCHEMA_CTX                pSchemaCtx,
    VMDIR_REPLICATION_AGREEMENT *   replAgr,
    VDIR_BERVALUE *                 lastLocalUsnProcessed)
{
    // Load my Replication Agreements
    int                  retVal = LDAP_SUCCESS;
    VDIR_OPERATION       op = {0};
    VDIR_MODIFICATION *  mod = NULL;
    VDIR_BERVALUE        attrLLUsnProcessed = {
                            {ATTR_LAST_LOCAL_USN_PROCESSED_LEN, ATTR_LAST_LOCAL_USN_PROCESSED}, 0, 0, NULL };
    VDIR_BERVALUE *      vals;

    retVal = VmDirInitStackOperation( &op,
                                      VDIR_OPERATION_TYPE_INTERNAL,
                                      LDAP_REQ_MODIFY,
                                      pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(retVal);

    if (VmDirBervalContentDup( &replAgr->dn, &op.reqDn ) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "UpdateReplicationAgreement: BervalContentDup failed." );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    op.pBEIF = VmDirBackendSelect(op.reqDn.lberbv.bv_val);
    assert(op.pBEIF);

    if (VmDirAllocateMemory( sizeof ( VDIR_MODIFICATION ), (PVOID *)&mod ) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "UpdateReplicationAgreement: VmDirAllocateMemory failed." );
        BAIL_ON_VMDIR_ERROR( retVal );
        retVal = LDAP_OPERATIONS_ERROR;
    }
    if (VmDirAllocateMemory( 2 * sizeof( VDIR_BERVALUE ), (PVOID *)&vals ) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "UpdateReplicationAgreement: VmDirAllocateMemory failed." );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    if (VmDirBervalContentDup( &op.reqDn, &op.request.modifyReq.dn ) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "UpdateReplicationAgreement: BervalContentDup failed." );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }
    op.request.modifyReq.mods = mod;
    op.request.modifyReq.numMods = 1;

    mod->operation = MOD_OP_REPLACE;
    mod->attr.type = attrLLUsnProcessed;
    mod->attr.vals = vals;
    vals[0] = *lastLocalUsnProcessed;
    vals[1].lberbv.bv_val = NULL;
    vals[1].lberbv.bv_len = 0;
    mod->attr.numVals = 1;

    if ((retVal = VmDirInternalModifyEntry( &op )) != 0)
    {
        // If VmDirInternall call failed, reset retVal to LDAP level error space (for B/C)
        retVal = op.ldapResult.errCode;

        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "UpdateReplicationAgreement: InternalModifyEntry failed. "
                  "Error code: %d, Error string: %s", retVal, VDIR_SAFE_STRING( op.ldapResult.pszErrMsg ) );
        BAIL_ON_VMDIR_ERROR( retVal );
    }

cleanup:
    VmDirFreeOperationContent(&op);

    return retVal;

error:
    goto cleanup;
}

static
int
UpdateServerObject(
    PVDIR_SCHEMA_CTX                pSchemaCtx,
    VDIR_BERVALUE *                 utdVector,
    VMDIR_REPLICATION_AGREEMENT *   replAgr)
{
    int                      retVal = LDAP_SUCCESS;
    VDIR_OPERATION           op = {0};
    VDIR_MODIFICATION *      mod = NULL;
    VDIR_BERVALUE            attrUtdVector = { {ATTR_UP_TO_DATE_VECTOR_LEN, ATTR_UP_TO_DATE_VECTOR}, 0, 0, NULL };
    VDIR_BERVALUE *          vals = NULL;
    VDIR_BERVALUE            utdVectorCopy = VDIR_BERVALUE_INIT;

    retVal = VmDirInitStackOperation( &op,
                                      VDIR_OPERATION_TYPE_INTERNAL,
                                      LDAP_REQ_MODIFY,
                                      pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(retVal);

    if (VmDirAllocateMemory( sizeof( VDIR_MODIFICATION ), (PVOID *)&mod ) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "UpdateServerObject: VmDirAllocateMemory failed." );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }
    if (VmDirAllocateMemory( 2 * sizeof( VDIR_BERVALUE ), (PVOID *)&vals ) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "UpdateServerObject: VmDirAllocateMemory failed." );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    if (VmDirBervalContentDup( &gVmdirServerGlobals.serverObjDN, &op.reqDn ) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "UpdateServerObject: BervalContentDup failed." );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    op.pBEIF = VmDirBackendSelect(op.reqDn.lberbv.bv_val);
    assert(op.pBEIF);

    if (VmDirBervalContentDup( &op.reqDn, &op.request.modifyReq.dn ) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "UpdateServerObject: BervalContentDup failed." );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    op.request.modifyReq.mods = mod;
    op.request.modifyReq.numMods = 1;

    mod->operation = MOD_OP_REPLACE;
    mod->attr.type = attrUtdVector;
    mod->attr.vals = vals;

    if (VmDirBervalContentDup( utdVector, &utdVectorCopy ) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "UpdateServerObject: BervalContentDup failed." );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    vals[0] = utdVectorCopy;
    vals[1].lberbv.bv_val = NULL;
    vals[1].lberbv.bv_len = 0;
    mod->attr.numVals = 1;

    if ((retVal = VmDirInternalModifyEntry( &op )) != 0)
    {
        // If VmDirInternall call failed, reset retVal to LDAP level error space (for B/C)
        retVal = op.ldapResult.errCode;

        if (retVal == LDAP_NO_SUCH_OBJECT)
        {
            // 1st replication cycle case. Perform some after-1st-replication-cycle server initialization tasks.
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "UpdateServerObject: Creating initial server objects");

            // Update global server ID and update maxServerId in the system domain object

            retVal = VmDirSetGlobalServerId();
            BAIL_ON_VMDIR_ERROR(retVal);

            // Update gVmdirKrbGlobals

            retVal = VmDirKrbInit();
            BAIL_ON_VMDIR_ERROR(retVal);

            // Create THE Server object

            retVal = VmDirSrvCreateServerObj(op.pSchemaCtx);
            BAIL_ON_VMDIR_ERROR(retVal);

            // Create Replication Agreements container

            retVal = VmDirSrvCreateReplAgrsContainer(op.pSchemaCtx);
            BAIL_ON_VMDIR_ERROR(retVal);

            // Create this Replication agreement

            retVal = VmDirSrvCreateReplAgrObj( op.pSchemaCtx, replAgr->ldapURI,
                                               replAgr->lastLocalUsnProcessed.lberbv.bv_val, replAgr->dn.lberbv.bv_val);
            BAIL_ON_VMDIR_ERROR(retVal);

            // re-try updating the server object.

            VMDIR_SAFE_FREE_MEMORY(op.ldapResult.pszErrMsg); // Free the previous error message string
            retVal = VmDirInternalModifyEntry( &op );
            if ( retVal )
            {
                // If VmDirInternall call failed, reset retVal to LDAP level error space (for B/C)
                retVal = op.ldapResult.errCode;
            }
            BAIL_ON_VMDIR_ERROR(retVal);
        }
        else
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "UpdateServerObject: InternalModifyEntry failed. "
                  "Error code: %d, Error string: %s", retVal, VDIR_SAFE_STRING( op.ldapResult.pszErrMsg ) );

            BAIL_ON_VMDIR_ERROR(retVal);
        }
    }

cleanup:
    VmDirFreeOperationContent(&op);

    return retVal;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "UpdateServerObject failed (%u)", retVal);

    goto cleanup;
}

// Update global server ID

static
DWORD
VmDirSetGlobalServerId()
{
    DWORD           dwError = 0;
    PVDIR_ENTRY     pEntry = NULL;
    PVDIR_ATTRIBUTE pMaxServerId = NULL;

    dwError = VmDirSimpleDNToEntry(gVmdirServerGlobals.systemDomainDN.bvnorm_val, &pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    pMaxServerId = VmDirEntryFindAttribute( ATTR_MAX_SERVER_ID, pEntry );
    assert( pMaxServerId != NULL );

    gVmdirServerGlobals.serverId = atoi(pMaxServerId->vals[0].lberbv.bv_val) + 1;

cleanup:
    if (pEntry)
    {
        VmDirFreeEntry(pEntry);
    }
    return dwError;

error:
    goto cleanup;
}

/*
 * Attempt connection with old password (with all available authentication
 * protocols) and then the current password. If the old password fails, it will
 * never be tried again (unless the process is restarted). If the current
 * password fails, it won't be tried for N (=10) minutes.
 */
static
DWORD
_vdirReplicationConnect(
    PSTR pszUPN,
    PSTR pszDN,
    PSTR pszPassword,
    PSTR pszOldPassword,
    PVMDIR_REPLICATION_AGREEMENT pReplAgr,
    LDAP **ppLd
    )
{
    DWORD dwError = 0;
    LDAP *ld = NULL;
    PSTR pszPartnerHostName = NULL;
    time_t currentTime = time(NULL);

    dwError = VmDirReplURIToHostname(pReplAgr->ldapURI, &pszPartnerHostName);
    if (dwError != 0)
    {
        VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "_vdirReplicationConnect: VmDirReplURIToHostname failed. %s",
            pReplAgr->ldapURI);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // Try new current password and it has not failed recently.
    // ld == NULL check is in case this block is copy/pasted after the next block of code.
    if (ld == NULL &&
        pszPassword != NULL &&
        (pReplAgr->newPasswordFailTime == 0 ||
         pReplAgr->newPasswordFailTime + 10 * SECONDS_IN_MINUTE < currentTime))
    {
        dwError = _vdirReplConnect(
                    pszPartnerHostName,
                    pszUPN,
                    pszDN,
                    pszPassword,
                    pReplAgr->ldapURI,
                    &ld);
        if (dwError == VMDIR_ERROR_USER_INVALID_CREDENTIAL || dwError == LDAP_INVALID_CREDENTIALS)
        {
            pReplAgr->newPasswordFailTime = currentTime;
        }
    }

    // Try old password if the current password doesn't work and this password hasn't failed before
    if (ld == NULL && pszOldPassword != NULL && pReplAgr->oldPasswordFailTime == 0)
    {
        dwError = _vdirReplConnect(
                    pszPartnerHostName,
                    pszUPN, pszDN,
                    pszOldPassword,
                    pReplAgr->ldapURI,
                    &ld);
        if (dwError == VMDIR_ERROR_USER_INVALID_CREDENTIAL || dwError == LDAP_INVALID_CREDENTIALS)
        {
            pReplAgr->oldPasswordFailTime = currentTime;
        }
    }

    *ppLd = ld;
error:
    VMDIR_SAFE_FREE_MEMORY(pszPartnerHostName);
    return dwError;
}

/*
 * Attempt a connection with all known authentication methods.
 */
static
DWORD
_vdirReplConnect(
    PSTR pszPartnerHostName,
    PSTR pszUPN,
    PSTR pszDN,
    PSTR pszPassword,
    PSTR pszLdapURI,
    LDAP **ppLd
    )
{
    DWORD dwError = 0;
    LDAP *ld = NULL;

    VmDirCacheKrb5Creds(pszUPN, pszPassword); // ignore error

    // Bind via SASL [krb,srp] mech
    dwError = VmDirSafeLDAPBind(&ld,
                                pszPartnerHostName,
                                pszUPN,
                                pszPassword);
    if (dwError != 0)
    {
        // Use SSL and LDAP URI for 5.5 compatibility
        dwError = VmDirSSLBind(&ld,
                               pszLdapURI,
                               pszDN,
                               pszPassword);
    }
    *ppLd = ld;
    return dwError;
}

/*
 * Load passwords from registry. If strings are passed in and a new password is
 * present, then the old strings will be freed and set *pbPasswordChanged TRUE.
 */
static
DWORD
_vdirReplicationLoadPasswords(
    PSTR *ppszPassword,
    PSTR *ppszOldPassword,
    PBOOLEAN pbPasswordChanged
    )
{
    DWORD dwError = 0;
    PSTR pszPassword = NULL;
    PSTR pszOldPassword = NULL;
    BOOLEAN bPasswordChanged = FALSE;

    assert(ppszPassword != NULL);
    assert(ppszOldPassword != NULL);
    assert(pbPasswordChanged != NULL);

    dwError = VmDirReadDCAccountPassword(&pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (*ppszPassword == NULL || strcmp(*ppszPassword, pszPassword) != 0)
    {
        bPasswordChanged = TRUE;
    }

    VmDirReadDCAccountOldPassword(&pszOldPassword);

    VMDIR_SECURE_FREE_STRINGA(*ppszPassword);
    VMDIR_SECURE_FREE_STRINGA(*ppszOldPassword);

    *ppszPassword = pszPassword;
    pszPassword = NULL;

    *ppszOldPassword = pszOldPassword;
    pszOldPassword = NULL;

    *pbPasswordChanged = bPasswordChanged;

cleanup:
    VMDIR_SECURE_FREE_STRINGA(pszPassword);
    VMDIR_SECURE_FREE_STRINGA(pszOldPassword);
    return dwError;

error:
    goto cleanup;
}

