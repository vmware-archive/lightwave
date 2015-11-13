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

#define VDIR_CFG_ROOT_ENTRY_INITIALIZER     \
{                                           \
    "objectclass",  "vmwDirCfg",            \
    "cn",           "config",               \
    NULL                                    \
}

#define VDIR_CFG_ORG_ENTRY_INITIALIZER      \
{                                           \
    "objectclass",  "vmwDirCfg",            \
    "cn",           "organization",         \
    NULL                                    \
}

#define VDIR_SCHEMA_NAMING_CONTEXT_ENTRY_INITIALIZER      \
{                                           \
    "objectclass",  "dmd",                  \
    "cn",           "schemacontext",        \
    NULL                                    \
}

#define VDIR_OPEN_FILES_MAX 16384

static
DWORD
InitializeSchemaEntry(
    VOID);

static
DWORD
InitializeCFGEntries(
    PVDIR_SCHEMA_CTX    pSchemaCtx);

static
int
InitializeSchema(
    BOOLEAN*    pbWriteSchemaEntry);

static
int
InitializeVmdirdSystemEntries(
    VOID);

static
DWORD
InitializeCFGIndicesEntry(
    PVDIR_SCHEMA_CTX    pSchemaCtx
    );

static
DWORD
InitializeResouceLimit(
    VOID
    );

static
DWORD
InitializeServerStatusGlobals(
    VOID
    );

static
DWORD
InitializeGlobalVars(
    VOID
    );

static
DWORD
_VmDirWriteBackInvocationId(VOID);

static
DWORD
_VmDirRestoreInstance(VOID);

static
DWORD
_VmDirGenerateInvocationId(VOID);

static
int
LoadServerGlobals(BOOLEAN *pbWriteInvocationId);

static
DWORD
_VmDirSrvCreatePersistedDSERoot(VOID);

static
DWORD _VmDirGetHostsInternal(
    PSTR**  ppServerInfo,
    size_t* pdwInfoCount
    );

/*
 * load krb master key into gVmdirKrbGlobals.bervMasterKey
 */
DWORD
VmDirKrbInit(
    VOID
    )
{
    DWORD               dwError = 0;
    PSTR                pszLocalRealm = NULL;
    PSTR                pszLocalDomain = NULL;
    VDIR_ENTRY_ARRAY    entryArray = {0};
    int                 iCnt = 0;
    BOOLEAN             bInLock = FALSE;

    //TODO, use PERSISTED_DSE_ROOT_DN.ATTR_ROOT_DOMAIN_NAMING_CONTEXT instead of "/SUBTREE search?
    // find domain entries (objectclass=dcobject)
    dwError = VmDirSimpleEqualFilterInternalSearch(
                    "",
                    LDAP_SCOPE_SUBTREE,
                    ATTR_OBJECT_CLASS,
                    OC_DC_OBJECT,
                    &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (iCnt = 0; iCnt < entryArray.iSize; iCnt++)
    {
        PVDIR_ATTRIBUTE pAttrKrbMKey = VmDirFindAttrByName(&(entryArray.pEntry[iCnt]), ATTR_KRB_MASTER_KEY);

        if (pAttrKrbMKey)
        {
            VMDIR_LOCK_MUTEX(bInLock, gVmdirKrbGlobals.pmutex);

            // BUGBUG BUGBUG, assume we only have one realm now
            dwError = VmDirNormalizeDNWrapper( &(entryArray.pEntry[iCnt].dn) );
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirAllocateStringA( entryArray.pEntry[iCnt].dn.bvnorm_val,
                                            &pszLocalDomain );
            BAIL_ON_VMDIR_ERROR(dwError);
            gVmdirKrbGlobals.pszDomainDN = pszLocalDomain;  // gVmdirKrbGlobals takes over pszLocalDomain
            pszLocalDomain = NULL;

            dwError = VmDirKrbSimpleDNToRealm( &(entryArray.pEntry[iCnt].dn), &pszLocalRealm);
            BAIL_ON_VMDIR_ERROR(dwError);
            gVmdirKrbGlobals.pszRealm = pszLocalRealm;   // gVmdirKrbGlobals takes over pszLocalRealm
            pszLocalRealm = NULL;

            dwError = VmDirBervalContentDup( &(pAttrKrbMKey->vals[0]), &gVmdirKrbGlobals.bervMasterKey);
            BAIL_ON_VMDIR_ERROR(dwError);

            VmDirConditionSignal(gVmdirKrbGlobals.pcond);   // wake up VmKdcInitKdcServiceThread
            VMDIR_UNLOCK_MUTEX(bInLock, gVmdirKrbGlobals.pmutex);

            break;
        }
    }

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "VmDirKrbInit, REALM (%s)", VDIR_SAFE_STRING(gVmdirKrbGlobals.pszRealm));

cleanup:

    VMDIR_UNLOCK_MUTEX(bInLock, gVmdirKrbGlobals.pmutex);

    VmDirFreeEntryArrayContent(&entryArray);

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirKrbInit failed (%d), REALM (%s)",
                     dwError, VDIR_SAFE_STRING(gVmdirKrbGlobals.pszRealm));

    VMDIR_SAFE_FREE_MEMORY(pszLocalRealm);
    VMDIR_SAFE_FREE_MEMORY(pszLocalDomain);

    goto cleanup;
}

