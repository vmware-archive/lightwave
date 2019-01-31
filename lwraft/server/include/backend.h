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
 * Module Name: Directory Backend
 *
 * Filename: backend.h
 *
 * Abstract:
 *
 * backend interface api
 *
 */

#ifndef __VIDRBACKEND_H__
#define __VIDRBACKEND_H__

#ifdef __cplusplus
extern "C" {
#endif


#define BE_REAL_EID_SIZE(eId)  8 /* (eId <= 0xff ? 1 : (eId <= 0xffff ? 2 : (eId <= 0xffffff ? 3 : (eId <= 0xffffffff ?
                                    4 : 5)))) */

#define BE_MAX_DB_NAME_LEN              100

#define BE_DB_FLAGS_ZERO                0
#define BE_DB_TXN_NULL                  NULL
#define BE_DB_PARENT_TXN_NULL           NULL


#define ENTRY_ID_SEQ_INITIAL_VALUE      100
#define ENTRY_ID_SEQ_MAX_VALUE          LONG_MAX
#define ENTRY_ID_SEQ_CACHE_SIZE         100
#define ENTRY_ID_SEQ_KEY                "entryIdSeq"

#define USN_SEQ_INITIAL_VALUE           100
#define USN_SEQ_MAX_VALUE               LONG_MAX /* on a 64 bit machine, it is 9,223,372,036,854,775,807 =>
                                                 With 100 updates/sec in the system, it will take 2924712086 years
                                                 to reach this limit */
#define USN_SEQ_CACHE_SIZE              100
#define USN_SEQ_KEY                     "usnSeq"

#define BE_INDEX_KEY_TYPE_FWD               '0'
#define BE_INDEX_KEY_TYPE_REV               '1'

#define BE_INDEX_OP_TYPE_CREATE             0x0001
#define BE_INDEX_OP_TYPE_DELETE             0x0002
#define BE_INDEX_OP_TYPE_UPDATE             0x0004
#define BE_INDEX_OP_TYPE_UPDATE_SINGLE      0x0008

// meta data buffer allocation size
#define BE_DB_META_NODE_INIT_SIZE           100
#define BE_DB_META_NODE_INC_SIZE            50

#define BE_CANDIDATES_START_ALLOC_SIZE      10

#define BE_OUTSTANDING_USN_LIST_SIZE        32

#define LOG1_DB_PATH LWRAFT_DB_DIR"/postlog1"
#define LOG2_DB_PATH LWRAFT_DB_DIR"/postlog2"
#define MAIN_DB_DIR  "post"
#define LOG1_DB_DIR  "postlog1"
#define LOG2_DB_DIR  "postlog2"

/*
 * name aliases of databases in post instance
 * these are used for selecting a particular database for
 * a specific operation.
*/
#define ALIAS_MAIN         "_backend_main"
#define ALIAS_LOG_CURRENT  "_backend_log_current"
#define ALIAS_LOG_PREVIOUS "_backend_log_previous"

typedef enum
{
    VDIR_BACKEND_ENTRY_LOCK_READ = 0,
    VDIR_BACKEND_ENTRY_LOCK_WRITE

} VDIR_BACKEND_ENTRY_LOCKTYPE;

typedef enum
{
    VDIR_BACKEND_TXN_READ = 0,
    VDIR_BACKEND_TXN_WRITE

} VDIR_BACKEND_TXN_MODE;

/* opaque handle for a database entry */
typedef PVOID PVDIR_DB_HANDLE;

typedef struct _VDIR_BACKEND_INDEX_ITERATOR
{
    PVOID   pIterator;
    BOOLEAN bHasNext;

} VDIR_BACKEND_INDEX_ITERATOR, *PVDIR_BACKEND_INDEX_ITERATOR;

typedef struct _VDIR_BACKEND_ENTRYBLOB_ITERATOR
{
    PVOID   pIterator;
    BOOLEAN bHasNext;
    ENTRYID startEID;
    ENTRYID maxEID;

} VDIR_BACKEND_ENTRYBLOB_ITERATOR, *PVDIR_BACKEND_ENTRYBLOB_ITERATOR;

