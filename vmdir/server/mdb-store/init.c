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

#define DB_BUFSIZE (1<<24)
static
DWORD
MDBOpenMainDB(
    VOID
    );

static
DWORD
MDBInitSequence(
    VDIR_DB     mdbDbi
    );

static
DWORD
MDBOpenSequence(
    VOID
    );

static
DWORD
MDBOpenGeneric(
    VOID
    );

static
void
MDBCloseDBs(
    VOID
    );

static
VOID
MDBFreeMdbGlobals(
    VOID
    );

static
DWORD
_VmDirDbCopyThread(
    PVOID pArg
    );

static
DWORD
_VmDirCpMdbFile(
    VOID
    );

static
DWORD
_VmDirOpenDbEnv(
    VOID
    );

static
DWORD
_VmdirCreateDbEnv(
    uint64_t db_max_mapsize
    );

DWORD
_VmDirDbCpReadRegistry(
    PDWORD pdwCopyDbWritesMin,
    PDWORD pdwCopyDbIntervalInSec,
    PDWORD pdwCopyDbBlockWriteInSec
    );

PVDIR_BACKEND_INTERFACE
VmDirMDBBEInterface (
    VOID)
{
    static VDIR_BACKEND_INTERFACE mdbBEInterface =
    {
        // NOTE: order of fields MUST stay in sync with struct definition...
        VMDIR_SF_INIT(.pfnBEInit, VmDirMDBInitializeDB),
        VMDIR_SF_INIT(.pfnBEShutdown, VmDirMDBShutdownDB),
        VMDIR_SF_INIT(.pfnBEIndexOpen, VmDirMDBIndexOpen),
        VMDIR_SF_INIT(.pfnBEIndexExist, VmDirMDBIndexExist),
        VMDIR_SF_INIT(.pfnBEIndexDelete, VmDirMDBIndexDelete),
        VMDIR_SF_INIT(.pfnBEIndexPopulate, VmDirMDBIndicesPopulate),
        VMDIR_SF_INIT(.pfnBEIndexIteratorInit, VmDirMDBIndexIteratorInit),
        VMDIR_SF_INIT(.pfnBEIndexIterate, VmDirMDBIndexIterate),
        VMDIR_SF_INIT(.pfnBEIndexIteratorFree, VmDirMDBIndexIteratorFree),
        VMDIR_SF_INIT(.pfnBEParentIdIndexIteratorInit, VmDirMDBParentIdIndexIteratorInit),
        VMDIR_SF_INIT(.pfnBEParentIdIndexIterate, VmDirMDBParentIdIndexIterate),
        VMDIR_SF_INIT(.pfnBEParentIdIndexIteratorFree, VmDirMDBParentIdIndexIteratorFree),
        VMDIR_SF_INIT(.pfnBEEntryBlobIteratorInit, VmDirMDBEntryBlobIteratorInit),
        VMDIR_SF_INIT(.pfnBEEntryBlobIterate, VmDirMDBEntryBlobIterate),
        VMDIR_SF_INIT(.pfnBEEntryBlobIteratorFree, VmDirMDBEntryBlobIteratorFree),
        VMDIR_SF_INIT(.pfnBETxnBegin, VmDirMDBTxnBegin),
        VMDIR_SF_INIT(.pfnBETxnAbort, VmDirMDBTxnAbort),
        VMDIR_SF_INIT(.pfnBETxnCommit, VmDirMDBTxnCommit),
        VMDIR_SF_INIT(.pfnBESimpleIdToEntry, VmDirMDBSimpleEIdToEntry),
        VMDIR_SF_INIT(.pfnBESimpleDnToEntry, VmDirMDBSimpleDnToEntry),
        VMDIR_SF_INIT(.pfnBEIdToEntry, VmDirMDBEIdToEntry),
        VMDIR_SF_INIT(.pfnBEDNToEntry, VmDirMDBDNToEntry),
        VMDIR_SF_INIT(.pfnBEDNToEntryId, VmDirMDBDNToEntryId),
        VMDIR_SF_INIT(.pfnBEObjectGUIDToEntryId, VmDirMDBObjectGUIDToEntryId),
        VMDIR_SF_INIT(.pfnBEChkDNReference, VmDirMDBCheckRefIntegrity),
        VMDIR_SF_INIT(.pfnBEChkIsLeafEntry, VmDirMDBCheckIfALeafNode),
        VMDIR_SF_INIT(.pfnBEGetCandidates, VmDirMDBGetCandidates),
        VMDIR_SF_INIT(.pfnBEEntryAdd, VmDirMDBAddEntry),
        VMDIR_SF_INIT(.pfnBEEntryDelete, VmDirMDBDeleteEntry),
        VMDIR_SF_INIT(.pfnBEEntryModify, VmDirMDBModifyEntry),
        VMDIR_SF_INIT(.pfnBEMaxEntryId, VmDirMDBMaxEntryId),
        VMDIR_SF_INIT(.pfnBEGetAttrMetaData, VmDirMDBGetAttrMetaData),
        VMDIR_SF_INIT(.pfnBEGetAllAttrsMetaData, VmDirMDBGetAllAttrsMetaData),
        VMDIR_SF_INIT(.pfnBEGetNextUSN, VmDirMDBGetNextUSN),
        VMDIR_SF_INIT(.pfnBEDupKeyGetValues, VmDirMDBDupKeyGetValues),
        VMDIR_SF_INIT(.pfnBEDupKeySetValues, VmDirMDBDupKeySetValues),
        VMDIR_SF_INIT(.pfnBEUniqKeyGetValue, VmDirMDBUniqKeyGetValue),
        VMDIR_SF_INIT(.pfnBEUniqKeySetValue, VmDirMDBUniqKeySetValue),
        VMDIR_SF_INIT(.pfnBEConfigureFsync, VmDirMDBConfigureFsync),
        VMDIR_SF_INIT(.pfnBEGetAttrValueMetaData, VmDirMDBGetAttrValueMetaData),
        VMDIR_SF_INIT(.pfnBEGetAllAttrValueMetaData, VmDirMDBGetAllAttrValueMetaData),
        VMDIR_SF_INIT(.pfnBEUpdateAttrValueMetaData, VmDirMdbUpdateAttrValueMetaData),
        VMDIR_SF_INIT(.pfnBEDeleteAllAttrValueMetaData, VmDirMdbDeleteAllAttrValueMetaData),
        VMDIR_SF_INIT(.pfnBEApplyIndicesNewMR, VmDirMdbApplyIndicesNewMR)
    };

    return &mdbBEInterface;
}