DWORD
VmDirInitBackend()
{
    DWORD   dwError = 0;
    PVDIR_BACKEND_INTERFACE pBE = NULL;
    BOOLEAN bWriteSchemaEntry = FALSE;

    dwError = VmDirBackendConfig();
    BAIL_ON_VMDIR_ERROR(dwError);

    pBE = VmDirBackendSelect(NULL);
    assert(pBE);

    {   // backend init phase 1 - initialize and open database
        dwError = pBE->pfnBEInit();
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = pBE->pfnBEDBOpen();
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = InitializeSchema(&bWriteSchemaEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAttrIndexLibInit();
    BAIL_ON_VMDIR_ERROR(dwError);

    {   // backend init phase 2 - open additional index database
        dwError = pBE->pfnBEIndexOpen();
        BAIL_ON_VMDIR_ERROR(dwError);

        // prepare USNList to guarantee safe USN for replication
        dwError = VmDirBackendInitUSNList(pBE);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (bWriteSchemaEntry)
    {
        dwError = _VmDirGenerateInvocationId(); // to be used in replication meta data for the entries created in
                                                // InitializeVmdirdSystemEntries()
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = InitializeVmdirdSystemEntries();
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _VmDirSrvCreatePersistedDSERoot();
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:

    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirInitBackend failed (%d)", dwError );

    goto cleanup;
}

/*
 * Initialize vmdird components
 */
DWORD
VmDirInit(
    VOID
    )
{
    DWORD   dwError = 0;
    BOOLEAN bWriteInvocationId = FALSE;
    BOOLEAN bWaitTimeOut = FALSE;
    VMDIR_RUNMODE runMode = VmDirdGetRunMode();

    dwError = InitializeGlobalVars();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = InitializeServerStatusGlobals();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSuperLoggingInit(&gVmdirGlobals.pLogger);
    BAIL_ON_VMDIR_ERROR(dwError);

#ifndef _WIN32
    dwError = InitializeResouceLimit();
    BAIL_ON_VMDIR_ERROR(dwError);
#endif

    dwError = ConstructSDForVmDirServ(&gVmdirGlobals.gpVmDirSrvSD);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirOpensslInit();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirPasswordSchemeInit();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirMiddleLayerLibInit();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirPluginInit();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInitBackend();
    BAIL_ON_VMDIR_ERROR(dwError);

    // load server globals before any write operations
    dwError = LoadServerGlobals(&bWriteInvocationId);
    if ( dwError == ERROR_BACKEND_ENTRY_NOTFOUND )
    {
        dwError = 0;    // vmdir not yet promoted
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirKrbInit();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirVmAclInit();
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!gVmdirGlobals.bPatchSchema)
    {
        dwError = VmDirRpcServerInit();
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirIpcServerInit();
    BAIL_ON_VMDIR_ERROR (dwError);

    if (VmDirdGetRestoreMode())
    {
        // TBD: What happens if server is started in restore mode even when it has not been promoted?
        dwError = _VmDirRestoreInstance(); // fix invocationId and up-to-date-vector before starting replicating in.
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    if ( runMode == VMDIR_RUNMODE_NORMAL )
    {
        dwError = VmDirReplicationLibInit();
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ( gVmdirGlobals.bPatchSchema && gVmdirGlobals.pszBootStrapSchemaFile )
    {
        dwError = VmDirSchemaPatchViaFile( gVmdirGlobals.pszBootStrapSchemaFile );
        BAIL_ON_VMDIR_ERROR(dwError);

        VmDirSetAdministratorPasswordNeverExpires(); // Ignore error
    }

    if (bWriteInvocationId) // Logic for backward compatibility. Needs to come after schema patch logic.
    {
        dwError = _VmDirWriteBackInvocationId();
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    //Will not free gVmdirGlobals.pPortListenSyncCounter since it maybe accessed when
    //  timeout occured (e.g. waiting for promote) though there is a onetime memory leak.
    dwError = VmDirAllocateSyncCounter( &gVmdirGlobals.pPortListenSyncCounter,
                                        VmDirGetAllLdapPortsCount(),
                                        SYNC_SIGNAL,
                                        5000);  // wait time 5 seconds
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!VmDirdGetRestoreMode())
    {
        dwError = VmDirInitConnAcceptThread();
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (gVmdirServerGlobals.serverId)
    {
        //Wait only if there is not a vdcprome pending.
        dwError = VmDirSyncCounterWaitEvent(gVmdirGlobals.pPortListenSyncCounter, &bWaitTimeOut);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (bWaitTimeOut)
        {
            VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL, "%s: NOT all LDAP ports are ready for accepting services.", __func__);
        } else
        {
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: all LDAP ports are ready for accepting services.", __func__);
        }
    }

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Config MaxLdapOpThrs (%d)", gVmdirGlobals.dwMaxFlowCtrlThr );

error:
    return dwError;
}

static
DWORD
_VmDirSrvCreatePersistedDSERoot(VOID)
{
    DWORD dwError = 0;
    PSTR ppszPersistedDSERootAttrs[] =
    {
            ATTR_OBJECT_CLASS,                  OC_DSE_ROOT,
            ATTR_OBJECT_CLASS,                  OC_TOP,
            ATTR_CN,                            PERSISTED_DSE_ROOT_NAMING_ATTR_VALUE,
            ATTR_SUPPORTED_LDAP_VERSION,        SUPPORTED_LDAP_VERSION,
            ATTR_INVOCATION_ID,                 gVmdirServerGlobals.invocationId.lberbv.bv_val,
            ATTR_SUPPORTED_CONTROL,             LDAP_CONTROL_SYNC,
            ATTR_SUPPORTED_CONTROL,             LDAP_CONTROL_SYNC_STATE,
            ATTR_SUPPORTED_CONTROL,             LDAP_CONTROL_SYNC_DONE,
            ATTR_SUPPORTED_CONTROL,             VDIR_LDAP_CONTROL_SHOW_DELETED_OBJECTS,
            ATTR_SUPPORTED_CONTROL,             LDAP_CONTROL_PAGEDRESULTS,
            ATTR_SERVER_VERSION,                VDIR_SERVER_VERSION,
            ATTR_PSC_VERSION,                   VDIR_PSC_VERSION,
            ATTR_SUPPORTED_SASL_MECHANISMS ,    SASL_MECH,
            NULL
    };
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSimpleEntryCreate(
                    pSchemaCtx,
                    ppszPersistedDSERootAttrs,
                    PERSISTED_DSE_ROOT_DN,
                    DSE_ROOT_ENTRY_ID);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }

    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirSrvCreatePersistedDSERoot failed (%d)", dwError );
    goto cleanup;
}

// _VmDirRestoreInstance():
// 1. Get new invocation ID.
//    So I can rejoin the federation with a fresh ID.
// 2. Fix the up-to-date vector and invocation id in the server object and in the DSE root entry.
// 3. Advance USN to proper value. i.e. My MAX(USN) seen by partners.
//    So partners will pick up new changes from me.
// 4. Advance RID sequence number.
//    So there will be no ObjectSid conflict with entries created after backup.

static
DWORD
_VmDirRestoreInstance(VOID)
{
    DWORD                   dwError = LDAP_SUCCESS;
    size_t                  i = 0;
    VDIR_OPERATION          op = {0};
    VDIR_BERVALUE           newUtdVector = VDIR_BERVALUE_INIT;
    USN                     nextUsn = 0;
    USN                     restoredUsn = 0;
    DWORD                   dwAdvanceRID = 1;  // as we advance nextUsn once before final nextUsn while loop
    char                    nextUsnStr[VMDIR_MAX_USN_STR_LEN] = {0};
    PSTR                    pszLocalErrMsg = NULL;
    PSTR                    pszDCAccount = NULL;
    PSTR*                   pServerInfo = NULL;
    size_t                  dwInfoCount = 0;

    dwError = VmDirRegReadDCAccount(&pszDCAccount);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirGetHostsInternal(&pServerInfo, &dwInfoCount);
    if (dwError != 0)
    {
        printf("_VmDirRestoreInstance: fail to get hosts from topology: %d\n", dwError );
    }
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
            "_VmDirRestoreInstance: fail to get hosts from topology: %d", dwError );

    if ( dwInfoCount == 1 )
    {
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Single node deployment topology, skip restore procedure.");
        printf("Single node deployment topology, skip restore procedure.\n");

        goto cleanup;
    }

    /*
     *  Try those servers one by one until one of the hosts can be reached and be used
     *  to query up-to-date servers topology, and then follow those servers if they
     *  are partners of the local host, and get the highest USN.
     */
    for (i=0; i<dwInfoCount; i++)
    {
        if (VmDirStringCompareA(pServerInfo[i], pszDCAccount, FALSE) == 0)
        {
            //Don't try to query self for the uptodate topology.
            continue;
        }
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Trying to get topology from host %s ...", pServerInfo[i]);
        printf("Trying to get topology from host %s ...\n", pServerInfo[i]);

        dwError = VmDirGetUsnFromPartners(pServerInfo[i], &restoredUsn);
        if (dwError == 0)
        {
             VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Got topology from host %s", pServerInfo[i]);
             printf("Topology obtained from host %s.\n", pServerInfo[i]);
             break;
        }
    }
    if (dwError !=0 || restoredUsn == 0)
    {
        if (restoredUsn == 0 )
        {
            dwError = ERROR_NOT_FOUND;
        }
        printf("_VmDirRestoreInstance: failed to get restored USN from partners, error code: %d\n.", dwError );
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
            "_VmDirRestoreInstance: cannot get restored USN from partners, error code: %d.", dwError );
    }

    dwError = VmDirInitStackOperation( &op, VDIR_OPERATION_TYPE_INTERNAL, LDAP_REQ_MODIFY, NULL );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
            "_VmDirRestoreInstance: VmDirInitStackOperation failed with error code: %d.", dwError );

    // Setup target DN

    dwError = VmDirBervalContentDup( &gVmdirServerGlobals.serverObjDN, &op.reqDn );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
            "_VmDirRestoreInstance: BervalContentDup failed with error code: %d.", dwError );

    op.pBEIF = VmDirBackendSelect(op.reqDn.lberbv.bv_val);
    assert(op.pBEIF);

    dwError = VmDirBervalContentDup( &op.reqDn, &op.request.modifyReq.dn );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                "_VmDirRestoreInstance: BervalContentDup failed with error code: %d.", dwError );

    // Setup utdVector mod

    // create an entry for the old invocationID in the up-to-date vector

    dwError = op.pBEIF->pfnBEGetNextUSN( op.pBECtx, &nextUsn );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
            "_VmDirRestoreInstance: pfnBEGetNextUSN failed with error code: %d, error message: %s", dwError,
            VDIR_SAFE_STRING(op.pBECtx->pszBEErrorMsg) );

    //gVmdirServerGlobals.initialNextUSN was set by the first pfnBEGetNextUSN call.
    //It's value less 1 is the one that has been consumed by the server to be restored.
    nextUsn = gVmdirServerGlobals.initialNextUSN - 1;

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirRestoreInstance: highest USN observed from partners %lu, local USN: %lu",
                   restoredUsn, nextUsn);
    printf("Highest USN observed from partners %lu, local USN: %lu\n", restoredUsn, nextUsn);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Utilize larger of %lu and %lu for new USN", restoredUsn, nextUsn );
    printf("Utilize larger of %lu and %lu for new USN \n", restoredUsn, nextUsn );

    dwError = VmDirStringNPrintFA( nextUsnStr, sizeof(nextUsnStr), sizeof(nextUsnStr) - 1, "%ld", nextUsn);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                "_VmDirRestoreInstance: VmDirStringNPrintFA failed with error code: %d", dwError,
                VDIR_SAFE_STRING(op.pBECtx->pszBEErrorMsg) );

    // <existing up-to-date vector>,<old invocation ID>:<local USN>,
    dwError = VmDirAllocateStringAVsnprintf( &(newUtdVector.lberbv.bv_val), "%s%s:%s,",
                                            gVmdirServerGlobals.utdVector.lberbv.bv_val,
                                            gVmdirServerGlobals.invocationId.lberbv.bv_val,
                                            nextUsnStr);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                "_VmDirRestoreInstance: VmDirAllocateStringAVsnprintf failed with error code: %d", dwError,
                VDIR_SAFE_STRING(op.pBECtx->pszBEErrorMsg) );

    newUtdVector.bOwnBvVal = TRUE;

    newUtdVector.lberbv.bv_len = VmDirStringLenA(newUtdVector.lberbv.bv_val);

    dwError = VmDirBervalContentDup( &newUtdVector, &(gVmdirServerGlobals.utdVector) );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                "_VmDirRestoreInstance: BervalContentDup failed with error code: %d.", dwError );

    dwError = VmDirAppendAMod( &op, MOD_OP_REPLACE, ATTR_UP_TO_DATE_VECTOR, ATTR_UP_TO_DATE_VECTOR_LEN,
                              gVmdirServerGlobals.utdVector.lberbv.bv_val, gVmdirServerGlobals.utdVector.lberbv.bv_len );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                "_VmDirRestoreInstance: VmDirAppendAMod failed with error code: %d.", dwError );

    // Setup invocationId mod

    dwError = _VmDirGenerateInvocationId();
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                "_VmDirRestoreInstance: _VmDirGenerateInvocationId failed with error code: %d.", dwError );

    dwError = VmDirAppendAMod( &op, MOD_OP_REPLACE, ATTR_INVOCATION_ID, ATTR_INVOCATION_ID_LEN,
                       gVmdirServerGlobals.invocationId.lberbv.bv_val, gVmdirServerGlobals.invocationId.lberbv.bv_len );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                "_VmDirRestoreInstance: VmDirAppendAMod failed with error code: %d.", dwError );

    dwError = VmDirInternalModifyEntry( &op );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg, "_VmDirRestoreInstance: InternalModifyEntry failed. DN: %s, "
              "Error code: %d, Error string: %s", op.reqDn.lberbv.bv_val, dwError,
              VDIR_SAFE_STRING( op.ldapResult.pszErrMsg ) );

    printf("Setup new invocationId [%s]\n", gVmdirServerGlobals.invocationId.lberbv.bv_val);

    // Advance the USN to the upToDateUsn passed in, which should be the maximum USN that has been seen by peer nodes.
    // This will avoid the situation where some new entries will be skipped in replication to peer nodes.
    // See Bug 1272548 for details.
    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Advancing USN if neccessary, current: %d, goal to restore to: %d",
                    nextUsn, restoredUsn );
    while ( nextUsn < restoredUsn )
    {
        dwAdvanceRID++;

        dwError = op.pBEIF->pfnBEGetNextUSN( op.pBECtx, &nextUsn );
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    // Advance RID for all realms, USN advance (all writes) should >= RID advance (new entries).
    dwError = VmDirAdvanceDomainRID( dwAdvanceRID );
    BAIL_ON_VMDIR_ERROR(dwError);
    printf("Domain RID advanced count=%u\n", dwAdvanceRID);

    printf("Lotus instance restore succeeded.\n");

cleanup:
    VmDirFreeBervalContent(&newUtdVector);
    VmDirFreeOperationContent(&op);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);
    VMDIR_SAFE_FREE_STRINGA(pszDCAccount);

    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, VDIR_SAFE_STRING(pszLocalErrMsg) );
    printf("Lotus instance restore failed, error (%s)(%u)\n", VDIR_SAFE_STRING(pszLocalErrMsg), dwError );
    goto cleanup;
}

