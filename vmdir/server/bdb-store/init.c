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

static DB *     l_seqDB = NULL;    // Sequence DB

static int
OpenDBs();

static int
OpenDB(
    DB **               db,
    const char *        dbName,
    const char *        fileName,
    PFN_BT_KEY_CMP      btKeyCmpFcn,
    u_int32_t           extraFlags);

static int
OpenSequence(
    const char *   seqKeyStr,
    db_seq_t       initialValue,
    db_seq_t       maxValue,
    int            cacheSize,
    DB_SEQUENCE ** seq);

static void
CloseDBs();

static int
OpenSequences();

static void
CloseSequences();

static
DWORD
BdbGlobalIndexStructCreate(
    VOID);

static
int
IntegerCompareFcn(DB *db, const DBT *dbt1, const DBT *dbt2);

static
VOID
BdbFreeIndexDBs(
    PVDIR_BDB_INDEX_DB_COLLECTION pBdnIndexCollection
    );

static
VOID
BdbFreeBdbGlobals(
    VOID
    );

DWORD
BdbBEInit ( // beInit
    VOID)
{
    // make sure ENTRYID and db_seq_t have same size
    return (sizeof(ENTRYID) == sizeof(db_seq_t)) ? 0 : ERROR_BACKEND_ERROR;
}

PVDIR_BACKEND_INTERFACE
BdbBEInterface (
    VOID)
{
    static VDIR_BACKEND_INTERFACE bdbBEInterface =
    {
        // NOTE: order of fields MUST stay in sync with struct definition...
        VMDIR_SF_INIT(.pfnBEInit, BdbBEInit),
        VMDIR_SF_INIT(.pfnBEDBOpen, InitializeBDB),
        VMDIR_SF_INIT(.pfnBEIndexOpen, InitializeBDBIndexDB),
        VMDIR_SF_INIT(.pfnBEIndexAdd, NULL),
        VMDIR_SF_INIT(.pfnBEShutdown, ShutdownBDB),
        VMDIR_SF_INIT(.pfnBETxnBegin, BdbTxnBegin),
        VMDIR_SF_INIT(.pfnBETxnAbort, BdbTxnAbort),
        VMDIR_SF_INIT(.pfnBETxnCommit, BdbTxnCommit),
        VMDIR_SF_INIT(.pfnBESimpleIdToEntry, BDBSimpleEIdToEntry),
        VMDIR_SF_INIT(.pfnBESimpleDnToEntry, BDBSimpleDnToEntry),
        VMDIR_SF_INIT(.pfnBEIdToEntry, BdbEIdToEntry),
        VMDIR_SF_INIT(.pfnBEDNToEntry, BdbDNToEntry),
        VMDIR_SF_INIT(.pfnBEDNToEntryId, BdbDNToEntryId),
        VMDIR_SF_INIT(.pfnBEChkDNReference, BdbCheckRefIntegrity),
        VMDIR_SF_INIT(.pfnBEChkIsLeafEntry, BdbCheckIfALeafNode),
        VMDIR_SF_INIT(.pfnBEGetCandidates, BdbGetCandidates),
        VMDIR_SF_INIT(.pfnBEEntryAdd, BdbAddEntry),
        VMDIR_SF_INIT(.pfnBEEntryDelete, BdbDeleteEntry),
        VMDIR_SF_INIT(.pfnBEEntryModify, BdbModifyEntry),
        VMDIR_SF_INIT(.pfnBEEntryAddIndices, VmDirBdbIndicesCreate),
        VMDIR_SF_INIT(.pfnBEMaxEntryId, BdbMaxEntryId),
        VMDIR_SF_INIT(.pfnBEGetAttrMetaData, BdbGetAttrMetaData),
        VMDIR_SF_INIT(.pfnBEGetAllAttrsMetaData, BdbGetAllAttrsMetaData),
        VMDIR_SF_INIT(.pfnBEGetNextUSN, BdbGetNextUSN)
    };

    return &bdbBEInterface;
}