/*
 last_pgno and max_pgs are logged. If last_pgno + pages for adding
 new data > max_pgs, mdb_put will fail with error MDB_MAP_FULL.
 Mdb first tries to reuse released pages before trying to get
 new pages from the free list. Thus even if an operation request
 new pages failed (last_pgno + pages > max_pgs),
 adding smaller data may still succeeded if the there are
 enough pages in the released pages. Max memory can be
 calculated from max_pgs * page size which is the same as the OS
 page size.
*/
void VmDirLogDBStats()
{
    MDB_envinfo env_stats = {0};
    MDB_stat db_stats = {0};

    if (mdb_env_info(gVdirMdbGlobals.mdbEnv, &env_stats) != MDB_SUCCESS ||
        mdb_env_stat(gVdirMdbGlobals.mdbEnv, &db_stats)!= MDB_SUCCESS)
    {
        goto error;
    }
    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "mdb stats: last_pgno %llu, max_pgs %lld",
                   env_stats.me_last_pgno, env_stats.me_mapsize/db_stats.ms_psize);

cleanup:
    return;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "Error retrieving MDB statistics");
    goto cleanup;
}

/*
 * Initialize MDB db
 * (reference openldap 2.4.31 back-mdb/init.c)
 */
DWORD
VmDirMDBInitializeDB(
    VOID)
{
    DWORD           dwError = 0;

    dwError = (sizeof(ENTRYID) == sizeof(VDIR_DB_SEQ_T)) ? 0 : ERROR_BACKEND_ERROR;
    BAIL_ON_VMDIR_ERROR( dwError );

    dwError = MDBInitConfig();
    BAIL_ON_VMDIR_ERROR( dwError );

    dwError = _VmDirOpenDbEnv();
    BAIL_ON_VMDIR_ERROR( dwError );

    /* Open main database. */
    dwError = MDBOpenMainDB();
    BAIL_ON_VMDIR_ERROR( dwError );

    /* Open sequences */
    dwError = MDBOpenSequence();
    BAIL_ON_VMDIR_ERROR( dwError );

    /* Open generic */
    dwError = MDBOpenGeneric();
    BAIL_ON_VMDIR_ERROR( dwError );

    /* Initialize indices */
    dwError = VmDirMDBInitializeIndexDB();
    BAIL_ON_VMDIR_ERROR( dwError );

    VmDirLogDBStats();

cleanup:
    VmDirLog( LDAP_DEBUG_TRACE, "MDBInitializeDB: End" );

    return dwError;

error:

    VmDirLog( LDAP_DEBUG_ANY, "MDBInitializeDB failed with error code: %d, error string: %s", dwError, mdb_strerror(dwError) );

//TODO, should shutdown here or caller will do that?
//gVdirMdbGlobals.mdbEnv = NULL;

    goto cleanup;
}

/*
 * Close all opened DBs and free environment
 * Free gVdirMdbGlobals.*
 */
DWORD
VmDirMDBShutdownDB(
    VOID)
{
    VmDirLog( LDAP_DEBUG_TRACE, "MDBShutdownDB: Begin" );

    VmDirLogDBStats();

    if (gVdirMdbGlobals.mdbEnv != NULL)
    {
        MDBCloseDBs();

        VmDirMDBShutdownIndexDB();

        // force buffer sync
        mdb_env_sync(gVdirMdbGlobals.mdbEnv, 1);

        mdb_env_close(gVdirMdbGlobals.mdbEnv);
        gVdirMdbGlobals.mdbEnv = NULL;
    }

    MDBFreeMdbGlobals();

    VmDirLog( LDAP_DEBUG_TRACE, "MDBShutdownDB: End" );

    return 0;
}