typedef struct _VDIR_BACKEND_USN_LIST*          PVDIR_BACKEND_USN_LIST;

/*
 * Next backend generated entry id
 * return error -
 * ERROR_BACKEND_ERROR:             all others
 */
typedef DWORD (*PFN_BACKEND_MAX_ENTRY_ID)(
                    PVDIR_BACKEND_INTERFACE pBE,
                    ENTRYID*                pEId
                    );
/*
 * Convenient id to entry lookup
 * return error -
 * ERROR_BACKEND_ENTRY_NOT_FOUND:   entry not found
 * ERROR_BACKEND_DEADLOCK:          deadlock
 * ERROR_BACKEND_ERROR:             all others
 */
typedef DWORD (*PFN_BACKEND_SIMPLE_ID_TO_ENTRY)(
                    PVDIR_BACKEND_INTERFACE pBE,
                    ENTRYID                 eId,
                    PVDIR_ENTRY             pEntry
                    );

/*
 * Convenient Dn to entry lookup
 * return error -
 * ERROR_BACKEND_ENTRY_NOT_FOUND:   entry not found
 * ERROR_BACKEND_DEADLOCK:          deadlock
 * ERROR_BACKEND_ERROR:             all others
 */
typedef DWORD (*PFN_BACKEND_SIMPLE_DN_TO_ENTRY)(
                    PVDIR_BACKEND_INTERFACE pBE,
                    PSTR                    pszObjectDn,
                    PVDIR_ENTRY             pEntry
                    );

/*
 * ID to entry lookup
 * return error -
 * ERROR_BACKEND_ENTRY_NOT_FOUND:   entry not found
 * ERROR_BACKEND_DEADLOCK:          deadlock
 * ERROR_BACKEND_ERROR:             all others
 */
typedef DWORD (*PFN_BACKEND_ID_TO_ENTRY)(
                    PVDIR_BACKEND_CTX           pBECtx,
                    PVDIR_SCHEMA_CTX            pSchemaCtx,
                    ENTRYID                     eId,
                    PVDIR_ENTRY                 pEntry,
                    VDIR_BACKEND_ENTRY_LOCKTYPE lockType
                    );
/*
 * DN to entry lookup
 * return error -
 * ERROR_BACKEND_ENTRY_NOT_FOUND:   entry not found
 * ERROR_BACKEND_DEADLOCK:          deadlock
 * ERROR_BACKEND_ERROR:             all others
 */
typedef DWORD (*PFN_BACKEND_DN_TO_ENTRY)(
                    PVDIR_BACKEND_CTX           pBECtx,
                    PVDIR_SCHEMA_CTX            pSchemaCtx,
                    VDIR_BERVALUE*              pDn,
                    PVDIR_ENTRY                 pEntry,
                    VDIR_BACKEND_ENTRY_LOCKTYPE lockType
                    );
/*
 * DN to entry id lookup
 * return error -
 * ERROR_BACKEND_ENTRY_NOT_FOUND:   entry not found
 * ERROR_BACKEND_DEADLOCK:          deadlock
 * ERROR_BACKEND_ERROR:             all others
 */
typedef DWORD (*PFN_BACKEND_DN_TO_ENTRY_ID)(
                    PVDIR_BACKEND_CTX           pBECtx,
                    VDIR_BERVALUE*              pDn,
                    ENTRYID*                    pEId
                    );

/*
 * ObjectGUID to entry id lookup
 * return error -
 * ERROR_BACKEND_ENTRY_NOT_FOUND:   entry not found
 * ERROR_BACKEND_DEADLOCK:          deadlock
 * ERROR_BACKEND_ERROR:             all others
 */
typedef DWORD (*PFN_BACKEND_OBJECTGUID_TO_ENTRY_ID)(
                    PVDIR_BACKEND_CTX           pBECtx,
                    PCSTR                       pszObjectGUID,
                    ENTRYID*                    pEId
                    );