static
DWORD
InitializeServerStatusGlobals(
    VOID
    )
{
    DWORD   dwError = 0;

    gVmdirGlobals.iServerStartupTime = VmDirGetTimeInMilliSec();

    dwError = VmDirInitOPStatisticGlobals();
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "InitializeServerStatusGlobals failed (%d)", dwError );

    goto cleanup;
}

/*
 * (have to delay schema entry write to after index dbs are initialized)
 */
static
int
InitializeSchema(
    BOOLEAN*    pbWriteSchemaEntry)
{
    int                     retVal = 0;
    PVDIR_ENTRY             pEntry = NULL;
    PVDIR_BACKEND_INTERFACE pBE = NULL;

    assert(pbWriteSchemaEntry);

    retVal = VmDirSchemaLibInit();
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirAllocateMemory(
            sizeof(VDIR_ENTRY),
            (PVOID*)&pEntry);
    BAIL_ON_VMDIR_ERROR(retVal);

    pBE = VmDirBackendSelect(NULL);
    assert(pBE);

    retVal = pBE->pfnBESimpleIdToEntry(
            SUB_SCEHMA_SUB_ENTRY_ID,
            pEntry);
    if (retVal != 0 && retVal != ERROR_BACKEND_ENTRY_NOTFOUND)
    {
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    if (retVal == ERROR_BACKEND_ENTRY_NOTFOUND)
    {
        PSTR    pszSchemaFilePath = gVmdirGlobals.pszBootStrapSchemaFile;
        if (!pszSchemaFilePath)
        {
            retVal = ERROR_NO_SCHEMA;
            BAIL_ON_VMDIR_ERROR(retVal);
        }

        // use bootstrap schema instance to jump start schema
        retVal = VmDirAttrIndexBootStrap();
        BAIL_ON_VMDIR_ERROR(retVal);

        retVal = VmDirSchemaInitializeViaFile(pszSchemaFilePath);
        BAIL_ON_VMDIR_ERROR(retVal);

        *pbWriteSchemaEntry = TRUE;
    }
    else
    {
        // load schema from entry
        retVal = VmDirSchemaInitializeViaEntry(pEntry);
        BAIL_ON_VMDIR_ERROR(retVal);
    }

cleanup:

    if (pEntry)
    {
        VmDirFreeEntry(pEntry);
    }

    return retVal;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "InitializeSchema failed (%d)", retVal );

    goto cleanup;
}