DWORD
MDBOpenDB(
    PVDIR_DB            pmdbDBi,
    const char *        dbName,
    const char *        fileName,
    PFN_BT_KEY_CMP      btKeyCmpFcn,
    unsigned int        extraFlags)
{
    DWORD               dwError = 0;
    MDB_txn*            pTxn = NULL;
    VDIR_DB             mdbDBi  = 0;

    VmDirLog( LDAP_DEBUG_TRACE, "MdbOpenDB: Begin, DN name = %s", fileName );

    assert(pmdbDBi);

    extraFlags |= MDB_CREATE;

    dwError = mdb_txn_begin( gVdirMdbGlobals.mdbEnv, NULL, BE_DB_FLAGS_ZERO, &pTxn );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = mdb_open( pTxn, dbName, extraFlags, &mdbDBi);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (NULL != btKeyCmpFcn)
    {
        // set customize "key" compare function.
        dwError = mdb_set_compare( pTxn, mdbDBi, btKeyCmpFcn);
        BAIL_ON_VMDIR_ERROR(dwError);

        // if db is opened with MDB_DUPSORT flag, you can set customize "data" compare function.
        // we use default lexical comparison.
        //dwError = mdb_set_dupsort( pTxn, mdbDBi, btKeyCmpFcn);
        //BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = mdb_txn_commit(pTxn);
    // regardless of commit result, pTxn should not be accessed anymore
    // see mdb-back/init.c mdb_db_open example.
    // this is consistent with BDB DB_TXN->commit() man page.
    pTxn = NULL;
    BAIL_ON_VMDIR_ERROR(dwError);

    *pmdbDBi = mdbDBi;

cleanup:

    VmDirLog( LDAP_DEBUG_TRACE, "MdbOpenDB: End" );
    return dwError;

error:

    if (pTxn)
    {
        mdb_txn_abort(pTxn);
        pTxn = NULL;
    }

    VmDirLog( LDAP_DEBUG_ANY, "MdbOpenDB failed with error code: %d, error string: %s", dwError, mdb_strerror(dwError) );

    goto cleanup;
}

VOID
MDBCloseDB(
    VDIR_DB    mdbDBi
    )
{
    mdb_close(gVdirMdbGlobals.mdbEnv, mdbDBi);
}

DWORD
MDBDropDB(
    VDIR_DB    mdbDBi
    )
{
    DWORD               dwError = 0;
    MDB_txn*            pTxn = NULL;

    dwError = mdb_txn_begin(gVdirMdbGlobals.mdbEnv, NULL, BE_DB_FLAGS_ZERO, &pTxn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = mdb_drop(pTxn, mdbDBi, 1);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = mdb_txn_commit(pTxn);
    pTxn = NULL;
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    if (pTxn)
    {
        mdb_txn_abort(pTxn);
        pTxn = NULL;
    }
    goto cleanup;
}

/*
 * Error map from MDB to BE space
 * If no map specified, ERROR_BACKEND_ERROR is returned.
 *
 * BECtx.dwBEErrorCode is set to the first mdb error encountered
 * BECtx.pszBEErrorMsg is set to the first mdb error text encountered
 *
 * NOTE, this could be called multiple times during one LDAP level operation.
 * The last one counts.
 */
DWORD
MDBToBackendError(
    DWORD               dwMdbError,
    DWORD               dwFromMdbError,
    DWORD               dwToBEError,
    PVDIR_BACKEND_CTX   pBECtx,
    PCSTR               pszErrorContext)
{
    DWORD   dwError = 0;

    assert(pBECtx);

    if (dwMdbError != 0)
    {
        pBECtx->dwBEErrorCode = dwMdbError;
        VMDIR_SAFE_FREE_MEMORY(pBECtx->pszBEErrorMsg);
        // ignore error
        VmDirAllocateStringPrintf(    &pBECtx->pszBEErrorMsg,
                                          "(%s)(%s)",
                                          mdb_strerror(dwMdbError),
                                          VDIR_SAFE_STRING(pszErrorContext));

        if (dwMdbError == dwFromMdbError)
        {
            dwError = dwToBEError;
        }
        else
        {
            dwError = ERROR_BACKEND_ERROR;
        }
    }

    return dwError;
}

/*
 * Open Entry/Blob Database.
 *
 * Called during server startup, so it is safe to access gVdirMdbGlobals
 * w/o protection.
 */
static
DWORD
MDBOpenMainDB(
    VOID
    )
{
    DWORD           dwError = 0;
    unsigned int    iDbFlags = 0;

    VmDirLog( LDAP_DEBUG_TRACE, "MdbOpenDBs: Begin" );

    // default database has unique key. i.e. no DUP key allowed.
    iDbFlags |= MDB_CREATE;
    //    iDbFlags |= MDB_INTEGERKEY; our keys do not have same size

    dwError = MDBOpenDB(&gVdirMdbGlobals.mdbEntryDB.pMdbDataFiles[0].mdbDBi,
                         gVdirMdbGlobals.mdbEntryDB.pMdbDataFiles[0].pszDBName,
                         gVdirMdbGlobals.mdbEntryDB.pMdbDataFiles[0].pszDBFile,
                         gVdirMdbGlobals.mdbEntryDB.btKeyCmpFcn,
                        iDbFlags);
    BAIL_ON_VMDIR_ERROR( dwError );

cleanup:

    VmDirLog( LDAP_DEBUG_TRACE, "MdbOpenDBs: End" );
    return dwError;

error:

    goto cleanup;
}

/*
 * Initialize ENTRYID and USN sequence.
 */
static
DWORD
MDBInitSequence(
    VDIR_DB     mdbDbi
    )
{
    DWORD           dwError = 0;
    PVDIR_DB_TXN    pTxn = NULL;
    VDIR_DB_DBT     key = {0};
    VDIR_DB_DBT     value = {0};
    unsigned char   EidBytes[sizeof( ENTRYID )] = {0};
    ENTRYID         initEIDValue = ENTRY_ID_SEQ_INITIAL_VALUE;
    ENTRYID         initUNSValue = USN_SEQ_INITIAL_VALUE;

    dwError = mdb_txn_begin( gVdirMdbGlobals.mdbEnv, NULL, BE_DB_FLAGS_ZERO, &pTxn );
    BAIL_ON_VMDIR_ERROR(dwError);

    key.mv_data = &EidBytes[0];
    MDBEntryIdToDBT(BE_MDB_ENTRYID_SEQ_KEY, &key);

    dwError =  mdb_get(pTxn, mdbDbi, &key, &value);
    if (dwError == MDB_NOTFOUND)
    {
        // first time, initialize two sequence records
        value.mv_data = &initEIDValue;
        value.mv_size = sizeof(initEIDValue);

        // set entryid sequence record
        dwError = mdb_put(pTxn, mdbDbi, &key, &value,  MDB_NOOVERWRITE);
        BAIL_ON_VMDIR_ERROR(dwError);

        MDBEntryIdToDBT(BE_MDB_USN_SEQ_KEY, &key);
        value.mv_data = &initUNSValue;
        value.mv_size = sizeof(initUNSValue);

        // set usn sequence record
        dwError = mdb_put(pTxn, mdbDbi, &key, &value, MDB_NOOVERWRITE);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = mdb_txn_commit(pTxn);
    pTxn = NULL;
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    return dwError;

error:

    if (pTxn)
    {
        mdb_txn_abort(pTxn);
    }

    goto cleanup;
}
/*
 * MDB has not SEQUENCE support.
 * Use a separate database and have one record representing one logic sequence.
 */
static
DWORD
MDBOpenSequence(
    VOID
    )
{
    DWORD           dwError = 0;
    unsigned int    iDbFlags = 0;
    VDIR_DB         mdbDBi = 0;

    VmDirLog( LDAP_DEBUG_TRACE, "MdbOpenSequence: Begin" );

    // default database has unique key. i.e. no DUP key allowed.
    iDbFlags |= MDB_CREATE;
    //    iDbFlags |= MDB_INTEGERKEY; our keys do not have same size

    dwError = MDBOpenDB( &mdbDBi,
                        BE_MDB_SEQ_DB_NAME,
                        gVdirMdbGlobals.mdbEntryDB.pMdbDataFiles[0].pszDBFile, // use same file as Entry DB
                        NULL,
                        iDbFlags);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = MDBInitSequence(mdbDBi);
    BAIL_ON_VMDIR_ERROR(dwError);

    gVdirMdbGlobals.mdbSeqDBi = mdbDBi;

cleanup:

    VmDirLog( LDAP_DEBUG_TRACE, "MdbOpenSequence: End" );

    return dwError;

error:

    goto cleanup;
}

static
DWORD
MDBOpenGeneric(
    VOID
    )
{
    DWORD           dwError = 0;
    unsigned int    iDbFlags = 0;
    VDIR_DB         mdbDBi = 0;

    iDbFlags |= MDB_CREATE;

    dwError = MDBOpenDB(
            &mdbDBi,
            BE_MDB_GENERIC_UNIQKEY_DB_NAME,
            gVdirMdbGlobals.mdbEntryDB.pMdbDataFiles[0].pszDBFile, // use same file as Entry DB
            NULL,
            iDbFlags);
    BAIL_ON_VMDIR_ERROR(dwError);

    gVdirMdbGlobals.mdbGenericUniqKeyDBi = mdbDBi;

    iDbFlags |= MDB_DUPSORT; // allow dup keys

    dwError = MDBOpenDB(
            &mdbDBi,
            BE_MDB_GENERIC_DUPKEY_DB_NAME,
            gVdirMdbGlobals.mdbEntryDB.pMdbDataFiles[0].pszDBFile, // use same file as Entry DB
            NULL,
            iDbFlags);
    BAIL_ON_VMDIR_ERROR(dwError);

    gVdirMdbGlobals.mdbGenericDupKeyDBi = mdbDBi;

cleanup:

    VmDirLog( LDAP_DEBUG_TRACE, "MDBOpenGeneric: End" );

    return dwError;

error:

    goto cleanup;
}

/*
 * Close all opened database.
 *
 * Called during server shutdown, so it is safe to access gVdirMdbGlobals
 * w/o protection.
 */
static
void
MDBCloseDBs()
{
    VmDirLog( LDAP_DEBUG_TRACE, "MdbCloseDBs: Begin" );

    if (gVdirMdbGlobals.mdbEntryDB.pMdbDataFiles)
    {
        // close entry db
        mdb_close(gVdirMdbGlobals.mdbEnv, gVdirMdbGlobals.mdbEntryDB.pMdbDataFiles[0].mdbDBi);
    }

    // close sequence db
    mdb_close(gVdirMdbGlobals.mdbEnv, gVdirMdbGlobals.mdbSeqDBi);

    // close generic dbs
    mdb_close(gVdirMdbGlobals.mdbEnv, gVdirMdbGlobals.mdbGenericDupKeyDBi);
    mdb_close(gVdirMdbGlobals.mdbEnv, gVdirMdbGlobals.mdbGenericUniqKeyDBi);

    VmDirLog( LDAP_DEBUG_TRACE, "MdbCloseDBs: End" );
}

static
VOID
MDBFreeMdbGlobals(
    VOID
    )
{
    // Free gVdirMdbGlobals
    if (gVdirMdbGlobals.mdbEntryDB.pMdbDataFiles)
    {
        VMDIR_SAFE_FREE_MEMORY(gVdirMdbGlobals.mdbEntryDB.pMdbDataFiles->pszDBFile);
        VMDIR_SAFE_FREE_MEMORY(gVdirMdbGlobals.mdbEntryDB.pMdbDataFiles->pszDBName);
        VMDIR_SAFE_FREE_MEMORY(gVdirMdbGlobals.mdbEntryDB.pMdbDataFiles);
    }
}

/*
 * See file client.c VmDirSetBackendState on parameters
 */
DWORD
VmDirSetMdbBackendState(
    DWORD               dwFileTransferState,
    DWORD               *pdwLogNum,
    DWORD               *pdwDbSizeMb,
    DWORD               *pdwDbMapSizeMb,
    PSTR                pszDbPath,
    DWORD               dwDbPathSize)
{
    DWORD dwError = 0;
    unsigned long lognum = 0L;
    unsigned long  dbSizeMb = 0L;
    unsigned long  dbMapSizeMb = 0L;

    if (dwFileTransferState < 0 || dwFileTransferState > 2)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (dwFileTransferState == 0)
    {
        VmDirdStateSet(VMDIRD_STATE_NORMAL);
    }
    else
    {
        VmDirdStateSet(VMDIRD_STATE_READ_ONLY);
    }

    *pdwLogNum = 0;
    *pdwDbSizeMb = 0;
    *pdwDbMapSizeMb = 0;
    dwError = mdb_env_set_state(gVdirMdbGlobals.mdbEnv, dwFileTransferState, &lognum, &dbSizeMb, &dbMapSizeMb, pszDbPath, dwDbPathSize);
    BAIL_ON_VMDIR_ERROR(dwError);
    *pdwLogNum = lognum;
    *pdwDbSizeMb = dbSizeMb;
    *pdwDbMapSizeMb = dbMapSizeMb;

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirInitDbCopyThread(
    void
    )
{
    DWORD dwError = 0;
    PVDIR_THREAD_INFO pThrInfo = NULL;

#ifdef WIN32
    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "VmDirInitDbCopyThread: database snapshot is not implemented for Windows.");
    goto cleanup;
#endif

    dwError = VmDirSrvThrInit(&pThrInfo, NULL, NULL, FALSE);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateThread(
                &pThrInfo->tid,
                pThrInfo->bJoinThr,
                _VmDirDbCopyThread,
                NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "VmDirInitDbCopyThread: database snapshot reg keys: CopyDbWritesMin %d CopyDbIntervalInSec %d CopyDbBlockWriteInSec %d",
                   gVmdirGlobals.dwCopyDbWritesMin, gVmdirGlobals.dwCopyDbIntervalInSec, gVmdirGlobals.dwCopyDbBlockWriteInSec);
cleanup:
    return dwError;

error:
    VmDirSrvThrFree(pThrInfo);
    goto cleanup;
}

static
DWORD
_VmDirDbCopyThread(
    PVOID pArg
    )
{
    time_t prev_dbcp_time = 0, cur_dbcp_time = 0;
    DWORD dwDbcpElapsedTime = 0;
    DWORD dwPrevCopyDbIntervalInSec = 0;
    DWORD dwPreCopyDbWritesMin = 0;
    DWORD dwPreCopyDbBlockWriteInSec = 0;
    DWORD dwTimeRemain = 0;
    BOOLEAN bHasCpOnce = FALSE;

    time(&cur_dbcp_time);
    prev_dbcp_time = cur_dbcp_time;
    dwDbcpElapsedTime = (DWORD)(cur_dbcp_time - prev_dbcp_time);
    while(VmDirdState() != VMDIRD_STATE_SHUTDOWN)
    {
        dwPrevCopyDbIntervalInSec = gVmdirGlobals.dwCopyDbIntervalInSec;
        dwPreCopyDbWritesMin = gVmdirGlobals.dwCopyDbWritesMin;
        dwPreCopyDbBlockWriteInSec = gVmdirGlobals.dwCopyDbBlockWriteInSec;

        _VmDirDbCpReadRegistry(&gVmdirGlobals.dwCopyDbWritesMin,
           &gVmdirGlobals.dwCopyDbIntervalInSec,
           &gVmdirGlobals.dwCopyDbBlockWriteInSec);

        if (dwPrevCopyDbIntervalInSec != gVmdirGlobals.dwCopyDbIntervalInSec)
        {
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirDbCopyThread: reg key %s changed from %d to %d",
              "CopyDbIntervalInSec", dwPrevCopyDbIntervalInSec, gVmdirGlobals.dwCopyDbIntervalInSec);
        }

        if (dwPreCopyDbWritesMin != gVmdirGlobals.dwCopyDbWritesMin)
        {
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirDbCopyThread: reg key %s changed from %d to %d",
              "CopyDbWritesMin", dwPreCopyDbWritesMin, gVmdirGlobals.dwCopyDbWritesMin);
        }

        if (dwPreCopyDbBlockWriteInSec != gVmdirGlobals.dwCopyDbBlockWriteInSec)
        {
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirDbCopyThread: reg key %s changed from %d to %d",
              "CopyDbBlockWriteInSec", dwPreCopyDbBlockWriteInSec, gVmdirGlobals.dwCopyDbBlockWriteInSec);
        }

        if (gVmdirGlobals.dwCopyDbIntervalInSec == 0)
        {
            VmDirSleep(60000);
            continue;
        }

        if (bHasCpOnce == FALSE ||
            (gVmdirGlobals.dwLdapWrites >= gVmdirGlobals.dwCopyDbWritesMin &&
             dwDbcpElapsedTime >= gVmdirGlobals.dwCopyDbIntervalInSec
            )
           )
        {
            prev_dbcp_time = cur_dbcp_time;
            _VmDirCpMdbFile();
            bHasCpOnce = TRUE; //Copy db when vmdird starts regardless LdapWrites and DbcpElapsedTime
        }

        time(&cur_dbcp_time);
        dwDbcpElapsedTime = (DWORD)(cur_dbcp_time - prev_dbcp_time);
        dwTimeRemain = gVmdirGlobals.dwCopyDbIntervalInSec - dwDbcpElapsedTime;
        if (dwTimeRemain > 0 )
        {
            if (dwTimeRemain < 60)
            {
                VmDirSleep(dwTimeRemain * 1000);
            } else
            {
                //Check reg key change every minute.
                VmDirSleep(60000);
            }
        } else
        {
            //Pause at least ten seconds
            VmDirSleep(10000);
        }
    }
    return 0;
}