/*
 * Add entry
 * return error -
 * ERROR_BACKEND_CONSTRAINT:        attribute value constraint violation
 * ERROR_BACKEND_PARENT_NOTFOUND:   parent not exists
 * ERROR_BACKEND_ENTRY_EXISTS:      entry already exists
 * ERROR_BACKEND_DEADLOCK:          deadlock
 * ERROR_BACKEND_ERROR:             all others
 */
typedef DWORD (*PFN_BACKEND_ENTRY_ADD)(
                    PVDIR_BACKEND_CTX   pBECtx,
                    PVDIR_ENTRY         pEntry
                    );
/*
 * Check DN reference
 * return error -
 * ERROR_BACKEND_CONSTRAINT:        reference dn not found
 * ERROR_BACKEND_DEADLOCK:          deadlock
 * ERROR_BACKEND_ERROR:             all others
 */
typedef DWORD (*PFN_BACKEND_CHK_DN_REFERENCE)(
                    PVDIR_BACKEND_CTX   pBECtx,
                    PVDIR_ENTRY         pEntry
                    );
/*
 * Delete entry
 * return error -
 * ERROR_BACKEND_DEADLOCK:          deadlock
 * ERROR_BACKEND_ERROR:             all others
 */
typedef DWORD (*PFN_BACKEND_ENTRY_DELETE)(
                    PVDIR_BACKEND_CTX   pBECtx,
                    PVDIR_MODIFICATION  pMods,
                    PVDIR_ENTRY         pEntry
                    );
/*
 * Modify entry
 * return error -
 * ERROR_BACKEND_CONSTRAINT:        attribute value constraint violation
 * ERROR_BACKEND_DEADLOCK:          deadlock
 * ERROR_BACKEND_ERROR:             all others
 */
typedef DWORD (*PFN_BACKEND_ENTRY_MODIFY)(
                    PVDIR_BACKEND_CTX   pBECtx,
                    VDIR_MODIFICATION*  pMods,
                    PVDIR_ENTRY         pEntry
                    );
/*
 * Check if leaf entry
 * return error -
 * ERROR_BACKEND_DEADLOCK:          deadlock
 * ERROR_BACKEND_ERROR:             all others
 */
typedef DWORD (*PFN_BACKEND_CHK_IS_LEAF_ENTRY)(
                    PVDIR_BACKEND_CTX   pBECtx,
                    ENTRYID             eId,
                    PBOOLEAN            pIsLeafEntry
                    );
/*
 * Add indices for a range of entries
 * return error -
 * ERROR_BACKEND_ENTRY_NOTFOUND:    no entry found
 * ERROR_BACKEND_ERROR:             all others
 */
typedef DWORD (*PFN_BACKEND_GET_CANDIDATES)(
                    PVDIR_BACKEND_CTX   pBECtx,
                    VDIR_FILTER*        pFilter,
                    ENTRYID             eStartingId
                    );
/*
 * Begin a transaction
 * return error -
 * ERROR_BACKEND_ERROR:             all others
 */
typedef DWORD (*PFN_BACKEND_TXN_BEGIN)(
                    PVDIR_BACKEND_CTX        pBECtx,
                    VDIR_BACKEND_TXN_MODE    txnMode,
                    PBOOLEAN                 pBnewTxn
                    );
/*
 * Abort a transaction
 * return error -
 * ERROR_BACKEND_ERROR:             all others
 */
typedef DWORD (*PFN_BACKEND_TXN_ABORT)(
                    PVDIR_BACKEND_CTX   pBECtx
                    );


/*
 * get a transaction' context
 * return error -
 * ERROR_BACKEND_ERROR:             all others
 */
typedef DWORD (*PFN_BACKEND_TXN_GETCTX)(
                    PVDIR_BACKEND_CTX   pBECtx
                    );

/*
 * Commit a transaction
 * return error -
 * ERROR_BACKEND_ERROR:             all others
 */
typedef DWORD (*PFN_BACKEND_TXN_COMMIT)(
                    PVDIR_BACKEND_CTX   pBECtx
                    );