DWORD
InitializeBDB(
    VOID)
{
    int          retVal = 0;
    unsigned int envFlags = 0;
    const char * dbHomeDir = VMDIR_DB_DIR;

    VmDirLog( LDAP_DEBUG_TRACE, "InitializeBDB: Begin, DB Home Dir = %s", dbHomeDir );

    retVal = InitBdbConfig();
    BAIL_ON_VMDIR_ERROR( retVal );

     /* Create the environment. flags = 0 is the only option here. */
     retVal = db_env_create (&gVdirBdbGlobals.bdbEnv, BDB_FLAGS_ZERO);
     BAIL_ON_VMDIR_ERROR( retVal );

    /*
     * Indicate that we want db to perform lock detection internally.
     * Also indicate that the transaction with the fewest number of
     * locks will receive the deadlock notification in the event of a deadlock.
     */
    retVal = gVdirBdbGlobals.bdbEnv->set_lk_detect (gVdirBdbGlobals.bdbEnv, DB_LOCK_MINLOCKS);
    BAIL_ON_VMDIR_ERROR( retVal );

    envFlags =
            DB_CREATE      |  /* Create the environment if it does not exist */
            DB_RECOVER     |  /* Run normal recovery. */
            DB_INIT_LOCK   |  /* Initialize the locking subsystem */
            DB_INIT_LOG    |  /* Initialize the logging subsystem */
            DB_INIT_TXN    |  /* Initialize the transactional subsystem. This
                              * also turns on logging. */
            DB_INIT_MPOOL  |  /* Initialize the memory pool (in-memory cache) */
            DB_THREAD;         /* Cause the environment to be free-threaded */

    /* Open the environment. Important note: DB_CONFIG file can be used to set various flags for the environment. */
    retVal = gVdirBdbGlobals.bdbEnv->open (gVdirBdbGlobals.bdbEnv, dbHomeDir, envFlags, BDB_FILE_MODE_ZERO);
    BAIL_ON_VMDIR_ERROR( retVal );

    /* Open databases. */
    retVal = OpenDBs();
    BAIL_ON_VMDIR_ERROR( retVal );

    /* Open sequences */
    retVal = OpenSequences();
    BAIL_ON_VMDIR_ERROR( retVal );

    /* start checkpoint thread */
    retVal = InitializeDbChkpointThread();
    BAIL_ON_VMDIR_ERROR(retVal);

cleanup:
    VmDirLog( LDAP_DEBUG_TRACE, "InitializeBDB: End" );
    return retVal;

error:
    VmDirLog( LDAP_DEBUG_ANY, "InitializeBDB failed with error code: %d, error string: %s", retVal, db_strerror(retVal) );
    gVdirBdbGlobals.bdbEnv = NULL;
    goto cleanup;
}


/*
 * Initialize Index DB based on the content of cn=indices entry
 */
