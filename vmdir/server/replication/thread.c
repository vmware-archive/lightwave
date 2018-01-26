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
VMDIR_DC_CONNECTION_STATE
_VmDirReplicationConnect(
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr
    );

static
DWORD
_VmDirWaitForReplicationAgreement(
    PBOOLEAN pbExitReplicationThread
    );

static
VOID
_VmDirConsumePartner(
    PVMDIR_REPLICATION_CONTEXT      pContext,
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr,
    PVMDIR_DC_CONNECTION            pConnection
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
_VmDirContinueReplicationCycle(
    uint64_t *puiStartTimeInShutdown,
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr,
    struct berval   bervalSyncDoneCtrl
    );

static
int
_VmDirFetchReplicationPage(
    PVMDIR_DC_CONNECTION        pConnection,
    USN                         lastSupplierUsnProcessed,
    USN                         initUsn,
    PVMDIR_REPLICATION_PAGE*    ppPage
    );

static
VOID
_VmDirFreeReplicationPage(
    PVMDIR_REPLICATION_PAGE pPage
    );

static
VOID
_VmDirProcessReplicationPage(
    PVMDIR_REPLICATION_CONTEXT pContext,
    PVMDIR_REPLICATION_PAGE pPage
    );

static
int
VmDirParseEntryForDn(
    LDAPMessage *ldapEntryMsg,
    PSTR *ppszDn
    );

static
DWORD
_VmDirFilterEmptyPageSyncDoneCtr(
    PCSTR           pszPattern,
    struct berval * pLocalCtrl,
    struct berval * pPageSyncDoneCtrl
    );

static
VOID
_VmDirReplicationUpdateTombStoneEntrySyncState(
    PVMDIR_REPLICATION_PAGE_ENTRY pPage
    );

DWORD
VmDirGetReplCycleCounter(
    VOID
    )
{
    BOOLEAN bInLock = FALSE;
    DWORD   dwCount = 0;

    VMDIR_LOCK_MUTEX(bInLock, gVmdirGlobals.replCycleDoneMutex);
    dwCount = gVmdirGlobals.dwReplCycleCounter;
    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirGlobals.replCycleDoneMutex);

    return dwCount;
}

