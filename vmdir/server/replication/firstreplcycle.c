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
#define LOCAL_PARTNER_DIR "partner"
#define VMDIR_MDB_DATA_FILE_NAME "data.mdb"

//Read data.mdb up to last_pgno + the safe margin (256MB). The margin is for the latency
// between calling mdb_env_set_state to obtain the dbSizeMb and the meta page(first block) is read,
// i.e. the provision at the remote server wouldn't increase DB size by 256MB during this period.
// For cold copy (remote database at read-only mode), VMDIR_MDB_COPY_SAFE_MARGIN is set to 0.
#define VMDIR_MDB_COPY_SAFE_MARGIN 0

/* Example of why you don't call public APIs internally */
ULONG
VmDirCreateBindingHandleA(
    PCSTR      pszNetworkAddress,
    PCSTR      pszNetworkEndpoint,
    handle_t   *ppBinding
    );

int
LoadServerGlobals(BOOLEAN *pbWriteInvocationId);

static
int
_VmDirGetRemoteDBUsingRPC(
    PCSTR   pszHostname,
    PCSTR   dbHomeDir);

static
int
_VmDirGetRemoteDBFileUsingRPC(
    PVMDIR_SERVER_CONTEXT hServer,
    PCSTR       dbRemoteFilename,
    PCSTR       localFilename,
    UINT32      remoteFileSizeMb,
    UINT32      remoteDBMapSizeMb);

static
int
_VmDirSwapDB(
    PCSTR   dbHomeDir);

static
int
_VmDirWrapUpFirstReplicationCycle(
    PCSTR                           pszHostname,
    VMDIR_REPLICATION_AGREEMENT *   pReplAgr);

static
int
_VmDirPatchDSERoot(
    PVDIR_SCHEMA_CTX    pSchemaCtx);

VOID
VmDirFreeBindingHandle(
    handle_t *ppBinding
    );

static
int
_VmGetHighestCommittedUSN(
    USN startUsn,
    USN *hcUsn);

static
DWORD
_VmDirMkdir(
    PCSTR path,
    int mode);
int
VmDirFirstReplicationCycle(
    PCSTR                           pszHostname,
    VMDIR_REPLICATION_AGREEMENT *   pReplAgr)
{
    int                     retVal = LDAP_SUCCESS;
    PSTR                    pszLocalErrorMsg = NULL;
    BOOLEAN                 bWriteInvocationId = FALSE;
#ifndef _WIN32
    const char  *dbHomeDir = VMDIR_DB_DIR;
#else
    _TCHAR      dbHomeDir[MAX_PATH];
    size_t last_char_pos = 0;
    const char   fileSeperator = '\\';

    retVal = VmDirMDBGetHomeDir(dbHomeDir);
    BAIL_ON_VMDIR_ERROR ( retVal );
    last_char_pos = strlen(dbHomeDir) - 1;
    if (dbHomeDir[last_char_pos] == fileSeperator)
    {
        dbHomeDir[last_char_pos] = '\0';
    }
#endif

    if ( gFirstReplCycleMode != FIRST_REPL_CYCLE_MODE_COPY_DB )
    {
        retVal = LDAP_OPERATIONS_ERROR;
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                  "VmDirFirstReplicationCycle: Not a special first replication cycle mode, nothing is to be done." );
        goto cleanup;
    }

    retVal = _VmDirGetRemoteDBUsingRPC(pszHostname, dbHomeDir);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
                "VmDirFirstReplicationCycle: _VmDirGetRemoteDBUsingRPC() call failed with error: %d", retVal );

    retVal = _VmDirSwapDB(dbHomeDir);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
                "VmDirFirstReplicationCycle: _VmDirSwapDB() call failed, error: %d.", retVal );

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Remote DB copied from %s, and swapped successfully", pszHostname);

    // Wrap up the 1st replication cycle by updating replication cookies.
    retVal = _VmDirWrapUpFirstReplicationCycle( pszHostname, pReplAgr );

    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "VmDirFirstReplicationCycle: _VmDirWrapUpFirstReplicationCycle() call failed, error: %d.", retVal );

    retVal = LoadServerGlobals(&bWriteInvocationId);

    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "VmDirFirstReplicationCycle: LoadServerGlobals call failed, error: %d.", retVal );
cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return retVal;

error:
    retVal = LDAP_OPERATIONS_ERROR;
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s", VDIR_SAFE_STRING(pszLocalErrorMsg) );
    goto cleanup;
}