static
DWORD
_VmDirCpMdbFile(
    VOID
)
{
#ifndef WIN32
    DWORD dwError = 0;
    static long copyDbKbPerSec = 50000;
    PSTR pszLocalErrorMsg = NULL;
    char dbFilename[VMDIR_MAX_FILE_NAME_LEN] = {0};
    char dbHomeDir[VMDIR_MAX_FILE_NAME_LEN] = {0};
    char dbSnapshotDir[VMDIR_MAX_FILE_NAME_LEN] = {0};
    char dbStagingDir[VMDIR_MAX_FILE_NAME_LEN] = {0};
    char dbStagingFilename[VMDIR_MAX_FILE_NAME_LEN] = {0};
    unsigned long xlognum = 0;
    unsigned long dbSizeMb = 0;
    time_t start_ts = 0, end_ts = 0;
    unsigned long dbMapSizeMb = 0;
    int duration = 0;
    int fd_in = -1, fd_out = -1;
    int estimate_time_sec = 0;
    VDIR_BACKEND_CTX beCtx = {0};
    BOOLEAN bHasTxn = FALSE, mdb_in_readonly = FALSE;
    const char   fileSeperator = '/';
    static char dbBuf[DB_BUFSIZE] = {0};
    int numRead = 0;
    unsigned long long snapshotcopy_lasttid = 0;

    time(&start_ts);
    //Obtain the database size to be copied only
    dwError = mdb_env_set_state(gVdirMdbGlobals.mdbEnv, 3, &xlognum,
                                &dbSizeMb, &dbMapSizeMb, dbHomeDir, VMDIR_MAX_FILE_NAME_LEN);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg),
            "_VmDirCpMdbFile: mdb_env_set_state call to get database size failed with error: %d", dwError);

    dwError = VmDirStringPrintFA(dbSnapshotDir, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s", dbHomeDir, fileSeperator, "snapshot");
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg),
            "_VmDirCpMdbFile: VmDirStringPrintFA() call failed with error: %s %d", dbHomeDir, dwError);

    dwError = access(dbSnapshotDir, R_OK|W_OK|X_OK);
    if(dwError != 0 && mkdir(dbSnapshotDir, 0700)!=0)
    {
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg),
            "_VmDirCpMdbFile: failed to access or create snapshot directory %s errno %d", dbSnapshotDir, errno);
    }

    dwError = VmDirStringPrintFA(dbStagingDir, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s", dbHomeDir, fileSeperator, "staging");
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg),
            "_VmDirCpMdbFile: VmDirStringPrintFA() call failed with error: %s %d", dbHomeDir, dwError);

    dwError = access(dbStagingDir, R_OK|W_OK|X_OK);
    if(dwError != 0 && mkdir(dbStagingDir, 0700)!=0)
    {
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg),
            "_VmDirCpMdbFile: failed to access or create staging directory %s errno %d", dbStagingDir, errno);
    }

    if (copyDbKbPerSec == 0)
    {
        copyDbKbPerSec = 1;
    }
    estimate_time_sec = dbSizeMb * 1000 / copyDbKbPerSec;

    if(estimate_time_sec > gVmdirGlobals.dwCopyDbBlockWriteInSec)
    {
        dwError = mdb_env_set_state(gVdirMdbGlobals.mdbEnv, 1, &xlognum,
                                &dbSizeMb, &dbMapSizeMb, dbHomeDir, VMDIR_MAX_FILE_NAME_LEN);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg),
            "_VmDirCpMdbFile: mdb_env_set_state call failed with MDB error: %d", dwError);
        mdb_in_readonly = TRUE;
    } else
    {
       beCtx.pBE = VmDirBackendSelect(PERSISTED_DSE_ROOT_DN);
       dwError = beCtx.pBE->pfnBETxnBegin(&beCtx, VDIR_BACKEND_TXN_WRITE);
       BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg),
            "_VmDirCpMdbFile: pfnBETxnBegin with error: %d", dwError);
       bHasTxn = TRUE;
    }

    snapshotcopy_lasttid = mdb_env_get_lasttid(gVdirMdbGlobals.mdbEnv);

    dwError = VmDirStringPrintFA(dbFilename, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s", dbHomeDir, fileSeperator, "data.mdb");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA(dbStagingFilename, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s", dbStagingDir, fileSeperator, "data.mdb");
    BAIL_ON_VMDIR_ERROR(dwError);

    if ((fd_out = creat(dbStagingFilename, S_IRUSR|S_IWUSR|O_TRUNC)) < 0)
    {
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg),
            "_VmDirCpMdbFile: creat file %s failed, errno %d", dbStagingFilename, errno);
    }

    if((fd_in = open(dbFilename, O_RDONLY )) < 0)
    {
       dwError = LDAP_OPERATIONS_ERROR;
       BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg),
            "_VmDirCpMdbFile: open file %s for read failed, errno %d", dbFilename, errno);
    }

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirCpMdbFile: making database snapshot with file size %dMb; will take approximate %d seconds; %d updates occurred since last snapshot.",
                   dbSizeMb, estimate_time_sec, gVmdirGlobals.dwLdapWrites);

    while ((numRead = read(fd_in, dbBuf, DB_BUFSIZE)) > 0)
    {
      if (VmDirdState() == VMDIRD_STATE_SHUTDOWN)
      {
         goto cleanup;
      }
      if (write(fd_out, dbBuf, numRead) != numRead)
      {
          dwError = LDAP_OPERATIONS_ERROR;
          BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg),
             "_VmDirCpMdbFile: write to file %s failed, errno %d", dbStagingFilename, errno);
      }
    }

    if (numRead < 0)
    {
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg),
             "_VmDirCpMdbFile: read from file %s failed, errno %d", dbFilename, errno);
    }

    close(fd_in);
    fd_in = -1;
    close(fd_out);
    fd_out = -1;

    if (mdb_in_readonly)
    {
        mdb_env_set_state(gVdirMdbGlobals.mdbEnv, 0, &xlognum,
                                &dbSizeMb, &dbMapSizeMb, dbHomeDir, VMDIR_MAX_FILE_NAME_LEN);
        mdb_in_readonly = FALSE;
    }

    if (bHasTxn)
    {
        beCtx.pBE->pfnBETxnAbort(&beCtx);
        bHasTxn = FALSE;
    }

    gVmdirGlobals.dwLdapWrites = 0;

    dwError = VmDirStringPrintFA(dbFilename, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s", dbSnapshotDir, fileSeperator, "data.mdb");
    BAIL_ON_VMDIR_ERROR(dwError);

    if (rename(dbStagingFilename, dbFilename) != 0)
    {
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
             "_VmDirCpMdbFile: rename file from %s to %s failed, errno %d", dbStagingFilename, dbFilename, errno );
    }

    time(&end_ts);
    duration = end_ts - start_ts;

    if (duration == 0)
    {
        duration = 1;
    }
    copyDbKbPerSec = dbSizeMb * 1000 / duration;

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirCpMdbFile: completed making snapshot with file size "
                   "%dMb in %d seconds; data transfer rate: %d.%dMB/sec",
                   dbSizeMb, duration, copyDbKbPerSec/1000, copyDbKbPerSec%1000/100, snapshotcopy_lasttid);