DWORD
InitializeReplicationThread(
    void)
{
    DWORD               dwError = 0;
    PVDIR_THREAD_INFO   pThrInfo = NULL;

    dwError = VmDirSrvThrInit(
            &pThrInfo,
            gVmdirGlobals.replAgrsMutex,
            gVmdirGlobals.replAgrsCondition,
            TRUE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateThread(
            &pThrInfo->tid,
            pThrInfo->bJoinThr,
            vdirReplicationThrFun,
            pThrInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirSrvThrAdd(pThrInfo);

cleanup:

    return dwError;

error:

    VmDirSrvThrFree(pThrInfo);

    goto cleanup;
}

// vdirReplicationThrFun is the main replication function that:
//  - Executes replication cycles endlessly
//  - Each replication cycle consist of processing all the RAs for this vmdird instance.
//  - Sleeps for gVmdirServerGlobals.replInterval between replication cycles.
//

/*
     1.  Wait for a replication agreement
     2.  While server running
     2a.   Load schema context
     2b.   Load credentials
     2c.   For each replication agreement
     2ci.      Connect to system
     2cii.     Fetch Page
     2ciii.    Process Page
*/
static
DWORD
vdirReplicationThrFun(
    PVOID   pArg
    )
{
    int                             retVal = 0;
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr = NULL;
    BOOLEAN                         bInReplAgrsLock = FALSE;
    BOOLEAN                         bInReplCycleDoneLock = FALSE;
    BOOLEAN                         bExitThread = FALSE;
    int                             i = 0;
    VMDIR_REPLICATION_CONTEXT       sContext = {0};

    /*
     * This is to address the backend's writer mutex contention problem so that
     * the replication thread wouldn't be starved by the local operational threads'
     * write operations.
     */
    VmDirRaiseThreadPriority(DEFAULT_THREAD_PRIORITY_DELTA);

    retVal = _VmDirWaitForReplicationAgreement(&bExitThread);
    BAIL_ON_VMDIR_ERROR(retVal);

    if (bExitThread)
    {
        goto cleanup;
    }

    VmDirClusterLoadCache();

    if (VmDirSchemaCtxAcquire(&sContext.pSchemaCtx) != 0)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "vdirReplicationThrFun: VmDirSchemaCtxAcquire failed.");

        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    retVal = dequeCreate(&sContext.pFailedEntriesQueue);
    BAIL_ON_VMDIR_ERROR(retVal);

    while (1)
    {
        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            goto cleanup;
        }

        if (VmDirdState() == VMDIRD_STATE_STANDALONE)
        {
            VmDirSleep(1000);
            continue;
        }

        VMDIR_LOG_VERBOSE(
                VMDIR_LOG_MASK_ALL,
                "vdirReplicationThrFun: Executing replication cycle %u.",
                gVmdirGlobals.dwReplCycleCounter + 1);

        // purge RAs that have been marked as isDeleted = TRUE
        VmDirRemoveDeletedRAsFromCache();

        VMDIR_LOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);
        pReplAgr = gVmdirReplAgrs;
        VMDIR_UNLOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);

        for (; pReplAgr != NULL; pReplAgr = pReplAgr->next )
        {
            if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
            {
                goto cleanup;
            }

            if (pReplAgr->isDeleted) // skip deleted RAs
            {
                continue;
            }

            if ( VmDirIsLiveSchemaCtx(sContext.pSchemaCtx) == FALSE )
            { // schema changed via local node schema entry LDAP modify, need to pick up new schema.

                VmDirSchemaCtxRelease(sContext.pSchemaCtx);
                sContext.pSchemaCtx = NULL;
                if (VmDirSchemaCtxAcquire(&sContext.pSchemaCtx) != 0)
                {
                    VMDIR_LOG_ERROR(
                            VMDIR_LOG_MASK_ALL,
                            "vdirReplicationThrFun: VmDirSchemaCtxAcquire failed.");
                    continue;
                }
                VMDIR_LOG_INFO(
                        VMDIR_LOG_MASK_ALL,
                        "vdirReplicationThrFun: Acquires new schema context");
            }

            if (_VmDirReplicationConnect(pReplAgr) != DC_CONNECTION_STATE_CONNECTED)
            {
                continue;
            }

            _VmDirConsumePartner(&sContext, pReplAgr, &pReplAgr->dcConn);
        }

        VMDIR_LOG_DEBUG(
                VMDIR_LOG_MASK_ALL,
                "vdirReplicationThrFun: Done executing the replication cycle.");

        VMDIR_LOCK_MUTEX(bInReplCycleDoneLock, gVmdirGlobals.replCycleDoneMutex);
        gVmdirGlobals.dwReplCycleCounter++;

        if ( gVmdirGlobals.dwReplCycleCounter == 1 )
        {   // during promotion scenario, start listening on ldap ports after first cycle.
            VmDirConditionSignal(gVmdirGlobals.replCycleDoneCondition);
        }
        VMDIR_UNLOCK_MUTEX(bInReplCycleDoneLock, gVmdirGlobals.replCycleDoneMutex);

        if (VmDirdState() == VMDIRD_STATE_RESTORE)
        {
            VMDIR_LOG_INFO(
                    VMDIR_LOG_MASK_ALL,
                    "vdirReplicationThrFun: Done restoring the server by catching up with it's replication partner(s).");
            VmDirForceExit();
            break;
        }

        VMDIR_LOG_DEBUG(
                VMDIR_LOG_MASK_ALL,
                "vdirReplicationThrFun: Sleeping for the replication interval: %d seconds.",
                gVmdirServerGlobals.replInterval);

        for (i=0; i<gVmdirServerGlobals.replInterval; i++)
        {
            if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
            {
                goto cleanup;
            }

            // An RPC call requested a replication cycle to start immediately
            if (VmDirdGetReplNow() == TRUE)
            {
                VmDirdSetReplNow(FALSE);
                break;
            }
            VmDirSleep( 1000 );
        }
    } // Endless replication loop

cleanup:
    VmDirSchemaCtxRelease(sContext.pSchemaCtx);
    VMDIR_SAFE_FREE_STRINGA(sContext.pszKrb5ErrorMsg);
    VmDirReplicationClearFailedEntriesFromQueue(&sContext);
    dequeFree(sContext.pFailedEntriesQueue);
    VMDIR_UNLOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);
    VMDIR_UNLOCK_MUTEX(bInReplCycleDoneLock, gVmdirGlobals.replCycleDoneMutex);
    return 0;