static
int
_VmDirGetRemoteDBUsingRPC(
    PCSTR   pszHostname,
    PCSTR   dbHomeDir)
{
    DWORD       retVal = 0;
    PSTR        pszLocalErrorMsg = NULL;
    char        dbRemoteFilename[VMDIR_MAX_FILE_NAME_LEN] = {0};
    char        localDir[VMDIR_MAX_FILE_NAME_LEN] = {0};
    char        localFilename[VMDIR_MAX_FILE_NAME_LEN] = {0};
    PSTR        pszDcAccountPwd = NULL;
    PVMDIR_SERVER_CONTEXT hServer = NULL;
    DWORD       low_xlognum = 0;
    DWORD       xlognum = 0;
    DWORD       remoteDbSizeMb = 0;
    DWORD       remoteDbMapSizeMb = 0;
    PBYTE       pDbPath = NULL;
#ifndef _WIN32
    const char   fileSeperator = '/';
#else
    const char   fileSeperator = '\\';
#endif

    retVal = VmDirAllocateMemory(VMDIR_MAX_FILE_NAME_LEN, (PVOID)&pDbPath );
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirReadDCAccountPassword(&pszDcAccountPwd);
    BAIL_ON_VMDIR_ERROR( retVal );

    retVal = VmDirOpenServerA(pszHostname, gVmdirServerGlobals.dcAccountUPN.lberbv_val, NULL, pszDcAccountPwd, 0, NULL, &hServer);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBUsingRPC: VmDirOpenServerA() call failed with error: %d, host name = %s",
            retVal, pszHostname  );
    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "_VmDirGetRemoteDBUsingRPC: Connected to the replication partner (%s).", pszHostname );

    //Set backend to read-only mode - currently no backend is supporting WAL yet.
    retVal = VmDirSetBackendState (hServer, 1, &low_xlognum, &remoteDbSizeMb, &remoteDbMapSizeMb, pDbPath, VMDIR_MAX_FILE_NAME_LEN);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBUsingRPC: VmDirSetBackendState failed with error: %d", retVal  );

    retVal = VmDirStringPrintFA( localDir, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s", dbHomeDir, fileSeperator, LOCAL_PARTNER_DIR);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBUsingRPC: VmDirStringPrintFA() call failed with error: %d", retVal );

    retVal = _VmDirMkdir(localDir, 0700);
    BAIL_ON_VMDIR_ERROR( retVal );

    retVal = VmDirStringPrintFA( dbRemoteFilename, VMDIR_MAX_FILE_NAME_LEN, "%s/%s", (char *)pDbPath,
                                 VMDIR_MDB_DATA_FILE_NAME );

    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBUsingRPC: VmDirStringPrintFA() call failed with error: %d", retVal );

    retVal = VmDirStringPrintFA( localFilename, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s%c%s", dbHomeDir,
                                 fileSeperator, LOCAL_PARTNER_DIR, fileSeperator, VMDIR_MDB_DATA_FILE_NAME );

    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBUsingRPC: VmDirStringPrintFA() call failed with error: %d", retVal );

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "_VmDirGetRemoteDBUsingRPC: copying remote file %s with data size %ld MB with Map size %ld MB ...",
                    dbRemoteFilename, remoteDbSizeMb, remoteDbMapSizeMb );

    retVal = _VmDirGetRemoteDBFileUsingRPC( hServer, dbRemoteFilename, localFilename, remoteDbSizeMb, remoteDbMapSizeMb );
    BAIL_ON_VMDIR_ERROR( retVal );

cleanup:
    if (hServer)
    {
        //clear backend read-only mode
        VmDirSetBackendState (hServer, 0, &xlognum, &remoteDbSizeMb, &remoteDbMapSizeMb, pDbPath, VMDIR_MAX_FILE_NAME_LEN);
        VmDirCloseServer( hServer);
    }
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    VMDIR_SAFE_FREE_MEMORY(pDbPath);
    VMDIR_SECURE_FREE_STRINGA(pszDcAccountPwd);
    return retVal;

error:
    retVal = LDAP_OPERATIONS_ERROR;
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s", VDIR_SAFE_STRING(pszLocalErrorMsg) );
    goto cleanup;
}

DWORD
VmDirReadDatabaseFile(
    PVMDIR_SERVER_CONTEXT   hServer,
    FILE *                  pFileHandle,
    UINT32 *                pdwCount,
    PBYTE                   pReadBuffer,
    UINT32                  bufferSize);

DWORD
VmDirCloseDatabaseFile(
    PVMDIR_SERVER_CONTEXT   hServer,
    FILE **                 ppFileHandle);

