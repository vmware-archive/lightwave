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

#if 0 /* Not used, as all of this depends on Kerberos */

#include "includes.h"

/* Example of why you don't call public APIs internally */
ULONG
VmDirCreateBindingHandleA(
    PCSTR      pszNetworkAddress,
    PCSTR      pszNetworkEndpoint,
    handle_t   *ppBinding
    );

static
int
_VmDirGetRemoteDBUsingRPC(
    PCSTR   pszHostname);

static
int
_VmDirGetRemoteDBFileUsingRPC(
    handle_t    hBinding,
    PCSTR       dbRemoteFilename);

static
int
_VmDirSwapDB();

static
int
_VmDirWrapUpFirstReplicationCycle(
    PCSTR                           pszHostname,
    VMDIR_REPLICATION_AGREEMENT *   pReplAgr);

static
int
_VmDirPatchDSERoot(
    PVDIR_SCHEMA_CTX    pSchemaCtx);

int
VmDirFirstReplicationCycle(
    PCSTR                           pszHostname,
    VMDIR_REPLICATION_AGREEMENT *   pReplAgr)
{
    int                     retVal = LDAP_SUCCESS;
    PSTR                    pszLocalErrorMsg = NULL;

    if ( gFirstReplCycleMode != FIRST_REPL_CYCLE_MODE_COPY_DB && gFirstReplCycleMode != FIRST_REPL_CYCLE_MODE_USE_COPIED_DB )
    {
        retVal = LDAP_SUCCESS;
        VmDirLog( LDAP_DEBUG_ANY,
                  "VmDirFirstReplicationCycle: Not a special first replication cycle mode, nothing is to be done." );
        goto cleanup;
    }

    if ( gFirstReplCycleMode == FIRST_REPL_CYCLE_MODE_COPY_DB )
    {
        retVal = _VmDirGetRemoteDBUsingRPC(pszHostname);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
                "VmDirFirstReplicationCycle: _VmDirGetRemoteDBUsingRPC() call failed with error: %d", retVal );

        retVal = _VmDirSwapDB();
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
                "VmDirFirstReplicationCycle: _VmDirSwapDB() call failed, error: %d.", retVal );
    }

    // Wrap up the 1st replication cycle by updating replication cookies.
    retVal = _VmDirWrapUpFirstReplicationCycle( pszHostname, pReplAgr );

    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "VmDirFirstReplicationCycle: _VmDirWrapUpFirstReplicationCycle() call failed, error: %d.", retVal );

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return retVal;

error:
    retVal = LDAP_OPERATIONS_ERROR;
    VmDirLog( LDAP_DEBUG_ANY, "%s", VDIR_SAFE_STRING(pszLocalErrorMsg) );
    goto cleanup;
}

#define VMDIR_MDB_DATA_FILE_NAME "data.mdb"
#define VMDIR_MDB_LOCK_FILE_NAME "lock.mdb"

