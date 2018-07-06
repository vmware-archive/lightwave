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
 * Module Name: Replication
 *
 * Filename: firstreplcycle.c
 *
 * Abstract: First replication cycle being implemented by copying the DB from partner, and "patching" it.
 *
 */

#include "includes.h"


/* Example of why you don't call public APIs internally */
int
LoadServerGlobals(
    BOOLEAN *pbWriteInvocationId
    );

static
int
_VmDirWrapUpFirstReplicationCycle(
    PCSTR                           pszHostname,
    VMDIR_REPLICATION_AGREEMENT *   pReplAgr);

static
DWORD
_VmDirSrvCreateReplAgrObj(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    PCSTR            pszReplURI,
    PCSTR            pszLastLocalUsnProcessed
    );

static
int
_VmDirFirstCycleCreateSrvObjTree(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    PCSTR           partnerName,
    PCSTR           pszLastLocalUsnProcessed
    );

static
DWORD
_VmDirInitJoinDBInfoGlobal(
    VOID
    );

int
VmDirFirstReplicationCycle(
    PCSTR                           pszPartnerHostname,    // partner node DSE Root ATTR_SERVER_NAME (server object cn).
    VMDIR_REPLICATION_AGREEMENT *   pReplAgr)
{
    int retVal = LDAP_SUCCESS;
    BOOLEAN bWriteInvocationId = FALSE;
    const char  *dbHomeDir = VMDIR_DB_DIR;

    assert(gFirstReplCycleMode == FIRST_REPL_CYCLE_MODE_COPY_DB);

    VmDirBkgdThreadShutdown();

    VmDirMetricsShutdown();

    //Shutdown local database
    VmDirShutdownDB();

    retVal = _VmDirInitJoinDBInfoGlobal();
    BAIL_ON_VMDIR_ERROR(retVal);

    if (gVdirReplJoinFlowInfo.bJoinWithPreCopiedDB)
    {
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s Join with pre-warm DB", __FUNCTION__);
    }
    else
    {
        retVal = VmDirCopyRemoteDB(pszPartnerHostname, dbHomeDir);
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    retVal = VmDirSwapDB(dbHomeDir);
    BAIL_ON_VMDIR_ERROR(retVal);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,"DB swapped successfully");

    // Wrap up the 1st replication cycle by updating replication cookies.
    retVal = _VmDirWrapUpFirstReplicationCycle(pszPartnerHostname, pReplAgr);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = LoadServerGlobals(&bWriteInvocationId);
    BAIL_ON_VMDIR_ERROR(retVal);

    if (gVdirReplJoinFlowInfo.bJoinWithPreCopiedDB)
    {
        assert(gVmdirKrbGlobals.pszRealm);
        assert(gVmdirServerGlobals.bvServerObjName.lberbv_val);

        retVal = VmDirCreateDomainController(
            gVmdirKrbGlobals.pszRealm,
            gVmdirServerGlobals.bvServerObjName.lberbv_val);
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    retVal = VmDirMetricsInitialize();
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirBkgdThreadInitialize();
    BAIL_ON_VMDIR_ERROR(retVal);

cleanup:
    return retVal;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL," error (%u)", retVal);

    goto cleanup;
}


/**
 * @ _VmDirShutdownDB()
 * shutdown the current backend
 * @return VOID
 */
VOID
VmDirShutdownDB(
    VOID
    )
{
    PVDIR_BACKEND_INTERFACE pBE = NULL;

    // Shutdown backend
    pBE = VmDirBackendSelect(NULL);
    assert(pBE);

    VmDirdStateSet(VMDIRD_STATE_SHUTDOWN);

    // in DR case, stop listening thread.
    // in Join case, listening thread is not in listen mode yet.
    VmDirShutdownConnAcceptThread();

    VmDirIndexLibShutdown();

    VmDirSchemaLibShutdown();

    pBE->pfnBEShutdown();
    VmDirBackendContentFree(pBE);
}