error:
    VmDirdStateSet(VMDIRD_STATE_FAILURE);
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "vdirReplicationThrFun: Replication has failed with unrecoverable error.");
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
            VmDirStringChrA(bvLastLocalUsnProcessed.lberbv.bv_val, ',') -
            bvLastLocalUsnProcessed.lberbv.bv_val;

    // Note: We are effectively over-writing in ctrls[0]->ldctl_value.bv_val here, which should be ok.
    bvLastLocalUsnProcessed.lberbv.bv_val[bvLastLocalUsnProcessed.lberbv.bv_len] = '\0';

    utdVector.lberbv.bv_val = syncDoneCtrlVal->bv_val + bvLastLocalUsnProcessed.lberbv.bv_len + 1;
    utdVector.lberbv.bv_len = syncDoneCtrlVal->bv_len - bvLastLocalUsnProcessed.lberbv.bv_len - 1;

    // if lastLocalUsnProcessed is different
    if (strcmp(bvLastLocalUsnProcessed.lberbv.bv_val, replAgr->lastLocalUsnProcessed.lberbv.bv_val) != 0)
    {
        VMDIR_LOG_DEBUG(
                LDAP_DEBUG_REPL,
                "VmDirReplUpdateCookies: Replication cycle done. Updating cookies");

        // Update disk copy of utdVector
        retVal = UpdateServerObject(pSchemaCtx, &utdVector, replAgr);
        BAIL_ON_VMDIR_ERROR(retVal);

        // Update memory copy of utdVector
        retVal = VmDirUTDVectorCacheUpdate(utdVector.lberbv.bv_val);
        BAIL_ON_VMDIR_ERROR(retVal);

        // Update disk copy of lastLocalUsnProcessed
        retVal = UpdateReplicationAgreement(pSchemaCtx, replAgr, &bvLastLocalUsnProcessed);
        BAIL_ON_VMDIR_ERROR(retVal);

        // Update memory copy of lastLocalUsnProcessed
        VmDirFreeBervalContent(&replAgr->lastLocalUsnProcessed);
        if (VmDirBervalContentDup(&bvLastLocalUsnProcessed, &replAgr->lastLocalUsnProcessed) != 0)
        {
            VMDIR_LOG_ERROR(
                    VMDIR_LOG_MASK_ALL,
                    "VmDirReplUpdateCookies: BervalContentDup failed.");

            retVal = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR(retVal);
        }
    }
    else
    {
        VMDIR_LOG_DEBUG(
                LDAP_DEBUG_REPL,
                "VmDirReplUpdateCookies: Replication cycle done.. Skipping updating Replication cookies.");
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
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
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

static
VMDIR_DC_CONNECTION_STATE
_VmDirReplicationConnect(
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr
    )
{
    if (DC_CONNECTION_STATE_NOT_CONNECTED == pReplAgr->dcConn.connState ||
        DC_CONNECTION_STATE_FAILED        == pReplAgr->dcConn.connState)
    {
        VmDirInitDCConnThread(&pReplAgr->dcConn);
    }

    return pReplAgr->dcConn.connState;
}

DWORD
_VmDirWaitForReplicationAgreement(
    PBOOLEAN pbExitReplicationThread
    )
{
    DWORD dwError = 0;
    BOOLEAN bInReplAgrsLock = FALSE;
    int retVal = 0;
    PSTR pszPartnerHostName = NULL;

    assert(pbExitReplicationThread != NULL);

    VMDIR_LOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);
    while (gVmdirReplAgrs == NULL)
    {
        VMDIR_UNLOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);
        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            goto cleanup;
        }

        if (VmDirdState() == VMDIRD_STATE_RESTORE)
        {
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirWaitForReplicationAgreement: Done restoring the server, no partner to catch up with.");
            VmDirForceExit();
            *pbExitReplicationThread = TRUE;
            goto cleanup;
        }

        VMDIR_LOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);
        // wait till a replication agreement is created e.g. by vdcpromo
        if (VmDirConditionWait( gVmdirGlobals.replAgrsCondition, gVmdirGlobals.replAgrsMutex ) != 0)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirWaitForReplicationAgreement: VmDirConditionWait failed.");
            dwError = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR( dwError );

        }
        VMDIR_UNLOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);

        // When 1st RA is created for non-1st replica => try to perform special 1st replication cycle
        if (gVmdirReplAgrs)
        {
            if ( gFirstReplCycleMode == FIRST_REPL_CYCLE_MODE_COPY_DB )
            {
                if ((retVal=VmDirReplURIToHostname(gVmdirReplAgrs->ldapURI, &pszPartnerHostName)) !=0 ||
                    (retVal=VmDirFirstReplicationCycle( pszPartnerHostName, gVmdirReplAgrs)) != 0)
                {
                    VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL,
                          "vdirReplicationThrFun: VmDirReplURIToHostname or VmDirFirstReplicationCycle failed, error (%d).",
                          gVmdirReplAgrs->ldapURI, retVal);
                    VmDirForceExit();
                    *pbExitReplicationThread = TRUE;
                    dwError = LDAP_OPERATIONS_ERROR;
                    BAIL_ON_VMDIR_ERROR( dwError );
                }
                else
                {
                    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: VmDirFirstReplicationCycle() SUCCEEDED." );
                }
            }
            else if (gFirstReplCycleMode == FIRST_REPL_CYCLE_MODE_NONE)
            {
                // already promoted node and first partner added
            }
            else
            {
                assert(0);  // Lightwave server always use DB copy to bootstrap partner node.
            }
        }
        VMDIR_LOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);
    }