static
int
_VmDirGetRemoteDBFileUsingRPC(
    PVMDIR_SERVER_CONTEXT hServer,
    PCSTR       dbRemoteFilename,
    PCSTR       localFilename,
    UINT32      remoteFileSizeMb,
    UINT32      remoteDbMapSizeMb)
{
//read block size of one MB.
#define VMDIR_DB_READ_BLOCK_SIZE     (1<<20)

    DWORD       retVal = 0;
#ifdef _WIN32
    HANDLE      localFileFd = INVALID_HANDLE_VALUE;
    LONG        sizelo = 0, sizehi = 0;
#else
    int         localFileFd = 0;
#endif

    FILE *      pRemoteFile = 0;
    UINT32      dwCount = 0;
    PBYTE       pReadBuffer = NULL;
    PSTR        pszLocalErrorMsg = NULL;
    DWORD       writeSize = 0;
    long long   readSizeTotal = 0;
    unsigned long readInc = 0;
    unsigned long totalWriteMb = 0;

    retVal = VmDirOpenDatabaseFile( hServer, dbRemoteFilename, &pRemoteFile);

    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBFileUsingRPC: RpcVmDirOpenDatabaseFile failed on remote file %s with error: %d", dbRemoteFilename, retVal );
#ifdef _WIN32
    if((localFileFd = CreateFile(localFilename, GENERIC_WRITE, 0,
                                 NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL))==INVALID_HANDLE_VALUE)
    {
        errno = GetLastError();
#else
    if ((localFileFd = creat(localFilename, S_IRUSR|S_IWUSR)) < 0)
    {
#endif
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBFileUsingRPC: creat local file %s failed, error %d",dbRemoteFilename, errno);
    }

    retVal = VmDirAllocateMemory(VMDIR_DB_READ_BLOCK_SIZE, (PVOID)&pReadBuffer );
    BAIL_ON_VMDIR_ERROR(retVal);

    for (;;)
    {
        retVal = VmDirReadDatabaseFile( hServer, pRemoteFile, &dwCount, pReadBuffer, VMDIR_DB_READ_BLOCK_SIZE);

        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
                "_VmDirGetRemoteDBFileUsingRPC: RpcVmDirReadDatabaseFile() failed on remote file %s with error: %d", dbRemoteFilename, retVal );

        if (dwCount==0)
        {
            break;
        }
#ifdef _WIN32
        if(WriteFile(localFileFd, pReadBuffer, dwCount, &writeSize, NULL)!=0)
        {
            writeSize = -1;
        }
#else
        writeSize = (UINT32)write(localFileFd, pReadBuffer, dwCount);
#endif

        if (writeSize < dwCount)
        {
            retVal = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
                    "_VmDirGetRemoteDBFileUsingRPC: write() call failed, recvSize: %d, writeSize: %d.",
                    dwCount, writeSize );
        }
        readSizeTotal += (long long)dwCount;
        readInc += dwCount;
        if (readInc >= (1L<<30))
        {
            // Log progress every 1 GB read.
            readInc = 0;
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirGetRemoteDBFileUsingRPC: remote file %s %lu MB copied ...",
                   dbRemoteFilename, readSizeTotal/(1L<<20));
        }

        if (dwCount < VMDIR_DB_READ_BLOCK_SIZE || (remoteFileSizeMb > 0 &&
            readSizeTotal > (((long long)remoteFileSizeMb << 20) + (long long)VMDIR_MDB_COPY_SAFE_MARGIN)))
        {
            //data.mdb may grow (during hot copy) when there is writing activity at the remote server.
            //But there is no need to copy the growning part beyond a safe margin.
            break;
        }
    }
    totalWriteMb = (unsigned long)(readSizeTotal/(1L<<20));

#ifdef _WIN32
    sizelo = (remoteDbMapSizeMb << 20 ) & 0xffffffff;
    sizehi = (LONG)(remoteDbMapSizeMb >> 12);
    if (remoteDbMapSizeMb > 0)
    {
        if (SetFilePointer(localFileFd, sizelo, &sizehi, FILE_BEGIN) != (DWORD)sizelo || !SetEndOfFile(localFileFd))
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirGetRemoteDBFileUsingRPC:: failed to extend file %s to %d MB", localFilename, remoteDbMapSizeMb);
            retVal = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR( retVal );
        }
    }
#endif
    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Complete copying remote file %s with actual size %lu MB", dbRemoteFilename, totalWriteMb);

cleanup:
    if (pRemoteFile != NULL)
    {
        DWORD       localRetVal = 0;
        if ((localRetVal = VmDirCloseDatabaseFile( hServer, &pRemoteFile )) != 0)
        {
            VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "_VmDirGetRemoteDBFileUsingRPC: RpcVmDirCloseDatabaseFile() call failed with error: %d",
                      localRetVal );
        }
        retVal = (retVal != 0) ? retVal : localRetVal;
    }