static
int
_VmDirFirstCycleCreateSrvObjTree(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    PCSTR           partnerName,
    PCSTR           pszLastLocalUsnProcessed
    )
{
    int     retVal = 0;
    PSTR    pszLdapURI = NULL;

    // set global serverID
    if (gVdirReplJoinFlowInfo.bJoinWithPreCopiedDB)
    {
        gVmdirServerGlobals.serverId = gVdirReplJoinFlowInfo.dwPreSetMaxServerId +1;
    }
    else
    {
        retVal = VmDirSetGlobalServerId();
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    // Update gVmdirKrbGlobals
    retVal = VmDirKrbInit();
    BAIL_ON_VMDIR_ERROR(retVal);

    // Create Server object and update maxServerID in system domain object
    retVal = VmDirSrvCreateServerObj(pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(retVal);

    // Create Replication Agreements container
    retVal = VmDirSrvCreateReplAgrsContainer(pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(retVal);

    // Create this Replication agreement
    retVal = VmDirAllocateStringPrintf(&pszLdapURI, "ldap://%s", partnerName);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = _VmDirSrvCreateReplAgrObj(pSchemaCtx, pszLdapURI, pszLastLocalUsnProcessed);
    BAIL_ON_VMDIR_ERROR(retVal);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s: Server object tree created", __FUNCTION__);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLdapURI);

    return retVal;

error:
    goto cleanup;
}

static
DWORD
_VmDirSrvCreateReplAgrObj(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    PCSTR            pszReplURI,
    PCSTR            pszLastLocalUsnProcessed
    )
{
    DWORD   dwError = 0;
    PSTR    pszDN = NULL;
    PSTR    ppszReplAgrObjAttrs[] =
    {
            ATTR_OBJECT_CLASS,                  OC_REPLICATION_AGREEMENT,
            ATTR_OBJECT_CLASS,                  OC_TOP,
            ATTR_LABELED_URI,                   (PSTR) pszReplURI,
            ATTR_LAST_LOCAL_USN_PROCESSED,      (PSTR) pszLastLocalUsnProcessed,
            NULL
    };

    dwError = VmDirAllocateStringPrintf(&pszDN,
        "%s=%s,%s=%s,%s",
        ATTR_LABELED_URI,
        pszReplURI,
        ATTR_CN,
        VMDIR_REPL_AGRS_CONTAINER_NAME,
        gVmdirServerGlobals.serverObjDN.lberbv.bv_val);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSimpleEntryCreate(pSchemaCtx, ppszReplAgrObjAttrs, pszDN, 0);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    VMDIR_SAFE_FREE_MEMORY(pszDN);

    return dwError;
}

static
int
_VmDirWrapUpFirstReplicationCycle(
    PCSTR                           pszHostname,
    VMDIR_REPLICATION_AGREEMENT *   pReplAgr)
{
    int                 retVal = LDAP_SUCCESS;
    struct berval       syncDoneCtrlVal = {0};
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;
    PVMDIR_SWAP_DB_INFO pSwapDBInfo = NULL;

    retVal = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirPrepareSwapDBInfo(pszHostname, &pSwapDBInfo);
    BAIL_ON_VMDIR_ERROR(retVal);

    // SyncDoneCtrolVal := <partnerLocalUSN>,<UTDVector>
    retVal = VmDirAllocateStringPrintf(&(syncDoneCtrlVal.bv_val),
        "%s,%s",
        pSwapDBInfo->pszMyHighWaterMark,
        pSwapDBInfo->pszMyUTDVcetor);
    BAIL_ON_VMDIR_ERROR(retVal);

    syncDoneCtrlVal.bv_len = VmDirStringLenA(syncDoneCtrlVal.bv_val);

    retVal = _VmDirFirstCycleCreateSrvObjTree(pSchemaCtx, pszHostname, pSwapDBInfo->pszMyHighWaterMark);
    BAIL_ON_VMDIR_ERROR(retVal);

    if ((retVal = VmDirReplCookieUpdate( pSchemaCtx, &(syncDoneCtrlVal), pReplAgr )) != LDAP_SUCCESS)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: UpdateCookies failed. Error: %d", __FUNCTION__, retVal);
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    if ((retVal = VmDirPatchDSERoot(pSchemaCtx)) != LDAP_SUCCESS)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: _VmDirPatchDSERoot failed. Error: %d", __FUNCTION__, retVal);
        BAIL_ON_VMDIR_ERROR(retVal);
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(syncDoneCtrlVal.bv_val);
    VmDirFreeSwapDBInfo(pSwapDBInfo);
    VmDirSchemaCtxRelease(pSchemaCtx);

    return retVal;

error:
    retVal = LDAP_OPERATIONS_ERROR;
    goto cleanup;
}

#ifndef VDIR_PSC_VERSION
#define VDIR_PSC_VERSION "6.7.0"
#endif

int
VmDirPatchDSERoot(
    PVDIR_SCHEMA_CTX    pSchemaCtx)
{
    int                      retVal = LDAP_SUCCESS;
    VDIR_OPERATION           op = {0};
    VDIR_BERVALUE            bvDSERootDN = VDIR_BERVALUE_INIT;

    VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "_VmDirPatchDSERoot: Begin" );

    bvDSERootDN.lberbv.bv_val = PERSISTED_DSE_ROOT_DN;
    bvDSERootDN.lberbv.bv_len = VmDirStringLenA( bvDSERootDN.lberbv.bv_val );

    retVal = VmDirInitStackOperation( &op,
                                      VDIR_OPERATION_TYPE_INTERNAL,
                                      LDAP_REQ_MODIFY,
                                      pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirNormalizeDN( &bvDSERootDN, pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirBervalContentDup( &bvDSERootDN, &op.reqDn );
    BAIL_ON_VMDIR_ERROR(retVal);

    op.pBEIF = VmDirBackendSelect(op.reqDn.lberbv.bv_val);
    assert(op.pBEIF);

    if (VmDirBervalContentDup( &op.reqDn, &op.request.modifyReq.dn ) != 0)
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "_VmDirPatchDSERoot: BervalContentDup failed." );
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    retVal = VmDirAppendAMod( &op, MOD_OP_REPLACE, ATTR_DC_ACCOUNT_UPN, ATTR_DC_ACCOUNT_UPN_LEN,
                              gVmdirServerGlobals.dcAccountUPN.lberbv.bv_val,
                              gVmdirServerGlobals.dcAccountUPN.lberbv.bv_len );
    BAIL_ON_VMDIR_ERROR( retVal );

    retVal = VmDirAppendAMod( &op, MOD_OP_REPLACE, ATTR_DC_ACCOUNT_DN, ATTR_DC_ACCOUNT_DN_LEN,
                              gVmdirServerGlobals.dcAccountDN.lberbv.bv_val,
                              gVmdirServerGlobals.dcAccountDN.lberbv.bv_len );
    BAIL_ON_VMDIR_ERROR( retVal );

    retVal = VmDirAppendAMod( &op, MOD_OP_REPLACE, ATTR_SERVER_NAME, ATTR_SERVER_NAME_LEN,
                              gVmdirServerGlobals.serverObjDN.lberbv.bv_val,
                              gVmdirServerGlobals.serverObjDN.lberbv.bv_len );
    BAIL_ON_VMDIR_ERROR( retVal );

    retVal = VmDirAppendAMod( &op, MOD_OP_REPLACE, ATTR_SITE_NAME, ATTR_SITE_NAME_LEN,
                              gVmdirServerGlobals.pszSiteName,
                              VmDirStringLenA(gVmdirServerGlobals.pszSiteName) );
    BAIL_ON_VMDIR_ERROR( retVal );

    retVal = VmDirAppendAMod( &op, MOD_OP_REPLACE, ATTR_PSC_VERSION, ATTR_PSC_VERSION_LEN,
                              VDIR_PSC_VERSION,
                              VmDirStringLenA(VDIR_PSC_VERSION) );
    BAIL_ON_VMDIR_ERROR( retVal );

    retVal = VmDirAppendAMod( &op, MOD_OP_REPLACE, ATTR_MAX_DOMAIN_FUNCTIONAL_LEVEL,
                              ATTR_MAX_DOMAIN_FUNCTIONAL_LEVEL_LEN,
                              VMDIR_MAX_DFL_STRING,
                              VmDirStringLenA(VMDIR_MAX_DFL_STRING) );
    BAIL_ON_VMDIR_ERROR( retVal );

    if ((retVal = VmDirInternalModifyEntry( &op )) != 0)
    {
        // If VmDirInternall call failed, reset retVal to LDAP level error space (for B/C)
        retVal = op.ldapResult.errCode;

        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "_VmDirPatchDSERoot: InternalModifyEntry failed. "
                  "Error code: %d, Error string: %s", retVal, VDIR_SAFE_STRING( op.ldapResult.pszErrMsg ) );
        BAIL_ON_VMDIR_ERROR( retVal );
    }

cleanup:
    VmDirFreeOperationContent(&op);
    VmDirFreeBervalContent(&bvDSERootDN);

    VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "_VmDirPatchDSERoot: End" );
    return retVal;

error:
    retVal = LDAP_OPERATIONS_ERROR;
    goto cleanup;
}

static
DWORD
_VmDirInitJoinDBInfoGlobal(
    VOID
    )
{
    DWORD   dwError = 0;

    gVdirReplJoinFlowInfo.bJoinWithPreCopiedDB = VmDirRegReadJoinWithPreCopiedDB();
    if (gVdirReplJoinFlowInfo.bJoinWithPreCopiedDB)
    {
        dwError = VmDirRegReadPreSetMaxServerId(&gVdirReplJoinFlowInfo.dwPreSetMaxServerId);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, " error (%d)", dwError);
    goto cleanup;
}