static
int
InitializeVmdirdSystemEntries(
    VOID)
{
    int     iError = 0;
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;

    iError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(iError);

    iError = InitializeSchemaEntry();
    BAIL_ON_VMDIR_ERROR(iError);

    iError = InitializeCFGEntries(
            pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(iError);

cleanup:

    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }

    return iError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "InitializeVmdirdSystemEntries failed (%d)", iError );

    goto cleanup;
}

static
DWORD
_VmDirGenerateInvocationId(VOID)
{
    DWORD           dwError = 0;
    uuid_t          guid = {0};
    VDIR_BERVALUE   bv = VDIR_BERVALUE_INIT;
    char            pszInvocationId[VMDIR_GUID_STR_LEN] = {0};
    PSTR            pszLocalErrMsg = NULL;

    if (VmDirUuidGenerate(&guid) != 0)
    {
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg, "_VmDirGenerateInvocationId: VmDirUuidGenerate() failed." );
    }
    if (VmDirUuidToStringLower( &guid, pszInvocationId, VMDIR_GUID_STR_LEN) != 0)
    {
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg, "_VmDirGenerateInvocationId: VmDirUuidToStringLower() failed." );
    }

    bv.lberbv.bv_val = pszInvocationId;
    bv.lberbv.bv_len = VmDirStringLenA( bv.lberbv.bv_val );
    if (VmDirBervalContentDup( &bv, &gVmdirServerGlobals.invocationId ) != 0)
    {
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg, "_VmDirGenerateInvocationId: VmDirBervalContentDup() failed." );
    }

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Invocation ID: %s", gVmdirServerGlobals.invocationId.lberbv.bv_val);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);
    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, VDIR_SAFE_STRING(pszLocalErrMsg) );
    goto cleanup;
}

//
// LoadServerGlobals()
//
// Description:
//  - This function should always find DSE Root entry in the DB
//  - If DSE Root entry does NOT contain server object DN, that means the instance has NOT been promoted.
//  - If DSE Root entry does contain server object DN, there are 2 cases:
//    - It is the normal case i.e. the instance has been promoted and being restarted.
//    - There is an abnormal case when the DB was copied externally from a partner.
//      In this case DSE Root entry exists, and it contains partner's server object DN. In this case also instance has
//      NOT been promoted yet, and this is detected by the absence of the DC account password file (HACK to handle
//      the abnormal case).
//  - There is no special handling of any of these scenarios in the function, code just flows. Only in the last abnormal
//    case the new invocation ID needs to be generated here.