#ifdef _WIN32
    if (localFileFd != INVALID_HANDLE_VALUE)
    {
        CloseHandle(localFileFd);
    }
#else
    if (localFileFd >= 0)
    {
        close(localFileFd);
    }
#endif
    VMDIR_SAFE_FREE_MEMORY(pReadBuffer);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return retVal;

error:
    retVal = LDAP_OPERATIONS_ERROR;
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s", VDIR_SAFE_STRING(pszLocalErrorMsg) );
    goto cleanup;
}

static
int
_VmDirSwapDB(
    PCSTR dbHomeDir)
{
    int                     retVal = LDAP_SUCCESS;
    char                    dbExistingFilename[VMDIR_MAX_FILE_NAME_LEN] = {0};
    char                    dbNewFilename[VMDIR_MAX_FILE_NAME_LEN] = {0};
    PVDIR_BACKEND_INTERFACE pBE = NULL;
    PSTR                    pszLocalErrorMsg = NULL;
    int                     errorCode = 0;
    BOOLEAN                 bLegacyDataLoaded = FALSE;

#ifndef _WIN32
    const char   fileSeperator = '/';
#else
    const char   fileSeperator = '\\';
#endif

    // Shutdown backend
    pBE = VmDirBackendSelect(NULL);
    assert(pBE);

    VmDirdStateSet(VMDIRD_STATE_SHUTDOWN);

    VmDirIndexLibShutdown();

    VmDirSchemaLibShutdown();

    pBE->pfnBEShutdown();
    VmDirBackendContentFree(pBE);

    // move .mdb files
    retVal = VmDirStringPrintFA( dbExistingFilename, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s%c%s", dbHomeDir, fileSeperator,
                                 LOCAL_PARTNER_DIR, fileSeperator, VMDIR_MDB_DATA_FILE_NAME);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirSwapDB: VmDirStringPrintFA() call failed with error: %d", retVal );

    retVal = VmDirStringPrintFA( dbNewFilename, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s", dbHomeDir, fileSeperator,
                                 VMDIR_MDB_DATA_FILE_NAME );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirSwapDB: VmDirStringPrintFA() call failed with error: %d", retVal );

#ifdef WIN32
    if (MoveFileEx(dbExistingFilename, dbNewFilename, MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING) == 0)
    {
        retVal = LDAP_OPERATIONS_ERROR;
        errorCode = GetLastError();
#else
    if (rename(dbExistingFilename, dbNewFilename) != 0)
    {
        retVal = LDAP_OPERATIONS_ERROR;
        errorCode = errno;
#endif
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirSwapDB: rename file from %s to %s failed, errno %d", dbExistingFilename, dbNewFilename, errorCode );
    }

    retVal = VmDirStringPrintFA( dbExistingFilename, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s", dbHomeDir, fileSeperator, LOCAL_PARTNER_DIR);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirSwapDB: VmDirStringPrintFA() call failed with error: %d", retVal );

#ifdef WIN32
    if (RemoveDirectory(dbExistingFilename)==0)
    {
        errorCode = GetLastError();
#else
    if (rmdir(dbExistingFilename) != 0)
    {
        errorCode = errno;
#endif
        VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL, "_VmDirSwapDB cannot remove directory %s, errno %d", dbExistingFilename, errorCode);
    }

    VmDirdStateSet(VMDIRD_STATE_STARTUP);

    retVal = VmDirInitBackend(&bLegacyDataLoaded);
    BAIL_ON_VMDIR_ERROR(retVal);

    if (bLegacyDataLoaded)
    {
        retVal = VmDirPatchLocalSubSchemaSubEntry();
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrorMsg,
                "_VmDirSwapDB: failed to patch subschema subentry: %d", retVal );

        retVal = VmDirWriteSchemaObjects();
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrorMsg,
                "_VmDirSwapDB: failed to create schema tree: %d", retVal );
    }

    VmDirdStateSet(VMDIRD_STATE_NORMAL);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return retVal;