cleanup:
    if (mdb_in_readonly)
    {
        mdb_env_set_state(gVdirMdbGlobals.mdbEnv, 0, &xlognum,
                                &dbSizeMb, &dbMapSizeMb, dbHomeDir, VMDIR_MAX_FILE_NAME_LEN);
    }
    if (fd_in >= 0)
    {
        close(fd_in);
    }
    if (fd_out >= 0)
    {
        close(fd_out);
    }
    if (bHasTxn)
    {
        beCtx.pBE->pfnBETxnAbort(&beCtx);
    }
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return dwError;

error:
    dwError = LDAP_OPERATIONS_ERROR;
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s", VDIR_SAFE_STRING(pszLocalErrorMsg) );
    goto cleanup;
#else
    return 0;
#endif
}

static
DWORD
_VmDirOpenDbEnv()
{
    DWORD dwError = 0;
    unsigned int    envFlags = 0;
    mdb_mode_t      oflags;
    uint64_t        db_max_mapsize = BE_MDB_ENV_MAX_MEM_MAPSIZE;
    DWORD           db_max_size_mb = 0;
    PSTR            pszLocalErrorMsg = NULL;
#ifndef _WIN32
    const char  *dbHomeDir = VMDIR_DB_DIR;
    char dbSnapshotDir[VMDIR_MAX_FILE_NAME_LEN] = {0};
    char dbSnapshotDb[VMDIR_MAX_FILE_NAME_LEN] = {0};
    char homeDb[VMDIR_MAX_FILE_NAME_LEN] = {0};
    char tmpBuf[(VMDIR_MAX_FILE_NAME_LEN<<1) + 3] = {0};
    const char fileSeperator = '/';
    unsigned long long snapshot_lasttid = 0, lasttid = 0;
#else
    _TCHAR      dbHomeDir[MAX_PATH];
    dwError = VmDirMDBGetHomeDir(dbHomeDir);
    BAIL_ON_VMDIR_ERROR ( dwError );
#endif

    VmDirLog( LDAP_DEBUG_TRACE, "MDBInitializeDB: Begin, DB Home Dir = %s", dbHomeDir );

    /* FROM mdb.h
     * The size should be a multiple of the OS page size. The default is
     * 10485760 bytes. The size of the memory map is also the maximum size
     * of the database. The value should be chosen as large as possible,
     * to accommodate future growth of the database.
     *
     * // TODO, this is also the max size of database (per logical mdb db or the total dbs)
     */

     dwError = VmDirGetMaxDbSizeMb(&db_max_size_mb);
     if (dwError != 0)
     {
         VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Use default max-database-size %llu", BE_MDB_ENV_MAX_MEM_MAPSIZE);
     } else
     {
         db_max_mapsize = (uint64_t)(db_max_size_mb)*1024*1024;
         if (db_max_mapsize < BE_MDB_ENV_MAX_MEM_MAPSIZE)
         {
             db_max_mapsize = BE_MDB_ENV_MAX_MEM_MAPSIZE;
             VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL, "RegKey %s value (%u) is too small. Use default max-database-size %llu",
                            VMDIR_REG_KEY_MAXIMUM_DB_SIZE_MB, db_max_size_mb, db_max_mapsize);
         } else
         {
             VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "max-database-size is set to %llu per RegKey %s",
                            db_max_mapsize, VMDIR_REG_KEY_MAXIMUM_DB_SIZE_MB);
         }
     }