/*
 * Initialize backend
 * return error -
 * ERROR_BACKEND_ERROR:             all others
 */
typedef DWORD (*PFN_BACKEND_INIT)(
                    BOOLEAN        bMainDB,
                    PCSTR          pszDbHomeDir,
                    PVDIR_DB_HANDLE *phHandle
                    );
/*
 * Open index database
 * return error -
 * ERROR_BACKEND_ERROR:             all others
 */
typedef DWORD (*PFN_BACKEND_INDEX_OPEN)(
                    PVDIR_BACKEND_INTERFACE pBE,
                    PVDIR_INDEX_CFG         pIndexCfg
                    );
/*
 * Check if index database exists
 * return error -
 * ERROR_BACKEND_ERROR:             all others
 */
typedef BOOLEAN (*PFN_BACKEND_INDEX_EXIST)(
                    PVDIR_BACKEND_INTERFACE pBE,
                    PVDIR_INDEX_CFG         pIndexCfg
                    );
/*
 * Delete index database
 * return error -
 * ERROR_BACKEND_ERROR:             all others
 */
typedef DWORD (*PFN_BACKEND_INDEX_DELETE)(
                    PVDIR_BACKEND_INTERFACE pBE,
                    PVDIR_INDEX_CFG         pIndexCfg
                    );
/*
 * Populate indices with entries in the given range
 * return error -
 * ERROR_BACKEND_MAX_RETRY:         max deadlock retry fail
 * ERROR_DATA_CONSTRAIN_VIOLATION:  data constraint violation
 * ERROR_BACKEND_ERROR:             all others
 */
typedef DWORD (*PFN_BACKEND_INDEX_POPULATE)(
                    PVDIR_BACKEND_INTERFACE pBE,
                    PLW_HASHMAP             pIndexCfgs,
                    PVMDIR_INDEXING_BATCH   pIndexingBatch
                    );
/*
 * Initialize index table iterator
 * return error -
 * ERROR_BACKEND_ERROR:             all others
 */
typedef DWORD (*PFN_BACKEND_INDEX_ITERATOR_INIT)(
                    PVDIR_BACKEND_INTERFACE         pBE,
                    PVDIR_INDEX_CFG                 pIndexCfg,
                    PSTR                            pszInitVal,
                    PVDIR_BACKEND_INDEX_ITERATOR*   ppIterator
                    );
/*
 * Iterate value and eid pairs in the index table
 * return error -
 * ERROR_BACKEND_ERROR:             all others
 */
typedef DWORD (*PFN_BACKEND_INDEX_ITERATE)(
                    PVDIR_BACKEND_INDEX_ITERATOR    pIterator,
                    PSTR*                           ppszVal,
                    ENTRYID*                        pEId
                    );
/*
 * Free index table iterator
 * return error -
 * ERROR_BACKEND_ERROR:             all others
 */
typedef VOID (*PFN_BACKEND_INDEX_ITERATOR_FREE)(
                    PVDIR_BACKEND_INDEX_ITERATOR    pIterator
                    );

/*
 * Initialize blob table iterator
 * return error -
 * ERROR_BACKEND_ERROR:             all others
 */
typedef DWORD (*PFN_BACKEND_ENTRYBLOB_ITERATOR_INIT)(
                    PVDIR_BACKEND_INTERFACE             pBE,
                    ENTRYID                             eId,
                    PVDIR_BACKEND_ENTRYBLOB_ITERATOR*   ppIterator
                    );
/*
 * Iterate eid in the blob table
 * return error -
 * ERROR_BACKEND_ERROR:             all others
 */
typedef DWORD (*PFN_BACKEND_ENTRYBLOB_ITERATE)(
                    PVDIR_BACKEND_ENTRYBLOB_ITERATOR    pIterator,
                    ENTRYID*                            pEntryId
                    );
/*
 * Free blob table iterator
 * return error -
 * ERROR_BACKEND_ERROR:             all others
 */
typedef VOID (*PFN_BACKEND_ENTRYBLOB_ITERATOR_FREE)(
                    PVDIR_BACKEND_ENTRYBLOB_ITERATOR  pIterator
                    );