static
int
_VmDirGetRemoteDBUsingRPC(
    PCSTR   pszHostname)
{
    DWORD       retVal = 0;
    PCSTR       pszServerEndpoint = NULL;
    handle_t    hBinding = NULL;
    PSTR        pszLocalErrorMsg = NULL;
    char        dbRemoteFilename[VMDIR_MAX_FILE_NAME_LEN] = {0};
    PSTR        pszDcAccountPwd = NULL;

#ifndef _WIN32
    const char  *dbHomeDir = VMDIR_DB_DIR;
    const char   fileSeperator = '/';
#else
    _TCHAR      dbHomeDir[MAX_PATH];
    const char   fileSeperator = '\\';

    retVal = VmDirMDBGetHomeDir(dbHomeDir);
    BAIL_ON_VMDIR_ERROR ( retVal );
#endif

    VmDirLog( LDAP_DEBUG_ANY, "_VmDirGetRemoteDBUsingRPC: Connecting to the replication partner (%s) ...", pszHostname );

    retVal = VmDirReadDCAccountPassword(&pszDcAccountPwd);
    BAIL_ON_VMDIR_ERROR( retVal );

    retVal = VmDirCacheKrb5Creds(gVmdirServerGlobals.dcAccountUPN.lberbv_val, pszDcAccountPwd);

    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
                "_VmDirGetRemoteDBUsingRPC: VmDirCacheKrb5Creds() call failed with error: %d", retVal );

    retVal = VmDirCreateBindingHandleA(pszHostname, pszServerEndpoint, &hBinding);

    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBUsingRPC: VmDirCreateBindingHandleA() call failed with error: %d, host name = %s",
            retVal, pszHostname  );

    retVal = VmDirSetStateH( NULL, VMDIRD_STATE_READ_ONLY );

    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
                "_VmDirGetRemoteDBUsingRPC: VmDirSetState() call failed with error: %d", retVal  );

    retVal = VmDirStringPrintFA( dbRemoteFilename, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s", dbHomeDir, fileSeperator,
                                 VMDIR_MDB_DATA_FILE_NAME );

    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBUsingRPC: VmDirStringPrintFA() call failed with error: %d", retVal );

    VmDirLog( LDAP_DEBUG_ANY, "_VmDirGetRemoteDBUsingRPC: receiving the DB file ... : %s", dbRemoteFilename );

    retVal = _VmDirGetRemoteDBFileUsingRPC( hBinding, dbRemoteFilename );

    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBUsingRPC: _VmDirGetRemoteDBFileUsingRPC() call failed with error: %d", retVal );

    retVal = VmDirStringPrintFA( dbRemoteFilename, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s", dbHomeDir, fileSeperator,
                                 VMDIR_MDB_LOCK_FILE_NAME );

    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBUsingRPC: VmDirStringPrintFA() call failed with error: %d", retVal );

    VmDirLog( LDAP_DEBUG_ANY, "_VmDirGetRemoteDBUsingRPC: receiving the DB file ... : %s", dbRemoteFilename );

    retVal = _VmDirGetRemoteDBFileUsingRPC( hBinding, dbRemoteFilename );

    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBUsingRPC: _VmDirGetRemoteDBFileUsingRPC() call failed with error: %d", retVal );

cleanup:
    if (hBinding)
    {
        DWORD       localRetVal = 0;
        if ((localRetVal = VmDirSetState( hBinding, VMDIRD_STATE_NORMAL )) != 0)
        {
            VmDirLog( LDAP_DEBUG_ANY, "_VmDirGetRemoteDBUsingRPC: VmDirSetState() call failed with error: %d", localRetVal );
        }

        retVal = (retVal != 0) ? retVal : localRetVal;

        VmDirFreeBindingHandle( &hBinding);
    }
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    VMDIR_SECURE_FREE_STRINGA(pszDcAccountPwd);
    return retVal;

error:
    retVal = LDAP_OPERATIONS_ERROR;
    VmDirLog( LDAP_DEBUG_ANY, "%s", VDIR_SAFE_STRING(pszLocalErrorMsg) );
    goto cleanup;
}

static
int
_VmDirGetRemoteDBFileUsingRPC(
    handle_t    hBinding,
    PCSTR       dbRemoteFilename)
{
#define VMDIR_DB_READ_BLOCK_SIZE     5000000

    DWORD       retVal = 0;
    char        dbLocalFilename[VMDIR_MAX_FILE_NAME_LEN] = {0};
    FILE *      pLocalFile = NULL;
    FILE *      pRemoteFile = 0;
    UINT32      dwCount = 0;
    PBYTE       pReadBuffer = NULL;
    PSTR        pszLocalErrorMsg = NULL;
    UINT32      writeSize = 0;

    retVal = VmDirOpenDBFile( hBinding, dbRemoteFilename, &pRemoteFile );

    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBFileUsingRPC: RpcVmDirOpenDBFile() call failed with error: %d", retVal );

    // Construct local file name
    if ((retVal = VmDirStringPrintFA( dbLocalFilename, VMDIR_MAX_FILE_NAME_LEN, "%s%s", dbRemoteFilename,
                                      ".partner" )) != 0)
    {
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
                "_VmDirGetRemoteDBFileUsingRPC: VmDirStringPrintFA() call failed." );
    }

    if ((pLocalFile = fopen(dbLocalFilename, "wb")) == NULL)
    {
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBFileUsingRPC: open() call failed, error(%d): %s.", errno, strerror(errno) );
    }

    for (;;)
    {
        dwCount = VMDIR_DB_READ_BLOCK_SIZE;

        retVal = VmDirReadDBFile( hBinding, pRemoteFile, &dwCount, &pReadBuffer );

        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
                "_VmDirGetRemoteDBFileUsingRPC: RpcVmDirReadDBFile() call failed with error: %d", retVal );

        writeSize = (UINT32)fwrite(pReadBuffer, 1, dwCount, pLocalFile);

        VMDIR_SAFE_FREE_MEMORY(pReadBuffer);

        if (writeSize < dwCount)
        {
            retVal = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
                    "_VmDirGetRemoteDBFileUsingRPC: write() call failed, recvSize: %d, writeSize: %d.",
                    dwCount, writeSize );
        }
        if (dwCount < VMDIR_DB_READ_BLOCK_SIZE)
        {
            VmDirLog( LDAP_DEBUG_ANY, "DONE copying the file %s \n", dbLocalFilename);
            break;
        }
    }

