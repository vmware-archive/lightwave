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
    PVDIR_SCHEMA_CTX*   ppOutSchemaCtx,
    BOOLEAN             bFirstReplicationCycle);

static int
ReplDeleteEntry(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    LDAPMessage *       entry);

static int
ReplModifyEntry(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    LDAPMessage *       entry,
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
    PVDIR_ENTRY         pEntry);

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
_VmDirReplicationLoadCredentials(
    PVMDIR_REPLICATION_CREDENTIALS pCreds
    );

static
VOID
_VmDirReplicationFreeCredentialsContents(
    PVMDIR_REPLICATION_CREDENTIALS pCreds
    );

static
DWORD
_VmDirReplicationConnect(
    PVMDIR_REPLICATION_CONTEXT pContext,
    PVMDIR_REPLICATION_AGREEMENT pReplAgr,
    PVMDIR_REPLICATION_CREDENTIALS pCreds,
    PVMDIR_REPLICATION_CONNECTION pConnection
    );

static
VOID
_VmDirReplicationDisconnect(
    PVMDIR_REPLICATION_CONNECTION pConnection
    );

static
DWORD
_VmDirWaitForReplicationAgreement(
    PBOOLEAN pbFirstReplicationCycle,
    PBOOLEAN pbExitReplicationThread
    );

static
int
_VmDirConsumePartner(
    PVMDIR_REPLICATION_CONTEXT pContext,
    PVMDIR_REPLICATION_AGREEMENT replAgr,
    PVMDIR_REPLICATION_CONNECTION pConnection
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

static
int
_VmDirFetchReplicationPage(
    PVMDIR_REPLICATION_CONNECTION pConnection,
    USN lastSupplierUsnProcessed,
    USN initUsn,
    PVMDIR_REPLICATION_PAGE *ppPage
    );

static
VOID
_VmDirFreeReplicationPage(
    PVMDIR_REPLICATION_PAGE pPage
    );

static
int
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

#if 0   // 2013 port
static DWORD
_VmDirStorePartnerCertificate(
    PCSTR pszPartnerHostName);
#endif

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

/*
 * If the old policy is a realtime one (RR or FIFO), then increase it by
 * REPL_THREAD_SCHED_PRIORITY, otherwise (e.g. old policy is SCHED_NORMAL),
 * then set the new policy to RR with priority REPL_THREAD_SCHED_PRIORITY.
 * Assume that all operational threads have the same schedule policy/priority as
 * the replication thread before this change so that the replication thread
 * would be scheduled ahead of the operational threads.  This is to address
 * the backend's writer mutex contention problem so that the replication thread
 * wouldn't be starved by the local operational threads' write operations.
 * Pitfall: the priority upgrade has no effect on Windows 2008 server with
 * process under NORMAL_PRIORITY_CLASS, and has slight effect with
 * IDLE_PRIORITY_CLASS. If appears that Windows wakes up threads (for those
 * waiting for a mutex in NORMAL_PRIORITY_CLASS) in a FIFO way regardless of
 * their priorities. Therefore, there is no implementation of this function
 * for Windows.
 */
static
void
vdirRaiseThreadSchedPriority()
{
    int                  old_sch_policy = 0;
    int                  new_sch_policy = 0;
    int                  max_sched_pri = 0;
    struct sched_param   old_sch_param = {0};
    struct sched_param   new_sch_param = {0};
    int                  retVal = 0;
    PSTR                 pszLocalErrorMsg = NULL;

    retVal=pthread_getschedparam(pthread_self(), &old_sch_policy, &old_sch_param);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
         "vdirRaiseThreadSchedPriority: pthread_getschedparam failed");

    if (old_sch_policy == SCHED_FIFO || old_sch_policy == SCHED_RR)
    {
        // Thread is already in a realtime policy,
        // though the current vmdird wouldn't be setup at this policy
        max_sched_pri = sched_get_priority_max(old_sch_policy);
        if (max_sched_pri < 0)
        {
            retVal = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
                  "vdirRaiseThreadSchedPriority: sched_get_priority_max failed on policy %d", old_sch_policy);

        }

        new_sch_policy = old_sch_policy;
        new_sch_param.sched_priority = old_sch_param.sched_priority + REPL_THREAD_SCHED_PRIORITY;
        if (new_sch_param.sched_priority > max_sched_pri )
        {
            new_sch_param.sched_priority =  max_sched_pri;
        }
    } else
    {
        /*
         * Thread is in a non-realtime policy
         * put it on the lowest RR priority which would be schduled ahead of
         * operational threads with SCHED_OTHER
         */
        new_sch_policy = SCHED_RR;
        new_sch_param.sched_priority = sched_get_priority_min(new_sch_policy);
        if (new_sch_param.sched_priority < 0)
        {
            retVal = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
                 "vdirRaiseThreadSchedPriority: sched_get_priority_min failed sch_policy=%d", new_sch_policy);
        }
    }

   retVal = pthread_setschedparam(pthread_self(), new_sch_policy, &new_sch_param);
   BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
        "vdirRaiseThreadSchedPriority: setschedparam failed: errno=%d old_sch_policy=%d old_sch_priority=%d new_sch_policy=%d new_sch_priority=%d",
        errno, old_sch_policy, old_sch_param.sched_priority, new_sch_policy, new_sch_param.sched_priority);

   VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
          "vdirRaiseThreadSchedPriority: old_sch_policy=%d old_sch_priority=%d new_sch_policy=%d new_sch_priority=%d",
          old_sch_policy, old_sch_param.sched_priority, new_sch_policy, new_sch_param.sched_priority);

done:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s", VDIR_SAFE_STRING(pszLocalErrorMsg));
    goto done;
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
    VMDIR_REPLICATION_AGREEMENT    *replAgr = NULL;
    BOOLEAN                         bInReplAgrsLock = FALSE;
    BOOLEAN                         bInReplCycleDoneLock = FALSE;
    BOOLEAN                         bExitThread = FALSE;
    int                             i = 0;
    VMDIR_REPLICATION_CREDENTIALS   sCreds = {0};
    VMDIR_REPLICATION_CONNECTION    sConnection = {0};
    VMDIR_REPLICATION_CONTEXT       sContext = {0};

#ifndef _WIN32
    vdirRaiseThreadSchedPriority();
#endif

    retVal = _VmDirWaitForReplicationAgreement(
                &sContext.bFirstReplicationCycle, &bExitThread);
    BAIL_ON_VMDIR_ERROR(retVal);

    if (bExitThread)
    {
        goto cleanup;
    }

    if (VmDirSchemaCtxAcquire(&sContext.pSchemaCtx) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: VmDirSchemaCtxAcquire failed.");
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    while (1)
    {
        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            goto cleanup;
        }
        VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: Executing replication cycle %u.", gVmdirGlobals.dwReplCycleCounter + 1 );

        // purge RAs that have been marked as isDeleted = TRUE
        VmDirRemoveDeletedRAsFromCache();

        VMDIR_LOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);
        replAgr = gVmdirReplAgrs;
        VMDIR_UNLOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);

        retVal = _VmDirReplicationLoadCredentials(&sCreds);
        if (retVal)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: Error loading passwords from registry: %u\n", retVal);
        }

        for (; replAgr != NULL; replAgr = replAgr->next )
        {
            if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
            {
                goto cleanup;
            }

            if (replAgr->isDeleted) // skip deleted RAs
            {
                continue;
            }

            if ( VmDirIsLiveSchemaCtx(sContext.pSchemaCtx) == FALSE )
            { // schema changed via local node schema entry LDAP modify, need to pick up new schema.

                VmDirSchemaCtxRelease(sContext.pSchemaCtx);
                sContext.pSchemaCtx = NULL;
                if (VmDirSchemaCtxAcquire(&sContext.pSchemaCtx) != 0)
                {
                    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: VmDirSchemaCtxAcquire failed.");
                    continue;
                }
                VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: Acquires new schema context");
            }

            if (sCreds.bChanged)
            {
                replAgr->newPasswordFailTime = 0;
                replAgr->oldPasswordFailTime = 0;
            }

            retVal = _VmDirReplicationConnect(&sContext, replAgr, &sCreds, &sConnection);
            if (retVal || sConnection.pLd == NULL)
            {
                // Bail on first cycle only
                if ( sContext.bFirstReplicationCycle )
                {
                    if( !retVal )
                    {
                        retVal = VMDIR_ERROR_UNAVAILABLE;
                    }

                    BAIL_ON_VMDIR_ERROR( retVal );
                }

                continue;
            }

            retVal = _VmDirConsumePartner(&sContext, replAgr, &sConnection);
            // Bail on first cycle only
            if ( sContext.bFirstReplicationCycle )
            {
                BAIL_ON_VMDIR_ERROR( retVal );
            }
            _VmDirReplicationDisconnect(&sConnection);
        }
        VMDIR_LOG_DEBUG(VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: Done executing the replication cycle.");


        VMDIR_LOCK_MUTEX(bInReplCycleDoneLock, gVmdirGlobals.replCycleDoneMutex);
        gVmdirGlobals.dwReplCycleCounter++;

        if ( gVmdirGlobals.dwReplCycleCounter == 1 )
        {   // during promotion scenario, start listening on ldap ports after first cycle.
            VmDirConditionSignal(gVmdirGlobals.replCycleDoneCondition);
        }
        VMDIR_UNLOCK_MUTEX(bInReplCycleDoneLock, gVmdirGlobals.replCycleDoneMutex);

        if (VmDirdGetRestoreMode())
        {
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
                "vdirReplicationThrFun: Done restoring the server by catching up with it's replication partner(s)." );
            VmDirForceExit();
            break;
        }

        VMDIR_LOG_DEBUG(
            VMDIR_LOG_MASK_ALL,
            "vdirReplicationThrFun: Sleeping for the replication interval: %d seconds.",
            gVmdirServerGlobals.replInterval );

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

    _VmDirReplicationDisconnect(&sConnection);

    _VmDirReplicationFreeCredentialsContents(&sCreds);

    VmDirSchemaCtxRelease(sContext.pSchemaCtx);
    VMDIR_SAFE_FREE_STRINGA(sContext.pszKrb5ErrorMsg);

    VMDIR_UNLOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);
    VMDIR_UNLOCK_MUTEX(bInReplCycleDoneLock, gVmdirGlobals.replCycleDoneMutex);

    return 0;