error:
    retVal = LDAP_OPERATIONS_ERROR;
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s", VDIR_SAFE_STRING(pszLocalErrorMsg) );
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
    PSTR                pszSeparator = NULL;

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
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "_VmDirWrapUpFirstReplicationCycle: pfnBEGetNextUSN failed with error code: %d, "
                  "error message: %s", retVal, VDIR_SAFE_STRING(beCtx.pszBEErrorMsg) );
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    retVal = _VmGetHighestCommittedUSN(localUsn, &partnerLocalUsn);
    BAIL_ON_VMDIR_ERROR( retVal );

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "_VmDirWrapUpFirstReplicationCycle: partnerLocalUsn %llu locaUsn %llu", partnerLocalUsn, localUsn);

    if ((retVal = VmDirStringNPrintFA( partnerlocalUsnStr, sizeof(partnerlocalUsnStr), sizeof(partnerlocalUsnStr) - 1,
                                       "%ld", partnerLocalUsn)) != 0)
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "_VmDirWrapUpFirstReplicationCycle: VmDirStringNPrintFA failed with error code: %d",
                  retVal );
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    if (pAttrUpToDateVector)
    {
        if (VmDirStringEndsWith( pAttrUpToDateVector->vals[0].lberbv.bv_val, ",", FALSE))
        {
            pszSeparator = "";
        }
        else
        {
            pszSeparator = ",";
        }

        // <partnerLocalUSN>,<partner up-to-date vector>,<partner server GUID>:<partnerLocalUSN>,
        retVal = VmDirAllocateStringPrintf( &(syncDoneCtrlVal.bv_val), "%s,%s%s%s:%s,",
                                                partnerlocalUsnStr,
                                                pAttrUpToDateVector->vals[0].lberbv.bv_val,
                                                pszSeparator,
                                                pAttrInvocationId->vals[0].lberbv.bv_val,
                                                partnerlocalUsnStr);
        BAIL_ON_VMDIR_ERROR(retVal);
    }
    else
    {
        // <partnerLocalUSN>,<partner server GUID>:<partnerLocalUSN>,
        retVal = VmDirAllocateStringPrintf( &(syncDoneCtrlVal.bv_val), "%s,%s:%s,",
                                                partnerlocalUsnStr,
                                                pAttrInvocationId->vals[0].lberbv.bv_val,
                                                partnerlocalUsnStr);
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    VmDirSetACLMode();

    syncDoneCtrlVal.bv_len = VmDirStringLenA(syncDoneCtrlVal.bv_val);

    if ((retVal = VmDirReplUpdateCookies( pSchemaCtx, &(syncDoneCtrlVal), pReplAgr )) != LDAP_SUCCESS)
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: UpdateCookies failed. Error: %d", retVal );
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    if ((retVal = _VmDirPatchDSERoot(pSchemaCtx)) != LDAP_SUCCESS)
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "vdirReplicationThrFun: _VmDirPatchDSERoot failed. Error: %d", retVal );
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

#ifndef VDIR_PSC_VERSION
#define VDIR_PSC_VERSION "6.7.0"
#endif

static
int
_VmDirPatchDSERoot(
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

    VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "_VmDirPatchDSERoot: End" );
    return retVal;

error:
    retVal = LDAP_OPERATIONS_ERROR;
    goto cleanup;
}

static
int
_VmGetHighestCommittedUSN(
    USN startUsn,
    USN *hcUsn)
{
    DWORD             dwError = 0;
    USN               usn = 0;
    VDIR_ENTRY_ARRAY  entryArray = {0};
    PSTR              usnStr = NULL;


    for (usn=startUsn; usn > 1LL; usn--)
    {
        VMDIR_SAFE_FREE_MEMORY(usnStr);
        VmDirFreeEntryArrayContent(&entryArray);

        dwError = VmDirAllocateStringPrintf(&usnStr, "%llu", usn);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirSimpleEqualFilterInternalSearch(
                    "", LDAP_SCOPE_SUBTREE, ATTR_USN_CHANGED, usnStr, &entryArray);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (entryArray.iSize == 1 )
        {
            *hcUsn = usn;
            goto cleanup;
        }
    }
    dwError = LDAP_OPERATIONS_ERROR;
    goto error;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(usnStr);
    VmDirFreeEntryArrayContent(&entryArray);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "_VmGetHighestCommittedUSN: fail to find an entry with USN <= %llu", startUsn);
    goto cleanup;
}

static
DWORD
_VmDirMkdir(
    PCSTR path,
    int mode
    )
{
    DWORD   dwError = 0;
#ifdef _WIN32
    if(CreateDirectory(path, NULL)==0)
    {
        errno = WSAGetLastError();
        dwError = VMDIR_ERROR_IO;
        goto error;
    }
#else
    if(mkdir(path, mode)!=0)
    {
        dwError = VMDIR_ERROR_IO;
        goto error;
    }
#endif

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "_VmDirMkdir on dir %s failed (%u) errno: (%d)", path, dwError, errno);
    goto cleanup;
}
