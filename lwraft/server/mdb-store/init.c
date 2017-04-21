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
    unsigned int    envFlags = 0;
    mdb_mode_t      oflags;
    uint64_t        db_max_mapsize = BE_MDB_ENV_MAX_MEM_MAPSIZE;
    DWORD           db_max_size_mb = 0;

    // TODO: fix the hard coded Database dir path
#ifndef _WIN32
    const char  *dbHomeDir = LWRAFT_DB_DIR;
#else
    _TCHAR      dbHomeDir[MAX_PATH];
    dwError = VmDirMDBGetHomeDir(dbHomeDir);
    BAIL_ON_VMDIR_ERROR ( dwError );
#endif

    VmDirLog( LDAP_DEBUG_TRACE, "MDBInitializeDB: Begin, DB Home Dir = %s", dbHomeDir );

    dwError = (sizeof(ENTRYID) == sizeof(VDIR_DB_SEQ_T)) ? 0 : ERROR_BACKEND_ERROR;
    BAIL_ON_VMDIR_ERROR( dwError );

    dwError = MDBInitConfig();
    BAIL_ON_VMDIR_ERROR( dwError );

    /* Create the environment */
    dwError = mdb_env_create ( &gVdirMdbGlobals.mdbEnv );
    BAIL_ON_VMDIR_ERROR( dwError );

    dwError = mdb_env_set_maxreaders( gVdirMdbGlobals.mdbEnv, BE_MDB_ENV_MAX_READERS );
    BAIL_ON_VMDIR_ERROR( dwError );

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

     dwError = mdb_env_set_mapsize( gVdirMdbGlobals.mdbEnv, db_max_mapsize);
     BAIL_ON_VMDIR_ERROR( dwError );

     dwError = mdb_env_set_maxdbs ( gVdirMdbGlobals.mdbEnv, BE_MDB_ENV_MAX_DBS );
     BAIL_ON_VMDIR_ERROR( dwError );

     mdb_set_raft_prepare_commit_func(gVdirMdbGlobals.mdbEnv, VmDirRaftPrepareCommit);

     mdb_set_raft_post_commit_func(gVdirMdbGlobals.mdbEnv, VmDirRaftPostCommit);

     mdb_set_raft_commit_fail_func(gVdirMdbGlobals.mdbEnv, VmDirRaftCommitFail);

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
#endif
    dwError = mdb_env_open ( gVdirMdbGlobals.mdbEnv, dbHomeDir, envFlags, oflags );
//TODO, what if open failed?  how to recover??
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
        VmDirAllocateStringAVsnprintf(    &pBECtx->pszBEErrorMsg,
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
    MDB_state_op        op,
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

    if (op < MDB_STATE_CLEAR || op > MDB_STATE_GETXLOGNUM)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *pdwLogNum = 0;
    *pdwDbSizeMb = 0;
    *pdwDbMapSizeMb = 0;
    dwError = mdb_env_set_state(gVdirMdbGlobals.mdbEnv, op, &lognum, &dbSizeMb, &dbMapSizeMb, pszDbPath, dwDbPathSize);
    BAIL_ON_VMDIR_ERROR(dwError);
    *pdwLogNum = lognum;
    *pdwDbSizeMb = dbSizeMb;
    *pdwDbMapSizeMb = dbMapSizeMb;

cleanup:
    return dwError;

error:
    goto cleanup;
}