#ifdef MDB_NOTLS
     envFlags = MDB_NOTLS; // Required for versions of mdb which have this flag
#endif

     // this is experimental from mdb.h comments
     //envFlags = MDB_FIXEDMAP;        /* use a fixed address for the mmap region */

     //envFlags |= MDB_NOSYNC       need sync for durability
     //envFlags |= MDB_RDONLY       need to open for read and write

    /* Open the environment.  */

#ifndef _WIN32
    oflags = O_RDWR;
#else
    oflags = GENERIC_READ|GENERIC_WRITE;
    goto open_default;
#endif

#ifndef _WIN32

    //Check if snapshot database file exists.
    dwError = VmDirStringPrintFA(dbSnapshotDir, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s", dbHomeDir, fileSeperator, "snapshot");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA(dbSnapshotDb, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s", dbSnapshotDir, fileSeperator, "data.mdb");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = access(dbSnapshotDb, R_OK|W_OK);
    if(dwError != 0)
    {
        //Snapshot database not found, open the default database file.
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirOpenDbEnv: snapshot database not exist; use default databsae file.");
        goto open_default;
    }

    //A snapshot database file exists, determine which one is newer, check the default first.
    dwError = _VmdirCreateDbEnv(db_max_mapsize);
    BAIL_ON_VMDIR_ERROR ( dwError );

    dwError = mdb_env_open (gVdirMdbGlobals.mdbEnv, dbSnapshotDir, envFlags, oflags );
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg),
            "_VmDirOpenDbEnv: mdb_env_open failed on database %s, MDB error %d", dbSnapshotDir, dwError);

    snapshot_lasttid = mdb_env_get_lasttid(gVdirMdbGlobals.mdbEnv);
    mdb_env_close(gVdirMdbGlobals.mdbEnv);
    gVdirMdbGlobals.mdbEnv = NULL;

    dwError = _VmdirCreateDbEnv(db_max_mapsize);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = mdb_env_open (gVdirMdbGlobals.mdbEnv, dbHomeDir, envFlags, oflags );
    if (dwError == 0)
    {
        lasttid = mdb_env_get_lasttid(gVdirMdbGlobals.mdbEnv);
        if(lasttid >= snapshot_lasttid)
        {
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
               "_VmDirOpenDbEnv: use default database file; its tid (%llu) >= snapshot tid (%llu)",
               lasttid, snapshot_lasttid);
            goto cleanup;
        }
        mdb_env_close(gVdirMdbGlobals.mdbEnv);
        gVdirMdbGlobals.mdbEnv = NULL;
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
          "_VmDirOpenDbEnv: use snapshot database; its tid (%llu) > default (%llu)",
          snapshot_lasttid, lasttid);
    } else
    {
         VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
          "_VmDirOpenDbEnv: fail to open default database; use snapshot database; last tid (%llu)", snapshot_lasttid);
    }

    // First try to save the default database file
    dwError = VmDirStringPrintFA(tmpBuf, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s",
                dbHomeDir, fileSeperator, "data.mdb.saved");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA(homeDb, VMDIR_MAX_FILE_NAME_LEN, "%s%c%s",
                             dbHomeDir, fileSeperator, "data.mdb");
    BAIL_ON_VMDIR_ERROR(dwError);

    rename(homeDb, tmpBuf);

    // Move snapshot database to default location
    dwError = rename(dbSnapshotDb, homeDb);
    if (dwError != 0)
    {
        if (errno == EXDEV)
        {
            //rename() would fail if the two files reside at different file systems, try system()
            dwError = VmDirStringPrintFA(tmpBuf, sizeof(tmpBuf), "mv %s %s", dbSnapshotDb, homeDb);
            BAIL_ON_VMDIR_ERROR(dwError);
            dwError = system(tmpBuf);
            if (dwError == 0)
            {
                goto open_default;
            }
        }
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg),
           "_VmDirOpenDbEnv: cannot rename or mv database file from: %s to %s errno %d; " \
           " please move snaphot database file manually, and then restart the server",
           dbSnapshotDb, homeDb, errno);
    }