cleanup:
    VMDIR_UNLOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);
    return dwError;

error:
    goto cleanup;
}

/*
 * Don't stop cycle even if there is nothing in the re-apply queue.
 * because supplier may have other changes needed that could cause ordering issues
 * if not consumed in same replication cycle.
 */
static
int
_VmDirContinueReplicationCycle(
    uint64_t *puiStartTimeInShutdown,
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr,
    struct berval   bervalSyncDoneCtrl
    )
{
    int retVal = LDAP_SUCCESS;

    if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
    {
        if (*puiStartTimeInShutdown == 0)
        {
            *puiStartTimeInShutdown = VmDirGetTimeInMilliSec();
        }
        else if ((VmDirGetTimeInMilliSec() - *puiStartTimeInShutdown) >=
                 gVmdirGlobals.dwReplConsumerThreadTimeoutInMilliSec)
        {
            VMDIR_LOG_WARNING(
                VMDIR_LOG_MASK_ALL,
                "%s: Force quit cycle without updating cookies, supplier %s initUsn: %s syncdonectrl: %s",
                __FUNCTION__,
                pReplAgr->ldapURI,
                VDIR_SAFE_STRING(pReplAgr->lastLocalUsnProcessed.lberbv.bv_val),
                VDIR_SAFE_STRING(bervalSyncDoneCtrl.bv_val));
            retVal = LDAP_CANCELLED;
        }
    }

    return retVal;
}

static
VOID
_VmDirConsumePartner(
    PVMDIR_REPLICATION_CONTEXT      pContext,
    PVMDIR_REPLICATION_AGREEMENT    pReplAgr,
    PVMDIR_DC_CONNECTION            pConnection
    )
{
    int retVal = LDAP_SUCCESS;
    USN initUsn = 0;
    USN lastSupplierUsnProcessed = 0;
    BOOLEAN bInReplLock = FALSE;
    BOOLEAN bContinue = FALSE;
    PVMDIR_REPLICATION_PAGE pPage = NULL;
    struct berval   bervalSyncDoneCtrl = {0};
    PVMDIR_REPLICATION_METRICS  pReplMetrics = NULL;
    uint64_t    uiStartTime = 0;
    uint64_t    uiEndTime = 0;
    uint64_t    uiStartTimeInShutdown = 0;

    VMDIR_RWLOCK_WRITELOCK(bInReplLock, gVmdirGlobals.replRWLock, 0);

    uiStartTime = VmDirGetTimeInMilliSec();
    /*
     * Replication thread acquires lock while main thread is blocked
     * to complete shutdown, don't perform replication cycle
     */
    if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
    {
        retVal = LDAP_CANCELLED;
        goto cleanup;
    }

    initUsn = VmDirStringToLA(pReplAgr->lastLocalUsnProcessed.lberbv.bv_val, NULL, 10);
    lastSupplierUsnProcessed = initUsn;

    do // do-while (bMoreUpdatesExpected == TRUE); paged results loop
    {
        _VmDirFreeReplicationPage(pPage);
        pPage = NULL;

        retVal = _VmDirContinueReplicationCycle(&uiStartTimeInShutdown, pReplAgr, bervalSyncDoneCtrl);
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

        retVal = _VmDirFetchReplicationPage(
                pConnection,
                lastSupplierUsnProcessed, // used in search filter
                initUsn,                  // used in syncRequestCtrl to send(supplier) high watermark.
                &pPage);
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

        _VmDirProcessReplicationPage(pContext, pPage);

        lastSupplierUsnProcessed = pPage->lastSupplierUsnProcessed;

        // When a page has 0 entry, we should selectively update bervalSyncDoneCtrl.
        retVal = _VmDirFilterEmptyPageSyncDoneCtr(
                pReplAgr->lastLocalUsnProcessed.lberbv.bv_val,
                &bervalSyncDoneCtrl,
                &(pPage->searchResCtrls[0]->ldctl_value));
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

        // Check if sync done control contains explicit continue indicator
        bContinue = VmDirStringStrA(
                   pPage->searchResCtrls[0]->ldctl_value.bv_val,
                   VMDIR_REPL_CONT_INDICATOR) ?
                   TRUE : FALSE;
    }
    while (bContinue);

    if (retVal == LDAP_SUCCESS)
    {
        // If page fetch return 0 entry, bervalSyncDoneCtrl.bv_val could be NULL. Do not update cookies in this case.
        if (pPage && bervalSyncDoneCtrl.bv_val)
        {
            retVal = VmDirReplUpdateCookies(
                    pContext->pSchemaCtx, &bervalSyncDoneCtrl, pReplAgr);
            BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

            VMDIR_LOG_INFO(
                    VMDIR_LOG_MASK_ALL,
                    "Replication supplier %s USN range (%llu,%s) processed",
                    pReplAgr->ldapURI,
                    initUsn,
                    pReplAgr->lastLocalUsnProcessed.lberbv_val);
        }
    }

collectmetrics:
    uiEndTime = VmDirGetTimeInMilliSec();
    if (VmDirReplMetricsCacheFind(pReplAgr->pszHostname, &pReplMetrics) == 0)
    {
        if (retVal == LDAP_SUCCESS)
        {
            VmMetricsHistogramUpdate(
                    pReplMetrics->pTimeCycleSucceeded,
                    VMDIR_RESPONSE_TIME(uiEndTime-uiStartTime));
        }
        else if (retVal != LDAP_BUSY)
        {
            VmMetricsHistogramUpdate(
                    pReplMetrics->pTimeCycleFailed,
                    VMDIR_RESPONSE_TIME(uiEndTime-uiStartTime));
        }
    }

cleanup:
    VmDirReplicationClearFailedEntriesFromQueue(pContext);
    VMDIR_RWLOCK_UNLOCK(bInReplLock, gVmdirGlobals.replRWLock);
    VMDIR_SAFE_FREE_MEMORY(bervalSyncDoneCtrl.bv_val);
    _VmDirFreeReplicationPage(pPage);
    return;

ldaperror:
    if (retVal != LDAP_BUSY)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "%s failed, error code (%d)",
                __FUNCTION__,
                retVal);
    }

    goto collectmetrics;
}