error:
    VmDirdStateSet( VMDIRD_STATE_FAILURE );
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "vdirReplicationThrFun: Replication has failed with unrecoverable error." );
    goto cleanup;
}

DWORD
VmDirCacheKrb5Creds(
    PCSTR pszDcAccountUPN,
    PCSTR pszDcAccountPwd,
    PSTR  *ppszErrorMsg
    )
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
    if (ppszErrorMsg != NULL)
    {
        *ppszErrorMsg = pszLocalErrorMsg;
        pszLocalErrorMsg = NULL;
    }
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);

    return dwError;

error:
    if (ppszErrorMsg == NULL)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s", VDIR_SAFE_STRING(pszLocalErrorMsg));
    }
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
static int
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
static int
ReplModifyEntry(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    LDAPMessage *       ldapMsg,
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

    if ((retVal = SetupReplModifyRequest( &modOp, tmpAddOp.request.addReq.pEntry)) != LDAP_SUCCESS)
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
        if (pAttr)
        {
            VmDirFreeAttribute( pAttr );
            pAttr = NULL;
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
 * Attributes for usnCreated/usnChagned are updated with current local USN
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
        assert( metaData != NULL);

        // Skip loser meta data (i.e. it's empty)
        if (*(metaData + 1 /* skip ':' */) == '\0')
        {
            continue;
        }

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

static
int
_VmDirGetUsnFromSyncDoneCtrl(
    struct berval* syncDoneCtrlVal,
    USN *pUsn)
{
    int retVal = LDAP_SUCCESS;
    PSTR pszEnd = NULL;
    USN usn = 0;

    usn = VmDirStringToLA(syncDoneCtrlVal->bv_val, &pszEnd, 10);

    *pUsn = usn;

    return retVal;
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
            retVal = LDAP_OPERATIONS_ERROR;
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirReplUpdateCookies: BervalContentDup failed." );
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

/*
 * Attempt connection with old password (with all available authentication
 * protocols) and then the current password. If the old password fails, it will
 * never be tried again (unless the process is restarted). If the current
 * password fails, it won't be tried for N (=10) minutes to preven lockout.
 */
static
DWORD
_VmDirReplicationConnect(
    PVMDIR_REPLICATION_CONTEXT pContext,
    PVMDIR_REPLICATION_AGREEMENT pReplAgr,
    PVMDIR_REPLICATION_CREDENTIALS pCreds,
    PVMDIR_REPLICATION_CONNECTION pConnection
    )
{
    DWORD dwError = 0;
    LDAP *pLd = NULL;
    PSTR pszPartnerHostName = NULL;
    VMDIR_REPLICATION_PASSWORD sPasswords[2];
    DWORD dwPasswords = 0;
    DWORD i = 0;
    PSTR pszErrorMsg = NULL;
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

    // Do we have a current password that has not failed recently?
    if (pCreds->pszPassword != NULL &&
        (pReplAgr->newPasswordFailTime == 0 ||
         pReplAgr->newPasswordFailTime + (10 * SECONDS_IN_MINUTE) < currentTime))
    {
        sPasswords[dwPasswords].pszPassword = pCreds->pszPassword;
        sPasswords[dwPasswords].pPasswordFailTime = &pReplAgr->newPasswordFailTime;
        dwPasswords++;
    }

    // Do we have an old password and has it not failed?
    if (pCreds->pszPassword != NULL &&
        pReplAgr->oldPasswordFailTime == 0)
    {
        sPasswords[dwPasswords].pszPassword = pCreds->pszOldPassword;
        sPasswords[dwPasswords].pPasswordFailTime = &pReplAgr->oldPasswordFailTime;
        dwPasswords++;
    }

    /* Try all passwords that may work. Record time of invalid credentials
     * so we don't keep trying and cause it to be locked out.
     */
    for (i = 0; i < dwPasswords; i++)
    {
        PCSTR pszPassword = sPasswords[i].pszPassword;

        if (VmDirCacheKrb5Creds(pCreds->pszUPN, pszPassword, &pszErrorMsg))
        {
            // If message from VmDirCacheKrb5Creds changes, log it.
            if (pszErrorMsg != NULL &&
                (pContext->pszKrb5ErrorMsg == NULL ||
                 strcmp(pContext->pszKrb5ErrorMsg, pszErrorMsg) != 0))
            {
                VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s", VDIR_SAFE_STRING(pszErrorMsg));
            }
            VMDIR_SAFE_FREE_STRINGA(pContext->pszKrb5ErrorMsg);
            pContext->pszKrb5ErrorMsg = pszErrorMsg;
            pszErrorMsg = NULL;
        }

        // Bind via SASL [srp,krb] mech
        dwError = VmDirSafeLDAPBind(&pLd,
                                    pszPartnerHostName,
                                    pCreds->pszUPN,
                                    pszPassword);
        if (dwError != 0)
        {
            // Use SSL and LDAP URI for 5.5 compatibility
            dwError = VmDirSSLBind(&pLd,
                                   pReplAgr->ldapURI,
                                   pCreds->pszDN,
                                   pszPassword);
        }

        if (dwError == LDAP_INVALID_CREDENTIALS)
        {
            *(sPasswords[i].pPasswordFailTime) = currentTime;
        }

        if (dwError == 0)
        {
            break;
        }
    }

    pConnection->pLd = pLd;

error:
    VMDIR_SAFE_FREE_STRINGA(pszErrorMsg);
    VMDIR_SAFE_FREE_STRINGA(pszPartnerHostName);
    return dwError;
}

/*
 */
static
VOID
_VmDirReplicationDisconnect(
    PVMDIR_REPLICATION_CONNECTION pConnection
    )
{
    if (pConnection)
    {
        VDIR_SAFE_UNBIND_EXT_S(pConnection->pLd);
        VMDIR_SAFE_FREE_STRINGA(pConnection->pszConnectionDescription);
    }
}

/*
 * Load credentials. If credentials are passed in and new information is
 * present, then the old credentials will be freed and *pbChanged set TRUE,
 * else FALSE.
 */
static
DWORD
_VmDirReplicationLoadCredentials(
    PVMDIR_REPLICATION_CREDENTIALS pCreds
    )
{
    DWORD dwError = 0;
    PSTR pszUPN = NULL;
    PSTR pszDN = NULL;
    PSTR pszPassword = NULL;
    PSTR pszOldPassword = NULL;
    BOOLEAN bChanged = FALSE;

    if (pCreds == NULL)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringA(gVmdirServerGlobals.dcAccountUPN.lberbv.bv_val, &pszUPN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(gVmdirServerGlobals.dcAccountDN.lberbv.bv_val, &pszDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirReadDCAccountPassword(&pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmDirReadDCAccountOldPassword(&pszOldPassword);

    if (pCreds->pszUPN == NULL ||
        strcmp(pCreds->pszUPN, pszUPN) != 0)
    {
        bChanged = TRUE;
    }

    if (pCreds->pszDN == NULL ||
        strcmp(pCreds->pszDN, pszDN) != 0)
    {
        bChanged = TRUE;
    }

    if (pCreds->pszPassword == NULL ||
        strcmp(pCreds->pszPassword, pszPassword) != 0)
    {
        bChanged = TRUE;
    }

    if (bChanged)
    {
        _VmDirReplicationFreeCredentialsContents(pCreds);

        pCreds->pszUPN = pszUPN;
        pCreds->pszDN = pszDN;
        pCreds->pszPassword = pszPassword;
        pCreds->pszOldPassword = pszOldPassword;

        pszUPN = NULL;
        pszDN = NULL;
        pszPassword = NULL;
        pszOldPassword = NULL;
    }

    pCreds->bChanged = bChanged;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszUPN);
    VMDIR_SAFE_FREE_STRINGA(pszDN);
    VMDIR_SECURE_FREE_STRINGA(pszPassword);
    VMDIR_SECURE_FREE_STRINGA(pszOldPassword);
    return dwError;

error:
    goto cleanup;
}

/*
 */
static
VOID
_VmDirReplicationFreeCredentialsContents(
    PVMDIR_REPLICATION_CREDENTIALS pCreds
    )
{
    if (pCreds != NULL)
    {
        VMDIR_SAFE_FREE_STRINGA(pCreds->pszUPN);
        VMDIR_SAFE_FREE_STRINGA(pCreds->pszDN);
        VMDIR_SAFE_FREE_STRINGA(pCreds->pszPassword);
        VMDIR_SAFE_FREE_STRINGA(pCreds->pszOldPassword);
    }
}

DWORD
_VmDirWaitForReplicationAgreement(
    PBOOLEAN pbFirstReplicationCycle,
    PBOOLEAN pbExitReplicationThread
    )
{
    DWORD dwError = 0;
    BOOLEAN bInReplAgrsLock = FALSE;
    int retVal = 0;
    PSTR pszPartnerHostName = NULL;

    assert(pbFirstReplicationCycle != NULL);
    assert(pbExitReplicationThread != NULL);

    VMDIR_LOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);
    while (gVmdirReplAgrs == NULL)
    {
        VMDIR_UNLOCK_MUTEX(bInReplAgrsLock, gVmdirGlobals.replAgrsMutex);
        if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
        {
            goto cleanup;
        }

        if (VmDirdGetRestoreMode())
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
        if (gVmdirReplAgrs && gVmdirServerGlobals.serverId != 1)
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
                } else
                {
                    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: VmDirFirstReplicationCycle() SUCCEEDED." );
                }
            } else
            {
                VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: performing normal replication logic." );
                *pbFirstReplicationCycle = TRUE;
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

static
int
_VmDirConsumePartner(
    PVMDIR_REPLICATION_CONTEXT pContext,
    PVMDIR_REPLICATION_AGREEMENT replAgr,
    PVMDIR_REPLICATION_CONNECTION pConnection
    )
{
    int retVal = LDAP_SUCCESS;
    USN initUsn = 0;
    USN lastSupplierUsnProcessed = 0;
    PVMDIR_REPLICATION_PAGE pPage = NULL;
    BOOLEAN bReplayEverything = FALSE;
    BOOLEAN bReTrialDesired = FALSE;
    struct berval bervalSyncDoneCtrl = {0};
    int iPreviousCycleEntriesAddedErrorNoSuchObject = -1;

    do // do-while ( bReTrialDesired )
    {
        int iEntriesAddedErrorNoSuchObject = 0;

        bReTrialDesired = FALSE;
        initUsn = VmDirStringToLA(replAgr->lastLocalUsnProcessed.lberbv.bv_val, NULL, 10 );

        if (bReplayEverything)
        {
            lastSupplierUsnProcessed = 0;
            bReplayEverything = FALSE;
        }
        else
        {
            lastSupplierUsnProcessed = initUsn;
        }

        do // do-while (bMoreUpdatesExpected == TRUE); paged results loop
        {
            if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
            {
                retVal = LDAP_CANCELLED;
                goto cleanup;
            }

            _VmDirFreeReplicationPage(pPage);
            pPage = NULL;

            retVal = _VmDirFetchReplicationPage(
                        pConnection,
                        lastSupplierUsnProcessed, // used in search filter
                        initUsn,                  // used in syncRequestCtrl to send(supplier) high watermark.
                        &pPage);
            BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

            retVal = _VmDirProcessReplicationPage(
                        pContext,
                        pPage);
            BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

            lastSupplierUsnProcessed = pPage->lastSupplierUsnProcessed;
            iEntriesAddedErrorNoSuchObject += pPage->iEntriesAddedErrorNoSuchObject;

            // When a page has 0 entry, we should selectively update bervalSyncDoneCtrl.
            retVal = _VmDirFilterEmptyPageSyncDoneCtr(replAgr->lastLocalUsnProcessed.lberbv.bv_val,
                                                      &bervalSyncDoneCtrl,
                                                      &(pPage->searchResCtrls[0]->ldctl_value));
            BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

        } while (pPage->iEntriesRequested > 0 &&
                 pPage->iEntriesReceived > 0 &&
                 pPage->iEntriesReceived == pPage->iEntriesRequested);

        if (iEntriesAddedErrorNoSuchObject > 0)
        {
            // first time we need a retrial
            if (iPreviousCycleEntriesAddedErrorNoSuchObject == -1)
            {
                iPreviousCycleEntriesAddedErrorNoSuchObject = iEntriesAddedErrorNoSuchObject;
                bReTrialDesired = TRUE;
            }
            // we've made progress on reducing the number of missing parents
            else if (iEntriesAddedErrorNoSuchObject < iPreviousCycleEntriesAddedErrorNoSuchObject)
            {
                iPreviousCycleEntriesAddedErrorNoSuchObject = iEntriesAddedErrorNoSuchObject;
                bReTrialDesired = TRUE;
            }
            // we've made no progress on filling the hole...try from first USN
            else
            {
                /*
                 * If the directory has a hole in it, then reset the USN
                 * back to a value that will lead to recovery.
                 * If that doesn't work, prevent continuous replications that
                 * will bog down the CPU and network, but yet retry on occasion.
                 */
                if (pContext->stLastTimeTriedToFillHoleInDirectory == 0 ||
                    pContext->stLastTimeTriedToFillHoleInDirectory + (SECONDS_IN_HOUR) < time(NULL))
                {
                    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirConsumePartner: Attempting to plug hole in directory.");
                    bReplayEverything = TRUE;
                    bReTrialDesired = TRUE;
                }
                else
                {
                    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirConsumePartner: Did not succesfully perform any updates.");
                    bReTrialDesired = FALSE; // Trying again probably won't help.
                    retVal = LDAP_CANCELLED; // We need an error value.
                }
                time(&pContext->stLastTimeTriedToFillHoleInDirectory);
            }
        }

        VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL,
            "_VmDirConsumePartner: bReTrialDesired %d", bReTrialDesired);

    } while ( bReTrialDesired );

    if (retVal == LDAP_SUCCESS)
    {
        // If page fetch return 0 entry, bervalSyncDoneCtrl.bv_val could be NULL. Do not update cookies in this case.
        if (pPage && bervalSyncDoneCtrl.bv_val)
        {
            retVal = VmDirReplUpdateCookies(
                    pContext->pSchemaCtx,
                    &bervalSyncDoneCtrl,
                    replAgr);
            BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
        }
        pContext->bFirstReplicationCycle = FALSE;
    }
    else if (pContext->bFirstReplicationCycle)
    {
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
    }

cleanup:
    if (pPage)
    {
        _VmDirFreeReplicationPage(pPage);
        pPage = NULL;
    }
    VMDIR_SAFE_FREE_MEMORY(bervalSyncDoneCtrl.bv_val);

    VmDirdSetLimitLocalUsnToBeSupplied(0);
    return retVal;

ldaperror:
    goto cleanup;
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

    // In WriteSyncDoneControl syncDoneCtrl value looks like: high watermark,utdVector.

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
    PVMDIR_REPLICATION_CONNECTION pConnection,
    USN lastSupplierUsnProcessed,
    USN initUsn,
    PVMDIR_REPLICATION_PAGE *ppPage
    )
{
    int retVal = LDAP_SUCCESS;
    LDAPControl *srvCtrls[2] = {NULL, NULL};
    LDAPControl **ctrls = NULL;
    PVMDIR_REPLICATION_PAGE pPage = NULL;
    LDAP *pLd = NULL;
    struct timeval tv = {0};
    struct timeval *pTv = NULL;

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
            &pPage->pszFilter, "%s>=%ld",
            ATTR_USN_CHANGED,
            lastSupplierUsnProcessed + 1))
    {
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
    }

    retVal = VmDirCreateSyncRequestControl(
                gVmdirServerGlobals.invocationId.lberbv.bv_val,
                initUsn,
                gVmdirServerGlobals.utdVector.lberbv.bv_val,
                &(pPage->syncReqCtrl));
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    srvCtrls[0] = &(pPage->syncReqCtrl);
    srvCtrls[1] = NULL;

    retVal = ldap_search_ext_s(pLd, "", LDAP_SCOPE_SUBTREE, pPage->pszFilter, NULL, FALSE,
                               srvCtrls, NULL, pTv, pPage->iEntriesRequested,
                               &(pPage->searchRes) );
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

        for (entry = ldap_first_entry( pLd, pPage->searchRes );
             entry != NULL && iEntries < pPage->iEntriesRequested;
             entry = ldap_next_entry( pLd, entry ) )
        {
            int entryState = -1;

            retVal = ldap_get_entry_controls( pLd, entry, &ctrls );
            BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

            retVal = ParseAndFreeSyncStateControl(&ctrls, &entryState);
            BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

            pPage->pEntries[iEntries].entry = entry;
            pPage->pEntries[iEntries].entryState = entryState;
            pPage->pEntries[iEntries].dwDnLength = 0;
            if (VmDirParseEntryForDn(entry, &(pPage->pEntries[iEntries].pszDn)) == 0)
            {
                pPage->pEntries[iEntries].dwDnLength = (DWORD) VmDirStringLenA(pPage->pEntries[iEntries].pszDn);
            }

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
        VmDirStringCompareA(pPage->searchResCtrls[0]->ldctl_oid, LDAP_CONTROL_SYNC_DONE, TRUE ) != 0 )
    {
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
    }

    // Get last local USN processed from the cookie
    retVal = _VmDirGetUsnFromSyncDoneCtrl(
                &(pPage->searchResCtrls[0]->ldctl_value),
                &(pPage->lastSupplierUsnProcessed));
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    *ppPage = pPage;

    VMDIR_LOG_INFO(
        LDAP_DEBUG_REPL,
        "_VmDirFetchReplicationPage: "
        "filter: '%s' requested: %d received: %d usn: %llu utd: '%s'",
        VDIR_SAFE_STRING(pPage->pszFilter),
        pPage->iEntriesRequested,
        pPage->iEntriesReceived,
        initUsn,
        VDIR_SAFE_STRING(gVmdirServerGlobals.utdVector.lberbv.bv_val));

cleanup:

    if (ctrls)
    {
        ldap_controls_free(ctrls);
        ctrls = NULL;
    }

    return retVal;

ldaperror:

    if (pPage)
    {
        VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "_VmDirFetchReplicationPage: "
            "error: %d filter: '%s' requested: %d received: %d usn: %llu utd: '%s'",
            retVal,
            VDIR_SAFE_STRING(pPage->pszFilter),
            pPage->iEntriesRequested,
            pPage->iEntriesReceived,
            initUsn,
            VDIR_SAFE_STRING(gVmdirServerGlobals.utdVector.lberbv.bv_val));
    }

    _VmDirFreeReplicationPage(pPage);
    pPage = NULL;
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

        VmDirDeleteSyncRequestControl(&pPage->syncReqCtrl);

        VMDIR_SAFE_FREE_MEMORY(pPage);
    }
}