#endif

open_default:
    dwError = _VmdirCreateDbEnv(db_max_mapsize);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = mdb_env_open (gVdirMdbGlobals.mdbEnv, dbHomeDir, envFlags, oflags );
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, (pszLocalErrorMsg), "_VmDirOpenDbEnv: open database at %d failed", dbHomeDir);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s", VDIR_SAFE_STRING(pszLocalErrorMsg));
    goto cleanup;
}

static
DWORD
_VmdirCreateDbEnv(uint64_t db_max_mapsize)
{
    DWORD dwError = 0;

    /* Create the environment */
    dwError = mdb_env_create ( &gVdirMdbGlobals.mdbEnv );
    BAIL_ON_VMDIR_ERROR( dwError );

    dwError = mdb_env_set_maxreaders( gVdirMdbGlobals.mdbEnv, BE_MDB_ENV_MAX_READERS );
    BAIL_ON_VMDIR_ERROR( dwError );

    dwError = mdb_env_set_mapsize( gVdirMdbGlobals.mdbEnv, db_max_mapsize);
    BAIL_ON_VMDIR_ERROR( dwError );

    dwError = mdb_env_set_maxdbs ( gVdirMdbGlobals.mdbEnv, BE_MDB_ENV_MAX_DBS );
    BAIL_ON_VMDIR_ERROR( dwError );
cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "_VmdirCreateDbEnv failed with db_max_mapsize %lld, MDB error %d",
                     db_max_mapsize, dwError);
    dwError = LDAP_OPERATIONS_ERROR;
    goto cleanup;
}