/*
 * Shutdown backend
 * return error -
 * ERROR_BACKEND_ERROR:             all others
 */
typedef DWORD (*PFN_BACKEND_SHUTDOWN)(
                    PVDIR_DB_HANDLE
                    );

typedef DWORD (*PFN_BACKEND_GET_ATTR_META_DATA)(
                    PVDIR_BACKEND_CTX,
                    VDIR_ATTRIBUTE *,
                    ENTRYID
                    );

typedef DWORD (*PFN_BACKEND_GET_ALL_ATTR_META_DATA)(
                    PVDIR_BACKEND_CTX,
                    ENTRYID,
                    PATTRIBUTE_META_DATA_NODE *,
                    int *
                    );

typedef DWORD (*PFN_BACKEND_GET_NEXT_USN)(
                    PVDIR_BACKEND_CTX,
                    USN *   pUSN);

typedef USN (*PFN_BACKEND_GET_LEAST_OUTSTANDING_USN)(
                    PVDIR_BACKEND_CTX,
                    BOOLEAN);

typedef USN (*PFN_BACKEND_GET_HIGHEST_COMMITTED_USN)(
                    PVDIR_BACKEND_CTX);

typedef USN (*PFN_BACKEND_GET_MAX_ORIGINATING_USN)(
                    PVDIR_BACKEND_CTX);


typedef VOID (*PFN_BACKEND_SET_MAX_ORIGINATING_USN)(
                    PVDIR_BACKEND_CTX, USN);

typedef DWORD (*PFN_BACKEND_DUPKEY_GET_VALUES)(
                    PVDIR_BACKEND_CTX,
                    PCSTR,
                    PVMDIR_STRING_LIST*);

typedef DWORD (*PFN_BACKEND_DUPKEY_SET_VALUES)(
                    PVDIR_BACKEND_CTX,
                    PCSTR,
                    PVMDIR_STRING_LIST);

typedef DWORD (*PFN_BACKEND_UNIQKEY_GET_VALUE)(
                    PVDIR_BACKEND_CTX,
                    PCSTR,
                    PSTR*);

typedef DWORD (*PFN_BACKEND_UNIQKEY_SET_VALUE)(
                    PVDIR_BACKEND_CTX,
                    PCSTR,
                    PCSTR);

typedef DWORD (*PFN_BACKEND_CONFIGURE_FSYNC)(
                    PVDIR_DB_HANDLE,
                    BOOLEAN);

/*******************************************************************************
 * if success, interface function return 0.
 * if fail, interface function return ERROR_BACKEND_XXX. also, VDIR_BACKEND_CTX should
 *      contain backend specific error code and error message.
 ******************************************************************************/