/*
 *  During a cycle, we fetch and process page in a loop.
 *
 *  pszPattern is used as the high watermark in the syncRequestCtrl for "ALL" fetch call within the cycle.
 *
 *  The supplier uses pszPattern value to initialize its syncDoneCtrl high watermark.
 *  (A) If supplier search result set > 0
 *     It sends entry per UTDVector filtering.
 *     It updates syncDoneCtrl high watermark
 *
 *  (B) If supplier search result set = 0, its syncDoneCtrl high watermark value does not change.
 *
 *  At the end of a replication search request, the supplier sends high watermark value in syncDoneCtrl.
 *
 *  Note, both (A) and (B) cases could return 0 entry.
 *  But we should ignore syncDoneCtrl in scenario (B), which could happen during page boundary condition
 *      where the last fetch return 0.  In this case, we should not use this syncDoneCtrl to update cookies.
 *
 */
static
DWORD
_VmDirFilterEmptyPageSyncDoneCtr(
    PCSTR           pszPattern,
    struct berval * pLocalCtrl,
    struct berval * pPageSyncDoneCtrl
    )
{
    DWORD   dwError= 0;
    PSTR    pszTmp = NULL;
    size_t  iPatternLen = VmDirStringLenA(pszPattern);

    // In WriteSyncDoneControl syncDoneCtrl value looks like: high watermark,utdVector,[continue].

    // Update pLocalCtrl only if high watermark value differs between supplier syncDoneCtrl and consumer syncRequestCtrl.
    if ( (VmDirStringNCompareA(pPageSyncDoneCtrl->bv_val, pszPattern, iPatternLen, FALSE) != 0) ||
         pPageSyncDoneCtrl->bv_val[iPatternLen] != ','
       )
    {
        VMDIR_SAFE_FREE_MEMORY(pLocalCtrl->bv_val);
        pLocalCtrl->bv_len = 0;
        dwError = VmDirAllocateStringA(pPageSyncDoneCtrl->bv_val, &pszTmp);
        BAIL_ON_VMDIR_ERROR(dwError);

        pLocalCtrl->bv_val = pszTmp;
        pLocalCtrl->bv_len = VmDirStringLenA(pszTmp);
        pszTmp = NULL;
    }

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszTmp);
    goto cleanup;
}

