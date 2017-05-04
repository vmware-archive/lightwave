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
    VMDIR_REPLICATION_AGREEMENT    *replAgr = NULL;
    BOOLEAN                         bInReplAgrsLock = FALSE;
    BOOLEAN                         bInReplCycleDoneLock = FALSE;
    BOOLEAN                         bExitThread = FALSE;
    int                             i = 0;
    VMDIR_REPLICATION_CREDENTIALS   sCreds = {0};
    VMDIR_REPLICATION_CONNECTION    sConnection = {0};
    VMDIR_REPLICATION_CONTEXT       sContext = {0};

    /*
     * This is to address the backend's writer mutex contention problem so that
     * the replication thread wouldn't be starved by the local operational threads'
     * write operations.
     */
    VmDirRaiseThreadPriority(DEFAULT_THREAD_PRIORITY_DELTA);

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

        if (VmDirdState() == VMDIRD_STATE_STANDALONE)
        {
            VmDirSleep(1000);
            continue;
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

        if (VmDirdState() == VMDIRD_STATE_RESTORE)
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

        dwError = VmDirAllocateStringPrintf(&pszTgtUPN, "krbtgt/%s@%s",
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
    BOOLEAN bContinue = FALSE;
    struct berval bervalSyncDoneCtrl = {0};
    int iPreviousCycleEntriesOutOfSequence = -1;
    BOOLEAN bInReplLock = FALSE;

    VMDIR_RWLOCK_WRITELOCK(bInReplLock, gVmdirGlobals.replRWLock, 0);

    do // do-while ( bReTrialDesired )
    {
        int iEntriesOutOfSequence = 0;

        bReTrialDesired = FALSE;
        bContinue = FALSE;
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
                if (iEntriesOutOfSequence == 0)
                {   // no parent/child out of sequence so far, should update UTDVector before existing.
                    // this avoids create->modify(s) scenario lost modify(s) if cycle force ended by service shutdown.
                    // i.e. next cycle would receive SYNC_STATE(2)/modify instead of SYNC_STATE(1)/create.
                    goto replcycledone;
                }

                // this should be very rare after LW1.0/PSC6.6 where we switch to db copy for first replication cycle.
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
            iEntriesOutOfSequence += pPage->iEntriesOutOfSequence;

            // When a page has 0 entry, we should selectively update bervalSyncDoneCtrl.
            retVal = _VmDirFilterEmptyPageSyncDoneCtr(replAgr->lastLocalUsnProcessed.lberbv.bv_val,
                                                      &bervalSyncDoneCtrl,
                                                      &(pPage->searchResCtrls[0]->ldctl_value));
            BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

            // Check if sync done control contains explicit continue indicator
            bContinue = VmDirStringStrA(pPage->searchResCtrls[0]->ldctl_value.bv_val,
                                        VMDIR_REPL_CONT_INDICATOR) ? TRUE : FALSE;

            // Check if we received a full page and need to continue
            bContinue |= pPage->iEntriesRequested > 0 &&
                         pPage->iEntriesReceived > 0 &&
                         pPage->iEntriesReceived == pPage->iEntriesRequested;

        } while (bContinue);

        if (iEntriesOutOfSequence > 0)
        {
            // first time we need a retrial
            if (iPreviousCycleEntriesOutOfSequence == -1)
            {
                iPreviousCycleEntriesOutOfSequence = iEntriesOutOfSequence;
                bReTrialDesired = TRUE;
            }
            // we've made progress on reducing the number of missing parents
            else if (iEntriesOutOfSequence < iPreviousCycleEntriesOutOfSequence)
            {
                iPreviousCycleEntriesOutOfSequence = iEntriesOutOfSequence;
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

replcycledone:
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

            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
                "Replication supplier %s USN range (%llu,%s) processed.",
                replAgr->ldapURI, initUsn, replAgr->lastLocalUsnProcessed.lberbv_val);
        }
        pContext->bFirstReplicationCycle = FALSE;
    }
    else if (pContext->bFirstReplicationCycle)
    {
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
    }

cleanup:
    VMDIR_RWLOCK_UNLOCK(bInReplLock, gVmdirGlobals.replRWLock);
    VMDIR_SAFE_FREE_MEMORY(bervalSyncDoneCtrl.bv_val);
    _VmDirFreeReplicationPage(pPage);
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
            USN ulPartnerUSN = 0;

            retVal = ldap_get_entry_controls( pLd, entry, &ctrls );
            BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

            retVal = ParseAndFreeSyncStateControl(&ctrls, &entryState, &ulPartnerUSN);
            BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

            pPage->pEntries[iEntries].entry = entry;
            pPage->pEntries[iEntries].entryState = entryState;
            pPage->pEntries[iEntries].ulPartnerUSN = ulPartnerUSN;
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
        "%s: filter: '%s' requested: %d received: %d usn: %llu utd: '%s'",
        __FUNCTION__,
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
            "%s: error: %d filter: '%s' requested: %d received: %d usn: %llu utd: '%s'",
            __FUNCTION__,
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
    pPage->iEntriesOutOfSequence = 0;
    for (i = 0; i < pPage->iEntriesReceived; i++)
    {
        int errVal = 0;
        int entryState = pPage->pEntries[i].entryState;

        if (entryState == LDAP_SYNC_ADD)
        {
            errVal = ReplAddEntry( pSchemaCtx, pPage->pEntries+i, &pSchemaCtx,
                    pContext->bFirstReplicationCycle );
            pContext->pSchemaCtx = pSchemaCtx ;

            if (errVal == LDAP_NO_SUCH_OBJECT)
            {
                // - out-of-sequence parent-child updates OR
                // - parent deleted on one node, child added on another
                pPage->iEntriesOutOfSequence++;
            }
        }
        else if (entryState == LDAP_SYNC_MODIFY)
        {
            errVal = ReplModifyEntry( pSchemaCtx, pPage->pEntries+i, &pSchemaCtx);
            if (errVal == LDAP_NOT_ALLOWED_ON_NONLEAF)
            {
                pPage->iEntriesOutOfSequence++;
            }

            pContext->pSchemaCtx = pSchemaCtx ;
        }
        else if (entryState == LDAP_SYNC_DELETE)
        {
            errVal = ReplDeleteEntry( pSchemaCtx, pPage->pEntries+i);
            if (errVal == LDAP_NOT_ALLOWED_ON_NONLEAF)
            {
                pPage->iEntriesOutOfSequence++;
            }
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
        "processed: %d out-of-sequence: %d ",
        __FUNCTION__,
        retVal,
        VDIR_SAFE_STRING(pPage->pszFilter),
        pPage->iEntriesRequested,
        pPage->iEntriesReceived,
        pPage->lastSupplierUsnProcessed,
        pPage->iEntriesProcessed,
        pPage->iEntriesOutOfSequence);

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