cleanup:
    if (pRemoteFile != 0)
    {
        DWORD       localRetVal = 0;
        if ((localRetVal = VmDirCloseDBFile( hBinding, pRemoteFile )) != 0)
        {
            VmDirLog( LDAP_DEBUG_ANY, "_VmDirGetRemoteDBFileUsingRPC: RpcVmDirCloseDBFile() call failed with error: %d",
                      localRetVal );
        }
        retVal = (retVal != 0) ? retVal : localRetVal;
    }
    if (pLocalFile != NULL)
    {
        fclose(pLocalFile);
    }
    VMDIR_SAFE_FREE_MEMORY(pReadBuffer);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return retVal;

error:
    retVal = LDAP_OPERATIONS_ERROR;
    VmDirLog( LDAP_DEBUG_ANY, "%s", VDIR_SAFE_STRING(pszLocalErrorMsg) );
    goto cleanup;
}

static
int
_VmDirSwapDB()
{
    int                     retVal = LDAP_SUCCESS;
    char                    dbExistingFilename[VMDIR_MAX_FILE_NAME_LEN] = {0};
    char                    dbNewFilename[VMDIR_MAX_FILE_NAME_LEN] = {0};
    char                    moveFileCmdLine[7 /* max of "mv" and "move /y" */ + 1 + VMDIR_MAX_FILE_NAME_LEN + 1 +
                                            VMDIR_MAX_FILE_NAME_LEN] = {0};
    PVDIR_BACKEND_INTERFACE pBE = NULL;
    PSTR                    pszLocalErrorMsg = NULL;

#ifndef _WIN32
    const char  *dbHomeDir = VMDIR_DB_DIR;
    const char * moveFileCmd = "mv";
    const char   fileSeperator = '/';
#else
    _TCHAR      dbHomeDir[MAX_PATH];
    const char * moveFileCmd = "move /y";
    const char   fileSeperator = '\\';

    retVal = VmDirMDBGetHomeDir(dbHomeDir);
    BAIL_ON_VMDIR_ERROR ( retVal );
#endif

    // Shutdown backend
    pBE = VmDirBackendSelect(NULL);
    assert(pBE);

    pBE->pfnBEShutdown();
    VmDirBackendContentFree(pBE);

    VmDirAttrIndexLibShutdown();

    VmDirSchemaLibShutdown();

    VmDirdStateSet(VMDIRD_STATE_SHUTDOWN);

    // move .mdb files
    retVal = VmDirStringPrintFA( dbExistingFilename, VMDIR_MAX_FILE_NAME_LEN, "\"%s%c%s%s\"", dbHomeDir, fileSeperator,
                                 VMDIR_MDB_DATA_FILE_NAME, ".partner" );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "VmDirFirstReplicationCycle: VmDirStringPrintFA() call failed with error: %d", retVal );

    retVal = VmDirStringPrintFA( dbNewFilename, VMDIR_MAX_FILE_NAME_LEN, "\"%s%c%s\"", dbHomeDir, fileSeperator,
                                 VMDIR_MDB_DATA_FILE_NAME );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "VmDirFirstReplicationCycle: VmDirStringPrintFA() call failed with error: %d", retVal );

    retVal = VmDirStringPrintFA( moveFileCmdLine, sizeof(moveFileCmdLine), "%s %s %s", moveFileCmd,
                                 dbExistingFilename, dbNewFilename  );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "VmDirFirstReplicationCycle: VmDirStringPrintFA() call failed with error: %d", retVal );

    retVal = VmDirRun(moveFileCmdLine);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "VmDirFirstReplicationCycle: VmDirRun() call failed, cmd: %s", moveFileCmdLine );

    retVal = VmDirStringPrintFA( dbExistingFilename, VMDIR_MAX_FILE_NAME_LEN, "\"%s%c%s%s\"", dbHomeDir, fileSeperator,
                                 VMDIR_MDB_LOCK_FILE_NAME, ".partner" );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "VmDirFirstReplicationCycle: VmDirStringPrintFA() call failed with error: %d", retVal );

    retVal = VmDirStringPrintFA( dbNewFilename, VMDIR_MAX_FILE_NAME_LEN, "\"%s%c%s\"", dbHomeDir, fileSeperator,
                                 VMDIR_MDB_LOCK_FILE_NAME );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "VmDirFirstReplicationCycle: VmDirStringPrintFA() call failed with error: %d", retVal );

    retVal = VmDirStringPrintFA( moveFileCmdLine, sizeof(moveFileCmdLine), "%s %s %s", moveFileCmd,
                                 dbExistingFilename, dbNewFilename );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "VmDirFirstReplicationCycle: VmDirStringPrintFA() call failed with error: %d", retVal );

    retVal = VmDirRun(moveFileCmdLine);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "VmDirFirstReplicationCycle: VmDirRun() call failed, cmd: %s", moveFileCmdLine );

    VmDirdStateSet(VMDIRD_STATE_STARTUP);

    retVal = VmDirInitBackend();
    BAIL_ON_VMDIR_ERROR(retVal);

    VmDirdStateSet(VMDIRD_STATE_NORMAL);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return retVal;