static
int
LoadServerGlobals(BOOLEAN *pbWriteInvocationId)
{
    DWORD               dwError = 0;
    VDIR_ENTRY          dseRoot = {0};
    VDIR_ENTRY          serverObj = {0};
    PVDIR_ATTRIBUTE     attr = NULL;
    VDIR_OPERATION      op = {0};
    VDIR_BERVALUE       bv = VDIR_BERVALUE_INIT;
    BOOLEAN             bHasTxn = FALSE;
    VDIR_BERVALUE       serverGuid = VDIR_BERVALUE_INIT;
    PSTR                pszDCGroupDN = NULL;
    PSTR                pszDCClientGroupDN = NULL;
    PSTR                pszServicesRootDN = NULL;
    PSTR                pszLocalErrMsg = NULL;
    PSTR                pszDcAccountPwd = NULL;

    dwError = VmDirInitStackOperation( &op,
                                       VDIR_OPERATION_TYPE_INTERNAL,
                                       LDAP_REQ_SEARCH,
                                       NULL );
    BAIL_ON_VMDIR_ERROR(dwError);

    op.pBEIF = VmDirBackendSelect(PERSISTED_DSE_ROOT_DN);
    assert(op.pBEIF);

    dwError = op.pBEIF->pfnBETxnBegin( op.pBECtx, VDIR_BACKEND_TXN_READ);
    BAIL_ON_VMDIR_ERROR( dwError );
    bHasTxn = TRUE;

    dwError = op.pBEIF->pfnBEIdToEntry( op.pBECtx, op.pSchemaCtx, DSE_ROOT_ENTRY_ID, &dseRoot, VDIR_BACKEND_TXN_READ);
    BAIL_ON_VMDIR_ERROR( dwError );

    for (attr = dseRoot.attrs; attr; attr = attr->next)
    {
        if (VmDirStringCompareA(attr->pATDesc->pszName, ATTR_ROOT_DOMAIN_NAMING_CONTEXT, FALSE) == 0)
        {
            if (VmDirBervalContentDup( &attr->vals[0], &gVmdirServerGlobals.systemDomainDN ) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "BervalContentDup failed while copying system domain DN.");
            }
            if (VmDirNormalizeDN( &gVmdirServerGlobals.systemDomainDN, op.pSchemaCtx) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "VmDirNormalizeDN failed for system domain DN.");
            }

            // set DomainControllerGroupDN (NOTE, this is a hard code name, same as in instance.c)
            dwError = VmDirAllocateStringAVsnprintf( &pszDCGroupDN,
                                                     "cn=%s,cn=%s,%s",
                                                     VMDIR_DC_GROUP_NAME,
                                                     VMDIR_BUILTIN_CONTAINER_NAME,
                                                     gVmdirServerGlobals.systemDomainDN.lberbv_val);
            BAIL_ON_VMDIR_ERROR(dwError);

            gVmdirServerGlobals.bvDCGroupDN.lberbv_val = pszDCGroupDN;
            gVmdirServerGlobals.bvDCGroupDN.lberbv_len = VmDirStringLenA(pszDCGroupDN);
            pszDCGroupDN = NULL;
            if (VmDirNormalizeDN( &(gVmdirServerGlobals.bvDCGroupDN), op.pSchemaCtx) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "VmDirNormalizeDN failed for bvDCGroupDN.");
            }

            // set DCClientGroupDN (NOTE, this is a hard code name, same as in instance.c)
            dwError = VmDirAllocateStringAVsnprintf( &pszDCClientGroupDN,
                                                     "cn=%s,cn=%s,%s",
                                                     VMDIR_DCCLIENT_GROUP_NAME,
                                                     VMDIR_BUILTIN_CONTAINER_NAME,
                                                     gVmdirServerGlobals.systemDomainDN.lberbv_val);
            BAIL_ON_VMDIR_ERROR(dwError);

            gVmdirServerGlobals.bvDCClientGroupDN.lberbv_val = pszDCClientGroupDN;
            gVmdirServerGlobals.bvDCClientGroupDN.lberbv_len = VmDirStringLenA(pszDCClientGroupDN);
            pszDCClientGroupDN = NULL;
            if (VmDirNormalizeDN( &(gVmdirServerGlobals.bvDCClientGroupDN), op.pSchemaCtx) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "VmDirNormalizeDN failed for pszDCClientGroupDN.");
            }

            // set ServicesRootDN (NOTE, this is a hard code name, same as in instance.c)
            dwError = VmDirAllocateStringAVsnprintf( &pszServicesRootDN,
                                                     "cn=%s,%s",
                                                     VMDIR_SERVICES_CONTAINER_NAME,
                                                     gVmdirServerGlobals.systemDomainDN.lberbv_val);
            BAIL_ON_VMDIR_ERROR(dwError);

            gVmdirServerGlobals.bvServicesRootDN.lberbv_val = pszServicesRootDN;
            gVmdirServerGlobals.bvServicesRootDN.lberbv_len = VmDirStringLenA(pszServicesRootDN);
            pszServicesRootDN = NULL;
            if (VmDirNormalizeDN( &(gVmdirServerGlobals.bvServicesRootDN), op.pSchemaCtx) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "VmDirNormalizeDN failed for pszServicesRootDN.");
            }

            continue;
        }
        if (VmDirStringCompareA(attr->pATDesc->pszName, ATTR_DEL_OBJS_CONTAINER, FALSE) == 0)
        {
            if (VmDirBervalContentDup( &attr->vals[0], &gVmdirServerGlobals.delObjsContainerDN ) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "BervalContentDup failed while copying deleted "
                                               "objects container DN.");
            }
            if (VmDirNormalizeDN( &gVmdirServerGlobals.delObjsContainerDN, op.pSchemaCtx) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "VmDirNormalizeDN failed for deleted objects container DN.");
            }
            continue;
        }
        if (VmDirStringCompareA(attr->pATDesc->pszName, ATTR_SERVER_NAME, FALSE) == 0)
        {
            if (VmDirBervalContentDup( &attr->vals[0], &gVmdirServerGlobals.serverObjDN ) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "BervalContentDup failed while copying serverDN.");
            }
            if (VmDirNormalizeDN( &(gVmdirServerGlobals.serverObjDN), op.pSchemaCtx) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "VmDirNormalizeDN failed for server object DN.");
            }
            continue;
        }
        if (VmDirStringCompareA(attr->pATDesc->pszName, ATTR_DC_ACCOUNT_DN, FALSE) == 0)
        {
            if (VmDirBervalContentDup( &attr->vals[0], &gVmdirServerGlobals.dcAccountDN ) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "BervalContentDup failed while copying dcAccountDN.");
            }
            if (VmDirNormalizeDN( &(gVmdirServerGlobals.dcAccountDN), op.pSchemaCtx) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "VmDirNormalizeDN failed for dcAccountDN.");
            }
            continue;
        }
        if (VmDirStringCompareA(attr->pATDesc->pszName, ATTR_DC_ACCOUNT_UPN, FALSE) == 0)
        {
            if (VmDirBervalContentDup( &attr->vals[0], &gVmdirServerGlobals.dcAccountUPN ) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "BervalContentDup failed while copying dcAccountUPN.");
            }
            continue;
        }
        if (VmDirStringCompareA(attr->pATDesc->pszName, ATTR_INVOCATION_ID, FALSE) == 0)
        {
            if (VmDirBervalContentDup( &attr->vals[0], &gVmdirServerGlobals.invocationId ) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "BervalContentDup failed." );
            }
            continue;
        }
        if (VmDirStringCompareA(attr->pATDesc->pszName, ATTR_DEFAULT_ADMIN_DN, FALSE) == 0)
        {
            if (VmDirBervalContentDup( &attr->vals[0], &gVmdirServerGlobals.bvDefaultAdminDN ) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "BervalContentDup failed." );
            }
            if (VmDirNormalizeDN( &(gVmdirServerGlobals.bvDefaultAdminDN), op.pSchemaCtx) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "VmDirNormalizeDN failed for bvDefaultAdminDN.");
            }
            continue;
        }
        if (VmDirStringCompareA(attr->pATDesc->pszName, ATTR_SITE_NAME, FALSE) == 0)
        {
            dwError = VmDirAllocateStringA( attr->vals[0].lberbv.bv_val, &gVmdirServerGlobals.pszSiteName);
            BAIL_ON_VMDIR_ERROR(dwError);

            continue;
        }
    }

    if (gVmdirServerGlobals.serverObjDN.lberbv.bv_len == 0) // => not vdcpromed yet.
    {
        goto cleanup;
    }
    else
    { // Hack
        dwError = VmDirReadDCAccountPassword(&pszDcAccountPwd);
        if (dwError == ERROR_FILE_NOT_FOUND) // registry key not found => not vdcpromed yet.
        { // Replica server not configured/promoted yet.
          // This is the scenario where DB has been copied from the partner i.e. where gVmdirServerGlobals.serverObjDN
          // exists in the DSE Root entry exists but DC account password file does NOT exist.
          // (bit of an hack logic to detect this scenario). Generate invocation ID now.

            dwError = ERROR_BACKEND_ENTRY_NOTFOUND;

            // Server object will be updated with this invocation ID during vdcpromo. Also till
            // vdcpromo is done, values in gVmdirServerGlobals.serverObjDN, gVmdirServerGlobals.dcAccountDN, and
            // gVmdirServerGlobals.dcAccountUPN are that of this server's partner's and not it's own.
            if (_VmDirGenerateInvocationId() != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "VmDirGenerateInvocationId() failed.");
            }
        }
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    // Load Server object

    op.pBEIF = VmDirBackendSelect(gVmdirServerGlobals.serverObjDN.lberbv.bv_val);
    assert(op.pBEIF);

    if (VmDirNormalizeDN( &(gVmdirServerGlobals.serverObjDN), op.pSchemaCtx ) != 0)
    {
        dwError = VMDIR_ERROR_GENERIC;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                      "VmDirNormalizeDN for serverObjDN failed. ");
    }

    dwError = op.pBEIF->pfnBEDNToEntry(  op.pBECtx, op.pSchemaCtx, &(gVmdirServerGlobals.serverObjDN), &serverObj,
                                         VDIR_BACKEND_ENTRY_LOCK_READ );
    BAIL_ON_VMDIR_ERROR( dwError );

    bv.lberbv.bv_val = "";
    bv.lberbv.bv_len = 0;
    VmDirBervalContentDup( &bv, &gVmdirServerGlobals.utdVector );

    for (attr = serverObj.attrs; attr; attr = attr->next)
    {
        if (VmDirStringCompareA(attr->pATDesc->pszName, ATTR_SERVER_GUID, FALSE) == 0) // for data migration scenario
        {
            if (VmDirBervalContentDup( &attr->vals[0], &serverGuid ) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "BervalContentDup failed." );
            }
            continue;
        }
        if (VmDirStringCompareA(attr->pATDesc->pszName, ATTR_INVOCATION_ID, FALSE) == 0)
        {
            if (VmDirBervalContentDup( &attr->vals[0], &gVmdirServerGlobals.invocationId ) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "BervalContentDup failed." );
            }
            continue;
        }
        if (VmDirStringCompareA(attr->pATDesc->pszName, ATTR_REPL_INTERVAL, FALSE) == 0)
        {
            gVmdirServerGlobals.replInterval = atoi( attr->vals[0].lberbv.bv_val );
            continue;
        }
        if (VmDirStringCompareA(attr->pATDesc->pszName, ATTR_REPL_PAGE_SIZE, FALSE) == 0)
        {
            gVmdirServerGlobals.replPageSize = atoi( attr->vals[0].lberbv.bv_val );
            continue;
        }
        if (VmDirStringCompareA(attr->pATDesc->pszName, ATTR_UP_TO_DATE_VECTOR, FALSE) == 0)
        {
            if (VmDirBervalContentDup( &attr->vals[0], &gVmdirServerGlobals.utdVector ) != 0)
            {
                dwError = VMDIR_ERROR_GENERIC;
                BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                              "BervalContentDup failed." );
            }
            continue;
        }
        if (VmDirStringCompareA(attr->pATDesc->pszName, ATTR_SERVER_ID, FALSE) == 0)
        {
            gVmdirServerGlobals.serverId = atoi(attr->vals[0].lberbv.bv_val);
        }
    }
    if (gVmdirServerGlobals.invocationId.lberbv.bv_len == 0)
    {
        if (serverGuid.lberbv.bv_len == 0) // for data migration scenario: not even serverGuid is present.
        {
            dwError = VMDIR_ERROR_GENERIC;
            BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                          "Server object entry does NOT contain invocationId attribute.");
        }
        if (VmDirBervalContentDup( &serverGuid, &gVmdirServerGlobals.invocationId ) != 0)
        {
            dwError = VMDIR_ERROR_GENERIC;
            BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                                          "BervalContentDup failed." );
        }
        *pbWriteInvocationId = TRUE;
    }
    else
    {
        *pbWriteInvocationId = FALSE;
    }

    dwError = op.pBEIF->pfnBETxnCommit( op.pBECtx );
    bHasTxn = FALSE;
    BAIL_ON_VMDIR_ERROR( dwError );

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Server ID (%d), InvocationID (%s)",
                                        gVmdirServerGlobals.serverId,
                                        gVmdirServerGlobals.invocationId.lberbv_val);