typedef struct _VDIR_BACKEND_INTERFACE
{
    // NOTE: order of fields MUST stay in sync with struct initializer...

    //////////////////////////////////////////////////////////////////////
    // backend life cycle functions
    //////////////////////////////////////////////////////////////////////
    /*
     * initialize backend
     */
    PFN_BACKEND_INIT                pfnBEInit;
    /*
     * shutdown backend
     */
    PFN_BACKEND_SHUTDOWN            pfnBEShutdown;

    //////////////////////////////////////////////////////////////////////
    // index management functions
    //////////////////////////////////////////////////////////////////////
    /*
     * open index database
     */
    PFN_BACKEND_INDEX_OPEN          pfnBEIndexOpen;
    /*
     * check if index exists
     */
    PFN_BACKEND_INDEX_EXIST         pfnBEIndexExist;
    /*
     * delete a index database
     */
    PFN_BACKEND_INDEX_DELETE        pfnBEIndexDelete;
    /*
     * populate indices with entries in the given range
     */
    PFN_BACKEND_INDEX_POPULATE      pfnBEIndexPopulate;
    /*
     * initialize index table enumerator
     */
    PFN_BACKEND_INDEX_ITERATOR_INIT pfnBEIndexIteratorInit;
    /*
     * enumerate value and eid pairs in the index table
     */
    PFN_BACKEND_INDEX_ITERATE       pfnBEIndexIterate;
    /*
     * Free index table enumerator
     */
    PFN_BACKEND_INDEX_ITERATOR_FREE pfnBEIndexIteratorFree;

    //////////////////////////////////////////////////////////////////////
    // EntryBlob iterator
    //////////////////////////////////////////////////////////////////////
    /*
     * initialize blob table enumerator
     */
    PFN_BACKEND_ENTRYBLOB_ITERATOR_INIT   pfnBEEntryBlobIteratorInit;
    /*
     * enumerate eid  in the blob table
     */
    PFN_BACKEND_ENTRYBLOB_ITERATE         pfnBEEntryBlobIterate;
    /*
     * Free blob table enumerator
     */
    PFN_BACKEND_ENTRYBLOB_ITERATOR_FREE   pfnBEEntryBlobIteratorFree;

    //////////////////////////////////////////////////////////////////////
    // transaction related functions
    // NO nested transaction support currently.
    //////////////////////////////////////////////////////////////////////
    /*
     * begin transaction
     */
    PFN_BACKEND_TXN_BEGIN           pfnBETxnBegin;
    /*
     * abort transaction
     */
    PFN_BACKEND_TXN_ABORT           pfnBETxnAbort;
    /*
     * commit transaction
     */
    PFN_BACKEND_TXN_COMMIT          pfnBETxnCommit;

    //////////////////////////////////////////////////////////////////////
    // entry search/read/write functions
    //////////////////////////////////////////////////////////////////////
    /*
     * get entry with ID
     */
    PFN_BACKEND_SIMPLE_ID_TO_ENTRY    pfnBESimpleIdToEntry;
    /*
     * get entry with DN
     */
    PFN_BACKEND_SIMPLE_DN_TO_ENTRY    pfnBESimpleDnToEntry;
    /*
     * get entry with ID
     */
    PFN_BACKEND_ID_TO_ENTRY           pfnBEIdToEntry;
    /*
     * get entry with DN
     */
    PFN_BACKEND_DN_TO_ENTRY           pfnBEDNToEntry;
    /*
     * get ID with DN
     */
    PFN_BACKEND_DN_TO_ENTRY_ID        pfnBEDNToEntryId;
    /*
     * get ID with ObjectGUID
     */
    PFN_BACKEND_OBJECTGUID_TO_ENTRY_ID pfnBEObjectGUIDToEntryId;
    /*
     * verify DN referenced attribute integrity, i.e. referred DN exists
     */
    PFN_BACKEND_CHK_DN_REFERENCE      pfnBEChkDNReference;
    /*
     * check if entry is a leaf node
     */
    PFN_BACKEND_CHK_IS_LEAF_ENTRY     pfnBEChkIsLeafEntry;
    /*
     * get candidate with filter
     */
    PFN_BACKEND_GET_CANDIDATES        pfnBEGetCandidates;
    /*
     * add entry
     */
    PFN_BACKEND_ENTRY_ADD             pfnBEEntryAdd;
    /*
     * delete entry
     */
    PFN_BACKEND_ENTRY_DELETE          pfnBEEntryDelete;
    /*
     * modify entry
     */
    PFN_BACKEND_ENTRY_MODIFY          pfnBEEntryModify;

    //////////////////////////////////////////////////////////////////////
    // entry id (sequence) function
    //////////////////////////////////////////////////////////////////////
    /*
     * get the maximum entry id in the system
     */
    PFN_BACKEND_MAX_ENTRY_ID         pfnBEMaxEntryId;

    //////////////////////////////////////////////////////////////////////
    // function to read attribute meta data
    //////////////////////////////////////////////////////////////////////
    /*
     * Read attribute meta data for a particular attribute of an entry
     */
    PFN_BACKEND_GET_ATTR_META_DATA  pfnBEGetAttrMetaData;

    //////////////////////////////////////////////////////////////////////
    // function to read attribute meta data for all the attributes of an entry
    //////////////////////////////////////////////////////////////////////
    /*
     * Read attribute meta data for all attributes of an entry
     */
    PFN_BACKEND_GET_ALL_ATTR_META_DATA  pfnBEGetAllAttrsMetaData;

    //////////////////////////////////////////////////////////////////////
    // function to read next USN from a sequence (e.g.) in a backend
    //////////////////////////////////////////////////////////////////////
    /*
     * Get next USN from a backend sequence
     */
    PFN_BACKEND_GET_NEXT_USN        pfnBEGetNextUSN;

    //////////////////////////////////////////////////////////////////////
    // generic read/write functions
    //////////////////////////////////////////////////////////////////////
    /*
     * There are two dbs for generic purpose, one allows duplicate keys
     * and the other does not. These dbs compare key lexically.
     *
     * Get values from the duplicate key based db
     */
    PFN_BACKEND_DUPKEY_GET_VALUES           pfnBEDupKeyGetValues;
    /*
     * Set values in the duplicate key based db
     */
    PFN_BACKEND_DUPKEY_SET_VALUES           pfnBEDupKeySetValues;
    /*
     * Get value from the unique key based db
     */
    PFN_BACKEND_UNIQKEY_GET_VALUE           pfnBEUniqKeyGetValue;
    /*
     * Set value in the unique key based db
     */
    PFN_BACKEND_UNIQKEY_SET_VALUE           pfnBEUniqKeySetValue;

    //////////////////////////////////////////////////////////////////////
    // configuration functions
    //////////////////////////////////////////////////////////////////////
    /*
     * Turn fsync on/off
     */
    PFN_BACKEND_CONFIGURE_FSYNC             pfnBEConfigureFsync;

    //////////////////////////////////////////////////////////////////////
    // function to get the least outstanding USN in BACKEND_USN_LIST
    // replication can trust and search USN change below this number
    //////////////////////////////////////////////////////////////////////
    /*
     * Get least outstanding USN
     */
    PFN_BACKEND_GET_LEAST_OUTSTANDING_USN   pfnBEGetLeastOutstandingUSN;

    //////////////////////////////////////////////////////////////////////
    // function to get the highest committed USN
    //////////////////////////////////////////////////////////////////////
    /*
     * Get highest committed USN
     */
    PFN_BACKEND_GET_HIGHEST_COMMITTED_USN   pfnBEGetHighestCommittedUSN;

    //////////////////////////////////////////////////////////////////////
    // function to get the maximum originating USN
    //////////////////////////////////////////////////////////////////////
    /*
     * Get Max Originating USN
     */
    PFN_BACKEND_GET_MAX_ORIGINATING_USN   pfnBEGetMaxOriginatingUSN;

    //////////////////////////////////////////////////////////////////////
    // function to set the maximum originating USN
    //////////////////////////////////////////////////////////////////////
    /*
     * Set Max Originating USN
     */
    PFN_BACKEND_SET_MAX_ORIGINATING_USN   pfnBESetMaxOriginatingUSN;

    //////////////////////////////////////////////////////////////////////
    // Structure to hold all outstanding USNs
    //////////////////////////////////////////////////////////////////////
    PVDIR_BACKEND_USN_LIST                  pBEUSNList;

} VDIR_BACKEND_INTERFACE;