error:
    retVal = LDAP_OPERATIONS_ERROR;
    VmDirLog( LDAP_DEBUG_ANY, "%s", VDIR_SAFE_STRING(pszLocalErrorMsg) );
    goto cleanup;
}

static
int
_VmDirWrapUpFirstReplicationCycle(
    PCSTR                           pszHostname,
    VMDIR_REPLICATION_AGREEMENT *   pReplAgr)
{
    int                 retVal = LDAP_SUCCESS;
    PVDIR_ENTRY         pPartnerServerEntry = NULL;
    PVDIR_ATTRIBUTE     pAttrUpToDateVector = NULL;
    PVDIR_ATTRIBUTE     pAttrInvocationId = NULL;
    USN                 localUsn = 0;
    USN                 partnerLocalUsn = 0;
    char                partnerlocalUsnStr[VMDIR_MAX_USN_STR_LEN];
    VDIR_BACKEND_CTX    beCtx = {0};
    struct berval       syncDoneCtrlVal = {0};
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;
    VDIR_OPERATION      searchOp = {0};
    PVDIR_FILTER        pSearchFilter = NULL;

    retVal = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR( retVal );

    retVal = VmDirInitStackOperation( &searchOp, VDIR_OPERATION_TYPE_INTERNAL, LDAP_REQ_SEARCH, pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(retVal);

    searchOp.pBEIF = VmDirBackendSelect(NULL);
    assert(searchOp.pBEIF);

    searchOp.reqDn.lberbv.bv_val = "";
    searchOp.reqDn.lberbv.bv_len = 0;
    searchOp.request.searchReq.scope = LDAP_SCOPE_SUBTREE;

    retVal = VmDirConcatTwoFilters(searchOp.pSchemaCtx, ATTR_CN, (PSTR) pszHostname, ATTR_OBJECT_CLASS, OC_DIR_SERVER,
                                    &pSearchFilter);
    BAIL_ON_VMDIR_ERROR(retVal);

    searchOp.request.searchReq.filter = pSearchFilter;

    retVal = VmDirInternalSearch(&searchOp);
    BAIL_ON_VMDIR_ERROR(retVal);

    if (searchOp.internalSearchEntryArray.iSize != 1)
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                    "_VmDirWrapUpFirstReplicationCycle: Unexpected (not 1) number of partner server entries found (%d)",
                    searchOp.internalSearchEntryArray.iSize );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    pPartnerServerEntry = searchOp.internalSearchEntryArray.pEntry;

    pAttrUpToDateVector = VmDirEntryFindAttribute( ATTR_UP_TO_DATE_VECTOR, pPartnerServerEntry );

    pAttrInvocationId = VmDirEntryFindAttribute( ATTR_INVOCATION_ID, pPartnerServerEntry );
    assert( pAttrInvocationId != NULL );

    beCtx.pBE = VmDirBackendSelect(NULL);
    assert(beCtx.pBE);

    if ((retVal = beCtx.pBE->pfnBEGetNextUSN( &beCtx, &localUsn )) != 0)
    {
        VmDirLog( LDAP_DEBUG_ANY, "_VmDirWrapUpFirstReplicationCycle: pfnBEGetNextUSN failed with error code: %d, "
                  "error message: %s", retVal, VDIR_SAFE_STRING(beCtx.pszBEErrorMsg) );
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    partnerLocalUsn = localUsn - 2; // 1 extra we got above, and 1 acquired by VmDirBackendInitUSNList() in _VmDirSwapDB()

    if ((retVal = VmDirStringNPrintFA( partnerlocalUsnStr, sizeof(partnerlocalUsnStr), sizeof(partnerlocalUsnStr) - 1,
                                       "%ld", partnerLocalUsn)) != 0)
    {
        VmDirLog( LDAP_DEBUG_ANY, "_VmDirWrapUpFirstReplicationCycle: VmDirStringNPrintFA failed with error code: %d",
                  retVal );
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    if (pAttrUpToDateVector)
    {
        // <partnerLocalUSN>,<partner up-to-date vector>,<partner server GUID>:<partnerLocalUSN>,
        retVal = VmDirAllocateStringAVsnprintf( &(syncDoneCtrlVal.bv_val), "%s,%s,%s:%s,",
                                                partnerlocalUsnStr,
                                                pAttrUpToDateVector->vals[0].lberbv.bv_val,
                                                pAttrInvocationId->vals[0].lberbv.bv_val,
                                                partnerlocalUsnStr);
        BAIL_ON_VMDIR_ERROR(retVal);
    }
    else
    {
        // <partnerLocalUSN>,<partner server GUID>:<partnerLocalUSN>,
        retVal = VmDirAllocateStringAVsnprintf( &(syncDoneCtrlVal.bv_val), "%s,%s:%s,",
                                                partnerlocalUsnStr,
                                                pAttrInvocationId->vals[0].lberbv.bv_val,
                                                partnerlocalUsnStr);
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    syncDoneCtrlVal.bv_len = VmDirStringLenA(syncDoneCtrlVal.bv_val);

    if ((retVal = VmDirReplUpdateCookies( pSchemaCtx, &(syncDoneCtrlVal), pReplAgr )) != LDAP_SUCCESS)
    {
        VmDirLog( LDAP_DEBUG_ANY, "vdirReplicationThrFun: UpdateCookies failed. Error: %d", retVal );
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    if ((retVal = _VmDirPatchDSERoot(pSchemaCtx)) != LDAP_SUCCESS)
    {
        VmDirLog( LDAP_DEBUG_ANY, "vdirReplicationThrFun: _VmDirPatchDSERoot failed. Error: %d", retVal );
        BAIL_ON_VMDIR_ERROR( retVal );
    }

cleanup:
    VmDirFreeOperationContent(&searchOp);
    VmDirBackendCtxContentFree(&beCtx);
    VMDIR_SAFE_FREE_MEMORY(syncDoneCtrlVal.bv_val);
    VmDirSchemaCtxRelease(pSchemaCtx);
    return retVal;

error:
    retVal = LDAP_OPERATIONS_ERROR;
    goto cleanup;
}

static
int
_VmDirPatchDSERoot(
    PVDIR_SCHEMA_CTX    pSchemaCtx)
{
    int                      retVal = LDAP_SUCCESS;
    VDIR_OPERATION           op = {0};
    VDIR_BERVALUE            bvDSERootDN = VDIR_BERVALUE_INIT;

    VmDirLog( LDAP_DEBUG_TRACE, "_VmDirPatchDSERoot: Begin" );

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
        VmDirLog( LDAP_DEBUG_ANY, "_VmDirPatchDSERoot: BervalContentDup failed." );
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

    if ((retVal = VmDirInternalModifyEntry( &op )) != 0)
    {
        // If VmDirInternall call failed, reset retVal to LDAP level error space (for B/C)
        retVal = op.ldapResult.errCode;

        VmDirLog( LDAP_DEBUG_ANY, "_VmDirPatchDSERoot: InternalModifyEntry failed. "
                  "Error code: %d, Error string: %s", retVal, VDIR_SAFE_STRING( op.ldapResult.pszErrMsg ) );
        BAIL_ON_VMDIR_ERROR( retVal );
    }

cleanup:
    VmDirFreeOperationContent(&op);

    VmDirLog( LDAP_DEBUG_TRACE, "_VmDirPatchDSERoot: End" );
    return retVal;

error:
    retVal = LDAP_OPERATIONS_ERROR;
    goto cleanup;
}

#endif /* if 0 */