cleanup:

    VMDIR_SECURE_FREE_STRINGA(pszDcAccountPwd);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);
    VMDIR_SAFE_FREE_MEMORY(pszDCGroupDN);
    VmDirFreeEntryContent( &dseRoot );
    VmDirFreeEntryContent( &serverObj );
    VmDirFreeBervalContent(&serverGuid);

    VmDirFreeOperationContent(&op);

    return dwError;

error:

    if ( dwError != ERROR_BACKEND_ENTRY_NOTFOUND )
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "LoadServerGlobals: (%u)(%s)",
                         dwError, VDIR_SAFE_STRING(pszLocalErrMsg));
    }

    if (bHasTxn)
    {
        op.pBEIF->pfnBETxnAbort( op.pBECtx );
    }

    goto cleanup;
}

/*
 * During upgrade, we can patch schema via this function.
 * Input: new version of Lotus schema file, which never delete existing definitions.
 *
 * 1. convert schema file into entry
 * 2. create internal modify operation
 * 3. call VmDirInternalModifyEntry
 */
DWORD
VmDirSchemaPatchViaFile(
    PCSTR       pszSchemaFilePath
    )
{
#define SCHEMA_ENTRY_CN "aggregate"

    // TODO TODO
    // we only support patching of attributetypes and objectclasses now.
    static PCSTR        pszPatchAttrList[] = { "attributetypes",
                                               "objectclasses",
                                               "ditcontentrules"
                                             };
    DWORD               dwError = 0;
    VDIR_OPERATION      ldapOp = {0};
    int                 iListSize = sizeof(pszPatchAttrList)/sizeof(pszPatchAttrList[0]);
    int                 iCnt = 0;
    VDIR_ENTRY_ARRAY    entryArray = {0};


    if ( IsNullOrEmptyString(pszSchemaFilePath) )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // get the current schema entry
    dwError = VmDirSimpleEqualFilterInternalSearch(
                    SUB_SCHEMA_SUB_ENTRY_DN,
                    LDAP_SCOPE_BASE,
                    ATTR_CN,
                    SCHEMA_ENTRY_CN,
                    &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    if ( entryArray.iSize != 1 )
    {
        dwError = VMDIR_ERROR_ENTRY_NOT_FOUND;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirEntryUnpack( entryArray.pEntry );
    BAIL_ON_VMDIR_ERROR(dwError);

    // merge new semantics from file into current schema - pEntry
    dwError = VmDirSchemaPatchFileToEntry(  pszSchemaFilePath,
                                            entryArray.pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInitStackOperation( &ldapOp,
                                       VDIR_OPERATION_TYPE_INTERNAL,
                                       LDAP_REQ_MODIFY,
                                       NULL );
    BAIL_ON_VMDIR_ERROR(dwError);

    ldapOp.pBEIF = VmDirBackendSelect(NULL);
    assert(ldapOp.pBEIF);

    ldapOp.reqDn.lberbv.bv_val = SUB_SCHEMA_SUB_ENTRY_DN;
    ldapOp.reqDn.lberbv.bv_len = VmDirStringLenA(SUB_SCHEMA_SUB_ENTRY_DN);
    // TODO, need this? (copy this behavior from ldap-head/modify.c)
    ldapOp.request.modifyReq.dn.lberbv.bv_val = ldapOp.reqDn.lberbv.bv_val;
    ldapOp.request.modifyReq.dn.lberbv.bv_len = ldapOp.reqDn.lberbv.bv_len;

    for (iCnt = 0; iCnt < iListSize ; iCnt++)
    {
        PVDIR_ATTRIBUTE pAttr = VmDirFindAttrByName( entryArray.pEntry, (PSTR) pszPatchAttrList[iCnt]);

        if ( pAttr )
        {
            dwError = VmDirOperationAddModReq(  &ldapOp,
                                                MOD_OP_REPLACE,
                                                pAttr->type.lberbv_val,
                                                pAttr->vals,
                                                pAttr->numVals);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    dwError = VmDirInternalModifyEntry(&ldapOp);
    if (ldapOp.ldapResult.vmdirErrCode == VMDIR_ERROR_SCHEMA_UPDATE_PASSTHROUGH)
    {
        dwError = 0; // noop, no db and cache upgrade needed.
        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Schema is up-to-date, no patch action needed." );
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, ">>>>>>>>>> Schema patch succeeded <<<<<<<<<<");

cleanup:

    VmDirFreeEntryArrayContent(&entryArray);

    VmDirFreeOperationContent(&ldapOp);

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "Schema patch failed (%d)(%d)(%s)(%s)",
                              dwError, ldapOp.ldapResult.errCode,
                              VDIR_SAFE_STRING(pszSchemaFilePath),
                              VDIR_SAFE_STRING(ldapOp.ldapResult.pszErrMsg));
    goto cleanup;
}

// _VmDirWriteBackInvocationId()

static
DWORD
_VmDirWriteBackInvocationId(VOID)
{
    DWORD                   dwError = LDAP_SUCCESS;
    PSTR                    pszLocalErrMsg = NULL;

    dwError = VmDirInternalEntryAttributeReplace( NULL, gVmdirServerGlobals.serverObjDN.bvnorm_val, ATTR_INVOCATION_ID,
                                                  &(gVmdirServerGlobals.invocationId));
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                "_VmDirWriteBackInvocationId: VmDirInternalEntryAttributeReplace failed. DN: %s, Error code: %d",
                gVmdirServerGlobals.serverObjDN.bvnorm_val, dwError );

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);

    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, VDIR_SAFE_STRING(pszLocalErrMsg) );
    goto cleanup;
}

/*
 * Write schema entry into db and free pEntry
 * root schema context entry: cn=schemacontext
 * subschema subentry       : cn=aggregate, cn=schemacontext
 */
static
DWORD
InitializeSchemaEntry(
    VOID)
{
    static PSTR ppszSchemaContext[] = VDIR_SCHEMA_NAMING_CONTEXT_ENTRY_INITIALIZER;

    DWORD           dwError = 0;
    // pEntry entry pointer is owned by caller, needs to be freed
    PVDIR_ENTRY     pEntry = VmDirSchemaAcquireAndOwnStartupEntry();
    VDIR_OPERATION  ldapOp = {0};

    BAIL_ON_VMDIR_INVALID_POINTER(pEntry, dwError);

    dwError = VmDirInitStackOperation( &ldapOp,
                                       VDIR_OPERATION_TYPE_INTERNAL,
                                       LDAP_REQ_ADD,
                                       NULL );
    BAIL_ON_VMDIR_ERROR(dwError);

    // create cn=schemacontext
    dwError = VmDirSimpleEntryCreate(
            pEntry->pSchemaCtx,
            ppszSchemaContext,
            SCHEMA_NAMING_CONTEXT_DN,
            SCHEMA_NAMING_CONTEXT_ID);
    BAIL_ON_VMDIR_ERROR(dwError);

    ldapOp.pBEIF = VmDirBackendSelect(NULL);
    assert(ldapOp.pBEIF && pEntry);

    // create cn=aggregate,cn=schemacontext
    ldapOp.reqDn.lberbv.bv_val = SUB_SCHEMA_SUB_ENTRY_DN;
    ldapOp.reqDn.lberbv.bv_len = VmDirStringLenA(SUB_SCHEMA_SUB_ENTRY_DN);

    dwError = VmDirResetAddRequestEntry( &ldapOp, pEntry );
    BAIL_ON_VMDIR_ERROR(dwError);
    pEntry = NULL; // ldapOp takes over pEntry

    dwError = VmDirInternalAddEntry(&ldapOp);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    VmDirFreeOperationContent(&ldapOp);
    // Free Entry pointer if exists
    VmDirFreeEntry(pEntry);

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "InitializeSchemaEntry failed (%d)", dwError );

    goto cleanup;
}