typedef struct _VDIR_BACKEND_INSTANCE
{
    PSTR                                  pszDbPath;
    PVDIR_BACKEND_INTERFACE               pBE;
    PVDIR_DB_HANDLE                       hDB;
} VDIR_BACKEND_INSTANCE, *PVDIR_BACKEND_INSTANCE;

typedef DWORD (*PFN_ITERATE_DB_INSTANCE_CB)(
                    PVDIR_BACKEND_INSTANCE,
                    PVOID pUserData
                    );

typedef struct _VDIR_BACKEND_GLOBALS
{
    /* to support existing code. points to main db backend */
    PVDIR_BACKEND_INTERFACE         pBE;
    /*
     * map from db path to instance. this is the only place
     * an on disk database path to instance data is kept.
     * startup and shutdown relies on this
    */
    PLW_HASHMAP                     pInstanceMap;
    /*
     * map from backend to instance. lookup helper
     * essentially pInstance->pBE to pInstance
     * see shutdown for cleanup sequence.
    */
    PLW_HASHMAP                     pBEToInstanceMap;
    /*
     * map from dn to backend.
     * application specific dn to backend mapping
     * used in select backend
    */
    PLW_HASHMAP                     pDNToBEMap;
//TODO: Not applicable for post. Remove.
    USN                             usnFirstNext;
} VDIR_BACKEND_GLOBALS, *PVDIR_BACKEND_GLOBALS;