static
int
_VmDirFetchReplicationPage(
    PVMDIR_DC_CONNECTION        pConnection,
    USN                         lastSupplierUsnProcessed,
    USN                         initUsn,
    PVMDIR_REPLICATION_PAGE*    ppPage
    )
{
    int retVal = LDAP_SUCCESS;
    BOOLEAN bLogErr = TRUE;
    PSTR    pszUtdVector = NULL;
    LDAPControl*    srvCtrls[2] = {NULL, NULL};
    LDAPControl**   ctrls = NULL;
    PVMDIR_REPLICATION_PAGE pPage = NULL;
    LDAP*   pLd = NULL;
    struct timeval  tv = {0};
    struct timeval* pTv = NULL;

    if (gVmdirGlobals.dwLdapSearchTimeoutSec > 0)
    {
        tv.tv_sec =  gVmdirGlobals.dwLdapSearchTimeoutSec;
        pTv = &tv;
    }

    pLd = pConnection->pLd;

    if (VmDirAllocateMemory(sizeof(*pPage), (PVOID)&pPage))
    {
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
    }

    pPage->iEntriesRequested = gVmdirServerGlobals.replPageSize;

    if (VmDirAllocateStringPrintf(
            &pPage->pszFilter,
            "%s>=%" PRId64,
            ATTR_USN_CHANGED,
            lastSupplierUsnProcessed + 1))
    {
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
    }

    retVal = VmDirUTDVectorCacheToString(&pszUtdVector);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    retVal = VmDirCreateSyncRequestControl(
            gVmdirServerGlobals.invocationId.lberbv.bv_val,
            initUsn,
            pszUtdVector,
            initUsn == lastSupplierUsnProcessed || 0 == lastSupplierUsnProcessed, // it's fetching first page if TRUE
            &(pPage->syncReqCtrl));
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    srvCtrls[0] = &(pPage->syncReqCtrl);
    srvCtrls[1] = NULL;

    retVal = ldap_search_ext_s(
            pLd,
            "",
            LDAP_SCOPE_SUBTREE,
            pPage->pszFilter,
            NULL,
            FALSE,
            srvCtrls,
            NULL,
            pTv,
            pPage->iEntriesRequested,
            &(pPage->searchRes));

    if (retVal == LDAP_BUSY)
    {
        VMDIR_LOG_INFO(
                LDAP_DEBUG_REPL,
                "%s: partner (%s) is busy",
                __FUNCTION__,
                pConnection->pszRemoteDCHostName);

        bLogErr = FALSE;
    }
    else if (retVal)
    {   // for all other errors, force disconnect
        pConnection->connState = DC_CONNECTION_STATE_NOT_CONNECTED;
    }
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    pPage->iEntriesReceived = ldap_count_entries(pLd, pPage->searchRes);
    if (pPage->iEntriesReceived > 0)
    {
        LDAPMessage *entry = NULL;
        size_t iEntries = 0;

        if (VmDirAllocateMemory(
                pPage->iEntriesReceived * sizeof(VMDIR_REPLICATION_PAGE_ENTRY),
                (PVOID)&pPage->pEntries))
        {
            retVal = LDAP_OPERATIONS_ERROR;
            BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
        }

        for (entry = ldap_first_entry(pLd, pPage->searchRes);
             entry != NULL && iEntries < pPage->iEntriesRequested;
             entry = ldap_next_entry(pLd, entry))
        {
            int entryState = -1;
            USN ulPartnerUSN = 0;

            retVal = ldap_get_entry_controls(pLd, entry, &ctrls);
            BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

            retVal = ParseAndFreeSyncStateControl(&ctrls, &entryState, &ulPartnerUSN);
            BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

            pPage->pEntries[iEntries].entry = entry;
            pPage->pEntries[iEntries].entryState = entryState;
            pPage->pEntries[iEntries].ulPartnerUSN = ulPartnerUSN;
            pPage->pEntries[iEntries].dwDnLength = 0;
            if (VmDirParseEntryForDn(entry, &(pPage->pEntries[iEntries].pszDn)) == 0)
            {
                pPage->pEntries[iEntries].dwDnLength = (DWORD)VmDirStringLenA(pPage->pEntries[iEntries].pszDn);
            }
            pPage->pEntries[iEntries].pBervEncodedEntry = NULL;

            iEntries++;
        }

        if (iEntries != pPage->iEntriesReceived)
        {
            retVal = LDAP_OPERATIONS_ERROR;
            BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
        }
    }

    retVal = ldap_parse_result(pLd, pPage->searchRes, NULL, NULL, NULL, NULL, &pPage->searchResCtrls, 0);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    if (pPage->searchResCtrls[0] == NULL ||
        VmDirStringCompareA(pPage->searchResCtrls[0]->ldctl_oid, LDAP_CONTROL_SYNC_DONE, TRUE) != 0)
    {
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
    }

    // Get last local USN processed from the cookie
    retVal = VmDirStringToUSN(
            pPage->searchResCtrls[0]->ldctl_value.bv_val,
            &(pPage->lastSupplierUsnProcessed));
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    *ppPage = pPage;

    if (pPage->iEntriesReceived > 0)
    {
        VMDIR_LOG_INFO(
                VMDIR_LOG_MASK_ALL,
                "%s: filter: '%s' to '%s' requested: %d received: %d usn: %" PRId64 " utd: '%s'",
                __FUNCTION__,
                VDIR_SAFE_STRING(pPage->pszFilter),
                VDIR_SAFE_STRING(pConnection->pszRemoteDCHostName),
                pPage->iEntriesRequested,
                pPage->iEntriesReceived,
                initUsn,
                VDIR_SAFE_STRING(pszUtdVector));
    }

cleanup:
    if (ctrls)
    {
        ldap_controls_free(ctrls);
        ctrls = NULL;
    }
    VMDIR_SAFE_FREE_MEMORY(pszUtdVector);
    return retVal;

ldaperror:
    if (bLogErr && pPage)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "%s: error: %d filter: '%s' requested: %d received: %d usn: %llu utd: '%s'",
                __FUNCTION__,
                retVal,
                VDIR_SAFE_STRING(pPage->pszFilter),
                pPage->iEntriesRequested,
                pPage->iEntriesReceived,
                initUsn,
                VDIR_SAFE_STRING(pszUtdVector));
    }
    _VmDirFreeReplicationPage(pPage);
    pPage = NULL;

    if (pConnection->connState != DC_CONNECTION_STATE_CONNECTED)
    {   // unbind after _VmDirFreeReplicationPage call
        VDIR_SAFE_UNBIND_EXT_S(pConnection->pLd);
    }
    goto cleanup;
}

