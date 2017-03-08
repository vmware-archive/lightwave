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

//Read data.mdb up to last_pgno + the safe margin (256MB). The margin is for the latency
// between calling mdb_env_set_state to obtain the dbSizeMb and the meta page(first block) is read,
// i.e. the provision at the remote server wouldn't increase DB size by 256MB during this period.
// For cold copy (remote database at read-only mode), VMDIR_MDB_COPY_SAFE_MARGIN is set to 0.
#define VMDIR_MDB_COPY_SAFE_MARGIN (256 << 20)

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
VOID
_VmDirShutdownDB();

static
int
_VmDirSwapDB(
    PCSTR   dbHomeDir);

VOID
VmDirFreeBindingHandle(
    handle_t *ppBinding
    );

static
DWORD
_VmDirMkdir(
    PCSTR path,
    int mode);

int
VmDirFirstReplicationCycle(
    PSTR pszHostname
    )
{
    int retVal = LDAP_SUCCESS;
    PSTR  pszLocalErrorMsg = NULL;
    PVDIR_SCHEMA_CTX pSchemaCtx = NULL;
#ifndef _WIN32
    const char  *dbHomeDir = LWRAFT_DB_DIR;
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

    //Shutdown local database
    _VmDirShutdownDB();

    retVal = _VmDirGetRemoteDBUsingRPC(pszHostname, dbHomeDir);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
                "VmDirFirstReplicationCycle: _VmDirGetRemoteDBUsingRPC() call failed with error: %d", retVal );

    retVal = _VmDirSwapDB(dbHomeDir);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
                "VmDirFirstReplicationCycle: _VmDirSwapDB() call failed, error: %d.", retVal );

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Remote DB copied from %s, and swapped successfully", pszHostname);

cleanup:
    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }
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
    char        localXlogDir[VMDIR_MAX_FILE_NAME_LEN] = {0};
    char        localFilename[VMDIR_MAX_FILE_NAME_LEN] = {0};
    PSTR        pszDcAccountPwd = NULL;
    PVMDIR_SERVER_CONTEXT hServer = NULL;
    DWORD       low_xlognum = 0;
    DWORD       high_xlognum = 0;
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

    //Set backend to KEEPXLOGS  mode
    retVal = VmDirSetBackendState (hServer, MDB_STATE_KEEPXLOGS, &low_xlognum, &remoteDbSizeMb, &remoteDbMapSizeMb, pDbPath, VMDIR_MAX_FILE_NAME_LEN);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBUsingRPC: VmDirSetBackendState failed with error: %d", retVal  );

    retVal = VmDirStringPrintFA( localDir, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s", dbHomeDir, fileSeperator, LOCAL_PARTNER_DIR);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBUsingRPC: VmDirStringPrintFA() call failed with error: %d", retVal );

    retVal = _VmDirMkdir(localDir, 0700);
    BAIL_ON_VMDIR_ERROR( retVal );

    retVal = VmDirStringPrintFA( localXlogDir, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s", localDir, fileSeperator, VMDIR_MDB_XLOGS_DIR_NAME);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBUsingRPC: VmDirStringPrintFA() call failed with error: %d", retVal );

    retVal = _VmDirMkdir(localXlogDir, 0700);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBUsingRPC: _VmDirMkdir() call failed with error: %d %s", retVal );

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

    //Query current xlog number
    retVal = VmDirSetBackendState (hServer, MDB_STATE_GETXLOGNUM, &high_xlognum, &remoteDbSizeMb, &remoteDbMapSizeMb, pDbPath, VMDIR_MAX_FILE_NAME_LEN);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBUsingRPC: VmDirSetBackendState failed to get current xlog: %d", retVal  );

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "_VmDirGetRemoteDBUsingRPC: start transfering XLOGS from %d to %d", low_xlognum, high_xlognum);
    for (xlognum = low_xlognum; xlognum <= high_xlognum; xlognum++)
    {
        retVal = VmDirStringPrintFA( dbRemoteFilename, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s%c%lu", dbHomeDir, fileSeperator,
                                 VMDIR_MDB_XLOGS_DIR_NAME, fileSeperator, xlognum );
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBUsingRPC: VmDirStringPrintFA() call failed with error: %d", retVal );

        retVal = VmDirStringPrintFA( localFilename, VMDIR_MAX_FILE_NAME_LEN, "%s%c%lu", localXlogDir, fileSeperator, xlognum);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBUsingRPC: VmDirStringPrintFA() call failed with error: %d", retVal );

        retVal = _VmDirGetRemoteDBFileUsingRPC( hServer, dbRemoteFilename, localFilename, 0, 0);
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirGetRemoteDBUsingRPC: _VmDirGetRemoteDBFileUsingRPC() call failed with error: %d", retVal );

        //This is a workaround for the DCERPC blocking issue, see PR 1827212
        VmDirSleep(300);
    }

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "_VmDirGetRemoteDBUsingRPC: complete transfering XLOGS from %d to %d", low_xlognum, high_xlognum);