// backend.c
DWORD
VmDirBackendConfig(
    VOID);

DWORD
VmDirBackendMapPreviousLogs(
    VOID
    );

VOID
VmDirBackendGetFirstNextUSN(
    USN *pUSN
    );

PVDIR_BACKEND_INTERFACE
VmDirBackendSelect(
    PCSTR   pszDN);

BOOLEAN
VmDirHasBackend(
    PCSTR pszDN
    );

DWORD
VmDirInstanceFromBE(
    PVDIR_BACKEND_INTERFACE pBE,
    PVDIR_BACKEND_INSTANCE *ppInstance
    );

VOID
VmDirBackendContentFree(
    PVDIR_BACKEND_INTERFACE     pBE
    );

DWORD
VmDirBackendCtxMoveContents(
    PVDIR_BACKEND_CTX pBECtxFrom,
    PVDIR_BACKEND_CTX pBECtxTo
    );

VOID
VmDirBackendCtxFree(
    PVDIR_BACKEND_CTX   pBECtx
    );

VOID
VmDirBackendCtxContentFree(
    PVDIR_BACKEND_CTX   pBECtx
    );

VOID
VmDirShutdownAndFreeAllBackends(
    VOID
    );

DWORD
VmDirBackendInitUSNList(
    PVDIR_BACKEND_INTERFACE   pBE
    );

VOID
VmDirBackendSetMaxOutstandingUSN(
    PVDIR_BACKEND_CTX   pBECtx,
    USN                 nextAvailableUSN
    );

DWORD
VmDirBackendAddOutstandingUSN(
    PVDIR_BACKEND_CTX      pBECtx
    );

VOID
VmDirBackendRemoveOutstandingUSN(
    PVDIR_BACKEND_CTX      pBECtx
    );

DWORD
VmDirBackendUniqKeyGetValue(
    PCSTR       pKey,
    PSTR*       ppValue
    );

DWORD
VmDirBackendUniqKeySetValue(
    PCSTR       pKey,
    PCSTR       pValue,
    BOOLEAN     bForce
    );

DWORD
VmDirIterateDBInstances(
    PFN_ITERATE_DB_INSTANCE_CB pfnCB,
    PVOID pUserData
    );

PVDIR_BACKEND_INTERFACE
VmDirSafeBEFromCtx(
    PVDIR_BACKEND_CTX pBECtx
    );

PVDIR_DB_HANDLE
VmDirSafeDBFromCtx(
    PVDIR_BACKEND_CTX pBECtx
    );

PVDIR_DB_HANDLE
VmDirSafeDBFromBE(
    PVDIR_BACKEND_INTERFACE pBE
    );

PVDIR_DB_HANDLE
VmDirSafeDBFromPath(
    PCSTR pszDbPath
    );

// util.c
DWORD
VmDirSimpleNormDNToEntry(
    PCSTR           pszNormDN,
    PVDIR_ENTRY*    ppEntry
    );

DWORD
VmDirSimpleDNToEntry(
    PCSTR           pszDN,
    PVDIR_ENTRY*    ppEntry
    );

DWORD
VmDirIsGroupHasThisMember(
    PVDIR_OPERATION    pOperation, /* Optional */
    PSTR               pszGroupDN,
    PSTR               pszTargetDN,
    PVDIR_ENTRY        pGroupEntry, /* Optional */
    PBOOLEAN           pbIsGroupMember
    );


#ifdef __cplusplus
}
#endif

#endif /* __VIDRBACKEND_H__ */