static
VOID
_VmDirFreeReplicationPage(
    PVMDIR_REPLICATION_PAGE pPage
    )
{
    if (pPage)
    {
        VMDIR_SAFE_FREE_MEMORY(pPage->pszFilter);

        if (pPage->pEntries)
        {
            int i = 0;
            for (i = 0; i < pPage->iEntriesReceived; i++)
            {
                VMDIR_SAFE_FREE_MEMORY(pPage->pEntries[i].pszDn);
                VmDirFreeBerval(pPage->pEntries[i].pBervEncodedEntry);
                VmDirFreeBervalContent(&pPage->pEntries[i].reqDn);
            }
            VMDIR_SAFE_FREE_MEMORY(pPage->pEntries);
        }

        if (pPage->searchResCtrls)
        {
            ldap_controls_free(pPage->searchResCtrls);
        }

        if (pPage->searchRes)
        {
            ldap_msgfree(pPage->searchRes);
        }

        VmDirFreeCtrlContent(&pPage->syncReqCtrl);

        VMDIR_SAFE_FREE_MEMORY(pPage);
    }
}

static
VOID
_VmDirReplicationUpdateTombStoneEntrySyncState(
    PVMDIR_REPLICATION_PAGE_ENTRY pPage
    )
{
    DWORD  dwError = 0;
    PSTR   pszObjectGuid = NULL;
    PSTR   pszTempString = NULL;
    PSTR   pszContext = NULL;
    PSTR   pszDupDn = NULL;
    VDIR_ENTRY_ARRAY  entryArray = {0};
    VDIR_BERVALUE     bvParentDn = VDIR_BERVALUE_INIT;
    VDIR_BERVALUE     bvDn = VDIR_BERVALUE_INIT;

    dwError = VmDirStringToBervalContent(pPage->pszDn, &bvDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetParentDN(&bvDn, &bvParentDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (VmDirIsDeletedContainer(bvParentDn.lberbv_val) &&
        pPage->entryState == LDAP_SYNC_ADD)
    {
        dwError = VmDirAllocateStringA(pPage->pszDn, &pszDupDn);
        BAIL_ON_VMDIR_ERROR(dwError);

        // Tombstone DN format: cn=<cn value>#objectGUID:<objectguid value>,<DeletedObjectsContainer>
        pszTempString = VmDirStringStrA(pszDupDn, "#objectGUID:");
        pszObjectGuid = VmDirStringTokA(pszTempString, ",", &pszContext);
        pszObjectGuid = pszObjectGuid + VmDirStringLenA("#objectGUID:");

        dwError = VmDirSimpleEqualFilterInternalSearch("", LDAP_SCOPE_SUBTREE, ATTR_OBJECT_GUID, pszObjectGuid, &entryArray);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (entryArray.iSize == 1)
        {
            pPage->entryState = LDAP_SYNC_DELETE;
            VMDIR_LOG_INFO(
                VMDIR_LOG_MASK_ALL,
                "%s: (tombstone handling) change sync state to delete: (%s)",
                __FUNCTION__,
                pPage->pszDn);
        }
    }

cleanup:
    VmDirFreeBervalContent(&bvParentDn);
    VmDirFreeBervalContent(&bvDn);
    VMDIR_SAFE_FREE_MEMORY(pszDupDn);
    VmDirFreeEntryArrayContent(&entryArray);
    return;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s: error = (%d)",
            __FUNCTION__,
            dwError);
    goto cleanup;
}

/*
 * Perform Add/Modify/Delete on entries in page
 */
static
VOID
_VmDirProcessReplicationPage(
    PVMDIR_REPLICATION_CONTEXT pContext,
    PVMDIR_REPLICATION_PAGE pPage
    )
{
    PVDIR_SCHEMA_CTX pSchemaCtx = NULL;
    size_t i = 0;

    pSchemaCtx = pContext->pSchemaCtx;
    pPage->iEntriesProcessed = 0;
    pPage->iEntriesOutOfSequence = 0;
    for (i = 0; i < pPage->iEntriesReceived; i++)
    {
        int errVal = 0;
        int entryState = 0;

        _VmDirReplicationUpdateTombStoneEntrySyncState(pPage->pEntries+i);

        entryState = pPage->pEntries[i].entryState;

        if (entryState == LDAP_SYNC_ADD)
        {
            errVal = ReplAddEntry(pSchemaCtx, pPage->pEntries+i, &pSchemaCtx);
            pContext->pSchemaCtx = pSchemaCtx ;
        }
        else if (entryState == LDAP_SYNC_MODIFY)
        {
            errVal = ReplModifyEntry(pSchemaCtx, pPage->pEntries+i, &pSchemaCtx);
            pContext->pSchemaCtx = pSchemaCtx ;
        }
        else if (entryState == LDAP_SYNC_DELETE)
        {
            errVal = ReplDeleteEntry(pSchemaCtx, pPage->pEntries+i);
        }
        else
        {
            errVal = LDAP_OPERATIONS_ERROR;
        }
        pPage->pEntries[i].errVal = errVal;
        if (errVal == LDAP_SUCCESS)
        {
            pPage->iEntriesProcessed++;
        }
        else
        {
            VmDirReplicationPushFailedEntriesToQueue(pContext, pPage->pEntries+i);
        }

        if (errVal)
        {
            // TODO Consolidate error log and move this to internal operations
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "%s: sync_state = (%d) dn = (%s) error = (%d)",
                    __FUNCTION__,
                    entryState,
                    pPage->pEntries[i].pszDn,
                    pPage->pEntries[i].errVal);
        }
        else
        {
            VMDIR_LOG_DEBUG(LDAP_DEBUG_REPL,
                    "%s: sync_state = (%d) dn = (%s) error = (%d)",
                    __FUNCTION__,
                    entryState,
                    pPage->pEntries[i].pszDn,
                    pPage->pEntries[i].errVal);
        }
    }

    pPage->iEntriesOutOfSequence = dequeGetSize(pContext->pFailedEntriesQueue);

    VMDIR_LOG_INFO(
        LDAP_DEBUG_REPL,
        "%s: "
        "filter: '%s' requested: %d received: %d last usn: %llu "
        "processed: %d out-of-sequence: %d ",
        __FUNCTION__,
        VDIR_SAFE_STRING(pPage->pszFilter),
        pPage->iEntriesRequested,
        pPage->iEntriesReceived,
        pPage->lastSupplierUsnProcessed,
        pPage->iEntriesProcessed,
        pPage->iEntriesOutOfSequence);

    VmDirReapplyFailedEntriesFromQueue(pContext);

    return;
}

static
int
VmDirParseEntryForDn(
    LDAPMessage *ldapEntryMsg,
    PSTR *ppszDn
    )
{
    int      retVal = LDAP_SUCCESS;
    BerElement *ber = NULL;
    BerValue lberbv = {0};
    PSTR pszDn = NULL;

    ber = ber_dup(ldapEntryMsg->lm_ber);
    if (ber == NULL)
    {
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
    }

    // Get entry DN. 'm' => pOperation->reqDn.lberbv.bv_val points to DN within (in-place) ber
    if ( ber_scanf(ber, "{m", &(lberbv)) == LBER_ERROR )
    {
        VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL, "VmDirParseEntryForDnLen: ber_scanf failed" );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
    }
    if (lberbv.bv_val)
    {
        VmDirAllocateStringA(lberbv.bv_val, &pszDn);
    }

    *ppszDn = pszDn;

cleanup:

    if (ber)
    {
        ber_free(ber, 0);
    }
    return retVal;

ldaperror:

    VMDIR_SAFE_FREE_STRINGA(pszDn);
    goto cleanup;
}