/*
 * Perform Add/Modify/Delete on entries in page
 */
static
int
_VmDirProcessReplicationPage(
    PVMDIR_REPLICATION_CONTEXT pContext,
    PVMDIR_REPLICATION_PAGE pPage
    )
{
    int retVal = 0;
    PVDIR_SCHEMA_CTX pSchemaCtx = NULL;
    size_t i = 0;

    pSchemaCtx = pContext->pSchemaCtx;
    pPage->iEntriesProcessed = 0;
    pPage->iEntriesAddedErrorNoSuchObject = 0;
    for (i = 0; i < pPage->iEntriesReceived; i++)
    {
        int errVal = 0;
        LDAPMessage *entry = pPage->pEntries[i].entry;
        int entryState = pPage->pEntries[i].entryState;

        if (entryState == LDAP_SYNC_ADD)
        {
            errVal = ReplAddEntry( pSchemaCtx, entry, &pSchemaCtx,
                    pContext->bFirstReplicationCycle );
            pContext->pSchemaCtx = pSchemaCtx ;

            if (errVal == LDAP_NO_SUCH_OBJECT)
            {
                // - out-of-sequence parent-child updates OR
                // - parent deleted on one node, child added on another
                pPage->iEntriesAddedErrorNoSuchObject++;
            }
        }
        else if (entryState == LDAP_SYNC_MODIFY)
        {
            errVal = ReplModifyEntry( pSchemaCtx, entry, &pSchemaCtx);
            pContext->pSchemaCtx = pSchemaCtx ;
        }
        else if (entryState == LDAP_SYNC_DELETE)
        {
            errVal = ReplDeleteEntry( pSchemaCtx, entry);
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

    VMDIR_LOG_INFO(
        LDAP_DEBUG_REPL,
        "%s: error %d "
        "filter: '%s' requested: %d received: %d last usn: %llu "
        "processed: %d nosuchobject: %d ",
        __FUNCTION__,
        retVal,
        VDIR_SAFE_STRING(pPage->pszFilter),
        pPage->iEntriesRequested,
        pPage->iEntriesReceived,
        pPage->lastSupplierUsnProcessed,
        pPage->iEntriesProcessed,
        pPage->iEntriesAddedErrorNoSuchObject);

    return retVal;
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