cleanup:
    if (hServer)
    {
        //clear backend transfering xlog files mode.
        VmDirSetBackendState (hServer, MDB_STATE_CLEAR, &xlognum, &remoteDbSizeMb, &remoteDbMapSizeMb, pDbPath, VMDIR_MAX_FILE_NAME_LEN);
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

            //We saw DCERPC calls timeout when transferring file larger than 100GB without this pause.
            // Should address this with PR 1827212
            VmDirSleep(2000);
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

/**
 * @ _VmDirShutdownDB()
 * shutdown the current backend
 * @return VOID
 */
static
VOID
_VmDirShutdownDB()
{
    PVDIR_BACKEND_INTERFACE pBE = NULL;

    // Shutdown backend
    pBE = VmDirBackendSelect(NULL);
    assert(pBE);

    VmDirdStateSet(VMDIRD_STATE_SHUTDOWN);

    VmDirSchemaLibShutdown();

    VmDirIndexLibShutdown();

    pBE->pfnBEShutdown();
    VmDirBackendContentFree(pBE);
}

static
int
_VmDirSwapDB(
    PCSTR dbHomeDir)
{
    int                     retVal = LDAP_SUCCESS;
    char                    dbExistingName[VMDIR_MAX_FILE_NAME_LEN] = {0};
    char                    dbNewName[VMDIR_MAX_FILE_NAME_LEN] = {0};
    PSTR                    pszLocalErrorMsg = NULL;
    int                     errorCode = 0;
    BOOLEAN                 bLegacyDataLoaded = FALSE;

#ifndef _WIN32
    const char   fileSeperator = '/';
#else
    const char   fileSeperator = '\\';
#endif

    // move .mdb files
    retVal = VmDirStringPrintFA( dbExistingName, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s%c%s", dbHomeDir, fileSeperator,
                                 LOCAL_PARTNER_DIR, fileSeperator, VMDIR_MDB_DATA_FILE_NAME);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirSwapDB: VmDirStringPrintFA() call failed with error: %d", retVal );

    retVal = VmDirStringPrintFA( dbNewName, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s", dbHomeDir, fileSeperator,
                                 VMDIR_MDB_DATA_FILE_NAME );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirSwapDB: VmDirStringPrintFA() call failed with error: %d", retVal );

#ifdef WIN32
    if (MoveFileEx(dbExistingName, dbNewName, MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING) == 0)
    {
        retVal = LDAP_OPERATIONS_ERROR;
        errorCode = GetLastError();
#else
    if (rename(dbExistingName, dbNewName) != 0)
    {
        retVal = LDAP_OPERATIONS_ERROR;
        errorCode = errno;
#endif
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirSwapDB: rename file from %s to %s failed, errno %d", dbExistingName, dbNewName, errorCode );
    }

    retVal = VmDirStringPrintFA(dbNewName, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s%c%s", dbHomeDir, fileSeperator, VMDIR_MDB_XLOGS_DIR_NAME);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirSwapDB: VmDirStringPrintFA() call failed with error: %d", retVal );

    retVal = VmDirStringPrintFA(dbExistingName, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s%c%s", dbHomeDir, fileSeperator,
                                LOCAL_PARTNER_DIR, fileSeperator, VMDIR_MDB_XLOGS_DIR_NAME);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirSwapDB: VmDirStringPrintFA() call failed with error: %d", retVal );

#ifdef WIN32
    if (MoveFileEx(dbExistingName, dbNewName, MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING) == 0)
    {
        retVal = LDAP_OPERATIONS_ERROR;
        errorCode = GetLastError();
#else
    if (rmdir(dbNewName) != 0)
    {
        retVal = LDAP_OPERATIONS_ERROR;
        errorCode = errno;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(retVal, (pszLocalErrorMsg), "_VmDirSwapDB cannot remove directory %s, errno %d",
                                     dbNewName, errorCode);
    }

    if (rename(dbExistingName, dbNewName) != 0)
    {
        retVal = LDAP_OPERATIONS_ERROR;
        errorCode = errno;
#endif
        BAIL_ON_VMDIR_ERROR_WITH_MSG(retVal, (pszLocalErrorMsg), "_VmDirSwapDB cannot move directory from %s to %s, errno %d",
                                     dbNewName, dbExistingName, errorCode);
    }

    retVal = VmDirStringPrintFA(dbExistingName, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s", dbHomeDir, fileSeperator, LOCAL_PARTNER_DIR);
    BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg),
            "_VmDirSwapDB: VmDirStringPrintFA() call failed with error: %d", retVal );

#ifdef WIN32
    if (RemoveDirectory(dbExistingName)==0)
    {
        errorCode = GetLastError();
#else
    if (rmdir(dbExistingName))
    {
        errorCode = errno;
#endif

        VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL, "cannot remove directory %s errno %d", dbExistingName, errorCode);
    }



    VmDirdStateSet(VMDIRD_STATE_STARTUP);

    retVal = VmDirInitBackend(&bLegacyDataLoaded);
    BAIL_ON_VMDIR_ERROR(retVal);

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