/*
 * create cn=indices,cn=config entry based on the pBootStrapIdxAttrDesc content
 */
static
DWORD
InitializeCFGIndicesEntry(
    PVDIR_SCHEMA_CTX    pSchemaCtx
    )
{

    DWORD   dwError = 0;
    PSTR*   ppszAttrList = NULL;
    int     iNumBootStrapIdx = 0;
    int     iCnt = 0;
    int     iTmp = 0;
    PVDIR_CFG_ATTR_INDEX_DESC   pIdxDesc = pBootStrapIdxAttrDesc;

    assert(pSchemaCtx);

    for (iNumBootStrapIdx = 0;
         pIdxDesc[iNumBootStrapIdx].pszAttrName != NULL;
         iNumBootStrapIdx++)
    {}

    // size = total attribute value * 2  + 1 (NULL terminate)
    dwError = VmDirAllocateMemory(
            sizeof(PSTR) * ((1+1+iNumBootStrapIdx) * 2 + 1),
            (PVOID)&ppszAttrList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
            "cn", &ppszAttrList[iCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
            "indices", &ppszAttrList[iCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
            "objectclass", &ppszAttrList[iCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
            "vmwDirCfg", &ppszAttrList[iCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (iTmp = 0; iTmp < iNumBootStrapIdx; iTmp++)
    {
        dwError = VmDirAllocateStringA(
                "vmwAttrIndexDesc", &ppszAttrList[iCnt++]);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringAVsnprintf(
                &ppszAttrList[iCnt++],
                "%s%s%s%s",
                pIdxDesc[iTmp].pszAttrName,
                pIdxDesc[iTmp].iTypes & INDEX_TYPE_EQUALITY ? " eq" : "",
                pIdxDesc[iTmp].iTypes & INDEX_TYPE_SUBSTR ? " sub" : "",
                pIdxDesc[iTmp].bIsUnique ? " unique" : "");
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSimpleEntryCreate(
            pSchemaCtx,
            ppszAttrList,
            CFG_INDEX_ENTRY_DN,
            CFG_INDEX_ENTRY_ID);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    if (ppszAttrList)
    {
        VmDirFreeStringArrayA(ppszAttrList);
        VMDIR_SAFE_FREE_MEMORY(ppszAttrList);
    }

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "InitializeCFGIndicesEntry failed (%d)", dwError );

    goto cleanup;
}

/*
 * Create default config tree entries
 * 1. cn=config
 * 2. cn=indice,cn=config
 * Called only during the very first time server startup to give default
 *      content to config DIT entries.
 */
static
DWORD
InitializeCFGEntries(
    PVDIR_SCHEMA_CTX    pSchemaCtx)
{
    DWORD   dwError = 0;
    static PSTR ppszCFG_ROOT[] = VDIR_CFG_ROOT_ENTRY_INITIALIZER;
    static PSTR ppszCFG_ORG[] = VDIR_CFG_ORG_ENTRY_INITIALIZER;

    dwError = VmDirSimpleEntryCreate(
            pSchemaCtx,
            ppszCFG_ROOT,
            CFG_ROOT_DN,
            CFG_ROOT_ENTRY_ID);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = InitializeCFGIndicesEntry(pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSimpleEntryCreate(
            pSchemaCtx,
            ppszCFG_ORG,
            CFG_INDEX_ORGANIZATION_DN,
            CFG_ORGANIZATION_ENTRY_ID);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "InitializeCFGEntries failed (%d)", dwError );

    goto cleanup;
}

/*
 * Set process resource limits
 */
static
DWORD
InitializeResouceLimit(
    VOID
    )
{
    DWORD           dwError = 0;
    BAIL_ON_VMDIR_ERROR(dwError);

#ifndef _WIN32
    struct rlimit   VMLimit = {0};

    // unlimited resource
    VMLimit.rlim_cur = RLIM_INFINITY;
    VMLimit.rlim_max = RLIM_INFINITY;

    if ( setrlimit(RLIMIT_AS, &VMLimit)     // virtual memory
         ||
         setrlimit(RLIMIT_CORE, &VMLimit)   // core file size
       )
    {
        dwError = ERROR_INVALID_CONFIGURATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMLimit.rlim_cur = VDIR_OPEN_FILES_MAX;
    VMLimit.rlim_max = VDIR_OPEN_FILES_MAX;
    if (setrlimit(RLIMIT_NOFILE, &VMLimit)!=0)
    {
       //If VDIR_OPEN_FILES_MAX is too large to set, try to set soft limit to hard limit
       if (getrlimit(RLIMIT_NOFILE, &VMLimit)==0)
       {
            VMLimit.rlim_cur = VMLimit.rlim_max;
            if (setrlimit(RLIMIT_NOFILE, &VMLimit)!=0)
            {
                VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL,
                    "Fail to increase RLIMIT_NOFILE - keep the default value.");
            }
       } else
       {
           VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL,
                    "Fail to obtain current RLIMIT_NOFILE - keep the default value.");

       }
    }

#endif

cleanup:

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "InitializeResouceLimit failed (%d)(%d)", dwError, errno );

    goto cleanup;
}


static
DWORD
InitializeGlobalVars(
    VOID
    )
{
    DWORD   dwError = 0;

    dwError = VmDirAllocateMutex(&gVmdirRunmodeGlobals.pMutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&gVmdirGlobals.mutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&gVmdirGlobals.replAgrsMutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateCondition(&gVmdirGlobals.replAgrsCondition);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&gVmdirGlobals.replCycleDoneMutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateCondition(&gVmdirGlobals.replCycleDoneCondition);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&gVmdirKrbGlobals.pmutex);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateCondition(&gVmdirKrbGlobals.pcond);
    BAIL_ON_VMDIR_ERROR(dwError);

    // LDAP operation threads shutdown synchronization, shutdown continue
    // when count == 0 (i.e. all op thrs are done)
    dwError = VmDirAllocateSyncCounter( &gVmdirGlobals.pOperationThrSyncCounter,
                                        0,
                                        SYNC_SIGNAL,
                                        10000);  // wait time 10 seconds
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMutex(&gVmdirGlobals.pMutexIPCConnection);
    BAIL_ON_VMDIR_ERROR(dwError)

    dwError = VmDirAllocateMutex(&gVmdirGlobals.pFlowCtrlMutex);
    BAIL_ON_VMDIR_ERROR(dwError);


cleanup:

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "InitializeGlobalVars failed (%d)", dwError );

    goto cleanup;
}

/*
 * Lookup servers topology internally first. Then one of the servers
 * will be used to query uptoupdate servers topology
 */
static
DWORD _VmDirGetHostsInternal(
    PSTR**  ppServerInfo,
    size_t* pdwInfoCount
    )
{
    DWORD               dwError = 0;
    DWORD               i = 0;
    VDIR_ENTRY_ARRAY    entryArray = {0};
    PSTR                pszSearchBaseDN = NULL;
    PVDIR_ATTRIBUTE     pAttr = NULL;
    PSTR*  pServerInfo = NULL;

    dwError = VmDirAllocateStringAVsnprintf(
                &pszSearchBaseDN,
                "cn=Sites,cn=Configuration,%s",
                gVmdirServerGlobals.systemDomainDN.bvnorm_val
                );

    dwError = VmDirSimpleEqualFilterInternalSearch(
                    pszSearchBaseDN,
                    LDAP_SCOPE_SUBTREE,
                    ATTR_OBJECT_CLASS,
                    OC_DIR_SERVER,
                    &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (entryArray.iSize == 0 )
    {
        dwError = LDAP_NO_SUCH_OBJECT;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory( entryArray.iSize*sizeof(PSTR), (PVOID*)&pServerInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i=0; i<entryArray.iSize; i++)
    {
         pAttr =  VmDirEntryFindAttribute(ATTR_CN, entryArray.pEntry+i);
         dwError = VmDirAllocateStringA( pAttr->vals[0].lberbv.bv_val, &pServerInfo[i]);
         BAIL_ON_VMDIR_ERROR(dwError);
    }
    *ppServerInfo = pServerInfo;
    *pdwInfoCount = entryArray.iSize;

cleanup:
    VmDirFreeEntryArrayContent(&entryArray);
    return dwError;
error:
    goto cleanup;
}