DWORD
InitializeBDBIndexDB(
    VOID)
{
    DWORD   dwError = 0;
    int     iCnt = 0;

    // Fill gVdirBdbGlobals.bdbIndexDBs contents...
    dwError = BdbGlobalIndexStructCreate();
    BAIL_ON_VMDIR_ERROR(dwError);

    for (iCnt = 0; iCnt < gVdirBdbGlobals.bdbIndexDBs.usNumIndexAttribute ; iCnt++)
    {
        PVDIR_BDB_INDEX_DATABASE pDbDesc = gVdirBdbGlobals.bdbIndexDBs.pIndexDBs + iCnt;

        dwError = OpenDB(
                &pDbDesc->pBdbDataFiles[0].pDB,
                pDbDesc->pBdbDataFiles[0].pszDBName,
                pDbDesc->pBdbDataFiles[0].pszDBFile,
                pDbDesc->btKeyCmpFcn,
                pDbDesc->pBdbDataFiles[0].bIsUnique ? BDB_FLAGS_ZERO : DB_DUP);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
ShutdownBDB(
    VOID)
{
    VmDirLog( LDAP_DEBUG_TRACE, "ShutdownBDB: Begin" );

    int retVal = 0;

    CloseSequences();
    CloseDBs();
    if (gVdirBdbGlobals.bdbEnv != NULL)
    {
        // Meaning of flag: When closing each database handle internally, synchronize the database.
        retVal = gVdirBdbGlobals.bdbEnv->close (gVdirBdbGlobals.bdbEnv, DB_FORCESYNC);
    }

    BdbFreeBdbGlobals();

    VmDirLog( LDAP_DEBUG_TRACE, "ShutdownBDB: End" );

    return 0;
}

/*
 * Add a new index database to bdb backend.
 * 1. next slot in gVdirBdbGlobals.bdbIndexDBs.pIndexDBs is taken
 * 2. gVdirBdbGlobals.bdbIndexDBs.usNumIndexAttribute++
 *
 * NO access protection for gVdirBdbGlobals.bdbIndexDBs.* as new indices always
 * use next available slot (pIndexDesc->iId).
 */
DWORD
VmDirBDBGlobalIndexStructAdd(
    PVDIR_CFG_ATTR_INDEX_DESC   pIndexDesc)
{
    DWORD                   dwError = 0;
    PVDIR_SCHEMA_CTX        pSchemaCtx = NULL;
    PVDIR_SCHEMA_AT_DESC    pATDesc = NULL;

    PVDIR_BDB_INDEX_DATABASE pIndexDB = NULL;

    if (pIndexDesc->iId >= BDB_MAX_INDEX_ATTRIBUTE)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pIndexDB = gVdirBdbGlobals.bdbIndexDBs.pIndexDBs + pIndexDesc->iId;
    assert(! pIndexDB->pBdbDataFiles);

    // add VDIR_CFG_BDB_DATAFILE_DESC and its content
    dwError = VmDirAllocateMemory(
            sizeof(VDIR_CFG_BDB_DATAFILE_DESC) * 1,
            (PVOID)&pIndexDB->pBdbDataFiles);
    BAIL_ON_VMDIR_ERROR(dwError);

    pIndexDB->usNumDataFiles = 1;
    //pIndexDB->pszAttrName not currently used as we use one db only scheme

    dwError = VmDirAllocateStringA(
            pIndexDesc->pszAttrName,
            &pIndexDB->pBdbDataFiles[0].pszDBName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
            "vmdir.db",
            &pIndexDB->pBdbDataFiles[0].pszDBFile);
    BAIL_ON_VMDIR_ERROR(dwError);

    pIndexDB->pBdbDataFiles[0].bIsUnique = pIndexDesc->bIsUnique;

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    if ((pATDesc = VmDirSchemaAttrNameToDesc(pSchemaCtx, pIndexDesc->pszAttrName)) != NULL &&
        pATDesc->pszOrderingMRName != NULL &&
        VmDirStringCompareA( pATDesc->pszOrderingMRName, "integerOrderingMatch", TRUE
                /* SJ-TBD should use VDIR_MATCHING_RULE_INTEGER_ORDERING_MATCH */ ) == 0)
    {
        pIndexDB->btKeyCmpFcn = IntegerCompareFcn;
    }

    // open bdb index data file
    dwError = OpenDB(
            &pIndexDB->pBdbDataFiles[0].pDB,
            pIndexDB->pBdbDataFiles[0].pszDBName,
            pIndexDB->pBdbDataFiles[0].pszDBFile,
            pIndexDB->btKeyCmpFcn,
            pIndexDB->pBdbDataFiles[0].bIsUnique ? BDB_FLAGS_ZERO : DB_DUP);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }
    return dwError;

error:

    goto cleanup;
}

DWORD
BdbToBackendError(
    DWORD   dwBdbError,
    DWORD   dwFromBdbError,
    DWORD   dwToBEError,
    PVDIR_BACKEND_CTX   pBECtx)
{
    DWORD   dwError = 0;

    assert(pBECtx);

    if (dwBdbError != 0)
    {
        if (pBECtx->dwBEErrorCode == 0)
        {
            pBECtx->dwBEErrorCode = dwBdbError;
        }

        if (dwBdbError == DB_LOCK_DEADLOCK)
        {
            dwError = ERROR_BACKEND_DEADLOCK;
        }
        else if (dwBdbError == dwFromBdbError)
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

/* The bt_compare_fcn function must return an integer value less than, equal to, or greater than zero if the first key
 * parameter is considered to be respectively less than, equal to, or greater than the second key parameter. In
 * addition, the comparison function must cause the keys in the database to be well-ordered. The comparison function
 * must correctly handle any key values used by the application (possibly including zero-length keys). In addition,
 * when Btree key prefix comparison is being performed (see DB->set_bt_prefix() for more information), the comparison
 * routine may be passed a prefix of any database key. The data and size  fields of the DBT are the only fields that
 * may be used for the purposes of this comparison, and no particular alignment of the memory to which by the data
 * field refers may be assumed.
 */
static
int
IntegerCompareFcn(DB *db, const DBT *dbt1, const DBT *dbt2)
{
    /* SJ-TBD: Should try to use the schema function:
     * static
     * BOOLEAN
     * compareIntegerString(
     *      VDIR_SCHEMA_MATCH_TYPE type,
     *      PBerval    pAssert,
     *      PBerval    pBerv
     *      )
     */

    if (dbt1->size < dbt2->size)
    {
        return -1;
    }
    if (dbt1->size > dbt2->size)
    {
        return 1;
    }
    return (memcmp( dbt1->data, dbt2->data, dbt1->size));
}

/*
 * Called during server startup, so it is safe to access gVdirBdbGlobals
 * w/o protection.
 */
static int
OpenDBs()
{
    int retVal = 0;

    VmDirLog( LDAP_DEBUG_TRACE, "OpenDBs: Begin" );

    /* Open Sequence DB. */
    /* BDB_FLAGS_ZERO => no duplicate records allowed. */
    OpenDB( &l_seqDB, SEQ_DB_NAME, VMDIR_DB_FILE_NAME, NULL /* BDB Key compare fcn */, BDB_FLAGS_ZERO );
    BAIL_ON_VMDIR_ERROR( retVal );

    // Open Entry DB (BDB_FLAGS_ZERO => no duplicate records allowed.)
    OpenDB( &gVdirBdbGlobals.bdbEntryDB.pBdbDataFiles[0].pDB,
            gVdirBdbGlobals.bdbEntryDB.pBdbDataFiles[0].pszDBName,
            gVdirBdbGlobals.bdbEntryDB.pBdbDataFiles[0].pszDBFile,
            gVdirBdbGlobals.bdbEntryDB.btKeyCmpFcn,
            BDB_FLAGS_ZERO);
    BAIL_ON_VMDIR_ERROR( retVal );

cleanup:
    VmDirLog( LDAP_DEBUG_TRACE, "OpenDBs: End" );
    return retVal;

error:
    goto cleanup;
}

static int
OpenDB(
    DB **               db,
    const char *        dbName,
    const char *        fileName,
    PFN_BT_KEY_CMP      btKeyCmpFcn,
    u_int32_t           extraFlags)
{
     int        retVal = 0;
     u_int32_t  openFlags = 0;
     DB*        pDb = NULL;

     VmDirLog( LDAP_DEBUG_TRACE, "OpenDB: Begin, DN name = %s", fileName );

     if (db == NULL)
     {
         VmDirLog( LDAP_DEBUG_ANY, "openDB: NULL input 1st parameter." );
         retVal = ERROR_INVALID_PARAMETER;
         BAIL_ON_VMDIR_ERROR(retVal);

     }

     *db = NULL;

     /* Initialize the DB handle. Only option for flags is: DB_XA_CREATE */
     retVal = db_create (&pDb, gVdirBdbGlobals.bdbEnv, BDB_FLAGS_ZERO);
     BAIL_ON_VMDIR_ERROR(retVal);

     if (extraFlags != 0)
     {
          retVal = pDb->set_flags (pDb, extraFlags);
          BAIL_ON_VMDIR_ERROR(retVal);
     }

     if (btKeyCmpFcn != NULL)
     {
         retVal = pDb->set_bt_compare(pDb, btKeyCmpFcn);
         BAIL_ON_VMDIR_ERROR(retVal);
     }

     /* Now open the database.
      * A comment on DB_AUTO_COMMIT flag: Allows auto commit of the open operation. If no transaction handle is
      * specified (as here in the open call), but the DB_AUTO_COMMIT flag is specified (as here in the open call),
      * the operation (open) will be implicitly transaction protected. Note that transactionally protected operations
      * on a DB handle requires the DB handle itself to be transactionally protected during its open.
      *
      * Another note on the flags related to isolation levels: Transaction isolation level related flags during DB open
      * call set the "support" for that isolation level, actual isolation level is set at the txn_begin() call time.
      * */

     openFlags = DB_CREATE              | /* Allow database creation */
                 DB_AUTO_COMMIT         |
                 DB_THREAD;               /* Cause the database to be free-threaded */

     retVal = pDb->open(
                     pDb,                /* Pointer to the database */
                     BDB_TXN_NULL,       /* Txn pointer */
                     fileName,           /* File name */
                     dbName,             /* Logical db name, for multiple databases in a single file scenario.*/
                     DB_BTREE,           /* Database type (using btree) */
                     openFlags,          /* Open flags */
                     BDB_FILE_MODE_ZERO  /* File mode. Using defaults */
                     );
     BAIL_ON_VMDIR_ERROR(retVal);

     *db = pDb;

cleanup:
    VmDirLog( LDAP_DEBUG_TRACE, "OpenDB: End" );
    return retVal;

error:
    VmDirLog( LDAP_DEBUG_ANY, "OpenDB failed with error code: %d, error string: %s", retVal, db_strerror(retVal) );
    goto cleanup;
}

/*
 * Called during server shutdown, so it is safe to access gVdirBdbGlobals
 * w/o protection.
 */
static void
CloseDBs()
{
    int i = 0;
    int iNumIdxDB = gVdirBdbGlobals.bdbIndexDBs.usNumIndexAttribute;
    DB* pDB = NULL;
    int retVal = 0;

    VmDirLog( LDAP_DEBUG_TRACE, "CloseDBs: Begin" );

    /* Close Seq DB. Only option for flags is: DB_NOSYNC, Do not flush cached information to disk.*/
    if (l_seqDB != NULL)
    {
        retVal = l_seqDB->close (l_seqDB, BDB_FLAGS_ZERO);
    }

    if (gVdirBdbGlobals.bdbEntryDB.pBdbDataFiles)
    {
        // close entry db
        pDB = gVdirBdbGlobals.bdbEntryDB.pBdbDataFiles[0].pDB;
        assert(pDB);
        retVal = pDB->close(pDB, BDB_FLAGS_ZERO);
    }

    if (gVdirBdbGlobals.bdbIndexDBs.pIndexDBs)
    {
        /* Close Indexed attributes DBs */
        for (i=0; i < iNumIdxDB; i++)
        {
            pDB = gVdirBdbGlobals.bdbIndexDBs.pIndexDBs[i].pBdbDataFiles[0].pDB;
            assert(pDB);
            retVal = pDB->close(pDB, BDB_FLAGS_ZERO);
        }
    }

    VmDirLog( LDAP_DEBUG_TRACE, "CloseDBs: End" );
}

static int
OpenSequences()
{
    int        retVal = 0;

    VmDirLog( LDAP_DEBUG_TRACE, "OpenSequences: Begin" );

    retVal = OpenSequence( ENTRY_ID_SEQ_KEY, ENTRY_ID_SEQ_INITIAL_VALUE, ENTRY_ID_SEQ_MAX_VALUE,
                           ENTRY_ID_SEQ_CACHE_SIZE, &gVdirBdbGlobals.bdbEidSeq );
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = OpenSequence( USN_SEQ_KEY, USN_SEQ_INITIAL_VALUE, USN_SEQ_MAX_VALUE,
                           USN_SEQ_CACHE_SIZE, &gVdirBdbGlobals.bdbUsnSeq );
    BAIL_ON_VMDIR_ERROR(retVal);

cleanup:
    VmDirLog( LDAP_DEBUG_TRACE, "OpenSequences: End" );
    return retVal;

error:
    goto cleanup;
}

static int
OpenSequence(
    const char *   seqKeyStr,
    db_seq_t       initialValue,
    db_seq_t       maxValue,
    int            cacheSize,
    DB_SEQUENCE ** seq)
{
    int             retVal = 0;
    DBT             seqKey = {0};
    DB_SEQUENCE *   pSeq = NULL;

    VmDirLog( LDAP_DEBUG_TRACE, "OpenSequence: Begin" );

    *seq = NULL;

    seqKey.data = (void *)seqKeyStr;
    seqKey.size = VmDirStringLenA(seqKeyStr);

    // 0 is the only option for flags.
    retVal = db_sequence_create (&pSeq, l_seqDB, BDB_FLAGS_ZERO);
    BAIL_ON_VMDIR_ERROR(retVal);

    // This call is only effective when the sequence is being created.
    retVal = pSeq->initial_value (pSeq, initialValue);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = pSeq->set_range (pSeq, initialValue, maxValue);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = pSeq->set_cachesize (pSeq, cacheSize);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = pSeq->open (pSeq, NULL, &seqKey, DB_CREATE | DB_THREAD);
    BAIL_ON_VMDIR_ERROR(retVal);

    *seq = pSeq;

cleanup:
    VmDirLog( LDAP_DEBUG_TRACE, "OpenSequence: End" );
    return retVal;

error:
    VmDirLog( LDAP_DEBUG_ANY, "OpenSequence() failed with error code: %d, error string: %s", retVal, db_strerror(retVal) );
    goto cleanup;
}

static void
CloseSequences()
{
    VmDirLog( LDAP_DEBUG_TRACE, "CloseSequences: Begin" );

    if (gVdirBdbGlobals.bdbEidSeq != NULL)
    {
        gVdirBdbGlobals.bdbEidSeq->close (gVdirBdbGlobals.bdbEidSeq, BDB_FLAGS_ZERO);  // flags = 0 is the only option.
    }

    if (gVdirBdbGlobals.bdbUsnSeq != NULL)
    {
        gVdirBdbGlobals.bdbUsnSeq->close (gVdirBdbGlobals.bdbUsnSeq, BDB_FLAGS_ZERO);  // flags = 0 is the only option.
    }

    VmDirLog( LDAP_DEBUG_TRACE, "CloseSequences: End" );
}

/*
 * Get a list of index descriptor from AttrIndexCache to prepare
 * gVdirBdbGlobals.bdbIndexDBs.*  contents.
 *
 * Note: after this call, bdbIndexDBs.* contents will NOT change except when
 * adding a new index.  In that case,
 * 1. usNumIndexAttribute will increase and
 * 2. next slot in pIndexDBs (VDIR_BDB_INDEX_DATABASE) will be taken
 */
static
DWORD
BdbGlobalIndexStructCreate(
    VOID)
{
    DWORD       dwError = 0;
    int         iCnt = 0;
    USHORT      usSize = 0;
    PVDIR_CFG_ATTR_INDEX_DESC pIndexDesc = NULL;
    PVDIR_SCHEMA_CTX        pSchemaCtx = NULL;
    PVDIR_SCHEMA_AT_DESC    pATDesc = NULL;
    int                     i = 0;
    int                     dbNameLen = 0;

    dwError = VmDirAttrIndexDescList(
            &usSize,
            &pIndexDesc);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaCtxAcquire( &pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

    for (iCnt=0; iCnt < usSize; iCnt++)
    {
        PVDIR_BDB_INDEX_DATABASE pIndexDB =
                gVdirBdbGlobals.bdbIndexDBs.pIndexDBs + pIndexDesc[iCnt].iId;

        if (iCnt == gVdirBdbGlobals.bdbIndexDBs.usMaxSize - 1)
        {
            dwError = ERROR_INVALID_CONFIGURATION;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        dwError = VmDirAllocateMemory(
                sizeof(VDIR_CFG_BDB_DATAFILE_DESC) * 1,
                (PVOID)&pIndexDB->pBdbDataFiles);
        BAIL_ON_VMDIR_ERROR(dwError);

        pIndexDB->usNumDataFiles = 1;
        //pIndexDB->pszAttrName not currently used as we use one db only scheme

        dwError = VmDirAllocateStringA(
                pIndexDesc[iCnt].pszAttrName,
                &pIndexDB->pBdbDataFiles[0].pszDBName);
        BAIL_ON_VMDIR_ERROR(dwError);

        // Normalize pszDBName to all lower case.
        dbNameLen = VmDirStringLenA( pIndexDB->pBdbDataFiles[0].pszDBName );
        for (i = 0; i < dbNameLen; i++)
        {
            pIndexDB->pBdbDataFiles[0].pszDBName[i] = tolower( pIndexDB->pBdbDataFiles[0].pszDBName[i] );
        }

        dwError = VmDirAllocateStringA(
                VMDIR_DB_FILE_NAME,
                &pIndexDB->pBdbDataFiles[0].pszDBFile);
        BAIL_ON_VMDIR_ERROR(dwError);

        pIndexDB->pBdbDataFiles[0].bIsUnique = pIndexDesc[iCnt].bIsUnique;

        if ((pATDesc = VmDirSchemaAttrNameToDesc( pSchemaCtx, pIndexDesc[iCnt].pszAttrName )) != NULL &&
            pATDesc->pszOrderingMRName != NULL &&
            VmDirStringCompareA( pATDesc->pszOrderingMRName, "integerOrderingMatch", TRUE
                    /* SJ-TBD should use VDIR_MATCHING_RULE_INTEGER_ORDERING_MATCH */ ) == 0)
        {
            pIndexDB->btKeyCmpFcn = IntegerCompareFcn;
        }
    }

    gVdirBdbGlobals.bdbIndexDBs.usNumIndexAttribute = usSize;

cleanup:
    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }

    return dwError;

error:

    goto cleanup;
}

static
VOID
BdbFreeIndexDBs(
    PVDIR_BDB_INDEX_DB_COLLECTION pBdnIndexCollection
    )
{
    int i = 0;

    if (pBdnIndexCollection)
    {
        for (; i<pBdnIndexCollection->usNumIndexAttribute; i++)
        {
            VDIR_BDB_INDEX_DATABASE IndexDBs = pBdnIndexCollection->pIndexDBs[i];

            VMDIR_SAFE_FREE_MEMORY((IndexDBs.pBdbDataFiles)->pszDBFile);
            VMDIR_SAFE_FREE_MEMORY((IndexDBs.pBdbDataFiles)->pszDBName);
            VMDIR_SAFE_FREE_MEMORY(IndexDBs.pBdbDataFiles);

            VMDIR_SAFE_FREE_MEMORY(IndexDBs.pszAttrName);
        }
        VMDIR_SAFE_FREE_MEMORY(pBdnIndexCollection->pIndexDBs);
    }
}


static
VOID
BdbFreeBdbGlobals(
    VOID
    )
{
    // Free gVdirBdbGlobals
    VMDIR_SAFE_FREE_MEMORY(gVdirBdbGlobals.bdbEntryDB.pBdbDataFiles->pszDBFile);
    VMDIR_SAFE_FREE_MEMORY(gVdirBdbGlobals.bdbEntryDB.pBdbDataFiles->pszDBName);
    VMDIR_SAFE_FREE_MEMORY(gVdirBdbGlobals.bdbEntryDB.pBdbDataFiles);

    BdbFreeIndexDBs(&gVdirBdbGlobals.bdbIndexDBs);
}
